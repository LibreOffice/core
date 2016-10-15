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

#define UNICODE

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <string.h>
#include <malloc.h>

#define CHART_COMPONENT 1
#define DRAW_COMPONENT 2
#define IMPRESS_COMPONENT 4
#define CALC_COMPONENT 8
#define WRITER_COMPONENT 16
#define MATH_COMPONENT 32

typedef int ( __stdcall * DllNativeRegProc ) ( int, BOOL, BOOL, const char* );
typedef int ( __stdcall * DllNativeUnregProc ) ( int, BOOL, BOOL );

BOOL UnicodeEquals( wchar_t const * pStr1, wchar_t const * pStr2 )
{
    if ( pStr1 == nullptr && pStr2 == nullptr )
        return TRUE;
    else if ( pStr1 == nullptr || pStr2 == nullptr )
        return FALSE;

    while( *pStr1 == *pStr2 && *pStr1 && *pStr2 )
    {
        pStr1++;
        pStr2++;
    }

    return ( *pStr1 == 0 && *pStr2 == 0 );
}


char* UnicodeToAnsiString( wchar_t* pUniString )
{
    int len = WideCharToMultiByte(
        CP_ACP, 0, pUniString, -1, nullptr, 0, nullptr, nullptr );

    char* buff = static_cast<char*>( malloc( len ) );

    WideCharToMultiByte(
        CP_ACP, 0, pUniString, -1, buff, len, nullptr, nullptr );

    return buff;
}


void RegisterActiveXNative( const char* pActiveXPath, int nMode, BOOL InstallForAllUser, BOOL InstallFor64Bit )
{
    HINSTANCE hModule = LoadLibraryExA( pActiveXPath, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH );
    if( hModule )
    {
        DllNativeRegProc pNativeProc = reinterpret_cast<DllNativeRegProc>(GetProcAddress( hModule, "DllRegisterServerNative" ));
        if( pNativeProc!=nullptr )
        {
            int nLen = strlen( pActiveXPath );
            int nRemoveLen = strlen( "\\so_activex.dll" );
            if ( nLen > nRemoveLen )
            {
                char* pProgramPath = static_cast<char*>( malloc( nLen - nRemoveLen + 1 ) );
                strncpy( pProgramPath, pActiveXPath, nLen - nRemoveLen );
                pProgramPath[ nLen - nRemoveLen ] = 0;

                ( *pNativeProc )( nMode, InstallForAllUser, InstallFor64Bit, pProgramPath );

                free( pProgramPath );
            }
        }

        FreeLibrary( hModule );
    }
}


void UnregisterActiveXNative( const char* pActiveXPath, int nMode, BOOL InstallForAllUser, BOOL InstallFor64Bit )
{
    HINSTANCE hModule = LoadLibraryExA( pActiveXPath, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH );
    if( hModule )
    {
        DllNativeUnregProc pNativeProc = reinterpret_cast<DllNativeUnregProc>(GetProcAddress( hModule, "DllUnregisterServerNative" ));
        if( pNativeProc!=nullptr )
            ( *pNativeProc )( nMode, InstallForAllUser, InstallFor64Bit );

        FreeLibrary( hModule );
    }
}


BOOL GetMsiPropW( MSIHANDLE hMSI, const wchar_t* pPropName, wchar_t** ppValue )
{
    DWORD sz = 0;
    if ( MsiGetPropertyW( hMSI, pPropName, const_cast<wchar_t *>(L""), &sz ) == ERROR_MORE_DATA )
    {
           sz++;
           DWORD nbytes = sz * sizeof( wchar_t );
           wchar_t* buff = static_cast<wchar_t*>( malloc( nbytes ) );
           ZeroMemory( buff, nbytes );
           MsiGetPropertyW( hMSI, pPropName, buff, &sz );
           *ppValue = buff;

        return TRUE;
    }

    return FALSE;
}


BOOL GetActiveXControlPath( MSIHANDLE hMSI, char** ppActiveXPath )
{
    wchar_t* pProgPath = nullptr;
    if ( GetMsiPropW( hMSI, L"INSTALLLOCATION", &pProgPath ) && pProgPath )
       {
        char* pCharProgPath = UnicodeToAnsiString( pProgPath );

        if ( pCharProgPath )
        {
            int nLen = strlen( pCharProgPath );
            *ppActiveXPath = static_cast<char*>( malloc( nLen + 23 ) );
            strncpy( *ppActiveXPath, pCharProgPath, nLen );
            strncpy( (*ppActiveXPath) + nLen, "program\\so_activex.dll", 22 );
            (*ppActiveXPath)[nLen+22] = 0;

            free( pCharProgPath );

            return TRUE;
        }

        free( pProgPath );
    }

    return FALSE;
}


BOOL GetDelta( MSIHANDLE hMSI, int& nOldInstallMode, int& nInstallMode, int& nDeinstallMode )
{
    // for now the chart is always installed
    nOldInstallMode = CHART_COMPONENT;
    nInstallMode = CHART_COMPONENT;
    nDeinstallMode = 0;

    INSTALLSTATE current_state;
       INSTALLSTATE future_state;

    if ( ERROR_SUCCESS == MsiGetFeatureState( hMSI, L"gm_p_Wrt_Bin", &current_state, &future_state ) )
    {
        // analyze writer installation mode
        if ( current_state == INSTALLSTATE_LOCAL )
            nOldInstallMode |= WRITER_COMPONENT;

        if ( future_state == INSTALLSTATE_LOCAL
          || ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_UNKNOWN ) )
            nInstallMode |= WRITER_COMPONENT;
           else if ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_ABSENT )
            nDeinstallMode |= WRITER_COMPONENT;
    }
    else
    {
        // assert( FALSE );
    }

    if ( ERROR_SUCCESS == MsiGetFeatureState( hMSI, L"gm_p_Calc_Bin", &current_state, &future_state ) )
    {
        // analyze calc installation mode
        if ( current_state == INSTALLSTATE_LOCAL )
            nOldInstallMode |= CALC_COMPONENT;

        if ( future_state == INSTALLSTATE_LOCAL
          || ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_UNKNOWN ) )
            nInstallMode |= CALC_COMPONENT;
           else if ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_ABSENT )
            nDeinstallMode |= CALC_COMPONENT;
    }
    else
    {
        // assert( FALSE );
    }

    if ( ERROR_SUCCESS == MsiGetFeatureState( hMSI, L"gm_p_Draw_Bin", &current_state, &future_state ) )
    {
        // analyze draw installation mode
        if ( current_state == INSTALLSTATE_LOCAL )
            nOldInstallMode |= DRAW_COMPONENT;

        if ( future_state == INSTALLSTATE_LOCAL
          || ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_UNKNOWN ) )
            nInstallMode |= DRAW_COMPONENT;
           else if ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_ABSENT )
            nDeinstallMode |= DRAW_COMPONENT;
    }
    else
    {
        // assert( FALSE );
    }

    if ( ERROR_SUCCESS == MsiGetFeatureState( hMSI, L"gm_p_Impress_Bin", &current_state, &future_state ) )
    {
        // analyze impress installation mode
        if ( current_state == INSTALLSTATE_LOCAL )
            nOldInstallMode |= IMPRESS_COMPONENT;

        if ( future_state == INSTALLSTATE_LOCAL
          || ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_UNKNOWN ) )
            nInstallMode |= IMPRESS_COMPONENT;
           else if ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_ABSENT )
            nDeinstallMode |= IMPRESS_COMPONENT;
    }
    else
    {
        // assert( FALSE );
    }

    if ( ERROR_SUCCESS == MsiGetFeatureState( hMSI, L"gm_p_Math_Bin", &current_state, &future_state ) )
    {
        // analyze math installation mode
        if ( current_state == INSTALLSTATE_LOCAL )
            nOldInstallMode |= MATH_COMPONENT;

        if ( future_state == INSTALLSTATE_LOCAL
          || ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_UNKNOWN ) )
            nInstallMode |= MATH_COMPONENT;
           else if ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_ABSENT )
            nDeinstallMode |= MATH_COMPONENT;
    }
    else
    {
        // assert( FALSE );
    }

    return TRUE;
}


BOOL MakeInstallForAllUsers( MSIHANDLE hMSI )
{
    BOOL bResult = FALSE;
    wchar_t* pVal = nullptr;
    if ( GetMsiPropW( hMSI, L"ALLUSERS", &pVal ) && pVal )
    {
        bResult = UnicodeEquals( pVal , L"1" );
        free( pVal );
    }

    return bResult;
}


BOOL MakeInstallFor64Bit( MSIHANDLE hMSI )
{
    BOOL bResult = FALSE;
    wchar_t* pVal = nullptr;
    if ( GetMsiPropW( hMSI, L"VersionNT64", &pVal ) && pVal )
    {
        bResult = TRUE;
        free( pVal );
    }

    return bResult;
}

extern "C" UINT __stdcall InstallActiveXControl( MSIHANDLE hMSI )
{
    INSTALLSTATE current_state;
    INSTALLSTATE future_state;

    if ( ERROR_SUCCESS == MsiGetFeatureState( hMSI, L"gm_o_Activexcontrol", &current_state, &future_state ) )
    {
        int nOldInstallMode = 0;
        int nInstallMode = 0;
        int nDeinstallMode = 0;
        BOOL bInstallForAllUser = MakeInstallForAllUsers( hMSI );
        BOOL bInstallFor64Bit = MakeInstallFor64Bit( hMSI );

        char* pActiveXPath = nullptr;
        if ( GetActiveXControlPath( hMSI, &pActiveXPath ) && pActiveXPath
        && GetDelta( hMSI, nOldInstallMode, nInstallMode, nDeinstallMode ) )
        {
            if ( future_state == INSTALLSTATE_LOCAL
              || ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_UNKNOWN ) )
            {
                // the control is installed in the new selected configuration

                if ( current_state == INSTALLSTATE_LOCAL && nDeinstallMode )
                    UnregisterActiveXNative( pActiveXPath, nDeinstallMode, bInstallForAllUser, bInstallFor64Bit );

                if ( nInstallMode )
                    RegisterActiveXNative( pActiveXPath, nInstallMode, bInstallForAllUser, bInstallFor64Bit );
            }
            else if ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_ABSENT )
            {
                if ( nOldInstallMode )
                    UnregisterActiveXNative( pActiveXPath, nOldInstallMode, bInstallForAllUser, bInstallFor64Bit );
            }
        }

        if ( pActiveXPath )
            free( pActiveXPath );
    }
    else
    {
        // assert( FALSE );
    }

    return ERROR_SUCCESS;
}


extern "C" UINT __stdcall DeinstallActiveXControl( MSIHANDLE hMSI )
{
    INSTALLSTATE current_state;
    INSTALLSTATE future_state;

    if ( ERROR_SUCCESS == MsiGetFeatureState( hMSI, L"gm_o_Activexcontrol", &current_state, &future_state ) )
    {
        char* pActiveXPath = nullptr;
        if ( current_state == INSTALLSTATE_LOCAL && GetActiveXControlPath( hMSI, &pActiveXPath ) && pActiveXPath )
        {
            BOOL bInstallForAllUser = MakeInstallForAllUsers( hMSI );
            BOOL bInstallFor64Bit = MakeInstallFor64Bit( hMSI );

            {
                UnregisterActiveXNative( pActiveXPath,
                                        CHART_COMPONENT
                                        | DRAW_COMPONENT
                                        | IMPRESS_COMPONENT
                                        | CALC_COMPONENT
                                        | WRITER_COMPONENT
                                        | MATH_COMPONENT,
                                        bInstallForAllUser,
                                        bInstallFor64Bit );
            }

            free( pActiveXPath );
        }
    }

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
