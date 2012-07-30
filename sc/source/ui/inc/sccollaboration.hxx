/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_COLLABORATION_HXX
#define INCLUDED_SC_COLLABORATION_HXX

#include <tubes/collaboration.hxx>
class ScDocFuncSend;

class ScCollaboration : public Collaboration
{
public:
                            ScCollaboration();
    virtual                 ~ScCollaboration();

    virtual TeleConference* GetConference() const;
    virtual void            SetCollaboration( TeleConference* pConference );
    virtual void            SendFile( TpContact* pContact, const OUString& rURL );
private:
    ScDocFuncSend*          GetScDocFuncSend() const;
};

#endif // INCLUDED_SC_COLLABORATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
