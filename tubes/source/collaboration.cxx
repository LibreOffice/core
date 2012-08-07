/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tubes/collaboration.hxx>

#include <tubes/conference.hxx>
#include <tubes/manager.hxx>

Collaboration::Collaboration() :
    mpConference( NULL ),
    mpContacts( NULL )
{
    TeleManager::registerCollaboration( this );
}

Collaboration::~Collaboration()
{
    TeleManager::unregisterCollaboration( this );
    if (mpConference)
        mpConference->close();
}

TeleConference* Collaboration::GetConference() const
{
    return mpConference;
}

sal_uInt64 Collaboration::GetId() const
{
    return reinterpret_cast<sal_uInt64> (this);
}

void Collaboration::SendFile( TpContact* pContact, const OUString& rURL ) const
{
    mpConference->sendFile( pContact, rURL, NULL, NULL );
}

void Collaboration::SendPacket( const OString& rPacket ) const
{
    mpConference->sendPacket( rPacket );
}

void Collaboration::SetConference( TeleConference* pConference )
{
    mpConference = pConference;
    mpConference->setCollaboration( this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
