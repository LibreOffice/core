/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stringconverter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 13:32:15 $
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

#ifndef _STRINGCONVERTER_HXX_
#include "stringconverter.hxx"
#endif

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>

/** Convert a Unicode string to an ANSI string based on CP_ACP
*/
std::string UnicodeToAnsiString(const std::wstring& UniString)
{
    int len = WideCharToMultiByte(
        CP_ACP, 0, UniString.c_str(), -1, 0, 0, 0, 0);

    char* buff = reinterpret_cast<char*>(_alloca(len));

    WideCharToMultiByte(
        CP_ACP, 0, UniString.c_str(), -1, buff, len, 0, 0);

    return std::string(buff);
}

/** Convert an ANSI string to unicode based on CP_ACP
*/
std::wstring AnsiToUnicodeString(const std::string& AnsiString)
{
    int len = MultiByteToWideChar(
        CP_ACP, 0, AnsiString.c_str(), -1, 0, 0);

    wchar_t* buff = reinterpret_cast<wchar_t*>(_alloca(len * sizeof(wchar_t)));

    MultiByteToWideChar(
        CP_ACP, 0, AnsiString.c_str(), -1, buff, len);

    return std::wstring(buff);
}


