/*************************************************************************
 *
 *  $RCSfile: viewimp.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:47:29 $
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
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifdef ACCESSIBLE_LAYOUT
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif
#endif

// OD 12.12.2002 #103492#
#ifndef _PAGEPREVIEWLAYOUT_HXX
#include <pagepreviewlayout.hxx>
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
    nRestoreActions( 0 ),
    // OD 12.12.2002 #103492#
    mpPgPrevwLayout( 0 )
#ifdef ACCESSIBLE_LAYOUT
    ,pAccMap( 0 )
#endif
{
    bResetXorVisibility = bShowHdlPaint =
    bResetHdlHiddenPaint = bScrolled =
    bPaintInScroll = bSmoothUpdate = bStopSmooth = bStopPrt = FALSE;
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
#ifdef ACCESSIBLE_LAYOUT
    delete pAccMap;
#endif

    // OD 12.12.2002 #103492#
    delete mpPgPrevwLayout;

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
        else if(rSh.GetViewOptions()->IsPagePreview()  &&
                    !SW_MOD()->GetAccessibilityOptions().GetIsForPagePreviews())
            aRet.SetColor(COL_WHITE);
        else
            aRet = SwViewOption::GetDocColor();
    }
    return aRet;
}

/** create page preview layout

    OD 12.12.2002 #103492#

    @author OD
*/
void SwViewImp::InitPagePreviewLayout()
{
    ASSERT( pSh->GetLayout(), "no layout - page preview layout can not be created.");
    if ( pSh->GetLayout() )
        mpPgPrevwLayout = new SwPagePreviewLayout( *pSh, *(pSh->GetLayout()) );
}

#ifdef ACCESSIBLE_LAYOUT
void SwViewImp::UpdateAccessible()
{
    // We require a layout and an XModel to be accessible.
    SwDoc *pDoc = GetShell()->GetDoc();
    Window *pWin = GetShell()->GetWin();
    ASSERT( pDoc->GetRootFrm(), "no layout, no access" );
    ASSERT( pWin, "no window, no access" );

    if( IsAccessible() && pDoc->GetRootFrm() && pWin )
        GetAccessibleMap().GetDocumentView();
}

void SwViewImp::DisposeAccessible( const SwFrm *pFrm,
                                   const SdrObject *pObj,
                                   sal_Bool bRecursive )
{
    ASSERT( !pFrm || pFrm->IsAccessibleFrm(), "frame is not accessible" );
    ViewShell *pVSh = GetShell();
    ViewShell *pTmp = pVSh;
    do
    {
        if( pTmp->Imp()->IsAccessible() )
            pTmp->Imp()->GetAccessibleMap().Dispose( pFrm, pObj, bRecursive );
        pTmp = (ViewShell *)pTmp->GetNext();
    } while ( pTmp != pVSh );
}

void SwViewImp::MoveAccessible( const SwFrm *pFrm, const SdrObject *pObj,
                                const SwRect& rOldFrm )
{
    ASSERT( !pFrm || pFrm->IsAccessibleFrm(), "frame is not accessible" );
    ViewShell *pVSh = GetShell();
    ViewShell *pTmp = pVSh;
    do
    {
        if( pTmp->Imp()->IsAccessible() )
            pTmp->Imp()->GetAccessibleMap().InvalidatePosOrSize( pFrm, pObj,
                                                                 rOldFrm );
        pTmp = (ViewShell *)pTmp->GetNext();
    } while ( pTmp != pVSh );
}

void SwViewImp::InvalidateAccessibleFrmContent( const SwFrm *pFrm )
{
    ASSERT( pFrm->IsAccessibleFrm(), "frame is not accessible" );
    ViewShell *pVSh = GetShell();
    ViewShell *pTmp = pVSh;
    do
    {
        if( pTmp->Imp()->IsAccessible() )
            pTmp->Imp()->GetAccessibleMap().InvalidateContent( pFrm );
        pTmp = (ViewShell *)pTmp->GetNext();
    } while ( pTmp != pVSh );
}

void SwViewImp::InvalidateAccessibleCursorPosition( const SwFrm *pFrm )
{
    if( IsAccessible() )
        GetAccessibleMap().InvalidateCursorPosition( pFrm );
}

void SwViewImp::InvalidateAccessibleEditableState( sal_Bool bAllShells,
                                                      const SwFrm *pFrm )
{
    if( bAllShells )
    {
        ViewShell *pVSh = GetShell();
        ViewShell *pTmp = pVSh;
        do
        {
            if( pTmp->Imp()->IsAccessible() )
                pTmp->Imp()->GetAccessibleMap().InvalidateStates( ACC_STATE_EDITABLE, pFrm );
            pTmp = (ViewShell *)pTmp->GetNext();
        } while ( pTmp != pVSh );
    }
    else if( IsAccessible() )
    {
        GetAccessibleMap().InvalidateStates( ACC_STATE_EDITABLE, pFrm );
    }
}

void SwViewImp::InvalidateAccessibleOpaqueState()
{
    ViewShell *pVSh = GetShell();
    ViewShell *pTmp = pVSh;
    do
    {
        if( pTmp->Imp()->IsAccessible() )
            pTmp->Imp()->GetAccessibleMap().InvalidateStates( ACC_STATE_OPAQUE );
        pTmp = (ViewShell *)pTmp->GetNext();
    } while ( pTmp != pVSh );
}

void SwViewImp::InvalidateAccessibleRelationSet( const SwFlyFrm *pMaster,
                                                 const SwFlyFrm *pFollow )
{
    ViewShell *pVSh = GetShell();
    ViewShell *pTmp = pVSh;
    do
    {
        if( pTmp->Imp()->IsAccessible() )
            pTmp->Imp()->GetAccessibleMap().InvalidateRelationSet( pMaster,
                                                                   pFollow );
        pTmp = (ViewShell *)pTmp->GetNext();
    } while ( pTmp != pVSh );
}

// OD 15.01.2003 #103492# - method signature change due to new page preview functionality
void SwViewImp::UpdateAccessiblePreview( const std::vector<PrevwPage*>& _rPrevwPages,
                                         const Fraction&  _rScale,
                                         const SwPageFrm* _pSelectedPageFrm,
                                         const Size&      _rPrevwWinSize )
{
    if( IsAccessible() )
        GetAccessibleMap().UpdatePreview( _rPrevwPages, _rScale,
                                          _pSelectedPageFrm, _rPrevwWinSize );
}

void SwViewImp::InvalidateAccessiblePreViewSelection( sal_uInt16 nSelPage )
{
    if( IsAccessible() )
        GetAccessibleMap().InvalidatePreViewSelection( nSelPage );
}

SwAccessibleMap *SwViewImp::CreateAccessibleMap()
{
    ASSERT( !pAccMap, "accessible map exists" )
    pAccMap = new SwAccessibleMap( GetShell() );
    return pAccMap;
}

void SwViewImp::FireAccessibleEvents()
{
    if( IsAccessible() )
        GetAccessibleMap().FireEvents();
}

IMPL_LINK(SwViewImp, SetStopPrt, void *, EMPTYARG)
{
    bStopPrt = TRUE;

    return 0;
}

#endif
