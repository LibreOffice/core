/*************************************************************************
 *
 *  $RCSfile: redcom.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#include "redcom.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"

//------------------------------------------------------------------------

ScRedComDialog::ScRedComDialog( Window* pParent, const SfxItemSet& rCoreSet,
                    ScDocShell *pShell,ScChangeAction *pAction,BOOL bPrevNext):
        SvxPostItDialog(pParent,rCoreSet,bPrevNext,TRUE )
{
    pDocShell=pShell;
    DontChangeAuthor();
    HideAuthor();

    SetPrevHdl(LINK( this, ScRedComDialog, PrevHdl));
    SetNextHdl(LINK( this, ScRedComDialog, NextHdl));

    ReInit(pAction);
}

ScRedComDialog::~ScRedComDialog()
{
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
        SetText(aTitle);
        aComment=pChangeAction->GetComment();

        BOOL bNext=FindNext(pChangeAction)!=NULL;
        BOOL bPrev=FindPrev(pChangeAction)!=NULL;
        EnableTravel(bNext,bPrev);

        String aAuthor = pChangeAction->GetUser();

        International* pIntl = ScGlobal::pScInternational;
        DateTime aDT = pChangeAction->GetDateTime();
        String aDate = pIntl->GetDate( aDT );
        aDate += ' ';
        aDate += pIntl->GetTime( aDT, FALSE, FALSE );

        ShowLastAuthor(aAuthor, aDate);
        SetNote(aComment);
    }
}

short ScRedComDialog::Execute()
{
    short nRet=SvxPostItDialog::Execute();

    if(nRet== RET_OK )
    {
        if ( pDocShell!=NULL && GetNote() != aComment )
            pDocShell->SetChangeComment( pChangeAction, GetNote());
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

IMPL_LINK(ScRedComDialog, PrevHdl, SvxPostItDialog*, pDlg )
{
    if (pDocShell!=NULL && GetNote() != aComment )
        pDocShell->SetChangeComment( pChangeAction, GetNote());

    ReInit(FindPrev(pChangeAction));
    SelectCell();

    return 0;
}

IMPL_LINK(ScRedComDialog, NextHdl, SvxPostItDialog*, pDlg )
{
    if ( pDocShell!=NULL && GetNote() != aComment )
        pDocShell->SetChangeComment( pChangeAction, GetNote());

    ReInit(FindNext(pChangeAction));
    SelectCell();

    return 0;
}

