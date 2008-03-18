/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: checkdirectory.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 12:53:34 $
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

#define _WIN32_WINNT 0x0401

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
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

#include <systools/win32/uwinapi.h>
#include <../tools/seterror.hxx>

static std::_tstring GetMsiProperty( MSIHANDLE handle, const std::_tstring& sProperty )
{
    std::_tstring result;
    TCHAR szDummy[1] = TEXT("");
    DWORD nChars = 0;

    if ( MsiGetProperty( handle, sProperty.c_str(), szDummy, &nChars ) == ERROR_MORE_DATA )
    {
        DWORD nBytes = ++nChars * sizeof(TCHAR);
        LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
        ZeroMemory( buffer, nBytes );
        MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
        result = buffer;
    }

    return result;
}

static void UnsetMsiProperty(MSIHANDLE handle, const std::_tstring& sProperty)
{
    MsiSetProperty(handle, sProperty.c_str(), NULL);
}

static void SetMsiProperty(MSIHANDLE handle, const std::_tstring& sProperty, const std::_tstring&)
{
    MsiSetProperty(handle, sProperty.c_str(), TEXT("1"));
}

extern "C" UINT __stdcall CheckInstallDirectory(MSIHANDLE handle)
{
    std::_tstring sInstallPath = GetMsiProperty(handle, TEXT("OFFICEINSTALLLOCATION"));

    // MessageBox(NULL, sInstallPath.c_str(), "DEBUG", MB_OK);

    // unsetting all properties

    UnsetMsiProperty( handle, TEXT("DIRECTORY_NOT_EMPTY") );

    // 1. Searching for file setup.ini

    std::_tstring sSetupIniPath = sInstallPath + TEXT("program\\setup.ini");

    WIN32_FIND_DATA data;
    HANDLE hdl = FindFirstFile(sSetupIniPath.c_str(), &data);

    // std::_tstring mystr = "Searching for " + sSetupIniPath;
    // MessageBox(NULL, mystr.c_str(), "DEBUG", MB_OK);

    if ( IsValidHandle(hdl) )
    {
        // setup.ini found -> directory cannot be used for installation.
        SetMsiProperty( handle, TEXT("DIRECTORY_NOT_EMPTY"), TEXT("1") );
        SetMsiErrorCode( MSI_ERROR_DIRECTORY_NOT_EMPTY );
        // std::_tstring notEmptyStr = "Directory is not empty. Please choose another installation directory.";
        // std::_tstring notEmptyTitle = "Directory not empty";
        // MessageBox(NULL, notEmptyStr.c_str(), notEmptyTitle.c_str(), MB_OK);
    }

    return ERROR_SUCCESS;
}
