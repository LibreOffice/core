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

#if defined(LINUX) && defined(ENABLE_GIO) && defined(ENABLE_DBUS)
#include <glib.h>
#include <gio/gio.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#endif
#include <tools/debug.hxx>
#include <tools/stream.hxx>

#ifdef LINUX
#include "bluetooth/bluetooth.h"
#include "bluetooth/rfcomm.h"
#endif


#include "Communicator.hxx"

using namespace sd;

BluetoothServer::BluetoothServer( std::vector<Communicator*>* pCommunicators ):
    Thread( "BluetoothServer" ),
    mpCommunicators( pCommunicators )
{
}

BluetoothServer::~BluetoothServer()
{
}

struct oslSocketImpl {
    int                 m_Socket;
    int                 m_nLastError;
    void*    m_CloseCallback;
    void*               m_CallbackArg;
    oslInterlockedCount m_nRefCount;
#if defined(LINUX)
    sal_Bool            m_bIsAccepting;
    sal_Bool            m_bIsInShutdown;
#endif
};


void BluetoothServer::execute()
{
#if defined(LINUX) && defined(ENABLE_GIO) && defined(ENABLE_DBUS)
#ifdef GLIB_VERSION_2_26
    g_type_init();
    GError* aError = NULL;

    GDBusConnection* aConnection = g_bus_get_sync( G_BUS_TYPE_SYSTEM, NULL, &aError );
    if ( aError )
    {
        g_error_free( aError );
        return; // We can't get a dbus connection
    }

    GVariant *aAdapter = g_dbus_connection_call_sync( aConnection,
                                "org.bluez", "/", "org.bluez.Manager",
                                "DefaultAdapter", NULL,
                                G_VARIANT_TYPE_TUPLE,
                                G_DBUS_CALL_FLAGS_NONE, -1, NULL, &aError);
    if ( aError )
    {
        g_error_free( aError );
        g_object_unref( aConnection );
        return; // We can't get an adapter -- no bluetooth possible
    }
    GVariant *aAdapterName = g_variant_get_child_value( aAdapter, 0 );

    GVariant *aRecordHandle = g_dbus_connection_call_sync( aConnection,
                                "org.bluez", g_variant_get_string( aAdapterName, NULL ), "org.bluez.Service",
                                "AddRecord",
                                 g_variant_new("(s)",
                                            "<?xml version='1.0' encoding= 'UTF-8' ?><record><attribute id='0x0001'><sequence><uuid value='0x1101' /></sequence></attribute><attribute id='0x0004'><sequence><sequence><uuid value='0x0100' /></sequence><sequence><uuid value='0x0003' /><uint8 value='0x05' /></sequence></sequence></attribute><attribute id='0x0005'><sequence><uuid value='0x1002' /></sequence></attribute><attribute id='0x0006'><sequence><uint16 value='0x656e' /><uint16 value='0x006a' /><uint16 value='0x0100' /></sequence></attribute><attribute id='0x0009'><sequence><sequence><uuid value='0x1101' /><uint16 value='0x0100' /></sequence></sequence></attribute><attribute id='0x0100'><text value='Serial Port' /></attribute><attribute id='0x0101'><text value='COM Port' /></attribute></record>"),
                                G_VARIANT_TYPE_TUPLE,
                                G_DBUS_CALL_FLAGS_NONE, -1, NULL, &aError);

    g_variant_unref( aAdapter );
    g_object_unref( aConnection );

    if ( aError )
    {
        g_object_unref( aAdapter );
        return; // Couldn't set up the service.
    }
    g_variant_unref( aRecordHandle ); // We don't need the handle
                                      // as the service is automatically
                                      // deregistered on exit.


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

#endif // GLIB_VERSION_2_26
#endif // LINUX

#ifdef WIN32

#endif

#ifdef MACOSX

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
