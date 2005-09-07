/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: utilities.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:39:50 $
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

#ifndef UTILITIES_HXX_INCLUDED
#define UTILITIES_HXX_INCLUDED

#include <malloc.h>
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#ifndef TYPES_HXX_INCLUDED
#include "internal/types.hxx"
#endif

#include <string>

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
bool is_windows_xp();

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

LCID LocaleSetToLCID( const LocaleSet_t & Locale );

#endif
