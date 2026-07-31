#include <CheapStepper.h>
#include <math.h>

CheapStepper stepperA(4, 5, 6, 7);
CheapStepper stepperB(8, 9, 10, 11);

// Hardware pin assignments
const int X_PIN = A0;
const int Y_PIN = A1;
const int SPEED_PIN = A2;
const int SW_PIN = 2;

// Joystick calibration values
int CENTER_X;
int CENTER_Y;
const int DEADZONE = 60;

// Invert joystick axes if needed
const bool INVERT_X = true;
const bool INVERT_Y = true;

// Invert motor directions if needed
const bool INVERT_MOTOR_A = false;
const bool INVERT_MOTOR_B = false;

// Stores the last step time for each motor
unsigned long lastStepA = 0;
unsigned long lastStepB = 0;

void setup() {

  Serial.begin(115200);
  pinMode(SW_PIN, INPUT_PULLUP);

  // Wait for the joystick to be released before calibration
  Serial.println("Joystick loslassen...");

  delay(1000);

  long sumX = 0;
  long sumY = 0;

  // Average multiple readings to find the joystick center
  for (int i = 0; i < 100; i++) {
    sumX += analogRead(X_PIN);
    sumY += analogRead(Y_PIN);
    delay(2);
  }

  CENTER_X = sumX / 100;
  CENTER_Y = sumY / 100;

  Serial.print("CENTER_X = ");
  Serial.println(CENTER_X);

  Serial.print("CENTER_Y = ");
  Serial.println(CENTER_Y);
}

void loop() {

  // Read and normalize the joystick position
  int rawX = analogRead(X_PIN);
  int rawY = analogRead(Y_PIN);

  // Offset from calibrated center
  float x = rawX - CENTER_X;
  float y = rawY - CENTER_Y;

  // Ignore small joystick movements
  if (abs(x) < DEADZONE) x = 0;
  if (abs(y) < DEADZONE) y = 0;

  // Normalize values to roughly -1..1
  x /= 480.0;
  y /= 480.0;

  if (fabs(x) < 0.08) x = 0;
  if (fabs(y) < 0.08) y = 0;

  // Apply optional axis inversion
  if (INVERT_X) x = -x;
  if (INVERT_Y) y = -y;

  // Convert X/Y movement into CoreXY motor movement
  float mixA = x + y;
  float mixB = x - y;

  // Keep motor values within the valid range
  mixA = constrain(mixA, -1.0, 1.0);
  mixB = constrain(mixB, -1.0, 1.0);

  // Read speed potentiometer and calculate motor speed
  float t = analogRead(SPEED_PIN) / 1023.0;

  // Cubic response curve for finer low-speed control
  // Range: 1 ms (fast) to 20 ms (slow)
  int baseInterval = 1 + (int)(19 * t * t * t);

  int intervalA = 99999;
  int intervalB = 99999;

  if (fabs(mixA) > 0.05)
    intervalA = max(1, (int)(baseInterval / fabs(mixA)));

  if (fabs(mixB) > 0.05)
    intervalB = max(1, (int)(baseInterval / fabs(mixB)));

  unsigned long now = millis();

  // Move motor A at the calculated interval
  if (now - lastStepA >= intervalA) {

    lastStepA = now;

    // Determine motor direction
    bool dir = (mixA > 0);

    if (INVERT_MOTOR_A)
      dir = !dir;

    // Execute one step on motor A
    stepperA.step(dir);
  }

  // Move motor B at the calculated interval
  if (now - lastStepB >= intervalB) {

    lastStepB = now;

    bool dir = (mixB > 0);

    if (INVERT_MOTOR_B)
      dir = !dir;

    // Execute one step on motor B
    stepperB.step(dir);
  }
}