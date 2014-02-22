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
 * <http:
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <comphelper/processfactory.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include "webdavprovider.hxx"
#include "webdavcontent.hxx"

#include "osl/mutex.hxx"

using namespace com::sun::star;
using namespace webdav_ucp;



//

//



ContentProvider::ContentProvider(
                const uno::Reference< uno::XComponentContext >& rxContext )
: ::ucbhelper::ContentProviderImplHelper( rxContext ),
  m_xDAVSessionFactory( new DAVSessionFactory() ),
  m_pProps( 0 )
{
}



ContentProvider::~ContentProvider()
{
    delete m_pProps;
}


//

//


XINTERFACE_IMPL_3( ContentProvider,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   ucb::XContentProvider );


//

//


XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      ucb::XContentProvider );


//

//


XSERVICEINFO_IMPL_1_CTX( ContentProvider,
                     OUString( "com.sun.star.comp.WebDAVContentProvider" ),
                     OUString( WEBDAV_CONTENT_PROVIDER_SERVICE_NAME ) );


//

//


ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );


//

//



uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const uno::Reference<
                    ucb::XContentIdentifier >& Identifier )
    throw( ucb::IllegalIdentifierException,
           uno::RuntimeException )
{
    

    const OUString aScheme
        = Identifier->getContentProviderScheme().toAsciiLowerCase();
    if ( aScheme != HTTP_URL_SCHEME && aScheme != HTTPS_URL_SCHEME && aScheme != WEBDAV_URL_SCHEME
      && aScheme != DAV_URL_SCHEME && aScheme != DAVS_URL_SCHEME && aScheme != FTP_URL_SCHEME )
        throw ucb::IllegalIdentifierException();

    
    OUString aURL = Identifier->getContentIdentifier();

    
    if ( aURL.getLength() < ( aScheme.getLength() + 3 ) )
        throw ucb::IllegalIdentifierException();

    if ( aURL.copy( aScheme.getLength(), 3 ) != ":
        throw ucb::IllegalIdentifierException();

    uno::Reference< ucb::XContentIdentifier > xCanonicId;

    bool bNewId = false;
    if ( aScheme == WEBDAV_URL_SCHEME )
    {
        aURL = aURL.replaceAt( 0,
                               WEBDAV_URL_SCHEME_LENGTH,
                               OUString( HTTP_URL_SCHEME ) );
        bNewId = true;
    }
    else if ( aScheme == DAV_URL_SCHEME )
    {
        aURL = aURL.replaceAt( 0,
                               DAV_URL_SCHEME_LENGTH,
                               OUString( HTTP_URL_SCHEME ) );
        bNewId = true;
    }
    else if ( aScheme == DAVS_URL_SCHEME )
    {
        aURL = aURL.replaceAt( 0,
                               DAVS_URL_SCHEME_LENGTH,
                               OUString( HTTPS_URL_SCHEME ) );
        bNewId = true;
    }

    sal_Int32 nPos = aURL.lastIndexOf( '/' );
    if ( nPos != aURL.getLength() - 1 )
    {
        
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

    
    uno::Reference< ucb::XContent > xContent
        = queryExistingContent( xCanonicId ).get();
    if ( xContent.is() )
        return xContent;

    

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
