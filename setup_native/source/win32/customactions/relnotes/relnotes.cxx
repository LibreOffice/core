/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: relnotes.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 13:34:11 $
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

