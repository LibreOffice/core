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
#pragma warning(push,1) // disable warnings within system headers
#endif
#include <windows.h>
#include <../tools/msiprop.hxx>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <string.h>
#include <malloc.h>
#include <stdio.h>

//----------------------------------------------------------
BOOL MakeInstallForAllUsers( MSIHANDLE hMSI )
{
    BOOL bResult = FALSE;
    LPTSTR* pVal = NULL;
    if ( GetMsiProp( hMSI, TEXT("ALLUSERS"), pVal ) )
    {
        bResult = ( int (pVal) == 1 );
        free( pVal );
    }

    return bResult;
}

//----------------------------------------------------------
extern "C" UINT __stdcall PatchActiveXControl( MSIHANDLE hMSI )
{
    // MessageBox(NULL, L"PatchActiveXControl", L"Information", MB_OK | MB_ICONINFORMATION);

    INSTALLSTATE current_state;
    INSTALLSTATE future_state;

    if ( ERROR_SUCCESS == MsiGetFeatureState( hMSI, L"gm_o_Activexcontrol", &current_state, &future_state ) )
    {
        BOOL bInstallForAllUsers = MakeInstallForAllUsers( hMSI );

        if ( future_state == INSTALLSTATE_LOCAL
          || ( current_state == INSTALLSTATE_LOCAL && future_state == INSTALLSTATE_UNKNOWN ) )
        {
            HKEY hkey = NULL;
            char* aSubKey = "Software\\Classes\\MIME\\DataBase\\Content Type\\application/vnd.sun.xml.base";
               if ( ERROR_SUCCESS == RegCreateKeyA(bInstallForAllUsers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, aSubKey, &hkey) )
            {
                   RegDeleteValueA( hkey, "CLSID" );
                RegCloseKey( hkey ), hkey = NULL;
            }
        }
    }
    else
    {
        // assert( FALSE );
    }

    return ERROR_SUCCESS;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
