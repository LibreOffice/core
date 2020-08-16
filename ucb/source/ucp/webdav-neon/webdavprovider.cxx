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

#include <sal/config.h>

#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/macros.hxx>
#include <cppuhelper/queryinterface.hxx>
#include "webdavprovider.hxx"
#include "webdavcontent.hxx"

#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>

using namespace com::sun::star;
using namespace webdav_ucp;


// ContentProvider Implementation.


ContentProvider::ContentProvider(
                const uno::Reference< uno::XComponentContext >& rxContext )
: ::ucbhelper::ContentProviderImplHelper( rxContext ),
  m_xDAVSessionFactory( new DAVSessionFactory )
{
}


// virtual
ContentProvider::~ContentProvider()
{
}


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

OUString
ContentProvider::getImplementationName()
{
    return "com.sun.star.comp.WebDAVContentProvider";
}

css::uno::Sequence< OUString >
ContentProvider::getSupportedServiceNames()
{
    return { WEBDAV_CONTENT_PROVIDER_SERVICE_NAME };
}

sal_Bool
ContentProvider::supportsService(const OUString& s)
{
    return cppu::supportsService(this, s);
}

// XContentProvider methods.


// virtual
uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const uno::Reference<
                    ucb::XContentIdentifier >& Identifier )
{
    // Check URL scheme...
    INetURLObject aURL( Identifier->getContentIdentifier() );

    if ( aURL.isSchemeEqualTo( INetProtocol::NotValid ) )
        throw ucb::IllegalIdentifierException();

    if ( !aURL.isAnyKnownWebDAVScheme() )
        throw ucb::IllegalIdentifierException();

    uno::Reference< ucb::XContentIdentifier > xCanonicId;

    if (aURL.isSchemeEqualTo( INetProtocol::VndSunStarWebdav ) ||
        aURL.isSchemeEqualTo(DAV_URL_SCHEME) ||
        aURL.isSchemeEqualTo( WEBDAV_URL_SCHEME ) )
    {
        aURL.changeScheme( INetProtocol::Http );
        xCanonicId = new ::ucbhelper::ContentIdentifier( aURL.getExternalURL() );
    }
    else if ( aURL.isSchemeEqualTo( VNDSUNSTARWEBDAVS_URL_SCHEME ) ||
        aURL.isSchemeEqualTo( DAVS_URL_SCHEME ) ||
        aURL.isSchemeEqualTo( WEBDAVS_URL_SCHEME ))
    {
        aURL.changeScheme( INetProtocol::Https );
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
        xContent = new ::webdav_ucp::Content(
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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_webdav_neon_ContentProvider_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ContentProvider(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
