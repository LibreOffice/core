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

#ifndef INCLUDED_TUBES_MANAGER_HXX
#define INCLUDED_TUBES_MANAGER_HXX

#include <sal/config.h>
#include "tubes/tubesdllapi.h"
#include "tubes/conference.hxx"
#include "tubes/packet.hxx"
#include "tubes/contact-list.hxx"
#include <rtl/ustring.hxx>
#include <salhelper/thread.hxx>
#include <rtl/ref.hxx>
#include <tools/link.hxx>
#include <telepathy-glib/telepathy-glib.h>

// For testing purposes, we might need more in future.
#define LIBO_TUBES_DBUS_INTERFACE "org.libreoffice.calc"
#define LIBO_TUBES_DBUS_MSG_METHOD "LibOMsg"
#define LIBO_TUBES_DBUS_PATH "/org/libreoffice/calc"

namespace osl { class Mutex; }
class TeleManagerImpl;

/** Interface to Telepathy DBus Tubes.

    Fragile, not working yet.

    Accounts need to have been setup within Empathy already.
 */

class TeleManager
{
public:

    enum AccountManagerStatus
    {
        AMS_UNINITIALIZED = 0,
        AMS_INPREPARATION,
        AMS_UNPREPARABLE,
        AMS_PREPARED
    };

    /** Prepare tube manager with account and service to be offered/listened
        to.

        @param bCreateOwnGMainLoop
            Whether to create and iterate an own GMainLoop. For testing
            purposes when no GMainLoop is available.
     */
    TeleManager( bool bCreateOwnGMainLoop = false );
    ~TeleManager();

    TUBES_DLLPUBLIC static TeleManager*     get();
    TUBES_DLLPUBLIC void                    unref();

    /** Connect to DBus and create AccountManager. */
    TUBES_DLLPUBLIC bool                    createAccountManager();

    /** Setup client handlers. */
    TUBES_DLLPUBLIC bool                    registerClients();

    /** Prepare the Telepathy Account Manager.
        Requires createAccountManager() to have succeeded.

        Invokes an async call that is not ready until meAccountManagerStatus is
        set! Until that is AMS_PREPARED nothing else will work.

        TODO: this needs some signalling mechanism
     */
    TUBES_DLLPUBLIC void                    prepareAccountManager();
    TUBES_DLLPUBLIC AccountManagerStatus    getAccountManagerStatus() const;

    /** Fetches the contact list. Returns 0 before connect() is called successfully.
        Is non-functional until prepareAccountManager().
     */
    TUBES_DLLPUBLIC ContactList*            getContactList() const;

    /** Start a group session in a MUC.

        @param pAccount
            The account to use. This must be a valid Jabber account.

        @param rConferenceRoom
            The MUC to be created/joined, e.g. "LibreOffice". If empty, the
            conference's UUID is used.

        @param rConferenceServer
            Server to create the MUC on, e.g. "conference.example.org". If
            empty, only the conference's UUID is used and rConferenceRoom is
            ignored, hopefully resulting in a local DBus tube.
     */
    TUBES_DLLPUBLIC TeleConference*         startGroupSession( TpAccount *pAccount,
                                                const rtl::OUString& rConferenceRoom,
                                                const rtl::OUString& rConferenceServer );

    /** Start a session with a buddy.

        @param pAccount
            The account to use. This must be a valid Jabber account.

        @param pBuddy
            The buddy to be connected. Must be a contact of pAccount.
     */
    TUBES_DLLPUBLIC TeleConference*         startBuddySession( TpAccount *pAccount, TpContact *pBuddy );

    /** Get a conference with current UUID to set a session. */
    TUBES_DLLPUBLIC TeleConference*         getConference();

    /** True if there has been tube channel received and is still not used. */
    TUBES_DLLPUBLIC static bool             hasWaitingConference();

    void                    disconnect();

    /// Only for use with MainLoopFlusher
    GMainLoop*              getMainLoop() const;

    GMainContext*           getMainContext() const;

    static rtl::OString     createUuid();


    // Only for callbacks.
    void                    setChannelReadyHandlerInvoked( bool b ) { mbChannelReadyHandlerInvoked = b; }
    bool                    isChannelReadyHandlerInvoked() const { return mbChannelReadyHandlerInvoked; }
    void                    setAccountManagerReadyHandlerInvoked( bool b );
    bool                    isAccountManagerReadyHandlerInvoked() const;

    /** Only the callback of prepareAccountManager() is to set this. */
    void                    setAccountManagerReady( bool bPrepared);

    /** Iterate our GMainLoop, blocking, unconditionally. */
    void                    iterateLoop();

    typedef bool (*CallBackInvokedFunc)();
    /** Iterate our GMainLoop, blocking, until the callback is done. */
    void                    iterateLoop( CallBackInvokedFunc pFunc );

    typedef bool (TeleManager::*ManagerCallBackInvokedFunc)() const;
    /** Iterate our GMainLoop, blocking, until the callback is done. */
    void                    iterateLoop( ManagerCallBackInvokedFunc pFunc );

    typedef bool (TeleConference::*ConferenceCallBackInvokedFunc)() const;
    /** Iterate our GMainLoop, blocking, until the callback is done. */
    void                    iterateLoop( const TeleConference* pConference, ConferenceCallBackInvokedFunc pFunc );

    /// "LibreOfficeWhatEver"
    static rtl::OString     getFullClientName();

    /// "org.libreoffice.calcWhatEver"
    static rtl::OString     getFullServiceName();

    /// "/org/libreoffice/calcWhatEver"
    static rtl::OString     getFullObjectPath();

    /** Add a suffix to the client name and DBus tube names, e.g. "WhatEver"

        Normally the client name is LibreOffice and the DBus tube service name
        is something like org.libreoffice.calc, this modifies the names to
        "LibreOffice"+pName and "org.libreoffice.calc"+pName to make tests not
        interfere with the real world. This is not to be used otherwise. If
        used it must be called before the first TeleManager is instanciated and
        connects.
     */
    static void             addSuffixToNames( const char* pName );

    TpAccount*              getAccount( const rtl::OString& rAccountID );

/* Callbacks; not for use outside this class. */
    static void             TransferDone( EmpathyFTHandler *handler, TpFileTransferChannel *, gpointer user_data);

    static void             DBusChannelHandler(
        TpSimpleHandler*            /*handler*/,
        TpAccount*                  pAccount,
        TpConnection*               /*connection*/,
        GList*                      pChannels,
        GList*                      /*requests_satisfied*/,
        gint64                      /*user_action_time*/,
        TpHandleChannelsContext*    pContext,
        gpointer                    pUserData);

private:
    void                    addConference( TeleConference* );
    void                    ensureLegacyChannel( TpAccount* pAccount, TpContact* pBuddy );

    bool                    mbChannelReadyHandlerInvoked : 1;

    static TeleManagerImpl* pImpl;
    static sal_uInt32       nRefCount;
    static rtl::OString     aNameSuffix;

    /* FIXME: double-singletonning is bad. These two are used by ::get and
     * ::unref, and are a quick hack so that we can have a demo working.
     */
    static TeleManager*     pSingleton;
    static sal_uInt32       nAnotherRefCount;
    static ::osl::Mutex&    GetAnotherMutex();

    friend class TeleManagerImpl;   // access to mutex

    static ::osl::Mutex&    GetMutex();
};


#endif // INCLUDED_TUBES_MANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
