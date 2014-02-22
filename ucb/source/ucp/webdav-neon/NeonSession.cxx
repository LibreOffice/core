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


#include <boost/unordered_map.hpp>
#include <vector>
#include <string.h>
#include "osl/diagnose.h"
#include "osl/time.h"
#include <rtl/string.h>
#include <ne_socket.h>
#include <ne_auth.h>
#include <ne_redirect.h>
#include <ne_ssl.h>


extern "C" {
#include <ne_compress.h>
}

#include "libxml/parser.h"
#include "rtl/ustrbuf.hxx"
#include "comphelper/processfactory.hxx"
#include "comphelper/sequence.hxx"
#include <comphelper/stl_types.hxx>
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

#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/security/CertificateContainerStatus.hpp>
#include <com/sun/star/security/CertificateContainer.hpp>
#include <com/sun/star/security/XCertificateContainer.hpp>
#include <com/sun/star/ucb/Lock.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <boost/bind.hpp>

using namespace com::sun::star;
using namespace webdav_ucp;

#ifndef EOL
#    define EOL "\r\n"
#endif

struct RequestData
{
    
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
        return (size_t)p;
    }
};

typedef boost::unordered_map
<
    ne_request*,
    RequestData,
    hashPtr,
    equalPtr
>
RequestDataMap;

static sal_uInt16 makeStatusCode( const OUString & rStatusText )
{
    
    

    if ( rStatusText.getLength() < 3 )
    {
        OSL_FAIL(
            "makeStatusCode - status text string to short!" );
        return 0;
    }

    sal_Int32 nPos = rStatusText.indexOf( ' ' );
    if ( nPos == -1 )
    {
        OSL_FAIL( "makeStatusCode - wrong status text format!" );
        return 0;
    }

    return sal_uInt16( rStatusText.copy( 0, nPos ).toInt32() );
}

static bool noKeepAlive( const uno::Sequence< beans::NamedValue >& rFlags )
{
    if ( !rFlags.hasElements() )
        return false;

    
    const beans::NamedValue* pAry(rFlags.getConstArray());
    const sal_Int32          nLen(rFlags.getLength());
    const beans::NamedValue* pValue(
        std::find_if(pAry,pAry+nLen,
                     boost::bind(comphelper::TNamedValueEqualFunctor(),
                                 _1,
                                 OUString("KeepAlive"))));
    if ( pValue != pAry+nLen && !pValue->Value.get<sal_Bool>() )
        return true;

    return false;
}

struct NeonRequestContext
{
    uno::Reference< io::XOutputStream >    xOutputStream;
    rtl::Reference< NeonInputStream >      xInputStream;
    const std::vector< OUString > * pHeaderNames;
    DAVResource *                          pResource;

    NeonRequestContext( uno::Reference< io::XOutputStream > & xOutStrm )
    : xOutputStream( xOutStrm ), xInputStream( 0 ),
      pHeaderNames( 0 ), pResource( 0 ) {}

    NeonRequestContext( const rtl::Reference< NeonInputStream > & xInStrm )
    : xOutputStream( 0 ), xInputStream( xInStrm ),
      pHeaderNames( 0 ), pResource( 0 ) {}

    NeonRequestContext( uno::Reference< io::XOutputStream > & xOutStrm,
                        const std::vector< OUString > & inHeaderNames,
                        DAVResource & ioResource )
    : xOutputStream( xOutStrm ), xInputStream( 0 ),
      pHeaderNames( &inHeaderNames ), pResource( &ioResource ) {}

    NeonRequestContext( const rtl::Reference< NeonInputStream > & xInStrm,
                        const std::vector< OUString > & inHeaderNames,
                        DAVResource & ioResource )
    : xOutputStream( 0 ), xInputStream( xInStrm ),
      pHeaderNames( &inHeaderNames ), pResource( &ioResource ) {}
};


extern "C" int NeonSession_ResponseBlockReader(void * inUserData,
                                               const char * inBuf,
                                               size_t inLen )
{
    
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


extern "C" int NeonSession_ResponseBlockWriter( void * inUserData,
                                                const char * inBuf,
                                                size_t inLen )
{
    
    if ( inLen > 0 )
    {
        NeonRequestContext * pCtx
            = static_cast< NeonRequestContext * >( inUserData );
        uno::Reference< io::XOutputStream > xOutputStream
            = pCtx->xOutputStream;

        if ( xOutputStream.is() )
        {
            const uno::Sequence< sal_Int8 > aSeq( (sal_Int8 *)inBuf, inLen );
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
        
        return -1;
    }
    OUString theUserName;
    OUString thePassWord;

    if ( attempt == 0 )
    {
        
        

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
            
            return -1;
        }
    }
    else
    {
        
        theUserName = OUString::createFromAscii( inoutUserName );
        
        
    }

    bool bCanUseSystemCreds = false;

#if defined NE_FEATURE_SSPI && ! defined SYSTEM_NEON
    bCanUseSystemCreds
        = (attempt == 0) && 
          ne_has_support( NE_FEATURE_SSPI ) && 
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
        OSL_FAIL(
            "NeonSession_NeonAuth - username to long!" );
        return -1;
    }

    OString aPass(
        OUStringToOString( thePassWord, RTL_TEXTENCODING_UTF8 ) );
    if ( aPass.getLength() > ( NE_ABUFSIZ - 1 ) )
    {
        OSL_FAIL(
            "NeonSession_NeonAuth - password to long!" );
        return -1;
    }

    strcpy( inoutUserName, 
            OUStringToOString( theUserName, RTL_TEXTENCODING_UTF8 ).getStr() );

    strcpy( inoutPassWord, 
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
            sPart = _rRawString.copy( nContStart, nContEnd - nContStart );
        }
        return sPart;
    }
} 

extern "C" int NeonSession_CertificationNotify( void *userdata,
                                                int failures,
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

    failures = 0;

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

    
    char * eeCertB64 = ne_ssl_cert_export( cert );

    OString sEECertB64( eeCertB64 );

    uno::Reference< security::XCertificate > xEECert(
        xSecurityEnv->createCertificateFromAscii(
            OStringToOUString( sEECertB64, RTL_TEXTENCODING_ASCII_US ) ) );

    ne_free( eeCertB64 );
    eeCertB64 = 0;

    std::vector< uno::Reference< security::XCertificate > > vecCerts;
    const ne_ssl_certificate * issuerCert = cert;
    do
    {
        
        
        
        issuerCert = ne_ssl_cert_signedby( issuerCert );
        if ( NULL == issuerCert )
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
        
        
        if( certValidity == security::CertificateValidity::VALID )
            return 0;
    }

    const uno::Reference< ucb::XCommandEnvironment > xEnv(
        pSession->getRequestEnvironment().m_xEnv );
    if ( xEnv.is() )
    {
        failures = static_cast< int >( certValidity );

        uno::Reference< task::XInteractionHandler > xIH(
            xEnv->getInteractionHandler() );
        if ( xIH.is() )
        {
            rtl::Reference< ucbhelper::SimpleCertificateValidationRequest >
                xRequest( new ucbhelper::SimpleCertificateValidationRequest(
                    (sal_Int32)failures, xEECert, pSession->getHostName() ) );
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
                        pSession->getHostName(), cert_subject,  sal_True );
                    return 0;
                }
                else
                {
                    
                    xCertificateContainer->addCertificate(
                        pSession->getHostName(), cert_subject, sal_False );
                    return 1;
                }
            }
        }
        else
        {
            
            xCertificateContainer->addCertificate(
                pSession->getHostName(), cert_subject, sal_False );
            return 1;
        }
    }
    return 1;
}

extern "C" void NeonSession_PreSendRequest( ne_request * req,
                                            void * userdata,
                                            ne_buffer * headers )
{
    

    NeonSession * pSession = static_cast< NeonSession * >( userdata );
    if ( pSession )
    {
        
        
        ne_buffer_concat( headers, "Pragma: no-cache", EOL, NULL );
        
        

        const RequestDataMap * pRequestData
            = static_cast< const RequestDataMap* >(
                pSession->getRequestData() );

        RequestDataMap::const_iterator it = pRequestData->find( req );
        if ( it != pRequestData->end() )
        {
            if ( !(*it).second.aContentType.isEmpty() )
            {
                char * pData = headers->data;
                if ( strstr( pData, "Content-Type:" ) == NULL )
                {
                    OString aType
                        = OUStringToOString( (*it).second.aContentType,
                                                  RTL_TEXTENCODING_UTF8 );
                    ne_buffer_concat( headers, "Content-Type: ",
                                      aType.getStr(), EOL, NULL );
                }
            }

            if ( !(*it).second.aReferer.isEmpty() )
            {
                char * pData = headers->data;
                if ( strstr( pData, "Referer:" ) == NULL )
                {
                    OString aReferer
                        = OUStringToOString( (*it).second.aReferer,
                                                  RTL_TEXTENCODING_UTF8 );
                    ne_buffer_concat( headers, "Referer: ",
                                      aReferer.getStr(), EOL, NULL );
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
                              aValue.getStr(), EOL, NULL );

            ++it1;
        }
    }
}


bool NeonSession::m_bGlobalsInited = false;





osl::Mutex aGlobalNeonMutex;
NeonLockStore NeonSession::m_aNeonLockStore;

NeonSession::NeonSession( const rtl::Reference< DAVSessionFactory > & rSessionFactory,
                          const OUString& inUri,
                          const uno::Sequence< beans::NamedValue >& rFlags,
                          const ucbhelper::InternetProxyDecider & rProxyDecider )
    throw ( DAVException )
    : DAVSession( rSessionFactory )
    , m_nProxyPort( 0 )
    , m_aFlags( rFlags )
    , m_pHttpSession( 0 )
    , m_pRequestData( new RequestDataMap )
    , m_rProxyDecider( rProxyDecider )
{
    NeonUri theUri( inUri );
    m_aScheme    = theUri.GetScheme();
    m_aHostName  = theUri.GetHost();
    m_nPort      = theUri.GetPort();
}

NeonSession::~NeonSession( )
{
    if ( m_pHttpSession )
    {
        {
            osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
            ne_session_destroy( m_pHttpSession );
        }
        m_pHttpSession = 0;
    }
    delete static_cast< RequestDataMap * >( m_pRequestData );
}

void NeonSession::Init( const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );
    m_aEnv = rEnv;
    Init();
}

void NeonSession::Init()
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    bool bCreateNewSession = false;

    if ( m_pHttpSession == 0 )
    {
        
        osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
        if ( !m_bGlobalsInited )
        {
            if ( ne_sock_init() != 0 )
                throw DAVException( DAVException::DAV_SESSION_CREATE,
                                    NeonUri::makeConnectionEndPointString(
                                                    m_aHostName, m_nPort ) );

            
            
            xmlInitParser();
#if OSL_DEBUG_LEVEL > 0
            
            
            
            ne_debug_init( stderr, NE_DBG_FLUSH
                           | NE_DBG_HTTP
                           
                           
                           
                           
                           | NE_DBG_LOCKS
                           | NE_DBG_SSL
                         );
#endif
            m_bGlobalsInited = true;
        }

        const ucbhelper::InternetProxyServer & rProxyCfg = getProxySettings();

        m_aProxyName = rProxyCfg.aName;
        m_nProxyPort = rProxyCfg.nPort;

        
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

            
            {
                osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
                ne_session_destroy( m_pHttpSession );
            }
            m_pHttpSession = 0;
            bCreateNewSession = true;
        }
    }

    if ( bCreateNewSession )
    {
        
        
        
        

        {
            osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
            m_pHttpSession = ne_session_create(
                OUStringToOString( m_aScheme, RTL_TEXTENCODING_UTF8 ).getStr(),
                /* theUri.GetUserInfo(),
                   @@@ for FTP via HTTP proxy, but not supported by Neon */
                OUStringToOString( m_aHostName, RTL_TEXTENCODING_UTF8 ).getStr(),
                m_nPort );
        }

        if ( m_pHttpSession == 0 )
            throw DAVException( DAVException::DAV_SESSION_CREATE,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        
        m_aNeonLockStore.registerSession( m_pHttpSession );

        if ( m_aScheme.equalsIgnoreAsciiCase(
            OUString(  "https"  ) ) )
        {
            
            ne_ssl_set_verify(
                m_pHttpSession, NeonSession_CertificationNotify, this);

            
            
            
            ne_ssl_trust_default_ca(m_pHttpSession);
        }

        

#if 0
        /* Hook called when a request is created. */
        
        

        ne_hook_create_request( m_pHttpSession, create_req_hook_fn, this );
#endif

        /* Hook called before the request is sent.  'header' is the raw HTTP
         * header before the trailing CRLF is added: add in more here. */
        
        

        ne_hook_pre_send( m_pHttpSession, NeonSession_PreSendRequest, this );
#if 0
        /* Hook called after the request is sent. May return:
         *  NE_OK     everything is okay
         *  NE_RETRY  try sending the request again.
         * anything else signifies an error, and the request is failed. The
         * return code is passed back the _dispatch caller, so the session error
         * must also be set appropriately (ne_set_error).
         */
        
        

        ne_hook_post_send( m_pHttpSession, post_send_req_hook_fn, this );

        /* Hook called when the request is destroyed. */
        

        ne_hook_destroy_request( m_pHttpSession, destroy_req_hook_fn, this );

        /* Hook called when the session is destroyed. */
        

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

        
        if ( noKeepAlive(m_aFlags) )
            ne_set_session_flag( m_pHttpSession, NE_SESSFLAG_PERSIST, 0 );

        
        ne_redirect_register( m_pHttpSession );

        
#if 1
        ne_add_server_auth( m_pHttpSession, NE_AUTH_ALL, NeonSession_NeonAuth, this );
        ne_add_proxy_auth ( m_pHttpSession, NE_AUTH_ALL, NeonSession_NeonAuth, this );
#else
        ne_set_server_auth( m_pHttpSession, NeonSession_NeonAuth, this );
        ne_set_proxy_auth ( m_pHttpSession, NeonSession_NeonAuth, this );
#endif
    }
}

sal_Bool NeonSession::CanUse( const OUString & inUri,
                              const uno::Sequence< beans::NamedValue >& rFlags )
{
    try
    {
        NeonUri theUri( inUri );
        if ( ( theUri.GetPort() == m_nPort ) &&
             ( theUri.GetHost() == m_aHostName ) &&
             ( theUri.GetScheme() == m_aScheme ) &&
             ( rFlags == m_aFlags ) )
            return sal_True;
    }
    catch ( DAVException const & )
    {
        return sal_False;
    }
    return sal_False;
}

sal_Bool NeonSession::UsesProxy()
{
    Init();
    return  !m_aProxyName.isEmpty() ;
}

void NeonSession::OPTIONS( const OUString & inPath,
                           DAVCapabilities & outCapabilities,
                           const DAVRequestEnvironment & rEnv )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    HttpServerCapabilities servercaps;
    memset( &servercaps, 0, sizeof( servercaps ) );

    int theRetVal = ne_options( m_pHttpSession,
                                OUStringToOString(
                                    inPath, RTL_TEXTENCODING_UTF8 ).getStr(),
                                &servercaps );

    HandleError( theRetVal, inPath, rEnv );

    outCapabilities.class1     = !!servercaps.dav_class1;
    outCapabilities.class2     = !!servercaps.dav_class2;
    outCapabilities.executable = !!servercaps.dav_executable;
}

void NeonSession::PROPFIND( const OUString & inPath,
                            const Depth inDepth,
                            const std::vector< OUString > & inPropNames,
                            std::vector< DAVResource > & ioResources,
                            const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

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
    throw( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    int theRetVal = NE_OK;
    NeonPropFindRequest theRequest( m_pHttpSession,
                                    OUStringToOString(
                                        inPath, RTL_TEXTENCODING_UTF8 ).getStr(),
                                    inDepth,
                                    ioResInfo,
                                    theRetVal );

    HandleError( theRetVal, inPath, rEnv );
}

void NeonSession::PROPPATCH( const OUString & inPath,
                             const std::vector< ProppatchValue > & inValues,
                             const DAVRequestEnvironment & rEnv )
    throw( DAVException )
{
    /* @@@ Which standard live properties can be set by the client?
           This is a known WebDAV RFC issue ( verified: 04/10/2001 )
           --> http:

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

    int n;  

    
    int nPropCount = inValues.size();
    ne_proppatch_operation* pItems
        = new ne_proppatch_operation[ nPropCount + 1 ];
    for ( n = 0; n < nPropCount; ++n )
    {
        const ProppatchValue & rValue = inValues[ n ];

        
        ne_propname * pName = new ne_propname;
        DAVProperties::createNeonPropName( rValue.name, *pName );
        pItems[ n ].name = pName;

        if ( rValue.operation == PROPSET )
        {
            pItems[ n ].type = ne_propset;

            OUString aStringValue;
            if ( DAVProperties::isUCBDeadProperty( *pName ) )
            {
                
                if ( !UCBDeadPropertyValue::toXML( rValue.value,
                                                   aStringValue ) )
                {
                    
                    pItems[ n ].value = 0;
                    theRetVal = NE_ERROR;
                    nPropCount = n + 1;
                    break;
                }
            }
            else if ( !( rValue.value >>= aStringValue ) )
            {
                
                if ( rValue.name == DAVProperties::SOURCE )
                {
                    uno::Sequence< ucb::Link > aLinks;
                    if ( rValue.value >>= aLinks )
                    {
                        LinkSequence::toXML( aLinks, aStringValue );
                    }
                    else
                    {
                        
                        pItems[ n ].value = 0;
                        theRetVal = NE_ERROR;
                        nPropCount = n + 1;
                        break;
                    }
                }
                else
                {
                    OSL_FAIL( "NeonSession::PROPPATCH - unsupported type!" );
                    
                    pItems[ n ].value = 0;
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
            pItems[ n ].value = 0;
        }
    }

    if ( theRetVal == NE_OK )
    {
        osl::Guard< osl::Mutex > theGuard( m_aMutex );

        Init( rEnv );

        pItems[ n ].name = 0;

        theRetVal = ne_proppatch( m_pHttpSession,
                                  OUStringToOString(
                                      inPath, RTL_TEXTENCODING_UTF8 ).getStr(),
                                  pItems );
    }

    for ( n = 0; n < nPropCount; ++n )
    {
        free( (void *)pItems[ n ].name->name );
        delete pItems[ n ].name;
        free( (void *)pItems[ n ].value );
    }

    delete [] pItems;

    HandleError( theRetVal, inPath, rEnv );
}

void NeonSession::HEAD( const OUString &  inPath,
                        const std::vector< OUString > & inHeaderNames,
                        DAVResource & ioResource,
                        const DAVRequestEnvironment & rEnv )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

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
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

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
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

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
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

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
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

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
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

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
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

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
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

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
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    int theRetVal = ne_mkcol( m_pHttpSession,
                              OUStringToOString(
                                  inPath, RTL_TEXTENCODING_UTF8 ).getStr() );

    HandleError( theRetVal, inPath, rEnv );
}

void NeonSession::COPY( const OUString & inSourceURL,
                        const OUString & inDestinationURL,
                        const DAVRequestEnvironment & rEnv,
                        sal_Bool inOverWrite )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

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
                        sal_Bool inOverWrite )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

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
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

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
                OSL_TRACE( "No chance to refresh lock before timeout!" );
            }
        }
        return lastChanceToSendRefreshRequest;
    }

} 


void NeonSession::LOCK( const OUString & inPath,
                        ucb::Lock & rLock,
                        const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    Init( rEnv );

    /* Create a depth zero, exclusive write lock, with default timeout
     * (allowing a server to pick a default).  token, owner and uri are
     * unset. */
    NeonLock * theLock = ne_lock_create();

    
    ne_uri aUri;
    ne_uri_parse( OUStringToOString( makeAbsoluteURL( inPath ),
                                          RTL_TEXTENCODING_UTF8 ).getStr(),
                  &aUri );
    theLock->uri = aUri;

    
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

    
    theLock->timeout = (long)rLock.Timeout;

    
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

        OSL_TRACE( "NeonSession::LOCK: created lock for %s. token: %s",
                   OUStringToOString( makeAbsoluteURL( inPath ),
                                           RTL_TEXTENCODING_UTF8 ).getStr(),
                   theLock->token );
    }
    else
    {
        ne_lock_destroy( theLock );

        OSL_TRACE( "NeonSession::LOCK: obtaining lock for %s failed!",
                   OUStringToOString( makeAbsoluteURL( inPath ),
                                           RTL_TEXTENCODING_UTF8 ).getStr() );
    }

    HandleError( theRetVal, inPath, rEnv );
}


sal_Int64 NeonSession::LOCK( const OUString & inPath,
                             sal_Int64 nTimeout,
                             const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    
    NeonLock * theLock
        = m_aNeonLockStore.findByUri( makeAbsoluteURL( inPath ) );
    if ( !theLock )
         throw DAVException( DAVException::DAV_NOT_LOCKED );

    Init( rEnv );

    
    theLock->timeout = static_cast< long >( nTimeout );

    TimeValue startCall;
    osl_getSystemTime( &startCall );

    int theRetVal = ne_lock_refresh( m_pHttpSession, theLock );

    if ( theRetVal == NE_OK )
    {
        m_aNeonLockStore.updateLock( theLock,
                                     lastChanceToSendRefreshRequest(
                                         startCall, theLock->timeout ) );
    }

    HandleError( theRetVal, inPath, rEnv );

    return theLock->timeout;
}


bool NeonSession::LOCK( NeonLock * pLock,
                        sal_Int32 & rlastChanceToSendRefreshRequest )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

#if OSL_DEBUG_LEVEL > 0
    char * p = ne_uri_unparse( &(pLock->uri) );
    OSL_TRACE( "NeonSession::LOCK: Refreshing lock for %s.", p );
    ne_free( p );
#endif

    

    TimeValue startCall;
    osl_getSystemTime( &startCall );

    if ( ne_lock_refresh( m_pHttpSession, pLock ) == NE_OK )
    {
        rlastChanceToSendRefreshRequest
            = lastChanceToSendRefreshRequest( startCall, pLock->timeout );

        OSL_TRACE( "Lock successfully refreshed." );
        return true;
    }
    else
    {
        OSL_TRACE( "Lock not refreshed!" );
        return false;
    }
}

void NeonSession::UNLOCK( const OUString & inPath,
                          const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    
    NeonLock * theLock
        = m_aNeonLockStore.findByUri( makeAbsoluteURL( inPath ) );
    if ( !theLock )
        throw DAVException( DAVException::DAV_NOT_LOCKED );

    Init( rEnv );

    int theRetVal = ne_unlock( m_pHttpSession, theLock );

    if ( theRetVal == NE_OK )
    {
        m_aNeonLockStore.removeLock( theLock );
        ne_lock_destroy( theLock );
    }
    else
    {
        OSL_TRACE( "NeonSession::UNLOCK: unlocking of %s failed.",
                   OUStringToOString( makeAbsoluteURL( inPath ),
                                           RTL_TEXTENCODING_UTF8 ).getStr() );
    }

    HandleError( theRetVal, inPath, rEnv );
}

bool NeonSession::UNLOCK( NeonLock * pLock )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

#if OSL_DEBUG_LEVEL > 0
    char * p = ne_uri_unparse( &(pLock->uri) );
    OSL_TRACE( "NeonSession::UNLOCK: Unlocking %s.", p );
    ne_free( p );
#endif

    if ( ne_unlock( m_pHttpSession, pLock ) == NE_OK )
    {
        OSL_TRACE( "UNLOCK succeeded." );
        return true;
    }
    else
    {
        OSL_TRACE( "UNLOCK failed!" );
        return false;
    }
}

void NeonSession::abort()
    throw ( DAVException )
{
    SAL_INFO("ucb.ucp.webdav", "neon commands cannot be aborted");
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

} 

bool NeonSession::removeExpiredLocktoken( const OUString & inURL,
                                          const DAVRequestEnvironment & rEnv )
{
    NeonLock * theLock = m_aNeonLockStore.findByUri( inURL );
    if ( !theLock )
        return false;

    
    try
    {
        

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
                    
                    break;
                }

                
                return false;
            }
            ++it;
        }

        
        
        OSL_TRACE( "NeonSession::removeExpiredLocktoken: Removing "
                   " expired lock token for %s. token: %s",
                   OUStringToOString( inURL,
                                           RTL_TEXTENCODING_UTF8 ).getStr(),
                   theLock->token );

        m_aNeonLockStore.removeLock( theLock );
        ne_lock_destroy( theLock );
        return true;
    }
    catch ( DAVException const & )
    {
    }
    return false;
}


void NeonSession::HandleError( int nError,
                               const OUString & inPath,
                               const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    m_aEnv = DAVRequestEnvironment();

    
    switch ( nError )
    {
        case NE_OK:
            return;

        case NE_ERROR:        
        {
            OUString aText = OUString::createFromAscii(
                ne_get_error( m_pHttpSession ) );

            sal_uInt16 code = makeStatusCode( aText );

            if ( code == SC_LOCKED )
            {
                if ( m_aNeonLockStore.findByUri(
                         makeAbsoluteURL( inPath ) ) == 0 )
                {
                    
                    throw DAVException( DAVException::DAV_LOCKED );
                }
                else
                {
                    
                    throw DAVException( DAVException::DAV_LOCKED_SELF );
                }
            }

            
            
            
            
            else if ( code == SC_BAD_REQUEST || code == SC_PRECONDITION_FAILED )
            {
                if ( removeExpiredLocktoken( makeAbsoluteURL( inPath ), rEnv ) )
                    throw DAVException( DAVException::DAV_LOCK_EXPIRED );
            }

            throw DAVException( DAVException::DAV_HTTP_ERROR, aText, code );
        }
        case NE_LOOKUP:       
            throw DAVException( DAVException::DAV_HTTP_LOOKUP,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_AUTH:         
            throw DAVException( DAVException::DAV_HTTP_AUTH,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_PROXYAUTH:    
            throw DAVException( DAVException::DAV_HTTP_AUTHPROXY,
                                NeonUri::makeConnectionEndPointString(
                                    m_aProxyName, m_nProxyPort ) );

        case NE_CONNECT:      
            throw DAVException( DAVException::DAV_HTTP_CONNECT,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_TIMEOUT:      
            throw DAVException( DAVException::DAV_HTTP_TIMEOUT,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_FAILED:       
            throw DAVException( DAVException::DAV_HTTP_FAILED,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_RETRY:        
            throw DAVException( DAVException::DAV_HTTP_RETRY,
                                NeonUri::makeConnectionEndPointString(
                                    m_aHostName, m_nPort ) );

        case NE_REDIRECT:
        {
            NeonUri aUri( ne_redirect_location( m_pHttpSession ) );
            throw DAVException(
                DAVException::DAV_HTTP_REDIRECT, aUri.GetURI() );
        }
        default:
        {
            OSL_TRACE( "NeonSession::HandleError : Unknown Neon error code!" );
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

        
        bool bIncludeIt = ( pCtx->pHeaderNames->empty() );

        if ( !bIncludeIt )
        {
            
            std::vector< OUString >::const_iterator it(
                pCtx->pHeaderNames->begin() );
            const std::vector< OUString >::const_iterator end(
                pCtx->pHeaderNames->end() );

            while ( it != end )
            {
                
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
            
            DAVPropertyValue thePropertyValue;
            thePropertyValue.IsCaseSensitive = false;
            thePropertyValue.Name = aHeaderName;

            if ( nPos < aHeader.getLength() )
                thePropertyValue.Value <<= aHeader.copy( nPos + 1 ).trim();

            
            pCtx->pResource->properties.push_back( thePropertyValue );
        }
    }
}

} 

int NeonSession::GET( ne_session * sess,
                      const char * uri,
                      ne_block_reader reader,
                      bool getheaders,
                      void * userdata )
{
    
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
        void *cursor = NULL;
        const char *name, *value;
        while ( ( cursor = ne_response_header_iterate(
                               req, cursor, &name, &value ) ) != NULL )
        {
            char buffer[8192];

            ne_snprintf(buffer, sizeof buffer, "%s: %s", name, value);
            runResponseHeaderHandler(userdata, buffer);
        }
    }

    if ( ret == NE_OK && ne_get_status( req )->klass != 2 )
        ret = NE_ERROR;

    if ( dc != 0 )
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

    ne_lock_using_resource( req, uri, 0 );
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
    
    int ret;

    RequestDataMap * pData = 0;

    if ( !rContentType.isEmpty() || !rReferer.isEmpty() )
    {
        
        
        pData = static_cast< RequestDataMap* >( m_pRequestData );
        (*pData)[ req ] = RequestData( rContentType, rReferer );
    }

    
    
    
    

    
    
    

    ne_add_response_body_reader( req, ne_accept_2xx, reader, userdata );

    ne_set_request_body_buffer( req, buffer, strlen( buffer ) );

    {
        osl::Guard< osl::Mutex > theGlobalGuard( aGlobalNeonMutex );
        ret = ne_request_dispatch( req );
    }

    
    
    
    if ( ret == NE_OK && ne_get_status( req )->klass != 2 )
        ret = NE_ERROR;

    ne_request_destroy( req );

    if ( pData )
    {
        
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
                
            }
            catch ( io::BufferSizeExceededException const & )
            {
                
            }
            catch ( io::IOException const & )
            {
                
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
                    memcpy( (void*)( rData.getArray() + nPos ),
                                    (const void*)aBuffer.getConstArray(),
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
                
            }
            catch ( io::BufferSizeExceededException const & )
            {
                
            }
            catch ( io::IOException const & )
            {
                
            }
        }
    }
    return false;
}

sal_Bool
NeonSession::isDomainMatch( OUString certHostName )
{
    OUString hostName = getHostName();

    if (hostName.equalsIgnoreAsciiCase( certHostName ) )
        return sal_True;

    if ( certHostName.startsWith( "*" ) &&
         hostName.getLength() >= certHostName.getLength()  )
    {
        OUString cmpStr = certHostName.copy( 1 );

        if ( hostName.matchIgnoreAsciiCase(
                cmpStr, hostName.getLength() -  cmpStr.getLength() ) )
            return sal_True;
    }
    return sal_False;
}

OUString NeonSession::makeAbsoluteURL( OUString const & rURL ) const
{
    try
    {
        
        if ( rURL[ 0 ] != '/' )
        {
            
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
    
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
