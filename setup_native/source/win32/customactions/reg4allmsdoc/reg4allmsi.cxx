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
#include <string>
#include <strsafe.h>

static const CHAR* g_Extensions[] =
{
    ".doc",     // Microsoft Word Text [0]
    ".dot",     // Microsoft Word Template
    ".rtf",     // rtf text
    ".docx",    // Office Word 2007 XML document
    ".docm",    // Office Word 2007 XML macro-enabled document
    ".dotx",    // Office Word 2007 XML template
    ".dotm",    // Office Word 2007 XML macro-enabled template
    ".xlw",     // Microsoft Excel
    ".xls",     // Microsoft Excel
    ".xlt",     // Microsoft Excel Template
    ".xlsx",    // Office Excel 2007 XML workbook
    ".xlsm",    // Office Excel 2007 XML macro-enabled workbook
    ".xltx",    // Office Excel 2007 XML template
    ".xltm",    // Office Excel 2007 XML macro-enabled template
    ".xlsb",    // Office Excel 2007 binary workbook (BIFF12)
    ".ppt",     // Microsoft Powerpoint
    ".pps",     // Microsoft Powerpoint
    ".pot",     // Microsoft Powerpoint Template
    ".pptx",    // Office PowerPoint 2007 XML presentation
    ".pptm",    // Office PowerPoint 2007 macro-enabled XML presentation
    ".potx",    // Office PowerPoint 2007 XML template
    ".potm",    // Office PowerPoint 2007 macro-enabled XML template
    ".ppsx",    // Office PowerPoint 2007 XML show
    ".vsd",     // Visio 2000/XP/2003 document
    ".vst",     // Visio 2000/XP/2003 template
    0
};

static const int WORD_START = 0;
static const int EXCEL_START = 7;
static const int POWERPOINT_START = 15;
static const int VISIO_START = 23;
static const int VISIO_END = 25;

//    ".xlam",    // Office Excel 2007 XML macro-enabled add-in
//    ".ppam",    // Office PowerPoint 2007 macro-enabled XML add-in
//    ".ppsm",    // Office PowerPoint 2007 macro-enabled XML show

#ifdef DEBUG
inline void OutputDebugStringFormat( LPCSTR pFormat, ... )
{
    CHAR    buffer[1024];
    va_list args;

    va_start( args, pFormat );
    StringCchVPrintfA( buffer, sizeof(buffer), pFormat, args );
    OutputDebugStringA( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCSTR, ... )
{
}
#endif

static BOOL CheckExtensionInRegistry( LPCSTR lpSubKey )
{
    BOOL    bRet = false;
    HKEY    hKey = NULL;
    LONG    lResult = RegOpenKeyExA( HKEY_CLASSES_ROOT, lpSubKey, 0, KEY_QUERY_VALUE, &hKey );

    if ( ERROR_SUCCESS == lResult )
    {
        CHAR    szBuffer[1024];
        DWORD   nSize = sizeof( szBuffer );

        lResult = RegQueryValueExA( hKey, "", NULL, NULL, (LPBYTE)szBuffer, &nSize );
        if ( ERROR_SUCCESS == lResult && nSize > 0 )
        {
            szBuffer[nSize] = '\0';
            OutputDebugStringFormat( "Found value [%s] for key [%s].\n", szBuffer, lpSubKey );

            if ( strncmp( szBuffer, "WordPad.Document.1", 18 ) == 0 )
            {   // We will replace registration for WordPad (alas, on XP only) FIXME
                bRet = true;
            }
            else if ( strncmp( szBuffer, "LibreOffice.", 12 ) == 0 )
            {   // We will replace registration for our own types, too
                bRet = true;
            }
            else if ( strncmp( szBuffer, "lostub.", 7 ) == 0 )
            {   // We will replace registration for lostub, too
                bRet = true;
            }
            else // we have a default value -> do not register, see fdo#39791
                bRet = false;
        }
        else    // no default value found -> return TRUE to register for that key
            bRet = true;

        RegCloseKey( hKey );
    }
    else // no key found -> return TRUE to register for that key
        bRet = true;

    return bRet;
}

static LONG DeleteSubKeyTree( HKEY RootKey, LPCSTR lpKey )
{
    HKEY hKey;
    LONG rc = RegOpenKeyExA( RootKey, lpKey, 0, KEY_READ | DELETE, &hKey );

    if (ERROR_SUCCESS == rc)
    {
        LPCSTR    lpSubKey;
        DWORD     nMaxSubKeyLen;

        rc = RegQueryInfoKeyA( hKey, 0, 0, 0, 0, &nMaxSubKeyLen, 0, 0, 0, 0, 0, 0 );
        nMaxSubKeyLen++; // space for trailing '\0'
        lpSubKey = reinterpret_cast<CHAR*>( _alloca( nMaxSubKeyLen*sizeof(CHAR) ) );

        while (ERROR_SUCCESS == rc)
        {
            DWORD nLen = nMaxSubKeyLen;
            rc = RegEnumKeyExA( hKey, 0, (LPSTR)lpSubKey, &nLen, 0, 0, 0, 0);    // always index zero

            if ( ERROR_NO_MORE_ITEMS == rc )
            {
                rc = RegDeleteKeyA( RootKey, lpKey );
                if ( rc == ERROR_SUCCESS )
                    OutputDebugStringFormat( "deleted key [%s] from registry.\n", lpKey );
                else
                    OutputDebugStringFormat( "RegDeleteKeyA %s returned %ld.\n", lpKey, rc );
                break;
            }
            else if ( rc == ERROR_SUCCESS )
            {
                rc = DeleteSubKeyTree( hKey, lpSubKey );
                if ( ERROR_SUCCESS != rc )
                    OutputDebugStringFormat( "RegDeleteKeyA %s returned %ld.\n", lpSubKey, rc );
            }

        }
        RegCloseKey(hKey);
    }
    else
    {
        OutputDebugStringFormat( "RegOpenKeyExA %s returned %ld.\n", lpKey, rc );
    }

    return rc;
}

bool GetMsiProp( MSIHANDLE handle, LPCSTR name, /*out*/std::string& value )
{
    DWORD sz = 0;
    LPSTR dummy = "";
    if (MsiGetPropertyA(handle, name, dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++;
        DWORD nbytes = sz * sizeof(TCHAR);
        LPSTR buff = reinterpret_cast<LPSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetPropertyA(handle, name, buff, &sz);
        value = buff;
        return true;
    }
    return false;
}

bool IsSetMsiProp( MSIHANDLE handle, LPCSTR name )
{
    std::string val;
    GetMsiProp( handle, name, val );
    return (val == "1");
}

static void registerForExtension( MSIHANDLE handle, const int nIndex, bool bRegister )
{
    CHAR sPropName[256];
    StringCchCopyA( sPropName, 256, "REGISTER_" );
    StringCchCatA( sPropName, 256, (g_Extensions[nIndex])+1 );
    CharUpperBuffA( sPropName+9, 4 );

    if ( bRegister ) {
        MsiSetPropertyA( handle, sPropName, "1" );
        OutputDebugStringFormat( "Set MSI property %s.\n", sPropName );
    } else {
        MsiSetPropertyA( handle, sPropName, "0" );
        OutputDebugStringFormat( "Unset MSI property %s.\n", sPropName );
    }
}

static void saveOldRegistration( LPCSTR lpSubKey )
{
    BOOL    bRet = false;
    HKEY    hKey = NULL;
    LONG    lResult = RegOpenKeyExA( HKEY_CLASSES_ROOT, lpSubKey, 0,
                                     KEY_QUERY_VALUE|KEY_SET_VALUE, &hKey );

    if ( ERROR_SUCCESS == lResult )
    {
        CHAR    szBuffer[1024];
        DWORD   nSize = sizeof( szBuffer );

        lResult = RegQueryValueExA( hKey, "", NULL, NULL, (LPBYTE)szBuffer, &nSize );
        if ( ERROR_SUCCESS == lResult )
        {
            szBuffer[nSize] = '\0';

            // No need to save assocations for our own types
            if ( strncmp( szBuffer, "LibreOffice.", 12 ) != 0 )
            {
                // Save the old association
                RegSetValueExA( hKey, "LOBackupAssociation", 0,
                                REG_SZ, (LPBYTE)szBuffer, nSize );
                // Also save what the old association means, just so we can try to verify
                // if/when restoring it that the old application still exists
                HKEY hKey2 = NULL;
                lResult = RegOpenKeyExA( HKEY_CLASSES_ROOT, szBuffer, 0,
                                         KEY_QUERY_VALUE, &hKey2 );
                if ( ERROR_SUCCESS == lResult )
                {
                    nSize = sizeof( szBuffer );
                    lResult = RegQueryValueExA( hKey2, "", NULL, NULL, (LPBYTE)szBuffer, &nSize );
                    if ( ERROR_SUCCESS == lResult )
                    {
                        RegSetValueExA( hKey, "LOBackupAssociationDeref", 0,
                                        REG_SZ, (LPBYTE)szBuffer, nSize );
                    }
                    RegCloseKey( hKey2 );
                }
            }
        }
        RegCloseKey( hKey );
    }
}

static void registerForExtensions( MSIHANDLE handle, BOOL bRegisterAll )
{ // Check all file extensions
    int nIndex = 0;
    while ( g_Extensions[nIndex] != 0 )
    {
        saveOldRegistration( g_Extensions[nIndex] );

        BOOL bRegister = bRegisterAll || CheckExtensionInRegistry( g_Extensions[nIndex] );
        if ( bRegister )
            registerForExtension( handle, nIndex, true );
        ++nIndex;
    }
}

static bool checkSomeExtensionInRegistry( const int nStart, const int nEnd )
{ // Check all file extensions
    int nIndex = nStart;
    bool bFound = false;

    while ( !bFound && ( g_Extensions[nIndex] != 0 ) && ( nIndex < nEnd ) )
    {
        bFound = ! CheckExtensionInRegistry( g_Extensions[nIndex] );

        if ( bFound )
            OutputDebugStringFormat( "Found registration for [%s].\n", g_Extensions[nIndex] );

        ++nIndex;
    }
    return bFound;
}

static void registerSomeExtensions( MSIHANDLE handle, const int nStart, const int nEnd, bool bRegister )
{ // Check all file extensions
    int nIndex = nStart;

    while ( ( g_Extensions[nIndex] != 0 ) && ( nIndex < nEnd ) )
    {
        registerForExtension( handle, nIndex++, bRegister );
    }
}

extern "C" UINT __stdcall LookForRegisteredExtensions( MSIHANDLE handle )
{
    OutputDebugStringFormat( "LookForRegisteredExtensions: " );

    INSTALLSTATE current_state;
    INSTALLSTATE future_state;

    bool bWriterEnabled = false;
    bool bCalcEnabled = false;
    bool bImpressEnabled = false;
    bool bDrawEnabled = false;
    bool bRegisterNone = IsSetMsiProp( handle, "REGISTER_NO_MSO_TYPES" );

    if ( ( ERROR_SUCCESS == MsiGetFeatureState( handle, L"gm_p_Wrt", &current_state, &future_state ) ) &&
         ( (future_state == INSTALLSTATE_LOCAL) || ((current_state == INSTALLSTATE_LOCAL) && (future_state == INSTALLSTATE_UNKNOWN) ) ) )
        bWriterEnabled = true;

    OutputDebugStringFormat( "LookForRegisteredExtensions: Install state Writer is [%d], will be [%d]", current_state, future_state );
    if ( bWriterEnabled )
        OutputDebugStringFormat( "LookForRegisteredExtensions: Writer is enabled" );
    else
        OutputDebugStringFormat( "LookForRegisteredExtensions: Writer is NOT enabled" );

    if ( ( ERROR_SUCCESS == MsiGetFeatureState( handle, L"gm_p_Calc", &current_state, &future_state ) ) &&
         ( (future_state == INSTALLSTATE_LOCAL) || ((current_state == INSTALLSTATE_LOCAL) && (future_state == INSTALLSTATE_UNKNOWN) ) ) )
        bCalcEnabled = true;

    OutputDebugStringFormat( "LookForRegisteredExtensions: Install state Calc is [%d], will be [%d]", current_state, future_state );
    if ( bCalcEnabled )
        OutputDebugStringFormat( "LookForRegisteredExtensions: Calc is enabled" );
    else
        OutputDebugStringFormat( "LookForRegisteredExtensions: Calc is NOT enabled" );

    if ( ( ERROR_SUCCESS == MsiGetFeatureState( handle, L"gm_p_Impress", &current_state, &future_state ) ) &&
         ( (future_state == INSTALLSTATE_LOCAL) || ((current_state == INSTALLSTATE_LOCAL) && (future_state == INSTALLSTATE_UNKNOWN) ) ) )
        bImpressEnabled = true;

    OutputDebugStringFormat( "LookForRegisteredExtensions: Install state Impress is [%d], will be [%d]", current_state, future_state );
    if ( bImpressEnabled )
        OutputDebugStringFormat( "LookForRegisteredExtensions: Impress is enabled" );
    else
        OutputDebugStringFormat( "LookForRegisteredExtensions: Impress is NOT enabled" );

    if ( ( ERROR_SUCCESS == MsiGetFeatureState( handle, L"gm_p_Draw", &current_state, &future_state ) ) &&
         ( (future_state == INSTALLSTATE_LOCAL) || ((current_state == INSTALLSTATE_LOCAL) && (future_state == INSTALLSTATE_UNKNOWN) ) ) )
        bDrawEnabled = true;

    OutputDebugStringFormat( "LookForRegisteredExtensions: Install state Draw is [%d], will be [%d]", current_state, future_state );
    if ( bImpressEnabled )
        OutputDebugStringFormat( "LookForRegisteredExtensions: Draw is enabled" );
    else
        OutputDebugStringFormat( "LookForRegisteredExtensions: Draw is NOT enabled" );

    MsiSetPropertyA( handle, "SELECT_WORD", "" );
    MsiSetPropertyA( handle, "SELECT_EXCEL", "" );
    MsiSetPropertyA( handle, "SELECT_POWERPOINT", "" );
    MsiSetPropertyA( handle, "SELECT_VISIO", "" );

    if ( ! bRegisterNone )
    {
        if ( IsSetMsiProp( handle, "REGISTER_ALL_MSO_TYPES" ) )
        {
            if ( bWriterEnabled )
                MsiSetPropertyA( handle, "SELECT_WORD", "1" );
            if ( bCalcEnabled )
                MsiSetPropertyA( handle, "SELECT_EXCEL", "1" );
            if ( bImpressEnabled )
                MsiSetPropertyA( handle, "SELECT_POWERPOINT", "1" );
            if ( bDrawEnabled )
                MsiSetPropertyA( handle, "SELECT_VISIO", "1" );
        }
        else
        {
            if ( bWriterEnabled && ! checkSomeExtensionInRegistry( WORD_START, EXCEL_START ) )
            {
                MsiSetPropertyA( handle, "SELECT_WORD", "1" );
                OutputDebugStringFormat( "LookForRegisteredExtensions: Register for Microsoft Word" );
            }
            if ( bCalcEnabled && ! checkSomeExtensionInRegistry( EXCEL_START, POWERPOINT_START ) )
            {
                MsiSetPropertyA( handle, "SELECT_EXCEL", "1" );
                OutputDebugStringFormat( "LookForRegisteredExtensions: Register for Microsoft Excel" );
            }
            if ( bImpressEnabled && ! checkSomeExtensionInRegistry( POWERPOINT_START, VISIO_START ) )
            {
                MsiSetPropertyA( handle, "SELECT_POWERPOINT", "1" );
                OutputDebugStringFormat( "LookForRegisteredExtensions: Register for Microsoft PowerPoint" );
            }
            if ( bImpressEnabled && ! checkSomeExtensionInRegistry( VISIO_START, VISIO_END ) )
            {
                MsiSetPropertyA( handle, "SELECT_VISIO", "1" );
                OutputDebugStringFormat( "LookForRegisteredExtensions: Register for Microsoft Visio" );
            }
        }
    }

    MsiSetPropertyA( handle, "FILETYPEDIALOGUSED", "1" );

    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall RegisterSomeExtensions( MSIHANDLE handle )
{
    OutputDebugStringFormat( "RegisterSomeExtensions: " );

    if ( IsSetMsiProp( handle, "SELECT_WORD" ) )
    {
        registerSomeExtensions( handle, WORD_START, EXCEL_START, true );
        MsiSetFeatureState( handle, L"gm_p_Wrt_MSO_Reg", INSTALLSTATE_LOCAL );
        OutputDebugStringFormat( "RegisterSomeExtensions: Register for Microsoft Word" );
    }
    else
    {
        registerSomeExtensions( handle, WORD_START, EXCEL_START, false );
        MsiSetFeatureState( handle, L"gm_p_Wrt_MSO_Reg", INSTALLSTATE_ABSENT );
    }

    if ( IsSetMsiProp( handle, "SELECT_EXCEL" ) )
    {
        registerSomeExtensions( handle, EXCEL_START, POWERPOINT_START, true );
        MsiSetFeatureState( handle, L"gm_p_Calc_MSO_Reg", INSTALLSTATE_LOCAL );
        OutputDebugStringFormat( "RegisterSomeExtensions: Register for Microsoft Excel" );
    }
    else
    {
        registerSomeExtensions( handle, EXCEL_START, POWERPOINT_START, false );
        MsiSetFeatureState( handle, L"gm_p_Calc_MSO_Reg", INSTALLSTATE_ABSENT );
    }

    if ( IsSetMsiProp( handle, "SELECT_POWERPOINT" ) )
    {
        registerSomeExtensions( handle, POWERPOINT_START, VISIO_START, true );
        MsiSetFeatureState( handle, L"gm_p_Impress_MSO_Reg", INSTALLSTATE_LOCAL );
        OutputDebugStringFormat( "RegisterSomeExtensions: Register for Microsoft PowerPoint" );
    }
    else
    {
        registerSomeExtensions( handle, POWERPOINT_START, VISIO_START, false );
        MsiSetFeatureState( handle, L"gm_p_Impress_MSO_Reg", INSTALLSTATE_ABSENT );
    }

    if ( IsSetMsiProp( handle, "SELECT_VISIO" ) )
    {
        registerSomeExtensions( handle, VISIO_START, VISIO_END, true );
        MsiSetFeatureState( handle, L"gm_p_Draw_MSO_Reg", INSTALLSTATE_LOCAL );
        OutputDebugStringFormat( "RegisterSomeExtensions: Register for Microsoft Visio" );
    }
    else
    {
        registerSomeExtensions( handle, VISIO_START, VISIO_END, false );
        MsiSetFeatureState( handle, L"gm_p_Draw_MSO_Reg", INSTALLSTATE_ABSENT );
    }
    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall FindRegisteredExtensions( MSIHANDLE handle )
{
    if ( IsSetMsiProp( handle, "FILETYPEDIALOGUSED" ) )
    {
        OutputDebugStringFormat( "FindRegisteredExtensions: FILETYPEDIALOGUSED!" );
        return ERROR_SUCCESS;
    }

    OutputDebugStringFormat( "FindRegisteredExtensions:" );

    bool bRegisterAll = IsSetMsiProp( handle, "REGISTER_ALL_MSO_TYPES" );

    if ( IsSetMsiProp( handle, "REGISTER_NO_MSO_TYPES" ) )
    {
        OutputDebugStringFormat( "FindRegisteredExtensions: Register none!" );
        return ERROR_SUCCESS;
    }
    else if ( bRegisterAll )
        OutputDebugStringFormat( "FindRegisteredExtensions: Force all on" );
    else
        OutputDebugStringFormat( "FindRegisteredExtensions: " );

    // setting the msi properties SELECT_* will force registering for all corresponding
    // file types
    if ( IsSetMsiProp( handle, "SELECT_WORD" ) )
        registerSomeExtensions( handle, WORD_START, EXCEL_START, true );
    if ( IsSetMsiProp( handle, "SELECT_EXCEL" ) )
        registerSomeExtensions( handle, EXCEL_START, POWERPOINT_START, true );
    if ( IsSetMsiProp( handle, "SELECT_POWERPOINT" ) )
        registerSomeExtensions( handle, POWERPOINT_START, VISIO_START, true );
    if ( IsSetMsiProp( handle, "SELECT_VISIO" ) )
        registerSomeExtensions( handle, VISIO_START, VISIO_END, true );

    registerForExtensions( handle, bRegisterAll );

    return ERROR_SUCCESS;
}

static void restoreOldRegistration( LPCSTR lpSubKey )
{
    BOOL    bRet = false;
    HKEY    hKey = NULL;
    LONG    lResult = RegOpenKeyExA( HKEY_CLASSES_ROOT, lpSubKey, 0,
                                     KEY_QUERY_VALUE|KEY_SET_VALUE, &hKey );

    if ( ERROR_SUCCESS == lResult )
    {
        CHAR    szBuffer[1024];
        DWORD   nSize = sizeof( szBuffer );

        lResult = RegQueryValueExA( hKey, "LOBackupAssociation", NULL, NULL,
                                    (LPBYTE)szBuffer, &nSize );
        if ( ERROR_SUCCESS == lResult )
        {
            HKEY hKey2 = NULL;
            lResult = RegOpenKeyExA( HKEY_CLASSES_ROOT, szBuffer, 0,
                                     KEY_QUERY_VALUE, &hKey2 );
            if ( ERROR_SUCCESS == lResult )
            {
                CHAR   szBuffer2[1024];
                DWORD  nSize2 = sizeof( szBuffer2 );

                lResult = RegQueryValueExA( hKey2, "", NULL, NULL, (LPBYTE)szBuffer2, &nSize2 );
                if ( ERROR_SUCCESS == lResult )
                {
                    CHAR   szBuffer3[1024];
                    DWORD  nSize3 = sizeof( szBuffer3 );

                    // Try to verify that the old association is OK to restore
                    lResult = RegQueryValueExA( hKey, "LOBackupAssociationDeref", NULL, NULL,
                                                (LPBYTE)szBuffer3, &nSize3 );
                    if ( ERROR_SUCCESS == lResult )
                    {
                        if ( nSize2 == nSize3 && strcmp (szBuffer2, szBuffer3) == 0)
                        {
                            // Yep. So restore it
                            RegSetValueExA( hKey, "", 0, REG_SZ, (LPBYTE)szBuffer, nSize );
                        }
                    }
                }
                RegCloseKey( hKey2 );
            }
            RegDeleteValueA( hKey, "LOBackupAssociation" );
        }
        RegDeleteValueA( hKey, "LOBackupAssociationDeref" );
        RegCloseKey( hKey );
    }
}

extern "C" UINT __stdcall RestoreRegAllMSDoc( MSIHANDLE /*handle*/ )
{
    OutputDebugStringFormat( "RestoreRegAllMSDoc\n" );

    int nIndex = 0;
    while ( g_Extensions[nIndex] != 0 )
    {
        restoreOldRegistration( g_Extensions[nIndex] );
        ++nIndex;
    }


    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
