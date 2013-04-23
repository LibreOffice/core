/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TUBES_CONFERENCE_HXX
#define INCLUDED_TUBES_CONFERENCE_HXX

#include <sal/config.h>
#include <rtl/ustring.hxx>

class Collaboration;
class TeleConferenceImpl;
typedef struct _TpAccount TpAccount;
typedef struct _TpContact TpContact;
typedef struct _TpDBusTubeChannel TpDBusTubeChannel;
typedef struct _GDBusConnection GDBusConnection;

/** Conference setup by TeleManager */
class TeleConference
{
public:

    TeleConference( TpAccount* pAccount,
                    TpDBusTubeChannel* pChannel,
                    const OString sUuid = OString(),
                    bool bMaster = false );
    ~TeleConference();

    /// Close channel and call finalize()
    void                    close();

    /// Unrefs, unregisters from manager and calls dtor if last reference!
    void                    finalize();

    bool                    sendPacket( const OString& rPacket );

    void                    invite( TpContact *pContact );

    typedef void          (*FileSentCallback)( bool aSuccess, void* pUserData);
    void                    sendFile( TpContact* pContact, const OUString& rURL, FileSentCallback pCallback, void* pUserData);
    const OString&          getUuid() const { return msUuid; }

    Collaboration*          getCollaboration() const;
    void                    setCollaboration( Collaboration* pCollaboration );

    // --- following only to be called only by manager's callbacks ---
    // TODO: make friends instead
    void                    setChannel( TpAccount* pAccount, TpDBusTubeChannel* pChannel );
    bool                    offerTube();
    bool                    acceptTube();

    // Only for callbacks.
    bool                    setTube( GDBusConnection* pTube );
    void                    setTubeOfferedHandlerInvoked( bool b );
    bool                    isTubeOfferedHandlerInvoked() const;
    bool                    isMaster() const;
    void                    setUuid( const OString& rUuid ) { msUuid = rUuid; }

private:
    friend class TeleManager;
    // Used only by TeleManager:
    /// got tube accepted on other end as well?
    bool                    isReady() const;

    // Private:
    bool                    spinUntilTubeEstablished();

    Collaboration*          mpCollaboration;
    TpAccount*              mpAccount;
    TpDBusTubeChannel*      mpChannel;
    OString                 msUuid;
    bool                    mbMaster;
    TeleConferenceImpl*     pImpl;
};

#endif // INCLUDED_TUBES_CONFERENCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
