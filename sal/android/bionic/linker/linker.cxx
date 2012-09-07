/*
 * Copyright (C) 2008, 2009 The Android Open Source Project
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

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/auxvec.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/atomics.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

// Private C library headers.
#include <private/bionic_tls.h>
#include <private/logd.h>

#include "linker.h"
#include "linker_debug.h"
#include "linker_environ.h"
#include "linker_format.h"
#include "linker_phdr.h"

#define ALLOW_SYMBOLS_FROM_MAIN 1
#define SO_MAX 128

/* Assume average path length of 64 and max 8 paths */
#define LDPATH_BUFSIZE 512
#define LDPATH_MAX 8

#define LDPRELOAD_BUFSIZE 512
#define LDPRELOAD_MAX 8

/* >>> IMPORTANT NOTE - READ ME BEFORE MODIFYING <<<
 *
 * Do NOT use malloc() and friends or pthread_*() code here.
 * Don't use printf() either; it's caused mysterious memory
 * corruption in the past.
 * The linker runs before we bring up libc and it's easiest
 * to make sure it does not depend on any complex libc features
 *
 * open issues / todo:
 *
 * - are we doing everything we should for ARM_COPY relocations?
 * - cleaner error reporting
 * - after linking, set as much stuff as possible to READONLY
 *   and NOEXEC
 * - linker hardcodes PAGE_SIZE and PAGE_MASK because the kernel
 *   headers provide versions that are negative...
 * - allocate space for soinfo structs dynamically instead of
 *   having a hard limit (SO_MAX)
 */


static int soinfo_link_image(soinfo *si);

static int socount = 0;
static soinfo sopool[SO_MAX];
static soinfo *freelist = NULL;
static soinfo *solist = &libdl_info;
static soinfo *sonext = &libdl_info;
#if ALLOW_SYMBOLS_FROM_MAIN
static soinfo *somain; /* main process, always the one after libdl_info */
#endif


static char ldpaths_buf[LDPATH_BUFSIZE];
static const char *ldpaths[LDPATH_MAX + 1];

static char ldpreloads_buf[LDPRELOAD_BUFSIZE];
static const char *ldpreload_names[LDPRELOAD_MAX + 1];

static soinfo *preloads[LDPRELOAD_MAX + 1];

#if LINKER_DEBUG
int debug_verbosity;
#endif

static int pid;

/* This boolean is set if the program being loaded is setuid */
static bool program_is_setuid;

enum RelocationKind {
    kRelocAbsolute = 0,
    kRelocRelative,
    kRelocCopy,
    kRelocSymbol,
    kRelocMax
};

#if STATS
struct linker_stats_t {
    int count[kRelocMax];
};

static linker_stats_t linker_stats;

static void count_relocation(RelocationKind kind) {
    ++linker_stats.count[kind];
}
#else
static void count_relocation(RelocationKind) {
}
#endif

#if COUNT_PAGES
static unsigned bitmask[4096];
#define MARK(offset) \
    do { \
        bitmask[((offset) >> 12) >> 3] |= (1 << (((offset) >> 12) & 7)); \
    } while(0)
#else
#define MARK(x) do {} while (0)
#endif

// You shouldn't try to call memory-allocating functions in the dynamic linker.
// Guard against the most obvious ones.
#define DISALLOW_ALLOCATION(return_type, name, ...)                             \
    return_type name __VA_ARGS__                                                \
    {                                                                           \
        const char* msg = "ERROR: " #name " called from the dynamic linker!\n"; \
         __libc_android_log_write(ANDROID_LOG_FATAL, "linker", msg);            \
        write(2, msg, sizeof(msg));                                             \
        abort();                                                                \
    }
#define UNUSED __attribute__((unused))
DISALLOW_ALLOCATION(void*, malloc, (size_t u UNUSED));
DISALLOW_ALLOCATION(void, free, (void* u UNUSED));
DISALLOW_ALLOCATION(void*, realloc, (void* u1 UNUSED, size_t u2 UNUSED));
DISALLOW_ALLOCATION(void*, calloc, (size_t u1 UNUSED, size_t u2 UNUSED));

static char tmp_err_buf[768];
static char __linker_dl_err_buf[768];
#define BASENAME(s) (strrchr(s, '/') != NULL ? strrchr(s, '/') + 1 : s)
#define DL_ERR(fmt, x...) \
    do { \
        format_buffer(__linker_dl_err_buf, sizeof(__linker_dl_err_buf), \
                      "%s(%s:%d): " fmt, \
                      __FUNCTION__, BASENAME(__FILE__), __LINE__, ##x); \
        ERROR(fmt "\n", ##x); \
    } while(0)

const char *linker_get_error(void)
{
    return (const char *)&__linker_dl_err_buf[0];
}

/*
 * This function is an empty stub where GDB locates a breakpoint to get notified
 * about linker activity.
 */
extern "C" void __attribute__((noinline)) __attribute__((visibility("default"))) rtld_db_dlactivity(void);

static r_debug _r_debug = {1, NULL, &rtld_db_dlactivity,
                                  RT_CONSISTENT, 0};
static link_map* r_debug_tail = 0;

static pthread_mutex_t _r_debug_lock = PTHREAD_MUTEX_INITIALIZER;

static void insert_soinfo_into_debug_map(soinfo * info) {
    // Copy the necessary fields into the debug structure.
    link_map* map = &(info->linkmap);
    map->l_addr = info->base;
    map->l_name = (char*) info->name;
    map->l_ld = (uintptr_t)info->dynamic;

    /* Stick the new library at the end of the list.
     * gdb tends to care more about libc than it does
     * about leaf libraries, and ordering it this way
     * reduces the back-and-forth over the wire.
     */
    if (r_debug_tail) {
        r_debug_tail->l_next = map;
        map->l_prev = r_debug_tail;
        map->l_next = 0;
    } else {
        _r_debug.r_map = map;
        map->l_prev = 0;
        map->l_next = 0;
    }
    r_debug_tail = map;
}

static void remove_soinfo_from_debug_map(soinfo* info) {
    link_map* map = &(info->linkmap);

    if (r_debug_tail == map) {
        r_debug_tail = map->l_prev;
    }

    if (map->l_prev) {
        map->l_prev->l_next = map->l_next;
    }
    if (map->l_next) {
        map->l_next->l_prev = map->l_prev;
    }
}

static void notify_gdb_of_load(soinfo* info) {
    if (info->flags & FLAG_EXE) {
        // GDB already knows about the main executable
        return;
    }

    pthread_mutex_lock(&_r_debug_lock);

    _r_debug.r_state = RT_ADD;
    rtld_db_dlactivity();

    insert_soinfo_into_debug_map(info);

    _r_debug.r_state = RT_CONSISTENT;
    rtld_db_dlactivity();

    pthread_mutex_unlock(&_r_debug_lock);
}

static void notify_gdb_of_unload(soinfo* info) {
    if (info->flags & FLAG_EXE) {
        // GDB already knows about the main executable
        return;
    }

    pthread_mutex_lock(&_r_debug_lock);

    _r_debug.r_state = RT_DELETE;
    rtld_db_dlactivity();

    remove_soinfo_from_debug_map(info);

    _r_debug.r_state = RT_CONSISTENT;
    rtld_db_dlactivity();

    pthread_mutex_unlock(&_r_debug_lock);
}

extern "C" void notify_gdb_of_libraries()
{
    _r_debug.r_state = RT_ADD;
    rtld_db_dlactivity();
    _r_debug.r_state = RT_CONSISTENT;
    rtld_db_dlactivity();
}

static soinfo *soinfo_alloc(const char *name)
{
    if (strlen(name) >= SOINFO_NAME_LEN) {
        DL_ERR("library name \"%s\" too long", name);
        return NULL;
    }

    /* The freelist is populated when we call soinfo_free(), which in turn is
       done only by dlclose(), which is not likely to be used.
    */
    if (!freelist) {
        if (socount == SO_MAX) {
            DL_ERR("too many libraries when loading \"%s\"", name);
            return NULL;
        }
        freelist = sopool + socount++;
        freelist->next = NULL;
    }

    soinfo* si = freelist;
    freelist = freelist->next;

    /* Make sure we get a clean block of soinfo */
    memset(si, 0, sizeof(soinfo));
    strlcpy((char*) si->name, name, sizeof(si->name));
    sonext->next = si;
    si->next = NULL;
    si->refcount = 0;
    sonext = si;

    TRACE("%5d name %s: allocated soinfo @ %p\n", pid, name, si);
    return si;
}

static void soinfo_free(soinfo* si)
{
    if (si == NULL) {
        return;
    }

    soinfo *prev = NULL, *trav;

    TRACE("%5d name %s: freeing soinfo @ %p\n", pid, si->name, si);

    for(trav = solist; trav != NULL; trav = trav->next){
        if (trav == si)
            break;
        prev = trav;
    }
    if (trav == NULL) {
        /* si was not ni solist */
        DL_ERR("name \"%s\" is not in solist!", si->name);
        return;
    }

    /* prev will never be NULL, because the first entry in solist is
       always the static libdl_info.
    */
    prev->next = si->next;
    if (si == sonext) sonext = prev;
    si->next = freelist;
    freelist = si;
}

#ifdef ANDROID_ARM_LINKER

/* For a given PC, find the .so that it belongs to.
 * Returns the base address of the .ARM.exidx section
 * for that .so, and the number of 8-byte entries
 * in that section (via *pcount).
 *
 * Intended to be called by libc's __gnu_Unwind_Find_exidx().
 *
 * This function is exposed via dlfcn.c and libdl.so.
 */
_Unwind_Ptr dl_unwind_find_exidx(_Unwind_Ptr pc, int *pcount)
{
    soinfo *si;
    unsigned addr = (unsigned)pc;

    for (si = solist; si != 0; si = si->next){
        if ((addr >= si->base) && (addr < (si->base + si->size))) {
            *pcount = si->ARM_exidx_count;
            return (_Unwind_Ptr)si->ARM_exidx;
        }
    }
   *pcount = 0;
    return NULL;
}

#elif defined(ANDROID_X86_LINKER) || defined(ANDROID_MIPS_LINKER)

/* Here, we only have to provide a callback to iterate across all the
 * loaded libraries. gcc_eh does the rest. */
int
dl_iterate_phdr(int (*cb)(dl_phdr_info *info, size_t size, void *data),
                void *data)
{
    int rv = 0;
    for (soinfo* si = solist; si != NULL; si = si->next) {
        dl_phdr_info dl_info;
        dl_info.dlpi_addr = si->linkmap.l_addr;
        dl_info.dlpi_name = si->linkmap.l_name;
        dl_info.dlpi_phdr = si->phdr;
        dl_info.dlpi_phnum = si->phnum;
        rv = cb(&dl_info, sizeof(dl_phdr_info), data);
        if (rv != 0) {
            break;
        }
    }
    return rv;
}

#endif

static Elf32_Sym *soinfo_elf_lookup(soinfo *si, unsigned hash, const char *name)
{
    Elf32_Sym *s;
    Elf32_Sym *symtab = si->symtab;
    const char *strtab = si->strtab;
    unsigned n;

    TRACE_TYPE(LOOKUP, "%5d SEARCH %s in %s@0x%08x %08x %d\n", pid,
               name, si->name, si->base, hash, hash % si->nbucket);
    n = hash % si->nbucket;

    for(n = si->bucket[hash % si->nbucket]; n != 0; n = si->chain[n]){
        s = symtab + n;
        if(strcmp(strtab + s->st_name, name)) continue;

            /* only concern ourselves with global and weak symbol definitions */
        switch(ELF32_ST_BIND(s->st_info)){
        case STB_GLOBAL:
        case STB_WEAK:
            if(s->st_shndx == SHN_UNDEF)
                continue;

            TRACE_TYPE(LOOKUP, "%5d FOUND %s in %s (%08x) %d\n", pid,
                       name, si->name, s->st_value, s->st_size);
            return s;
        }
    }

    return NULL;
}

static unsigned elfhash(const char *_name)
{
    const unsigned char *name = (const unsigned char *) _name;
    unsigned h = 0, g;

    while(*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}

static Elf32_Sym *
soinfo_do_lookup(soinfo *si, const char *name, Elf32_Addr *offset,
                 soinfo *needed[], bool ignore_local)
{
    unsigned elf_hash = elfhash(name);
    Elf32_Sym *s = NULL;
    soinfo *lsi = si;
    int i;

    if (!ignore_local) {
        /* Look for symbols in the local scope (the object who is
         * searching). This happens with C++ templates on i386 for some
         * reason.
         *
         * Notes on weak symbols:
         * The ELF specs are ambiguous about treatment of weak definitions in
         * dynamic linking.  Some systems return the first definition found
         * and some the first non-weak definition.   This is system dependent.
         * Here we return the first definition found for simplicity.  */

        s = soinfo_elf_lookup(si, elf_hash, name);
        if(s != NULL)
            goto done;
    }

    /* Next, look for it in the preloads list */
    for(i = 0; preloads[i] != NULL; i++) {
        lsi = preloads[i];
        s = soinfo_elf_lookup(lsi, elf_hash, name);
        if(s != NULL)
            goto done;
    }

    for(i = 0; needed[i] != NULL; i++) {
        lsi = needed[i];
        DEBUG("%5d %s: looking up %s in %s\n",
              pid, si->name, name, lsi->name);
        s = soinfo_elf_lookup(lsi, elf_hash, name);
        if (s != NULL)
            goto done;
    }

#if ALLOW_SYMBOLS_FROM_MAIN
    /* If we are resolving relocations while dlopen()ing a library, it's OK for
     * the library to resolve a symbol that's defined in the executable itself,
     * although this is rare and is generally a bad idea.
     */
    if (somain) {
        lsi = somain;
        DEBUG("%5d %s: looking up %s in executable %s\n",
              pid, si->name, name, lsi->name);
        s = soinfo_elf_lookup(lsi, elf_hash, name);
    }
#endif

done:
    if(s != NULL) {
        TRACE_TYPE(LOOKUP, "%5d si %s sym %s s->st_value = 0x%08x, "
                   "found in %s, base = 0x%08x, load bias = 0x%08x\n",
                   pid, si->name, name, s->st_value,
                   lsi->name, lsi->base, lsi->load_bias);
        *offset = lsi->load_bias;
        return s;
    }

    return NULL;
}

/* This is used by dl_sym().  It performs symbol lookup only within the
   specified soinfo object and not in any of its dependencies.
 */
Elf32_Sym *soinfo_lookup(soinfo *si, const char *name)
{
    return soinfo_elf_lookup(si, elfhash(name), name);
}

/* This is used by dl_sym().  It performs a global symbol lookup.
 */
Elf32_Sym *lookup(const char *name, soinfo **found, soinfo *start)
{
    unsigned elf_hash = elfhash(name);
    Elf32_Sym *s = NULL;
    soinfo *si;

    if(start == NULL) {
        start = solist;
    }

    for(si = start; (s == NULL) && (si != NULL); si = si->next)
    {
        if(si->flags & FLAG_ERROR)
            continue;
        s = soinfo_elf_lookup(si, elf_hash, name);
        if (s != NULL) {
            *found = si;
            break;
        }
    }

    if(s != NULL) {
        TRACE_TYPE(LOOKUP, "%5d %s s->st_value = 0x%08x, "
                   "si->base = 0x%08x\n", pid, name, s->st_value, si->base);
        return s;
    }

    return NULL;
}

soinfo *find_containing_library(const void *addr)
{
    soinfo *si;

    for(si = solist; si != NULL; si = si->next)
    {
        if((unsigned)addr >= si->base && (unsigned)addr - si->base < si->size) {
            return si;
        }
    }

    return NULL;
}

Elf32_Sym *soinfo_find_symbol(soinfo* si, const void *addr)
{
    unsigned int i;
    unsigned soaddr = (unsigned)addr - si->base;

    /* Search the library's symbol table for any defined symbol which
     * contains this address */
    for(i=0; i<si->nchain; i++) {
        Elf32_Sym *sym = &si->symtab[i];

        if(sym->st_shndx != SHN_UNDEF &&
           soaddr >= sym->st_value &&
           soaddr < sym->st_value + sym->st_size) {
            return sym;
        }
    }

    return NULL;
}

#if 0
static void dump(soinfo *si)
{
    Elf32_Sym *s = si->symtab;
    unsigned n;

    for(n = 0; n < si->nchain; n++) {
        TRACE("%5d %04d> %08x: %02x %04x %08x %08x %s\n", pid, n, s,
               s->st_info, s->st_shndx, s->st_value, s->st_size,
               si->strtab + s->st_name);
        s++;
    }
}
#endif

static const char * const sopaths[] = {
    "/vendor/lib",
    "/system/lib",
    0
};

static int _open_lib(const char* name) {
    // TODO: why not just call open?
    struct stat sb;
    if (stat(name, &sb) == -1 || !S_ISREG(sb.st_mode)) {
        return -1;
    }
    return TEMP_FAILURE_RETRY(open(name, O_RDONLY));
}

static int open_library(const char *name)
{
    int fd;
    char buf[512];
    const char * const*path;
    int n;

    TRACE("[ %5d opening %s ]\n", pid, name);

    if(name == 0) return -1;
    if(strlen(name) > 256) return -1;

    if ((name[0] == '/') && ((fd = _open_lib(name)) >= 0))
        return fd;

    for (path = ldpaths; *path; path++) {
        n = format_buffer(buf, sizeof(buf), "%s/%s", *path, name);
        if (n < 0 || n >= (int)sizeof(buf)) {
            WARN("Ignoring very long library path: %s/%s\n", *path, name);
            continue;
        }
        if ((fd = _open_lib(buf)) >= 0)
            return fd;
    }
    for (path = sopaths; *path; path++) {
        n = format_buffer(buf, sizeof(buf), "%s/%s", *path, name);
        if (n < 0 || n >= (int)sizeof(buf)) {
            WARN("Ignoring very long library path: %s/%s\n", *path, name);
            continue;
        }
        if ((fd = _open_lib(buf)) >= 0)
            return fd;
    }

    return -1;
}

// Returns 'true' if the library is prelinked or on failure so we error out
// either way. We no longer support prelinking.
static bool is_prelinked(int fd, const char* name)
{
    struct prelink_info_t {
        long mmap_addr;
        char tag[4]; // "PRE ".
    };

    off_t sz = lseek(fd, -sizeof(prelink_info_t), SEEK_END);
    if (sz < 0) {
        DL_ERR("lseek failed: %s", strerror(errno));
        return true;
    }

    prelink_info_t info;
    int rc = TEMP_FAILURE_RETRY(read(fd, &info, sizeof(info)));
    if (rc != sizeof(info)) {
        DL_ERR("could not read prelink_info_t structure for \"%s\":", name, strerror(errno));
        return true;
    }

    if (memcmp(info.tag, "PRE ", 4) == 0) {
        DL_ERR("prelinked libraries no longer supported: %s", name);
        return true;
    }
    return false;
}

/* verify_elf_header
 *      Verifies the content of an ELF header.
 *
 * Args:
 *
 * Returns:
 *       0 on success
 *      -1 if no valid ELF object is found @ base.
 */
static int
verify_elf_header(const Elf32_Ehdr* hdr)
{
    if (hdr->e_ident[EI_MAG0] != ELFMAG0) return -1;
    if (hdr->e_ident[EI_MAG1] != ELFMAG1) return -1;
    if (hdr->e_ident[EI_MAG2] != ELFMAG2) return -1;
    if (hdr->e_ident[EI_MAG3] != ELFMAG3) return -1;
    if (hdr->e_type != ET_DYN) return -1;

    /* TODO: Should we verify anything else in the header? */
#ifdef ANDROID_ARM_LINKER
    if (hdr->e_machine != EM_ARM) return -1;
#elif defined(ANDROID_X86_LINKER)
    if (hdr->e_machine != EM_386) return -1;
#elif defined(ANDROID_MIPS_LINKER)
    if (hdr->e_machine != EM_MIPS) return -1;
#endif
    return 0;
}

struct scoped_fd {
    ~scoped_fd() {
        if (fd != -1) {
            close(fd);
        }
    }
    int fd;
};

struct soinfo_ptr {
    soinfo_ptr(const char* name) {
        const char* bname = strrchr(name, '/');
        ptr = soinfo_alloc(bname ? bname + 1 : name);
    }
    ~soinfo_ptr() {
        soinfo_free(ptr);
    }
    soinfo* release() {
        soinfo* result = ptr;
        ptr = NULL;
        return result;
    }
    soinfo* ptr;
};

// TODO: rewrite linker_phdr.h to use a class, then lose this.
struct phdr_ptr {
    phdr_ptr() : phdr_mmap(NULL) {}
    ~phdr_ptr() {
        if (phdr_mmap != NULL) {
            phdr_table_unload(phdr_mmap, phdr_size);
        }
    }
    void* phdr_mmap;
    Elf32_Addr phdr_size;
};

static soinfo* load_library(const char* name)
{
    // Open the file.
    scoped_fd fd;
    fd.fd = open_library(name);
    if (fd.fd == -1) {
        DL_ERR("library \"%s\" not found", name);
        return NULL;
    }

    // Read the ELF header.
    Elf32_Ehdr header[1];
    int ret = TEMP_FAILURE_RETRY(read(fd.fd, (void*)header, sizeof(header)));
    if (ret < 0) {
        DL_ERR("can't read file \"%s\": %s", name, strerror(errno));
        return NULL;
    }
    if (ret != (int)sizeof(header)) {
        DL_ERR("too small to be an ELF executable: %s", name);
        return NULL;
    }
    if (verify_elf_header(header) < 0) {
        DL_ERR("not a valid ELF executable: %s", name);
        return NULL;
    }

    // Read the program header table.
    const Elf32_Phdr* phdr_table;
    phdr_ptr phdr_holder;
    ret = phdr_table_load(fd.fd, header->e_phoff, header->e_phnum,
                          &phdr_holder.phdr_mmap, &phdr_holder.phdr_size, &phdr_table);
    if (ret < 0) {
        DL_ERR("can't load program header table: %s: %s", name, strerror(errno));
        return NULL;
    }
    size_t phdr_count = header->e_phnum;

    // Get the load extents.
    Elf32_Addr ext_sz = phdr_table_get_load_size(phdr_table, phdr_count);
    TRACE("[ %5d - '%s' wants sz=0x%08x ]\n", pid, name, ext_sz);
    if (ext_sz == 0) {
        DL_ERR("no loadable segments in file: %s", name);
        return NULL;
    }

    // We no longer support pre-linked libraries.
    if (is_prelinked(fd.fd, name)) {
        return NULL;
    }

    // Reserve address space for all loadable segments.
    void* load_start = NULL;
    Elf32_Addr load_size = 0;
    Elf32_Addr load_bias = 0;
    ret = phdr_table_reserve_memory(phdr_table,
                                    phdr_count,
                                    &load_start,
                                    &load_size,
                                    &load_bias);
    if (ret < 0) {
        DL_ERR("can't reserve %d bytes in address space for \"%s\": %s",
               ext_sz, name, strerror(errno));
        return NULL;
    }

    TRACE("[ %5d allocated memory for %s @ %p (0x%08x) ]\n",
          pid, name, load_start, load_size);

    /* Map all the segments in our address space with default protections */
    ret = phdr_table_load_segments(phdr_table,
                                   phdr_count,
                                   load_bias,
                                   fd.fd);
    if (ret < 0) {
        DL_ERR("can't map loadable segments for \"%s\": %s",
               name, strerror(errno));
        return NULL;
    }

    soinfo_ptr si(name);
    if (si.ptr == NULL) {
        return NULL;
    }

    si.ptr->base = (Elf32_Addr) load_start;
    si.ptr->size = load_size;
    si.ptr->load_bias = load_bias;
    si.ptr->flags = 0;
    si.ptr->entry = 0;
    si.ptr->dynamic = (unsigned *)-1;
    si.ptr->phnum = phdr_count;
    si.ptr->phdr = phdr_table_get_loaded_phdr(phdr_table, phdr_count, load_bias);
    if (si.ptr->phdr == NULL) {
        DL_ERR("can't find loaded PHDR for \"%s\"", name);
        return NULL;
    }

    return si.release();
}

static soinfo *
init_library(soinfo *si)
{
    /* At this point we know that whatever is loaded @ base is a valid ELF
     * shared library whose segments are properly mapped in. */
    TRACE("[ %5d init_library base=0x%08x sz=0x%08x name='%s') ]\n",
          pid, si->base, si->size, si->name);

    if(soinfo_link_image(si)) {
        munmap((void *)si->base, si->size);
        return NULL;
    }

    return si;
}

static soinfo *find_loaded_library(const char *name)
{
    soinfo *si;
    const char *bname;

    // TODO: don't use basename only for determining libraries
    // http://code.google.com/p/android/issues/detail?id=6670

    bname = strrchr(name, '/');
    bname = bname ? bname + 1 : name;

    for(si = solist; si != NULL; si = si->next){
        if(!strcmp(bname, si->name)) {
            return si;
        }
    }
    return NULL;
}

soinfo *find_library(const char *name)
{
    soinfo *si;

#if ALLOW_SYMBOLS_FROM_MAIN
    if (name == NULL)
        return somain;
#else
    if (name == NULL)
        return NULL;
#endif

    si = find_loaded_library(name);
    if (si != NULL) {
        if(si->flags & FLAG_ERROR) {
            DL_ERR("\"%s\" failed to load previously", name);
            return NULL;
        }
        if(si->flags & FLAG_LINKED) return si;
        DL_ERR("OOPS: recursive link to \"%s\"", si->name);
        return NULL;
    }

    TRACE("[ %5d '%s' has not been loaded yet.  Locating...]\n", pid, name);
    si = load_library(name);
    if(si == NULL)
        return NULL;
    return init_library(si);
}

static void call_destructors(soinfo *si);

int soinfo_unload(soinfo* si) {
    if (si->refcount == 1) {
        TRACE("%5d unloading '%s'\n", pid, si->name);
        call_destructors(si);

        for (unsigned* d = si->dynamic; *d; d += 2) {
            if(d[0] == DT_NEEDED){
                soinfo *lsi = find_loaded_library(si->strtab + d[1]);
                if (lsi) {
                    TRACE("%5d %s needs to unload %s\n", pid,
                          si->name, lsi->name);
                    soinfo_unload(lsi);
                } else {
                    // TODO: should we return -1 in this case?
                    DL_ERR("\"%s\": could not unload dependent library",
                           si->name);
                }
            }
        }

        munmap((char *)si->base, si->size);
        notify_gdb_of_unload(si);
        soinfo_free(si);
        si->refcount = 0;
    } else {
        si->refcount--;
        PRINT("%5d not unloading '%s', decrementing refcount to %d\n",
              pid, si->name, si->refcount);
    }
    return 0;
}

/* TODO: don't use unsigned for addrs below. It works, but is not
 * ideal. They should probably be either uint32_t, Elf32_Addr, or unsigned
 * long.
 */
static int soinfo_relocate(soinfo *si, Elf32_Rel *rel, unsigned count,
                           soinfo *needed[])
{
    Elf32_Sym *symtab = si->symtab;
    const char *strtab = si->strtab;
    Elf32_Sym *s;
    Elf32_Addr offset;
    Elf32_Rel *start = rel;

    for (size_t idx = 0; idx < count; ++idx, ++rel) {
        unsigned type = ELF32_R_TYPE(rel->r_info);
        unsigned sym = ELF32_R_SYM(rel->r_info);
        unsigned reloc = (unsigned)(rel->r_offset + si->load_bias);
        unsigned sym_addr = 0;
        char *sym_name = NULL;

        DEBUG("%5d Processing '%s' relocation at index %d\n", pid,
              si->name, idx);
        if (type == 0) { // R_*_NONE
            continue;
        }
        if(sym != 0) {
            sym_name = (char *)(strtab + symtab[sym].st_name);
            bool ignore_local = false;
#if defined(ANDROID_ARM_LINKER)
            ignore_local = (type == R_ARM_COPY);
#endif
            s = soinfo_do_lookup(si, sym_name, &offset, needed, ignore_local);
            if(s == NULL) {
                /* We only allow an undefined symbol if this is a weak
                   reference..   */
                s = &symtab[sym];
                if (ELF32_ST_BIND(s->st_info) != STB_WEAK) {
                    DL_ERR("cannot locate symbol \"%s\" referenced by \"%s\"...", sym_name, si->name);
                    return -1;
                }

                /* IHI0044C AAELF 4.5.1.1:

                   Libraries are not searched to resolve weak references.
                   It is not an error for a weak reference to remain
                   unsatisfied.

                   During linking, the value of an undefined weak reference is:
                   - Zero if the relocation type is absolute
                   - The address of the place if the relocation is pc-relative
                   - The address of nominal base address if the relocation
                     type is base-relative.
                  */

                switch (type) {
#if defined(ANDROID_ARM_LINKER)
                case R_ARM_JUMP_SLOT:
                case R_ARM_GLOB_DAT:
                case R_ARM_ABS32:
                case R_ARM_RELATIVE:    /* Don't care. */
#elif defined(ANDROID_X86_LINKER)
                case R_386_JMP_SLOT:
                case R_386_GLOB_DAT:
                case R_386_32:
                case R_386_RELATIVE:    /* Dont' care. */
#endif /* ANDROID_*_LINKER */
                    /* sym_addr was initialized to be zero above or relocation
                       code below does not care about value of sym_addr.
                       No need to do anything.  */
                    break;

#if defined(ANDROID_X86_LINKER)
                case R_386_PC32:
                    sym_addr = reloc;
                    break;
#endif /* ANDROID_X86_LINKER */

#if defined(ANDROID_ARM_LINKER)
                case R_ARM_COPY:
                    /* Fall through.  Can't really copy if weak symbol is
                       not found in run-time.  */
#endif /* ANDROID_ARM_LINKER */
                default:
                    DL_ERR("unknown weak reloc type %d @ %p (%d)",
                                 type, rel, (int) (rel - start));
                    return -1;
                }
            } else {
                /* We got a definition.  */
#if 0
                if((base == 0) && (si->base != 0)){
                        /* linking from libraries to main image is bad */
                    DL_ERR("cannot locate \"%s\"...",
                           strtab + symtab[sym].st_name);
                    return -1;
                }
#endif
                sym_addr = (unsigned)(s->st_value + offset);
            }
            count_relocation(kRelocSymbol);
        } else {
            s = NULL;
        }

/* TODO: This is ugly. Split up the relocations by arch into
 * different files.
 */
        switch(type){
#if defined(ANDROID_ARM_LINKER)
        case R_ARM_JUMP_SLOT:
            count_relocation(kRelocAbsolute);
            MARK(rel->r_offset);
            TRACE_TYPE(RELO, "%5d RELO JMP_SLOT %08x <- %08x %s\n", pid,
                       reloc, sym_addr, sym_name);
            *((unsigned*)reloc) = sym_addr;
            break;
        case R_ARM_GLOB_DAT:
            count_relocation(kRelocAbsolute);
            MARK(rel->r_offset);
            TRACE_TYPE(RELO, "%5d RELO GLOB_DAT %08x <- %08x %s\n", pid,
                       reloc, sym_addr, sym_name);
            *((unsigned*)reloc) = sym_addr;
            break;
        case R_ARM_ABS32:
            count_relocation(kRelocAbsolute);
            MARK(rel->r_offset);
            TRACE_TYPE(RELO, "%5d RELO ABS %08x <- %08x %s\n", pid,
                       reloc, sym_addr, sym_name);
            *((unsigned*)reloc) += sym_addr;
            break;
        case R_ARM_REL32:
            count_relocation(kRelocRelative);
            MARK(rel->r_offset);
            TRACE_TYPE(RELO, "%5d RELO REL32 %08x <- %08x - %08x %s\n", pid,
                       reloc, sym_addr, rel->r_offset, sym_name);
            *((unsigned*)reloc) += sym_addr - rel->r_offset;
            break;
#elif defined(ANDROID_X86_LINKER)
        case R_386_JMP_SLOT:
            count_relocation(kRelocAbsolute);
            MARK(rel->r_offset);
            TRACE_TYPE(RELO, "%5d RELO JMP_SLOT %08x <- %08x %s\n", pid,
                       reloc, sym_addr, sym_name);
            *((unsigned*)reloc) = sym_addr;
            break;
        case R_386_GLOB_DAT:
            count_relocation(kRelocAbsolute);
            MARK(rel->r_offset);
            TRACE_TYPE(RELO, "%5d RELO GLOB_DAT %08x <- %08x %s\n", pid,
                       reloc, sym_addr, sym_name);
            *((unsigned*)reloc) = sym_addr;
            break;
#elif defined(ANDROID_MIPS_LINKER)
    case R_MIPS_JUMP_SLOT:
            count_relocation(kRelocAbsolute);
            MARK(rel->r_offset);
            TRACE_TYPE(RELO, "%5d RELO JMP_SLOT %08x <- %08x %s\n", pid,
                       reloc, sym_addr, sym_name);
            *((unsigned*)reloc) = sym_addr;
            break;
    case R_MIPS_REL32:
            count_relocation(kRelocAbsolute);
            MARK(rel->r_offset);
            TRACE_TYPE(RELO, "%5d RELO REL32 %08x <- %08x %s\n", pid,
                       reloc, sym_addr, (sym_name) ? sym_name : "*SECTIONHDR*");
            if (s) {
                *((unsigned*)reloc) += sym_addr;
            } else {
                *((unsigned*)reloc) += si->base;
            }
            break;
#endif /* ANDROID_*_LINKER */

#if defined(ANDROID_ARM_LINKER)
        case R_ARM_RELATIVE:
#elif defined(ANDROID_X86_LINKER)
        case R_386_RELATIVE:
#endif /* ANDROID_*_LINKER */
            count_relocation(kRelocRelative);
            MARK(rel->r_offset);
            if (sym) {
                DL_ERR("odd RELATIVE form...", pid);
                return -1;
            }
            TRACE_TYPE(RELO, "%5d RELO RELATIVE %08x <- +%08x\n", pid,
                       reloc, si->base);
            *((unsigned*)reloc) += si->base;
            break;

#if defined(ANDROID_X86_LINKER)
        case R_386_32:
            count_relocation(kRelocRelative);
            MARK(rel->r_offset);

            TRACE_TYPE(RELO, "%5d RELO R_386_32 %08x <- +%08x %s\n", pid,
                       reloc, sym_addr, sym_name);
            *((unsigned *)reloc) += (unsigned)sym_addr;
            break;

        case R_386_PC32:
            count_relocation(kRelocRelative);
            MARK(rel->r_offset);
            TRACE_TYPE(RELO, "%5d RELO R_386_PC32 %08x <- "
                       "+%08x (%08x - %08x) %s\n", pid, reloc,
                       (sym_addr - reloc), sym_addr, reloc, sym_name);
            *((unsigned *)reloc) += (unsigned)(sym_addr - reloc);
            break;
#endif /* ANDROID_X86_LINKER */

#ifdef ANDROID_ARM_LINKER
        case R_ARM_COPY:
            if ((si->flags & FLAG_EXE) == 0) {
                /*
                 * http://infocenter.arm.com/help/topic/com.arm.doc.ihi0044d/IHI0044D_aaelf.pdf
                 *
                 * Section 4.7.1.10 "Dynamic relocations"
                 * R_ARM_COPY may only appear in executable objects where e_type is
                 * set to ET_EXEC.
                 *
                 * TODO: FLAG_EXE is set for both ET_DYN and ET_EXEC executables.
                 * We should explicitly disallow ET_DYN executables from having
                 * R_ARM_COPY relocations.
                 */
                DL_ERR("%s R_ARM_COPY relocations only supported for ET_EXEC", si->name);
                return -1;
            }
            count_relocation(kRelocCopy);
            MARK(rel->r_offset);
            TRACE_TYPE(RELO, "%5d RELO %08x <- %d @ %08x %s\n", pid,
                       reloc, s->st_size, sym_addr, sym_name);
            if (reloc == sym_addr) {
                DL_ERR("Internal linker error detected. reloc == symaddr");
                return -1;
            }
            memcpy((void*)reloc, (void*)sym_addr, s->st_size);
            break;
#endif /* ANDROID_ARM_LINKER */

        default:
            DL_ERR("unknown reloc type %d @ %p (%d)",
                   type, rel, (int) (rel - start));
            return -1;
        }
    }
    return 0;
}

#ifdef ANDROID_MIPS_LINKER
static int mips_relocate_got(soinfo* si, soinfo* needed[]) {
    unsigned *got;
    unsigned local_gotno, gotsym, symtabno;
    Elf32_Sym *symtab, *sym;
    unsigned g;

    got = si->plt_got;
    local_gotno = si->mips_local_gotno;
    gotsym = si->mips_gotsym;
    symtabno = si->mips_symtabno;
    symtab = si->symtab;

    /*
     * got[0] is address of lazy resolver function
     * got[1] may be used for a GNU extension
     * set it to a recognizable address in case someone calls it
     * (should be _rtld_bind_start)
     * FIXME: maybe this should be in a separate routine
     */

    if ((si->flags & FLAG_LINKER) == 0) {
        g = 0;
        got[g++] = 0xdeadbeef;
        if (got[g] & 0x80000000) {
            got[g++] = 0xdeadfeed;
        }
        /*
         * Relocate the local GOT entries need to be relocated
         */
        for (; g < local_gotno; g++) {
            got[g] += si->load_bias;
        }
    }

    /* Now for the global GOT entries */
    sym = symtab + gotsym;
    got = si->plt_got + local_gotno;
    for (g = gotsym; g < symtabno; g++, sym++, got++) {
        const char *sym_name;
        unsigned base;
        Elf32_Sym *s;

        /* This is an undefined reference... try to locate it */
        sym_name = si->strtab + sym->st_name;
        s = soinfo_do_lookup(si, sym_name, &base, needed, false);
        if (s == NULL) {
            /* We only allow an undefined symbol if this is a weak
               reference..   */
            s = &symtab[g];
            if (ELF32_ST_BIND(s->st_info) != STB_WEAK) {
                DL_ERR("cannot locate \"%s\"...", sym_name);
                return -1;
            }
            *got = 0;
        }
        else {
            /* FIXME: is this sufficient?
             * For reference see NetBSD link loader
             * http://cvsweb.netbsd.org/bsdweb.cgi/src/libexec/ld.elf_so/arch/mips/mips_reloc.c?rev=1.53&content-type=text/x-cvsweb-markup
             */
             *got = base + s->st_value;
        }
    }
    return 0;
}
#endif

/* Please read the "Initialization and Termination functions" functions.
 * of the linker design note in bionic/linker/README.TXT to understand
 * what the following code is doing.
 *
 * The important things to remember are:
 *
 *   DT_PREINIT_ARRAY must be called first for executables, and should
 *   not appear in shared libraries.
 *
 *   DT_INIT should be called before DT_INIT_ARRAY if both are present
 *
 *   DT_FINI should be called after DT_FINI_ARRAY if both are present
 *
 *   DT_FINI_ARRAY must be parsed in reverse order.
 */

static void call_array(unsigned *ctor, int count, int reverse)
{
    int n, inc = 1;

    if (reverse) {
        ctor += (count-1);
        inc   = -1;
    }

    for(n = count; n > 0; n--) {
        TRACE("[ %5d Looking at %s *0x%08x == 0x%08x ]\n", pid,
              reverse ? "dtor" : "ctor",
              (unsigned)ctor, (unsigned)*ctor);
        void (*func)() = (void (*)()) *ctor;
        ctor += inc;
        if(((int) func == 0) || ((int) func == -1)) continue;
        TRACE("[ %5d Calling func @ 0x%08x ]\n", pid, (unsigned)func);
        func();
    }
}

static void soinfo_call_preinit_constructors(soinfo *si)
{
  TRACE("[ %5d Calling preinit_array @ 0x%08x [%d] for '%s' ]\n",
      pid, (unsigned)si->preinit_array, si->preinit_array_count,
      si->name);
  call_array(si->preinit_array, si->preinit_array_count, 0);
  TRACE("[ %5d Done calling preinit_array for '%s' ]\n", pid, si->name);
}

void soinfo_call_constructors(soinfo *si)
{
    if (si->constructors_called)
        return;

    // Set this before actually calling the constructors, otherwise it doesn't
    // protect against recursive constructor calls. One simple example of
    // constructor recursion is the libc debug malloc, which is implemented in
    // libc_malloc_debug_leak.so:
    // 1. The program depends on libc, so libc's constructor is called here.
    // 2. The libc constructor calls dlopen() to load libc_malloc_debug_leak.so.
    // 3. dlopen() calls soinfo_call_constructors() with the newly created
    //    soinfo for libc_malloc_debug_leak.so.
    // 4. The debug so depends on libc, so soinfo_call_constructors() is
    //    called again with the libc soinfo. If it doesn't trigger the early-
    //    out above, the libc constructor will be called again (recursively!).
    si->constructors_called = 1;

    if (!(si->flags & FLAG_EXE) && si->preinit_array) {
      DL_ERR("shared library \"%s\" has a preinit_array table @ 0x%08x. "
          "This is INVALID.", si->name, (unsigned) si->preinit_array);
    }

    if (si->dynamic) {
        unsigned *d;
        for(d = si->dynamic; *d; d += 2) {
            if(d[0] == DT_NEEDED){
                soinfo* lsi = find_loaded_library(si->strtab + d[1]);
                if (!lsi) {
                    DL_ERR("\"%s\": could not initialize dependent library",
                           si->name);
                } else {
                    soinfo_call_constructors(lsi);
                }
            }
        }
    }

    if (si->init_func) {
        TRACE("[ %5d Calling init_func @ 0x%08x for '%s' ]\n", pid,
              (unsigned)si->init_func, si->name);
        si->init_func();
        TRACE("[ %5d Done calling init_func for '%s' ]\n", pid, si->name);
    }

    if (si->init_array) {
        TRACE("[ %5d Calling init_array @ 0x%08x [%d] for '%s' ]\n", pid,
              (unsigned)si->init_array, si->init_array_count, si->name);
        call_array(si->init_array, si->init_array_count, 0);
        TRACE("[ %5d Done calling init_array for '%s' ]\n", pid, si->name);
    }

}

static void call_destructors(soinfo *si)
{
    if (si->fini_array) {
        TRACE("[ %5d Calling fini_array @ 0x%08x [%d] for '%s' ]\n", pid,
              (unsigned)si->fini_array, si->fini_array_count, si->name);
        call_array(si->fini_array, si->fini_array_count, 1);
        TRACE("[ %5d Done calling fini_array for '%s' ]\n", pid, si->name);
    }

    if (si->fini_func) {
        TRACE("[ %5d Calling fini_func @ 0x%08x for '%s' ]\n", pid,
              (unsigned)si->fini_func, si->name);
        si->fini_func();
        TRACE("[ %5d Done calling fini_func for '%s' ]\n", pid, si->name);
    }
}

/* Force any of the closed stdin, stdout and stderr to be associated with
   /dev/null. */
static int nullify_closed_stdio (void)
{
    int dev_null, i, status;
    int return_value = 0;

    dev_null = TEMP_FAILURE_RETRY(open("/dev/null", O_RDWR));
    if (dev_null < 0) {
        DL_ERR("cannot open /dev/null: %s", strerror(errno));
        return -1;
    }
    TRACE("[ %5d Opened /dev/null file-descriptor=%d]\n", pid, dev_null);

    /* If any of the stdio file descriptors is valid and not associated
       with /dev/null, dup /dev/null to it.  */
    for (i = 0; i < 3; i++) {
        /* If it is /dev/null already, we are done. */
        if (i == dev_null) {
            continue;
        }

        TRACE("[ %5d Nullifying stdio file descriptor %d]\n", pid, i);
        status = TEMP_FAILURE_RETRY(fcntl(i, F_GETFL));

        /* If file is opened, we are good. */
        if (status != -1) {
            continue;
        }

        /* The only error we allow is that the file descriptor does not
           exist, in which case we dup /dev/null to it. */
        if (errno != EBADF) {
            DL_ERR("fcntl failed: %s", strerror(errno));
            return_value = -1;
            continue;
        }

        /* Try dupping /dev/null to this stdio file descriptor and
           repeat if there is a signal.  Note that any errors in closing
           the stdio descriptor are lost.  */
        status = TEMP_FAILURE_RETRY(dup2(dev_null, i));
        if (status < 0) {
            DL_ERR("dup2 failed: %s", strerror(errno));
            return_value = -1;
            continue;
        }
    }

    /* If /dev/null is not one of the stdio file descriptors, close it. */
    if (dev_null > 2) {
        TRACE("[ %5d Closing /dev/null file-descriptor=%d]\n", pid, dev_null);
        status = TEMP_FAILURE_RETRY(close(dev_null));
        if (status == -1) {
            DL_ERR("close failed: %s", strerror(errno));
            return_value = -1;
        }
    }

    return return_value;
}

static int soinfo_link_image(soinfo *si)
{
    unsigned *d;
    /* "base" might wrap around UINT32_MAX. */
    Elf32_Addr base = si->load_bias;
    const Elf32_Phdr *phdr = si->phdr;
    int phnum = si->phnum;
    int relocating_linker = (si->flags & FLAG_LINKER) != 0;
    soinfo **needed, **pneeded;
    size_t dynamic_count;

    /* We can't debug anything until the linker is relocated */
    if (!relocating_linker) {
        INFO("[ %5d linking %s ]\n", pid, si->name);
        DEBUG("%5d si->base = 0x%08x si->flags = 0x%08x\n", pid,
            si->base, si->flags);
    }

    /* Extract dynamic section */
    phdr_table_get_dynamic_section(phdr, phnum, base, &si->dynamic,
                                   &dynamic_count);
    if (si->dynamic == NULL) {
        if (!relocating_linker) {
            DL_ERR("missing PT_DYNAMIC?!");
        }
        goto fail;
    } else {
        if (!relocating_linker) {
            DEBUG("%5d dynamic = %p\n", pid, si->dynamic);
        }
    }

#ifdef ANDROID_ARM_LINKER
    (void) phdr_table_get_arm_exidx(phdr, phnum, base,
                                    &si->ARM_exidx, &si->ARM_exidx_count);
#endif

    /* extract useful information from dynamic section */
    for(d = si->dynamic; *d; d++){
        DEBUG("%5d d = %p, d[0] = 0x%08x d[1] = 0x%08x\n", pid, d, d[0], d[1]);
        switch(*d++){
        case DT_HASH:
            si->nbucket = ((unsigned *) (base + *d))[0];
            si->nchain = ((unsigned *) (base + *d))[1];
            si->bucket = (unsigned *) (base + *d + 8);
            si->chain = (unsigned *) (base + *d + 8 + si->nbucket * 4);
            break;
        case DT_STRTAB:
            si->strtab = (const char *) (base + *d);
            break;
        case DT_SYMTAB:
            si->symtab = (Elf32_Sym *) (base + *d);
            break;
        case DT_PLTREL:
            if(*d != DT_REL) {
                DL_ERR("DT_RELA not supported");
                goto fail;
            }
            break;
        case DT_JMPREL:
            si->plt_rel = (Elf32_Rel*) (base + *d);
            break;
        case DT_PLTRELSZ:
            si->plt_rel_count = *d / 8;
            break;
        case DT_REL:
            si->rel = (Elf32_Rel*) (base + *d);
            break;
        case DT_RELSZ:
            si->rel_count = *d / 8;
            break;
        case DT_PLTGOT:
            /* Save this in case we decide to do lazy binding. We don't yet. */
            si->plt_got = (unsigned *)(base + *d);
            break;
        case DT_DEBUG:
#if !defined(ANDROID_MIPS_LINKER)
            // Set the DT_DEBUG entry to the address of _r_debug for GDB
            *d = (int) &_r_debug;
#endif
            break;
         case DT_RELA:
            DL_ERR("DT_RELA not supported");
            goto fail;
        case DT_INIT:
            si->init_func = (void (*)(void))(base + *d);
            DEBUG("%5d %s constructors (init func) found at %p\n",
                  pid, si->name, si->init_func);
            break;
        case DT_FINI:
            si->fini_func = (void (*)(void))(base + *d);
            DEBUG("%5d %s destructors (fini func) found at %p\n",
                  pid, si->name, si->fini_func);
            break;
        case DT_INIT_ARRAY:
            si->init_array = (unsigned *)(base + *d);
            DEBUG("%5d %s constructors (init_array) found at %p\n",
                  pid, si->name, si->init_array);
            break;
        case DT_INIT_ARRAYSZ:
            si->init_array_count = ((unsigned)*d) / sizeof(Elf32_Addr);
            break;
        case DT_FINI_ARRAY:
            si->fini_array = (unsigned *)(base + *d);
            DEBUG("%5d %s destructors (fini_array) found at %p\n",
                  pid, si->name, si->fini_array);
            break;
        case DT_FINI_ARRAYSZ:
            si->fini_array_count = ((unsigned)*d) / sizeof(Elf32_Addr);
            break;
        case DT_PREINIT_ARRAY:
            si->preinit_array = (unsigned *)(base + *d);
            DEBUG("%5d %s constructors (preinit_array) found at %p\n",
                  pid, si->name, si->preinit_array);
            break;
        case DT_PREINIT_ARRAYSZ:
            si->preinit_array_count = ((unsigned)*d) / sizeof(Elf32_Addr);
            break;
        case DT_TEXTREL:
            si->has_text_relocations = true;
            break;
#if defined(ANDROID_MIPS_LINKER)
        case DT_NEEDED:
        case DT_STRSZ:
        case DT_SYMENT:
        case DT_RELENT:
             break;
        case DT_MIPS_RLD_MAP:
            // Set the DT_MIPS_RLD_MAP entry to the address of _r_debug for GDB.
            {
              r_debug** dp = (r_debug**) *d;
              *dp = &_r_debug;
            }
            break;
        case DT_MIPS_RLD_VERSION:
        case DT_MIPS_FLAGS:
        case DT_MIPS_BASE_ADDRESS:
        case DT_MIPS_UNREFEXTNO:
        case DT_MIPS_RWPLT:
            break;

        case DT_MIPS_PLTGOT:
#if 0
            /* not yet... */
            si->mips_pltgot = (unsigned *)(si->base + *d);
#endif
            break;

        case DT_MIPS_SYMTABNO:
            si->mips_symtabno = *d;
            break;

        case DT_MIPS_LOCAL_GOTNO:
            si->mips_local_gotno = *d;
            break;

        case DT_MIPS_GOTSYM:
            si->mips_gotsym = *d;
            break;

        default:
            DEBUG("%5d Unused DT entry: type 0x%08x arg 0x%08x\n",
                  pid, d[-1], d[0]);
            break;
#endif
        }
    }

    DEBUG("%5d si->base = 0x%08x, si->strtab = %p, si->symtab = %p\n",
           pid, si->base, si->strtab, si->symtab);

    if((si->strtab == 0) || (si->symtab == 0)) {
        DL_ERR("missing essential tables");
        goto fail;
    }

    /* if this is the main executable, then load all of the preloads now */
    if(si->flags & FLAG_EXE) {
        int i;
        memset(preloads, 0, sizeof(preloads));
        for(i = 0; ldpreload_names[i] != NULL; i++) {
            soinfo *lsi = find_library(ldpreload_names[i]);
            if(lsi == 0) {
                strlcpy(tmp_err_buf, linker_get_error(), sizeof(tmp_err_buf));
                DL_ERR("could not load library \"%s\" needed by \"%s\"; caused by %s",
                       ldpreload_names[i], si->name, tmp_err_buf);
                goto fail;
            }
            lsi->refcount++;
            preloads[i] = lsi;
        }
    }

    /* dynamic_count is an upper bound for the number of needed libs */
    pneeded = needed = (soinfo**) alloca((1 + dynamic_count) * sizeof(soinfo*));

    for(d = si->dynamic; *d; d += 2) {
        if(d[0] == DT_NEEDED){
            DEBUG("%5d %s needs %s\n", pid, si->name, si->strtab + d[1]);
            soinfo *lsi = find_library(si->strtab + d[1]);
            if(lsi == 0) {
                strlcpy(tmp_err_buf, linker_get_error(), sizeof(tmp_err_buf));
                DL_ERR("could not load library \"%s\" needed by \"%s\"; caused by %s",
                       si->strtab + d[1], si->name, tmp_err_buf);
                goto fail;
            }
            *pneeded++ = lsi;
            lsi->refcount++;
        }
    }
    *pneeded = NULL;

    if (si->has_text_relocations) {
        /* Unprotect the segments, i.e. make them writable, to allow
         * text relocations to work properly. We will later call
         * phdr_table_protect_segments() after all of them are applied
         * and all constructors are run.
         */
        if (phdr_table_unprotect_segments(si->phdr, si->phnum, si->load_bias) < 0) {
            DL_ERR("can't unprotect loadable segments for \"%s\": %s",
                   si->name, strerror(errno));
            goto fail;
        }
    }

    if(si->plt_rel) {
        DEBUG("[ %5d relocating %s plt ]\n", pid, si->name );
        if(soinfo_relocate(si, si->plt_rel, si->plt_rel_count, needed))
            goto fail;
    }
    if(si->rel) {
        DEBUG("[ %5d relocating %s ]\n", pid, si->name );
        if(soinfo_relocate(si, si->rel, si->rel_count, needed))
            goto fail;
    }

#ifdef ANDROID_MIPS_LINKER
    if(mips_relocate_got(si, needed)) {
        goto fail;
    }
#endif

    si->flags |= FLAG_LINKED;
    DEBUG("[ %5d finished linking %s ]\n", pid, si->name);

    if (si->has_text_relocations) {
        /* All relocations are done, we can protect our segments back to
         * read-only. */
        if (phdr_table_protect_segments(si->phdr, si->phnum, si->load_bias) < 0) {
            DL_ERR("can't protect segments for \"%s\": %s",
                   si->name, strerror(errno));
            goto fail;
        }
    }

    /* We can also turn on GNU RELRO protection */
    if (phdr_table_protect_gnu_relro(si->phdr, si->phnum, si->load_bias) < 0) {
        DL_ERR("can't enable GNU RELRO protection for \"%s\": %s",
               si->name, strerror(errno));
        goto fail;
    }

    /* If this is a SET?ID program, dup /dev/null to opened stdin,
       stdout and stderr to close a security hole described in:

    ftp://ftp.freebsd.org/pub/FreeBSD/CERT/advisories/FreeBSD-SA-02:23.stdio.asc

     */
    if (program_is_setuid) {
        nullify_closed_stdio();
    }
    notify_gdb_of_load(si);
    return 0;

fail:
    ERROR("failed to link %s\n", si->name);
    si->flags |= FLAG_ERROR;
    return -1;
}

static void parse_path(const char* path, const char* delimiters,
                       const char** array, char* buf, size_t buf_size, size_t max_count)
{
    if (path == NULL) {
        return;
    }

    size_t len = strlcpy(buf, path, buf_size);

    size_t i = 0;
    char* buf_p = buf;
    while (i < max_count && (array[i] = strsep(&buf_p, delimiters))) {
        if (*array[i] != '\0') {
            ++i;
        }
    }

    // Forget the last path if we had to truncate; this occurs if the 2nd to
    // last char isn't '\0' (i.e. wasn't originally a delimiter).
    if (i > 0 && len >= buf_size && buf[buf_size - 2] != '\0') {
        array[i - 1] = NULL;
    } else {
        array[i] = NULL;
    }
}

static void parse_LD_LIBRARY_PATH(const char* path) {
    parse_path(path, ":", ldpaths,
               ldpaths_buf, sizeof(ldpaths_buf), LDPATH_MAX);
}

static void parse_LD_PRELOAD(const char* path) {
    // We have historically supported ':' as well as ' ' in LD_PRELOAD.
    parse_path(path, " :", ldpreload_names,
               ldpreloads_buf, sizeof(ldpreloads_buf), LDPRELOAD_MAX);
}

/*
 * This code is called after the linker has linked itself and
 * fixed it's own GOT. It is safe to make references to externs
 * and other non-local data at this point.
 */
static unsigned __linker_init_post_relocation(unsigned **elfdata, unsigned linker_base)
{
    static soinfo linker_soinfo;

    int argc = (int) *elfdata;
    char **argv = (char**) (elfdata + 1);
    unsigned *vecs = (unsigned*) (argv + argc + 1);
    unsigned *v;
    soinfo *si;
    int i;
    const char *ldpath_env = NULL;
    const char *ldpreload_env = NULL;

    /* NOTE: we store the elfdata pointer on a special location
     *       of the temporary TLS area in order to pass it to
     *       the C Library's runtime initializer.
     *
     *       The initializer must clear the slot and reset the TLS
     *       to point to a different location to ensure that no other
     *       shared library constructor can access it.
     */
    __libc_init_tls(elfdata);

    pid = getpid();

#if TIMING
    struct timeval t0, t1;
    gettimeofday(&t0, 0);
#endif

    /* Initialize environment functions, and get to the ELF aux vectors table */
    vecs = linker_env_init(vecs);

    /* Check auxv for AT_SECURE first to see if program is setuid, setgid,
       has file caps, or caused a SELinux/AppArmor domain transition. */
    for (v = vecs; v[0]; v += 2) {
        if (v[0] == AT_SECURE) {
            /* kernel told us whether to enable secure mode */
            program_is_setuid = v[1];
            goto sanitize;
        }
    }

    /* Kernel did not provide AT_SECURE - fall back on legacy test. */
    program_is_setuid = (getuid() != geteuid()) || (getgid() != getegid());

sanitize:
    /* Sanitize environment if we're loading a setuid program */
    if (program_is_setuid) {
        linker_env_secure();
    }

    debugger_init();

    /* Get a few environment variables */
    {
#if LINKER_DEBUG
        const char* env;
        env = linker_env_get("DEBUG"); /* XXX: TODO: Change to LD_DEBUG */
        if (env)
            debug_verbosity = atoi(env);
#endif

        /* Normally, these are cleaned by linker_env_secure, but the test
         * against program_is_setuid doesn't cost us anything */
        if (!program_is_setuid) {
            ldpath_env = linker_env_get("LD_LIBRARY_PATH");
            ldpreload_env = linker_env_get("LD_PRELOAD");
        }
    }

    INFO("[ android linker & debugger ]\n");
    DEBUG("%5d elfdata @ 0x%08x\n", pid, (unsigned)elfdata);

    si = soinfo_alloc(argv[0]);
    if(si == 0) {
        exit(-1);
    }

    /* bootstrap the link map, the main exe always needs to be first */
    si->flags |= FLAG_EXE;
    link_map* map = &(si->linkmap);

    map->l_addr = 0;
    map->l_name = argv[0];
    map->l_prev = NULL;
    map->l_next = NULL;

    _r_debug.r_map = map;
    r_debug_tail = map;

        /* gdb expects the linker to be in the debug shared object list.
         * Without this, gdb has trouble locating the linker's ".text"
         * and ".plt" sections. Gdb could also potentially use this to
         * relocate the offset of our exported 'rtld_db_dlactivity' symbol.
         * Don't use soinfo_alloc(), because the linker shouldn't
         * be on the soinfo list.
         */
    strlcpy((char*) linker_soinfo.name, "/system/bin/linker", sizeof linker_soinfo.name);
    linker_soinfo.flags = 0;
    linker_soinfo.base = linker_base;
    /*
     * Set the dynamic field in the link map otherwise gdb will complain with
     * the following:
     *   warning: .dynamic section for "/system/bin/linker" is not at the
     *   expected address (wrong library or version mismatch?)
     */
    Elf32_Ehdr *elf_hdr = (Elf32_Ehdr *) linker_base;
    Elf32_Phdr *phdr =
        (Elf32_Phdr *)((unsigned char *) linker_base + elf_hdr->e_phoff);
    phdr_table_get_dynamic_section(phdr, elf_hdr->e_phnum, linker_base,
                                   &linker_soinfo.dynamic, NULL);
    insert_soinfo_into_debug_map(&linker_soinfo);

    /* extract information passed from the kernel */
    while(vecs[0] != 0){
        switch(vecs[0]){
        case AT_PHDR:
            si->phdr = (Elf32_Phdr*) vecs[1];
            break;
        case AT_PHNUM:
            si->phnum = (int) vecs[1];
            break;
        case AT_ENTRY:
            si->entry = vecs[1];
            break;
        }
        vecs += 2;
    }

    /* Compute the value of si->base. We can't rely on the fact that
     * the first entry is the PHDR because this will not be true
     * for certain executables (e.g. some in the NDK unit test suite)
     */
    int nn;
    si->base = 0;
    si->size = phdr_table_get_load_size(si->phdr, si->phnum);
    si->load_bias = 0;
    for ( nn = 0; nn < si->phnum; nn++ ) {
        if (si->phdr[nn].p_type == PT_PHDR) {
            si->load_bias = (Elf32_Addr)si->phdr - si->phdr[nn].p_vaddr;
            si->base = (Elf32_Addr) si->phdr - si->phdr[nn].p_offset;
            break;
        }
    }
    si->dynamic = (unsigned *)-1;
    si->refcount = 1;

    // Use LD_LIBRARY_PATH and LD_PRELOAD (but only if we aren't setuid/setgid).
    parse_LD_LIBRARY_PATH(ldpath_env);
    parse_LD_PRELOAD(ldpreload_env);

    if(soinfo_link_image(si)) {
        char errmsg[] = "CANNOT LINK EXECUTABLE\n";
        write(2, __linker_dl_err_buf, strlen(__linker_dl_err_buf));
        write(2, errmsg, sizeof(errmsg));
        exit(-1);
    }

    soinfo_call_preinit_constructors(si);

    for(i = 0; preloads[i] != NULL; i++) {
        soinfo_call_constructors(preloads[i]);
    }

    soinfo_call_constructors(si);

#if ALLOW_SYMBOLS_FROM_MAIN
    /* Set somain after we've loaded all the libraries in order to prevent
     * linking of symbols back to the main image, which is not set up at that
     * point yet.
     */
    somain = si;
#endif

#if TIMING
    gettimeofday(&t1,NULL);
    PRINT("LINKER TIME: %s: %d microseconds\n", argv[0], (int) (
               (((long long)t1.tv_sec * 1000000LL) + (long long)t1.tv_usec) -
               (((long long)t0.tv_sec * 1000000LL) + (long long)t0.tv_usec)
               ));
#endif
#if STATS
    PRINT("RELO STATS: %s: %d abs, %d rel, %d copy, %d symbol\n", argv[0],
           linker_stats.count[kRelocAbsolute],
           linker_stats.count[kRelocRelative],
           linker_stats.count[kRelocCopy],
           linker_stats.count[kRelocSymbol]);
#endif
#if COUNT_PAGES
    {
        unsigned n;
        unsigned i;
        unsigned count = 0;
        for(n = 0; n < 4096; n++){
            if(bitmask[n]){
                unsigned x = bitmask[n];
                for(i = 0; i < 8; i++){
                    if(x & 1) count++;
                    x >>= 1;
                }
            }
        }
        PRINT("PAGES MODIFIED: %s: %d (%dKB)\n", argv[0], count, count * 4);
    }
#endif

#if TIMING || STATS || COUNT_PAGES
    fflush(stdout);
#endif

    TRACE("[ %5d Ready to execute '%s' @ 0x%08x ]\n", pid, si->name,
          si->entry);
    return si->entry;
}

/*
 * Find the value of AT_BASE passed to us by the kernel. This is the load
 * location of the linker.
 */
static unsigned find_linker_base(unsigned **elfdata) {
    int argc = (int) *elfdata;
    char **argv = (char**) (elfdata + 1);
    unsigned *vecs = (unsigned*) (argv + argc + 1);
    while (vecs[0] != 0) {
        vecs++;
    }

    /* The end of the environment block is marked by two NULL pointers */
    vecs++;

    while(vecs[0]) {
        if (vecs[0] == AT_BASE) {
            return vecs[1];
        }
        vecs += 2;
    }

    return 0; // should never happen
}

/* Compute the load-bias of an existing executable. This shall only
 * be used to compute the load bias of an executable or shared library
 * that was loaded by the kernel itself.
 *
 * Input:
 *    elf    -> address of ELF header, assumed to be at the start of the file.
 * Return:
 *    load bias, i.e. add the value of any p_vaddr in the file to get
 *    the corresponding address in memory.
 */
static Elf32_Addr
get_elf_exec_load_bias(const Elf32_Ehdr* elf)
{
    Elf32_Addr        offset     = elf->e_phoff;
    const Elf32_Phdr* phdr_table = (const Elf32_Phdr*)((char*)elf + offset);
    const Elf32_Phdr* phdr_end   = phdr_table + elf->e_phnum;
    const Elf32_Phdr* phdr;

    for (phdr = phdr_table; phdr < phdr_end; phdr++) {
        if (phdr->p_type == PT_LOAD) {
            return (Elf32_Addr)elf + phdr->p_offset - phdr->p_vaddr;
        }
    }
    return 0;
}

/*
 * This is the entry point for the linker, called from begin.S. This
 * method is responsible for fixing the linker's own relocations, and
 * then calling __linker_init_post_relocation().
 *
 * Because this method is called before the linker has fixed it's own
 * relocations, any attempt to reference an extern variable, extern
 * function, or other GOT reference will generate a segfault.
 */
extern "C" unsigned __linker_init(unsigned **elfdata) {
    unsigned linker_addr = find_linker_base(elfdata);
    Elf32_Ehdr *elf_hdr = (Elf32_Ehdr *) linker_addr;
    Elf32_Phdr *phdr =
        (Elf32_Phdr *)((unsigned char *) linker_addr + elf_hdr->e_phoff);

    soinfo linker_so;
    memset(&linker_so, 0, sizeof(soinfo));

    linker_so.base = linker_addr;
    linker_so.size = phdr_table_get_load_size(phdr, elf_hdr->e_phnum);
    linker_so.load_bias = get_elf_exec_load_bias(elf_hdr);
    linker_so.dynamic = (unsigned *) -1;
    linker_so.phdr = phdr;
    linker_so.phnum = elf_hdr->e_phnum;
    linker_so.flags |= FLAG_LINKER;

    if (soinfo_link_image(&linker_so)) {
        // It would be nice to print an error message, but if the linker
        // can't link itself, there's no guarantee that we'll be able to
        // call write() (because it involves a GOT reference).
        //
        // This situation should never occur unless the linker itself
        // is corrupt.
        exit(-1);
    }

    // We have successfully fixed our own relocations. It's safe to run
    // the main part of the linker now.
    return __linker_init_post_relocation(elfdata, linker_addr);
}
