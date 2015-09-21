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

#include <string>

#include "hintids.hxx"
#include <vcl/svapp.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ruler.hxx>
#include <editeng/lrspitem.hxx>
#include <svl/srchitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/request.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <viewopt.hxx>
#include <frmatr.hxx>
#include <wdocsh.hxx>
#include <uitool.hxx>
#include <edtwin.hxx>
#include <pagedesc.hxx>
#include <IMark.hxx>
#include <fldbas.hxx>
#include <workctrl.hxx>
#include <usrpref.hxx>
#include <scroll.hxx>
#include <wview.hxx>

#include <cmdid.h>
#include <view.hrc>
#include <ribbar.hrc>
#include <helpid.h>
#include <globals.hrc>

#include <IDocumentSettingAccess.hxx>
#include <PostItMgr.hxx>

sal_uInt16  SwView::m_nMoveType = NID_PGE;
sal_Int32 SwView::m_nActMark = 0;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

void SwView::SetZoom( SvxZoomType eZoomType, short nFactor, bool bViewOnly )
{
    bool const bCrsrIsVisible(m_pWrtShell->IsCrsrVisible());
    _SetZoom( GetEditWin().GetOutputSizePixel(), eZoomType, nFactor, bViewOnly );
    // fdo#40465 force the cursor to stay in view whilst zooming
    if (bCrsrIsVisible)
        m_pWrtShell->ShowCrsr();
}

void SwView::_SetZoom( const Size &rEditSize, SvxZoomType eZoomType,
                        short nFactor, bool bViewOnly )
{
    bool bUnLockView = !m_pWrtShell->IsViewLocked();
    m_pWrtShell->LockView( true );
    m_pWrtShell->LockPaint();

    { // start of SwActContext scope
    SwActContext aActContext(m_pWrtShell);

    long nFac = nFactor;

    const bool bWeb = this->ISA(SwWebView);
    SwMasterUsrPref *pUsrPref = const_cast<SwMasterUsrPref*>(SW_MOD()->GetUsrPref(bWeb));

    const SwPageDesc &rDesc = m_pWrtShell->GetPageDesc( m_pWrtShell->GetCurPageDesc() );
    const SvxLRSpaceItem &rLRSpace = rDesc.GetMaster().GetLRSpace();
    const SwViewOption *pOpt = m_pWrtShell->GetViewOptions();
    long lLeftMargin = 0;

    if( eZoomType != SvxZoomType::PERCENT )
    {
        const bool bAutomaticViewLayout = 0 == pOpt->GetViewLayoutColumns();

        const SwRect aPageRect( m_pWrtShell->GetAnyCurRect( RECT_PAGE_CALC ) );
        const SwRect aRootRect( m_pWrtShell->GetAnyCurRect( RECT_PAGES_AREA ) );
        Size aPageSize( aPageRect.SSize() );
        Size aRootSize( aRootRect.SSize() );

        //mod #i6193# added sidebar width
        SwPostItMgr* pPostItMgr = GetPostItMgr();
        if (pPostItMgr->HasNotes() && pPostItMgr->ShowNotes())
            aPageSize.Width() += pPostItMgr->GetSidebarWidth() + pPostItMgr->GetSidebarBorderWidth();

        const MapMode aTmpMap( MAP_TWIP );
        const Size aWindowSize( GetEditWin().PixelToLogic( rEditSize, aTmpMap ) );

        if( nsUseOnPage::PD_MIRROR == rDesc.GetUseOn() )    // mirrored pages
        {
            const SvxLRSpaceItem &rLeftLRSpace = rDesc.GetLeft().GetLRSpace();
            aPageSize.Width() += std::abs( long(rLeftLRSpace.GetLeft()) - long(rLRSpace.GetLeft()) );
        }

        if( SvxZoomType::OPTIMAL == eZoomType )
        {
            if (!pPostItMgr->HasNotes() || !pPostItMgr->ShowNotes())
                aPageSize.Width() -= ( rLRSpace.GetLeft() + rLRSpace.GetRight() + nLeftOfst * 2 );
            lLeftMargin = long(rLRSpace.GetLeft()) + DOCUMENTBORDER + nLeftOfst;
            nFac = aWindowSize.Width() * 100 / aPageSize.Width();
        }
        else if(SvxZoomType::WHOLEPAGE == eZoomType || SvxZoomType::PAGEWIDTH == eZoomType )
        {
            const long nOf = DOCUMENTBORDER * 2L;
            long nTmpWidth = bAutomaticViewLayout ? aPageSize.Width() : aRootSize.Width();
            nTmpWidth += nOf;
            aPageSize.Height() += nOf;
            nFac = aWindowSize.Width() * 100 / nTmpWidth;

            if ( SvxZoomType::WHOLEPAGE == eZoomType )
            {
                long nVisPercent = aWindowSize.Height() * 100 / aPageSize.Height();
                nFac = std::min( nFac, nVisPercent );
            }
        }
        else
        {
            const long nTmpWidth = bAutomaticViewLayout ? aPageSize.Width() : aRootSize.Width();
            nFac = aWindowSize.Width() * 100 / nTmpWidth;
        }
    }

    nFac = std::max( long( MINZOOM ), nFac );
    const sal_uInt16 nZoomFac = static_cast<sal_uInt16>(nFac);

    SwViewOption aOpt( *pOpt );
    if ( !GetViewFrame()->GetFrame().IsInPlace() )
    {
        //Update MasterUsrPrefs and after that update the ViewOptions of the current View.
        if ( !bViewOnly &&
                (nZoomFac != pUsrPref->GetZoom() ||
                eZoomType != pUsrPref->GetZoomType()) )
        {
            pUsrPref->SetZoom(nZoomFac);
            pUsrPref->SetZoomType(eZoomType);
            SW_MOD()->ApplyUsrPref(*pUsrPref, 0);
            pUsrPref->SetModified();
        }
        if ( pOpt->GetZoom() != nZoomFac )
        {
            aOpt.SetZoom(nZoomFac);
            aOpt.SetReadonly(pOpt->IsReadonly());
            m_pWrtShell->ApplyViewOptions( aOpt );
        }
        if ( eZoomType != SvxZoomType::PERCENT )
        {
            Point aPos;

            if ( eZoomType == SvxZoomType::WHOLEPAGE )
                aPos.Y() = m_pWrtShell->GetAnyCurRect(RECT_PAGE).Top() - DOCUMENTBORDER;
            else
            {
                // Make sure that the cursor is in the visible range, so that
                // the scrolling will be performed only once.
                aPos.X() = lLeftMargin;
                const SwRect &rCharRect = m_pWrtShell->GetCharRect();
                if ( rCharRect.Top() > GetVisArea().Bottom() ||
                    rCharRect.Bottom() < aPos.Y() )
                    aPos.Y() = rCharRect.Top() - rCharRect.Height();
                else
                    aPos.Y() = GetVisArea().Top();
            }
            SetVisArea( aPos );
        }
        // Compromise solution - Under certain circumstances SetZoom is called
        // in CalcVisAreas again and thus be set wrong values.
        const_cast<SwViewOption*>(m_pWrtShell->GetViewOptions())->SetZoomType( eZoomType );
        CalcVisArea( rEditSize );   // for the recalculation of the viewable area
    }
    else if ( nZoomFac != pOpt->GetZoom() )
    {
        aOpt.SetZoom( nZoomFac );
        m_pWrtShell->ApplyViewOptions( aOpt );
    }

    const Fraction aFrac( nFac, 100 );
    m_pVRuler->SetZoom( aFrac );
    m_pVRuler->ForceUpdate();
    m_pHRuler->SetZoom( aFrac );
    m_pHRuler->ForceUpdate();
    const_cast<SwViewOption*>(m_pWrtShell->GetViewOptions())->SetZoomType( eZoomType );
    } // end of SwActContext scope

    m_pWrtShell->UnlockPaint();
    if( bUnLockView )
        m_pWrtShell->LockView( false );
}

void SwView::SetViewLayout( sal_uInt16 nColumns, bool bBookMode, bool bViewOnly )
{
    const bool bUnLockView = !m_pWrtShell->IsViewLocked();
    m_pWrtShell->LockView( true );
    m_pWrtShell->LockPaint();

    {

    SwActContext aActContext(m_pWrtShell);

    if ( !GetViewFrame()->GetFrame().IsInPlace() && !bViewOnly )
    {
        const bool bWeb = this->ISA(SwWebView);
        SwMasterUsrPref *pUsrPref = const_cast<SwMasterUsrPref*>(SW_MOD()->GetUsrPref(bWeb));

        // Update MasterUsrPrefs and after that update the ViewOptions of the current View.
        if ( nColumns  != pUsrPref->GetViewLayoutColumns() ||
             bBookMode != pUsrPref->IsViewLayoutBookMode() )
        {
            pUsrPref->SetViewLayoutColumns(nColumns);
            pUsrPref->SetViewLayoutBookMode(bBookMode);
            SW_MOD()->ApplyUsrPref(*pUsrPref, 0);
            pUsrPref->SetModified();
        }
    }

    const SwViewOption *pOpt = m_pWrtShell->GetViewOptions();

    if ( nColumns  != pOpt->GetViewLayoutColumns() ||
         bBookMode != pOpt->IsViewLayoutBookMode() )
    {
        SwViewOption aOpt( *pOpt );
        aOpt.SetViewLayoutColumns( nColumns );
        aOpt.SetViewLayoutBookMode( bBookMode );
        m_pWrtShell->ApplyViewOptions( aOpt );
    }

    m_pVRuler->ForceUpdate();
    m_pHRuler->ForceUpdate();

    }

    m_pWrtShell->UnlockPaint();
    if( bUnLockView )
        m_pWrtShell->LockView( false );

    SfxBindings& rBnd = GetViewFrame()->GetBindings();
    rBnd.Invalidate( SID_ATTR_VIEWLAYOUT );
    rBnd.Invalidate( SID_ATTR_ZOOMSLIDER);
}

// Scrollbar - Handler

IMPL_LINK_TYPED( SwView, WindowChildEventListener, VclWindowEvent&, rEvent, void )
{
    OSL_ENSURE( rEvent.GetWindow(), "Window???" );
    vcl::Window* pChildWin = static_cast< vcl::Window* >( rEvent.GetData() );

    switch ( rEvent.GetId() )
    {
        case VCLEVENT_WINDOW_HIDE:
            if( pChildWin == m_pHScrollbar )
                ShowHScrollbar( false );
            else if( pChildWin == m_pVScrollbar )
                ShowVScrollbar( false );
            break;
        case VCLEVENT_WINDOW_SHOW:
            if( pChildWin == m_pHScrollbar )
                ShowHScrollbar( true );
            else if( pChildWin == m_pVScrollbar )
                ShowVScrollbar( true );
            break;
    }
}

int SwView::_CreateScrollbar( bool bHori )
{
    vcl::Window *pMDI = &GetViewFrame()->GetWindow();
    VclPtr<SwScrollbar>& ppScrollbar = bHori ? m_pHScrollbar : m_pVScrollbar;

    assert(!ppScrollbar.get()); //check beforehand!

    ppScrollbar = VclPtr<SwScrollbar>::Create( pMDI, bHori );
    UpdateScrollbars();
    if(bHori)
        ppScrollbar->SetScrollHdl( LINK( this, SwView, EndScrollHdl ));
    else
        ppScrollbar->SetScrollHdl( LINK( this, SwView, ScrollHdl ));
    ppScrollbar->SetEndScrollHdl( LINK( this, SwView, EndScrollHdl ));

    ppScrollbar->EnableDrag();

    if(GetWindow())
        InvalidateBorder();

    if (!m_bShowAtResize)
        ppScrollbar->ExtendedShow();

    return 1;
}

IMPL_LINK_TYPED( SwView, MoveNavigationHdl, void*, p, void )
{
    bool* pbNext = static_cast<bool*>(p);
    if ( !pbNext )
        return;
    const bool bNext = *pbNext;
    SwWrtShell& rSh = GetWrtShell();
    switch( m_nMoveType )
    {
        case NID_PGE:
            bNext ? PhyPageDown() : PhyPageUp();
        break;
        case NID_TBL :
            rSh.EnterStdMode();
            if(bNext)
                rSh.MoveTable(fnTableNext, fnTableStart);
            else
                rSh.MoveTable(fnTablePrev, fnTableStart);
        break;
        case NID_FRM :
        case NID_GRF:
        case NID_OLE:
        {
            GotoObjFlags eType = GotoObjFlags::FlyFrm;
            if(m_nMoveType == NID_GRF)
                eType = GotoObjFlags::FlyGrf;
            else if(m_nMoveType == NID_OLE)
                eType = GotoObjFlags::FlyOLE;
            bool bSuccess = bNext ?
                    rSh.GotoNextFly(eType) :
                        rSh.GotoPrevFly(eType);
            if(bSuccess)
            {
                rSh.HideCrsr();
                rSh.EnterSelFrmMode();
            }
        }
        break;
        case NID_DRW :
        case NID_CTRL:
            rSh.GotoObj(bNext,
                    m_nMoveType == NID_DRW ?
                        GotoObjFlags::DrawSimple :
                        GotoObjFlags::DrawControl);
        break;
        case NID_REG :
            rSh.EnterStdMode();
            if(bNext)
                rSh.MoveRegion(fnRegionNext, fnRegionStart);
            else
                rSh.MoveRegion(fnRegionPrev, fnRegionStart);

        break;
        case NID_BKM :
            rSh.EnterStdMode();
            GetViewFrame()->GetDispatcher()->Execute(bNext ?
                                        FN_NEXT_BOOKMARK :
                                            FN_PREV_BOOKMARK);
        break;
        case NID_OUTL:
            rSh.EnterStdMode();
            bNext ? rSh.GotoNextOutline() : rSh.GotoPrevOutline();
        break;
        case NID_SEL :
            bNext ? rSh.GoNextCrsr() : rSh.GoPrevCrsr();
        break;
        case NID_FTN:
            rSh.EnterStdMode();
            bNext ?
                rSh.GotoNextFootnoteAnchor() :
                    rSh.GotoPrevFootnoteAnchor();
        break;
        case NID_MARK:
        {
            // unselect
            rSh.MoveCrsr();
            rSh.EnterStdMode();

            // collect navigator reminders
            IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
            ::std::vector< const ::sw::mark::IMark* > vNavMarks;
            for( IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getAllMarksBegin();
                ppMark != pMarkAccess->getAllMarksEnd();
                ++ppMark)
            {
                if( IDocumentMarkAccess::GetType(**ppMark) == IDocumentMarkAccess::MarkType::NAVIGATOR_REMINDER )
                    vNavMarks.push_back(ppMark->get());
            }

            // move
            if(!vNavMarks.empty())
            {
                if(bNext)
                {
                    m_nActMark++;
                    if (m_nActMark >= MAX_MARKS || m_nActMark >= static_cast<sal_Int32>(vNavMarks.size()))
                        m_nActMark = 0;
                }
                else
                {
                    m_nActMark--;
                    if (m_nActMark < 0 || m_nActMark >= static_cast<sal_Int32>(vNavMarks.size()))
                        m_nActMark = vNavMarks.size()-1;
                }
                rSh.GotoMark(vNavMarks[m_nActMark]);
            }
        }
        break;

        case NID_POSTIT:
            {
                rSh.EnterStdMode();
                sw::sidebarwindows::SwSidebarWin* pPostIt = GetPostItMgr()->GetActiveSidebarWin();
                if (pPostIt)
                    GetPostItMgr()->SetActiveSidebarWin(0);
                SwFieldType* pFieldType = rSh.GetFieldType(0, RES_POSTITFLD);
                if ( rSh.MoveFieldType( pFieldType, bNext ) )
                    GetViewFrame()->GetDispatcher()->Execute(FN_POSTIT);
                else
                    //first/last item
                    GetPostItMgr()->SetActiveSidebarWin(pPostIt);
            }
            break;

        case NID_SRCH_REP:
        if(m_pSrchItem)
        {
            bool bBackward = m_pSrchItem->GetBackward();
            if (rSh.HasSelection() && bNext != rSh.IsCrsrPtAtEnd())
                rSh.SwapPam();
            m_pSrchItem->SetBackward(!bNext);
            SfxRequest aReq(FN_REPEAT_SEARCH, SfxCallMode::SLOT, GetPool());
            ExecSearch(aReq);
            m_pSrchItem->SetBackward(bBackward);
        }
        break;
        case NID_INDEX_ENTRY:
            rSh.GotoNxtPrvTOXMark(bNext);
        break;

        case NID_TABLE_FORMULA:
            rSh.GotoNxtPrvTableFormula( bNext );
            break;

        case NID_TABLE_FORMULA_ERROR:
            rSh.GotoNxtPrvTableFormula( bNext, true );
            break;
    }
    m_pEditWin->GrabFocus();
    delete pbNext;
}

int SwView::CreateTab()
{
    m_pHRuler->SetActive(GetFrame() && IsActive());

    m_pHRuler->Show();
    InvalidateBorder();
    return 1;
}

int SwView::KillTab()
{
    m_pHRuler->Hide();
    InvalidateBorder();
    return 1;
}

void SwView::ChangeTabMetric( FieldUnit eUnit )
{
    if(m_pHRuler->GetUnit() != eUnit )
    {
        m_pHRuler->SetUnit( eUnit );
        m_pHRuler->Invalidate();
    }
}

void SwView::ChangeVRulerMetric( FieldUnit eUnit )
{
    if(m_pVRuler->GetUnit() != eUnit)
    {
        m_pVRuler->SetUnit( eUnit );
        m_pVRuler->Invalidate();
    }
}

void SwView::GetVRulerMetric(FieldUnit& eToFill) const
{
    eToFill = m_pVRuler->GetUnit();
}

void SwView::GetHRulerMetric(FieldUnit& eToFill) const
{
    eToFill = m_pHRuler->GetUnit();
}

int SwView::CreateVRuler()
{
    m_pHRuler->SetBorderPos( m_pVRuler->GetSizePixel().Width()-1 );

    m_pVRuler->SetActive(GetFrame() && IsActive());
    m_pVRuler->Show();
    InvalidateBorder();
    return 1;
}

int SwView::KillVRuler()
{
    m_pVRuler->Hide();
    m_pHRuler->SetBorderPos();
    InvalidateBorder();
    return 1;
}

IMPL_LINK_TYPED( SwView, ExecRulerClick, Ruler *, pRuler, void )
{
    OUString sDefPage;
    switch( pRuler->GetClickType() )
    {
        case RULER_TYPE_DONTKNOW:
        case RULER_TYPE_OUTSIDE:
        case RULER_TYPE_INDENT:
        case RULER_TYPE_MARGIN1:
        case RULER_TYPE_MARGIN2:
            sDefPage = "indents";
        break;
        default:
            sDefPage = "tabs";

    }

    SfxStringItem aDefPage(SID_PARA_DLG, sDefPage);
    GetViewFrame()->GetDispatcher()->Execute( SID_PARA_DLG,
                                SfxCallMode::SYNCHRON|SfxCallMode::RECORD,
                                &aDefPage, 0L );
}

sal_uInt16 SwView::GetMoveType()
{
    return m_nMoveType;
}

void SwView::SetMoveType(sal_uInt16 nSet)
{
    m_nMoveType = nSet;
}

void SwView::SetActMark(sal_Int32 nSet)
{
    m_nActMark = nSet;
}

void SwView::ShowHScrollbar(bool bShow)
{
    assert(m_pHScrollbar && "Scrollbar invalid");
    m_pHScrollbar->ExtendedShow(bShow);
}

bool SwView::IsHScrollbarVisible()const
{
    assert(m_pHScrollbar && "Scrollbar invalid");
    return m_pHScrollbar->IsVisible( false ) || m_pHScrollbar->IsAuto();
}

void SwView::ShowVScrollbar(bool bShow)
{
    assert(m_pVScrollbar && "Scrollbar invalid");
    m_pVScrollbar->ExtendedShow(bShow);
}

bool SwView::IsVScrollbarVisible()const
{
    assert(m_pVScrollbar && "Scrollbar invalid");
    return m_pVScrollbar->IsVisible( false );
}

void SwView::EnableHScrollbar(bool bEnable)
{
    if (m_bHScrollbarEnabled != bEnable)
    {
        m_bHScrollbarEnabled = bEnable;
        InvalidateBorder();
    }
}

void SwView::EnableVScrollbar(bool bEnable)
{
    if (m_bVScrollbarEnabled != bEnable)
    {
        m_bVScrollbarEnabled = bEnable;
        InvalidateBorder();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
