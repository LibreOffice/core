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
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "rtl/ustrbuf.hxx"
#include "osl/diagnose.h"
#include "comphelper/storagehelper.hxx"

#include "../inc/urihelper.hxx"

#include "pkguri.hxx"

using namespace package_ucp;

using ::rtl::OUString;

//=========================================================================
//=========================================================================
//
// PackageUri Implementation.
//
//=========================================================================
//=========================================================================

static void normalize( OUString& rURL )
{
    sal_Int32 nPos = 0;
    do
    {
        nPos = rURL.indexOf( '%', nPos );
        if ( nPos != -1 )
        {
            if ( nPos < ( rURL.getLength() - 2 ) )
            {
                OUString aTmp = rURL.copy( nPos + 1, 2 );
                rURL = rURL.replaceAt( nPos + 1, 2, aTmp.toAsciiUpperCase() );
                nPos++;
            }
        }
    }
    while ( nPos != -1 );
}

//=========================================================================
void PackageUri::init() const
{
    // Already inited?
    if ( m_aUri.getLength() && !m_aPath.getLength() )
    {
        // Note: Maybe it's a re-init, setUri only resets m_aPath!
        m_aPackage = m_aParentUri = m_aName = m_aParam = m_aScheme
            = OUString();

        // URI must match at least: <sheme>://<non_empty_url_to_file>
        if ( ( m_aUri.getLength() < PACKAGE_URL_SCHEME_LENGTH + 4 ) )
        {
            // error, but remember that we did a init().
            m_aPath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) );
            return;
        }

        // Scheme must be followed by '://'
        if ( ( m_aUri.getStr()[ PACKAGE_URL_SCHEME_LENGTH ]
                != sal_Unicode( ':' ) )
             ||
             ( m_aUri.getStr()[ PACKAGE_URL_SCHEME_LENGTH + 1 ]
                != sal_Unicode( '/' ) )
             ||
             ( m_aUri.getStr()[ PACKAGE_URL_SCHEME_LENGTH + 2 ]
                != sal_Unicode( '/' ) ) )
        {
            // error, but remember that we did a init().
            m_aPath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) );
            return;
        }

        rtl::OUString aPureUri;
        sal_Int32 nParam = m_aUri.indexOf( '?' );
        if( nParam >= 0 )
        {
            m_aParam = m_aUri.copy( nParam );
            aPureUri = m_aUri.copy( 0, nParam );
        }
        else
            aPureUri = m_aUri;

        // Scheme is case insensitive.
        m_aScheme = aPureUri.copy(
            0, PACKAGE_URL_SCHEME_LENGTH ).toAsciiLowerCase();

        if ( m_aScheme.equalsAsciiL(
                 RTL_CONSTASCII_STRINGPARAM( PACKAGE_URL_SCHEME ) )
          || m_aScheme.equalsAsciiL(
              RTL_CONSTASCII_STRINGPARAM( PACKAGE_ZIP_URL_SCHEME ) ) )
        {
            if ( m_aScheme.equalsAsciiL(
                     RTL_CONSTASCII_STRINGPARAM( PACKAGE_ZIP_URL_SCHEME ) ) )
            {
                m_aParam +=
                    ( m_aParam.getLength()
                      ? ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "&purezip" ) )
                      : ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "?purezip" ) ) );
            }

            aPureUri = aPureUri.replaceAt( 0,
                                           m_aScheme.getLength(),
                                           m_aScheme );

            sal_Int32 nStart = PACKAGE_URL_SCHEME_LENGTH + 3;
            sal_Int32 nEnd   = aPureUri.lastIndexOf( '/' );
            if ( nEnd == PACKAGE_URL_SCHEME_LENGTH + 3 )
            {
                // Only <scheme>:/// - Empty authority

                // error, but remember that we did a init().
                m_aPath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) );
                return;
            }
            else if ( nEnd == ( aPureUri.getLength() - 1 ) )
            {
                if ( aPureUri.getStr()[ aPureUri.getLength() - 2 ]
                                                == sal_Unicode( '/' ) )
                {
                    // Only <scheme>://// or <scheme>://<something>//

                    // error, but remember that we did a init().
                    m_aPath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) );
                    return;
                }

                // Remove trailing slash.
                aPureUri = aPureUri.copy( 0, nEnd );
            }


            nEnd = aPureUri.indexOf( '/', nStart );
            if ( nEnd == -1 )
            {
                // root folder.

                OUString aNormPackage = aPureUri.copy( nStart );
                normalize( aNormPackage );

                aPureUri = aPureUri.replaceAt(
                    nStart, aPureUri.getLength() - nStart, aNormPackage );
                m_aPackage
                    = ::ucb_impl::urihelper::decodeSegment( aNormPackage );
                m_aPath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) );
                m_aUri = m_aUri.replaceAt( 0,
                                           ( nParam >= 0 )
                                           ? nParam
                                           : m_aUri.getLength(), aPureUri );

                sal_Int32 nLastSlash = m_aPackage.lastIndexOf( '/' );
                if ( nLastSlash != -1 )
                    m_aName = ::ucb_impl::urihelper::decodeSegment(
                        m_aPackage.copy( nLastSlash + 1 ) );
                else
                    m_aName
                        = ::ucb_impl::urihelper::decodeSegment( m_aPackage );
            }
            else
            {
                m_aPath = aPureUri.copy( nEnd + 1 );

                // Unexpected sequences of characters:
                // - empty path segments
                // - encoded slashes
                // - parent folder segments ".."
                // - current folder segments "."
                if ( m_aPath.indexOf( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "//" ) ) ) != -1
                  || m_aPath.indexOf( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "%2F" ) ) ) != -1
                  || m_aPath.indexOf( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "%2f" ) ) ) != -1
                  || ::comphelper::OStorageHelper::PathHasSegment( m_aPath, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".." ) ) )
                  || ::comphelper::OStorageHelper::PathHasSegment( m_aPath, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "." ) ) ) )
                {
                    // error, but remember that we did a init().
                    m_aPath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) );
                    return;
                }

                OUString aNormPackage = aPureUri.copy( nStart, nEnd - nStart );
                normalize( aNormPackage );

                aPureUri = aPureUri.replaceAt(
                    nStart, nEnd - nStart, aNormPackage );
                aPureUri = aPureUri.replaceAt(
                    nEnd + 1,
                    aPureUri.getLength() - nEnd - 1,
                    ::ucb_impl::urihelper::encodeURI( m_aPath ) );

                m_aPackage
                    = ::ucb_impl::urihelper::decodeSegment( aNormPackage );
                m_aPath = ::ucb_impl::urihelper::decodeSegment( m_aPath );
                m_aUri = m_aUri.replaceAt( 0,
                                           ( nParam >= 0 )
                                           ? nParam
                                           : m_aUri.getLength(), aPureUri );

                sal_Int32 nLastSlash = aPureUri.lastIndexOf( '/' );
                if ( nLastSlash != -1 )
                {
                    m_aParentUri = aPureUri.copy( 0, nLastSlash );
                    m_aName = ::ucb_impl::urihelper::decodeSegment(
                        aPureUri.copy( nLastSlash + 1 ) );
                }
            }

            // success
            m_bValid = true;
        }
        else
        {
            // error, but remember that we did a init().
            m_aPath = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
