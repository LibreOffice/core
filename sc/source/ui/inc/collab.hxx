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

#ifndef INCLUDED_SC_COLLAB_HXX
#define INCLUDED_SC_COLLAB_HXX

#include <sal/config.h>
#include <boost/signals2.hpp>

typedef struct _TpContact TpContact;
typedef struct _TpAccount TpAccount;
class TeleManager;
class TeleConference;
class TelePacket;
namespace rtl { class OString; class OUString; }

class ScCollaboration
{
public:

                            ScCollaboration();
                            ~ScCollaboration();

    bool                    initManager();
    bool                    initAccountContact();
    bool                    startCollaboration();

    bool                    sendPacket( const rtl::OString& rString );
    /** Emitted when a packet is received
     */
    boost::signals2::signal<void (TeleConference*, rtl::OString& rString )> sigPacketReceived;
    bool                    recvPacket( rtl::OString& rString, TeleConference* pConference );

    void                    sendFile( rtl::OUString &rFileURL );
    /** Emitted when a file is received
     */
    boost::signals2::signal<void ( rtl::OUString *pFileURL )> sigFileReceived;

    /* Internal callbacks */
    void                    packetReceivedCallback( TeleConference *pConference, TelePacket &rPacket );
    void                    receivedFile( rtl::OUString &rFileURL );

private:

    TpAccount*      mpAccount;
    TpContact*      mpContact;
    TeleManager*    mpManager;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
