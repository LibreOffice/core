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

#include <vector>
#include <string.h>
#include <rtl/string.h>
#include "comphelper/processfactory.hxx"
#include "comphelper/sequence.hxx"
#include "ucbhelper/simplecertificatevalidationrequest.hxx"

#include "AprEnv.hxx"
#include <apr_strings.h>

#include "DAVAuthListener.hxx"
#include "SerfSession.hxx"
#include "SerfUri.hxx"
#include "SerfRequestProcessor.hxx"
#include "SerfCallbacks.hxx"
#include "SerfInputStream.hxx"
#include "UCBDeadPropertyValue.hxx"

#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/security/CertificateContainerStatus.hpp>
#include <com/sun/star/security/CertificateContainer.hpp>
#include <com/sun/star/security/XCertificateContainer.hpp>
#include <com/sun/star/security/CertAltNameEntry.hpp>
#include <com/sun/star/security/XSanExtension.hpp>
#define OID_SUBJECT_ALTERNATIVE_NAME "2.5.29.17"

#include <com/sun/star/ucb/Lock.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>

using namespace com::sun::star;
using namespace http_dav_ucp;

// Constructor

SerfSession::SerfSession(
        const rtl::Reference< DAVSessionFactory > & rSessionFactory,
        const OUString& inUri,
        const ucbhelper::InternetProxyDecider & rProxyDecider )
    throw ( DAVException )
    : DAVSession( rSessionFactory )
    , m_aMutex()
    , m_aUri( inUri )
    , m_aProxyName()
    , m_nProxyPort( 0 )
    , m_pSerfConnection( 0 )
    , m_pSerfContext( 0 )
    , m_bIsHeadRequestInProgress( false )
    , m_bUseChunkedEncoding( false )
    , m_bNoOfTransferEncodingSwitches( 0 )
    , m_rProxyDecider( rProxyDecider )
    , m_aEnv()
{
    m_pSerfContext = serf_context_create( getAprPool() );

    m_pSerfBucket_Alloc = serf_bucket_allocator_create( getAprPool(), NULL, NULL );
}


// Destructor

SerfSession::~SerfSession( )
{
    if ( m_pSerfConnection )
    {
        serf_connection_close( m_pSerfConnection );
        m_pSerfConnection = 0;
    }
}


void SerfSession::Init( const DAVRequestEnvironment & rEnv )
  throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    m_aEnv = rEnv;
    Init();
}


void SerfSession::Init()
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    bool bCreateNewSession = false;

    if ( m_pSerfConnection == 0 )
    {
        const ucbhelper::InternetProxyServer & rProxyCfg = getProxySettings();

        m_aProxyName = rProxyCfg.aName;
        m_nProxyPort = rProxyCfg.nPort;

        // Not yet initialized. Create new session.
        bCreateNewSession = true;
    }
    else
    {
        const ucbhelper::InternetProxyServer & rProxyCfg = getProxySettings();

        if ( ( rProxyCfg.aName != m_aProxyName )
             || ( rProxyCfg.nPort != m_nProxyPort ) )
        {
            m_aProxyName = rProxyCfg.aName;
            m_nProxyPort = rProxyCfg.nPort;

            // new session needed, destroy old first
            serf_connection_close( m_pSerfConnection );
            m_pSerfConnection = 0;
            bCreateNewSession = true;
        }
    }

    if ( bCreateNewSession )
    {
        // TODO - close_connection callback
        apr_status_t status = serf_connection_create2( &m_pSerfConnection,
                                                       m_pSerfContext,
                                                       m_aUri.getAprUri(),
                                                       Serf_ConnectSetup, this,
                                                       0 /* close connection callback */, 0 /* close connection baton */,
                                                       getAprPool() );

        if ( m_pSerfConnection == 0 ||status != APR_SUCCESS )
        {
            throw DAVException( DAVException::DAV_SESSION_CREATE,
                                SerfUri::makeConnectionEndPointString( m_aUri.GetHost(), m_aUri.GetPort() ) );
        }

        // Register the session with the lock store
//        m_aSerfLockStore.registerSession( m_pSerfConnection );

        if ( m_aProxyName.getLength() )
        {
            apr_sockaddr_t *proxy_address = NULL;
            status = apr_sockaddr_info_get( &proxy_address,
                                            OUStringToOString( m_aProxyName, RTL_TEXTENCODING_UTF8 ).getStr(),
                                            APR_UNSPEC,
                                            static_cast<apr_port_t>(m_nProxyPort),
                                            0, getAprPool() );

            if ( status != APR_SUCCESS )
            {
                throw DAVException( DAVException::DAV_SESSION_CREATE,
                                    SerfUri::makeConnectionEndPointString( m_aUri.GetHost(), m_aUri.GetPort() ) );
            }

            serf_config_proxy( m_pSerfContext, proxy_address );
        }


        serf_config_credentials_callback( m_pSerfContext, Serf_Credentials );

        m_bUseChunkedEncoding = isSSLNeeded();
    }
}

apr_pool_t* SerfSession::getAprPool()
{
    return apr_environment::AprEnv::getAprEnv()->getAprPool();
}

serf_bucket_alloc_t* SerfSession::getSerfBktAlloc()
{
    return m_pSerfBucket_Alloc;
}

serf_context_t* SerfSession::getSerfContext()
{
    return m_pSerfContext;
}

serf_connection_t* SerfSession::getSerfConnection()
{
    return m_pSerfConnection;
}

bool SerfSession::isHeadRequestInProgress()
{
    return m_bIsHeadRequestInProgress;
}

bool SerfSession::isSSLNeeded()
{
    return m_aUri.GetScheme().equalsIgnoreAsciiCase( "https" );
}

char* SerfSession::getHostinfo()
{
    return m_aUri.getAprUri().hostinfo;
}


// virtual
bool SerfSession::CanUse( const OUString & inUri )
{
    try
    {
        SerfUri theUri( inUri );
        if ( ( theUri.GetPort() == m_aUri.GetPort() ) &&
             ( theUri.GetHost() == m_aUri.GetHost() ) &&
             ( theUri.GetScheme() == m_aUri.GetScheme() ) )
        {
            return true;
        }
    }
    catch ( DAVException const & )
    {
        return false;
    }
    return false;
}


// virtual
bool SerfSession::UsesProxy()
{
    Init();
    return ( m_aProxyName.getLength() > 0 );
}

apr_status_t SerfSession::setupSerfConnection( apr_socket_t * inAprSocket,
                                               serf_bucket_t **outSerfInputBucket,
                                               serf_bucket_t **outSerfOutputBucket,
                                               apr_pool_t* /*inAprPool*/ )
{
    serf_bucket_t *tmpInputBkt;
    tmpInputBkt = serf_context_bucket_socket_create( getSerfContext(),
                                                     inAprSocket,
                                                     getSerfBktAlloc() );

    if ( isSSLNeeded() )
    {
        tmpInputBkt = serf_bucket_ssl_decrypt_create( tmpInputBkt,
                                                      0,
                                                      getSerfBktAlloc() );
        /** Set the callback that is called to authenticate the
            certificate (chain).
        */
        serf_ssl_server_cert_chain_callback_set(
            serf_bucket_ssl_decrypt_context_get(tmpInputBkt),
            NULL,
            Serf_CertificateChainValidation,
            this);
        serf_ssl_set_hostname( serf_bucket_ssl_decrypt_context_get( tmpInputBkt ),
                               getHostinfo() );

        *outSerfOutputBucket = serf_bucket_ssl_encrypt_create( *outSerfOutputBucket,
                                                               serf_bucket_ssl_decrypt_context_get( tmpInputBkt ),
                                                               getSerfBktAlloc() );
    }

    *outSerfInputBucket = tmpInputBkt;

    return APR_SUCCESS;
}

apr_status_t SerfSession::provideSerfCredentials( bool bGiveProvidedCredentialsASecondTry,
                                                  char ** outUsername,
                                                  char ** outPassword,
                                                  serf_request_t * /*inRequest*/,
                                                  int /*inCode*/,
                                                  const char *inAuthProtocol,
                                                  const char *inRealm,
                                                  apr_pool_t *inAprPool )
{
    DAVAuthListener * pListener = getRequestEnvironment().m_xAuthListener.get();
    if ( !pListener )
    {
        // abort
        return SERF_ERROR_AUTHN_FAILED;
    }

    OUString theUserName;
    OUString thePassWord;
    try
    {
        SerfUri uri( getRequestEnvironment().m_aRequestURI );
        OUString aUserInfo( uri.GetUserInfo() );
        if ( aUserInfo.getLength() )
        {
            sal_Int32 nPos = aUserInfo.indexOf( '@' );
            if ( nPos == -1 )
            {
                theUserName = aUserInfo;
            }
            else
            {
                theUserName = aUserInfo.copy( 0, nPos );
                thePassWord = aUserInfo.copy( nPos + 1 );
            }
        }
    }
    catch ( DAVException const & )
    {
        // abort
        return SERF_ERROR_AUTHN_FAILED;
    }

    const bool bCanUseSystemCreds = ( ( strcasecmp( inAuthProtocol, "NTLM" ) == 0 ) ||
                                      ( strcasecmp( inAuthProtocol, "Negotiate" ) == 0 ) );

    int theRetVal = pListener->authenticate( OUString::createFromAscii( inRealm ),
                                             getHostName(),
                                             theUserName,
                                             thePassWord,
                                             bCanUseSystemCreds,
                                             bGiveProvidedCredentialsASecondTry );

    if ( theRetVal == 0 )
    {
        *outUsername = apr_pstrdup( inAprPool, OUStringToOString( theUserName, RTL_TEXTENCODING_UTF8 ).getStr() );
        *outPassword = apr_pstrdup( inAprPool, OUStringToOString( thePassWord, RTL_TEXTENCODING_UTF8 ).getStr() );
    }

    return theRetVal != 0 ? SERF_ERROR_AUTHN_FAILED : APR_SUCCESS;
}

apr_status_t SerfSession::verifySerfCertificateChain (
    int,
    const serf_ssl_certificate_t * const * pCertificateChainBase64Encoded,
    int nCertificateChainLength)
{
    // Check arguments.
    if (pCertificateChainBase64Encoded == NULL || nCertificateChainLength<=0)
    {
        assert(pCertificateChainBase64Encoded != NULL);
        assert(nCertificateChainLength>0);
        return SERF_SSL_CERT_UNKNOWN_FAILURE;
    }

    // When called from SerfLockStore::~SerfLockStore(),
    // css::xml::crypto::SEInitializer::create() will fail
    // but we want to send unlock commands anyway,
    // so just ignore certificates and return here.
    if (apr_environment::AprEnv::getAprEnv()->getSerfLockStore()->finishing())
        return APR_SUCCESS;

    // Create some crypto objects to decode and handle the base64
    // encoded certificate chain.
    uno::Reference< security::XCertificateContainer > xCertificateContainer;
    uno::Reference< xml::crypto::XXMLSecurityContext > xSecurityContext;
    uno::Reference< xml::crypto::XSecurityEnvironment > xSecurityEnv;
    try
    {
        css::uno::Reference< css::uno::XComponentContext > xContext =
            ::comphelper::getProcessComponentContext();
        // Create a certificate container.
        xCertificateContainer = security::CertificateContainer::create( xContext );

        css::uno::Reference< css::xml::crypto::XSEInitializer > xSEInitializer =
            css::xml::crypto::SEInitializer::create( xContext );

        xSecurityContext = xSEInitializer->createSecurityContext( OUString() );
        if (xSecurityContext.is())
            xSecurityEnv = xSecurityContext->getSecurityEnvironment();

        if ( ! xSecurityContext.is() || ! xSecurityEnv.is())
        {
            // Do we have to dispose xSEInitializer or xCertificateContainer?
            return SERF_SSL_CERT_UNKNOWN_FAILURE;
        }
    }
    catch ( uno::Exception const &)
    {
        return SERF_SSL_CERT_UNKNOWN_FAILURE;
    }

    // Decode the server certificate.
    const char* sBase64EncodedServerCertificate (
        serf_ssl_cert_export(
            pCertificateChainBase64Encoded[0],
            getAprPool()));
    uno::Reference< security::XCertificate > xServerCertificate(
        xSecurityEnv->createCertificateFromAscii(
            OUString::createFromAscii(sBase64EncodedServerCertificate)));
    if ( ! xServerCertificate.is())
        return SERF_SSL_CERT_UNKNOWN_FAILURE;

    // Get the subject from the server certificate.
    OUString sServerCertificateSubject (xServerCertificate->getSubjectName());
    sal_Int32 nIndex = 0;
    while (nIndex >= 0)
    {
        const OUString sToken (sServerCertificateSubject.getToken(0, ',', nIndex));
        if (sToken.startsWith("CN="))
        {
            sServerCertificateSubject = sToken.copy(3);
            break;
        }
        else if (sToken.startsWith(" CN="))
        {
            sServerCertificateSubject = sToken.copy(4);
            break;
        }
    }

    // When the certificate container already contains a (trusted)
    // entry for the server then we do not have to authenticate any
    // certificate.
    const security::CertificateContainerStatus eStatus (
        xCertificateContainer->hasCertificate(
            getHostName(), sServerCertificateSubject ) );
    if (eStatus != security::CertificateContainerStatus_NOCERT)
    {
        return eStatus == security::CertificateContainerStatus_TRUSTED
               ? APR_SUCCESS
               : SERF_SSL_CERT_UNKNOWN_FAILURE;
    }

    // The shortcut failed, so try to verify the whole chain.  This is
    // done outside the isDomainMatch() block because the result is
    // used by the interaction handler.
    std::vector< uno::Reference< security::XCertificate > > aChain;
    for (nIndex = 1; nIndex < nCertificateChainLength; ++nIndex)
    {
        const char* sBase64EncodedCertificate (
            serf_ssl_cert_export(
                pCertificateChainBase64Encoded[nIndex],
                getAprPool()));
        uno::Reference< security::XCertificate > xCertificate(
            xSecurityEnv->createCertificateFromAscii(
                OUString::createFromAscii(sBase64EncodedCertificate)));
        if ( ! xCertificate.is())
            return SERF_SSL_CERT_UNKNOWN_FAILURE;
        aChain.push_back(xCertificate);
    }
    const sal_Int64 nVerificationResult (xSecurityEnv->verifyCertificate(
            xServerCertificate,
            ::comphelper::containerToSequence(aChain)));

    // When the certificate matches the host name then we can use the
    // result of the verification.
    bool bHostnameMatchesCertHostnames = false;
    {
        uno::Sequence< uno::Reference< security::XCertificateExtension > > extensions = xServerCertificate->getExtensions();
        uno::Sequence< security::CertAltNameEntry > altNames;
        for (sal_Int32 i = 0 ; i < extensions.getLength(); ++i)
        {
            uno::Reference< security::XCertificateExtension >element = extensions[i];

            const rtl::OString aId ( reinterpret_cast<const sal_Char *>(const_cast<const signed char *>(element->getExtensionId().getArray())), element->getExtensionId().getLength());
            if ( aId.equals( OID_SUBJECT_ALTERNATIVE_NAME ) )
            {
                uno::Reference< security::XSanExtension > sanExtension ( element, uno::UNO_QUERY );
                altNames =  sanExtension->getAlternativeNames();
                break;
            }
        }

        uno::Sequence< ::rtl::OUString > certHostNames(altNames.getLength() + 1);
        certHostNames[0] = sServerCertificateSubject;
        for( int n = 0; n < altNames.getLength(); ++n )
        {
            if (altNames[n].Type ==  security::ExtAltNameType_DNS_NAME)
            {
                altNames[n].Value >>= certHostNames[n+1];
            }
        }

        for ( int i = 0; i < certHostNames.getLength() && !bHostnameMatchesCertHostnames; ++i )
        {
            bHostnameMatchesCertHostnames = isDomainMatch( certHostNames[i] );
        }

    }
    if ( bHostnameMatchesCertHostnames )
    {

        if (nVerificationResult == 0)
        {
            // Certificate (chain) is valid.
            xCertificateContainer->addCertificate(getHostName(), sServerCertificateSubject,  sal_True);
            return APR_SUCCESS;
        }
        else if ((nVerificationResult & security::CertificateValidity::CHAIN_INCOMPLETE) != 0)
        {
            // We do not have enough information for verification,
            // neither automatically (as we just discovered) nor
            // manually (so there is no point in showing any dialog.)
            return SERF_SSL_CERT_UNKNOWN_FAILURE;
        }
        else if ((nVerificationResult &
                (security::CertificateValidity::INVALID | security::CertificateValidity::REVOKED)) != 0)
        {
            // Certificate (chain) is invalid.
            xCertificateContainer->addCertificate(getHostName(), sServerCertificateSubject,  sal_False);
            return SERF_SSL_CERT_UNKNOWN_FAILURE;
        }
        else
        {
            // For all other we have to ask the user.
        }
    }

    // We have not been able to automatically verify (or falsify) the
    // certificate chain.  To resolve this we have to ask the user.
    const uno::Reference< ucb::XCommandEnvironment > xEnv( getRequestEnvironment().m_xEnv );
    if ( xEnv.is() )
    {
        uno::Reference< task::XInteractionHandler > xIH( xEnv->getInteractionHandler() );
        if ( xIH.is() )
        {
            rtl::Reference< ucbhelper::SimpleCertificateValidationRequest >
                xRequest( new ucbhelper::SimpleCertificateValidationRequest(
                        static_cast<sal_Int32>(nVerificationResult), xServerCertificate, getHostName() ) );
            xIH->handle( xRequest.get() );

            rtl::Reference< ucbhelper::InteractionContinuation > xSelection
                = xRequest->getSelection();

            if ( xSelection.is() )
            {
                uno::Reference< task::XInteractionApprove > xApprove( xSelection.get(), uno::UNO_QUERY );
                if ( xApprove.is() )
                {
                    xCertificateContainer->addCertificate( getHostName(), sServerCertificateSubject,  sal_True );
                    return APR_SUCCESS;
                }
                else
                {
                    // Don't trust cert
                    xCertificateContainer->addCertificate( getHostName(), sServerCertificateSubject, sal_False );
                    return SERF_SSL_CERT_UNKNOWN_FAILURE;
                }
            }
        }
        else
        {
            // Don't trust cert
            xCertificateContainer->addCertificate( getHostName(), sServerCertificateSubject, sal_False );
            return SERF_SSL_CERT_UNKNOWN_FAILURE;
        }
    }

    return SERF_SSL_CERT_UNKNOWN_FAILURE;
}

serf_bucket_t* SerfSession::acceptSerfResponse( serf_request_t * inSerfRequest,
                                                serf_bucket_t * inSerfStreamBucket,
                                                apr_pool_t* /*inAprPool*/ )
{
    // get the per-request bucket allocator
    serf_bucket_alloc_t* SerfBktAlloc = serf_request_get_alloc( inSerfRequest );

    // create a barrier bucket so the response doesn't eat us!
    serf_bucket_t *responseBkt = serf_bucket_barrier_create( inSerfStreamBucket,
                                                             SerfBktAlloc );

    // create response bucket
    responseBkt = serf_bucket_response_create( responseBkt,
                                               SerfBktAlloc );

    if ( isHeadRequestInProgress() )
    {
        // advise the response bucket that this was from a HEAD request and that it should not expect to see a response body.
        serf_bucket_response_set_head( responseBkt );
    }

    return responseBkt;
}

SerfRequestProcessor* SerfSession::createReqProc( const OUString & inPath )
{
    return new SerfRequestProcessor( *this,
                                     inPath,
                                     m_bUseChunkedEncoding );
}


// PROPFIND - allprop & named

void SerfSession::PROPFIND( const OUString & inPath,
                            const Depth inDepth,
                            const std::vector< OUString > & inPropNames,
                            std::vector< DAVResource > & ioResources,
                            const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    apr_status_t status = APR_SUCCESS;
    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    aReqProc->processPropFind( inDepth,
                               inPropNames,
                               ioResources,
                               status );

    if ( status == APR_SUCCESS &&
         aReqProc->mpDAVException == 0 &&
         ioResources.empty() )
    {
        m_aEnv = DAVRequestEnvironment();
        throw DAVException( DAVException::DAV_HTTP_ERROR, inPath, (sal_uInt16)APR_EGENERAL );
    }
    HandleError( aReqProc );
}


// PROPFIND - propnames

void SerfSession::PROPFIND( const OUString & inPath,
                            const Depth inDepth,
                            std::vector< DAVResourceInfo > & ioResInfo,
                            const DAVRequestEnvironment & rEnv )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    apr_status_t status = APR_SUCCESS;
    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    aReqProc->processPropFind( inDepth,
                               ioResInfo,
                               status );

    if ( status == APR_SUCCESS &&
         aReqProc->mpDAVException == 0 &&
         ioResInfo.empty() )
    {
        m_aEnv = DAVRequestEnvironment();
        throw DAVException( DAVException::DAV_HTTP_ERROR, inPath, (sal_uInt16)APR_EGENERAL );
    }
    HandleError( aReqProc );
}


// PROPPATCH

void SerfSession::PROPPATCH( const OUString & inPath,
                             const std::vector< ProppatchValue > & inValues,
                             const DAVRequestEnvironment & rEnv )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    apr_status_t status = APR_SUCCESS;
    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    aReqProc->processPropPatch( inValues,
                                status );

    HandleError( aReqProc );
}


// HEAD

void SerfSession::HEAD( const OUString & inPath,
                        const std::vector< OUString > & inHeaderNames,
                        DAVResource & ioResource,
                        const DAVRequestEnvironment & rEnv )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    m_bIsHeadRequestInProgress = true;

    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    ioResource.uri = inPath;
    ioResource.properties.clear();
    apr_status_t status = APR_SUCCESS;
    aReqProc->processHead( inHeaderNames,
                           ioResource,
                           status );

    m_bIsHeadRequestInProgress = false;

    HandleError( aReqProc );
}


// GET

uno::Reference< io::XInputStream >
SerfSession::GET( const OUString & inPath,
                  const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    uno::Reference< SerfInputStream > xInputStream( new SerfInputStream );
    apr_status_t status = APR_SUCCESS;
    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    aReqProc->processGet( xInputStream,
                          status );

    HandleError( aReqProc );

    return uno::Reference< io::XInputStream >( xInputStream.get() );
}


// GET

void SerfSession::GET( const OUString & inPath,
                       uno::Reference< io::XOutputStream > & ioOutputStream,
                       const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    apr_status_t status = APR_SUCCESS;
    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    aReqProc->processGet( ioOutputStream,
                          status );

    HandleError( aReqProc );
}


// GET

uno::Reference< io::XInputStream >
SerfSession::GET( const OUString & inPath,
                  const std::vector< OUString > & inHeaderNames,
                  DAVResource & ioResource,
                  const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    uno::Reference< SerfInputStream > xInputStream( new SerfInputStream );
    ioResource.uri = inPath;
    ioResource.properties.clear();
    apr_status_t status = APR_SUCCESS;
    aReqProc->processGet( xInputStream,
                          inHeaderNames,
                          ioResource,
                          status );

    HandleError( aReqProc );

    return uno::Reference< io::XInputStream >( xInputStream.get() );
}


// GET

void SerfSession::GET( const OUString & inPath,
                       uno::Reference< io::XOutputStream > & ioOutputStream,
                       const std::vector< OUString > & inHeaderNames,
                       DAVResource & ioResource,
                       const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    ioResource.uri = inPath;
    ioResource.properties.clear();
    apr_status_t status = APR_SUCCESS;
    aReqProc->processGet( ioOutputStream,
                          inHeaderNames,
                          ioResource,
                          status );

    HandleError( aReqProc );
}


// PUT

void SerfSession::PUT( const OUString & inPath,
                       const uno::Reference< io::XInputStream > & inInputStream,
                       const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    uno::Sequence< sal_Int8 > aDataToSend;
    if ( !getDataFromInputStream( inInputStream, aDataToSend, false ) )
        throw DAVException( DAVException::DAV_INVALID_ARG );
    apr_status_t status = APR_SUCCESS;
    aReqProc->processPut( reinterpret_cast< const char * >( aDataToSend.getConstArray() ),
                          aDataToSend.getLength(),
                          status );

    HandleError( aReqProc );
}


// POST

uno::Reference< io::XInputStream >
SerfSession::POST( const OUString & inPath,
                   const OUString & rContentType,
                   const OUString & rReferer,
                   const uno::Reference< io::XInputStream > & inInputStream,
                   const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    uno::Sequence< sal_Int8 > aDataToSend;
    if ( !getDataFromInputStream( inInputStream, aDataToSend, true ) )
    {
        throw DAVException( DAVException::DAV_INVALID_ARG );
    }

    Init( rEnv );

    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    uno::Reference< SerfInputStream > xInputStream( new SerfInputStream );
    apr_status_t status = APR_SUCCESS;
    aReqProc->processPost( reinterpret_cast< const char * >( aDataToSend.getConstArray() ),
                           aDataToSend.getLength(),
                           rContentType,
                           rReferer,
                           xInputStream,
                           status );

    HandleError( aReqProc );
    return uno::Reference< io::XInputStream >( xInputStream.get() );
}


// POST

void SerfSession::POST( const OUString & inPath,
                        const OUString & rContentType,
                        const OUString & rReferer,
                        const uno::Reference< io::XInputStream > & inInputStream,
                        uno::Reference< io::XOutputStream > & oOutputStream,
                        const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    uno::Sequence< sal_Int8 > aDataToSend;
    if ( !getDataFromInputStream( inInputStream, aDataToSend, true ) )
    {
        throw DAVException( DAVException::DAV_INVALID_ARG );
    }

    Init( rEnv );

    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    apr_status_t status = APR_SUCCESS;
    aReqProc->processPost( reinterpret_cast< const char * >( aDataToSend.getConstArray() ),
                           aDataToSend.getLength(),
                           rContentType,
                           rReferer,
                           oOutputStream,
                           status );

    HandleError( aReqProc );
}


// MKCOL

void SerfSession::MKCOL( const OUString & inPath,
                         const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    apr_status_t status = APR_SUCCESS;
    aReqProc->processMkCol( status );

    HandleError( aReqProc );
}


// COPY

void SerfSession::COPY( const OUString & inSourceURL,
                        const OUString & inDestinationURL,
                        const DAVRequestEnvironment & rEnv,
                        bool inOverWrite )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    SerfUri theSourceUri( inSourceURL );
    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( theSourceUri.GetPath() ) );
    apr_status_t status = APR_SUCCESS;
    aReqProc->processCopy( inDestinationURL, inOverWrite, status );

    HandleError( aReqProc );
}


// MOVE

void SerfSession::MOVE( const OUString & inSourceURL,
                        const OUString & inDestinationURL,
                        const DAVRequestEnvironment & rEnv,
                        bool inOverWrite )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    SerfUri theSourceUri( inSourceURL );
    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( theSourceUri.GetPath() ) );
    apr_status_t status = APR_SUCCESS;
    aReqProc->processMove( inDestinationURL, inOverWrite, status );

    HandleError( aReqProc );
}


// DESTROY

void SerfSession::DESTROY( const OUString & inPath,
                           const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    apr_status_t status = APR_SUCCESS;
    aReqProc->processDelete( status );

    HandleError( aReqProc );
}


/*
namespace
{
    sal_Int32 lastChanceToSendRefreshRequest( TimeValue const & rStart,
                                              int timeout )
    {
        TimeValue aEnd;
        osl_getSystemTime( &aEnd );

        // Try to estimate a safe absolute time for sending the
        // lock refresh request.
        sal_Int32 lastChanceToSendRefreshRequest = -1;
        if ( timeout != NE_TIMEOUT_INFINITE )
        {
            sal_Int32 calltime = aEnd.Seconds - rStart.Seconds;
            if ( calltime <= timeout )
            {
                lastChanceToSendRefreshRequest
                    = aEnd.Seconds + timeout - calltime;
            }
            else
            {
                SAL_INFO("ucb.ucp.webdav",  "No chance to refresh lock before timeout!" );
            }
        }
        return lastChanceToSendRefreshRequest;
    }

} // namespace
*/

// LOCK (set new lock)

void SerfSession::LOCK( const OUString & inPath,
                        ucb::Lock & rLock,
                        const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    aReqProc->processLock( rLock );

    HandleError( aReqProc );
}


// LOCK (refresh existing lock)

sal_Int64 SerfSession::LOCK( const OUString & /*inPath*/,
                             sal_Int64 nTimeout,
                             const DAVRequestEnvironment & /*rEnv*/ )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    return nTimeout;
    /*
    // Try to get the neon lock from lock store
    SerfLock * theLock
        = m_aSerfLockStore.findByUri( makeAbsoluteURL( inPath ) );
    if ( !theLock )
         throw DAVException( DAVException::DAV_NOT_LOCKED );

    Init( rEnv );

    // refresh existing lock.
    theLock->timeout = static_cast< long >( nTimeout );

    TimeValue startCall;
    osl_getSystemTime( &startCall );

    int theRetVal = ne_lock_refresh( m_pHttpSession, theLock );

    if ( theRetVal == NE_OK )
    {
        m_aSerfLockStore.updateLock( theLock,
                                     lastChanceToSendRefreshRequest(
                                         startCall, theLock->timeout ) );
    }

    HandleError( theRetVal, inPath, rEnv );

    return theLock->timeout;
    */
}


// LOCK (refresh existing lock)

bool SerfSession::LOCK( const OUString& rLock,
                        sal_Int32 *plastChanceToSendRefreshRequest )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( rLock ) );
    aReqProc->processLock( ucb::Lock(), plastChanceToSendRefreshRequest );

    try
    {
        HandleError( aReqProc );
        SAL_INFO("ucb.ucp.webdav",  "Refreshing LOCK of " << rLock << " succeeded." );
        return true;
    }
    catch(...)
    {
        SAL_INFO("ucb.ucp.webdav",  "Refreshing LOCK of " << rLock << " failed!" );
        return false;
    }
}


// UNLOCK

void SerfSession::UNLOCK( const OUString & inPath,
                          const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( inPath ) );
    aReqProc->processUnlock();

    try
    {
        HandleError( aReqProc );
        SAL_INFO("ucb.ucp.webdav",  "UNLOCK of " << inPath << " succeeded." );
        apr_environment::AprEnv::getAprEnv()->getSerfLockStore()->removeLock( inPath );
    }
    catch(...)
    {
        SAL_INFO("ucb.ucp.webdav",  "UNLOCK of " << inPath << " failed!" );
    }
}


// UNLOCK

void SerfSession::UNLOCK( const OUString& rLock )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    std::shared_ptr<SerfRequestProcessor> aReqProc( createReqProc( rLock ) );
    aReqProc->processUnlock();

    try
    {
        HandleError( aReqProc );
        SAL_INFO("ucb.ucp.webdav",  "UNLOCK of " << rLock << " succeeded." );
    }
    catch(...)
    {
        SAL_INFO("ucb.ucp.webdav",  "UNLOCK of " << rLock << " failed!" );
    }
}


void SerfSession::abort()
    throw ( DAVException )
{
    // 11.11.09 (tkr): The following code lines causing crashes if
    // closing a ongoing connection. It turned out that this existing
    // solution doesn't work in multi-threading environments.
    // So I disabled them in 3.2. . Issue #73893# should fix it in OOo 3.3.
    //if ( m_pHttpSession )
    //    ne_close_connection( m_pHttpSession );
}


const ucbhelper::InternetProxyServer & SerfSession::getProxySettings() const
{
    if ( m_aUri.GetScheme() == "http" || m_aUri.GetScheme() == "https" )
    {
        return m_rProxyDecider.getProxy( m_aUri.GetScheme(),
                                         m_aUri.GetHost(),
                                         m_aUri.GetPort() );
    }
    else
    {
        // TODO: figure out, if this case can occur
        return m_rProxyDecider.getProxy( m_aUri.GetScheme(),
                                         OUString() /* not used */,
                                         -1 /* not used */ );
    }
}

/*

namespace {

bool containsLocktoken( const uno::Sequence< ucb::Lock > & rLocks,
                        const char * token )
{
    for ( sal_Int32 n = 0; n < rLocks.getLength(); ++n )
    {
        const uno::Sequence< OUString > & rTokens
            = rLocks[ n ].LockTokens;
        for ( sal_Int32 m = 0; m < rTokens.getLength(); ++m )
        {
            if ( rTokens[ m ].equalsAscii( token ) )
                return true;
        }
    }
    return false;
}

} // namespace
*/


bool SerfSession::removeExpiredLocktoken( const OUString & /*inURL*/,
                                          const DAVRequestEnvironment & /*rEnv*/ )
{
    return true;
    /*
    SerfLock * theLock = m_aSerfLockStore.findByUri( inURL );
    if ( !theLock )
        return false;

    // do a lockdiscovery to check whether this lock is still valid.
    try
    {
        // @@@ Alternative: use ne_lock_discover() => less overhead

        std::vector< DAVResource > aResources;
        std::vector< OUString > aPropNames;
        aPropNames.push_back( DAVProperties::LOCKDISCOVERY );

        PROPFIND( rEnv.m_aRequestURI, DAVZERO, aPropNames, aResources, rEnv );

        if ( aResources.empty() )
            return false;

        std::vector< DAVPropertyValue >::const_iterator it
            = aResources[ 0 ].properties.begin();
        std::vector< DAVPropertyValue >::const_iterator end
            = aResources[ 0 ].properties.end();

        while ( it != end )
        {
            if ( (*it).Name.equals( DAVProperties::LOCKDISCOVERY ) )
            {
                uno::Sequence< ucb::Lock > aLocks;
                if ( !( (*it).Value >>= aLocks ) )
                    return false;

                if ( !containsLocktoken( aLocks, theLock->token ) )
                {
                    // expired!
                    break;
                }

                // still valid.
                return false;
            }
            ++it;
        }

        // No lockdiscovery prop in propfind result / locktoken not found
        // in propfind result -> not locked
        SAL_INFO("ucb.ucp.webdav",  "SerfSession::removeExpiredLocktoken: Removing "
                   " expired lock token for " << inURL << ". token: " << theLock->token );

        m_aSerfLockStore.removeLock( theLock );
        ne_lock_destroy( theLock );
        return true;
    }
    catch ( DAVException const & )
    {
    }
    return false;
    */
}


// HandleError
// Common Error Handler

void SerfSession::HandleError( std::shared_ptr<SerfRequestProcessor> rReqProc )
    throw ( DAVException )
{
    m_aEnv = DAVRequestEnvironment();

    if ( rReqProc->mpDAVException )
    {
        DAVException* mpDAVExp( rReqProc->mpDAVException );

        serf_connection_reset( getSerfConnection() );

        if ( mpDAVExp->getStatus() == 413 &&
             m_bNoOfTransferEncodingSwitches < 2 )
        {
            m_bUseChunkedEncoding = !m_bUseChunkedEncoding;
            ++m_bNoOfTransferEncodingSwitches;
        }

        throw DAVException( mpDAVExp->getError(),
                            mpDAVExp->getData(),
                            mpDAVExp->getStatus() );
    }

    /*
    // Map error code to DAVException.
    switch ( nError )
    {
        case NE_OK:
            return;

        case NE_ERROR:        // Generic error
        {
            OUString aText = OUString::createFromAscii(
                ne_get_error( m_pHttpSession ) );

            sal_uInt16 code = makeStatusCode( aText );

            if ( code == SC_LOCKED )
            {
                if ( m_aSerfLockStore.findByUri(
                         makeAbsoluteURL( inPath ) ) == 0 )
                {
                    // locked by 3rd party
                    throw DAVException( DAVException::DAV_LOCKED );
                }
                else
                {
                    // locked by ourself
                    throw DAVException( DAVException::DAV_LOCKED_SELF );
                }
            }

            // Special handling for 400 and 412 status codes, which may indicate
            // that a lock previously obtained by us has been released meanwhile
            // by the server. Unfortunately, RFC is not clear at this point,
            // thus server implementations behave different...
            else if ( code == SC_BAD_REQUEST || code == SC_PRECONDITION_FAILED )
            {
                if ( removeExpiredLocktoken( makeAbsoluteURL( inPath ), rEnv ) )
                    throw DAVException( DAVException::DAV_LOCK_EXPIRED );
            }

            throw DAVException( DAVException::DAV_HTTP_ERROR, aText, code );
        }
        case NE_LOOKUP:       // Name lookup failed.
            throw DAVException( DAVException::DAV_HTTP_LOOKUP,
                                SerfUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_AUTH:         // User authentication failed on server
            throw DAVException( DAVException::DAV_HTTP_AUTH,
                                SerfUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_PROXYAUTH:    // User authentication failed on proxy
            throw DAVException( DAVException::DAV_HTTP_AUTHPROXY,
                                SerfUri::makeConnectionEndPointString(
                                    m_aProxyName, m_nProxyPort ) );

        case NE_CONNECT:      // Could not connect to server
            throw DAVException( DAVException::DAV_HTTP_CONNECT,
                                SerfUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_TIMEOUT:      // Connection timed out
            throw DAVException( DAVException::DAV_HTTP_TIMEOUT,
                                SerfUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_FAILED:       // The precondition failed
            throw DAVException( DAVException::DAV_HTTP_FAILED,
                                SerfUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_RETRY:        // Retry request (ne_end_request ONLY)
            throw DAVException( DAVException::DAV_HTTP_RETRY,
                                SerfUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_REDIRECT:
        {
            SerfUri aUri( ne_redirect_location( m_pHttpSession ) );
            throw DAVException(
                DAVException::DAV_HTTP_REDIRECT, aUri.GetURI() );
        }
        default:
        {
            SAL_INFO("ucb.ucp.webdav",  "SerfSession::HandleError : Unknown Serf error code!" );
            throw DAVException( DAVException::DAV_HTTP_ERROR,
                                OUString::createFromAscii(
                                    ne_get_error( m_pHttpSession ) ) );
        }
    }
    */
}


// static
bool
SerfSession::getDataFromInputStream(
    const uno::Reference< io::XInputStream > & xStream,
    uno::Sequence< sal_Int8 > & rData,
    bool bAppendTrailingZeroByte )
{
    if ( xStream.is() )
    {
        uno::Reference< io::XSeekable > xSeekable( xStream, uno::UNO_QUERY );
        if ( xSeekable.is() )
        {
            try
            {
                sal_Int32 nSize
                    = sal::static_int_cast<sal_Int32>(xSeekable->getLength());
                sal_Int32 nRead
                    = xStream->readBytes( rData, nSize );

                if ( nRead == nSize )
                {
                    if ( bAppendTrailingZeroByte )
                    {
                        rData.realloc( nSize + 1 );
                        rData[ nSize ] = sal_Int8( 0 );
                    }
                    return true;
                }
            }
            catch ( io::NotConnectedException const & )
            {
                // readBytes
            }
            catch ( io::BufferSizeExceededException const & )
            {
                // readBytes
            }
            catch ( io::IOException const & )
            {
                // getLength, readBytes
            }
        }
        else
        {
            try
            {
                uno::Sequence< sal_Int8 > aBuffer;
                sal_Int32 nPos = 0;

                sal_Int32 nRead = xStream->readSomeBytes( aBuffer, 65536 );
                while ( nRead > 0 )
                {
                    if ( rData.getLength() < ( nPos + nRead ) )
                        rData.realloc( nPos + nRead );

                    aBuffer.realloc( nRead );
                    memcpy( rData.getArray() + nPos, aBuffer.getConstArray(), nRead );
                    nPos += nRead;

                    aBuffer.realloc( 0 );
                    nRead = xStream->readSomeBytes( aBuffer, 65536 );
                }

                if ( bAppendTrailingZeroByte )
                {
                    rData.realloc( nPos + 1 );
                    rData[ nPos ] = sal_Int8( 0 );
                }
                return true;
            }
            catch ( io::NotConnectedException const & )
            {
                // readBytes
            }
            catch ( io::BufferSizeExceededException const & )
            {
                // readBytes
            }
            catch ( io::IOException const & )
            {
                // readBytes
            }
        }
    }
    return false;
}


bool
SerfSession::isDomainMatch( const OUString & certHostName )
{
    OUString hostName = getHostName();

    if (hostName.equalsIgnoreAsciiCase( certHostName ) )
        return true;

    if ( certHostName.startsWith( "*" ) &&
         hostName.getLength() >= certHostName.getLength()  )
    {
        OUString cmpStr = certHostName.copy( 1 );

        if ( hostName.matchIgnoreAsciiCase(
                cmpStr, hostName.getLength() -  cmpStr.getLength() ) )
            return true;
    }
    return false;
}

/*

OUString SerfSession::makeAbsoluteURL( OUString const & rURL ) const
{
    try
    {
        // Is URL relative or already absolute?
        if ( rURL[ 0 ] != '/' )
        {
            // absolute.
            return OUString( rURL );
        }
        else
        {
            ne_uri aUri;
            memset( &aUri, 0, sizeof( aUri ) );

            ne_fill_server_uri( m_pHttpSession, &aUri );
            aUri.path
                = ne_strdup( OUStringToOString(
                    rURL, RTL_TEXTENCODING_UTF8 ).getStr() );
            SerfUri aSerfUri( &aUri );
            ne_uri_free( &aUri );
            return aSerfUri.GetURI();
        }
    }
    catch ( DAVException const & )
    {
    }
    // error.
    return OUString();
}
*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
