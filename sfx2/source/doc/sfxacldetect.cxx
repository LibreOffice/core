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


#include "sfxacldetect.hxx"

#if EXTRA_ACL_CHECK

#ifdef WNT

// necessary to include system headers without warnings
#ifdef _MSC_VER
#pragma warning(disable:4668 4917)
#endif

#include <windows.h>
#include <lmaccess.h>
#include <sal/types.h>

sal_Bool IsReadonlyAccordingACL( const sal_Unicode* pFilePath )
{
    sal_Bool bResult = sal_False;

    sal_uInt32 nFDSize = 0;
    GetFileSecurityW( reinterpret_cast< LPCWSTR >(pFilePath), DACL_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION|OWNER_SECURITY_INFORMATION, NULL, 0, &nFDSize );
    if ( nFDSize )
    {
        PSECURITY_DESCRIPTOR pFileDescr = reinterpret_cast< PSECURITY_DESCRIPTOR >( malloc( nFDSize ) );
        if ( GetFileSecurityW( reinterpret_cast< LPCWSTR >(pFilePath), DACL_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION|OWNER_SECURITY_INFORMATION, pFileDescr, nFDSize, &nFDSize ) )
        {
            HANDLE hToken = NULL;
            if ( OpenThreadToken( GetCurrentThread(), TOKEN_DUPLICATE|TOKEN_QUERY, TRUE, &hToken )
              || OpenProcessToken( GetCurrentProcess(), TOKEN_DUPLICATE|TOKEN_QUERY, &hToken) )
            {
                HANDLE hImpersonationToken = NULL;
                if ( DuplicateToken( hToken, SecurityImpersonation, &hImpersonationToken) )
                {
                    sal_uInt32 nDesiredAccess = ACCESS_WRITE;
                    GENERIC_MAPPING aGenericMapping = { ACCESS_READ, ACCESS_WRITE, 0, ACCESS_READ | ACCESS_WRITE };
                    MapGenericMask( &nDesiredAccess, &aGenericMapping );

                    PRIVILEGE_SET aPrivilegeSet;
                    sal_uInt32 nPrivilegeSetSize = sizeof( PRIVILEGE_SET );

                    sal_uInt32 nGrantedAccess;
                    BOOL bAccessible = TRUE;
                    if ( AccessCheck( pFileDescr,
                                      hImpersonationToken,
                                      nDesiredAccess,
                                      &aGenericMapping,
                                      &aPrivilegeSet,
                                      &nPrivilegeSetSize,
                                      &nGrantedAccess,
                                      &bAccessible ) )
                    {
                        bResult = !bAccessible;
                    }

                    CloseHandle( hImpersonationToken );
                }

                CloseHandle( hToken );
            }
        }

        free( pFileDescr );
    }

    return bResult;
}

#else // this is UNX


#include <sal/types.h>

sal_Bool IsReadonlyAccordingACL( const sal_Unicode* )
{
    // to be implemented
    return sal_False;
}

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
