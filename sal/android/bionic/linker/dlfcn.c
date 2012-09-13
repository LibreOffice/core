/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include "linker.h"
#include "linker_format.h"

#define DL_SUCCESS                    0
#define DL_ERR_CANNOT_LOAD_LIBRARY    1
#define DL_ERR_INVALID_LIBRARY_HANDLE 2
#define DL_ERR_BAD_SYMBOL_NAME        3
#define DL_ERR_SYMBOL_NOT_FOUND       4
#define DL_ERR_SYMBOL_NOT_GLOBAL      5

static char dl_err_buf[1024];
static const char *dl_err_str;

static const char *dl_errors[] = {
    [DL_ERR_CANNOT_LOAD_LIBRARY] = "Cannot load library",
    [DL_ERR_INVALID_LIBRARY_HANDLE] = "Invalid library handle",
    [DL_ERR_BAD_SYMBOL_NAME] = "Invalid symbol name",
    [DL_ERR_SYMBOL_NOT_FOUND] = "Symbol not found",
    [DL_ERR_SYMBOL_NOT_GLOBAL] = "Symbol is not global",
};

#define likely(expr)   __builtin_expect (expr, 1)
#define unlikely(expr) __builtin_expect (expr, 0)

pthread_mutex_t dl_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;

static void set_dlerror(int err)
{
    format_buffer(dl_err_buf, sizeof(dl_err_buf), "%s: %s", dl_errors[err],
             linker_get_error());
    dl_err_str = (const char *)&dl_err_buf[0];
};

void *__lo_dlopen(const char *filename, int flag)
{
    soinfo *ret;

    (void) flag;

    pthread_mutex_lock(&dl_lock);
    ret = find_library(filename);
    if (unlikely(ret == NULL)) {
        set_dlerror(DL_ERR_CANNOT_LOAD_LIBRARY);
    } else {
        soinfo_call_constructors(ret);
        ret->refcount++;
    }
    pthread_mutex_unlock(&dl_lock);
    return ret;
}

const char *__lo_dlerror(void)
{
    const char *tmp = dl_err_str;
    dl_err_str = NULL;
    return (const char *)tmp;
}

void *__lo_dlsym(void *handle, const char *symbol)
{
    soinfo *found;
    Elf32_Sym *sym;
    unsigned bind;

    pthread_mutex_lock(&dl_lock);

    if(unlikely(handle == 0)) {
        set_dlerror(DL_ERR_INVALID_LIBRARY_HANDLE);
        goto err;
    }
    if(unlikely(symbol == 0)) {
        set_dlerror(DL_ERR_BAD_SYMBOL_NAME);
        goto err;
    }

    if(handle == RTLD_DEFAULT) {
        sym = lookup(symbol, &found, NULL);
    } else if(handle == RTLD_NEXT) {
        void *ret_addr = __builtin_return_address(0);
        soinfo *si = find_containing_library(ret_addr);

        sym = NULL;
        if(si && si->next) {
            sym = lookup(symbol, &found, si->next);
        }
    } else {
        found = (soinfo*)handle;
        sym = soinfo_lookup(found, symbol);
    }

    if(likely(sym != 0)) {
        bind = ELF32_ST_BIND(sym->st_info);

        if(likely((bind == STB_GLOBAL) && (sym->st_shndx != 0))) {
            unsigned ret = sym->st_value + found->base;
            pthread_mutex_unlock(&dl_lock);
            return (void*)ret;
        }

        set_dlerror(DL_ERR_SYMBOL_NOT_GLOBAL);
    }
    else
        set_dlerror(DL_ERR_SYMBOL_NOT_FOUND);

err:
    pthread_mutex_unlock(&dl_lock);
    return 0;
}

int __lo_dladdr(const void *addr, Dl_info *info)
{
    int ret = 0;
    soinfo *si;

    pthread_mutex_lock(&dl_lock);

    /* Determine if this address can be found in any library currently mapped */
    si = find_containing_library(addr);

    if(si) {
        Elf32_Sym *sym;

        memset(info, 0, sizeof(Dl_info));

        info->dli_fname = si->name;
        info->dli_fbase = (void*)si->base;

        /* Determine if any symbol in the library contains the specified address */
        sym = soinfo_find_symbol(si, addr);

        if(sym != NULL) {
            info->dli_sname = si->strtab + sym->st_name;
            info->dli_saddr = (void*)(si->base + sym->st_value);
        }

        ret = 1;
    }

    pthread_mutex_unlock(&dl_lock);

    return ret;
}

int __lo_dlclose(void* handle) {
    int result;
    pthread_mutex_lock(&dl_lock);
    result = soinfo_unload((soinfo*)handle);
    pthread_mutex_unlock(&dl_lock);
    return result;
}

//                     0000000000 11111111112 222222222 3
//                     0123456789 01234567890 123456789 0
#define LO_LIBLO_BOOTSTRAP_STRTAB                                       \
                      "lo_dlopen\0lo_dlclose\0lo_dlsym\0lo_dladdr\0"

void __lo_dummy_foo(void)
{
}

/* The wrappers in lo-bootstrap.c */
extern void *lo_dlopen(const char *path, int flags);
extern void *lo_dlsym(void *handle, const char *symbol);
extern int lo_dlclose(void *handle);
extern int lo_dladdr(void *addr, Dl_info *info);

static Elf32_Sym liblo_bootstrap_symtab[] = {
      // total length of liblo_bootstrap_info.strtab, including trailing 0
      // This is actually the the STH_UNDEF entry. Technically, it's
      // supposed to have st_name == 0, but instead, it points to an index
      // in the strtab with a \0 to make iterating through the symtab easier.
    { st_name: sizeof(LO_LIBLO_BOOTSTRAP_STRTAB) - 1,
    },
    { st_name: 0,   // starting index of the name in liblo_bootstrap_info.strtab
      st_value: (Elf32_Addr) &lo_dlopen,
      st_info: STB_GLOBAL << 4,
      st_shndx: 1,
    },
    { st_name: 10,
      st_value: (Elf32_Addr) &lo_dlclose,
      st_info: STB_GLOBAL << 4,
      st_shndx: 1,
    },
    { st_name: 21,
      st_value: (Elf32_Addr) &lo_dlsym,
      st_info: STB_GLOBAL << 4,
      st_shndx: 1,
    },
    { st_name: 30,
      st_value: (Elf32_Addr) &lo_dladdr,
      st_info: STB_GLOBAL << 4,
      st_shndx: 1,
    },
};

/* Fake out a hash table with a single bucket.
 * A search of the hash table will look through
 * liblo_bootstrap_symtab starting with index [1], then
 * use liblo_bootstrap_chains to find the next index to
 * look at.  liblo_bootstrap_chains should be set up to
 * walk through every element in liblo_bootstrap_symtab,
 * and then end with 0 (sentinel value).
 *
 * I.e., liblo_bootstrap_chains should look like
 * { 0, 2, 3, ... N, 0 } where N is the number
 * of actual symbols, or nelems(liblo_bootstrap_symtab)-1
 * (since the first element of liblo_bootstrap_symtab is not
 * a real symbol).
 *
 * (see _elf_lookup())
 */
static unsigned liblo_bootstrap_buckets[1] = { 1 };
static unsigned liblo_bootstrap_chains[5] = { 0, 2, 3, 4, 0 };

soinfo liblo_bootstrap_info = {
    name: "liblo-bootstrap.so",
    flags: FLAG_LINKED,

    strtab: LO_LIBLO_BOOTSTRAP_STRTAB,
    symtab: liblo_bootstrap_symtab,

    nbucket: 1,
    nchain: 5,
    bucket: liblo_bootstrap_buckets,
    chain: liblo_bootstrap_chains,
};
