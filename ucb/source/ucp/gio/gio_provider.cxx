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

#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/contenthelper.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include "gio_provider.hxx"
#include "gio_content.hxx"

using namespace com::sun::star;

namespace gio
{
uno::Reference< css::ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const uno::Reference< css::ucb::XContentIdentifier >& Identifier )
    throw( css::ucb::IllegalIdentifierException,
           uno::RuntimeException, std::exception )
{
    SAL_INFO("ucb.ucp.gio", "QueryContent: " << Identifier->getContentIdentifier());
    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent = queryExistingContent( Identifier ).get();
    if ( xContent.is() )
        return xContent;

    try
    {
        xContent = new ::gio::Content(m_xContext, this, Identifier);
    }
    catch ( css::ucb::ContentCreationException const & )
    {
        throw css::ucb::IllegalIdentifierException();
    }

    if ( !xContent->getIdentifier().is() )
        throw css::ucb::IllegalIdentifierException();

    return xContent;
}

ContentProvider::ContentProvider(
    const uno::Reference< uno::XComponentContext >& rxContext )
: ::ucbhelper::ContentProviderImplHelper( rxContext )
{
}

ContentProvider::~ContentProvider()
{
}

// XInterface
void SAL_CALL ContentProvider::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ContentProvider::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL ContentProvider::queryInterface( const css::uno::Type & rType )
    throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< lang::XTypeProvider* >(this)),
                                               (static_cast< lang::XServiceInfo* >(this)),
                                               (static_cast< css::ucb::XContentProvider* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      css::ucb::XContentProvider );

XSERVICEINFO_IMPL_1_CTX( ContentProvider,
                     OUString( "com.sun.star.comp.GIOContentProvider" ),
                     "com.sun.star.ucb.GIOContentProvider" );

ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL ucpgio1_component_getFactory( const sal_Char *pImplName,
    void *pServiceManager, void * )
{
    void * pRet = 0;

    uno::Reference< lang::XMultiServiceFactory > xSMgr
        (static_cast< lang::XMultiServiceFactory * >( pServiceManager ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;
#if !GLIB_CHECK_VERSION(2,36,0)
    g_type_init();
#endif
    if ( ::gio::ContentProvider::getImplementationName_Static().equalsAscii( pImplName ) )
        xFactory = ::gio::ContentProvider::createServiceFactory( xSMgr );

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
