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

#include <sal/log.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/contenthelper.hxx>
#include <ucbhelper/getcomponentcontext.hxx>
#include <ucbhelper/macros.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include "gio_provider.hxx"
#include "gio_content.hxx"

namespace gio
{
css::uno::Reference< css::ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier )
{
    SAL_INFO("ucb.ucp.gio", "QueryContent: " << Identifier->getContentIdentifier());
    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    css::uno::Reference< css::ucb::XContent > xContent = queryExistingContent( Identifier ).get();
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
    const css::uno::Reference< css::uno::XComponentContext >& rxContext )
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
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               static_cast< css::lang::XTypeProvider* >(this),
                                               static_cast< css::lang::XServiceInfo* >(this),
                                               static_cast< css::ucb::XContentProvider* >(this)
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

XTYPEPROVIDER_IMPL_3( ContentProvider,
                      css::lang::XTypeProvider,
                      css::lang::XServiceInfo,
                      css::ucb::XContentProvider );

XSERVICEINFO_COMMOM_IMPL( ContentProvider,
                          "com.sun.star.comp.GIOContentProvider" )
/// @throws css::uno::Exception
static css::uno::Reference< css::uno::XInterface >
ContentProvider_CreateInstance( const css::uno::Reference< css::lang::XMultiServiceFactory> & rSMgr )
{
    css::lang::XServiceInfo* pX =
        static_cast<css::lang::XServiceInfo*>(new ContentProvider( ucbhelper::getComponentContext(rSMgr) ));
    return css::uno::Reference< css::uno::XInterface >::query( pX );
}

css::uno::Sequence< OUString >
ContentProvider::getSupportedServiceNames_Static()
{
    css::uno::Sequence< OUString > aSNS { "com.sun.star.ucb.GIOContentProvider" };
    return aSNS;
}

ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );

}

extern "C" SAL_DLLPUBLIC_EXPORT void * ucpgio1_component_getFactory( const sal_Char *pImplName,
    void *pServiceManager, void * )
{
    void * pRet = nullptr;

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMgr
        (static_cast< css::lang::XMultiServiceFactory * >( pServiceManager ) );
    css::uno::Reference< css::lang::XSingleServiceFactory > xFactory;
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
