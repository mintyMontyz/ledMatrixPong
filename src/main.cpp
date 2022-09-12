#include <Arduino.h>
#include <LedControl.h>

int joystickpin = A1;

int ANIMDELAY = 100;  // animation delay, deafault value is 100
int INTENSITYMIN = 0; // minimum brightness, valid range [0,15]
int INTENSITYMAX = 8; // maximum brightness, valid range [0,15]

byte MAXREG_DECODEMODE = 0x09;
byte MAXREG_INTENSITY  = 0x0a;
byte MAXREG_SCANLIMIT  = 0x0b;
byte MAXREG_SHUTDOWN   = 0x0c;
byte MAXREG_DISPTEST   = 0x0f;

int DIN_PIN = 10;      // DIN connects to pin 10
int CS_PIN = 9;       // CS connects to pin 8 
int CLK_PIN = 8;      // CLK connects to pin 9

int matrix = 0;
int ON = 1;
int OFF = 0;
int WIDTH = (matrix+1) * 8;
int HEIGHT = (matrix+1) * 8;
int dimensions = 8;
int frameRate = 100;
bool GameOver = false;

int BallXpos = 5;
int BallYpos = 3;
int BallXspeed = 1;
int BallYspeed = 1;

int RacketXpos = 1;
int RacketYposSeg1 = 2;
int RacketYposSeg2 = 3;
int RacketYposSeg3 = 4;
int RacketSpeed = 0;

const unsigned char heart[] =
{
  B01100110,
  B11111111,
  B11111111,
  B11111111,
  B01111110,
  B00111100,
  B00011000,
  B00000000
};


LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, 1);

void draw(){
  lc.clearDisplay(0);

  if (GameOver) {
  setRegistry(MAXREG_SCANLIMIT, 0x07);
  setRegistry(MAXREG_DECODEMODE, 0x00);  // using an led matrix (not digits)
  setRegistry(MAXREG_SHUTDOWN, 0x01);    // not in shutdown mode
  setRegistry(MAXREG_DISPTEST, 0x00);    // no display test
  setRegistry(MAXREG_INTENSITY, 0x0f & INTENSITYMIN);

  // draw hearth
  setRegistry(1, heart[0]);
  setRegistry(2, heart[1]);
  setRegistry(3, heart[2]);
  setRegistry(4, heart[3]);
  setRegistry(5, heart[4]);
  setRegistry(6, heart[5]);
  setRegistry(7, heart[6]);
  setRegistry(8, heart[7]);
    
  }
  else {
  
  lc.setLed(matrix, BallYpos, BallXpos, ON);
  lc.setLed(matrix, RacketYposSeg1, RacketXpos, ON);
  lc.setLed(matrix, RacketYposSeg2, RacketXpos, ON);
  lc.setLed(matrix, RacketYposSeg3, RacketXpos, ON);

  delay(frameRate);
}
}

void setRegistry(byte reg, byte value)
{
  digitalWrite(CS_PIN, LOW);

  putByte(reg);   // specify register
  putByte(value); // send data

  digitalWrite(CS_PIN, LOW);
  digitalWrite(CS_PIN, HIGH);
}

void putByte(byte data)
{
  byte i = 8;
  byte mask;
  while (i > 0)
  {
    mask = 0x01 << (i - 1);        // get bitmask
    digitalWrite( CLK_PIN, LOW);   // tick
    if (data & mask)               // choose bit
      digitalWrite(DIN_PIN, HIGH); // send 1
    else
      digitalWrite(DIN_PIN, LOW);  // send 0
    digitalWrite(CLK_PIN, HIGH);   // tock
    --i;                           // move to lesser bit
  }
}

int BallupdateX(int row, int Xvel){
  
    row += Xvel;
    return row;
}

int BallupdateY(int coll, int Yvel){
  
    coll += Yvel;
    return coll;
  }

int hitEnd(int posi, int vel, bool x) {
  if (x == false) {
  if (posi >= dimensions - 1 || posi <= 0) {
    vel = -vel;

  }
  }
  else if (x == true) {
    if (posi >= dimensions -1) {
      vel = -vel;
    }
    if (posi <= 0) {
      GameOver = true;
    }
    
  }
  return vel;
}

void hitRacket(int xpos, int ypos) {
  if (xpos == RacketXpos) {
    if (ypos == RacketYposSeg1 || ypos == RacketYposSeg2 || ypos == RacketYposSeg3) {
      BallXspeed = -BallXspeed;
      BallYspeed = random(-1, 2);
      BallXpos = 1;
    }
  }
}

int Racketupdate(int pos, int velocity) {
  pos += velocity;
  if (pos == 6) {
    pos = 5;
  }
  if (pos == -1) {
    pos = 0;
  }
  return pos;
}

int joystick(){
  if (analogRead(joystick) > 700){
    return 1;
  }
  else if (analogRead(joystick) < 300) {
    return -1;
  }
  else {
    return 0;
  }
}

void setup() {
  pinMode(joystickpin, INPUT);
  
  lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);  
}

void loop(){
  draw();
  
  if (joystick() == 1) {
    RacketSpeed = -1;
  }
  if ( joystick() == -1) {
    RacketSpeed = 1;
  }

  BallXspeed = hitEnd(BallXpos, BallXspeed, true);
  BallYspeed = hitEnd(BallYpos, BallYspeed, false);
  hitRacket(BallXpos, BallYpos);

  RacketYposSeg1 = Racketupdate(RacketYposSeg1, RacketSpeed);
  RacketYposSeg2 = RacketYposSeg1 + 1;
  RacketYposSeg3 = RacketYposSeg2 + 1;

  BallXpos = BallupdateX(BallXpos, BallXspeed);
  BallYpos = BallupdateY(BallYpos, BallYspeed);

  
  RacketSpeed = 0;

}