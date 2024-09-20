#include <iostream>
#include <SDL.h>
#include <cstdint>
#include <stack>
#include <array>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <Windows.h>

int main(int argc, char *argv[])
{

    uint16_t PC = 0x200; // Program Counter
    uint16_t I = 0x0000; // Index register
    std::stack<uint16_t> Stack; // Stack for 16-bit addresses
    uint8_t delayTimer = 0x00; // Delay timer, decriments at 60 Hz until it reaches 0x00
    uint8_t soundTimer = 0x00; // Sound timer, beeps when not 0x00
    std::array<uint8_t, 16> V; // 16 Variable registers
    V.fill(0); // Initialising Variable registers with 0s
    std::array<uint8_t, 4096> RAM; // RAM
    RAM.fill(0); // Initialising RAM with 0s

    // Config variables
    bool legacyShiftBehaviour = false;
    bool legacyJumpWithOffset = false;


    // Create VRAM and initialise with 0s
    std::array<std::array<bool, 32>, 64> VRAM;
    for (int i = 0; i < 64; i++) {
        VRAM[i].fill(0);
    };

    // Keyboard
    std::array<bool, 16> Keyboard;
    Keyboard.fill(false);

    // Clock
    clock_t lastClockTick = clock();
    clock_t lastCPUTick = clock();
    clock_t ClockSpeed = CLOCKS_PER_SEC / 60;
    clock_t CPUClockSpeed = CLOCKS_PER_SEC / 1000;

    // Random
    srand(time(0));

    // Hardcoded font values to be loaded into RAM
    std::array<uint8_t, 80> font = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    // Loading font into RAM from 0x050 to 0x09F
    for (int i = 0; i < font.size(); i++) {
        RAM[0x050 + i] = font[i]; // Set RAM location to font with an offset of 0x050
        //std::cout << "Inserted " << std::hex << (int)font[i] << " at " << std::hex << 0x050 + i << std::endl;
    }

    // Opening ROM
    std::ifstream ROM("Breakou.ch8", std::ios_base::binary);

    ROM.seekg(0, std::ios_base::end);
    auto length = ROM.tellg();
    ROM.seekg(0, std::ios_base::beg);

    std::vector<uint8_t> buffer(length);
    ROM.read(reinterpret_cast<char*>(buffer.data()), length);

    ROM.close();

    // Loading ROM into RAM
    for (int i = 0; i < buffer.size(); i++) {
        RAM[0x200 + i] = buffer[i];
    }

    // Creating SDL window
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(64 * 10, 32 * 10, 0, &window, &renderer);
    SDL_RenderSetScale(renderer, 10, 10);
    SDL_SetWindowTitle(window, "maeve's super awesome and amazing chip-8 emulator :3");

    SDL_Event event;


    /*
    for (int i = 0; i < RAM.size(); i++) {
        std::cout << std::hex << (int)RAM[i];
    }
    std::cout << std::endl;
    */


    // Main update loop
    while (true) {

        if (lastClockTick + ClockSpeed < clock()) {

            lastClockTick = clock();

            // Update Graphics
            SDL_RenderPresent(renderer);

            // Update delay timer
            if (delayTimer != 0) { delayTimer -= 1; };

            // Update sound timer
            if (soundTimer != 0) { 
                soundTimer -= 1;
                //Beep(523, 50);
            };

        }

        if (lastCPUTick + CPUClockSpeed < clock()) {
            
            lastCPUTick = clock();

            // Keyboard Events
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                    case SDLK_1:
                        Keyboard[0x1] = true;
                        break;
                    case SDLK_2:
                        Keyboard[0x2] = true;
                        break;
                    case SDLK_3:
                        Keyboard[0x3] = true;
                        break;
                    case SDLK_4:
                        Keyboard[0xC] = true;
                        break;
                    case SDLK_q:
                        Keyboard[0x4] = true;
                        break;
                    case SDLK_w:
                        Keyboard[0x5] = true;
                        break;
                    case SDLK_e:
                        Keyboard[0x6] = true;
                        break;
                    case SDLK_r:
                        Keyboard[0xD] = true;
                        break;
                    case SDLK_a:
                        Keyboard[0x7] = true;
                        break;
                    case SDLK_s:
                        Keyboard[0x8] = true;
                        break;
                    case SDLK_d:
                        Keyboard[0x9] = true;
                        break;
                    case SDLK_f:
                        Keyboard[0xE] = true;
                        break;
                    case SDLK_z:
                        Keyboard[0xA] = true;
                        break;
                    case SDLK_x:
                        Keyboard[0x0] = true;
                        break;
                    case SDLK_c:
                        Keyboard[0xB] = true;
                        break;
                    case SDLK_v:
                        Keyboard[0xF] = true;
                        break;
                    default:
                        break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                    case SDLK_1:
                        Keyboard[0x1] = false;
                        break;
                    case SDLK_2:
                        Keyboard[0x2] = false;
                        break;
                    case SDLK_3:
                        Keyboard[0x3] = false;
                        break;
                    case SDLK_4:
                        Keyboard[0xC] = false;
                        break;
                    case SDLK_q:
                        Keyboard[0x4] = false;
                        break;
                    case SDLK_w:
                        Keyboard[0x5] = false;
                        break;
                    case SDLK_e:
                        Keyboard[0x6] = false;
                        break;
                    case SDLK_r:
                        Keyboard[0xD] = false;
                        break;
                    case SDLK_a:
                        Keyboard[0x7] = false;
                        break;
                    case SDLK_s:
                        Keyboard[0x8] = false;
                        break;
                    case SDLK_d:
                        Keyboard[0x9] = false;
                        break;
                    case SDLK_f:
                        Keyboard[0xE] = false;
                        break;
                    case SDLK_z:
                        Keyboard[0xA] = false;
                        break;
                    case SDLK_x:
                        Keyboard[0x0] = false;
                        break;
                    case SDLK_c:
                        Keyboard[0xB] = false;
                        break;
                    case SDLK_v:
                        Keyboard[0xF] = false;
                        break;
                    default:
                        break;
                    }
                default:
                    break;
                }
            }

            // Fetch the two bytes from RAM
            uint16_t address1 = RAM[PC];
            uint16_t address2 = RAM[PC + 1];

            // Shift the first and OR them
            address1 = address1 << 8;
            uint16_t instruction = address1 | address2;

            // Immediately increment Program Counter by 2 for opcodes
            PC = PC + 2;

            int VX = (instruction & 0x0F00) >> 8;
            int VY = (instruction & 0x00F0) >> 4;

            switch (instruction & 0xF000) {
            case 0x0000: // Machine language routine
                if (instruction == 0x00E0) { // Clear Screen
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderClear(renderer);
                }
                else if (instruction == 0x00EE) { // Return from subroutine
                    PC = Stack.top(); // Get address from stack
                    Stack.pop(); // Pop address as it is no longer needed
                }
                break;
            case 0x1000: // Jump
                PC = instruction & 0x0FFF; // Get address from instruction
                break;
            case 0x2000: // Call subroutine
                Stack.push(PC); // Push address to stack to return to later
                PC = instruction & 0x0FFF; // Move to subroutine location
                break;
            case 0x3000: // Skip if X == NN
                if (V[VX] == (instruction & 0x00FF)) {
                    PC = PC + 2;
                };
                break;
            case 0x4000: // Skip if X != NN
                if (V[VX] != (instruction & 0x00FF)) {
                    PC = PC + 2;
                };
                break;
            case 0x5000: // Skips if X == Y
                if (V[VX] == V[VY]) {
                    PC = PC + 2;
                };
                break;
            case 0x9000: // Skips if X != Y
                if (V[VX] != V[VY]) {
                    PC = PC + 2;
                };
                break;
            case 0x6000: // Set X to NN
                V[VX] = instruction & 0x00FF;
                break;
            case 0x7000: // Add NN to X
                V[VX] = V[VX] + instruction & 0x00FF;
                break;
            case 0x8000: // Logical and arithmetic instructions

                switch (instruction & 0x000F) {
                case 0x0000: // Set X to Y
                    V[VX] = V[VY];
                    break;
                case 0x0001: // Set X to X | Y
                    V[VX] = V[VX] | V[VY];
                    break;
                case 0x0002: // Set X to X & Y
                    V[VX] = V[VX] & V[VY];
                    break;
                case 0x0003: // Set X to X ^ Y
                    V[VX] = V[VX] ^ V[VY];
                    break;
                case 0x0004: // Set X to X + Y and set F to 1 if overflow or 0 if not
                    if (V[VY] > 0 && V[VX] > (0xFF - V[VY])) {
                        V[0xF] = 1;
                    }
                    else {
                        V[0xF] = 0;
                    };
                    V[VX] = V[VX] + V[VY];
                    break;
                case 0x0005: // Set X to X - Y and set F to 0 if underflow or 1 if not
                    if (V[VX] < V[VY]) {
                        V[0xF] = 0;
                    }
                    else {
                        V[0xF] = 1;
                    };
                    V[VX] = V[VX] - V[VY];
                    break;
                case 0x0007: // Set X to Y - X and set F to 0 if underflow or 1 if not
                    if (V[VY] < V[VX]) {
                        V[0xF] = 0;
                    }
                    else {
                        V[0xF] = 1;
                    };
                    V[VX] = V[VY] - V[VX];
                    break;
                case 0x0006: // Bitshift right
                    if (legacyShiftBehaviour == true) { // Legacy behaviour
                        V[VX] = V[VY];
                    }
                    V[0xF] = V[VX] & 0b00000001; // Set flag register to shifted out bit
                    V[VX] = V[VX] >> 1; // Shift right
                    break;
                case 0x000E: // Bitshift left
                    if (legacyShiftBehaviour == true) { // Legacy behaviour
                        V[VX] = V[VY];
                    }
                    V[0xF] = (V[VX] & 0b10000000) >> 7; // Set flag register to shifted out bit
                    V[VX] = V[VX] << 1;
                    break;
                default:
                    std::cout << "Unknown opcode " << instruction << " at " << PC;
                    break;
                }
                break;
            case 0xA000: // Set I to NNN
                I = instruction & 0x0FFF;
                break;
            case 0xD000: { // Draw sprite to screen
                // Get X and Y coords and wrap if out of bounds
                uint8_t x = V[VX] & 63;
                uint8_t y = V[VY] & 31;
                uint8_t n = instruction & 0x000F;
                // Set flag register to 0
                V[0xF] = 0;

                for (int i = 0; i < n; i++) {
                    uint8_t pixels = RAM[I + i];
                    for (int j = 0; j < 8; j++) {
                        if ((pixels & 0b10000000) == 0b10000000) { // Draw pixel to screen if true
                            if (VRAM[x + j][y + i] == 1) { // If already on, turn off
                                VRAM[x + j][y + i] = 0;
                                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                                SDL_RenderDrawPoint(renderer, x + j, y + i);
                                V[0xF] = 1; // Set VF flag on
                            }
                            else {
                                VRAM[x + j][y + i] = 1;
                                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                                SDL_RenderDrawPoint(renderer, x + j, y + i);
                            }
                        }
                        pixels = pixels << 1; // Shift pixels one bit left
                        if ((x + j) == 63) {
                            break;
                        }
                    }
                    if ((y + i) == 31) {
                        break;
                    }
                }
                break; }
            case 0xB000: // Jump with offset
                if (legacyJumpWithOffset == false) {
                    PC = (instruction & 0x0FFF) + V[VX];
                }
                else { // Legacy instruction
                    PC = (instruction & 0x0FFF) + V[0];
                }
                break;
            case 0xC000: // Random number
                V[VX] = (rand() % 256) & (instruction & 0x00FF);
                break;
            case 0xE000: // Key pressed
                if ((instruction & 0x00FF) == 0x009E) { // Skip if key down
                    if (Keyboard[VX] == true) {
                        PC = PC + 2;
                    }
                }
                else { // Skip if key not down
                    if (Keyboard[VX] == false) {
                        PC = PC + 2;
                    }
                }
                break;
            case 0xF000: // 0xF000 instructions
                switch (instruction & 0x00FF) {
                case 0x0007: // Set VX to delay timer
                    V[VX] = delayTimer;
                    break;
                case 0x0015: // Set delay timer to VX
                    delayTimer = V[VX];
                    break;
                case 0x0018: // Set sound timer to VX
                    soundTimer = V[VX];
                    break;
                case 0x001E: // Add VX to I
                    I = I + V[VX];
                    break;
                case 0x000A: { // Get Key
                    bool keyDown = false;
                    for (uint8_t i = 0; i < 16; i++) {
                        if (Keyboard[i] == true) {
                            V[VX] = i;
                            keyDown = true;
                            break;
                        }
                    }
                    if (keyDown == false) {
                        PC = PC - 2;
                    }
                    break; }
                case 0x0029: // Font character
                    I = (0x050 + (V[VX] * 5)); // Point I to correct character in font RAM
                    break;
                case 0x0033: // Binary-coded decimal conversion
                    RAM[I] = (V[VX] / 100); // Hundreds digit
                    RAM[I + 1] = ((V[VX] / 10) % 10); // Tens digit
                    RAM[I + 2] = (V[VX] % 10); // Units digit
                    break;
                case 0x0055: // Store V into RAM
                    for (int i = 0; i <= VX; i++) {
                        RAM[I + i] = V[i];
                    }
                    break;
                case 0x0065: // Load RAM into V
                    for (int i = 0; i <= VX; i++) {
                        V[i] = RAM[I + i];
                    }
                    break;
                }
                break;
            default:
                std::cout << "Unknown opcode " << instruction << " at " << PC;
                break;
            }

            if (PC >= 4096) {
                std::cout << std::endl << "PC hit end of RAM";
                break;
            }
        }
    }

    return 0;

}