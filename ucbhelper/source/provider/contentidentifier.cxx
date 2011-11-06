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
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <ucbhelper/contentidentifier.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/mutex.hxx>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;

namespace ucbhelper
{

//=========================================================================
//=========================================================================
//
// struct ContentIdentifier_Impl.
//
//=========================================================================
//=========================================================================

struct ContentIdentifier_Impl
{
    Reference< XMultiServiceFactory > m_xSMgr;
    OUString                          m_aContentId;
    OUString                          m_aProviderScheme;
    osl::Mutex                        m_aMutex;

    ContentIdentifier_Impl( const Reference< XMultiServiceFactory >& rSMgr,
                              const OUString& rURL );
};

//=========================================================================
//
// ContentIdentifier_Impl Implementation.
//
//=========================================================================

ContentIdentifier_Impl::ContentIdentifier_Impl(
                  const Reference< XMultiServiceFactory >& rSMgr,
                  const OUString& rURL )
: m_xSMgr( rSMgr )
{
    // Normalize URL scheme ( it's case insensitive ).

    // The content provider scheme is the part before the first ':'
    // within the content id.
    sal_Int32 nPos = rURL.indexOf( ':', 0 );
    if ( nPos != -1 )
    {
        OUString aScheme( rURL.copy( 0, nPos ) );
        m_aProviderScheme = aScheme.toAsciiLowerCase();
        m_aContentId = rURL.replaceAt( 0, nPos, aScheme );
    }
}

//=========================================================================
//
// ContentIdentifier Implementation.
//
//=========================================================================

ContentIdentifier::ContentIdentifier(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        const OUString& rURL )
{
    m_pImpl = new ContentIdentifier_Impl( rxSMgr, rURL );
}

//=========================================================================
ContentIdentifier::ContentIdentifier( const OUString& rURL )
{
    m_pImpl = new ContentIdentifier_Impl(
                    Reference< XMultiServiceFactory >(), rURL );
}

//=========================================================================
// virtual
ContentIdentifier::~ContentIdentifier()
{
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

//=========================================================================
// virtual
void SAL_CALL ContentIdentifier::acquire() throw()
{
    OWeakObject::acquire();
}

//=========================================================================
// virtual
void SAL_CALL ContentIdentifier::release() throw()
{
    OWeakObject::release();
}

//=========================================================================
// virtual
Any SAL_CALL
ContentIdentifier::queryInterface( const Type & rType )
    throw ( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                static_cast< XTypeProvider * >( this ),
                static_cast< XContentIdentifier * >( this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

// virtual
Sequence< sal_Int8 > SAL_CALL
ContentIdentifier::getImplementationId()
    throw( RuntimeException )
{
    static cppu::OImplementationId* pId = NULL;
      if ( !pId )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pId )
          {
              static cppu::OImplementationId id( sal_False );
              pId = &id;
          }
      }
      return (*pId).getImplementationId();
}

//=========================================================================
// virtual
Sequence< com::sun::star::uno::Type > SAL_CALL
ContentIdentifier::getTypes()
    throw( RuntimeException )
{
    static cppu::OTypeCollection* pCollection = NULL;
      if ( !pCollection )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                    getCppuType( static_cast<
                        Reference < XTypeProvider > * >( 0 ) ),
                    getCppuType( static_cast<
                        Reference< XContentIdentifier > * >( 0 ) ) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}

//=========================================================================
//
// XContentIdentifier methods.
//
//=========================================================================

// virtual
OUString SAL_CALL ContentIdentifier::getContentIdentifier()
    throw( RuntimeException )
{
    return m_pImpl->m_aContentId;
}

//=========================================================================
// virtual
OUString SAL_CALL ContentIdentifier::getContentProviderScheme()
    throw( RuntimeException )
{
    return m_pImpl->m_aProviderScheme;
}

} /* namespace ucbhelper */

