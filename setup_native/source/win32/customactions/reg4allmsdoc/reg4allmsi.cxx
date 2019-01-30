/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>

#include <malloc.h>
#include <string>
#include <strsafe.h>

static const WCHAR* g_Extensions[] =
{
    L".doc",     // Microsoft Word Text [0]
    L".dot",     // Microsoft Word Template
    L".wps",     // Kingsoft Writer Document
    L".wpt",     // Kingsoft Writer Template
    L".rtf",     // rtf text
    L".docx",    // Office Word 2007 XML document
    L".docm",    // Office Word 2007 XML macro-enabled document
    L".dotx",    // Office Word 2007 XML template
    L".dotm",    // Office Word 2007 XML macro-enabled template
    L".xlw",     // Microsoft Excel
    L".xls",     // Microsoft Excel
    L".xlt",     // Microsoft Excel Template
    L".xlsx",    // Office Excel 2007 XML workbook
    L".xlsm",    // Office Excel 2007 XML macro-enabled workbook
    L".xltx",    // Office Excel 2007 XML template
    L".xltm",    // Office Excel 2007 XML macro-enabled template
    L".xlsb",    // Office Excel 2007 binary workbook (BIFF12)
    L".iqy",     // Microsoft Excel Web Query File
    L".et",      // Kingsoft Spreadsheet
    L".ett",     // Kingsoft SpreadSheet Template
    L".ppt",     // Microsoft Powerpoint
    L".pps",     // Microsoft Powerpoint
    L".pot",     // Microsoft Powerpoint Template
    L".pptx",    // Office PowerPoint 2007 XML presentation
    L".pptm",    // Office PowerPoint 2007 macro-enabled XML presentation
    L".potx",    // Office PowerPoint 2007 XML template
    L".potm",    // Office PowerPoint 2007 macro-enabled XML template
    L".ppsx",    // Office PowerPoint 2007 XML show
    L".dps",     // Kingsoft Presentation
    L".dpt",     // Kingsoft Presentation Template
    L".vsd",     // Visio 2000/XP/2003 document
    L".vst",     // Visio 2000/XP/2003 template
    nullptr
};

static const int WORD_START = 0;
static const int EXCEL_START = 9;
static const int POWERPOINT_START = 20;
static const int VISIO_START = 30;
static const int VISIO_END = 32;

//    ".xlam",    // Office Excel 2007 XML macro-enabled add-in
//    ".ppam",    // Office PowerPoint 2007 macro-enabled XML add-in
//    ".ppsm",    // Office PowerPoint 2007 macro-enabled XML show

#ifdef DEBUG
inline void OutputDebugStringFormatW( LPCWSTR pFormat, ... )
{
    WCHAR    buffer[1024];
    va_list args;

    va_start( args, pFormat );
    StringCchVPrintfW( buffer, sizeof(buffer)/sizeof(*buffer), pFormat, args );
    OutputDebugStringW( buffer );
    va_end(args);
}
#else
static void OutputDebugStringFormatW( LPCWSTR, ... )
{
}
#endif

static BOOL CheckExtensionInRegistry( LPCWSTR lpSubKey )
{
    BOOL    bRet = false;
    HKEY    hKey = nullptr;
    LONG    lResult = RegOpenKeyExW( HKEY_CLASSES_ROOT, lpSubKey, 0, KEY_QUERY_VALUE, &hKey );

    if ( ERROR_SUCCESS == lResult )
    {
        WCHAR szBuffer[1024];
        DWORD nSize = sizeof( szBuffer );

        lResult = RegQueryValueExW( hKey, L"", nullptr, nullptr, reinterpret_cast<LPBYTE>(szBuffer), &nSize );
        if ( ERROR_SUCCESS == lResult && nSize > 0 )
        {
            szBuffer[nSize/sizeof(*szBuffer)] = L'\0';
            OutputDebugStringFormatW( L"Found value [%s] for key [%s].\n", szBuffer, lpSubKey );

            if ( wcsncmp( szBuffer, L"WordPad.Document.1", 18 ) == 0 )
            {   // We will replace registration for WordPad (alas, on XP only) FIXME
                bRet = true;
            }
            else if ( wcsncmp( szBuffer, L"LibreOffice.", 12 ) == 0 )
            {   // We will replace registration for our own types, too
                bRet = true;
            }
            else if ( wcsncmp( szBuffer, L"lostub.", 7 ) == 0 )
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

static bool GetMsiPropW( MSIHANDLE handle, LPCWSTR name, /*out*/std::wstring& value )
{
    DWORD sz = 0;
    LPWSTR dummy = const_cast<LPWSTR>(L"");
    if (MsiGetPropertyW(handle, name, dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++;
        DWORD nbytes = sz * sizeof(WCHAR);
        LPWSTR buff = static_cast<LPWSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetPropertyW(handle, name, buff, &sz);
        value = buff;
        return true;
    }
    return false;
}

static bool IsSetMsiPropW( MSIHANDLE handle, LPCWSTR name )
{
    std::wstring val;
    GetMsiPropW( handle, name, val );
    return (val == L"1");
}

static void registerForExtension( MSIHANDLE handle, const int nIndex, bool bRegister )
{
    WCHAR sPropName[256];
    StringCchCopyW( sPropName, 256, L"REGISTER_" );
    StringCchCatW( sPropName, 256, (g_Extensions[nIndex])+1 );
    CharUpperBuffW( sPropName+9, 4 );

    if ( bRegister ) {
        MsiSetPropertyW( handle, sPropName, L"1" );
        OutputDebugStringFormatW( L"Set MSI property %s.\n", sPropName );
    } else {
        MsiSetPropertyW( handle, sPropName, L"0" );
        OutputDebugStringFormatW( L"Unset MSI property %s.\n", sPropName );
    }
}

static void saveOldRegistration( LPCWSTR lpSubKey )
{
    HKEY    hKey = nullptr;
    LONG    lResult = RegOpenKeyExW( HKEY_CLASSES_ROOT, lpSubKey, 0,
                                     KEY_QUERY_VALUE|KEY_SET_VALUE, &hKey );

    if ( ERROR_SUCCESS == lResult )
    {
        WCHAR   szBuffer[1024];
        DWORD   nSize = sizeof( szBuffer );

        lResult = RegQueryValueExW( hKey, L"", nullptr, nullptr, reinterpret_cast<LPBYTE>(szBuffer), &nSize );
        if ( ERROR_SUCCESS == lResult )
        {
            szBuffer[nSize/sizeof(*szBuffer)] = L'\0';

            // No need to save associations for our own types
            if ( wcsncmp( szBuffer, L"LibreOffice.", 12 ) != 0 )
            {
                // Save the old association
                RegSetValueExW( hKey, L"LOBackupAssociation", 0,
                                REG_SZ, reinterpret_cast<LPBYTE>(szBuffer), nSize );
                // Also save what the old association means, just so we can try to verify
                // if/when restoring it that the old application still exists
                HKEY hKey2 = nullptr;
                lResult = RegOpenKeyExW( HKEY_CLASSES_ROOT, szBuffer, 0,
                                         KEY_QUERY_VALUE, &hKey2 );
                if ( ERROR_SUCCESS == lResult )
                {
                    nSize = sizeof( szBuffer );
                    lResult = RegQueryValueExW( hKey2, L"", nullptr, nullptr, reinterpret_cast<LPBYTE>(szBuffer), &nSize );
                    if ( ERROR_SUCCESS == lResult )
                    {
                        RegSetValueExW( hKey, L"LOBackupAssociationDeref", 0,
                                        REG_SZ, reinterpret_cast<LPBYTE>(szBuffer), nSize );
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
    while ( g_Extensions[nIndex] != nullptr )
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

    while ( !bFound && (nIndex < nEnd) && (g_Extensions[nIndex] != nullptr) )
    {
        bFound = ! CheckExtensionInRegistry( g_Extensions[nIndex] );

        if ( bFound )
            OutputDebugStringFormatW( L"Found registration for [%s].\n", g_Extensions[nIndex] );

        ++nIndex;
    }
    return bFound;
}

static void registerSomeExtensions( MSIHANDLE handle, const int nStart, const int nEnd, bool bRegister )
{ // Check all file extensions
    int nIndex = nStart;

    while ( (nIndex < nEnd) && (g_Extensions[nIndex] != nullptr) )
    {
        registerForExtension( handle, nIndex++, bRegister );
    }
}

extern "C" __declspec(dllexport) UINT __stdcall LookForRegisteredExtensions( MSIHANDLE handle )
{
    OutputDebugStringFormatW( L"LookForRegisteredExtensions: " );

    INSTALLSTATE current_state;
    INSTALLSTATE future_state;

    bool bWriterEnabled = false;
    bool bCalcEnabled = false;
    bool bImpressEnabled = false;
    bool bDrawEnabled = false;
    bool bRegisterNone = IsSetMsiPropW( handle, L"REGISTER_NO_MSO_TYPES" );

    if ( ( ERROR_SUCCESS == MsiGetFeatureStateW( handle, L"gm_p_Wrt", &current_state, &future_state ) ) &&
         ( (future_state == INSTALLSTATE_LOCAL) || ((current_state == INSTALLSTATE_LOCAL) && (future_state == INSTALLSTATE_UNKNOWN) ) ) )
        bWriterEnabled = true;

    OutputDebugStringFormatW( L"LookForRegisteredExtensions: Install state Writer is [%d], will be [%d]", current_state, future_state );
    if ( bWriterEnabled )
        OutputDebugStringFormatW( L"LookForRegisteredExtensions: Writer is enabled" );
    else
        OutputDebugStringFormatW( L"LookForRegisteredExtensions: Writer is NOT enabled" );

    if ( ( ERROR_SUCCESS == MsiGetFeatureStateW( handle, L"gm_p_Calc", &current_state, &future_state ) ) &&
         ( (future_state == INSTALLSTATE_LOCAL) || ((current_state == INSTALLSTATE_LOCAL) && (future_state == INSTALLSTATE_UNKNOWN) ) ) )
        bCalcEnabled = true;

    OutputDebugStringFormatW( L"LookForRegisteredExtensions: Install state Calc is [%d], will be [%d]", current_state, future_state );
    if ( bCalcEnabled )
        OutputDebugStringFormatW( L"LookForRegisteredExtensions: Calc is enabled" );
    else
        OutputDebugStringFormatW( L"LookForRegisteredExtensions: Calc is NOT enabled" );

    if ( ( ERROR_SUCCESS == MsiGetFeatureStateW( handle, L"gm_p_Impress", &current_state, &future_state ) ) &&
         ( (future_state == INSTALLSTATE_LOCAL) || ((current_state == INSTALLSTATE_LOCAL) && (future_state == INSTALLSTATE_UNKNOWN) ) ) )
        bImpressEnabled = true;

    OutputDebugStringFormatW( L"LookForRegisteredExtensions: Install state Impress is [%d], will be [%d]", current_state, future_state );
    if ( bImpressEnabled )
        OutputDebugStringFormatW( L"LookForRegisteredExtensions: Impress is enabled" );
    else
        OutputDebugStringFormatW( L"LookForRegisteredExtensions: Impress is NOT enabled" );

    if ( ( ERROR_SUCCESS == MsiGetFeatureStateW( handle, L"gm_p_Draw", &current_state, &future_state ) ) &&
         ( (future_state == INSTALLSTATE_LOCAL) || ((current_state == INSTALLSTATE_LOCAL) && (future_state == INSTALLSTATE_UNKNOWN) ) ) )
        bDrawEnabled = true;

    OutputDebugStringFormatW( L"LookForRegisteredExtensions: Install state Draw is [%d], will be [%d]", current_state, future_state );
    if ( bImpressEnabled )
        OutputDebugStringFormatW( L"LookForRegisteredExtensions: Draw is enabled" );
    else
        OutputDebugStringFormatW( L"LookForRegisteredExtensions: Draw is NOT enabled" );

    MsiSetPropertyW( handle, L"SELECT_WORD", L"" );
    MsiSetPropertyW( handle, L"SELECT_EXCEL", L"" );
    MsiSetPropertyW( handle, L"SELECT_POWERPOINT", L"" );
    MsiSetPropertyW( handle, L"SELECT_VISIO", L"" );

    if ( ! bRegisterNone )
    {
        if ( IsSetMsiPropW( handle, L"REGISTER_ALL_MSO_TYPES" ) )
        {
            if ( bWriterEnabled )
                MsiSetPropertyW( handle, L"SELECT_WORD", L"1" );
            if ( bCalcEnabled )
                MsiSetPropertyW( handle, L"SELECT_EXCEL", L"1" );
            if ( bImpressEnabled )
                MsiSetPropertyW( handle, L"SELECT_POWERPOINT", L"1" );
            if ( bDrawEnabled )
                MsiSetPropertyW( handle, L"SELECT_VISIO", L"1" );
        }
        else
        {
            if ( bWriterEnabled && ! checkSomeExtensionInRegistry( WORD_START, EXCEL_START ) )
            {
                MsiSetPropertyW( handle, L"SELECT_WORD", L"1" );
                OutputDebugStringFormatW( L"LookForRegisteredExtensions: Register for Microsoft Word" );
            }
            if ( bCalcEnabled && ! checkSomeExtensionInRegistry( EXCEL_START, POWERPOINT_START ) )
            {
                MsiSetPropertyW( handle, L"SELECT_EXCEL", L"1" );
                OutputDebugStringFormatW( L"LookForRegisteredExtensions: Register for Microsoft Excel" );
            }
            if ( bImpressEnabled && ! checkSomeExtensionInRegistry( POWERPOINT_START, VISIO_START ) )
            {
                MsiSetPropertyW( handle, L"SELECT_POWERPOINT", L"1" );
                OutputDebugStringFormatW( L"LookForRegisteredExtensions: Register for Microsoft PowerPoint" );
            }
            if ( bImpressEnabled && ! checkSomeExtensionInRegistry( VISIO_START, VISIO_END ) )
            {
                MsiSetPropertyW( handle, L"SELECT_VISIO", L"1" );
                OutputDebugStringFormatW( L"LookForRegisteredExtensions: Register for Microsoft Visio" );
            }
        }
    }

    MsiSetPropertyW( handle, L"FILETYPEDIALOGUSED", L"1" );

    return ERROR_SUCCESS;
}

extern "C" __declspec(dllexport) UINT __stdcall RegisterSomeExtensions( MSIHANDLE handle )
{
    OutputDebugStringFormatW( L"RegisterSomeExtensions: " );

    if ( IsSetMsiPropW( handle, L"SELECT_WORD" ) )
    {
        registerSomeExtensions( handle, WORD_START, EXCEL_START, true );
        MsiSetFeatureStateW( handle, L"gm_p_Wrt_MSO_Reg", INSTALLSTATE_LOCAL );
        OutputDebugStringFormatW( L"RegisterSomeExtensions: Register for Microsoft Word" );
    }
    else
    {
        registerSomeExtensions( handle, WORD_START, EXCEL_START, false );
        MsiSetFeatureStateW( handle, L"gm_p_Wrt_MSO_Reg", INSTALLSTATE_ABSENT );
    }

    if ( IsSetMsiPropW( handle, L"SELECT_EXCEL" ) )
    {
        registerSomeExtensions( handle, EXCEL_START, POWERPOINT_START, true );
        MsiSetFeatureStateW( handle, L"gm_p_Calc_MSO_Reg", INSTALLSTATE_LOCAL );
        OutputDebugStringFormatW( L"RegisterSomeExtensions: Register for Microsoft Excel" );
    }
    else
    {
        registerSomeExtensions( handle, EXCEL_START, POWERPOINT_START, false );
        MsiSetFeatureStateW( handle, L"gm_p_Calc_MSO_Reg", INSTALLSTATE_ABSENT );
    }

    if ( IsSetMsiPropW( handle, L"SELECT_POWERPOINT" ) )
    {
        registerSomeExtensions( handle, POWERPOINT_START, VISIO_START, true );
        MsiSetFeatureStateW( handle, L"gm_p_Impress_MSO_Reg", INSTALLSTATE_LOCAL );
        OutputDebugStringFormatW( L"RegisterSomeExtensions: Register for Microsoft PowerPoint" );
    }
    else
    {
        registerSomeExtensions( handle, POWERPOINT_START, VISIO_START, false );
        MsiSetFeatureStateW( handle, L"gm_p_Impress_MSO_Reg", INSTALLSTATE_ABSENT );
    }

    if ( IsSetMsiPropW( handle, L"SELECT_VISIO" ) )
    {
        registerSomeExtensions( handle, VISIO_START, VISIO_END, true );
        MsiSetFeatureStateW( handle, L"gm_p_Draw_MSO_Reg", INSTALLSTATE_LOCAL );
        OutputDebugStringFormatW( L"RegisterSomeExtensions: Register for Microsoft Visio" );
    }
    else
    {
        registerSomeExtensions( handle, VISIO_START, VISIO_END, false );
        MsiSetFeatureStateW( handle, L"gm_p_Draw_MSO_Reg", INSTALLSTATE_ABSENT );
    }
    return ERROR_SUCCESS;
}

extern "C" __declspec(dllexport) UINT __stdcall FindRegisteredExtensions( MSIHANDLE handle )
{
    if ( IsSetMsiPropW( handle, L"FILETYPEDIALOGUSED" ) )
    {
        OutputDebugStringFormatW( L"FindRegisteredExtensions: FILETYPEDIALOGUSED!" );
        return ERROR_SUCCESS;
    }

    OutputDebugStringFormatW( L"FindRegisteredExtensions:" );

    bool bRegisterAll = IsSetMsiPropW( handle, L"REGISTER_ALL_MSO_TYPES" );

    if ( IsSetMsiPropW( handle, L"REGISTER_NO_MSO_TYPES" ) )
    {
        OutputDebugStringFormatW( L"FindRegisteredExtensions: Register none!" );
        return ERROR_SUCCESS;
    }
    else if ( bRegisterAll )
        OutputDebugStringFormatW( L"FindRegisteredExtensions: Force all on" );
    else
        OutputDebugStringFormatW( L"FindRegisteredExtensions: " );

    // setting the msi properties SELECT_* will force registering for all corresponding
    // file types
    if ( IsSetMsiPropW( handle, L"SELECT_WORD" ) )
        registerSomeExtensions( handle, WORD_START, EXCEL_START, true );
    if ( IsSetMsiPropW( handle, L"SELECT_EXCEL" ) )
        registerSomeExtensions( handle, EXCEL_START, POWERPOINT_START, true );
    if ( IsSetMsiPropW( handle, L"SELECT_POWERPOINT" ) )
        registerSomeExtensions( handle, POWERPOINT_START, VISIO_START, true );
    if ( IsSetMsiPropW( handle, L"SELECT_VISIO" ) )
        registerSomeExtensions( handle, VISIO_START, VISIO_END, true );

    registerForExtensions( handle, bRegisterAll );

    return ERROR_SUCCESS;
}

static void restoreOldRegistration( LPCWSTR lpSubKey )
{
    HKEY    hKey = nullptr;
    LONG    lResult = RegOpenKeyExW( HKEY_CLASSES_ROOT, lpSubKey, 0,
                                     KEY_QUERY_VALUE|KEY_SET_VALUE, &hKey );

    if ( ERROR_SUCCESS == lResult )
    {
        WCHAR   szBuffer[1024];
        DWORD   nSize = sizeof( szBuffer );

        lResult = RegQueryValueExW( hKey, L"LOBackupAssociation", nullptr, nullptr,
                                    reinterpret_cast<LPBYTE>(szBuffer), &nSize );
        if ( ERROR_SUCCESS == lResult )
        {
            szBuffer[nSize/sizeof(*szBuffer)] = L'\0';
            HKEY hKey2 = nullptr;
            lResult = RegOpenKeyExW( HKEY_CLASSES_ROOT, szBuffer, 0,
                                     KEY_QUERY_VALUE, &hKey2 );
            if ( ERROR_SUCCESS == lResult )
            {
                WCHAR  szBuffer2[1024];
                DWORD  nSize2 = sizeof( szBuffer2 );

                lResult = RegQueryValueExW( hKey2, L"", nullptr, nullptr, reinterpret_cast<LPBYTE>(szBuffer2), &nSize2 );
                if ( ERROR_SUCCESS == lResult )
                {
                    WCHAR  szBuffer3[1024];
                    DWORD  nSize3 = sizeof( szBuffer3 );

                    // Try to verify that the old association is OK to restore
                    lResult = RegQueryValueExW( hKey, L"LOBackupAssociationDeref", nullptr, nullptr,
                                                reinterpret_cast<LPBYTE>(szBuffer3), &nSize3 );
                    if ( ERROR_SUCCESS == lResult )
                    {
                        if ( nSize2 == nSize3 && wcsncmp (szBuffer2, szBuffer3, nSize2/sizeof(*szBuffer2)) == 0)
                        {
                            // Yep. So restore it
                            RegSetValueExW( hKey, L"", 0, REG_SZ, reinterpret_cast<LPBYTE>(szBuffer), nSize );
                        }
                    }
                }
                RegCloseKey( hKey2 );
            }
            RegDeleteValueW( hKey, L"LOBackupAssociation" );
        }
        RegDeleteValueW( hKey, L"LOBackupAssociationDeref" );
        RegCloseKey( hKey );
    }
}

extern "C" __declspec(dllexport) UINT __stdcall RestoreRegAllMSDoc( MSIHANDLE /*handle*/ )
{
    OutputDebugStringFormatW( L"RestoreRegAllMSDoc\n" );

    int nIndex = 0;
    while ( g_Extensions[nIndex] != nullptr )
    {
        restoreOldRegistration( g_Extensions[nIndex] );
        ++nIndex;
    }


    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
