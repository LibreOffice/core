/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: regactivex.cxx,v $
 * $Revision: 1.16 $
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

// #define OWN_DEBUG_PRINT

typedef int ( __stdcall * DllNativeProc ) ( int, BOOL );

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

//----------------------------------------------------------
char* UnicodeToAnsiString( wchar_t* pUniString )
{
    int len = WideCharToMultiByte(
        CP_ACP, 0, pUniString, -1, 0, 0, 0, 0 );

    char* buff = reinterpret_cast<char*>( malloc( len ) );

    WideCharToMultiByte(
        CP_ACP, 0, pUniString, -1, buff, len, 0, 0 );

    return buff;
}

#ifdef OWN_DEBUG_PRINT
void WarningMessageInt( wchar_t* pWarning, unsigned int nValue )
{
    wchar_t pStr[5] = { nValue%10000/1000 + 48, nValue%1000/100 + 48, nValue%100/10 + 48, nValue%10 + 48, 0 };
       MessageBox(NULL, pStr, pWarning, MB_OK | MB_ICONINFORMATION);
}
#endif

//----------------------------------------------------------
void RegisterActiveXNative( const char* pActiveXPath, int nMode, BOOL InstallForAllUser )
{
    // For Win98/WinME the values should be written to the local machine
    OSVERSIONINFO       aVerInfo;
    aVerInfo.dwOSVersionInfoSize = sizeof( aVerInfo );
    if ( GetVersionEx( &aVerInfo ) && aVerInfo.dwPlatformId != VER_PLATFORM_WIN32_NT )
        InstallForAllUser = TRUE;

    HINSTANCE hModule = LoadLibraryExA( pActiveXPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
    if( !( hModule <= ( HINSTANCE )HINSTANCE_ERROR ) )
    {
        DllNativeProc pNativeProc = ( DllNativeProc )GetProcAddress( hModule, "DllRegisterServerNative" );
        if( pNativeProc!=NULL )
            ( *pNativeProc )( nMode, InstallForAllUser );

        FreeLibrary( hModule );
    }
}

//----------------------------------------------------------
void UnregisterActiveXNative( const char* pActiveXPath, int nMode, BOOL InstallForAllUser )
{
    // For Win98/WinME the values should be written to the local machine
    OSVERSIONINFO       aVerInfo;
    aVerInfo.dwOSVersionInfoSize = sizeof( aVerInfo );
    if ( GetVersionEx( &aVerInfo ) && aVerInfo.dwPlatformId != VER_PLATFORM_WIN32_NT )
        InstallForAllUser = TRUE;

    HINSTANCE hModule = LoadLibraryExA( pActiveXPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
    if( !( hModule <= ( HINSTANCE )HINSTANCE_ERROR ) )
    {
        DllNativeProc pNativeProc = ( DllNativeProc )GetProcAddress( hModule, "DllUnregisterServerNative" );
        if( pNativeProc!=NULL )
            ( *pNativeProc )( nMode, InstallForAllUser );

        FreeLibrary( hModule );
    }
}

//----------------------------------------------------------
BOOL GetMsiProp( MSIHANDLE hMSI, const wchar_t* pPropName, wchar_t** ppValue )
{
    DWORD sz = 0;
       if ( MsiGetProperty( hMSI, pPropName, L"", &sz ) == ERROR_MORE_DATA )
       {
           sz++;
           DWORD nbytes = sz * sizeof( wchar_t );
           wchar_t* buff = reinterpret_cast<wchar_t*>( malloc( nbytes ) );
           ZeroMemory( buff, nbytes );
           MsiGetProperty( hMSI, pPropName, buff, &sz );
           *ppValue = buff;

        return TRUE;
    }

    return FALSE;
}

//----------------------------------------------------------
BOOL GetActiveXControlPath( MSIHANDLE hMSI, char** ppActiveXPath )
{
    wchar_t* pProgPath = NULL;
    if ( GetMsiProp( hMSI, L"BASISINSTALLLOCATION", &pProgPath ) && pProgPath )
       {
        char* pCharProgPath = UnicodeToAnsiString( pProgPath );
        if ( pCharProgPath )
        {
            int nLen = strlen( pCharProgPath );
            *ppActiveXPath = reinterpret_cast<char*>( malloc( nLen + 23 ) );
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

//----------------------------------------------------------
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
#ifdef OWN_DEBUG_PRINT
        WarningMessageInt( L"writer current_state = ", current_state );
        WarningMessageInt( L"writer future_state = ", future_state );
#endif

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
#ifdef OWN_DEBUG_PRINT
        WarningMessageInt( L"calc current_state = ", current_state );
        WarningMessageInt( L"calc future_state = ", future_state );
#endif

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

//----------------------------------------------------------
BOOL MakeInstallForAllUsers( MSIHANDLE hMSI )
{
    BOOL bResult = FALSE;
    wchar_t* pVal = NULL;
    if ( GetMsiProp( hMSI, L"ALLUSERS", &pVal ) && pVal )
    {
        bResult = UnicodeEquals( pVal , L"1" );
        free( pVal );
    }

    return bResult;
}

//----------------------------------------------------------
extern "C" UINT __stdcall InstallActiveXControl( MSIHANDLE hMSI )
{
    int nOldInstallMode = 0;
    int nInstallMode = 0;
    int nDeinstallMode = 0;

#ifdef OWN_DEBUG_PRINT
    MessageBox(NULL, L"InstallActiveXControl", L"Information", MB_OK | MB_ICONINFORMATION);
#endif

    INSTALLSTATE current_state;
    INSTALLSTATE future_state;

    if ( ERROR_SUCCESS == MsiGetFeatureState( hMSI, L"gm_o_Activexcontrol", &current_state, &future_state ) )
    {
#ifdef OWN_DEBUG_PRINT
        MessageBox(NULL, L"InstallActiveXControl Step2", L"Information", MB_OK | MB_ICONINFORMATION);
#endif

        BOOL bInstallForAllUser = MakeInstallForAllUsers( hMSI );
        char* pActiveXPath = NULL;
        if ( GetActiveXControlPath( hMSI, &pActiveXPath ) && pActiveXPath
        && GetDelta( hMSI, nOldInstallMode, nInstallMode, nDeinstallMode ) )
        {
#ifdef OWN_DEBUG_PRINT
            MessageBox(NULL, L"InstallActiveXControl Step3", L"Information", MB_OK | MB_ICONINFORMATION);
#endif

            if ( future_state == INSTALLSTATE_LOCAL
              || ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_UNKNOWN ) )
            {
#ifdef OWN_DEBUG_PRINT
                MessageBox(NULL, L"InstallActiveXControl, adjusting", L"Information", MB_OK | MB_ICONINFORMATION);
                WarningMessageInt( L"nInstallMode = ", nInstallMode );
#endif
                // the control is installed in the new selected configuration

                if ( current_state == INSTALLSTATE_LOCAL && nDeinstallMode )
                    UnregisterActiveXNative( pActiveXPath, nDeinstallMode, bInstallForAllUser );

                if ( nInstallMode )
                    RegisterActiveXNative( pActiveXPath, nInstallMode, bInstallForAllUser );
            }
            else if ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_ABSENT )
            {
#ifdef OWN_DEBUG_PRINT
                MessageBox(NULL, L"InstallActiveXControl, removing", L"Information", MB_OK | MB_ICONINFORMATION);
#endif
                if ( nOldInstallMode )
                    UnregisterActiveXNative( pActiveXPath, nOldInstallMode, bInstallForAllUser );
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

//----------------------------------------------------------
extern "C" UINT __stdcall DeinstallActiveXControl( MSIHANDLE hMSI )
{
    INSTALLSTATE current_state;
    INSTALLSTATE future_state;

#ifdef OWN_DEBUG_PRINT
    MessageBox(NULL, L"DeinstallActiveXControl", L"Information", MB_OK | MB_ICONINFORMATION);
#endif

    if ( ERROR_SUCCESS == MsiGetFeatureState( hMSI, L"gm_o_Activexcontrol", &current_state, &future_state ) )
    {
        char* pActiveXPath = NULL;
        if ( current_state == INSTALLSTATE_LOCAL && GetActiveXControlPath( hMSI, &pActiveXPath ) && pActiveXPath )
        {
            BOOL bInstallForAllUser = MakeInstallForAllUsers( hMSI );

            {
                UnregisterActiveXNative( pActiveXPath,
                                        CHART_COMPONENT
                                        | DRAW_COMPONENT
                                        | IMPRESS_COMPONENT
                                        | CALC_COMPONENT
                                        | WRITER_COMPONENT
                                        | MATH_COMPONENT,
                                        bInstallForAllUser );
            }

            free( pActiveXPath );
        }
    }

    return ERROR_SUCCESS;
}
