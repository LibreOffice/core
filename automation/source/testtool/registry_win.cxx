/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_automation.hxx"

////////////////////////////////////////////////////////////////////////////
////
////                  Windows ONLY
////
////////////////////////////////////////////////////////////////////////////


#include <tools/prewin.h>
#include "winreg.h"
#include <tools/postwin.h>
// as we define it ourselves further down the line we remove it here
#ifdef IS_ERROR
//    #undef IS_ERROR
#endif

//#include <tchar.h>
#include "registry_win.hxx"
#include <osl/thread.h>


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
        sal_Unicode PathW[_MAX_PATH];
        DWORD lSize = sizeof(PathW) / sizeof( sal_Unicode );
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


