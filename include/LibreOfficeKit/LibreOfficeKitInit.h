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

#include <LibreOfficeKit/LibreOfficeKit.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if defined __GNUC__ || defined __clang__
#  define LOK_TOLERATE_UNUSED __attribute__((used))
#else
#  define LOK_TOLERATE_UNUSED
#endif

#if defined(__linux__) || defined (__FreeBSD_kernel__) || defined(_AIX) ||\
    defined(_WIN32) || defined(__APPLE__) || defined (__NetBSD__) ||\
    defined (__sun) || defined(__OpenBSD__)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef _WIN32

    #include <dlfcn.h>

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

#if !defined(IOS)
        static void *lok_loadlib(const char *pFN)
    {
        return dlopen(pFN, RTLD_LAZY
#if defined LOK_LOADLIB_GLOBAL
                      | RTLD_GLOBAL
#endif
                      );
    }

    static char *lok_dlerror(void)
    {
        return dlerror();
    }

    // This function must be called to release memory allocated by lok_dlerror()
    static void lok_dlerror_free(char *pErrMessage)
    {
        (void)pErrMessage;
        // Do nothing for return of dlerror()
    }

    static void extendUnoPath(const char *pPath)
    {
        (void)pPath;
    }
#endif // IOS

    static void *lok_dlsym(void *Hnd, const char *pName)
    {
        return dlsym(Hnd, pName);
    }

    static int lok_dlclose(void *Hnd)
    {
        return dlclose(Hnd);
    }


#else
    #pragma warning(disable:4996)
    #if !defined WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include  <windows.h>
    #define TARGET_LIB        "sofficeapp" ".dll"
    #define TARGET_MERGED_LIB "mergedlo" ".dll"
    #define SEPARATOR         '\\'
    #define UNOPATH           "\\..\\URE\\bin"

    static void *lok_loadlib(const char *pFN)
    {
        return (void *) LoadLibraryA(pFN);
    }

    static char *lok_dlerror(void)
    {
        LPSTR buf = NULL;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, GetLastError(), 0, reinterpret_cast<LPSTR>(&buf), 0, NULL);
        return buf;
    }

    // This function must be called to release memory allocated by lok_dlerror()
    static void lok_dlerror_free(char *pErrMessage)
    {
        HeapFree(GetProcessHeap(), 0, pErrMessage);
    }

    static void *lok_dlsym(void *Hnd, const char *pName)
    {
        return reinterpret_cast<void *>(GetProcAddress((HINSTANCE) Hnd, pName));
    }

    static int lok_dlclose(void *Hnd)
    {
        return FreeLibrary((HINSTANCE) Hnd);
    }

    static void extendUnoPath(const char *pPath)
    {
        char *sNewPath = NULL, *sEnvPath = NULL;
        size_t size_sEnvPath = 0, buffer_size = 0;
        DWORD cChars;

        if (!pPath)
            return;

        cChars = GetEnvironmentVariableA("PATH", sEnvPath, 0);
        if (cChars > 0)
        {
            sEnvPath = (char *) malloc(cChars);
            cChars = GetEnvironmentVariableA("PATH", sEnvPath, cChars);
            //If PATH is not set then it is no error
            if (cChars == 0 && GetLastError() != ERROR_ENVVAR_NOT_FOUND)
            {
                free(sEnvPath);
                return;
            }
        }
        //prepare the new PATH. Add the Ure/bin directory at the front.
        //note also adding ';'
        if(sEnvPath)
            size_sEnvPath = strlen(sEnvPath);
        buffer_size = size_sEnvPath + 2*strlen(pPath) + strlen(UNOPATH) + 4;
        sNewPath = (char *) malloc(buffer_size);
        sNewPath[0] = L'\0';
        strcat_s(sNewPath, buffer_size, pPath);     // program to PATH
        strcat_s(sNewPath, buffer_size, ";");
        strcat_s(sNewPath, buffer_size, UNOPATH);   // UNO to PATH
        if (size_sEnvPath > 0)
        {
            strcat_s(sNewPath, buffer_size, ";");
            strcat_s(sNewPath, buffer_size, sEnvPath);
        }

        SetEnvironmentVariableA("PATH", sNewPath);

        free(sNewPath);
        free(sEnvPath);
    }
#endif

static void *lok_dlopen( const char *install_path, char ** _imp_lib )
{
    char *imp_lib;
    void *dlhandle;

#if !defined(IOS)
    size_t partial_length, imp_lib_size;
    struct stat dir_st;

    *_imp_lib = NULL;

    if (!install_path)
        return NULL;

    if (stat(install_path, &dir_st) != 0)
    {
        fprintf(stderr, "installation path \"%s\" does not exist\n", install_path);
        return NULL;
    }

    // allocate large enough buffer
    partial_length = strlen(install_path);
    imp_lib_size = partial_length + sizeof(TARGET_LIB) + sizeof(TARGET_MERGED_LIB) + 2;
    imp_lib = (char *) malloc(imp_lib_size);
    if (!imp_lib)
    {
        fprintf( stderr, "failed to open library : not enough memory\n");
        return NULL;
    }

    strncpy(imp_lib, install_path, imp_lib_size);

    extendUnoPath(install_path);

    imp_lib[partial_length++] = SEPARATOR;
    strncpy(imp_lib + partial_length, TARGET_LIB, imp_lib_size - partial_length);

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
            char *pErrMessage = lok_dlerror();
            fprintf(stderr, "failed to open library '%s': %s\n",
                    imp_lib, pErrMessage);
            lok_dlerror_free(pErrMessage);
            free(imp_lib);
            return NULL;
        }

        strncpy(imp_lib + partial_length, TARGET_MERGED_LIB, imp_lib_size - partial_length);

        dlhandle = lok_loadlib(imp_lib);
        if (!dlhandle)
        {
            char *pErrMessage = lok_dlerror();
            fprintf(stderr, "failed to open library '%s': %s\n",
                    imp_lib, pErrMessage);
            lok_dlerror_free(pErrMessage);
            free(imp_lib);
            return NULL;
        }
    }
#else
    (void)install_path;
    imp_lib = strdup("the app executable");
    dlhandle = RTLD_MAIN_ONLY;
#endif
    *_imp_lib = imp_lib;
    return dlhandle;
}

typedef LibreOfficeKit *(LokHookFunction)( const char *install_path);

typedef LibreOfficeKit *(LokHookFunction2)( const char *install_path, const char *user_profile_url );

typedef int             (LokHookPreInit)  ( const char *install_path, const char *user_profile_url );

static LibreOfficeKit *lok_init_2( const char *install_path,  const char *user_profile_url )
{
    char *imp_lib;
    void *dlhandle;
    LokHookFunction *pSym;
    LokHookFunction2 *pSym2;

    dlhandle = lok_dlopen(install_path, &imp_lib);
    if (!dlhandle)
        return NULL;

    pSym2 = (LokHookFunction2 *) lok_dlsym(dlhandle, "libreofficekit_hook_2");
    if (!pSym2)
    {
        if (user_profile_url != NULL)
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
    return pSym2( install_path, user_profile_url );
}

static LOK_TOLERATE_UNUSED
LibreOfficeKit *lok_init( const char *install_path )
{
    return lok_init_2( install_path, NULL );
}

#if !defined(IOS)
static LOK_TOLERATE_UNUSED
int lok_preinit( const char *install_path,  const char *user_profile_url )
{
    void *dlhandle;
    char *imp_lib;
    LokHookPreInit *pSym;

    dlhandle = lok_dlopen(install_path, &imp_lib);
    if (!dlhandle)
        return -1;

    pSym = (LokHookPreInit *) lok_dlsym(dlhandle, "lok_preinit");
    if (!pSym)
    {
        fprintf( stderr, "failed to find pre-init hook in library '%s'\n", imp_lib );
        lok_dlclose( dlhandle );
        free( imp_lib );
        return -1;
    }

    free( imp_lib );

    // dlhandle is "leaked"
    // coverity[leaked_storage]
    return pSym( install_path, user_profile_url );
}
#endif

#undef SEPARATOR // It is used at least in enum class MenuItemType

#endif // defined(__linux__) || defined (__FreeBSD_kernel__) || defined(_AIX) || defined(_WIN32) || defined(__APPLE__)

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITINIT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
