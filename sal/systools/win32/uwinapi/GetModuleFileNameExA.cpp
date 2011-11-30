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

#include "macros.h"
#ifdef _MSC_VER
#pragma warning(push,1) // disable warnings within system headers
#endif
#include <psapi.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <tlhelp32.h>

IMPLEMENT_THUNK( psapi, WINDOWS, DWORD, WINAPI, GetModuleFileNameExA, (HANDLE hProcess, HMODULE hModule, LPSTR lpFileName, DWORD nSize ) )
{
    DWORD   dwProcessId = 0;
    DWORD   dwResult = 0;

    if ( !hProcess || hProcess == GetCurrentProcess() || GetCurrentProcessId() == (dwProcessId = GetProcessId( hProcess )) )
        return GetModuleFileNameA( hModule, lpFileName, nSize );

    HANDLE  hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, dwProcessId );

    if ( IsValidHandle( hSnapshot ) )
    {
        MODULEENTRY32   me;

        me.dwSize = sizeof(me);
        if ( Module32First( hSnapshot, &me ) )
        {
            BOOL    fFound = FALSE;

            if ( NULL == hModule )
                fFound = TRUE;
            else do
            {
                fFound = (me.hModule == hModule);
            } while ( !fFound && Module32Next( hSnapshot, &me ) );

            if ( fFound )
            {
                dwResult = _tcslen( me.szExePath );

                if ( dwResult > nSize && nSize > 0 )
                    lpFileName[nSize -1] = 0;

                _tcsncpy( lpFileName, me.szExePath, nSize );
            }
        }

        CloseHandle( hSnapshot );
    }

    return dwResult;
}

