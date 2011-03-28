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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_automation.hxx"

////////////////////////////////////////////////////////////////////////////
////
////                  Windows ONLY
////
////////////////////////////////////////////////////////////////////////////


#include <windows.h>
#include "registry_win.hxx"
#include <osl/thread.h>
#include <sal/macros.h>
#include <windows.h>


String ReadRegistry( String aKey, String aValueName )
{
    String aRet;
    HKEY hBaseKey;
    if ( aKey.GetToken( 0, '\\' ).EqualsIgnoreCaseAscii( "HKEY_CURRENT_USER" ) )
        hBaseKey = HKEY_CURRENT_USER;
    else if ( aKey.GetToken( 0, '\\' ).EqualsIgnoreCaseAscii( "HKEY_LOCAL_MACHINE" ) )
        hBaseKey = HKEY_LOCAL_MACHINE;
    else
    {
        return aRet;
    }

    aKey.Erase( 0, aKey.Search( '\\' )+1 );

    HKEY hRegKey;

    if ( RegOpenKeyW( hBaseKey,
                reinterpret_cast<LPCWSTR>(aKey.GetBuffer()),
                &hRegKey ) == ERROR_SUCCESS )
    {
        LONG lRet;
        sal_Unicode PathW[_MAX_PATH ];
        DWORD lSize = SAL_N_ELEMENTS(PathW);
        DWORD Type = REG_SZ;

        lRet = RegQueryValueExW(hRegKey, reinterpret_cast<LPCWSTR>(aValueName.GetBuffer()), NULL, &Type, (LPBYTE)PathW, &lSize);

        if ((lRet == ERROR_SUCCESS) && (Type == REG_SZ))
        {
            aRet = String( PathW );
        }

        RegCloseKey(hRegKey);
    }
    return aRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
