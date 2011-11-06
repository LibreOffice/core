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
                m_aInternalPath = rtl::OUString::createFromAscii( "/" );
        }

        m_eState = VALID;
    }
}
