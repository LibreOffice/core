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

ScCollaboration::ScCollaboration()
{
}

ScCollaboration::~ScCollaboration()
{
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
    return reinterpret_cast<sal_uInt64> (SfxObjectShell::Current());
}

void ScCollaboration::SetCollaboration( TeleConference* pConference )
{
    ScDocShell* pScDocShell = dynamic_cast<ScDocShell*> (SfxObjectShell::Current());
    ScDocFunc* pDocFunc = pScDocShell ? &pScDocShell->GetDocFunc() : NULL;
    ScDocFuncSend* pSender = dynamic_cast<ScDocFuncSend*> (pDocFunc);
    if (!pSender)
    {
        // This means pDocFunc has to be ScDocFuncDirect* and we are not collaborating yet.
        ScDocFuncDirect *pDirect = dynamic_cast<ScDocFuncDirect*> (pDocFunc);
        ScDocFuncRecv *pReceiver = new ScDocFuncRecv( pDirect );
        pSender = new ScDocFuncSend( *pScDocShell, pReceiver );
        pScDocShell->SetDocFunc( pSender );
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
    ScDocShell *pScDocShell = dynamic_cast<ScDocShell*> (SfxObjectShell::Current());
    ScDocFunc *pDocFunc = pScDocShell ? &pScDocShell->GetDocFunc() : NULL;
    return dynamic_cast<ScDocFuncSend*> (pDocFunc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
