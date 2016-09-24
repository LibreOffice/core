/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_SCCOLLABORATION_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SCCOLLABORATION_HXX

#include <sal/config.h>

#include <tubes/collaboration.hxx>
class ScDocFuncSend;
class ScDocShell;

class ScCollaboration : public Collaboration
{
    ScDocShell* mpScDocShell;
public:
            ScCollaboration( ScDocShell* pScDocShell );
    virtual ~ScCollaboration();

    virtual void  EndCollaboration() const;
    virtual void  PacketReceived( const OString& rPacket ) const;
    virtual void  SaveAndSendFile( TpContact* pContact ) const;
    virtual void  StartCollaboration( TeleConference* pConference );
private:
    friend class ScDocShell;
    ScDocFuncSend* GetScDocFuncSend() const;
};

#endif // INCLUDED_SC_SOURCE_UI_INC_SCCOLLABORATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
