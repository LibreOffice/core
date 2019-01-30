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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <strsafe.h>

#include "seterror.hxx"


#ifdef DEBUG
inline void OutputDebugStringFormatW( PCWSTR pFormat, ... )
{
    WCHAR    buffer[1024];
    va_list  args;

    va_start( args, pFormat );
    StringCchVPrintfW( buffer, sizeof(buffer)/sizeof(buffer[0]), pFormat, args );
    OutputDebugStringW( buffer );
    va_end(args);
}
#else
static void OutputDebugStringFormatW( PCWSTR, ... )
{
}
#endif


void SetMsiErrorCode( int nErrorCode )
{
    const WCHAR sMemMapName[] = L"Global\\MsiErrorObject";

    HANDLE hMapFile;
    int *pBuf;

    hMapFile = OpenFileMappingW(
                    FILE_MAP_ALL_ACCESS,    // read/write access
                    FALSE,                  // do not inherit the name
                    sMemMapName );          // name of mapping object

    if ( hMapFile == nullptr )                 // can not set error code
    {
        OutputDebugStringFormatW( L"Could not open map file (%d).\n", GetLastError() );
        return;
    }

    pBuf = static_cast<int*>(MapViewOfFile( hMapFile,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        sizeof( int ) ));
    if ( pBuf )
    {
        *pBuf = nErrorCode;
        UnmapViewOfFile( pBuf );
    }
    else
        OutputDebugStringFormatW( L"Could not map view of file (%d).\n", GetLastError() );

    CloseHandle( hMapFile );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
