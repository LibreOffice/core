/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "tools/pathutils.hxx"

#include "extendloaderenvironment.hxx"

namespace {

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
    WCHAR * iniDirEnd = tools::filename(iniDirectory);
    WCHAR name[MAX_PATH + MY_LENGTH(L".bin")];
        // hopefully std::size_t is large enough to not overflow
    WCHAR * nameEnd = name;
    for (WCHAR * p = iniDirEnd; *p != L'\0'; ++p) {
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
    tools::buildPath(binPath, iniDirectory, iniDirEnd, name, nameEnd - name);
    *iniDirEnd = L'\0';
    WCHAR path[MAX_PATH];
    WCHAR * pathEnd = tools::buildPath(
        path, iniDirectory, iniDirEnd, MY_STRING(L"..\\basis-link"));
    if (pathEnd == NULL) {
        fail();
    }
    std::size_t const maxEnv = 32767;
    WCHAR pad[2 * MAX_PATH + maxEnv];
        // hopefully std::size_t is large enough to not overflow
    WCHAR * padEnd = NULL;
    WCHAR env[maxEnv];
    DWORD n = GetEnvironmentVariableW(L"PATH", env, maxEnv);
    if (n >= maxEnv || n == 0 && GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
        fail();
    }
    env[n] = L'\0';
    bool exclude1;
    pathEnd = tools::resolveLink(path);
    if (pathEnd == NULL) {
        if (GetLastError() != ERROR_FILE_NOT_FOUND) {
            fail();
        }
        // This path is only taken by testtool.exe in basis program directory;
        // its PATH needs to include the brand program directory:
        pathEnd = tools::buildPath(
            path, iniDirectory, iniDirEnd, MY_STRING(L".."));
        if (pathEnd == NULL) {
            fail();
        }
        padEnd = tools::buildPath(
            pad, path, pathEnd, MY_STRING(L"\\..\\program"));
        if (padEnd == NULL) {
            fail();
        }
        exclude1 = contains(env, pad, padEnd);
    } else {
        exclude1 = true;
    }
    WCHAR * pad2 = exclude1 ? pad : padEnd + 1;
    pathEnd = tools::buildPath(path, path, pathEnd, MY_STRING(L"\\ure-link"));
    if (pathEnd == NULL) {
        fail();
    }
    pathEnd = tools::resolveLink(path);
    if (pathEnd == NULL) {
        fail();
    }
    padEnd = tools::buildPath(pad2, path, pathEnd, MY_STRING(L"\\bin"));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
