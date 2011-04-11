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

#define UNICODE

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
#include <strsafe.h>

// MinGW doesn't know anything about RegDeleteKeyExW if WINVER < 0x0502.
extern "C" {
WINADVAPI LONG WINAPI RegDeleteKeyExW(HKEY,LPCWSTR,REGSAM,DWORD);
}

// to provide windows xp as build systems for mingw we need to define KEY_WOW64_64KEY
// in mingw 3.13 KEY_WOW64_64KEY isn't available < Win2003 systems.
// Also defined in setup_native\source\win32\customactions\reg64\reg64.cxx,source\win32\customactions\shellextensions\shellextensions.cxx and
// extensions\source\activex\main\so_activex.cpp

#ifndef KEY_WOW64_64KEY
    #define KEY_WOW64_64KEY (0x0100)
#endif


#define TABLE_NAME L"Reg64"
#define INSTALLLOCATION L"[INSTALLLOCATION]"

bool isInstall4AllUsers;
wchar_t * sBasisInstallLocation;


enum OPERATION {
    SET,
    REMOVE
};

#ifdef DEBUG
inline void OutputDebugStringFormat( const wchar_t* pFormat, ... )
{
    wchar_t    buffer[1024];
    va_list args;

    va_start( args, pFormat );
    StringCchVPrintf( buffer, sizeof(buffer), pFormat, args );
    OutputDebugString( buffer );
}
#else
static inline void OutputDebugStringFormat( const wchar_t*, ... )
{
}
#endif

bool WriteRegistry( MSIHANDLE & hMSI, OPERATION op, const wchar_t* componentName)
{
    INSTALLSTATE current_state;
    INSTALLSTATE comp_state;
    UINT ret = MsiGetComponentState( hMSI, componentName, &current_state, &comp_state );
    if ( ERROR_SUCCESS == ret )
    {
        if (current_state == INSTALLSTATE_ABSENT)
            OutputDebugStringFormat(L"WriteRegistry - current_state: INSTALLSTATE_ABSENT");
        else if (current_state == INSTALLSTATE_DEFAULT)
            OutputDebugStringFormat(L"WriteRegistry - current_state: INSTALLSTATE_DEFAULT");
        else if (current_state == INSTALLSTATE_LOCAL)
            OutputDebugStringFormat(L"WriteRegistry - current_state: INSTALLSTATE_LOCAL");
        else if (current_state == INSTALLSTATE_REMOVED)
            OutputDebugStringFormat(L"WriteRegistry - current_state: INSTALLSTATE_REMOVED");
        else if (current_state == INSTALLSTATE_SOURCE)
            OutputDebugStringFormat(L"WriteRegistry - current_state: INSTALLSTATE_SOURCE");
        else if (current_state == INSTALLSTATE_UNKNOWN)
            OutputDebugStringFormat(L"WriteRegistry - current_state: INSTALLSTATE_UNKNOWN");

        if (comp_state == INSTALLSTATE_ABSENT)
            OutputDebugStringFormat(L"WriteRegistry - comp_state: INSTALLSTATE_ABSENT");
        else if (comp_state == INSTALLSTATE_DEFAULT)
            OutputDebugStringFormat(L"WriteRegistry - comp_state: INSTALLSTATE_DEFAULT");
        else if (comp_state == INSTALLSTATE_LOCAL)
            OutputDebugStringFormat(L"WriteRegistry - comp_state: INSTALLSTATE_LOCAL");
        else if (comp_state == INSTALLSTATE_REMOVED)
            OutputDebugStringFormat(L"WriteRegistry - comp_state: INSTALLSTATE_REMOVED");
        else if (comp_state == INSTALLSTATE_SOURCE)
            OutputDebugStringFormat(L"WriteRegistry - comp_state: INSTALLSTATE_SOURCE");
        else if (comp_state == INSTALLSTATE_UNKNOWN)
            OutputDebugStringFormat(L"WriteRegistry - comp_state: INSTALLSTATE_UNKNOWN");

        switch (op)
        {
            case SET :
                if ( comp_state == INSTALLSTATE_LOCAL || ( current_state == INSTALLSTATE_LOCAL && comp_state == INSTALLSTATE_UNKNOWN ) )
                {
                    return true;
                }
                break;
            case REMOVE:
                OutputDebugStringFormat(L"WriteRegistry - Remove\n" );
                if ( current_state == INSTALLSTATE_LOCAL && (comp_state == INSTALLSTATE_ABSENT || comp_state == INSTALLSTATE_REMOVED) )
                {
                    OutputDebugStringFormat(L"WriteRegistry - To be removed\n" );
                    return true;
                }
        }
    } else
    {
        if (ERROR_INVALID_HANDLE == ret) OutputDebugStringFormat(L"WriteRegistry - Invalid handle");
        if (ERROR_UNKNOWN_FEATURE  == ret) OutputDebugStringFormat(L"WriteRegistry - Unknown feature");
    }

    return false;
}

BOOL UnicodeEquals( wchar_t* pStr1, wchar_t* pStr2 )
{
    if ( pStr1 == NULL && pStr2 == NULL )
        return TRUE;
    else if ( pStr1 == NULL || pStr2 == NULL )
        return FALSE;

    while( *pStr1 == *pStr2 && *pStr1 && *pStr2 )
        pStr1++, pStr2++;

    return ( *pStr1 == 0 && *pStr2 == 0 );
}

BOOL GetMsiProp( MSIHANDLE hMSI, const wchar_t* pPropName, wchar_t** ppValue )
{
    OutputDebugStringFormat(L"GetMsiProp - START\n" );
    DWORD sz = 0;
    UINT ret = MsiGetProperty( hMSI, pPropName, L"", &sz );
       if ( ret == ERROR_MORE_DATA )
       {
           sz++;
           DWORD nbytes = sz * sizeof( wchar_t );
           wchar_t* buff = reinterpret_cast<wchar_t*>( malloc( nbytes ) );
           ZeroMemory( buff, nbytes );
           MsiGetProperty( hMSI, pPropName, buff, &sz );

        OutputDebugStringFormat(L"GetMsiProp - Value" );
        OutputDebugStringFormat( buff );
           *ppValue = buff;

        return TRUE;
    } else if (ret  == ERROR_INVALID_HANDLE)
    {
        OutputDebugStringFormat(L"GetMsiProp - ERROR_INVALID_HANDLE" );
    } else if (ret == ERROR_INVALID_PARAMETER)
    {
        OutputDebugStringFormat(L"GetMsiProp - ERROR_INVALID_PARAMETER" );
    } else if (ret == ERROR_SUCCESS)
    {
        OutputDebugStringFormat(L"GetMsiProp - ERROR_SUCCESS" );
    }


    OutputDebugStringFormat(L"GetMsiProp - ENDE\n" );
    return FALSE;
}

bool IsInstallForAllUsers( MSIHANDLE hMSI )
{
    OutputDebugStringFormat(L"IsInstallForAllUsers - START\n" );
    bool bResult = FALSE;
    wchar_t* pVal = NULL;
    if ( GetMsiProp( hMSI, L"ALLUSERS", &pVal ) && pVal )
    {
        bResult = UnicodeEquals( pVal , L"1" );
        free( pVal );
    }

    OutputDebugStringFormat(L"IsInstallForAllUsers - ENDE\n" );
    return bResult;
}

wchar_t* GetBasisInstallLocation( MSIHANDLE hMSI )
{
    OutputDebugStringFormat(L"GetBasisInstallLocation - START\n" );
    wchar_t* pVal = NULL;
    GetMsiProp( hMSI, L"INSTALLLOCATION", &pVal);

    OutputDebugStringFormat(L"GetBasisInstallLocation - ENDE\n" );

    return pVal;
}


bool QueryReg64Table(MSIHANDLE& rhDatabase, MSIHANDLE& rhView)
{
    OutputDebugStringFormat(L"QueryReg64Table - START\n" );
    int const arraysize = 400;
    wchar_t szSelect[arraysize];
    StringCbPrintfW(szSelect, arraysize * sizeof(wchar_t), L"SELECT * FROM %s",TABLE_NAME);
    OutputDebugStringFormat( szSelect );

    UINT ret = MsiDatabaseOpenView(rhDatabase,szSelect,&rhView);
    if (ret != ERROR_SUCCESS)
    {
        if ( ret == ERROR_BAD_QUERY_SYNTAX)
            OutputDebugStringFormat(L"QueryReg64Table - MsiDatabaseOpenView - FAILED - ERROR_BAD_QUERY_SYNTAX\n" );
        if ( ret == ERROR_INVALID_HANDLE)
            OutputDebugStringFormat(L"QueryReg64Table - MsiDatabaseOpenView - FAILED - ERROR_INVALID_HANDLE\n" );
        return false;
    }
    // execute query - not a parameter query so second parameter is NULL.
    if (MsiViewExecute(rhView,NULL) != ERROR_SUCCESS)
    {
        OutputDebugStringFormat(L"QueryReg64Table - MsiViewExecute - FAILED\n" );
        return false;
    }

    OutputDebugStringFormat(L"QueryReg64Table - ENDE\n" );
    return true;
}

//---------------------------------------
bool DeleteRegistryKey(HKEY RootKey, const wchar_t* KeyName)
{
    int rc = RegDeleteKeyExW(
        RootKey, KeyName, KEY_WOW64_64KEY, 0);

    return (ERROR_SUCCESS == rc);
}

bool SetRegistryKey(HKEY RootKey, const wchar_t* KeyName, const wchar_t* ValueName, const wchar_t* Value)
{
    HKEY hSubKey;

    // open or create the desired key
    int rc = RegCreateKeyEx(
        RootKey, KeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_WOW64_64KEY, 0, &hSubKey, 0);

    if (ERROR_SUCCESS == rc)
    {
        OutputDebugStringFormat(L"SetRegistryKey - Created\n" );
        rc = RegSetValueEx(
            hSubKey, ValueName, 0, REG_SZ, reinterpret_cast<const BYTE*>(Value), (wcslen(Value) + 1) * sizeof(wchar_t));

        RegCloseKey(hSubKey);
    } else {
        OutputDebugStringFormat(L"SetRegistryKey - FAILED\n" );
    }


    return (ERROR_SUCCESS == rc);
}

bool DoRegEntries( MSIHANDLE& rhMSI, OPERATION op, MSIHANDLE& rhView)
{
    OutputDebugStringFormat(L"DoRegEntries - START\n" );

    MSIHANDLE hRecord;

    long lRoot;
    wchar_t  szKey[255];
    wchar_t  szName[255];
    wchar_t  szValue[1024];
    wchar_t  szComponent[255];

    /// read records until there are no more records
    while (MsiViewFetch(rhView,&hRecord) == ERROR_SUCCESS)
    {
        DWORD    dwKey = 255;
        DWORD    dwName = 255;
        DWORD    dwValue = 1024;
        DWORD    dwComponent = 255;

        szKey[0] = '\0';
        szName[0] = '\0';
        szValue[0] = '\0';
        szComponent[0] = '\0';

        lRoot = MsiRecordGetInteger(hRecord,2);
        MsiRecordGetString(hRecord,3,szKey,&dwKey);

        if (!MsiRecordIsNull(hRecord, 4))
            MsiRecordGetString(hRecord,4,szName,&dwName);

        if (!MsiRecordIsNull(hRecord, 5))
        {
            MsiRecordGetString(hRecord,5,szValue,&dwValue);



            wchar_t* nPos = wcsstr(szValue , INSTALLLOCATION);
            if ( NULL != nPos)
            {

                DWORD nPrefixSize = nPos - szValue;

                DWORD nPropSize = wcslen(sBasisInstallLocation);
                DWORD nPostfixSize = dwValue - wcslen( INSTALLLOCATION );

                DWORD nNewValueBytes = (nPropSize + nPostfixSize + 1) * sizeof( wchar_t );
                   wchar_t* newValue = reinterpret_cast<wchar_t*>( malloc( nNewValueBytes ) );
                   ZeroMemory( newValue, nNewValueBytes );

                // prefix
                wcsncpy(newValue, szValue, nPrefixSize);

                // basis location
                wcsncat(newValue, sBasisInstallLocation, nPropSize * sizeof( wchar_t ));

                // postfix
                wcsncat(newValue, nPos + ( wcslen( INSTALLLOCATION ) ), nPropSize * sizeof( wchar_t ));

                wcsncpy(szValue, newValue, nNewValueBytes <=1024? nNewValueBytes: 1024);

                free(newValue);
            }

        }


        MsiRecordGetString(hRecord,6,szComponent,&dwComponent);

        OutputDebugStringFormat(L"****** DoRegEntries *******" );
        OutputDebugStringFormat(L"Root:" );
        HKEY key = HKEY_CURRENT_USER;
        switch (lRoot)
        {
            case(-1):
                    if (isInstall4AllUsers)
                    {
                        key = HKEY_LOCAL_MACHINE;
                        OutputDebugStringFormat(L"HKEY_LOCAL_MACHINE" );
                    }
                    else
                    {
                        key = HKEY_CURRENT_USER;
                        OutputDebugStringFormat(L"HKEY_CURRENT_USER" );
                    }
                break;
            case(0):
                    key = HKEY_CLASSES_ROOT;
                    OutputDebugStringFormat(L"HKEY_CLASSES_ROOT" );
                break;
            case(1):
                    key = HKEY_CURRENT_USER;
                    OutputDebugStringFormat(L"HKEY_CURRENT_USER" );
                break;
            case(2):
                    key = HKEY_LOCAL_MACHINE;
                    OutputDebugStringFormat(L"HKEY_LOCAL_MACHINE" );
                break;
            case(3):
                    key = HKEY_USERS;
                    OutputDebugStringFormat(L"HKEY_USERS" );
                break;
            default:
                    OutputDebugStringFormat(L"Unknown Root!" );
                break;
        }

        OutputDebugStringFormat(L"Key:");
        OutputDebugStringFormat( szKey );
        OutputDebugStringFormat(L"Name:");
        OutputDebugStringFormat( szName );
        OutputDebugStringFormat(L"Value:");
        OutputDebugStringFormat( szValue);
        OutputDebugStringFormat(L"Component:");
        OutputDebugStringFormat( szComponent );
        OutputDebugStringFormat(L"*******************" );
        switch (op)
        {
            case SET:

                    if (WriteRegistry(rhMSI, SET, szComponent))
                    {
                        OutputDebugStringFormat(L"DoRegEntries - Write\n" );
                        SetRegistryKey(key, szKey, szName, szValue);
                    }
                break;
            case REMOVE:
                    OutputDebugStringFormat(L"DoRegEntries - PreRemove\n" );
                    if (WriteRegistry(rhMSI, REMOVE, szComponent))
                    {
                        OutputDebugStringFormat(L"DoRegEntries - Remove\n" );
                        DeleteRegistryKey(key, szKey);
                    }
                break;
        }
    }

    MsiCloseHandle(rhView);


    OutputDebugStringFormat(L"DoRegEntries - ENDE\n" );

    return true;
}


bool Reg64(MSIHANDLE& rhMSI, OPERATION op)
{
    isInstall4AllUsers = IsInstallForAllUsers(rhMSI);
    sBasisInstallLocation = GetBasisInstallLocation(rhMSI);

    if (NULL == sBasisInstallLocation)
    {
        OutputDebugStringFormat(L"BASISINSTALLLOCATION is NULL\n" );
        return false;
    }

    MSIHANDLE hView;
    MSIHANDLE hDatabase = MsiGetActiveDatabase(rhMSI);

    QueryReg64Table(hDatabase, hView);
    OutputDebugStringFormat(L"Do something\n" );
    DoRegEntries( rhMSI, op, hView);
    OutputDebugStringFormat(L"Something done\n" );

    MsiCloseHandle(hView);
    MsiCloseHandle(hDatabase);
    free(sBasisInstallLocation);

    return true;
}

extern "C" UINT __stdcall InstallReg64(MSIHANDLE hMSI)
{
    OutputDebugStringFormat(L"InstallReg64\n" );
    Reg64(hMSI, SET);
    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall DeinstallReg64(MSIHANDLE hMSI)
{
    OutputDebugStringFormat(L"DeinstallReg64\n" );
    Reg64(hMSI, REMOVE);
    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
