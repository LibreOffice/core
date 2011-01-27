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
#include "../inc/urihelper.hxx"

#include "tdoc_uri.hxx"

using namespace tdoc_ucp;

//=========================================================================
//=========================================================================
//
// Uri Implementation.
//
//=========================================================================
//=========================================================================

void Uri::init() const
{
    // Already inited?
    if ( m_eState == UNKNOWN )
    {
        m_eState = INVALID;

        // Check for proper length: must be at least length of <sheme>:/
        if ( ( m_aUri.getLength() < TDOC_URL_SCHEME_LENGTH + 2 ) )
        {
            // Invaild length (to short).
            return;
        }

        // Check for proper scheme. (Scheme is case insensitive.)
        rtl::OUString aScheme
            = m_aUri.copy( 0, TDOC_URL_SCHEME_LENGTH ).toAsciiLowerCase();
        if ( !aScheme.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( TDOC_URL_SCHEME ) ) )
        {
            // Invaild scheme.
            return;
        }

        // Remember normalized scheme string.
        m_aUri = m_aUri.replaceAt( 0, aScheme.getLength(), aScheme );

        if ( m_aUri.getStr()[ TDOC_URL_SCHEME_LENGTH ]
                != sal_Unicode( ':' ) )
        {
            // Invaild (no ':' after <scheme>).
            return;
        }

        if ( m_aUri.getStr()[ TDOC_URL_SCHEME_LENGTH + 1 ]
                != sal_Unicode( '/' ) )
        {
            // Invaild (no '/' after <scheme>:).
            return;
        }

        m_aPath = m_aUri.copy( TDOC_URL_SCHEME_LENGTH + 1 );

        // Note: There must be at least one slash; see above.
        sal_Int32 nLastSlash = m_aUri.lastIndexOf( '/' );
        bool bTrailingSlash = false;
        if ( nLastSlash == m_aUri.getLength() - 1 )
        {
            // ignore trailing slash
            bTrailingSlash = true;
            nLastSlash = m_aUri.lastIndexOf( '/', nLastSlash );
        }

        if ( nLastSlash != -1 ) // -1 is valid for the root folder
        {
            m_aParentUri = m_aUri.copy( 0, nLastSlash + 1 );

            if ( bTrailingSlash )
                m_aName = m_aUri.copy( nLastSlash + 1,
                                       m_aUri.getLength() - nLastSlash - 2 );
            else
                m_aName = m_aUri.copy( nLastSlash + 1 );

            m_aDecodedName = ::ucb_impl::urihelper::decodeSegment( m_aName );

            sal_Int32 nSlash = m_aPath.indexOf( '/', 1 );
            if ( nSlash == -1 )
                m_aDocId = m_aPath.copy( 1 );
            else
                m_aDocId = m_aPath.copy( 1, nSlash - 1 );
        }

        if ( m_aDocId.getLength() > 0 )
        {
            sal_Int32 nSlash = m_aPath.indexOf( '/', 1 );
            if ( nSlash != - 1 )
                m_aInternalPath = m_aPath.copy( nSlash );
            else
                m_aInternalPath = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
        }

        m_eState = VALID;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
