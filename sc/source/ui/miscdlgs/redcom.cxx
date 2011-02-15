/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#include <vcl/msgbox.hxx>
#include <unotools/localedatawrapper.hxx>

#include "redcom.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
//------------------------------------------------------------------------

ScRedComDialog::ScRedComDialog( Window* pParent, const SfxItemSet& rCoreSet,
                    ScDocShell *pShell,ScChangeAction *pAction,sal_Bool bPrevNext)
{
    //CHINA001 pDlg = new SvxPostItDialog(pParent,rCoreSet,bPrevNext,sal_True);
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if(pFact)
    {
        pDlg = pFact->CreateSvxPostItDialog( pParent, rCoreSet, bPrevNext, sal_True );
        DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
        pDocShell=pShell;
        pDlg->DontChangeAuthor();
        pDlg->HideAuthor();

        pDlg->SetPrevHdl(LINK( this, ScRedComDialog, PrevHdl));
        pDlg->SetNextHdl(LINK( this, ScRedComDialog, NextHdl));

        ReInit(pAction);
    }
}

ScRedComDialog::~ScRedComDialog()
{
    delete pDlg;
}

ScChangeAction *ScRedComDialog::FindPrev(ScChangeAction *pAction)
{
    if(pAction!=NULL && pDocShell !=NULL)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScChangeViewSettings* pSettings = pDoc->GetChangeViewSettings();

        pAction=pAction->GetPrev();

        while(pAction!=NULL)
        {
            if( pAction->GetState()==SC_CAS_VIRGIN &&
                pAction->IsDialogRoot() &&
                ScViewUtil::IsActionShown(*pAction,*pSettings,*pDoc)) break;

            pAction=pAction->GetPrev();
        }
    }
    return pAction;
}

ScChangeAction *ScRedComDialog::FindNext(ScChangeAction *pAction)
{
    if(pAction!=NULL && pDocShell !=NULL)
    {
        ScDocument* pDoc = pDocShell->GetDocument();
        ScChangeViewSettings* pSettings = pDoc->GetChangeViewSettings();

        pAction=pAction->GetNext();

        while(pAction!=NULL)
        {
            if( pAction->GetState()==SC_CAS_VIRGIN &&
                pAction->IsDialogRoot() &&
                ScViewUtil::IsActionShown(*pAction,*pSettings,*pDoc)) break;

            pAction=pAction->GetNext();
        }
    }
    return pAction;
}

void ScRedComDialog::ReInit(ScChangeAction *pAction)
{
    pChangeAction=pAction;
    if(pChangeAction!=NULL && pDocShell !=NULL)
    {
        String aTitle;
        pChangeAction->GetDescription( aTitle, pDocShell->GetDocument());
        pDlg->SetText(aTitle);
        aComment=pChangeAction->GetComment();

        sal_Bool bNext=FindNext(pChangeAction)!=NULL;
        sal_Bool bPrev=FindPrev(pChangeAction)!=NULL;
        pDlg->EnableTravel(bNext,bPrev);

        String aAuthor = pChangeAction->GetUser();

        DateTime aDT = pChangeAction->GetDateTime();
        String aDate = ScGlobal::pLocaleData->getDate( aDT );
        aDate += ' ';
        aDate += ScGlobal::pLocaleData->getTime( aDT, sal_False, sal_False );

        pDlg->ShowLastAuthor(aAuthor, aDate);
        pDlg->SetNote(aComment);
    }
}

short ScRedComDialog::Execute()
{
    short nRet=pDlg->Execute();

    if(nRet== RET_OK )
    {
        if ( pDocShell!=NULL && pDlg->GetNote() != aComment )
            pDocShell->SetChangeComment( pChangeAction, pDlg->GetNote());
    }

    return nRet;
}

void ScRedComDialog::SelectCell()
{
    if(pChangeAction!=NULL)
    {
        const ScChangeAction* pAction=pChangeAction;
        const ScBigRange& rRange = pAction->GetBigRange();

        if(rRange.IsValid(pDocShell->GetDocument()))
        {
            ScViewData* pViewData=pDocShell->GetViewData();
            ScRange aRef=rRange.MakeRange();
            ScTabView* pTabView=pViewData->GetView();
            pTabView->MarkRange(aRef);
        }
    }
}

IMPL_LINK(ScRedComDialog, PrevHdl, AbstractSvxPostItDialog*, pDlgP )
{
    if (pDocShell!=NULL && pDlgP->GetNote() != aComment )
        pDocShell->SetChangeComment( pChangeAction, pDlgP->GetNote());

    ReInit(FindPrev(pChangeAction));
    SelectCell();

    return 0;
}

IMPL_LINK(ScRedComDialog, NextHdl, AbstractSvxPostItDialog*, pDlgP )
{
    if ( pDocShell!=NULL && pDlgP->GetNote() != aComment )
        pDocShell->SetChangeComment( pChangeAction, pDlgP->GetNote());

    ReInit(FindNext(pChangeAction));
    SelectCell();

    return 0;
}

