/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swappatchfiles.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-23 16:22:00 $
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



#define WININIT_FILENAME    "wininit.ini"
#define RENAME_SECTION      "rename"

static BOOL MoveFileEx9x( LPCSTR lpExistingFileNameA, LPCSTR lpNewFileNameA, DWORD dwFlags )
{
    BOOL    fSuccess = FALSE;   // assume failure

    // Windows 9x has a special mechanism to move files after reboot

    if ( dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT )
    {
        CHAR    szExistingFileNameA[MAX_PATH];
        CHAR    szNewFileNameA[MAX_PATH] = "NUL";

        // Path names in WININIT.INI must be in short path name form

        if (
            GetShortPathNameA( lpExistingFileNameA, szExistingFileNameA, MAX_PATH ) &&
            (!lpNewFileNameA || GetShortPathNameA( lpNewFileNameA, szNewFileNameA, MAX_PATH ))
            )
        {
            CHAR    szBuffer[32767];    // The buffer size must not exceed 32K
            DWORD   dwBufLen = GetPrivateProfileSectionA( RENAME_SECTION, szBuffer, elementsof(szBuffer), WININIT_FILENAME );

            CHAR    szRename[MAX_PATH]; // This is enough for at most to times 67 chracters
            strcpy( szRename, szNewFileNameA );
            strcat( szRename, "=" );
            strcat( szRename, szExistingFileNameA );
            size_t  lnRename = strlen(szRename);

            if ( dwBufLen + lnRename + 2 <= elementsof(szBuffer) )
            {
                CopyMemory( &szBuffer[dwBufLen], szRename, lnRename );
                szBuffer[dwBufLen + lnRename ] = 0;
                szBuffer[dwBufLen + lnRename + 1 ] = 0;

                fSuccess = WritePrivateProfileSectionA( RENAME_SECTION, szBuffer, WININIT_FILENAME );
            }
            else
                SetLastError( ERROR_BUFFER_OVERFLOW );
        }
    }
    else
    {

        fSuccess = MoveFileA( lpExistingFileNameA, lpNewFileNameA );

        if ( !fSuccess && 0 != (dwFlags & (MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) )
        {
            BOOL    bFailIfExist = 0 == (dwFlags & MOVEFILE_REPLACE_EXISTING);

            fSuccess = CopyFileA( lpExistingFileNameA, lpNewFileNameA, bFailIfExist );

            // In case of successfull copy do not return FALSE if delete fails.
            // Error detection is done by GetLastError()

            if ( fSuccess )
            {
                SetLastError( NO_ERROR );
                DeleteFileA( lpExistingFileNameA );
            }
        }

    }

    return fSuccess;
}

static BOOL MoveFileExImpl( LPCSTR lpExistingFileNameA, LPCSTR lpNewFileNameA, DWORD dwFlags )
{
    if ( 0 > ((LONG)GetVersion())) // High order bit indicates Win 9x
        return MoveFileEx9x( lpExistingFileNameA, lpNewFileNameA, dwFlags );
    else
        return MoveFileExA( lpExistingFileNameA, lpNewFileNameA, dwFlags );
}


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
    fSuccess = MoveFileExImpl( sFileName1.c_str(), sTempFileName.c_str(),
                           MOVEFILE_REPLACE_EXISTING  );

    std::_tstring   mystr;

    if ( fSuccess )
    {
        fSuccess = MoveFileExImpl( sFileName2.c_str(), sFileName1.c_str(), MOVEFILE_REPLACE_EXISTING );

        if ( fSuccess )
        {
            fSuccess = MoveFileExImpl( sTempFileName.c_str(), sFileName2.c_str(),
                                       MOVEFILE_REPLACE_EXISTING );
            if ( !fSuccess )
            {
                MoveFileExImpl( sFileName1.c_str(), sFileName2.c_str(), MOVEFILE_REPLACE_EXISTING );
            }
        }
        else
        {
            MoveFileExImpl( sTempFileName.c_str(), sFileName1.c_str(), MOVEFILE_REPLACE_EXISTING  );
        }
    }
    else
    {
        //It could be that there is no original file and therefore copying the original to a temp
        // file failed. Examine if there is no original and if so then move file2 to file1

        WIN32_FIND_DATA data;
        HANDLE hdl = FindFirstFile(sFileName1.c_str(), &data);
        if (hdl == INVALID_HANDLE_VALUE)
        {
            fSuccess = MoveFileExImpl( sFileName2.c_str(), sFileName1.c_str(), MOVEFILE_REPLACE_EXISTING );

            // if ( fSuccess )
            // {
            //  mystr = "Success";
            //  MessageBox( NULL, mystr.c_str(), "Titel", MB_OK );
            // }
            // else
            // {
            //  char buff[256];
            //  wsprintf(buff, "Failure %d", GetLastError());
            //  MessageBox( NULL, buff, "Titel", MB_OK );
            // }
        }
        else
        {
            FindClose(hdl);
        }
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

    TCHAR   szSectionNames[32767];
    TCHAR   szKeyNames[32767];

    // std::_tstring    mystr;
    // mystr = "Patchfile: " + sPatchFile;
    // MessageBox( NULL, mystr.c_str(), "Titel", MB_OK );

    if ( GetPrivateProfileString( NULL, NULL, TEXT(""), szSectionNames, elementsof(szSectionNames), sPatchFile.c_str() ) )
    {
        TCHAR   *pSectionName = szSectionNames;

        while ( *pSectionName )
        {
            std::_tstring   sSectionName = pSectionName;
            // mystr = "Section: " + sSectionName;
            // MessageBox( NULL, mystr.c_str(), "Titel", MB_OK );

            if ( GetPrivateProfileString( pSectionName, NULL, TEXT(""), szKeyNames, elementsof(szKeyNames), sPatchFile.c_str() ) )
            {
                TCHAR   *pKeyName = szKeyNames;

                while ( *pKeyName )
                {
                    TCHAR   szValue[4096];

                    if ( GetPrivateProfileString( pSectionName, pKeyName, TEXT(""), szValue, elementsof(szValue), sPatchFile.c_str() ) )
                    {
                        std::_tstring   sFileName1 = pKeyName;
                        std::_tstring   sExtension = szValue;
                        std::_tstring   sFileName2;

                        sFileName1 = strip( sFileName1, '\"' );
                        sExtension = strip( sExtension, '\"' );

                        sFileName1 = sInstDir + sFileName1;
                        sFileName2 = sFileName1 + sExtension;

                        // mystr = "Convert: " + sFileName1 + " to " + sFileName2;
                        // MessageBox( NULL, mystr.c_str(), "Titel", MB_OK );

                        SwapFiles( sFileName1, sFileName2 );
                    }

                    pKeyName += _tcslen(pKeyName) + 1;
                }
            }

            pSectionName += _tcslen(pSectionName) + 1;
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

    TCHAR   szSectionNames[32767];
    TCHAR   szKeyNames[32767];

    // std::_tstring    mystr;
    // mystr = "Patchfile: " + sPatchFile;
    // MessageBox( NULL, mystr.c_str(), "Titel", MB_OK );

    if ( GetPrivateProfileString( NULL, NULL, TEXT(""), szSectionNames, elementsof(szSectionNames), sPatchFile.c_str() ) )
    {
        TCHAR   *pSectionName = szSectionNames;

        while ( *pSectionName )
        {
            std::_tstring   sSectionName = pSectionName;
            // mystr = "Section: " + sSectionName;
            // MessageBox( NULL, mystr.c_str(), "Titel", MB_OK );

            if ( GetPrivateProfileString( pSectionName, NULL, TEXT(""), szKeyNames, elementsof(szKeyNames), sPatchFile.c_str() ) )
            {
                TCHAR   *pKeyName = szKeyNames;

                while ( *pKeyName )
                {
                    TCHAR   szValue[4096];

                    if ( GetPrivateProfileString( pSectionName, pKeyName, TEXT(""), szValue, elementsof(szValue), sPatchFile.c_str() ) )
                    {
                        std::_tstring   sFileName1 = pKeyName;
                        std::_tstring   sExtension = szValue;
                        std::_tstring   sFileName2;

                        sFileName1 = strip( sFileName1, '\"' );
                        sExtension = strip( sExtension, '\"' );

                        sFileName1 = sInstDir + sFileName1;
                        sFileName2 = sFileName1 + sExtension;

                        // mystr = "Convert: " + sFileName1 + " to " + sFileName2;
                        // MessageBox( NULL, mystr.c_str(), "Titel", MB_OK );

                        SwapFiles( sFileName2, sFileName1 );
                    }

                    pKeyName += _tcslen(pKeyName) + 1;
                }
            }

            pSectionName += _tcslen(pSectionName) + 1;
        }
    }

    return ERROR_SUCCESS;
}

