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
    // This is in fact of type TubeContacts* from anonymous namespace
    void* mpContacts;
public:
            Collaboration();
    virtual ~Collaboration();

    /** Returns to normal editing mode */
    virtual void EndCollaboration() const = 0;
    virtual void PacketReceived( const OString& rPacket ) const = 0;
    /** Saves current document and then calls SendFile() with the file URL */
    virtual void SaveAndSendFile( TpContact* pContact ) const = 0;
    /** Prepares document for collaboration and should call SetConference() */
    virtual void StartCollaboration( TeleConference* pConference ) = 0;

    TUBES_DLLPRIVATE sal_uInt64 GetId() const;
    TUBES_DLLPRIVATE void Invite( TpContact* pContact ) const;

    /** Application calls this to display contacts dialog from where can the collaboration start */
    void DisplayContacts();
    void SendFile( TpContact* pContact, const OUString& rURL ) const;
    void SendPacket( const OString& rPacket ) const;
    void SetConference( TeleConference* pConference );
};

#endif // INCLUDED_TUBES_COLLABORATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
