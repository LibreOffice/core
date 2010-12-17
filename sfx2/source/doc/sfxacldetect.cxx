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
#include "precompiled_sfx2.hxx"


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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"


#include <sal/types.h>

sal_Bool IsReadonlyAccordingACL( const sal_Unicode* )
{
    // to be implemented
    return sal_False;
}

#endif

