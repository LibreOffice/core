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

#include "tubes/manager.hxx"
#include "tubes/constants.h"
#include <rtl/strbuf.hxx>
#include <rtl/uuid.h>
#include <osl/mutex.hxx>
#include <cstring>


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


using namespace rtl;
using namespace osl;


TeleManagerImpl* TeleManager::pImpl     = NULL;
sal_uInt32       TeleManager::nRefCount = 0;
rtl::OString     TeleManager::aNameSuffix;


/** Refcounted singleton implementation class. */
class TeleManagerImpl
{
public:
    GMainLoop*                          mpLoop;
    TpDBusDaemon*                       mpDBus;
    TpBaseClient*                       mpClient;
    TpBaseClient*                       mpFileTransferClient;
    TpAccountManager*                   mpAccountManager;
    TeleManager::AccountManagerStatus   meAccountManagerStatus;
    bool                                mbAccountManagerReadyHandlerInvoked;
    ContactList*                        mpContactList;

                            TeleManagerImpl();
                            ~TeleManagerImpl();
};


static void TeleManager_DBusTubeAcceptHandler(
        TpChannel*      pChannel,
        const char*     pAddress,
        const GError*   pError,
        gpointer        pUserData,
        GObject*        pWeakObject)
{
    INFO_LOGGER_F( "TeleManager_DBusTubeAcceptHandler");

    TpAccount* pAccount = TP_ACCOUNT(pWeakObject);

    SAL_WARN_IF( pError, "tubes", "TeleManager_DBusTubeAcceptHandler: entered with error: " << pError->message);
    if (pError)
    {
        g_object_unref(pAccount);
        return;
    }

    TeleManager* pManager = reinterpret_cast<TeleManager*>(pUserData);
    SAL_WARN_IF( !pManager, "tubes", "TeleManager_DBusTubeAcceptHandler: no manager");
    if (!pManager)
    {
        g_object_unref(pAccount);
        return;
    }

    pManager->acceptTube( pAccount, pChannel, pAddress);
    g_object_unref (pAccount);
}


static void TeleManager_DBusChannelHandler(
        TpSimpleHandler*            /*handler*/,
        TpAccount*                  pAccount,
        TpConnection*               /*connection*/,
        GList*                      pChannels,
        GList*                      /*requests_satisfied*/,
        gint64                      /*user_action_time*/,
        TpHandleChannelsContext*    pContext,
        gpointer                    pUserData)
{
    bool aAccepted = false;
    INFO_LOGGER_F( "TeleManager_DBusChannelHandler");

    TeleManager* pManager = reinterpret_cast<TeleManager*>(pUserData);
    SAL_WARN_IF( !pManager, "tubes", "TeleManager_DBusChannelHandler: no manager");
    if (!pManager)
        return;

    for (GList* p = pChannels; p; p = p->next)
    {
        TpChannel* pChannel = TP_CHANNEL(p->data);
        if (!pChannel)
            continue;

        SAL_INFO( "tubes", "TeleManager_DBusChannelHandler: incoming dbus channel: "
                << tp_channel_get_identifier( pChannel));

        if (tp_channel_get_channel_type_id( pChannel) == TP_IFACE_QUARK_CHANNEL_TYPE_DBUS_TUBE)
        {
            SAL_INFO( "tubes", "accepting");
            aAccepted = true;
            g_object_ref( pAccount);
            tp_cli_channel_type_dbus_tube_call_accept( pChannel, -1,
                    TP_SOCKET_ACCESS_CONTROL_CREDENTIALS,
                    TeleManager_DBusTubeAcceptHandler, pUserData, NULL,
                    G_OBJECT (pAccount));
        }
        else
        {
            SAL_INFO( "tubes", "ignored");
        }
    }

    if (aAccepted)
        tp_handle_channels_context_accept( pContext);
    else
    {
        GError *pError = g_error_new_literal( TP_ERRORS, TP_ERROR_CONFUSED,
            "None of these channels were LibreOffice D-Bus tubes; "
            "why did the Channel Dispatcher give them to us?");
        tp_handle_channels_context_fail( pContext, pError);
        g_clear_error (&pError);
    }
}

void TeleManager::TransferDone( EmpathyFTHandler *handler, TpFileTransferChannel *, gpointer pUserData)
{
    TeleManager* pManager = reinterpret_cast<TeleManager*>(pUserData);

    SAL_INFO( "tubes", "TeleConference_TransferDone: hooray!");
    GFile *gfile = empathy_ft_handler_get_gfile( handler);
    char *uri = g_file_get_uri( gfile);
    rtl::OUString aUri( uri, strlen( uri), RTL_TEXTENCODING_UTF8);
    g_free( uri);

    pManager->mpFileReceivedCallback( aUri, pManager->mpFileReceivedCallbackData);

    g_object_unref( handler);
}

static void TeleManager_TransferError( EmpathyFTHandler *handler, const GError *error, void*)
{
    SAL_INFO( "tubes", "TeleConference_TransferError: " << error->message);

    g_object_unref( handler);
}

static void
TeleManager_IncomingHandlerReady (
    EmpathyFTHandler*   pHandler,
    GError*             pError,
    void*               pUserData)
{
    TeleManager* pManager = reinterpret_cast<TeleManager*>(pUserData);

    if (pError)
    {
        SAL_INFO ("tubes", "failed to prepare incoming transfer: " << pError->message);
        g_object_unref( pHandler);
        return;
    }

    /* The filename suggested by the sender, which in our case is the last bit
     * of whatever URI got passed to ::sendFile()
     */
    const char* pFileName = empathy_ft_handler_get_filename( pHandler);
    char* pLocalUri = g_strdup_printf( "file:///tmp/LibreOffice-collab-%s", pFileName);
    GFile *pDestination = g_file_new_for_uri( pLocalUri);
    g_free( pLocalUri);

    empathy_ft_handler_incoming_set_destination( pHandler, pDestination);
    g_object_unref( pDestination);

    g_signal_connect( pHandler, "transfer-done", G_CALLBACK (&TeleManager::TransferDone), pManager);
    g_signal_connect( pHandler, "transfer-error", G_CALLBACK (TeleManager_TransferError), pManager);
    empathy_ft_handler_start_transfer( pHandler);
}

static void TeleManager_FileTransferHandler(
        TpSimpleHandler*            /*handler*/,
        TpAccount*                  /*Account*/,
        TpConnection*               /*connection*/,
        GList*                      pChannels,
        GList*                      /*requests_satisfied*/,
        gint64                      /*user_action_time*/,
        TpHandleChannelsContext*    pContext,
        gpointer                    pUserData)
{
    bool aAccepted = false;
    INFO_LOGGER_F( "TeleManager_FileTransferHandler");

    TeleManager* pManager = reinterpret_cast<TeleManager*>(pUserData);
    SAL_WARN_IF( !pManager, "tubes", "TeleManager_FileTransferHandler: no manager");
    if (!pManager)
        return;

    for (GList* p = pChannels; p; p = p->next)
    {
        TpChannel* pChannel = TP_CHANNEL(p->data);

        SAL_INFO( "tubes", "TeleManager_FileTransferHandler: incoming dbus channel: "
                << tp_channel_get_identifier( pChannel));

        if (TP_IS_FILE_TRANSFER_CHANNEL( pChannel))
        {
            SAL_INFO( "tubes", "accepting file transfer");
            empathy_ft_handler_new_incoming( TP_FILE_TRANSFER_CHANNEL( pChannel),
                TeleManager_IncomingHandlerReady, pManager);
            aAccepted = true;
        }
        else
        {
            SAL_INFO( "tubes", "ignored");
        }
    }

    if (aAccepted)
        tp_handle_channels_context_accept( pContext);
    else
    {
        GError *pError = g_error_new_literal( TP_ERRORS, TP_ERROR_CONFUSED,
            "None of these channels were file transfers; "
            "why did the Channel Dispatcher give them to us?");
        tp_handle_channels_context_fail( pContext, pError);
        g_clear_error (&pError);
    }
}

static void TeleManager_ChannelReadyHandler(
        GObject*        pSourceObject,
        GAsyncResult*   pResult,
        gpointer        pUserData
        )
{
    INFO_LOGGER_F( "TeleManager_ChannelReadyHandler");

    TeleConference* pConference = reinterpret_cast<TeleConference*>(pUserData);
    SAL_WARN_IF( !pConference, "tubes", "TeleManager_ChannelReadyHandler: no conference");
    if (!pConference)
        return;

    TeleManager* pManager = pConference->getManager();
    SAL_WARN_IF( !pManager, "tubes", "TeleManager_ChannelReadyHandler: no manager");
    if (!pManager)
        return;

    pManager->setChannelReadyHandlerInvoked( true);

    TpAccountChannelRequest* pChannelRequest = TP_ACCOUNT_CHANNEL_REQUEST( pSourceObject);
    GError* pError = NULL;
    TpChannel * pChannel = tp_account_channel_request_create_and_handle_channel_finish(
            pChannelRequest, pResult, NULL, &pError);
    if (!pChannel)
    {
        // "account isn't Enabled" means just that..
        /* FIXME: detect and handle, domain=132, code=3 */
        SAL_WARN( "tubes", "TeleManager_ChannelReadyHandler: no channel: " << pError->message);
        g_error_free( pError);
        return;
    }

    pConference->setChannel( tp_account_channel_request_get_account( pChannelRequest),
            pChannel);
    pConference->offerTube();
}

static gboolean caps_hack_timeout_cb (void *pUserData)
{
    TeleManager* pManager = reinterpret_cast<TeleManager*>(pUserData);

    pManager->setAccountManagerReadyHandlerInvoked( true);
    return FALSE;
}

static void TeleManager_AccountManagerReadyHandler(
        GObject*        pSourceObject,
        GAsyncResult*   pResult,
        gpointer        pUserData
        )
{
    INFO_LOGGER_F( "TeleManager_AccountManagerReadyHandler");

    TeleManager* pManager = reinterpret_cast<TeleManager*>(pUserData);
    SAL_WARN_IF( !pManager, "tubes", "TeleManager_AccountManagerReadyHandler: no manager");
    if (!pManager)
        return;

    // Hack
    // pManager->setAccountManagerReadyHandlerInvoked( true);
    g_timeout_add_seconds( 2, caps_hack_timeout_cb, pManager);

    GError* pError = NULL;
    gboolean bPrepared = tp_proxy_prepare_finish( pSourceObject, pResult, &pError);
    SAL_WARN_IF( !bPrepared, "tubes", "TeleManager_AccountManagerReadyHandler: not prepared");
    if (!bPrepared || pError)
    {
        SAL_WARN_IF( pError, "tubes", "TeleManager_AccountManagerReadyHandler: error: " << pError->message);
        g_error_free( pError);
    }

    pManager->setAccountManagerReady( bPrepared);
}


TeleManager::TeleManager( const Link& rLink, bool bCreateOwnGMainLoop )
    :
        maLink( rLink),
        mbChannelReadyHandlerInvoked( false)
{
    // The glib object types need to be initialized, else we aren't going
    // anywhere.
    g_type_init();

    MutexGuard aGuard( GetMutex());
    ++nRefCount;
    if (!pImpl)
        pImpl = new TeleManagerImpl;

    // We need a main loop, else no callbacks.
    /* TODO: could the loop be run in another thread? */
    if (bCreateOwnGMainLoop && !pImpl->mpLoop)
        pImpl->mpLoop = g_main_loop_new( NULL, FALSE);
}


TeleManager::~TeleManager()
{
    MutexGuard aGuard( GetMutex());
    if (!--nRefCount)
    {
        disconnect();

        delete pImpl;
        pImpl = NULL;
    }
}


bool TeleManager::connect()
{
    INFO_LOGGER( "TeleManager::connect");

    MutexGuard aGuard( GetMutex());

    /* TODO: also check whether client could be registered and retry if not? */
    SAL_INFO_IF( pImpl->mpDBus && pImpl->mpClient, "tubes", "TeleManager::connect: already connected");
    if (pImpl->mpDBus && pImpl->mpClient)
        return true;

    GError* pError = NULL;
    pImpl->mpDBus = tp_dbus_daemon_dup( &pError);
    SAL_WARN_IF( !pImpl->mpDBus, "tubes", "TeleManager::connect: no dbus daemon");
    if (!pImpl->mpDBus || pError)
    {
        SAL_WARN_IF( pError, "tubes", "TeleManager::connect: dbus daemon error: " << pError->message);
        g_error_free( pError);
        return false;
    }

    TpAutomaticClientFactory* pFactory = tp_automatic_client_factory_new( pImpl->mpDBus);
    SAL_WARN_IF( !pFactory, "tubes", "TeleManager::connect: no client factory");
    if (!pFactory)
        return false;

    TpAccountManager* pAccountManager = tp_account_manager_new_with_factory (
        TP_SIMPLE_CLIENT_FACTORY (pFactory));
    tp_account_manager_set_default( pAccountManager);

    /* Takes our ref. */
    pImpl->mpAccountManager = pAccountManager;

    pImpl->mpContactList = new ContactList(pAccountManager);

    pImpl->mpClient = tp_simple_handler_new_with_factory(
            TP_SIMPLE_CLIENT_FACTORY (pFactory), // factory
            FALSE,                          // bypass_approval
            FALSE,                          // requests
            getFullClientName().getStr(),   // name
            FALSE,                          // uniquify
            TeleManager_DBusChannelHandler, // callback
            this,                           // user_data
            NULL                            // destroy
            );
    SAL_WARN_IF( !pImpl->mpClient, "tubes", "TeleManager::connect: no client");
    if (!pImpl->mpClient)
        return false;

    // Setup client handler for buddy channels with our service.
    tp_base_client_take_handler_filter( pImpl->mpClient,
            tp_asv_new(
                TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
                TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT, TP_HANDLE_TYPE_CONTACT,
                TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, getFullServiceName().getStr(),
                NULL));

    /* TODO: setup filters for LibreOfficeCalc, LibreOfficeWriter, ... */

/* FIXME: once we can handle MUCs, this is additional to buddy channels! */
#if 0
    // Setup client handler for MUC channels with our service.
    tp_base_client_take_handler_filter( pImpl->mpClient,
            tp_asv_new(
                TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
                TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT, TP_HANDLE_TYPE_ROOM,
                TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, getFullServiceName().getStr(),
                NULL));
#endif

    if (!tp_base_client_register( pImpl->mpClient, &pError))
    {
        SAL_WARN( "tubes", "TeleManager::connect: error registering client handler: " << pError->message);
        g_error_free( pError);
        return false;
    }

    SAL_INFO( "tubes", "TeleManager::connect: bus name: " << tp_base_client_get_bus_name( pImpl->mpClient));
    SAL_INFO( "tubes", "TeleManager::connect: object path: " << tp_base_client_get_object_path( pImpl->mpClient));

    /* Register a second "head" for incoming file transfers. This uses a more
     * specific filter than Empathy's handler by matching on the file
     * transfer's ServiceName property, and uses bypass_approval to ensure the
     * user isn't prompted before the channel gets passed to us.
     */
    pImpl->mpFileTransferClient = tp_simple_handler_new_with_factory (
            TP_SIMPLE_CLIENT_FACTORY( pFactory),            // factory
            TRUE,                                           // bypass_approval
            FALSE,                                          // requests
            getFullClientName().getStr(),                   // name
            TRUE,                                           // uniquify to get a different bus name to the main client, above
            TeleManager_FileTransferHandler,                // callback
            this,                                           // user_data
            NULL                                            // destroy
            );
    tp_base_client_take_handler_filter( pImpl->mpFileTransferClient,
            tp_asv_new(
                TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER,
                TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT, TP_HANDLE_TYPE_CONTACT,
                TP_PROP_CHANNEL_TYPE_FILE_TRANSFER_DESCRIPTION, G_TYPE_STRING, getFullServiceName().getStr(),
                NULL));

    if (!tp_base_client_register( pImpl->mpFileTransferClient, &pError))
    {
        /* This shouldn't fail if registering the main handler succeeded */
        SAL_WARN( "tubes", "TeleManager::connect: error registering file transfer handler: " << pError->message);
        g_error_free( pError);
        return false;
    }

    return true;
}


#if 0
/* TODO: factor out common code with startBuddySession() */
bool TeleManager::startGroupSession( const rtl::OUString& rUConferenceRoom, const rtl::OUString& rUConferenceServer )
{
    INFO_LOGGER( "TeleManager::startGroupSession");

    if (!getMyAccount())
        return false;

    OString aSessionId( TeleManager::createUuid());

    TeleConferencePtr pConference( new TeleConference( this, NULL, aSessionId));
    maConferences.push_back( pConference);

    /* TODO: associate the document with this session and conference */

    /* FIXME: does this work at all _creating_ a MUC? */
    // Use conference and server if given, else create conference.
    OString aConferenceRoom( OUStringToOString( rUConferenceRoom, RTL_TEXTENCODING_UTF8));
    OString aConferenceServer( OUStringToOString( rUConferenceServer, RTL_TEXTENCODING_UTF8));
    OStringBuffer aBuf(64);
    if (!aConferenceRoom.isEmpty() && !aConferenceServer.isEmpty())
        aBuf.append( aConferenceRoom).append( '@').append( aConferenceServer);
    else
    {
        aBuf.append( aSessionId);
        if (!aConferenceServer.isEmpty())
            aBuf.append( '@').append( aConferenceServer);
        /* FIXME: else? bail out? we have only a session ID without server then */
    }
    OString aTarget( aBuf.makeStringAndClear());
    pConference->setTarget( aTarget);

    SAL_INFO( "tubes", "TeleManager::startGroupSession: creating channel request from "
            << maAccountID.getStr() << " to " << aTarget.getStr());

    // MUC request
    GHashTable* pRequest = tp_asv_new(
            TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
            TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, TP_TYPE_HANDLE, TP_HANDLE_TYPE_ROOM,
            TP_PROP_CHANNEL_TARGET_ID, G_TYPE_STRING, aTarget.getStr(),
            TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, getFullServiceName().getStr(),
            NULL);

    TpAccountChannelRequest * pChannelRequest = tp_account_channel_request_new(
            mpAccount, pRequest, TP_USER_ACTION_TIME_NOT_USER_ACTION);
    g_hash_table_unref( pRequest);
    SAL_WARN_IF( !pChannelRequest, "tubes", "TeleManager::startGroupSession: no channel");
    if (!pChannelRequest)
        return false;

    setChannelReadyHandlerInvoked( false);

    tp_account_channel_request_create_and_handle_channel_async(
            pChannelRequest, NULL, TeleManager_ChannelReadyHandler, pConference.get());

    iterateLoop( &TeleManager::isChannelReadyHandlerInvoked);

    return pConference->getChannel() != NULL && pConference->isTubeOpen();
}
#endif


/* TODO: factor out common code with startGroupSession() */
bool TeleManager::startBuddySession( TpAccount *pAccount, TpContact *pBuddy )
{
    INFO_LOGGER( "TeleManager::startBuddySession");

    OString aSessionId( TeleManager::createUuid());

    TeleConferencePtr pConference( new TeleConference( this, NULL, NULL, aSessionId));
    maConferences.push_back( pConference);

    /* TODO: associate the document with this session and conference */
    const char *pIdentifier = tp_contact_get_identifier( pBuddy);
    SAL_INFO( "tubes", "TeleManager::startBuddySession: creating channel request from "
            << tp_account_get_path_suffix( pAccount)
            << " to " << pIdentifier);

    GHashTable* pRequest = tp_asv_new(
            TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
            TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, TP_TYPE_HANDLE, TP_HANDLE_TYPE_CONTACT,
            TP_PROP_CHANNEL_TARGET_ID, G_TYPE_STRING, pIdentifier,
            TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, getFullServiceName().getStr(),
            NULL);

    TpAccountChannelRequest * pChannelRequest = tp_account_channel_request_new(
            pAccount, pRequest, TP_USER_ACTION_TIME_NOT_USER_ACTION);
    SAL_WARN_IF( !pChannelRequest, "tubes", "TeleManager::startBuddySession: no channel");
    if (!pChannelRequest)
    {
        g_hash_table_unref( pRequest);
        return false;
    }

    setChannelReadyHandlerInvoked( false);

    tp_account_channel_request_create_and_handle_channel_async(
            pChannelRequest, NULL, TeleManager_ChannelReadyHandler, pConference.get());

    iterateLoop( &TeleManager::isChannelReadyHandlerInvoked);

    g_object_unref( pChannelRequest);
    g_hash_table_unref( pRequest);

    return pConference->getChannel() != NULL && pConference->isTubeOpen();
}

void TeleManager::prepareAccountManager()
{
    INFO_LOGGER( "TeleManager::prepareAccountManager");

    MutexGuard aGuard( GetMutex());

    SAL_INFO_IF( pImpl->meAccountManagerStatus == AMS_PREPARED, "tubes",
            "TeleManager::prepareAccountManager: already prepared");
    if (pImpl->meAccountManagerStatus == AMS_PREPARED)
        return;

    SAL_WARN_IF( pImpl->meAccountManagerStatus == AMS_INPREPARATION, "tubes",
            "TeleManager::prepareAccountManager: already in preparation");
    if (pImpl->meAccountManagerStatus == AMS_INPREPARATION)
        return;

    SAL_WARN_IF( pImpl->meAccountManagerStatus != AMS_UNINITIALIZED, "tubes",
            "TeleManager::prepareAccountManager: yet another attempt");

    SAL_WARN_IF( !pImpl->mpAccountManager, "tubes",
            "TeleManager::prepareAccountManager: called before ::connect()");
    if (!pImpl->mpAccountManager)
        return;

    pImpl->meAccountManagerStatus = AMS_INPREPARATION;
    setAccountManagerReadyHandlerInvoked( false);

    tp_proxy_prepare_async( pImpl->mpAccountManager, NULL, TeleManager_AccountManagerReadyHandler, this);

    iterateLoop( &TeleManager::isAccountManagerReadyHandlerInvoked);
}


TeleManager::AccountManagerStatus TeleManager::getAccountManagerStatus() const
{
    return pImpl->meAccountManagerStatus;
}


void TeleManager::setAccountManagerReadyHandlerInvoked( bool b )
{
    pImpl->mbAccountManagerReadyHandlerInvoked = b;
}


bool TeleManager::isAccountManagerReadyHandlerInvoked() const
{
    return pImpl->mbAccountManagerReadyHandlerInvoked;
}

ContactList* TeleManager::getContactList()
{
    return pImpl->mpContactList;
}

TpAccount* TeleManager::getAccount( const rtl::OString& rAccountID )
{
    INFO_LOGGER( "TeleManager::getMyAccount");

    SAL_WARN_IF( pImpl->meAccountManagerStatus != AMS_PREPARED, "tubes",
            "TeleManager::getMyAccount: Account Manager not prepared");
    if (pImpl->meAccountManagerStatus != AMS_PREPARED)
        return NULL;

    GList* pAccounts = tp_account_manager_get_valid_accounts( pImpl->mpAccountManager);
    SAL_WARN_IF( !pAccounts, "tubes", "TeleManager::getMyAccount: no valid accounts");
    if (!pAccounts)
        return NULL;

    // Find our account to use.
    TpAccount* pAccount = NULL;
    for (GList* pA = pAccounts; pA; pA = pA->next)
    {
        TpAccount* pAcc = TP_ACCOUNT( pA->data);
        const gchar* pID = tp_account_get_normalized_name( pAcc);
        if (pID && rAccountID == pID)
        {
            pAccount = pAcc;
            break;  // for
        }
    }
    g_list_free( pAccounts);

    SAL_WARN_IF( !pAccount, "tubes", "TeleManager::getMyAccount: no account");
    if (!pAccount)
        return NULL;

    return pAccount;
}


sal_uInt32 TeleManager::sendPacket( const TelePacket& rPacket ) const
{
    INFO_LOGGER( "TeleManager::sendPacket");

    sal_uInt32 nSent = 0;
    // Access to data ByteStream array forces reference count of one, provide
    // non-const instance here before passing it down to each conference.
    TelePacket aPacket( rPacket);
    for (TeleConferenceVector::const_iterator it = maConferences.begin(); it != maConferences.end(); ++it)
    {
        if ((*it)->sendPacket( aPacket))
            ++nSent;
        /* TODO: what if failed? */
    }
    return nSent;
}


long TeleManager::callbackOnRecieved( TeleConference* pConference ) const
{
    INFO_LOGGER( "TeleManager::callbackOnRecieved");

    if (maLink.IsSet())
        return maLink.Call( pConference);
    return 0;
}


bool TeleManager::popPacket( TelePacket& rPacket )
{
    INFO_LOGGER( "TeleManager::popPacket");

    for (TeleConferenceVector::const_iterator it = maConferences.begin(); it != maConferences.end(); ++it)
    {
        if ((*it)->popPacket( rPacket))
            return true;
    }
    return false;
}

void TeleManager::sendFile( rtl::OUString &localUri, TeleConference::FileSentCallback pCallback, void* pUserData)
{
    INFO_LOGGER( "TeleManager::sendFile");

    /* TODO: pluralize */
    for (TeleConferenceVector::const_iterator it = maConferences.begin(); it != maConferences.end(); ++it)
    {
        (*it)->sendFile( localUri, pCallback, pUserData);
        return;
    }
}

void TeleManager::setFileReceivedCallback( TeleManager::FileReceivedCallback callback, void* pUserData )
{
    mpFileReceivedCallback = callback;
    mpFileReceivedCallbackData = pUserData;
}

void TeleManager::unregisterConference( TeleConferencePtr pConference )
{
    INFO_LOGGER( "TeleManager::unregisterConference");

    TeleConferenceVector::iterator it = ::std::find( maConferences.begin(), maConferences.end(), pConference);
    if (it != maConferences.end())
        maConferences.erase( it);
}


void TeleManager::disconnect()
{
    INFO_LOGGER( "TeleManager::disconnect");

    if (!pImpl->mpClient)
        return;

    tp_base_client_unregister( pImpl->mpClient);
    pImpl->mpClient = NULL;

    tp_base_client_unregister( pImpl->mpFileTransferClient);
    pImpl->mpFileTransferClient = NULL;

    size_t nSize = maConferences.size();
    for (size_t i=0; i < nSize; /*nop*/)
    {
        maConferences[i]->close();
        // close() may remove the conference from the vector and move following
        // elements to this position (hence we don't use an iterator here),
        // adjust accordingly.
        size_t n = maConferences.size();
        if (n < nSize)
            nSize = n;
        else
            ++i;
    }
}


void TeleManager::acceptTube( TpAccount* pAccount, TpChannel* pChannel, const char* pAddress )
{
    INFO_LOGGER( "TeleManager::acceptTube");

    SAL_INFO( "tubes", "TeleManager::acceptTube: address " << pAddress);

    SAL_WARN_IF( !pChannel || !pAddress, "tubes", "TeleManager::acceptTube: no channel or no address");
    if (!pChannel || !pAddress)
        return;

    TeleConferencePtr pConference( new TeleConference( this, pAccount, pChannel, ""));
    maConferences.push_back( pConference);
    pConference->acceptTube( pAddress);
}


void TeleManager::setAccountManagerReady( bool bPrepared)
{
    pImpl->meAccountManagerStatus = (bPrepared ? AMS_PREPARED : AMS_UNPREPARABLE);
}


rtl::OString TeleManager::getFullClientName()
{
    OStringBuffer aBuf(64);
    aBuf.append( RTL_CONSTASCII_STRINGPARAM( LIBO_CLIENT_SUFFIX)).append( aNameSuffix);
    return aBuf.makeStringAndClear();
}


rtl::OString TeleManager::getFullServiceName()
{
    OStringBuffer aBuf(64);
    aBuf.append( RTL_CONSTASCII_STRINGPARAM( LIBO_DTUBE_SERVICE)).append( aNameSuffix);
    return aBuf.makeStringAndClear();
}


rtl::OString TeleManager::getFullObjectPath()
{
    OStringBuffer aBuf(64);
    aBuf.append( '/').append( RTL_CONSTASCII_STRINGPARAM( LIBO_DTUBE_SERVICE)).append( aNameSuffix);
    OString aStr( aBuf.makeStringAndClear().replace( '.', '/'));
    return aStr;
}


void TeleManager::iterateLoop()
{
    GMainContext* pContext = getMainContext();
    g_main_context_iteration( pContext, TRUE);
}


void TeleManager::iterateLoop( CallBackInvokedFunc pFunc )
{
    GMainContext* pContext = getMainContext();
    while (!(*pFunc)())
    {
        g_main_context_iteration( pContext, TRUE);
    }
}


void TeleManager::iterateLoop( ManagerCallBackInvokedFunc pFunc )
{
    GMainContext* pContext = getMainContext();
    while (!(this->*pFunc)())
    {
        g_main_context_iteration( pContext, TRUE);
    }
}


void TeleManager::iterateLoop( const TeleConference* pConference, ConferenceCallBackInvokedFunc pFunc )
{
    GMainContext* pContext = getMainContext();
    while (!(pConference->*pFunc)())
    {
        g_main_context_iteration( pContext, TRUE);
    }
}


void TeleManager::flushLoop() const
{
    if (pImpl->mpLoop)
    {
        GMainContext* pContext = g_main_loop_get_context( pImpl->mpLoop);
        while (g_main_context_iteration( pContext, FALSE))
        {
        }
    }
}


GMainLoop* TeleManager::getMainLoop() const
{
    return pImpl->mpLoop;
}


GMainContext* TeleManager::getMainContext() const
{
    return (pImpl->mpLoop ? g_main_loop_get_context( pImpl->mpLoop) : NULL);
}


// static
rtl::OString TeleManager::createUuid()
{
    sal_uInt8 nId[16];
    rtl_createUuid( nId, 0, sal_True);
    char aBuf[33];
    for (size_t i=0; i<16; ++i)
    {
        snprintf( aBuf+2*i, 3, "%02x", (unsigned char)nId[i]);
    }
    aBuf[32] = 0;
    return rtl::OString( aBuf);
}


// static
Mutex& TeleManager::GetMutex()
{
    static Mutex* pMutex = NULL;
    if (!pMutex)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex());
        if (!pMutex)
            pMutex = new Mutex;
    }
    return *pMutex;
}


// static
void TeleManager::addSuffixToNames( const char* pName )
{
    aNameSuffix = pName;
}

// ===========================================================================

TeleManagerImpl::TeleManagerImpl()
    :
        mpLoop( NULL),
        mpDBus( NULL),
        mpClient( NULL),
        mpFileTransferClient( NULL),
        mpAccountManager( NULL),
        meAccountManagerStatus( TeleManager::AMS_UNINITIALIZED),
        mbAccountManagerReadyHandlerInvoked( false)
{
}


TeleManagerImpl::~TeleManagerImpl()
{
    if (mpClient)
        g_object_unref( mpClient);
    if (mpFileTransferClient)
        g_object_unref( mpFileTransferClient);
    if (mpDBus)
        g_object_unref( mpDBus);
    if (mpAccountManager)
        g_object_unref( mpAccountManager);
    if (mpContactList)
        delete mpContactList;
    if (mpLoop)
        g_main_loop_unref( mpLoop);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
