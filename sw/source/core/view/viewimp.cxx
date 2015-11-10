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
#include <swmodule.hxx>
#include <svx/svdpage.hxx>
#include <accmap.hxx>

#include <pagepreviewlayout.hxx>
#include <comcore.hrc>
#include <svx/svdundo.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <drawdoc.hxx>

void SwViewShellImp::Init( const SwViewOption *pNewOpt )
{
    OSL_ENSURE( m_pDrawView, "SwViewShellImp::Init without DrawView" );
    //Create PageView if it doesn't exist
    SwRootFrm *pRoot = m_pShell->GetLayout();
    if ( !m_pSdrPageView )
    {
        IDocumentDrawModelAccess& rIDDMA = m_pShell->getIDocumentDrawModelAccess();
        if ( !pRoot->GetDrawPage() )
            pRoot->SetDrawPage( rIDDMA.GetDrawModel()->GetPage( 0 ) );

        if ( pRoot->GetDrawPage()->GetSize() != pRoot->Frm().SSize() )
            pRoot->GetDrawPage()->SetSize( pRoot->Frm().SSize() );

        m_pSdrPageView = m_pDrawView->ShowSdrPage( pRoot->GetDrawPage());
        // OD 26.06.2003 #108784# - notify drawing page view about invisible
        // layers.
        rIDDMA.NotifyInvisibleLayers( *m_pSdrPageView );
    }
    m_pDrawView->SetDragStripes( pNewOpt->IsCrossHair() );
    m_pDrawView->SetGridSnap( pNewOpt->IsSnap() );
    m_pDrawView->SetGridVisible( pNewOpt->IsGridVisible() );
    const Size &rSz = pNewOpt->GetSnapSize();
    m_pDrawView->SetGridCoarse( rSz );
    const Size aFSize
            ( rSz.Width() ? rSz.Width() /std::max(short(1),pNewOpt->GetDivisionX()):0,
              rSz.Height()? rSz.Height()/std::max(short(1),pNewOpt->GetDivisionY()):0);
     m_pDrawView->SetGridFine( aFSize );
    Fraction aSnGrWdtX(rSz.Width(), pNewOpt->GetDivisionX() + 1);
    Fraction aSnGrWdtY(rSz.Height(), pNewOpt->GetDivisionY() + 1);
    m_pDrawView->SetSnapGridWidth( aSnGrWdtX, aSnGrWdtY );

    if ( pRoot->Frm().HasArea() )
        m_pDrawView->SetWorkArea( pRoot->Frm().SVRect() );

    if ( GetShell()->IsPreview() )
        m_pDrawView->SetAnimationEnabled( false );

    m_pDrawView->SetUseIncompatiblePathCreateInterface( false );

    // set handle size to 9 pixels, always
    m_pDrawView->SetMarkHdlSizePixel(9);
}

/// CTor for the core internals
SwViewShellImp::SwViewShellImp( SwViewShell *pParent ) :
    m_pShell( pParent ),
    m_pDrawView( nullptr ),
    m_pSdrPageView( nullptr ),
    m_pFirstVisiblePage( nullptr ),
    m_pRegion( nullptr ),
    m_pLayAction( nullptr ),
    m_pIdleAct( nullptr ),
    m_pAccessibleMap( nullptr ),
    m_pSdrObjectCached(nullptr),
    m_bFirstPageInvalid( true ),
    m_bResetHdlHiddenPaint( false ),
    m_bSmoothUpdate( false ),
    m_bStopSmooth( false ),
    m_nRestoreActions( 0 ),
    // OD 12.12.2002 #103492#
    m_pPagePreviewLayout( nullptr )
{
}

SwViewShellImp::~SwViewShellImp()
{
    delete m_pAccessibleMap;

    // OD 12.12.2002 #103492#
    delete m_pPagePreviewLayout;

    //JP 29.03.96: after ShowSdrPage  HideSdrPage must also be executed!!!
    if( m_pDrawView )
         m_pDrawView->HideSdrPage();

    delete m_pDrawView;

    DelRegion();

    OSL_ENSURE( !m_pLayAction, "Have action for the rest of your life." );
    OSL_ENSURE( !m_pIdleAct,"Be idle for the rest of your life." );
}

void SwViewShellImp::DelRegion()
{
    DELETEZ(m_pRegion);
}

bool SwViewShellImp::AddPaintRect( const SwRect &rRect )
{
    // In case of tiled rendering the visual area is the last painted tile -> not interesting.
    if ( rRect.IsOver( m_pShell->VisArea() ) || m_pShell->isTiledRendering() )
    {
        if ( !m_pRegion )
        {
            // In case of normal rendering, this makes sure only visible rectangles are painted.
            // Otherwise get the rectangle of the full document, so all paint rectangles are invalidated.
            const SwRect& rArea = m_pShell->isTiledRendering() ? m_pShell->GetLayout()->Frm() : m_pShell->VisArea();
            m_pRegion = new SwRegionRects( rArea );
        }
        (*m_pRegion) -= rRect;
        return true;
    }
    return false;
}

void SwViewShellImp::CheckWaitCrsr()
{
    if ( m_pLayAction )
        m_pLayAction->CheckWaitCrsr();
}

bool SwViewShellImp::IsCalcLayoutProgress() const
{
    return m_pLayAction && m_pLayAction->IsCalcLayout();
}

bool SwViewShellImp::IsUpdateExpFields()
{
    if ( m_pLayAction && m_pLayAction->IsCalcLayout() )
    {
        m_pLayAction->SetUpdateExpFields();
        return true;
    }
    return false;
}

void SwViewShellImp::SetFirstVisPage(OutputDevice* pRenderContext)
{
    if ( m_pShell->mbDocSizeChgd && m_pShell->VisArea().Top() > m_pShell->GetLayout()->Frm().Height() )
    {
        //We are in an action and because of erase actions the VisArea is
        //after the first visible page.
        //To avoid excessive formatting, hand back the last page.
        m_pFirstVisiblePage = static_cast<SwPageFrm*>(m_pShell->GetLayout()->Lower());
        while ( m_pFirstVisiblePage && m_pFirstVisiblePage->GetNext() )
            m_pFirstVisiblePage = static_cast<SwPageFrm*>(m_pFirstVisiblePage->GetNext());
    }
    else
    {
        const SwViewOption* pSwViewOption = GetShell()->GetViewOptions();
        const bool bBookMode = pSwViewOption->IsViewLayoutBookMode();

        SwPageFrm *pPage = static_cast<SwPageFrm*>(m_pShell->GetLayout()->Lower());
        SwRect aPageRect = pPage->GetBoundRect(pRenderContext);
        while ( pPage && !aPageRect.IsOver( m_pShell->VisArea() ) )
        {
            pPage = static_cast<SwPageFrm*>(pPage->GetNext());
            if ( pPage )
            {
                aPageRect = pPage->GetBoundRect(pRenderContext);
                if ( bBookMode && pPage->IsEmptyPage() )
                {
                    const SwPageFrm& rFormatPage = pPage->GetFormatPage();
                    aPageRect.SSize() = rFormatPage.GetBoundRect(pRenderContext).SSize();
                }
            }
        }
        m_pFirstVisiblePage = pPage ? pPage : static_cast<SwPageFrm*>(m_pShell->GetLayout()->Lower());
    }
    m_bFirstPageInvalid = false;
}

void SwViewShellImp::MakeDrawView()
{
    IDocumentDrawModelAccess& rIDDMA = GetShell()->getIDocumentDrawModelAccess();

    // the else here is not an error, _MakeDrawModel() calls this method again
    // after the DrawModel is created to create DrawViews for all shells...
    if( !rIDDMA.GetDrawModel() )
    {
        rIDDMA._MakeDrawModel();
    }
    else
    {
        if ( !m_pDrawView )
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

            m_pDrawView = new SwDrawView( *this, rIDDMA.GetDrawModel(), pOutDevForDrawView);
        }

        GetDrawView()->SetActiveLayer("Heaven");
        const SwViewOption* pSwViewOption = GetShell()->GetViewOptions();
        Init(pSwViewOption);

        // #i68597# If document is read-only, we will not profit from overlay,
        // so switch it off.
        if (m_pDrawView->IsBufferedOverlayAllowed())
        {
            if(pSwViewOption->IsReadonly())
            {
                m_pDrawView->SetBufferedOverlayAllowed(false);
            }
        }
    }
}

Color SwViewShellImp::GetRetoucheColor() const
{
    Color aRet( COL_TRANSPARENT );
    const SwViewShell &rSh = *GetShell();
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

SwPageFrm *SwViewShellImp::GetFirstVisPage(OutputDevice* pRenderContext)
{
    if ( m_bFirstPageInvalid )
        SetFirstVisPage(pRenderContext);
    return m_pFirstVisiblePage;
}

const SwPageFrm *SwViewShellImp::GetFirstVisPage(OutputDevice* pRenderContext) const
{
    if ( m_bFirstPageInvalid )
        const_cast<SwViewShellImp*>(this)->SetFirstVisPage(pRenderContext);
    return m_pFirstVisiblePage;
}

// create page preview layout
void SwViewShellImp::InitPagePreviewLayout()
{
    OSL_ENSURE( m_pShell->GetLayout(), "no layout - page preview layout can not be created.");
    if ( m_pShell->GetLayout() )
        m_pPagePreviewLayout = new SwPagePreviewLayout( *m_pShell, *(m_pShell->GetLayout()) );
}

void SwViewShellImp::UpdateAccessible()
{
    // We require a layout and an XModel to be accessible.
    IDocumentLayoutAccess& rIDLA = GetShell()->getIDocumentLayoutAccess();
    vcl::Window *pWin = GetShell()->GetWin();
    OSL_ENSURE( GetShell()->GetLayout(), "no layout, no access" );
    OSL_ENSURE( pWin, "no window, no access" );

    if( IsAccessible() && rIDLA.GetCurrentViewShell() && pWin )
        GetAccessibleMap().GetDocumentView();
}

void SwViewShellImp::DisposeAccessible( const SwFrm *pFrm,
                                   const SdrObject *pObj,
                                   bool bRecursive )
{
    OSL_ENSURE( !pFrm || pFrm->IsAccessibleFrm(), "frame is not accessible" );
    for(SwViewShell& rTmp : GetShell()->GetRingContainer())
    {
        if( rTmp.Imp()->IsAccessible() )
            rTmp.Imp()->GetAccessibleMap().Dispose( pFrm, pObj, nullptr, bRecursive );
    }
}

void SwViewShellImp::MoveAccessible( const SwFrm *pFrm, const SdrObject *pObj,
                                const SwRect& rOldFrm )
{
    OSL_ENSURE( !pFrm || pFrm->IsAccessibleFrm(), "frame is not accessible" );
    for(SwViewShell& rTmp : GetShell()->GetRingContainer())
    {
        if( rTmp.Imp()->IsAccessible() )
            rTmp.Imp()->GetAccessibleMap().InvalidatePosOrSize( pFrm, pObj, nullptr,
                                                                 rOldFrm );
    }
}

void SwViewShellImp::InvalidateAccessibleFrmContent( const SwFrm *pFrm )
{
    OSL_ENSURE( pFrm->IsAccessibleFrm(), "frame is not accessible" );
    for(SwViewShell& rTmp : GetShell()->GetRingContainer())
    {
        if( rTmp.Imp()->IsAccessible() )
            rTmp.Imp()->GetAccessibleMap().InvalidateContent( pFrm );
    }
}

void SwViewShellImp::InvalidateAccessibleCursorPosition( const SwFrm *pFrm )
{
    if( IsAccessible() )
        GetAccessibleMap().InvalidateCursorPosition( pFrm );
}

void SwViewShellImp::InvalidateAccessibleEditableState( bool bAllShells,
                                                      const SwFrm *pFrm )
{
    if( bAllShells )
    {
        for(SwViewShell& rTmp : GetShell()->GetRingContainer())
        {
            if( rTmp.Imp()->IsAccessible() )
                rTmp.Imp()->GetAccessibleMap().InvalidateStates( AccessibleStates::EDITABLE, pFrm );
        }
    }
    else if( IsAccessible() )
    {
        GetAccessibleMap().InvalidateStates( AccessibleStates::EDITABLE, pFrm );
    }
}

void SwViewShellImp::InvalidateAccessibleRelationSet( const SwFlyFrm *pMaster,
                                                 const SwFlyFrm *pFollow )
{
    for(SwViewShell& rTmp : GetShell()->GetRingContainer())
    {
        if( rTmp.Imp()->IsAccessible() )
            rTmp.Imp()->GetAccessibleMap().InvalidateRelationSet( pMaster,
                                                                   pFollow );
    }
}

/// invalidate CONTENT_FLOWS_FROM/_TO relation for paragraphs
void SwViewShellImp::_InvalidateAccessibleParaFlowRelation( const SwTextFrm* _pFromTextFrm,
                                                       const SwTextFrm* _pToTextFrm )
{
    if ( !_pFromTextFrm && !_pToTextFrm )
    {
        // No text frame provided. Thus, nothing to do.
        return;
    }

    for(SwViewShell& rTmp : GetShell()->GetRingContainer())
    {
        if ( rTmp.Imp()->IsAccessible() )
        {
            if ( _pFromTextFrm )
            {
                rTmp.Imp()->GetAccessibleMap().
                            InvalidateParaFlowRelation( *_pFromTextFrm, true );
            }
            if ( _pToTextFrm )
            {
                rTmp.Imp()->GetAccessibleMap().
                            InvalidateParaFlowRelation( *_pToTextFrm, false );
            }
        }
    }
}

/// invalidate text selection for paragraphs
void SwViewShellImp::_InvalidateAccessibleParaTextSelection()
{
    for(SwViewShell& rTmp : GetShell()->GetRingContainer())
    {
        if ( rTmp.Imp()->IsAccessible() )
        {
            rTmp.Imp()->GetAccessibleMap().InvalidateTextSelectionOfAllParas();
        }
    }
}

/// invalidate attributes for paragraphs
void SwViewShellImp::_InvalidateAccessibleParaAttrs( const SwTextFrm& rTextFrm )
{
    for(SwViewShell& rTmp : GetShell()->GetRingContainer())
    {
        if ( rTmp.Imp()->IsAccessible() )
        {
            rTmp.Imp()->GetAccessibleMap().InvalidateAttr( rTextFrm );
        }
    }
}

// OD 15.01.2003 #103492# - method signature change due to new page preview functionality
void SwViewShellImp::UpdateAccessiblePreview( const std::vector<PreviewPage*>& _rPreviewPages,
                                         const Fraction&  _rScale,
                                         const SwPageFrm* _pSelectedPageFrm,
                                         const Size&      _rPreviewWinSize )
{
    if( IsAccessible() )
        GetAccessibleMap().UpdatePreview( _rPreviewPages, _rScale,
                                          _pSelectedPageFrm, _rPreviewWinSize );
}

void SwViewShellImp::InvalidateAccessiblePreviewSelection( sal_uInt16 nSelPage )
{
    if( IsAccessible() )
        GetAccessibleMap().InvalidatePreviewSelection( nSelPage );
}

SwAccessibleMap *SwViewShellImp::CreateAccessibleMap()
{
    OSL_ENSURE( !m_pAccessibleMap, "accessible map exists" );
    m_pAccessibleMap = new SwAccessibleMap( GetShell() );
    return m_pAccessibleMap;
}

void SwViewShellImp::FireAccessibleEvents()
{
    if( IsAccessible() )
        GetAccessibleMap().FireEvents();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
