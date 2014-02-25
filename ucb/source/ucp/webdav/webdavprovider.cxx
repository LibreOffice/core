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
#include "webdavprovider.hxx"
#include "webdavcontent.hxx"
#include "webdavuseragent.hxx"

#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

using namespace com::sun::star;
using namespace http_dav_ucp;


OUString &WebDAVUserAgent::operator()() const
{
    OUStringBuffer aBuffer;
    aBuffer.append( "$ooName/$ooSetupVersion" );
#if OSL_DEBUG_LEVEL > 0
#ifdef APR_VERSION
    aBuffer.append( " apr/" APR_VERSION );
#endif

#ifdef APR_UTIL_VERSION
    aBuffer.append( " apr-util/" APR_UTIL_VERSION );
#endif

#ifdef SERF_VERSION
    aBuffer.append( " serf/" SERF_VERSION );
#endif
#endif
    static OUString aUserAgent( aBuffer.makeStringAndClear() );
    return aUserAgent;
}




// ContentProvider Implementation.




ContentProvider::ContentProvider(
                const uno::Reference< uno::XComponentContext >& rContext )
: ::ucbhelper::ContentProviderImplHelper( rContext ),
  m_xDAVSessionFactory( new DAVSessionFactory() ),
  m_pProps( 0 )
{
    static bool bInit = false;
    if ( bInit )
        return;
    bInit = true;
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
            rContext->getServiceManager()->createInstanceWithContext(
                OUString("com.sun.star.configuration.ConfigurationProvider"), rContext),
            uno::UNO_QUERY_THROW );

        beans::NamedValue aNodePath;
        aNodePath.Name = "nodepath";
        aNodePath.Value <<= OUString( "/org.openoffice.Setup/Product");

        uno::Sequence< uno::Any > aArgs( 1 );
        aArgs[0] <<= aNodePath;

        uno::Reference< container::XNameAccess > xConfigAccess(
            xConfigProvider->createInstanceWithArguments(
                OUString("com.sun.star.configuration.ConfigurationAccess"), aArgs),
            uno::UNO_QUERY_THROW );

        OUString aVal;
        xConfigAccess->getByName(OUString("ooName")) >>= aVal;

        OUString &aUserAgent = WebDAVUserAgent::get();
        sal_Int32 nIndex = aUserAgent.indexOf( "$ooName" );
        if ( !aVal.getLength() || nIndex == -1 )
            return;
        aUserAgent = aUserAgent.replaceAt( nIndex, RTL_CONSTASCII_LENGTH( "$ooName" ), aVal );

        xConfigAccess->getByName(OUString("ooSetupVersion")) >>= aVal;
        nIndex = aUserAgent.indexOf( "$ooSetupVersion" );
        if ( !aVal.getLength() || nIndex == -1 )
            return;
        aUserAgent = aUserAgent.replaceAt( nIndex, RTL_CONSTASCII_LENGTH( "$ooSetupVersion" ), aVal );

    }
    catch ( const uno::Exception &e )
    {
        OSL_TRACE( "ContentProvider -caught exception! %s",
                   OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        (void) e;
    }
}


// virtual
ContentProvider::~ContentProvider()
{
    delete m_pProps;
}



// XInterface methods.



XINTERFACE_IMPL_3( ContentProvider,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   ucb::XContentProvider );



// XTypeProvider methods.



XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      ucb::XContentProvider );



// XServiceInfo methods.



XSERVICEINFO_IMPL_1_CTX( ContentProvider,
                     OUString( "com.sun.star.comp.WebDAVContentProvider" ),
                     OUString( WEBDAV_CONTENT_PROVIDER_SERVICE_NAME ) );



// Service factory implementation.



ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );



// XContentProvider methods.



// virtual
uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const uno::Reference<
                    ucb::XContentIdentifier >& Identifier )
    throw( ucb::IllegalIdentifierException,
           uno::RuntimeException )
{
    // Check URL scheme...

    const OUString aScheme
        = Identifier->getContentProviderScheme().toAsciiLowerCase();
    if ( aScheme != HTTP_URL_SCHEME && aScheme != HTTPS_URL_SCHEME &&
         aScheme != WEBDAV_URL_SCHEME && aScheme != DAV_URL_SCHEME &&
         aScheme != DAVS_URL_SCHEME )
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
