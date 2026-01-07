/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* Wrappers to use Unix-style file APIs on Windows.
 *
 * Of course, LibreOffice has tons of APIs from before whose whole point is to hide system
 * dependencies behind a cross-platform API. All the "sal" and "osl" stuff, for starters. The
 * wrappers in this file is not intended to replace those. The point with these wrappers is to
 * quickly port code that was written for Unix *only* to Windows, with minimal changes to the source
 * code. Basically you just need to prefix calls to functions like open() and stat() with wrap_.
 *
 * In Windows, "file descriptors" are a thing in the C library, not the operating system. But the C
 * library does provide the same core set of functionality as Unix-like systems do. The functions
 * use almost the same names, except that for some reason, to avoid compilation warnings, you need
 * to call variants with names preceded by an underscore, like _close().

 * Another Windows complication is that the file system is based on UTF-16 names. You can't just
 * pass file names as eight-bit characters to _open() and expect it to work for all file
 * names. Instead, you need to call a function with a "w" prefix, like _wopen(), that takes a wide
 * character string (UTF-16).

 * This header is in C, not C++, because we want it to be usable from some external libraries
 * written in C that we compile as part of LibreOffice in some circumstances, particularly for
 * Windows.
 *
 * All functions behave like the wrapped ones, set errno on errors.
 */

#ifndef INCLUDED_TOOLS_UNIXWRAPPERS_H
#define INCLUDED_TOOLS_UNIXWRAPPERS_H

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#include <io.h>
#include <string.h>
#include <assert.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
/* Undo the mapping in <Windows.h> of CreateFont => CreateFontW etc, as vcl also uses these
 * identifiers.
 */
#undef CreateFont
#undef GetGlyphOutline
#else
#include <unistd.h>
#include <sys/mman.h>
#endif

#ifdef _WIN32

#define MAP_FAILED ((void*)-1)

#define S_ISDIR(m) (((m)&_S_IFMT) == _S_IFDIR)

#define F_OK 00
#define W_OK 02
#define R_OK 04
#define X_OK R_OK

#define PATH_MAX MAX_PATH

#endif

#if defined __cplusplus && __cplusplus >= 201703L
#define UNIXWRAPPERS_H_MAYBE_UNUSED [[maybe_unused]]
#else
#define UNIXWRAPPERS_H_MAYBE_UNUSED
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ifdef _WIN32
static wchar_t* string_to_wide_string(const char* string)
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
#endif

/* Pass in UTF-8 filename */
UNIXWRAPPERS_H_MAYBE_UNUSED static int wrap_open(const char* path, int flags, int mode)
{
#ifdef _WIN32
    wchar_t* wpath = string_to_wide_string(path);
    int result = _wopen(wpath, flags, mode);
    free(wpath);
    return result;
#else
    return open(path, flags, mode);
#endif
}

#if 0

/* Pass in UTF-16 filename */
UNIXWRAPPERS_H_MAYBE_UNUSED static int wrap_wopen(const wchar_t* path, int flags, int mode)
{
#ifdef _WIN32
    return _wopen(path, flags, mode);
#else
    rtl_String* str;
    int len = wcslen(path);
    bool success = rtl_convertUStringToString(&str, path, len, RTL_TEXTENCODING_UTF8,
                                              RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                                                  | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR);
    int result;
    if (success)
    {
        char* spath = malloc(len + 1);
        strcat(spath, str, len);
        spath[len] = 0;
        result = open(str->buffer, flags, path);
        free(spath);
    }
    else
    {
        result = -1;
        errno = EIO;
    }
    rtl_string_release(str);
    return result;
#endif
}

#endif

UNIXWRAPPERS_H_MAYBE_UNUSED static int wrap_read(int fd, void* buf, int nbytes)
{
#ifdef _WIN32
    return _read(fd, buf, nbytes);
#else
    return read(fd, buf, nbytes);
#endif
}

UNIXWRAPPERS_H_MAYBE_UNUSED static int wrap_write(int fd, const void* buf, int nbytes)
{
#ifdef _WIN32
    return _write(fd, buf, nbytes);
#else
    return write(fd, buf, nbytes);
#endif
}

UNIXWRAPPERS_H_MAYBE_UNUSED static int wrap_fstat(int fd, struct stat* st)
{
#ifdef _WIN32
    /* Sadly just "struct stat" in the Microsoft C library means a legacy one with 32-bit size,
     * 32-bit time one. But it is a *different* type than struct _stat32, even if identical.
     */
    struct _stat32 st32;
    int result = _fstat32(fd, &st32);
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
    return fstat(fd, st);
#endif
}

UNIXWRAPPERS_H_MAYBE_UNUSED static void* wrap_mmap(int64_t size, int fd, intptr_t* handle)
{
#ifdef _WIN32
    *handle = (intptr_t)CreateFileMappingW((HANDLE)_get_osfhandle(fd), NULL,
                                           SEC_COMMIT | PAGE_READONLY, 0, 0, NULL);
    return MapViewOfFile((HANDLE)*handle, FILE_MAP_READ, 0, 0, size);
#else
    /* No handle needs to be stored */
    (void)handle;
    return mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
#endif
}

UNIXWRAPPERS_H_MAYBE_UNUSED static int wrap_munmap(void* pointer, int64_t size, intptr_t handle)
{
#ifdef _WIN32
    (void)size;
    CloseHandle((HANDLE)handle);
    if (!UnmapViewOfFile(pointer))
    {
        errno = EIO;
        return -1;
    }
    return 0;
#else
    (void)handle;
    return munmap(pointer, size);
#endif
}

UNIXWRAPPERS_H_MAYBE_UNUSED static int wrap_dup(int fd)
{
#ifdef _WIN32
    return _dup(fd);
#else
    return dup(fd);
#endif
}

UNIXWRAPPERS_H_MAYBE_UNUSED static int wrap_close(int fd)
{
#ifdef _WIN32
    return _close(fd);
#else
    return close(fd);
#endif
}

/* This doesn't usea file descriptor but is related to the above functionality so keep here
 * anyway.
 */

UNIXWRAPPERS_H_MAYBE_UNUSED static FILE* wrap_fopen(const char* path, const char* mode)
{
#ifdef _WIN32
    wchar_t* wpath = string_to_wide_string(path);
    wchar_t* wmode = string_to_wide_string(mode);
    FILE* result = _wfopen(wpath, wmode);
    free(wpath);
    free(wmode);
    return result;
#else
    return fopen(path, mode);
#endif
}

UNIXWRAPPERS_H_MAYBE_UNUSED static int wrap_stat(const char* path, struct stat* st)
{
#ifdef _WIN32
    struct _stat32 st32;
    wchar_t* wpath = string_to_wide_string(path);
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

UNIXWRAPPERS_H_MAYBE_UNUSED static int wrap_access(const char* path, int mode)
{
#ifdef _WIN32
    wchar_t* wpath = string_to_wide_string(path);
    int result = _waccess(wpath, mode);
    free(wpath);
    return result;
#else
    return access(path, mode);
#endif
}

UNIXWRAPPERS_H_MAYBE_UNUSED static const char* wrap_realpath(const char* path, char* resolved_path)
{
#ifdef _WIN32
    strcpy(resolved_path, path);
    return resolved_path;
#else
    return realpath(path, resolved_path);
#endif
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef __cplusplus
}
#endif

#undef UNIXWRAPPERS_H_MAYBE_UNUSED

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
