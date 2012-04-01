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

void TeleConference::methodCallHandler(
    GDBusConnection*       /*pConnection*/,
    const gchar*           pSender,
    const gchar*           /*pObjectPath*/,
    const gchar*           pInterfaceName,
    const gchar*           pMethodName,
    GVariant*              pParameters,
    GDBusMethodInvocation* pInvocation,
    void*                  pUserData)
{
    INFO_LOGGER_F( "TeleConference::methodCallHandler");

    TeleConference* pConference = reinterpret_cast<TeleConference*>(pUserData);
    SAL_WARN_IF( !pConference, "tubes", "TeleConference::methodCallHandler: no conference");
    if (!pConference)
        return;

    TeleManager* pManager = pConference->getManager();
    SAL_WARN_IF( !pManager, "tubes", "TeleConference::methodCallHandler: no manager");
    if (!pManager)
        return;

    if (tp_strdiff (pMethodName, LIBO_TUBES_DBUS_MSG_METHOD))
    {
        g_dbus_method_invocation_return_error ( pInvocation,
                G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
                "Unknown method '%s' on interface %s",
                pMethodName, pInterfaceName );
        return;
    }

    if (!g_variant_is_of_type ( pParameters, G_VARIANT_TYPE ("(ay)")))
    {
        g_dbus_method_invocation_return_error ( pInvocation,
                G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
                "'%s' takes an array of bytes, not %s",
                pMethodName,
                g_variant_get_type_string (pParameters));
        return;
    }

    GVariant *ay;
    g_variant_get( pParameters, "(@ay)", &ay);
    const char* pPacketData = reinterpret_cast<const char*>( g_variant_get_data( ay));
    gsize nPacketSize = g_variant_get_size( ay);

    SAL_WARN_IF( !pPacketData, "tubes", "TeleConference::methodCallHandler: couldn't get packet data");
    if (!pPacketData)
        return;

    SAL_INFO( "tubes", "TeleConference::methodCallHandler: received packet from sender "
            << (pSender ? pSender : "(null)") << " with size " << nPacketSize);
    pConference->queue( pSender, pPacketData, nPacketSize);
    g_dbus_method_invocation_return_value( pInvocation, 0 );

    g_variant_unref( ay);
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


void TeleConference::TubeOfferedHandler(
        TpChannel*      pBaseChannel,
        const gchar*    pAddress,
        const GError*   pError,
        gpointer        pUserData,
        GObject*        /*weak_object*/
        )
{
    INFO_LOGGER_F( "TeleConference::TubeOfferedHandler");

    TeleConference* pConference = reinterpret_cast<TeleConference*>(pUserData);
    SAL_WARN_IF( !pConference, "tubes", "TeleConference_TubeOfferedHandler: no conference");
    if (!pConference)
        return;

    pConference->setTubeOfferedHandlerInvoked( true);

    // "can't find contact ... presence" means contact is not a contact.
    /* FIXME: detect and handle */
    SAL_WARN_IF( pError, "tubes", "TeleConference::TubeOfferedHandler: entered with error: " << pError->message);
    if (pError)
        return;

    SAL_WARN_IF( !pAddress, "tubes", "TeleConference::TubeOfferedHandler: no address");
    if (!pAddress)
        return;

    TpDBusTubeChannel* pChannel = TP_DBUS_TUBE_CHANNEL( pBaseChannel);
    SAL_WARN_IF( pChannel != pConference->getChannel(), "tubes", "TeleConference::TubeOfferedHandler: not my channel");
    if ((pChannel) != pConference->getChannel())
        return;

    pConference->mpAddress = g_strdup( pAddress );
    pConference->tryToOpen();
}


bool TeleConference::tryToOpen()
{
    if (mpTube)
        return true;

    if (!isTubeOpen())
        return false;

    if (!mpAddress)
        return false;

    return setTube( mpAddress);
}

void TeleConference::TubeChannelStateChangedHandler(
        TpChannel*  pBaseChannel,
        guint       nState,
        gpointer    pUserData,
        GObject*    /*weak_object*/
        )
{
    INFO_LOGGER_F( "TeleConference::TubeChannelStateChangedHandler");

    SAL_INFO( "tubes", "TeleConference::TubeChannelStateChangedHandler: state: " << static_cast<sal_uInt32>(nState));

    TeleConference* pConference = reinterpret_cast<TeleConference*>(pUserData);
    SAL_WARN_IF( !pConference, "tubes", "TeleConference_DBusMessageHandler: no conference");
    if (!pConference)
        return;

    TpDBusTubeChannel* pChannel = TP_DBUS_TUBE_CHANNEL( pBaseChannel);
    SAL_WARN_IF( pChannel != pConference->getChannel(), "tubes",
            "TeleConference::TubeChannelStateChangedHandler: not my channel");
    if (pChannel != pConference->getChannel())
        return;

    pConference->setTubeChannelState( static_cast<TpTubeChannelState>(nState));
    pConference->tryToOpen();
}


TeleConference::TeleConference( TeleManager* pManager, TpAccount* pAccount, TpDBusTubeChannel* pChannel, const rtl::OString& rSessionId )
    :
        maSessionId( rSessionId ),
        mpManager( pManager),
        mpAccount( NULL),
        mpChannel( NULL),
        mpAddress( NULL),
        mpTube( NULL),
        meTubeChannelState( TP_TUBE_CHANNEL_STATE_NOT_OFFERED),
        mbTubeOfferedHandlerInvoked( false)
{
    setChannel( pAccount, pChannel );
}


TeleConference::~TeleConference()
{
    // Do nothing here, we're destructed from finalize()
}


void TeleConference::setChannel( TpAccount *pAccount, TpDBusTubeChannel* pChannel )
{
    OSL_ENSURE( !mpChannel, "TeleConference::setChannel: already have channel");
    if (mpChannel)
        g_object_unref( mpChannel);
    if (mpAccount)
        g_object_unref( mpAccount);

    mpChannel = pChannel;
    if (mpChannel) {
        g_object_ref( mpChannel);

        /* TODO: remember the TpProxySignalConnection and disconnect in finalize */
        GError* pError = NULL;
        TpProxySignalConnection* pProxySignalConnection =
            tp_cli_channel_interface_tube_connect_to_tube_channel_state_changed(
                    TP_CHANNEL( mpChannel),
                    &TeleConference::TubeChannelStateChangedHandler,
                    this,
                    NULL,
                    NULL,
                    &pError);

        if (!pProxySignalConnection || pError)
        {
            SAL_WARN_IF( pError, "tubes",
                "TeleConference::setChannel: channel state changed error: " << pError->message);
            g_error_free( pError);
        }
    }

    mpAccount = pAccount;
    if (mpAccount)
        g_object_ref( mpAccount);
}


bool TeleConference::spinUntilTubeEstablished()
{
    mpManager->iterateLoop( this, &TeleConference::isTubeOfferedHandlerInvoked);
    mpManager->iterateLoop( this, &TeleConference::isTubeChannelStateChangedToOpen);

    bool bOpen = isTubeOpen();
    SAL_INFO( "tubes", "TeleConference::spinUntilTubeEstablished: tube open: " << bOpen);
    return bOpen;
}


bool TeleConference::acceptTube()
{
    INFO_LOGGER( "TeleConference::acceptTube");

    SAL_WARN_IF( !mpChannel, "tubes", "TeleConference::acceptTube: no channel setup");
    SAL_WARN_IF( mpTube, "tubes", "TeleConference::acceptTube: already tubed");
    if (!mpChannel || mpTube)
        return false;

    tp_cli_channel_type_dbus_tube_call_accept( TP_CHANNEL( mpChannel), -1,
            TP_SOCKET_ACCESS_CONTROL_CREDENTIALS,
            &TeleConference::TubeOfferedHandler,
            this, NULL, NULL);
    return spinUntilTubeEstablished();
}


bool TeleConference::offerTube()
{
    INFO_LOGGER( "TeleConference::offerTube");

    OSL_ENSURE( mpChannel, "TeleConference::offerTube: no channel");
    if (!mpChannel)
        return false;

    // We must pass a hash table, it could be empty though.
    /* TODO: anything meaningful to go in here? */
    GHashTable* pParams = tp_asv_new(
            "LibreOffice", G_TYPE_STRING, "Collaboration",
            NULL);

    tp_cli_channel_type_dbus_tube_call_offer(
            TP_CHANNEL( mpChannel),                 // proxy
            -1,                                     // timeout_ms
            pParams,                                // in_parameters
            TP_SOCKET_ACCESS_CONTROL_CREDENTIALS,   // in_access_control
            &TeleConference::TubeOfferedHandler,    // callback
            this,                                   // user_data
            NULL,                                   // destroy
            NULL);                                  // weak_object

    g_hash_table_unref( pParams);
    return spinUntilTubeEstablished();
}


bool TeleConference::setTube(  const char* pAddress )
{
    INFO_LOGGER( "TeleConference::setTube");

    OSL_ENSURE( !mpTube, "TeleConference::setTube: already tubed");

    GError *aError = NULL;

    mpTube = g_dbus_connection_new_for_address_sync( pAddress,
            G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
            NULL, NULL, &aError);
    if (!mpTube)
    {
        SAL_WARN( "tubes", "TeleConference::setTube: no dbus connection: " << aError->message);
        g_clear_error( &aError);
        return false;
    }

    GDBusNodeInfo *introspection_data;
    guint registration_id;
    static const GDBusInterfaceVTable interface_vtable =
    {
        &TeleConference::methodCallHandler,
        NULL,
        NULL,
        { NULL },
    };
    static const gchar introspection_xml[] =
        "<node>"
        "  <interface name='" LIBO_TUBES_DBUS_INTERFACE "'>"
        "    <method name='" LIBO_TUBES_DBUS_MSG_METHOD "'>"
        "      <arg type='ay' name='packet' direction='in'/>"
        "    </method>"
        "  </interface>"
        "</node>";

    introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
    g_assert (introspection_data != NULL);

    registration_id = g_dbus_connection_register_object( mpTube,
            LIBO_TUBES_DBUS_PATH, introspection_data->interfaces[0],
            &interface_vtable, this, NULL, NULL);
    g_assert (registration_id > 0);

    g_dbus_node_info_unref (introspection_data);

    return true;
}


void TeleConference::close()
{
    INFO_LOGGER( "TeleConference::close");

    if (mpChannel)
        tp_cli_channel_call_close( TP_CHANNEL( mpChannel), 5000, TeleConference_ChannelCloseHandler, this, NULL, NULL);
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

    if (mpAccount)
    {
        g_object_unref( mpAccount);
        mpAccount = NULL;
    }

    if (mpTube)
    {
        g_dbus_connection_unregister_object( mpTube, maObjectRegistrationId);
        g_dbus_connection_close_sync( mpTube, NULL, NULL );
        g_object_unref( mpTube );
        mpTube = NULL;
    }

    if (mpAddress)
    {
        g_free( mpAddress);
        mpAddress = NULL;
    }

    TeleConferencePtr pThis( shared_from_this());
    mpManager->unregisterConference( pThis);

    //! *this gets destructed here!
}


bool TeleConference::sendPacket( TelePacket& rPacket )
{
    INFO_LOGGER( "TeleConference::sendPacket");

    OSL_ENSURE( mpManager, "tubes: TeleConference::sendPacket: no TeleManager");
    SAL_WARN_IF( !mpTube, "tubes", "TeleConference::sendPacket: no tube");
    if (!(mpManager && mpTube))
        return false;

    /* FIXME: in GLib 2.32 we can use g_variant_new_fixed_array(). It does
     * essentially this.
     */
    void* pData = g_memdup( rPacket.getData(), rPacket.getSize() );
    GVariant *pParameters = g_variant_new_from_data( G_VARIANT_TYPE("(ay)"),
            pData, rPacket.getSize(),
            FALSE,
            g_free, pData);

    g_dbus_connection_call( mpTube,
            NULL, /* bus name; in multi-user case we'd address this to the master. */
            LIBO_TUBES_DBUS_PATH,
            LIBO_TUBES_DBUS_INTERFACE,
            LIBO_TUBES_DBUS_MSG_METHOD,
            pParameters, /* consumes the floating reference */
            NULL,
            G_DBUS_CALL_FLAGS_NONE,
            -1, NULL, NULL, NULL);

    /* FIXME: need to impose an ordering on packets. */
    queue( rPacket );

    return true;
}


void TeleConference::queue( TelePacket &rPacket )
{
    INFO_LOGGER( "TeleConference::queue");

    maPacketQueue.push( rPacket);

    getManager()->callbackOnRecieved( this, rPacket);
}


void TeleConference::queue( const char* pDBusSender, const char* pPacketData, int nPacketSize )
{
    TelePacket aPacket( pDBusSender, pPacketData, nPacketSize );
    queue( aPacket );
}


class SendFileRequest {
public:
    SendFileRequest( TeleConference *pSelf,
        TeleConference::FileSentCallback pCallback, void* pUserData)
        : mpSelf(pSelf)
        , mpCallback(pCallback)
        , mpUserData(pUserData) {};

    /* FIXME: make a shared pointer? */
    TeleConference*                     mpSelf;
    TeleConference::FileSentCallback    mpCallback;
    void*                               mpUserData;
};

static void TeleConference_TransferDone( EmpathyFTHandler *handler, TpFileTransferChannel *, gpointer user_data)
{
    SendFileRequest *request = reinterpret_cast<SendFileRequest *>(user_data);

    request->mpCallback(true, request->mpUserData);
    delete request;
    g_object_unref (handler);
}

static void TeleConference_TransferError( EmpathyFTHandler *handler, const GError *error, gpointer user_data)
{
    SendFileRequest *request = reinterpret_cast<SendFileRequest *>(user_data);

    SAL_INFO( "tubes", "TeleConference_TransferError: " << error->message);

    request->mpCallback(false, request->mpUserData);
    delete request;
    g_object_unref (handler);
}

void TeleConference::FTReady( EmpathyFTHandler *handler, GError *error, gpointer user_data)
{
    SendFileRequest *request = reinterpret_cast<SendFileRequest *>(user_data);

    if ( error != 0 )
    {
        request->mpCallback(error == 0, request->mpUserData);
        delete request;
        g_object_unref (handler);
    }
    else
    {
        g_signal_connect(handler, "transfer-done",
            G_CALLBACK (TeleConference_TransferDone), request);
        g_signal_connect(handler, "transfer-error",
            G_CALLBACK (TeleConference_TransferError), request);
        empathy_ft_handler_set_service_name(handler, request->mpSelf->mpManager->getFullServiceName().getStr());
        empathy_ft_handler_start_transfer(handler);
    }
}


void TeleConference::sendFile( rtl::OUString &localUri, FileSentCallback pCallback, void* pUserData)
{
    INFO_LOGGER( "TeleConference::sendFile");

    SAL_WARN_IF( ( !mpAccount || !mpChannel), "tubes",
        "can't send a file before the tube is set up");
    if ( !mpAccount || !mpChannel)
        return;

    GFile *pSource = g_file_new_for_uri(
        OUStringToOString( localUri, RTL_TEXTENCODING_UTF8).getStr() );
    SendFileRequest *pReq = new SendFileRequest( this, pCallback, pUserData);

    empathy_ft_handler_new_outgoing( mpAccount,
        tp_channel_get_target_contact( TP_CHANNEL( mpChannel)),
        pSource,
        0,
        &TeleConference::FTReady, pReq);
}


bool TeleConference::popPacket( TelePacket& rPacket )
{
    INFO_LOGGER( "TeleConference::popPacket");

    if (maPacketQueue.empty())
        return false;
    rPacket = maPacketQueue.front();
    maPacketQueue.pop();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
