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

#include <contacts.hxx>

Collaboration::Collaboration() :
    mpConference( nullptr )
{
    TeleManager::registerCollaboration( this );
}

Collaboration::~Collaboration()
{
    TeleManager::unregisterCollaboration( this );
    if (mpConference)
        mpConference->close();
}

sal_uInt64 Collaboration::GetId() const
{
    return reinterpret_cast<sal_uInt64> (this);
}

void Collaboration::Invite( TpContact* pContact ) const
{
    if (mpConference)
    {
        mpConference->invite( pContact );
        SaveAndSendFile( pContact );
    }
}

void Collaboration::DisplayContacts()
{
    if (!mpContacts)
        mpContacts = VclPtr<tubes::TubeContacts>::Create( this );
    mpContacts->Populate();
}

void Collaboration::SendFile( TpContact* pContact, const OUString& rURL ) const
{
    if (mpConference)
        mpConference->sendFile( pContact, rURL, nullptr, nullptr );
}

void Collaboration::SendPacket( const OString& rPacket ) const
{
    if (mpConference)
        mpConference->sendPacket( rPacket );
}

void Collaboration::SetConference( TeleConference* pConference )
{
    mpConference = pConference;
    mpConference->setCollaboration( this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
