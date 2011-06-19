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

#define _WIN32_WINNT 0x0401

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
#include <strsafe.h>

#include <systools/win32/uwinapi.h>

//----------------------------------------------------------
#ifdef DEBUG
inline void OutputDebugStringFormat( LPCSTR pFormat, ... )
{
    CHAR    buffer[1024];
    va_list args;

    va_start( args, pFormat );
    StringCchVPrintfA( buffer, sizeof(buffer), pFormat, args );
    OutputDebugStringA( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCSTR, ... )
{
}
#endif

static std::_tstring GetMsiProperty( MSIHANDLE handle, const std::_tstring& sProperty )
{
    std::_tstring result;
    TCHAR szDummy[1] = TEXT("");
    DWORD nChars = 0;

    if ( MsiGetProperty( handle, sProperty.c_str(), szDummy, &nChars ) == ERROR_MORE_DATA )
    {
        DWORD nBytes = ++nChars * sizeof(TCHAR);
        LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
        ZeroMemory( buffer, nBytes );
        MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
        result = buffer;
    }

    return result;
}

static void SetMsiProperty( MSIHANDLE handle, const std::_tstring& sProperty )
{
    MsiSetProperty( handle, sProperty.c_str(), TEXT("1") );
}

extern "C" UINT __stdcall CheckPatchList( MSIHANDLE handle )
{
    std::_tstring sPatchList = GetMsiProperty( handle, TEXT("PATCH") );
    std::_tstring sRequiredPatch = GetMsiProperty( handle, TEXT("PREREQUIREDPATCH") );

    OutputDebugStringFormat( "CheckPatchList called with PATCH=%s and PRQ= %s\n", sPatchList.c_str(), sRequiredPatch.c_str() );

    if ( ( sPatchList.length() != 0 ) && ( sRequiredPatch.length() != 0 ) )
    {
        if ( _tcsstr( sPatchList.c_str(), sRequiredPatch.c_str() ) )
        {
            SetMsiProperty( handle, TEXT("IGNOREPREREQUIREDPATCH") );
            OutputDebugStringFormat( "Set Property IgnorePrerequiredPatch!\n" );
        }
    }
    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
