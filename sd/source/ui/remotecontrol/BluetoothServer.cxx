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

#ifdef LINUX
#include <gio/gio.h>
#include <sys/unistd.h>
#endif
#include <stdint.h>
#include <sys/socket.h>
#include <tools/debug.hxx>
#include <tools/stream.hxx>

#ifdef LINUX
#include "bluetooth/bluetooth.h"
#include "bluetooth/rfcomm.h"
#endif

using namespace sd;

BluetoothServer::BluetoothServer():
    Thread( "BluetoothServer" )
{

}

BluetoothServer::~BluetoothServer()
{
}

void BluetoothServer::execute()
{
#ifdef LINUX
//     g_type_init();
//     GError* aError = NULL;
//     GDBusConnection* aConnection = g_bus_get_sync( G_BUS_TYPE_SYSTEM, NULL, &aError );
//     if ( aError )
//     {
//         fprintf( stderr, aError->message );
//         g_error_free( aError );
//     }
//     GDBusObjectManager* aManager = g_dbus_object_manager_client_new_sync( aConnection,
//                     G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, "org.bluez.Manager", "/org/bluez",
//                     NULL, NULL, NULL, NULL, &aError );
//     if ( aError )
//     {
//         fprintf( stderr, aError->message );
//         g_error_free( aError );
//     }
//
//     GVariant *aRet = g_dbus_connection_call_sync( aConnection,
//                                 "org.bluez", "/", "org.bluez.Database",
//                                 "AddServiceRecordFromXml",
//                                  g_variant_new ("s",
//                                             ""),,
//                                 G_VARIANT_TYPE_UINT32,
//                                 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &aError);
//     g_variant_unref( aRet );
//     fprintf( stderr, "Manager gotten\n" );
//
//     // Name for default adapter
//     GVariant *aAdapter = g_dbus_connection_call_sync( aConnection,
//                                 "org.bluez", "/", "org.bluez.Manager",
//                                 "DefaultAdapter", NULL,
//                                 G_VARIANT_TYPE_TUPLE,
//                                 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &aError);
//     GVariant *aAdapterName = g_variant_get_child_value( aAdapter, 0 );
//     if ( aError )
//     {
//         fprintf( stderr, aError->message );
//         g_error_free( aError );
//     }
//     fprintf( stderr, (const char*) g_variant_get_string( aAdapterName, NULL ) );




//     g_type_init();
//     GError* aError = NULL;
//     GDBusConnection* aConnection = g_bus_get_sync( G_BUS_TYPE_SYSTEM, NULL, &aError );
//     fprintf( stderr, "Connection gotten\n" );
//     if ( aError )
//     {
//         fprintf( stderr, aError->message );
//         g_error_free( aError );
//     }
// //     GDBusObjectManager* aManager = g_dbus_object_manager_client_new_sync( aConnection,
// //                     G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, "org.bluez.Manager", "/org/bluez",
// //                     NULL, NULL, NULL, NULL, &aError );
// //     if ( aError )
// //     {
// //         fprintf( stderr, aError->message );
// //         g_error_free( aError );
// //     }
//     fprintf( stderr, "Manager gotten\n" );
//
//     // Name for default adapter
//     GVariant *aAdapter = g_dbus_connection_call_sync( aConnection,
//                                 "org.bluez", "/", "org.bluez.Manager",
//                                 "DefaultAdapter", NULL,
//                                 G_VARIANT_TYPE_TUPLE,
//                                 G_DBUS_CALL_FLAGS_NONE, -1, NULL, &aError);
//     GVariant *aAdapterName = g_variant_get_child_value( aAdapter, 0 );
//     if ( aError )
//     {
//         fprintf( stderr, aError->message );
//         g_error_free( aError );
//     }
//     fprintf( stderr, (const char*) g_variant_get_string( aAdapterName, NULL ) );


    // ---------------- DEVICE ADDRESS
    int aSocket;
    if ( (aSocket = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM )) < 0 )
    {
        // Error
        return;
    }

    sockaddr_rc aAddr;
    aAddr.rc_family = AF_BLUETOOTH;
//      aAddr.rc_bdaddr = *BDADDR_ANY;
    aAddr.rc_channel = 1;

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
    if ( accept(aSocket, (sockaddr*) &aRemoteAddr, &aRemoteAddrLen) < 0 )
    {
        close( aSocket );
        return;
    } else {
        fprintf( stderr, "Accepted Bluetooth\n" );
    }

#endif

#ifdef WIN32

#endif

#ifdef MACOSX

#endif
}


BluetoothServer *sd::BluetoothServer::spServer = NULL;

void BluetoothServer::setup()
{
    if (spServer)
        return;

    spServer = new BluetoothServer();
    spServer->launch();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
