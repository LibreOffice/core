/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stringconverter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:38:19 $
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

#include <windows.h>
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


