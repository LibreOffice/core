/*************************************************************************
 *
 *  $RCSfile: NeonSession.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kso $ $Date: 2001-06-25 08:51:54 $
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

#ifndef HTTPAUTH_H
#include <http_auth.h>
#endif
#ifndef HTTP_REDIRECT_H
#include <http_redirect.h>
#endif
#ifndef DAV_BASIC_H
#include <dav_basic.h>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
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
#ifndef _NEONPUTFILE_HXX_
#include "NeonPUTFile.hxx"
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

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::ucb;
using namespace webdav_ucp;

sal_Bool NeonSession::sSockInited = sal_False;

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
// Constructor
// -------------------------------------------------------------------
NeonSession::NeonSession( DAVSessionFactory* pSessionFactory,
                          const OUString& inUri,
                          const ProxyConfig& rProxyCfg )
    throw ( DAVException )
: m_pSessionFactory( pSessionFactory )
{
    // @@@ We need to keep the char buffer for hostname and proxyname
    // for the whole session lifetime because neon only stores a pointer
    // to that buffer (last verified with neon 0.11.0)!!! We do this
    // by having the members mHostName and mProxyName, which are OStrings!

    NeonUri theUri( inUri );

    mHostName = theUri.GetHost();
    mPort = theUri.GetPort();

    mProxyName = rProxyCfg.aName;
    mProxyPort = rProxyCfg.nPort;

    Init();

    mHttpSession = CreateSession( mHostName,
                                  theUri.GetPort(),
                                  mProxyName,
                                  rProxyCfg.nPort );
    if ( mHttpSession == NULL )
        throw DAVException( DAVException::DAV_SESSION_CREATE,
                            theUri.makeConnectionEndPointString() );

    // Note: Uncomment the following if locking support is required
    /*
    mNeonLockSession = dav_lock_register( mHttpSession );

    if ( mNeonLockSession == NULL )
        throw DAVException( DAVException::DAV_SESSION_CREATE,
                            theUri::makeConnectionEndPointString() );
    */

    // Register redirect callbacks.
    http_redirect_register(
            mHttpSession, RedirectConfirm, RedirectNotify, this );
}

// -------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------
NeonSession::~NeonSession( )
{
    m_pSessionFactory->ReleaseDAVSession( this );

    if ( mHttpSession != NULL )
    {
        http_session_destroy( mHttpSession );
        mHttpSession = NULL;
        // Note: Uncomment the following if locking support is required
        /*
        if ( mNeonLockSession != NULL )
        {
            dav_lock_unregister( mNeonLockSession );
            mNeonLockSession = NULL;
        }
        */
    }
}

sal_Bool NeonSession::CanUse( const OUString & inUri )
{
    sal_Bool IsConnected = sal_False;
    NeonUri theUri( inUri );
    if ( ( theUri.GetPort() == mPort ) &&
         ( theUri.GetHost() == mHostName ) )
         IsConnected = sal_True;
    return IsConnected;
}

void NeonSession::setServerAuthListener(DAVAuthListener * inDAVAuthListener)
{
    if ( inDAVAuthListener != NULL )
    {
        mListener = inDAVAuthListener;
        http_set_server_auth( mHttpSession, NeonAuth, this );
    }
}

void NeonSession::setProxyAuthListener(DAVAuthListener * inDAVAuthListener)
{
    // Note: Content is currently not using proxy auth
}

// -------------------------------------------------------------------
// OPTIONS
// -------------------------------------------------------------------
void NeonSession::OPTIONS( const OUString & inPath,
                           DAVCapabilities & outCapabilities,
                           const com::sun::star::uno::Reference<
                               com::sun::star::ucb::XCommandEnvironment >& inEnv )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( mMutex );

    mEnv = inEnv;

    HttpServerCapabilities servercaps;
    memset( &servercaps, 0, sizeof( servercaps ) );

    int theRetVal = http_options( mHttpSession,
                                   OUStringToOString( inPath,
                                                     RTL_TEXTENCODING_UTF8 ),
                                   &servercaps );
    HandleError( theRetVal );

    outCapabilities.class1     = !!servercaps.dav_class1;
    outCapabilities.class2     = !!servercaps.dav_class2;
    outCapabilities.executable = !!servercaps.dav_executable;
}

// -------------------------------------------------------------------
// PROPFIND - allprop & named
// -------------------------------------------------------------------
void NeonSession::PROPFIND( const OUString &                inPath,
                            const Depth                     inDepth,
                            const std::vector< OUString > & inPropNames,
                            std::vector< DAVResource > &    ioResources,
                            const com::sun::star::uno::Reference<
                             com::sun::star::ucb::XCommandEnvironment >& inEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( mMutex );

    mEnv = inEnv;

    int theRetVal = HTTP_OK;
    NeonPropFindRequest theRequest( mHttpSession,
                                    OUStringToOString( inPath,
                                                          RTL_TEXTENCODING_UTF8 ),
                                    inDepth,
                                    inPropNames,
                                    ioResources,
                                    theRetVal );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// PROPFIND - propnames
// -------------------------------------------------------------------
void NeonSession::PROPFIND( const ::rtl::OUString &                 inPath,
                            const Depth                             inDepth,
                            std::vector< DAVResourceInfo >&         ioResInfo,
                            const Reference< XCommandEnvironment >& inEnv )
    throw( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( mMutex );

    mEnv = inEnv;

    int theRetVal = HTTP_OK;
    NeonPropFindRequest theRequest( mHttpSession,
                                    OUStringToOString( inPath,
                                                       RTL_TEXTENCODING_UTF8 ),
                                    inDepth,
                                    ioResInfo,
                                    theRetVal );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// PROPPATCH
// -------------------------------------------------------------------
void NeonSession::PROPPATCH( const ::rtl::OUString &                 inPath,
                             const std::vector< ProppatchValue > &   inValues,
                             const Reference< XCommandEnvironment >& inEnv )
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

    int theRetVal = HTTP_OK;

    // Generate the list of properties we want to set.
    int nPropCount = inValues.size();
    dav_proppatch_operation* pItems
        = new dav_proppatch_operation[ nPropCount + 1 ];
    for ( int n = 0; n < nPropCount; ++n )
    {
        const ProppatchValue & rValue = inValues[ n ];

        // Split fullname into namespace and name!
        dav_propname * pName = new dav_propname;
        DAVProperties::createNeonPropName( rValue.name, *pName );
        pItems[ n ].name = pName;

        if ( rValue.operation == PROPSET )
        {
            pItems[ n ].type = dav_proppatch_operation::dav_propset;

            OUString aStringValue;
            if ( DAVProperties::isUCBDeadProperty( *pName ) )
            {
                // DAV dead property added by WebDAV UCP?
                if ( !UCBDeadPropertyValue::toXML(
                                        rValue.value, aStringValue ) )
                {
                    // Error!
                    pItems[ n ].value = 0;
                    theRetVal = HTTP_ERROR;
                    nPropCount = n + 1;
                    break;
                }
            }
            else if ( !( rValue.value >>= aStringValue ) )
            {
                // complex properties...
                if ( rValue.name == DAVProperties::SOURCE )
                {
                    Sequence< Link > aLinks;
                    if ( rValue.value >>= aLinks )
                    {
                        LinkSequence::toXML( aLinks, aStringValue );
                    }
                    else
                    {
                        // Error!
                        pItems[ n ].value = 0;
                        theRetVal = HTTP_ERROR;
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
                    theRetVal = HTTP_ERROR;
                    nPropCount = n + 1;
                    break;
                }
            }
            pItems[ n ].value
                = strdup( OUStringToOString( aStringValue,
                                                RTL_TEXTENCODING_UTF8 ) );
        }
        else
        {
            pItems[ n ].type  = dav_proppatch_operation::dav_propremove;
            pItems[ n ].value = 0;
        }
    }

    if ( theRetVal == HTTP_OK )
    {
        pItems[ n ].name = NULL;

        theRetVal = dav_proppatch( mHttpSession,
                                       OUStringToOString( inPath,
                                                        RTL_TEXTENCODING_UTF8 ),
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
// GET
// -------------------------------------------------------------------
Reference< XInputStream > NeonSession::GET( const OUString & inPath,
                            const com::sun::star::uno::Reference<
                             com::sun::star::ucb::XCommandEnvironment >& inEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( mMutex );

    mEnv = inEnv;

    NeonInputStream * theInputStream = new NeonInputStream;
    int theRetVal = http_read_file( mHttpSession,
                                     OUStringToOString( inPath,
                                                       RTL_TEXTENCODING_UTF8 ),
                                     GETReader,
                                     theInputStream );
    HandleError( theRetVal );
    return theInputStream;
}

// -------------------------------------------------------------------
// GET
// -------------------------------------------------------------------
void NeonSession::GET( const OUString &             inPath,
                       Reference< XOutputStream > & ioOutputStream,
                       const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XCommandEnvironment >& inEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( mMutex );

    mEnv = inEnv;

    int theRetVal = http_read_file( mHttpSession,
                                     OUStringToOString( inPath,
                                                       RTL_TEXTENCODING_UTF8 ),
                                     GETWriter,
                                     &ioOutputStream );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// PUT
// -------------------------------------------------------------------
void NeonSession::PUT( const OUString &             inPath,
                       const Reference< XInputStream > &    inInputStream,
                       const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XCommandEnvironment >& inEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( mMutex );

    mEnv = inEnv;

    NeonPUTFile thePUTFile( inInputStream );
    int theRetVal = http_put( mHttpSession,
                              OUStringToOString( inPath,
                                                 RTL_TEXTENCODING_UTF8 ),
                              thePUTFile.GetFILE() );
    thePUTFile.Remove();
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// MKCOL
// -------------------------------------------------------------------
void NeonSession::MKCOL( const OUString & inPath,
                         const com::sun::star::uno::Reference<
                          com::sun::star::ucb::XCommandEnvironment >& inEnv )
     throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( mMutex );

    mEnv = inEnv;

    int theRetVal = dav_mkcol( mHttpSession,
                               OUStringToOString( inPath,
                                                  RTL_TEXTENCODING_UTF8 ) );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// COPY
// -------------------------------------------------------------------
void NeonSession::COPY( const OUString &    inSourceURL,
                        const OUString &    inDestinationURL,
                        const com::sun::star::uno::Reference<
                         com::sun::star::ucb::XCommandEnvironment >& inEnv,
                        sal_Bool            inOverWrite )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( mMutex );

    mEnv = inEnv;

    NeonUri theSourceUri( inSourceURL );
    NeonUri theDestinationUri( inDestinationURL );

    int theRetVal = dav_copy( mHttpSession,
                              inOverWrite ? 1 : 0,
                              OUStringToOString( theSourceUri.GetPath(),
                                                 RTL_TEXTENCODING_UTF8 ),
                              OUStringToOString( theDestinationUri.GetPath(),
                                                 RTL_TEXTENCODING_UTF8 ) );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// MOVE
// -------------------------------------------------------------------
void NeonSession::MOVE( const OUString &    inSourceURL,
                        const OUString &    inDestinationURL,
                        const com::sun::star::uno::Reference<
                         com::sun::star::ucb::XCommandEnvironment >& inEnv,
                        sal_Bool            inOverWrite )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( mMutex );

    mEnv = inEnv;

    NeonUri theSourceUri( inSourceURL );
    NeonUri theDestinationUri( inDestinationURL );
    int theRetVal = dav_move( mHttpSession,
                                 inOverWrite ? 1 : 0,
                                 OUStringToOString( theSourceUri.GetPath(),
                                                 RTL_TEXTENCODING_UTF8 ),
                                 OUStringToOString( theDestinationUri.GetPath(),
                                                 RTL_TEXTENCODING_UTF8 ) );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// DESTROY
// -------------------------------------------------------------------
void NeonSession::DESTROY( const OUString & inPath,
                           const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandEnvironment >& inEnv )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( mMutex );

    mEnv = inEnv;

    int theRetVal = dav_delete( mHttpSession,
                                OUStringToOString( inPath,
                                                   RTL_TEXTENCODING_UTF8 ) );
    HandleError( theRetVal );
}

// -------------------------------------------------------------------
// LOCK
// -------------------------------------------------------------------
// Note: Uncomment the following if locking support is required
/*
void NeonSession::LOCK( const Lock & inLock,
                        const com::sun::star::uno::Reference<
                         com::sun::star::ucb::XCommandEnvironment >& inEnv )
    throw ( DAVException )
{
    mEnv = inEnv;

    Lockit( inLock, true );
}
*/

// -------------------------------------------------------------------
// UNLOCK
// -------------------------------------------------------------------
// Note: Uncomment the following if locking support is required
/*
void NeonSession::UNLOCK( const Lock & inLock,
                          const com::sun::star::uno::Reference<
                           com::sun::star::ucb::XCommandEnvironment >& inEnv )
    throw ( DAVException )
{
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
    if ( sSockInited == sal_False )
    {
        if ( sock_init() != 0 )
            throw DAVException( DAVException::DAV_SESSION_CREATE,
                                NeonUri::makeConnectionEndPointString(
                                                        mHostName, mPort ) );
        sSockInited = sal_True;
    }
}

// -------------------------------------------------------------------
// HandleError
// Common Error Handler
// -------------------------------------------------------------------
void NeonSession::HandleError( int nError )
    throw ( DAVException )
{
    // Map error code to DAVException.
    switch ( nError )
    {
        case HTTP_OK:
        {
            // Cleanup.
            mEnv = 0;
            mPrevUserName = mPrevPassWord = OUString();
            return;
        }

        case HTTP_ERROR:        // Generic error
        {
            OUString aText = OUString::createFromAscii(
                                    http_get_error( mHttpSession ) );
            throw DAVException( DAVException::DAV_HTTP_ERROR,
                                aText,
                                makeStatusCode( aText ) );
        }

        case HTTP_LOOKUP:       // Name lookup failed.
            throw DAVException( DAVException::DAV_HTTP_LOOKUP,
                                NeonUri::makeConnectionEndPointString(
                                                    mHostName, mPort ) );

        case HTTP_AUTH:         // User authentication failed on server
            throw DAVException( DAVException::DAV_HTTP_AUTH,
                                NeonUri::makeConnectionEndPointString(
                                                    mHostName, mPort ) );

        case HTTP_AUTHPROXY:    // User authentication failed on proxy
            throw DAVException( DAVException::DAV_HTTP_AUTHPROXY,
                                NeonUri::makeConnectionEndPointString(
                                                    mProxyName, mProxyPort ) );

        case HTTP_SERVERAUTH:   // Server authentication failed
            throw DAVException( DAVException::DAV_HTTP_SERVERAUTH,
                                NeonUri::makeConnectionEndPointString(
                                                    mHostName, mPort ) );

        case HTTP_PROXYAUTH:    // Proxy authentication failed
            throw DAVException( DAVException::DAV_HTTP_PROXYAUTH,
                                NeonUri::makeConnectionEndPointString(
                                                    mProxyName, mProxyPort ) );

        case HTTP_CONNECT:      // Could not connect to server
            throw DAVException( DAVException::DAV_HTTP_CONNECT,
                                NeonUri::makeConnectionEndPointString(
                                                    mHostName, mPort ) );

#if 0
        case HTTP_TIMEOUT:      // Connection timed out
        case HTTP_FAILED:       // The precondition failed
        case HTTP_RETRY:        // Retry request (http_end_request ONLY)
#endif

        case HTTP_REDIRECT:
            throw DAVException( DAVException::DAV_HTTP_REDIRECT,
                                OUString::createFromAscii(
                                    http_redirect_location( mHttpSession ) ) );
        default:
            throw DAVException( DAVException::DAV_HTTP_ERROR,
                                OUString::createFromAscii(
                                    http_get_error( mHttpSession ) ) );
    }
}

// -------------------------------------------------------------------
// CreateSession
// Creates a new neon session.
// -------------------------------------------------------------------
HttpSession * NeonSession::CreateSession( const OUString & inHostName,
                                             int inPort,
                                          const OUString & inProxyName,
                                             int inProxyPort )
    throw ( DAVException )
{
    if ( inHostName.getLength() == 0 || inPort <= 0 )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    HttpSession * theHttpSession;
    if ( ( theHttpSession = http_session_create() ) == NULL )
        throw DAVException( DAVException::DAV_SESSION_CREATE,
                            NeonUri::makeConnectionEndPointString(
                                                inHostName, inPort ) );

    // Set a progress callback for the session.
    http_set_progress( theHttpSession, ProgressNotify, theHttpSession );

    // Set a status notification callback for the session, to report
    // connection status.
    http_set_status( theHttpSession, StatusNotify, theHttpSession );

    if ( inProxyName.getLength() )
    {
        if ( http_session_proxy( theHttpSession,
                                 OUStringToOString( inProxyName,
                                                    RTL_TEXTENCODING_UTF8 )
                                    .getStr(),
                                 inProxyPort ) != HTTP_OK )
        {
            http_session_destroy( theHttpSession );
            throw DAVException( DAVException::DAV_HTTP_LOOKUP,
                                NeonUri::makeConnectionEndPointString(
                                                inProxyName, inProxyPort ) );
        }
    }

    if ( http_session_server( theHttpSession,
                              OUStringToOString( inHostName,
                                                 RTL_TEXTENCODING_UTF8 )
                                .getStr(),
                              inPort ) != HTTP_OK )
    {
        http_session_destroy( theHttpSession );
        throw DAVException( DAVException::DAV_HTTP_LOOKUP,
                            NeonUri::makeConnectionEndPointString(
                                                inHostName, inPort ) );
    }

    return theHttpSession;
}

// -------------------------------------------------------------------
// GETReader
// A simple Neon http_block_reader for use with a http GET method
// in conjunction with an XInputStream
// -------------------------------------------------------------------
void NeonSession::GETReader( void *         inUserData,
                             const char *   inBuf,
                             size_t         inLen )
{
    // neon calls this function with (inLen == 0)...
    if ( inLen > 0 )
    {
        NeonInputStream * theInputStream = ( NeonInputStream *) inUserData;
        theInputStream->AddToStream( inBuf, inLen );
    }
}

// -------------------------------------------------------------------
// GETWriter
// A simple Neon http_block_reader for use with a http GET method
// in conjunction with an XOutputStream
// -------------------------------------------------------------------
void NeonSession::GETWriter( void *         inUserData,
                             const char *   inBuf,
                             size_t         inLen )
{
    // neon calls this function with (inLen == 0)...
    if ( inLen > 0 )
    {
        Reference< XOutputStream > * theOutputStreamPtr =
                static_cast< Reference< XOutputStream > * >( inUserData );
        Reference< XOutputStream > theOutputStream = *theOutputStreamPtr;

        const Sequence< sal_Int8 > theSequence( ( sal_Int8 *) inBuf, inLen );
        theOutputStream->writeBytes( theSequence );
    }
}

// Note: Uncomment the following if locking support is required
/*
void NeonSession::Lockit( const Lock & inLock, bool inLockit )
    throw ( DAVException )
{
    osl::Guard< osl::Mutex > theGuard( mMutex );

    // Create the neon lock
    NeonLock * theLock = new NeonLock;
    int theRetVal;

    // Set the lock uri
    NeonUri theUri( inLock.uri );
    theLock->uri = const_cast< char * >
        ( OUStringToOString( theUri.GetPath(), RTL_TEXTENCODING_UTF8 ).getStr() );

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
                theLock->scope = dav_lockscope_exclusive;
                break;
            case SHARED:
                theLock->scope = dav_lockscope_shared;
                break;
            default:
                throw DAVException( DAVException::DAV_INVALID_ARG );
                break;
        }

        // Set the lock owner
        const char * theOwner =
                    OUStringToOString( inLock.owner, RTL_TEXTENCODING_UTF8 );
        theLock->owner = const_cast< char * > ( theOwner );

        // Set the lock timeout
        // Note: Neon ignores the timeout
        //theLock->timeout = inLock.timeout;

        theRetVal = dav_lock( mHttpSession, theLock );
    }
    else
    {

        // Set the lock token
        OUString theToken = inLock.locktoken.getConstArray()[ 0 ];
        theLock->token = const_cast< char * >
            ( OUStringToOString( theToken, RTL_TEXTENCODING_UTF8 ).getStr() );

        theRetVal = dav_unlock( mHttpSession, theLock );
    }

    HandleError( theRetVal );
}
*/

int NeonSession::NeonAuth( void *       inUserData,
                           const char * inRealm,
                           char **      inUserName,
                           char **      inPassWord )
{
    NeonSession * theSession = ( NeonSession * )inUserData;
    if ( !theSession->mListener )
    {
        // abort
        return -1;
    }

    OUStringBuffer theUserNameBuffer = theSession->mPrevUserName;
    OUStringBuffer thePassWordBuffer = theSession->mPrevPassWord;

    int theRetVal = theSession->mListener->authenticate(
                            OUString::createFromAscii( inRealm ),
                            theSession->mHostName,
                            theUserNameBuffer,
                            thePassWordBuffer,
                            theSession->mEnv );

    theSession->mPrevUserName = theUserNameBuffer.makeStringAndClear();
    OString theStr( theSession->mPrevUserName.getStr(),
                    theSession->mPrevUserName.getLength(),
                    RTL_TEXTENCODING_UTF8 );
    inUserName[0] = strdup( theStr.getStr() );

    theSession->mPrevPassWord = thePassWordBuffer.makeStringAndClear();
    theStr = OString( theSession->mPrevPassWord.getStr(),
                      theSession->mPrevPassWord.getLength(),
                      RTL_TEXTENCODING_UTF8 );
    inPassWord[0] = strdup( theStr.getStr() );

    return theRetVal;
}

/* Get confirmation from the user that a redirect from
 * URI 'src' to URI 'dest' is acceptable. Should return:
 *   Non-Zero to FOLLOW the redirect
 *   Zero to NOT follow the redirect
 */

// static
int NeonSession::RedirectConfirm(
                    void * userdata, const char * src, const char * dest )
{
//  NeonSession * theSession = ( NeonSession * )userdata;
    return 1;
}

/* Notify the user that a redirect has been automatically
 * followed from URI 'src' to URI 'dest'
 */

// static
void NeonSession::RedirectNotify(
                    void * userdata, const char * src, const char * dest )
{
//  NeonSession * theSession = ( NeonSession * )userdata;
}

// static
void NeonSession::ProgressNotify( void * userdata, off_t progress, off_t total )
{
    // progress: bytes read so far
    // total:    total bytes to read, -1 -> total count not known
}

// static
void NeonSession::StatusNotify(
            void * userdata, http_conn_status status, const char *info )
{
#if 0
    typedef enum {
        http_conn_namelookup, /* lookup up hostname (info = hostname) */
        http_conn_connecting, /* connecting to host (info = hostname) */
        http_conn_connected, /* connected to host (info = hostname) */
        http_conn_secure /* connection now secure (info = crypto level) */
    } http_conn_status;
#endif

    // info: hostname
}

