/*************************************************************************
 *
 *  $RCSfile: edtwin3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:35 $
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

#ifndef _SV_SETTINGS_HXX
#include <vcl/settings.hxx>
#endif
#ifndef _SVX_RULER_HXX //autogen
#include <svx/ruler.hxx>
#endif

#ifndef _VIEWOPT_HXX //autogen
#include <viewopt.hxx>
#endif
#include "view.hxx"
#include "wrtsh.hxx"
#include "basesh.hxx"
#include "pview.hxx"
#include "mdiexp.hxx"
#include "edtwin.hxx"
#include "swmodule.hxx"
#include "modcfg.hxx"
#include "swtable.hxx"
#include "docsh.hxx"


/*--------------------------------------------------------------------
    Beschreibung:   Core-Notify
 --------------------------------------------------------------------*/



void ScrollMDI( ViewShell* pVwSh, const SwRect &rRect,
                USHORT nRangeX, USHORT nRangeY)
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && pSfxVwSh->ISA(SwView))
        ((SwView *)pSfxVwSh)->Scroll( rRect.SVRect(), nRangeX, nRangeY );
}

/*--------------------------------------------------------------------
    Beschreibung:   Docmdi - verschiebbar
 --------------------------------------------------------------------*/



BOOL IsScrollMDI( ViewShell* pVwSh, const SwRect &rRect )
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && pSfxVwSh->ISA(SwView))
        return (((SwView *)pSfxVwSh)->IsScroll(rRect.SVRect()));
    return FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung:   Notify fuer Groessen-Aenderung
 --------------------------------------------------------------------*/



void SizeNotify(ViewShell* pVwSh, const Size &rSize)
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh)
    {
        if (pSfxVwSh->ISA(SwView))
            ((SwView *)pSfxVwSh)->DocSzChgd(rSize);
        else if (pSfxVwSh->ISA(SwPagePreView))
            ((SwPagePreView *)pSfxVwSh)->DocSzChgd( rSize );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Notify fuer Seitenzahl-Update
 --------------------------------------------------------------------*/



void PageNumNotify( ViewShell* pVwSh, USHORT nPhyNum, USHORT nVirtNum,
                                                    const String& rPgStr)
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if ( pSfxVwSh && pSfxVwSh->ISA(SwView) &&
         ((SwView*)pSfxVwSh)->GetCurShell() )
            ((SwView *)pSfxVwSh)->UpdatePageNums(nPhyNum, nVirtNum, rPgStr);
}

/******************************************************************************
 *  Methode     :   void FrameNotify( DocMDIBase *pWin, FlyMode eMode )
 *  Beschreibung:
 *  Erstellt    :   OK 08.02.94 13:49
 *  Aenderung   :
 ******************************************************************************/



void FrameNotify( ViewShell* pVwSh, FlyMode eMode )
{
    if ( pVwSh->ISA(SwCrsrShell) )
        SwBaseShell::SetFrmMode( eMode, (SwWrtShell*)pVwSh );
}

/*--------------------------------------------------------------------
    Beschreibung:   Notify fuer Seitenzahl-Update
 --------------------------------------------------------------------*/



BOOL SwEditWin::RulerClook( SwView& rView , const MouseEvent& rMEvt)
{
    return (!rView.GetHLineal()->StartDocDrag( rMEvt, RULER_TYPE_BORDER ) &&
            !rView.GetHLineal()->StartDocDrag( rMEvt, RULER_TYPE_MARGIN1) &&
            !rView.GetHLineal()->StartDocDrag( rMEvt, RULER_TYPE_MARGIN2));
}



Dialog* GetSearchDialog()
{
    return SwView::GetSearchDialog();
}



void JavaScriptScrollMDI( SfxFrame* pFrame, INT32 nX, INT32 nY )
{
    SfxViewShell *pSfxVwSh = pFrame->GetCurrentViewFrame()->GetViewShell();
    if( pSfxVwSh && pSfxVwSh->ISA( SwView ))
    {
        SwView* pView = (SwView *)pSfxVwSh;

        Size aSz( nX, nY );
        aSz = pView->GetEditWin().PixelToLogic( aSz );

        Point aTopLeft( aSz.Width(), aSz.Height() );
        if( aTopLeft.X() < DOCUMENTBORDER ) aTopLeft.X() = DOCUMENTBORDER;
        if( aTopLeft.Y() < DOCUMENTBORDER ) aTopLeft.Y() = DOCUMENTBORDER;

        const Size& rVisSize = pView->GetVisArea().GetSize();
        Size aDocSize( pView->GetDocSz() );
        aDocSize.Width() += DOCUMENTBORDER;
        aDocSize.Height() += DOCUMENTBORDER;

        if( aTopLeft.X() + rVisSize.Width() > aDocSize.Width() )
            aTopLeft.X() = rVisSize.Width() > aDocSize.Width()
                                ? DOCUMENTBORDER
                                : aDocSize.Width() - rVisSize.Width();

        if( aTopLeft.Y() + rVisSize.Height() > aDocSize.Height() )
            aTopLeft.Y() = rVisSize.Height() > aDocSize.Height()
                                ? DOCUMENTBORDER
                                : aDocSize.Height() - rVisSize.Height();

        pView->SetVisArea( aTopLeft );
    }
}



USHORT GetTblChgDefaultMode()
{
    SwModuleOptions* pOpt = SW_MOD()->GetModuleConfig();
    return pOpt ? pOpt->GetTblMode() : TBLVAR_CHGABS;
}



void RepaintPagePreview( ViewShell* pVwSh, const SwRect& rRect )
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if (pSfxVwSh && pSfxVwSh->ISA( SwPagePreView ))
        ((SwPagePreView *)pSfxVwSh)->RepaintCoreRect( rRect );
}

BOOL JumpToSwMark( ViewShell* pVwSh, const String& rMark )
{
    SfxViewShell *pSfxVwSh = pVwSh->GetSfxViewShell();
    if( pSfxVwSh && pSfxVwSh->ISA( SwView ) )
        return ((SwView *)pSfxVwSh)->JumpToSwMark( rMark );
    return FALSE;
}

void SwEditWin::DataChanged( const DataChangedEvent &rEvt )
{
    SwWrtShell &rSh = GetView().GetWrtShell();
    rSh.LockView( TRUE );
    rSh.LockPaint();
    GetView().InvalidateBorder();               //Scrollbarbreiten
    GetView().GetDocShell()->UpdateFontList();  //z.B. Druckerwechsel
    rSh.LockView( FALSE );
    rSh.UnlockPaint();
}


/***********************************************************************

        $Log: not supported by cvs2svn $
        Revision 1.26  2000/09/18 16:05:24  willem.vandorp
        OpenOffice header added.

        Revision 1.25  1999/03/10 11:00:28  MA
        #52642# Fontwechsel im DataChanged


      Rev 1.24   10 Mar 1999 12:00:28   MA
   #52642# Fontwechsel im DataChanged

      Rev 1.23   24 Jun 1998 18:43:10   MA
   DataChanged fuer ScrollBar und Retouche, Retouche ganz umgestellt

      Rev 1.22   21 Apr 1998 15:25:14   JP
   Bug #49438#: JavaScriptScrollMDI - Grenzen beachten, absolut positionieren

      Rev 1.21   25 Nov 1997 10:33:04   MA
   includes

      Rev 1.20   03 Nov 1997 13:14:06   MA
   precomp entfernt

      Rev 1.19   17 Sep 1997 12:34:22   JP
   neu: JumpToSwMark - springe eine Marke an

      Rev 1.18   01 Sep 1997 13:15:38   OS
   DLL-Umstellung

      Rev 1.17   17 Jun 1997 15:47:06   MA
   DrawTxtShell nicht von BaseShell ableiten + Opts

      Rev 1.16   06 Jun 1997 11:08:40   OS
   Seitennummer mit richtigem NumType anzeigen

      Rev 1.15   11 Mar 1997 15:56:10   MA
   fix: FrameNotify, SubShell vorhanden?

      Rev 1.14   09 Feb 1997 20:58:10   JP
   Bug #35760#: beim Core-Repaint die Bereiche der PagePreView mitteilen

      Rev 1.13   15 Jan 1997 15:04:42   JP
   default des TablenChgMode aus dem ConfigItem holen

      Rev 1.12   25 Nov 1996 11:22:06   JP
   neu: JavaScriptScrollMDI

      Rev 1.11   28 Aug 1996 11:25:48   OS
   includes

      Rev 1.10   14 Dec 1995 17:32:16   OS
   Search wieder an der view

      Rev 1.9   30 Nov 1995 16:14:00   OS
   SearchDialog kommt von der App

      Rev 1.8   30 Nov 1995 13:25:32   MA
   del: IsWinForPagePreview() entfernt

      Rev 1.7   24 Nov 1995 16:58:36   OM
   PCH->PRECOMPILED

      Rev 1.6   30 Oct 1995 18:42:04   OM
   GetData und GetViewWin entfernt

      Rev 1.5   25 Sep 1995 21:55:56   JP
   PageNumNotify kann auch kommen, wenn noch keine Shell gesetzt ist (Bug19864)

      Rev 1.4   11 Sep 1995 11:04:20   JP
   neu: GetSearchDialog - CrsrShell braucht den Pointer fuer die QueryBox

      Rev 1.3   17 Aug 1995 08:38:52   OS
   +RulerClook

      Rev 1.2   14 Jul 1995 19:19:30   ER
   segprag

      Rev 1.1   11 Jun 1995 18:57:36   JP
   neu: IsWinForPagePreview - Window von der SeitenAnsicht?

      Rev 1.0   28 Mar 1995 09:51:24   SWG
   Initial revision.

**********************************************************************/



