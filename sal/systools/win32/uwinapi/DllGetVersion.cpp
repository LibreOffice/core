/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DllGetVersion.cpp,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 13:49:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#define WIN32_LEAN_AND_MEAN
#ifdef _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#include <shlwapi.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <malloc.h>

extern HMODULE      UWINAPI_BaseAddress;

// This function should be exported by every DLL that wants to provide it's version number.
// This code automaticly generates the information from the version resource

extern "C" HRESULT CALLBACK DllGetVersion( DLLVERSIONINFO *pdvi )
{
    TCHAR   szModulePath[MAX_PATH];
    BOOL    fSuccess = FALSE;

    if ( UWINAPI_BaseAddress && GetModuleFileName( UWINAPI_BaseAddress, szModulePath, MAX_PATH ) )
    {
        DWORD   dwHandle = 0;
        DWORD   dwSize = GetFileVersionInfoSize( szModulePath, &dwHandle );
        LPVOID  lpData = _alloca( dwSize );

        if ( GetFileVersionInfo( szModulePath, dwHandle, dwSize, lpData ) )
        {
            VS_FIXEDFILEINFO    *lpBuffer = NULL;
            UINT    uLen = 0;

            if ( VerQueryValue( lpData, TEXT("\\"), (LPVOID *)&lpBuffer, &uLen ) )
            {
                pdvi->dwMajorVersion = HIWORD( lpBuffer->dwFileVersionMS );
                pdvi->dwMinorVersion = LOWORD( lpBuffer->dwFileVersionMS );
                pdvi->dwBuildNumber = HIWORD( lpBuffer->dwFileVersionLS );
                pdvi->dwPlatformID = (DWORD) ((lpBuffer->dwFileOS & VOS_NT) ? DLLVER_PLATFORM_NT : DLLVER_PLATFORM_WINDOWS);

                fSuccess = TRUE;
            }
        }
    }

    return fSuccess ? HRESULT_FROM_WIN32( GetLastError() ) : HRESULT_FROM_WIN32( NO_ERROR );
}
