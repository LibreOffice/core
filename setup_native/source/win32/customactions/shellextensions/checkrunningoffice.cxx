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

#define _WIN32_WINDOWS 0x0410

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include <assert.h>

#ifdef UNICODE
#define _UNICODE
#define _tstring    wstring
#else
#define _tstring    string
#endif
#include <tchar.h>
#include <string>
#include <queue>
#include <stdio.h>
#include <sal/macros.h>

// works with Windows XP as well as with Windows 7
#define PSAPI_VERSION 1
#include <psapi.h>

#include <systools/win32/uwinapi.h>
#include <../tools/seterror.hxx>

#ifdef DEBUG
inline void OutputDebugStringFormat( LPCTSTR pFormat, ... )
{
    _TCHAR  buffer[1024];
    va_list args;

    va_start( args, pFormat );
    _vsntprintf( buffer, SAL_N_ELEMENTS(buffer), pFormat, args );
    OutputDebugString( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCTSTR, ... )
{
}
#endif

static std::_tstring GetMsiProperty( MSIHANDLE handle, const std::_tstring& sProperty )
{
    std::_tstring   result;
    TCHAR   szDummy[1] = TEXT("");
    DWORD   nChars = 0;

    if ( MsiGetProperty( handle, sProperty.c_str(), szDummy, &nChars ) == ERROR_MORE_DATA )
    {
        DWORD nBytes = ++nChars * sizeof(TCHAR);
        LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
        ZeroMemory( buffer, nBytes );
        MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
        result = buffer;
    }

    return  result;
}

extern "C" UINT __stdcall IsOfficeRunning( MSIHANDLE handle )
{
    std::_tstring sOfficeInstallPath = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));
    // Property empty -> no office installed
    if ( sOfficeInstallPath.length() == 0 )
        return ERROR_SUCCESS;

    DWORD aProcesses[1024], cbNeeded, cProcesses; /* 1024 processes ought to be enough for anybody */

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return ERROR_INSTALL_FAILURE;

    cProcesses = cbNeeded / sizeof(DWORD);

    // Check if there is "soffice.bin" among the processes
    for ( unsigned int i = 0; i < cProcesses; i++ )
    {
        if( aProcesses[i] != 0 )
        {
            TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
            HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,  aProcesses[i] );
            if (NULL != hProcess )
            {
                HMODULE hMod;
                if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), &cbNeeded) )
                    GetModuleBaseName( hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(TCHAR) );
            }
            CloseHandle( hProcess );
            if ( _tcscmp( szProcessName, TEXT("soffice.bin") ) == 0 )
            {
                MsiSetProperty( handle, TEXT("OFFICERUNS"), TEXT("1") );
                SetMsiErrorCode( MSI_ERROR_OFFICE_IS_RUNNING );
            }
        }
    }
    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
