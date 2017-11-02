/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/pathutils.hxx>
#include "loader.hxx"
#include <cassert>

namespace {

void fail()
{
    LPWSTR buf = nullptr;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
        GetLastError(), 0, reinterpret_cast< LPWSTR >(&buf), 0, nullptr);
    MessageBoxW(nullptr, buf, nullptr, MB_OK | MB_ICONERROR);
    HeapFree(GetProcessHeap(), 0, buf);
    TerminateProcess(GetCurrentProcess(), 255);
}

}

namespace desktop_win32 {

void extendLoaderEnvironment(WCHAR * binPath, WCHAR * iniDirectory) {
    if (!GetModuleFileNameW(nullptr, iniDirectory, MAX_PATH)) {
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
    std::size_t const maxEnv = 32767;
    WCHAR env[maxEnv];
    DWORD n = GetEnvironmentVariableW(L"PATH", env, maxEnv);
    if ((n >= maxEnv || n == 0) && GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
        fail();
    }
    // must be first in PATH to override other entries
    assert(*(iniDirEnd - 1) == L'\\'); // hence -1 below
    if (wcsncmp(env, iniDirectory, iniDirEnd - iniDirectory - 1) != 0
        || env[iniDirEnd - iniDirectory - 1] != L';')
    {
        WCHAR pad[MAX_PATH + maxEnv];
            // hopefully std::size_t is large enough to not overflow
        WCHAR * p = commandLineAppend(pad, iniDirectory, iniDirEnd - iniDirectory - 1);
        if (n != 0) {
            *p++ = L';';
            for (DWORD i = 0; i <= n; ++i) {
                *p++ = env[i];
            }
        } else {
            *p++ = L'\0';
        }
        if (!SetEnvironmentVariableW(L"PATH", pad)) {
            fail();
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
