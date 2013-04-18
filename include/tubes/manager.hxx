/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TUBES_MANAGER_HXX
#define INCLUDED_TUBES_MANAGER_HXX

#include <sal/config.h>
#include <tubes/tubesdllapi.h>
#include <rtl/ustring.hxx>

#include <utility>
#include <vector>

// For testing purposes, we might need more in future.
#define LIBO_TUBES_DBUS_INTERFACE "org.libreoffice.calc"
#define LIBO_TUBES_DBUS_MSG_METHOD "LibOMsg"
#define LIBO_TUBES_DBUS_PATH "/org/libreoffice/calc"

namespace osl { class Mutex; }
class Collaboration;
class TeleConference;
class TeleManagerImpl;
typedef struct _TpAccount TpAccount;
typedef struct _TpContact TpContact;

typedef ::std::pair< TpAccount *, TpContact * > AccountContactPair;
typedef ::std::vector< AccountContactPair > AccountContactPairV;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
/** Interface to Telepathy DBus Tubes.

    Fragile, not working yet.

    Accounts need to have been setup within Empathy already.
 */

class TeleManager
{
public:
    /** Prepare tube manager with account and service to be offered/listened
        to.
     */
    TUBES_DLLPUBLIC static bool             init( bool bListen );

    TUBES_DLLPUBLIC static void             finalize();

    /** True if there has been tube channel received and is still not used. */
    TUBES_DLLPUBLIC static bool             hasWaitingConference();

    /** Get a conference with current UUID to set a session. */
    TUBES_DLLPUBLIC static TeleConference*  getConference();

    /** Connect to DBus, create and prepare the Telepathy Account Manager. */
    static bool             createAccountManager();

    /** Setup client handlers. */
    static bool             registerClients();

    /** Fetches the contact list.
        Is non-functional until createAccountManager().
     */
    // exported for unit test
    TUBES_DLLPUBLIC static AccountContactPairV getContacts();

    /** Start a demo session where all local documents are shared to each other */
    static TeleConference*  startDemoSession();

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
    static TeleConference*  startGroupSession( TpAccount *pAccount,
                                                const OUString& rConferenceRoom,
                                                const OUString& rConferenceServer );

    /** Start a session with a buddy.

        @param pAccount
            The account to use. This must be a valid Jabber account.

        @param pBuddy
            The buddy to be connected. Must be a contact of pAccount.
     */
    // exported for unit test
    TUBES_DLLPUBLIC static TeleConference* startBuddySession( TpAccount *pAccount, TpContact *pBuddy );

    static void             registerCollaboration( Collaboration* pCollaboration );
    static void             unregisterCollaboration( Collaboration* pCollaboration );
    /** Used to determine whether we are closing the channel by ourselves.
     * @return true if the Collaboration is still registered  */
    static bool             existsCollaboration( Collaboration* pCollaboration );
    /** Display contact list dialog for all documents. */
    static void             displayAllContacts();

    static void             registerDemoConference( TeleConference* pConference );
    static void             unregisterDemoConference( TeleConference* pConference );
    /** Broadcast packet to all conferences. Used for demo mode. */
    static void             broadcastPacket( const OString& rPacket );


    // Only for callbacks.
    static void             addConference( TeleConference* pConference );
    static OString     createUuid();
    /** @param rUuid
            is stored so that accepted conference with this UUID could be
            then retrieved by getConference() when loading new document
    */
    static void             setCurrentUuid( const OString& rUuid );

    /// "LibreOfficeWhatEver"
    static OString     getFullClientName();

    /// "org.libreoffice.calcWhatEver"
    static OString     getFullServiceName();

    /// "/org/libreoffice/calcWhatEver"
    static OString     getFullObjectPath();

    /** Add a suffix to the client name and DBus tube names, e.g. "WhatEver"

        Normally the client name is LibreOffice and the DBus tube service name
        is something like org.libreoffice.calc, this modifies the names to
        "LibreOffice"+pName and "org.libreoffice.calc"+pName to make tests not
        interfere with the real world. This is not to be used otherwise. If
        used it must be called before the first TeleManager is instanciated and
        connects.
     */
    // exported for unit test
    TUBES_DLLPUBLIC static void addSuffixToNames( const char* pName );

private:
    static TeleManagerImpl* pImpl;

    static ::osl::Mutex&    GetMutex();
};

#endif // INCLUDED_TUBES_MANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
