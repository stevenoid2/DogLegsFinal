//#define PWM_SETTING_MIN 108 // This generates a 0.5 msec pulse
#define PWM_SETTING_MIN 124 // This is min with 99 steps of 4 per step
//#define PWM_SETTING_MIN 216 // This generates a 1.0 msec pulse
#define PWM_SETTING_MID 324 // This generates a 1.5 msec pulse
//#define PWM_SETTING_MAX 432 // This generates a 2.0 msec pulse
#define PWM_SETTING_MAX 520 // This is 124 + (99*4), i.e. 99 steps
//#define PWM_SETTING_MAX 540 // This generates a 2.5 msec pulse

#define QX4_MIN         0
#define QX4_MID         50
#define QX4_MAX         99
#define QX4_STEP_SIZE   4

#define Qx4ToQx1(Qx4Val) (PWM_SETTING_MIN +(Qx4Val*4))

#define CONTROLLER_X    0
#define CONTROLLER_Y    1

#define HANDED_LEFT     0
#define HANDED_RIGHT    1

#define PWM_STEP_SIZE   1
#define INTERSTEP_DELAY 1
#define INTERCASE_DELAY 5

#define POSITION_SIT    0
#define POSITION_STAND  1
#define POSITION_WALK   2

// Define the servo output channels by their index within the servo struct array
#define SERVO_LFC_A 0x00
#define SERVO_LMT_B 0x01
#define SERVO_RFC_C 0x02
#define SERVO_LBT_D 0x03
#define SERVO_RBT_E 0x04
#define SERVO_RMC_F 0x05
#define SERVO_RBC_G 0x06
#define SERVO_RMT_H 0x07
#define SERVO_LBC_I 0x08
#define SERVO_RFT_J 0x09
#define SERVO_LFT_K 0x0A
#define SERVO_LMC_L 0x0B
#define SERVO_LER_M 0x0C
#define SERVO_RER_N 0x0D
#define SERVO_TRL_O 0x0E
#define SERVO_NOT_P 0x0F // Not used

#define SERVO_MIN   SERVO_LFC_A
#define SERVO_MAX   SERVO_NOT_P
#define SERVO_COUNT ((SERVO_MAX-SERVO_MIN)+1)

// These bitmaps are used to set multiple active servos simultaneously
#define SERVO_BITMAP_LFC 0x0001
#define SERVO_BITMAP_LMT 0x0002
#define SERVO_BITMAP_RFC 0x0004
#define SERVO_BITMAP_LBT 0x0008
#define SERVO_BITMAP_RBT 0x0010
#define SERVO_BITMAP_RMC 0x0020
#define SERVO_BITMAP_RBC 0x0040
#define SERVO_BITMAP_RMT 0x0080
#define SERVO_BITMAP_LBC 0x0100
#define SERVO_BITMAP_RFT 0x0200
#define SERVO_BITMAP_LFT 0x0400
#define SERVO_BITMAP_LMC 0x0800
#define SERVO_BITMAP_LER 0x1000
#define SERVO_BITMAP_RER 0x2000
#define SERVO_BITMAP_TRL 0x4000
//#define SERVO_BITMAP_NOT 0x8000 // Not used

// Define the servo output channels by their controller (X or Y) and PCA9685 output 
#define SERVO_X_LFC 0x00 // LFC = Left Front Calf
#define SERVO_X_LMT 0x03 // LMT = Left Middle Thigh
#define SERVO_X_RFC 0x06
#define SERVO_X_LER 0x07
#define SERVO_X_RER 0x08
#define SERVO_X_LBT 0x09
#define SERVO_X_RBT 0x0C // RBT = Right Back Thigh, etc...
#define SERVO_X_RMC 0x0F
#define SERVO_Y_RBC 0x00 
#define SERVO_Y_RMT 0x03 
#define SERVO_Y_LBC 0x06 
#define SERVO_Y_TRL 0x07 
#define SERVO_Y_NOT 0x08 
#define SERVO_Y_RFT 0x09 
#define SERVO_Y_LFT 0x0C
#define SERVO_Y_LMC 0x0F

// These correction values are all 1X
#define LFC_CORRECTION 12   // A
#define LMT_CORRECTION 11   // B
#define RFC_CORRECTION 1   // C
#define LBT_CORRECTION 8    // D
#define RBT_CORRECTION -12  // E
#define RMC_CORRECTION 12  // F
#define RBC_CORRECTION 1   // G
#define RMT_CORRECTION 5    // H
#define LBC_CORRECTION -10  // I
#define RFT_CORRECTION 1   // J
#define LFT_CORRECTION 1   // K
#define LMC_CORRECTION 1  // L
#define LER_CORRECTION 0    // M
#define RER_CORRECTION 0    // N
#define TRL_CORRECTION 8    // O
#define NOT_CORRECTION 0    // P - Not used

#define LFC_MAX_VALUE PWM_SETTING_MAX
#define LMT_MAX_VALUE PWM_SETTING_MAX
#define RFC_MAX_VALUE PWM_SETTING_MAX
#define LBT_MAX_VALUE 444
#define RBT_MAX_VALUE 444
#define RMC_MAX_VALUE PWM_SETTING_MAX
#define RBC_MAX_VALUE PWM_SETTING_MAX
#define RMT_MAX_VALUE PWM_SETTING_MAX
#define LBC_MAX_VALUE PWM_SETTING_MAX
#define RFT_MAX_VALUE PWM_SETTING_MAX
#define LFT_MAX_VALUE PWM_SETTING_MAX
#define LMC_MAX_VALUE PWM_SETTING_MAX
#define LER_MAX_VALUE PWM_SETTING_MAX
#define RER_MAX_VALUE PWM_SETTING_MAX
#define TRL_MAX_VALUE PWM_SETTING_MAX
#define NOT_MAX_VALUE PWM_SETTING_MAX

#define LFC_MIN_VALUE PWM_SETTING_MIN
#define LMT_MIN_VALUE PWM_SETTING_MIN
#define RFC_MIN_VALUE PWM_SETTING_MIN
#define LBT_MIN_VALUE PWM_SETTING_MIN
#define RBT_MIN_VALUE PWM_SETTING_MIN
#define RMC_MIN_VALUE PWM_SETTING_MIN
#define RBC_MIN_VALUE PWM_SETTING_MIN
#define RMT_MIN_VALUE PWM_SETTING_MIN
#define LBC_MIN_VALUE PWM_SETTING_MIN
#define RFT_MIN_VALUE 236
#define LFT_MIN_VALUE 236
#define LMC_MIN_VALUE PWM_SETTING_MIN
#define LER_MIN_VALUE PWM_SETTING_MIN
#define RER_MIN_VALUE PWM_SETTING_MIN
#define TRL_MIN_VALUE PWM_SETTING_MIN
#define NOT_MIN_VALUE PWM_SETTING_MIN

