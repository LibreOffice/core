/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITINIT_H
#define INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITINIT_H

#include "LibreOfficeKit.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__linux__) || defined (__FreeBSD_kernel__) || defined(_AIX) || defined(_WIN32) || defined(__APPLE__)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
    #include "dlfcn.h"
    #ifdef  _AIX
    #  include <sys/ldr.h>
    #endif
    #ifdef __APPLE__
        #define TARGET_LIB        "lib" "sofficeapp" ".dylib"
        #define TARGET_MERGED_LIB "lib" "mergedlo" ".dylib"
    #else
        #define TARGET_LIB        "lib" "sofficeapp" ".so"
        #define TARGET_MERGED_LIB "lib" "mergedlo" ".so"
    #endif
    #define SEPERATOR         '/'

    void *_dlopen(const char *pFN)
    {
        return dlopen(pFN, RTLD_LAZY
#if defined __clang__ && defined __linux__ \
    && defined ENABLE_RUNTIME_OPTIMIZATIONS
#if !ENABLE_RUNTIME_OPTIMIZATIONS
                      | RTLD_GLOBAL
#endif
#endif
                      );
    }

    void *_dlsym(void *Hnd, const char *pName)
    {
        return dlsym(Hnd, pName);
    }

    int _dlclose(void *Hnd)
    {
        return dlclose(Hnd);
    }

    void extendUnoPath(const char *pPath)
    {
        (void)pPath;
    }

#else

    #include <windows.h>
    #define TARGET_LIB        "sofficeapp" ".dll"
    #define TARGET_MERGED_LIB "mergedlo" ".dll"
    #define SEPERATOR         '\\'
    #define UNOPATH           "\\..\\URE\\bin"

    void *_dlopen(const char *pFN)
    {
        return (void *) LoadLibrary(pFN);
    }

    void *_dlsym(void *Hnd, const char *pName)
    {
        return GetProcAddress((HINSTANCE) Hnd, pName);
    }

    int _dlclose(void *Hnd)
    {
        return FreeLibrary((HINSTANCE) Hnd);
    }

    void extendUnoPath(const char *pPath)
    {
        if (!pPath)
            return;

        char* sEnvPath = NULL;
        DWORD  cChars = GetEnvironmentVariable("PATH", sEnvPath, 0);
        if (cChars > 0)
        {
            sEnvPath = new char[cChars];
            cChars = GetEnvironmentVariable("PATH", sEnvPath, cChars);
            //If PATH is not set then it is no error
            if (cChars == 0 && GetLastError() != ERROR_ENVVAR_NOT_FOUND)
            {
                delete[] sEnvPath;
                return;
            }
        }
        //prepare the new PATH. Add the Ure/bin directory at the front.
        //note also adding ';'
        char * sNewPath = new char[strlen(sEnvPath) + strlen(pPath) + strlen(UNOPATH) + 2];
        sNewPath[0] = L'\0';
        strcat(sNewPath, pPath);
        strcat(sNewPath, UNOPATH);
        if (strlen(sEnvPath))
        {
            strcat(sNewPath, ";");
            strcat(sNewPath, sEnvPath);
        }

        SetEnvironmentVariable("PATH", sNewPath);

        delete[] sEnvPath;
        delete[] sNewPath;
    }
#endif

typedef LibreOfficeKit *(HookFunction)( const char *install_path);

static LibreOfficeKit *lok_init( const char *install_path )
{
    char *imp_lib;
    void *dlhandle;
    HookFunction *pSym;

#if !(defined(__APPLE__) && defined(__arm__))
    size_t partial_length;

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

    extendUnoPath(install_path);

    imp_lib[partial_length++] = SEPERATOR;
    strcpy(imp_lib + partial_length, TARGET_LIB);

    dlhandle = _dlopen(imp_lib);
    if (!dlhandle)
    {
        strcpy(imp_lib + partial_length, TARGET_MERGED_LIB);

        dlhandle = _dlopen(imp_lib);
        if (!dlhandle)
        {
            fprintf(stderr, "failed to open library '%s' or '%s' in '%s/'\n",
                    TARGET_LIB, TARGET_MERGED_LIB, install_path);
            free(imp_lib);
            return NULL;
        }
    }
#else
    imp_lib = strdup("the app executable");
    dlhandle = RTLD_MAIN_ONLY;
#endif

    pSym = (HookFunction *) _dlsym( dlhandle, "libreofficekit_hook" );
    if (!pSym)
    {
        fprintf( stderr, "failed to find hook in library '%s'\n", imp_lib );
        _dlclose( dlhandle );
        free( imp_lib );
        return NULL;
    }

    free( imp_lib );
    return pSym( install_path );
}

#endif // defined(__linux__) || defined (__FreeBSD_kernel__) || defined(_AIX) || defined(_WIN32) || defined(__APPLE__)

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITINIT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
