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

#include <sal/config.h>

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <shlwapi.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <tools/pathutils.hxx>

#include "loader.hxx"

namespace desktop_win32 {

void getPaths(WCHAR * binPath, WCHAR * iniDirectory) {
    if (!GetModuleFileNameW(NULL, iniDirectory, MAX_PATH)) {
        LPWSTR buf = NULL;
        FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
            GetLastError(), 0, reinterpret_cast< LPWSTR >(&buf), 0, NULL);
        MessageBoxW(NULL, buf, NULL, MB_OK | MB_ICONERROR);
        LocalFree(buf);
        TerminateProcess(GetCurrentProcess(), 255);
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
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
