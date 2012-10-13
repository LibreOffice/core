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

#include <tubes/manager.hxx>

#include <tubes/collaboration.hxx>
#include <tubes/conference.hxx>
#include <tubes/constants.h>
#include <tubes/file-transfer-helper.h>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/uuid.h>

#include <telepathy-glib/telepathy-glib.h>
#include <stdio.h>
#include <map>
#include <set>

namespace css = ::com::sun::star;

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
#else
#define INFO_LOGGER_F(s)
#endif // SAL_LOG_INFO


using namespace rtl;
using namespace osl;

/** Refcounted singleton implementation class. */
class TeleManagerImpl
{
public:
    TpSimpleClientFactory*              mpFactory;
    TpBaseClient*                       mpClient;
    TpBaseClient*                       mpFileTransferClient;
    TpAccountManager*                   mpAccountManager;
    static bool                         mbAccountManagerReady;
    static bool                         mbAccountManagerReadyHandlerInvoked;
    static bool                         mbChannelReadyHandlerInvoked;
    OString                             msCurrentUUID;
    OString                             msNameSuffix;
    typedef std::map< OString, TeleConference* > MapStringConference;
    MapStringConference                 maAcceptedConferences;
    typedef std::set< TeleConference* > DemoConferences;
    DemoConferences                     maDemoConferences;
    typedef std::set< Collaboration* >  Collaborations;
    Collaborations                      maCollaborations;
    typedef std::set< TpContact* >      RegisteredContacts;
    RegisteredContacts                  maRegisteredContacts;

                            TeleManagerImpl();
                            ~TeleManagerImpl();
    static void AccountManagerReadyHandler( GObject* pSourceObject, GAsyncResult* pResult, gpointer pUserData );
    static void ChannelReadyHandler( GObject* pSourceObject, GAsyncResult* pResult, gpointer pUserData );
};

TeleManagerImpl* TeleManager::pImpl = new TeleManagerImpl();
bool TeleManagerImpl::mbAccountManagerReady;
bool TeleManagerImpl::mbAccountManagerReadyHandlerInvoked;
bool TeleManagerImpl::mbChannelReadyHandlerInvoked;

static void TeleManager_DBusChannelHandler(
        TpSimpleHandler*            /*handler*/,
        TpAccount*                  pAccount,
        TpConnection*               /*connection*/,
        GList*                      pChannels,
        GList*                      /*requests_satisfied*/,
        gint64                      /*user_action_time*/,
        TpHandleChannelsContext*    pContext,
        gpointer                    /*pUserData*/ )
{
    bool aAccepted = false;
    INFO_LOGGER_F( "TeleManager_DBusChannelHandler");

    for (GList* p = pChannels; p; p = p->next)
    {
        TpChannel* pChannel = TP_CHANNEL(p->data);
        if (!pChannel)
            continue;

        SAL_INFO( "tubes", "TeleManager_DBusChannelHandler: incoming dbus channel: "
                << tp_channel_get_identifier( pChannel));

        if (TP_IS_DBUS_TUBE_CHANNEL( pChannel))
        {
            SAL_INFO( "tubes", "accepting");
            aAccepted = true;

            TeleConference* pConference = new TeleConference( pAccount, TP_DBUS_TUBE_CHANNEL( pChannel ) );
            pConference->acceptTube();
            TeleManager::addConference( pConference );
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

void TeleManager::addConference( TeleConference* pConference )
{
    MutexGuard aGuard( GetMutex());

    SAL_WARN_IF( pConference->getUuid().isEmpty(), "tubes",
            "Adding conference with empty UUID should not happen!" );
    pImpl->maAcceptedConferences[ pConference->getUuid() ] = pConference;
}

TeleConference* TeleManager::getConference()
{
    MutexGuard aGuard( GetMutex());

    TeleManagerImpl::MapStringConference::const_iterator it =
            pImpl->maAcceptedConferences.find( pImpl->msCurrentUUID );
    TeleConference* pConference = NULL;
    if (it != pImpl->maAcceptedConferences.end())
        pConference = it->second;
    SAL_WARN_IF( !pConference, "tubes", "TeleManager::getConference: "
            << pImpl->msCurrentUUID.getStr() << " not found!" );
    pImpl->msCurrentUUID = OString();
    return pConference;
}

void TeleManager::registerCollaboration( Collaboration* pCollaboration )
{
    MutexGuard aGuard( GetMutex());

    pImpl->maCollaborations.insert( pCollaboration );
}

void TeleManager::unregisterCollaboration( Collaboration* pCollaboration )
{
    MutexGuard aGuard( GetMutex());

    pImpl->maCollaborations.erase( pCollaboration );
}

bool TeleManager::existsCollaboration( Collaboration* pCollaboration )
{
    MutexGuard aGuard( GetMutex());

    return pImpl->maCollaborations.find( pCollaboration ) != pImpl->maCollaborations.end();
}

void TeleManager::displayAllContacts()
{
    MutexGuard aGuard( GetMutex());

    for (TeleManagerImpl::Collaborations::iterator it = pImpl->maCollaborations.begin();
            it != pImpl->maCollaborations.end(); ++it)
        (*it)->DisplayContacts();
}

void TeleManager::registerDemoConference( TeleConference* pConference )
{
    MutexGuard aGuard( GetMutex());

    pImpl->maDemoConferences.insert( pConference );
}

void TeleManager::unregisterDemoConference( TeleConference* pConference )
{
    MutexGuard aGuard( GetMutex());

    pImpl->maDemoConferences.erase( pConference );
}

void TeleManager::broadcastPacket( const OString& rPacket )
{
    MutexGuard aGuard( GetMutex());

    INFO_LOGGER_F( "TeleManager::broadcastPacket" );
    for (TeleManagerImpl::DemoConferences::iterator it = pImpl->maDemoConferences.begin();
            it != pImpl->maDemoConferences.end(); ++it)
        if ((*it)->getCollaboration())
            (*it)->getCollaboration()->PacketReceived( rPacket );
}

bool TeleManager::hasWaitingConference()
{
    MutexGuard aGuard( GetMutex());

    return !pImpl->msCurrentUUID.isEmpty();
}

void TeleManager::setCurrentUuid( const OString& rUuid )
{
    MutexGuard aGuard( GetMutex());

    pImpl->msCurrentUUID = rUuid;
}

// FIXME: should be static and not used in conference.cxx
void TeleManager_fileReceived( const OUString& rStr, const OString& rUuid )
{
    SAL_INFO( "tubes", "TeleManager_fileReceived: incoming file: " << rStr );

    OString sUuid( rUuid );
    if (sUuid == "demo")
    {
        sUuid = TeleManager::createUuid();
        TeleConference* pConference = new TeleConference( NULL, NULL, sUuid );
        TeleManager::addConference( pConference );
        TeleManager::registerDemoConference( pConference );
    }
    TeleManager::setCurrentUuid( sUuid );

    try
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > rFactory =
            comphelper::getProcessServiceFactory();

        css::uno::Reference < css::frame::XComponentLoader > xLoader(
                ::comphelper::getProcessServiceFactory()->createInstance(
                        "com.sun.star.frame.Desktop" ),
                        css::uno::UNO_QUERY_THROW );
        css::uno::Sequence < css::beans::PropertyValue > args(0);
        css::uno::Reference < css::util::XCloseable > xDoc(
                xLoader->loadComponentFromURL( rStr, "_blank", 0, args ),
                css::uno::UNO_QUERY_THROW );
    }
    catch ( const css::uno::Exception& e )
    {
        // Expected to happen for unit test
        SAL_WARN( "tubes", "TeleManager_fileReceived: exception when loading: " << e.Message );
    }
}

static void TeleManager_TransferDone( EmpathyFTHandler *handler, TpFileTransferChannel *, gpointer )
{
    SAL_INFO( "tubes", "TeleManager_TransferDone: hooray!");
    GFile *gfile = empathy_ft_handler_get_gfile( handler);
    char *uri = g_file_get_uri( gfile);
    rtl::OUString aUri( OUString::createFromAscii( uri ) );
    g_free( uri);

    TeleManager_fileReceived( aUri, empathy_ft_handler_get_description( handler ) );

    g_object_unref( handler);
}

static void TeleManager_TransferError( EmpathyFTHandler *handler, const GError *error, void*)
{
    SAL_INFO( "tubes", "TeleManager_TransferError: " << error->message);

    g_object_unref( handler);
}

static void lcl_IncomingHandlerReady (
    EmpathyFTHandler*   pHandler,
    GError*             pError,
    void*               /*pUserData*/ )
{
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

    g_signal_connect( pHandler, "transfer-done", G_CALLBACK (TeleManager_TransferDone), NULL);
    g_signal_connect( pHandler, "transfer-error", G_CALLBACK (TeleManager_TransferError), NULL);
    SAL_INFO ("tubes", "lcl_IncomingHandlerReady: starting file transfer..");
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
        gpointer                    /*pUserData*/ )
{
    bool aAccepted = false;
    INFO_LOGGER_F( "TeleManager_FileTransferHandler");

    for (GList* p = pChannels; p; p = p->next)
    {
        TpChannel* pChannel = TP_CHANNEL(p->data);

        SAL_INFO( "tubes", "TeleManager_FileTransferHandler: incoming dbus channel: "
                << tp_channel_get_identifier( pChannel));

        if (TP_IS_FILE_TRANSFER_CHANNEL( pChannel))
        {
            SAL_INFO( "tubes", "accepting file transfer");
            empathy_ft_handler_new_incoming( TP_FILE_TRANSFER_CHANNEL( pChannel),
                lcl_IncomingHandlerReady, NULL);
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

void TeleManagerImpl::ChannelReadyHandler(
        GObject*        pSourceObject,
        GAsyncResult*   pResult,
        gpointer        pUserData
        )
{
    INFO_LOGGER_F( "TeleManagerImpl::ChannelReadyHandler");

    TeleConference* pConference = reinterpret_cast<TeleConference*>(pUserData);
    SAL_WARN_IF( !pConference, "tubes", "TeleManagerImpl::ChannelReadyHandler: no conference");
    if (!pConference)
        return;

    mbChannelReadyHandlerInvoked = true;

    TpAccountChannelRequest* pChannelRequest = TP_ACCOUNT_CHANNEL_REQUEST( pSourceObject);
    GError* pError = NULL;
    TpChannel * pChannel = tp_account_channel_request_create_and_handle_channel_finish(
            pChannelRequest, pResult, NULL, &pError);
    if (!pChannel)
    {
        // "account isn't Enabled" means just that..
        /* FIXME: detect and handle, domain=132, code=3 */
        SAL_WARN( "tubes", "TeleManagerImpl::ChannelReadyHandler: no channel: " << pError->message);
        g_error_free( pError);
        return;
    }
    pConference->setChannel( tp_account_channel_request_get_account( pChannelRequest),
            TP_DBUS_TUBE_CHANNEL (pChannel));
    pConference->offerTube();
}

void TeleManagerImpl::AccountManagerReadyHandler(
        GObject*        pSourceObject,
        GAsyncResult*   pResult,
        gpointer        /*pUserData*/
        )
{
    INFO_LOGGER_F( "TeleManagerImpl::AccountManagerReadyHandler");

    GError* pError = NULL;
    gboolean bPrepared = tp_proxy_prepare_finish( pSourceObject, pResult, &pError);
    SAL_WARN_IF( !bPrepared, "tubes", "TeleManagerImpl::AccountManagerReadyHandler: not prepared");
    if (!bPrepared || pError)
    {
        SAL_WARN_IF( pError, "tubes", "TeleManagerImpl::AccountManagerReadyHandler: error: " << pError->message);
        g_error_free( pError);
    }

    mbAccountManagerReady = bPrepared;
    mbAccountManagerReadyHandlerInvoked = true;
}

bool TeleManager::init( bool bListen )
{
    if (createAccountManager())
    {
        if (bListen && !registerClients())
            SAL_WARN( "tubes", "TeleManager::init: Could not register client handlers." );

        return true;
    }
    else
        SAL_WARN( "tubes", "TeleManager::init: Could not create AccountManager." );

    return false;
}

void TeleManager::finalize()
{
    delete pImpl;
}

bool TeleManager::createAccountManager()
{
    INFO_LOGGER_F( "TeleManager::createAccountManager");

    MutexGuard aGuard( GetMutex());

    SAL_INFO_IF( pImpl->mpAccountManager, "tubes", "TeleManager::createAccountManager: already connected");
    if (pImpl->mpAccountManager)
        return true;

    GError* pError = NULL;
    TpDBusDaemon *pDBus = tp_dbus_daemon_dup( &pError);
    SAL_WARN_IF( !pDBus, "tubes", "TeleManager::createAccountManager: no dbus daemon");
    if (!pDBus || pError)
    {
        SAL_WARN_IF( pError, "tubes", "TeleManager::createAccountManager: dbus daemon error: " << pError->message);
        g_error_free( pError);
        return false;
    }

    pImpl->mpFactory = TP_SIMPLE_CLIENT_FACTORY( tp_automatic_client_factory_new( pDBus));
    g_object_unref( pDBus);
    SAL_WARN_IF( !pImpl->mpFactory, "tubes", "TeleManager::createAccountManager: no client factory");
    if (!pImpl->mpFactory)
        return false;

    /* Tell the client factory (which creates and prepares proxy objects) to
     * get the features we need ready before giving us any objects.
     */
    /* We need every online account's connection object to be available... */
    tp_simple_client_factory_add_account_features_varargs (pImpl->mpFactory,
        TP_ACCOUNT_FEATURE_CONNECTION,
        0);
    /* ...and we want those connection objects to have the contact list
     * available... */
    tp_simple_client_factory_add_connection_features_varargs (pImpl->mpFactory,
        TP_CONNECTION_FEATURE_CONTACT_LIST,
        0);
    /* ...and those contacts should have their alias and their capabilities
     * available.
     */
    tp_simple_client_factory_add_contact_features_varargs (pImpl->mpFactory,
        TP_CONTACT_FEATURE_ALIAS,
        TP_CONTACT_FEATURE_AVATAR_DATA,
        TP_CONTACT_FEATURE_CAPABILITIES,
        TP_CONTACT_FEATURE_PRESENCE,
        TP_CONTACT_FEATURE_INVALID);

    pImpl->mpAccountManager = tp_account_manager_new_with_factory (pImpl->mpFactory);
    tp_account_manager_set_default (pImpl->mpAccountManager);

    pImpl->mbAccountManagerReadyHandlerInvoked = false;
    tp_proxy_prepare_async( pImpl->mpAccountManager, NULL, TeleManagerImpl::AccountManagerReadyHandler, NULL);
    while (!pImpl->mbAccountManagerReadyHandlerInvoked)
        g_main_context_iteration( NULL, TRUE);

    return pImpl->mbAccountManagerReady;
}

bool TeleManager::registerClients()
{
    INFO_LOGGER_F( "TeleManager::registerClients");

    MutexGuard aGuard( GetMutex());

    /* TODO: also check whether client could be registered and retry if not? */
    SAL_INFO_IF( pImpl->mpClient && pImpl->mpFileTransferClient, "tubes", "TeleManager::registerClients: already registered");
    if (pImpl->mpClient && pImpl->mpFileTransferClient)
        return true;

    pImpl->mpClient = tp_simple_handler_new_with_factory(
            pImpl->mpFactory,               // factory
            FALSE,                          // bypass_approval
            FALSE,                          // requests
            getFullClientName().getStr(),   // name
            FALSE,                          // uniquify
            TeleManager_DBusChannelHandler, // callback
            NULL,                           // user_data
            NULL                            // destroy
            );
    SAL_WARN_IF( !pImpl->mpClient, "tubes", "TeleManager::registerClients: no client");
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

    // Setup client handler for MUC channels with our service.
    tp_base_client_take_handler_filter( pImpl->mpClient,
            tp_asv_new(
                TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
                TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT, TP_HANDLE_TYPE_ROOM,
                TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, getFullServiceName().getStr(),
                NULL));

    GError* pError = NULL;
    if (!tp_base_client_register( pImpl->mpClient, &pError))
    {
        SAL_WARN( "tubes", "TeleManager::registerClients: error registering client handler: " << pError->message);
        g_error_free( pError);
        return false;
    }

    SAL_INFO( "tubes", "TeleManager::registerClients: bus name: " << tp_base_client_get_bus_name( pImpl->mpClient));
    SAL_INFO( "tubes", "TeleManager::registerClients: object path: " << tp_base_client_get_object_path( pImpl->mpClient));

    /* Register a second "head" for incoming file transfers. This uses a more
     * specific filter than Empathy's handler by matching on the file
     * transfer's ServiceName property, and uses bypass_approval to ensure the
     * user isn't prompted before the channel gets passed to us.
     */
    pImpl->mpFileTransferClient = tp_simple_handler_new_with_factory (
            pImpl->mpFactory,                               // factory
            TRUE,                                           // bypass_approval
            FALSE,                                          // requests
            getFullClientName().getStr(),                   // name
            TRUE,                                           // uniquify to get a different bus name to the main client, above
            TeleManager_FileTransferHandler,                // callback
            NULL,                                           // user_data
            NULL                                            // destroy
            );
    tp_base_client_take_handler_filter( pImpl->mpFileTransferClient,
            tp_asv_new(
                TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_FILE_TRANSFER,
                TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT, TP_HANDLE_TYPE_CONTACT,
                TP_PROP_CHANNEL_INTERFACE_FILE_TRANSFER_METADATA_SERVICE_NAME, G_TYPE_STRING, getFullServiceName().getStr(),
                NULL));

    if (!tp_base_client_register( pImpl->mpFileTransferClient, &pError))
    {
        /* This shouldn't fail if registering the main handler succeeded */
        SAL_WARN( "tubes", "TeleManager::registerClients: error registering file transfer handler: " << pError->message);
        g_error_free( pError);
        return false;
    }

    return true;
}

TeleConference* TeleManager::startDemoSession()
{
    INFO_LOGGER_F( "TeleManager::startDemoSession");

    TeleConference* pConference = new TeleConference( NULL, NULL, "demo" );
    registerDemoConference( pConference );

    return pConference;
}

/* TODO: factor out common code with startBuddySession() */
TeleConference* TeleManager::startGroupSession( TpAccount *pAccount,
                                     const rtl::OUString& rUConferenceRoom,
                                     const rtl::OUString& rUConferenceServer )
{
    INFO_LOGGER_F( "TeleManager::startGroupSession");

    OString aSessionId( TeleManager::createUuid());

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

    SAL_INFO( "tubes", "TeleManager::startGroupSession: creating channel request from "
            << tp_account_get_path_suffix( pAccount ) << " to " << aTarget.getStr() );

    // MUC request
    GHashTable* pRequest = tp_asv_new(
            TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
            TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, TP_TYPE_HANDLE, TP_HANDLE_TYPE_ROOM,
            TP_PROP_CHANNEL_TARGET_ID, G_TYPE_STRING, aTarget.getStr(),
            TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, getFullServiceName().getStr(),
            NULL);

    TpAccountChannelRequest * pChannelRequest = tp_account_channel_request_new(
            pAccount, pRequest, TP_USER_ACTION_TIME_NOT_USER_ACTION);
    SAL_WARN_IF( !pChannelRequest, "tubes", "TeleManager::startGroupSession: no channel");
    if (!pChannelRequest)
    {
        g_hash_table_unref( pRequest);
        return NULL;
    }

    pImpl->mbChannelReadyHandlerInvoked = false;

    TeleConference* pConference = new TeleConference( NULL, NULL, aSessionId );

    tp_account_channel_request_create_and_handle_channel_async(
            pChannelRequest, NULL, TeleManagerImpl::ChannelReadyHandler, pConference);

    while (!pImpl->mbChannelReadyHandlerInvoked)
        g_main_context_iteration( NULL, TRUE );

    g_object_unref( pChannelRequest);
    g_hash_table_unref( pRequest);

    if (!pConference->isReady())
        return NULL;

    return pConference;
}


static void lcl_ensureLegacyChannel( TpAccount* pAccount, TpContact* pBuddy )
{
    /* This is a workaround for a Telepathy bug.
     * <https://bugs.freedesktop.org/show_bug.cgi?id=47760>. The first time you
     * request a tube to a contact on an account, you actually get two channels
     * back: the tube you asked for, along with a legacy Channel.Type.Tubes
     * object. This breaks create_and_handle_channel_async(), which expects to
     * only get one channel back.
     *
     * To work around this, we make sure the legacy Tubes channel already
     * exists before we request the channel we actually want. We don't actually
     * have to wait for this request to succeed—we fire it off and forget about
     * it.
     */
    GHashTable* pRequest = tp_asv_new(
            TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_TUBES,
            TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, TP_TYPE_HANDLE, TP_HANDLE_TYPE_CONTACT,
            TP_PROP_CHANNEL_TARGET_ID, G_TYPE_STRING, tp_contact_get_identifier (pBuddy),
            NULL);
    TpAccountChannelRequest* pChannelRequest = tp_account_channel_request_new(
            pAccount, pRequest, TP_USER_ACTION_TIME_NOT_USER_ACTION);
    tp_account_channel_request_ensure_channel_async( pChannelRequest, NULL,
            NULL, NULL, NULL );
    g_object_unref( pChannelRequest );
    g_hash_table_unref( pRequest );
}


/* TODO: factor out common code with startGroupSession() */
TeleConference* TeleManager::startBuddySession( TpAccount *pAccount, TpContact *pBuddy )
{
    INFO_LOGGER_F( "TeleManager::startBuddySession");

    lcl_ensureLegacyChannel( pAccount, pBuddy );

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
        return NULL;
    }

    pImpl->mbChannelReadyHandlerInvoked = false;

    TeleConference* pConference = new TeleConference( NULL, NULL, createUuid(), true );

    tp_account_channel_request_create_and_handle_channel_async(
            pChannelRequest, NULL, TeleManagerImpl::ChannelReadyHandler, pConference );

    while (!pImpl->mbChannelReadyHandlerInvoked)
        g_main_context_iteration( NULL, TRUE );

    g_object_unref( pChannelRequest);
    g_hash_table_unref( pRequest);

    if (!pConference->isReady())
        return NULL;

    return pConference;
}

static bool tb_presence_is_online( const TpConnectionPresenceType& presence )
{
    switch (presence)
    {
        case TP_CONNECTION_PRESENCE_TYPE_UNSET:
        case TP_CONNECTION_PRESENCE_TYPE_OFFLINE:
            return false;
        case TP_CONNECTION_PRESENCE_TYPE_AVAILABLE:
        case TP_CONNECTION_PRESENCE_TYPE_AWAY:
        case TP_CONNECTION_PRESENCE_TYPE_EXTENDED_AWAY:
        case TP_CONNECTION_PRESENCE_TYPE_HIDDEN:
        case TP_CONNECTION_PRESENCE_TYPE_BUSY:
            return true;
        case TP_CONNECTION_PRESENCE_TYPE_UNKNOWN:
        case TP_CONNECTION_PRESENCE_TYPE_ERROR:
        default:
            return false;
    }
}

static bool tb_contact_is_online( TpContact *contact )
{
    return tb_presence_is_online (tp_contact_get_presence_type (contact));
}

static void presence_changed_cb( TpContact* /* contact */,
                                 guint      /* type */,
                                 gchar*     /* status */,
                                 gchar*     /* message */,
                                 gpointer   /* pContactList*/ )
{
    TeleManager::displayAllContacts();
}

AccountContactPairV TeleManager::getContacts()
{
    AccountContactPairV pairs;

    for (GList *accounts = tp_account_manager_get_valid_accounts (pImpl->mpAccountManager);
            accounts != NULL; accounts = g_list_delete_link (accounts, accounts))
    {
        TpAccount *account = reinterpret_cast<TpAccount *>(accounts->data);
        TpConnection *connection = tp_account_get_connection (account);

        /* Verify account is online and received its contact list. If state is not
         * SUCCESS this means we didn't received the roster from server yet and
         * we would have to wait for the "notify:contact-list-state" signal. */
        if (connection == NULL || tp_connection_get_contact_list_state (connection) !=
                TP_CONTACT_LIST_STATE_SUCCESS)
            continue;

        TpContact *self = tp_connection_get_self_contact (connection);
        GPtrArray *contacts = tp_connection_dup_contact_list (connection);
        for (guint i = 0; i < contacts->len; i++)
        {
            TpContact *contact = reinterpret_cast<TpContact *>(g_ptr_array_index (contacts, i));
            if (pImpl->maRegisteredContacts.find (contact) == pImpl->maRegisteredContacts.end())
            {
                pImpl->maRegisteredContacts.insert (contact);
                g_signal_connect (contact, "presence-changed",
                        G_CALLBACK (presence_changed_cb), NULL );
            }
            if (contact != self && tb_contact_is_online (contact))
            {
                g_object_ref (account);
                g_object_ref (contact);

                AccountContactPair pair(account, contact);
                pairs.push_back(pair);
            }
        }
        g_ptr_array_unref (contacts);
    }

    return pairs;
}

rtl::OString TeleManager::getFullClientName()
{
    OStringBuffer aBuf(64);
    aBuf.append( RTL_CONSTASCII_STRINGPARAM( LIBO_CLIENT_SUFFIX)).append( pImpl->msNameSuffix);
    return aBuf.makeStringAndClear();
}

rtl::OString TeleManager::getFullServiceName()
{
    OStringBuffer aBuf(64);
    aBuf.append( RTL_CONSTASCII_STRINGPARAM( LIBO_DTUBE_SERVICE)).append( pImpl->msNameSuffix);
    return aBuf.makeStringAndClear();
}

rtl::OString TeleManager::getFullObjectPath()
{
    OStringBuffer aBuf(64);
    aBuf.append( '/').append( RTL_CONSTASCII_STRINGPARAM( LIBO_DTUBE_SERVICE)).append( pImpl->msNameSuffix);
    OString aStr( aBuf.makeStringAndClear().replace( '.', '/'));
    return aStr;
}

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

void TeleManager::addSuffixToNames( const char* pName )
{
    pImpl->msNameSuffix = pName;
}

// ===========================================================================

TeleManagerImpl::TeleManagerImpl()
    :
        mpFactory( NULL),
        mpClient( NULL),
        mpFileTransferClient( NULL),
        mpAccountManager( NULL)
{
    g_type_init();
}

TeleManagerImpl::~TeleManagerImpl()
{
    // There may be unused conferences left opened, so close them.
    // It should not make a problem to close already closed conference.
    for (MapStringConference::iterator it = maAcceptedConferences.begin();
            it != maAcceptedConferences.end(); ++it)
        it->second->close();
    if (mpClient)
    {
        tp_base_client_unregister( mpClient);
        g_object_unref( mpClient);
    }
    if (mpFileTransferClient)
    {
        tp_base_client_unregister( mpFileTransferClient);
        g_object_unref( mpFileTransferClient);
    }
    if (mpFactory)
        g_object_unref( mpFactory);
    if (mpAccountManager)
        g_object_unref( mpAccountManager);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
