/*************************************************************************
 *
 *  $RCSfile: fldtdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:36 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

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


SFX_IMPL_CHILDWINDOW(SwFldDlgWrapper, FN_INSERT_FIELD)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwChildWinWrapper::SwChildWinWrapper(Window *pParentWindow, USHORT nId) :
        SfxChildWindow(pParentWindow, nId),
        pDocSh(0)
{
    // Flackern der Buttons vermeiden:
    aUpdateTimer.SetTimeout(200);
    aUpdateTimer.SetTimeoutHdl(LINK(this, SwChildWinWrapper, UpdateHdl));
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_LINK( SwChildWinWrapper, UpdateHdl, void*, EMPTYARG )
{
    GetWindow()->Activate();    // Dialog aktualisieren

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Dok-Wechsel Dialog neu initialisieren
 --------------------------------------------------------------------*/

BOOL SwChildWinWrapper::ReInitDlg(SwDocShell *pDocSh)
{
    BOOL bRet = FALSE;

    if (pDocSh != GetOldDocShell())
    {
        aUpdateTimer.Stop();
        bRet = TRUE;            // Sofortiges Update
    }
    else
        aUpdateTimer.Start();

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFldDlgWrapper::SwFldDlgWrapper( Window* pParent, USHORT nId,
                                        SfxBindings* pB,
                                        SfxChildWinInfo* pInfo) :
    SwChildWinWrapper( pParent, nId )

{
    pWindow = new SwFldDlg( pB, this, pParent );
    SwFldDlg *pWin = (SwFldDlg*)pWindow;
    pWin->Start();

    ((SwFldDlg*)pWindow)->Initialize(pInfo);
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxChildWinInfo SwFldDlgWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    // ((SfxFloatingWindow*)GetWindow())->FillInfo( aInfo );
    return aInfo;
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Dok-Wechsel Dialog neu initialisieren
 --------------------------------------------------------------------*/

BOOL SwFldDlgWrapper::ReInitDlg(SwDocShell *pDocSh)
{
    BOOL bRet;

    if ((bRet = SwChildWinWrapper::ReInitDlg(pDocSh)) == TRUE)  // Sofort aktualisieren, Dok-Wechsel
        ((SwFldDlg*)GetWindow())->ReInitDlg();

    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldDlgWrapper::ShowPage(USHORT nPage)
{
    ((SwFldDlg*)GetWindow())->ShowPage(nPage ? nPage : TP_FLD_REF);
}

/*--------------------------------------------------------------------
    Beschreibung:   Der Traeger des Dialoges
 --------------------------------------------------------------------*/


SwFldDlg::SwFldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, Window *pParent) :
    SfxTabDialog(pParent, SW_RES(DLG_FLD_INSERT)),
    pChildWin(pCW)
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
        Execute(FN_INSERT_FIELD, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
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

void SwFldDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
/*  switch( nId )
    {
        case TP_CHAR_DB:
//          ((SvxCharExtPage&)rPage).DisableControls(DISABLE_CASEMAP);
            break;
    }*/
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

    const SwWrtShell& rSh = ::GetActiveView()->GetWrtShell();
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

void SwFldDlg::ReInitTabPage(USHORT nPageId)
{
    SwFldPage* pPage = (SwFldPage* )GetTabPage(nPageId);

    if (pPage)
        pPage->EditNewField();  // TabPage neu initialisieren
}

/*--------------------------------------------------------------------
    Beschreibung: Nach Aktivierung einige TabPages neu initialisieren
 --------------------------------------------------------------------*/

void SwFldDlg::Activate()
{
    BOOL bHtmlMode = (::GetHtmlMode((SwDocShell*)SfxObjectShell::Current()) & HTMLMODE_ON) != 0;

    const SwWrtShell& rSh = ::GetActiveView()->GetWrtShell();
    GetOKButton().Enable( !rSh.IsReadOnlyAvailable() ||
                          !rSh.HasReadonlySel() );

    ReInitTabPage(TP_FLD_VAR);

    if (!bHtmlMode)
        ReInitTabPage(TP_FLD_REF);

}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwFldDlg::EnableInsert(BOOL bEnable)
{
    if( bEnable )
    {
        const SwWrtShell& rSh = ::GetActiveView()->GetWrtShell();
        if( rSh.IsReadOnlyAvailable() && rSh.HasReadonlySel() )
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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.26  2000/09/18 16:05:29  willem.vandorp
    OpenOffice header added.

    Revision 1.25  2000/09/07 15:59:24  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.24  2000/02/11 14:45:56  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.23  1999/01/20 13:16:52  JP
    Task #58677#: Crsr in Readonly Bereichen zulassen


      Rev 1.22   20 Jan 1999 14:16:52   JP
   Task #58677#: Crsr in Readonly Bereichen zulassen

      Rev 1.21   22 Oct 1998 12:20:42   OM
   #58157# Querverweis einfuegen

      Rev 1.20   21 Oct 1998 16:16:04   OM
   #58157# Querverweise einfuegen

      Rev 1.19   09 Jul 1998 09:52:58   JP
   EmptyStr benutzen

      Rev 1.18   31 Mar 1998 13:31:38   OM
   Korrekt abgleichen

      Rev 1.17   27 Mar 1998 14:14:16   OM
   ChildWindows im Modified-Hdl updaten

      Rev 1.16   18 Mar 1998 10:33:56   OM
   #48197# Focus restaurieren nach InputDlg

      Rev 1.15   10 Mar 1998 17:27:12   OM
   Korrekte Windowbits setzen

      Rev 1.14   03 Mar 1998 13:31:02   OM
   Im Html-Mode nicht alle TabPages anzeigen

      Rev 1.13   09 Jan 1998 16:57:18   OM
   Bei Dok-Wechsel updaten

      Rev 1.12   06 Jan 1998 18:13:36   OM
   Felbefehl-Dlg

      Rev 1.11   19 Dec 1997 18:25:00   OM
   Feldbefehl-bearbeiten Dlg

      Rev 1.10   16 Dec 1997 17:02:46   OM
   Feldbefehle bearbeiten

      Rev 1.9   12 Dec 1997 16:11:12   OM
   AutoUpdate bei FocusWechsel u.a.

      Rev 1.8   12 Dec 1997 10:49:38   OM
   ExchangeSupport implementiert

      Rev 1.7   09 Dec 1997 17:16:46   OM
   Kein alter Feldbefehl-Dialog mehr

      Rev 1.6   21 Nov 1997 17:19:42   OM
   Feldbefehl-Umstellung: DocInfo

      Rev 1.5   18 Nov 1997 10:33:10   OM
   Neuer Feldbefehldialog

      Rev 1.4   11 Nov 1997 10:03:40   OM
   Neuer Feldbefehl-Dialog

      Rev 1.3   04 Nov 1997 10:05:42   OM
   Neuer Felddialog

      Rev 1.2   30 Oct 1997 16:29:42   OM
   Feldbefehl-Umstellung

      Rev 1.1   30 Oct 1997 14:30:48   OM
   Feldbefehl-Umstellung

      Rev 1.0   28 Oct 1997 15:05:38   OM
   Initial revision.

------------------------------------------------------------------------*/
