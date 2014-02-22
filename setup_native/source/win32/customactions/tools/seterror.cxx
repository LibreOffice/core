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

#define UNICODE

#ifdef _MSC_VER
#pragma warning(push,1) // disable warnings within system headers
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include "strsafe.h"

#include <seterror.hxx>


#ifdef DEBUG
inline void OutputDebugStringFormat( LPCTSTR pFormat, ... )
{
    TCHAR    buffer[1024];
    va_list  args;

    va_start( args, pFormat );
    StringCchVPrintf( buffer, sizeof(buffer), pFormat, args );
    OutputDebugString( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCTSTR, ... )
{
}
#endif


void SetMsiErrorCode( int nErrorCode )
{
    const TCHAR sMemMapName[] = TEXT( "Global\\MsiErrorObject" );

    HANDLE hMapFile;
    int *pBuf;

    hMapFile = OpenFileMapping(
                    FILE_MAP_ALL_ACCESS,    // read/write access
                    FALSE,                  // do not inherit the name
                    sMemMapName );          // name of mapping object

    if ( hMapFile == NULL )                 // can not set error code
    {
        OutputDebugStringFormat( TEXT("Could not open map file (%d).\n"), GetLastError() );
        return;
    }

    pBuf = (int*) MapViewOfFile( hMapFile,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        sizeof( int ) );
    if ( pBuf )
    {
        *pBuf = nErrorCode;
        UnmapViewOfFile( pBuf );
    }
    else
        OutputDebugStringFormat( TEXT("Could not map view of file (%d).\n"), GetLastError() );

    CloseHandle( hMapFile );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
