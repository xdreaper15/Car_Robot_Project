#include <Motoron.h>
#include <Servo.h>



class TolerantServo {
  private:
    Servo servo1;
    int currentPos = 90;  // default neutral
    int targetPos = 90;
    int errorRange = 3;   // default ±2 degrees
    int minPos = 40;
    int maxPos = 140;
    int pin = 9;
    int lastPos = 90;
    int cmd = 0;

  public:
    volatile bool IN_MOTION = false;

    // Tolerantservo1(){
    //   Serial.println("Constructor");
    // }

    void attach(int pin) {
      servo1.attach(pin);
      if (servo1.attached()) Serial.println("Attached!");
      else Serial.println("Failed to attach");
      currentPos = getCurrentPos();
      targetPos = currentPos;
      }

    void write(int cmd) {
      currentPos = getCurrentPos();
      if (abs(cmd - currentPos) > errorRange) {
        Serial.print("Pre-write pos: "); Serial.println(servo1.read());
        servo1.write(cmd);
        Serial.print("Post-write pos: "); Serial.println(servo1.read());
        targetPos = cmd;
        Serial.print("Commanded servo to "); Serial.println(targetPos);
        IN_MOTION = true;
      }
    }

    void setErrorRange(int range) {
      errorRange = range;
      Serial.print("Error range set to: "); Serial.println(errorRange);
    }

    int getCurrentPos() {return servo1.read();}
    int getTargetPos() {return targetPos;}
    

};

// Servo Variables
int servo_currentPos = 90;
int servo_pin = 9;
int servo_errorRange = 3;
TolerantServo ts;

// Motor Variables
MotoronI2C mc(16);
int mot_accelDecelValue = 400; //Both motors, same accel and decel
int mot_speed = 10;


void setupServo(){
  ts.attach(servo_pin);
  ts.setErrorRange(servo_errorRange);
}
void servoLoop(){
  servo_currentPos = ts.getCurrentPos();
}

void setupMotors(){

  mc.reinitialize(); // set controller to all default options
  mc.disableCrc();
  // Clear the reset flag, which is set after the controller
  // reinitializes and counts as an error.
  mc.clearResetFlag();
  mc.disableCommandTimeout();  // Disable command timeout

  // Configure motors with the same acceleration and deceleration values
  mc.setMaxAcceleration(2, mot_accelDecelValue);
  mc.setMaxDeceleration(2, mot_accelDecelValue);
  mc.setMaxAcceleration(3, mot_accelDecelValue);
  mc.setMaxDeceleration(3, mot_accelDecelValue);
  // mc.clearMotorFaultUnconditional(); now?
}

void motorLoop(){
  
}

void setup(){
  Wire.begin();
  Serial.begin(9600);
  delay(500);
  setupServo();
  setupMotors();
}
void loop(){

  if (Serial.available() > 0){
    // Read Serial Input without bufferoverflow stuff
    String input = Serial.readStringUntil('\n');

    if ( input.startsWith("m_") ) {
      input.remove(0, 2);
      int newCmd = input.toInt();
      Serial.print("Setting motors to speed: "); Serial.println(newCmd);
      mc.setSpeed(2, newCmd);
      mc.setSpeed(3, -newCmd);

      if ( input.startsWith("e_") ) {
        uint16_t status = mc.getStatusFlags();
        mc.reset();
        Serial.print(F("Controller error: 0x"));
        Serial.println(status, HEX);
        delay(1000);
      }

    }

    if (input.startsWith("s_")) {
      input.remove(0, 2);
      int newCmd = input.toInt();
      ts.write(newCmd);
      Serial.print("Setting servo to position: "); Serial.println(newCmd);
    }

  }

  motorLoop();
  servoLoop();
}

