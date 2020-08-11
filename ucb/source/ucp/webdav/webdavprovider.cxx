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
#include <ucbhelper/getcomponentcontext.hxx>
#include <ucbhelper/macros.hxx>
#include "webdavprovider.hxx"
#include "webdavcontent.hxx"

#include <cppuhelper/queryinterface.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>

#include <tools/urlobj.hxx>

using namespace com::sun::star;
using namespace http_dav_ucp;


// ContentProvider Implementation.


ContentProvider::ContentProvider(
                const uno::Reference< uno::XComponentContext >& rContext )
: ::ucbhelper::ContentProviderImplHelper( rContext ),
  m_xDAVSessionFactory( new DAVSessionFactory )
{
}


// virtual
ContentProvider::~ContentProvider()
{}


// XInterface methods.
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
    static_cast< lang::XTypeProvider* >(this),
    static_cast< lang::XServiceInfo* >(this),
    static_cast< ucb::XContentProvider* >(this)
                    );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.


XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      ucb::XContentProvider );


// XServiceInfo methods.
OUString SAL_CALL ContentProvider::getImplementationName()
{
    return getImplementationName_Static();
}

OUString ContentProvider::getImplementationName_Static()
{
    return "com.sun.star.comp.WebDAVContentProvider";
}

sal_Bool SAL_CALL ContentProvider::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL ContentProvider::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

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
    css::uno::Sequence< OUString > aSNS { WEBDAV_CONTENT_PROVIDER_SERVICE_NAME };
    return aSNS;
}

// Service factory implementation.

css::uno::Reference< css::lang::XSingleServiceFactory >
ContentProvider::createServiceFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& rxServiceMgr )
{
    return cppu::createOneInstanceFactory(
                rxServiceMgr,
                ContentProvider::getImplementationName_Static(),
                ContentProvider_CreateInstance,
                ContentProvider::getSupportedServiceNames_Static() );
}



// XContentProvider methods.


// virtual
uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const uno::Reference<
                    ucb::XContentIdentifier >& Identifier )
{
    // Check URL scheme...
    INetURLObject aURL(Identifier->getContentIdentifier());

    if (aURL.isSchemeEqualTo(INetProtocol::NotValid))
        throw ucb::IllegalIdentifierException();

    if (!aURL.isAnyKnownWebDAVScheme())
        throw ucb::IllegalIdentifierException();

    uno::Reference< ucb::XContentIdentifier > xCanonicId;

    if (aURL.isSchemeEqualTo(INetProtocol::VndSunStarWebdav) ||
        aURL.isSchemeEqualTo(DAV_URL_SCHEME) ||
        aURL.isSchemeEqualTo(WEBDAV_URL_SCHEME))
    {
        aURL.changeScheme(INetProtocol::Http);
        xCanonicId = new ::ucbhelper::ContentIdentifier( aURL.getExternalURL() );
    }
    else if (aURL.isSchemeEqualTo(VNDSUNSTARWEBDAVS_URL_SCHEME) ||
        aURL.isSchemeEqualTo(DAVS_URL_SCHEME) ||
        aURL.isSchemeEqualTo(WEBDAVS_URL_SCHEME))
    {
        aURL.changeScheme(INetProtocol::Https);
        xCanonicId = new ::ucbhelper::ContentIdentifier( aURL.getExternalURL() );
    }
    else
    {
        xCanonicId = Identifier;
    }

    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent
        = queryExistingContent( xCanonicId ).get();
    if ( xContent.is() )
        return xContent;

    // Create a new content.

    try
    {
        xContent = new ::http_dav_ucp::Content(
                        m_xContext, this, xCanonicId, m_xDAVSessionFactory );
        registerNewContent( xContent );
    }
    catch ( ucb::ContentCreationException const & )
    {
        throw ucb::IllegalIdentifierException();
    }

    if ( !xContent->getIdentifier().is() )
        throw ucb::IllegalIdentifierException();

    return xContent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
