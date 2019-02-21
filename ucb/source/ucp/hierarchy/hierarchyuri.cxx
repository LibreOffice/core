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

#include <rtl/ustrbuf.hxx>

#include "hierarchyuri.hxx"

using namespace hierarchy_ucp;


#define DEFAULT_DATA_SOURCE_SERVICE \
                    "com.sun.star.ucb.DefaultHierarchyDataSource"


// HierarchyUri Implementation.


void HierarchyUri::init() const
{
    // Already inited?
    if ( !m_aUri.isEmpty() && m_aPath.isEmpty() )
    {
        // Note: Maybe it's a re-init, setUri only resets m_aPath!
        m_aService.clear();
        m_aParentUri.clear();

        // URI must match at least: <sheme>:
        if ( m_aUri.getLength() < HIERARCHY_URL_SCHEME_LENGTH + 1 )
        {
            // error, but remember that we did a init().
            m_aPath = "/";
            return;
        }

        // Scheme is case insensitive.
        OUString aScheme
            = m_aUri.copy( 0, HIERARCHY_URL_SCHEME_LENGTH ).toAsciiLowerCase();
        if ( aScheme == HIERARCHY_URL_SCHEME )
        {
            m_aUri = m_aUri.replaceAt( 0, aScheme.getLength(), aScheme );

            sal_Int32 nPos = 0;

            // If the URI has no service specifier, insert default service.
            // This is for backward compatibility and for convenience.

            if ( m_aUri.getLength() == HIERARCHY_URL_SCHEME_LENGTH + 1 )
            {
                // root folder URI without path and service specifier.
                m_aUri += "//" DEFAULT_DATA_SOURCE_SERVICE "/";
                m_aService = DEFAULT_DATA_SOURCE_SERVICE ;

                nPos = m_aUri.getLength() - 1;
            }
            else if ( ( m_aUri.getLength() == HIERARCHY_URL_SCHEME_LENGTH + 2 )
                      &&
                      ( m_aUri[ HIERARCHY_URL_SCHEME_LENGTH + 1 ] == '/' ) )
            {
                // root folder URI without service specifier.
                m_aUri += "/" DEFAULT_DATA_SOURCE_SERVICE "/";
                m_aService = DEFAULT_DATA_SOURCE_SERVICE;

                nPos = m_aUri.getLength() - 1;
            }
            else if ( ( m_aUri.getLength() > HIERARCHY_URL_SCHEME_LENGTH + 2 )
                      &&
                      ( m_aUri[ HIERARCHY_URL_SCHEME_LENGTH + 2 ] != '/' ) )
            {
                // other (no root folder) URI without service specifier.
                m_aUri = m_aUri.replaceAt(
                            HIERARCHY_URL_SCHEME_LENGTH + 2,
                            0,
                            "/" DEFAULT_DATA_SOURCE_SERVICE "/" );
                m_aService = DEFAULT_DATA_SOURCE_SERVICE;

                nPos
                    = HIERARCHY_URL_SCHEME_LENGTH + 3 + m_aService.getLength();
            }
            else
            {
                // URI with service specifier.
                sal_Int32 nStart = HIERARCHY_URL_SCHEME_LENGTH + 3;

                // Here: - m_aUri has at least the form "<scheme>://"
                //       - nStart points to char after <scheme>:

                // Only <scheme>:// ?
                if ( nStart == m_aUri.getLength() )
                {
                    // error, but remember that we did a init().
                    m_aPath = "/";
                    return;
                }

                // Empty path segments?
                if ( m_aUri.indexOf("//", nStart) != -1 )
                {
                    // error, but remember that we did a init().
                    m_aPath = "/";
                    return;
                }

                sal_Int32 nEnd = m_aUri.indexOf( '/', nStart );

                // Only <scheme>:/// ?
                if ( nEnd == nStart )
                {
                    // error, but remember that we did a init().
                    m_aPath = "/";
                    return;
                }

                if ( nEnd == -1 )
                {
                    // Trailing slash missing.
                    nEnd = m_aUri.getLength();
                    m_aUri += "/";
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
            }

            // success
            m_bValid = true;
        }
        else
        {
            // error, but remember that we did a init().
            m_aPath = "/";
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
