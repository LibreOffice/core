/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
