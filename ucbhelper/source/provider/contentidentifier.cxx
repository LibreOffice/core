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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <ucbhelper/contentidentifier.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <osl/mutex.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;


namespace ucbhelper
{




// struct ContentIdentifier_Impl.




struct ContentIdentifier_Impl
{
    OUString                          m_aContentId;
    OUString                          m_aProviderScheme;
    osl::Mutex                        m_aMutex;

    explicit ContentIdentifier_Impl( const OUString& rURL );
};



// ContentIdentifier_Impl Implementation.



ContentIdentifier_Impl::ContentIdentifier_Impl(const OUString& rURL )
{
    // Normalize URL scheme ( it's case insensitive ).

    // The content provider scheme is the part before the first ':'
    // within the content id.
    sal_Int32 nPos = rURL.indexOf( ':' );
    if ( nPos != -1 )
    {
        OUString aScheme( rURL.copy( 0, nPos ) );
        m_aProviderScheme = aScheme.toAsciiLowerCase();
        m_aContentId = rURL.replaceAt( 0, nPos, aScheme );
    }
}



// ContentIdentifier Implementation.



ContentIdentifier::ContentIdentifier( const OUString& rURL )
{
    m_pImpl = new ContentIdentifier_Impl( rURL );
}


// virtual
ContentIdentifier::~ContentIdentifier()
{
    delete m_pImpl;
}



// XInterface methods.




// virtual
void SAL_CALL ContentIdentifier::acquire() throw()
{
    OWeakObject::acquire();
}


// virtual
void SAL_CALL ContentIdentifier::release() throw()
{
    OWeakObject::release();
}


// virtual
Any SAL_CALL
ContentIdentifier::queryInterface( const Type & rType )
    throw ( RuntimeException, std::exception )
{
    Any aRet = cppu::queryInterface( rType,
                static_cast< XTypeProvider * >( this ),
                static_cast< XContentIdentifier * >( this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}



// XTypeProvider methods.



// virtual
Sequence< sal_Int8 > SAL_CALL
ContentIdentifier::getImplementationId()
    throw( RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}


// virtual
Sequence< css::uno::Type > SAL_CALL
ContentIdentifier::getTypes()
    throw( RuntimeException, std::exception )
{
    static cppu::OTypeCollection* pCollection = nullptr;
      if ( !pCollection )
      {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                    cppu::UnoType<XTypeProvider>::get(),
                    cppu::UnoType<XContentIdentifier>::get() );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}



// XContentIdentifier methods.



// virtual
OUString SAL_CALL ContentIdentifier::getContentIdentifier()
    throw( RuntimeException, std::exception )
{
    return m_pImpl->m_aContentId;
}


// virtual
OUString SAL_CALL ContentIdentifier::getContentProviderScheme()
    throw( RuntimeException, std::exception )
{
    return m_pImpl->m_aProviderScheme;
}

} /* namespace ucbhelper */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
