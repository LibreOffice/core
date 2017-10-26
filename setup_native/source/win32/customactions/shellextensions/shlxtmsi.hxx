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

#ifndef INCLUDED_SETUP_NATIVE_SOURCE_WIN32_CUSTOMACTIONS_SHELLEXTENSIONS_SHLXTMSI_HXX
#define INCLUDED_SETUP_NATIVE_SOURCE_WIN32_CUSTOMACTIONS_SHELLEXTENSIONS_SHLXTMSI_HXX

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>

#include <string>

static inline std::wstring GetMsiPropertyW( MSIHANDLE handle, const std::wstring& sProperty )
{
    std::wstring   result;
    WCHAR szDummy[1] = L"";
    DWORD nChars = 0;

    if ( MsiGetPropertyW( handle, sProperty.c_str(), szDummy, &nChars ) == ERROR_MORE_DATA )
    {
        DWORD nBytes = ++nChars * sizeof(WCHAR);
        PWSTR buffer = static_cast<PWSTR>(_alloca(nBytes));
        ZeroMemory( buffer, nBytes );
        MsiGetPropertyW( handle, sProperty.c_str(), buffer, &nChars );
        result = buffer;
    }

    return  result;
}

static inline void SetMsiPropertyW( MSIHANDLE handle, const std::wstring& sProperty, const std::wstring& sValue )
{
    MsiSetPropertyW( handle, sProperty.c_str(), sValue.c_str() );
}

static inline void UnsetMsiPropertyW( MSIHANDLE handle, const std::wstring& sProperty )
{
    MsiSetPropertyW( handle, sProperty.c_str(), nullptr );
}

#endif // INCLUDED_SETUP_NATIVE_SOURCE_WIN32_CUSTOMACTIONS_SHELLEXTENSIONS_SHLXTMSI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
