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
#include <tlhelp32.h>

IMPLEMENT_THUNK( psapi, WINDOWS, BOOL, WINAPI, EnumProcesses, ( LPDWORD lpProcesses, DWORD cbSize, LPDWORD lpcbCopied ) )
{
    BOOL    fSuccess = FALSE;
    HANDLE  hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

    if ( IsValidHandle( hSnapshot ) )
    {
        PROCESSENTRY32  pe;

        if ( lpcbCopied )
            *lpcbCopied = 0;

        pe.dwSize = sizeof(pe);
        if ( Process32First( hSnapshot, &pe ) )
        {
            fSuccess = TRUE;

            while ( cbSize >= sizeof(*lpProcesses) )
            {
                *(lpProcesses++) = pe.th32ProcessID;
                if ( lpcbCopied )
                    *lpcbCopied += sizeof(*lpProcesses);
                cbSize -= sizeof(*lpProcesses);

                if ( !Process32Next( hSnapshot, &pe ) )
                    break;
            }
        }

        CloseHandle( hSnapshot );
    }
    else
        SetLastError( ERROR_INVALID_HANDLE );

    return fSuccess;
}


