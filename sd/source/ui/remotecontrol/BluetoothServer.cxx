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

#if (defined(LINUX) && !defined(__FreeBSD_kernel__)) && defined(ENABLE_DBUS)
  #include <glib.h>
  #include <dbus/dbus-glib.h>
  #include <errno.h>
  #include <sys/unistd.h>
  #include <sys/socket.h>
  #include <bluetooth/bluetooth.h>
  #include <bluetooth/rfcomm.h>
  #define DBUS_TYPE_G_STRING_ANY_HASHTABLE (dbus_g_type_get_map( "GHashTable", G_TYPE_STRING, G_TYPE_VALUE ))
  #ifndef G_VALUE_INIT
    #define G_VALUE_INIT {0,{{0}}} // G_VALUE_INIT only present in glib >= 2.30
  #endif
  #ifndef DBusGObjectPath
    #define DBusGObjectPath char // DBusGObjectPath is only present in newer version of dbus-glib
  #endif
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
  #if MACOSX_SDK_VERSION >= 1070
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

#if (defined(LINUX) && !defined(__FreeBSD_kernel__)) && defined(ENABLE_DBUS)
DBusGProxy* bluezGetDefaultAdapter( DBusGConnection* aConnection,
                                    const gchar* aInterfaceType = "org.bluez.Adapter" )
{
    GError *aError = NULL;

    DBusGProxy *aManager = NULL;
    aManager = dbus_g_proxy_new_for_name( aConnection, "org.bluez", "/",
                                          "org.bluez.Manager" );

    if ( aManager == NULL )
    {
        SAL_WARN( "sdremote.bluetooth", "getting org.bluez.Manager failed" );
        dbus_g_connection_unref( aConnection );
        return NULL;
    }

    gboolean aResult;
    DBusGObjectPath* aAdapterPath = NULL;
    aResult = dbus_g_proxy_call( aManager, "DefaultAdapter", &aError,
                                 G_TYPE_INVALID,
                                 DBUS_TYPE_G_OBJECT_PATH, &aAdapterPath,
                                 G_TYPE_INVALID);

    g_object_unref( G_OBJECT( aManager ));
    if ( !aResult || aError )
    {
        SAL_WARN( "sdremote.bluetooth", "getting DefaultAdapter path failed" );
        if ( aError )
            g_error_free( aError );
        return NULL;
    }

    DBusGProxy *aAdapter = NULL;
    aAdapter = dbus_g_proxy_new_for_name( aConnection, "org.bluez",
                                          aAdapterPath, aInterfaceType );
    g_free( aAdapterPath );

    SAL_INFO( "sdremote.bluetooth", "DefaultAdapter retrieved" );
    return aAdapter;
}
#endif // defined(LINUX) && defined(ENABLE_DBUS)

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

sal_Int32 OSXBluetoothWrapper::readLine( rtl::OString& aLine )
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
                    else if (*p < ' ' || (*p >= 0x7F && *p <= 0xFF))
                        s << "\\0x" << std::hex << std::setw(2) << std::setfill('0') << (int) *p << std::setfill(' ') << std::setw(1) << std::dec;
                    else
                        s << *p;
                }
            }
            SAL_INFO( "sdremote.bluetooth", "  mBuffer:  \"" << s.str() << "\"" );
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

#endif // MACOSX


BluetoothServer::BluetoothServer( std::vector<Communicator*>* pCommunicators )
  : mpCommunicators( pCommunicators )
{
}

BluetoothServer::~BluetoothServer()
{
}

bool BluetoothServer::isDiscoverable()
{
#if (defined(LINUX) && !defined(__FreeBSD_kernel__)) && defined(ENABLE_DBUS)
    SAL_INFO( "sdremote.bluetooth", "BluetoothServer::isDiscoverable called" );
    g_type_init();
    gboolean aResult;

    GError *aError = NULL;

    DBusGConnection *aConnection = NULL;
    aConnection = dbus_g_bus_get( DBUS_BUS_SYSTEM, &aError );

    if ( aError != NULL ) {
        g_error_free (aError);
        SAL_INFO( "sdremote.bluetooth", "did not get DBusGConnection" );
        return false;
    }

    DBusGProxy* aAdapter = bluezGetDefaultAdapter( aConnection );
    if ( aAdapter == NULL )
    {
        dbus_g_connection_unref( aConnection );
        SAL_INFO( "sdremote.bluetooth", "did not get default adaptor" );
        return false;
    }

    GHashTable* aProperties = NULL;
    aResult = dbus_g_proxy_call( aAdapter, "GetProperties", &aError,
                                G_TYPE_INVALID,
                                DBUS_TYPE_G_STRING_ANY_HASHTABLE, &aProperties,
                                G_TYPE_INVALID);
    g_object_unref( G_OBJECT( aAdapter ));
    dbus_g_connection_unref( aConnection );
    if ( !aResult || aError )
    {
        if ( aError )
            g_error_free( aError );
        SAL_INFO( "sdremote.bluetooth", "did not get properties" );
        return false;
    }

    gboolean aIsDiscoverable = g_value_get_boolean( (GValue*) g_hash_table_lookup(
                aProperties, "Discoverable" ) );

    g_hash_table_unref( aProperties );

    SAL_INFO( "sdremote.bluetooth", "BluetoothServer::isDiscoverable() returns " << static_cast< bool >( aIsDiscoverable ) );
    return aIsDiscoverable;
#else // defined(LINUX) && defined(ENABLE_DBUS)
    return false;
#endif
}

void BluetoothServer::setDiscoverable( bool aDiscoverable )
{
#if (defined(LINUX) && !defined(__FreeBSD_kernel__)) && defined(ENABLE_DBUS)
    SAL_INFO( "sdremote.bluetooth", "BluetoothServer::setDiscoverable called" );
    g_type_init();
    gboolean aResult;

    GError *aError = NULL;

    DBusGConnection *aConnection = NULL;
    aConnection = dbus_g_bus_get( DBUS_BUS_SYSTEM, &aError );

    if ( aError != NULL )
    {
        g_error_free (aError);
        return;
    }

    DBusGProxy* aAdapter = bluezGetDefaultAdapter( aConnection );
    if ( aAdapter == NULL )
    {
        dbus_g_connection_unref( aConnection );
        return;
    }

    GHashTable* aProperties;
    aResult = dbus_g_proxy_call( aAdapter, "GetProperties", &aError,
                                G_TYPE_INVALID,
                                DBUS_TYPE_G_STRING_ANY_HASHTABLE, &aProperties,
                                G_TYPE_INVALID);

    if ( !aResult || aError )
    {
        SAL_WARN( "sdremote.bluetooth", "GetProperties failed" );
        if ( aError )
        {
            g_error_free( aError );
            SAL_WARN( "sdremote.bluetooth", "with error " << aError->message );
        }
        return;
    }

    gboolean aPowered = g_value_get_boolean( (GValue*) g_hash_table_lookup(
                aProperties, "Powered" ) );

    g_hash_table_unref( aProperties );
    if ( !aPowered )
    {
        SAL_INFO( "sdremote.bluetooth", "Bluetooth adapter not powered, returning" );
        g_object_unref( G_OBJECT( aAdapter ));
        return;
    }

    GValue aTimeout = G_VALUE_INIT;
    g_value_init( &aTimeout, G_TYPE_UINT );
    g_value_set_uint( &aTimeout, 0 );
    aResult = dbus_g_proxy_call( aAdapter, "SetProperty", &aError,
                                G_TYPE_STRING, "DiscoverableTimeout",
                                 G_TYPE_VALUE, &aTimeout, G_TYPE_INVALID, G_TYPE_INVALID);
    if ( !aResult || aError )
    {
        SAL_WARN( "sdremote.bluetooth", "SetProperty(DiscoverableTimeout) failed" );
        if ( aError )
        {
            g_error_free( aError );
            SAL_WARN( "sdremote.bluetooth", "with error " << aError->message );
        }
        return;
    }

    GValue aDiscoverableGValue = G_VALUE_INIT;
    g_value_init( &aDiscoverableGValue, G_TYPE_BOOLEAN );
    g_value_set_boolean( &aDiscoverableGValue, aDiscoverable );
    aResult = dbus_g_proxy_call( aAdapter, "SetProperty", &aError,
                                G_TYPE_STRING, "Discoverable",
                                 G_TYPE_VALUE, &aDiscoverableGValue, G_TYPE_INVALID, G_TYPE_INVALID);
    if ( !aResult || aError )
    {
        SAL_WARN( "sdremote.bluetooth", "SetProperty(Discoverable) failed" );
        if ( aError )
        {
            g_error_free( aError );
            SAL_WARN( "sdremote.bluetooth", "with error " << aError->message );
        }
        return;
    }

    g_object_unref( G_OBJECT( aAdapter ));
    dbus_g_connection_unref( aConnection );
#else // defined(LINUX) && defined(ENABLE_DBUS)
    (void) aDiscoverable; // avoid warnings
#endif
}

void BluetoothServer::addCommunicator( Communicator* pCommunicator )
{
    mpCommunicators->push_back( pCommunicator );
}

void SAL_CALL BluetoothServer::run()
{
    SAL_INFO( "sdremote.bluetooth", "BluetoothServer::run called" );
#if (defined(LINUX) && !defined(__FreeBSD_kernel__)) && defined(ENABLE_DBUS)
    g_type_init();

    GError *aError = NULL;

    DBusGConnection *aConnection = NULL;
    aConnection = dbus_g_bus_get( DBUS_BUS_SYSTEM, &aError );

    if ( aError != NULL ) {
        SAL_WARN( "sdremote.bluetooth", "failed to get dbus system bus" );
        g_error_free (aError);
        return;
    }

    DBusGProxy* aAdapter = bluezGetDefaultAdapter( aConnection, "org.bluez.Service" );
    if ( aAdapter == NULL )
    {
        SAL_WARN( "sdremote.bluetooth", "failed to retrieve default adapter" );
        dbus_g_connection_unref( aConnection );
        return;
    }

    // Add the record -- the handle can be used to release it again, but we
    // don't bother as the record is automatically released when LO exits.
    guint aHandle;
    gboolean aResult = dbus_g_proxy_call( aAdapter, "AddRecord", &aError,
                                G_TYPE_STRING, bluetooth_service_record,
                                G_TYPE_INVALID,
                                G_TYPE_UINT, &aHandle,
                                G_TYPE_INVALID);

    g_object_unref( G_OBJECT( aAdapter ));
    dbus_g_connection_unref( aConnection );
    if ( !aResult)
    {
        SAL_WARN( "sdremote.bluetooth", "SDP registration failed" );
        return;
    }

    // ---------------- Socket code
    int aSocket;
    if ( (aSocket = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM )) < 0 )
    {
        SAL_WARN( "sdremote.bluetooth", "failed to open bluetooth socket with error " << aSocket );
        return;
    }

    sockaddr_rc aAddr;
    aAddr.rc_family = AF_BLUETOOTH;
    // BDADDR_ANY is broken, so use memset to set to 0.
    memset( &aAddr.rc_bdaddr, 0, sizeof( aAddr.rc_bdaddr ) );
    aAddr.rc_channel = 5;

    int a;
    if ( ( a = bind( aSocket, (sockaddr*) &aAddr, sizeof(aAddr) ) ) < 0 ) {
        SAL_WARN( "sdremote.bluetooth", "bind failed with error" << a );
        close( aSocket );
        return;
    }

    if ( ( a = listen( aSocket, 1 ) ) < 0 )
    {
        SAL_WARN( "sdremote.bluetooth", "listen failed with error" << a );
        close( aSocket );
        return;
    }

    sockaddr_rc aRemoteAddr;
    socklen_t  aRemoteAddrLen = sizeof(aRemoteAddr);
    while ( true )
    {
        int bSocket;
        SAL_INFO( "sdremote.bluetooth", "waiting on accept" );
        if ( (bSocket = accept(aSocket, (sockaddr*) &aRemoteAddr, &aRemoteAddrLen)) < 0 )
        {
            int err = errno;
            SAL_WARN( "sdremote.bluetooth", "accept failed with errno " << err );
            close( aSocket );
            return;
        } else {
            SAL_INFO( "sdremote.bluetooth", "connection accepted" );
            Communicator* pCommunicator = new Communicator( new BufferedStreamSocket( bSocket ) );
            mpCommunicators->push_back( pCommunicator );
            pCommunicator->launch();
        }
    }

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
    aRecord.lpszServiceInstanceName = "LibreOffice Impress Remote Control";
    aRecord.lpszComment = "Remote control of presentations over bluetooth.";
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
