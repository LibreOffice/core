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

#include "crsrsh.hxx"
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "viewimp.hxx"
#include "viewopt.hxx"
#include "flyfrm.hxx"
#include "frmfmt.hxx"
#include "layact.hxx"
#include "swregion.hxx"
#include "dflyobj.hxx"
#include "dview.hxx"
#include <tools/shl.hxx>
#include <swmodule.hxx>
#include <svx/svdpage.hxx>
#include <accmap.hxx>

// OD 12.12.2002 #103492#
#include <pagepreviewlayout.hxx>

#include <comcore.hrc>
#include <svx/svdundo.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentSettingAccess.hxx>

void SwViewImp::Init( const SwViewOption *pNewOpt )
{
    OSL_ENSURE( pDrawView, "SwViewImp::Init without DrawView" );
    //Create PageView if it doesn't exist
    SwRootFrm *pRoot = pSh->GetLayout();    //swmod 071108//swmod 071225
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
        pDrawView->SetAnimationEnabled( sal_False );

    pDrawView->SetUseIncompatiblePathCreateInterface( sal_False );

    // set handle size to 9 pixels, always
    pDrawView->SetMarkHdlSizePixel(9);
}

// SwViewImp::SwViewImp()  CTor for the core internals
SwViewImp::SwViewImp( ViewShell *pParent ) :
    pSh( pParent ),
    pDrawView( 0 ),
    pSdrPageView( 0 ),
    pFirstVisPage( 0 ),
    pRegion( 0 ),
    pLayAct( 0 ),
    pIdleAct( 0 ),
    pAccMap( 0 ),
    pSdrObjCached(NULL),
    nRestoreActions( 0 ),
    // OD 12.12.2002 #103492#
    mpPgPrevwLayout( 0 )
{
    bResetHdlHiddenPaint =
    bSmoothUpdate = bStopSmooth = bStopPrt = sal_False;
    bFirstPageInvalid = sal_True;
}


SwViewImp::~SwViewImp()
{
    delete pAccMap;

    // OD 12.12.2002 #103492#
    delete mpPgPrevwLayout;

    //JP 29.03.96: after ShowSdrPage  HideSdrPage must also be executed!!!
    if( pDrawView )
         pDrawView->HideSdrPage();

    delete pDrawView;

    DelRegion();

    OSL_ENSURE( !pLayAct, "Have action for the rest of your life." );
    OSL_ENSURE( !pIdleAct,"Be idle for the rest of your life." );
}


void SwViewImp::DelRegion()
{
    DELETEZ(pRegion);
}


sal_Bool SwViewImp::AddPaintRect( const SwRect &rRect )
{
    if ( rRect.IsOver( pSh->VisArea() ) )
    {
        if ( !pRegion )
            pRegion = new SwRegionRects( pSh->VisArea() );
        (*pRegion) -= rRect;
        return sal_True;
    }
    return sal_False;
}


void SwViewImp::CheckWaitCrsr()
{
    if ( pLayAct )
        pLayAct->CheckWaitCrsr();
}


sal_Bool SwViewImp::IsCalcLayoutProgress() const
{
    if ( pLayAct )
        return pLayAct->IsCalcLayout();
    return sal_False;
}


sal_Bool SwViewImp::IsUpdateExpFlds()
{
    if ( pLayAct && pLayAct->IsCalcLayout() )
    {
        pLayAct->SetUpdateExpFlds();
        return sal_True;
    }
     return sal_False;
}



void SwViewImp::SetFirstVisPage()
{
    if ( pSh->mbDocSizeChgd && pSh->VisArea().Top() > pSh->GetLayout()->Frm().Height() )
    {
        //We are in an action and because of erase actions the VisArea is
        //after the first visible page.
        //To avoid excessive formatting, hand back the last page.
        pFirstVisPage = (SwPageFrm*)pSh->GetLayout()->Lower();
        while ( pFirstVisPage && pFirstVisPage->GetNext() )
            pFirstVisPage = (SwPageFrm*)pFirstVisPage->GetNext();
    }
    else
    {
        const SwViewOption* pSwViewOption = GetShell()->GetViewOptions();
        const bool bBookMode = pSwViewOption->IsViewLayoutBookMode();

        SwPageFrm *pPage = (SwPageFrm*)pSh->GetLayout()->Lower();
        SwRect aPageRect = pPage->GetBoundRect();
        while ( pPage && !aPageRect.IsOver( pSh->VisArea() ) )
        {
            pPage = (SwPageFrm*)pPage->GetNext();
            if ( pPage )
            {
                aPageRect = pPage->GetBoundRect();
                if ( bBookMode && pPage->IsEmptyPage() )
                {
                    const SwPageFrm& rFormatPage = pPage->GetFormatPage();
                    aPageRect.SSize() = rFormatPage.GetBoundRect().SSize();
                }
            }
        }
        pFirstVisPage = pPage ? pPage : (SwPageFrm*)pSh->GetLayout()->Lower();
    }
    bFirstPageInvalid = sal_False;
}


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
                pOutDevForDrawView = GetShell()->GetOut();
            }

            pDrawView = new SwDrawView( *this, pIDDMA->GetDrawModel(), pOutDevForDrawView);
        }

        GetDrawView()->SetActiveLayer(OUString("Heaven"));
        const SwViewOption* pSwViewOption = GetShell()->GetViewOptions();
        Init(pSwViewOption);

        // #i68597# If document is read-only, we will not profit from overlay,
        // so switch it off.
        if(pDrawView && pDrawView->IsBufferedOverlayAllowed())
        {
            if(pSwViewOption->IsReadonly())
            {
                pDrawView->SetBufferedOverlayAllowed(false);
            }
        }
    }
}


Color SwViewImp::GetRetoucheColor() const
{
    Color aRet( COL_TRANSPARENT );
    const ViewShell &rSh = *GetShell();
    if ( rSh.GetWin() )
    {
        if ( rSh.GetViewOptions()->getBrowseMode() &&
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

// create page preview layout
void SwViewImp::InitPagePreviewLayout()
{
    OSL_ENSURE( pSh->GetLayout(), "no layout - page preview layout can not be created.");
    if ( pSh->GetLayout() )
        mpPgPrevwLayout = new SwPagePreviewLayout( *pSh, *(pSh->GetLayout()) );
}

void SwViewImp::UpdateAccessible()
{
    // We require a layout and an XModel to be accessible.
    IDocumentLayoutAccess* pIDLA = GetShell()->getIDocumentLayoutAccess();
    Window *pWin = GetShell()->GetWin();
    OSL_ENSURE( GetShell()->GetLayout(), "no layout, no access" );  //swmod 071108//swmod 071225
    OSL_ENSURE( pWin, "no window, no access" );

    if( IsAccessible() && pIDLA->GetCurrentViewShell() && pWin )    //swmod 071108//swmod 071225
        GetAccessibleMap().GetDocumentView();
}

void SwViewImp::DisposeAccessible( const SwFrm *pFrm,
                                   const SdrObject *pObj,
                                   sal_Bool bRecursive )
{
    OSL_ENSURE( !pFrm || pFrm->IsAccessibleFrm(), "frame is not accessible" );
    ViewShell *pVSh = GetShell();
    ViewShell *pTmp = pVSh;
    do
    {
        if( pTmp->Imp()->IsAccessible() )
            pTmp->Imp()->GetAccessibleMap().Dispose( pFrm, pObj, 0, bRecursive );
        pTmp = (ViewShell *)pTmp->GetNext();
    } while ( pTmp != pVSh );
}

void SwViewImp::MoveAccessible( const SwFrm *pFrm, const SdrObject *pObj,
                                const SwRect& rOldFrm )
{
    OSL_ENSURE( !pFrm || pFrm->IsAccessibleFrm(), "frame is not accessible" );
    ViewShell *pVSh = GetShell();
    ViewShell *pTmp = pVSh;
    do
    {
        if( pTmp->Imp()->IsAccessible() )
            pTmp->Imp()->GetAccessibleMap().InvalidatePosOrSize( pFrm, pObj, 0,
                                                                 rOldFrm );
        pTmp = (ViewShell *)pTmp->GetNext();
    } while ( pTmp != pVSh );
}

void SwViewImp::InvalidateAccessibleFrmContent( const SwFrm *pFrm )
{
    OSL_ENSURE( pFrm->IsAccessibleFrm(), "frame is not accessible" );
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

// invalidate CONTENT_FLOWS_FROM/_TO relation for paragraphs
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

// invalidate text selection for paragraphs
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

// invalidate attributes for paragraphs
void SwViewImp::_InvalidateAccessibleParaAttrs( const SwTxtFrm& rTxtFrm )
{
    ViewShell* pVSh = GetShell();
    ViewShell* pTmp = pVSh;
    do
    {
        if ( pTmp->Imp()->IsAccessible() )
        {
            pTmp->Imp()->GetAccessibleMap().InvalidateAttr( rTxtFrm );
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
    OSL_ENSURE( !pAccMap, "accessible map exists" );
    pAccMap = new SwAccessibleMap( GetShell() );
    return pAccMap;
}

void SwViewImp::FireAccessibleEvents()
{
    if( IsAccessible() )
        GetAccessibleMap().FireEvents();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
