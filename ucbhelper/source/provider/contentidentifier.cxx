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
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <ucbhelper/contentidentifier.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/mutex.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;

using ::rtl::OUString;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
