/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewimp.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:00:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include "scrrect.hxx"
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
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif

// OD 12.12.2002 #103492#
#ifndef _PAGEPREVIEWLAYOUT_HXX
#include <pagepreviewlayout.hxx>
#endif

#include <comcore.hrc>
#include <svx/svdundo.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentSettingAccess.hxx>

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
    SwRootFrm *pRoot = pSh->getIDocumentLayoutAccess()->GetRootFrm();
    if ( !pSdrPageView )
    {
        IDocumentDrawModelAccess* pIDDMA = pSh->getIDocumentDrawModelAccess();
        if ( !pRoot->GetDrawPage() )
            pRoot->SetDrawPage( pIDDMA->GetDrawModel()->GetPage( 0 ) );

        if ( pRoot->GetDrawPage()->GetSize() != pRoot->Frm().SSize() )
            pRoot->GetDrawPage()->SetSize( pRoot->Frm().SSize() );

        pSdrPageView = pDrawView->ShowSdrPage( pRoot->GetDrawPage());
        // OD 26.06.2003 #108784# - notify drawing page view about invisible
        // layers.
        pIDDMA->NotifyInvisibleLayers( *pSdrPageView );
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
    Fraction aSnGrWdtX(rSz.Width(), pNewOpt->GetDivisionX() + 1);
    Fraction aSnGrWdtY(rSz.Height(), pNewOpt->GetDivisionY() + 1);
    pDrawView->SetSnapGridWidth( aSnGrWdtX, aSnGrWdtY );

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
    pDrawView( 0 ),
    pSdrPageView( 0 ),
    pFirstVisPage( 0 ),
    pRegion( 0 ),
    pScrollRects( 0 ),
    pScrolledArea( 0 ),
    pLayAct( 0 ),
    pIdleAct( 0 ),
    pAccMap( 0 ),
    pSdrObjCached(NULL),
    nRestoreActions( 0 ),
    // OD 12.12.2002 #103492#
    mpPgPrevwLayout( 0 )
{
    //bResetXorVisibility =
    //HMHbShowHdlPaint =
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
    delete pAccMap;

    // OD 12.12.2002 #103492#
    delete mpPgPrevwLayout;

    //JP 29.03.96: nach ShowSdrPage muss auch HideSdrPage gemacht werden!!!
    if( pDrawView )
         pDrawView->HideSdrPage();

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
        const SwViewOption* pSwViewOption = GetShell()->GetViewOptions();
        const bool bBookMode = pSwViewOption->IsViewLayoutBookMode();

        SwPageFrm *pPage = (SwPageFrm*)pSh->GetLayout()->Lower();
        SwRect aPageRect = pPage->Frm();
        while ( pPage && !aPageRect.IsOver( pSh->VisArea() ) )
        {
            pPage = (SwPageFrm*)pPage->GetNext();
            if ( pPage )
            {
                aPageRect = pPage->Frm();
                if ( bBookMode && pPage->IsEmptyPage() )
                {
                    const SwPageFrm& rFormatPage = pPage->GetFormatPage();
                    aPageRect.SSize() = rFormatPage.Frm().SSize();
                }
            }
        }
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
    IDocumentDrawModelAccess* pIDDMA = GetShell()->getIDocumentDrawModelAccess();

    // the else here is not an error, _MakeDrawModel() calls this method again
    // after the DrawModel is created to create DrawViews for all shells...
    if( !pIDDMA->GetDrawModel() )
    {
        pIDDMA->_MakeDrawModel();
    }
    else
    {
        if ( !pDrawView )
        {
            // #i72809#
            // Discussed with FME, he also thinks that the getPrinter is old and not correct. When i got
            // him right, it anyways returns GetOut() when it's a printer, but NULL when not. He suggested
            // to use GetOut() and check the existing cases.
            // Check worked well. Took a look at viewing, printing, PDF export and print preview with a test
            // document which has an empty 2nd page (right page, see bug)
            OutputDevice* pOutDevForDrawView = GetShell()->GetWin();

            if(!pOutDevForDrawView)
            {
                // pOutDevForDrawView = (OutputDevice*)GetShell()->getIDocumentDeviceAccess()->getPrinter( false );
                pOutDevForDrawView = GetShell()->GetOut();
            }

            pDrawView = new SwDrawView( *this, pIDDMA->GetDrawModel(), pOutDevForDrawView);
        }

        GetDrawView()->SetActiveLayer( XubString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Heaven" ) ) );
        const SwViewOption* pSwViewOption = GetShell()->GetViewOptions();
        Init(pSwViewOption);

        // #i68597# If document is read-only, we will not profit from overlay,
        // so switch it off.
        if(pDrawView && pDrawView->IsBufferedOverlayAllowed())
        {
            bool bIsReadOnly(pSwViewOption->IsReadonly());

#ifdef DBG_UTIL
            // add test possibilities
            static bool bAlwaysActivateForTest(false);
            if(bAlwaysActivateForTest && bIsReadOnly)
            {
                bIsReadOnly = false;
            }
#endif

            if(bIsReadOnly)
            {
                pDrawView->SetBufferedOverlayAllowed(false);
            }
        }
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
        if ( rSh.getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) &&
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

void SwViewImp::UpdateAccessible()
{
    // We require a layout and an XModel to be accessible.
    IDocumentLayoutAccess* pIDLA = GetShell()->getIDocumentLayoutAccess();
    Window *pWin = GetShell()->GetWin();
    ASSERT( pIDLA->GetRootFrm(), "no layout, no access" );
    ASSERT( pWin, "no window, no access" );

    if( IsAccessible() && pIDLA->GetRootFrm() && pWin )
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

 /** invalidate CONTENT_FLOWS_FROM/_TO relation for paragraphs

    OD 2005-12-01 #i27138#

    @author OD
*/
void SwViewImp::_InvalidateAccessibleParaFlowRelation( const SwTxtFrm* _pFromTxtFrm,
                                                       const SwTxtFrm* _pToTxtFrm )
{
    if ( !_pFromTxtFrm && !_pToTxtFrm )
    {
        // No text frame provided. Thus, nothing to do.
        return;
    }

    ViewShell* pVSh = GetShell();
    ViewShell* pTmp = pVSh;
    do
    {
        if ( pTmp->Imp()->IsAccessible() )
        {
            if ( _pFromTxtFrm )
            {
                pTmp->Imp()->GetAccessibleMap().
                            InvalidateParaFlowRelation( *_pFromTxtFrm, true );
            }
            if ( _pToTxtFrm )
            {
                pTmp->Imp()->GetAccessibleMap().
                            InvalidateParaFlowRelation( *_pToTxtFrm, false );
            }
        }
        pTmp = (ViewShell *)pTmp->GetNext();
    } while ( pTmp != pVSh );
}

/** invalidate text selection for paragraphs

    OD 2005-12-12 #i27301#

    @author OD
*/
void SwViewImp::_InvalidateAccessibleParaTextSelection()
{
    ViewShell* pVSh = GetShell();
    ViewShell* pTmp = pVSh;
    do
    {
        if ( pTmp->Imp()->IsAccessible() )
        {
            pTmp->Imp()->GetAccessibleMap().InvalidateTextSelectionOfAllParas();
        }

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

