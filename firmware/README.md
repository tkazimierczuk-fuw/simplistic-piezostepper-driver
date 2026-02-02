# A Simplistic Driver for Slip-Stick Piezo Positioners - firmware

The logic of the device is programmed in Raspberry Pico microcontroller. The software simply monitors the state of the lever switches and accordingly changes the MOSFET states in a pre-defined sequence corresponding to forward or backward movement. Step frequency is fixed and no computer communication is implemented.

## How to use it

The code is a standalone C file that can be compiled with Pico SDK. A simple way to do it under Windows is to (1) install VS Code, (2) install Pico extension in VS Code, (3) create a new C Pico project.

## License & Disclaimer

This project is released under the MIT License. You are free to use, modify, and distribute it according to the terms of that license.

The design is provided as-is, without any warranties or guarantees of performance, safety, or suitability for any particular purpose. This is a DIY project; use it at your own risk.
