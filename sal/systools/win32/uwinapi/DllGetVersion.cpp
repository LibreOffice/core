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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
