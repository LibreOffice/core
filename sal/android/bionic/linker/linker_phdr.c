/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <errno.h>
#include <sys/mman.h>

#include "linker_phdr.h"

/**
  TECHNICAL NOTE ON ELF LOADING.

  An ELF file's program header table contains one or more PT_LOAD
  segments, which corresponds to portions of the file that need to
  be mapped into the process' address space.

  Each loadable segment has the following important properties:

    p_offset  -> segment file offset
    p_filesz  -> segment file size
    p_memsz   -> segment memory size (always >= p_filesz)
    p_vaddr   -> segment's virtual address
    p_flags   -> segment flags (e.g. readable, writable, executable)

  We will ignore the p_paddr and p_align fields of Elf32_Phdr for now.

  The loadable segments can be seen as a list of [p_vaddr ... p_vaddr+p_memsz)
  ranges of virtual addresses. A few rules apply:

  - the virtual address ranges should not overlap.

  - if a segment's p_filesz is smaller than its p_memsz, the extra bytes
    between them should always be initialized to 0.

  - ranges do not necessarily start or end at page boundaries. Two distinct
    segments can have their start and end on the same page. In this case, the
    page inherits the mapping flags of the latter segment.

  Finally, the real load addrs of each segment is not p_vaddr. Instead the
  loader decides where to load the first segment, then will load all others
  relative to the first one to respect the initial range layout.

  For example, consider the following list:

    [ offset:0,      filesz:0x4000, memsz:0x4000, vaddr:0x30000 ],
    [ offset:0x4000, filesz:0x2000, memsz:0x8000, vaddr:0x40000 ],

  This corresponds to two segments that cover these virtual address ranges:

       0x30000...0x34000
       0x40000...0x48000

  If the loader decides to load the first segment at address 0xa0000000
  then the segments' load address ranges will be:

       0xa0030000...0xa0034000
       0xa0040000...0xa0048000

  In other words, all segments must be loaded at an address that has the same
  constant offset from their p_vaddr value. This offset is computed as the
  difference between the first segment's load address, and its p_vaddr value.

  However, in practice, segments do _not_ start at page boundaries. Since we
  can only memory-map at page boundaries, this means that the bias is
  computed as:

       load_bias = phdr0_load_address - PAGE_START(phdr0->p_vaddr)

  (NOTE: The value must be used as a 32-bit unsigned integer, to deal with
          possible wrap around UINT32_MAX for possible large p_vaddr values).

  And that the phdr0_load_address must start at a page boundary, with
  the segment's real content starting at:

       phdr0_load_address + PAGE_OFFSET(phdr0->p_vaddr)

  Note that ELF requires the following condition to make the mmap()-ing work:

      PAGE_OFFSET(phdr0->p_vaddr) == PAGE_OFFSET(phdr0->p_offset)

  The load_bias must be added to any p_vaddr value read from the ELF file to
  determine the corresponding memory address.

 **/

#define MAYBE_MAP_FLAG(x,from,to)    (((x) & (from)) ? (to) : 0)
#define PFLAGS_TO_PROT(x)            (MAYBE_MAP_FLAG((x), PF_X, PROT_EXEC) | \
                                      MAYBE_MAP_FLAG((x), PF_R, PROT_READ) | \
                                      MAYBE_MAP_FLAG((x), PF_W, PROT_WRITE))

/* Load the program header table from an ELF file into a read-only private
 * anonymous mmap-ed block.
 *
 * Input:
 *   fd           -> file descriptor
 *   phdr_offset  -> file offset of phdr table
 *   phdr_num     -> number of entries in the table.
 *
 * Output:
 *   phdr_mmap    -> address of mmap block in memory.
 *   phdr_memsize -> size of mmap block in memory.
 *   phdr_table   -> address of first entry in memory.
 *
 * Return:
 *   -1 on error, or 0 on success.
 */
int phdr_table_load(int                fd,
                    Elf32_Addr         phdr_offset,
                    Elf32_Half         phdr_num,
                    void**             phdr_mmap,
                    Elf32_Addr*        phdr_size,
                    const Elf32_Phdr** phdr_table)
{
    Elf32_Addr  page_min, page_max, page_offset;
    void*       mmap_result;

    /* Just like the kernel, we only accept program header tables that
     * are smaller than 64KB. */
    if (phdr_num < 1 || phdr_num > 65536/sizeof(Elf32_Phdr)) {
        errno = EINVAL;
        return -1;
    }

    page_min = PAGE_START(phdr_offset);
    page_max = PAGE_END(phdr_offset + phdr_num*sizeof(Elf32_Phdr));
    page_offset = PAGE_OFFSET(phdr_offset);

    mmap_result = mmap(NULL,
                       page_max - page_min,
                       PROT_READ,
                       MAP_PRIVATE,
                       fd,
                       page_min);

    if (mmap_result == MAP_FAILED) {
        return -1;
    }

    *phdr_mmap = mmap_result;
    *phdr_size = page_max - page_min;
    *phdr_table = (Elf32_Phdr*)((char*)mmap_result + page_offset);

    return 0;
}

void phdr_table_unload(void* phdr_mmap, Elf32_Addr phdr_memsize)
{
    munmap(phdr_mmap, phdr_memsize);
}


/* Compute the extent of all loadable segments in an ELF program header
 * table. This corresponds to the page-aligned size in bytes that needs to be
 * reserved in the process' address space
 *
 * This returns 0 if there are no loadable segments.
 */
Elf32_Addr phdr_table_get_load_size(const Elf32_Phdr* phdr_table,
                                    size_t phdr_count)
{
    Elf32_Addr min_vaddr = 0xFFFFFFFFU;
    Elf32_Addr max_vaddr = 0x00000000U;

    for (size_t i = 0; i < phdr_count; ++i) {
        const Elf32_Phdr* phdr = &phdr_table[i];

        if (phdr->p_type != PT_LOAD) {
            continue;
        }

        if (phdr->p_vaddr < min_vaddr) {
            min_vaddr = phdr->p_vaddr;
        }

        if (phdr->p_vaddr + phdr->p_memsz > max_vaddr) {
            max_vaddr = phdr->p_vaddr + phdr->p_memsz;
        }
    }

    if (min_vaddr > max_vaddr) {
        return 0;
    }

    min_vaddr = PAGE_START(min_vaddr);
    max_vaddr = PAGE_END(max_vaddr);

    return max_vaddr - min_vaddr;
}

/* Reserve a virtual address range big enough to hold all loadable
 * segments of a program header table. This is done by creating a
 * private anonymous mmap() with PROT_NONE.
 *
 * Input:
 *   phdr_table    -> program header table
 *   phdr_count    -> number of entries in the tables
 * Output:
 *   load_start    -> first page of reserved address space range
 *   load_size     -> size in bytes of reserved address space range
 *   load_bias     -> load bias, as described in technical note above.
 *
 * Return:
 *   0 on success, -1 otherwise. Error code in errno.
 */
int
phdr_table_reserve_memory(const Elf32_Phdr* phdr_table,
                          size_t phdr_count,
                          void** load_start,
                          Elf32_Addr* load_size,
                          Elf32_Addr* load_bias)
{
    Elf32_Addr size = phdr_table_get_load_size(phdr_table, phdr_count);
    if (size == 0) {
        errno = EINVAL;
        return -1;
    }

    int mmap_flags = MAP_PRIVATE | MAP_ANONYMOUS;
    void* start = mmap(NULL, size, PROT_NONE, mmap_flags, -1, 0);
    if (start == MAP_FAILED) {
        return -1;
    }

    *load_start = start;
    *load_size  = size;
    *load_bias  = 0;

    for (size_t i = 0; i < phdr_count; ++i) {
        const Elf32_Phdr* phdr = &phdr_table[i];
        if (phdr->p_type == PT_LOAD) {
            *load_bias = (Elf32_Addr)start - PAGE_START(phdr->p_vaddr);
            break;
        }
    }
    return 0;
}

/* Map all loadable segments in process' address space.
 * This assumes you already called phdr_table_reserve_memory to
 * reserve the address space range for the library.
 *
 * Input:
 *   phdr_table    -> program header table
 *   phdr_count    -> number of entries in the table
 *   load_bias     -> load offset.
 *   fd            -> input file descriptor.
 *
 * Return:
 *   0 on success, -1 otherwise. Error code in errno.
 */
int
phdr_table_load_segments(const Elf32_Phdr* phdr_table,
                         int               phdr_count,
                         Elf32_Addr        load_bias,
                         int               fd)
{
    int nn;

    for (nn = 0; nn < phdr_count; nn++) {
        const Elf32_Phdr* phdr = &phdr_table[nn];
        void* seg_addr;

        if (phdr->p_type != PT_LOAD)
            continue;

        /* Segment addresses in memory */
        Elf32_Addr seg_start = phdr->p_vaddr + load_bias;
        Elf32_Addr seg_end   = seg_start + phdr->p_memsz;

        Elf32_Addr seg_page_start = PAGE_START(seg_start);
        Elf32_Addr seg_page_end   = PAGE_END(seg_end);

        Elf32_Addr seg_file_end   = seg_start + phdr->p_filesz;

        /* File offsets */
        Elf32_Addr file_start = phdr->p_offset;
        Elf32_Addr file_end   = file_start + phdr->p_filesz;

        Elf32_Addr file_page_start = PAGE_START(file_start);
        Elf32_Addr file_page_end   = PAGE_END(file_end);

        seg_addr = mmap((void*)seg_page_start,
                        file_end - file_page_start,
                        PFLAGS_TO_PROT(phdr->p_flags),
                        MAP_FIXED|MAP_PRIVATE,
                        fd,
                        file_page_start);

        if (seg_addr == MAP_FAILED) {
            return -1;
        }

        /* if the segment is writable, and does not end on a page boundary,
         * zero-fill it until the page limit. */
        if ((phdr->p_flags & PF_W) != 0 && PAGE_OFFSET(seg_file_end) > 0) {
            memset((void*)seg_file_end, 0, PAGE_SIZE - PAGE_OFFSET(seg_file_end));
        }

        seg_file_end = PAGE_END(seg_file_end);

        /* seg_file_end is now the first page address after the file
         * content. If seg_end is larger, we need to zero anything
         * between them. This is done by using a private anonymous
         * map for all extra pages.
         */
        if (seg_page_end > seg_file_end) {
            void* zeromap = mmap((void*)seg_file_end,
                                    seg_page_end - seg_file_end,
                                    PFLAGS_TO_PROT(phdr->p_flags),
                                    MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE,
                                    -1,
                                    0);
            if (zeromap == MAP_FAILED) {
                return -1;
            }
        }
    }
    return 0;
}

/* Used internally. Used to set the protection bits of all loaded segments
 * with optional extra flags (i.e. really PROT_WRITE). Used by
 * phdr_table_protect_segments and phdr_table_unprotect_segments.
 */
static int
_phdr_table_set_load_prot(const Elf32_Phdr* phdr_table,
                          int               phdr_count,
                          Elf32_Addr        load_bias,
                          int               extra_prot_flags)
{
    const Elf32_Phdr* phdr = phdr_table;
    const Elf32_Phdr* phdr_limit = phdr + phdr_count;

    for (; phdr < phdr_limit; phdr++) {
        if (phdr->p_type != PT_LOAD || (phdr->p_flags & PF_W) != 0)
            continue;

        Elf32_Addr seg_page_start = PAGE_START(phdr->p_vaddr) + load_bias;
        Elf32_Addr seg_page_end   = PAGE_END(phdr->p_vaddr + phdr->p_memsz) + load_bias;

        int ret = mprotect((void*)seg_page_start,
                           seg_page_end - seg_page_start,
                           PFLAGS_TO_PROT(phdr->p_flags) | extra_prot_flags);
        if (ret < 0) {
            return -1;
        }
    }
    return 0;
}

/* Restore the original protection modes for all loadable segments.
 * You should only call this after phdr_table_unprotect_segments and
 * applying all relocations.
 *
 * Input:
 *   phdr_table  -> program header table
 *   phdr_count  -> number of entries in tables
 *   load_bias   -> load bias
 * Return:
 *   0 on error, -1 on failure (error code in errno).
 */
int
phdr_table_protect_segments(const Elf32_Phdr* phdr_table,
                            int               phdr_count,
                            Elf32_Addr        load_bias)
{
    return _phdr_table_set_load_prot(phdr_table, phdr_count,
                                      load_bias, 0);
}

/* Change the protection of all loaded segments in memory to writable.
 * This is useful before performing relocations. Once completed, you
 * will have to call phdr_table_protect_segments to restore the original
 * protection flags on all segments.
 *
 * Note that some writable segments can also have their content turned
 * to read-only by calling phdr_table_protect_gnu_relro. This is no
 * performed here.
 *
 * Input:
 *   phdr_table  -> program header table
 *   phdr_count  -> number of entries in tables
 *   load_bias   -> load bias
 * Return:
 *   0 on error, -1 on failure (error code in errno).
 */
int
phdr_table_unprotect_segments(const Elf32_Phdr* phdr_table,
                              int               phdr_count,
                              Elf32_Addr        load_bias)
{
    return _phdr_table_set_load_prot(phdr_table, phdr_count,
                                      load_bias, PROT_WRITE);
}

/* Used internally by phdr_table_protect_gnu_relro and
 * phdr_table_unprotect_gnu_relro.
 */
static int
_phdr_table_set_gnu_relro_prot(const Elf32_Phdr* phdr_table,
                               int               phdr_count,
                               Elf32_Addr        load_bias,
                               int               prot_flags)
{
    const Elf32_Phdr* phdr = phdr_table;
    const Elf32_Phdr* phdr_limit = phdr + phdr_count;

    for (phdr = phdr_table; phdr < phdr_limit; phdr++) {
        if (phdr->p_type != PT_GNU_RELRO)
            continue;

        /* Tricky: what happens when the relro segment does not start
         * or end at page boundaries?. We're going to be over-protective
         * here and put every page touched by the segment as read-only.
         *
         * This seems to match Ian Lance Taylor's description of the
         * feature at http://www.airs.com/blog/archives/189.
         *
         * Extract:
         *    Note that the current dynamic linker code will only work
         *    correctly if the PT_GNU_RELRO segment starts on a page
         *    boundary. This is because the dynamic linker rounds the
         *    p_vaddr field down to the previous page boundary. If
         *    there is anything on the page which should not be read-only,
         *    the program is likely to fail at runtime. So in effect the
         *    linker must only emit a PT_GNU_RELRO segment if it ensures
         *    that it starts on a page boundary.
         */
        Elf32_Addr seg_page_start = PAGE_START(phdr->p_vaddr) + load_bias;
        Elf32_Addr seg_page_end   = PAGE_END(phdr->p_vaddr + phdr->p_memsz) + load_bias;

        int ret = mprotect((void*)seg_page_start,
                           seg_page_end - seg_page_start,
                           prot_flags);
        if (ret < 0) {
            return -1;
        }
    }
    return 0;
}

/* Apply GNU relro protection if specified by the program header. This will
 * turn some of the pages of a writable PT_LOAD segment to read-only, as
 * specified by one or more PT_GNU_RELRO segments. This must be always
 * performed after relocations.
 *
 * The areas typically covered are .got and .data.rel.ro, these are
 * read-only from the program's POV, but contain absolute addresses
 * that need to be relocated before use.
 *
 * Input:
 *   phdr_table  -> program header table
 *   phdr_count  -> number of entries in tables
 *   load_bias   -> load bias
 * Return:
 *   0 on error, -1 on failure (error code in errno).
 */
int
phdr_table_protect_gnu_relro(const Elf32_Phdr* phdr_table,
                             int               phdr_count,
                             Elf32_Addr        load_bias)
{
    return _phdr_table_set_gnu_relro_prot(phdr_table,
                                          phdr_count,
                                          load_bias,
                                          PROT_READ);
}

#ifdef ANDROID_ARM_LINKER

#  ifndef PT_ARM_EXIDX
#    define PT_ARM_EXIDX    0x70000001      /* .ARM.exidx segment */
#  endif

/* Return the address and size of the .ARM.exidx section in memory,
 * if present.
 *
 * Input:
 *   phdr_table  -> program header table
 *   phdr_count  -> number of entries in tables
 *   load_bias   -> load bias
 * Output:
 *   arm_exidx       -> address of table in memory (NULL on failure).
 *   arm_exidx_count -> number of items in table (0 on failure).
 * Return:
 *   0 on error, -1 on failure (_no_ error code in errno)
 */
int
phdr_table_get_arm_exidx(const Elf32_Phdr* phdr_table,
                         int               phdr_count,
                         Elf32_Addr        load_bias,
                         Elf32_Addr**      arm_exidx,
                         unsigned*         arm_exidx_count)
{
    const Elf32_Phdr* phdr = phdr_table;
    const Elf32_Phdr* phdr_limit = phdr + phdr_count;

    for (phdr = phdr_table; phdr < phdr_limit; phdr++) {
        if (phdr->p_type != PT_ARM_EXIDX)
            continue;

        *arm_exidx = (Elf32_Addr*)(load_bias + phdr->p_vaddr);
        *arm_exidx_count = (unsigned)(phdr->p_memsz / 8);
        return 0;
    }
    *arm_exidx = NULL;
    *arm_exidx_count = 0;
    return -1;
}
#endif /* ANDROID_ARM_LINKER */

/* Return the address and size of the ELF file's .dynamic section in memory,
 * or NULL if missing.
 *
 * Input:
 *   phdr_table  -> program header table
 *   phdr_count  -> number of entries in tables
 *   load_bias   -> load bias
 * Output:
 *   dynamic       -> address of table in memory (NULL on failure).
 *   dynamic_count -> number of items in table (0 on failure).
 * Return:
 *   void
 */
void
phdr_table_get_dynamic_section(const Elf32_Phdr* phdr_table,
                               int               phdr_count,
                               Elf32_Addr        load_bias,
                               Elf32_Addr**      dynamic,
                               size_t*           dynamic_count)
{
    const Elf32_Phdr* phdr = phdr_table;
    const Elf32_Phdr* phdr_limit = phdr + phdr_count;

    for (phdr = phdr_table; phdr < phdr_limit; phdr++) {
        if (phdr->p_type != PT_DYNAMIC) {
            continue;
        }

        *dynamic = (Elf32_Addr*)(load_bias + phdr->p_vaddr);
        if (dynamic_count) {
            *dynamic_count = (unsigned)(phdr->p_memsz / 8);
        }
        return;
    }
    *dynamic = NULL;
    if (dynamic_count) {
        *dynamic_count = 0;
    }
}

/* Return the address of the program header table as it appears in the loaded
 * segments in memory. This is in contrast with the input 'phdr_table' which
 * is temporary and will be released before the library is relocated.
 *
 * Input:
 *   phdr_table  -> program header table
 *   phdr_count  -> number of entries in tables
 *   load_bias   -> load bias
 * Return:
 *   Address of loaded program header table on success (it has
 *   'phdr_count' entries), or NULL on failure (no error code).
 */
const Elf32_Phdr*
phdr_table_get_loaded_phdr(const Elf32_Phdr*   phdr_table,
                           int                 phdr_count,
                           Elf32_Addr          load_bias)
{
    const Elf32_Phdr* phdr = phdr_table;
    const Elf32_Phdr* phdr_limit = phdr + phdr_count;
    Elf32_Addr  loaded = 0;
    Elf32_Addr  loaded_end;

    /* If there is a PT_PHDR, use it directly */
    for (phdr = phdr_table; phdr < phdr_limit; phdr++) {
        if (phdr->p_type == PT_PHDR) {
            loaded = load_bias + phdr->p_vaddr;
            goto CHECK;
        }
    }

    /* Otherwise, check the first loadable segment. If its file offset
     * is 0, it starts with the ELF header, and we can trivially find the
     * loaded program header from it. */
    for (phdr = phdr_table; phdr < phdr_limit; phdr++) {
        if (phdr->p_type == PT_LOAD) {
            if (phdr->p_offset == 0) {
                Elf32_Addr  elf_addr = load_bias + phdr->p_vaddr;
                const Elf32_Ehdr* ehdr = (const Elf32_Ehdr*)(void*)elf_addr;
                Elf32_Addr  offset = ehdr->e_phoff;
                loaded = (Elf32_Addr)ehdr + offset;
                goto CHECK;
            }
            break;
        }
    }

    /* We didn't find it, let the client know. He may be able to
     * keep a copy of the input phdr_table instead. */
    return NULL;

CHECK:
    /* Ensure that our program header is actually within a loadable
     * segment. This should help catch badly-formed ELF files that
     * would cause the linker to crash later when trying to access it.
     */
    loaded_end = loaded + phdr_count*sizeof(Elf32_Phdr);

    for (phdr = phdr_table; phdr < phdr_limit; phdr++) {
        if (phdr->p_type != PT_LOAD)
            continue;
        Elf32_Addr seg_start = phdr->p_vaddr + load_bias;
        Elf32_Addr seg_end   = phdr->p_filesz + seg_start;

        if (seg_start <= loaded && loaded_end <= seg_end) {
            return (const Elf32_Phdr*)loaded;
        }
    }
    return NULL;
}
