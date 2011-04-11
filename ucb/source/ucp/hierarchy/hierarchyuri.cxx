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

#include "hierarchyuri.hxx"

using namespace hierarchy_ucp;

//=========================================================================

#define DEFAULT_DATA_SOURCE_SERVICE \
                    "com.sun.star.ucb.DefaultHierarchyDataSource"

//=========================================================================
//=========================================================================
//
// HierarchyUri Implementation.
//
//=========================================================================
//=========================================================================

void HierarchyUri::init() const
{
    // Already inited?
    if ( m_aUri.getLength() && !m_aPath.getLength() )
    {
        // Note: Maybe it's a re-init, setUri only resets m_aPath!
        m_aService = m_aParentUri = m_aName = rtl::OUString();

        // URI must match at least: <sheme>:
        if ( ( m_aUri.getLength() < HIERARCHY_URL_SCHEME_LENGTH + 1 ) )
        {
            // error, but remember that we did a init().
            m_aPath = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
            return;
        }

        // Scheme is case insensitive.
        rtl::OUString aScheme
            = m_aUri.copy( 0, HIERARCHY_URL_SCHEME_LENGTH ).toAsciiLowerCase();
        if ( aScheme.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( HIERARCHY_URL_SCHEME ) ) )
        {
            m_aUri = m_aUri.replaceAt( 0, aScheme.getLength(), aScheme );

            sal_Int32 nPos = 0;

            // If the URI has no service specifier, insert default service.
            // This is for backward compatibility and for convenience.

            if ( m_aUri.getLength() == HIERARCHY_URL_SCHEME_LENGTH + 1 )
            {
                // root folder URI without path and service specifier.
                m_aUri += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                "//" DEFAULT_DATA_SOURCE_SERVICE "/" ) );
                m_aService
                    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                            DEFAULT_DATA_SOURCE_SERVICE ) );

                nPos = m_aUri.getLength() - 1;
            }
            else if ( ( m_aUri.getLength() == HIERARCHY_URL_SCHEME_LENGTH + 2 )
                      &&
                      ( m_aUri.getStr()[ HIERARCHY_URL_SCHEME_LENGTH + 1 ]
                                                    == sal_Unicode( '/' ) ) )
            {
                // root folder URI without service specifier.
                m_aUri += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                "/" DEFAULT_DATA_SOURCE_SERVICE "/" ) );
                m_aService
                    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                            DEFAULT_DATA_SOURCE_SERVICE ) );

                nPos = m_aUri.getLength() - 1;
            }
            else if ( ( m_aUri.getLength() > HIERARCHY_URL_SCHEME_LENGTH + 2 )
                      &&
                      ( m_aUri.getStr()[ HIERARCHY_URL_SCHEME_LENGTH + 2 ]
                                                    != sal_Unicode( '/' ) ) )
            {
                // other (no root folder) URI without service specifier.
                m_aUri = m_aUri.replaceAt(
                            HIERARCHY_URL_SCHEME_LENGTH + 2,
                            0,
                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                "/" DEFAULT_DATA_SOURCE_SERVICE "/" ) ) );
                m_aService
                    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                            DEFAULT_DATA_SOURCE_SERVICE ) );

                nPos
                    = HIERARCHY_URL_SCHEME_LENGTH + 3 + m_aService.getLength();
            }
            else
            {
                // URI with service specifier.
                sal_Int32 nStart = HIERARCHY_URL_SCHEME_LENGTH + 3;

                // Here: - m_aUri has at least the form "<scheme>://"
                //       - nStart points to char after <scheme>://

                // Only <scheme>:// ?
                if ( nStart == m_aUri.getLength() )
                {
                    // error, but remember that we did a init().
                    m_aPath = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
                    return;
                }

                // Empty path segments?
                if ( m_aUri.indexOf(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("//")),
                        nStart ) != -1 )
                {
                    // error, but remember that we did a init().
                    m_aPath = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
                    return;
                }

                sal_Int32 nEnd = m_aUri.indexOf( '/', nStart );

                // Only <scheme>:/// ?
                if ( nEnd == nStart )
                {
                    // error, but remember that we did a init().
                    m_aPath = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
                    return;
                }

                if ( nEnd == -1 )
                {
                    // Trailing slash missing.
                    nEnd = m_aUri.getLength();
                    m_aUri += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
                }

                m_aService = m_aUri.copy( nStart, nEnd - nStart );

                nPos = nEnd;
            }

            // Here: - m_aUri has at least the form "<scheme>://<service>/"
            //       - m_aService was set
            //       - m_aPath, m_aParentPath, m_aName not yet set
            //       - nPos points to slash after service specifier

            // Remove trailing slash, if not a root folder URI.
            sal_Int32 nEnd = m_aUri.lastIndexOf( '/' );
            if ( ( nEnd > nPos ) && ( nEnd == ( m_aUri.getLength() - 1 ) ) )
                m_aUri = m_aUri.copy( 0, nEnd );

            // Path (includes leading slash)
            m_aPath = m_aUri.copy( nPos );

            // parent URI + name
            sal_Int32 nLastSlash = m_aUri.lastIndexOf( '/' );
            if ( ( nLastSlash != -1 ) &&
                 ( nLastSlash != m_aUri.getLength() - 1 ) ) // root
            {
                m_aParentUri = m_aUri.copy( 0, nLastSlash );
                m_aName      = m_aUri.copy( nLastSlash + 1 );
            }

            // success
            m_bValid = true;
        }
        else
        {
            // error, but remember that we did a init().
            m_aPath = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
