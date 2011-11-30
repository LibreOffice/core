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



#define UNICODE
#include "macros.h"

EXTERN_C DWORD WINAPI GetLongPathNameW_NT( LPCWSTR lpShortPath, LPWSTR lpLongPath, DWORD cchBuffer )
#include "GetLongPathName.cpp"

EXTERN_C DWORD WINAPI GetLongPathNameW_WINDOWS( LPCWSTR lpShortPathW, LPWSTR lpLongPathW, DWORD cchBuffer )
{
    AUTO_WSTR2STR( lpShortPath );
    AUTO_STR( lpLongPath, cchBuffer );

    DWORD   dwResult = GetLongPathNameA( lpShortPathA, lpLongPathA, cchBuffer );

    if ( dwResult && dwResult < cchBuffer )
        STR2WSTR( lpLongPath, cchBuffer );

    return dwResult;
}


EXTERN_C void WINAPI ResolveThunk_GetLongPathNameW( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName )
{
    if ( (LONG)GetVersion() < 0 )
        *lppfn = (FARPROC)GetLongPathNameW_WINDOWS;
    else
    {
        FARPROC lpfnResult = GetProcAddress( LoadLibraryA( lpLibFileName ), lpFuncName );
        if ( !lpfnResult )
            lpfnResult = (FARPROC)GetLongPathNameW_NT;

        *lppfn = lpfnResult;
    }
}


DEFINE_CUSTOM_THUNK( kernel32, GetLongPathNameW, DWORD, WINAPI, GetLongPathNameW, ( LPCWSTR lpShortPathW, LPWSTR lpLongPathW, DWORD cchBuffer ) );
