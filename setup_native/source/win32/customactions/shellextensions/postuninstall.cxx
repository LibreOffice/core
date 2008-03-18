/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: postuninstall.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 12:54:23 $
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

#define _WIN32_WINDOWS 0x0410
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

#ifdef UNICODE
#define _UNICODE
#define _tstring    wstring
#else
#define _tstring    string
#endif
#include <tchar.h>
#include <string>

#include <io.h>

static std::_tstring GetMsiProperty( MSIHANDLE handle, const std::_tstring& sProperty )
{
    std::_tstring   result;
    TCHAR   szDummy[1] = TEXT("");
    DWORD   nChars = 0;

    if ( MsiGetProperty( handle, sProperty.c_str(), szDummy, &nChars ) == ERROR_MORE_DATA )
    {
        DWORD nBytes = ++nChars * sizeof(TCHAR);
        LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
        ZeroMemory( buffer, nBytes );
        MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
        result = buffer;
    }

    return  result;
}


static BOOL ExecuteCommand( LPCTSTR lpCommand, BOOL bSync )
{
    BOOL                fSuccess = FALSE;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);

    fSuccess = CreateProcess(
        NULL,
        (LPTSTR)lpCommand,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
        );

    if ( fSuccess )
    {
        if ( bSync )
            WaitForSingleObject( pi.hProcess, INFINITE );

        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
    }

    return fSuccess;
}

extern "C" UINT __stdcall ExecutePostUninstallScript( MSIHANDLE handle )
{
    TCHAR   szValue[8192];
    DWORD   nValueSize = sizeof(szValue);
    HKEY    hKey;
    std::_tstring   sInstDir;

    std::_tstring   sProductKey = GetMsiProperty( handle, TEXT("FINDPRODUCT") );

    // MessageBox( NULL, sProductKey.c_str(), "Titel", MB_OK );

    if ( ERROR_SUCCESS == RegOpenKey( HKEY_CURRENT_USER,  sProductKey.c_str(), &hKey ) )
    {
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("OFFICEINSTALLLOCATION"), NULL, NULL, (LPBYTE)szValue, &nValueSize ) )
        {
            sInstDir = szValue;
        }
        RegCloseKey( hKey );
    }
    else if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE,  sProductKey.c_str(), &hKey ) )
    {
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("OFFICEINSTALLLOCATION"), NULL, NULL, (LPBYTE)szValue, &nValueSize ) )
        {
            sInstDir = szValue;
        }
        RegCloseKey( hKey );
    }
    else
        return ERROR_SUCCESS;

    std::_tstring   sInfFile = sInstDir + TEXT("program\\postuninstall.inf");
    std::_tstring   sCommand = _T("RUNDLL32.EXE ");

    // MessageBox( NULL, sInfFile.c_str(), "Titel", MB_OK );

    if ( (LONG)GetVersion() < 0 )
        sCommand += _T("setupx.dll");
    else
        sCommand += _T("setupapi.dll");

    sCommand += _T(",InstallHinfSection PostUninstall 132 ");
    sCommand += sInfFile;

    if ( 0 == _taccess( sInfFile.c_str(), 2 ) )
        ExecuteCommand( sCommand.c_str(), TRUE );

    DeleteFile( sInfFile.c_str() );

    return ERROR_SUCCESS;
}

