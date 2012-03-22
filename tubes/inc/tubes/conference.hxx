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

#ifndef INCLUDED_TUBES_CONFERENCE_HXX
#define INCLUDED_TUBES_CONFERENCE_HXX

#include <sal/config.h>
#include "tubes/packet.hxx"
#include "tubes/file-transfer-helper.h"
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <rtl/ustring.hxx>
#include <telepathy-glib/telepathy-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <queue>

typedef ::std::queue<TelePacket> TelePacketQueue;

class TeleManager;

/** Conference setup by TeleManager */
class TeleConference : public boost::enable_shared_from_this<TeleConference>
{
public:

    TeleConference( TeleManager* pManager, TpAccount *pAccount, TpChannel* pChannel, const rtl::OString& rSessionId );
    ~TeleConference();

    /// Close channel and call finalize()
    void                    close();

    /// Unrefs, unregisters from manager and calls dtor if last reference!
    void                    finalize();

    TeleManager*            getManager() const  { return mpManager; }

    /** @param rPacket
            non-const on purpose, see TelePacket::getData()
     */
    bool                    sendPacket( TelePacket& rPacket ) const;

    /** Pop a received packet. */
    bool                    popPacket( TelePacket& rPacket );

    /** Queue incoming data as TelePacket */
    void                    queue( const char* pDBusSender, const char* pPacket, int nSize );


    typedef void          (*FileSentCallback)( bool aSuccess, void* pUserData);
    void                    sendFile( rtl::OUString &localUri, FileSentCallback pCallback, void* pUserData);

    // --- following only to be called only by manager's callbacks ---
    // TODO: make friends instead

    void                    setChannel( TpAccount* pAccount, TpChannel* pChannel );
    TpChannel*              getChannel() const  { return mpChannel; }
    bool                    offerTube();
    bool                    setTube( DBusConnection* pTube );
    bool                    acceptTube( const char* pAddress );
    /// got tube accepted on other end as well?
    bool                    isTubeOpen() const { return meTubeChannelState == TP_TUBE_CHANNEL_STATE_OPEN; }

    // Only for callbacks.
    void                    setTubeOfferedHandlerInvoked( bool b ) { mbTubeOfferedHandlerInvoked = b; }
    bool                    isTubeOfferedHandlerInvoked() const { return mbTubeOfferedHandlerInvoked; }
    void                    setTubeChannelStateChangedHandlerInvoked( bool b )
                                { mbTubeChannelStateChangedHandlerInvoked = b; }
    bool                    isTubeChannelStateChangedHandlerInvoked() const
                                { return mbTubeChannelStateChangedHandlerInvoked; }
    void                    setTubeChannelState( TpTubeChannelState eState ) { meTubeChannelState = eState; }

private:

    rtl::OString            maSessionId;
    TeleManager*            mpManager;
    TpAccount*              mpAccount;
    TpChannel*              mpChannel;
    DBusConnection*         mpTube;
    TelePacketQueue         maPacketQueue;
    TpTubeChannelState      meTubeChannelState;

    bool                    mbTubeOfferedHandlerInvoked : 1;
    bool                    mbTubeChannelStateChangedHandlerInvoked : 1;

    // hide from the public
    using boost::enable_shared_from_this<TeleConference>::shared_from_this;

};


typedef boost::shared_ptr<TeleConference> TeleConferencePtr;

#endif // INCLUDED_TUBES_CONFERENCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
