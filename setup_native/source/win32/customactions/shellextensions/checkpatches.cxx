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

#include "shlxtmsi.hxx"
#include <strsafe.h>
#include <systools/win32/uwinapi.h>

#ifdef DEBUG
inline void OutputDebugStringFormatW( PCWSTR pFormat, ... )
{
    WCHAR   buffer[1024];
    va_list args;

    va_start( args, pFormat );
    StringCchVPrintfW( buffer, SAL_N_ELEMENTS(buffer), pFormat, args );
    OutputDebugStringW( buffer );
    va_end(args);
}
#else
static void OutputDebugStringFormatW( PCWSTR, ... )
{
}
#endif

extern "C" __declspec(dllexport) UINT __stdcall CheckPatchList( MSIHANDLE handle )
{
    std::wstring sPatchList = GetMsiPropertyW( handle, L"PATCH" );
    std::wstring sRequiredPatch = GetMsiPropertyW( handle, L"PREREQUIREDPATCH" );

    OutputDebugStringFormatW( L"CheckPatchList called with PATCH=%s and PRQ=%s\n", sPatchList.c_str(), sRequiredPatch.c_str() );

    if ( ( sPatchList.length() != 0 ) && ( sRequiredPatch.length() != 0 ) )
    {
        if ( wcsstr( sPatchList.c_str(), sRequiredPatch.c_str() ) )
        {
            SetMsiPropertyW( handle, L"IGNOREPREREQUIREDPATCH", L"1" );
            OutputDebugStringFormatW( L"Set Property IgnorePrerequiredPatch!\n" );
        }
    }
    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
