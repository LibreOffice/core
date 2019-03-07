/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <unotools/localedatawrapper.hxx>

#include <chgtrack.hxx>
#include <redcom.hxx>
#include <docsh.hxx>
#include <dbfunc.hxx>
#include <tabview.hxx>
#include <viewutil.hxx>
#include <svx/svxdlg.hxx>

ScRedComDialog::ScRedComDialog( weld::Window* pParent, const SfxItemSet& rCoreSet,
                    ScDocShell *pShell, ScChangeAction *pAction, bool bPrevNext)
    : pChangeAction(nullptr)
    , pDocShell(nullptr)
    , pDlg(nullptr)
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    pDlg = pFact->CreateSvxPostItDialog( pParent, rCoreSet, bPrevNext );
    pDocShell=pShell;
    pDlg->DontChangeAuthor();
    pDlg->HideAuthor();

    pDlg->SetPrevHdl(LINK( this, ScRedComDialog, PrevHdl));
    pDlg->SetNextHdl(LINK( this, ScRedComDialog, NextHdl));

    ReInit(pAction);
}

ScRedComDialog::~ScRedComDialog()
{
    pDlg.disposeAndClear();
}

ScChangeAction *ScRedComDialog::FindPrev(ScChangeAction *pAction)
{
    if(pAction!=nullptr && pDocShell !=nullptr)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScChangeViewSettings* pSettings = rDoc.GetChangeViewSettings();

        pAction=pAction->GetPrev();

        while(pAction!=nullptr)
        {
            if( pAction->GetState()==SC_CAS_VIRGIN &&
                pAction->IsDialogRoot() &&
                ScViewUtil::IsActionShown(*pAction,*pSettings,rDoc)) break;

            pAction=pAction->GetPrev();
        }
    }
    return pAction;
}

ScChangeAction *ScRedComDialog::FindNext(ScChangeAction *pAction)
{
    if(pAction!=nullptr && pDocShell !=nullptr)
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        ScChangeViewSettings* pSettings = rDoc.GetChangeViewSettings();

        pAction=pAction->GetNext();

        while(pAction!=nullptr)
        {
            if( pAction->GetState()==SC_CAS_VIRGIN &&
                pAction->IsDialogRoot() &&
                ScViewUtil::IsActionShown(*pAction,*pSettings,rDoc)) break;

            pAction=pAction->GetNext();
        }
    }
    return pAction;
}

void ScRedComDialog::ReInit(ScChangeAction *pAction)
{
    pChangeAction=pAction;
    if(pChangeAction!=nullptr && pDocShell !=nullptr)
    {
        OUString aTitle;
        pChangeAction->GetDescription( aTitle, &pDocShell->GetDocument());
        pDlg->SetText(aTitle);
        aComment=pChangeAction->GetComment();

        bool bNext=FindNext(pChangeAction)!=nullptr;
        bool bPrev=FindPrev(pChangeAction)!=nullptr;
        pDlg->EnableTravel(bNext,bPrev);

        OUString aAuthor = pChangeAction->GetUser();

        DateTime aDT = pChangeAction->GetDateTime();
        OUString aDate = ScGlobal::pLocaleData->getDate( aDT );
        aDate += " ";
        aDate += ScGlobal::pLocaleData->getTime( aDT, false );

        pDlg->ShowLastAuthor(aAuthor, aDate);
        pDlg->SetNote(aComment);
    }
}

void ScRedComDialog::Execute()
{
    short nRet=pDlg->Execute();

    if(nRet== RET_OK )
    {
        if ( pDocShell!=nullptr && pDlg->GetNote() != aComment )
            pDocShell->SetChangeComment( pChangeAction, pDlg->GetNote());
    }
}

void ScRedComDialog::SelectCell()
{
    if(pChangeAction!=nullptr)
    {
        const ScChangeAction* pAction=pChangeAction;
        const ScBigRange& rRange = pAction->GetBigRange();

        if(rRange.IsValid(&pDocShell->GetDocument()))
        {
            ScViewData* pViewData=ScDocShell::GetViewData();
            ScRange aRef=rRange.MakeRange();
            ScTabView* pTabView=pViewData->GetView();
            pTabView->MarkRange(aRef);
        }
    }
}

IMPL_LINK(ScRedComDialog, PrevHdl, AbstractSvxPostItDialog&, rDlgP, void )
{
    if (pDocShell!=nullptr && rDlgP.GetNote() != aComment )
        pDocShell->SetChangeComment( pChangeAction, rDlgP.GetNote());

    ReInit(FindPrev(pChangeAction));
    SelectCell();
}

IMPL_LINK(ScRedComDialog, NextHdl, AbstractSvxPostItDialog&, rDlgP, void )
{
    if ( pDocShell!=nullptr && rDlgP.GetNote() != aComment )
        pDocShell->SetChangeComment( pChangeAction, rDlgP.GetNote());

    ReInit(FindNext(pChangeAction));
    SelectCell();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
