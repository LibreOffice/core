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

#define _WIN32_WINNT 0x0401

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <../tools/msiprop.hxx>
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

extern "C" UINT __stdcall CheckPatchList( MSIHANDLE handle )
{
    LPTSTR sPatchList = NULL;
    LPTSTR sRequiredPatch = NULL;

    if ( GetMsiProp( handle, TEXT("PATCH"), &sPatchList ) && GetMsiProp( handle, TEXT("PREREQUIREDPATCH"), &sRequiredPatch ) )
    {
        OutputDebugStringFormat( "CheckPatchList called with PATCH=%s and PRQ= %s\n", sPatchList, sRequiredPatch );
        if ( _tcsstr( sPatchList, sRequiredPatch ) )
        {
            MsiSetProperty( handle, TEXT("IGNOREPREREQUIREDPATCH"), TEXT("1") );
            OutputDebugStringFormat( "Set Property IgnorePrerequiredPatch!\n" );
        }
    }
    else
    {
        OutputDebugStringFormat( "CheckPatchList called with PATCH=%s and PRQ= %s\n", sPatchList, sRequiredPatch );
    }
    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
