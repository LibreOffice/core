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

#if defined(__linux__) || defined (__FreeBSD_kernel__) || defined(_AIX) ||\
    defined(_WIN32) || defined(__APPLE__) || defined (__NetBSD__) ||\
    defined (__sun)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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
    #define SEPARATOR         '/'

    void *lok_loadlib(const char *pFN)
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

    char *lok_dlerror(void)
    {
        return dlerror();
    }

    void *lok_dlsym(void *Hnd, const char *pName)
    {
        return dlsym(Hnd, pName);
    }

    int lok_dlclose(void *Hnd)
    {
        return dlclose(Hnd);
    }

    void extendUnoPath(const char *pPath)
    {
        (void)pPath;
    }

#else

    #include "prewin.h"
    #include "postwin.h"
    #define TARGET_LIB        "sofficeapp" ".dll"
    #define TARGET_MERGED_LIB "mergedlo" ".dll"
    #define SEPARATOR         '\\'
    #define UNOPATH           "\\..\\URE\\bin"

    void *lok_loadlib(const char *pFN)
    {
        return (void *) LoadLibrary(pFN);
    }

    char *lok_dlerror(void)
    {
        LPSTR buf = NULL;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, reinterpret_cast<LPSTR>(&buf), 0, NULL);
        return buf;
    }

    void *lok_dlsym(void *Hnd, const char *pName)
    {
        return reinterpret_cast<void *>(GetProcAddress((HINSTANCE) Hnd, pName));
    }

    int lok_dlclose(void *Hnd)
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

static void *lok_dlopen( const char *install_path, char ** _imp_lib )
{
    char *imp_lib;
    void *dlhandle;

    *_imp_lib = NULL;

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

    imp_lib[partial_length++] = SEPARATOR;
    strcpy(imp_lib + partial_length, TARGET_LIB);

    dlhandle = lok_loadlib(imp_lib);
    if (!dlhandle)
    {
        // If TARGET_LIB exists, and likely is a real library (not a
        // small one-line text stub as in the --enable-mergedlib
        // case), but dlopen failed for some reason, don't try
        // TARGET_MERGED_LIB.
        struct stat st;
        if (stat(imp_lib, &st) == 0 && st.st_size > 100)
        {
            fprintf(stderr, "failed to open library '%s': %s\n",
                    imp_lib, lok_dlerror());
            free(imp_lib);
            return NULL;
        }

        strcpy(imp_lib + partial_length, TARGET_MERGED_LIB);

        dlhandle = lok_loadlib(imp_lib);
        if (!dlhandle)
        {
            fprintf(stderr, "failed to open library '%s': %s\n",
                    imp_lib, lok_dlerror());
            free(imp_lib);
            return NULL;
        }
    }
#else
    imp_lib = strdup("the app executable");
    dlhandle = RTLD_MAIN_ONLY;
#endif
    *_imp_lib = imp_lib;
    return dlhandle;
}

typedef LibreOfficeKit *(LokHookFunction)( const char *install_path);

typedef LibreOfficeKit *(LokHookFunction2)( const char *install_path, const char *user_profile_path );

typedef int             (LokHookPreInit)  ( const char *install_path, const char *user_profile_path );

static LibreOfficeKit *lok_init_2( const char *install_path,  const char *user_profile_path )
{
    char *imp_lib;
    void *dlhandle;
    LokHookFunction *pSym;
    LokHookFunction2 *pSym2;

    dlhandle = lok_dlopen(install_path, &imp_lib);

    pSym2 = (LokHookFunction2 *) lok_dlsym(dlhandle, "libreofficekit_hook_2");
    if (!pSym2)
    {
        if (user_profile_path != NULL)
        {
            fprintf( stderr, "the LibreOffice version in '%s' does not support passing a user profile to the hook function\n",
                     imp_lib );
            lok_dlclose( dlhandle );
            free( imp_lib );
            return NULL;
        }
        pSym = (LokHookFunction *) lok_dlsym( dlhandle, "libreofficekit_hook" );
        if (!pSym)
        {
            fprintf( stderr, "failed to find hook in library '%s'\n", imp_lib );
            lok_dlclose( dlhandle );
            free( imp_lib );
            return NULL;
        }
        free( imp_lib );
        // dlhandle is "leaked"
        // coverity[leaked_storage]
        return pSym( install_path );
    }

    free( imp_lib );
    // dlhandle is "leaked"
    // coverity[leaked_storage]
    return pSym2( install_path, user_profile_path );
}

static
#if defined __GNUC__ || defined __clang__
__attribute__((used))
#endif
LibreOfficeKit *lok_init( const char *install_path )
{
    return lok_init_2( install_path, NULL );
}

#undef SEPARATOR // It is used at least in enum class MenuItemType

#endif // defined(__linux__) || defined (__FreeBSD_kernel__) || defined(_AIX) || defined(_WIN32) || defined(__APPLE__)

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITINIT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
