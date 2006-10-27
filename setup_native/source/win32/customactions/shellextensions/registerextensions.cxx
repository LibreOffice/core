/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: registerextensions.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-10-27 12:10:48 $
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

#undef UNICODE
#undef _UNICODE

#pragma warning(push, 1) /* disable warnings within system headers */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#pragma warning(pop)

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

#ifdef _WIN32_WINNT
#error YES
#endif

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

extern "C" UINT __stdcall RegisterExtensions(MSIHANDLE handle)
{
    std::_tstring sInstDir = GetMsiProperty( handle, TEXT("INSTALLLOCATION") );
    std::_tstring sUnoPkgFile = sInstDir + TEXT("program\\unopkg.exe");
    std::_tstring sShareInstallDir = sInstDir + TEXT("share\\extension\\install\\");
    std::_tstring sPattern = sShareInstallDir + TEXT("*.oxt");
    std::_tstring mystr;

    WIN32_FIND_DATA aFindFileData;

    // Find unopkg.exe

    HANDLE hFindUnopkg = FindFirstFile( sUnoPkgFile.c_str(), &aFindFileData );

    if ( hFindUnopkg != INVALID_HANDLE_VALUE )
    {
        // unopkg.exe exists in program directory

        // Finding all oxt files in sShareInstallDir

        HANDLE hFindOxt = FindFirstFile( sPattern.c_str(), &aFindFileData );

        if ( hFindOxt != INVALID_HANDLE_VALUE )
        {
            bool fNextFile = false;

            do
            {
                std::_tstring sOxtFile = sShareInstallDir + aFindFileData.cFileName;
                std::_tstring sCommand = sUnoPkgFile + " add --shared " + "\"" + sOxtFile + "\"";

                mystr = "Command: " + sCommand;
                // MessageBox(NULL, mystr.c_str(), "Command", MB_OK);

                bool fSuccess = ExecuteCommand( sCommand.c_str(), TRUE );

                if ( fSuccess )
                {
                    mystr = "Executed successfully!";
                    // MessageBox(NULL, mystr.c_str(), "Command", MB_OK);
                }
                else
                {
                    mystr = "An error occured during execution!";
                    // MessageBox(NULL, mystr.c_str(), "Command", MB_OK);
                }

                fNextFile = FindNextFile( hFindOxt, &aFindFileData );

            } while ( fNextFile );

            FindClose( hFindOxt );
        }
    }
    // else
    // {
    //     mystr = "Error: Did not find " + sUnoPkgFile;
    //     MessageBox(NULL, mystr.c_str(), "Command", MB_OK);
    // }

    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall DeregisterExtensions(MSIHANDLE handle)
{
    std::_tstring mystr;

    // Finding the product with the help of the propery FINDPRODUCT,
    // that contains a Windows Registry key, that points to the install location.

    TCHAR szValue[8192];
    DWORD nValueSize = sizeof(szValue);
    HKEY  hKey;
    std::_tstring sInstDir;

    std::_tstring sProductKey = GetMsiProperty( handle, TEXT("FINDPRODUCT") );
    // MessageBox( NULL, sProductKey.c_str(), "Titel", MB_OK );

    if ( ERROR_SUCCESS == RegOpenKey( HKEY_CURRENT_USER,  sProductKey.c_str(), &hKey ) )
    {
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("INSTALLLOCATION"), NULL, NULL, (LPBYTE)szValue, &nValueSize ) )
        {
            sInstDir = szValue;
        }
        RegCloseKey( hKey );
    }
    else if ( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE,  sProductKey.c_str(), &hKey ) )
    {
        if ( ERROR_SUCCESS == RegQueryValueEx( hKey, TEXT("INSTALLLOCATION"), NULL, NULL, (LPBYTE)szValue, &nValueSize ) )
        {
            sInstDir = szValue;
        }
        RegCloseKey( hKey );
    }
    else
    {
        return ERROR_SUCCESS;
    }

    // MessageBox( NULL, sInstDir.c_str(), "install location", MB_OK );

    // Searching for the unopkg.exe

    std::_tstring sUnoPkgFile = sInstDir + TEXT("program\\unopkg.exe");
    std::_tstring sShareInstallDir = sInstDir + TEXT("share\\extension\\install\\");
    std::_tstring sPattern = sShareInstallDir + TEXT("*.oxt");

    WIN32_FIND_DATA aFindFileData;

    // Find unopkg.exe

    HANDLE hFindUnopkg = FindFirstFile( sUnoPkgFile.c_str(), &aFindFileData );

    if ( hFindUnopkg != INVALID_HANDLE_VALUE )
    {
        // unopkg.exe exists in program directory

        // Finding all oxt files in sShareInstallDir

        HANDLE hFindOxt = FindFirstFile( sPattern.c_str(), &aFindFileData );

        if ( hFindOxt != INVALID_HANDLE_VALUE )
        {
            bool fNextFile = false;

            do
            {
                // When removing extensions, only the oxt file name is required, without path
                // Therefore no quoting is required
                // std::_tstring sOxtFile = sShareInstallDir + aFindFileData.cFileName;
                std::_tstring sOxtFile = aFindFileData.cFileName;
                std::_tstring sCommand = sUnoPkgFile + " remove --shared " + "\"" + sOxtFile + "\"";

                mystr = "Command: " + sCommand;
                // MessageBox(NULL, mystr.c_str(), "Command", MB_OK);

                bool fSuccess = ExecuteCommand( sCommand.c_str(), TRUE );

                if ( fSuccess )
                {
                    mystr = "Executed successfully!";
                    // MessageBox(NULL, mystr.c_str(), "Command", MB_OK);
                }
                else
                {
                    mystr = "An error occured during execution!";
                    // MessageBox(NULL, mystr.c_str(), "Command", MB_OK);
                }

                fNextFile = FindNextFile( hFindOxt, &aFindFileData );

            } while ( fNextFile );

            FindClose( hFindOxt );
        }
    }
    // else
    // {
    //     mystr = "Not found: " + sUnoPkgFile;
    //     MessageBox(NULL, mystr.c_str(), "Command", MB_OK);
    // }

    return ERROR_SUCCESS;
}
