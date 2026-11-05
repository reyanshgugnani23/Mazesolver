# Maze Solving Robot (RCBoard)

A left-wall-following maze robot built on the **RCBoard**. It uses three ultrasonic sensors (front, left, right) and a PD controller to stay centred along the wall, and handles junctions, dead ends and stuck situations automatically.

---

## Components

| Qty | Component | Purpose |
|-----|-----------|---------|
| 1 | RCBoard (with `RCBoard.h` library) | Main controller and motor driver |
| 3 | HC-SR04 ultrasonic sensors (or compatible 4-pin) | Front, left and right distance sensing |
| 2 | DC gear motors | Left and right drive |
| 1 | Robot chassis with 2 drive wheels + caster/support wheel | Mounting |
| 1 | Battery pack (as required by your RCBoard and motors) | Power |
| 1 | Onboard push button (`LB`) | Start trigger |
| 2 | Onboard LEDs (`LED1`, `LED2`) | Status indication |
| – | Jumper wires | Connections |

---

## Connections

### Ultrasonic sensors

All three sensors use the RCBoard analog header pins as digital I/O.

| Sensor | Sensor Pin | RCBoard Pin |
|--------|-----------|-------------|
| **Front** | VCC | 5V |
| | GND | GND |
| | TRIG | A0 |
| | ECHO | A1 |
| **Right** | VCC | 5V |
| | GND | GND |
| | TRIG | A2 |
| | ECHO | A3 |
| **Left** | VCC | 5V |
| | GND | GND |
| | TRIG | A4 |
| | ECHO | A5 |

### Motors

Connect the **left motor** and **right motor** to the RCBoard's two motor output terminals. The code calls `Robot.drive(a, b)`, where `a` is the left motor and `b` is the right motor. If the robot drives backwards or turns the wrong way, swap the wires of the affected motor.

### Onboard peripherals (no wiring needed)

| Item | Use in code |
|------|-------------|
| `LED1` | On while waiting for the start button |
| `LED2` | On while the robot is running |
| `LB` button | Press to start the robot (`Robot.waitFor(LB)`) |

---

## Sensor Layout

```
        [FRONT]
          ▲
 [LEFT] ◄─ robot ─► [RIGHT]
```

Mount the front sensor facing forward and the left and right sensors facing sideways, perpendicular to the direction of travel.

---

## How It Works

The robot follows the **left-wall rule**. Each loop it scans all three sensors, then decides:

1. **Left side open** (`ld >= SIDE_OPEN`): move forward a little if the front is clear, then turn left.
2. **Stuck** (front reading unchanged for more than 15 loops): reverse, then turn toward the more open side.
3. **Front clear** (`fd > FRONT_STOP`): follow the walls with PD control and slow down as the front wall gets closer.
4. **Front blocked:**
   - Right open: turn right.
   - Otherwise: U-turn.

After every turn, the robot re-aligns with `fix()` (small corrections until the front is clear) and then `creep()`s forward while wall following.

### Wall following (PD control)

- Both walls visible: error = `left - right`, which centres the robot.
- Only left wall visible: error = `left - SIDE_GAP`.
- Only right wall visible: error = `SIDE_GAP - right`.
- Steering = `KP * error + KD * (change in error)`, applied as a speed difference between the two motors.
- Base speed scales with front distance, from `MIN_SPD` (near a wall) up to `SPEED` (clear path).

### Sensor filtering

- Each sensor keeps its last 3 readings, and the **median** is used to reject noise.
- Echo timeout is 5 ms, so a missed echo reads as `MAXD` (80 cm).
- If a sensor times out right after a very close reading (under 4 cm), it's treated as 1 cm, since ultrasonic sensors often fail when an object is too close.

---

## Tunable Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `SPEED` | 83 | Top forward speed |
| `MIN_SPD` | 55 | Slowest speed when approaching a front wall |
| `TURN_SPD` | 78 | Speed while spinning |
| `MAXD` | 80 | Max distance reported (cm) |
| `FRONT_STOP` | 9 | Front distance treated as blocked (cm) |
| `FRONT_SLOW` | 28 | Distance where slowing begins (cm) |
| `SIDE_OPEN` | 20 | Side distance treated as an opening (cm) |
| `SIDE_GAP` | 10 | Target distance from a single wall (cm) |
| `KP` | 4 | Proportional gain |
| `KD` | 2 | Derivative gain |
| `TURN_MS` | 300 | Duration of a 90° spin (ms) |
| `ADV_MS` | 180 | Forward move before a left turn (ms) |
| `CREEP_MS` | 300 | Wall-following time after a turn (ms) |

`TURN_MS` depends on battery level, wheel grip and `TURN_SPD`, so calibrate it so the robot turns a clean 90°.

---

## Debugging

Set `#define DEBUG 1` in the code, then open the Serial Monitor at **9600 baud**. It prints the readings as:

```
front left right
```

Set it back to `0` for normal runs, since serial printing slows the loop.

---

## Usage

1. Install the RCBoard library and open the sketch in the Arduino IDE.
2. Select the RCBoard and upload the code.
3. Place the robot in the maze, with the left wall on its left side.
4. Press the **LB** button. `LED1` turns off, `LED2` turns on, and after a 1 second delay the robot starts moving.

---

## Libraries

- `RCBoard.h`
- `https://github.com/Techgeeks-store/Blueprint01-Robot-Controller-Board.git`
