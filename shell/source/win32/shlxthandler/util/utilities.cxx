/*************************************************************************
 *
 *  $RCSfile: utilities.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-07 11:18:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef UTILITIES_HXX_INCLUDED
#include "internal/utilities.hxx"
#endif

#ifndef CONFIG_HXX_INCLUDED
#include "internal/config.hxx"
#endif

#ifndef DBGMACROS_HXX_INCLUDED
#include "internal/dbgmacros.hxx"
#endif

#include <malloc.h>
#include <windows.h>

//-----------------------------
// constants
//-----------------------------

const size_t MAX_RES_STRING = 1024;


//---------------------------------
/**
*/
std::wstring StringToWString(const std::string& String)
{
    int len = MultiByteToWideChar(
        CP_ACP, 0, String.c_str(), -1, 0, 0);

    wchar_t* buff = reinterpret_cast<wchar_t*>(
        _alloca(len * sizeof(wchar_t)));

    MultiByteToWideChar(
        CP_ACP, 0, String.c_str(), -1, buff, len);

    return std::wstring(buff);
}

//---------------------------------
/**
*/
std::string WStringToString(const std::wstring& String)
{
    int len = WideCharToMultiByte(
        CP_ACP, 0, String.c_str(), -1, 0, 0, 0, 0);

    char* buff = reinterpret_cast<char*>(
        _alloca(len * sizeof(char)));

    WideCharToMultiByte(
        CP_ACP, 0, String.c_str(), -1, buff, len, 0, 0);

    return std::string(buff);
}

//---------------------------------
/**
*/
std::wstring GetResString(int ResId)
{
    wchar_t szResStr[MAX_RES_STRING];

    int rc = LoadStringW(
        GetModuleHandleW(MODULE_NAME),
        ResId,
        szResStr,
        sizeof(szResStr));

    ENSURE(rc, "String resource not found");

    return std::wstring(szResStr);
}

//---------------------------------
/**
*/
bool is_windows_xp()
{
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    return (5 == osvi.dwMajorVersion && 1 == osvi.dwMinorVersion);
}

//---------------------------------
/**
*/
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

void SaveDebugInfoToFile( const std::wstring& str )
{
   int handle;
   char msg[] = "Hello world";

   if ((handle = open("c:\\temp\\SHELLRESULT.$$$", O_CREAT | O_RDWR | O_APPEND )) == -1)
   {
      perror("Error: open file error");
      return;
   }
   write(handle, str.c_str(), str.length() );
   close(handle);
}