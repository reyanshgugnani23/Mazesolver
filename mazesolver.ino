#include <RCBoard.h>

RCBoard Robot;

#define TRIG_F A0
#define ECHO_F A1
#define TRIG_L A4
#define ECHO_L A5
#define TRIG_R A2
#define ECHO_R A3
#define DEBUG 0

const int SPEED = 83;
const int MIN_SPD = 55;
const int TURN_SPD = 78;
const int MAXD = 80;
const int FRONT_STOP = 9;
const int FRONT_SLOW = 28;
const int SIDE_OPEN = 20;
const int SIDE_GAP = 10;
const float KP = 4;
const float KD = 2;
const unsigned long TURN_MS = 300;
const unsigned long ADV_MS = 180;
const unsigned long CREEP_MS = 300;

int hist[3][3];
int fd, ld, rd;
int prevErr = 0;
int lastF = 0;
int same = 0;

int ping(int t, int e) {
  digitalWrite(t, LOW);
  delayMicroseconds(4);
  digitalWrite(t, HIGH);
  delayMicroseconds(10);
  digitalWrite(t, LOW);
  long us = pulseIn(e, HIGH, 5000UL);
  if (us == 0) return MAXD;
  return constrain(us / 58, 1, MAXD);
}

int med(int a, int b, int c) {
  return max(min(a, b), min(max(a, b), c));
}

int dist(int i, int t, int e) {
  int d = ping(t, e);
  if (d == MAXD && hist[i][0] < 4) d = 1;
  hist[i][2] = hist[i][1];
  hist[i][1] = hist[i][0];
  hist[i][0] = d;
  return med(hist[i][0], hist[i][1], hist[i][2]);
}

void scan() {
  fd = dist(0, TRIG_F, ECHO_F);
  delay(6);
  ld = dist(1, TRIG_L, ECHO_L);
  delay(6);
  rd = dist(2, TRIG_R, ECHO_R);
#if DEBUG
  Serial.print(fd);
  Serial.print(" ");
  Serial.print(ld);
  Serial.print(" ");
  Serial.println(rd);
#endif
}

void follow() {
  int err = 0;
  if (ld < SIDE_OPEN && rd < SIDE_OPEN) err = ld - rd;
  else if (ld < SIDE_OPEN) err = ld - SIDE_GAP;
  else if (rd < SIDE_OPEN) err = SIDE_GAP - rd;
  err = constrain(err, -10, 10);
  int dErr = constrain(err - prevErr, -5, 5);
  prevErr = err;
  int steer = KP * err + KD * dErr;
  int f = constrain(fd, FRONT_STOP, FRONT_SLOW);
  int spd = map(f, FRONT_STOP, FRONT_SLOW, MIN_SPD, SPEED);
  int a = constrain(spd - steer, 0, 255);
  int b = constrain(spd + steer, 0, 255);
  Robot.drive(a, b);
}

bool stuck() {
  if (fd < FRONT_SLOW && fd == lastF) same++;
  else same = 0;
  lastF = fd;
  return same > 15;
}

void back() {
  Robot.drive(-SPEED, -SPEED);
  delay(250);
  Robot.drive(0, 0);
  same = 0;
}

void spin(int dir, unsigned long ms) {
  prevErr = 0;
  same = 0;
  Robot.drive(0, 0);
  delay(60);
  Robot.drive(-dir * TURN_SPD, dir * TURN_SPD);
  delay(ms);
  Robot.drive(0, 0);
  delay(60);
}

void fix(int dir) {
  for (int i = 0; i < 6; i++) {
    scan();
    if (fd > FRONT_STOP + 3) return;
    spin(dir, 40);
  }
}

void advance() {
  unsigned long t = millis();
  while (millis() - t < ADV_MS) {
    scan();
    if (fd < FRONT_STOP) break;
    Robot.drive(SPEED, SPEED);
  }
}

void creep() {
  unsigned long t = millis();
  while (millis() - t < CREEP_MS) {
    scan();
    if (fd < FRONT_STOP) break;
    follow();
  }
}

void turnLeft() {
  spin(1, TURN_MS);
  fix(1);
  creep();
}

void turnRight() {
  spin(-1, TURN_MS);
  fix(-1);
  creep();
}

void turnAround() {
  spin(-1, TURN_MS * 2);
  fix(-1);
  creep();
}

void setup() {
#if DEBUG
  Serial.begin(9600);
#endif
  Robot.begin();
  pinMode(TRIG_F, OUTPUT);
  pinMode(ECHO_F, INPUT);
  pinMode(TRIG_L, OUTPUT);
  pinMode(ECHO_L, INPUT);
  pinMode(TRIG_R, OUTPUT);
  pinMode(ECHO_R, INPUT);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) hist[i][j] = MAXD;
  }
  Robot.ledOn(LED1);
  Robot.waitFor(LB);
  Robot.ledOff(LED1);
  Robot.ledOn(LED2);
  delay(1000);
}

void loop() {
  scan();
  if (ld >= SIDE_OPEN) {
    if (fd > FRONT_SLOW) advance();
    turnLeft();
    return;
  }
  if (stuck()) {
    back();
    if (ld > rd) turnLeft();
    else turnRight();
    return;
  }
  if (fd > FRONT_STOP) {
    follow();
    return;
  }
  if (rd >= SIDE_OPEN) turnRight();
  else turnAround();
}
