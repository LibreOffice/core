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
#include <rtl/ustring.hxx>
#include <salhelper/thread.hxx>
#include <rtl/ref.hxx>
#include <telepathy-glib/telepathy-glib.h>
#include <vector>

// For testing purposes, we might need more in future.
#define LIBO_TUBES_DBUS_MSG_METHOD "LibOMsg"

typedef ::std::vector<TeleConferencePtr> TeleConferenceVector;

namespace osl { class Mutex; }
class TeleManagerImpl;

/** Interface to Telepathy DBus Tubes.

    Fragile, not working yet.

    Accounts need to have been setup within Empathy already.
 */

/* FIXME: selective dllprivate methods */
class TUBES_DLLPUBLIC TeleManager
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

    /** Prepare the Telepathy Account Manager.

        Invokes an async call that is not ready until meAccountManagerStatus is
        set! Until that is AMS_PREPARED nothing else will work.

        TODO: this needs some signalling mechanism
     */
    void                    prepareAccountManager();
    AccountManagerStatus    getAccountManagerStatus() const;

#if 0
    /** Start a group session in a MUC.

        @param rConferenceRoom
            The MUC to be created/joined, e.g. "LibreOffice". If empty, the
            conference's UUID is used.

        @param rConferenceServer
            Server to create the MUC on, e.g. "conference.example.org". If
            empty, only the conference's UUID is used and rConferenceRoom is
            ignored, hopefully resulting in a local DBus tube.
     */
    bool                    startGroupSession( const rtl::OUString& rConferenceRoom,
                                               const rtl::OUString& rConferenceServer );
#endif

    /** Start a session with a buddy.

        @param pAccount
            The account to use. This must be a valid Jabber account.

        @param pBuddy
            The buddy to be connected. Must be a contact of rAccount.
     */
    bool                    startBuddySession( TpAccount *pAccount, TpContact *pBuddy);

    void                    unregisterConference( TeleConferencePtr pConference );

    /** Connect to DBus and setup client handler. */
    bool                    connect();

    void                    disconnect();

    void                    acceptTube( TpAccount* pAccount, TpChannel* pChannel, const char* pAddress );

    /** Send data to all registered conferences.

        @returns to how many conferences the packet was send
     */
    sal_uInt32              sendPacket( const TelePacket& rPacket ) const;

    /** Pop a received data packet.

        XXX This needs to be elaborated to pop from a specific conference, or
        for a specific document. Currently the conferences are simply iterated
        and the first non-empty queue is popped.

        @returns whether there was any packet to pop
     */
    bool                    popPacket( TelePacket& rPacket );

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

    /** Iterate our GMainLoop, non-blocking, until nothing pending. */
    void                    flushLoop() const;

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

    TeleConferenceVector    maConferences;

    bool                    mbChannelReadyHandlerInvoked : 1;

    static TeleManagerImpl* pImpl;
    static sal_uInt32       nRefCount;
    static rtl::OString     aNameSuffix;

    friend class TeleManagerImpl;   // access to mutex

    TUBES_DLLPRIVATE static ::osl::Mutex&   GetMutex();

};


/** The most ugly workaround for not having a GMainLoop running, i.e. in
    cppunittest.
 */
class MainLoopFlusher
{
public:
    explicit MainLoopFlusher( const TeleManager* pManager )
        :
            mpManager( pManager)
    {
        flush();
    }

    ~MainLoopFlusher()
    {
        flush();
    }

    void flush() const
    {
        mpManager->flushLoop();
    }

private:
    const TeleManager* mpManager;
};


#endif // INCLUDED_TUBES_MANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
