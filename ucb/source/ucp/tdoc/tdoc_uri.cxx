/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tdoc_uri.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:03:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"

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

            m_aDecodedName = decodeSegment( m_aName );

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
                m_aInternalPath = rtl::OUString::createFromAscii( "/" );
        }

        m_eState = VALID;
    }
}

//============================================================================
// static
rtl::OUString Uri::encodeURL( const rtl::OUString& rSource )
{
    rtl::OUStringBuffer aResult;

    sal_Int32 nIndex = 0;
    do
    {
        aResult.append(
            rtl::Uri::encode( rSource.getToken( 0, '/', nIndex ),
                              rtl_UriCharClassPchar,
                              rtl_UriEncodeCheckEscapes,
                              RTL_TEXTENCODING_UTF8 ) );
        if ( nIndex >= 0 )
            aResult.append( sal_Unicode( '/' ) );
    }
    while ( nIndex >= 0 );

    return aResult.makeStringAndClear();
}

//=========================================================================
// static
rtl::OUString Uri::encodeSegment( const rtl::OUString& rSource )
{
    return rtl::Uri::encode( rSource,
                             rtl_UriCharClassPchar,
                             rtl_UriEncodeIgnoreEscapes,
                             RTL_TEXTENCODING_UTF8 );
}

//=========================================================================
// static
rtl::OUString Uri::decodeSegment( const rtl::OUString& rSource )
{
    return rtl::Uri::decode( rSource,
                             rtl_UriDecodeWithCharset,
                             RTL_TEXTENCODING_UTF8 );
}
