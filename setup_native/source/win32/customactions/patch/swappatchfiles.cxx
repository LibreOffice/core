/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <sal/macros.h>

#include <systools/win32/uwinapi.h>
#include <../tools/seterror.hxx>

#define WININIT_FILENAME    "wininit.ini"
#define RENAME_SECTION      "rename"

#ifdef DEBUG
inline void OutputDebugStringFormat( LPCTSTR pFormat, ... )
{
    _TCHAR  buffer[1024];
    va_list args;

    va_start( args, pFormat );
    _vsntprintf( buffer, SAL_N_ELEMENTS(buffer), pFormat, args );
    OutputDebugString( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCTSTR, ... )
{
}
#endif

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
            DWORD   dwBufLen = GetPrivateProfileSectionA( RENAME_SECTION, szBuffer, SAL_N_ELEMENTS(szBuffer), WININIT_FILENAME );

            CHAR    szRename[MAX_PATH]; // This is enough for at most to times 67 chracters
            strcpy( szRename, szNewFileNameA );
            strcat( szRename, "=" );
            strcat( szRename, szExistingFileNameA );
            size_t  lnRename = strlen(szRename);

            if ( dwBufLen + lnRename + 2 <= SAL_N_ELEMENTS(szBuffer) )
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
        }
        else
        {
            FindClose(hdl);
        }
    }

    OutputDebugStringFormat( TEXT("%s <-> %s: %s"), sFileName1.c_str(), sFileName2.c_str(), fSuccess ? TEXT("OK") : TEXT("FAILED") );

    if (!fSuccess )
    {
        DWORD   dwError = GetLastError();
        LPVOID lpMsgBuf;
        if ( FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPTSTR) &lpMsgBuf,
            0,
            NULL ))
        {
            OutputDebugStringFormat( TEXT("Error Code %d: %s"), dwError, lpMsgBuf );
            LocalFree( lpMsgBuf );
        }
        else
            OutputDebugStringFormat( TEXT("Error Code %d: Unknown"), dwError );
        SetMsiErrorCode( dwError );
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

static std::_tstring trim( const std::_tstring& rString )
{
    std::_tstring temp = rString;

    while ( temp.length() && temp[0] == ' ' || temp[0] == '\t' )
        temp.erase( 0, 1 );

    std::_tstring::size_type    len = temp.length();

    while ( len && temp[len-1] == ' ' || temp[len-1] == '\t' )
    {
        temp.erase( len - 1, 1 );
        len = temp.length();
    }

    return temp;
}

static bool readLine( FILE *fp, std::_tstring& rLine )
{
    _TCHAR szBuffer[1024];
    bool    bSuccess = false;
    bool    bEOL = false;
    std::_tstring   line;


    while ( !bEOL && _fgetts( szBuffer, sizeof(szBuffer), fp ) )
    {
        int len = _tcslen(szBuffer);

        bSuccess = true;

        while ( len && szBuffer[len - 1] == '\n' )
        {
            szBuffer[--len] = 0;
            bEOL = true;
        }

        line.append( szBuffer );
    }

    rLine = line;
    return bSuccess;
}


static std::_tstring getProfileString(
    const std::_tstring& aFileName,
    const std::_tstring& aSectionName,
    const std::_tstring& aKeyName,
    const std::_tstring& aDefault = _T("") )
{
    FILE    *fp = _tfopen( aFileName.c_str(), _T("r") );
    std::_tstring   retValue = aDefault.length() ? aDefault : _T("");

    if ( fp )
    {
        std::_tstring line;
        std::_tstring section;

        while ( readLine( fp, line ) )
        {
            line = trim( line );

            if ( line.length() && line[0] == '[' )
            {
                line.erase( 0, 1 );
                std::_tstring::size_type end = line.find( ']', 0 );

                if ( std::_tstring::npos != end )
                    section = trim( line.substr( 0, end ) );
            }
            else
            {

                std::_tstring::size_type iEqualSign = line.find( '=', 0 );

                if ( iEqualSign != std::_tstring::npos )
                {
                    std::_tstring   keyname = line.substr( 0, iEqualSign );
                    keyname = trim( keyname );

                    std::_tstring   value = line.substr( iEqualSign + 1 /*, std::_tstring::npos */ );
                    value = trim( value );

                    if (
                        0 == _tcsicmp( section.c_str(), aSectionName.c_str() ) &&
                        0 == _tcsicmp( keyname.c_str(), aKeyName.c_str() )
                         )
                    {
                        retValue = value;
                        break;
                    }
                }
            }
        }

        fclose( fp );
    }

    return retValue;
}

static std::queue< std::_tstring > getProfileSections( const std::_tstring& aFileName )
{
    FILE    *fp = _tfopen( aFileName.c_str(), _T("r") );
    std::queue< std::_tstring > aResult;

    OutputDebugStringFormat( TEXT("*** Retrieving Section Names ****") );

    if ( fp )
    {
        std::_tstring line;
        std::_tstring section;

        while ( readLine( fp, line ) )
        {
            line = trim( line );

            if ( line.length() && line[0] == '[' )
            {
                line.erase( 0, 1 );
                std::_tstring::size_type end = line.find( ']', 0 );

                if ( std::_tstring::npos != end )
                    section = trim( line.substr( 0, end ) );

                aResult.push( section );

                OutputDebugStringFormat( TEXT("Section: %s"), section.c_str() );

            }
        }

        fclose( fp );
    }

    OutputDebugStringFormat( TEXT("*** Done Section Names ***") );

    return aResult;
}

static std::queue< std::_tstring > getProfileKeys( const std::_tstring& aFileName, const std::_tstring& aSectionName )
{
    FILE    *fp = _tfopen( aFileName.c_str(), _T("r") );
    std::queue< std::_tstring > aResult;

    OutputDebugStringFormat( TEXT("*** Retrieving Key Names for [%s] ***"), aSectionName.c_str() );

    if ( fp )
    {
        std::_tstring line;
        std::_tstring section;

        while ( readLine( fp, line ) )
        {
            line = trim( line );

            if ( line.length() && line[0] == '[' )
            {
                line.erase( 0, 1 );
                std::_tstring::size_type end = line.find( ']', 0 );

                if ( std::_tstring::npos != end )
                    section = trim( line.substr( 0, end ) );
            }
            else
            {

                std::_tstring::size_type iEqualSign = line.find( '=', 0 );

                if ( iEqualSign != std::_tstring::npos )
                {
                    std::_tstring   keyname = line.substr( 0, iEqualSign );
                    keyname = trim( keyname );

                    if ( 0 == _tcsicmp( section.c_str(), aSectionName.c_str() ) )
                    {
                        aResult.push( keyname );

                        OutputDebugStringFormat( keyname.c_str() );

                    }
                }
            }
        }

        fclose( fp );
    }

    OutputDebugStringFormat( TEXT("*** Done Key Names for [%s] ***"), aSectionName.c_str() );

    return aResult;
}

extern "C" UINT __stdcall InstallPatchedFiles( MSIHANDLE handle )
{
    std::_tstring   sInstDir = GetMsiProperty( handle, TEXT("INSTALLLOCATION") );
    std::_tstring   sProgramDir = sInstDir + TEXT("Basis\\program\\");
    std::_tstring   sPatchFile = sProgramDir + TEXT("patchlist.txt");

    std::queue< std::_tstring > aSectionNames;
    std::queue< std::_tstring > aKeyNames;

    OutputDebugStringA( "Starting Custom Action" );

    aSectionNames = getProfileSections( sPatchFile );
    while ( !aSectionNames.empty() )
    {
        std::_tstring   sSectionName = aSectionNames.front();
        if ( std::_tstring(TEXT("_root")) == sSectionName ) { sSectionName = TEXT(""); }

        aKeyNames = getProfileKeys( sPatchFile, sSectionName );
        while ( !aKeyNames.empty() )
        {
            std::_tstring   sKeyName = aKeyNames.front();
            std::_tstring   sValue = getProfileString( sPatchFile, sSectionName, sKeyName );

            if ( sValue.length() )
            {
                std::_tstring   sFileName1 = sKeyName;
                std::_tstring   sExtension = sValue;
                std::_tstring   sFileName2;

                sFileName1 = strip( sFileName1, '\"' );
                sExtension = strip( sExtension, '\"' );

                sFileName1 = sInstDir + sSectionName + sFileName1;
                sFileName2 = sFileName1 + sExtension;

                SwapFiles( sFileName1, sFileName2 );
            }

            aKeyNames.pop();
        }

        aSectionNames.pop();
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

    std::_tstring   sProgramDir = sInstDir + TEXT("Basis\\program\\");
    std::_tstring   sPatchFile = sProgramDir + TEXT("patchlist.txt");

    std::queue< std::_tstring > aSectionNames;
    std::queue< std::_tstring > aKeyNames;

    aSectionNames = getProfileSections( sPatchFile );
    while ( !aSectionNames.empty() )
    {
        std::_tstring   sSectionName = aSectionNames.front();
        if ( std::_tstring(TEXT("_root")) == sSectionName ) { sSectionName = TEXT(""); }

        aKeyNames = getProfileKeys( sPatchFile, sSectionName );
        while( !aKeyNames.empty() )
        {
            std::_tstring   sKeyName = aKeyNames.front();
            std::_tstring   sValue = getProfileString( sPatchFile, sSectionName, sKeyName );

            if ( sValue.length() )
            {
                std::_tstring   sFileName1 = sKeyName;
                std::_tstring   sExtension = sValue;
                std::_tstring   sFileName2;

                sFileName1 = strip( sFileName1, '\"' );
                sExtension = strip( sExtension, '\"' );

                sFileName1 = sInstDir + sSectionName + sFileName1;
                sFileName2 = sFileName1 + sExtension;

                SwapFiles( sFileName2, sFileName1 );
            }

            aKeyNames.pop();
        }

        aSectionNames.pop();
    }

    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall IsOfficeRunning( MSIHANDLE handle )
{
    std::_tstring   sInstDir = GetMsiProperty( handle, TEXT("INSTALLLOCATION") );
    std::_tstring   sResourceDir = sInstDir + TEXT("Basis\\program\\resource\\");
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
        {
            MsiSetProperty(handle, TEXT("OFFICERUNS"), TEXT("1"));
            SetMsiErrorCode( MSI_ERROR_OFFICE_IS_RUNNING );
        }

        FindClose( hFind );
    }


    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall SetFeatureState( MSIHANDLE handle )
{
    std::_tstring   mystr;

    // 1. Reading Product Code from setup.ini of installed Office

    std::_tstring sInstallPath = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));
    std::_tstring sSetupiniPath = sInstallPath + TEXT("program\\setup.ini");

    TCHAR szProductCode[32767];

    GetPrivateProfileString(
        TEXT("Bootstrap"),
        TEXT("ProductCode"),
        TEXT("NOTFOUND"),
        szProductCode,
        SAL_N_ELEMENTS(szProductCode),
        sSetupiniPath.c_str()
        );

    if ( !_tcsicmp( szProductCode, TEXT("NOTFOUND") ) )
    {
        // No setup.ini or no "ProductCode" in setup.ini. This is an invalid directory.
        return ERROR_SUCCESS;
    }

    // 2. Converting Product code

    std::_tstring productCode = TEXT(szProductCode);
    productCode = ConvertGuid(std::_tstring(productCode.c_str() + 1, productCode.length() - 2));
    mystr = TEXT("Changed product code: ") + productCode;

    // 3. Setting path in the Windows registry to find installed features

    std::_tstring registryKey;
    HKEY registryRoot;

    if ( IsSetMsiProperty(handle, TEXT("ALLUSERS")) )
    {
        registryRoot = HKEY_LOCAL_MACHINE;
        registryKey = TEXT("Software\\Classes\\Installer\\Features\\") + productCode;
        mystr = registryKey;
    }
    else
    {
        registryRoot = HKEY_CURRENT_USER;
        registryKey = TEXT("Software\\Microsoft\\Installer\\Features\\") + productCode;
        mystr = registryKey;
    }

    // 4. Collecting all installed features from Windows registry

    HKEY hKey;
    if (RegOpenKey(registryRoot, registryKey.c_str(), &hKey) == ERROR_SUCCESS)
    {
        int counter = 0;
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

                // Does this feature exist in this patch?
                if ( IsSetMsiProperty(handle, sValueName) )
                {
                    // Feature is not installed, if szValueData starts with a "square" (ascii 6)
                    if ( 6 == szValueData[0] )
                    {
                        MsiSetFeatureState(handle,sValueName.c_str(),INSTALLSTATE_ABSENT); // do not install this feature
                    }
                    else
                    {
                        MsiSetFeatureState(handle,sValueName.c_str(),INSTALLSTATE_LOCAL); // do install this feature
                    }
                }
            }

            counter = counter + 1;

        } while ( ERROR_SUCCESS == lEnumResult );

        RegCloseKey( hKey );
    }

    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall SetNewFeatureState( MSIHANDLE handle )
{
    std::_tstring mystr;
    std::_tstring sValueName;

    sValueName = TEXT("gm_o_Onlineupdate");

    if (IsSetMsiProperty(handle, TEXT("SELECT_OU_FEATURE")))
    {
        MsiSetFeatureState(handle,sValueName.c_str(),INSTALLSTATE_LOCAL); // do install this feature
    }
    else
    {
        MsiSetFeatureState(handle,sValueName.c_str(),INSTALLSTATE_ABSENT); // do not install this feature
    }

    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall ShowOnlineUpdateDialog( MSIHANDLE handle )
{
    // Checking existence of file "updchk.uno.dll", which shows, that
    // Online Update functionality is always available. Then the dialog
    // that offers the Online Update is superfluous.

    std::_tstring sInstDir = GetMsiProperty( handle, TEXT("INSTALLLOCATION") );
    std::_tstring sProgramDir = sInstDir + TEXT("Basis\\program\\");
    std::_tstring sSearchFile = sProgramDir + TEXT("updchk.uno.dll");

    WIN32_FIND_DATA data;
    HANDLE hdl = FindFirstFile(sSearchFile.c_str(), &data);
    if (hdl != INVALID_HANDLE_VALUE)  // the file exists
    {
        // And finally setting property SHOW_ONLINEUPDATE_DIALOG
        // to hide this dialog
        UnsetMsiProperty(handle, TEXT("SHOW_ONLINEUPDATE_DIALOG"));

        // Setting SELECT_OU_FEATURE to 1, which is probably superfluous
        // because this is already the default value. But only this
        // guarantees, that CustomAction SetNewFeatureState always sets
        // the correct FeatureState for "gm_o_Onlineupdate", if it is
        // already installed.
        SetMsiProperty(handle, TEXT("SELECT_OU_FEATURE"));
    }
    else
    {
        // If the file does not exist, the Online Update dialog
        // has to be shown.
        SetMsiProperty(handle, TEXT("SHOW_ONLINEUPDATE_DIALOG"));
        FindClose(hdl);
    }

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
