/*************************************************************************
 *
 *  $RCSfile: NeonSession.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-25 11:39:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <hash_map>

#include <string.h>

#include <neon/config.h> /* for USE_DAV_LOCKS */

#ifndef NE_AUTH_H
#include <neon/ne_auth.h>
#endif
#ifndef NE_REDIRECT_H
#include <neon/ne_redirect.h>
#endif
#ifndef NE_LOCKS_H
#include <neon/ne_locks.h>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _UCBHELPER_BACKGROUNDDOWNLOAD_HXX_
#include <ucbhelper/backgrounddownload.hxx>
#endif

#ifndef _DAVAUTHLISTENER_HXX_
#include "DAVAuthListener.hxx"
#endif
#ifndef _NEONTYPES_HXX_
#include "NeonTypes.hxx"
#endif
#ifndef _NEONSESSION_HXX_
#include "NeonSession.hxx"
#endif
#ifndef _NEONINPUTSTREAM_HXX_
#include "NeonInputStream.hxx"
#endif
#ifndef _NEONPROPFINDREQUEST_HXX_
#include "NeonPropFindRequest.hxx"
#endif
#ifndef _NEONHEADREQUEST_HXX_
#include "NeonHeadRequest.hxx"
#endif
#ifndef _NEONURI_HXX_
#include "NeonUri.hxx"
#endif
#ifndef _LINKSEQUENCE_HXX_
#include "LinkSequence.hxx"
#endif
#ifndef _UCBDEADPROPERTYVALUE_HXX_
#include "UCBDeadPropertyValue.hxx"
#endif

using namespace com::sun::star;
using namespace webdav_ucp;

// -------------------------------------------------------------------
// RequestData
// -------------------------------------------------------------------

struct RequestData
{
    // POST
    rtl::OUString aContentType;
    rtl::OUString aReferer;

    RequestData() {}
    RequestData( const rtl::OUString & rContentType,
                 const rtl::OUString & rReferer )
    : aContentType( rContentType ), aReferer( rReferer ) {}
};

// -------------------------------------------------------------------
// RequestDataMap
// -------------------------------------------------------------------

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

typedef std::hash_map
<
    ne_request*,
    RequestData,
    hashPtr,
    equalPtr
>
RequestDataMap;

// -------------------------------------------------------------------
// static members!
sal_Bool NeonSession::m_bSockInited = sal_False;

// -------------------------------------------------------------------
// Helper fuction
// -------------------------------------------------------------------
static sal_uInt16 makeStatusCode( const rtl::OUString & rStatusText )
{
    // Extract status code from session error string. Unfortunately
    // neon provides no direct access to the status code...

    if ( rStatusText.getLength() < 3 )
    {
        OSL_ENSURE(
            sal_False, "makeStatusCode - status text string to short!" );
        return 0;
    }

    sal_Int32 nPos = rStatusText.indexOf( ' ' );
    if ( nPos == -1 )
    {
        OSL_ENSURE( sal_False, "makeStatusCode - wrong status text format!" );
        return 0;
    }

    return sal_uInt16( rStatusText.copy( 0, nPos ).toInt32() );
}

// -------------------------------------------------------------------
struct NeonRequestContext
{
    void*                                  pUserData;
    uno::Reference< io::XOutputStream >    xOutputStream;
    rtl::Reference< NeonInputStream >      xInputStream;
    const std::vector< ::rtl::OUString > * pHeaderNames;
    DAVResource *                          pResource;

    NeonRequestContext( uno::Reference< io::XOutputStream > & xOutStrm )
    : pUserData(0),xOutputStream( xOutStrm ), xInputStream( 0 ),
      pHeaderNames( 0 ), pResource( 0 ) {}

    NeonRequestContext( const rtl::Reference< NeonInputStream > & xInStrm )
    : pUserData(0),xOutputStream( 0 ), xInputStream( xInStrm ),
      pHeaderNames( 0 ), pResource( 0 ) {}

    NeonRequestContext( void* userData )
    : pUserData(userData),xOutputStream( 0 ), xInputStream( 0 ),
      pHeaderNames( 0 ), pResource( 0 ) {}

    NeonRequestContext( uno::Reference< io::XOutputStream > & xOutStrm,
                        const std::vector< ::rtl::OUString > & inHeaderNames,
                        DAVResource & ioResource )
    : pUserData(0),xOutputStream( xOutStrm ), xInputStream( 0 ),
      pHeaderNames( &inHeaderNames ), pResource( &ioResource ) {}

    NeonRequestContext( const rtl::Reference< NeonInputStream > & xInStrm,
                        const std::vector< ::rtl::OUString > & inHeaderNames,
                        DAVResource & ioResource )
    : pUserData(0),xOutputStream( 0 ), xInputStream( xInStrm ),
      pHeaderNames( &inHeaderNames ), pResource( &ioResource ) {}

    NeonRequestContext( void* userData,
                        const std::vector< ::rtl::OUString > & inHeaderNames,
                        DAVResource & ioResource )
    : pUserData(userData),xOutputStream( 0 ), xInputStream( 0 ),
      pHeaderNames( &inHeaderNames ), pResource( &ioResource ) {}

};

//--------------------------------------------------------------------
//--------------------------------------------------------------------
//
// Callback functions
//
//--------------------------------------------------------------------
//--------------------------------------------------------------------

// -------------------------------------------------------------------
// ResponseBlockReader
// A simple Neon response_block_reader for use with an XInputStream
// -------------------------------------------------------------------
extern "C" void NeonSession_ResponseBlockReader( void *       inUserData,
                                                 const char * inBuf,
                                                 size_t       inLen )
{
    // neon calls this function with (inLen == 0)...
    if ( inLen > 0 )
    {
        NeonRequestContext * pCtx
            = static_cast< NeonRequestContext * >( inUserData );

        if(pCtx->pUserData)
            ::ucb::writeToDataSink((void*)inBuf,1,inLen,pCtx->pUserData);
        else {
            rtl::Reference< NeonInputStream > xInputStream(
                pCtx->xInputStream);

            if ( xInputStream.is() )
                xInputStream->AddToStream( inBuf, inLen );
        }
    }
}

// -------------------------------------------------------------------
// ResponseBlockWriter
// A simple Neon response_block_reader for use with an XOutputStream
// -------------------------------------------------------------------
extern "C" void NeonSession_ResponseBlockWriter( void *       inUserData,
                                                 const char * inBuf,
                                                 size_t       inLen )
{
    // neon calls this function with (inLen == 0)...
    if ( inLen > 0 )
    {
        NeonRequestContext * pCtx
            = static_cast< NeonRequestContext * >( inUserData );
        uno::Reference< io::XOutputStream > xOutputStream = pCtx->xOutputStream;

        if ( xOutputStream.is() )
        {
            const uno::Sequence< sal_Int8 > aSeq( (sal_Int8 *)inBuf, inLen );
            xOutputStream->writeBytes( aSeq );
        }
    }
}

// -------------------------------------------------------------------
extern "C" int NeonSession_NeonAuth( void *       inUserData,
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

#if 0
    // Give'em only a limited mumber of retries..
    if ( attempt > 9 )
    {
        // abort
        return -1;
    }
#endif

    NeonSession * theSession = static_cast< NeonSession * >( inUserData );
    DAVAuthListener * pListener
        = theSession->getRequestEnvironment().m_xAuthListener.get();
    if ( !pListener )
    {
        // abort
        return -1;
    }
    rtl::OUString theUserName;
    rtl::OUString thePassWord;

    if ( attempt == 0 )
    {
        // neon does not handle username supplied with request URI (for instance
        // when doing FTP over proxy - last checked: 0.23.5 )

        NeonUri uri( theSession->getRequestEnvironment().m_aRequestURI );
        rtl::OUString aUserInfo( uri.GetUserInfo() );
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
    else
    {
        // username buffer is prefilled with user name from last attempt.
        theUserName = rtl::OUString::createFromAscii( inoutUserName );
        // @@@ Neon does not initialize password buffer (last checked: 0.22.0).
        //thePassWord = rtl::OUString::createFromAscii( inoutPassWord );
    }

    int theRetVal = pListener->authenticate(
                            rtl::OUString::createFromAscii( inRealm ),
                            theSession->getHostName(),
                            theUserName,
                            thePassWord );
    rtl::OString aUser(
        rtl::OUStringToOString( theUserName, RTL_TEXTENCODING_UTF8 ) );
    if ( aUser.getLength() > ( NE_ABUFSIZ - 1 ) )
    {
        OSL_ENSURE(
            sal_False, "NeonSession_NeonAuth - username to long!" );
        return -1;
    }

    rtl::OString aPass(
        rtl::OUStringToOString( thePassWord, RTL_TEXTENCODING_UTF8 ) );
    if ( aPass.getLength() > ( NE_ABUFSIZ - 1 ) )
    {
        OSL_ENSURE(
            sal_False, "NeonSession_NeonAuth - password to long!" );
        return -1;
    }

    strcpy( inoutUserName, // #100211# - checked
            rtl::OUStringToOString( theUserName, RTL_TEXTENCODING_UTF8 ) );

    strcpy( inoutPassWord, // #100211# - checked
            rtl::OUStringToOString( thePassWord, RTL_TEXTENCODING_UTF8 ) );

    return theRetVal;
}

// -------------------------------------------------------------------
extern "C" void NeonSession_ProgressNotify( void * userdata,
                                            off_t progress,
                                            off_t total )
{
    // progress: bytes read so far
    // total:    total bytes to read, -1 -> total count not known
}

// -------------------------------------------------------------------
extern "C" void NeonSession_StatusNotify( void * userdata,
                                          ne_conn_status status,
                                          const char *info )
{
#if 0
    typedef enum {
        ne_conn_namelookup, /* lookup up hostname (info = hostname) */
        ne_conn_connecting, /* connecting to host (info = hostname) */
        ne_conn_connected, /* connected to host (info = hostname) */
        ne_conn_secure /* connection now secure (info = crypto level) */
    } ne_conn_status;
#endif

    // info: hostname
}

// -------------------------------------------------------------------
extern "C" void NeonSession_PreSendRequest( ne_request * req,
                                            void * userdata,
                                            ne_buffer * headers )
{
    // userdata -> value returned by 'create'

    NeonSession * pSession = static_cast< NeonSession * >( userdata );
    if ( pSession )
    {
        const RequestDataMap * pRequestData
            = static_cast< const RequestDataMap* >(
                pSession->getRequestData() );

        RequestDataMap::const_iterator it = pRequestData->find( req );
        if ( it != pRequestData->end() )
        {
            if ( (*it).second.aContentType.getLength() )
            {
                char * pData = headers->data;
                if ( strstr( pData, "Content-Type:" ) == NULL )
                {
                    rtl::OString aType
                        = rtl::OUStringToOString( (*it).second.aContentType,
                                                  RTL_TEXTENCODING_UTF8 );
                    ne_buffer_concat( headers, "Content-Type: ",
                                    aType.getStr(), EOL, NULL );
                }
            }

            if ( (*it).second.aReferer.getLength() )
            {
                char * pData = headers->data;
                if ( strstr( pData, "Referer:" ) == NULL )
                {
                    rtl::OString aReferer
                        = rtl::OUStringToOString( (*it).second.aReferer,
                                                  RTL_TEXTENCODING_UTF8 );
                    ne_buffer_concat( headers, "Referer: ",
                                    aReferer.getStr(), EOL, NULL );
                }
            }
        }
    }
}

// -------------------------------------------------------------------
extern "C" void NeonSession_ResponseHeaderCatcher( void * userdata,
                                                   const char * value )
{
    rtl::OUString aHeader( rtl::OUString::createFromAscii( value ) );
    sal_Int32 nPos = aHeader.indexOf( ':' );

    if ( nPos != -1 )
    {
        rtl::OUString aHeaderName( aHeader.copy( 0, nPos ) );

        NeonRequestContext * pCtx
            = static_cast< NeonRequestContext * >( userdata );

        // Note: Empty vector means that all headers are requested.
        bool bIncludeIt = ( pCtx->pHeaderNames->size() == 0 );

        if ( !bIncludeIt )
        {
            // Check whether this header was requested.
            std::vector< ::rtl::OUString >::const_iterator it(
                pCtx->pHeaderNames->begin() );
            const std::vector< ::rtl::OUString >::const_iterator end(
                pCtx->pHeaderNames->end() );

            while ( it != end )
            {
                if ( (*it) == aHeaderName )
                    break;

                ++it;
            }

            if ( it != end )
                bIncludeIt = true;
        }

        if ( bIncludeIt )
        {
            // Create & set the PropertyValue
            beans::PropertyValue thePropertyValue;
            thePropertyValue.Handle = -1;
            thePropertyValue.Name   = aHeaderName;
            thePropertyValue.State  = beans::PropertyState_DIRECT_VALUE;

            if ( nPos < aHeader.getLength() )
                thePropertyValue.Value <<= aHeader.copy( nPos + 1 ).trim();

            // Add the newly created PropertyValue
            pCtx->pResource->properties.push_back( thePropertyValue );
        }
    }
}

// -------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------
NeonSession::NeonSession(
            rtl::Reference< DAVSessionFactory > const & rSessionFactory,
            const rtl::OUString& inUri,
            const ucbhelper::InternetProxyServer& rProxyCfg )
    throw ( DAVException )
: DAVSession( rSessionFactory ),
  m_pRequestData( new RequestDataMap )
{
    // @@@ We need to keep the char buffer for hostname and proxyname
    // for the whole session lifetime because neon only stores a pointer
    // to that buffer (last verified with neon 0.11.0)!!! We do this
    // by having the members mHostName and mProxyName, which are OStrings!

    NeonUri theUri( inUri );
    m_aScheme    = theUri.GetScheme();
    m_aHostName  = theUri.GetHost();
    m_nPort      = theUri.GetPort();

    m_aProxyName = rProxyCfg.aName;
    m_nProxyPort = rProxyCfg.nPort;

    // Note: Uses m_aHostName and m_nPort (in case of error)
    Init();

    // @@@ For FTP over HTTP proxy inUserInfo is needed to be able to
    //     build the complete request URI (including user:pass), but
    //     currently (0.22.0) neon does not allow to pass the user info
    //     to the session

    m_pHttpSession = ne_session_create(
                rtl::OUStringToOString( m_aScheme,
                                        RTL_TEXTENCODING_UTF8 ).getStr(),
                /* theUri.GetUserInfo(),
                   @@@ for FTP via HTTP proxy, but not supported by Neon */
                rtl::OUStringToOString( m_aHostName,
                                        RTL_TEXTENCODING_UTF8 ).getStr(),
                m_nPort );

    if ( m_pHttpSession == 0 )
        throw DAVException( DAVException::DAV_SESSION_CREATE,
                            theUri.makeConnectionEndPointString() );

    // Set a progress callback for the session.
    ne_set_progress( m_pHttpSession, NeonSession_ProgressNotify, this );

    // Set a status notification callback for the session, to report
    // connection status.
    ne_set_status( m_pHttpSession, NeonSession_StatusNotify, this );

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
     * anything else signifies an error, and the request is failed. The return
     * code is passed back the _dispatch caller, so the session error must
     * also be set appropriately (ne_set_error).
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

    if ( m_aProxyName.getLength() )
    {
        ne_session_proxy( m_pHttpSession,
                          rtl::OUStringToOString( m_aProxyName,
                                                  RTL_TEXTENCODING_UTF8 )
                            .getStr(),
                          m_nProxyPort );
    }

    // Note: Uncomment the following if locking support is required
    /*
    mNeonLockSession = ne_lock_register( m_pHttpSession );

    if ( mNeonLockSession == NULL )
        throw DAVException( DAVException::DAV_SESSION_CREATE,
                            theUri::makeConnectionEndPointString() );
    */

    // Register for redirects.
    ne_redirect_register( m_pHttpSession );

    // authentication callbacks.

    // Note: Calling ne_set_[server|proxy]_auth more than once per
    //       m_pHttpSession instance sometimes(?) crashes Neon! ( last
    //       checked: 0.22.0)
    ne_set_server_auth( m_pHttpSession, NeonSession_NeonAuth, this );
    ne_set_proxy_auth ( m_pHttpSession, NeonSession_NeonAuth, this );
}

// -------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------
NeonSession::~NeonSession( )
{
    if ( m_pHttpSession )
    {
        ne_session_destroy( m_pHttpSession );
        m_pHttpSession = 0;
        // Note: Uncomment the following if locking support is required
        /*
        if ( mNeonLockSession != NULL )
        {
            ne_lock_unregister( mNeonLockSession );
            mNeonLockSession = NULL;
        }
        */
    }

    delete m_pRequestData;
}

// -------------------------------------------------------------------
// virtual
sal_Bool NeonSession::CanUse( const rtl::OUString & inUri )
{
    NeonUri theUri( inUri );
    if ( ( theUri.GetPort() == m_nPort ) &&
         ( theUri.GetHost() == m_aHostName ) &&
         ( theUri.GetScheme() == m_aScheme ) )
        return sal_True;

    return sal_False;
}

// -------------------------------------------------------------------
// virtual
sal_Bool NeonSession::UsesProxy()
{
    return ( m_aProxyName.getLength() > 0 );
}

// -------------------------------------------------------------------
// OPTIONS
// -------------------------------------------------------------------
void NeonSession::OPTIONS( const rtl::OUString & inPath,
                           DAVCapabilities & outCapabilities,
                           const DAVRequestEnvironment & rEnv )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    HttpServerCapabilities servercaps;
    memset( &servercaps, 0, sizeof( servercaps ) );

    int theRetVal = ne_options( m_pHttpSession,
                                rtl::OUStringToOString(
                                    inPath, RTL_TEXTENCODING_UTF8 ),
                                &servercaps );
    HandleError( theRetVal );

    outCapabilities.class1     = !!servercaps.dav_class1;
    outCapabilities.class2     = !!servercaps.dav_class2;
    outCapabilities.executable = !!servercaps.dav_executable;
}

// -------------------------------------------------------------------
// PROPFIND - allprop & named
// -------------------------------------------------------------------
void NeonSession::PROPFIND( const rtl::OUString &                inPath,
                            const Depth                          inDepth,
                            const std::vector< rtl::OUString > & inPropNames,
                            std::vector< DAVResource > &         ioResources,
                            const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    int theRetVal = NE_OK;
    NeonPropFindRequest theRequest( m_pHttpSession,
                                    rtl::OUStringToOString(
                                        inPath, RTL_TEXTENCODING_UTF8 ),
                                    inDepth,
                                    inPropNames,
                                    ioResources,
                                    theRetVal );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// PROPFIND - propnames
// -------------------------------------------------------------------
void NeonSession::PROPFIND( const rtl::OUString &                   inPath,
                            const Depth                             inDepth,
                            std::vector< DAVResourceInfo >&         ioResInfo,
                            const DAVRequestEnvironment & rEnv )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    int theRetVal = NE_OK;
    NeonPropFindRequest theRequest( m_pHttpSession,
                                    rtl::OUStringToOString(
                                        inPath, RTL_TEXTENCODING_UTF8 ),
                                    inDepth,
                                    ioResInfo,
                                    theRetVal );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// PROPPATCH
// -------------------------------------------------------------------
void NeonSession::PROPPATCH( const rtl::OUString &                   inPath,
                             const std::vector< ProppatchValue > &   inValues,
                             const DAVRequestEnvironment & rEnv )
    throw( DAVException )
{
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

    // Generate the list of properties we want to set.
    int nPropCount = inValues.size();
    ne_proppatch_operation* pItems
        = new ne_proppatch_operation[ nPropCount + 1 ];
    for ( int n = 0; n < nPropCount; ++n )
    {
        const ProppatchValue & rValue = inValues[ n ];

        // Split fullname into namespace and name!
        ne_propname * pName = new ne_propname;
        DAVProperties::createNeonPropName( rValue.name, *pName );
        pItems[ n ].name = pName;

        if ( rValue.operation == PROPSET )
        {
            pItems[ n ].type = ne_proppatch_operation::ne_propset;

            rtl::OUString aStringValue;
            if ( DAVProperties::isUCBDeadProperty( *pName ) )
            {
                // DAV dead property added by WebDAV UCP?
                if ( !UCBDeadPropertyValue::toXML(
                                        rValue.value, aStringValue ) )
                {
                    // Error!
                    pItems[ n ].value = 0;
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
                    uno::Sequence< ::com::sun::star::ucb::Link > aLinks;
                    if ( rValue.value >>= aLinks )
                    {
                        LinkSequence::toXML( aLinks, aStringValue );
                    }
                    else
                    {
                        // Error!
                        pItems[ n ].value = 0;
                        theRetVal = NE_ERROR;
                        nPropCount = n + 1;
                        break;
                    }
                }
                else
                {
                    OSL_ENSURE( sal_False,
                                "NeonSession::PROPPATCH - unsupported type!" );
                    // Error!
                    pItems[ n ].value = 0;
                    theRetVal = NE_ERROR;
                    nPropCount = n + 1;
                    break;
                }
            }
            pItems[ n ].value
                = strdup( rtl::OUStringToOString( aStringValue,
                                                  RTL_TEXTENCODING_UTF8 ) );
        }
        else
        {
            pItems[ n ].type  = ne_proppatch_operation::ne_propremove;
            pItems[ n ].value = 0;
        }
    }

    if ( theRetVal == NE_OK )
    {
        osl::Guard< osl::Mutex > theGuard( m_aMutex );

        m_aEnv = rEnv;

        pItems[ n ].name = NULL;

        theRetVal = ne_proppatch( m_pHttpSession,
                                  rtl::OUStringToOString(
                                            inPath, RTL_TEXTENCODING_UTF8 ),
                                  pItems );
    }

    for ( n = 0; n < nPropCount; ++n )
    {
        free( (void *)pItems[ n ].name->name );
        delete (void *)pItems[ n ].name;
        free( (void *)pItems[ n ].value );
    }

    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// HEAD
// -------------------------------------------------------------------
void NeonSession::HEAD( const ::rtl::OUString &  inPath,
                        const std::vector< ::rtl::OUString > & inHeaderNames,
                        DAVResource & ioResource,
                        const DAVRequestEnvironment & rEnv )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    int theRetVal = NE_OK;
    NeonHeadRequest theRequest( m_pHttpSession,
                                inPath,
                                inHeaderNames,
                                ioResource,
                                theRetVal );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// GET
// -------------------------------------------------------------------
uno::Reference< io::XInputStream > NeonSession::GET(
                                    const rtl::OUString &           inPath,
                                    const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    rtl::Reference< NeonInputStream > xInputStream( new NeonInputStream );
    NeonRequestContext aCtx( xInputStream );
    int theRetVal = GET( m_pHttpSession,
                         rtl::OUStringToOString(
                             inPath, RTL_TEXTENCODING_UTF8 ),
                         NeonSession_ResponseBlockReader,
                         0,
                         &aCtx );
    HandleError( theRetVal );
    return uno::Reference< io::XInputStream >( xInputStream.get() );
}


// -------------------------------------------------------------------
// GET
// -------------------------------------------------------------------
void
NeonSession::GET(
    void* userData,
    const ::rtl::OUString & inPath,
    const DAVRequestEnvironment & rEnv )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    NeonRequestContext aCtx( userData );
    int theRetVal = GET( m_pHttpSession,
                         rtl::OUStringToOString(
                             inPath, RTL_TEXTENCODING_UTF8 ),
                         NeonSession_ResponseBlockReader,
                         0,
                         &aCtx );
    HandleError( theRetVal );
}


// -------------------------------------------------------------------
// GET
// -------------------------------------------------------------------
void NeonSession::GET( const rtl::OUString &                 inPath,
                       uno::Reference< io::XOutputStream > & ioOutputStream,
                       const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    NeonRequestContext aCtx( ioOutputStream );
    int theRetVal = GET( m_pHttpSession,
                         rtl::OUStringToOString(
                             inPath, RTL_TEXTENCODING_UTF8 ),
                         NeonSession_ResponseBlockWriter,
                         0,
                         &aCtx );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// GET
// -------------------------------------------------------------------
uno::Reference< io::XInputStream > NeonSession::GET(
                    const rtl::OUString & inPath,
                    const std::vector< ::rtl::OUString > & inHeaderNames,
                    DAVResource & ioResource,
                    const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    ioResource.uri = inPath;
    ioResource.properties.clear();

    rtl::Reference< NeonInputStream > xInputStream( new NeonInputStream );
    NeonRequestContext aCtx( xInputStream, inHeaderNames, ioResource );
    int theRetVal = GET( m_pHttpSession,
                         rtl::OUStringToOString(
                             inPath, RTL_TEXTENCODING_UTF8 ),
                         NeonSession_ResponseBlockReader,
                         NeonSession_ResponseHeaderCatcher,
                         &aCtx );
    HandleError( theRetVal );
    return uno::Reference< io::XInputStream >( xInputStream.get() );
}


// -------------------------------------------------------------------
// GET
// -------------------------------------------------------------------
void
NeonSession::GET(
    void* userData,
    const ::rtl::OUString & inPath,
    const std::vector< ::rtl::OUString > & inHeaderNames,
    DAVResource & ioResource,
    const DAVRequestEnvironment & rEnv )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    ioResource.uri = inPath;
    ioResource.properties.clear();

    NeonRequestContext aCtx( userData,inHeaderNames,ioResource );
    int theRetVal = GET( m_pHttpSession,
                         rtl::OUStringToOString(
                             inPath, RTL_TEXTENCODING_UTF8 ),
                         NeonSession_ResponseBlockReader,
                         NeonSession_ResponseHeaderCatcher,
                         &aCtx );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// GET
// -------------------------------------------------------------------
void NeonSession::GET(
                    const rtl::OUString & inPath,
                    uno::Reference< io::XOutputStream > & ioOutputStream,
                    const std::vector< ::rtl::OUString > & inHeaderNames,
                    DAVResource & ioResource,
                    const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    ioResource.uri = inPath;
    ioResource.properties.clear();

    NeonRequestContext aCtx( ioOutputStream, inHeaderNames, ioResource );
    int theRetVal = GET( m_pHttpSession,
                         rtl::OUStringToOString(
                             inPath, RTL_TEXTENCODING_UTF8 ),
                         NeonSession_ResponseBlockWriter,
                         NeonSession_ResponseHeaderCatcher,
                         &aCtx );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// PUT
// -------------------------------------------------------------------
void NeonSession::PUT( const rtl::OUString &                      inPath,
                       const uno::Reference< io::XInputStream > & inInputStream,
                       const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    uno::Sequence< sal_Int8 > aDataToSend;
    if ( !getDataFromInputStream( inInputStream, aDataToSend ) )
    {
        // @@@ error
        return;
    }

    int theRetVal = PUT( m_pHttpSession,
                         rtl::OUStringToOString(
                            inPath, RTL_TEXTENCODING_UTF8 ),
                         reinterpret_cast< const char * >(
                            aDataToSend.getConstArray() ),
                         aDataToSend.getLength() );

    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// POST
// -------------------------------------------------------------------
uno::Reference< io::XInputStream > NeonSession::POST(
                    const rtl::OUString & inPath,
                    const rtl::OUString & rContentType,
                    const rtl::OUString & rReferer,
                    const uno::Reference< io::XInputStream > & inInputStream,
                    const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    uno::Sequence< sal_Int8 > aDataToSend;
    if ( !getDataFromInputStream( inInputStream, aDataToSend ) )
    {
        // @@@ error
        return uno::Reference< io::XInputStream >();
    }

    m_aEnv = rEnv;

    rtl::Reference< NeonInputStream > xInputStream( new NeonInputStream );
    NeonRequestContext aCtx( xInputStream );
    int theRetVal = POST( m_pHttpSession,
                          rtl::OUStringToOString(
                            inPath, RTL_TEXTENCODING_UTF8 ),
                          reinterpret_cast< const char * >(
                              aDataToSend.getConstArray() ),
                          NeonSession_ResponseBlockReader,
                          &aCtx,
                          rContentType,
                          rReferer );

    HandleError( theRetVal );
    return uno::Reference< io::XInputStream >( xInputStream.get() );
}

// -------------------------------------------------------------------
// POST
// -------------------------------------------------------------------
void NeonSession::POST( const rtl::OUString & inPath,
                        const rtl::OUString & rContentType,
                        const rtl::OUString & rReferer,
                        const uno::Reference< io::XInputStream > & inInputStream,
                        uno::Reference< io::XOutputStream > & oOutputStream,
                        const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    uno::Sequence< sal_Int8 > aDataToSend;
    if ( !getDataFromInputStream( inInputStream, aDataToSend ) )
    {
        // @@@ error
    }

    m_aEnv = rEnv;

    NeonRequestContext aCtx( oOutputStream );
    int theRetVal = POST( m_pHttpSession,
                          rtl::OUStringToOString(
                            inPath, RTL_TEXTENCODING_UTF8 ),
                          reinterpret_cast< const char * >(
                              aDataToSend.getConstArray() ),
                          NeonSession_ResponseBlockWriter,
                          &aCtx,
                          rContentType,
                          rReferer );

    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// MKCOL
// -------------------------------------------------------------------
void NeonSession::MKCOL( const rtl::OUString &                  inPath,
                         const DAVRequestEnvironment & rEnv )
     throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    int theRetVal = ne_mkcol( m_pHttpSession,
                              rtl::OUStringToOString(
                                    inPath, RTL_TEXTENCODING_UTF8 ) );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// COPY
// -------------------------------------------------------------------
void NeonSession::COPY( const rtl::OUString &         inSourceURL,
                        const rtl::OUString &         inDestinationURL,
                        const DAVRequestEnvironment & rEnv,
                        sal_Bool                      inOverWrite )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    NeonUri theSourceUri( inSourceURL );
    NeonUri theDestinationUri( inDestinationURL );

    int theRetVal = ne_copy( m_pHttpSession,
                             inOverWrite ? 1 : 0,
                             NE_DEPTH_INFINITE,
                             rtl::OUStringToOString(
                                        theSourceUri.GetPath(),
                                        RTL_TEXTENCODING_UTF8 ),
                             rtl::OUStringToOString(
                                        theDestinationUri.GetPath(),
                                        RTL_TEXTENCODING_UTF8 ) );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// MOVE
// -------------------------------------------------------------------
void NeonSession::MOVE( const rtl::OUString &         inSourceURL,
                        const rtl::OUString &         inDestinationURL,
                        const DAVRequestEnvironment & rEnv,
                        sal_Bool                      inOverWrite )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    NeonUri theSourceUri( inSourceURL );
    NeonUri theDestinationUri( inDestinationURL );
    int theRetVal = ne_move( m_pHttpSession,
                             inOverWrite ? 1 : 0,
                             rtl::OUStringToOString(
                                                theSourceUri.GetPath(),
                                                RTL_TEXTENCODING_UTF8 ),
                             rtl::OUStringToOString(
                                                theDestinationUri.GetPath(),
                                                RTL_TEXTENCODING_UTF8 ) );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// DESTROY
// -------------------------------------------------------------------
void NeonSession::DESTROY( const rtl::OUString & inPath,
                           const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    int theRetVal = ne_delete( m_pHttpSession,
                               rtl::OUStringToOString(
                                    inPath, RTL_TEXTENCODING_UTF8 ) );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// LOCK
// -------------------------------------------------------------------
// Note: Uncomment the following if locking support is required
/*
void NeonSession::LOCK( const Lock & inLock,
                        const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    Lockit( inLock, true );
}
*/

// -------------------------------------------------------------------
// UNLOCK
// -------------------------------------------------------------------
// Note: Uncomment the following if locking support is required
/*
void NeonSession::UNLOCK( const Lock & inLock,
                          const DAVRequestEnvironment & rEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    m_aEnv = rEnv;

    Lockit( inLock, false );
}
*/

// -------------------------------------------------------------------
// Init
// Initialises "Neon sockets"
// -------------------------------------------------------------------
void NeonSession::Init( void )
    throw ( DAVException )
{
    if ( m_bSockInited == sal_False )
    {
        if ( ne_sock_init() != 0 )
            throw DAVException( DAVException::DAV_SESSION_CREATE,
                                NeonUri::makeConnectionEndPointString(
                                                    m_aHostName, m_nPort ) );
        m_bSockInited = sal_True;
    }
}

// -------------------------------------------------------------------
// HandleError
// Common Error Handler
// -------------------------------------------------------------------
void NeonSession::HandleError( int nError )
    throw ( DAVException )
{
    m_aEnv = DAVRequestEnvironment();

    // Map error code to DAVException.
    switch ( nError )
    {
        case NE_OK:
            // Cleanup.
            return;

        case NE_ERROR:        // Generic error
        {
            rtl::OUString aText = rtl::OUString::createFromAscii(
                                        ne_get_error( m_pHttpSession ) );
            throw DAVException( DAVException::DAV_HTTP_ERROR,
                                aText,
                                makeStatusCode( aText ) );
        }

        case NE_LOOKUP:       // Name lookup failed.
            throw DAVException( DAVException::DAV_HTTP_LOOKUP,
                                NeonUri::makeConnectionEndPointString(
                                                    m_aHostName, m_nPort ) );

        case NE_AUTH:         // User authentication failed on server
            throw DAVException( DAVException::DAV_HTTP_AUTH,
                                NeonUri::makeConnectionEndPointString(
                                                    m_aHostName, m_nPort ) );

        case NE_PROXYAUTH:    // User authentication failed on proxy
            throw DAVException( DAVException::DAV_HTTP_AUTHPROXY,
                                NeonUri::makeConnectionEndPointString(
                                                m_aProxyName, m_nProxyPort ) );

        case NE_CONNECT:      // Could not connect to server
            throw DAVException( DAVException::DAV_HTTP_CONNECT,
                                NeonUri::makeConnectionEndPointString(
                                                    m_aHostName, m_nPort ) );

        case NE_TIMEOUT:      // Connection timed out
            throw DAVException( DAVException::DAV_HTTP_TIMEOUT,
                                NeonUri::makeConnectionEndPointString(
                                                    m_aHostName, m_nPort ) );

        case NE_FAILED:       // The precondition failed
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
            throw DAVException(
                    DAVException::DAV_HTTP_REDIRECT, aUri.GetURI() );
        }
        default:
        {
            OSL_TRACE( "NeonSession::HandleError : Unknown Neon error code!" );
            throw DAVException( DAVException::DAV_HTTP_ERROR,
                                rtl::OUString::createFromAscii(
                                    ne_get_error( m_pHttpSession ) ) );
        }
    }
}

// Note: Uncomment the following if locking support is required
/*
void NeonSession::Lockit( const Lock & inLock, bool inLockit )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( m_aMutex );

    // Create the neon lock
    NeonLock * theLock = new NeonLock;
    int theRetVal;

    // Set the lock uri
    NeonUri theUri( inLock.uri );
    theLock->uri = const_cast< char * >
        ( rtl::OUStringToOString(
                theUri.GetPath(), RTL_TEXTENCODING_UTF8 ).getStr() );

    if ( inLockit )
    {
        // Set the lock depth
        switch( inLock.depth )
        {
            case ZERO:
            case INFINITY:
                theLock->depth = int ( inLock.depth );
                break;
            default:
                throw DAVException( DAVException::DAV_INVALID_ARG );
                break;
        }

        // Set the lock scope
        switch ( inLock.scope )
        {
            case EXCLUSIVE:
                theLock->scope = ne_lockscope_exclusive;
                break;
            case SHARED:
                theLock->scope = ne_lockscope_shared;
                break;
            default:
                throw DAVException( DAVException::DAV_INVALID_ARG );
                break;
        }

        // Set the lock owner
        const char * theOwner = rtl::OUStringToOString( inLock.owner,
                                                        RTL_TEXTENCODING_UTF8 );
        theLock->owner = const_cast< char * > ( theOwner );

        // Set the lock timeout
        // Note: Neon ignores the timeout
        //theLock->timeout = inLock.timeout;

        theRetVal = ne_lock( m_pHttpSession, theLock );
    }
    else
    {

        // Set the lock token
        rtl::OUString theToken = inLock.locktoken.getConstArray()[ 0 ];
        theLock->token = const_cast< char * >
            ( rtl::OUStringToOString(
                    theToken, RTL_TEXTENCODING_UTF8 ).getStr() );

        theRetVal = ne_unlock( m_pHttpSession, theLock );
    }

    HandleError( theRetVal );
}
*/

// static
int NeonSession::GET( ne_session * sess,
                      const char * uri,
                      ne_block_reader reader,
                      ne_header_handler handler,
                      void * userdata )
{
    //struct get_context ctx;
    ne_request * req = ne_request_create( sess, "GET", uri );
    int ret;

    ne_add_response_header_catcher( req, handler, userdata );

    ne_add_response_body_reader( req, ne_accept_2xx, reader, userdata );

    ret = ne_request_dispatch( req );

    if ( ret == NE_OK && ne_get_status( req )->klass != 2 )
        ret = NE_ERROR;

    ne_request_destroy( req );
    return ret;
}

// static
int NeonSession::PUT( ne_session * sess,
                      const char * uri,
                      const char * buffer,
                      size_t size)
{
    ne_request * req = ne_request_create( sess, "PUT", uri );
    int ret;

#ifdef USE_DAV_LOCKS
    ne_lock_using_resource( req, uri, 0 );
    ne_lock_using_parent( req, uri );
#endif

    ne_set_request_body_buffer( req, buffer, size );

    ret = ne_request_dispatch( req );

    if ( ret == NE_OK && ne_get_status( req )->klass != 2 )
        ret = NE_ERROR;

    ne_request_destroy( req );
    return ret;
}

//
int NeonSession::POST( ne_session * sess,
                       const char * uri,
                       const char * buffer,
                       ne_block_reader reader,
                       void * userdata,
                       const rtl::OUString & rContentType,
                       const rtl::OUString & rReferer )
{
    ne_request * req = ne_request_create( sess, "POST", uri );
    //struct get_context ctx;
    int ret;

    RequestDataMap * pData = 0;

    if ( rContentType.getLength() || rReferer.getLength() )
    {
        // Remember contenttype and referer. Data will be added to HTTP request
        // header in in 'PreSendRequest' callback.
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

    ret = ne_request_dispatch( req );

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

// -------------------------------------------------------------------
// static
bool NeonSession::getDataFromInputStream(
                            const uno::Reference< io::XInputStream > & xStream,
                            uno::Sequence< sal_Int8 > & rData )
{
    if ( xStream.is() )
    {
        uno::Reference< io::XSeekable > xSeekable( xStream, uno::UNO_QUERY );
        if ( xSeekable.is() )
        {
            try
            {
                sal_Int32 nSize = xSeekable->getLength();
                sal_Int32 nRead = xStream->readBytes( rData, nSize );

                if ( nRead == nSize )
                {
                    rData.realloc( nSize + 1 );
                    rData[ nSize ] = sal_Int8( 0 );
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
                    rtl_copyMemory( (void*)( rData.getArray() + nPos ),
                                    (const void*)aBuffer.getConstArray(),
                                    nRead );
                    nPos += nRead;

                    aBuffer.realloc( 0 );
                    nRead = xStream->readSomeBytes( aBuffer, 65536 );
                }

                rData.realloc( nPos + 1 );
                rData[ nPos ] = sal_Int8( 0 );
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


