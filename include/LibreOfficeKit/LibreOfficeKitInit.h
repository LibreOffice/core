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

#if defined __GNUC__ || defined __clang__
#  define LOK_TOLERATE_UNUSED __attribute__((used))
#else
#  define LOK_TOLERATE_UNUSED
#endif

#if defined(__linux__) || defined (__FreeBSD__) ||\
    defined(_WIN32) || defined(__APPLE__) || defined (__NetBSD__) ||\
    defined (__sun) || defined(__OpenBSD__) || defined(__EMSCRIPTEN__)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef _WIN32

    #include <dlfcn.h>

    #ifdef __APPLE__
        #define SOFFICEAPP_LIB "libsofficeapp.dylib"
        #define MERGED_LIB "libmergedlo.dylib"

        #if (!defined TARGET_OS_IPHONE || TARGET_OS_IPHONE == 0) && (!defined TARGET_OS_OSX || TARGET_OS_OSX == 0)
            #error LibreOfficeKit is not supported on tvOS, visionOS or watchOS
        #endif
    #else
        #define SOFFICEAPP_LIB "libsofficeapp.so"
        #define MERGED_LIB "libmergedlo.so"
    #endif
    #define SEPARATOR         '/'

#else

    #if !defined WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #define SOFFICEAPP_LIB "sofficeapp.dll"
    #define MERGED_LIB "mergedlo.dll"
    #define SEPARATOR '\\'

    #undef DELETE

#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _WIN32

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

    static void *lok_dlsym(void *Hnd, const char *pName)
    {
        return dlsym(Hnd, pName);
    }

    static int lok_dlclose(void *Hnd)
    {
        return dlclose(Hnd);
    }
#endif // IOS


#else // _WIN32

    static wchar_t* lok_string_to_wide_string(const char* string)
    {
        const size_t len = strlen(string);
        if (len == 0)
        {
            return _wcsdup(L"");
        }

        if (len > INT_MAX)
        {
            /* Trying to be funny, eh? */
            return _wcsdup(L"");
        }

        const int wlen = MultiByteToWideChar(CP_UTF8, 0, string, (int)len, NULL, 0);
        if (wlen <= 0)
        {
            return NULL;
        }

        wchar_t* result = (wchar_t*)malloc(wlen * 2 + 2);
        assert(result);
        MultiByteToWideChar(CP_UTF8, 0, string, (int)len, result, wlen);
        result[wlen] = L'\0';

        return result;
    }

    static char* lok_wide_string_to_string(const wchar_t* wstring)
    {
        if (wstring == NULL)
            return NULL;

        int len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wstring, -1, NULL, 0, NULL, NULL);
        if (len <= 0)
            return NULL;

        char* result = (char*)malloc(len);
        if (result == NULL)
            return NULL;

        len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wstring, -1, (char*)result, len, NULL,
                                  NULL);
        if (len <= 0)
        {
            free(result);
            return NULL;
        }

        return result;
    }

    static void *lok_loadlib(const char *pFN)
    {
        wchar_t* wpFN = lok_string_to_wide_string(pFN);
        void* result = LoadLibraryW(wpFN);
        free(wpFN);
        return result;
    }

    static char *lok_dlerror(void)
    {
        LPWSTR wbuf = NULL;
        int rc = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, GetLastError(), 0, reinterpret_cast<LPWSTR>(&wbuf), 0, NULL);
        if (rc == 0)
            return _strdup("");

        /* Strip trailing CRLF */
        if (wbuf[wcslen(wbuf)-1] == L'\n')
            wbuf[wcslen(wbuf)-1] = L'\0';
        if (wbuf[wcslen(wbuf)-1] == L'\r')
            wbuf[wcslen(wbuf)-1] = L'\0';

        char* result = lok_wide_string_to_string(wbuf);
        HeapFree(GetProcessHeap(), 0, wbuf);
        return result;
    }

    // This function must be called to release memory allocated by lok_dlerror()
    static void lok_dlerror_free(char *pErrMessage)
    {
        free(pErrMessage);
    }

    static void *lok_dlsym(void *Hnd, const char *pName)
    {
        return reinterpret_cast<void *>(GetProcAddress((HINSTANCE) Hnd, pName));
    }

    static int lok_dlclose(void *Hnd)
    {
        return FreeLibrary((HINSTANCE) Hnd);
    }

#endif

#if !defined(IOS)
static int lok_stat(const char* path, struct stat* st)
{
#ifdef _WIN32
    struct _stat32 st32;
    wchar_t* wpath = lok_string_to_wide_string(path);
    int result = _wstat32(wpath, &st32);
    free(wpath);
    if (result != -1)
    {
        st->st_dev = st32.st_dev;
        st->st_ino = st32.st_ino;
        st->st_mode = st32.st_mode;
        st->st_nlink = st32.st_nlink;
        st->st_uid = st32.st_uid;
        st->st_gid = st32.st_gid;
        st->st_rdev = st32.st_rdev;
        st->st_size = st32.st_size;
        st->st_atime = st32.st_atime;
        st->st_mtime = st32.st_mtime;
        st->st_ctime = st32.st_ctime;
    }
    return result;
#else
    return stat(path, st);
#endif
}

static void *lok_dlopen( const char *install_path, char ** _imp_lib )
{
    char *imp_lib;
    void *dlhandle;

    size_t partial_length, imp_lib_size;
    struct stat dir_st;

    *_imp_lib = NULL;

    if (!install_path)
        return NULL;

    if (lok_stat(install_path, &dir_st) != 0)
    {
        fprintf(stderr, "installation path \"%s\" does not exist\n", install_path);
        return NULL;
    }

    // allocate large enough buffer
    partial_length = strlen(install_path);
    imp_lib_size = partial_length + sizeof(SOFFICEAPP_LIB) + sizeof(MERGED_LIB) + 2;
    imp_lib = (char *) malloc(imp_lib_size);
    if (!imp_lib)
    {
        fprintf( stderr, "failed to open library : not enough memory\n");
        return NULL;
    }

    memcpy(imp_lib, install_path, partial_length);

    imp_lib[partial_length++] = SEPARATOR;
    strncpy(imp_lib + partial_length, SOFFICEAPP_LIB, imp_lib_size - partial_length);

    struct stat st;
    // If SOFFICEAPP_LIB exists but is ridiculously small, it is the
    // one-line text stub as in the --enable-mergedlib case.
    if (lok_stat(imp_lib, &st) == 0 && st.st_size > 1000)
    {
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
    else
    {
        strncpy(imp_lib + partial_length, MERGED_LIB, imp_lib_size - partial_length);

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
    *_imp_lib = imp_lib;
    return dlhandle;
}
#endif

typedef LibreOfficeKit *(LokHookFunction)( const char *install_path);

typedef LibreOfficeKit *(LokHookFunction2)( const char *install_path, const char *user_profile_url );

typedef int             (LokHookPreInit)  ( const char *install_path, const char *user_profile_url );

typedef int             (LokHookPreInit2) ( const char *install_path, const char *user_profile_url, LibreOfficeKit** kit);

#if defined(IOS) || defined(ANDROID) || defined(__EMSCRIPTEN__)
LibreOfficeKit *libreofficekit_hook_2(const char* install_path, const char* user_profile_path);
#endif

// install_path is the pathname to the LibreOffice installation
// directory, the one with the subdirectories "program", "share" etc.
// On Linux there is nothing special here, you just pass such a
// pathname.
//
// On Windows, in the actual file system names are in UTF-16. To work
// in arbitrary situations, on Windows one should use APIs that take
// wide characters when accessing the file system. Here you pass in
// the pathname in UTF-8. *Not* in the system codepage (or some other
// codepage). Of course, in the common (lucky?) case of a pathname
// that is just ASCII anyway, that is already UTF-8.
//
// On macOS it should be the pathname of the bundle (.app directory)
// the code is in.
//
// user_profile_url is a file: URI for the user profile. Can be NULL.

static LibreOfficeKit *lok_init_2( const char *install_path,  const char *user_profile_url )
{
#if !defined(IOS) && !defined(ANDROID) && !defined(__EMSCRIPTEN__)
    void *dlhandle;
    char *imp_lib;
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
        // coverity[leaked_storage] - on purpose
        return pSym( install_path );
    }

    if (user_profile_url != NULL && user_profile_url[0] == '/')
    {
        // It should be either a file: URL or a vnd.sun.star.pathname: URL.
        fprintf( stderr, "second parameter to lok_init_2 '%s' should be a URL, not a pathname\n", user_profile_url );
        lok_dlclose( dlhandle );
        free( imp_lib );
        return NULL;
    }

    free( imp_lib );
    // dlhandle is "leaked"
    // coverity[leaked_storage] - on purpose
    return pSym2( install_path, user_profile_url );
#else
    return libreofficekit_hook_2( install_path, user_profile_url );
#endif
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
    // coverity[leaked_storage] - on purpose
    return pSym( install_path, user_profile_url );
}
#endif

#undef SEPARATOR // It is used at least in enum class MenuItemType

#ifdef __cplusplus
}
#endif

#endif // defined(__linux__) || defined (__FreeBSD__) || defined(_WIN32) || defined(__APPLE__)

#endif // INCLUDED_LIBREOFFICEKIT_LIBREOFFICEKITINIT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
