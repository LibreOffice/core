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

//----------------------------------------------------------
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

//----------------------------------------------------------
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


