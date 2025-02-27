/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "main.h"

int target_speed;  // Global target speed

// For use in this file only
void raw_set_intake(int input) {
  for (auto i : intake_motors) {
    i.move_voltage(input * (12000.0 / 127.0));
  }
}

// This is used outside of this file
void set_intake(int input) {
  raw_set_intake(input);
  target_speed = input;
}

// Intake task with antijam logic
void intake_task() {
  const int wait_time = 30;
  const int outtake_time = 300;
  const int min_speed = 20;  // minimum speed to move the intake
  int jam_counter = 0;
  bool is_jammed = false;

  while (true) {
    if(RUN_JAM){
    // Run intake full power in opposite direction for outtake_time ms when jammed, then
    // set intake back to normal
    if (is_jammed) {
      raw_set_intake(-127 * ez::util::sgn(target_speed));
      jam_counter += ez::util::DELAY_TIME;
      if (jam_counter > outtake_time) {
        is_jammed = false;
        jam_counter = 0;
        raw_set_intake(target_speed);
      }
    }

    // Detect a jam if velocity is 0 for wait_time ms
    else if (abs(target_speed) >= min_speed && intake_motors[0].get_actual_velocity() == 0) {
      jam_counter += ez::util::DELAY_TIME;
      if (jam_counter > wait_time) {
        jam_counter = 0;
        is_jammed = true;
      }
    }

    // Reset jam_counter when button is released
    if (target_speed <= min_speed) {
      jam_counter = 0;
    }
    }
    pros::delay(ez::util::DELAY_TIME);
  }
}
pros::Task Intake_Task(intake_task);

// Opcontrol
void intake_opcontrol() {
  if (master.get_digital_new_press(DIGITAL_R1))
    Intake.move(127);
  else if (master.get_digital_new_press(DIGITAL_R2))
    Intake.move(-127);
  else
    Intake.move(0);
}