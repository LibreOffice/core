/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_INC_LIBREOFFICEKIT_INIT_H
#define INCLUDED_DESKTOP_INC_LIBREOFFICEKIT_INIT_H

#include "LibreOfficeKit.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__linux__) || defined(_AIX)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dlfcn.h>
#ifdef  _AIX
#  include <sys/ldr.h>
#endif

#define TARGET_LIB        "lib" "sofficeapp" ".so"
#define TARGET_MERGED_LIB "lib" "mergedlo" ".so"

typedef LibreOfficeKit *(HookFunction)( const char *install_path);

static LibreOfficeKit *lok_init( const char *install_path )
{
    char *imp_lib;
    size_t partial_length;
    void *dlhandle;
    HookFunction *pSym;

    if (!install_path)
        return NULL;

    // allocate large enough buffer
    partial_length = strlen(install_path);
    imp_lib = (char *) malloc(partial_length + sizeof(TARGET_LIB) + sizeof(TARGET_MERGED_LIB) + 2);
    if (!imp_lib)
    {
        fprintf( stderr, "failed to open library : not enough memory\n");
        return NULL;
    }

    strcpy(imp_lib, install_path);

    imp_lib[partial_length++] = '/';
    strcpy(imp_lib + partial_length, TARGET_LIB);

    dlhandle = dlopen(imp_lib, RTLD_LAZY);
    if (!dlhandle)
    {
        strcpy(imp_lib + partial_length, TARGET_MERGED_LIB);

        dlhandle = dlopen(imp_lib, RTLD_LAZY);
        if (!dlhandle)
        {
            fprintf(stderr, "failed to open library '%s' or '%s' in '%s/'\n",
                    TARGET_LIB, TARGET_MERGED_LIB, install_path);
            free(imp_lib);
            return NULL;
        }
    }

    pSym = (HookFunction *) dlsym( dlhandle, "libreofficekit_hook" );
    if (!pSym)
    {
        fprintf( stderr, "failed to find hook in library '%s'\n", imp_lib );
        dlclose( dlhandle );
        free( imp_lib );
        return NULL;
    }

    free( imp_lib );
    return pSym( install_path );
}

#endif // defined(__linux__) || defined(_AIX)

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
