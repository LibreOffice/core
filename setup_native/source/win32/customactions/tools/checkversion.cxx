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
#include <msiquery.h>

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <strsafe.h>

#include "seterror.hxx"


static BOOL GetMsiPropW( MSIHANDLE hMSI, const wchar_t* pPropName, wchar_t** ppValue )
{
    DWORD sz = 0;
    if ( MsiGetPropertyW( hMSI, pPropName, const_cast<wchar_t *>(L""), &sz ) == ERROR_MORE_DATA )
    {
        sz++;
        DWORD nbytes = sz * sizeof( wchar_t );
        wchar_t* buff = static_cast<wchar_t*>( malloc( nbytes ) );
        ZeroMemory( buff, nbytes );
        MsiGetPropertyW( hMSI, pPropName, buff, &sz );
        *ppValue = buff;

        return TRUE;
    }

    return FALSE;
}


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


extern "C" __declspec(dllexport) UINT __stdcall CheckVersions( MSIHANDLE hMSI )
{
    // MessageBoxW(NULL, L"CheckVersions", L"Information", MB_OK | MB_ICONINFORMATION);

    wchar_t* pVal = nullptr;

    if ( GetMsiPropW( hMSI, L"NEWPRODUCTS", &pVal ) && pVal )
    {
        OutputDebugStringFormatW( L"DEBUG: NEWPRODUCTS found [%s]", pVal );
        if ( *pVal != 0 )
            SetMsiErrorCode( MSI_ERROR_NEW_VERSION_FOUND );
        free( pVal );
    }
    pVal = nullptr;
    if ( GetMsiPropW( hMSI, L"OLDPRODUCTS", &pVal ) && pVal )
    {
        OutputDebugStringFormatW( L"DEBUG: OLDPRODUCTS found [%s]", pVal );
        if ( *pVal != 0 )
            SetMsiErrorCode( MSI_ERROR_OLD_VERSION_FOUND );
        free( pVal );
    }
    pVal = nullptr;

    return ERROR_SUCCESS;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
