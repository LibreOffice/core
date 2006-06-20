/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swappatchfiles.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:36:43 $
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
#include <stdio.h>

#ifdef _WIN32_WINNT
#error YES
#endif

#include <systools/win32/uwinapi.h>

#define WININIT_FILENAME    "wininit.ini"
#define RENAME_SECTION      "rename"


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

// The provided GUID must be without surounding '{}'
static std::_tstring GetGuidPart(const std::_tstring& guid, int index)
{
    assert((guid.length() == 36) && "No GUID or wrong format!");
    assert(((index > -1) && (index < 5)) && "Out of range!");

    if (index == 0) return std::_tstring(guid.c_str(), 8);
    if (index == 1) return std::_tstring(guid.c_str() + 9, 4);
    if (index == 2) return std::_tstring(guid.c_str() + 14, 4);
    if (index == 3) return std::_tstring(guid.c_str() + 19, 4);
    if (index == 4) return std::_tstring(guid.c_str() + 24, 12);

    return std::_tstring();
}

static void Swap(char* p1, char* p2)
{
    char tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
}

static std::_tstring Invert(const std::_tstring& str)
{
    char* buff = reinterpret_cast<char*>(_alloca(str.length()));
    strncpy(buff, str.c_str(), str.length());

    char* front = buff;
    char* back = buff + str.length() - 1;

    while (front < back)
        Swap(front++, back--);

    return std::_tstring(buff, str.length());
}

// Convert the upgrade code (which is a GUID) according
// to the way the windows installer does when writing it
// to the registry
// The first 8 bytes will be inverted, from the the last
// 8 bytes always the nibbles will be inverted for further
// details look in the MSDN under compressed registry keys
static std::_tstring ConvertGuid(const std::_tstring& guid)
{
    std::_tstring convertedGuid;

    std::_tstring part = GetGuidPart(guid, 0);
    convertedGuid = Invert(part);

    part = GetGuidPart(guid, 1);
    convertedGuid += Invert(part);

    part = GetGuidPart(guid, 2);
    convertedGuid += Invert(part);

    part = GetGuidPart(guid, 3);
    convertedGuid += Invert(std::_tstring(part.c_str(), 2));
    convertedGuid += Invert(std::_tstring(part.c_str() + 2, 2));

    part = GetGuidPart(guid, 4);
    int pos = 0;
    for (int i = 0; i < 6; i++)
    {
        convertedGuid += Invert(std::_tstring(part.c_str() + pos, 2));
        pos += 2;
    }
    return convertedGuid;
}

static inline bool IsSetMsiProperty(MSIHANDLE handle, const std::_tstring& sProperty)
{
    std::_tstring value = GetMsiProperty(handle, sProperty);
    return (value.length() > 0);
}

static inline void UnsetMsiProperty(MSIHANDLE handle, const std::_tstring& sProperty)
{
    MsiSetProperty(handle, sProperty.c_str(), NULL);
}

static inline void SetMsiProperty(MSIHANDLE handle, const std::_tstring& sProperty)
{
    MsiSetProperty(handle, sProperty.c_str(), TEXT("1"));
}

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

        if ( !fSuccess && GetLastError() != ERROR_ACCESS_DENIED &&
            0 != (dwFlags & (MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) )
        {
            BOOL    bFailIfExist = 0 == (dwFlags & MOVEFILE_REPLACE_EXISTING);

            fSuccess = CopyFileA( lpExistingFileNameA, lpNewFileNameA, bFailIfExist );

            if ( fSuccess )
                fSuccess = DeleteFileA( lpExistingFileNameA );
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

static bool SwapFiles( const std::_tstring& sFileName1, const std::_tstring& sFileName2 )
{
    std::_tstring   sTempFileName = sFileName1 + TEXT(".tmp");

    bool fSuccess = true;

    //Try to move the original file to a temp file
    fSuccess = MoveFileExImpl( sFileName1.c_str(), sTempFileName.c_str(), MOVEFILE_REPLACE_EXISTING);

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
            if ( std::_tstring(TEXT("_root")) == sSectionName ) { sSectionName = TEXT(""); }
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

                        sFileName1 = sInstDir + sSectionName + sFileName1;
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
            if ( std::_tstring(TEXT("_root")) == sSectionName ) { sSectionName = TEXT(""); }
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

                        sFileName1 = sInstDir + sSectionName + sFileName1;
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

extern "C" UINT __stdcall IsOfficeRunning( MSIHANDLE handle )
{
    std::_tstring   sInstDir = GetMsiProperty( handle, TEXT("INSTALLLOCATION") );
    std::_tstring   sResourceDir = sInstDir + TEXT("program\\resource\\");
    std::_tstring   sPattern = sResourceDir + TEXT("vcl*.res");

    WIN32_FIND_DATA aFindFileData;
    HANDLE  hFind = FindFirstFile( sPattern.c_str(), &aFindFileData );

    if ( IsValidHandle(hFind) )
    {
        BOOL    fSuccess = false;
        bool    fRenameSucceeded;

        do
        {
            std::_tstring   sResourceFile = sResourceDir + aFindFileData.cFileName;
            std::_tstring   sIntermediate = sResourceFile + TEXT(".tmp");

            fRenameSucceeded = MoveFileExImpl( sResourceFile.c_str(), sIntermediate.c_str(), MOVEFILE_REPLACE_EXISTING );
            if ( fRenameSucceeded )
            {
                MoveFileExImpl( sIntermediate.c_str(), sResourceFile.c_str(), 0 );
                fSuccess = FindNextFile( hFind, &aFindFileData );
            }
        } while ( fSuccess && fRenameSucceeded );

        if ( !fRenameSucceeded )
            MsiSetProperty(handle, TEXT("OFFICERUNS"), TEXT("1"));

        FindClose( hFind );
    }


    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall SetFeatureState( MSIHANDLE handle )
{
    std::_tstring   mystr;

    // 1. Reading Product Code from setup.ini of installed Office

    std::_tstring sInstallPath = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));
    // MessageBox(NULL, sInstallPath.c_str(), "INSTALLLOCATION", MB_OK);
    std::_tstring sSetupiniPath = sInstallPath + TEXT("program\\setup.ini");

    TCHAR szProductCode[32767];

    GetPrivateProfileString(
        TEXT("Bootstrap"),
        TEXT("ProductCode"),
        TEXT("NOTFOUND"),
        szProductCode,
        elementsof(szProductCode),
        sSetupiniPath.c_str()
        );

    if ( !_tcsicmp( szProductCode, TEXT("NOTFOUND") ) )
    {
        // No setup.ini or no "ProductCode" in setup.ini. This is an invalid directory.
        // MessageBox(NULL, "NOTFOUND set", "DEBUG", MB_OK);
        return ERROR_SUCCESS;
    }

    // 2. Converting Product code

    std::_tstring productCode = TEXT(szProductCode);
    productCode = ConvertGuid(std::_tstring(productCode.c_str() + 1, productCode.length() - 2));
    mystr = TEXT("Changed product code: ") + productCode;
    // MessageBox(NULL, mystr.c_str(), "ProductCode", MB_OK);

    // 3. Setting path in the Windows registry to find installed features

    std::_tstring registryKey;
    HKEY registryRoot;

    if ( IsSetMsiProperty(handle, TEXT("ALLUSERS")) )
    {
        registryRoot = HKEY_LOCAL_MACHINE;
        registryKey = TEXT("Software\\Classes\\Installer\\Features\\") + productCode;
        mystr = registryKey;
        // MessageBox( NULL, mystr.c_str(), "ALLUSERS", MB_OK );
    }
    else
    {
        registryRoot = HKEY_CURRENT_USER;
        registryKey = TEXT("Software\\Microsoft\\Installer\\Features\\") + productCode;
        mystr = registryKey;
        // MessageBox( NULL, mystr.c_str(), "ALLUSERS", MB_OK );
    }

    // 4. Collecting all installed features from Windows registry

    HKEY hKey;
    if (RegOpenKey(registryRoot, registryKey.c_str(), &hKey) == ERROR_SUCCESS)
    {
        int counter = 0;
        // DWORD counter = 0;
        LONG lEnumResult;

        do
        {
            TCHAR szValueName[8192];
            DWORD nValueNameSize = sizeof(szValueName);
            LPDWORD pValueNameSize = &nValueNameSize;
            TCHAR szValueData[8192];
            DWORD nValueDataSize = sizeof(szValueData);

            lEnumResult = RegEnumValue( hKey, counter, szValueName, pValueNameSize, NULL, NULL, (LPBYTE)szValueData, &nValueDataSize);

            if ( ERROR_SUCCESS == lEnumResult )
            {
                std::_tstring sValueName = szValueName;
                std::_tstring sValueData = szValueData;

                // mystr = sValueName;
                // MessageBox( NULL, mystr.c_str(), "ValueName", MB_OK );
                // mystr = sValueData;
                // MessageBox( NULL, mystr.c_str(), "ValueData", MB_OK );

                // Does this feature exist in this patch?
                if ( IsSetMsiProperty(handle, sValueName) )
                {
                    // Feature is not installed, if szValueData starts with a "square" (ascii 6)
                    if ( 6 == szValueData[0] )
                    {
                        MsiSetFeatureState(handle,sValueName.c_str(),INSTALLSTATE_ABSENT); // do not install this feature
                        // mystr = TEXT("Do NOT install: ") + sValueName;
                        // MessageBox( NULL, mystr.c_str(), "ValueName", MB_OK );
                    }
                    else
                    {
                        MsiSetFeatureState(handle,sValueName.c_str(),INSTALLSTATE_LOCAL); // do install this feature
                        // mystr = TEXT("Do install: ") + sValueName;
                        // MessageBox( NULL, mystr.c_str(), "ValueName", MB_OK );
                    }
                }
            }

            counter = counter + 1;

        } while ( ERROR_SUCCESS == lEnumResult );

        RegCloseKey( hKey );
    }

    return ERROR_SUCCESS;
}
