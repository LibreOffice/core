/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "BluetoothServer.hxx"

#include <iostream>
#include <iomanip>

#include <sal/log.hxx>

#ifdef LINUX_BLUETOOTH
  #include <glib.h>
  #include <dbus/dbus.h>
  #include <errno.h>
  #include <fcntl.h>
  #include <sys/unistd.h>
  #include <sys/socket.h>
  #include <bluetooth/bluetooth.h>
  #include <bluetooth/rfcomm.h>
  #include "BluetoothServiceRecord.hxx"
  #include "BufferedStreamSocket.hxx"
#endif

#ifdef WIN32
  // LO vs WinAPI conflict
  #undef WB_LEFT
  #undef WB_RIGHT
  #include <winsock2.h>
  #include <ws2bth.h>
  #include "BufferedStreamSocket.hxx"
#endif

#ifdef MACOSX
  #include <osl/conditn.hxx> // Include this early to avoid error as check() gets defined by some SDK header to empty
  #include <premac.h>
  #if MACOSX_SDK_VERSION == 1070 || MACOSX_SDK_VERSION == 1080
    #import <IOBluetooth/IOBluetooth.h>
  #else
    #import <CoreFoundation/CoreFoundation.h>
    #import <IOBluetooth/IOBluetoothUtilities.h>
    #import <IOBluetooth/objc/IOBluetoothSDPUUID.h>
    #import <IOBluetooth/objc/IOBluetoothSDPServiceRecord.h>
  #endif
  #include <postmac.h>
  #import "OSXBluetooth.h"
  #include "OSXBluetoothWrapper.hxx"
#endif

#ifdef __MINGW32__
// Value taken from http://msdn.microsoft.com/en-us/library/windows/desktop/ms738518%28v=vs.85%29.aspx
#define NS_BTH 16
#endif

#include "Communicator.hxx"

using namespace sd;

#ifdef LINUX_BLUETOOTH

struct DBusObject {
    OString maBusName;
    OString maPath;
    OString maInterface;

    DBusObject() { }
    DBusObject( const char *pBusName, const char *pPath, const char *pInterface )
        : maBusName( pBusName ), maPath( pPath ), maInterface( pInterface ) { }

    DBusMessage *getMethodCall( const char *pName )
    {
        return dbus_message_new_method_call( maBusName.getStr(), maPath.getStr(),
                                             maInterface.getStr(), pName );
    }
    DBusObject *cloneForInterface( const char *pInterface )
    {
        DBusObject *pObject = new DBusObject();

        pObject->maBusName = maBusName;
        pObject->maPath = maPath;
        pObject->maInterface = pInterface;

        return pObject;
    }
};

struct sd::BluetoothServer::Impl {
    // the glib mainloop running in the thread
    GMainContext *mpContext;
    DBusConnection *mpConnection;
    DBusObject *mpService;
    volatile bool mbExitMainloop;

    Impl()
        : mpContext( g_main_context_new() )
        , mpConnection( NULL )
        , mpService( NULL )
        , mbExitMainloop( false )
    { }

    DBusObject *getAdapter()
    {
        if( !mpService )
            return NULL;
        return mpService->cloneForInterface( "org.bluez.Adapter" );
    }
};

static DBusConnection *
dbusConnectToNameOnBus()
{
    DBusError aError;
    DBusConnection *pConnection;

    dbus_error_init( &aError );

    pConnection = dbus_bus_get( DBUS_BUS_SYSTEM, &aError );
    if( !pConnection || dbus_error_is_set( &aError ))
    {
        SAL_WARN( "sdremote.bluetooth", "failed to get dbus system bus: " << aError.message );
        dbus_error_free( &aError );
        return NULL;
    }

    return pConnection;
}

static DBusMessage *
sendUnrefAndWaitForReply( DBusConnection *pConnection, DBusMessage *pMsg )
{
    DBusPendingCall *pPending = NULL;

    if( !pMsg || !dbus_connection_send_with_reply( pConnection, pMsg, &pPending,
                                                   -1 /* default timeout */ ) )
    {
        SAL_WARN( "sdremote.bluetooth", "Memory allocation failed on message send" );
        dbus_message_unref( pMsg );
        return NULL;
    }
    dbus_connection_flush( pConnection );
    dbus_message_unref( pMsg );

    dbus_pending_call_block( pPending ); // block for reply

    pMsg = dbus_pending_call_steal_reply( pPending );
    if( !pMsg )
        SAL_WARN( "sdremote.bluetooth", "no valid reply / timeout" );

    dbus_pending_call_unref( pPending );
    return pMsg;
}

static DBusObject *
bluezGetDefaultService( DBusConnection *pConnection )
{
    DBusMessage *pMsg;
    DBusMessageIter it;
    const gchar* pInterfaceType = "org.bluez.Service";

    pMsg = DBusObject( "org.bluez", "/", "org.bluez.Manager" ).getMethodCall( "DefaultAdapter" );
    pMsg = sendUnrefAndWaitForReply( pConnection, pMsg );

    if(!pMsg || !dbus_message_iter_init( pMsg, &it ) )
        return NULL;

    if( DBUS_TYPE_OBJECT_PATH != dbus_message_iter_get_arg_type( &it ) )
        SAL_WARN( "sdremote.bluetooth", "invalid type of reply to DefaultAdapter: '"
                  << dbus_message_iter_get_arg_type( &it ) << "'" );
    else
    {
        const char *pObjectPath = NULL;
        dbus_message_iter_get_basic( &it, &pObjectPath );
        SAL_INFO( "sdremote.bluetooth", "DefaultAdapter retrieved: '"
                  << pObjectPath << "' '" << pInterfaceType << "'" );
        return new DBusObject( "org.bluez", pObjectPath, pInterfaceType );
    }
    dbus_message_unref( pMsg );

    return NULL;
}

static bool
bluezRegisterServiceRecord( DBusConnection *pConnection, DBusObject *pAdapter,
                            const char *pServiceRecord )
{
    DBusMessage *pMsg;
    DBusMessageIter it;

    pMsg = pAdapter->getMethodCall( "AddRecord" );
    dbus_message_iter_init_append( pMsg, &it );
    dbus_message_iter_append_basic( &it, DBUS_TYPE_STRING, &pServiceRecord );

    pMsg = sendUnrefAndWaitForReply( pConnection, pMsg );

    if( !pMsg || !dbus_message_iter_init( pMsg, &it ) ||
        dbus_message_iter_get_arg_type( &it ) != DBUS_TYPE_UINT32 )
    {
        SAL_WARN( "sdremote.bluetooth", "SDP registration failed" );
        return false;
    }

    // We ignore the uint de-registration handle we get back:
    // bluez will clean us up automatically on exit

    return true;
}

static void
bluezCreateAttachListeningSocket( GMainContext *pContext, GPollFD *pSocketFD )
{
    int nSocket;

    pSocketFD->fd = -1;

    if( ( nSocket = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM ) ) < 0 )
    {
        SAL_WARN( "sdremote.bluetooth", "failed to open bluetooth socket with error " << nSocket );
        return;
    }

    sockaddr_rc aAddr;
    // Initialize whole structure. Mainly to appease valgrind, which
    // doesn't know about the padding at the end of sockaddr_rc which
    // it will dutifully check for definedness. But also the standard
    // definition of BDADDR_ANY is unusable in C++ code, so just use
    // memset to set aAddr.rc_bdaddr to 0.
    memset( &aAddr, 0, sizeof( aAddr ) );
    aAddr.rc_family = AF_BLUETOOTH;
    aAddr.rc_channel = 5;

    int a;
    if ( ( a = bind( nSocket, (sockaddr*) &aAddr, sizeof(aAddr) ) ) < 0 ) {
        SAL_WARN( "sdremote.bluetooth", "bind failed with error" << a );
        close( nSocket );
        return;
    }

    if ( ( a = listen( nSocket, 1 ) ) < 0 )
    {
        SAL_WARN( "sdremote.bluetooth", "listen failed with error" << a );
        close( nSocket );
        return;
    }

    // set non-blocking behaviour ...
    if( fcntl( nSocket, F_SETFL, O_NONBLOCK) < 0 )
    {
        close( nSocket );
        return;
    }

    pSocketFD->fd = nSocket;
    pSocketFD->events = G_IO_IN | G_IO_PRI;
    pSocketFD->revents = 0;

    g_main_context_add_poll( pContext, pSocketFD, G_PRIORITY_DEFAULT );
}

static void
bluezDetachCloseSocket( GMainContext *pContext, GPollFD *pSocketFD )
{
    if( pSocketFD->fd >= 0 )
    {
        close( pSocketFD->fd );
        g_main_context_remove_poll( pContext, pSocketFD );
        pSocketFD->fd = -1;
    }
}

#endif // LINUX_BLUETOOTH

#if defined(MACOSX)

OSXBluetoothWrapper::OSXBluetoothWrapper( IOBluetoothRFCOMMChannel* channel ) :
    mpChannel(channel),
    mnMTU(0),
    mHaveBytes(),
    mMutex(),
    mBuffer()
{
    // silly enough, can't write more than mnMTU bytes at once
    mnMTU = [channel getMTU];

    SAL_INFO( "sdremote.bluetooth", "OSXBluetoothWrapper::OSXBluetoothWrapper(): mnMTU=" << mnMTU );
}

sal_Int32 OSXBluetoothWrapper::readLine( OString& aLine )
{
    SAL_INFO( "sdremote.bluetooth", "OSXBluetoothWrapper::readLine()" );

    while( true )
    {
        {
            SAL_INFO( "sdremote.bluetooth", "OSXBluetoothWrapper::readLine: entering mutex" );
            ::osl::MutexGuard aQueueGuard( mMutex );
            SAL_INFO( "sdremote.bluetooth", "OSXBluetoothWrapper::readLine: entered mutex" );

#ifdef SAL_LOG_INFO
            // We should have in the sal logging some standard way to
            // output char buffers with non-printables escaped.
            std::ostringstream s;
            if (mBuffer.size() > 0)
            {
                for (unsigned char *p = (unsigned char *) &mBuffer.front(); p != (unsigned char *) &mBuffer.front() + mBuffer.size(); p++)
                {
                    if (*p == '\n')
                        s << "\\n";
                    else if (*p < ' ' || *p >= 0x7F)
                        s << "\\0x" << std::hex << std::setw(2) << std::setfill('0') << (int) *p << std::setfill(' ') << std::setw(1) << std::dec;
                    else
                        s << *p;
                }
            }
            SAL_INFO( "sdremote.bluetooth", "OSXBluetoothWrapper::readLine mBuffer:  \"" << s.str() << "\"" );
#endif

            // got enough bytes to return a line?
            std::vector<char>::iterator aIt;
            if ( (aIt = find( mBuffer.begin(), mBuffer.end(), '\n' ))
                 != mBuffer.end() )
            {
                sal_uInt64 aLocation = aIt - mBuffer.begin();

                aLine = OString( &(*mBuffer.begin()), aLocation );

                mBuffer.erase( mBuffer.begin(), aIt + 1 ); // Also delete the empty line

                // yeps
                SAL_INFO( "sdremote.bluetooth", "  returning, got \"" << OStringToOUString( aLine, RTL_TEXTENCODING_UTF8 ) << "\"" );
                return aLine.getLength() + 1;
            }

            // nope - wait some more (after releasing the mutex)
            SAL_INFO( "sdremote.bluetooth", "  resetting mHaveBytes" );
            mHaveBytes.reset();
            SAL_INFO( "sdremote.bluetooth", "  leaving mutex" );
        }

        SAL_INFO( "sdremote.bluetooth", "  waiting for mHaveBytes" );
        mHaveBytes.wait();
        SAL_INFO( "sdremote.bluetooth", "OSXBluetoothWrapper::readLine: got mHaveBytes" );
    }
}

sal_Int32 OSXBluetoothWrapper::write( const void* pBuffer, sal_uInt32 n )
{
    SAL_INFO( "sdremote.bluetooth", "OSXBluetoothWrapper::write(" << pBuffer << ", " << n << ") mpChannel=" << mpChannel );

    char* ptr = (char*)pBuffer;
    sal_uInt32 nBytesWritten = 0;

    if (mpChannel == nil)
        return 0;

    while( nBytesWritten < n )
    {
        int toWrite = n - nBytesWritten;
        toWrite = toWrite <= mnMTU ? toWrite : mnMTU;
        if ( [mpChannel writeSync:ptr length:toWrite] != kIOReturnSuccess )
        {
            SAL_INFO( "sdremote.bluetooth", "  [mpChannel writeSync:" << (void *) ptr << " length:" << toWrite << "] returned error, total written " << nBytesWritten );
            return nBytesWritten;
        }
        ptr += toWrite;
        nBytesWritten += toWrite;
    }
    SAL_INFO( "sdremote.bluetooth", "  total written " << nBytesWritten );
    return nBytesWritten;
}

void OSXBluetoothWrapper::appendData(void* pBuffer, size_t len)
{
    SAL_INFO( "sdremote.bluetooth", "OSXBluetoothWrapper::appendData(" << pBuffer << ", " << len << ")" );

    if( len )
    {
        SAL_INFO( "sdremote.bluetooth", "OSXBluetoothWrapper::appendData: entering mutex" );
        ::osl::MutexGuard aQueueGuard( mMutex );
        SAL_INFO( "sdremote.bluetooth", "OSXBluetoothWrapper::appendData: entered mutex" );
        mBuffer.insert(mBuffer.begin()+mBuffer.size(),
                       (char*)pBuffer, (char *)pBuffer+len);
        SAL_INFO( "sdremote.bluetooth", "  setting mHaveBytes" );
        mHaveBytes.set();
        SAL_INFO( "sdremote.bluetooth", "  leaving mutex" );
    }
}

void OSXBluetoothWrapper::channelClosed()
{
    SAL_INFO( "sdremote.bluetooth", "OSXBluetoothWrapper::channelClosed()" );

    mpChannel = nil;
}

void incomingCallback( void *userRefCon,
                       IOBluetoothUserNotificationRef inRef,
                       IOBluetoothObjectRef objectRef )
{
    (void) inRef;

    SAL_INFO( "sdremote.bluetooth", "incomingCallback()" );

    BluetoothServer* pServer = (BluetoothServer*)userRefCon;

    IOBluetoothRFCOMMChannel* channel = [IOBluetoothRFCOMMChannel withRFCOMMChannelRef:(IOBluetoothRFCOMMChannelRef)objectRef];

    OSXBluetoothWrapper* socket = new OSXBluetoothWrapper( channel);
    Communicator* pCommunicator = new Communicator( socket );
    pServer->addCommunicator( pCommunicator );

    ChannelDelegate* delegate = [[ChannelDelegate alloc] initWithCommunicatorAndSocket: pCommunicator socket: socket];
    [channel setDelegate: delegate];
    [delegate retain];

    pCommunicator->launch();
}

void BluetoothServer::addCommunicator( Communicator* pCommunicator )
{
    mpCommunicators->push_back( pCommunicator );
}

#endif // MACOSX

#ifdef LINUX_BLUETOOTH

extern "C" {
    static gboolean ensureDiscoverable_cb(gpointer)
    {
        BluetoothServer::doEnsureDiscoverable();
        return FALSE; // remove source
    }
    static gboolean restoreDiscoverable_cb(gpointer)
    {
        BluetoothServer::doRestoreDiscoverable();
        return FALSE; // remove source
    }
}

static bool
getBooleanProperty( DBusConnection *pConnection, DBusObject *pAdapter,
                    const char *pPropertyName, bool *pBoolean )
{
    *pBoolean = false;

    if( !pAdapter )
        return false;

    DBusMessage *pMsg;
    pMsg = sendUnrefAndWaitForReply( pConnection,
                                     pAdapter->getMethodCall( "GetProperties" ) );

    DBusMessageIter it;
    if( !pMsg || !dbus_message_iter_init( pMsg, &it ) )
    {
        SAL_WARN( "sdremote.bluetooth", "no valid reply / timeout" );
        return false;
    }

    if( DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type( &it ) )
    {
        SAL_WARN( "sdremote.bluetooth", "no valid reply / timeout" );
        return false;
    }

    DBusMessageIter arrayIt;
    dbus_message_iter_recurse( &it, &arrayIt );

    while( dbus_message_iter_get_arg_type( &arrayIt ) == DBUS_TYPE_DICT_ENTRY )
    {
        DBusMessageIter dictIt;
        dbus_message_iter_recurse( &arrayIt, &dictIt );

        const char *pName = NULL;
        if( dbus_message_iter_get_arg_type( &dictIt ) == DBUS_TYPE_STRING )
        {
            dbus_message_iter_get_basic( &dictIt, &pName );
            if( pName != NULL && !strcmp( pName, pPropertyName ) )
            {
                SAL_INFO( "sdremote.bluetooth", "hit " << pPropertyName << " property" );
                dbus_message_iter_next( &dictIt );
                dbus_bool_t bBool = false;

                if( dbus_message_iter_get_arg_type( &dictIt ) == DBUS_TYPE_VARIANT )
                {
                    DBusMessageIter variantIt;
                    dbus_message_iter_recurse( &dictIt, &variantIt );

                    if( dbus_message_iter_get_arg_type( &variantIt ) == DBUS_TYPE_BOOLEAN )
                    {
                        dbus_message_iter_get_basic( &variantIt, &bBool );
                        SAL_INFO( "sdremote.bluetooth", "" << pPropertyName << " is " << bBool );
                        *pBoolean = bBool;
                        return true;
                    }
                    else
                        SAL_WARN( "sdremote.bluetooth", "" << pPropertyName << " type " <<
                                  dbus_message_iter_get_arg_type( &variantIt ) );
                }
                else
                    SAL_WARN( "sdremote.bluetooth", "variant type ? " <<
                              dbus_message_iter_get_arg_type( &dictIt ) );
            }
            else
            {
                const char *pStr = pName ? pName : "<null>";
                SAL_INFO( "sdremote.bluetooth", "property '" << pStr << "'" );
            }
        }
        else
            SAL_WARN( "sdremote.bluetooth", "unexpected property key type "
                      << dbus_message_iter_get_arg_type( &dictIt ) );
        dbus_message_iter_next( &arrayIt );
    }
    dbus_message_unref( pMsg );

    return false;
}

static void
setDiscoverable( DBusConnection *pConnection, DBusObject *pAdapter, bool bDiscoverable )
{
    SAL_INFO( "sdremote.bluetooth", "setDiscoverable to " << bDiscoverable );

    bool bPowered = false;
    if( !getBooleanProperty( pConnection, pAdapter, "Powered", &bPowered ) || !bPowered )
        return; // nothing to do

    DBusMessage *pMsg;
    DBusMessageIter it, varIt;

    // set timeout to zero
    pMsg = pAdapter->getMethodCall( "SetProperty" );
    dbus_message_iter_init_append( pMsg, &it );
    const char *pTimeoutStr = "DiscoverableTimeout";
    dbus_message_iter_append_basic( &it, DBUS_TYPE_STRING, &pTimeoutStr );
    dbus_message_iter_open_container( &it, DBUS_TYPE_VARIANT,
                                      DBUS_TYPE_UINT32_AS_STRING, &varIt );
    dbus_uint32_t nTimeout = 0;
    dbus_message_iter_append_basic( &varIt, DBUS_TYPE_UINT32, &nTimeout );
    dbus_message_iter_close_container( &it, &varIt );
    dbus_connection_send( pConnection, pMsg, NULL ); // async send - why not ?
    dbus_message_unref( pMsg );

    // set discoverable value
    pMsg = pAdapter->getMethodCall( "SetProperty" );
    dbus_message_iter_init_append( pMsg, &it );
    const char *pDiscoverableStr = "Discoverable";
    dbus_message_iter_append_basic( &it, DBUS_TYPE_STRING, &pDiscoverableStr );
    dbus_message_iter_open_container( &it, DBUS_TYPE_VARIANT,
                                      DBUS_TYPE_BOOLEAN_AS_STRING, &varIt );
    dbus_bool_t bValue = bDiscoverable;
    dbus_message_iter_append_basic( &varIt, DBUS_TYPE_BOOLEAN, &bValue );
    dbus_message_iter_close_container( &it, &varIt ); // async send - why not ?
    dbus_connection_send( pConnection, pMsg, NULL );
    dbus_message_unref( pMsg );
}

static DBusObject *
registerWithDefaultAdapter( DBusConnection *pConnection )
{
    DBusObject *pService;
    pService = bluezGetDefaultService( pConnection );
    if( !pService )
        return NULL;

    if( !bluezRegisterServiceRecord( pConnection, pService,
                                     bluetooth_service_record ) )
    {
        delete pService;
        return NULL;
    }

    return pService;
}

#endif // LINUX_BLUETOOTH

BluetoothServer::BluetoothServer( std::vector<Communicator*>* pCommunicators )
  : meWasDiscoverable( UNKNOWN ),
    mpCommunicators( pCommunicators )
{
#ifdef LINUX_BLUETOOTH
    mpImpl.reset(new BluetoothServer::Impl());
#endif
}

BluetoothServer::~BluetoothServer()
{
}

void BluetoothServer::ensureDiscoverable()
{
#ifdef LINUX_BLUETOOTH
    // Push it all across into our mainloop
    if( !spServer )
        return;
    GSource *pIdle = g_idle_source_new();
    g_source_set_callback( pIdle, ensureDiscoverable_cb, NULL, NULL );
    g_source_set_priority( pIdle, G_PRIORITY_DEFAULT );
    g_source_attach( pIdle, spServer->mpImpl->mpContext );
    g_source_unref( pIdle );
#endif
}

void BluetoothServer::restoreDiscoverable()
{
#ifdef LINUX_BLUETOOTH
    // Push it all across into our mainloop
    if( !spServer )
        return;
    GSource *pIdle = g_idle_source_new();
    g_source_set_callback( pIdle, restoreDiscoverable_cb, NULL, NULL );
    g_source_set_priority( pIdle, G_PRIORITY_DEFAULT_IDLE );
    g_source_attach( pIdle, spServer->mpImpl->mpContext );
    g_source_unref( pIdle );
#endif
}

void BluetoothServer::doEnsureDiscoverable()
{
#ifdef LINUX_BLUETOOTH
    if (!spServer->mpImpl->mpConnection ||
        spServer->meWasDiscoverable != UNKNOWN )
        return;

    // Find out if we are discoverable already ...
    DBusObject *pAdapter = spServer->mpImpl->getAdapter();
    if( !pAdapter )
        return;

    bool bDiscoverable;
    if( getBooleanProperty( spServer->mpImpl->mpConnection, pAdapter,
                            "Discoverable", &bDiscoverable ) )
    {
        spServer->meWasDiscoverable = bDiscoverable ? DISCOVERABLE : NOT_DISCOVERABLE;
        if( !bDiscoverable )
            setDiscoverable( spServer->mpImpl->mpConnection, pAdapter, true );
    }

    delete pAdapter;
#endif
}

void BluetoothServer::doRestoreDiscoverable()
{
    if( spServer->meWasDiscoverable == NOT_DISCOVERABLE )
    {
#ifdef LINUX_BLUETOOTH
        DBusObject *pAdapter = spServer->mpImpl->getAdapter();
        if( !pAdapter )
            return;
        setDiscoverable( spServer->mpImpl->mpConnection, pAdapter, false );
        delete pAdapter;
#endif
    }
    spServer->meWasDiscoverable = UNKNOWN;
}

// We have to have all our clients shut otherwise we can't
// re-bind to the same port number it appears.
void BluetoothServer::cleanupCommunicators()
{
    for (std::vector<Communicator *>::iterator it = mpCommunicators->begin();
         it != mpCommunicators->end(); ++it)
        (*it)->forceClose();
    // the hope is that all the threads then terminate cleanly and
    // clean themselves up.
}

void SAL_CALL BluetoothServer::run()
{
    SAL_INFO( "sdremote.bluetooth", "BluetoothServer::run called" );

#ifdef LINUX_BLUETOOTH
    DBusConnection *pConnection = dbusConnectToNameOnBus();
    if( !pConnection )
        return;

    // listen for connection state and power changes - we need to close
    // and re-create our socket code on suspend / resume, enable/disable
    DBusError aError;
    dbus_error_init( &aError );
    dbus_bus_add_match( pConnection, "type='signal',interface='org.bluez.Manager'", &aError );
    dbus_connection_flush( pConnection );

    // Try to setup the default adapter, otherwise wait for add/remove signal
    mpImpl->mpService = registerWithDefaultAdapter( pConnection );

    // poll on our bluetooth socket - if we can.
    GPollFD aSocketFD;
    if( mpImpl->mpService )
        bluezCreateAttachListeningSocket( mpImpl->mpContext, &aSocketFD );

    // also poll on our dbus connection
    int fd = -1;
    GPollFD aDBusFD;
    if( dbus_connection_get_unix_fd( pConnection, &fd ) && fd >= 0 )
    {
        aDBusFD.fd = fd;
        aDBusFD.events = G_IO_IN | G_IO_PRI;
        g_main_context_add_poll( mpImpl->mpContext, &aDBusFD, G_PRIORITY_DEFAULT );
    }
    else
        SAL_WARN( "sdremote.bluetooth", "failed to poll for incoming dbus signals" );

    mpImpl->mpConnection = pConnection;

    while( !mpImpl->mbExitMainloop )
    {
        aDBusFD.revents = 0;
        aSocketFD.revents = 0;
        g_main_context_iteration( mpImpl->mpContext, TRUE );

        SAL_INFO( "sdremote.bluetooth", "main-loop spin "
                  << aDBusFD.revents << " " << aSocketFD.revents );
        if( aDBusFD.revents )
        {
            dbus_connection_read_write( pConnection, 0 );
            DBusMessage *pMsg = dbus_connection_pop_message( pConnection );
            if( pMsg )
            {
                if( dbus_message_is_signal( pMsg, "org.bluez.Manager", "AdapterRemoved" ) )
                {
                    SAL_WARN( "sdremote.bluetooth", "lost adapter - cleaning up sockets" );
                    bluezDetachCloseSocket( mpImpl->mpContext, &aSocketFD );
                    cleanupCommunicators();
                }
                else if( dbus_message_is_signal( pMsg, "org.bluez.Manager", "AdapterAdded" ) ||
                         dbus_message_is_signal( pMsg, "org.bluez.Manager", "DefaultAdapterChanged" ) )
                {
                    SAL_WARN( "sdremote.bluetooth", "gained adapter - re-generating sockets" );
                    bluezDetachCloseSocket( mpImpl->mpContext, &aSocketFD );
                    cleanupCommunicators();
                    mpImpl->mpService = registerWithDefaultAdapter( pConnection );
                    if( mpImpl->mpService )
                        bluezCreateAttachListeningSocket( mpImpl->mpContext, &aSocketFD );
                }
                else
                    SAL_INFO( "sdremote.bluetooth", "unknown incoming dbus message, "
                              << " type: " << dbus_message_get_type( pMsg )
                              << " path: '" << dbus_message_get_path( pMsg )
                              << "' interface: '" << dbus_message_get_interface( pMsg )
                              << "' member: '" << dbus_message_get_member( pMsg ) );
            }
            dbus_message_unref( pMsg );
        }

        if( aSocketFD.revents )
        {
            sockaddr_rc aRemoteAddr;
            socklen_t aRemoteAddrLen = sizeof(aRemoteAddr);

            int nClient;
            SAL_INFO( "sdremote.bluetooth", "performing accept" );
            if ( ( nClient = accept( aSocketFD.fd, (sockaddr*) &aRemoteAddr, &aRemoteAddrLen)) < 0 &&
                 errno != EAGAIN )
            {
                SAL_WARN( "sdremote.bluetooth", "accept failed with errno " << errno );
            } else {
                SAL_INFO( "sdremote.bluetooth", "connection accepted " << nClient );
                Communicator* pCommunicator = new Communicator( new BufferedStreamSocket( nClient ) );
                mpCommunicators->push_back( pCommunicator );
                pCommunicator->launch();
            }
        }
    }

    g_main_context_unref( mpImpl->mpContext );
    mpImpl->mpConnection = NULL;
    mpImpl->mpContext = NULL;

#elif defined(WIN32)
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 2);

    if ( WSAStartup(wVersionRequested, &wsaData) )
    {
        return; // winsock dll couldn't be loaded
    }

    int aSocket = socket( AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM );
    if ( !aSocket )
    {
        WSACleanup();
        return;
    }
    SOCKADDR_BTH aAddr;
    aAddr.addressFamily = AF_BTH;
    aAddr.btAddr = 0;
    aAddr.serviceClassId = GUID_NULL;
    aAddr.port = BT_PORT_ANY; // Select any free socket.
    if ( bind( aSocket, (SOCKADDR*) &aAddr, sizeof(aAddr) ) == SOCKET_ERROR )
    {
        closesocket( aSocket );
        WSACleanup();
        return;
    }

    SOCKADDR aName;
    int aNameSize = sizeof(aAddr);
    getsockname( aSocket, &aName, &aNameSize ); // Retrieve the local address and port

    CSADDR_INFO aAddrInfo;
    memset( &aAddrInfo, 0, sizeof(aAddrInfo) );
    aAddrInfo.LocalAddr.lpSockaddr = &aName;
    aAddrInfo.LocalAddr.iSockaddrLength = sizeof( SOCKADDR_BTH );
    aAddrInfo.RemoteAddr.lpSockaddr = &aName;
    aAddrInfo.RemoteAddr.iSockaddrLength = sizeof( SOCKADDR_BTH );
    aAddrInfo.iSocketType = SOCK_STREAM;
    aAddrInfo.iProtocol = BTHPROTO_RFCOMM;

    // To be used for setting a custom UUID once available.
//    GUID uuid;
//    uuid.Data1 = 0x00001101;
//  memset( &uuid, 0x1000 + UUID*2^96, sizeof( GUID ) );
//    uuid.Data2 = 0;
//    uuid.Data3 = 0x1000;
//    ULONGLONG aData4 = 0x800000805F9B34FB;
//    memcpy( uuid.Data4, &aData4, sizeof(uuid.Data4) );

    WSAQUERYSET aRecord;
    memset( &aRecord, 0, sizeof(aRecord));
    aRecord.dwSize = sizeof(aRecord);
    aRecord.lpszServiceInstanceName = (char *)"LibreOffice Impress Remote Control";
    aRecord.lpszComment = (char *)"Remote control of presentations over bluetooth.";
    aRecord.lpServiceClassId = (LPGUID) &SerialPortServiceClass_UUID;
    aRecord.dwNameSpace = NS_BTH;
    aRecord.dwNumberOfCsAddrs = 1;
    aRecord.lpcsaBuffer = &aAddrInfo;

    if ( WSASetService( &aRecord, RNRSERVICE_REGISTER, 0 ) == SOCKET_ERROR )
    {
        closesocket( aSocket );
        WSACleanup();
        return;
    }

    if ( listen( aSocket, 1 ) == SOCKET_ERROR )
    {
        closesocket( aSocket );
        WSACleanup();
        return;
    }

    SOCKADDR_BTH aRemoteAddr;
    int aRemoteAddrLen = sizeof(aRemoteAddr);
    while ( true )
    {
        SOCKET socket;
        if ( (socket = accept(aSocket, (sockaddr*) &aRemoteAddr, &aRemoteAddrLen)) == INVALID_SOCKET )
        {
            closesocket( aSocket );
            WSACleanup();
            return;
        } else {
            Communicator* pCommunicator = new Communicator( new BufferedStreamSocket( socket) );
            mpCommunicators->push_back( pCommunicator );
            pCommunicator->launch();
        }
    }

#elif defined(MACOSX)
    // Build up dictionary at run-time instead of bothering with a
    // .plist file, using the Objective-C API

    // Compare to BluetoothServiceRecord.hxx

    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    NSDictionary *dict =
        [NSDictionary dictionaryWithObjectsAndKeys:

         // Service class ID list
         [NSArray arrayWithObject:
          [IOBluetoothSDPUUID uuid16: kBluetoothSDPUUID16ServiceClassSerialPort]],
         @"0001 - ServiceClassIDList",

         // Protocol descriptor list
         [NSArray arrayWithObjects:
          [NSArray arrayWithObject: [IOBluetoothSDPUUID uuid16: kBluetoothSDPUUID16L2CAP]],
          [NSArray arrayWithObjects:
           [IOBluetoothSDPUUID uuid16: kBluetoothL2CAPPSMRFCOMM],
           [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithInt: 1],
            @"DataElementSize",
            [NSNumber numberWithInt: 1],
            @"DataElementType",
            [NSNumber numberWithInt: 5], // RFCOMM port number, will be replaced if necessary automatically
            @"DataElementValue",
            nil],
           nil],
          nil],
         @"0004 - Protocol descriptor list",

         // Browse group list
         [NSArray arrayWithObject:
          [IOBluetoothSDPUUID uuid16: kBluetoothSDPUUID16ServiceClassPublicBrowseGroup]],
         @"0005 - BrowseGroupList",

         // Language base attribute ID list
         [NSArray arrayWithObjects:
          [NSData dataWithBytes: "en" length: 2],
          [NSDictionary dictionaryWithObjectsAndKeys:
           [NSNumber numberWithInt: 2],
           @"DataElementSize",
           [NSNumber numberWithInt: 1],
           @"DataElementType",
           [NSNumber numberWithInt: 0x006a], // encoding
           @"DataElementValue",
           nil],
          [NSDictionary dictionaryWithObjectsAndKeys:
           [NSNumber numberWithInt: 2],
           @"DataElementSize",
           [NSNumber numberWithInt: 1],
           @"DataElementType",
           [NSNumber numberWithInt: 0x0100], // offset
           @"DataElementValue",
           nil],
          nil],
         @"0006 - LanguageBaseAttributeIDList",

         // Bluetooth profile descriptor list
         [NSArray arrayWithObject:
          [NSArray arrayWithObjects:
           [IOBluetoothSDPUUID uuid16: kBluetoothSDPUUID16ServiceClassSerialPort],
           [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithInt: 2],
            @"DataElementSize",
            [NSNumber numberWithInt: 1],
            @"DataElementType",
            [NSNumber numberWithInt: 0x0100], // version number ?
            @"DataElementValue",
            nil],
           nil]],
         @"0009 - BluetoothProfileDescriptorList",

         // Attributes pointed to by the LanguageBaseAttributeIDList
         @"LibreOffice Impress Remote Control",
         @"0100 - ServiceName",
         @"The Document Foundation",
         @"0102 - ProviderName",
         nil];

    // Create service
    IOBluetoothSDPServiceRecordRef serviceRecordRef;
    IOReturn rc = IOBluetoothAddServiceDict((CFDictionaryRef) dict, &serviceRecordRef);

    SAL_INFO("sdremote.bluetooth", "IOBluetoothAddServiceDict returned " << rc);

    if (rc == kIOReturnSuccess)
    {
        IOBluetoothSDPServiceRecord *serviceRecord =
            [IOBluetoothSDPServiceRecord withSDPServiceRecordRef: serviceRecordRef];

        BluetoothRFCOMMChannelID channelID;
        [serviceRecord getRFCOMMChannelID: &channelID];

        BluetoothSDPServiceRecordHandle serviceRecordHandle;
        [serviceRecord getServiceRecordHandle: &serviceRecordHandle];

        // Register callback for incoming connections
        IOBluetoothUserNotificationRef callbackRef =
            IOBluetoothRegisterForFilteredRFCOMMChannelOpenNotifications(
                incomingCallback,
                this,
                channelID,
                kIOBluetoothUserNotificationChannelDirectionIncoming);

        (void) callbackRef;

        [serviceRecord release];
    }

    [pool release];

    (void) mpCommunicators;
#else
    (void) mpCommunicators; // avoid warnings about unused member
#endif
}

BluetoothServer *sd::BluetoothServer::spServer = NULL;

void BluetoothServer::setup( std::vector<Communicator*>* pCommunicators )
{
    if (spServer)
        return;

    spServer = new BluetoothServer( pCommunicators );
    spServer->create();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
