/*************************************************************************
 *
 *  $RCSfile: fldtdlg.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:25:20 $
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


#pragma hdrstop

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif

#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FLDWRAP_HXX
#include <fldwrap.hxx>
#endif
#ifndef _FLDDB_HXX
#include <flddb.hxx>
#endif
#ifndef _FLDDINF_HXX
#include <flddinf.hxx>
#endif
#ifndef _FLDVAR_HXX
#include <fldvar.hxx>
#endif
#ifndef _FLDDOK_HXX
#include <flddok.hxx>
#endif
#ifndef _FLDFUNC_HXX
#include <fldfunc.hxx>
#endif
#ifndef _FLDREF_HXX
#include <fldref.hxx>
#endif
#ifndef _WRTSH_HXX //autogen
#include <wrtsh.hxx>
#endif
#ifndef _SWVIEW_HXX //autogen
#include <view.hxx>
#endif
#ifndef _FLDTDLG_HXX
#include <fldtdlg.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif

#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _FLDTDLG_HRC
#include <fldtdlg.hrc>
#endif

//CHINA001 #include "swabstdlg.hxx" //CHINA001
//CHINA001
//CHINA001 SFX_IMPL_CHILDWINDOW(SwFldDlgWrapper, FN_INSERT_FIELD)
//CHINA001
//CHINA001 /*--------------------------------------------------------------------
//CHINA001 Beschreibung:
//CHINA001 --------------------------------------------------------------------*/
//CHINA001
//CHINA001 SwChildWinWrapper::SwChildWinWrapper(Window *pParentWindow, USHORT nId) :
//CHINA001 SfxChildWindow(pParentWindow, nId),
//CHINA001 pDocSh(0)
//CHINA001 {
//CHINA001 // Flackern der Buttons vermeiden:
//CHINA001 aUpdateTimer.SetTimeout(200);
//CHINA001 aUpdateTimer.SetTimeoutHdl(LINK(this, SwChildWinWrapper, UpdateHdl));
//CHINA001 }
//CHINA001
//CHINA001 /*--------------------------------------------------------------------
//CHINA001 Beschreibung:
//CHINA001 --------------------------------------------------------------------*/
//CHINA001
//CHINA001 IMPL_LINK( SwChildWinWrapper, UpdateHdl, void*, EMPTYARG )
//CHINA001 {
//CHINA001 GetWindow()->Activate(); // Dialog aktualisieren
//CHINA001
//CHINA001 return 0;
//CHINA001 }
//CHINA001
//CHINA001 /*--------------------------------------------------------------------
//CHINA001 Beschreibung: Nach Dok-Wechsel Dialog neu initialisieren
//CHINA001 --------------------------------------------------------------------*/
//CHINA001
//CHINA001 BOOL SwChildWinWrapper::ReInitDlg(SwDocShell *pDocSh)
//CHINA001 {
//CHINA001 BOOL bRet = FALSE;
//CHINA001
//CHINA001 if (pDocSh != GetOldDocShell())
//CHINA001  {
//CHINA001 aUpdateTimer.Stop();
//CHINA001 bRet = TRUE;         // Sofortiges Update
//CHINA001  }
//CHINA001  else
//CHINA001 aUpdateTimer.Start();
//CHINA001
//CHINA001 return bRet;
//CHINA001 }
//CHINA001
//CHINA001 /*--------------------------------------------------------------------
//CHINA001 Beschreibung:
//CHINA001 --------------------------------------------------------------------*/
//CHINA001
//CHINA001 SfxChildWinInfo SwFldDlgWrapper::GetInfo() const
//CHINA001 {
//CHINA001 SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
//CHINA001 aInfo.aPos = GetWindow()->OutputToAbsoluteScreenPixel(aInfo.aPos);
//CHINA001 return aInfo;
//CHINA001 }
//CHINA001
//CHINA001
//CHINA001 /*--------------------------------------------------------------------
//CHINA001 Beschreibung:
//CHINA001 --------------------------------------------------------------------*/
//CHINA001
//CHINA001 SwFldDlgWrapper::SwFldDlgWrapper( Window* pParent, USHORT nId,
//CHINA001 SfxBindings* pB,
//CHINA001 SfxChildWinInfo* pInfo )
//CHINA001 : SwChildWinWrapper( pParent, nId )
//CHINA001 {
//CHINA001 //CHINA001   SwFldDlg *pDlg = new SwFldDlg( pB, this, pParent );
//CHINA001 //CHINA001   pWindow = pDlg;
//CHINA001 SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();//CHINA001
//CHINA001 DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");//CHINA001
//CHINA001
//CHINA001 AbstractSwFldDlg* pDlg = pFact->CreateSwFldDlg(pB, this, pParent,ResId( DLG_FLD_INSERT ));
//CHINA001 DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
//CHINA001 pDlgInterface = pDlg;
//CHINA001 pWindow = pDlg->GetWindow();
//CHINA001 pDlg->Start();
//CHINA001 eChildAlignment = SFX_ALIGN_NOALIGNMENT;
//CHINA001 }
//CHINA001
//CHINA001 /*--------------------------------------------------------------------
//CHINA001 Beschreibung: Nach Dok-Wechsel Dialog neu initialisieren
//CHINA001 --------------------------------------------------------------------*/
//CHINA001
//CHINA001 BOOL SwFldDlgWrapper::ReInitDlg(SwDocShell *pDocSh)
//CHINA001 {
//CHINA001 BOOL bRet;
//CHINA001
//CHINA001 if ((bRet = SwChildWinWrapper::ReInitDlg(pDocSh)) == TRUE)   // Sofort aktualisieren, Dok-Wechsel
//CHINA001  {
//CHINA001 //CHINA001 ((SwFldDlg*)GetWindow())->ReInitDlg();
//CHINA001 pDlgInterface->ReInitDlg(); //CHINA001
//CHINA001  }
//CHINA001
//CHINA001 return bRet;
//CHINA001 }
//CHINA001
//CHINA001 /*--------------------------------------------------------------------
//CHINA001 Beschreibung:
//CHINA001 --------------------------------------------------------------------*/
//CHINA001
//CHINA001 void SwFldDlgWrapper::ShowPage(USHORT nPage)
//CHINA001 {
//CHINA001 //CHINA001 ((SwFldDlg*)GetWindow())->ShowPage(nPage ? nPage : TP_FLD_REF);
//CHINA001 pDlgInterface->ShowPage(nPage ? nPage : TP_FLD_REF); //CHINA001
//CHINA001 }
//CHINA001
//CHINA001 SFX_IMPL_CHILDWINDOW(SwFldDataOnlyDlgWrapper, FN_INSERT_FIELD_DATA_ONLY)
//CHINA001
//CHINA001 /* -----------------04.02.2003 14:17-----------------
//CHINA001 *
//CHINA001 * --------------------------------------------------*/
//CHINA001 SfxChildWinInfo SwFldDataOnlyDlgWrapper::GetInfo() const
//CHINA001 {
//CHINA001 SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
//CHINA001 // prevent instatiation of dialog other than by calling
//CHINA001 // the mail merge dialog
//CHINA001 aInfo.bVisible = FALSE;
//CHINA001 return aInfo;
//CHINA001 }
//CHINA001 /* -----------------04.02.2003 14:17-----------------
//CHINA001 *
//CHINA001 * --------------------------------------------------*/
//CHINA001 SwFldDataOnlyDlgWrapper::SwFldDataOnlyDlgWrapper( Window* pParent, USHORT nId,
//CHINA001 SfxBindings* pB,
//CHINA001 SfxChildWinInfo* pInfo )
//CHINA001 : SwChildWinWrapper( pParent, nId )
//CHINA001 {
//CHINA001 //CHINA001     SwFldDlg *pDlg = new SwFldDlg( pB, this, pParent );
//CHINA001 //CHINA001   pWindow = pDlg;
//CHINA001 SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();//CHINA001
//CHINA001 DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");//CHINA001
//CHINA001
//CHINA001 AbstractSwFldDlg* pDlg = pFact->CreateSwFldDlg(pB, this, pParent,ResId( DLG_FLD_INSERT ));
//CHINA001 DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
//CHINA001 pDlgInterface = pDlg; //CHINA001
//CHINA001
//CHINA001 pWindow = pDlg->GetWindow();//CHINA001
//CHINA001 pDlg->ActivateDatabasePage();
//CHINA001 pDlg->Start();
//CHINA001 pDlg->Initialize( pInfo );
//CHINA001 eChildAlignment = SFX_ALIGN_NOALIGNMENT;
//CHINA001 }
//CHINA001 /* -----------------04.02.2003 14:17-----------------
//CHINA001 * re-init after doc activation
//CHINA001 * --------------------------------------------------*/
//CHINA001 BOOL SwFldDataOnlyDlgWrapper::ReInitDlg(SwDocShell *pDocSh)
//CHINA001 {
//CHINA001 BOOL bRet;
//CHINA001 if ((bRet = SwChildWinWrapper::ReInitDlg(pDocSh)) == TRUE)  // Sofort aktualisieren, Dok-Wechsel
//CHINA001  {
//CHINA001 //CHINA001 ((SwFldDlg*)GetWindow())->ReInitDlg();
//CHINA001 pDlgInterface->ReInitDlg(); //CHINA001
//CHINA001  }
//CHINA001
//CHINA001 return bRet;
//CHINA001 }

/*--------------------------------------------------------------------
    Beschreibung:   Der Traeger des Dialoges
 --------------------------------------------------------------------*/


SwFldDlg::SwFldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, Window *pParent)
    : SfxTabDialog( pParent, SW_RES( DLG_FLD_INSERT )),
    pChildWin(pCW),
    bDataBaseMode(FALSE),
    pBindings(pB)
{
    SetStyle(GetStyle()|WB_STDMODELESS);
    bHtmlMode = (::GetHtmlMode((SwDocShell*)SfxObjectShell::Current()) & HTMLMODE_ON) != 0;

    RemoveResetButton();

    GetOKButton().SetText(String(SW_RES(STR_FLD_INSERT)));
    GetOKButton().SetHelpId(HID_FIELD_INSERT);
    GetOKButton().SetHelpText(aEmptyStr);   // Damit generierter Hilfetext verwendet wird

    GetCancelButton().SetText(String(SW_RES(STR_FLD_CLOSE)));
    GetCancelButton().SetHelpId(HID_FIELD_CLOSE);
    GetCancelButton().SetHelpText(aEmptyStr);   // Damit generierter Hilfetext verwendet wird

    FreeResource();

    GetOKButton().SetClickHdl(LINK(this, SwFldDlg, OKHdl));

    AddTabPage(TP_FLD_DOK, SwFldDokPage::Create, 0);
    AddTabPage(TP_FLD_VAR, SwFldVarPage::Create, 0);
    AddTabPage(TP_FLD_DOKINF, SwFldDokInfPage::Create, 0);

    if (!bHtmlMode)
    {
        AddTabPage(TP_FLD_REF, SwFldRefPage::Create, 0);
        AddTabPage(TP_FLD_FUNC, SwFldFuncPage::Create, 0);
        AddTabPage(TP_FLD_DB, SwFldDBPage::Create, 0);
    }
    else
    {
        RemoveTabPage(TP_FLD_REF);
        RemoveTabPage(TP_FLD_FUNC);
        RemoveTabPage(TP_FLD_DB);
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldDlg::~SwFldDlg()
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

BOOL SwFldDlg::Close()
{
    SfxViewFrame::Current()->GetDispatcher()->
        Execute(bDataBaseMode ? FN_INSERT_FIELD_DATA_ONLY : FN_INSERT_FIELD,
        SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
    return TRUE;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldDlg::Initialize(SfxChildWinInfo *pInfo)
{
    Point aPos;
    Size aSize;

    if ( pInfo->aSize.Width() != 0 && pInfo->aSize.Height() != 0 )
    {
        aPos = pInfo->aPos;
        if ( GetStyle() & WB_SIZEABLE )
            SetSizePixel( pInfo->aSize );

        // Initiale Gr"o\se aus pInfo merken
        aSize = GetSizePixel();

        // Soll das FloatingWindow eingezoomt werden ?
        if ( pInfo->nFlags & SFX_CHILDWIN_ZOOMIN )
            RollUp();
    }
    else
    {
        // Initiale Gr"o\se aus Resource oder ctor merken
        aSize = GetSizePixel();

        Size aParentSize = GetParent()->GetOutputSizePixel();
        aPos.X() += ( aParentSize.Width() - aSize.Width() ) / 2;
        aPos.Y() += ( aParentSize.Height() - aSize.Height() ) / 2;
    }

    Point aPoint;
    Rectangle aRect = GetDesktopRectPixel();
    aPoint.X() = aRect.Right() - aSize.Width();
    aPoint.Y() = aRect.Bottom() - aSize.Height();

    aPoint = OutputToScreenPixel( aPoint );

    if ( aPos.X() > aPoint.X() )
        aPos.X() = aPoint.X() ;
    if ( aPos.Y() > aPoint.Y() )
        aPos.Y() = aPoint.Y();

    if ( aPos.X() < 0 ) aPos.X() = 0;
    if ( aPos.Y() < 0 ) aPos.Y() = 0;

    SetPosPixel( aPos );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxItemSet* SwFldDlg::CreateInputItemSet( USHORT nId )
{
    return 0;
}

/*--------------------------------------------------------------------
     Beschreibung: Einfuegen von neuen Feldern anstossen
 --------------------------------------------------------------------*/

IMPL_LINK( SwFldDlg, OKHdl, Button *, pBtn )
{
    if (GetOKButton().IsEnabled())
    {
        SfxTabPage* pPage = GetTabPage(GetCurPageId());
        pPage->FillItemSet(*(SfxItemSet*)0);

        GetOKButton().GrabFocus();  // Wegen InputField-Dlg
    }

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Dok-Wechsel Dialog neu initialisieren
 --------------------------------------------------------------------*/

void SwFldDlg::ReInitDlg()
{
    SwDocShell* pDocSh = (SwDocShell*)SfxObjectShell::Current();
    BOOL bNewMode = (::GetHtmlMode(pDocSh) & HTMLMODE_ON) != 0;

    if (bNewMode != bHtmlMode)
    {
        SfxViewFrame::Current()->GetDispatcher()->
            Execute(FN_INSERT_FIELD, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
        Close();
    }

    SwView* pActiveView = ::GetActiveView();
    if(!pActiveView)
        return;
    const SwWrtShell& rSh = pActiveView->GetWrtShell();
    GetOKButton().Enable( !rSh.IsReadOnlyAvailable() ||
                          !rSh.HasReadonlySel() );

    ReInitTabPage(TP_FLD_DOK);
    ReInitTabPage(TP_FLD_VAR);
    ReInitTabPage(TP_FLD_DOKINF);

    if (!bHtmlMode)
    {
        ReInitTabPage(TP_FLD_REF);
        ReInitTabPage(TP_FLD_FUNC);
        ReInitTabPage(TP_FLD_DB);
    }

    pChildWin->SetOldDocShell(pDocSh);
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Dok-Wechsel TabPage neu initialisieren
 --------------------------------------------------------------------*/

void SwFldDlg::ReInitTabPage( USHORT nPageId, BOOL bOnlyActivate )
{
    SwFldPage* pPage = (SwFldPage* )GetTabPage(nPageId);

    if ( pPage )
        pPage->EditNewField( bOnlyActivate );   // TabPage neu initialisieren
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Aktivierung einige TabPages neu initialisieren
 --------------------------------------------------------------------*/

void SwFldDlg::Activate()
{
    SwView* pView = ::GetActiveView();
    if( pView )
    {
        BOOL bHtmlMode = (::GetHtmlMode((SwDocShell*)SfxObjectShell::Current()) & HTMLMODE_ON) != 0;
        const SwWrtShell& rSh = pView->GetWrtShell();
        GetOKButton().Enable( !rSh.IsReadOnlyAvailable() ||
                              !rSh.HasReadonlySel() );

        ReInitTabPage( TP_FLD_VAR, TRUE );

        if( !bHtmlMode )
        {
            ReInitTabPage( TP_FLD_REF, TRUE );
            ReInitTabPage( TP_FLD_FUNC, TRUE );
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldDlg::EnableInsert(BOOL bEnable)
{
    if( bEnable )
    {
        SwView* pView = ::GetActiveView();
        DBG_ASSERT(pView, "no view found");
        if( !pView ||
                (pView->GetWrtShell().IsReadOnlyAvailable() &&
                    pView->GetWrtShell().HasReadonlySel()) )
            bEnable = FALSE;
    }
    GetOKButton().Enable(bEnable);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldDlg::InsertHdl()
{
    GetOKButton().Click();
}
/* -----------------27.11.2002 15:24-----------------
 *
 * --------------------------------------------------*/
void SwFldDlg::ActivateDatabasePage()
{
    bDataBaseMode = TRUE;
    ShowPage( TP_FLD_DB );
    SfxTabPage* pDBPage =  GetTabPage( TP_FLD_DB );
    if( pDBPage )
    {
        ((SwFldDBPage*)pDBPage)->ActivateMailMergeAddress();
    }
    //remove all other pages
    RemoveTabPage(TP_FLD_DOK);
    RemoveTabPage(TP_FLD_VAR);
    RemoveTabPage(TP_FLD_DOKINF);
    RemoveTabPage(TP_FLD_REF);
    RemoveTabPage(TP_FLD_FUNC);
}
/*-- 07.10.2003 14:01:44---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwFldDlg::PageCreated(USHORT nId, SfxTabPage& rPage)
{
    if( TP_FLD_DB == nId)
    {
        SfxDispatcher* pDispatch = pBindings->GetDispatcher();
        SfxViewFrame* pViewFrame = pDispatch ? pDispatch->GetFrame() : 0;
        if(pViewFrame)
        {
            const TypeId aSwViewTypeId = TYPE(SwView);
            SfxViewShell* pViewShell = SfxViewShell::GetFirst( &aSwViewTypeId );
            while(pViewShell && pViewShell->GetViewFrame() != pViewFrame)
            {
                pViewShell = SfxViewShell::GetNext( *pViewShell, &aSwViewTypeId );
            }
            if(pViewShell)
                static_cast<SwFldDBPage&>(rPage).SetWrtShell(static_cast<SwView*>(pViewShell)->GetWrtShell());
        }
    }
}


