/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Red Hat, Inc., Eike Rathke <erack@redhat.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <tubes/conference.hxx>
#include <tubes/manager.hxx>


#if defined SAL_LOG_INFO
namespace
{
struct InfoLogger
{
    const void* mpThat;
    const char* mpMethod;
    explicit InfoLogger( const void* pThat, const char* pMethod )
        :
            mpThat( pThat),
            mpMethod( pMethod)
    {
        SAL_INFO( "tubes.method", mpThat << " entering " << mpMethod);
    }
    ~InfoLogger()
    {
        SAL_INFO( "tubes.method", mpThat << " leaving  " << mpMethod);
    }
};
}
#define INFO_LOGGER_F(s)    InfoLogger aLogger(0,(s))
#define INFO_LOGGER(s)      InfoLogger aLogger(this,(s))
#else
#define INFO_LOGGER_F(s)
#define INFO_LOGGER(s)
#endif // SAL_LOG_INFO


static DBusHandlerResult TeleConference_DBusMessageHandler(
        DBusConnection* pConnection,
        DBusMessage*    pMessage,
        void*           pUserData
        )
{
    INFO_LOGGER_F( "TeleConference_DBusMessageHandler");

    SAL_WARN_IF( !pConnection || !pMessage, "tubes", "TeleConference_DBusMessageHandler: unhandled");
    if (!pConnection || !pMessage)
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    TeleConference* pConference = reinterpret_cast<TeleConference*>(pUserData);
    SAL_WARN_IF( !pConference, "tubes", "TeleConference_DBusMessageHandler: no conference");
    if (!pConference)
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    TeleManager* pManager = pConference->getManager();
    SAL_WARN_IF( !pManager, "tubes", "TeleConference_DBusMessageHandler: no manager");
    if (!pManager)
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    if (dbus_message_is_method_call( pMessage, pManager->getFullServiceName().getStr(), LIBO_TUBES_DBUS_MSG_METHOD))
    {
        const char* pSender = dbus_message_get_sender( pMessage);

        DBusError aDBusError;
        dbus_error_init( &aDBusError);
        const char* pPacketData = 0;
        int nPacketSize = 0;
        if (dbus_message_get_args( pMessage, &aDBusError,
                    DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &pPacketData, &nPacketSize,
                    DBUS_TYPE_INVALID))
        {
            SAL_INFO( "tubes", "TeleConference_DBusMessageHandler: received packet from sender "
                    << pSender << " with size " << nPacketSize);
            pConference->queue( pSender, pPacketData, nPacketSize);
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        else
        {
            SAL_INFO( "tubes", "TeleConference_DBusMessageHandler: unhandled message from sender "
                    << pSender << " " << aDBusError.message);
            dbus_error_free( &aDBusError);
        }
    }
    else
    {
        SAL_INFO( "tubes", "TeleConference_DBusMessageHandler: unhandled method");
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


static void TeleConference_ChannelCloseHandler(
        TpChannel*      /*proxy*/,
        const GError*   pError,
        gpointer        pUserData,
        GObject*        /*weak_object*/
        )
{
    INFO_LOGGER_F( "TeleConference_ChannelCloseHandler");

    TeleConference* pConference = reinterpret_cast<TeleConference*>(pUserData);
    SAL_WARN_IF( !pConference, "tubes", "TeleConference_ChannelCloseHandler: no conference");
    if (!pConference)
        return;

    if (pError)
    {
        SAL_WARN( "tubes", "TeleConference_ChannelCloseHandler: entered with error: " << pError->message);
        pConference->finalize();
        return;
    }

    pConference->finalize();
}


static void TeleConference_TubeOfferedHandler(
        TpChannel*      pChannel,
        const gchar*    pOutAddress,
        const GError*   pError,
        gpointer        pUserData,
        GObject*        /*weak_object*/
        )
{
    INFO_LOGGER_F( "TeleConference_TubeOfferedHandler");

    // "can't find contact ... presence" means contact is not a contact.
    /* FIXME: detect and handle */
    SAL_WARN_IF( pError, "tubes", "TeleConference_TubeOfferedHandler: entered with error: " << pError->message);
    if (pError)
        return;

    SAL_WARN_IF( !pOutAddress, "tubes", "TeleConference_TubeOfferedHandler: no out address");
    if (!pOutAddress)
        return;

    TeleConference* pConference = reinterpret_cast<TeleConference*>(pUserData);
    SAL_WARN_IF( !pConference, "tubes", "TeleConference_TubeOfferedHandler: no conference");
    if (!pConference)
        return;

    SAL_WARN_IF( pChannel != pConference->getChannel(), "tubes", "TeleConference_TubeOfferedHandler: not my channel");
    if (pChannel != pConference->getChannel())
        return;

    TeleManager* pManager = pConference->getManager();
    SAL_WARN_IF( !pManager, "tubes", "TeleConference_TubeOfferedHandler: no manager");
    if (!pManager)
        return;

    pConference->setTubeOfferedHandlerInvoked( true);

    DBusError aDBusError;
    dbus_error_init( &aDBusError);
    DBusConnection* pTube = dbus_connection_open_private( pOutAddress, &aDBusError);
    if (!pTube)
    {
        SAL_WARN( "tubes", "TeleConference_TubeOfferedHandler: no dbus connection: " << aDBusError.message);
        dbus_error_free( &aDBusError);
        return;
    }

    pConference->setTube( pTube);
}


TeleConference::TeleConference( TeleManager* pManager, TpChannel* pChannel, const rtl::OString& rSessionId )
    :
        maSessionId( rSessionId ),
        mpManager( pManager),
        mpChannel( pChannel),
        mpTube( NULL),
        mbTubeOfferedHandlerInvoked( false)
{
    if (mpChannel)
        g_object_ref( mpChannel);
}


TeleConference::~TeleConference()
{
    // Do nothing here, we're destructed from finalize()
}


void TeleConference::setTarget( const rtl::OString& rTarget )
{
    maTarget = rTarget;
}


void TeleConference::setChannel( TpChannel* pChannel )
{
    OSL_ENSURE( !mpChannel, "TeleConference::setChannel: already have channel");
    if (mpChannel)
        g_object_unref( mpChannel);
    mpChannel = pChannel;
    if (mpChannel)
        g_object_ref( mpChannel);
}


bool TeleConference::acceptTube( const char* pAddress )
{
    INFO_LOGGER( "TeleConference::acceptTube");

    SAL_WARN_IF( !pAddress, "tubes", "TeleConference::acceptTube: no address");
    if (!pAddress)
        return false;
    SAL_INFO( "tubes", "TeleConference::acceptTube: address: " << pAddress);

    SAL_WARN_IF( !mpChannel, "tubes", "TeleConference::acceptTube: no channel setup");
    SAL_WARN_IF( mpTube, "tubes", "TeleConference::acceptTube: already tubed");
    if (!mpChannel || mpTube)
        return false;

    DBusError aDBusError;
    dbus_error_init( &aDBusError);
    mpTube = dbus_connection_open_private( pAddress, &aDBusError);
    if (!mpTube)
    {
        SAL_WARN( "tubes", "TeleConference::acceptTube: no dbus connection: " << aDBusError.message);
        dbus_error_free( &aDBusError);
        return false;
    }

    dbus_connection_setup_with_g_main( mpTube, mpManager->getMainContext());
    dbus_connection_add_filter( mpTube, TeleConference_DBusMessageHandler, this, NULL);

    return true;
}


bool TeleConference::offerTube()
{
    INFO_LOGGER( "TeleConference::offerTube");

    OSL_ENSURE( mpChannel, "TeleConference::offerTube: no channel");
    if (!mpChannel)
        return false;

    setTubeOfferedHandlerInvoked( false);

    // There must be a hash table with some content.
    /* TODO: anything meaningful to go in here? */
    GHashTable* pParams = tp_asv_new(
            "LibreOffice", G_TYPE_STRING, "Collaboration",
            NULL);

    tp_cli_channel_type_dbus_tube_call_offer(
            mpChannel,                          // proxy
            -1,                                 // timeout_ms
            pParams,                            // in_parameters
            TP_SOCKET_ACCESS_CONTROL_LOCALHOST, // in_access_control
            TeleConference_TubeOfferedHandler,  // callback
            this,                               // user_data
            NULL,                               // destroy
            NULL);                              // weak_object

    mpManager->iterateLoop( this, &TeleConference::isTubeOfferedHandlerInvoked);

    g_hash_table_unref( pParams);

    return true;
}


bool TeleConference::setTube( DBusConnection* pTube )
{
    INFO_LOGGER( "TeleConference::setTube");

    OSL_ENSURE( !mpTube, "TeleConference::setTube: already tubed");
    mpTube = pTube;

    dbus_connection_setup_with_g_main( mpTube, mpManager->getMainContext());
    dbus_connection_add_filter( mpTube, TeleConference_DBusMessageHandler, this, NULL);

    /* TODO: anything else? */

    return true;
}


void TeleConference::close()
{
    INFO_LOGGER( "TeleConference::close");

    if (mpChannel)
        tp_cli_channel_call_close( mpChannel, 5000, TeleConference_ChannelCloseHandler, this, NULL, NULL);
    else
        finalize();
}


void TeleConference::finalize()
{
    INFO_LOGGER( "TeleConference::finalize");

    if (mpChannel)
    {
        g_object_unref( mpChannel);
        mpChannel = NULL;
    }

    if (mpTube)
    {
        dbus_connection_remove_filter( mpTube, TeleConference_DBusMessageHandler, this);
        dbus_connection_close( mpTube);
        dbus_connection_unref( mpTube);
        mpTube = NULL;
    }

    TeleConferencePtr pThis( shared_from_this());
    mpManager->unregisterConference( pThis);

    //! *this gets destructed here!
}


bool TeleConference::sendPacket( TelePacket& rPacket ) const
{
    INFO_LOGGER( "TeleConference::sendPacket");

    OSL_ENSURE( mpManager, "tubes: TeleConference::sendPacket: no TeleManager");
    SAL_WARN_IF( !mpTube, "tubes", "TeleConference::sendPacket: no tube");
    if (!(mpManager && mpTube))
        return false;

    DBusMessage* pMessage = dbus_message_new_method_call(
            mpManager->getFullServiceName().getStr(),
            mpManager->getFullObjectPath().getStr(),
            mpManager->getFullServiceName().getStr(),
            LIBO_TUBES_DBUS_MSG_METHOD);
    SAL_WARN_IF( !pMessage, "tubes", "TeleConference::sendPacket: no DBusMessage");
    if (!pMessage)
        return false;

    const char* pPacketData = rPacket.getData();
    dbus_message_append_args( pMessage,
            DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &pPacketData, rPacket.getSize(),
            DBUS_TYPE_INVALID);

    SAL_INFO( "tubes", "TeleConference::sendPacket: from " <<
            dbus_message_get_sender( pMessage) << " to " << dbus_message_get_destination( pMessage));

    bool bSent = dbus_connection_send( mpTube, pMessage, NULL);
    SAL_WARN_IF( !bSent, "tubes", "TeleConference::sendPacket: not sent");
    dbus_message_unref( pMessage);
    return bSent;
}


void TeleConference::queue( const char* pDBusSender, const char* pPacketData, int nPacketSize )
{
    INFO_LOGGER( "TeleConference::queue");

    maPacketQueue.push( TelePacket( pDBusSender, pPacketData, nPacketSize));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
