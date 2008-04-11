/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: extendloaderenvironment.cxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_desktop.hxx"
#include "sal/config.h"

#include <cstddef>

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <shlwapi.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include "sal/types.h"

#include "extendloaderenvironment.hxx"

namespace {

WCHAR * filename(WCHAR * path) {
    WCHAR * f = path;
    for (WCHAR * p = path;;) {
        switch (*p++) {
        case L'\0':
            return f;
        case L'\\':
            f = p;
            break;
        }
    }
}

// Concatenate two strings.
//
// @param path
// An output paremeter taking the resulting path; must point at a valid range of
// memory of size at least MAX_PATH.  If NULL is returned, the content is
// unspecified.
//
// @param frontBegin, frontEnd
// Forms a valid range [frontBegin .. frontEnd) of less than MAX_PATH size.
//
// @param backBegin, backLength
// Forms a valid range [backBeghin .. backBegin + backLength) of less than
// MAX_PATH size.
//
// @return
// A pointer to the terminating null character of the concatenation, or NULL if
// a failure occurred.
WCHAR * buildPath(
    WCHAR * path, WCHAR const * frontBegin, WCHAR const * frontEnd,
    WCHAR const * backBegin, std::size_t backLength)
{
    if (backLength <
        static_cast< std::size_t >(MAX_PATH - (frontEnd - frontBegin)))
        // hopefully std::size_t is large enough
    {
        WCHAR * p;
        if (frontBegin == path) {
            p = const_cast< WCHAR * >(frontEnd);
        } else {
            p = path;
            while (frontBegin != frontEnd) {
                *p++ = *frontBegin++;
            }
        }
        for (; backLength > 0; --backLength) {
            *p++ = *backBegin++;
        }
        *p = L'\0';
        return p;
    } else {
        SetLastError(ERROR_FILENAME_EXCED_RANGE);
        return NULL;
    }
}

// Resolve a link file.
//
// @param path
// An input/output parameter taking the path; must point at a valid range of
// memory of size at least MAX_PATH.  On input, contains the null-terminated
// full path of the link file.  On output, contains the null-terminated full
// path of the resolved link; if NULL is returned, the content is unspecified.
//
// @return
// A pointer to the terminating null character of path, or NULL if a failure
// occurred.
WCHAR * resolveLink(WCHAR * path) {
    HANDLE h = CreateFileW(
        path, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        return NULL;
    }
    char p1[MAX_PATH];
    DWORD n;
    BOOL ok = ReadFile(h, p1, MAX_PATH, &n, NULL);
    CloseHandle(h);
    if (!ok) {
        return NULL;
    }
    WCHAR p2[MAX_PATH];
    std::size_t n2 = 0;
    bool colon = false;
    for (DWORD i = 0; i < n;) {
        unsigned char c = static_cast< unsigned char >(p1[i++]);
        switch (c) {
        case '\0':
            SetLastError(ERROR_BAD_PATHNAME);
            return NULL;
        case '\x0A':
        case '\x0D':
            if (n2 == MAX_PATH) {
                SetLastError(ERROR_FILENAME_EXCED_RANGE);
                return NULL;
            }
            p2[n2] = L'\0';
            break;
        case ':':
            colon = true;
            // fall through
        default:
            // Convert from UTF-8 to UTF-16:
            if (c <= 0x7F) {
                p2[n2++] = c;
            } else if (c >= 0xC2 && c <= 0xDF && i < n &&
                       static_cast< unsigned char >(p1[i]) >= 0x80 &&
                       static_cast< unsigned char >(p1[i]) <= 0xBF)
            {
                p2[n2++] = ((c & 0x1F) << 6) |
                    (static_cast< unsigned char >(p1[i++]) & 0x3F);
            } else if (n - i > 1 &&
                       ((c == 0xE0 &&
                         static_cast< unsigned char >(p1[i]) >= 0xA0 &&
                         static_cast< unsigned char >(p1[i]) <= 0xBF) ||
                        ((c >= 0xE1 && c <= 0xEC || c >= 0xEE && c <= 0xEF) &&
                         static_cast< unsigned char >(p1[i]) >= 0x80 &&
                         static_cast< unsigned char >(p1[i]) <= 0xBF) ||
                        (c == 0xED &&
                         static_cast< unsigned char >(p1[i]) >= 0x80 &&
                         static_cast< unsigned char >(p1[i]) <= 0x9F)) &&
                       static_cast< unsigned char >(p1[i + 1]) >= 0x80 &&
                       static_cast< unsigned char >(p1[i + 1]) <= 0xBF)
            {
                p2[n2++] = ((c & 0x0F) << 12) |
                    ((static_cast< unsigned char >(p1[i]) & 0x3F) << 6) |
                    (static_cast< unsigned char >(p1[i + 1]) & 0x3F);
                i += 2;
            } else if (n - 2 > 1 &&
                       ((c == 0xF0 &&
                         static_cast< unsigned char >(p1[i]) >= 0x90 &&
                         static_cast< unsigned char >(p1[i]) <= 0xBF) ||
                        (c >= 0xF1 && c <= 0xF3 &&
                         static_cast< unsigned char >(p1[i]) >= 0x80 &&
                         static_cast< unsigned char >(p1[i]) <= 0xBF) ||
                        (c == 0xF4 &&
                         static_cast< unsigned char >(p1[i]) >= 0x80 &&
                         static_cast< unsigned char >(p1[i]) <= 0x8F)) &&
                       static_cast< unsigned char >(p1[i + 1]) >= 0x80 &&
                       static_cast< unsigned char >(p1[i + 1]) <= 0xBF &&
                       static_cast< unsigned char >(p1[i + 2]) >= 0x80 &&
                       static_cast< unsigned char >(p1[i + 2]) <= 0xBF)
            {
                sal_Int32 u = ((c & 0x07) << 18) |
                    ((static_cast< unsigned char >(p1[i]) & 0x3F) << 12) |
                    ((static_cast< unsigned char >(p1[i + 1]) & 0x3F) << 6) |
                    (static_cast< unsigned char >(p1[i + 2]) & 0x3F);
                i += 3;
                p2[n2++] = static_cast< WCHAR >(((u - 0x10000) >> 10) | 0xD800);
                p2[n2++] = static_cast< WCHAR >(
                    ((u - 0x10000) & 0x3FF) | 0xDC00);
            } else {
                SetLastError(ERROR_BAD_PATHNAME);
                return NULL;
            }
            break;
        }
    }
    WCHAR * end;
    if (colon || p2[0] == L'\\') {
        // Interpret p2 as an absolute path:
        end = path;
    } else {
        // Interpret p2 as a relative path:
        end = filename(path);
    }
    return buildPath(path, path, end, p2, n2);
}

void fail() {
    LPWSTR buf = NULL;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
        GetLastError(), 0, reinterpret_cast< LPWSTR >(&buf), 0, NULL);
    MessageBoxW(NULL, buf, NULL, MB_OK | MB_ICONERROR);
    LocalFree(buf);
    TerminateProcess(GetCurrentProcess(), 255);
}

bool contains(WCHAR const * paths, WCHAR const * path, WCHAR const * pathEnd) {
    WCHAR const * q = path;
    for (WCHAR const * p = paths;; ++p) {
        WCHAR c = *p;
        switch (c) {
        case L'\0':
            return q == pathEnd;
        case L';':
            if (q == pathEnd) {
                return true;
            }
            q = path;
            break;
        default:
            if (q != NULL) {
                if (q != pathEnd && *q == c) {
                    ++q;
                } else {
                    q = NULL;
                }
            }
            break;
        }
    }
}

}

namespace desktop_win32 {

void extendLoaderEnvironment(WCHAR * binPath, WCHAR * iniDirectory) {
    if (!GetModuleFileNameW(NULL, iniDirectory, MAX_PATH)) {
        fail();
    }
    WCHAR * pathEnd = filename(iniDirectory);
    WCHAR name[MAX_PATH + MY_LENGTH(L".bin")];
        // hopefully std::size_t is large enough to not overflow
    WCHAR * nameEnd = name;
    for (WCHAR * p = pathEnd; *p != L'\0'; ++p) {
        *nameEnd++ = *p;
    }
    if (!(nameEnd - name >= 4 && nameEnd[-4] == L'.' &&
          (nameEnd[-3] == L'E' || nameEnd[-3] == L'e') &&
          (nameEnd[-2] == L'X' || nameEnd[-2] == L'x') &&
          (nameEnd[-1] == L'E' || nameEnd[-1] == L'e')))
    {
        *nameEnd = L'.';
        nameEnd += 4;
    }
    nameEnd[-3] = 'b';
    nameEnd[-2] = 'i';
    nameEnd[-1] = 'n';
    buildPath(binPath, iniDirectory, pathEnd, name, nameEnd - name);
    WCHAR path[MAX_PATH];
    pathEnd = buildPath(
        path, iniDirectory, pathEnd, MY_STRING(L"..\\basis-link"));
    if (pathEnd == NULL) {
        fail();
    }
    pathEnd = resolveLink(path);
    if (pathEnd == NULL) {
        fail();
    }
    std::size_t const maxEnv = 32767;
    WCHAR pad[2 * MAX_PATH + maxEnv];
        // hopefully std::size_t is large enough to not overflow
    WCHAR * padEnd = buildPath(pad, path, pathEnd, MY_STRING(L"\\program"));
    if (padEnd == NULL) {
        fail();
    }
    WCHAR env[maxEnv];
    DWORD n = GetEnvironmentVariableW(L"PATH", env, maxEnv);
    if (n >= maxEnv || n == 0 && GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
        fail();
    }
    env[n] = L'\0';
    bool exclude1 = contains(env, pad, padEnd);
    WCHAR * pad2 = exclude1 ? pad : padEnd + 1;
    pathEnd = buildPath(path, path, pathEnd, MY_STRING(L"\\ure-link"));
    if (pathEnd == NULL) {
        fail();
    }
    pathEnd = resolveLink(path);
    if (pathEnd == NULL) {
        fail();
    }
    padEnd = buildPath(pad2, path, pathEnd, MY_STRING(L"\\bin"));
    if (padEnd == NULL) {
        fail();
    }
    bool exclude2 = contains(env, pad2, padEnd);
    if (!(exclude1 && exclude2)) {
        if (!(exclude1 || exclude2)) {
            pad2[-1] = L';';
        }
        WCHAR * p = exclude2 ? pad2 - 1 : padEnd;
        if (n != 0) {
            *p++ = L';';
        }
        for (DWORD i = 0; i <= n; ++i) {
            *p++ = env[i];
        }
        if (!SetEnvironmentVariableW(L"PATH", pad)) {
            fail();
        }
    }
}

}
