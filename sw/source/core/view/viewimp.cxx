/*************************************************************************
 *
 *  $RCSfile: viewimp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:29 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "scrrect.hxx"
#include "doc.hxx"
#include "crsrsh.hxx"
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "viewimp.hxx"
#include "errhdl.hxx"
#include "viewopt.hxx"
#include "flyfrm.hxx"
#include "frmfmt.hxx"
#include "layact.hxx"
#include "swregion.hxx"
#include "dflyobj.hxx"
#include "dview.hxx"

#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif

/*************************************************************************
|*
|*  SwViewImp::Init()
|*
|*  Ersterstellung      MA 25. Jul. 94
|*  Letzte Aenderung    MA 03. Nov. 95
|*
|*************************************************************************/

void SwViewImp::Init( const SwViewOption *pNewOpt )
{
    ASSERT( pDrawView, "SwViewImp::Init without DrawView" );
    //Jetzt die PageView erzeugen wenn sie noch nicht existiert.
    SwRootFrm *pRoot = pSh->GetDoc()->GetRootFrm();
    if ( !pSdrPageView )
    {
        if ( !pRoot->GetDrawPage() )
            pRoot->SetDrawPage( pSh->GetDoc()->GetDrawModel()->GetPage( 0 ) );

        pRoot->GetDrawPage()->SetSize( pRoot->Frm().SSize() );
         pSdrPageView = pDrawView->ShowPage( pRoot->GetDrawPage(), Point());
    }
    pDrawView->SetDragStripes( pNewOpt->IsCrossHair() );
    pDrawView->SetGridSnap( pNewOpt->IsSnap() );
    pDrawView->SetGridVisible( pNewOpt->IsGridVisible() );
    const Size &rSz = pNewOpt->GetSnapSize();
    pDrawView->SetGridCoarse( rSz );
    const Size aFSize
            ( rSz.Width() ? rSz.Width() /Max(short(1),pNewOpt->GetDivisionX()):0,
              rSz.Height()? rSz.Height()/Max(short(1),pNewOpt->GetDivisionY()):0);
     pDrawView->SetGridFine( aFSize );
     pDrawView->SetSnapGrid( aFSize );
    Fraction aSnGrWdtX(rSz.Width(), pNewOpt->GetDivisionX() + 1);
    Fraction aSnGrWdtY(rSz.Height(), pNewOpt->GetDivisionY() + 1);
    pDrawView->SetSnapGridWidth( aSnGrWdtX, aSnGrWdtY );

    //Ersatzdarstellung
    FASTBOOL bDraw = !pNewOpt->IsDraw();
    pDrawView->SetLineDraft( bDraw );
    pDrawView->SetFillDraft( bDraw );
    pDrawView->SetGrafDraft( bDraw );
    pDrawView->SetTextDraft( bDraw );

    if ( pRoot->Frm().HasArea() )
        pDrawView->SetWorkArea( pRoot->Frm().SVRect() );

    if ( GetShell()->IsPreView() )
        pDrawView->SetAnimationEnabled( FALSE );

    pDrawView->SetUseIncompatiblePathCreateInterface( FALSE );
    pDrawView->SetSolidMarkHdl(pNewOpt->IsSolidMarkHdl());

    // it's a JOE interface !
    pDrawView->SetMarkHdlSizePixel(pNewOpt->IsBigMarkHdl() ? 9 : 7);
}

/*************************************************************************
|*
|*  SwViewImp::SwViewImp()  CTor fuer die Core-Internas
|*
|*  Ersterstellung      MA 25. Jul. 94
|*  Letzte Aenderung    MA 06. Sep. 96
|*
|*************************************************************************/

SwViewImp::SwViewImp( ViewShell *pParent ) :
    pSh( pParent ),
    pFirstVisPage( 0 ),
    pRegion( 0 ),
    pScrollRects( 0 ),
    pScrolledArea( 0 ),
    pLayAct( 0 ),
    pIdleAct( 0 ),
    pSdrPageView( 0 ),
    pDrawView( 0 ),
    nRestoreActions( 0 )
{
    bResetXorVisibility = bShowHdlPaint =
    bResetHdlHiddenPaint = bScrolled =
    bPaintInScroll = bSmoothUpdate = bStopSmooth = FALSE;
    bFirstPageInvalid = bScroll = bNextScroll = TRUE;

    aScrollTimer.SetTimeout( 1500 );
    aScrollTimer.SetTimeoutHdl( LINK( this, SwViewImp, RefreshScrolledHdl));
    aScrollTimer.Stop();
}

/******************************************************************************
|*
|*  SwViewImp::~SwViewImp()
|*
|*  Ersterstellung      MA 25. Jul. 94
|*  Letzte Aenderung    MA 16. Dec. 94
|*
******************************************************************************/

SwViewImp::~SwViewImp()
{
    //JP 29.03.96: nach ShowPage muss auch HidePage gemacht werden!!!
    if( pDrawView )
         pDrawView->HidePage( pSdrPageView );

    delete pDrawView;

    DelRegions();
    delete pScrolledArea;

    ASSERT( !pLayAct, "Have action for the rest of your life." );
    ASSERT( !pIdleAct,"Be idle for the rest of your life." );
}

/******************************************************************************
|*
|*  SwViewImp::DelRegions()
|*
|*  Ersterstellung      MA 14. Apr. 94
|*  Letzte Aenderung    MA 14. Apr. 94
|*
******************************************************************************/

void SwViewImp::DelRegions()
{
    DELETEZ(pRegion);
    DELETEZ(pScrollRects);
}

/******************************************************************************
|*
|*  SwViewImp::AddPaintRect()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 27. Jul. 94
|*
******************************************************************************/

BOOL SwViewImp::AddPaintRect( const SwRect &rRect )
{
    if ( rRect.IsOver( pSh->VisArea() ) )
    {
        if ( !pRegion )
            pRegion = new SwRegionRects( pSh->VisArea() );
        (*pRegion) -= rRect;
        return TRUE;
    }
    return FALSE;
}


/******************************************************************************
|*
|*  ViewImp::CheckWaitCrsr()
|*
|*  Ersterstellung      MA 10. Aug. 94
|*  Letzte Aenderung    MA 10. Aug. 94
|*
******************************************************************************/

void SwViewImp::CheckWaitCrsr()
{
    if ( pLayAct )
        pLayAct->CheckWaitCrsr();
}

/******************************************************************************
|*
|*  ViewImp::IsCalcLayoutProgress()
|*
|*  Ersterstellung      MA 12. Aug. 94
|*  Letzte Aenderung    MA 12. Aug. 94
|*
******************************************************************************/

BOOL SwViewImp::IsCalcLayoutProgress() const
{
    if ( pLayAct )
        return pLayAct->IsCalcLayout();
    return FALSE;
}

/******************************************************************************
|*
|*  ViewImp::IsUpdateExpFlds()
|*
|*  Ersterstellung      MA 28. Mar. 96
|*  Letzte Aenderung    MA 28. Mar. 96
|*
******************************************************************************/

BOOL SwViewImp::IsUpdateExpFlds()
{
    if ( pLayAct && pLayAct->IsCalcLayout() )
    {
        pLayAct->SetUpdateExpFlds();
        return TRUE;
    }
     return FALSE;
}


/******************************************************************************
|*
|*  SwViewImp::SetFirstVisPage(), ImplGetFirstVisPage();
|*
|*  Ersterstellung      MA 21. Sep. 93
|*  Letzte Aenderung    MA 08. Mar. 94
|*
******************************************************************************/

void SwViewImp::SetFirstVisPage()
{
    if ( pSh->bDocSizeChgd && pSh->VisArea().Top() > pSh->GetLayout()->Frm().Height() )
    {
        //Wir stecken in einer Action und die VisArea sitzt wegen
        //Loeschoperationen hinter der erste sichtbaren Seite.
        //Damit nicht zu heftig Formatiert wird, liefern wir die letzte Seite
        //zurueck.
        pFirstVisPage = (SwPageFrm*)pSh->GetLayout()->Lower();
        while ( pFirstVisPage && pFirstVisPage->GetNext() )
            pFirstVisPage = (SwPageFrm*)pFirstVisPage->GetNext();
    }
    else
    {
        SwPageFrm *pPage = (SwPageFrm*)pSh->GetLayout()->Lower();
        while ( pPage && !pPage->Frm().IsOver( pSh->VisArea() ) )
            pPage = (SwPageFrm*)pPage->GetNext();
        pFirstVisPage = pPage ? pPage : (SwPageFrm*)pSh->GetLayout()->Lower();
    }
    bFirstPageInvalid = FALSE;
}

/******************************************************************************
|*
|*  SwViewImp::MakeDrawView();
|*
|*  Ersterstellung      AMA 01. Nov. 95
|*  Letzte Aenderung    AMA 01. Nov. 95
|*
******************************************************************************/

void SwViewImp::MakeDrawView()
{
    if( !GetShell()->GetDoc()->GetDrawModel() )
        GetShell()->GetDoc()->_MakeDrawModel();
    else
    {
        if ( !pDrawView )
        {
            pDrawView = new SwDrawView( *this,
                        GetShell()->GetDoc()->GetDrawModel(),
                           GetShell()->GetWin() ?
                            GetShell()->GetWin() :
                            (OutputDevice*)GetShell()->GetDoc()->GetPrt() );
        }
        GetDrawView()->SetActiveLayer( XubString::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM( "Heaven" ) ) );
        Init( GetShell()->GetViewOptions() );
    }
}

/******************************************************************************
|*
|*  SwViewImp::GetRetoucheColor()
|*
|*  Ersterstellung      MA 24. Jun. 98
|*  Letzte Aenderung    MA 24. Jun. 98
|*
******************************************************************************/

Color SwViewImp::GetRetoucheColor() const
{
    Color aRet( COL_TRANSPARENT );
    const ViewShell &rSh = *GetShell();
    if ( rSh.GetWin() )
    {
        if ( rSh.GetDoc()->IsBrowseMode() &&
             COL_TRANSPARENT != rSh.GetViewOptions()->GetRetoucheColor().GetColor() )
            aRet = rSh.GetViewOptions()->GetRetoucheColor();
        else
            aRet = rSh.GetWin()->GetSettings().GetStyleSettings().GetWindowColor();
    }
    return aRet;
}

/************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.96  2000/09/18 16:04:37  willem.vandorp
      OpenOffice header added.

      Revision 1.95  2000/07/17 10:31:08  ama
      Opt: Smarter scrolling for RVP

      Revision 1.94  2000/05/09 11:35:43  ama
      Unicode changes

      Revision 1.93  2000/04/27 07:37:23  os
      UNICODE

      Revision 1.92  2000/03/03 15:17:22  os
      StarView remainders removed

      Revision 1.91  2000/02/11 14:36:08  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.90  1999/09/22 12:37:10  os
      big handles

      Revision 1.89  1999/09/06 13:19:00  aw
      changes due to support of new handles


      Rev 1.85   13 Aug 1999 15:11:32   MA
   adoption to new markers, but still inkomplete

      Rev 1.84   08 Apr 1999 12:36:28   MA
   #64467# Complete und Retouche ggf. zuruecksetzen

      Rev 1.83   30 Mar 1999 17:11:30   AW
   #41275# changed handling of Snap-functionality

      Rev 1.82   16 Jul 1998 18:55:36   AMA
   Fix #50348#51949#: Controls bei virtuellen Outputdevice extra painten

      Rev 1.81   07 Jul 1998 13:23:02   OS
   alle Actions fuer layoutabhaengige UNO-Operationen kurzfristig aufheben

      Rev 1.80   24 Jun 1998 18:45:22   MA
   DataChanged fuer ScrollBar und Retouche, Retouche ganz umgestellt

      Rev 1.79   03 Jun 1998 09:25:20   MA
   #50392# Handles und Xor aufgeraeumt

      Rev 1.78   27 Apr 1998 15:09:12   MA
   ein paar sv2vcl

      Rev 1.77   28 Jan 1998 13:40:52   MA
   ueberfluessiges Hell-Paint vom Text entfernt

      Rev 1.76   28 Nov 1997 09:08:46   MA
   includes

      Rev 1.75   03 Nov 1997 13:07:30   MA
   precomp entfernt

      Rev 1.74   21 Oct 1997 14:10:24   MA
   #44844# VirDev zu klein

      Rev 1.73   13 Oct 1997 10:30:18   MA
   Umbau/Vereinfachung Paint

      Rev 1.72   15 Aug 1997 12:24:06   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.71   12 Aug 1997 15:08:52   MH
   chg: header

      Rev 1.70   07 Jul 1997 16:33:42   OS
   SnapSize auch im Init auf den gleichen Wert wie SetGridFine setzen #41380#

      Rev 1.69   11 Jun 1997 12:58:40   MH
   add: include Win16

      Rev 1.68   04 Apr 1997 17:37:12   NF
   includes

      Rev 1.67   25 Mar 1997 14:46:28   MA
   Smooth

      Rev 1.66   23 Mar 1997 13:08:42   MA
   new: bPainInScroll

      Rev 1.65   25 Feb 1997 09:10:54   MA
   chg: Option fuer SolidHdl

      Rev 1.64   25 Feb 1997 08:45:44   MA
   chg: SolidHdl ueberm Berg

      Rev 1.63   19 Feb 1997 11:04:04   MA
   chg: neue huebsche Handles

      Rev 1.62   05 Feb 1997 09:17:14   MA
   fix: Refresh, virtuelles Device hinreichend gross einstellen

      Rev 1.61   23 Jan 1997 14:21:42   OM
   Klickverhalten von polygonen geaendert

      Rev 1.60   16 Jan 1997 17:34:34   MA
   chg: Paint oder nicht sagt uns jetzt SwFlyFrm::IsPaint

*************************************************************************/


