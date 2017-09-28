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

#ifndef INCLUDED_SHELL_INC_INTERNAL_UTILITIES_HXX
#define INCLUDED_SHELL_INC_INTERNAL_UTILITIES_HXX

#include <malloc.h>
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#if !defined WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include "types.hxx"

#include <string>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

/** Convert a string to a wstring
    using CP_ACP
*/
std::wstring StringToWString(const std::string& String);


/** Convert a wstring to a string
    using CP_ACP
*/
std::string WStringToString(const std::wstring& String);


/** Convert a string to a wstring
    using CP_UTF8
*/
std::wstring UTF8ToWString(const std::string& String);


/** Retrieve a string from the
    resources of this module
*/
std::wstring GetResString(int ResId);


/** helper function to judge if the string is only has spaces.
    @returns
        <TRUE>if the provided string contains only but at least one space
        character else <FALSE/>.
*/
bool HasOnlySpaces(const std::wstring& String);

/** convert LocaleSet pair into Windows LCID identifier.
    @returns
        Windows Locale Identifier corresponding to input LocaleSet.
*/


/** Convert a long path name using Windows api call GetShortPathName
*/
std::wstring getShortPathName( const std::wstring& aLongName );


LCID LocaleSetToLCID( const LocaleSet_t & Locale );


#ifdef DEBUG
inline void OutputDebugStringFormatW( LPCWSTR pFormat, ... )
{
    WCHAR    buffer[1024];
    va_list args;

    va_start( args, pFormat );
    StringCchVPrintfW( buffer, sizeof(buffer)/sizeof(*buffer), pFormat, args );
    va_end( args );
    OutputDebugStringW( buffer );
}
#else
static inline void OutputDebugStringFormatW( LPCWSTR, ... )
{
}
#endif


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
