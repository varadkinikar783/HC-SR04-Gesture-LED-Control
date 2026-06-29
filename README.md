# HC-SR04-Gesture-LED-Control

Wave your hand in front of a sensor to toggle LEDs on and off. Wave twice to trigger a bouncing light pattern. Wave once to stop it.

---

## DEMO:

[Watch demo video.](https://youtube.com/shorts/6qwCP7jn9BU?feature=share)
---

## Built On

This project is a direct extension of [LED-Toggle-Using-HC-SR04-Sensor](https://github.com/varadkinikar783/LED-Toggle-Using-HC-SR04-Sensor) — an earlier build that used a 3-state FSM and circular buffer to differentiate a hand swipe from a stationary object on a single LED. That project's core problem was solved there; this one takes the gesture input further — multi-swipe detection, five LEDs, and a live wave pattern.

---

## What does it do?

An HC-SR04 ultrasonic sensor watches for hand movements within 20cm. A single swipe toggles all five LEDs on or off. Two quick swipes in a row triggers a wave pattern — one LED at a time bouncing left to right and back, continuously, until a single swipe kills it. No buttons, no physical contact — just gesture.

---

## Hardware

| Component | Quantity |
|---|---|
| Arduino Uno | 1 |
| HC-SR04 Ultrasonic Sensor | 1 |
| LEDs (Green x2, Yellow x1, Red x2) | 5 |
| 220Ω resistors | 5 |

---

## Wiring

| Pin | Connection |
|---|---|
| D2 | HC-SR04 Echo — Interrupt |
| D12 | HC-SR04 Trig |
| D11 | Green LED 1 (220Ω to GND) |
| D10 | Green LED 2 (220Ω to GND) |
| D9 | Yellow LED (220Ω to GND) |
| D6 | Red LED 1 (220Ω to GND) |
| D5 | Red LED 2 (220Ω to GND) |

---

## How it works

The sensor fires a pulse every 60ms. An interrupt on the Echo pin catches the pulse return and calculates distance.

When a hand enters the 20cm zone, it registers as one swipe and starts a 1000ms window. If a second hand entry happens inside that window, swipe count goes to 2. When the window closes, the count decides what fires — 1 swipe toggles, 2 swipes activates the wave.

The wave steps one LED every 150ms and bounces direction at the endpoints. Gesture detection stays live the whole time — a single swipe kills the wave mid-bounce instantly.

---

## Skills Demonstrated

- Hardware interrupts for real-time pulse capture
- Edge detection using a `handWasPresent` flag instead of polling raw distance every frame
- Gesture state machine — swipe counting with debounce and timeout window
- LED animation running in parallel with live sensor input

---

## What Broke and How It Was Fixed

**States locking up after first swipe — Failure 1.** Original design used a 2-element boolean array `swipes[2]` with strict checks like `!swipes[0] && !swipes[1]` gating every function. Once swipe 1 was confirmed, the gate conditions made it impossible for the second swipe to register. Fixed by scrapping the array entirely and replacing it with a simple `swipeCount` integer — no gates, just increment on every detected entry.

**State edges overwriting mid-swipe — Failure 2.** Used a `pre_state / stateC` pattern updating every 100ms sample tick. A swipe slower than one tick caused `pre_state` to catch up to `stateC` mid-gesture, making the system forget the hand was ever there. Fixed by switching to a `handWasPresent` flag that locks on hand entry and only clears on physical exit — immune to sample timing.

**Sensor jamming and CPU freeze — Failure 3.** Sampling faster than 40ms fired a new trigger pulse before the previous echo had returned, leaving the interrupt pin stuck HIGH and freezing the loop. Fixed by setting the sample interval to 60ms, giving sound enough time to travel and return cleanly before the next trigger.

**Function call chain breaking gesture timing — Failure 4.** Early versions had `analyze()` calling `time_C()` which called `Operations()` all in one continuous chain. By the time `Operations()` ran, the swipe state had already shifted and the wrong branch fired. Fixed by moving all outcome decisions into `loop()` directly — functions only update state, `loop()` decides what to do with it.

---

## Why I Built This

The previous project could tell a swipe from a stationary object — that was the hard part. Once that was solved, the natural question was can i assign differnt functions to differnt gestures and as it turns out a single HC-SR04 can distinguish gesture count, timing, and duration with nothing but some flags and a counter😊. Now that i know that it can do this in the futuree i'm thinking about using more gestures as well as sensors to replicate some of the cool projects i have seen online such as a media player controller, making a system that recognizes a set of gestures and sends them to another microprocesser for further processing.

---

## Future Improvements

- Variable wave speed — faster bouncing with each additional double swipe
- Triple swipe for a third pattern
- PWM fade instead of hard on/off for the wave steps
- Port to ESP32-S3 with WebSocket control panel to change gesture sensitivity live

---

## Repository Structure

```
HC-SR04-Gesture-LED-Control/
├── src/
│   └── main.cpp
├── README.md
└── platformio.ini
```
