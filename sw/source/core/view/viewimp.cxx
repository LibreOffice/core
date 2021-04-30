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

#include <config_wasm_strip.h>

#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <viewimp.hxx>
#include <viewopt.hxx>
#include <flyfrm.hxx>
#include <layact.hxx>
#include <dview.hxx>
#include <swmodule.hxx>
#include <svx/svdpage.hxx>
#include <accmap.hxx>

#include <officecfg/Office/Common.hxx>
#include <pagepreviewlayout.hxx>
#include <comphelper/lok.hxx>
#include <tools/diagnose_ex.h>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <prevwpage.hxx>

void SwViewShellImp::Init( const SwViewOption *pNewOpt )
{
    OSL_ENSURE( m_pDrawView, "SwViewShellImp::Init without DrawView" );
    //Create PageView if it doesn't exist
    SwRootFrame *pRoot = m_pShell->GetLayout();
    if ( !m_pSdrPageView )
    {
        IDocumentDrawModelAccess& rIDDMA = m_pShell->getIDocumentDrawModelAccess();
        if ( !pRoot->GetDrawPage() )
            pRoot->SetDrawPage( rIDDMA.GetDrawModel()->GetPage( 0 ) );

        if ( pRoot->GetDrawPage()->GetSize() != pRoot->getFrameArea().SSize() )
            pRoot->GetDrawPage()->SetSize( pRoot->getFrameArea().SSize() );

        m_pSdrPageView = m_pDrawView->ShowSdrPage( pRoot->GetDrawPage());
        // Notify drawing page view about invisible layers
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

    if ( pRoot->getFrameArea().HasArea() )
        m_pDrawView->SetWorkArea( pRoot->getFrameArea().SVRect() );

    if ( GetShell()->IsPreview() )
        m_pDrawView->SetAnimationEnabled( false );

    m_pDrawView->SetUseIncompatiblePathCreateInterface( false );

    // set handle size to 9 pixels, always
    m_pDrawView->SetMarkHdlSizePixel(9);
}

/// CTor for the core internals
SwViewShellImp::SwViewShellImp( SwViewShell *pParent ) :
    m_pShell( pParent ),
    m_pSdrPageView( nullptr ),
    m_pFirstVisiblePage( nullptr ),
    m_pLayAction( nullptr ),
    m_pIdleAct( nullptr ),
    m_bFirstPageInvalid( true ),
    m_bResetHdlHiddenPaint( false ),
    m_bSmoothUpdate( false ),
    m_bStopSmooth( false ),
    m_nRestoreActions( 0 )
{
}

SwViewShellImp::~SwViewShellImp()
{
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
    m_pAccessibleMap.reset();
#endif

    m_pPagePreviewLayout.reset();

    // Make sure HideSdrPage is also executed after ShowSdrPage.
    if( m_pDrawView )
         m_pDrawView->HideSdrPage();

    m_pDrawView.reset();

    DeletePaintRegion();

    OSL_ENSURE( !m_pLayAction, "Have action for the rest of your life." );
    OSL_ENSURE( !m_pIdleAct,"Be idle for the rest of your life." );
}

bool SwViewShellImp::AddPaintRect( const SwRect &rRect )
{
    // In case of tiled rendering the visual area is the last painted tile -> not interesting.
    if ( rRect.Overlaps( m_pShell->VisArea() ) || comphelper::LibreOfficeKit::isActive() )
    {
        if ( !m_pPaintRegion )
        {
            // In case of normal rendering, this makes sure only visible rectangles are painted.
            // Otherwise get the rectangle of the full document, so all paint rectangles are invalidated.
            const SwRect& rArea = comphelper::LibreOfficeKit::isActive() ? m_pShell->GetLayout()->getFrameArea() : m_pShell->VisArea();
            m_pPaintRegion.reset(new SwRegionRects);
            m_pPaintRegion->ChangeOrigin(rArea);
        }
        if(!m_pPaintRegion->empty())
        {
            // This function often gets called with rectangles that line up vertically.
            // Try to extend the last one downwards to include the new one.
            SwRect& last = m_pPaintRegion->back();
            if(last.Left() == rRect.Left() && last.Width() == rRect.Width()
                && last.Bottom() + 1 >= rRect.Top() && last.Bottom() <= rRect.Bottom())
            {
                last = SwRect( last.TopLeft(), rRect.BottomRight());
                // And these rectangles lined up vertically often come up in groups
                // that line up horizontally. Try to extend the previous rectangle
                // to the right to include the last one.
                if(m_pPaintRegion->size() > 1)
                {
                    SwRect& last2 = (*m_pPaintRegion)[m_pPaintRegion->size() - 2];
                    if(last2.Top() == last.Top() && last2.Height() == last.Height()
                        && last2.Right() + 1 >= last.Left() && last2.Right() <= last2.Right())
                    {
                        last2 = SwRect( last.TopLeft(), rRect.BottomRight());
                        m_pPaintRegion->pop_back();
                        return true;
                    }
                }
                return true;
            }
        }
        (*m_pPaintRegion) += rRect;
        return true;
    }
    return false;
}

void SwViewShellImp::AddPendingLOKInvalidation( const SwRect& rRect )
{
    // These are often repeated, so check first for duplicates.
    std::vector<SwRect>& l = m_pendingLOKInvalidations;
    if( std::find( l.begin(), l.end(), rRect ) == l.end())
        l.push_back( rRect );
}

std::vector<SwRect> SwViewShellImp::TakePendingLOKInvalidations()
{
    std::vector<SwRect> ret;
    std::swap(ret, m_pendingLOKInvalidations);
    return ret;
}

void SwViewShellImp::CheckWaitCursor()
{
    if ( m_pLayAction )
        m_pLayAction->CheckWaitCursor();
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

void SwViewShellImp::SetFirstVisPage(OutputDevice const * pRenderContext)
{
    if ( m_pShell->mbDocSizeChgd && m_pShell->VisArea().Top() > m_pShell->GetLayout()->getFrameArea().Height() )
    {
        //We are in an action and because of erase actions the VisArea is
        //after the first visible page.
        //To avoid excessive formatting, hand back the last page.
        m_pFirstVisiblePage = static_cast<SwPageFrame*>(m_pShell->GetLayout()->Lower());
        while ( m_pFirstVisiblePage && m_pFirstVisiblePage->GetNext() )
            m_pFirstVisiblePage = static_cast<SwPageFrame*>(m_pFirstVisiblePage->GetNext());
    }
    else
    {
        const SwViewOption* pSwViewOption = GetShell()->GetViewOptions();
        const bool bBookMode = pSwViewOption->IsViewLayoutBookMode();

        SwPageFrame *pPage = static_cast<SwPageFrame*>(m_pShell->GetLayout()->Lower());
        SwRect aPageRect = pPage->GetBoundRect(pRenderContext);
        while ( pPage && !aPageRect.Overlaps( m_pShell->VisArea() ) )
        {
            pPage = static_cast<SwPageFrame*>(pPage->GetNext());
            if ( pPage )
            {
                aPageRect = pPage->GetBoundRect(pRenderContext);
                if ( bBookMode && pPage->IsEmptyPage() )
                {
                    const SwPageFrame& rFormatPage = pPage->GetFormatPage();
                    aPageRect.SSize( rFormatPage.GetBoundRect(pRenderContext).SSize() );
                }
            }
        }
        m_pFirstVisiblePage = pPage ? pPage : static_cast<SwPageFrame*>(m_pShell->GetLayout()->Lower());
    }
    m_bFirstPageInvalid = false;
}

void SwViewShellImp::MakeDrawView()
{
    IDocumentDrawModelAccess& rIDDMA = GetShell()->getIDocumentDrawModelAccess();

    // the else here is not an error, MakeDrawModel_() calls this method again
    // after the DrawModel is created to create DrawViews for all shells...
    if( !rIDDMA.GetDrawModel() )
    {
        rIDDMA.MakeDrawModel_();
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
            auto pWin = GetShell()->GetWin();
            OutputDevice* pOutDevForDrawView = pWin ? pWin->GetOutDev() : nullptr;

            if(!pOutDevForDrawView)
            {
                pOutDevForDrawView = GetShell()->GetOut();
            }

            m_pDrawView.reset( new SwDrawView(
                *this,
                *rIDDMA.GetOrCreateDrawModel(),
                pOutDevForDrawView) );
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
             COL_TRANSPARENT != rSh.GetViewOptions()->GetRetoucheColor() )
            aRet = rSh.GetViewOptions()->GetRetoucheColor();
        else if(rSh.GetViewOptions()->IsPagePreview()  &&
                    !officecfg::Office::Common::Accessibility::IsForPagePreviews::get())
            aRet = COL_WHITE;
        else
            aRet = SwViewOption::GetDocColor();
    }
    return aRet;
}

SwPageFrame *SwViewShellImp::GetFirstVisPage(OutputDevice const * pRenderContext)
{
    if ( m_bFirstPageInvalid )
        SetFirstVisPage(pRenderContext);
    return m_pFirstVisiblePage;
}

const SwPageFrame *SwViewShellImp::GetFirstVisPage(OutputDevice const * pRenderContext) const
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
        m_pPagePreviewLayout.reset( new SwPagePreviewLayout( *m_pShell, *(m_pShell->GetLayout()) ) );
}

#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
void SwViewShellImp::UpdateAccessible()
{
    // We require a layout and an XModel to be accessible.
    IDocumentLayoutAccess& rIDLA = GetShell()->getIDocumentLayoutAccess();
    vcl::Window *pWin = GetShell()->GetWin();
    OSL_ENSURE( GetShell()->GetLayout(), "no layout, no access" );
    OSL_ENSURE( pWin, "no window, no access" );

    if( IsAccessible() && rIDLA.GetCurrentViewShell() && pWin )
    {
        try
        {
            GetAccessibleMap().GetDocumentView();
        }
        catch (uno::Exception const&)
        {
            TOOLS_WARN_EXCEPTION("sw.a11y", "");
            assert(!"SwViewShellImp::UpdateAccessible: unhandled exception");
        }
    }
}

void SwViewShellImp::DisposeAccessible(const SwFrame *pFrame,
                                       const SdrObject *pObj,
                                       bool bRecursive,
                                       bool bCanSkipInvisible)
{
    OSL_ENSURE( !pFrame || pFrame->IsAccessibleFrame(), "frame is not accessible" );
    for(SwViewShell& rTmp : GetShell()->GetRingContainer())
    {
        if( rTmp.Imp()->IsAccessible() )
            rTmp.Imp()->GetAccessibleMap().A11yDispose( pFrame, pObj, nullptr, bRecursive, bCanSkipInvisible );
    }
}

void SwViewShellImp::MoveAccessible( const SwFrame *pFrame, const SdrObject *pObj,
                                const SwRect& rOldFrame )
{
    OSL_ENSURE( !pFrame || pFrame->IsAccessibleFrame(), "frame is not accessible" );
    for(SwViewShell& rTmp : GetShell()->GetRingContainer())
    {
        if( rTmp.Imp()->IsAccessible() )
            rTmp.Imp()->GetAccessibleMap().InvalidatePosOrSize( pFrame, pObj, nullptr,
                                                                 rOldFrame );
    }
}

void SwViewShellImp::InvalidateAccessibleFrameContent( const SwFrame *pFrame )
{
    OSL_ENSURE( pFrame->IsAccessibleFrame(), "frame is not accessible" );
    for(SwViewShell& rTmp : GetShell()->GetRingContainer())
    {
        if( rTmp.Imp()->IsAccessible() )
            rTmp.Imp()->GetAccessibleMap().InvalidateContent( pFrame );
    }
}

void SwViewShellImp::InvalidateAccessibleCursorPosition( const SwFrame *pFrame )
{
    if( IsAccessible() )
        GetAccessibleMap().InvalidateCursorPosition( pFrame );
}

void SwViewShellImp::InvalidateAccessibleEditableState( bool bAllShells,
                                                      const SwFrame *pFrame )
{
    if( bAllShells )
    {
        for(SwViewShell& rTmp : GetShell()->GetRingContainer())
        {
            if( rTmp.Imp()->IsAccessible() )
                rTmp.Imp()->GetAccessibleMap().InvalidateEditableStates( pFrame );
        }
    }
    else if( IsAccessible() )
    {
        GetAccessibleMap().InvalidateEditableStates( pFrame );
    }
}

void SwViewShellImp::InvalidateAccessibleRelationSet( const SwFlyFrame *pMaster,
                                                 const SwFlyFrame *pFollow )
{
    for(SwViewShell& rTmp : GetShell()->GetRingContainer())
    {
        if( rTmp.Imp()->IsAccessible() )
            rTmp.Imp()->GetAccessibleMap().InvalidateRelationSet( pMaster,
                                                                   pFollow );
    }
}

/// invalidate CONTENT_FLOWS_FROM/_TO relation for paragraphs
void SwViewShellImp::InvalidateAccessibleParaFlowRelation_( const SwTextFrame* _pFromTextFrame,
                                                       const SwTextFrame* _pToTextFrame )
{
    if ( !_pFromTextFrame && !_pToTextFrame )
    {
        // No text frame provided. Thus, nothing to do.
        return;
    }

    for(SwViewShell& rTmp : GetShell()->GetRingContainer())
    {
        if ( rTmp.Imp()->IsAccessible() )
        {
            if ( _pFromTextFrame )
            {
                rTmp.Imp()->GetAccessibleMap().
                            InvalidateParaFlowRelation( *_pFromTextFrame, true );
            }
            if ( _pToTextFrame )
            {
                rTmp.Imp()->GetAccessibleMap().
                            InvalidateParaFlowRelation( *_pToTextFrame, false );
            }
        }
    }
}

/// invalidate text selection for paragraphs
void SwViewShellImp::InvalidateAccessibleParaTextSelection_()
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
void SwViewShellImp::InvalidateAccessibleParaAttrs_( const SwTextFrame& rTextFrame )
{
    for(SwViewShell& rTmp : GetShell()->GetRingContainer())
    {
        if ( rTmp.Imp()->IsAccessible() )
        {
            rTmp.Imp()->GetAccessibleMap().InvalidateAttr( rTextFrame );
        }
    }
}

void SwViewShellImp::UpdateAccessiblePreview( const std::vector<std::unique_ptr<PreviewPage>>& _rPreviewPages,
                                         const Fraction&  _rScale,
                                         const SwPageFrame* _pSelectedPageFrame,
                                         const Size&      _rPreviewWinSize )
{
    if( IsAccessible() )
        GetAccessibleMap().UpdatePreview( _rPreviewPages, _rScale,
                                          _pSelectedPageFrame, _rPreviewWinSize );
}

void SwViewShellImp::InvalidateAccessiblePreviewSelection( sal_uInt16 nSelPage )
{
    if( IsAccessible() )
        GetAccessibleMap().InvalidatePreviewSelection( nSelPage );
}

SwAccessibleMap *SwViewShellImp::CreateAccessibleMap()
{
    assert(!m_pAccessibleMap);
    m_pAccessibleMap = std::make_shared<SwAccessibleMap>(GetShell());
    return m_pAccessibleMap.get();
}

void SwViewShellImp::FireAccessibleEvents()
{
    if( IsAccessible() )
        GetAccessibleMap().FireEvents();
}
#endif // ENABLE_WASM_STRIP_ACCESSIBILITY

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
