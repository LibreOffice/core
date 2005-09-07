/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swappatchfiles.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:33:28 $
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#include <malloc.h>

#ifdef UNICODE
#define _UNICODE
#define _tstring    wstring
#else
#define _tstring    string
#endif
#include <tchar.h>
#include <string>
#include <stdio.h>

#ifdef _WIN32_WINNT
#error YES
#endif

#include <systools/win32/uwinapi.h>

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

static bool SwapFiles( const std::_tstring& sFileName1, const std::_tstring& sFileName2 )
{
    std::_tstring   sTempFileName = sFileName1 + TEXT(".tmp");

    bool fSuccess = true;

    //Try to move the original file to a temp file
    fSuccess = MoveFileEx( sFileName1.c_str(), sTempFileName.c_str(),
                           MOVEFILE_REPLACE_EXISTING  );

    if ( fSuccess )
    {
        fSuccess = MoveFileEx( sFileName2.c_str(), sFileName1.c_str(), MOVEFILE_REPLACE_EXISTING );

        if ( fSuccess )
        {
            fSuccess = MoveFileEx( sTempFileName.c_str(), sFileName2.c_str(),
                                       MOVEFILE_REPLACE_EXISTING );
            if ( !fSuccess )
                MoveFileEx( sFileName1.c_str(), sFileName2.c_str(), MOVEFILE_REPLACE_EXISTING );
        }
        else
        {
            MoveFileEx( sTempFileName.c_str(), sFileName1.c_str(), MOVEFILE_REPLACE_EXISTING  );
        }
    }
    else
    {
        //It could be that there is no original file and therefore copying the original to a temp
        // file failed. Examine if there is no original and if so then move file2 to file1
        WIN32_FIND_DATA data;
        HANDLE hdl = FindFirstFile(sFileName1.c_str(), &data);
        if (hdl == INVALID_HANDLE_VALUE)
            fSuccess = MoveFileEx( sFileName2.c_str(), sFileName1.c_str(),
                                   MOVEFILE_REPLACE_EXISTING );
        else
            FindClose(hdl);
    }


    return fSuccess;
}

static std::_tstring strip( const std::_tstring& s, _TCHAR c )
{
    std::_tstring   result = s;

    std::_tstring::size_type f;

    do
    {
        f = result.find( c );
        if ( f != std::_tstring::npos )
            result.erase( f, 1 );
    } while ( f != std::_tstring::npos );

    return result;
}

extern "C" UINT __stdcall InstallPatchedFiles( MSIHANDLE handle )
{
    std::_tstring   sInstDir = GetMsiProperty( handle, TEXT("INSTALLLOCATION") );
    std::_tstring   sProgramDir = sInstDir + TEXT("program\\");
    std::_tstring   sPatchFile = sProgramDir + TEXT("patchlist.txt");

    TCHAR   szKeyNames[32767];

    // std::_tstring    mystr;
    // mystr = "Patchfile: " + sPatchFile;
    // MessageBox( NULL, mystr.c_str(), "Titel", MB_OK );

    if ( GetPrivateProfileString( TEXT("SwapFiles"), NULL, TEXT(""), szKeyNames, elementsof(szKeyNames), sPatchFile.c_str() ) )
    {
        TCHAR   *pKeyName = szKeyNames;

        while ( *pKeyName )
        {
            TCHAR   szValue[4096];

            if ( GetPrivateProfileString( TEXT("SwapFiles"), pKeyName, TEXT(""), szValue, elementsof(szValue), sPatchFile.c_str() ) )
            {
                std::_tstring   sFileName1 = pKeyName;
                std::_tstring   sFileName2 = szValue;

                sFileName1 = strip( sFileName1, '\"' );
                sFileName2 = strip( sFileName2, '\"' );

                sFileName1 = sInstDir + sFileName1;
                sFileName2 = sInstDir + sFileName2;

                // mystr = "Convert: " + sFileName1 + " to " + sFileName2;
                // MessageBox( NULL, mystr.c_str(), "Titel", MB_OK );

                SwapFiles( sFileName1, sFileName2 );
            }

            pKeyName += _tcslen(pKeyName) + 1;
        }
    }

    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall UninstallPatchedFiles( MSIHANDLE handle )
{
    TCHAR   szValue[8192];
    DWORD   nValueSize = sizeof(szValue);
    HKEY    hKey;

    std::_tstring   sInstDir;

    std::_tstring   sProductKey = GetMsiProperty( handle, TEXT("FINDPRODUCT") );

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
        return ERROR_SUCCESS;

    std::_tstring   sProgramDir = sInstDir + TEXT("program\\");
    std::_tstring   sPatchFile = sProgramDir + TEXT("patchlist.txt");

    TCHAR   szKeyNames[32767];

    // std::_tstring    mystr;
    // mystr = "Patchfile: " + sPatchFile;
    // MessageBox( NULL, mystr.c_str(), "Titel", MB_OK );

    if ( GetPrivateProfileString( TEXT("SwapFiles"), NULL, TEXT(""), szKeyNames, elementsof(szKeyNames), sPatchFile.c_str() ) )
    {
        TCHAR   *pKeyName = szKeyNames;

        while ( *pKeyName )
        {
            TCHAR   szValue[4096];

            if ( GetPrivateProfileString( TEXT("SwapFiles"), pKeyName, TEXT(""), szValue, elementsof(szValue), sPatchFile.c_str() ) )
            {
                std::_tstring   sFileName1 = pKeyName;
                std::_tstring   sFileName2 = szValue;

                sFileName1 = strip( sFileName1, '\"' );
                sFileName2 = strip( sFileName2, '\"' );

                sFileName1 = sInstDir + sFileName1;
                sFileName2 = sInstDir + sFileName2;

                // mystr = "Convert: " + sFileName1 + " to " + sFileName2;
                // MessageBox( NULL, mystr.c_str(), "Titel", MB_OK );

                SwapFiles( sFileName2, sFileName1 );
            }

            pKeyName += _tcslen(pKeyName) + 1;
        }
    }

    return ERROR_SUCCESS;
}

