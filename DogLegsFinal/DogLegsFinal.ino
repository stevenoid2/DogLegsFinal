#include "PCA9685.h"
#include "ServoDefs.h"

#define REPORT_VERBOSE 1

PCA9685 pwmControllerX(Wire, 400000, B000000); // Controller X for front left diagonal
PCA9685 pwmControllerY(Wire, 400000, B000001); // Controller Y for back right diagonal

typedef struct ServoChannel {
  int Controller;   // X (0) or Y (1)
  int ChannelIndex; // Output of a PCA9685, 0..15
  int Handedness;   // Left (0) or Right (1)
  int Correction;   // These three
  int NowSetting;   //   values are
  int TgtSetting;   //   in X1 quantum
  int Qx4Value;     // Here is the X4 public (00..99) value
  String Name; // Three printable chars + NULL termination
} ServoChannel;

ServoChannel ServoStructArray[16];

// Define the position strings used throughout
const char SafePosition[SERVO_COUNT] =  {19,80,19,80,80,0,0,80,0,28,29,0,50,50,50,50};
const char LayPosition[SERVO_COUNT] =   {85,72,80,80,80,15,8,74,9,63,61,17,50,50,50,50};

const char SitPosition[SERVO_COUNT] =   {57,83,57,80,80,29, 3,83, 5,50,50,31,35,45,50,50};
const char Sit2Stand1[SERVO_COUNT] =    {56,81,56,78,78,32, 8,81, 6,52,52,34,35,45,50,50};
const char Sit2Stand2[SERVO_COUNT] =    {55,79,55,76,76,36,13,79,13,54,54,38,35,45,50,50};
const char Sit2Stand3[SERVO_COUNT] =    {54,77,54,74,74,41,21,77,21,56,56,42,35,45,50,50};
const char Sit2Stand4[SERVO_COUNT] =    {52,75,52,72,72,46,29,75,29,58,58,47,35,45,50,50};
const char Sit2Stand5[SERVO_COUNT] =    {50,73,50,71,71,51,37,73,37,60,60,52,35,45,50,50};
const char Sit2Stand6[SERVO_COUNT] =    {47,71,47,70,70,56,45,71,45,62,62,56,35,45,50,50};
const char Sit2Stand7[SERVO_COUNT] =    {44,69,44,69,69,62,53,69,53,63,63,62,35,45,50,50};
const char Sit2Stand8[SERVO_COUNT] =    {40,68,40,68,68,68,61,68,61,64,64,68,35,45,50,50};
const char Sit2Stand9[SERVO_COUNT] =    {36,67,36,67,67,73,69,67,69,65,65,73,35,45,50,50};
const char StandPosition[SERVO_COUNT] = {31,66,31,66,66,77,77,66,77,66,66,77,35,45,50,50};
const char Stand2Walk1[SERVO_COUNT] =   {31,73,23,66,73,77,69,66,77,73,66,69,25,35,20,50};
const char Stand2Walk2[SERVO_COUNT] =   {23,66,31,73,66,69,77,73,69,66,73,77,45,55,70,50};

int tempInt;
int finalInt;
bool newIntReceived = false; // Only need to update the display when variable updated
bool LegsMoving;

int PosStateIndex = 0; // This tracks the variable names written in reports
int CurrentPosition;
int SelectedServo = -1; // Invalidate
int NewQx4Value;

char receivedChar;

void setup() {
  Serial.begin(115200);
  while (Serial.available() > 0) { // Purge the RX buffer
    Serial.read();
  }

#if REPORT_VERBOSE
  Serial.println("Dog initialized...");
#endif

  tempInt = -1;
  newIntReceived = false;
  ServoStructArrayInitialize();
  delay(1000); // Give a second for both power switches to be turned ON
  Wire.begin();
  pwmControllerX.resetDevices();       // Resets all PCA9685 devices on i2c line, both X and Y
  pwmControllerX.init();               // Initializes module using default totem-pole driver mode, and default disabled phase balancer
  pwmControllerY.init();
  pwmControllerX.setPWMFreqServo();    // 50Hz provides standard 20ms servo phase length
  pwmControllerY.setPWMFreqServo();
  pwmControllerX.setAllChannelsPWM(PWM_SETTING_MID-1); // These two calls directly
  pwmControllerY.setAllChannelsPWM(PWM_SETTING_MID-1); // set the PWMs in X1
  for (int i = SERVO_MIN; i <= SERVO_MAX; i++) {
    ServoStructArray[i].TgtSetting = PWM_SETTING_MID-1;
    ServoStructArray[i].NowSetting = PWM_SETTING_MID-1;
    ServoStructArray[i].Qx4Value = QX4_MID-1;
  }
  ServoSetAllQx4(QX4_MID); // Step towards true middle in X4 with corrections applied
  delay(1000); // Give a second for servos to get to mid position
  SetTargetString(SitPosition);
  CurrentPosition = POSITION_SIT;
  //SitDown();
}

void loop() {
  
  if (Serial.available() > 0) {
    receivedChar = Serial.read();
    if ((receivedChar >= '0') && (receivedChar <= '9')) {
      if (NewQx4Value < 0) {
        NewQx4Value = 0;
      }
      else {
        NewQx4Value *= 10;
      }
      NewQx4Value += receivedChar - '0';
    }
    else if (receivedChar == '\n') {
      ServoSetSingleQx4(SelectedServo, NewQx4Value);
      NewQx4Value = -1;
    }
    else if ((receivedChar >= 'a') && (receivedChar <= 'p')) {
      SelectedServo = receivedChar - 'a';
    }
    else {
      switch (receivedChar) {
        case 'r': // Print report of servo QX4 settings
          ServoReportAll();
        break;
        case '-': // Decrement QX4 of SelectedServo
          ServoDecrementSingleQx4(SelectedServo);
        break;
        case '+': // Increment QX4 of SelectedServo
        case '=': //   '=' works so no need to shift
          ServoIncrementSingleQx4(SelectedServo);
        break;
        case 'I': // Identify thyself
          Serial.write('l'); // Legs, in this case
        break;
        case 'W': // Walk
          Serial.write('w');
          StartWalk();
        break;
        case 'U': // stand Up
          Serial.write('u');
          StandUp();
        break;
        case 'S': // Sit
          Serial.write('s');
          SitDown();
        break;
        case 'H': // act Happy
          Serial.write('h');
        break;
        case 'B': // scolded Bad
          Serial.write('b');
        break;
        case '\r': // echo and ignore carriage return
          Serial.println(" ");
        break;
        default:
          Serial.write('?'); // Send '?' for unknown command
        break;
      }
    }
  }
  delay(INTERSTEP_DELAY);
  MoveTowardTarget();

}

void SetTargetString(const char * StringPointer) {
  int TempInt;
  for (int i = SERVO_MIN; i < SERVO_COUNT; i++) {
    TempInt = StringPointer[i];
    if (TempInt < QX4_MIN) {
      TempInt = QX4_MIN;
    }
    else if (TempInt > QX4_MAX) {
      TempInt = QX4_MAX;
    }
    ServoStructArray[i].Qx4Value = TempInt;
    TempInt = Qx4ToQx1(TempInt);
    if (ServoStructArray[i].Handedness == HANDED_LEFT) {
      TempInt = (2*PWM_SETTING_MID) - TempInt;
    }
    ServoStructArray[i].TgtSetting = TempInt;
  }
  MoveTowardTarget();
  while (LegsMoving) {
    delay(INTERCASE_DELAY);
    MoveTowardTarget();
  }
}

void SitDown() {
  if (CurrentPosition == POSITION_STAND) {
    SetTargetString(Sit2Stand9);
    SetTargetString(Sit2Stand8);
    SetTargetString(Sit2Stand7);
    SetTargetString(Sit2Stand6);
    SetTargetString(Sit2Stand5);
    SetTargetString(Sit2Stand4);
    SetTargetString(Sit2Stand3);
    SetTargetString(Sit2Stand2);
    SetTargetString(Sit2Stand1);
    SetTargetString(SitPosition);
    CurrentPosition = POSITION_SIT;
  }
}

void StandUp() {
  if (CurrentPosition == POSITION_SIT) {
    SetTargetString(Sit2Stand1);
    SetTargetString(Sit2Stand2);
    SetTargetString(Sit2Stand3);
    SetTargetString(Sit2Stand4);
    SetTargetString(Sit2Stand5);
    SetTargetString(Sit2Stand6);
    SetTargetString(Sit2Stand7);
    SetTargetString(Sit2Stand8);
    SetTargetString(Sit2Stand9);
    SetTargetString(StandPosition);
    CurrentPosition = POSITION_STAND;
  }
}

void StartWalk() {
  if (CurrentPosition == POSITION_STAND) {
    SetTargetString(Stand2Walk1);
    delay(1000);
    SetTargetString(StandPosition);
    delay(1000);
    SetTargetString(Stand2Walk2);
    delay(1000);
    SetTargetString(StandPosition);
    delay(1000);
    SetTargetString(Stand2Walk1);
    delay(1000);
    SetTargetString(StandPosition);
    delay(1000);
    SetTargetString(Stand2Walk2);
    delay(1000);
    SetTargetString(StandPosition);
  }
}

void ServoIncrementSingleQx4(int ServoIndex) {
  int Temp;
  if ((ServoIndex < SERVO_MIN) || (ServoIndex > SERVO_MAX)) {
    return; // Don't handle invalid case
  }
  Temp = ServoStructArray[ServoIndex].Qx4Value + 1;
  if (Temp > QX4_MAX) {
    Temp = QX4_MAX;
  }
  ServoSetSingleQx4(ServoIndex, Temp);
}

void ServoDecrementSingleQx4(int ServoIndex) {
  int Temp;
  Temp = ServoStructArray[ServoIndex].Qx4Value - 1;
  if (Temp < QX4_MIN) {
    Temp = QX4_MIN;
  }
  ServoSetSingleQx4(ServoIndex, Temp);
}

void ServoSetSingleQx4(int ServoIndex, int Value) {
  int TempInt;
  if (Value < QX4_MIN) {
    return; // Don't handle invalid case
  }
  else if (Value > QX4_MAX) {
    return; // Don't handle invalid case
  }
  ServoStructArray[ServoIndex].Qx4Value = Value;
  TempInt = Qx4ToQx1(Value);
  if (ServoStructArray[ServoIndex].Handedness == HANDED_LEFT) {
    TempInt = (2*PWM_SETTING_MID) - TempInt;
  }
  ServoStructArray[ServoIndex].TgtSetting = TempInt;
  ServoReportSingle(ServoIndex);
}

void ServoSetAllQx4(int Value) {
  for (int i = SERVO_MIN; i <= SERVO_MAX; i++) {
    ServoSetSingleQx4(i, Value);
  }
}

void ServoReportSingle(int ServoIndex) {
#if REPORT_VERBOSE
  Serial.print(ServoStructArray[ServoIndex].Name);
  Serial.write(' ');
  Serial.println(ServoStructArray[ServoIndex].Qx4Value);
  //Serial.println(ServoStructArray[ServoIndex].TgtSetting);
#endif
}

void ServoReportAll() {
  Serial.print("const char PosState");
  Serial.print(PosStateIndex++);
  Serial.print("[SERVO_COUNT] = {");
  for (int i = SERVO_MIN; i <= SERVO_MAX-1; i++) {
    Serial.print(ServoStructArray[i].Qx4Value);
    Serial.write(',');
  }
  Serial.print(ServoStructArray[SERVO_MAX].Qx4Value);
  Serial.println("};");
}

bool MoveTowardTarget() {
  LegsMoving = false;
  for (int i = SERVO_MIN; i <= SERVO_MAX; i++) {
    if (ServoStructArray[i].TgtSetting != ServoStructArray[i].NowSetting) {
      LegsMoving = true;
      if (ServoStructArray[i].TgtSetting < ServoStructArray[i].NowSetting) {
        if ((ServoStructArray[i].NowSetting - ServoStructArray[i].TgtSetting) > PWM_STEP_SIZE) {
          ServoStructArray[i].NowSetting -= PWM_STEP_SIZE;
        }
        else {
          ServoStructArray[i].NowSetting = ServoStructArray[i].TgtSetting;
        }
      }
      else {
        if ((ServoStructArray[i].TgtSetting - ServoStructArray[i].NowSetting) > PWM_STEP_SIZE) {
          ServoStructArray[i].NowSetting += PWM_STEP_SIZE;
        }
        else {
          ServoStructArray[i].NowSetting = ServoStructArray[i].TgtSetting;
        }
      }
      if (ServoStructArray[i].Controller == CONTROLLER_X) {
        pwmControllerX.setChannelPWM(
          ServoStructArray[i].ChannelIndex, 
          ServoStructArray[i].NowSetting + ServoStructArray[i].Correction
        );
      }
      else {
        pwmControllerY.setChannelPWM(
          ServoStructArray[i].ChannelIndex,
          ServoStructArray[i].NowSetting + ServoStructArray[i].Correction
        );
      }
    }
  }
  return(LegsMoving);
}

void ServoStructArrayInitialize() {
  ServoStructArray[SERVO_LFC_A].Controller   = CONTROLLER_X;
  ServoStructArray[SERVO_LMT_B].Controller   = CONTROLLER_X;
  ServoStructArray[SERVO_RFC_C].Controller   = CONTROLLER_X;
  ServoStructArray[SERVO_LBT_D].Controller   = CONTROLLER_X;
  ServoStructArray[SERVO_RBT_E].Controller   = CONTROLLER_X;
  ServoStructArray[SERVO_RMC_F].Controller   = CONTROLLER_X;
  ServoStructArray[SERVO_RBC_G].Controller   = CONTROLLER_Y;
  ServoStructArray[SERVO_RMT_H].Controller   = CONTROLLER_Y;
  ServoStructArray[SERVO_LBC_I].Controller   = CONTROLLER_Y;
  ServoStructArray[SERVO_RFT_J].Controller   = CONTROLLER_Y;
  ServoStructArray[SERVO_LFT_K].Controller   = CONTROLLER_Y;
  ServoStructArray[SERVO_LMC_L].Controller   = CONTROLLER_Y;
  ServoStructArray[SERVO_LER_M].Controller   = CONTROLLER_X;
  ServoStructArray[SERVO_RER_N].Controller   = CONTROLLER_X;
  ServoStructArray[SERVO_TRL_O].Controller   = CONTROLLER_Y;
  ServoStructArray[SERVO_NOT_P].Controller   = CONTROLLER_Y;
  ServoStructArray[SERVO_LFC_A].ChannelIndex = SERVO_X_LFC;
  ServoStructArray[SERVO_LMT_B].ChannelIndex = SERVO_X_LMT;
  ServoStructArray[SERVO_RFC_C].ChannelIndex = SERVO_X_RFC;
  ServoStructArray[SERVO_LBT_D].ChannelIndex = SERVO_X_LBT;
  ServoStructArray[SERVO_RBT_E].ChannelIndex = SERVO_X_RBT;
  ServoStructArray[SERVO_RMC_F].ChannelIndex = SERVO_X_RMC;
  ServoStructArray[SERVO_RBC_G].ChannelIndex = SERVO_Y_RBC;
  ServoStructArray[SERVO_RMT_H].ChannelIndex = SERVO_Y_RMT;
  ServoStructArray[SERVO_LBC_I].ChannelIndex = SERVO_Y_LBC;
  ServoStructArray[SERVO_RFT_J].ChannelIndex = SERVO_Y_RFT;
  ServoStructArray[SERVO_LFT_K].ChannelIndex = SERVO_Y_LFT;
  ServoStructArray[SERVO_LMC_L].ChannelIndex = SERVO_Y_LMC;
  ServoStructArray[SERVO_LER_M].ChannelIndex = SERVO_X_LER;
  ServoStructArray[SERVO_RER_N].ChannelIndex = SERVO_X_RER;
  ServoStructArray[SERVO_TRL_O].ChannelIndex = SERVO_Y_TRL;
  ServoStructArray[SERVO_NOT_P].ChannelIndex = SERVO_Y_NOT;
  ServoStructArray[SERVO_LFC_A].Handedness   = HANDED_LEFT;
  ServoStructArray[SERVO_LMT_B].Handedness   = HANDED_LEFT;
  ServoStructArray[SERVO_RFC_C].Handedness   = HANDED_RIGHT;
  ServoStructArray[SERVO_LBT_D].Handedness   = HANDED_LEFT;
  ServoStructArray[SERVO_RBT_E].Handedness   = HANDED_RIGHT;
  ServoStructArray[SERVO_RMC_F].Handedness   = HANDED_RIGHT;
  ServoStructArray[SERVO_RBC_G].Handedness   = HANDED_RIGHT;
  ServoStructArray[SERVO_RMT_H].Handedness   = HANDED_RIGHT;
  ServoStructArray[SERVO_LBC_I].Handedness   = HANDED_LEFT;
  ServoStructArray[SERVO_RFT_J].Handedness   = HANDED_RIGHT;
  ServoStructArray[SERVO_LFT_K].Handedness   = HANDED_LEFT;
  ServoStructArray[SERVO_LMC_L].Handedness   = HANDED_LEFT;
  ServoStructArray[SERVO_LER_M].Handedness   = HANDED_LEFT; // These TBD
  ServoStructArray[SERVO_RER_N].Handedness   = HANDED_LEFT; // These TBD
  ServoStructArray[SERVO_TRL_O].Handedness   = HANDED_LEFT; // These TBD
  ServoStructArray[SERVO_NOT_P].Handedness   = HANDED_LEFT; // These TBD
  ServoStructArray[SERVO_LFC_A].Correction   = LFC_CORRECTION;
  ServoStructArray[SERVO_LMT_B].Correction   = LMT_CORRECTION;
  ServoStructArray[SERVO_RFC_C].Correction   = RFC_CORRECTION;
  ServoStructArray[SERVO_LBT_D].Correction   = LBT_CORRECTION;
  ServoStructArray[SERVO_RBT_E].Correction   = RBT_CORRECTION;
  ServoStructArray[SERVO_RMC_F].Correction   = RMC_CORRECTION;
  ServoStructArray[SERVO_RBC_G].Correction   = RBC_CORRECTION;
  ServoStructArray[SERVO_RMT_H].Correction   = RMT_CORRECTION;
  ServoStructArray[SERVO_LBC_I].Correction   = LBC_CORRECTION;
  ServoStructArray[SERVO_RFT_J].Correction   = RFT_CORRECTION;
  ServoStructArray[SERVO_LFT_K].Correction   = LFT_CORRECTION;
  ServoStructArray[SERVO_LMC_L].Correction   = LMC_CORRECTION;
  ServoStructArray[SERVO_LER_M].Correction   = LER_CORRECTION;
  ServoStructArray[SERVO_RER_N].Correction   = RER_CORRECTION;
  ServoStructArray[SERVO_TRL_O].Correction   = TRL_CORRECTION;
  ServoStructArray[SERVO_NOT_P].Correction   = NOT_CORRECTION;
  ServoStructArray[SERVO_LFC_A].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LMT_B].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RFC_C].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LBT_D].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RBT_E].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RMC_F].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RBC_G].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RMT_H].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LBC_I].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RFT_J].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LFT_K].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LMC_L].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LER_M].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RER_N].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_TRL_O].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_NOT_P].NowSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LFC_A].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LMT_B].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RFC_C].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LBT_D].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RBT_E].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RMC_F].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RBC_G].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RMT_H].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LBC_I].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RFT_J].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LFT_K].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LMC_L].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LER_M].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_RER_N].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_TRL_O].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_NOT_P].TgtSetting   = PWM_SETTING_MID;
  ServoStructArray[SERVO_LFC_A].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_LMT_B].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_RFC_C].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_LBT_D].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_RBT_E].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_RMC_F].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_RBC_G].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_RMT_H].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_LBC_I].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_RFT_J].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_LFT_K].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_LMC_L].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_LER_M].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_RER_N].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_TRL_O].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_NOT_P].Qx4Value     = QX4_MID;
  ServoStructArray[SERVO_LFC_A].Name         = "LFC";
  ServoStructArray[SERVO_LMT_B].Name         = "LMT";
  ServoStructArray[SERVO_RFC_C].Name         = "RFC";
  ServoStructArray[SERVO_LBT_D].Name         = "LBT";
  ServoStructArray[SERVO_RBT_E].Name         = "RBT";
  ServoStructArray[SERVO_RMC_F].Name         = "RMC";
  ServoStructArray[SERVO_RBC_G].Name         = "RBC";
  ServoStructArray[SERVO_RMT_H].Name         = "RMT";
  ServoStructArray[SERVO_LBC_I].Name         = "LBC";
  ServoStructArray[SERVO_RFT_J].Name         = "RFT";
  ServoStructArray[SERVO_LFT_K].Name         = "LFT";
  ServoStructArray[SERVO_LMC_L].Name         = "LMC";
  ServoStructArray[SERVO_LER_M].Name         = "LER";
  ServoStructArray[SERVO_RER_N].Name         = "RER";
  ServoStructArray[SERVO_TRL_O].Name         = "TRL";
  ServoStructArray[SERVO_NOT_P].Name         = "NOT";
}
