#include <stdio.h>
#include <stdint.h>
#define MAXROMSIZE 1000000
#define RAMSIZE 2048
#define NES_ROM_HEADER_SIZE 16

char rom[MAXROMSIZE];

// Initialize array of 2048 bytes to represent the NES memory
unsigned char ram[RAMSIZE];
uint16_t stack[RAMSIZE];
int reg_sp = 0; // Stack pointer

char readByte(uint16_t pc) {
  //char address = (char)(pc & 0xFF);
  //printf("Original PC passed in: 0x%04X\n", pc);
  //printf("Converted PC to 8-bit keeping LSB: 0x%02X\n", address);
  //printf("Reading from ROM: 0x%04X\n", address);

  return (unsigned char)(rom[pc - 0xC000] & 0xFF);
}

char rom[MAXROMSIZE];
uint16_t readWord(uint16_t pc) {
  uint16_t word = (0x00FF & rom[pc - 0xC000]) | (0xFF00 & rom[pc - 0xC000 + 1] << 8);

  //printf("Reading from ROM: %04x\n", word);

  return word;
}

int pushToStack(uint16_t pc) {
  stack[reg_sp++] = pc;
  return reg_sp;
}

uint16_t popFromStack() {
  return stack[reg_sp--];
}

void start() {
  // Initialize CPU registers
  unsigned char reg_a = 0;
  unsigned char reg_x = 0;
  unsigned char reg_y = 0;

  uint16_t reg_pc = 0xC000; // Program counter

  // char reg_p = 0; // Status register
  int flag_carry = 0;
  int flag_zero = 0;
  int flag_interrupt_disable = 0;
  int flag_decimal = 0;
  int flag_overflow = 0;
  int flag_negative = 0;

  unsigned char op; // Current OP code

  //for(;;) {
  for(int i = 0; i < 64; i++) {
    //printf("Tick %d\n", i);
    printf("%04x  A:%02x X:%02x Y:%02x P:%02x SP:%02x\n", reg_pc, reg_a, reg_x, reg_y, reg_p, reg_sp);

    op = readByte(reg_pc++);
    //printf("OP code: %02x\n", op);

    switch(op) {
      case 0x4C: // JMP immediate
        uint16_t new_address = readWord(reg_pc);
        //printf("OP: JMP to %04X\n", new_address);
        reg_pc = new_address;
        break;

      case 0xA2: // LDX immediate
        reg_x = readByte(reg_pc++);
        //printf("OP: LDX with %02X\n", reg_x);
        break;

      case 0x86: // STX
        char new_zero_page_address = readByte(reg_pc++);
        //printf("OP: STX - Store value (%02X) to zero page address (%04X)\n", reg_x, new_zero_page_address);
        ram[new_zero_page_address] = reg_x;
        break;

      case 0x20: // JSR
        pushToStack(reg_pc);
        reg_pc = readWord(reg_pc++);
        //printf("OP: JSR to %02X\n", reg_pc);
        break;

      case 0xEA: // NOP
        //printf("OP: NOP\n");
        break;

      case 0x38: // SEC
        //printf("OP: SEC\n");
        flag_carry = 1;
        break;

      case 0xB0: // BCS
        if (flag_carry) {
          reg_pc += readByte(reg_pc);
        }

        printf("OP: BCS to %02X\n", reg_pc++);
        break;

      case 0x18: // CLC
        printf("OP: CLC\n");

        flag_carry = 0;
        break;

      case 0x90: // BCC
        if (!flag_carry) {
          reg_pc += readByte(reg_pc);
        }

        printf("OP: BCC to %02X\n", reg_pc++);
        break;

      case 0xA9: // LDA
        reg_a = readByte(reg_pc++);
        printf("OP: LDA with %02X\n", reg_a);
        break;

      case 0xF0: // BEQ
        if (flag_zero) {
          reg_pc += readByte(reg_pc);
        }

        printf("OP: BEQ to %02X\n", reg_pc++);
        break;

      case 0xD0: // BNE
        if (!flag_zero) {
          reg_pc += readByte(reg_pc);
        }

        printf("OP: BNE to %02X\n", reg_pc++);
        break;

      case 0x85: // STA
        writetozeropage
        printf("OP: STA to %02X\n", reg_pc++);
        break;

      default:
        printf("OP: Unknown: %02X.\n", op);
        break;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing ROM file, must be passed as first argument.\n");
    return 1;
  }

  printf("\nLoading ROM: %s\n", argv[1]);

  FILE *fp;
  char ch;

  fp = fopen(argv[1], "rb");
  fseek(fp, NES_ROM_HEADER_SIZE, SEEK_SET);

  size_t romSize = fread(rom, sizeof(char), MAXROMSIZE, fp);
  if (ferror(fp) != 0) {
    printf("Error reading file\n", stderr);
    return 1;
  } else {
    printf("ROM loaded, total bytes: %i\n", romSize);
  }

  fclose(fp);
  start();

  return 0;
}
