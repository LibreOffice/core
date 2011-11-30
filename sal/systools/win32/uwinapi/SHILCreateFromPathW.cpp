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

#include "uwinapi.h"

EXTERN_C LPITEMIDLIST WINAPI SHSimpleIDListFromPathW_Failure( LPCWSTR lpPathW )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return NULL;
}

EXTERN_C LPITEMIDLIST WINAPI SHSimpleIDListFromPathW_WINDOWS( LPCWSTR lpPathW )
{
    AUTO_WSTR2STR( lpPath );

    return SHSimpleIDListFromPathA( lpPathA );
}


EXTERN_C void WINAPI ResolveThunk_SHSimpleIDListFromPathW( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName )
{
    if ( (LONG)GetVersion < 0 )
        *lppfn = (FARPROC)SHSimpleIDListFromPathW_WINDOWS;
    else
    {
        FARPROC lpfnResult = GetProcAddress( LoadLibraryA( lpLibFileName ), MAKEINTRESOURCE(162) );
        if ( !lpfnResult )
            lpfnResult = (FARPROC)SHSimpleIDListFromPathW_Failure;

        *lppfn = lpfnResult;
    }
}


DEFINE_CUSTOM_THUNK( kernel32, GetLongPathNameW, DWORD, WINAPI, GetLongPathNameW, ( LPCWSTR lpShortPathW, LPWSTR lpLongPathW, DWORD cchBuffer ) );
