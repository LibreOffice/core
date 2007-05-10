/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: seterror.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: gm $ $Date: 2007-05-10 11:05:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 ************************************************************************/

#define UNICODE

#pragma warning(push,1) // disable warnings within system headers
#include <windows.h>
#pragma warning(pop)

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


