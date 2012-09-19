/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "BluetoothServer.hxx"
#include <stdio.h>

#if (defined(LINUX) && !defined(__FreeBSD_kernel__)) && defined(ENABLE_DBUS)
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#define DBUS_TYPE_G_STRING_ANY_HASHTABLE (dbus_g_type_get_map( "GHashTable", G_TYPE_STRING, G_TYPE_VALUE ))

#ifndef G_VALUE_INIT
  #define G_VALUE_INIT {0,{{0}}} // G_VALUE_INIT only present in glib >= 2.30
#endif
#endif

#ifdef WIN32
  #undef MSC // Unset a legacy define, as otherwise ws2bth.h breaks
  #include <winsock2.h>
  #include <ws2bth.h>
#endif

// FIXME: move this into an external file and look at sharing definitions
// across OS's (i.e. UUID and port ).
// Also look at determining which ports are available.
// Alternatively use the binary sdp record
#define BLUETOOTH_SERVICE_RECORD "<?xml version='1.0' encoding= 'UTF-8' ?><record><attribute id='0x0001'><sequence><uuid value='0x1101' /></sequence></attribute><attribute id='0x0004'><sequence><sequence><uuid value='0x0100' /></sequence><sequence><uuid value='0x0003' /><uint8 value='0x05' /></sequence></sequence></attribute><attribute id='0x0005'><sequence><uuid value='0x1002' /></sequence></attribute><attribute id='0x0006'><sequence><uint16 value='0x656e' /><uint16 value='0x006a' /><uint16 value='0x0100' /></sequence></attribute><attribute id='0x0009'><sequence><sequence><uuid value='0x1101' /><uint16 value='0x0100' /></sequence></sequence></attribute><attribute id='0x0100'><text value='Serial Port' /></attribute><attribute id='0x0101'><text value='COM Port' /></attribute></record>"

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
        dbus_g_connection_unref( aConnection );
        return NULL;
    }

    gboolean aResult;
    // The following should be a DBusGObjectPath, however the necessary
    // typedef is missing in older version of dbus-glib.
    char *aAdapterPath = NULL;
    aResult = dbus_g_proxy_call( aManager, "DefaultAdapter", &aError,
                                 G_TYPE_INVALID,
                                 DBUS_TYPE_G_OBJECT_PATH, &aAdapterPath,
                                 G_TYPE_INVALID);

    g_object_unref( G_OBJECT( aManager ));
    if ( !aResult || aError )
    {
        if ( aError )
            g_error_free( aError );
        return NULL;
    }

    DBusGProxy *aAdapter = NULL;
    aAdapter = dbus_g_proxy_new_for_name( aConnection, "org.bluez",
                                          aAdapterPath, aInterfaceType );
    g_free( aAdapterPath );

    return aAdapter;
}
#endif // defined(LINUX) && defined(ENABLE_DBUS)

BluetoothServer::BluetoothServer( std::vector<Communicator*>* pCommunicators ):
    Thread( "BluetoothServer" ),
    mpCommunicators( pCommunicators )
{
}

BluetoothServer::~BluetoothServer()
{
}

bool BluetoothServer::isDiscoverable()
{
#if (defined(LINUX) && !defined(__FreeBSD_kernel__)) && defined(ENABLE_DBUS)
    g_type_init();
    gboolean aResult;

    GError *aError = NULL;

    DBusGConnection *aConnection = NULL;
    aConnection = dbus_g_bus_get( DBUS_BUS_SYSTEM, &aError );

    if ( aError != NULL ) {
        g_error_free (aError);
        return false;
    }

    DBusGProxy* aAdapter = bluezGetDefaultAdapter( aConnection );
    if ( aAdapter == NULL )
    {
        dbus_g_connection_unref( aConnection );
        return false;
    }

    GHashTable* aProperties;
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
        return false;
    }

    gboolean aIsDiscoverable = g_value_get_boolean( (GValue*) g_hash_table_lookup(
                aProperties, "Discoverable" ) );

    g_free( aProperties );
    return aIsDiscoverable;
#else // defined(LINUX) && defined(ENABLE_DBUS)
    return false;
#endif
}

void BluetoothServer::setDiscoverable( bool aDiscoverable )
{
#if (defined(LINUX) && !defined(__FreeBSD_kernel__)) && defined(ENABLE_DBUS)
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
        if ( aError )
            g_error_free( aError );
        return;
    }

    gboolean aPowered = g_value_get_boolean( (GValue*) g_hash_table_lookup(
                aProperties, "Powered" ) );

    g_free( aProperties );
    if ( !aPowered )
    {
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
        if ( aError )
            g_error_free( aError );
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
        if ( aError )
            g_error_free( aError );
        return;
    }

    g_object_unref( G_OBJECT( aAdapter ));
    dbus_g_connection_unref( aConnection );
#else // defined(LINUX) && defined(ENABLE_DBUS)
    return;
#endif
}

void BluetoothServer::execute()
{
#if (defined(LINUX) && !defined(__FreeBSD_kernel__)) && defined(ENABLE_DBUS)
    g_type_init();

    GError *aError = NULL;

    DBusGConnection *aConnection = NULL;
    aConnection = dbus_g_bus_get( DBUS_BUS_SYSTEM, &aError );

    if ( aError != NULL ) {
        g_error_free (aError);
        return;
    }

    DBusGProxy* aAdapter = bluezGetDefaultAdapter( aConnection, "org.bluez.Service" );
    if ( aAdapter == NULL )
    {
        dbus_g_connection_unref( aConnection );
        return;
    }

    // Add the record -- the handle can be used to release it again, but we
    // don't bother as the record is automatically released when LO exits.
    guint aHandle;
    gboolean aResult = dbus_g_proxy_call( aAdapter, "AddRecord", &aError,
                                G_TYPE_STRING, BLUETOOTH_SERVICE_RECORD ,
                                G_TYPE_INVALID,
                                G_TYPE_UINT, &aHandle,
                                G_TYPE_INVALID);

    g_object_unref( G_OBJECT( aAdapter ));
    dbus_g_connection_unref( aConnection );
    if ( !aResult)
    {
        return;
    }

    // ---------------- Socket code
    int aSocket;
    if ( (aSocket = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM )) < 0 )
    {
        return;
    }

    sockaddr_rc aAddr;
    aAddr.rc_family = AF_BLUETOOTH;
    // BDADDR_ANY is broken, so use memset to set to 0.
    memset( &aAddr.rc_bdaddr, 0, sizeof( aAddr.rc_bdaddr ) );
    aAddr.rc_channel = 5;

    if ( bind( aSocket, (sockaddr*) &aAddr, sizeof(aAddr)) < 0 ) {
        close( aSocket );
        return;
    }

    if ( listen( aSocket, 1 ) < 0 )
    {
        close( aSocket );
        return;
    }

    sockaddr_rc aRemoteAddr;
    socklen_t  aRemoteAddrLen = sizeof(aRemoteAddr);
    while ( true )
    {
        int bSocket;
        if ( (bSocket = accept(aSocket, (sockaddr*) &aRemoteAddr, &aRemoteAddrLen)) < 0 )
        {
            close( aSocket );
            return;
        } else {
            Communicator* pCommunicator = new Communicator( new BufferedStreamSocket( bSocket) );
            mpCommunicators->push_back( pCommunicator );
            pCommunicator->launch();
        }
    }

// LINUX && ENABLE_DBUS
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
    aRecord.lpszServiceInstanceName = "LibreOffice-SDRemote"; // Optional
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
        int bSocket;
        if ( (bSocket = accept(aSocket, (sockaddr*) &aRemoteAddr, &aRemoteAddrLen)) == INVALID_SOCKET )
        {
            closesocket( aSocket );
            WSACleanup();
            return;
        } else {
            Communicator* pCommunicator = new Communicator( new BufferedStreamSocket( bSocket) );
            mpCommunicators->push_back( pCommunicator );
            pCommunicator->launch();
        }
    }

// WIN32
#else // !((defined(LINUX) && !defined(__FreeBSD_kernel__)) && defined(ENABLE_DBUS)) && !defined(WIN32)
    (void) mpCommunicators; // avoid warnings about unused member
#endif
}


BluetoothServer *sd::BluetoothServer::spServer = NULL;

void BluetoothServer::setup( std::vector<Communicator*>* pCommunicators )
{
    if (spServer)
        return;

    spServer = new BluetoothServer( pCommunicators );
    spServer->launch();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
