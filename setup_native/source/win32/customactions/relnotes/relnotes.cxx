/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: relnotes.cxx,v $
 * $Revision: 1.4 $
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

#ifdef _MSC_VER
#pragma warning(push,1) // disable warnings within system headers
#pragma warning(disable: 4917)
#endif
#include <windows.h>
#include <msiquery.h>
#include <shlobj.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include "strsafe.h"

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
inline bool IsValidHandle( HANDLE handle )
{
    return (NULL != handle) && (INVALID_HANDLE_VALUE != handle);
}

//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
UINT ShowReleaseNotes( MSIHANDLE , TCHAR* pFileName )
{
    TCHAR szPath[MAX_PATH];

    if( FAILED( SHGetSpecialFolderPath( NULL, szPath, CSIDL_COMMON_DOCUMENTS, true ) ) )
        return ERROR_SUCCESS;

    if ( FAILED( StringCchCat( szPath, sizeof( szPath ), pFileName ) ) )
        return ERROR_SUCCESS;

    HANDLE hFile = CreateFile( szPath, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if ( IsValidHandle(hFile) )
    {
        CloseHandle( hFile );
        OutputDebugStringFormat( TEXT("DEBUG: ShowReleaseNotes: Found file [%s]"), szPath );

        SHELLEXECUTEINFOW aExecInf;
        ZeroMemory( &aExecInf, sizeof( aExecInf ) );

        aExecInf.cbSize = sizeof( aExecInf );
        aExecInf.fMask  = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI;
        aExecInf.lpVerb = TEXT("open");
        aExecInf.lpFile = szPath;
        aExecInf.lpDirectory = NULL;
        aExecInf.nShow = SW_SHOWNORMAL;

        SetLastError( 0 );
        ShellExecuteEx( &aExecInf );
    }
    else
    {
        OutputDebugStringFormat( TEXT("DEBUG: ShowReleaseNotes: File not found [%s]"), szPath );
        return ERROR_SUCCESS;
    }

    return ERROR_SUCCESS;
}

//----------------------------------------------------------
extern "C" UINT __stdcall ShowReleaseNotesBefore( MSIHANDLE hMSI )
{
    OutputDebugString( TEXT("DEBUG: ShowReleaseNotesBefore called") );
    return ShowReleaseNotes( hMSI, TEXT("\\sun\\releasenote1.url") );
}

//----------------------------------------------------------
extern "C" UINT __stdcall ShowReleaseNotesAfter( MSIHANDLE hMSI )
{
    OutputDebugString( TEXT("DEBUG: ShowReleaseNotesAfter called") );
    return ShowReleaseNotes( hMSI, TEXT("\\sun\\releasenote2.url") );
}

