/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TUBES_COLLABORATION_HXX
#define INCLUDED_TUBES_COLLABORATION_HXX

#include <sal/config.h>

#include <rtl/ustring.hxx>

class TeleConference;
typedef struct _TpContact TpContact;

class Collaboration
{
public:
            Collaboration() {}
    virtual ~Collaboration() {}

    virtual void                ContactLeft() = 0;
    virtual TeleConference*     GetConference() = 0;
    virtual sal_uInt64          GetId() = 0;
    virtual void                PacketReceived( const OString& rPacket ) = 0;
    virtual void                SetCollaboration( TeleConference* pConference ) = 0;
    // TODO: I think this could be moved to TeleManager later.
    virtual void                SendFile( TpContact* pContact, const OUString& rURL ) = 0;
};

#endif // INCLUDED_TUBES_COLLABORATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
