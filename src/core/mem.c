#include "mem.h"
#include <stdio.h>
#include "log.h"

void init_mem(mem_t* mem) {
  memset(mem->bios, 0, BIOS_SIZE);
  memset(mem->iWRAM, 0, IWRAM_SIZE);
  memset(mem->eWRAM, 0, EWRAM_SIZE);
}

void load_rom(mem_t* mem, const char* path) {
  logdebug("Loading rom: %s\n", path);
  FILE* fp = fopen(path, "rb");
  if(fp == NULL) {
    logfatal("Failed to open rom %s\n", path);
  }

  fseek(fp, 0, SEEK_END);
  size_t rom_size = ftell(fp);
  size_t rounded_rom_size = next_power_of_two(rom_size);
  logdebug("Rom size: %zu (rounded to %zu)\n", rom_size, rounded_rom_size);
  mem->rom_size = rounded_rom_size;
  fseek(fp, 0, SEEK_SET);

  mem->rom = (u8*)malloc(rounded_rom_size);
  if(fread(mem->rom, 1, rom_size, fp) != rom_size) {
    logfatal("Failed to load rom\n");
  }
  memset(&mem->rom[rom_size], 0, rounded_rom_size - rom_size);

  fclose(fp);
}

u8 read_8(mem_t* mem, u32 addr) {
  u8 val = 0xff;
  switch(addr) {
  case 0x00000000 ... 0x00003FFF:
    val = mem->bios[addr];
    logdebug("[INFO][MEM] Read (%02X) from BIOS (%08X)\n", val, addr);
    break;
  case 0x00004000 ... 0x01FFFFFF:
    logdebug("[WARN][MEM] Open bus read! (%08X)(0xFF)\n", addr);
    return 0xff;
  case 0x02000000 ... 0x0203FFFF:
    val = mem->eWRAM[addr & EWRAM_DSIZE];
    logdebug("[INFO][MEM] Read (%02X) from eWRAM (%08X)\n", val, addr);
    break;
  case 0x03000000 ... 0x03FFFFFF:
    val = mem->iWRAM[addr & IWRAM_DSIZE];
    logdebug("[INFO][MEM] Read (%02X) from iWRAM (%08X)\n", val, addr);
    break;
  case 0x04000000 ... 0x04000058:
    val = read8_io_ppu(&mem->ppu, addr);
    logdebug("[INFO][MEM] Read (%02X) from IO (%08X)\n", val, addr);
    break;
  case 0x04000208:
    val = mem->ime;
    logdebug("[INFO][MEM] Read (%02X) from IME (%08X) [WARN][STUB!]\n", val, addr);
    break;
  case 0x05000000 ... 0x050003FF:
    val = mem->ppu.pram[addr & PRAM_DSIZE];
    logdebug("[INFO][MEM] Read (%02X) from PRAM (%08X)\n", val, addr);
    break;
  case 0x06000000 ... 0x06017FFF:
    val = mem->ppu.vram[addr & VRAM_DSIZE];
    logdebug("[INFO][MEM] Read (%02X) from VRAM (%08X)\n", val, addr);
    break;
  case 0x07000000 ... 0x070003FF:
    val = mem->ppu.oam[addr & OAM_DSIZE];
    logdebug("[INFO][MEM] Read (%02X) from OAM (%08X)\n", val, addr);
    break;
  case 0x08000000 ... 0x0DFFFFFF:
    val = mem->rom[addr & (mem->rom_size - 1)];
    logdebug("[INFO][MEM] Read (%02X) from ROM (%08X)\n", val, addr);
    break;
  default:
    logfatal("[ERR ][MEM] Read on unhandled address! (%08X)\n", addr);
  }

  return val;
}

u16 read_16(mem_t* mem, u32 addr) {
  u16 val = 0xffff;
  assert((addr & 1) == 0);
  switch(addr) {
  case 0x00000000 ... 0x00003FFF:
    val = *(u16*)&mem->bios[addr];
    logdebug("[INFO][MEM] Read (%04X) from BIOS (%08X)\n", val, addr);
    break;
  case 0x00004000 ... 0x01FFFFFF:
    logdebug("[WARN][MEM] Open bus read! (%08X)(0xFFFF)\n", addr);
    return 0xffff;
  case 0x02000000 ... 0x0203FFFF:
    val = *(u16*)&mem->eWRAM[addr & EWRAM_DSIZE];
    logdebug("[INFO][MEM] Read (%04X) from eWRAM (%08X)\n", val, addr);
    break;
  case 0x03000000 ... 0x03FFFFFF:
    val = *(u16*)&mem->iWRAM[addr & IWRAM_DSIZE];
    logdebug("[INFO][MEM] Read (%04X) from iWRAM (%08X)\n", val, addr);
    break;
  case 0x04000000 ... 0x04000058:
    val = read16_io_ppu(&mem->ppu, addr);
    logdebug("[INFO][MEM] Read (%04X) from IO (%08X)\n", val, addr);
    break;
  case 0x04000208:
    val = mem->ime;
    logdebug("[INFO][MEM] Read (%02X) from IME (%08X) [WARN][STUB!]\n", val, addr);
    break;
  case 0x05000000 ... 0x050003FF:
    val = *(u16*)&mem->ppu.pram[addr & PRAM_DSIZE];
    logdebug("[INFO][MEM] Read (%04X) from PRAM (%08X)\n", val, addr);
    break;
  case 0x06000000 ... 0x06017FFF:
    val = *(u16*)&mem->ppu.vram[addr & VRAM_DSIZE];
    logdebug("[INFO][MEM] Read (%04X) from VRAM (%08X)\n", val, addr);
    break;
  case 0x07000000 ... 0x070003FF:
    val = *(u16*)&mem->ppu.oam[addr & OAM_DSIZE];
    logdebug("[INFO][MEM] Read (%04X) from OAM (%08X)\n", val, addr);
    break;
  case 0x08000000 ... 0x0DFFFFFF:
    val = *(u16*)&mem->rom[addr & (mem->rom_size - 1)];
    logdebug("[INFO][MEM] Read (%04X) from ROM (%08X)\n", val, addr);
    break;
  default:
    logfatal("[ERR ][MEM] Read on unhandled address! (addr: %08X)\n", addr);
  }

  return val;
}

u32 read_32(mem_t* mem, u32 addr) {
  u32 val = 0xffffffff;
  assert((addr & 3) == 0);
  switch(addr) {
  case 0x00000000 ... 0x00003FFF:
    val = *(u32*)&mem->bios[addr];
    logdebug("[INFO][MEM] Read (%08X) from BIOS (%08X)\n", val, addr);
    break;
  case 0x00004000 ... 0x01FFFFFF:
    logdebug("[WARN][MEM] Open bus read! (%08X)(0xFFFFFFFF)\n", addr);
    return 0xff;
  case 0x02000000 ... 0x0203FFFF:
    val = *(u32*)&mem->eWRAM[addr & EWRAM_DSIZE];
    logdebug("[INFO][MEM] Read (%08X) from eWRAM (%08X)\n", val, addr);
    break;
  case 0x03000000 ... 0x03FFFFFF:
    val = *(u32*)&mem->iWRAM[addr & IWRAM_DSIZE];
    logdebug("[INFO][MEM] Read (%08X) from iWRAM (%08X)\n", val, addr);
    break;
  case 0x04000000 ... 0x04000058:
    val = read32_io_ppu(&mem->ppu, addr);
    logdebug("[INFO][MEM] Read (%08X) from IO (%08X)\n", val, addr);
    break;
  case 0x04000208:
    val = mem->ime;
    logdebug("[INFO][MEM] Read (%02X) from IME (%08X) [WARN][STUB!]\n", val, addr);
    break;
  case 0x05000000 ... 0x050003FF:
    val = *(u32*)&mem->ppu.pram[addr & PRAM_DSIZE];
    logdebug("[INFO][MEM] Read (%08X) from PRAM (%08X)\n", val, addr);
    break;
  case 0x06000000 ... 0x06017FFF:
    val = *(u32*)&mem->ppu.vram[addr & VRAM_DSIZE];
    logdebug("[INFO][MEM] Read (%08X) from VRAM (%08X)\n", val, addr);
    break;
  case 0x07000000 ... 0x070003FF:
    val = *(u32*)&mem->ppu.oam[addr & OAM_DSIZE];
    logdebug("[INFO][MEM] Read (%08X) from OAM (%08X)\n", val, addr);
    break;
  case 0x08000000 ... 0x0DFFFFFF:
    val = *(u32*)&mem->rom[addr & (mem->rom_size - 1)];
    logdebug("[INFO][MEM] Read (%08X) from ROM (%08X)\n", val, addr);
    break;
  default:
    logfatal("[ERR ][MEM] Read on unhandled address! (addr: %08X)\n", addr);
  }

  return val;
}

void write_8(mem_t* mem, u32 addr, u8 val) {
  switch(addr) {
  case 0x00004000 ... 0x01FFFFFF:
    logdebug("[WARN][MEM] Open bus write! (%08X, %02X)\n", addr, val);
    break;
  case 0x02000000 ... 0x0203FFFF:
    logdebug("[INFO][MEM] Write (%02X) to eWRAM (%08X)\n", val, addr);
    mem->eWRAM[addr & EWRAM_DSIZE] = val;
    break;
  case 0x03000000 ... 0x03FFFFFF:
    logdebug("[INFO][MEM] Write (%02X) to iWRAM (%08X)\n", val, addr);
    mem->iWRAM[addr & IWRAM_DSIZE] = val;
    break;
  case 0x04000000 ... 0x04000058:
    logdebug("[INFO][MEM] Write (%02X) to IO (%08X)\n", val, addr);
    write8_io_ppu(&mem->ppu, addr, val);
    break;
  case 0x04000208:
    logdebug("[INFO][MEM] Write (%02X) to IME (%08X) [WARN][STUB!]\n", val, addr);
    mem->ime = val;
    break;
  case 0x05000000 ... 0x050003FF:
    logdebug("[INFO][MEM] Write (%02X) to PRAM (%08X)\n", val, addr);
    mem->ppu.pram[addr & PRAM_DSIZE] = val;
    break;
  case 0x06000000 ... 0x06017FFF:
    logdebug("[INFO][MEM] Write (%02X) to VRAM (%08X)\n", val, addr);
    mem->ppu.vram[addr & VRAM_DSIZE] = val;
    break;
  case 0x07000000 ... 0x070003FF:
    logdebug("[INFO][MEM] Write (%02X) to OAM (%08X)\n", val, addr);
    mem->ppu.oam[addr & OAM_DSIZE] = val;
    break;
  default:
    logfatal("[ERR ][MEM] Write to unhandled address! (addr: %08X, val: %02X)\n", addr, val);
  }
}

void write_16(mem_t* mem, u32 addr, u16 val) {
  assert((addr & 1) == 0);
  switch(addr) {
  case 0x00004000 ... 0x01FFFFFF:
    logdebug("[WARN][MEM] Open bus write! (%08X, %02X)\n", addr, val);
    break;
  case 0x02000000 ... 0x0203FFFF:
    logdebug("[INFO][MEM] Write (%04X) to eWRAM (%08X)\n", val, addr);
    *(u16*)&mem->eWRAM[addr & EWRAM_DSIZE] = val;
    break;
  case 0x03000000 ... 0x03FFFFFF:
    logdebug("[INFO][MEM] Write (%04X) to iWRAM (%08X)\n", val, addr);
    *(u16*)&mem->iWRAM[addr & IWRAM_DSIZE] = val;
    break;
  case 0x04000000 ... 0x04000058:
    logdebug("[INFO][MEM] Write (%04X) to IO (%08X)\n", val, addr);
    write16_io_ppu(&mem->ppu, addr, val);
    break;
  case 0x04000208:
    logdebug("[INFO][MEM] Write (%02X) to IME (%08X) [WARN][STUB!]\n", val, addr);
    mem->ime = val;
    break;
  case 0x05000000 ... 0x050003FF:
    logdebug("[INFO][MEM] Write (%04X) to PRAM (%08X)\n", val, addr);
    *(u16*)&mem->ppu.pram[addr & PRAM_DSIZE] = val;
    break;
  case 0x06000000 ... 0x06017FFF:
    logdebug("[INFO][MEM] Write (%04X) to VRAM (%08X)\n", val, addr);
    *(u16*)&mem->ppu.vram[addr & VRAM_DSIZE] = val;
    break;
  case 0x07000000 ... 0x070003FF:
    logdebug("[INFO][MEM] Write (%04X) to OAM (%08X)\n", val, addr);
    *(u16*)&mem->ppu.oam[addr & OAM_DSIZE] = val;
    break;
  default:
    logfatal("[ERR ][MEM] Write to unhandled address! (addr: %08X, val: %04X)\n", addr, val);
  }
}

void write_32(mem_t* mem, u32 addr, u32 val) {
  assert((addr & 3) == 0);
  switch(addr) {
  case 0x00004000 ... 0x01FFFFFF:
    logdebug("[WARN][MEM] Open bus write! (%08X, %02X)\n", addr, val);
    break;
  case 0x02000000 ... 0x0203FFFF:
    logdebug("[INFO][MEM] Write (%08X) to eWRAM (%08X)\n", val, addr);
    *(u32*)&mem->eWRAM[addr & EWRAM_DSIZE] = val;
    break;
  case 0x03000000 ... 0x03FFFFFF:
    logdebug("[INFO][MEM] Write (%08X) to iWRAM (%08X)\n", val, addr);
    *(u32*)&mem->iWRAM[addr & IWRAM_DSIZE] = val;
    break;
  case 0x04000000 ... 0x04000058:
    logdebug("[INFO][MEM] Write (%08X) to IO (%08X)\n", val, addr);
    write32_io_ppu(&mem->ppu, addr, val);
    break;
  case 0x04000208:
    logdebug("[INFO][MEM] Write (%02X) to IME (%08X) [WARN][STUB!]\n", val, addr);
    mem->ime = val;
    break;
  case 0x05000000 ... 0x050003FF:
    logdebug("[INFO][MEM] Write (%08X) to PRAM (%08X)\n", val, addr);
    *(u32*)&mem->ppu.pram[addr & PRAM_DSIZE] = val;
    break;
  case 0x06000000 ... 0x06017FFF:
    logdebug("[INFO][MEM] Write (%08X) to VRAM (%08X)\n", val, addr);
    *(u32*)&mem->ppu.vram[addr & VRAM_DSIZE] = val;
    break;
  case 0x07000000 ... 0x070003FF:
    logdebug("[INFO][MEM] Write (%08X) to OAM (%08X)\n", val, addr);
    *(u32*)&mem->ppu.oam[addr & OAM_DSIZE] = val;
    break;
  default:
    logfatal("[ERR ][MEM] Write to unhandled address! (addr: %08X, val: %08X)\n", addr, val);
  }
}
