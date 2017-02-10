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
#include <memory>
#include <new>

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

#ifdef _WIN32
  // LO vs WinAPI conflict
  #undef WB_LEFT
  #undef WB_RIGHT
  #include <winsock2.h>
  #include <ws2bth.h>
  #include "BufferedStreamSocket.hxx"
#endif

#ifdef MACOSX
  #include <osl/conditn.hxx>
  #include <premac.h>
  #if MACOSX_SDK_VERSION == 1080
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

static DBusObject* getBluez5Adapter(DBusConnection *pConnection);

struct sd::BluetoothServer::Impl {
    // the glib mainloop running in the thread
    GMainContext *mpContext;
    DBusConnection *mpConnection;
    DBusObject *mpService;
    volatile bool mbExitMainloop;
    enum BluezVersion { BLUEZ4, BLUEZ5, UNKNOWN };
    BluezVersion maBluezVersion;

    Impl()
        : mpContext( g_main_context_new() )
        , mpConnection( nullptr )
        , mpService( nullptr )
        , mbExitMainloop( false )
        , maBluezVersion( UNKNOWN )
    { }

    DBusObject *getAdapter()
    {
        if (mpService)
        {
            DBusObject* pAdapter = mpService->cloneForInterface( "org.bluez.Adapter" );
            return pAdapter;
        }
        else if (spServer->mpImpl->maBluezVersion == BLUEZ5)
        {
            return getBluez5Adapter(mpConnection);
        }
        else
        {
            return nullptr;
        }
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
        return nullptr;
    }

    return pConnection;
}

static DBusMessage *
sendUnrefAndWaitForReply( DBusConnection *pConnection, DBusMessage *pMsg )
{
    DBusPendingCall *pPending = nullptr;

    if( !pMsg || !dbus_connection_send_with_reply( pConnection, pMsg, &pPending,
                                                   -1 /* default timeout */ ) )
    {
        SAL_WARN( "sdremote.bluetooth", "Memory allocation failed on message send" );
        dbus_message_unref( pMsg );
        return nullptr;
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

static bool
isBluez5Available(DBusConnection *pConnection)
{
    DBusMessage *pMsg;

    // Simplest wasy to check whether we have Bluez 5+ is to check
    // that we can obtain adapters using the new interfaces.
    // The first two error checks however don't tell us anything as they should
    // succeed as long as dbus is working correctly.
    pMsg = DBusObject( "org.bluez", "/", "org.freedesktop.DBus.ObjectManager" ).getMethodCall( "GetManagedObjects" );
    if (!pMsg)
    {
        SAL_INFO("sdremote.bluetooth", "No GetManagedObjects call created");
        return false;
    }

    pMsg = sendUnrefAndWaitForReply( pConnection, pMsg );
    if (!pMsg)
    {
        SAL_INFO("sdremote.bluetooth", "No reply received");
        return false;
    }

    // If dbus is working correctly and we aren't on bluez 5 this is where we
    // should actually get the error.
    if (dbus_message_get_error_name( pMsg ))
    {
        SAL_INFO( "sdremote.bluetooth", "GetManagedObjects call failed with \""
                    << dbus_message_get_error_name( pMsg )
                    << "\" -- we don't seem to have Bluez 5 available");
        return false;
    }
    SAL_INFO("sdremote.bluetooth", "GetManagedObjects call seems to have succeeded -- we must be on Bluez 5");
    dbus_message_unref(pMsg);
    return true;
}

static DBusObject*
getBluez5Adapter(DBusConnection *pConnection)
{
    DBusMessage *pMsg;
    // This returns a list of objects where we need to find the first
    // org.bluez.Adapter1 .
    pMsg = DBusObject( "org.bluez", "/", "org.freedesktop.DBus.ObjectManager" ).getMethodCall( "GetManagedObjects" );
    if (!pMsg)
        return nullptr;

    const gchar* const pInterfaceType = "org.bluez.Adapter1";

    pMsg = sendUnrefAndWaitForReply( pConnection, pMsg );

    DBusMessageIter aObjectIterator;
    if (pMsg && dbus_message_iter_init(pMsg, &aObjectIterator))
    {
        if (DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&aObjectIterator))
        {
            DBusMessageIter aObject;
            dbus_message_iter_recurse(&aObjectIterator, &aObject);
            do
            {
                if (DBUS_TYPE_DICT_ENTRY == dbus_message_iter_get_arg_type(&aObject))
                {
                    DBusMessageIter aContainerIter;
                    dbus_message_iter_recurse(&aObject, &aContainerIter);
                    char *pPath = nullptr;
                    do
                    {
                        if (DBUS_TYPE_OBJECT_PATH == dbus_message_iter_get_arg_type(&aContainerIter))
                        {
                            dbus_message_iter_get_basic(&aContainerIter, &pPath);
                            SAL_INFO( "sdremote.bluetooth", "Something retrieved: '"
                            << pPath << "' '");
                        }
                        else if (DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&aContainerIter))
                        {
                            DBusMessageIter aInnerIter;
                            dbus_message_iter_recurse(&aContainerIter, &aInnerIter);
                            do
                            {
                                if (DBUS_TYPE_DICT_ENTRY == dbus_message_iter_get_arg_type(&aInnerIter))
                                {
                                    DBusMessageIter aInnerInnerIter;
                                    dbus_message_iter_recurse(&aInnerIter, &aInnerInnerIter);
                                    do
                                    {
                                        if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&aInnerInnerIter))
                                        {
                                            char* pMessage;

                                            dbus_message_iter_get_basic(&aInnerInnerIter, &pMessage);
                                            if (OString(pMessage) == "org.bluez.Adapter1")
                                            {
                                                dbus_message_unref(pMsg);
                                                if (pPath)
                                                {
                                                    return new DBusObject( "org.bluez", pPath, pInterfaceType );
                                                }
                                                assert(false); // We should already have pPath provided for us.
                                            }
                                        }
                                    }
                                    while (dbus_message_iter_next(&aInnerInnerIter));
                                }
                            }
                            while (dbus_message_iter_next(&aInnerIter));
                        }
                    }
                    while (dbus_message_iter_next(&aContainerIter));
                }
            }
            while (dbus_message_iter_next(&aObject));
        }
        dbus_message_unref(pMsg);
    }

    return nullptr;
}

static DBusObject *
bluez4GetDefaultService( DBusConnection *pConnection )
{
    DBusMessage *pMsg;
    DBusMessageIter it;
    const gchar* const pInterfaceType = "org.bluez.Service";

    // org.bluez.manager only exists for bluez 4.
    // getMethodCall should return NULL if there is any issue e.g. the
    // if org.bluez.manager doesn't exist.
    pMsg = DBusObject( "org.bluez", "/", "org.bluez.Manager" ).getMethodCall( "DefaultAdapter" );

    if (!pMsg)
    {
        SAL_WARN("sdremote.bluetooth", "Couldn't retrieve DBusObject for DefaultAdapter");
        return nullptr;
    }

    SAL_INFO("sdremote.bluetooth", "successfully retrieved org.bluez.Manager.DefaultAdapter, attempting to use.");
    pMsg = sendUnrefAndWaitForReply( pConnection, pMsg );

    if(!pMsg || !dbus_message_iter_init( pMsg, &it ) )
    {
        return nullptr;
    }

    // This works for Bluez 4
    if( DBUS_TYPE_OBJECT_PATH == dbus_message_iter_get_arg_type( &it ) )
    {
        const char *pObjectPath = nullptr;
        dbus_message_iter_get_basic( &it, &pObjectPath );
        SAL_INFO( "sdremote.bluetooth", "DefaultAdapter retrieved: '"
                << pObjectPath << "' '" << pInterfaceType << "'" );
        dbus_message_unref( pMsg );
        return new DBusObject( "org.bluez", pObjectPath, pInterfaceType );
    }
    // Some form of error, e.g. if we have bluez 5 we get a message that
    // this method doesn't exist.
    else if ( DBUS_TYPE_STRING == dbus_message_iter_get_arg_type( &it ) )
    {
        const char *pMessage = nullptr;
        dbus_message_iter_get_basic( &it, &pMessage );
        SAL_INFO( "sdremote.bluetooth", "Error message: '"
                << pMessage << "' '" << pInterfaceType << "'" );
    }
    else
    {
        SAL_INFO( "sdremote.bluetooth", "invalid type of reply to DefaultAdapter: '"
                << (const char) dbus_message_iter_get_arg_type( &it ) << "'" );
    }
    dbus_message_unref(pMsg);
    return nullptr;
}

static bool
bluez4RegisterServiceRecord( DBusConnection *pConnection, DBusObject *pAdapter,
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
    if ( ( a = bind( nSocket, reinterpret_cast<sockaddr*>(&aAddr), sizeof(aAddr) ) ) < 0 ) {
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
                for (unsigned char *p = reinterpret_cast<unsigned char *>(&mBuffer.front()); p != reinterpret_cast<unsigned char *>(&mBuffer.front()) + mBuffer.size(); p++)
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

    char const * ptr = static_cast<char const *>(pBuffer);
    sal_uInt32 nBytesWritten = 0;

    if (mpChannel == nil)
        return 0;

    while( nBytesWritten < n )
    {
        int toWrite = n - nBytesWritten;
        toWrite = toWrite <= mnMTU ? toWrite : mnMTU;
        if ( [mpChannel writeSync:const_cast<char *>(ptr) length:toWrite] != kIOReturnSuccess )
        {
            SAL_INFO( "sdremote.bluetooth", "  [mpChannel writeSync:" << static_cast<void const *>(ptr) << " length:" << toWrite << "] returned error, total written " << nBytesWritten );
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
                       static_cast<char*>(pBuffer), static_cast<char *>(pBuffer)+len);
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

    BluetoothServer* pServer = static_cast<BluetoothServer*>(userRefCon);

    IOBluetoothRFCOMMChannel* channel = [IOBluetoothRFCOMMChannel withRFCOMMChannelRef:reinterpret_cast<IOBluetoothRFCOMMChannelRef>(objectRef)];

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

/*
 * Bluez 4 uses custom methods for setting properties, whereas Bluez 5+
 * implements properties using the generic "org.freedesktop.DBus.Properties"
 * interface -- hence we have a specific Bluez 4 function to deal with the
 * old style of reading properties.
 */
static bool
getBluez4BooleanProperty( DBusConnection *pConnection, DBusObject *pAdapter,
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

        const char *pName = nullptr;
        if( dbus_message_iter_get_arg_type( &dictIt ) == DBUS_TYPE_STRING )
        {
            dbus_message_iter_get_basic( &dictIt, &pName );
            if( pName != nullptr && !strcmp( pName, pPropertyName ) )
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

/*
 * This gets an org.freedesktop.DBus.Properties boolean
 * (as opposed to the old Bluez 4 custom properties methods as visible above).
 */
static bool
getDBusBooleanProperty( DBusConnection *pConnection, DBusObject *pAdapter,
                        const char *pPropertyName, bool *pBoolean )
{
    assert( pAdapter );

    *pBoolean = false;
    bool bRet = false;

    std::unique_ptr< DBusObject > pProperties (
            pAdapter->cloneForInterface( "org.freedesktop.DBus.Properties" ) );

    DBusMessage *pMsg = pProperties->getMethodCall( "Get" );

    DBusMessageIter itIn;
    dbus_message_iter_init_append( pMsg, &itIn );
    const char* pInterface = "org.bluez.Adapter1";
    dbus_message_iter_append_basic( &itIn, DBUS_TYPE_STRING, &pInterface );
    dbus_message_iter_append_basic( &itIn, DBUS_TYPE_STRING, &pPropertyName );
    pMsg = sendUnrefAndWaitForReply( pConnection, pMsg );

    DBusMessageIter it;
    if( !pMsg || !dbus_message_iter_init( pMsg, &it ) )
    {
        SAL_WARN( "sdremote.bluetooth", "no valid reply / timeout" );
        return false;
    }

    if( DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type( &it ) )
    {
        SAL_WARN( "sdremote.bluetooth", "invalid return type" );
    }
    else
    {
        DBusMessageIter variantIt;
        dbus_message_iter_recurse( &it, &variantIt );

        if( dbus_message_iter_get_arg_type( &variantIt ) == DBUS_TYPE_BOOLEAN )
        {
            dbus_bool_t bBool = false;
            dbus_message_iter_get_basic( &variantIt, &bBool );
            SAL_INFO( "sdremote.bluetooth", "" << pPropertyName << " is " << bBool );
            *pBoolean = bBool;
            bRet = true;
        }
        else
        {
            SAL_WARN( "sdremote.bluetooth", "" << pPropertyName << " type " <<
                        dbus_message_iter_get_arg_type( &variantIt ) );
        }

        const char* pError = dbus_message_get_error_name( pMsg );
        if ( pError )
        {
            SAL_WARN( "sdremote.bluetooth",
                      "Get failed for " << pPropertyName << " on " <<
                      pAdapter->maPath  << " with error: " << pError );
        }
    }
    dbus_message_unref( pMsg );

    return bRet;
}

static void
setDBusBooleanProperty( DBusConnection *pConnection, DBusObject *pAdapter,
                        const char *pPropertyName, bool bBoolean )
{
    assert( pAdapter );

    std::unique_ptr< DBusObject > pProperties(
            pAdapter->cloneForInterface( "org.freedesktop.DBus.Properties" ) );

    DBusMessage *pMsg = pProperties->getMethodCall( "Set" );

    DBusMessageIter itIn;
    dbus_message_iter_init_append( pMsg, &itIn );
    const char* pInterface = "org.bluez.Adapter1";
    dbus_message_iter_append_basic( &itIn, DBUS_TYPE_STRING, &pInterface );
    dbus_message_iter_append_basic( &itIn, DBUS_TYPE_STRING, &pPropertyName );

    {
        DBusMessageIter varIt;
        dbus_message_iter_open_container( &itIn, DBUS_TYPE_VARIANT,
                                        DBUS_TYPE_BOOLEAN_AS_STRING, &varIt );
        dbus_bool_t bDBusBoolean = bBoolean;
        dbus_message_iter_append_basic( &varIt, DBUS_TYPE_BOOLEAN, &bDBusBoolean );
        dbus_message_iter_close_container( &itIn, &varIt );
    }

    pMsg = sendUnrefAndWaitForReply( pConnection, pMsg );

    if( !pMsg )
    {
        SAL_WARN( "sdremote.bluetooth", "no valid reply / timeout" );
    }
    else
    {
        const char* pError = dbus_message_get_error_name( pMsg );
        if ( pError )
        {
            SAL_WARN( "sdremote.bluetooth",
                      "Set failed for " << pPropertyName << " on " <<
                      pAdapter->maPath << " with error: " << pError );
        }
        dbus_message_unref( pMsg );
    }
}

static bool
getDiscoverable( DBusConnection *pConnection, DBusObject *pAdapter )
{
    if (pAdapter->maInterface == "org.bluez.Adapter") // Bluez 4
    {
        bool bDiscoverable;
        if( getBluez4BooleanProperty(pConnection, pAdapter, "Discoverable", &bDiscoverable ) )
            return bDiscoverable;
    }
    else if (pAdapter->maInterface == "org.bluez.Adapter1") // Bluez 5
    {
        bool bDiscoverable;
        if ( getDBusBooleanProperty(pConnection, pAdapter, "Discoverable", &bDiscoverable ) )
            return bDiscoverable;
    }
    return false;
}

static void
setDiscoverable( DBusConnection *pConnection, DBusObject *pAdapter, bool bDiscoverable )
{
    SAL_INFO( "sdremote.bluetooth", "setDiscoverable to " << bDiscoverable );

    if (pAdapter->maInterface == "org.bluez.Adapter") // Bluez 4
    {
        bool bPowered = false;
        if( !getBluez4BooleanProperty( pConnection, pAdapter, "Powered", &bPowered ) || !bPowered )
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
        dbus_connection_send( pConnection, pMsg, nullptr ); // async send - why not ?
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
        dbus_connection_send( pConnection, pMsg, nullptr );
        dbus_message_unref( pMsg );
    }
    else if  (pAdapter->maInterface == "org.bluez.Adapter1") // Bluez 5
    {
        setDBusBooleanProperty(pConnection, pAdapter, "Discoverable", bDiscoverable );
    }
}

static DBusObject *
registerWithDefaultAdapter( DBusConnection *pConnection )
{
    DBusObject *pService;
    pService = bluez4GetDefaultService( pConnection );
    if( pService )
    {
        if( !bluez4RegisterServiceRecord( pConnection, pService,
                                     bluetooth_service_record ) )
        {
            delete pService;
            return nullptr;
        }
    }

    return pService;
}

void ProfileUnregisterFunction
(DBusConnection *connection, void *user_data)
{
    // We specifically don't need to do anything here.
    (void) connection;
    (void) user_data;
}

DBusHandlerResult ProfileMessageFunction
(DBusConnection *pConnection, DBusMessage *pMessage, void *user_data)
{
    SAL_INFO("sdremote.bluetooth", "ProfileMessageFunction||" << dbus_message_get_interface(pMessage) << "||" <<  dbus_message_get_member(pMessage));
    DBusHandlerResult aRet = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    if (OString(dbus_message_get_interface(pMessage)).equals("org.bluez.Profile1"))
    {
        if (OString(dbus_message_get_member(pMessage)).equals("Release"))
        {
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        else if (OString(dbus_message_get_member(pMessage)).equals("NewConnection"))
        {
            if (!dbus_message_has_signature(pMessage, "oha{sv}"))
            {
                SAL_WARN("sdremote.bluetooth", "wrong signature for NewConnection");
            }

            DBusMessageIter it;
            if (!dbus_message_iter_init(pMessage, &it))
                SAL_WARN( "sdremote.bluetooth", "error init dbus" );
            else
            {
                char* pPath;
                dbus_message_iter_get_basic(&it, &pPath);
                SAL_INFO("sdremote.bluetooth", "Adapter path:" << pPath);

                if (!dbus_message_iter_next(&it))
                    SAL_WARN("sdremote.bluetooth", "not enough parameters passed");

                // DBUS_TYPE_UNIX_FD == 'h' -- doesn't exist in older versions
                // of dbus (< 1.3?) hence defined manually for now
                if ('h' == dbus_message_iter_get_arg_type(&it))
                {

                    int nDescriptor;
                    dbus_message_iter_get_basic(&it, &nDescriptor);
                    std::vector<Communicator*>* pCommunicators = static_cast<std::vector<Communicator*>*>(user_data);

                    // Bluez gives us non-blocking sockets, but our code relies
                    // on blocking behaviour.
                    (void)fcntl(nDescriptor, F_SETFL, fcntl(nDescriptor, F_GETFL) & ~O_NONBLOCK);

                    SAL_INFO( "sdremote.bluetooth", "connection accepted " << nDescriptor);
                    Communicator* pCommunicator = new Communicator( new BufferedStreamSocket( nDescriptor ) );
                    pCommunicators->push_back( pCommunicator );
                    pCommunicator->launch();
                }

                // For some reason an (empty?) reply is expected.
                DBusMessage* pRet = dbus_message_new_method_return(pMessage);
                dbus_connection_send(pConnection, pRet, nullptr);
                dbus_message_unref(pRet);

                // We could read the remote profile version and features here
                // (i.e. they are provided as part of the DBusMessage),
                // however for us they are irrelevant (as our protocol handles
                // equivalent functionality independently of whether we're on
                // bluetooth or normal network connection).
                return DBUS_HANDLER_RESULT_HANDLED;
            }
        }
        else if (OString(dbus_message_get_member(pMessage)).equals("RequestDisconnection"))
        {
            return DBUS_HANDLER_RESULT_HANDLED;
        }
    }
    SAL_WARN("sdremote.bluetooth", "Couldn't handle message correctly.");
    return aRet;

}

static void
setupBluez5Profile1(DBusConnection* pConnection, std::vector<Communicator*>* pCommunicators)
{
    bool bErr;

    SAL_INFO("sdremote.bluetooth", "Attempting to register our org.bluez.Profile1");
    static DBusObjectPathVTable aVTable;
    aVTable.unregister_function = ProfileUnregisterFunction;
    aVTable.message_function = ProfileMessageFunction;

    // dbus_connection_try_register_object_path could be used but only exists for
    // dbus-glib >= 1.2 -- we really shouldn't be trying this twice in any case.
    // (dbus_connection_try_register_object_path also returns an error with more
    // information which could be useful for debugging purposes.)
    bErr = !dbus_connection_register_object_path(pConnection, "/org/libreoffice/bluez/profile1", &aVTable, pCommunicators);

    if (bErr)
    {
        SAL_WARN("sdremote.bluetooth", "Failed to register Bluez 5 Profile1 callback, bluetooth won't work.");
    }

    dbus_connection_flush( pConnection );
}

static void
unregisterBluez5Profile(DBusConnection* pConnection)
{
    DBusMessage* pMsg = dbus_message_new_method_call("org.bluez", "/org/bluez",
                                        "org.bluez.ProfileManager1", "UnregisterProfile");
    DBusMessageIter it;
    dbus_message_iter_init_append(pMsg, &it);

    const char *pPath = "/org/libreoffice/bluez/profile1";
    dbus_message_iter_append_basic(&it, DBUS_TYPE_OBJECT_PATH, &pPath);

    pMsg = sendUnrefAndWaitForReply( pConnection, pMsg );

    if (pMsg)
        dbus_message_unref(pMsg);

    dbus_connection_unregister_object_path( pConnection, "/org/libreoffice/bluez/profile1");

    dbus_connection_flush(pConnection);
}

static bool
registerBluez5Profile(DBusConnection* pConnection, std::vector<Communicator*>* pCommunicators)
{
    setupBluez5Profile1(pConnection, pCommunicators);

    DBusMessage *pMsg;
    DBusMessageIter it;

    pMsg = dbus_message_new_method_call("org.bluez", "/org/bluez",
                                        "org.bluez.ProfileManager1", "RegisterProfile");
    dbus_message_iter_init_append(pMsg, &it);

    const char *pPath = "/org/libreoffice/bluez/profile1";
    dbus_message_iter_append_basic(&it, DBUS_TYPE_OBJECT_PATH, &pPath);
    const char *pUUID =  "spp"; // Bluez translates this to 0x1101 for spp
    dbus_message_iter_append_basic(&it, DBUS_TYPE_STRING, &pUUID);

    DBusMessageIter aOptionsIter;
    dbus_message_iter_open_container(&it, DBUS_TYPE_ARRAY, "{sv}", &aOptionsIter);

    DBusMessageIter aEntry;

    {
        dbus_message_iter_open_container(&aOptionsIter, DBUS_TYPE_DICT_ENTRY, nullptr, &aEntry);

        const char *pString = "Name";
        dbus_message_iter_append_basic(&aEntry, DBUS_TYPE_STRING, &pString);

        const char *pValue = "LibreOffice Impress Remote";
        DBusMessageIter aValue;
        dbus_message_iter_open_container(&aEntry, DBUS_TYPE_VARIANT, "s", &aValue);
        dbus_message_iter_append_basic(&aValue, DBUS_TYPE_STRING, &pValue);
        dbus_message_iter_close_container(&aEntry, &aValue);
        dbus_message_iter_close_container(&aOptionsIter, &aEntry);
    }

    dbus_message_iter_close_container(&it, &aOptionsIter);

    // Other properties that we could set (but don't, since they appear
    // to be useless for us):
    // "Service": "0x1101" (not needed, but we used to have it in the manually defined profile).
    // "Role": setting this to "server" breaks things, although we think we're a server?
    // "Channel": seems to be dealt with automatically (but we used to use 5 in the manual profile).

    bool bSuccess = true;

    pMsg = sendUnrefAndWaitForReply( pConnection, pMsg );

    DBusError aError;
    dbus_error_init(&aError);
    if (pMsg && dbus_set_error_from_message( &aError, pMsg ))
    {
        bSuccess = false;
        SAL_WARN("sdremote.bluetooth",
                 "Failed to register our Profile1 with bluez ProfileManager "
                 << (aError.message ? aError.message : "<null>"));
    }

    dbus_error_free(&aError);
    if (pMsg)
        dbus_message_unref(pMsg);

    dbus_connection_flush(pConnection);

    return bSuccess;
}

#endif // LINUX_BLUETOOTH

BluetoothServer::BluetoothServer( std::vector<Communicator*>* pCommunicators )
  : meWasDiscoverable( UNKNOWN ),
    mpCommunicators( pCommunicators )
{
#ifdef LINUX_BLUETOOTH
    // D-Bus requires the following in order to be thread-safe (and we
    // potentially access D-Bus from different threads in different places of
    // the code base):
    if (!dbus_threads_init_default()) {
        throw std::bad_alloc();
    }

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
    g_source_set_callback( pIdle, ensureDiscoverable_cb, nullptr, nullptr );
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
    g_source_set_callback( pIdle, restoreDiscoverable_cb, nullptr, nullptr );
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

    bool bDiscoverable = getDiscoverable(spServer->mpImpl->mpConnection, pAdapter );

    spServer->meWasDiscoverable = bDiscoverable ? DISCOVERABLE : NOT_DISCOVERABLE;
    if( !bDiscoverable )
        setDiscoverable( spServer->mpImpl->mpConnection, pAdapter, true );

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
    osl::Thread::setName("BluetoothServer");
#ifdef LINUX_BLUETOOTH
    DBusConnection *pConnection = dbusConnectToNameOnBus();
    if( !pConnection )
        return;

    // For either implementation we need to poll the dbus fd
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

    if (isBluez5Available(pConnection))
    {
        SAL_INFO("sdremote.bluetooth", "Using Bluez 5");
        registerBluez5Profile(pConnection, mpCommunicators);
        mpImpl->mpConnection = pConnection;
        mpImpl->maBluezVersion = Impl::BLUEZ5;

        // We don't need to listen to adapter changes anymore -- profile
        // registration is done globally for the entirety of bluez, so we only
        // need adapters when setting discoverability, which can be done
        // dynamically without the need to listen for changes.

        // TODO: exit on SD deinit
        // Probably best to do that in SdModule::~SdModule?
        while (!mpImpl->mbExitMainloop)
        {
            aDBusFD.revents = 0;
            g_main_context_iteration( mpImpl->mpContext, TRUE );
            if( aDBusFD.revents )
            {
                dbus_connection_read_write( pConnection, 0 );
                while (DBUS_DISPATCH_DATA_REMAINS == dbus_connection_get_dispatch_status( pConnection ))
                    dbus_connection_dispatch( pConnection );
            }
        }
        unregisterBluez5Profile( pConnection );
        g_main_context_unref( mpImpl->mpContext );
        mpImpl->mpConnection = nullptr;
        mpImpl->mpContext = nullptr;
        return;
    }

    // Otherwise we could be on Bluez 4 and continue as usual.
    mpImpl->maBluezVersion = Impl::BLUEZ4;

    // Try to setup the default adapter, otherwise wait for add/remove signal
    mpImpl->mpService = registerWithDefaultAdapter( pConnection );
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
                                 " type: " << dbus_message_get_type( pMsg )
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
            if ( ( nClient = accept( aSocketFD.fd, reinterpret_cast<sockaddr*>(&aRemoteAddr), &aRemoteAddrLen)) < 0 &&
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

    unregisterBluez5Profile( pConnection );
    g_main_context_unref( mpImpl->mpContext );
    mpImpl->mpConnection = nullptr;
    mpImpl->mpContext = nullptr;

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
    if ( bind( aSocket, reinterpret_cast<SOCKADDR*>(&aAddr), sizeof(aAddr) ) == SOCKET_ERROR )
    {
        closesocket( aSocket );
        WSACleanup();
        return;
    }

    SOCKADDR aName;
    int aNameSize = sizeof(aName);
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

    WSAQUERYSETW aRecord;
    memset( &aRecord, 0, sizeof(aRecord));
    aRecord.dwSize = sizeof(aRecord);
    aRecord.lpszServiceInstanceName = const_cast<wchar_t *>(
        L"LibreOffice Impress Remote Control");
    aRecord.lpszComment = const_cast<wchar_t *>(
        L"Remote control of presentations over bluetooth.");
    aRecord.lpServiceClassId = const_cast<LPGUID>(&SerialPortServiceClass_UUID);
    aRecord.dwNameSpace = NS_BTH;
    aRecord.dwNumberOfCsAddrs = 1;
    aRecord.lpcsaBuffer = &aAddrInfo;

    if (WSASetServiceW( &aRecord, RNRSERVICE_REGISTER, 0 ) == SOCKET_ERROR)
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
        if ( (socket = accept(aSocket, reinterpret_cast<sockaddr*>(&aRemoteAddr), &aRemoteAddrLen)) == INVALID_SOCKET )
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
    SAL_WNODEPRECATED_DECLARATIONS_PUSH //TODO: 10.9 IOBluetoothAddServiceDict
    IOReturn rc = IOBluetoothAddServiceDict(reinterpret_cast<CFDictionaryRef>(dict), &serviceRecordRef);
    SAL_WNODEPRECATED_DECLARATIONS_POP

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

BluetoothServer *sd::BluetoothServer::spServer = nullptr;

void BluetoothServer::setup( std::vector<Communicator*>* pCommunicators )
{
    if (spServer)
        return;

    spServer = new BluetoothServer( pCommunicators );
    spServer->create();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
