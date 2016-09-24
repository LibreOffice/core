/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tubes/conference.hxx>

#include <tubes/collaboration.hxx>
#include <tubes/constants.h>
#include <tubes/file-transfer-helper.h>
#include <tubes/manager.hxx>

#include <gtk/gtk.h>
#include <telepathy-glib/telepathy-glib.h>

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

class TeleConferenceImpl
{
public:
    guint                   maObjectRegistrationId;
    GDBusConnection*        mpTube;
    bool                    mbTubeOfferedHandlerInvoked : 1;

    TeleConferenceImpl() :
        mpTube( NULL ),
        mbTubeOfferedHandlerInvoked( false )
    {}

    ~TeleConferenceImpl() {}
};

static void TeleConference_MethodCallHandler(
    GDBusConnection*       /*pConnection*/,
    const gchar*           pSender,
    const gchar*           /*pObjectPath*/,
    const gchar*           pInterfaceName,
    const gchar*           pMethodName,
    GVariant*              pParameters,
    GDBusMethodInvocation* pInvocation,
    void*                  pUserData)
{
    INFO_LOGGER_F( "TeleConference_MethodCallHandler");

    TeleConference* pConference = reinterpret_cast<TeleConference*>(pUserData);
    SAL_WARN_IF( !pConference, "tubes", "TeleConference_MethodCallHandler: no conference");
    if (!pConference)
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

    SAL_WARN_IF( !pPacketData, "tubes", "TeleConference_MethodCallHandler: couldn't get packet data");
    if (!pPacketData)
        return;

    SAL_INFO( "tubes", "TeleConference_MethodCallHandler: received packet from sender "
            << (pSender ? pSender : "(null)") << " with size " << nPacketSize);
    OString aPacket( pPacketData, nPacketSize );
    if (pConference->getCollaboration())
        pConference->getCollaboration()->PacketReceived( aPacket );
    // Master needs to send the packet back to impose ordering,
    // so the slave can execute his command.
    if (pConference->isMaster())
        pConference->sendPacket( aPacket );

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


static void TeleConference_TubeOfferedHandler(
        GObject*      pSource,
        GAsyncResult* pResult,
        gpointer      pUserData)
{
    INFO_LOGGER_F( "TeleConference_TubeOfferedHandler");

    TeleConference* pConference = reinterpret_cast<TeleConference*>(pUserData);
    SAL_WARN_IF( !pConference, "tubes", "TeleConference_TubeOfferedHandler: no conference");
    if (!pConference)
        return;

    pConference->setTubeOfferedHandlerInvoked( true);

    TpDBusTubeChannel* pChannel = TP_DBUS_TUBE_CHANNEL( pSource);
    GError* pError = NULL;
    GDBusConnection* pTube = tp_dbus_tube_channel_offer_finish(
            pChannel, pResult, &pError);

    // "can't find contact ... presence" means contact is not a contact.
    /* FIXME: detect and handle */
    SAL_WARN_IF( !pTube, "tubes", "TeleConference_TubeOfferedHandler: entered with error: " << pError->message);
    if (pError) {
        g_error_free( pError);
        return;
    }

    pConference->setTube( pTube);
}


static void TeleConference_TubeAcceptedHandler(
        GObject*      pSource,
        GAsyncResult* pResult,
        gpointer      pUserData)
{
    INFO_LOGGER_F( "TeleConference_TubeAcceptedHandler");

    TeleConference* pConference = reinterpret_cast<TeleConference*>(pUserData);
    SAL_WARN_IF( !pConference, "tubes", "TeleConference_TubeAcceptedHandler: no conference");
    if (!pConference)
        return;

    pConference->setTubeOfferedHandlerInvoked( true);

    TpDBusTubeChannel* pChannel = TP_DBUS_TUBE_CHANNEL( pSource);
    GError* pError = NULL;
    GDBusConnection* pTube = tp_dbus_tube_channel_accept_finish(
            pChannel, pResult, &pError);

    SAL_WARN_IF( !pTube, "tubes", "TeleConference_TubeAcceptedHandler: entered with error: " << pError->message);
    if (pError) {
        g_error_free( pError);
        return;
    }
    GHashTable* pParameters = tp_dbus_tube_channel_get_parameters( pChannel);
    const char* sUuid = tp_asv_get_string( pParameters, LIBO_TUBES_UUID);
    pConference->setUuid( OString( sUuid));

    pConference->setTube( pTube);
}


TeleConference::TeleConference( TpAccount* pAccount,
        TpDBusTubeChannel* pChannel, const OString sUuid, bool bMaster )
    :
        mpCollaboration( NULL ),
        mpAccount( NULL ),
        mpChannel( NULL ),
        msUuid( sUuid ),
        mbMaster( bMaster ),
        pImpl( new TeleConferenceImpl() )
{
    setChannel( pAccount, pChannel );
}


TeleConference::~TeleConference()
{
    // We're destructed from finalize()
    delete pImpl;
}

static void channel_closed_cb( TpChannel *channel, gpointer user_data, GObject * /* weak_object */ )
{
    Collaboration* pCollaboration = reinterpret_cast<Collaboration*> (user_data);
    if (TeleManager::existsCollaboration( pCollaboration ))
    {
        GtkWidget *dialog = gtk_message_dialog_new( NULL, static_cast<GtkDialogFlags> (0),
                GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
                "Contact %s lost, you'll now be working locally.",
                tp_contact_get_alias (tp_channel_get_target_contact (channel)) );
        g_signal_connect_swapped (dialog, "response",
                G_CALLBACK (gtk_widget_destroy), dialog);
        gtk_widget_show_all (dialog);

        pCollaboration->EndCollaboration();
    }
}


void TeleConference::setChannel( TpAccount *pAccount, TpDBusTubeChannel* pChannel )
{
    SAL_WARN_IF( mpChannel, "tubes", "TeleConference::setChannel: already have channel");
    if (mpChannel)
        g_object_unref( mpChannel);
    if (mpAccount)
        g_object_unref( mpAccount);

    mpChannel = pChannel;
    if (mpChannel)
        g_object_ref( mpChannel);

    mpAccount = pAccount;
    if (mpAccount)
        g_object_ref( mpAccount);
}


bool TeleConference::spinUntilTubeEstablished()
{
    while (!isTubeOfferedHandlerInvoked())
    {
        g_main_context_iteration( NULL, TRUE );
    }

    bool bOpen = pImpl->mpTube != NULL;
    SAL_INFO( "tubes", "TeleConference::spinUntilTubeEstablished: tube open: " << bOpen);
    return bOpen;
}


bool TeleConference::acceptTube()
{
    INFO_LOGGER( "TeleConference::acceptTube");

    SAL_WARN_IF( !mpChannel, "tubes", "TeleConference::acceptTube: no channel setup");
    SAL_WARN_IF( pImpl->mpTube, "tubes", "TeleConference::acceptTube: already tubed");
    if (!mpChannel || pImpl->mpTube)
        return false;

    tp_dbus_tube_channel_accept_async( mpChannel,
            TeleConference_TubeAcceptedHandler,
            this);
    return spinUntilTubeEstablished();
}


bool TeleConference::offerTube()
{
    INFO_LOGGER( "TeleConference::offerTube");

    SAL_WARN_IF( !mpChannel, "tubes", "TeleConference::offerTube: no channel");
    if (!mpChannel)
        return false;

    GHashTable* pParameters = tp_asv_new (
            LIBO_TUBES_UUID, G_TYPE_STRING, msUuid.getStr(),
            NULL);

    tp_dbus_tube_channel_offer_async(
            mpChannel,
            pParameters,
            TeleConference_TubeOfferedHandler,
            this);

    return spinUntilTubeEstablished();
}


bool TeleConference::setTube( GDBusConnection* pTube)
{
    INFO_LOGGER( "TeleConference::setTube");

    SAL_WARN_IF( pImpl->mpTube, "tubes", "TeleConference::setTube: already tubed");

    pImpl->mpTube = pTube;

    GDBusNodeInfo *introspection_data;
    static const GDBusInterfaceVTable interface_vtable =
    {
        TeleConference_MethodCallHandler,
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

    pImpl->maObjectRegistrationId = g_dbus_connection_register_object( pImpl->mpTube,
            LIBO_TUBES_DBUS_PATH, introspection_data->interfaces[0],
            &interface_vtable, this, NULL, NULL);
    g_assert (pImpl->maObjectRegistrationId > 0);

    g_dbus_node_info_unref (introspection_data);

    return true;
}

void TeleConference::setTubeOfferedHandlerInvoked( bool b )
{
    pImpl->mbTubeOfferedHandlerInvoked = b;
}

bool TeleConference::isTubeOfferedHandlerInvoked() const
{
    return pImpl->mbTubeOfferedHandlerInvoked;
}

bool TeleConference::isReady() const
{
    return mpChannel && pImpl->mpTube;
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

    TeleManager::unregisterDemoConference( this );

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

    if (pImpl->mpTube)
    {
        g_dbus_connection_unregister_object( pImpl->mpTube, pImpl->maObjectRegistrationId);
        g_dbus_connection_close_sync( pImpl->mpTube, NULL, NULL );
        g_object_unref( pImpl->mpTube );
        pImpl->mpTube = NULL;
    }

    //! *this gets destructed here!
}


bool TeleConference::sendPacket( const OString& rPacket )
{
    INFO_LOGGER( "TeleConference::sendPacket");

    if (!mpChannel && !pImpl->mpTube)
    {
        TeleManager::broadcastPacket( rPacket );
        return true;
    }

    SAL_WARN_IF( !pImpl->mpTube, "tubes", "TeleConference::sendPacket: no tube");
    if (!pImpl->mpTube)
        return false;

    /* FIXME: in GLib 2.32 we can use g_variant_new_fixed_array(). It does
     * essentially this.
     */
    void* pData = g_memdup( rPacket.getStr(), rPacket.getLength() );
    GVariant *pParameters = g_variant_new_from_data( G_VARIANT_TYPE("(ay)"),
            pData, rPacket.getLength(),
            FALSE,
            g_free, pData);

    g_dbus_connection_call( pImpl->mpTube,
            NULL, /* bus name; in multi-user case we'd address this to the master. */
            LIBO_TUBES_DBUS_PATH,
            LIBO_TUBES_DBUS_INTERFACE,
            LIBO_TUBES_DBUS_MSG_METHOD,
            pParameters, /* consumes the floating reference */
            NULL,
            G_DBUS_CALL_FLAGS_NONE,
            -1, NULL, NULL, NULL);

    // If we started the session, we can execute commands immediately.
    if (mbMaster && mpCollaboration)
        mpCollaboration->PacketReceived( rPacket );

    return true;
}

bool TeleConference::isMaster() const
{
    return mbMaster;
}

Collaboration* TeleConference::getCollaboration() const
{
    return mpCollaboration;
}

void TeleConference::setCollaboration( Collaboration* pCollaboration )
{
    mpCollaboration = pCollaboration;
    if (mpChannel)
    {
        GError *error = NULL;
        if (!tp_cli_channel_connect_to_closed( TP_CHANNEL (mpChannel),
                    channel_closed_cb, mpCollaboration, NULL, NULL, &error ))
        {
            SAL_WARN( "tubes", "Error when connecting to signal closed: " << error->message );
            g_error_free (error);
        }
    }
}

void TeleConference::invite( TpContact *pContact )
{
    INFO_LOGGER( "TeleConference::invite" );
    TpHandle aHandle = tp_contact_get_handle( pContact );
    GArray handles = { reinterpret_cast<gchar *> (&aHandle), 1 };
    tp_cli_channel_interface_group_call_add_members( TP_CHANNEL( mpChannel ),
            -1, &handles, NULL, NULL, NULL, NULL, NULL );
}

namespace {
class SendFileRequest {
public:
    SendFileRequest( TeleConference::FileSentCallback pCallback, void* pUserData, const char* sUuid )
        : mpCallback(pCallback)
        , mpUserData(pUserData)
        , msUuid(sUuid)
    {}

    TeleConference::FileSentCallback    mpCallback;
    void*                               mpUserData;
    const char*                         msUuid;
};
}

static void TeleConference_TransferDone( EmpathyFTHandler *handler, TpFileTransferChannel *, gpointer user_data)
{
    SendFileRequest *request = reinterpret_cast<SendFileRequest *>(user_data);

    if (request->mpCallback)
        request->mpCallback(true, request->mpUserData);
    delete request;
    g_object_unref (handler);
}

static void TeleConference_TransferError( EmpathyFTHandler *handler, const GError *error, gpointer user_data)
{
    SendFileRequest *request = reinterpret_cast<SendFileRequest *>(user_data);

    SAL_INFO( "tubes", "TeleConference_TransferError: " << error->message);

    if (request->mpCallback)
        request->mpCallback(false, request->mpUserData);
    delete request;
    g_object_unref (handler);
}

static void TeleConference_FTReady( EmpathyFTHandler *handler, GError *error, gpointer user_data)
{
    SendFileRequest *request = reinterpret_cast<SendFileRequest *>(user_data);

    if ( error != 0 )
    {
        if (request->mpCallback)
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
        empathy_ft_handler_set_service_name(handler, TeleManager::getFullServiceName().getStr());
        empathy_ft_handler_set_description(handler, request->msUuid);
        empathy_ft_handler_start_transfer(handler);
    }
}

// TODO: move sending file to TeleManager
extern void TeleManager_fileReceived( const OUString&, const OString& );
void TeleConference::sendFile( TpContact* pContact, const OUString& localUri, FileSentCallback pCallback, void* pUserData)
{
    INFO_LOGGER( "TeleConference::sendFile");

    if (!pContact)
    {
        // used in demo mode
        TeleManager_fileReceived( localUri, "demo" );
        return;
    }

    SAL_WARN_IF( ( !mpAccount || !mpChannel), "tubes",
        "can't send a file before the tube is set up");
    if ( !mpAccount || !mpChannel)
        return;

    GFile *pSource = g_file_new_for_uri(
        OUStringToOString( localUri, RTL_TEXTENCODING_UTF8).getStr() );
    SendFileRequest *pReq = new SendFileRequest( pCallback, pUserData, msUuid.getStr() );

    empathy_ft_handler_new_outgoing( mpAccount,
        pContact,
        pSource,
        0,
        TeleConference_FTReady, pReq);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
