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

IMPLEMENT_THUNK( psapi, WINDOWS, DWORD, WINAPI, GetModuleFileNameExW, (HANDLE hProcess, HMODULE hModule, LPWSTR lpFileNameW, DWORD nSize ) )
{
    AUTO_STR( lpFileName, 2 * nSize );

    if ( GetModuleFileNameExA( hProcess, hModule, lpFileNameA, 2 * nSize ) )
        return (DWORD) STR2WSTR( lpFileName, nSize );
    else
        return 0;
}