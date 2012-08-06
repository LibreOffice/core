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
#include <tubes/tubesdllapi.h>

class TeleConference;
typedef struct _TpContact TpContact;

class TUBES_DLLPUBLIC Collaboration
{
    TeleConference* mpConference;
public:
            Collaboration();
    virtual ~Collaboration();

    virtual void ContactLeft() const = 0;
    virtual void PacketReceived( const OString& rPacket ) const = 0;
    virtual void SaveAndSendFile( TpContact* pContact, const OUString& rURL ) const = 0;
    virtual void StartCollaboration( TeleConference* pConference ) = 0;

    TUBES_DLLPRIVATE TeleConference* GetConference() const;
    TUBES_DLLPRIVATE sal_uInt64 GetId() const;

    void SendFile( TpContact* pContact, const OUString& rURL ) const;
    void SendPacket( const OString& rPacket ) const;
    void SetConference( TeleConference* pConference );
};

#endif // INCLUDED_TUBES_COLLABORATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
