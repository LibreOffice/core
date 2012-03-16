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

        @param rAccount
            The account (JID) to use. This must be a valid JID that has been
            setup with Empathy or another Telepathy client providing
            Jabber/XMPP.

        @param rService
            "WhatEver", is prepended with "...LibreOffice"

        @param bCreateOwnGMainLoop
            Whether to create and iterate an own GMainLoop. For testing
            purposes when no GMainLoop is available.
     */
    TeleManager( const rtl::OUString& rAccount, const rtl::OUString& rService, bool bCreateOwnGMainLoop = false );
    ~TeleManager();

    /** Prepare the Telepathy Account Manager.

        Invokes an async call that is not ready until meAccountManagerStatus is
        set! Until that is AMS_PREPARED nothing else will work.

        TODO: this needs some signalling mechanism
     */
    void                    prepareAccountManager();
    AccountManagerStatus    getAccountManagerStatus() const
    {
        return meAccountManagerStatus;
    }

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

    /** Start a session with a buddy.

        @param rBuddy
            The buddy to be connected.
     */
    bool                    startBuddySession( const rtl::OUString& rBuddy );

    void                    unregisterConference( TeleConferencePtr pConference );
    bool                    connect();
    void                    disconnect();
    void                    acceptTube( TpChannel* pChannel, const char* pAddress );

    /** Only the callback of prepareAccountManager() is to set this. */
    void                    setAccountManagerReady( bool bPrepared);

    /** Send data to all registered conferences. */
    bool                    sendPacket( const TelePacket& rPacket ) const;

    /// "org.freedesktop.Telepathy.Client.LibreOfficeWhatEver"
    rtl::OString            getFullServiceName() const;

    /// "/org/freedesktop/Telepathy/Client/LibreOfficeWhatEver"
    rtl::OString            getFullObjectPath() const;

    /// Only for use with MainLoopFlusher
    GMainLoop*              getMainLoop() const { return mpLoop; }

    GMainContext*           getMainContext() const { return (mpLoop ? g_main_loop_get_context( mpLoop) : NULL); }

    static rtl::OString     createUuid();


    // Only for callbacks.
    void                    setChannelReadyHandlerInvoked( bool b ) { mbChannelReadyHandlerInvoked = b; }
    bool                    isChannelReadyHandlerInvoked() const { return mbChannelReadyHandlerInvoked; }
    void                    setAccountManagerReadyHandlerInvoked( bool b ) { mbAccountManagerReadyHandlerInvoked = b; }
    bool                    isAccountManagerReadyHandlerInvoked() const { return mbAccountManagerReadyHandlerInvoked; }

    typedef bool (TeleManager::*CallBackInvokedFunc)() const;
    /** Iterate our GMainLoop, blocking, until the callback is done. */
    void                    iterateLoop( CallBackInvokedFunc pFunc );

    typedef bool (TeleConference::*ConferenceCallBackInvokedFunc)() const;
    /** Iterate our GMainLoop, blocking, until the callback is done. */
    void                    iterateLoop( const TeleConference* pConference, ConferenceCallBackInvokedFunc pFunc );

    /** Iterate our GMainLoop, non-blocking, until nothing pending. */
    void                    flushLoop() const;

private:

    rtl::OString            maAccountID;
    rtl::OString            maService;      // the "WhatEver" part
    TeleConferenceVector    maConferences;
    GMainLoop*              mpLoop;
    TpDBusDaemon*           mpDBus;
    TpAccountManager*       mpAccountManager;
    TpAccount*              mpAccount;
    TpConnection*           mpConnection;
    TpBaseClient*           mpClient;
    AccountManagerStatus    meAccountManagerStatus;

    bool                    mbChannelReadyHandlerInvoked : 1;
    bool                    mbAccountManagerReadyHandlerInvoked : 1;

    TpAccount*              getMyAccount();

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
