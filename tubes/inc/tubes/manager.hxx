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
#include <tubes/tubesdllapi.h>
#include <rtl/ustring.hxx>

// For testing purposes, we might need more in future.
#define LIBO_TUBES_DBUS_INTERFACE "org.libreoffice.calc"
#define LIBO_TUBES_DBUS_MSG_METHOD "LibOMsg"
#define LIBO_TUBES_DBUS_PATH "/org/libreoffice/calc"

namespace osl { class Mutex; }
class Collaboration;
class ContactList;
class TeleConference;
class TeleManagerImpl;
typedef struct _TpAccount TpAccount;
typedef struct _TpContact TpContact;

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
     */
    TUBES_DLLPUBLIC                         TeleManager();
    TUBES_DLLPUBLIC                         ~TeleManager();

    TUBES_DLLPUBLIC bool                    init( bool bListen );

    /** Connect to DBus and create AccountManager. */
    bool                    createAccountManager();

    /** Setup client handlers. */
    bool                    registerClients();

    /** Prepare the Telepathy Account Manager.
        Requires createAccountManager() to have succeeded.

        Invokes an async call that is not ready until meAccountManagerStatus is
        set! Until that is AMS_PREPARED nothing else will work.

        TODO: this needs some signalling mechanism
     */
    void                    prepareAccountManager();
    AccountManagerStatus    getAccountManagerStatus() const;

    /** Fetches the contact list. Returns 0 before connect() is called successfully.
        Is non-functional until prepareAccountManager().
     */
    ContactList*            getContactList() const;

    /** Start a demo session where all local documents are shared to each other */
    TeleConference*         startDemoSession();

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
    TeleConference*         startGroupSession( TpAccount *pAccount,
                                                const rtl::OUString& rConferenceRoom,
                                                const rtl::OUString& rConferenceServer );

    /** Start a session with a buddy.

        @param pAccount
            The account to use. This must be a valid Jabber account.

        @param pBuddy
            The buddy to be connected. Must be a contact of pAccount.
     */
    TeleConference*         startBuddySession( TpAccount *pAccount, TpContact *pBuddy );

    /** Get a conference with current UUID to set a session. */
    TUBES_DLLPUBLIC static TeleConference*  getConference();

    static void                             registerCollaboration( Collaboration* pCollaboration );
    static void                             unregisterCollaboration( Collaboration* pCollaboration );
    /** Display contact list dialog for all documents. */
    static void                             displayAllContacts();

    static void                             registerDemoConference( TeleConference* pConference );
    static void                             unregisterDemoConference( TeleConference* pConference );
    /** Broadcast packet to all conferences. Used for demo mode. */
    static void                             broadcastPacket( const OString& rPacket );

    /** True if there has been tube channel received and is still not used. */
    TUBES_DLLPUBLIC static bool             hasWaitingConference();
    static void                             setCurrentUuid( const OString& rUuid );

    void                    disconnect();

    static rtl::OString     createUuid();


    // Only for callbacks.
    static void             addConference( TeleConference* pConference );
    static void             setChannelReadyHandlerInvoked( bool b );
    bool                    isChannelReadyHandlerInvoked() const;
    void                    setAccountManagerReadyHandlerInvoked( bool b );
    bool                    isAccountManagerReadyHandlerInvoked() const;

    /** Only the callback of prepareAccountManager() is to set this. */
    void                    setAccountManagerReady( bool bPrepared);

    typedef bool (*CallBackInvokedFunc)();
    /** Iterate our GMainLoop, blocking, until the callback is done. */
    void                    iterateLoop( CallBackInvokedFunc pFunc );

    typedef bool (TeleManager::*ManagerCallBackInvokedFunc)() const;
    /** Iterate our GMainLoop, blocking, until the callback is done. */
    void                    iterateLoop( ManagerCallBackInvokedFunc pFunc );

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

private:
    void                    ensureLegacyChannel( TpAccount* pAccount, TpContact* pBuddy );

    static TeleManagerImpl* pImpl;
    static sal_uInt32       nRefCount;
    static rtl::OString     aNameSuffix;

    static ::osl::Mutex&    GetMutex();
};


#endif // INCLUDED_TUBES_MANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
