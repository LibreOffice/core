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


#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/ucb/XWebDAVCommandEnvironment.hpp>

#include <ucbhelper/simpleauthenticationrequest.hxx>
#include <comphelper/seekableinput.hxx>

#include "DAVAuthListenerImpl.hxx"
#include "DAVResourceAccess.hxx"
#include "webdavprovider.hxx"

#include <officecfg/Office/Security.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <utility>

using namespace http_dav_ucp;
using namespace com::sun::star;


// DAVAuthListener_Impl Implementation.


// virtual
int DAVAuthListener_Impl::authenticate(
    const OUString & inRealm,
    const OUString & inHostName,
    OUString & inoutUserName,
    OUString & outPassWord,
    bool bCanUseSystemCredentials )
{
    if ( m_xEnv.is() )
    {
        uno::Reference< task::XInteractionHandler > xIH
            = m_xEnv->getInteractionHandler();

        if ( xIH.is() )
        {
            // Providing previously retrieved credentials will cause the password
            // container to reject these. Thus, the credential input dialog will be shown again.
            // #102871# - Supply username and password from previous try.
            // Password container service depends on this!
            if ( inoutUserName.isEmpty() )
                inoutUserName = m_aPrevUsername;

            if ( outPassWord.isEmpty() )
                outPassWord = m_aPrevPassword;

            rtl::Reference< ucbhelper::SimpleAuthenticationRequest > xRequest
                = new ucbhelper::SimpleAuthenticationRequest(
                    m_aURL, inHostName, inRealm, inoutUserName,
                    outPassWord,
                    bCanUseSystemCredentials );
            xIH->handle( xRequest );

            rtl::Reference< ucbhelper::InteractionContinuation > xSelection
                = xRequest->getSelection();

            if ( xSelection.is() )
            {
                // Handler handled the request.
                uno::Reference< task::XInteractionAbort > xAbort(
                    xSelection.get(), uno::UNO_QUERY );
                if ( !xAbort.is() )
                {
                    const rtl::Reference<
                        ucbhelper::InteractionSupplyAuthentication > & xSupp
                        = xRequest->getAuthenticationSupplier();

                    bool bUseSystemCredentials = false;

                    if ( bCanUseSystemCredentials )
                        bUseSystemCredentials
                            = xSupp->getUseSystemCredentials();

                    if ( bUseSystemCredentials )
                    {
                        // This is the (strange) way to tell neon to use
                        // system credentials.
                        inoutUserName.clear();
                        outPassWord.clear();
                    }
                    else
                    {
                        inoutUserName = xSupp->getUserName();
                        outPassWord   = xSupp->getPassword();
                    }

                    // #102871# - Remember username and password.
                    m_aPrevUsername = inoutUserName;
                    m_aPrevPassword = outPassWord;

                    // go on.
                    return 0;
                }
            }
        }
    }
    // Abort.
    return -1;
}


// DAVResourceAccess Implementation.

constexpr size_t g_nRedirectLimit = 5;

DAVResourceAccess::DAVResourceAccess(
    uno::Reference< uno::XComponentContext > xContext,
    rtl::Reference< DAVSessionFactory > xSessionFactory,
    OUString aURL )
: m_aURL(std::move( aURL )),
  m_xSessionFactory(std::move( xSessionFactory )),
  m_xContext(std::move( xContext ))
{
}


DAVResourceAccess::DAVResourceAccess( const DAVResourceAccess & rOther )
: m_aURL( rOther.m_aURL ),
  m_aPath( rOther.m_aPath ),
  m_aFlags( rOther.m_aFlags ),
  m_xSession( rOther.m_xSession ),
  m_xSessionFactory( rOther.m_xSessionFactory ),
  m_xContext( rOther.m_xContext ),
  m_aRedirectURIs( rOther.m_aRedirectURIs )
{
}


DAVResourceAccess & DAVResourceAccess::operator=(
    const DAVResourceAccess & rOther )
{
    m_aURL            = rOther.m_aURL;
    m_aPath           = rOther.m_aPath;
    m_aFlags          = rOther.m_aFlags;
    m_xSession        = rOther.m_xSession;
    m_xSessionFactory = rOther.m_xSessionFactory;
    m_xContext           = rOther.m_xContext;
    m_aRedirectURIs   = rOther.m_aRedirectURIs;

    return *this;
}

void DAVResourceAccess::OPTIONS(
    DAVOptions & rOptions,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;

            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_OPTIONS,
                                   aHeaders );

            m_xSession->OPTIONS( getRequestURI(),
                                  rOptions,
                                  DAVRequestEnvironment(
                                      new DAVAuthListener_Impl( xEnv, m_aURL ),
                                      aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

void DAVResourceAccess::PROPFIND(
    const Depth nDepth,
    const std::vector< OUString > & rPropertyNames,
    std::vector< DAVResource > & rResources,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;

            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_PROPFIND,
                                   aHeaders );

            m_xSession->PROPFIND( getRequestURI(),
                                  nDepth,
                                  rPropertyNames,
                                  rResources,
                                  DAVRequestEnvironment(
                                      new DAVAuthListener_Impl( xEnv, m_aURL ),
                                      aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}


void DAVResourceAccess::PROPFIND(
    const Depth nDepth,
    std::vector< DAVResourceInfo > & rResInfo,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_PROPFIND,
                                   aHeaders );

            m_xSession->PROPFIND( getRequestURI(),
                                  nDepth,
                                  rResInfo,
                                  DAVRequestEnvironment(
                                      new DAVAuthListener_Impl( xEnv, m_aURL ),
                                      aHeaders ) ) ;
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}


void DAVResourceAccess::PROPPATCH(
    const std::vector< ProppatchValue >& rValues,
    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    initialize();

    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_PROPPATCH,
                                   aHeaders );

            m_xSession->PROPPATCH( getRequestURI(),
                                   rValues,
                                   DAVRequestEnvironment(
                                       new DAVAuthListener_Impl( xEnv, m_aURL ),
                                       aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}


void DAVResourceAccess::HEAD(
    const std::vector< OUString > & rHeaderNames,
    DAVResource & rResource,
    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    initialize();

    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_HEAD,
                                   aHeaders );

            m_xSession->HEAD( getRequestURI(),
                              rHeaderNames,
                              rResource,
                              DAVRequestEnvironment(
                                  new DAVAuthListener_Impl( xEnv, m_aURL ),
                                  aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}


uno::Reference< io::XInputStream > DAVResourceAccess::GET(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    uno::Reference< io::XInputStream > xStream;
    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_GET,
                                   aHeaders );

            xStream = m_xSession->GET( getRequestURI(),
                                       DAVRequestEnvironment(
                                           new DAVAuthListener_Impl(
                                               xEnv, m_aURL ),
                                           aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );

    return xStream;
}


void DAVResourceAccess::GET(
    uno::Reference< io::XOutputStream > & rStream,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_GET,
                                   aHeaders );

            m_xSession->GET( getRequestURI(),
                             rStream,
                             DAVRequestEnvironment(
                                 new DAVAuthListener_Impl( xEnv, m_aURL ),
                                 aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}


uno::Reference< io::XInputStream > DAVResourceAccess::GET(
    const std::vector< OUString > & rHeaderNames,
    DAVResource & rResource,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    uno::Reference< io::XInputStream > xStream;
    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_GET,
                                   aHeaders );

            xStream = m_xSession->GET( getRequestURI(),
                                       rHeaderNames,
                                       rResource,
                                       DAVRequestEnvironment(
                                           new DAVAuthListener_Impl(
                                               xEnv, m_aURL ),
                                           aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );

    return xStream;
}


// used as HEAD substitute when HEAD is not implemented on server
void DAVResourceAccess::GET0(
    DAVRequestHeaders &rRequestHeaders,
    const std::vector< OUString > & rHeaderNames,
    DAVResource & rResource,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_GET,
                                   rRequestHeaders );

            m_xSession->GET( getRequestURI(),
                              rHeaderNames,
                              rResource,
                              DAVRequestEnvironment(
                                  new DAVAuthListener_Impl(
                                      xEnv, m_aURL ),
                                  rRequestHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}


void DAVResourceAccess::GET(
    uno::Reference< io::XOutputStream > & rStream,
    const std::vector< OUString > & rHeaderNames,
    DAVResource & rResource,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    bool bRetry;
    int errorCount = 0;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_GET,
                                   aHeaders );

            m_xSession->GET( getRequestURI(),
                             rStream,
                             rHeaderNames,
                             rResource,
                             DAVRequestEnvironment(
                                 new DAVAuthListener_Impl( xEnv, m_aURL ),
                                 aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}


void DAVResourceAccess::abort()
{
    // seems pointless to call initialize() here, but prepare for nullptr
    decltype(m_xSession) xSession;
    {
        osl::Guard<osl::Mutex> const g(m_aMutex);
        xSession = m_xSession;
    }
    if (xSession.is())
    {
        xSession->abort();
    }
}


namespace {

    /// @throws DAVException
    void resetInputStream( const uno::Reference< io::XInputStream > & rStream )
    {
        try
        {
            uno::Reference< io::XSeekable > xSeekable(
                rStream, uno::UNO_QUERY );
            if ( xSeekable.is() )
            {
                xSeekable->seek( 0 );
                return;
            }
        }
        catch ( lang::IllegalArgumentException const & )
        {
        }
        catch ( io::IOException const & )
        {
        }

        throw DAVException( DAVException::DAV_INVALID_ARG );
    }

} // namespace


void DAVResourceAccess::PUT(
    const uno::Reference< io::XInputStream > & rStream,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    // Make stream seekable, if it not. Needed, if request must be retried.
    uno::Reference< io::XInputStream > xSeekableStream
        = comphelper::OSeekableInputWrapper::CheckSeekableCanWrap(
            rStream, m_xContext );

    int errorCount = 0;
    bool bRetry = false;
    do
    {
        if ( bRetry )
            resetInputStream( xSeekableStream );

        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_PUT,
                                   aHeaders );

            m_xSession->PUT( getRequestURI(),
                             xSeekableStream,
                             DAVRequestEnvironment(
                                 new DAVAuthListener_Impl( xEnv, m_aURL ),
                                 aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}


uno::Reference< io::XInputStream > DAVResourceAccess::POST(
    const OUString & rContentType,
    const OUString & rReferer,
    const uno::Reference< io::XInputStream > & rInputStream,
    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    initialize();

    // Make stream seekable, if it not. Needed, if request must be retried.
    uno::Reference< io::XInputStream > xSeekableStream
        = comphelper::OSeekableInputWrapper::CheckSeekableCanWrap(
            rInputStream, m_xContext );

    uno::Reference< io::XInputStream > xStream;
    int errorCount = 0;
    bool bRetry = false;
    do
    {
        if ( bRetry )
        {
            resetInputStream( xSeekableStream );
            bRetry = false;
        }

        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_POST,
                                   aHeaders );

            xStream = m_xSession->POST( getRequestURI(),
                                        rContentType,
                                        rReferer,
                                        xSeekableStream,
                                        DAVRequestEnvironment(
                                            new DAVAuthListener_Impl(
                                                xEnv, m_aURL ),
                                            aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;

            if ( e.getError() == DAVException::DAV_HTTP_REDIRECT )
            {
                // #i74980# - Upon POST redirect, do a GET.
                return GET( xEnv );
            }
        }
    }
    while ( bRetry );

    return xStream;
}


void DAVResourceAccess::POST(
    const OUString & rContentType,
    const OUString & rReferer,
    const uno::Reference< io::XInputStream > & rInputStream,
    uno::Reference< io::XOutputStream > & rOutputStream,
    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    initialize();

    // Make stream seekable, if it not. Needed, if request must be retried.
    uno::Reference< io::XInputStream > xSeekableStream
        = comphelper::OSeekableInputWrapper::CheckSeekableCanWrap(
            rInputStream, m_xContext );

    int errorCount = 0;
    bool bRetry  = false;
    do
    {
        if ( bRetry )
        {
            resetInputStream( xSeekableStream );
            bRetry = false;
        }

        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_POST,
                                   aHeaders );

            m_xSession->POST( getRequestURI(),
                              rContentType,
                              rReferer,
                              xSeekableStream,
                              rOutputStream,
                              DAVRequestEnvironment(
                                  new DAVAuthListener_Impl( xEnv, m_aURL ),
                                  aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;

            if ( e.getError() == DAVException::DAV_HTTP_REDIRECT )
            {
                // #i74980# - Upon POST redirect, do a GET.
                GET( rOutputStream, xEnv );
                return;
            }
        }
    }
    while ( bRetry );
}


void DAVResourceAccess::MKCOL(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_MKCOL,
                                   aHeaders );

            m_xSession->MKCOL( getRequestURI(),
                               DAVRequestEnvironment(
                                   new DAVAuthListener_Impl( xEnv, m_aURL ),
                                   aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}


void DAVResourceAccess::COPY(
    const OUString & rSourcePath,
    const OUString & rDestinationURI,
    bool bOverwrite,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_COPY,
                                   aHeaders );

            m_xSession->COPY( rSourcePath,
                              rDestinationURI,
                              DAVRequestEnvironment(
                                  new DAVAuthListener_Impl( xEnv, m_aURL ),
                                  aHeaders ),
                              bOverwrite );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}


void DAVResourceAccess::MOVE(
    const OUString & rSourcePath,
    const OUString & rDestinationURI,
    bool bOverwrite,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_MOVE,
                                   aHeaders );

            m_xSession->MOVE( rSourcePath,
                              rDestinationURI,
                              DAVRequestEnvironment(
                                  new DAVAuthListener_Impl( xEnv, m_aURL ),
                                  aHeaders ),
                              bOverwrite );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}


void DAVResourceAccess::DESTROY(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_DELETE,
                                   aHeaders );

            m_xSession->DESTROY( getRequestURI(),
                                 DAVRequestEnvironment(
                                     new DAVAuthListener_Impl( xEnv, m_aURL ),
                                     aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}


// set new lock.
void DAVResourceAccess::LOCK(
    ucb::Lock & inLock,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_LOCK,
                                   aHeaders );

            m_xSession->LOCK( getRequestURI(),
                              inLock,
                              DAVRequestEnvironment(
                                  new DAVAuthListener_Impl( xEnv, m_aURL ),
                                  aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

void DAVResourceAccess::UNLOCK(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    initialize();

    int errorCount = 0;
    bool bRetry;
    do
    {
        bRetry = false;
        try
        {
            DAVRequestHeaders aHeaders;
            getUserRequestHeaders( xEnv,
                                   getRequestURI(),
                                   ucb::WebDAVHTTPMethod_UNLOCK,
                                   aHeaders );

            m_xSession->UNLOCK( getRequestURI(),
                                DAVRequestEnvironment(
                                    new DAVAuthListener_Impl( xEnv, m_aURL ),
                                    aHeaders ) );
        }
        catch (DAVException const& e)
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

void DAVResourceAccess::setFlags( const uno::Sequence< beans::NamedValue >& rFlags )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_aFlags = rFlags;
}

void DAVResourceAccess::setURL( const OUString & rNewURL )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_aURL  = rNewURL;
    m_aPath.clear(); // Next initialize() will create new session.
}


// init dav session and path
void DAVResourceAccess::initialize()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if ( m_aPath.isEmpty() )
    {
        CurlUri aURI(m_aURL);
        assert(aURI.GetScheme() == HTTP_URL_SCHEME || aURI.GetScheme() == HTTPS_URL_SCHEME);
        if (aURI.GetScheme() == HTTP_URL_SCHEME)
        {
            if (!officecfg::Office::Security::Net::AllowInsecureProtocols::get())
            {
                // "http" not allowed -> immediately redirect to "https",
                // better than showing confusing error to user
                aURI.SetScheme(HTTPS_URL_SCHEME);
            }
        }
        OUString aPath( aURI.GetRelativeReference() );

        /* #134089# - Check URI */
        if ( aPath.isEmpty() )
            throw DAVException( DAVException::DAV_INVALID_ARG );

        /* #134089# - Check URI */
        if ( aURI.GetHost().isEmpty() )
            throw DAVException( DAVException::DAV_INVALID_ARG );

        if ( !m_xSession.is() || !m_xSession->CanUse( m_aURL, m_aFlags ) )
        {
            m_xSession.clear();

            // create new webdav session
            m_xSession = m_xSessionFactory->createDAVSession(aURI.GetURI(), m_aFlags, m_xContext);

            if ( !m_xSession.is() )
                return;
        }

        // Own URI is needed to redirect cycle detection.
        m_aRedirectURIs.push_back( aURI );

        // Success.
        m_aPath = aPath;

        // Not only the path has to be encoded
        m_aURL = aURI.GetURI();
    }
}


const OUString & DAVResourceAccess::getRequestURI() const
{
    assert(m_xSession.is() &&
                "DAVResourceAccess::getRequestURI - Not initialized!");

    // In case a proxy is used we have to use the absolute URI for a request.
    if ( m_xSession->UsesProxy() )
        return m_aURL;

    return m_aPath;
}


// static
void DAVResourceAccess::getUserRequestHeaders(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv,
    const OUString & rURI,
    ucb::WebDAVHTTPMethod eMethod,
    DAVRequestHeaders & rRequestHeaders )
{
    if ( !xEnv.is() )
        return;

    uno::Reference< ucb::XWebDAVCommandEnvironment > xDAVEnv(
        xEnv, uno::UNO_QUERY );

    if ( !xDAVEnv.is() )
        return;

    uno::Sequence< beans::StringPair > aRequestHeaders
        = xDAVEnv->getUserRequestHeaders( rURI, eMethod );

    for ( sal_Int32 n = 0; n < aRequestHeaders.getLength(); ++n )
    {
        rRequestHeaders.push_back(
            DAVRequestHeader( aRequestHeaders[ n ].First,
                              aRequestHeaders[ n ].Second ) );
    }
}

// This function member implements the control on cyclical redirections
bool DAVResourceAccess::detectRedirectCycle(
        ::std::u16string_view const rRedirectURL)
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    CurlUri const aUri( rRedirectURL );

    // Check for maximum number of redirections
    // according to <https://tools.ietf.org/html/rfc7231#section-6.4>.
    // A practical limit may be 5, due to earlier specifications:
    // <https://tools.ietf.org/html/rfc2068#section-10.3>
    // it can be raised keeping in mind the added net activity.
    if( g_nRedirectLimit <= m_aRedirectURIs.size() )
        return true;

    // try to detect a cyclical redirection
    return std::any_of(m_aRedirectURIs.begin(), m_aRedirectURIs.end(),
        [&aUri](const CurlUri& rUri) { return aUri == rUri; });
}


void DAVResourceAccess::resetUri()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if ( ! m_aRedirectURIs.empty() )
    {
        auto const it = m_aRedirectURIs.begin();

        CurlUri const aUri( *it );
        m_aRedirectURIs.clear();
        setURL ( aUri.GetURI() );
        initialize();
    }
}


bool DAVResourceAccess::handleException(DAVException const& e, int const errorCount)
{
    switch ( e.getError() )
    {
    case DAVException::DAV_HTTP_REDIRECT:
        if ( !detectRedirectCycle( e.getData() ) )
        {
            // set new URL and path.
            setURL( e.getData() );
            initialize();
            return true;
        }
        return false;
        // i#67048 copy & paste images doesn't display. This bug refers
        // to an old OOo problem about getting resources from sites with a bad connection.
        // If we have a bad connection try again. Up to three times.
    case DAVException::DAV_HTTP_ERROR:
        // retry up to three times, if not a client-side error (4xx error codes)
        if ( e.getStatus() < SC_BAD_REQUEST && errorCount < 3 )
            return true;
        // check the server side errors
        switch( e.getStatus() )
        {
            // the HTTP server side response status codes that can be retried
            // [Serf TODO? i#119036] case SC_REQUEST_ENTITY_TOO_LARGE:
            case SC_BAD_GATEWAY:        // retry, can be an excessive load
            case SC_GATEWAY_TIMEOUT:    // retry, may be we get lucky
            case SC_SERVICE_UNAVAILABLE: // retry, the service may become available
            case SC_INSUFFICIENT_STORAGE: // space may be freed, retry
            {
                if ( errorCount < 3 )
                    return true;
                else
                    return false;
            }
            break;
            // all the other HTTP server response status codes are NOT retry
            default:
                return false;
        }
        break;
    // if connection has said retry then retry!
    case DAVException::DAV_HTTP_RETRY:
        return true;
    // <--
    default:
        return false; // Abort
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
