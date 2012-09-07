/*
 * Copyright (C) 2010 The Android Open Source Project
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
#include "linker_environ.h"
#include <stddef.h>

static char** _envp;

/* Returns 1 if 'str' points to a valid environment variable definition.
 * For now, we check that:
 *  - It is smaller than MAX_ENV_LEN (to detect non-zero terminated strings)
 *  - It contains at least one equal sign that is not the first character
 */
static int
_is_valid_definition(const char*  str)
{
    int   pos = 0;
    int   first_equal_pos = -1;

    /* According to its sources, the kernel uses 32*PAGE_SIZE by default
     * as the maximum size for an env. variable definition.
     */
    const int MAX_ENV_LEN = 32*4096;

    if (str == NULL)
        return 0;

    /* Parse the string, looking for the first '=' there, and its size */
    do {
        if (str[pos] == '\0')
            break;
        if (str[pos] == '=' && first_equal_pos < 0)
            first_equal_pos = pos;
        pos++;
    } while (pos < MAX_ENV_LEN);

    if (pos >= MAX_ENV_LEN)  /* Too large */
        return 0;

    if (first_equal_pos < 1)  /* No equal sign, or it is the first character */
        return 0;

    return 1;
}

unsigned*
linker_env_init(unsigned* vecs)
{
    /* Store environment pointer - can't be NULL */
    _envp = (char**) vecs;

    /* Skip over all definitions */
    while (vecs[0] != 0)
        vecs++;
    /* The end of the environment block is marked by two NULL pointers */
    vecs++;

    /* As a sanity check, we're going to remove all invalid variable
     * definitions from the environment array.
     */
    {
        char** readp  = _envp;
        char** writep = _envp;
        for ( ; readp[0] != NULL; readp++ ) {
            if (!_is_valid_definition(readp[0]))
                continue;
            writep[0] = readp[0];
            writep++;
        }
        writep[0] = NULL;
    }

    /* Return the address of the aux vectors table */
    return vecs;
}

/* Check if the environment variable definition at 'envstr'
 * starts with '<name>=', and if so return the address of the
 * first character after the equal sign. Otherwise return NULL.
 */
static char*
env_match(char* envstr, const char* name)
{
    size_t  cnt = 0;

    while (envstr[cnt] == name[cnt] && name[cnt] != '\0')
        cnt++;

    if (name[cnt] == '\0' && envstr[cnt] == '=')
        return envstr + cnt + 1;

    return NULL;
}

#define MAX_ENV_LEN  (16*4096)

const char*
linker_env_get(const char* name)
{
    char** readp = _envp;

    if (name == NULL || name[0] == '\0')
        return NULL;

    for ( ; readp[0] != NULL; readp++ ) {
        char* val = env_match(readp[0], name);
        if (val != NULL) {
            /* Return NULL for empty strings, or if it is too large */
            if (val[0] == '\0')
                val = NULL;
            return val;
        }
    }
    return NULL;
}


void
linker_env_unset(const char* name)
{
    char**  readp = _envp;
    char**  writep = readp;

    if (name == NULL || name[0] == '\0')
        return;

    for ( ; readp[0] != NULL; readp++ ) {
        if (env_match(readp[0], name))
            continue;
        writep[0] = readp[0];
        writep++;
    }
    /* end list with a NULL */
    writep[0] = NULL;
}



/* Remove unsafe environment variables. This should be used when
 * running setuid programs. */
void
linker_env_secure(void)
{
    /* The same list than GLibc at this point */
    static const char* const unsec_vars[] = {
        "GCONV_PATH",
        "GETCONF_DIR",
        "HOSTALIASES",
        "LD_AUDIT",
        "LD_DEBUG",
        "LD_DEBUG_OUTPUT",
        "LD_DYNAMIC_WEAK",
        "LD_LIBRARY_PATH",
        "LD_ORIGIN_PATH",
        "LD_PRELOAD",
        "LD_PROFILE",
        "LD_SHOW_AUXV",
        "LD_USE_LOAD_BIAS",
        "LOCALDOMAIN",
        "LOCPATH",
        "MALLOC_TRACE",
        "MALLOC_CHECK_",
        "NIS_PATH",
        "NLSPATH",
        "RESOLV_HOST_CONF",
        "RES_OPTIONS",
        "TMPDIR",
        "TZDIR",
        "LD_AOUT_LIBRARY_PATH",
        "LD_AOUT_PRELOAD",
        NULL
    };

    int count;
    for (count = 0; unsec_vars[count] != NULL; count++) {
        linker_env_unset(unsec_vars[count]);
    }
}
