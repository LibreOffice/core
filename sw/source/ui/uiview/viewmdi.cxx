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


#define VIEW_IMAGECOLOR COL_LIGHTBLUE

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

void SwView::SetZoom( SvxZoomType eZoomType, short nFactor, sal_Bool bViewOnly )
{
    bool const bCrsrIsVisible(m_pWrtShell->IsCrsrVisible());
    _SetZoom( GetEditWin().GetOutputSizePixel(), eZoomType, nFactor, bViewOnly );
    // fdo#40465 force the cursor to stay in view whilst zooming
    if (bCrsrIsVisible)
        m_pWrtShell->ShowCrsr();
}

void SwView::_SetZoom( const Size &rEditSize, SvxZoomType eZoomType,
                        short nFactor, sal_Bool bViewOnly )
{
    sal_Bool bUnLockView = !m_pWrtShell->IsViewLocked();
    m_pWrtShell->LockView( sal_True );
    m_pWrtShell->LockPaint();

    {
    SwActContext aActContext(m_pWrtShell);

    long nFac = nFactor;

    sal_Bool bWeb = 0 != PTR_CAST(SwWebView, this);
    SwMasterUsrPref *pUsrPref = (SwMasterUsrPref*)SW_MOD()->GetUsrPref(bWeb);

    const SwPageDesc &rDesc = m_pWrtShell->GetPageDesc( m_pWrtShell->GetCurPageDesc() );
    const SvxLRSpaceItem &rLRSpace = rDesc.GetMaster().GetLRSpace();
    const SwViewOption *pOpt = m_pWrtShell->GetViewOptions();
    long lLeftMargin = 0;

    if( eZoomType != SVX_ZOOM_PERCENT )
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

        if( SVX_ZOOM_OPTIMAL == eZoomType )
        {
            if (!pPostItMgr->HasNotes() || !pPostItMgr->ShowNotes())
                aPageSize.Width() -= ( rLRSpace.GetLeft() + rLRSpace.GetRight() + nLeftOfst * 2 );
            lLeftMargin = long(rLRSpace.GetLeft()) + DOCUMENTBORDER + nLeftOfst;
            nFac = aWindowSize.Width() * 100 / aPageSize.Width();
        }
        else if(SVX_ZOOM_WHOLEPAGE == eZoomType || SVX_ZOOM_PAGEWIDTH == eZoomType )
        {
            const long nOf = DOCUMENTBORDER * 2L;
            long nTmpWidth = bAutomaticViewLayout ? aPageSize.Width() : aRootSize.Width();
            nTmpWidth += nOf;
            aPageSize.Height() += nOf;
            nFac = aWindowSize.Width() * 100 / nTmpWidth;

            if ( SVX_ZOOM_WHOLEPAGE == eZoomType )
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

    SwViewOption aOpt( *pOpt );
    if ( !GetViewFrame()->GetFrame().IsInPlace() )
    {
        //Update MasterUsrPrefs and after that update the ViewOptions of the current View.
        if ( !bViewOnly &&
                (sal_uInt16(nFac)      != pUsrPref->GetZoom() ||
                sal_uInt8  (eZoomType) != pUsrPref->GetZoomType()) )
        {
            pUsrPref->SetZoom    ( sal_uInt16(nFac) );
            pUsrPref->SetZoomType( eZoomType );
            SW_MOD()->ApplyUsrPref( *pUsrPref,
                    bViewOnly ? this: 0,
                    bViewOnly ? VIEWOPT_DEST_VIEW_ONLY : 0 );
            pUsrPref->SetModified();
        }
        if ( pOpt->GetZoom() != (sal_uInt16) nFac )
        {
            aOpt.SetZoom    ( sal_uInt16(nFac) );
            aOpt.SetReadonly(pOpt->IsReadonly());
            m_pWrtShell->ApplyViewOptions( aOpt );
        }
        if ( eZoomType != SVX_ZOOM_PERCENT )
        {
            Point aPos;

            if ( eZoomType == SVX_ZOOM_WHOLEPAGE )
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
        ((SwViewOption*)m_pWrtShell->GetViewOptions())->SetZoomType( eZoomType );
        CalcVisArea( rEditSize );   // for the recalculation of the viewable area
    }
    else if ( sal_uInt16(nFac) != pOpt->GetZoom() )
    {
        aOpt.SetZoom    ( sal_uInt16(nFac) );
        m_pWrtShell->ApplyViewOptions( aOpt );
    }

    const Fraction aFrac( nFac, 100 );
    m_pVRuler->SetZoom( aFrac );
    m_pVRuler->ForceUpdate();
    m_pHRuler->SetZoom( aFrac );
    m_pHRuler->ForceUpdate();
    ((SwViewOption*)m_pWrtShell->GetViewOptions())->SetZoomType( eZoomType );
    }
    m_pWrtShell->UnlockPaint();
    if( bUnLockView )
        m_pWrtShell->LockView( sal_False );
}

void SwView::SetViewLayout( sal_uInt16 nColumns, bool bBookMode, sal_Bool bViewOnly )
{
    const sal_Bool bUnLockView = !m_pWrtShell->IsViewLocked();
    m_pWrtShell->LockView( sal_True );
    m_pWrtShell->LockPaint();

    {

    SwActContext aActContext(m_pWrtShell);

    if ( !GetViewFrame()->GetFrame().IsInPlace() && !bViewOnly )
    {
        const sal_Bool bWeb = 0 != PTR_CAST(SwWebView, this);
        SwMasterUsrPref *pUsrPref = (SwMasterUsrPref*)SW_MOD()->GetUsrPref(bWeb);

        // Update MasterUsrPrefs and after that update the ViewOptions of the current View.
        if ( nColumns  != pUsrPref->GetViewLayoutColumns() ||
             bBookMode != pUsrPref->IsViewLayoutBookMode() )
        {
            pUsrPref->SetViewLayoutColumns( nColumns );
            pUsrPref->SetViewLayoutBookMode( bBookMode );
            SW_MOD()->ApplyUsrPref( *pUsrPref,
                    bViewOnly ? this: 0,
                    bViewOnly ? VIEWOPT_DEST_VIEW_ONLY : 0 );
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
        m_pWrtShell->LockView( sal_False );

    SfxBindings& rBnd = GetViewFrame()->GetBindings();
    rBnd.Invalidate( SID_ATTR_VIEWLAYOUT );
    rBnd.Invalidate( SID_ATTR_ZOOMSLIDER);
}

// Scrollbar - Handler

IMPL_LINK( SwView, WindowChildEventListener, VclSimpleEvent*, pEvent )
{
    OSL_ENSURE( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );
    if ( pEvent && pEvent->ISA( VclWindowEvent ) )
    {
        VclWindowEvent *pVclEvent = static_cast< VclWindowEvent * >( pEvent );
        OSL_ENSURE( pVclEvent->GetWindow(), "Window???" );
        Window* pChildWin = static_cast< Window* >( pVclEvent->GetData() );

        switch ( pVclEvent->GetId() )
        {
            case VCLEVENT_WINDOW_HIDE:
                if( pChildWin == m_pHScrollbar )
                    ShowHScrollbar( sal_False );
                else if( pChildWin == m_pVScrollbar )
                    ShowVScrollbar( sal_False );
                break;
            case VCLEVENT_WINDOW_SHOW:
                if( pChildWin == m_pHScrollbar )
                    ShowHScrollbar( sal_True );
                else if( pChildWin == m_pVScrollbar )
                    ShowVScrollbar( sal_True );
                break;
        }
    }

    return 0;
}

int SwView::_CreateScrollbar( sal_Bool bHori )
{
    Window *pMDI = &GetViewFrame()->GetWindow();
    SwScrollbar** ppScrollbar = bHori ? &m_pHScrollbar : &m_pVScrollbar;

    OSL_ENSURE( !*ppScrollbar, "check beforehand!" );

    if( !bHori )
        CreatePageButtons( !m_bShowAtResize );

    *ppScrollbar = new SwScrollbar( pMDI, bHori );
    UpdateScrollbars();
    if(bHori)
        (*ppScrollbar)->SetScrollHdl( LINK( this, SwView, EndScrollHdl ));
    else
        (*ppScrollbar)->SetScrollHdl( LINK( this, SwView, ScrollHdl ));
    (*ppScrollbar)->SetEndScrollHdl( LINK( this, SwView, EndScrollHdl ));

    (*ppScrollbar)->EnableDrag( sal_True );

    if(GetWindow())
        InvalidateBorder();

    // The scrollbar has to be tested again, as in InvalidateBorder possibly
    // the scrollbar has been deleted.
    if ( !m_bShowAtResize && (*ppScrollbar))
        (*ppScrollbar)->ExtendedShow();

    return 1;
}

void SwView::CreatePageButtons(sal_Bool bShow)
{
    Window *pMDI = &GetViewFrame()->GetWindow();
    m_pPageUpBtn      = new SwHlpImageButton(pMDI, SW_RES( BTN_PAGEUP ), sal_True );
    m_pPageUpBtn->SetHelpId(HID_SCRL_PAGEUP);
    m_pPageDownBtn    = new SwHlpImageButton(pMDI, SW_RES( BTN_PAGEDOWN ), sal_False );
    m_pPageDownBtn->SetHelpId(HID_SCRL_PAGEDOWN);
    Reference< XFrame > xFrame = GetViewFrame()->GetFrame().GetFrameInterface();
    m_pNaviBtn = new SwNaviImageButton(pMDI, xFrame );
    m_pNaviBtn->SetHelpId(HID_SCRL_NAVI);
    Link aLk( LINK( this, SwView, BtnPage ) );
    m_pPageUpBtn->SetClickHdl( aLk );
    m_pPageDownBtn->SetClickHdl( aLk );
    if(m_nMoveType != NID_PGE)
    {
        Color aColor(VIEW_IMAGECOLOR);
        SetImageButtonColor(aColor);
    }

    if(bShow)
    {
        m_pPageUpBtn->Show();
        m_pPageDownBtn->Show();
        m_pNaviBtn->Show();
    }
};

// Button-Handler

IMPL_LINK( SwView, BtnPage, Button *, pButton )
{
    // #i75416# move the execution of the search to an asynchronously called static link
    bool* pbNext = new bool( (pButton == m_pPageDownBtn) );
    Application::PostUserEvent( STATIC_LINK(this, SwView, MoveNavigationHdl), pbNext );
    return 0;
}

IMPL_STATIC_LINK( SwView, MoveNavigationHdl, bool *, pbNext )
{
    if ( !pbNext )
        return 0;
    bool bNext = *pbNext;
    SwWrtShell& rSh = pThis->GetWrtShell();
    switch( m_nMoveType )
    {
        case NID_PGE:
            bNext ? pThis->PhyPageDown() : pThis->PhyPageUp();
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
            sal_uInt16 eType = GOTOOBJ_FLY_FRM;
            if(m_nMoveType == NID_GRF)
                eType = GOTOOBJ_FLY_GRF;
            else if(m_nMoveType == NID_OLE)
                eType = GOTOOBJ_FLY_OLE;
            sal_Bool bSuccess = bNext ?
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
                        GOTOOBJ_DRAW_SIMPLE :
                        GOTOOBJ_DRAW_CONTROL);
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
            pThis->GetViewFrame()->GetDispatcher()->Execute(bNext ?
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
                rSh.GotoNextFtnAnchor() :
                    rSh.GotoPrevFtnAnchor();
        break;
        case NID_MARK:
        {
            // unselect
            rSh.MoveCrsr();
            rSh.EnterStdMode();

            // collect navigator reminders
            IDocumentMarkAccess* const pMarkAccess = rSh.getIDocumentMarkAccess();
            ::std::vector< const ::sw::mark::IMark* > vNavMarks;
            for( IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getMarksBegin();
                ppMark != pMarkAccess->getMarksEnd();
                ppMark++)
            {
                if( IDocumentMarkAccess::GetType(**ppMark) == IDocumentMarkAccess::NAVIGATOR_REMINDER )
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
            sw::sidebarwindows::SwSidebarWin* pPostIt = pThis->GetPostItMgr()->GetActiveSidebarWin();
            if (pPostIt)
                pThis->GetPostItMgr()->SetActiveSidebarWin(0);
            SwFieldType* pFldType = rSh.GetFldType(0, RES_POSTITFLD);
            if (rSh.MoveFldType(pFldType, bNext))
                pThis->GetViewFrame()->GetDispatcher()->Execute(FN_POSTIT);
            else
                //first/last item
                pThis->GetPostItMgr()->SetActiveSidebarWin(pPostIt);
        }
        break;
        case NID_SRCH_REP:
        if(m_pSrchItem)
        {
            sal_Bool bBackward = m_pSrchItem->GetBackward();
            if(rSh.HasSelection() && !bNext == rSh.IsCrsrPtAtEnd())
                rSh.SwapPam();
            m_pSrchItem->SetBackward(!bNext);
            SfxRequest aReq(FN_REPEAT_SEARCH, SFX_CALLMODE_SLOT, pThis->GetPool());
            pThis->ExecSearch(aReq);
            m_pSrchItem->SetBackward(bBackward);
        }
        break;
        case NID_INDEX_ENTRY:
            rSh.GotoNxtPrvTOXMark(bNext);
        break;

        case NID_TABLE_FORMULA:
            rSh.GotoNxtPrvTblFormula( bNext );
            break;

        case NID_TABLE_FORMULA_ERROR:
            rSh.GotoNxtPrvTblFormula( bNext, sal_True );
            break;
    }
    pThis->m_pEditWin->GrabFocus();
    delete pbNext;
    return 0;
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
    m_pHRuler->SetBorderPos( 0 );
    InvalidateBorder();
    return 1;
}

IMPL_LINK( SwView, ExecRulerClick, Ruler *, pRuler )
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
                                SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD,
                                &aDefPage, 0L );
    return 0;
}

sal_uInt16 SwView::GetMoveType()
{
    return m_nMoveType;
}

void SwView::SetMoveType(sal_uInt16 nSet)
{
    sal_Bool bLastPage = m_nMoveType == NID_PGE;
    m_nMoveType = nSet;
    sal_Bool bNewPage = m_nMoveType == NID_PGE;
    if(bNewPage != bLastPage)
    {
        Color aColor(bNewPage ? COL_BLACK : VIEW_IMAGECOLOR);
        const TypeId aTypeId = TYPE(SwView);
        SwView* pView = (SwView*)SfxViewShell::GetFirst(&aTypeId);
        while( pView )
        {
            pView->SetImageButtonColor(aColor);
            pView = (SwView*)SfxViewShell::GetNext(*pView, &aTypeId);
        }
    }
}

void SwView::SetActMark(sal_Int32 nSet)
{
    m_nActMark = nSet;
}

void SwView::SetImageButtonColor(Color& rColor)
{
    if(m_pPageUpBtn)
    {
        m_pPageUpBtn->SetControlForeground(rColor);
        m_pPageDownBtn->SetControlForeground(rColor);
    }
}

void SwView::ShowHScrollbar(sal_Bool bShow)
{
    OSL_ENSURE(m_pHScrollbar, "Scrollbar invalid");
    m_pHScrollbar->ExtendedShow(bShow);
}

sal_Bool SwView::IsHScrollbarVisible()const
{
    OSL_ENSURE(m_pHScrollbar, "Scrollbar invalid");
    return m_pHScrollbar->IsVisible( sal_False ) || m_pHScrollbar->IsAuto();
}

void SwView::ShowVScrollbar(sal_Bool bShow)
{
    OSL_ENSURE(m_pVScrollbar, "Scrollbar invalid");
    m_pVScrollbar->ExtendedShow(bShow);
    m_pPageUpBtn->Show(bShow);
    m_pPageDownBtn->Show(bShow);
    m_pNaviBtn->Show(bShow);
}

sal_Bool SwView::IsVScrollbarVisible()const
{
    OSL_ENSURE(m_pVScrollbar, "Scrollbar invalid");
    return m_pVScrollbar->IsVisible( sal_False );
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
