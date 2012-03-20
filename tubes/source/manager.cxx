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
#include <rtl/strbuf.hxx>
#include <rtl/uuid.h>


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

// To form "org.freedesktop.Telepathy.Client.LibreOfficeWhatEver" (bus name)
// or "/org/freedesktop/Telepathy/Client/LibreOfficeWhatEver" (object path)
#define LIBO_TP_NAME_PREFIX "LibreOffice"


TpAccountManager* TeleManager::mpAccountManager = NULL;
TeleManager::AccountManagerStatus TeleManager::meAccountManagerStatus = AMS_UNINITIALIZED;
bool TeleManager::mbAccountManagerReadyHandlerInvoked = false;


static void TeleManager_DBusTubeAcceptHandler(
        TpChannel*      pChannel,
        const char*     pAddress,
        const GError*   pError,
        gpointer        pUserData,
        GObject*        /*weak_object*/)
{
    INFO_LOGGER_F( "TeleManager_DBusTubeAcceptHandler");

    SAL_WARN_IF( pError, "tubes", "TeleManager_DBusTubeAcceptHandler: entered with error: " << pError->message);
    if (pError)
        return;

    TeleManager* pManager = reinterpret_cast<TeleManager*>(pUserData);
    SAL_WARN_IF( !pManager, "tubes", "TeleManager_DBusTubeAcceptHandler: no manager");
    if (!pManager)
        return;

    pManager->acceptTube( pChannel, pAddress);
}


static void TeleManager_DBusChannelHandler(
        TpSimpleHandler*            /*handler*/,
        TpAccount*                  /*account*/,
        TpConnection*               /*connection*/,
        GList*                      pChannels,
        GList*                      /*requests_satisfied*/,
        gint64                      /*user_action_time*/,
        TpHandleChannelsContext*    pContext,
        gpointer                    pUserData)
{
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
            tp_cli_channel_type_dbus_tube_call_accept( pChannel, -1,
                    TP_SOCKET_ACCESS_CONTROL_CREDENTIALS,
                    TeleManager_DBusTubeAcceptHandler, pUserData, NULL, NULL);
        }
        else
        {
            SAL_INFO( "tubes", "ignored");
        }
    }

    tp_handle_channels_context_accept( pContext);
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

    GError* pError = NULL;
    TpChannel * pChannel = tp_account_channel_request_create_and_handle_channel_finish(
            TP_ACCOUNT_CHANNEL_REQUEST( pSourceObject), pResult, NULL, &pError);
    if (!pChannel)
    {
        // "account isn't Enabled" means just that..
        /* FIXME: detect and handle, domain=132, code=3 */
        SAL_WARN( "tubes", "TeleManager_ChannelReadyHandler: no channel: " << pError->message);
        g_error_free( pError);
        return;
    }

    pConference->setChannel( pChannel);
    pConference->offerTube();
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

    TeleManager::setAccountManagerReadyHandlerInvoked( true);

    GError* pError = NULL;
    gboolean bPrepared = tp_proxy_prepare_finish( pSourceObject, pResult, &pError);
    SAL_WARN_IF( !bPrepared, "tubes", "TeleManager_AccountManagerReadyHandler: not prepared");
    if (!bPrepared || pError)
    {
        SAL_WARN_IF( pError, "tubes", "TeleManager_AccountManagerReadyHandler: error: " << pError->message);
        g_error_free( pError);
    }

    TeleManager::setAccountManagerReady( bPrepared);
}


TeleManager::TeleManager( const rtl::OUString& rAccount, const rtl::OUString& rService, bool bCreateOwnGMainLoop )
    :
        maAccountID( OUStringToOString( rAccount, RTL_TEXTENCODING_UTF8)),
        mpLoop( NULL),
        mpAccount( NULL),
        mpConnection( NULL),
        mpDBus( NULL),
        mpClient( NULL),
        mbChannelReadyHandlerInvoked( false)
{
    OStringBuffer aBuf(64);
    aBuf.append( RTL_CONSTASCII_STRINGPARAM( LIBO_TP_NAME_PREFIX)).append(
            OUStringToOString( rService, RTL_TEXTENCODING_UTF8));
    maService = aBuf.makeStringAndClear();
    // Ensure a clean name suitable for object paths.
    maService.replace( '.', '_');

    // The glib object types need to be initialized, else we aren't going
    // anywhere.
    g_type_init();

    // We need a main loop, else no callbacks.
    /* TODO: could the loop be run in another thread? */
    if (bCreateOwnGMainLoop)
        mpLoop = g_main_loop_new( NULL, FALSE);
}


TeleManager::~TeleManager()
{
    disconnect();

    if (mpAccount)
        g_object_unref( mpAccount);
    if (mpConnection)
        g_object_unref( mpConnection);
    if (mpClient)
        g_object_unref( mpClient);
    if (mpDBus)
        g_object_unref( mpDBus);

    if (mpLoop)
        g_main_loop_unref( mpLoop);
}


bool TeleManager::connect()
{
    INFO_LOGGER( "TeleManager::connect");

    MainLoopFlusher aFlusher( this);

    SAL_WARN_IF( mpDBus || mpClient, "tubes", "TeleManager::connect: already connected");
    if (mpDBus || mpClient)
        return false;

    GError* pError = NULL;
    mpDBus = tp_dbus_daemon_dup( &pError);
    SAL_WARN_IF( !mpDBus, "tubes", "TeleManager::connect: no dbus daemon");
    if (!mpDBus || pError)
    {
        SAL_WARN_IF( pError, "tubes", "TeleManager::connect: dbus daemon error: " << pError->message);
        g_error_free( pError);
        return false;
    }

    TpSimpleClientFactory* pFactory = tp_simple_client_factory_new( mpDBus);
    SAL_WARN_IF( !pFactory, "tubes", "TeleManager::connect: no client factory");
    if (!pFactory)
        return false;

    /* TODO: does that still work with uniquify? Or is the service not
     * recognized anymore? Without uniquify it is not possible to register two
     * instances of clients on the same DBus. */

    /* FIXME: testing, first real, all others uniquified */
    static gboolean bUniquify = FALSE;

    mpClient = tp_simple_handler_new_with_factory(
            pFactory,                       // factory
            TRUE,                           // bypass_approval
            FALSE,                          // requests
            maService.getStr(),             // name
            bUniquify,                      // uniquify
            TeleManager_DBusChannelHandler, // callback
            this,                           // user_data
            NULL                            // destroy
            );
    bUniquify = TRUE;
    SAL_WARN_IF( !mpClient, "tubes", "TeleManager::connect: no client");
    if (!mpClient)
        return false;

    // Setup client handler for buddy channels with our service.
    tp_base_client_take_handler_filter( mpClient,
            tp_asv_new(
                TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
                TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT, TP_HANDLE_TYPE_CONTACT,
                TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, getFullServiceName().getStr(),
                NULL));

/* FIXME: once we can handle MUCs, this is additional to buddy channels! */
#if 0
    // Setup client handler for MUC channels with our service.
    tp_base_client_take_handler_filter( mpClient,
            tp_asv_new(
                TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
                TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT, TP_HANDLE_TYPE_ROOM,
                TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, getFullServiceName().getStr(),
                NULL));
#endif

    if (!tp_base_client_register( mpClient, &pError))
    {
        SAL_WARN( "tubes", "TeleManager::connect: error registering client handler: " << pError->message);
        g_error_free( pError);
        return false;
    }

    SAL_INFO( "tubes", "TeleManager::connect: bus name: " << tp_base_client_get_bus_name( mpClient));
    SAL_INFO( "tubes", "TeleManager::connect: object path: " << tp_base_client_get_object_path( mpClient));

    return true;
}


/* TODO: factor out common code with startBuddySession() */
bool TeleManager::startGroupSession( const rtl::OUString& rUConferenceRoom, const rtl::OUString& rUConferenceServer )
{
    INFO_LOGGER( "TeleManager::startGroupSession");

    MainLoopFlusher aFlusher( this);

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


/* TODO: factor out common code with startGroupSession() */
bool TeleManager::startBuddySession( const rtl::OUString& rBuddy )
{
    INFO_LOGGER( "TeleManager::startBuddySession");

    MainLoopFlusher aFlusher( this);

    if (!getMyAccount())
        return false;

    OString aSessionId( TeleManager::createUuid());

    TeleConferencePtr pConference( new TeleConference( this, NULL, aSessionId));
    maConferences.push_back( pConference);

    /* TODO: associate the document with this session and conference */

    OString aTarget( OUStringToOString( rBuddy, RTL_TEXTENCODING_UTF8));
    pConference->setTarget( aTarget);

    SAL_INFO( "tubes", "TeleManager::startBuddySession: creating channel request from "
            << maAccountID.getStr() << " to " << aTarget.getStr());

    GHashTable* pRequest = tp_asv_new(
            TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
            TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, TP_TYPE_HANDLE, TP_HANDLE_TYPE_CONTACT,
            TP_PROP_CHANNEL_TARGET_ID, G_TYPE_STRING, aTarget.getStr(),
            TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, getFullServiceName().getStr(),
            NULL);

    TpAccountChannelRequest * pChannelRequest = tp_account_channel_request_new(
            mpAccount, pRequest, TP_USER_ACTION_TIME_NOT_USER_ACTION);
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

    MainLoopFlusher aFlusher( this);

    SAL_INFO_IF( meAccountManagerStatus == AMS_PREPARED, "tubes",
            "TeleManager::prepareAccountManager: already prepared");
    if (meAccountManagerStatus == AMS_PREPARED)
        return;

    SAL_WARN_IF( meAccountManagerStatus == AMS_INPREPARATION, "tubes",
            "TeleManager::prepareAccountManager: already in preparation");
    if (meAccountManagerStatus == AMS_INPREPARATION)
        return;

    SAL_WARN_IF( meAccountManagerStatus != AMS_UNINITIALIZED, "tubes",
            "TeleManager::prepareAccountManager: yet another attempt");

    if (!mpAccountManager)
    {
        mpAccountManager = tp_account_manager_dup();
        SAL_WARN_IF( !mpAccountManager, "tubes", "TeleManager::prepareAccountManager: no account manager");
        if (!mpAccountManager)
            return;
        g_object_ref( mpAccountManager);
    }

    meAccountManagerStatus = AMS_INPREPARATION;
    setAccountManagerReadyHandlerInvoked( false);

    tp_proxy_prepare_async( mpAccountManager, NULL, TeleManager_AccountManagerReadyHandler, this);

    iterateLoop( &TeleManager::isAccountManagerReadyHandlerInvoked);
}


TpAccount* TeleManager::getMyAccount()
{
    INFO_LOGGER( "TeleManager::getMyAccount");

    MainLoopFlusher aFlusher( this);

    if (mpAccount)
        return mpAccount;

    SAL_WARN_IF( meAccountManagerStatus != AMS_PREPARED, "tubes",
            "TeleManager::getMyAccount: Account Manager not prepared");
    if (meAccountManagerStatus != AMS_PREPARED)
        return NULL;

    GList* pAccounts = tp_account_manager_get_valid_accounts( mpAccountManager);
    SAL_WARN_IF( !pAccounts, "tubes", "TeleManager::getMyAccount: no valid accounts");
    if (!pAccounts)
        return NULL;

    // Find our account to use.
    for (GList* pA = pAccounts; pA; pA = pA->next)
    {
        TpAccount* pAcc = TP_ACCOUNT( pA->data);
        const GHashTable* pPar = tp_account_get_parameters( pAcc);
        const gchar* pID = tp_asv_get_string( pPar, "account");
        SAL_WARN_IF( !pID, "tubes", "TeleManager::getMyAccount: account without account??");
        if (pID && maAccountID == pID)
        {
            mpAccount = pAcc;
            break;  // for
        }
    }

    g_list_free( pAccounts);
    SAL_WARN_IF( !mpAccount, "tubes", "TeleManager::getMyAccount: no account");
    if (!mpAccount)
        return NULL;

    g_object_ref( mpAccount);
    return mpAccount;
}


sal_uInt32 TeleManager::sendPacket( const TelePacket& rPacket ) const
{
    INFO_LOGGER( "TeleManager::sendPacket");

    MainLoopFlusher aFlusher( this);

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


void TeleManager::unregisterConference( TeleConferencePtr pConference )
{
    TeleConferenceVector::iterator it = ::std::find( maConferences.begin(), maConferences.end(), pConference);
    if (it != maConferences.end())
        maConferences.erase( it);
}


void TeleManager::disconnect()
{
    INFO_LOGGER( "TeleManager::disconnect");

    //! No MainLoopFlusher here!

    if (!mpClient)
        return;

    tp_base_client_unregister( mpClient);
    mpClient = NULL;

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


void TeleManager::acceptTube( TpChannel* pChannel, const char* pAddress )
{
    INFO_LOGGER( "TeleManager::acceptTube");

    SAL_INFO( "tubes", "TeleManager::acceptTube: address " << pAddress);

    MainLoopFlusher aFlusher( this);

    SAL_WARN_IF( !pChannel || !pAddress, "tubes", "TeleManager::acceptTube: no channel or no address");
    if (!pChannel || !pAddress)
        return;

    TeleConferencePtr pConference( new TeleConference( this, pChannel, ""));
    maConferences.push_back( pConference);
    pConference->acceptTube( pAddress);
}


void TeleManager::setAccountManagerReady( bool bPrepared)
{
    meAccountManagerStatus = (bPrepared ? AMS_PREPARED : AMS_UNPREPARABLE);
}


rtl::OString TeleManager::getFullServiceName() const
{
    OStringBuffer aBuf(64);
    aBuf.append( RTL_CONSTASCII_STRINGPARAM( TP_CLIENT_BUS_NAME_BASE)).append( maService);
    return aBuf.makeStringAndClear();
}


rtl::OString TeleManager::getFullObjectPath() const
{
    OStringBuffer aBuf(64);
    aBuf.append( RTL_CONSTASCII_STRINGPARAM( TP_CLIENT_OBJECT_PATH_BASE)).append( maService);
    return aBuf.makeStringAndClear();
}


void TeleManager::iterateLoop()
{
    GMainContext* pContext = (mpLoop ? g_main_loop_get_context( mpLoop) : NULL);
    SAL_INFO( "tubes.loop", "TeleManager::iterateLoop: once");
    g_main_context_iteration( pContext, TRUE);
}


void TeleManager::iterateLoop( CallBackInvokedFunc pFunc )
{
    GMainContext* pContext = (mpLoop ? g_main_loop_get_context( mpLoop) : NULL);
    while (!(*pFunc)())
    {
        SAL_INFO( "tubes.loop", "TeleManager::iterateLoop: CallBackInvokedFunc");
        g_main_context_iteration( pContext, TRUE);
    }
}


void TeleManager::iterateLoop( ManagerCallBackInvokedFunc pFunc )
{
    GMainContext* pContext = (mpLoop ? g_main_loop_get_context( mpLoop) : NULL);
    while (!(this->*pFunc)())
    {
        SAL_INFO( "tubes.loop", "TeleManager::iterateLoop: ManagerCallBackInvokedFunc");
        g_main_context_iteration( pContext, TRUE);
    }
}


void TeleManager::iterateLoop( const TeleConference* pConference, ConferenceCallBackInvokedFunc pFunc )
{
    GMainContext* pContext = (mpLoop ? g_main_loop_get_context( mpLoop) : NULL);
    while (!(pConference->*pFunc)())
    {
        SAL_INFO( "tubes.loop", "TeleManager::iterateLoop: ConferenceCallBackInvokedFunc");
        g_main_context_iteration( pContext, TRUE);
    }
}


void TeleManager::flushLoop() const
{
    if (mpLoop)
    {
        GMainContext* pContext = g_main_loop_get_context( mpLoop);
        while (g_main_context_iteration( pContext, FALSE))
        {
            SAL_INFO( "tubes.loop", "TeleManager::flushLoop");
        }
    }
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
