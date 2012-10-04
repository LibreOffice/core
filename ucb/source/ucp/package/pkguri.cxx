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
    if ( !m_aUri.isEmpty() && m_aPath.isEmpty() )
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

        if ( m_aScheme == PACKAGE_URL_SCHEME || m_aScheme == PACKAGE_ZIP_URL_SCHEME )
        {
            if ( m_aScheme == PACKAGE_ZIP_URL_SCHEME )
            {
                m_aParam +=
                    ( !m_aParam.isEmpty()
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
                if ( m_aPath.indexOf( "//" ) != -1
                  || m_aPath.indexOf( "%2F" ) != -1
                  || m_aPath.indexOf( "%2f" ) != -1
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
