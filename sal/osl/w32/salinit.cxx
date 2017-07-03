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

#include "sal/config.h"

#include "system.h"
#include "time.hxx"

#include <osl/process.h>
#include <sal/main.h>
#include <sal/types.h>

extern "C" {

// Prototypes for initialization and deinitialization of SAL library

void sal_detail_initialize(int argc, char ** argv)
{
    sal_initGlobalTimer();
    // SetProcessDEPPolicy(PROCESS_DEP_ENABLE);
    // SetDllDirectoryW(L"");
    // SetSearchPathMode(
    //   BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT);
    HMODULE h = GetModuleHandleW(L"kernel32.dll");
    if (h != nullptr) {
        FARPROC p;
#ifndef _WIN64
        p = GetProcAddress(h, "SetProcessDEPPolicy");
        if (p != 0) {
            reinterpret_cast< BOOL (WINAPI *)(DWORD) >(p)(0x00000001);
        }
#endif
        p = GetProcAddress(h, "SetDllDirectoryW");
        if (p != nullptr) {
            reinterpret_cast< BOOL (WINAPI *)(LPCWSTR) >(p)(L"");
        }
        p = GetProcAddress(h, "SetSearchPathMode");
        if (p != nullptr) {
            reinterpret_cast< BOOL (WINAPI *)(DWORD) >(p)(0x8001);
        }
    }

    WSADATA wsaData;
    int     error;
    WORD    wVersionRequested;

    wVersionRequested = MAKEWORD(1, 1);

    error = WSAStartup(wVersionRequested, &wsaData);
    if ( 0 == error )
    {
        WORD const wMajorVersionRequired = 1;
        WORD const wMinorVersionRequired = 1;

        if ((LOBYTE(wsaData.wVersion) <  wMajorVersionRequired) ||
            ((LOBYTE(wsaData.wVersion) == wMajorVersionRequired) &&
            ((HIBYTE(wsaData.wVersion) < wMinorVersionRequired))))
            {
                // How to handle a very unlikely error ???
            }
    }
    else
    {
        // How to handle a very unlikely error ???
    }

    osl_setCommandArgs(argc, argv);
}

void sal_detail_deinitialize()
{
    if ( SOCKET_ERROR == WSACleanup() )
    {
        // We should never reach this point or we did wrong elsewhere
    }
}

}   // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
