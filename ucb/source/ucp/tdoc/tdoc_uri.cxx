/*************************************************************************
 *
 *  $RCSfile: tdoc_uri.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-14 13:44:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

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
