/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _LINKER_H_
#define _LINKER_H_

#include <unistd.h>
#include <sys/types.h>
#include <elf.h>
#include <sys/exec_elf.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <link.h>

#undef PAGE_MASK
#undef PAGE_SIZE
#define PAGE_SIZE 4096
#define PAGE_MASK (PAGE_SIZE-1)

/* Convenience macros to make page address/offset computations more explicit */

/* Returns the address of the page starting at address 'x' */
#define PAGE_START(x)  ((x) & ~PAGE_MASK)

/* Returns the offset of address 'x' in its memory page, i.e. this is the
 * same than 'x' - PAGE_START(x) */
#define PAGE_OFFSET(x) ((x) & PAGE_MASK)

/* Returns the address of the next page after address 'x', unless 'x' is
 * itself at the start of a page. Equivalent to:
 *
 *  (x == PAGE_START(x)) ? x : PAGE_START(x)+PAGE_SIZE
 */
#define PAGE_END(x)    PAGE_START((x) + (PAGE_SIZE-1))

void debugger_init();

/* magic shared structures that GDB knows about */

struct link_map
{
    uintptr_t l_addr;
    char * l_name;
    uintptr_t l_ld;
    struct link_map * l_next;
    struct link_map * l_prev;
};

// Values for r_debug->state
enum {
    RT_CONSISTENT,
    RT_ADD,
    RT_DELETE
};

struct r_debug
{
    int32_t r_version;
    struct link_map * r_map;
    void (*r_brk)(void);
    int32_t r_state;
    uintptr_t r_ldbase;
};

typedef struct soinfo soinfo;

#define FLAG_LINKED     0x00000001
#define FLAG_ERROR      0x00000002
#define FLAG_EXE        0x00000004 // The main executable
#define FLAG_LINKER     0x00000010 // The linker itself

#define SOINFO_NAME_LEN 128

struct soinfo
{
    char name[SOINFO_NAME_LEN];
    const Elf32_Phdr *phdr;
    int phnum;
    unsigned entry;
    unsigned base;
    unsigned size;

    int unused;  // DO NOT USE, maintained for compatibility.

    unsigned *dynamic;

    unsigned unused2; // DO NOT USE, maintained for compatibility
    unsigned unused3; // DO NOT USE, maintained for compatibility

    soinfo *next;
    unsigned flags;

    const char *strtab;
    Elf32_Sym *symtab;

    unsigned nbucket;
    unsigned nchain;
    unsigned *bucket;
    unsigned *chain;

    unsigned *plt_got;

    Elf32_Rel *plt_rel;
    unsigned plt_rel_count;

    Elf32_Rel *rel;
    unsigned rel_count;

    unsigned *preinit_array;
    unsigned preinit_array_count;

    unsigned *init_array;
    unsigned init_array_count;
    unsigned *fini_array;
    unsigned fini_array_count;

    void (*init_func)(void);
    void (*fini_func)(void);

#if defined(ANDROID_ARM_LINKER)
    /* ARM EABI section used for stack unwinding. */
    unsigned *ARM_exidx;
    unsigned ARM_exidx_count;
#elif defined(ANDROID_MIPS_LINKER)
#if 0
     /* not yet */
     unsigned *mips_pltgot
#endif
     unsigned mips_symtabno;
     unsigned mips_local_gotno;
     unsigned mips_gotsym;
#endif /* ANDROID_*_LINKER */

    unsigned refcount;
    struct link_map linkmap;

    int constructors_called;

    /* When you read a virtual address from the ELF file, add this
     * value to get the corresponding address in the process' address space */
    Elf32_Addr load_bias;
    int has_text_relocations;
};


extern soinfo libdl_info;


#include <asm/elf.h>

#if defined(ANDROID_ARM_LINKER)

// These aren't defined in <arch-arm/asm/elf.h>.
#define R_ARM_REL32      3
#define R_ARM_COPY       20
#define R_ARM_GLOB_DAT   21
#define R_ARM_JUMP_SLOT  22
#define R_ARM_RELATIVE   23

#elif defined(ANDROID_MIPS_LINKER)

// These aren't defined in <arch-arm/mips/elf.h>.
#define R_MIPS_JUMP_SLOT       127

#define DT_MIPS_PLTGOT         0x70000032
#define DT_MIPS_RWPLT          0x70000034

#elif defined(ANDROID_X86_LINKER)

// x86 has everything it needs in <arch-arm/x86/elf.h>.

#endif /* ANDROID_*_LINKER */

#ifndef DT_INIT_ARRAY
#define DT_INIT_ARRAY      25
#endif

#ifndef DT_FINI_ARRAY
#define DT_FINI_ARRAY      26
#endif

#ifndef DT_INIT_ARRAYSZ
#define DT_INIT_ARRAYSZ    27
#endif

#ifndef DT_FINI_ARRAYSZ
#define DT_FINI_ARRAYSZ    28
#endif

#ifndef DT_PREINIT_ARRAY
#define DT_PREINIT_ARRAY   32
#endif

#ifndef DT_PREINIT_ARRAYSZ
#define DT_PREINIT_ARRAYSZ 33
#endif

soinfo *find_library(const char *name);
Elf32_Sym *lookup(const char *name, soinfo **found, soinfo *start);
soinfo *find_containing_library(const void *addr);
const char *linker_get_error(void);

int soinfo_unload(soinfo* si);
Elf32_Sym *soinfo_find_symbol(soinfo* si, const void *addr);
Elf32_Sym *soinfo_lookup(soinfo *si, const char *name);
void soinfo_call_constructors(soinfo *si);

#ifdef __cplusplus
};
#endif

#endif
