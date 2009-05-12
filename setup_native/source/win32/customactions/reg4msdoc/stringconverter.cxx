/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: stringconverter.cxx,v $
 * $Revision: 1.5 $
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

#include "stringconverter.hxx"

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


