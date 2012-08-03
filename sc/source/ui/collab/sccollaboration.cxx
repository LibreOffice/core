/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sccollaboration.hxx"

#include "docsh.hxx"
#include "sendfunc.hxx"

ScCollaboration::ScCollaboration( ScDocShell* pScDocShell ) :
    mpScDocShell( pScDocShell )
{
}

ScCollaboration::~ScCollaboration()
{
}

void ScCollaboration::ContactLeft()
{
    SAL_INFO( "sc.tubes", "Contact has left the collaboration" );
    ScDocFuncSend* pSender = GetScDocFuncSend();
    if (pSender)
    {
        delete pSender;
        mpScDocShell->SetDocFunc( new ScDocFuncDirect( *mpScDocShell ) );
    }
}

TeleConference* ScCollaboration::GetConference()
{
    ScDocFuncSend* pSender = GetScDocFuncSend();
    if (pSender)
        return pSender->GetConference();

    return NULL;
}

sal_uInt64 ScCollaboration::GetId()
{
    return reinterpret_cast<sal_uInt64> (mpScDocShell);
}

void ScCollaboration::SetCollaboration( TeleConference* pConference )
{
    ScDocFunc* pDocFunc = &mpScDocShell->GetDocFunc();
    ScDocFuncSend* pSender = dynamic_cast<ScDocFuncSend*> (pDocFunc);
    if (!pSender)
    {
        // This means pDocFunc has to be ScDocFuncDirect* and we are not collaborating yet.
        ScDocFuncDirect *pDirect = dynamic_cast<ScDocFuncDirect*> (pDocFunc);
        ScDocFuncRecv *pReceiver = new ScDocFuncRecv( pDirect );
        pSender = new ScDocFuncSend( *mpScDocShell, pReceiver );
        mpScDocShell->SetDocFunc( pSender );
    }
    pSender->SetCollaboration( pConference );
}

void ScCollaboration::SendFile( TpContact* pContact, const OUString& rURL )
{
    ScDocFuncSend* pSender = GetScDocFuncSend();
    if (pSender)
        pSender->SendFile( pContact, rURL );
}

ScDocFuncSend* ScCollaboration::GetScDocFuncSend()
{
    return dynamic_cast<ScDocFuncSend*> (&mpScDocShell->GetDocFunc());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
