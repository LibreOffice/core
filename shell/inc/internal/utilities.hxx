/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef UTILITIES_HXX_INCLUDED
#define UTILITIES_HXX_INCLUDED

#include <malloc.h>
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#ifndef OS2
#include <windows.h>
#endif
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include "internal/types.hxx"

#include <string>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

//---------------------------------
/** Convert a string to a wstring
    using CP_ACP
*/
std::wstring StringToWString(const std::string& String);

//---------------------------------
/** Convert a wstring to a string
    using CP_ACP
*/
std::string WStringToString(const std::wstring& String);

//---------------------------------
/** Retrieve a string from the
    resources of this module
*/
std::wstring GetResString(int ResId);

//---------------------------------
/** Returns whether we are running
    on Windows XP or not
*/
bool is_windows_xp_or_above();

//---------------------------------
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

//---------------------------------
/** Convert a long path name using Windows api call GetShortPathName
*/
std::wstring getShortPathName( const std::wstring& aLongName );


#ifndef OS2
LCID LocaleSetToLCID( const LocaleSet_t & Locale );
#endif

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
//----------------------------------------------------------


#endif
