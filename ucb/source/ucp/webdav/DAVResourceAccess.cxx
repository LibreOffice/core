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

#include <osl/diagnose.h>

#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/ucb/XWebDAVCommandEnvironment.hpp>

#include <ucbhelper/simpleauthenticationrequest.hxx>
#include <comphelper/seekableinput.hxx>

#include "DAVAuthListenerImpl.hxx"
#include "DAVResourceAccess.hxx"

using namespace http_dav_ucp;
using namespace com::sun::star;

//=========================================================================
//=========================================================================
//
// DAVAuthListener_Impl Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
// virtual
int DAVAuthListener_Impl::authenticate(
    const OUString & inRealm,
    const OUString & inHostName,
    OUString & inoutUserName,
    OUString & outPassWord,
    sal_Bool bCanUseSystemCredentials,
    sal_Bool bUsePreviousCredentials )
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
            if ( inoutUserName.isEmpty() && bUsePreviousCredentials )
                inoutUserName = m_aPrevUsername;

            if ( outPassWord.isEmpty() && bUsePreviousCredentials )
                outPassWord = m_aPrevPassword;

            rtl::Reference< ucbhelper::SimpleAuthenticationRequest > xRequest
                = new ucbhelper::SimpleAuthenticationRequest(
                    m_aURL, inHostName, inRealm, inoutUserName,
                    outPassWord, OUString(),
                    true /*bAllowPersistentStoring*/,
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

                    sal_Bool bUseSystemCredentials = sal_False;

                    if ( bCanUseSystemCredentials )
                        bUseSystemCredentials
                            = xSupp->getUseSystemCredentials();

                    if ( bUseSystemCredentials )
                    {
                        // This is the (strange) way to tell neon to use
                        // system credentials.
                        inoutUserName = OUString();
                        outPassWord   = OUString();
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

//=========================================================================
//=========================================================================
//
// DAVResourceAccess Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
DAVResourceAccess::DAVResourceAccess(
    const uno::Reference< lang::XMultiServiceFactory > & rSMgr,
    rtl::Reference< DAVSessionFactory > const & rSessionFactory,
    const OUString & rURL )
: m_aURL( rURL ),
  m_xSessionFactory( rSessionFactory ),
  m_xSMgr( rSMgr )
{
}

//=========================================================================
DAVResourceAccess::DAVResourceAccess( const DAVResourceAccess & rOther )
: m_aURL( rOther.m_aURL ),
  m_aPath( rOther.m_aPath ),
  m_xSession( rOther.m_xSession ),
  m_xSessionFactory( rOther.m_xSessionFactory ),
  m_xSMgr( rOther.m_xSMgr ),
  m_aRedirectURIs( rOther.m_aRedirectURIs )
{
}

//=========================================================================
DAVResourceAccess & DAVResourceAccess::operator=(
    const DAVResourceAccess & rOther )
{
    m_aURL            = rOther.m_aURL;
    m_aPath           = rOther.m_aPath;
    m_xSession        = rOther.m_xSession;
    m_xSessionFactory = rOther.m_xSessionFactory;
    m_xSMgr           = rOther.m_xSMgr;
    m_aRedirectURIs   = rOther.m_aRedirectURIs;

    return *this;
}

//=========================================================================
void DAVResourceAccess::PROPFIND(
    const Depth nDepth,
    const std::vector< OUString > & rPropertyNames,
    std::vector< DAVResource > & rResources,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw( DAVException )
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
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::PROPFIND(
    const Depth nDepth,
    std::vector< DAVResourceInfo > & rResInfo,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw( DAVException )
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
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::PROPPATCH(
    const std::vector< ProppatchValue >& rValues,
    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
  throw( DAVException )
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
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::HEAD(
    const std::vector< OUString > & rHeaderNames,
    DAVResource & rResource,
    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
  throw( DAVException )
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
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
uno::Reference< io::XInputStream > DAVResourceAccess::GET(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw( DAVException )
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
        catch ( DAVException & e )
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

//=========================================================================
void DAVResourceAccess::GET(
    uno::Reference< io::XOutputStream > & rStream,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw( DAVException )
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
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
uno::Reference< io::XInputStream > DAVResourceAccess::GET(
    const std::vector< OUString > & rHeaderNames,
    DAVResource & rResource,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw( DAVException )
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
        catch ( DAVException & e )
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

//=========================================================================
uno::Reference< io::XInputStream > DAVResourceAccess::GET(
    DAVRequestHeaders &rRequestHeaders,
    const std::vector< OUString > & rHeaderNames,
    DAVResource & rResource,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw( DAVException )
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

            xStream = m_xSession->GET( getRequestURI(),
                                       rHeaderNames,
                                       rResource,
                                       DAVRequestEnvironment(
                                           getRequestURI(),
                                           new DAVAuthListener_Impl(
                                               xEnv, m_aURL ),
                                           rRequestHeaders, xEnv ) );
        }
        catch ( DAVException & e )
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

//=========================================================================
void DAVResourceAccess::GET(
    uno::Reference< io::XOutputStream > & rStream,
    const std::vector< OUString > & rHeaderNames,
    DAVResource & rResource,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw( DAVException )
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
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::abort()
  throw( DAVException )
{
    // 17.11.09 (tkr): abort currently disabled caused by issue i106766
    // initialize();
    // m_xSession->abort();
    OSL_TRACE( "Not implemented. -> #i106766#" );
}

//=========================================================================
namespace {

    void resetInputStream( const uno::Reference< io::XInputStream > & rStream )
        throw( DAVException )
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

//=========================================================================
void DAVResourceAccess::PUT(
    const uno::Reference< io::XInputStream > & rStream,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw( DAVException )
{
    initialize();

    // Make stream seekable, if it not. Needed, if request must be retried.
    uno::Reference< io::XInputStream > xSeekableStream
        = comphelper::OSeekableInputWrapper::CheckSeekableCanWrap(
            rStream, m_xSMgr );

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
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
uno::Reference< io::XInputStream > DAVResourceAccess::POST(
    const OUString & rContentType,
    const OUString & rReferer,
    const uno::Reference< io::XInputStream > & rInputStream,
    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
  throw ( DAVException )
{
    initialize();

    // Make stream seekable, if it not. Needed, if request must be retried.
    uno::Reference< io::XInputStream > xSeekableStream
        = comphelper::OSeekableInputWrapper::CheckSeekableCanWrap(
            rInputStream, m_xSMgr );

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
        catch ( DAVException & e )
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

//=========================================================================
void DAVResourceAccess::POST(
    const OUString & rContentType,
    const OUString & rReferer,
    const uno::Reference< io::XInputStream > & rInputStream,
    uno::Reference< io::XOutputStream > & rOutputStream,
    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
  throw ( DAVException )
{
    initialize();

    // Make stream seekable, if it not. Needed, if request must be retried.
    uno::Reference< io::XInputStream > xSeekableStream
        = comphelper::OSeekableInputWrapper::CheckSeekableCanWrap(
            rInputStream, m_xSMgr );

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
        catch ( DAVException & e )
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

//=========================================================================
void DAVResourceAccess::MKCOL(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw( DAVException )
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
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::COPY(
    const OUString & rSourcePath,
    const OUString & rDestinationURI,
    sal_Bool bOverwrite,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw( DAVException )
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
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::MOVE(
    const OUString & rSourcePath,
    const OUString & rDestinationURI,
    sal_Bool bOverwrite,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw( DAVException )
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
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::DESTROY(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw( DAVException )
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
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
// set new lock.
void DAVResourceAccess::LOCK(
    ucb::Lock & inLock,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw ( DAVException )
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
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

#if 0 // currently not used, but please don't remove code
//=========================================================================
// refresh existing lock.
sal_Int64 DAVResourceAccess::LOCK(
    sal_Int64 nTimeout,
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw ( DAVException )
{
    initialize();

    sal_Int64 nNewTimeout = 0;
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

            nNewTimeout = m_xSession->LOCK( getRequestURI(),
                                            nTimeout,
                                            DAVRequestEnvironment(
                                                getRequestURI(),
                                                new DAVAuthListener_Impl(
                                                    xEnv, m_aURL ),
                                            aHeaders, xEnv ) );
        }
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );

    return nNewTimeout;
}
#endif

//=========================================================================
void DAVResourceAccess::UNLOCK(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv )
  throw ( DAVException )
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
        catch ( DAVException & e )
        {
            errorCount++;
            bRetry = handleException( e, errorCount );
            if ( !bRetry )
                throw;
        }
    }
    while ( bRetry );
}

//=========================================================================
void DAVResourceAccess::setURL( const OUString & rNewURL )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_aURL  = rNewURL;
    m_aPath = OUString(); // Next initialize() will create new session.
}

//=========================================================================
// init dav session and path
void DAVResourceAccess::initialize()
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if ( m_aPath.isEmpty() )
    {
        SerfUri aURI( m_aURL );
        OUString aPath( aURI.GetPath() );

        /* #134089# - Check URI */
        if ( aPath.isEmpty() )
            throw DAVException( DAVException::DAV_INVALID_ARG );

        /* #134089# - Check URI */
        if ( aURI.GetHost().isEmpty() )
            throw DAVException( DAVException::DAV_INVALID_ARG );

        if ( !m_xSession.is() || !m_xSession->CanUse( m_aURL ) )
        {
            m_xSession.clear();

            // create new webdav session
            m_xSession
                = m_xSessionFactory->createDAVSession( m_aURL, m_xSMgr );

            if ( !m_xSession.is() )
                return;
        }

        // Own URI is needed for redirect cycle detection.
        m_aRedirectURIs.push_back( aURI );

        // Success.
        m_aPath = aPath;

        // Not only the path has to be encoded
        m_aURL = aURI.GetURI();
    }
}

//=========================================================================
const OUString & DAVResourceAccess::getRequestURI() const
{
    OSL_ENSURE( m_xSession.is(),
                "DAVResourceAccess::getRequestURI - Not initialized!" );

    // In case a proxy is used we have to use the absolute URI for a request.
    if ( m_xSession->UsesProxy() )
        return m_aURL;

    return m_aPath;
}

//=========================================================================
// static
void DAVResourceAccess::getUserRequestHeaders(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv,
    const rtl::OUString & rURI,
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

//=========================================================================
sal_Bool DAVResourceAccess::detectRedirectCycle(
                                const OUString& rRedirectURL )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    SerfUri aUri( rRedirectURL );

    std::vector< SerfUri >::const_iterator it  = m_aRedirectURIs.begin();
    std::vector< SerfUri >::const_iterator end = m_aRedirectURIs.end();

    while ( it != end )
    {
        if ( aUri == (*it) )
            return sal_True;

        ++it;
    }

    return sal_False;
}

//=========================================================================
void DAVResourceAccess::resetUri()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if ( ! m_aRedirectURIs.empty() )
    {
        std::vector< SerfUri >::const_iterator it  = m_aRedirectURIs.begin();

        SerfUri aUri( (*it) );
        m_aRedirectURIs.clear();
        setURL ( aUri.GetURI() );
        initialize();
    }
}

//=========================================================================
sal_Bool DAVResourceAccess::handleException( DAVException & e, int errorCount )
    throw ( DAVException )
{
    switch ( e.getError() )
    {
    case DAVException::DAV_HTTP_REDIRECT:
        if ( !detectRedirectCycle( e.getData() ) )
        {
            // set new URL and path.
            setURL( e.getData() );
            initialize();
            return sal_True;
        }
        return sal_False;
    // --> tkr #67048# copy & paste images doesn't display.
    // if we have a bad connection try again. Up to three times.
    case DAVException::DAV_HTTP_ERROR:
        // retry up to three times, if not a client-side error.
        if ( ( e.getStatus() < 400 || e.getStatus() >= 500 ||
               e.getStatus() == 413 ) &&
             errorCount < 3 )
        {
            return sal_True;
        }
        return sal_False;
    // <--
    // --> tkr: if connection has said retry then retry!
    case DAVException::DAV_HTTP_RETRY:
        return sal_True;
    // <--
    default:
        return sal_False; // Abort
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
