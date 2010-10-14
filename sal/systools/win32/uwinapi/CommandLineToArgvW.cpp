/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
