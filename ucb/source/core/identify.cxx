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

#include "identify.hxx"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;

//=========================================================================
//
// ContentIdentifier Implementation.
//
//=========================================================================

ContentIdentifier::ContentIdentifier(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        const OUString& ContentId )
: m_xSMgr( rxSMgr ),
  m_aContentId( ContentId )
{
}

//=========================================================================
// virtual
ContentIdentifier::~ContentIdentifier()
{
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_2( ContentIdentifier,
                   XTypeProvider,
                   XContentIdentifier );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( ContentIdentifier,
                      XTypeProvider,
                      XContentIdentifier );

//=========================================================================
//
// XContentIdentifier methods.
//
//=========================================================================

// virtual
OUString SAL_CALL ContentIdentifier::getContentIdentifier()
    throw( RuntimeException )
{
    return m_aContentId;
}

//=========================================================================
// virtual
OUString SAL_CALL ContentIdentifier::getContentProviderScheme()
    throw( RuntimeException )
{
    if ( !m_aProviderScheme.getLength() && m_aContentId.getLength() )
    {
        // The content provider scheme is the part before the first ':'
        // within the content id.
        sal_Int32 nPos = m_aContentId.indexOf( ':', 0 );
        if ( nPos != -1 )
        {
            OUString aScheme( m_aContentId.copy( 0, nPos ) );
            m_aProviderScheme = aScheme.toAsciiLowerCase();
        }
    }

    return m_aProviderScheme;
}

