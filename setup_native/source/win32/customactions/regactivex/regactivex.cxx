/*************************************************************************
 *
 *  $RCSfile: regactivex.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mav $ $Date: 2004-05-25 16:17:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 ( the "License" ); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor( s ): _______________________________________
 *
 *
 ************************************************************************/

#define UNICODE

#include <windows.h>
#include <msiquery.h>
#include <string.h>
#include <malloc.h>

#define CHART_COMPONENT 1
#define DRAW_COMPONENT 2
#define IMPRESS_COMPONENT 4
#define CALC_COMPONENT 8
#define WRITER_COMPONENT 16

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

//----------------------------------------------------------
void RegisterActiveXNative( const char* pActiveXPath, int nMode, BOOL InstallForAllUser )
{
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
    if ( GetMsiProp( hMSI, L"OfficeFolder", &pProgPath ) && pProgPath )
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
        // analyze writer installation mode
        if ( current_state == INSTALLSTATE_LOCAL )
            nOldInstallMode |= WRITER_COMPONENT;

        if ( future_state == INSTALLSTATE_LOCAL )
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

        if ( future_state == INSTALLSTATE_LOCAL )
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

        if ( future_state == INSTALLSTATE_LOCAL )
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

        if ( future_state == INSTALLSTATE_LOCAL )
            nInstallMode |= IMPRESS_COMPONENT;
           else if ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_ABSENT )
            nDeinstallMode |= IMPRESS_COMPONENT;
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
extern "C" UINT InstallActiveXControl( MSIHANDLE hMSI )
{
    int nOldInstallMode = 0;
    int nInstallMode = 0;
    int nDeinstallMode = 0;

    MessageBox(NULL, L"InstallActiveXControl", L"Information", MB_OK | MB_ICONINFORMATION);

    INSTALLSTATE current_state;
    INSTALLSTATE future_state;

    if ( ERROR_SUCCESS == MsiGetFeatureState( hMSI, L"gm_o_Activexcontrol", &current_state, &future_state ) )
    {
        BOOL bInstallForAllUser = MakeInstallForAllUsers( hMSI );
        char* pActiveXPath = NULL;
        if ( GetActiveXControlPath( hMSI, &pActiveXPath ) && pActiveXPath
        && GetDelta( hMSI, nOldInstallMode, nInstallMode, nDeinstallMode ) )
        {
            if ( future_state == INSTALLSTATE_LOCAL )
            {
                // the control is installed in the new selected configuration

                if ( current_state == INSTALLSTATE_LOCAL && nDeinstallMode )
                    UnregisterActiveXNative( pActiveXPath, nInstallMode, bInstallForAllUser );

                if ( nInstallMode )
                    RegisterActiveXNative( pActiveXPath, nInstallMode, bInstallForAllUser );
            }
            else if ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_ABSENT )
            {
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
extern "C" UINT DeinstallActiveXControl( MSIHANDLE hMSI )
{
    INSTALLSTATE current_state;
    INSTALLSTATE future_state;

    MessageBox(NULL, L"DeinstallActiveXControl", L"Information", MB_OK | MB_ICONINFORMATION);

    if ( ERROR_SUCCESS == MsiGetFeatureState( hMSI, L"gm_o_Activexcontrol", &current_state, &future_state ) )
    {
        char* pActiveXPath = NULL;
        if ( current_state == INSTALLSTATE_LOCAL && GetActiveXControlPath( hMSI, &pActiveXPath ) && pActiveXPath )
        {
            BOOL bInstallForAllUser = MakeInstallForAllUsers( hMSI );

            wchar_t* rm = NULL;
            if ( GetMsiProp( hMSI, L"REMOVE", &rm ) && rm && UnicodeEquals( rm, L"ALL" ) )
            {
                UnregisterActiveXNative( pActiveXPath,
                                        CHART_COMPONENT
                                        | DRAW_COMPONENT
                                        | IMPRESS_COMPONENT
                                        | CALC_COMPONENT
                                        | WRITER_COMPONENT,
                                        bInstallForAllUser );
            }

            if ( rm )
                free( rm );

            free( pActiveXPath );
        }
    }

    return ERROR_SUCCESS;
}

