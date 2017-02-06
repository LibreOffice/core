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


#include "osl/diagnose.h"
#include "com/sun/star/io/IOException.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/ucb/XWebDAVCommandEnvironment.hpp"

#include "ucbhelper/simpleauthenticationrequest.hxx"
#include "comphelper/processfactory.hxx"
#include "comphelper/seekableinput.hxx"

#include "DAVAuthListenerImpl.hxx"
#include "DAVResourceAccess.hxx"

using namespace webdav_ucp;
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
            // #102871# - Supply username and password from previous try.
            // Password container service depends on this!
            if ( inoutUserName.isEmpty() )
                inoutUserName = m_aPrevUsername;

            if ( outPassWord.isEmpty() )
                outPassWord = m_aPrevPassword;

            rtl::Reference< ucbhelper::SimpleAuthenticationRequest > xRequest
                = new ucbhelper::SimpleAuthenticationRequest(
                    m_aURL, inHostName, inRealm, inoutUserName,
                    outPassWord, OUString(),
                    bCanUseSystemCredentials );
            xIH->handle( xRequest.get() );

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


DAVResourceAccess::DAVResourceAccess(
    const uno::Reference< uno::XComponentContext > & rxContext,
    rtl::Reference< DAVSessionFactory > const & rSessionFactory,
    const OUString & rURL )
: m_aURL( rURL ),
  m_xSessionFactory( rSessionFactory ),
  m_xContext( rxContext ),
  m_nRedirectLimit( 5 )
{
}


DAVResourceAccess::DAVResourceAccess( const DAVResourceAccess & rOther )
: m_aURL( rOther.m_aURL ),
  m_aPath( rOther.m_aPath ),
  m_aFlags( rOther.m_aFlags ),
  m_xSession( rOther.m_xSession ),
  m_xSessionFactory( rOther.m_xSessionFactory ),
  m_xContext( rOther.m_xContext ),
  m_aRedirectURIs( rOther.m_aRedirectURIs ),
  m_nRedirectLimit( rOther.m_nRedirectLimit )
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
    m_xContext        = rOther.m_xContext;
    m_aRedirectURIs   = rOther.m_aRedirectURIs;
    m_nRedirectLimit = rOther.m_nRedirectLimit;

    return *this;
}

void DAVResourceAccess::OPTIONS(
    DAVOptions & rOptions,
    const css::uno::Reference<
    css::ucb::XCommandEnvironment > & xEnv )
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
                                   css::ucb::WebDAVHTTPMethod_OPTIONS,
                                   aHeaders );

            m_xSession->OPTIONS( getRequestURI(),
                                 rOptions,
                                 DAVRequestEnvironment(
                                     getRequestURI(),
                                     new DAVAuthListener_Impl( xEnv, m_aURL ),
                                     aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
                                      getRequestURI(),
                                      new DAVAuthListener_Impl( xEnv, m_aURL ),
                                      aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
                                      getRequestURI(),
                                      new DAVAuthListener_Impl( xEnv, m_aURL ),
                                      aHeaders, xEnv ) ) ;
        }
        catch ( const DAVException & e )
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
                                       getRequestURI(),
                                       new DAVAuthListener_Impl( xEnv, m_aURL ),
                                       aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
                                  getRequestURI(),
                                  new DAVAuthListener_Impl( xEnv, m_aURL ),
                                  aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
                                           getRequestURI(),
                                           new DAVAuthListener_Impl(
                                               xEnv, m_aURL ),
                                           aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
                                 getRequestURI(),
                                 new DAVAuthListener_Impl( xEnv, m_aURL ),
                                 aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
                                           getRequestURI(),
                                           new DAVAuthListener_Impl(
                                               xEnv, m_aURL ),
                                           aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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

    uno::Reference< io::XInputStream > xStream;
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

            m_xSession->GET0( getRequestURI(),
                              rHeaderNames,
                              rResource,
                              DAVRequestEnvironment(
                                  getRequestURI(),
                                  new DAVAuthListener_Impl(
                                      xEnv, m_aURL ),
                                  rRequestHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
                                 getRequestURI(),
                                 new DAVAuthListener_Impl( xEnv, m_aURL ),
                                 aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
    initialize();
    m_xSession->abort();
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
                                 getRequestURI(),
                                 new DAVAuthListener_Impl( xEnv, m_aURL ),
                                 aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
                                            getRequestURI(),
                                            new DAVAuthListener_Impl(
                                                xEnv, m_aURL ),
                                            aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
                                  getRequestURI(),
                                  new DAVAuthListener_Impl( xEnv, m_aURL ),
                                  aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
                                   getRequestURI(),
                                   new DAVAuthListener_Impl( xEnv, m_aURL ),
                                   aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
                                  getRequestURI(),
                                  new DAVAuthListener_Impl( xEnv, m_aURL ),
                                  aHeaders, xEnv ),
                              bOverwrite );
        }
        catch ( const DAVException & e )
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
                                  getRequestURI(),
                                  new DAVAuthListener_Impl( xEnv, m_aURL ),
                                  aHeaders, xEnv ),
                              bOverwrite );
        }
        catch ( const DAVException & e )
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
                                     getRequestURI(),
                                     new DAVAuthListener_Impl( xEnv, m_aURL ),
                                     aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
                                  getRequestURI(),
                                  new DAVAuthListener_Impl( xEnv, m_aURL ),
                                  aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
                                    getRequestURI(),
                                    new DAVAuthListener_Impl( xEnv, m_aURL ),
                                    aHeaders, xEnv ) );
        }
        catch ( const DAVException & e )
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
        NeonUri aURI( m_aURL );
        OUString aPath( aURI.GetPath() );

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
            m_xSession
                = m_xSessionFactory->createDAVSession( m_aURL, m_aFlags, m_xContext );

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
    assert( m_xSession.is() && "DAVResourceAccess::getRequestURI - Not initialized!" );

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
    if ( xEnv.is() )
    {
        uno::Reference< ucb::XWebDAVCommandEnvironment > xDAVEnv(
            xEnv, uno::UNO_QUERY );

        if ( xDAVEnv.is() )
        {
            uno::Sequence< beans::StringPair > aRequestHeaders
                = xDAVEnv->getUserRequestHeaders( rURI, eMethod );

            for ( sal_Int32 n = 0; n < aRequestHeaders.getLength(); ++n )
            {
                rRequestHeaders.push_back(
                    DAVRequestHeader(
                        aRequestHeaders[ n ].First,
                        aRequestHeaders[ n ].Second ) );
            }
        }
    }

    // Make sure a User-Agent header is always included, as at least
    // en.wikipedia.org:80 forces back 403 "Scripts should use an informative
    // User-Agent string with contact information, or they may be IP-blocked
    // without notice" otherwise:
    for ( DAVRequestHeaders::iterator i(rRequestHeaders.begin());
          i != rRequestHeaders.end(); ++i )
    {
        if ( i->first.equalsIgnoreAsciiCase( "User-Agent" ) )
        {
            return;
        }
    }
    rRequestHeaders.push_back(
        DAVRequestHeader( "User-Agent", "LibreOffice" ) );
}

// This function member implements the control on cyclical redirections
bool DAVResourceAccess::detectRedirectCycle(
                                const OUString& rRedirectURL )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    NeonUri aUri( rRedirectURL );

    std::vector< NeonUri >::const_iterator it  = m_aRedirectURIs.begin();
    std::vector< NeonUri >::const_iterator end = m_aRedirectURIs.end();

    // Check for maximum number of redirections
    // according to <https://tools.ietf.org/html/rfc7231#section-6.4>.
    // A pratical limit may be 5, due to earlier specifications:
    // <https://tools.ietf.org/html/rfc2068#section-10.3>
    // it can be raised keeping in mind the added net activity.
    if( static_cast< size_t >( m_nRedirectLimit ) <= m_aRedirectURIs.size() )
        return true;

    // try to detect a cyclical redirection
    while ( it != end )
    {
        // if equal, cyclical redirection detected
        if ( aUri == (*it) )
            return true;

        ++it;
    }

    return false;
}


void DAVResourceAccess::resetUri()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if ( !m_aRedirectURIs.empty() )
    {
        std::vector< NeonUri >::const_iterator it  = m_aRedirectURIs.begin();

        NeonUri aUri( (*it) );
        m_aRedirectURIs.clear();
        setURL ( aUri.GetURI() );
        initialize();
    }
}


bool DAVResourceAccess::handleException( const DAVException & e, int errorCount )
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
        // #67048# copy & paste images doesn't display. This bug refers
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
    default:
        return false; // Abort
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
