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
#include "webdavprovider.hxx"
#include "webdavcontent.hxx"

#include "osl/mutex.hxx"

using namespace com::sun::star;
using namespace webdav_ucp;


// ContentProvider Implementation.


ContentProvider::ContentProvider(
                const uno::Reference< uno::XComponentContext >& rxContext )
: ::ucbhelper::ContentProviderImplHelper( rxContext ),
  m_xDAVSessionFactory( new DAVSessionFactory() ),
  m_pProps( nullptr )
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
    (static_cast< lang::XTypeProvider* >(this)),
    (static_cast< lang::XServiceInfo* >(this)),
    (static_cast< ucb::XContentProvider* >(this))
                    );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.


XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      ucb::XContentProvider );


// XServiceInfo methods.

XSERVICEINFO_COMMOM_IMPL( ContentProvider,
                          OUString( "com.sun.star.comp.WebDAVContentProvider" ) )
/// @throws css::uno::Exception
static css::uno::Reference< css::uno::XInterface > SAL_CALL
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


ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );


// XContentProvider methods.


// virtual
uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const uno::Reference<
                    ucb::XContentIdentifier >& Identifier )
{
    // Check URL scheme...

    const OUString aScheme
        = Identifier->getContentProviderScheme().toAsciiLowerCase();
    if ( aScheme != HTTP_URL_SCHEME && aScheme != HTTPS_URL_SCHEME
         && aScheme != WEBDAV_URL_SCHEME && aScheme != WEBDAVS_URL_SCHEME
      && aScheme != DAV_URL_SCHEME && aScheme != DAVS_URL_SCHEME && aScheme != FTP_URL_SCHEME )
        throw ucb::IllegalIdentifierException();

    // Normalize URL and create new Id, if nessacary.
    OUString aURL = Identifier->getContentIdentifier();

    // At least: <scheme> + "://"
    if ( aURL.getLength() < ( aScheme.getLength() + 3 ) )
        throw ucb::IllegalIdentifierException();

    if ( aURL.copy( aScheme.getLength(), 3 ) != "://" )
        throw ucb::IllegalIdentifierException();

    uno::Reference< ucb::XContentIdentifier > xCanonicId;

    bool bNewId = false;
    if ( aScheme == WEBDAV_URL_SCHEME )
    {
        aURL = aURL.replaceAt( 0,
                               WEBDAV_URL_SCHEME_LENGTH,
                               HTTP_URL_SCHEME );
        bNewId = true;
    }
    else if ( aScheme == WEBDAVS_URL_SCHEME )
    {
        aURL = aURL.replaceAt( 0,
                               WEBDAVS_URL_SCHEME_LENGTH,
                               HTTPS_URL_SCHEME );
        bNewId = true;
    }
    else if ( aScheme == DAV_URL_SCHEME )
    {
        aURL = aURL.replaceAt( 0,
                               DAV_URL_SCHEME_LENGTH,
                               HTTP_URL_SCHEME );
        bNewId = true;
    }
    else if ( aScheme == DAVS_URL_SCHEME )
    {
        aURL = aURL.replaceAt( 0,
                               DAVS_URL_SCHEME_LENGTH,
                               HTTPS_URL_SCHEME );
        bNewId = true;
    }

    sal_Int32 nPos = aURL.lastIndexOf( '/' );
    if ( nPos != aURL.getLength() - 1 )
    {
        // Find second slash in URL.
        nPos = aURL.indexOf( '/', aURL.indexOf( '/' ) + 1 );
        if ( nPos == -1 )
            throw ucb::IllegalIdentifierException();

        nPos = aURL.indexOf( '/', nPos + 1 );
        if ( nPos == -1 )
        {
            aURL += "/";
            bNewId = true;
        }
    }

    if ( bNewId )
        xCanonicId = new ::ucbhelper::ContentIdentifier( aURL );
    else
        xCanonicId = Identifier;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
