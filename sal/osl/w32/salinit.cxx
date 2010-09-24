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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include "system.h"
#include <osl/process.h>
#include <sal/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Prototypes for initialization and deinitialization of SAL library

void SAL_CALL sal_detail_initialize(int argc, char ** argv)
{
    // SetProcessDEPPolicy(PROCESS_DEP_ENABLE);
    // SetDllDirectoryW(L"");
    // SetSearchPathMode(
    //   BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT);
    HMODULE h = GetModuleHandleW(L"kernel32.dll");
    if (h != 0) {
        FARPROC p = GetProcAddress(h, "SetProcessDEPPolicy");
        if (p != 0) {
            reinterpret_cast< BOOL (WINAPI *)(DWORD) >(p)(0x00000001);
    }
    p = GetProcAddress(h, "SetDllDirectoryW");
        if (p != 0) {
            reinterpret_cast< BOOL (WINAPI *)(LPCWSTR) >(p)(L"");
        }
        p = GetProcAddress(h, "SetSearchPathMode");
        if (p != 0) {
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
        WORD wMajorVersionRequired = 1;
        WORD wMinorVersionRequired = 1;

        if ((LOBYTE(wsaData.wVersion) <  wMajorVersionRequired) ||
            (LOBYTE(wsaData.wVersion) == wMajorVersionRequired) &&
            ((HIBYTE(wsaData.wVersion) < wMinorVersionRequired)))
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

void SAL_CALL sal_detail_deinitialize()
{
    if ( SOCKET_ERROR == WSACleanup() )
    {
        // We should never reach this point or we did wrong elsewhere
    }
}



#ifdef __cplusplus
}   // extern "C"
#endif
