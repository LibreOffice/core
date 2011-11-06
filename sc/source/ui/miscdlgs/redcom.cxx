/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

