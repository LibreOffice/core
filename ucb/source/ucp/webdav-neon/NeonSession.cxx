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


#include <unordered_map>
#include <vector>
#include <string.h>
#include "osl/diagnose.h"
#include "osl/time.h"
#include <rtl/string.h>
#include <ne_socket.h>
#include <ne_auth.h>
#include <ne_redirect.h>
#include <ne_ssl.h>

// old neon versions forgot to set this
extern "C" {
#include <ne_compress.h>
}

#include "libxml/parser.h"
#include "rtl/ustrbuf.hxx"
#include "comphelper/processfactory.hxx"
#include "comphelper/sequence.hxx"
#include "ucbhelper/simplecertificatevalidationrequest.hxx"

#include "DAVAuthListener.hxx"
#include "NeonTypes.hxx"
#include "NeonSession.hxx"
#include "NeonInputStream.hxx"
#include "NeonPropFindRequest.hxx"
#include "NeonHeadRequest.hxx"
#include "NeonUri.hxx"
#include "LinkSequence.hxx"
#include "UCBDeadPropertyValue.hxx"

#include <officecfg/Inet.hxx>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/security/CertificateContainerStatus.hpp>
#include <com/sun/star/security/CertificateContainer.hpp>
#include <com/sun/star/security/XCertificateContainer.hpp>
#include <com/sun/star/ucb/Lock.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>


using namespace com::sun::star;
using namespace webdav_ucp;

#ifndef EOL
#    define EOL "\r\n"
#endif

struct RequestData
{
    // POST
    OUString aContentType;
    OUString aReferer;

    RequestData() {}
    RequestData( const OUString & rContentType,
                 const OUString & rReferer )
    : aContentType( rContentType ), aReferer( rReferer ) {}
};

struct equalPtr
{
    bool operator()( const ne_request* p1, const ne_request* p2 ) const
    {
        return p1 == p2;
    }
};

struct hashPtr
{
    size_t operator()( const ne_request* p ) const
    {
        return reinterpret_cast<size_t>(p);
    }
};

typedef std::unordered_map
<
    ne_request*,
    RequestData,
    hashPtr,
    equalPtr
>
RequestDataMap;

static sal_uInt16 makeStatusCode( const OUString & rStatusText )
{
    // Extract status code from session error string. Unfortunately
    // neon provides no direct access to the status code...

    if ( rStatusText.getLength() < 3 )
    {
        SAL_WARN( "ucb.ucp.webdav", "makeStatusCode - status text string to short!" );
        return 0;
    }

    sal_Int32 nPos = rStatusText.indexOf( ' ' );
    if ( nPos == -1 )
    {
        SAL_WARN( "ucb.ucp.webdav", "makeStatusCode - wrong status text format!" );
        return 0;
    }

    return sal_uInt16( rStatusText.copy( 0, nPos ).toInt32() );
}

static bool noKeepAlive( const uno::Sequence< beans::NamedValue >& rFlags )
{
    if ( !rFlags.hasElements() )
        return false;

    // find "KeepAlive" property
    const beans::NamedValue* pAry(rFlags.getConstArray());
    const sal_Int32          nLen(rFlags.getLength());
    const beans::NamedValue* pValue(
        std::find_if(pAry,pAry+nLen,
            [] (beans::NamedValue const& rNV) { return rNV.Name == "KeepAlive"; } ));
    if ( pValue != pAry+nLen && !pValue->Value.get<bool>() )
        return true;

    return false;
}

struct NeonRequestContext
{
    uno::Reference< io::XOutputStream >    xOutputStream;
    rtl::Reference< NeonInputStream >      xInputStream;
    const std::vector< OUString > * pHeaderNames;
    DAVResource *                          pResource;

    explicit NeonRequestContext( uno::Reference< io::XOutputStream > & xOutStrm )
    : xOutputStream( xOutStrm ), xInputStream( nullptr ),
      pHeaderNames( nullptr ), pResource( nullptr ) {}

    explicit NeonRequestContext( const rtl::Reference< NeonInputStream > & xInStrm )
    : xOutputStream( nullptr ), xInputStream( xInStrm ),
      pHeaderNames( nullptr ), pResource( nullptr ) {}

    NeonRequestContext( uno::Reference< io::XOutputStream > & xOutStrm,
                        const std::vector< OUString > & inHeaderNames,
                        DAVResource & ioResource )
    : xOutputStream( xOutStrm ), xInputStream( nullptr ),
      pHeaderNames( &inHeaderNames ), pResource( &ioResource ) {}

    NeonRequestContext( const rtl::Reference< NeonInputStream > & xInStrm,
                        const std::vector< OUString > & inHeaderNames,
                        DAVResource & ioResource )
    : xOutputStream( nullptr ), xInputStream( xInStrm ),
      pHeaderNames( &inHeaderNames ), pResource( &ioResource ) {}
};

// A simple Neon response_block_reader for use with an XInputStream
extern "C" int NeonSession_ResponseBlockReader(void * inUserData,
                                               const char * inBuf,
                                               size_t inLen )
{
    // neon sometimes calls this function with (inLen == 0)...
    if ( inLen > 0 )
    {
        NeonRequestContext * pCtx
            = static_cast< NeonRequestContext * >( inUserData );

        rtl::Reference< NeonInputStream > xInputStream(
            pCtx->xInputStream );

        if ( xInputStream.is() )
            xInputStream->AddToStream( inBuf, inLen );
    }
    return 0;
}

// A simple Neon response_block_reader for use with an XOutputStream
extern "C" int NeonSession_ResponseBlockWriter( void * inUserData,
                                                const char * inBuf,
                                                size_t inLen )
{
    // neon calls this function with (inLen == 0)...
    if ( inLen > 0 )
    {
        NeonRequestContext * pCtx
            = static_cast< NeonRequestContext * >( inUserData );
        uno::Reference< io::XOutputStream > xOutputStream
            = pCtx->xOutputStream;

        if ( xOutputStream.is() )
        {
            const uno::Sequence< sal_Int8 > aSeq( reinterpret_cast<sal_Int8 const *>(inBuf), inLen );
            xOutputStream->writeBytes( aSeq );
        }
    }
    return 0;
}

extern "C" int NeonSession_NeonAuth( void *       inUserData,
#if defined NE_FEATURE_SSPI && ! defined SYSTEM_NEON
                                     const char * inAuthProtocol,
#endif
                                     const char * inRealm,
                                     int          attempt,
                                     char *       inoutUserName,
                                     char *       inoutPassWord )
{
/* The callback used to request the username and password in the given
 * realm. The username and password must be copied into the buffers
 * which are both of size NE_ABUFSIZ.  The 'attempt' parameter is zero
 * on the first call to the callback, and increases by one each time
 * an attempt to authenticate fails.
 *
 * The callback must return zero to indicate that authentication
 * should be attempted with the username/password, or non-zero to
 * cancel the request. (if non-zero, username and password are
 * ignored.)  */

    NeonSession * theSession = static_cast< NeonSession * >( inUserData );
    DAVAuthListener * pListener
        = theSession->getRequestEnvironment().m_xAuthListener.get();
    if ( !pListener )
    {
        // abort
        return -1;
    }
    OUString theUserName;
    OUString thePassWord;

    if ( attempt == 0 )
    {
        // neon does not handle username supplied with request URI (for
        // instance when doing FTP over proxy - last checked: 0.23.5 )

        try
        {
            NeonUri uri( theSession->getRequestEnvironment().m_aRequestURI );
            OUString aUserInfo( uri.GetUserInfo() );
            if ( !aUserInfo.isEmpty() )
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
            return -1;
        }
    }
    else
    {
        // username buffer is prefilled with user name from last attempt.
        theUserName = OUString::createFromAscii( inoutUserName );
        // @@@ Neon does not initialize password buffer (last checked: 0.22.0).
        //thePassWord = OUString::createFromAscii( inoutPassWord );
    }

    bool bCanUseSystemCreds = false;

#if defined NE_FEATURE_SSPI && ! defined SYSTEM_NEON
    bCanUseSystemCreds
        = (attempt == 0) && // avoid endless loops
          ne_has_support( NE_FEATURE_SSPI ) && // Windows-only feature.
          ( ( ne_strcasecmp( inAuthProtocol, "NTLM" ) == 0 ) ||
            ( ne_strcasecmp( inAuthProtocol, "Negotiate" ) == 0 ) );
#endif

    int theRetVal = pListener->authenticate(
                            OUString::createFromAscii( inRealm ),
                            theSession->getHostName(),
                            theUserName,
                            thePassWord,
                            bCanUseSystemCreds);

    OString aUser(
        OUStringToOString( theUserName, RTL_TEXTENCODING_UTF8 ) );
    if ( aUser.getLength() > ( NE_ABUFSIZ - 1 ) )
    {
        SAL_WARN( "ucb.ucp.webdav", "NeonSession_NeonAuth - username too long!" );
        return -1;
    }

    OString aPass(
        OUStringToOString( thePassWord, RTL_TEXTENCODING_UTF8 ) );
    if ( aPass.getLength() > ( NE_ABUFSIZ - 1 ) )
    {
        SAL_WARN( "ucb.ucp.webdav", "NeonSession_NeonAuth - password too long!" );
        return -1;
    }

    strcpy( inoutUserName, // #100211# - checked
            OUStringToOString( theUserName, RTL_TEXTENCODING_UTF8 ).getStr() );

    strcpy( inoutPassWord, // #100211# - checked
            OUStringToOString( thePassWord, RTL_TEXTENCODING_UTF8 ).getStr() );

    return theRetVal;
}

namespace {
    OUString GetHostnamePart( const OUString& _rRawString )
    {
        OUString sPart;
        OUString sPartId("CN=");
        sal_Int32 nContStart = _rRawString.indexOf( sPartId );
        if ( nContStart != -1 )
        {
            nContStart += sPartId.getLength();
            sal_Int32 nContEnd = _rRawString.indexOf( ',', nContStart );
            sPart = nContEnd != -1 ?
                _rRawString.copy(nContStart, nContEnd - nContStart) :
                _rRawString.copy(nContStart);
        }
        return sPart;
    }
} // namespace

extern "C" int NeonSession_CertificationNotify( void *userdata,
                                                int,
                                                const ne_ssl_certificate *cert )
{
    OSL_ASSERT( cert );

    NeonSession * pSession = static_cast< NeonSession * >( userdata );
    uno::Reference< security::XCertificateContainer > xCertificateContainer;
    try
    {
        xCertificateContainer = security::CertificateContainer::create( pSession->getComponentContext() );
    }
    catch ( uno::Exception const & )
    {
    }

    if ( !xCertificateContainer.is() )
        return 1;

    char * dn = ne_ssl_readable_dname( ne_ssl_cert_subject( cert ) );
    OUString cert_subject( dn, strlen( dn ), RTL_TEXTENCODING_UTF8, 0 );

    ne_free( dn );

    security::CertificateContainerStatus certificateContainer(
        xCertificateContainer->hasCertificate(
            pSession->getHostName(), cert_subject ) );

    if ( certificateContainer != security::CertificateContainerStatus_NOCERT )
        return
            certificateContainer == security::CertificateContainerStatus_TRUSTED
            ? 0
            : 1;

    uno::Reference< xml::crypto::XSEInitializer > xSEInitializer;
    try
    {
        xSEInitializer = xml::crypto::SEInitializer::create( pSession->getComponentContext() );
    }
    catch ( uno::Exception const & )
    {
    }

    if ( !xSEInitializer.is() )
        return 1;

    uno::Reference< xml::crypto::XXMLSecurityContext > xSecurityContext(
        xSEInitializer->createSecurityContext( OUString() ) );

    uno::Reference< xml::crypto::XSecurityEnvironment > xSecurityEnv(
        xSecurityContext->getSecurityEnvironment() );

    //The end entity certificate
    char * eeCertB64 = ne_ssl_cert_export( cert );

    OString sEECertB64( eeCertB64 );

    uno::Reference< security::XCertificate > xEECert(
        xSecurityEnv->createCertificateFromAscii(
            OStringToOUString( sEECertB64, RTL_TEXTENCODING_ASCII_US ) ) );

    ne_free( eeCertB64 );
    eeCertB64 = nullptr;

    std::vector< uno::Reference< security::XCertificate > > vecCerts;
    const ne_ssl_certificate * issuerCert = cert;
    do
    {
        //get the intermediate certificate
        //the returned value is const ! Therefore it does not need to be freed
        //with ne_ssl_cert_free, which takes a non-const argument
        issuerCert = ne_ssl_cert_signedby( issuerCert );
        if ( nullptr == issuerCert )
            break;

        char * imCertB64 = ne_ssl_cert_export( issuerCert );
        OString sInterMediateCertB64( imCertB64 );
        ne_free( imCertB64 );

        uno::Reference< security::XCertificate> xImCert(
            xSecurityEnv->createCertificateFromAscii(
                OStringToOUString( sInterMediateCertB64, RTL_TEXTENCODING_ASCII_US ) ) );
        if ( xImCert.is() )
            vecCerts.push_back( xImCert );
    }
    while ( true );

    sal_Int64 certValidity = xSecurityEnv->verifyCertificate( xEECert,
        ::comphelper::containerToSequence( vecCerts ) );

    if ( pSession->isDomainMatch(
        GetHostnamePart( xEECert.get()->getSubjectName() ) ) )
    {
        // if host name matched with certificate then look if the
        // certificate was ok
        if( certValidity == security::CertificateValidity::VALID )
            return 0;
    }

    const uno::Reference< ucb::XCommandEnvironment > xEnv(
        pSession->getRequestEnvironment().m_xEnv );
    if ( xEnv.is() )
    {
        uno::Reference< task::XInteractionHandler > xIH(
            xEnv->getInteractionHandler() );
        if ( xIH.is() )
        {
            rtl::Reference< ucbhelper::SimpleCertificateValidationRequest >
                xRequest( new ucbhelper::SimpleCertificateValidationRequest(
                    (sal_Int32)certValidity, xEECert, pSession->getHostName() ) );
            xIH->handle( xRequest.get() );

            rtl::Reference< ucbhelper::InteractionContinuation > xSelection
                = xRequest->getSelection();

            if ( xSelection.is() )
            {
                uno::Reference< task::XInteractionApprove > xApprove(
                    xSelection.get(), uno::UNO_QUERY );
                if ( xApprove.is() )
                {
                    xCertificateContainer->addCertificate(
                        pSession->getHostName(), cert_subject,  true );
                    return 0;
                }
                else
                {
                    // Don't trust cert
                    xCertificateContainer->addCertificate(
                        pSession->getHostName(), cert_subject, false );
                    return 1;
                }
            }
        }
        else
        {
            // Don't trust cert
            xCertificateContainer->addCertificate(
                pSession->getHostName(), cert_subject, false );
            return 1;
        }
    }
    return 1;
}

extern "C" void NeonSession_PreSendRequest( ne_request * req,
                                            void * userdata,
                                            ne_buffer * headers )
{
    // userdata -> value returned by 'create'

    NeonSession * pSession = static_cast< NeonSession * >( userdata );
    if ( pSession )
    {
        // If there is a proxy server in between, it shall never use
        // cached data. We always want 'up-to-date' data.
        ne_buffer_concat( headers, "Pragma: no-cache", EOL, nullptr );
        // alternative, but understoud by HTTP 1.1 servers only:
        // ne_buffer_concat( headers, "Cache-Control: max-age=0", EOL, NULL );

        const RequestDataMap * pRequestData
            = static_cast< const RequestDataMap* >(
                pSession->getRequestData() );

        RequestDataMap::const_iterator it = pRequestData->find( req );
        if ( it != pRequestData->end() )
        {
            if ( !(*it).second.aContentType.isEmpty() )
            {
                char * pData = headers->data;
                if ( strstr( pData, "Content-Type:" ) == nullptr )
                {
                    OString aType
                        = OUStringToOString( (*it).second.aContentType,
                                                  RTL_TEXTENCODING_UTF8 );
                    ne_buffer_concat( headers, "Content-Type: ",
                                      aType.getStr(), EOL, nullptr );
                }
            }

            if ( !(*it).second.aReferer.isEmpty() )
            {
                char * pData = headers->data;
                if ( strstr( pData, "Referer:" ) == nullptr )
                {
                    OString aReferer
                        = OUStringToOString( (*it).second.aReferer,
                                                  RTL_TEXTENCODING_UTF8 );
                    ne_buffer_concat( headers, "Referer: ",
                                      aReferer.getStr(), EOL, nullptr );
                }
            }
        }

        const DAVRequestHeaders & rHeaders
            = pSession->getRequestEnvironment().m_aRequestHeaders;

        DAVRequestHeaders::const_iterator it1( rHeaders.begin() );
        const DAVRequestHeaders::const_iterator end1( rHeaders.end() );

        while ( it1 != end1 )
        {
            OString aHeader
                = OUStringToOString( (*it1).first,
                                          RTL_TEXTENCODING_UTF8 );
            OString aValue
                = OUStringToOString( (*it1).second,
                                          RTL_TEXTENCODING_UTF8 );
            ne_buffer_concat( headers, aHeader.getStr(), ": ",
                              aValue.getStr(), EOL, nullptr );

            ++it1;
        }
    }
}

// static members
bool NeonSession::m_bGlobalsInited = false;
//See https://bugzilla.redhat.com/show_bug.cgi?id=544619#c4
//neon is threadsafe, but uses gnutls which is only thread-safe
//if initialized to be thread-safe. cups, unfortunately, generally
//initializes it first, and as non-thread-safe, leaving the entire
//stack unsafe
osl::Mutex aGlobalNeonMutex;
NeonLockStore NeonSession::m_aNeonLockStore;

NeonSession::NeonSession( const rtl::Reference< DAVSessionFactory > & rSessionFactory,
                          const OUString& inUri,
                          const uno::Sequence< beans::NamedValue >& rFlags,
                          const ucbhelper::InternetProxyDecider & rProxyDecider )
    throw ( std::exception )
    : DAVSession( rSessionFactory )
    , m_nProxyPort( 0 )
    , m_aFlags( rFlags )
    , m_pHttpSession( nullptr )
    , m_pRequestData( new RequestDataMap )
    , m_rProxyDecider( rProxyDecider )
{
    NeonUri theUri( inUri );
    m_aScheme    = theUri.GetScheme();
    m_aHostName  = theUri.GetHost();
    m_nPort      = theUri.GetPort();
    SAL_INFO( "ucb.ucp.webdav", "NeonSession ctor - URL <" << inUri << ">" );
}

NeonSession::~NeonSession( )
{
    if ( m_pHttpSession )
    {
        {
            osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
            ne_session_destroy( m_pHttpSession );
        }
        m_pHttpSession = nullptr;
    }
    delete static_cast< RequestDataMap * >( m_pRequestData );
}

void NeonSession::Init( const DAVRequestEnvironment & rEnv )
    throw (css::uno::RuntimeException, std::exception)
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    m_aEnv = rEnv;
    Init();
}

void NeonSession::Init()
    throw (css::uno::RuntimeException, std::exception)
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    bool bCreateNewSession = false;

    if ( m_pHttpSession == nullptr )
    {
        // Ensure that Neon sockets are initialized
        osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
        if ( !m_bGlobalsInited )
        {
            if ( ne_sock_init() != 0 )
                throw DAVException( DAVException::DAV_SESSION_CREATE,
                                    NeonUri::makeConnectionEndPointString(
                                                    m_aHostName, m_nPort ) );

            // #122205# - libxml2 needs to be initialized once if used by
            // multithreaded programs like OOo.
            xmlInitParser();
#if OSL_DEBUG_LEVEL > 0
            // for more debug flags see ne_utils.h; NE_DEBUGGING must be defined
            // while compiling neon in order to actually activate neon debug
            // output.
            ne_debug_init( stderr, NE_DBG_FLUSH
                           | NE_DBG_HTTP
                           // | NE_DBG_HTTPBODY
                           // | NE_DBG_HTTPAUTH
                           // | NE_DBG_XML
                           // | NE_DBG_XMLPARSE
                           | NE_DBG_LOCKS
                           | NE_DBG_SSL
                         );
#endif
            m_bGlobalsInited = true;
        }

        const ucbhelper::InternetProxyServer & rProxyCfg = getProxySettings();

        m_aProxyName = rProxyCfg.aName;
        m_nProxyPort = rProxyCfg.nPort;

        // Not yet initialized. Create new session.
        bCreateNewSession = true;
    }
    else
    {
        // #112271# Check whether proxy settings are still valid (They may
        // change at any time). If not, create new Neon session.

        const ucbhelper::InternetProxyServer & rProxyCfg = getProxySettings();

        if ( ( rProxyCfg.aName != m_aProxyName )
             || ( rProxyCfg.nPort != m_nProxyPort ) )
        {
            m_aProxyName = rProxyCfg.aName;
            m_nProxyPort = rProxyCfg.nPort;

            // new session needed, destroy old first
            {
                osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
                ne_session_destroy( m_pHttpSession );
            }
            m_pHttpSession = nullptr;
            bCreateNewSession = true;
        }
    }

    if ( bCreateNewSession )
    {
        const sal_Int32    nConnectTimeoutMax = 180;
        const sal_Int32    nConnectTimeoutMin = 2;
        const sal_Int32    nReadTimeoutMax = 180;
        const sal_Int32    nReadTimeoutMin = 20;

        // @@@ For FTP over HTTP proxy inUserInfo is needed to be able to
        //     build the complete request URI (including user:pass), but
        //     currently (0.22.0) neon does not allow to pass the user info
        //     to the session

        {
            osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
            m_pHttpSession = ne_session_create(
                OUStringToOString( m_aScheme, RTL_TEXTENCODING_UTF8 ).getStr(),
                /* theUri.GetUserInfo(),
                   @@@ for FTP via HTTP proxy, but not supported by Neon */
                OUStringToOString( m_aHostName, RTL_TEXTENCODING_UTF8 ).getStr(),
                m_nPort );
        }

        if ( m_pHttpSession == nullptr )
            throw DAVException( DAVException::DAV_SESSION_CREATE,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        // Register the session with the lock store
        m_aNeonLockStore.registerSession( m_pHttpSession );

        if ( m_aScheme.equalsIgnoreAsciiCase("https") )
        {
            // Set a failure callback for certificate check
            ne_ssl_set_verify(
                m_pHttpSession, NeonSession_CertificationNotify, this);

            // Tell Neon to tell the SSL library used (OpenSSL or
            // GnuTLS, I guess) to use a default set of root
            // certificates.
            ne_ssl_trust_default_ca(m_pHttpSession);
        }

        // Add hooks (i.e. for adding additional headers to the request)

#if 0
        /* Hook called when a request is created. */
        //typedef void (*ne_create_request_fn)(ne_request *req, void *userdata,
        //                 const char *method, const char *path);

        ne_hook_create_request( m_pHttpSession, create_req_hook_fn, this );
#endif

        /* Hook called before the request is sent.  'header' is the raw HTTP
         * header before the trailing CRLF is added: add in more here. */
        //typedef void (*ne_pre_send_fn)(ne_request *req, void *userdata,
        //               ne_buffer *header);

        ne_hook_pre_send( m_pHttpSession, NeonSession_PreSendRequest, this );
#if 0
        /* Hook called after the request is sent. May return:
         *  NE_OK     everything is okay
         *  NE_RETRY  try sending the request again.
         * anything else signifies an error, and the request is failed. The
         * return code is passed back the _dispatch caller, so the session error
         * must also be set appropriately (ne_set_error).
         */
        //typedef int (*ne_post_send_fn)(ne_request *req, void *userdata,
        //               const ne_status *status);

        ne_hook_post_send( m_pHttpSession, post_send_req_hook_fn, this );

        /* Hook called when the request is destroyed. */
        //typedef void (*ne_destroy_req_fn)(ne_request *req, void *userdata);

        ne_hook_destroy_request( m_pHttpSession, destroy_req_hook_fn, this );

        /* Hook called when the session is destroyed. */
        //typedef void (*ne_destroy_sess_fn)(void *userdata);

        ne_hook_destroy_session( m_pHttpSession, destroy_sess_hook_fn, this );
#endif

        if ( !m_aProxyName.isEmpty() )
        {
            ne_session_proxy( m_pHttpSession,
                              OUStringToOString(
                                  m_aProxyName,
                                  RTL_TEXTENCODING_UTF8 ).getStr(),
                              m_nProxyPort );
        }

        // avoid KeepAlive?
        if ( noKeepAlive(m_aFlags) )
            ne_set_session_flag( m_pHttpSession, NE_SESSFLAG_PERSIST, 0 );

        // Register for redirects.
        ne_redirect_register( m_pHttpSession );

        // authentication callbacks.
#if 1
        ne_add_server_auth( m_pHttpSession, NE_AUTH_ALL, NeonSession_NeonAuth, this );
        ne_add_proxy_auth ( m_pHttpSession, NE_AUTH_ALL, NeonSession_NeonAuth, this );
#else
        ne_set_server_auth( m_pHttpSession, NeonSession_NeonAuth, this );
        ne_set_proxy_auth ( m_pHttpSession, NeonSession_NeonAuth, this );
#endif
        // set timeout to connect
        // if connect_timeout is not set, neon returns NE_CONNECT when the TCP socket default
        // timeout elapses
        // with connect_timeout set neon returns NE_TIMEOUT if elapsed when the connection
        // didn't succeed
        // grab it from configuration
        uno::Reference< uno::XComponentContext > rContext = m_xFactory->getComponentContext();

        // set the timeout (in seconds) used when making a connection
        sal_Int32 nConnectTimeout = officecfg::Inet::Settings::ConnectTimeout::get( rContext );
        ne_set_connect_timeout( m_pHttpSession,
                                (int) ( std::max( nConnectTimeoutMin,
                                                  std::min( nConnectTimeout, nConnectTimeoutMax ) ) ) );

        // provides a read time out facility as well
        // set the timeout (in seconds) used when reading from a socket.
        sal_Int32 nReadTimeout =  officecfg::Inet::Settings::ReadTimeout::get( rContext );
        ne_set_read_timeout( m_pHttpSession,
                             (int) ( std::max( nReadTimeoutMin,
                                               std::min( nReadTimeout, nReadTimeoutMax ) ) ) );
    }
}

bool NeonSession::CanUse( const OUString & inUri,
                          const uno::Sequence< beans::NamedValue >& rFlags )
{
    try
    {
        NeonUri theUri( inUri );
        if ( ( theUri.GetPort() == m_nPort ) &&
             ( theUri.GetHost() == m_aHostName ) &&
             ( theUri.GetScheme() == m_aScheme ) &&
             ( rFlags == m_aFlags ) )
            return true;
    }
    catch ( DAVException const & )
    {
        return false;
    }
    return false;
}

bool NeonSession::UsesProxy()
{
    Init();
    return  !m_aProxyName.isEmpty() ;
}

void NeonSession::OPTIONS( const OUString & inPath,
                           DAVOptions & rOptions, // contains the name+values of every header
                           const DAVRequestEnvironment & rEnv )
    throw( std::exception )
{

    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "OPTIONS - relative URL <" << inPath << ">" );

    rOptions.reset();
    int theRetVal = NE_OK;

    Init( rEnv );

    ne_request *req = ne_request_create(m_pHttpSession, "OPTIONS", OUStringToOString(
                                            inPath, RTL_TEXTENCODING_UTF8 ).getStr());

    theRetVal = ne_request_dispatch(req);

    //check if http error is in the 200 class (no error)
    if (theRetVal == NE_OK && ne_get_status(req)->klass != 2) {
        theRetVal = NE_ERROR;
    }

    if ( theRetVal == NE_OK )
    {
        void *cursor = nullptr;
        const char *name, *value;
        while ( ( cursor = ne_response_header_iterate(
                      req, cursor, &name, &value ) ) != nullptr )
        {
            OUString aHeaderName( OUString::createFromAscii( name ).toAsciiLowerCase() );
            OUString aHeaderValue( OUString::createFromAscii( value ) );

            // display the single header
            SAL_INFO( "ucb.ucp.webdav", "OPTIONS - received header: " << aHeaderName << ":" << aHeaderValue );

            if ( aHeaderName == "allow" )
            {
                rOptions.setAllowedMethods( aHeaderValue );
            }
            else if ( aHeaderName == "dav" )
            {
                // check type of dav capability
                // need to parse the value, token separator: ","
                // see <http://tools.ietf.org/html/rfc4918#section-10.1>,
                // <http://tools.ietf.org/html/rfc4918#section-18>,
                // and <http://tools.ietf.org/html/rfc7230#section-3.2>
                // we detect the class (1, 2 and 3), other elements (token, URL)
                // are not used for now
                // silly parser written using OUString, not very efficient
                // but quick and esy to write...
                sal_Int32 nFromIndex = 0;
                sal_Int32 nNextIndex = 0;
                while( ( nNextIndex = aHeaderValue.indexOf( ",", nFromIndex ) ) != -1 )
                { // found a comma
                    // try to convert from nFromIndex to nNextIndex -1 in a number
                    // if this is 1 or 2 or 3, use for class setting
                    sal_Int32 nClass =
                        aHeaderValue.copy( nFromIndex, nNextIndex - nFromIndex ).toInt32();
                    switch( nClass )
                    {
                        case 1:
                            rOptions.setClass1();
                            break;
                        case 2:
                            rOptions.setClass2();
                            break;
                        case 3:
                            rOptions.setClass3();
                            break;
                        default:
                            ;
                    }
                    // next starting point
                    nFromIndex = nNextIndex + 1;
                }
                // check for last fragment
                if ( nFromIndex < aHeaderValue.getLength() )
                {
                    sal_Int32 nClass = aHeaderValue.copy( nFromIndex ).toInt32();
                    switch( nClass )
                    {
                        case 1:
                            rOptions.setClass1();
                            break;
                        case 2:
                            rOptions.setClass2();
                            break;
                        case 3:
                            rOptions.setClass3();
                            break;
                        default:
                            ;
                    }
                }
            }
        }
        rOptions.setResourceFound();
    }

    ne_request_destroy(req);

    HandleError( theRetVal, inPath, rEnv );
}

void NeonSession::PROPFIND( const OUString & inPath,
                            const Depth inDepth,
                            const std::vector< OUString > & inPropNames,
                            std::vector< DAVResource > & ioResources,
                            const DAVRequestEnvironment & rEnv )
    throw ( std::exception )
{

    osl::Guard< osl::Mutex > theGuard( m_aMutex );

#if defined SAL_LOG_INFO
    { //debug
        SAL_INFO( "ucb.ucp.webdav", "PROPFIND - relative URL: <" << inPath << "> Depth: " << inDepth );
         for(std::vector< OUString >::const_iterator it = inPropNames.begin();
             it < inPropNames.end(); ++it)
         {
            SAL_INFO( "ucb.ucp.webdav", "PROPFIND - property requested: " << *it );
         }
    } //debug
#endif

    Init( rEnv );

    int theRetVal = NE_OK;
    NeonPropFindRequest theRequest( m_pHttpSession,
                                    OUStringToOString(
                                        inPath, RTL_TEXTENCODING_UTF8 ).getStr(),
                                    inDepth,
                                    inPropNames,
                                    ioResources,
                                    theRetVal );

    HandleError( theRetVal, inPath, rEnv );
}

void NeonSession::PROPFIND( const OUString & inPath,
                            const Depth inDepth,
                            std::vector< DAVResourceInfo > & ioResInfo,
                            const DAVRequestEnvironment & rEnv )
    throw( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "PROPFIND - relative URL: <" << inPath << "> Depth: " << inDepth );

    Init( rEnv );

    int theRetVal = NE_OK;
    NeonPropFindRequest theRequest( m_pHttpSession,
                                    OUStringToOString(
                                        inPath, RTL_TEXTENCODING_UTF8 ).getStr(),
                                    inDepth,
                                    ioResInfo,
                                    theRetVal );

#if defined SAL_LOG_INFO
    { //debug
        for ( std::vector< DAVResourceInfo >::const_iterator itres = ioResInfo.begin();
              itres < ioResInfo.end(); ++itres)
        {
            for ( std::vector< OUString >::const_iterator it = (*itres).properties.begin();
                  it < (*itres).properties.end(); ++it)
            {
                SAL_INFO( "ucb.ucp.webdav", "PROPFIND - returned property (name only): " << *it );
            }
        }
    } //debug
#endif

    HandleError( theRetVal, inPath, rEnv );
}

void NeonSession::PROPPATCH( const OUString & inPath,
                             const std::vector< ProppatchValue > & inValues,
                             const DAVRequestEnvironment & rEnv )
    throw( std::exception )
{
    SAL_INFO( "ucb.ucp.webdav", "PROPPATCH - relative URL <" << inPath << ">" );

    /* @@@ Which standard live properties can be set by the client?
           This is a known WebDAV RFC issue ( verified: 04/10/2001 )
           --> http://www.ics.uci.edu/pub/ietf/webdav/protocol/issues.html

        mod_dav implementation:

        creationdate        r ( File System prop )
        displayname         w
        getcontentlanguage  r ( #ifdef DAV_DISABLE_WRITEABLE_PROPS )
        getcontentlength    r ( File System prop )
        getcontenttype      r ( #ifdef DAV_DISABLE_WRITEABLE_PROPS )
        getetag             r ( File System prop )
        getlastmodified     r ( File System prop )
        lockdiscovery       r
        resourcetype        r
        source              w
        supportedlock       r
        executable          w ( #ifndef WIN32 )

        All dead properties are of course writable.
    */

    int theRetVal = NE_OK;

    int n;  // for the "for" loop

    // Generate the list of properties we want to set.
    int nPropCount = inValues.size();
    ne_proppatch_operation* pItems
        = new ne_proppatch_operation[ nPropCount + 1 ];
    for ( n = 0; n < nPropCount; ++n )
    {
        const ProppatchValue & rValue = inValues[ n ];

        // Split fullname into namespace and name!
        ne_propname * pName = new ne_propname;
        DAVProperties::createNeonPropName( rValue.name, *pName );
        pItems[ n ].name = pName;

        if ( rValue.operation == PROPSET )
        {
            pItems[ n ].type = ne_propset;

            OUString aStringValue;
            if ( DAVProperties::isUCBDeadProperty( *pName ) )
            {
                // DAV dead property added by WebDAV UCP?
                if ( !UCBDeadPropertyValue::toXML( rValue.value,
                                                   aStringValue ) )
                {
                    // Error!
                    pItems[ n ].value = nullptr;
                    theRetVal = NE_ERROR;
                    nPropCount = n + 1;
                    break;
                }
            }
            else if ( !( rValue.value >>= aStringValue ) )
            {
                // complex properties...
                if ( rValue.name == DAVProperties::SOURCE )
                {
                    uno::Sequence< ucb::Link > aLinks;
                    if ( rValue.value >>= aLinks )
                    {
                        LinkSequence::toXML( aLinks, aStringValue );
                    }
                    else
                    {
                        // Error!
                        pItems[ n ].value = nullptr;
                        theRetVal = NE_ERROR;
                        nPropCount = n + 1;
                        break;
                    }
                }
                else
                {
                    SAL_WARN( "ucb.ucp.webdav", "PROPPATCH - Unsupported type!" );
                    // Error!
                    pItems[ n ].value = nullptr;
                    theRetVal = NE_ERROR;
                    nPropCount = n + 1;
                    break;
                }
            }
            pItems[ n ].value
                = strdup( OUStringToOString( aStringValue,
                                                  RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        else
        {
            pItems[ n ].type  = ne_propremove;
            pItems[ n ].value = nullptr;
        }
    }

    if ( theRetVal == NE_OK )
    {
        osl::Guard< osl::Mutex > theGuard( m_aMutex );

        Init( rEnv );

        pItems[ n ].name = nullptr;

        theRetVal = ne_proppatch( m_pHttpSession,
                                  OUStringToOString(
                                      inPath, RTL_TEXTENCODING_UTF8 ).getStr(),
                                  pItems );
    }

    for ( n = 0; n < nPropCount; ++n )
    {
        free( const_cast<char *>(pItems[ n ].name->name) );
        delete pItems[ n ].name;
        free( const_cast<char *>(pItems[ n ].value) );
    }

    delete [] pItems;

    HandleError( theRetVal, inPath, rEnv );
}

void NeonSession::HEAD( const OUString &  inPath,
                        const std::vector< OUString > & inHeaderNames,
                        DAVResource & ioResource,
                        const DAVRequestEnvironment & rEnv )
    throw( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "HEAD - relative URL <" << inPath << ">" );

    Init( rEnv );

    int theRetVal = NE_OK;
    NeonHeadRequest theRequest( m_pHttpSession,
                                inPath,
                                inHeaderNames,
                                ioResource,
                                theRetVal );

    HandleError( theRetVal, inPath, rEnv );
}

uno::Reference< io::XInputStream >
NeonSession::GET( const OUString & inPath,
                  const DAVRequestEnvironment & rEnv )
    throw ( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "GET - relative URL <" << inPath << ">" );

    Init( rEnv );

    rtl::Reference< NeonInputStream > xInputStream( new NeonInputStream );
    NeonRequestContext aCtx( xInputStream );
    int theRetVal = GET( m_pHttpSession,
                         OUStringToOString(
                             inPath, RTL_TEXTENCODING_UTF8 ).getStr(),
                         NeonSession_ResponseBlockReader,
                         false,
                         &aCtx );

    HandleError( theRetVal, inPath, rEnv );

    return uno::Reference< io::XInputStream >( xInputStream.get() );
}

void NeonSession::GET( const OUString & inPath,
                       uno::Reference< io::XOutputStream > & ioOutputStream,
                       const DAVRequestEnvironment & rEnv )
    throw ( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "GET - relative URL <" << inPath << ">" );

    Init( rEnv );

    NeonRequestContext aCtx( ioOutputStream );
    int theRetVal = GET( m_pHttpSession,
                         OUStringToOString(
                             inPath, RTL_TEXTENCODING_UTF8 ).getStr(),
                         NeonSession_ResponseBlockWriter,
                         false,
                         &aCtx );

    HandleError( theRetVal, inPath, rEnv );
}

uno::Reference< io::XInputStream >
NeonSession::GET( const OUString & inPath,
                  const std::vector< OUString > & inHeaderNames,
                  DAVResource & ioResource,
                  const DAVRequestEnvironment & rEnv )
    throw ( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "GET - relative URL <" << inPath << ">" );

    Init( rEnv );

    ioResource.uri = inPath;
    ioResource.properties.clear();

    rtl::Reference< NeonInputStream > xInputStream( new NeonInputStream );
    NeonRequestContext aCtx( xInputStream, inHeaderNames, ioResource );
    int theRetVal = GET( m_pHttpSession,
                         OUStringToOString(
                             inPath, RTL_TEXTENCODING_UTF8 ).getStr(),
                         NeonSession_ResponseBlockReader,
                         true,
                         &aCtx );

    HandleError( theRetVal, inPath, rEnv );

    return uno::Reference< io::XInputStream >( xInputStream.get() );
}

void NeonSession::GET( const OUString & inPath,
                       uno::Reference< io::XOutputStream > & ioOutputStream,
                       const std::vector< OUString > & inHeaderNames,
                       DAVResource & ioResource,
                       const DAVRequestEnvironment & rEnv )
    throw ( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "GET - relative URL <" << inPath << ">" );

    Init( rEnv );

    ioResource.uri = inPath;
    ioResource.properties.clear();

    NeonRequestContext aCtx( ioOutputStream, inHeaderNames, ioResource );
    int theRetVal = GET( m_pHttpSession,
                         OUStringToOString(
                             inPath, RTL_TEXTENCODING_UTF8 ).getStr(),
                         NeonSession_ResponseBlockWriter,
                         true,
                         &aCtx );

    HandleError( theRetVal, inPath, rEnv );
}

void NeonSession::PUT( const OUString & inPath,
                       const uno::Reference< io::XInputStream > & inInputStream,
                       const DAVRequestEnvironment & rEnv )
    throw ( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "PUT - relative URL <" << inPath << ">" );

    uno::Sequence< sal_Int8 > aDataToSend;
    if ( !getDataFromInputStream( inInputStream, aDataToSend, false ) )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    Init( rEnv );

    int theRetVal = PUT( m_pHttpSession,
                         OUStringToOString(
                             inPath, RTL_TEXTENCODING_UTF8 ).getStr(),
                         reinterpret_cast< const char * >(
                            aDataToSend.getConstArray() ),
                         aDataToSend.getLength() );

    HandleError( theRetVal, inPath, rEnv );
}

uno::Reference< io::XInputStream >
NeonSession::POST( const OUString & inPath,
                   const OUString & rContentType,
                   const OUString & rReferer,
                   const uno::Reference< io::XInputStream > & inInputStream,
                   const DAVRequestEnvironment & rEnv )
    throw ( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "POST - relative URL <" << inPath << ">" );

    uno::Sequence< sal_Int8 > aDataToSend;
    if ( !getDataFromInputStream( inInputStream, aDataToSend, true ) )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    Init( rEnv );

    rtl::Reference< NeonInputStream > xInputStream( new NeonInputStream );
    NeonRequestContext aCtx( xInputStream );
    int theRetVal = POST( m_pHttpSession,
                          OUStringToOString(
                              inPath, RTL_TEXTENCODING_UTF8 ).getStr(),
                          reinterpret_cast< const char * >(
                              aDataToSend.getConstArray() ),
                          NeonSession_ResponseBlockReader,
                          &aCtx,
                          rContentType,
                          rReferer );

    HandleError( theRetVal, inPath, rEnv );

    return uno::Reference< io::XInputStream >( xInputStream.get() );
}

void NeonSession::POST( const OUString & inPath,
                        const OUString & rContentType,
                        const OUString & rReferer,
                        const uno::Reference< io::XInputStream > & inInputStream,
                        uno::Reference< io::XOutputStream > & oOutputStream,
                        const DAVRequestEnvironment & rEnv )
    throw ( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "POST - relative URL <" << inPath << ">" );

    uno::Sequence< sal_Int8 > aDataToSend;
    if ( !getDataFromInputStream( inInputStream, aDataToSend, true ) )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    Init( rEnv );

    NeonRequestContext aCtx( oOutputStream );
    int theRetVal = POST( m_pHttpSession,
                          OUStringToOString(
                              inPath, RTL_TEXTENCODING_UTF8 ).getStr(),
                          reinterpret_cast< const char * >(
                              aDataToSend.getConstArray() ),
                          NeonSession_ResponseBlockWriter,
                          &aCtx,
                          rContentType,
                          rReferer );

    HandleError( theRetVal, inPath, rEnv );
}

void NeonSession::MKCOL( const OUString & inPath,
                         const DAVRequestEnvironment & rEnv )
    throw ( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "MKCOL - relative URL <" << inPath << ">" );

    Init( rEnv );

    int theRetVal = ne_mkcol( m_pHttpSession,
                              OUStringToOString(
                                  inPath, RTL_TEXTENCODING_UTF8 ).getStr() );

    HandleError( theRetVal, inPath, rEnv );
}

void NeonSession::COPY( const OUString & inSourceURL,
                        const OUString & inDestinationURL,
                        const DAVRequestEnvironment & rEnv,
                        bool inOverWrite )
    throw ( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "COPY - inSourceURL: "<<inSourceURL<<" inDestinationURL: "<<inDestinationURL);

    Init( rEnv );

    NeonUri theSourceUri( inSourceURL );
    NeonUri theDestinationUri( inDestinationURL );

    int theRetVal = ne_copy( m_pHttpSession,
                             inOverWrite ? 1 : 0,
                             NE_DEPTH_INFINITE,
                             OUStringToOString(
                                 theSourceUri.GetPath(),
                                 RTL_TEXTENCODING_UTF8 ).getStr(),
                             OUStringToOString(
                                 theDestinationUri.GetPath(),
                                 RTL_TEXTENCODING_UTF8 ).getStr() );

    HandleError( theRetVal, inSourceURL, rEnv );
}

void NeonSession::MOVE( const OUString & inSourceURL,
                        const OUString & inDestinationURL,
                        const DAVRequestEnvironment & rEnv,
                        bool inOverWrite )
    throw ( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "MOVE - inSourceURL: "<<inSourceURL<<" inDestinationURL: "<<inDestinationURL);

    Init( rEnv );

    NeonUri theSourceUri( inSourceURL );
    NeonUri theDestinationUri( inDestinationURL );
    int theRetVal = ne_move( m_pHttpSession,
                             inOverWrite ? 1 : 0,
                             OUStringToOString(
                                 theSourceUri.GetPath(),
                                 RTL_TEXTENCODING_UTF8 ).getStr(),
                             OUStringToOString(
                                 theDestinationUri.GetPath(),
                                 RTL_TEXTENCODING_UTF8 ).getStr() );

    HandleError( theRetVal, inSourceURL, rEnv );
}

void NeonSession::DESTROY( const OUString & inPath,
                           const DAVRequestEnvironment & rEnv )
    throw ( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "DESTROY - relative URL <" << inPath << ">" );

    Init( rEnv );

    int theRetVal = ne_delete( m_pHttpSession,
                               OUStringToOString(
                                   inPath, RTL_TEXTENCODING_UTF8 ).getStr() );

    HandleError( theRetVal, inPath, rEnv );
}

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
                SAL_WARN( "ucb.ucp.webdav", "LOCK - no chance to refresh lock before timeout!" );
            }
        }
        return lastChanceToSendRefreshRequest;
    }

} // namespace

// Set new lock
void NeonSession::LOCK( const OUString & inPath,
                        ucb::Lock & rLock,
                        const DAVRequestEnvironment & rEnv )
    throw ( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    SAL_INFO( "ucb.ucp.webdav", "LOCK (create) - relative URL: <" << inPath << ">" );

    // before issuing the lock command,
    // better check first if we already have one on this href
    if ( m_aNeonLockStore.findByUri(
                         makeAbsoluteURL( inPath ) ) != nullptr )
    {
        // we already own a lock for this href
        // no need to ask for another
        // TODO: add a lockdiscovery request for confirmation
        // checking the locktoken, the only item that's unique
        return;
    }

    Init( rEnv );

    /* Create a depth zero, exclusive write lock, with default timeout
     * (allowing a server to pick a default).  token, owner and uri are
     * unset. */
    NeonLock * theLock = ne_lock_create();

    // Set the lock uri
    ne_uri aUri;
    ne_uri_parse( OUStringToOString( makeAbsoluteURL( inPath ),
                                          RTL_TEXTENCODING_UTF8 ).getStr(),
                  &aUri );
    theLock->uri = aUri;

    // Set the lock depth
    switch( rLock.Depth )
    {
    case ucb::LockDepth_ZERO:
        theLock->depth = NE_DEPTH_ZERO;
        break;
    case ucb::LockDepth_ONE:
        theLock->depth = NE_DEPTH_ONE;
        break;
    case ucb::LockDepth_INFINITY:
        theLock->depth = NE_DEPTH_INFINITE;
        break;
    default:
        throw DAVException( DAVException::DAV_INVALID_ARG );
    }

    // Set the lock scope
    switch ( rLock.Scope )
    {
    case ucb::LockScope_EXCLUSIVE:
        theLock->scope = ne_lockscope_exclusive;
        break;
    case ucb::LockScope_SHARED:
        theLock->scope = ne_lockscope_shared;
        break;
    default:
        throw DAVException( DAVException::DAV_INVALID_ARG );
    }

    // Set the lock timeout
    theLock->timeout = (long)rLock.Timeout;

    // Set the lock owner
    OUString aValue;
    rLock.Owner >>= aValue;
    theLock->owner =
        ne_strdup( OUStringToOString( aValue,
                                           RTL_TEXTENCODING_UTF8 ).getStr() );
    TimeValue startCall;
    osl_getSystemTime( &startCall );

    int theRetVal = ne_lock( m_pHttpSession, theLock );

    if ( theRetVal == NE_OK )
    {
        m_aNeonLockStore.addLock( theLock,
                                  this,
                                  lastChanceToSendRefreshRequest(
                                      startCall, theLock->timeout ) );

        uno::Sequence< OUString > aTokens( 1 );
        aTokens[ 0 ] = OUString::createFromAscii( theLock->token );
        rLock.LockTokens = aTokens;

        SAL_INFO( "ucb.ucp.webdav", "LOCK (create) - Created lock for <" << makeAbsoluteURL( inPath )
                  << "> token: <" << theLock->token << "> timeout: " << theLock->timeout << " sec.");
    }
    else
    {
        ne_lock_destroy( theLock );

        SAL_INFO( "ucb.ucp.webdav", "LOCK (create) - Obtaining lock for <"
                  << makeAbsoluteURL( inPath ) << " failed!" );
    }

    HandleError( theRetVal, inPath, rEnv );
}

// Refresh existing lock
bool NeonSession::LOCK( NeonLock * pLock,
                        sal_Int32 & rlastChanceToSendRefreshRequest )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

#if defined SAL_LOG_INFO
    {
        char * p = ne_uri_unparse( &(pLock->uri) );
        SAL_INFO( "ucb.ucp.webdav", "LOCK (refresh) - relative URL: <" << p << "> token: <" << pLock->token << ">" );
        ne_free( p );
    }
#endif

    // refresh existing lock.

    TimeValue startCall;
    osl_getSystemTime( &startCall );

    if ( ne_lock_refresh( m_pHttpSession, pLock ) == NE_OK )
    {
        rlastChanceToSendRefreshRequest
            = lastChanceToSendRefreshRequest( startCall, pLock->timeout );

        SAL_INFO( "ucb.ucp.webdav", "LOCK (refresh) - Lock successfully refreshed." );
        return true;
    }
    else
    {
#if defined SAL_LOG_WARN
        char * p = ne_uri_unparse( &(pLock->uri) );
        SAL_WARN( "ucb.ucp.webdav", "LOCK (refresh) - not refreshed! Relative URL: <" << p << "> token: <" << pLock->token << ">"  );
        ne_free( p );
#endif
        return false;
    }
}

void NeonSession::UNLOCK( const OUString & inPath,
                          const DAVRequestEnvironment & rEnv )
    throw ( std::exception )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    // get the neon lock from lock store
    NeonLock * theLock
        = m_aNeonLockStore.findByUri( makeAbsoluteURL( inPath ) );
    if ( !theLock )
        throw DAVException( DAVException::DAV_NOT_LOCKED );

    SAL_INFO( "ucb.ucp.webdav", "UNLOCK - relative URL: <" << inPath << "> token: <" << theLock->token << ">"  );
    Init( rEnv );

    int theRetVal = ne_unlock( m_pHttpSession, theLock );

    if ( theRetVal == NE_OK )
    {
        m_aNeonLockStore.removeLock( theLock );
        ne_lock_destroy( theLock );
    }
    else
    {
        SAL_INFO( "ucb.ucp.webdav", "UNLOCK - Unlocking of <"
                  << makeAbsoluteURL( inPath ) << "> failed." );
    }

    HandleError( theRetVal, inPath, rEnv );
}

bool NeonSession::UNLOCK( NeonLock * pLock )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

#if defined SAL_LOG_INFO
    {
        char * p = ne_uri_unparse( &(pLock->uri) );
        SAL_INFO( "ucb.ucp.webdav", "UNLOCK (from store) - relative URL: <" << p << "> token: <" << pLock->token << ">"   );
        ne_free( p );
    }
#endif

    if ( ne_unlock( m_pHttpSession, pLock ) == NE_OK )
    {
#if defined SAL_LOG_INFO
    {
        char * p = ne_uri_unparse( &(pLock->uri) );
        SAL_INFO( "ucb.ucp.webdav", "UNLOCK (from store) - relative URL: <" << p << "> token: <" << pLock->token << "> succeeded." );
        ne_free( p );
    }
#endif
        return true;
    }
    else
    {
#if defined SAL_LOG_INFO
    {
        char * p = ne_uri_unparse( &(pLock->uri) );
        SAL_INFO( "ucb.ucp.webdav", "UNLOCK (from store) - relative URL: <" << p << "> token: <" << pLock->token << "> failed!" );
        ne_free( p );
    }
#endif
        return false;
    }
}

void NeonSession::abort()
    throw ( std::exception )
{
    SAL_INFO( "ucb.ucp.webdav", "neon commands cannot be aborted" );
}

const ucbhelper::InternetProxyServer & NeonSession::getProxySettings() const
{
    if ( m_aScheme == "http" || m_aScheme == "https" )
    {
        return m_rProxyDecider.getProxy( m_aScheme,
                                         m_aHostName,
                                         m_nPort );
    }
    else
    {
        return m_rProxyDecider.getProxy( m_aScheme,
                                         OUString() /* not used */,
                                         -1 /* not used */ );
    }
}

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

bool NeonSession::removeExpiredLocktoken( const OUString & inURL,
                                          const DAVRequestEnvironment & rEnv )
{
    NeonLock * theLock = m_aNeonLockStore.findByUri( inURL );
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
        SAL_WARN( "ucb.ucp.webdav", "Removing expired lock token for <" << inURL
                  << "> token: " << theLock->token );

        m_aNeonLockStore.removeLock( theLock );
        ne_lock_destroy( theLock );
        return true;
    }
    catch ( DAVException const & )
    {
    }
    return false;
}

// Common error handler
void NeonSession::HandleError( int nError,
                               const OUString & inPath,
                               const DAVRequestEnvironment & rEnv )
    throw ( std::exception )
{
    m_aEnv = DAVRequestEnvironment();

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

            SAL_WARN( "ucb.ucp.webdav", "Neon returned NE_ERROR, http response status code was: '" << aText << "'" );
            if ( code == SC_LOCKED )
            {
                if ( m_aNeonLockStore.findByUri(
                         makeAbsoluteURL( inPath ) ) == nullptr )
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
            SAL_WARN( "ucb.ucp.webdav", "Name lookup failed" );
            throw DAVException( DAVException::DAV_HTTP_LOOKUP,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_AUTH:         // User authentication failed on server
            throw DAVException( DAVException::DAV_HTTP_AUTH,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_PROXYAUTH:    // User authentication failed on proxy
            SAL_WARN( "ucb.ucp.webdav", "DAVException::DAV_HTTP_AUTHPROXY" );
            throw DAVException( DAVException::DAV_HTTP_AUTHPROXY,
                                NeonUri::makeConnectionEndPointString(
                                    m_aProxyName, m_nProxyPort ) );

        case NE_CONNECT:      // Could not connect to server
            SAL_WARN( "ucb.ucp.webdav", "DAVException::DAV_HTTP_CONNECT" );
            throw DAVException( DAVException::DAV_HTTP_CONNECT,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_TIMEOUT:      // Connection timed out
            SAL_WARN( "ucb.ucp.webdav", "DAVException::DAV_HTTP_TIMEOUT" );
            throw DAVException( DAVException::DAV_HTTP_TIMEOUT,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_FAILED:       // The precondition failed
            SAL_WARN( "ucb.ucp.webdav", "The precondition failed" );
            throw DAVException( DAVException::DAV_HTTP_FAILED,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_RETRY:        // Retry request (ne_end_request ONLY)
            throw DAVException( DAVException::DAV_HTTP_RETRY,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_REDIRECT:
        {
            NeonUri aUri( ne_redirect_location( m_pHttpSession ) );
            SAL_INFO( "ucb.ucp.webdav", "DAVException::DAV_HTTP_REDIRECT: new URI: " << aUri.GetURI() );
            throw DAVException(
                DAVException::DAV_HTTP_REDIRECT, aUri.GetURI() );
        }
        default:
        {
            SAL_WARN( "ucb.ucp.webdav", "Unknown Neon error code!" );
            throw DAVException( DAVException::DAV_HTTP_ERROR,
                                OUString::createFromAscii(
                                    ne_get_error( m_pHttpSession ) ) );
        }
    }
}

namespace {

void runResponseHeaderHandler( void * userdata,
                               const char * value )
{
    OUString aHeader( OUString::createFromAscii( value ) );
    sal_Int32 nPos = aHeader.indexOf( ':' );

    if ( nPos != -1 )
    {
        OUString aHeaderName( aHeader.copy( 0, nPos ) );

        NeonRequestContext * pCtx
            = static_cast< NeonRequestContext * >( userdata );

        // Note: Empty vector means that all headers are requested.
        bool bIncludeIt = ( pCtx->pHeaderNames->empty() );

        if ( !bIncludeIt )
        {
            // Check whether this header was requested.
            std::vector< OUString >::const_iterator it(
                pCtx->pHeaderNames->begin() );
            const std::vector< OUString >::const_iterator end(
                pCtx->pHeaderNames->end() );

            while ( it != end )
            {
                // header names are case insensitive
                if ( (*it).equalsIgnoreAsciiCase( aHeaderName ) )
                {
                    aHeaderName = (*it);
                    break;
                }
                ++it;
            }

            if ( it != end )
                bIncludeIt = true;
        }

        if ( bIncludeIt )
        {
            // Create & set the PropertyValue
            DAVPropertyValue thePropertyValue;
            // header names are case insensitive, so are the
            // corresponding property names.
            thePropertyValue.Name = aHeaderName.toAsciiLowerCase();
            thePropertyValue.IsCaseSensitive = false;

            if ( nPos < aHeader.getLength() )
                thePropertyValue.Value <<= aHeader.copy( nPos + 1 ).trim();

            // Add the newly created PropertyValue
            pCtx->pResource->properties.push_back( thePropertyValue );
        }
    }
}

} // namespace

int NeonSession::GET( ne_session * sess,
                      const char * uri,
                      ne_block_reader reader,
                      bool getheaders,
                      void * userdata )
{
    //struct get_context ctx;
    ne_request * req = ne_request_create( sess, "GET", uri );
    int ret;

    ne_decompress * dc
        = ne_decompress_reader( req, ne_accept_2xx, reader, userdata );

    {
        osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
        ret = ne_request_dispatch( req );
    }

    if ( getheaders )
    {
        void *cursor = nullptr;
        const char *name, *value;
        while ( ( cursor = ne_response_header_iterate(
                               req, cursor, &name, &value ) ) != nullptr )
        {
            char buffer[8192];

            SAL_INFO( "ucb.ucp.webdav", "GET - received header: " << name << ": " << value );
            ne_snprintf(buffer, sizeof buffer, "%s: %s", name, value);
            runResponseHeaderHandler(userdata, buffer);
        }
    }

    if ( ret == NE_OK && ne_get_status( req )->klass != 2 )
        ret = NE_ERROR;

    if ( dc != nullptr )
        ne_decompress_destroy(dc);

    ne_request_destroy( req );
    return ret;
}

int NeonSession::PUT( ne_session * sess,
                      const char * uri,
                      const char * buffer,
                      size_t size)
{
    ne_request * req = ne_request_create( sess, "PUT", uri );
    int ret;

    // tdf#99246
    // extract the path of uri
    // ne_lock_using_resource below compares path, ignores all the rest.
    // in case of Web proxy active, this function uri parameter is instead absolute
    ne_uri aUri;
    ne_uri_parse( uri, &aUri );
    ne_lock_using_resource( req, aUri.path, 0 );
    ne_lock_using_parent( req, uri );

    ne_set_request_body_buffer( req, buffer, size );

    {
        osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
        ret = ne_request_dispatch( req );
    }

    if ( ret == NE_OK && ne_get_status( req )->klass != 2 )
        ret = NE_ERROR;

    ne_request_destroy( req );
    return ret;
}

int NeonSession::POST( ne_session * sess,
                       const char * uri,
                       const char * buffer,
                       ne_block_reader reader,
                       void * userdata,
                       const OUString & rContentType,
                       const OUString & rReferer )
{
    ne_request * req = ne_request_create( sess, "POST", uri );
    //struct get_context ctx;
    int ret;

    RequestDataMap * pData = nullptr;

    if ( !rContentType.isEmpty() || !rReferer.isEmpty() )
    {
        // Remember contenttype and referer. Data will be added to HTTP request
        // header in 'PreSendRequest' callback.
        pData = static_cast< RequestDataMap* >( m_pRequestData );
        (*pData)[ req ] = RequestData( rContentType, rReferer );
    }

    //ctx.total = -1;
    //ctx.fd = fd;
    //ctx.error = 0;
    //ctx.session = sess;

    ///* Read the value of the Content-Length header into ctx.total */
    //ne_add_response_header_handler( req, "Content-Length",
    //                 ne_handle_numeric_header, &ctx.total );

    ne_add_response_body_reader( req, ne_accept_2xx, reader, userdata );

    ne_set_request_body_buffer( req, buffer, strlen( buffer ) );

    {
        osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
        ret = ne_request_dispatch( req );
    }

    //if ( ctx.error )
    //    ret = NE_ERROR;
    //else
    if ( ret == NE_OK && ne_get_status( req )->klass != 2 )
        ret = NE_ERROR;

    ne_request_destroy( req );

    if ( pData )
    {
        // Remove request data from session's list.
        RequestDataMap::iterator it = pData->find( req );
        if ( it != pData->end() )
            pData->erase( it );
    }

    return ret;
}

bool
NeonSession::getDataFromInputStream(
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
                    memcpy( static_cast<void*>( rData.getArray() + nPos ),
                                    static_cast<const void*>(aBuffer.getConstArray()),
                                    nRead );
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
NeonSession::isDomainMatch( const OUString& certHostName )
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

OUString NeonSession::makeAbsoluteURL( OUString const & rURL ) const
{
    try
    {
        // Is URL relative or already absolute?
        if ( !rURL.isEmpty() && rURL[ 0 ] != '/' )
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
            NeonUri aNeonUri( &aUri );
            ne_uri_free( &aUri );
            return aNeonUri.GetURI();
        }
    }
    catch ( DAVException const & )
    {
    }
    // error.
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
