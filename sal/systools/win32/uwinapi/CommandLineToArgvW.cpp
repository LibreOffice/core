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


#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(disable:4740)
#endif

#include "macros.h"

#ifdef __cplusplus
#define local inline
#else
#define local static
#endif

local LPCWSTR SkipBlanks( LPCWSTR lpScan )
{
    while ( ' ' == *lpScan || '\t' == *lpScan )
        lpScan++;

    return lpScan;
}


local LPCWSTR SkipArgument( LPCWSTR lpScan )
{
    BOOL    fQuoted = FALSE;
    LPCWSTR lpArgEnd = NULL;

    do
    {
        switch ( *lpScan )
        {
        case ' ':
        case '\t':
            if ( fQuoted )
                lpScan++;
            else
                lpArgEnd = lpScan;
            break;
        case '\"':
            lpScan++;
            fQuoted = !fQuoted;
            break;
        case '\0':
            lpArgEnd = lpScan;
            break;
        default:
            lpScan++;
            break;
        }
    } while( *lpScan && !lpArgEnd );

    return lpScan;
}


IMPLEMENT_THUNK( shell32, WINDOWS, LPWSTR *, WINAPI, CommandLineToArgvW, ( LPCWSTR lpCmdLineW, int *pNumArgs ) )
{
    LPWSTR  *lpArgvW = NULL;

    if ( !lpCmdLineW || !*lpCmdLineW )
    {
        CHAR    szFileName[MAX_PATH];

        DWORD   dwResult = GetModuleFileNameA( NULL, szFileName, MAX_PATH );

        if ( dwResult && dwResult < MAX_PATH )
        {
            int cchNeeded = MultiByteToWideChar( CP_ACP, 0, szFileName, -1, NULL, 0 );

            lpArgvW = (LPWSTR *)GlobalAlloc( 0, cchNeeded * sizeof(WCHAR) + sizeof(LPWSTR) );

            if ( lpArgvW )
            {
                lpArgvW[0] = (LPWSTR)(lpArgvW + 1);

                MultiByteToWideChar( CP_ACP, 0, szFileName, -1, lpArgvW[0], cchNeeded );
                *pNumArgs = 1;
            }
            else
                SetLastError( ERROR_OUTOFMEMORY );
        }
    }
    else
    {
        LPCWSTR lpScan = lpCmdLineW;
        int     nTokens = 0;
        int     cchNeeded = 0;

        // Count arguments and required size

        while ( *lpScan )
        {
            lpScan = SkipBlanks( lpScan );
            if ( *lpScan )
            {
                LPCWSTR lpArgEnd = SkipArgument( lpScan );

                nTokens++;
                cchNeeded += lpArgEnd - lpScan + 1;
                lpScan = lpArgEnd;
            }
        }

        // Allocate space for one additional NULL pointer to terminate list

        lpArgvW = (LPWSTR *)GlobalAlloc( 0, sizeof(LPWSTR) * (nTokens + 1) + sizeof(WCHAR) * cchNeeded );

        if ( lpArgvW )
        {
            // Collect arguments

            LPWSTR  lpDestination = (LPWSTR)&lpArgvW[nTokens + 1];

            lpScan = lpCmdLineW;
            nTokens = 0;

            while ( *lpScan )
            {
                lpScan = SkipBlanks( lpScan );
                if ( *lpScan )
                {
                    LPCWSTR lpArgEnd = SkipArgument( lpScan );

                    lpArgvW[nTokens++] = lpDestination;

                    while ( lpScan < lpArgEnd )
                    {
                        if ( '\"' != *lpScan )
                            *lpDestination++ = *lpScan;

                        lpScan++;
                    }
                    *lpDestination++ = 0;
                }
            }

            lpArgvW[nTokens] = NULL;

            *pNumArgs = nTokens;
        }
        else
            SetLastError( ERROR_OUTOFMEMORY );

    }

    return lpArgvW;
}