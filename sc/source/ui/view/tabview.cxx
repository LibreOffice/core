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

#include <scitems.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
#include <vcl/cursor.hxx>
#include <sal/log.hxx>
#include <tools/svborder.hxx>
#include <tools/json_writer.hxx>
#include <o3tl/unit_conversion.hxx>

#include <pagedata.hxx>
#include <tabview.hxx>
#include <tabvwsh.hxx>
#include <document.hxx>
#include <gridwin.hxx>
#include <olinewin.hxx>
#include <olinetab.hxx>
#include <tabsplit.hxx>
#include <colrowba.hxx>
#include <tabcont.hxx>
#include <scmod.hxx>
#include <sc.hrc>
#include <globstr.hrc>
#include <scresid.hxx>
#include <drawview.hxx>
#include <docsh.hxx>
#include <viewuno.hxx>
#include <appoptio.hxx>
#include <attrib.hxx>
#include <spellcheckcontext.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>
#include <osl/diagnose.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>

#include <algorithm>

#include <basegfx/utils/zoomtools.hxx>

#define SPLIT_MARGIN      30
#define SPLIT_HANDLE_SIZE 5
constexpr sal_Int32 TAB_HEIGHT_MARGIN = 10;

#define SC_ICONSIZE     36

#define SC_SCROLLBAR_MIN 30
#define SC_TABBAR_MIN 6

using namespace ::com::sun::star;

//  Corner-Button

ScCornerButton::ScCornerButton( vcl::Window* pParent, ScViewData* pData ) :
    Window( pParent, WinBits( 0 ) ),
    pViewData( pData )
{
    EnableRTL( false );
}

ScCornerButton::~ScCornerButton()
{
}

void ScCornerButton::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    SetBackground(rStyleSettings.GetFaceColor());

    Size aSize(GetOutputSizePixel());
    tools::Long nPosX = aSize.Width() - 1;
    tools::Long nPosY = aSize.Height() - 1;

    Window::Paint(rRenderContext, rRect);

    bool bLayoutRTL = pViewData->GetDocument().IsLayoutRTL( pViewData->GetTabNo() );
    tools::Long nDarkX = bLayoutRTL ? 0 : nPosX;

    //  both buttons have the same look now - only dark right/bottom lines
    rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
    rRenderContext.DrawLine(Point(0, nPosY), Point(nPosX, nPosY));
    rRenderContext.DrawLine(Point(nDarkX, 0), Point(nDarkX, nPosY));
}

void ScCornerButton::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( rStyleSettings.GetFaceColor() );
    Invalidate();
}

void ScCornerButton::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( rStyleSettings.GetFaceColor() );
    Invalidate();
}

void ScCornerButton::Resize()
{
    Invalidate();
}

void ScCornerButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    ScModule* pScMod = SC_MOD();
    bool bDisable = pScMod->IsFormulaMode() || pScMod->IsModalMode();
    if (!bDisable)
    {
        ScTabViewShell* pViewSh = pViewData->GetViewShell();
        pViewSh->SetActive();                                   // Appear and SetViewFrame
        pViewSh->ActiveGrabFocus();

        bool bControl = rMEvt.IsMod1();
        pViewSh->SelectAll( bControl );
    }
}
namespace
{

bool lcl_HasColOutline( const ScViewData& rViewData )
{
    const ScOutlineTable* pTable = rViewData.GetDocument().GetOutlineTable(rViewData.GetTabNo());
    if (pTable)
    {
        const ScOutlineArray& rArray = pTable->GetColArray();
        if ( rArray.GetDepth() > 0 )
            return true;
    }
    return false;
}

bool lcl_HasRowOutline( const ScViewData& rViewData )
{
    const ScOutlineTable* pTable = rViewData.GetDocument().GetOutlineTable(rViewData.GetTabNo());
    if (pTable)
    {
        const ScOutlineArray& rArray = pTable->GetRowArray();
        if ( rArray.GetDepth() > 0 )
            return true;
    }
    return false;
}

} // anonymous namespace

ScTabView::ScTabView( vcl::Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell ) :
    pFrameWin( pParent ),
    aViewData( rDocSh, pViewShell ),
    aFunctionSet( &aViewData ),
    aHdrFunc( &aViewData ),
    aVScrollTop( VclPtr<ScrollAdaptor>::Create( pFrameWin, false ) ),
    aVScrollBottom( VclPtr<ScrollAdaptor>::Create( pFrameWin, false ) ),
    aHScrollLeft( VclPtr<ScrollAdaptor>::Create( pFrameWin, true ) ),
    aHScrollRight( VclPtr<ScrollAdaptor>::Create( pFrameWin, true ) ),
    aCornerButton( VclPtr<ScCornerButton>::Create( pFrameWin, &aViewData ) ),
    aTopButton( VclPtr<ScCornerButton>::Create( pFrameWin, &aViewData ) ),
    aScrollTimer("ScTabView aScrollTimer"),
    pTimerWindow( nullptr ),
    aExtraEditViewManager( pViewShell, pGridWin ),
    nTipVisible( nullptr ),
    nTipAlign( QuickHelpFlags::NONE ),
    nPrevDragPos( 0 ),
    meBlockMode(None),
    meHighlightBlockMode(None),
    nBlockStartX( 0 ),
    nBlockStartXOrig( 0 ),
    nBlockEndX( 0 ),
    nBlockStartY( 0 ),
    nBlockStartYOrig( 0 ),
    nBlockEndY( 0 ),
    nBlockStartZ( 0 ),
    nBlockEndZ( 0 ),
    nOldCurX( 0 ),
    nOldCurY( 0 ),
    mfPendingTabBarWidth( -1.0 ),
    mnLOKStartHeaderRow( -2 ),
    mnLOKEndHeaderRow( -1 ),
    mnLOKStartHeaderCol( -2 ),
    mnLOKEndHeaderCol( -1 ),
    bMinimized( false ),
    bInUpdateHeader( false ),
    bInActivatePart( false ),
    bInZoomUpdate( false ),
    bMoveIsShift( false ),
    bDrawSelMode( false ),
    bLockPaintBrush( false ),
    bDragging( false ),
    bBlockNeg( false ),
    bBlockCols( false ),
    bBlockRows( false ),
    mbInlineWithScrollbar( false )
{
    // copy settings of existing shell for this document
    if (ScTabViewShell* pExistingViewShell = rDocSh.GetBestViewShell())
    {
        aViewRenderingData = pExistingViewShell->GetViewRenderingData();
        EnableAutoSpell(pExistingViewShell->IsAutoSpell());
    }
    Init();
}

void ScTabView::InitScrollBar(ScrollAdaptor& rScrollBar, tools::Long nMaxVal, const Link<weld::Scrollbar&, void>& rLink)
{
    rScrollBar.SetRange( Range( 0, nMaxVal ) );
    rScrollBar.SetLineSize( 1 );
    rScrollBar.SetPageSize( 1 );                // is queried separately
    rScrollBar.SetVisibleSize( 10 );            // is reset by Resize

    rScrollBar.SetScrollHdl(rLink);
    rScrollBar.SetMouseReleaseHdl(LINK(this, ScTabView, EndScrollHdl));

    rScrollBar.EnableRTL( aViewData.GetDocument().IsLayoutRTL( aViewData.GetTabNo() ) );

    // Related: tdf#155266 Eliminate delayed scrollbar redrawing when swiping
    // By default, the layout idle timer in the InterimWindowItem class
    // is set to TaskPriority::RESIZE. That is too high of a priority as
    // it appears that other timers are drawing after the scrollbar has been
    // redrawn.
    // As a result, when swiping, the content moves fluidly but the scrollbar
    // thumb does not move until after swiping stops or pauses. Then, after a
    // short lag, the scrollbar thumb finally "jumps" to the expected
    // position.
    // So, to fix this scrollbar "stickiness" when swiping, setting the
    // priority to TaskPriority::POST_PAINT causes the scrollbar to be
    // redrawn after any competing timers.
    rScrollBar.SetPriority(TaskPriority::POST_PAINT);
}

//  Scroll-Timer
void ScTabView::SetTimer( ScGridWindow* pWin, const MouseEvent& rMEvt )
{
    pTimerWindow = pWin;
    aTimerMEvt   = rMEvt;
    aScrollTimer.Start();
}

void ScTabView::ResetTimer()
{
    aScrollTimer.Stop();
    pTimerWindow = nullptr;
}

IMPL_LINK_NOARG(ScTabView, TimerHdl, Timer *, void)
{
    if (pTimerWindow)
        pTimerWindow->MouseMove( aTimerMEvt );
}

// --- Resize ---------------------------------------------------------------------

static void lcl_SetPosSize( vcl::Window& rWindow, const Point& rPos, const Size& rSize,
                            tools::Long nTotalWidth, bool bLayoutRTL )
{
    Point aNewPos = rPos;
    if ( bLayoutRTL )
    {
        aNewPos.setX( nTotalWidth - rPos.X() - rSize.Width() );
        if ( aNewPos == rWindow.GetPosPixel() && rSize.Width() != rWindow.GetSizePixel().Width() )
        {
            //  Document windows are manually painted right-to-left, so they need to
            //  be repainted if the size changes.
            rWindow.Invalidate();
        }
    }
    rWindow.SetPosSizePixel( aNewPos, rSize );
}

void ScTabView::DoResize( const Point& rOffset, const Size& rSize, bool bInner )
{
    HideListBox();

    bool bHasHint = HasHintWindow();
    if (bHasHint)
        RemoveHintWindow();

    bool bLayoutRTL = aViewData.GetDocument().IsLayoutRTL( aViewData.GetTabNo() );
    tools::Long nTotalWidth = rSize.Width();
    if ( bLayoutRTL )
        nTotalWidth += 2*rOffset.X();

    bool bVScroll    = aViewData.IsVScrollMode();
    bool bHScroll    = aViewData.IsHScrollMode();
    bool bTabControl = aViewData.IsTabMode();
    bool bHeaders    = aViewData.IsHeaderMode();
    bool bOutlMode   = aViewData.IsOutlineMode();
    bool bHOutline   = bOutlMode && lcl_HasColOutline(aViewData);
    bool bVOutline   = bOutlMode && lcl_HasRowOutline(aViewData);

    if ( aViewData.GetDocShell()->IsPreview() )
        bHScroll = bVScroll = bTabControl = bHeaders = bHOutline = bVOutline = false;

    tools::Long nBarX = 0;
    tools::Long nBarY = 0;
    tools::Long nOutlineX = 0;
    tools::Long nOutlineY = 0;
    tools::Long nOutPosX;
    tools::Long nOutPosY;

    tools::Long nPosX = rOffset.X();
    tools::Long nPosY = rOffset.Y();
    tools::Long nSizeX = rSize.Width();
    tools::Long nSizeY = rSize.Height();

    bMinimized = ( nSizeX<=SC_ICONSIZE || nSizeY<=SC_ICONSIZE );
    if ( bMinimized )
        return;

    float fScaleFactor = pFrameWin->GetDPIScaleFactor();

    tools::Long nSplitSizeX = SPLIT_HANDLE_SIZE * fScaleFactor;
    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
        nSplitSizeX = 1;
    tools::Long nSplitSizeY = SPLIT_HANDLE_SIZE * fScaleFactor;
    if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
        nSplitSizeY = 1;

    aBorderPos = rOffset;
    aFrameSize = rSize;

    const StyleSettings& rStyleSettings = pFrameWin->GetSettings().GetStyleSettings();


    Size aFontSize = rStyleSettings.GetTabFont().GetFontSize();
    MapMode aPtMapMode(MapUnit::MapPoint);
    aFontSize = pFrameWin->LogicToPixel(aFontSize, aPtMapMode);
    sal_Int32 nTabHeight = aFontSize.Height() + TAB_HEIGHT_MARGIN;

    if ( aViewData.GetHSplitMode() != SC_SPLIT_NONE )
    {
        if ( aViewData.GetHSplitPos() > nSizeX - SPLIT_MARGIN )
        {
            aViewData.SetHSplitMode( SC_SPLIT_NONE );
            if ( WhichH( aViewData.GetActivePart() ) == SC_SPLIT_RIGHT )
                ActivatePart( SC_SPLIT_BOTTOMLEFT );
            InvalidateSplit();
        }
    }
    if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
    {
        if ( aViewData.GetVSplitPos() > nSizeY - SPLIT_MARGIN )
        {
            aViewData.SetVSplitMode( SC_SPLIT_NONE );
            if ( WhichV( aViewData.GetActivePart() ) == SC_SPLIT_TOP )
                ActivatePart( SC_SPLIT_BOTTOMLEFT );
            InvalidateSplit();
        }
    }

    UpdateShow();

    if (bHScroll || bVScroll)       // Scrollbars horizontal or vertical
    {
        tools::Long nScrollBarSize = rStyleSettings.GetScrollBarSize();
        if (bVScroll)
        {
            nBarX = nScrollBarSize;
            nSizeX -= nBarX;
        }
        if (bHScroll)
        {
            nBarY = nTabHeight;

            if (!mbInlineWithScrollbar)
                nBarY += nScrollBarSize;

            nSizeY -= nBarY;
        }

        if (bHScroll) // Scrollbars horizontal
        {
            tools::Long nSizeLt = 0;       // left scroll bar
            tools::Long nSizeRt = 0;       // right scroll bar
            tools::Long nSizeSp = 0;       // splitter

            switch (aViewData.GetHSplitMode())
            {
                case SC_SPLIT_NONE:
                    nSizeSp = nSplitSizeX;
                    nSizeLt = nSizeX - nSizeSp; // Convert the corner
                    break;
                case SC_SPLIT_NORMAL:
                    nSizeSp = nSplitSizeX;
                    nSizeLt = aViewData.GetHSplitPos();
                    break;
                case SC_SPLIT_FIX:
                    nSizeSp = 0;
                    nSizeLt = 0;
                    break;
            }
            nSizeRt = nSizeX - nSizeLt - nSizeSp;

            tools::Long nTabSize = 0;

            if (bTabControl)
            {
                // pending relative tab bar width from extended document options
                if( mfPendingTabBarWidth >= 0.0 )
                {
                    SetRelTabBarWidth( mfPendingTabBarWidth );
                    mfPendingTabBarWidth = -1.0;
                }

                if (mbInlineWithScrollbar)
                {
                    nTabSize = pTabControl->GetSizePixel().Width();

                    if ( aViewData.GetHSplitMode() != SC_SPLIT_FIX ) // left Scrollbar
                    {
                        if (nTabSize > nSizeLt-SC_SCROLLBAR_MIN)
                            nTabSize = nSizeLt-SC_SCROLLBAR_MIN;
                        if (nTabSize < SC_TABBAR_MIN)
                            nTabSize = SC_TABBAR_MIN;
                        nSizeLt -= nTabSize;
                    }
                    else // right Scrollbar
                    {
                        if (nTabSize > nSizeRt-SC_SCROLLBAR_MIN)
                            nTabSize = nSizeRt-SC_SCROLLBAR_MIN;
                        if (nTabSize < SC_TABBAR_MIN)
                            nTabSize = SC_TABBAR_MIN;
                        nSizeRt -= nTabSize;
                    }
                }
            }

            if (mbInlineWithScrollbar)
            {
                Point aTabPoint(nPosX, nPosY + nSizeY);
                Size aTabSize(nTabSize, nBarY);
                lcl_SetPosSize(*pTabControl, aTabPoint, aTabSize, nTotalWidth, bLayoutRTL);
                pTabControl->SetSheetLayoutRTL(bLayoutRTL);

                Point aHScrollLeftPoint(nPosX + nTabSize, nPosY + nSizeY);
                Size aHScrollLeftSize(nSizeLt, nBarY);
                lcl_SetPosSize(*aHScrollLeft, aHScrollLeftPoint, aHScrollLeftSize, nTotalWidth, bLayoutRTL);

                Point aHSplitterPoint(nPosX + nTabSize + nSizeLt, nPosY + nSizeY);
                Size aHSplitterSize(nSizeSp, nBarY);
                lcl_SetPosSize(*pHSplitter, aHSplitterPoint, aHSplitterSize, nTotalWidth, bLayoutRTL);

                Point aHScrollRightPoint(nPosX + nTabSize + nSizeLt + nSizeSp, nPosY + nSizeY);
                Size aHScrollRightSize(nSizeRt, nBarY);
                lcl_SetPosSize(*aHScrollRight, aHScrollRightPoint, aHScrollRightSize, nTotalWidth, bLayoutRTL);
            }
            else
            {
                Point aTabPoint(nPosX, nPosY + nSizeY + nScrollBarSize);
                Size aTabSize(nSizeX, nTabHeight);
                lcl_SetPosSize(*pTabControl, aTabPoint, aTabSize, nTotalWidth, bLayoutRTL);
                pTabControl->SetSheetLayoutRTL(bLayoutRTL);

                Point aHScrollLeftPoint(nPosX, nPosY + nSizeY);
                Size aHScrollLeftSize(nSizeLt, nScrollBarSize);
                lcl_SetPosSize(*aHScrollLeft, aHScrollLeftPoint, aHScrollLeftSize, nTotalWidth, bLayoutRTL);

                Point aHSplitterPoint(nPosX + nSizeLt, nPosY + nSizeY);
                Size aHSplitterSize(nSizeSp, nScrollBarSize);
                lcl_SetPosSize(*pHSplitter, aHSplitterPoint, aHSplitterSize, nTotalWidth, bLayoutRTL);

                Point aHScrollRightPoint(nPosX + nSizeLt + nSizeSp, nPosY + nSizeY);
                Size aHScrollRightSize(nSizeRt, nScrollBarSize);
                lcl_SetPosSize(*aHScrollRight, aHScrollRightPoint, aHScrollRightSize, nTotalWidth, bLayoutRTL);
            }
            //  SetDragRectPixel is done below
        }

        if (bVScroll)
        {
            tools::Long nSizeUp = 0;       // upper scroll bar
            tools::Long nSizeSp = 0;       // splitter
            tools::Long nSizeDn;           // lower scroll bar

            switch (aViewData.GetVSplitMode())
            {
                case SC_SPLIT_NONE:
                    nSizeUp = 0;
                    nSizeSp = nSplitSizeY;
                    break;
                case SC_SPLIT_NORMAL:
                    nSizeUp = aViewData.GetVSplitPos();
                    nSizeSp = nSplitSizeY;
                    break;
                case SC_SPLIT_FIX:
                    nSizeUp = 0;
                    nSizeSp = 0;
                    break;
            }
            nSizeDn = nSizeY - nSizeUp - nSizeSp;

            lcl_SetPosSize( *aVScrollTop, Point(nPosX + nSizeX, nPosY),
                                            Size(nBarX, nSizeUp), nTotalWidth, bLayoutRTL );
            lcl_SetPosSize( *pVSplitter, Point( nPosX + nSizeX, nPosY+nSizeUp ),
                                            Size( nBarX, nSizeSp ), nTotalWidth, bLayoutRTL );
            lcl_SetPosSize( *aVScrollBottom, Point(nPosX + nSizeX,
                                                nPosY + nSizeUp + nSizeSp),
                                            Size(nBarX, nSizeDn), nTotalWidth, bLayoutRTL );

            //  SetDragRectPixel is done below
        }
    }

    //  SetDragRectPixel also without Scrollbars etc., when already split
    if ( bHScroll || aViewData.GetHSplitMode() != SC_SPLIT_NONE )
        pHSplitter->SetDragRectPixel(
            tools::Rectangle( nPosX, nPosY, nPosX+nSizeX, nPosY+nSizeY ), pFrameWin );
    if ( bVScroll || aViewData.GetVSplitMode() != SC_SPLIT_NONE )
        pVSplitter->SetDragRectPixel(
            tools::Rectangle( nPosX, nPosY, nPosX+nSizeX, nPosY+nSizeY ), pFrameWin );

    if (bTabControl && ! bHScroll )
    {
        nBarY = aHScrollLeft->GetSizePixel().Height();

        tools::Long nSize1 = nSizeX;

        tools::Long nTabSize = nSize1;
        if (nTabSize < 0) nTabSize = 0;

        lcl_SetPosSize( *pTabControl, Point(nPosX, nPosY+nSizeY-nBarY),
                                        Size(nTabSize, nBarY), nTotalWidth, bLayoutRTL );
        nSizeY -= nBarY;

        if( bVScroll )
        {
            Size aVScrSize = aVScrollBottom->GetSizePixel();
            aVScrSize.AdjustHeight( -nBarY );
            aVScrollBottom->SetSizePixel( aVScrSize );
        }
    }

    nOutPosX = nPosX;
    nOutPosY = nPosY;

    // Outline-Controls
    if (bVOutline && pRowOutline[SC_SPLIT_BOTTOM])
    {
        nOutlineX = pRowOutline[SC_SPLIT_BOTTOM]->GetDepthSize();
        nSizeX -= nOutlineX;
        nPosX += nOutlineX;
    }
    if (bHOutline && pColOutline[SC_SPLIT_LEFT])
    {
        nOutlineY = pColOutline[SC_SPLIT_LEFT]->GetDepthSize();
        nSizeY -= nOutlineY;
        nPosY += nOutlineY;
    }

    if (bHeaders)                               // column/row header
    {
        nBarX = pRowBar[SC_SPLIT_BOTTOM]->GetSizePixel().Width();
        nBarY = pColBar[SC_SPLIT_LEFT]->GetSizePixel().Height();
        nSizeX -= nBarX;
        nSizeY -= nBarY;
        nPosX += nBarX;
        nPosY += nBarY;
    }
    else
        nBarX = nBarY = 0;

        // evaluate splitter

    tools::Long nLeftSize   = nSizeX;
    tools::Long nRightSize  = 0;
    tools::Long nTopSize    = 0;
    tools::Long nBottomSize = nSizeY;
    tools::Long nSplitPosX  = nPosX;
    tools::Long nSplitPosY  = nPosY;

    if ( aViewData.GetHSplitMode() != SC_SPLIT_NONE )
    {
        tools::Long nSplitHeight = rSize.Height();
        if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
        {
            // Do not allow freeze splitter to overlap scroll bar/tab bar
            if ( bHScroll )
                nSplitHeight -= aHScrollLeft->GetSizePixel().Height();
            else if ( bTabControl && pTabControl )
                nSplitHeight -= pTabControl->GetSizePixel().Height();
        }
        nSplitPosX = aViewData.GetHSplitPos();
        lcl_SetPosSize( *pHSplitter,
                        Point(nSplitPosX, nOutPosY),
                        Size(nSplitSizeX, nSplitHeight - nTabHeight), nTotalWidth, bLayoutRTL);
        nLeftSize = nSplitPosX - nPosX;
        nSplitPosX += nSplitSizeX;
        nRightSize = nSizeX - nLeftSize - nSplitSizeX;
    }
    if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
    {
        tools::Long nSplitWidth = rSize.Width();
        if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX && bVScroll )
            nSplitWidth -= aVScrollBottom->GetSizePixel().Width();
        nSplitPosY = aViewData.GetVSplitPos();
        lcl_SetPosSize( *pVSplitter,
            Point( nOutPosX, nSplitPosY ), Size( nSplitWidth, nSplitSizeY ), nTotalWidth, bLayoutRTL );
        nTopSize = nSplitPosY - nPosY;
        nSplitPosY += nSplitSizeY;
        nBottomSize = nSizeY - nTopSize - nSplitSizeY;
    }

    //  ShowHide for pColOutline / pRowOutline happens in UpdateShow

    if (bHOutline)                              // Outline-Controls
    {
        if (pColOutline[SC_SPLIT_LEFT])
        {
            pColOutline[SC_SPLIT_LEFT]->SetHeaderSize( nBarX );
            lcl_SetPosSize( *pColOutline[SC_SPLIT_LEFT],
                    Point(nPosX-nBarX,nOutPosY), Size(nLeftSize+nBarX,nOutlineY), nTotalWidth, bLayoutRTL );
        }
        if (pColOutline[SC_SPLIT_RIGHT])
        {
            pColOutline[SC_SPLIT_RIGHT]->SetHeaderSize( 0 );    // always call to update RTL flag
            lcl_SetPosSize( *pColOutline[SC_SPLIT_RIGHT],
                    Point(nSplitPosX,nOutPosY), Size(nRightSize,nOutlineY), nTotalWidth, bLayoutRTL );
        }
    }
    if (bVOutline)
    {
        if (nTopSize)
        {
            if (pRowOutline[SC_SPLIT_TOP] && pRowOutline[SC_SPLIT_BOTTOM])
            {
                pRowOutline[SC_SPLIT_TOP]->SetHeaderSize( nBarY );
                lcl_SetPosSize( *pRowOutline[SC_SPLIT_TOP],
                        Point(nOutPosX,nPosY-nBarY), Size(nOutlineX,nTopSize+nBarY), nTotalWidth, bLayoutRTL );
                pRowOutline[SC_SPLIT_BOTTOM]->SetHeaderSize( 0 );
                lcl_SetPosSize( *pRowOutline[SC_SPLIT_BOTTOM],
                        Point(nOutPosX,nSplitPosY), Size(nOutlineX,nBottomSize), nTotalWidth, bLayoutRTL );
            }
        }
        else if (pRowOutline[SC_SPLIT_BOTTOM])
        {
            pRowOutline[SC_SPLIT_BOTTOM]->SetHeaderSize( nBarY );
            lcl_SetPosSize( *pRowOutline[SC_SPLIT_BOTTOM],
                    Point(nOutPosX,nSplitPosY-nBarY), Size(nOutlineX,nBottomSize+nBarY), nTotalWidth, bLayoutRTL );
        }
    }
    if (bHOutline && bVOutline)
    {
        lcl_SetPosSize( *aTopButton, Point(nOutPosX,nOutPosY), Size(nOutlineX,nOutlineY), nTotalWidth, bLayoutRTL );
        aTopButton->Show();
    }
    else
        aTopButton->Hide();

    if (bHeaders)                               // column/row header
    {
        lcl_SetPosSize( *pColBar[SC_SPLIT_LEFT],
            Point(nPosX,nPosY-nBarY), Size(nLeftSize,nBarY), nTotalWidth, bLayoutRTL );
        if (pColBar[SC_SPLIT_RIGHT])
            lcl_SetPosSize( *pColBar[SC_SPLIT_RIGHT],
                Point(nSplitPosX,nPosY-nBarY), Size(nRightSize,nBarY), nTotalWidth, bLayoutRTL );

        if (pRowBar[SC_SPLIT_TOP])
            lcl_SetPosSize( *pRowBar[SC_SPLIT_TOP],
                Point(nPosX-nBarX,nPosY), Size(nBarX,nTopSize), nTotalWidth, bLayoutRTL );
        lcl_SetPosSize( *pRowBar[SC_SPLIT_BOTTOM],
            Point(nPosX-nBarX,nSplitPosY), Size(nBarX,nBottomSize), nTotalWidth, bLayoutRTL );

        lcl_SetPosSize( *aCornerButton, Point(nPosX-nBarX,nPosY-nBarY), Size(nBarX,nBarY), nTotalWidth, bLayoutRTL );
        aCornerButton->Show();
        pColBar[SC_SPLIT_LEFT]->Show();
        pRowBar[SC_SPLIT_BOTTOM]->Show();
    }
    else
    {
        aCornerButton->Hide();
        pColBar[SC_SPLIT_LEFT]->Hide();         // always here
        pRowBar[SC_SPLIT_BOTTOM]->Hide();
    }

                                            // Grid-Windows

    if (bInner)
    {
        tools::Long nInnerPosX = bLayoutRTL ? ( nTotalWidth - nPosX - nLeftSize ) : nPosX;
        pGridWin[SC_SPLIT_BOTTOMLEFT]->SetPosPixel( Point(nInnerPosX,nSplitPosY) );
    }
    else
    {
        lcl_SetPosSize( *pGridWin[SC_SPLIT_BOTTOMLEFT],
            Point(nPosX,nSplitPosY), Size(nLeftSize,nBottomSize), nTotalWidth, bLayoutRTL );
        if ( aViewData.GetHSplitMode() != SC_SPLIT_NONE )
            lcl_SetPosSize( *pGridWin[SC_SPLIT_BOTTOMRIGHT],
                Point(nSplitPosX,nSplitPosY), Size(nRightSize,nBottomSize), nTotalWidth, bLayoutRTL );
        if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
            lcl_SetPosSize( *pGridWin[SC_SPLIT_TOPLEFT],
                Point(nPosX,nPosY), Size(nLeftSize,nTopSize), nTotalWidth, bLayoutRTL );
        if ( aViewData.GetHSplitMode() != SC_SPLIT_NONE && aViewData.GetVSplitMode() != SC_SPLIT_NONE )
            lcl_SetPosSize( *pGridWin[SC_SPLIT_TOPRIGHT],
                Point(nSplitPosX,nPosY), Size(nRightSize,nTopSize), nTotalWidth, bLayoutRTL );
    }

                // update scroll bars

    if (!bInUpdateHeader)
    {
        UpdateScrollBars();     // don't reset scroll bars when scrolling
        UpdateHeaderWidth();

        InterpretVisible();     // have everything calculated before painting
    }

    if (bHasHint)
        TestHintWindow();       // reposition

    UpdateVarZoom();    //  update variable zoom types (after resizing GridWindows)

    if (aViewData.GetViewShell()->HasAccessibilityObjects())
        aViewData.GetViewShell()->BroadcastAccessibility(SfxHint(SfxHintId::ScAccWindowResized));
}

void ScTabView::UpdateVarZoom()
{
    //  update variable zoom types

    SvxZoomType eZoomType = GetZoomType();
    if (eZoomType == SvxZoomType::PERCENT || bInZoomUpdate)
        return;

    bInZoomUpdate = true;
    const Fraction& rOldX = GetViewData().GetZoomX();
    const Fraction& rOldY = GetViewData().GetZoomY();
    tools::Long nOldPercent = tools::Long(rOldY * 100);
    sal_uInt16 nNewZoom = CalcZoom( eZoomType, static_cast<sal_uInt16>(nOldPercent) );
    Fraction aNew( nNewZoom, 100 );

    if ( aNew != rOldX || aNew != rOldY )
    {
        SetZoom( aNew, aNew, false );   // always separately per sheet
        PaintGrid();
        PaintTop();
        PaintLeft();
        aViewData.GetViewShell()->GetViewFrame().GetBindings().Invalidate( SID_ATTR_ZOOM );
        aViewData.GetViewShell()->GetViewFrame().GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER );
        aViewData.GetBindings().Invalidate(SID_ZOOM_IN);
        aViewData.GetBindings().Invalidate(SID_ZOOM_OUT);
    }
    bInZoomUpdate = false;
}

void ScTabView::UpdateFixPos()
{
    bool bResize = false;
    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
        if (aViewData.UpdateFixX())
            bResize = true;
    if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
        if (aViewData.UpdateFixY())
            bResize = true;
    if (bResize)
        RepeatResize(false);
}

void ScTabView::RepeatResize( bool bUpdateFix )
{
    if ( bUpdateFix )
    {
        ScSplitMode eHSplit = aViewData.GetHSplitMode();
        ScSplitMode eVSplit = aViewData.GetVSplitMode();

        // #i46796# UpdateFixX / UpdateFixY uses GetGridOffset, which requires the
        // outline windows to be available. So UpdateShow has to be called before
        // (also called from DoResize).
        if ( eHSplit == SC_SPLIT_FIX || eVSplit == SC_SPLIT_FIX )
            UpdateShow();

        if ( eHSplit == SC_SPLIT_FIX )
            aViewData.UpdateFixX();
        if ( eVSplit == SC_SPLIT_FIX )
            aViewData.UpdateFixY();
    }

    DoResize( aBorderPos, aFrameSize );

    //! border must be reset ???
}

void ScTabView::GetBorderSize( SvBorder& rBorder, const Size& /* rSize */ )
{
    bool bScrollBars = aViewData.IsVScrollMode();
    bool bHeaders    = aViewData.IsHeaderMode();
    bool bOutlMode   = aViewData.IsOutlineMode();
    bool bHOutline   = bOutlMode && lcl_HasColOutline(aViewData);
    bool bVOutline   = bOutlMode && lcl_HasRowOutline(aViewData);
    bool bLayoutRTL  = aViewData.GetDocument().IsLayoutRTL( aViewData.GetTabNo() );

    rBorder = SvBorder();

    if (bScrollBars)                            // Scrollbars horizontal or vertical
    {
        rBorder.Right()  += aVScrollBottom->GetSizePixel().Width();
        rBorder.Bottom() += aHScrollLeft->GetSizePixel().Height();
    }

    // Outline-Controls
    if (bVOutline && pRowOutline[SC_SPLIT_BOTTOM])
        rBorder.Left() += pRowOutline[SC_SPLIT_BOTTOM]->GetDepthSize();
    if (bHOutline && pColOutline[SC_SPLIT_LEFT])
        rBorder.Top()  += pColOutline[SC_SPLIT_LEFT]->GetDepthSize();

    if (bHeaders)                               // column/row headers
    {
        rBorder.Left() += pRowBar[SC_SPLIT_BOTTOM]->GetSizePixel().Width();
        rBorder.Top()  += pColBar[SC_SPLIT_LEFT]->GetSizePixel().Height();
    }

    if ( bLayoutRTL )
        ::std::swap( rBorder.Left(), rBorder.Right() );
}

IMPL_LINK_NOARG(ScTabView, TabBarResize, TabBar*, void)
{
    if (!aViewData.IsHScrollMode())
        return;

    tools::Long nSize = pTabControl->GetSplitSize();

    if (aViewData.GetHSplitMode() != SC_SPLIT_FIX)
    {
        tools::Long nMax = pHSplitter->GetPosPixel().X();
        if( pTabControl->IsEffectiveRTL() )
            nMax = pFrameWin->GetSizePixel().Width() - nMax;
        --nMax;
        if (nSize>nMax) nSize = nMax;
    }

    if ( nSize != pTabControl->GetSizePixel().Width() )
    {
        pTabControl->SetSizePixel( Size( nSize,
                                    pTabControl->GetSizePixel().Height() ) );
        RepeatResize();
    }
}

void ScTabView::SetTabBarWidth( tools::Long nNewWidth )
{
    Size aSize = pTabControl->GetSizePixel();

    if ( aSize.Width() != nNewWidth )
    {
        aSize.setWidth( nNewWidth );
        pTabControl->SetSizePixel( aSize );
    }
}

void ScTabView::SetRelTabBarWidth( double fRelTabBarWidth )
{
    if( (0.0 <= fRelTabBarWidth) && (fRelTabBarWidth <= 1.0) )
        if( tools::Long nFrameWidth = pFrameWin->GetSizePixel().Width() )
            SetTabBarWidth( static_cast< tools::Long >( fRelTabBarWidth * nFrameWidth + 0.5 ) );
}

void ScTabView::SetPendingRelTabBarWidth( double fRelTabBarWidth )
{
    mfPendingTabBarWidth = fRelTabBarWidth;
    SetRelTabBarWidth( fRelTabBarWidth );
}

tools::Long ScTabView::GetTabBarWidth() const
{
    return pTabControl->GetSizePixel().Width();
}

double ScTabView::GetRelTabBarWidth()
{
    return 0.5;
}

ScGridWindow* ScTabView::GetActiveWin()
{
    ScSplitPos ePos = aViewData.GetActivePart();
    OSL_ENSURE(pGridWin[ePos],"no active window");
    return pGridWin[ePos];
}

void ScTabView::SetActivePointer( PointerStyle nPointer )
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
        if (pWin)
            pWin->SetPointer( nPointer );
}

void ScTabView::ActiveGrabFocus()
{
    ScSplitPos ePos = aViewData.GetActivePart();
    if (pGridWin[ePos])
        pGridWin[ePos]->GrabFocus();
}

ScSplitPos ScTabView::FindWindow( const vcl::Window* pWindow ) const
{
    ScSplitPos eVal = SC_SPLIT_BOTTOMLEFT;      // Default
    for (sal_uInt16 i=0; i<4; i++)
        if ( pGridWin[i] == pWindow )
            eVal = static_cast<ScSplitPos>(i);

    return eVal;
}

Point ScTabView::GetGridOffset() const
{
    Point aPos;

        // size as in DoResize

    bool bHeaders    = aViewData.IsHeaderMode();
    bool bOutlMode   = aViewData.IsOutlineMode();
    bool bHOutline   = bOutlMode && lcl_HasColOutline(aViewData);
    bool bVOutline   = bOutlMode && lcl_HasRowOutline(aViewData);

    // Outline-Controls
    if (bVOutline && pRowOutline[SC_SPLIT_BOTTOM])
        aPos.AdjustX(pRowOutline[SC_SPLIT_BOTTOM]->GetDepthSize() );
    if (bHOutline && pColOutline[SC_SPLIT_LEFT])
        aPos.AdjustY(pColOutline[SC_SPLIT_LEFT]->GetDepthSize() );

    if (bHeaders)                               // column/row headers
    {
        if (pRowBar[SC_SPLIT_BOTTOM])
            aPos.AdjustX(pRowBar[SC_SPLIT_BOTTOM]->GetSizePixel().Width() );
        if (pColBar[SC_SPLIT_LEFT])
            aPos.AdjustY(pColBar[SC_SPLIT_LEFT]->GetSizePixel().Height() );
    }

    return aPos;
}

// ---  Scroll-Bars  --------------------------------------------------------

void ScTabView::SetZoomPercentFromCommand(sal_uInt16 nZoomPercent)
{
    // scroll wheel doesn't set the AppOptions default

    bool bSyncZoom = SC_MOD()->GetAppOptions().GetSynchronizeZoom();
    SetZoomType(SvxZoomType::PERCENT, bSyncZoom);
    Fraction aFract(nZoomPercent, 100);
    SetZoom(aFract, aFract, bSyncZoom);
    PaintGrid();
    PaintTop();
    PaintLeft();
    aViewData.GetBindings().Invalidate( SID_ATTR_ZOOM);
    aViewData.GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER);
    aViewData.GetBindings().Invalidate( SID_ZOOM_IN);
    aViewData.GetBindings().Invalidate( SID_ZOOM_OUT);
}

bool ScTabView::ScrollCommand( const CommandEvent& rCEvt, ScSplitPos ePos )
{
    HideNoteMarker();

    bool bDone = false;
    const CommandWheelData* pData = rCEvt.GetWheelData();
    if (pData && pData->GetMode() == CommandWheelMode::ZOOM)
    {
        if ( !aViewData.GetViewShell()->GetViewFrame().GetFrame().IsInPlace() )
        {
            //  for ole inplace editing, the scale is defined by the visarea and client size
            //  and can't be changed directly

            const Fraction& rOldY = aViewData.GetZoomY();
            sal_uInt16 nOld = static_cast<tools::Long>( rOldY * 100 );
            sal_uInt16 nNew;
            if ( pData->GetDelta() < 0 )
                nNew = std::max( MINZOOM, basegfx::zoomtools::zoomOut( nOld ));
            else
                nNew = std::min( MAXZOOM, basegfx::zoomtools::zoomIn( nOld ));
            if ( nNew != nOld )
            {
                SetZoomPercentFromCommand(nNew);
            }

            bDone = true;
        }
    }
    else
    {
        ScHSplitPos eHPos = WhichH(ePos);
        ScVSplitPos eVPos = WhichV(ePos);
        ScrollAdaptor* pHScroll = ( eHPos == SC_SPLIT_LEFT ) ? aHScrollLeft.get() : aHScrollRight.get();
        ScrollAdaptor* pVScroll = ( eVPos == SC_SPLIT_TOP )  ? aVScrollTop.get()  : aVScrollBottom.get();
        if ( pGridWin[ePos] )
            bDone = pGridWin[ePos]->HandleScrollCommand( rCEvt, pHScroll, pVScroll );
    }
    return bDone;
}

bool ScTabView::GestureZoomCommand(const CommandEvent& rCEvt)
{
    HideNoteMarker();

    const CommandGestureZoomData* pData = rCEvt.GetGestureZoomData();
    if (!pData)
        return false;

    if (aViewData.GetViewShell()->GetViewFrame().GetFrame().IsInPlace())
        return false;

    if (pData->meEventType == GestureEventZoomType::Begin)
    {
        mfLastZoomScale = pData->mfScaleDelta;
        return true;
    }

    if (pData->meEventType == GestureEventZoomType::Update)
    {
        double deltaBetweenEvents = (pData->mfScaleDelta - mfLastZoomScale) / mfLastZoomScale;
        mfLastZoomScale = pData->mfScaleDelta;

        // Accumulate fractional zoom to avoid small zoom changes from being ignored
        mfAccumulatedZoom += deltaBetweenEvents;
        int nZoomChangePercent = mfAccumulatedZoom * 100;
        mfAccumulatedZoom -= nZoomChangePercent / 100.0;

        const Fraction& rOldY = aViewData.GetZoomY();
        sal_uInt16 nOld = static_cast<tools::Long>(rOldY * 100);
        sal_uInt16 nNew = nOld + nZoomChangePercent;
        nNew = std::clamp<sal_uInt16>(nNew, MINZOOM, MAXZOOM);

        if (nNew != nOld)
        {
            SetZoomPercentFromCommand(nNew);
        }

        return true;
    }
    return true;
}

IMPL_LINK_NOARG(ScTabView, HScrollLeftHdl, weld::Scrollbar&, void)
{
    ScrollHdl(aHScrollLeft.get());
}

IMPL_LINK_NOARG(ScTabView, HScrollRightHdl, weld::Scrollbar&, void)
{
    ScrollHdl(aHScrollRight.get());
}

IMPL_LINK_NOARG(ScTabView, VScrollTopHdl, weld::Scrollbar&, void)
{
    ScrollHdl(aVScrollTop.get());
}

IMPL_LINK_NOARG(ScTabView, VScrollBottomHdl, weld::Scrollbar&, void)
{
    ScrollHdl(aVScrollBottom.get());
}

IMPL_LINK_NOARG(ScTabView, EndScrollHdl, const MouseEvent&, bool)
{
    if (bDragging)
    {
        UpdateScrollBars();
        bDragging = false;
    }
    return false;
}

void ScTabView::ScrollHdl(ScrollAdaptor* pScroll)
{
    bool bHoriz = ( pScroll == aHScrollLeft.get() || pScroll == aHScrollRight.get() );
    tools::Long nViewPos;
    if ( bHoriz )
        nViewPos = aViewData.GetPosX( (pScroll == aHScrollLeft.get()) ?
                                        SC_SPLIT_LEFT : SC_SPLIT_RIGHT );
    else
        nViewPos = aViewData.GetPosY( (pScroll == aVScrollTop.get()) ?
                                        SC_SPLIT_TOP : SC_SPLIT_BOTTOM );

    bool bLayoutRTL = bHoriz && aViewData.GetDocument().IsLayoutRTL( aViewData.GetTabNo() );

    ScrollType eType = pScroll->GetScrollType();
    if ( eType == ScrollType::Drag )
    {
        if (!bDragging)
        {
            bDragging = true;
            nPrevDragPos = nViewPos;
        }

        // show scroll position
        // (only QuickHelp, there is no entry for it in the status bar)

        if (Help::IsQuickHelpEnabled())
        {
            Size aSize = pScroll->GetSizePixel();

            /*  Convert scrollbar mouse position to screen position. If RTL
                mode of scrollbar differs from RTL mode of its parent, then the
                direct call to Window::OutputToNormalizedScreenPixel() will
                give unusable results, because calculation of screen position
                is based on parent orientation and expects equal orientation of
                the child position. Need to mirror mouse position before. */
            Point aMousePos = pScroll->GetPointerPosPixel();
            if( pScroll->IsRTLEnabled() != pScroll->GetParent()->IsRTLEnabled() )
                aMousePos.setX( aSize.Width() - aMousePos.X() - 1 );
            aMousePos = pScroll->OutputToNormalizedScreenPixel( aMousePos );

            // convert top-left position of scrollbar to screen position
            Point aPos = pScroll->OutputToNormalizedScreenPixel( Point() );

            // get scrollbar scroll position for help text (row number/column name)
            tools::Long nScrollMin = 0;        // simulate RangeMin
            if ( aViewData.GetHSplitMode()==SC_SPLIT_FIX && pScroll == aHScrollRight.get() )
                nScrollMin = aViewData.GetFixPosX();
            if ( aViewData.GetVSplitMode()==SC_SPLIT_FIX && pScroll == aVScrollBottom.get() )
                nScrollMin = aViewData.GetFixPosY();
            tools::Long nScrollPos = GetScrollBarPos( *pScroll, bLayoutRTL ) + nScrollMin;

            OUString aHelpStr;
            tools::Rectangle aRect;
            QuickHelpFlags nAlign;
            if (bHoriz)
            {
                aHelpStr = ScResId(STR_COLUMN) +
                           " " + ScColToAlpha(static_cast<SCCOL>(nScrollPos));

                aRect.SetLeft( aMousePos.X() );
                aRect.SetTop( aPos.Y() - 4 );
                nAlign       = QuickHelpFlags::Bottom|QuickHelpFlags::Center;
            }
            else
            {
                aHelpStr = ScResId(STR_ROW) +
                           " " + OUString::number(nScrollPos + 1);

                // note that bLayoutRTL is always false here, because bLayoutRTL depends on bHoriz

                // show quicktext always inside sheet area
                aRect.SetLeft(aPos.X() - 8);
                aRect.SetTop( aMousePos.Y() );
                nAlign = QuickHelpFlags::Right | QuickHelpFlags::VCenter;
            }
            aRect.SetRight( aRect.Left() );
            aRect.SetBottom( aRect.Top() );

            Help::ShowQuickHelp(pScroll->GetParent(), aRect, aHelpStr, nAlign);
        }
    }
    else
        bDragging = false;

    if ( bHoriz && bLayoutRTL )
    {
        // change scroll type so visible/previous cells calculation below remains the same
        switch ( eType )
        {
            case ScrollType::LineUp:   eType = ScrollType::LineDown; break;
            case ScrollType::LineDown: eType = ScrollType::LineUp;   break;
            case ScrollType::PageUp:   eType = ScrollType::PageDown; break;
            case ScrollType::PageDown: eType = ScrollType::PageUp;   break;
            default:
            {
                // added to avoid warnings
            }
        }
    }

    tools::Long nDelta(0);
    switch ( eType )
    {
        case ScrollType::LineUp:
            nDelta = -1;
            break;
        case ScrollType::LineDown:
            nDelta = 1;
            break;
        case ScrollType::PageUp:
            if ( pScroll == aHScrollLeft.get() ) nDelta = -static_cast<tools::Long>(aViewData.PrevCellsX( SC_SPLIT_LEFT ));
            if ( pScroll == aHScrollRight.get() ) nDelta = -static_cast<tools::Long>(aViewData.PrevCellsX( SC_SPLIT_RIGHT ));
            if ( pScroll == aVScrollTop.get() ) nDelta = -static_cast<tools::Long>(aViewData.PrevCellsY( SC_SPLIT_TOP ));
            if ( pScroll == aVScrollBottom.get() ) nDelta = -static_cast<tools::Long>(aViewData.PrevCellsY( SC_SPLIT_BOTTOM ));
            if (nDelta==0) nDelta=-1;
            break;
        case ScrollType::PageDown:
            if ( pScroll == aHScrollLeft.get() ) nDelta = aViewData.VisibleCellsX( SC_SPLIT_LEFT );
            if ( pScroll == aHScrollRight.get() ) nDelta = aViewData.VisibleCellsX( SC_SPLIT_RIGHT );
            if ( pScroll == aVScrollTop.get() ) nDelta = aViewData.VisibleCellsY( SC_SPLIT_TOP );
            if ( pScroll == aVScrollBottom.get() ) nDelta = aViewData.VisibleCellsY( SC_SPLIT_BOTTOM );
            if (nDelta==0) nDelta=1;
            break;
        default:
            {
                // only scroll in the correct direction, do not jitter around hidden ranges
                tools::Long nScrollMin = 0;        // simulate RangeMin
                if ( aViewData.GetHSplitMode()==SC_SPLIT_FIX && pScroll == aHScrollRight.get() )
                    nScrollMin = aViewData.GetFixPosX();
                if ( aViewData.GetVSplitMode()==SC_SPLIT_FIX && pScroll == aVScrollBottom.get() )
                    nScrollMin = aViewData.GetFixPosY();

                tools::Long nScrollPos = GetScrollBarPos( *pScroll, bLayoutRTL ) + nScrollMin;
                nDelta = nScrollPos - nViewPos;

                // tdf#152406 Disable anti-jitter code for scroll wheel events
                // After moving thousands of columns to the right via
                // horizontal scroll wheel or trackpad swipe events, most
                // vertical scroll wheel or trackpad swipe events will trigger
                // the anti-jitter code because nScrollPos and nPrevDragPos
                // will be equal and nDelta will be overridden and set to zero.
                // So, only use the anti-jitter code for mouse drag events.
                if ( eType == ScrollType::Drag )
                {
                    if ( nScrollPos > nPrevDragPos )
                    {
                        if (nDelta<0) nDelta=0;
                    }
                    else if ( nScrollPos < nPrevDragPos )
                    {
                        if (nDelta>0) nDelta=0;
                    }
                    else
                        nDelta = 0;
                }
                else if ( bHoriz )
                {
                    // tdf#135478 Reduce sensitivity of horizontal scrollwheel
                    // Problem: at least on macOS, swipe events are very
                    // precise. So, when swiping at a slight angle off of
                    // vertical, swipe events will include a small amount
                    // of horizontal movement. Since horizontal swipe units
                    // are measured in cell widths, these small amounts of
                    // horizontal movement results in shifting many columns
                    // to the right or left while swiping almost vertically.
                    // So my hacky fix is to reduce the amount of horizontal
                    // swipe events to roughly match the "visual distance"
                    // of vertical swipe events.
                    // The reduction factor is arbitrary but is set to
                    // roughly the ratio of default cell width divided by
                    // default cell height. This hacky fix isn't a perfect
                    // fix, but hopefully it reduces the amount of
                    // unexpected horizontal shifting while swiping
                    // vertically to a tolerable amount for most users.
                    // Note: the potential downside of doing this is that
                    // some users might find horizontal swiping to be
                    // slower than they are used to. If that becomes an
                    // issue for enough users, the reduction factor may
                    // need to be lowered to find a good balance point.
                    static const sal_uInt16 nHScrollReductionFactor = 8;
                    if ( pScroll == aHScrollLeft.get() )
                    {
                        mnPendingaHScrollLeftDelta += nDelta;
                        nDelta = 0;
                        if ( abs(mnPendingaHScrollLeftDelta) > nHScrollReductionFactor )
                        {
                            nDelta = mnPendingaHScrollLeftDelta / nHScrollReductionFactor;
                            mnPendingaHScrollLeftDelta = mnPendingaHScrollLeftDelta % nHScrollReductionFactor;
                        }
                    }
                    else if ( pScroll == aHScrollRight.get() )
                    {
                        mnPendingaHScrollRightDelta += nDelta;
                        nDelta = 0;
                        if ( abs(mnPendingaHScrollRightDelta) > nHScrollReductionFactor )
                        {
                            nDelta = mnPendingaHScrollRightDelta / nHScrollReductionFactor;
                            mnPendingaHScrollRightDelta = mnPendingaHScrollRightDelta % nHScrollReductionFactor;
                        }
                    }
                }

                nPrevDragPos = nScrollPos;
            }
            break;
    }

    if (nDelta)
    {
        bool bUpdate = ( eType != ScrollType::Drag );    // don't alter the ranges while dragging
        if ( bHoriz )
            ScrollX( nDelta, (pScroll == aHScrollLeft.get()) ? SC_SPLIT_LEFT : SC_SPLIT_RIGHT, bUpdate );
        else
            ScrollY( nDelta, (pScroll == aVScrollTop.get()) ? SC_SPLIT_TOP : SC_SPLIT_BOTTOM, bUpdate );
    }
}

void ScTabView::ScrollX( tools::Long nDeltaX, ScHSplitPos eWhich, bool bUpdBars )
{
    ScDocument& rDoc = aViewData.GetDocument();
    SCCOL nOldX = aViewData.GetPosX(eWhich);
    SCCOL nNewX = nOldX + static_cast<SCCOL>(nDeltaX);
    if ( nNewX < 0 )
    {
        nDeltaX -= nNewX;
        nNewX = 0;
    }
    if ( nNewX > rDoc.MaxCol() )
    {
        nDeltaX -= nNewX - rDoc.MaxCol();
        nNewX = rDoc.MaxCol();
    }

    SCCOL nDir = ( nDeltaX > 0 ) ? 1 : -1;
    SCTAB nTab = aViewData.GetTabNo();
    while ( rDoc.ColHidden(nNewX, nTab) &&
            nNewX+nDir >= 0 && nNewX+nDir <= rDoc.MaxCol() )
        nNewX = sal::static_int_cast<SCCOL>( nNewX + nDir );

    // freeze

    if (aViewData.GetHSplitMode() == SC_SPLIT_FIX)
    {
        if (eWhich == SC_SPLIT_LEFT)
            nNewX = nOldX;          // always keep the left part
        else
        {
            SCCOL nFixX = aViewData.GetFixPosX();
            if (nNewX < nFixX)
                nNewX = nFixX;
        }
    }
    if (nNewX == nOldX)
        return;

    HideAllCursors();

    if ( nNewX >= 0 && nNewX <= rDoc.MaxCol() && nDeltaX )
    {
        SCCOL nTrackX = std::max( nOldX, nNewX );

        // with VCL Update() affects all windows at the moment, that is why
        // calling Update after scrolling of the GridWindow would possibly
        // already have painted the column/row bar with updated position.  -
        // Therefore call Update once before on column/row bar
        if (pColBar[eWhich])
            pColBar[eWhich]->PaintImmediately();

        tools::Long nOldPos = aViewData.GetScrPos( nTrackX, 0, eWhich ).X();
        aViewData.SetPosX( eWhich, nNewX );
        tools::Long nDiff = aViewData.GetScrPos( nTrackX, 0, eWhich ).X() - nOldPos;

        if ( eWhich==SC_SPLIT_LEFT )
        {
            pGridWin[SC_SPLIT_BOTTOMLEFT]->ScrollPixel( nDiff, 0 );
            if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
                pGridWin[SC_SPLIT_TOPLEFT]->ScrollPixel( nDiff, 0 );
        }
        else
        {
            pGridWin[SC_SPLIT_BOTTOMRIGHT]->ScrollPixel( nDiff, 0 );
            if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
                pGridWin[SC_SPLIT_TOPRIGHT]->ScrollPixel( nDiff, 0 );
        }
        if (pColBar[eWhich])     { pColBar[eWhich]->Scroll( nDiff,0 ); pColBar[eWhich]->PaintImmediately(); }
        if (pColOutline[eWhich]) pColOutline[eWhich]->ScrollPixel( nDiff );
        if (bUpdBars)
            UpdateScrollBars();
    }

    if (nDeltaX==1 || nDeltaX==-1)
        pGridWin[aViewData.GetActivePart()]->PaintImmediately();

    ShowAllCursors();

    SetNewVisArea();            // MapMode must already be set

    TestHintWindow();
}

void ScTabView::ScrollY( tools::Long nDeltaY, ScVSplitPos eWhich, bool bUpdBars )
{
    ScDocument& rDoc = aViewData.GetDocument();
    SCROW nOldY = aViewData.GetPosY(eWhich);
    SCROW nNewY = nOldY + static_cast<SCROW>(nDeltaY);
    if ( nNewY < 0 )
    {
        nDeltaY -= nNewY;
        nNewY = 0;
    }
    if ( nNewY > rDoc.MaxRow() )
    {
        nDeltaY -= nNewY - rDoc.MaxRow();
        nNewY = rDoc.MaxRow();
    }

    SCROW nDir = ( nDeltaY > 0 ) ? 1 : -1;
    SCTAB nTab = aViewData.GetTabNo();
    while ( rDoc.RowHidden(nNewY, nTab) &&
            nNewY+nDir >= 0 && nNewY+nDir <= rDoc.MaxRow() )
        nNewY += nDir;

    // freeze

    if (aViewData.GetVSplitMode() == SC_SPLIT_FIX)
    {
        if (eWhich == SC_SPLIT_TOP)
            nNewY = nOldY;                 // always keep the upper part
        else
        {
            SCROW nFixY = aViewData.GetFixPosY();
            if (nNewY < nFixY)
                nNewY = nFixY;
        }
    }
    if (nNewY == nOldY)
        return;

    HideAllCursors();

    if ( nNewY >= 0 && nNewY <= rDoc.MaxRow() && nDeltaY )
    {
        SCROW nTrackY = std::max( nOldY, nNewY );

        // adjust row headers before the actual scrolling, so it does not get painted twice
        // PosY may then also not be set yet, pass on new value
        SCROW nUNew = nNewY;
        UpdateHeaderWidth( &eWhich, &nUNew );               // adjust row headers

        if (pRowBar[eWhich])
            pRowBar[eWhich]->PaintImmediately();

        tools::Long nOldPos = aViewData.GetScrPos( 0, nTrackY, eWhich ).Y();
        aViewData.SetPosY( eWhich, nNewY );
        tools::Long nDiff = aViewData.GetScrPos( 0, nTrackY, eWhich ).Y() - nOldPos;

        if ( eWhich==SC_SPLIT_TOP )
        {
            pGridWin[SC_SPLIT_TOPLEFT]->ScrollPixel( 0, nDiff );
            if ( aViewData.GetHSplitMode() != SC_SPLIT_NONE )
                pGridWin[SC_SPLIT_TOPRIGHT]->ScrollPixel( 0, nDiff );
        }
        else
        {
            pGridWin[SC_SPLIT_BOTTOMLEFT]->ScrollPixel( 0, nDiff );
            if ( aViewData.GetHSplitMode() != SC_SPLIT_NONE )
                pGridWin[SC_SPLIT_BOTTOMRIGHT]->ScrollPixel( 0, nDiff );
        }
        if (pRowBar[eWhich])     { pRowBar[eWhich]->Scroll( 0,nDiff ); pRowBar[eWhich]->PaintImmediately(); }
        if (pRowOutline[eWhich]) pRowOutline[eWhich]->ScrollPixel( nDiff );
        if (bUpdBars)
            UpdateScrollBars();
    }

    if (nDeltaY==1 || nDeltaY==-1)
        pGridWin[aViewData.GetActivePart()]->PaintImmediately();

    ShowAllCursors();

    SetNewVisArea();            // MapMode must already be set

    TestHintWindow();
}

void ScTabView::ScrollLines( tools::Long nDeltaX, tools::Long nDeltaY )
{
    ScSplitPos eWhich = aViewData.GetActivePart();
    if (nDeltaX)
        ScrollX(nDeltaX,WhichH(eWhich));
    if (nDeltaY)
        ScrollY(nDeltaY,WhichV(eWhich));
}

namespace
{

SCROW lcl_LastVisible( const ScViewData& rViewData )
{
    // If many rows are hidden at end of the document,
    // then there should not be a switch to wide row headers because of this
    ScDocument& rDoc = rViewData.GetDocument();
    SCTAB nTab = rViewData.GetTabNo();

    SCROW nVis = rDoc.MaxRow();
    SCROW startRow;
    while ( nVis > 0 && rDoc.GetRowHeight( nVis, nTab, &startRow, nullptr ) == 0 )
        nVis = std::max<SCROW>( startRow - 1, 0 );
    return nVis;
}

} // anonymous namespace

void ScTabView::UpdateHeaderWidth( const ScVSplitPos* pWhich, const SCROW* pPosY )
{
    if (!pRowBar[SC_SPLIT_BOTTOM])
        return;

    ScDocument& rDoc = aViewData.GetDocument();
    SCROW nEndPos = rDoc.MaxRow();
    if ( !aViewData.GetViewShell()->GetViewFrame().GetFrame().IsInPlace() )
    {
        //  for OLE Inplace always MAXROW

        if ( pWhich && *pWhich == SC_SPLIT_BOTTOM && pPosY )
            nEndPos = *pPosY;
        else
            nEndPos = aViewData.GetPosY( SC_SPLIT_BOTTOM );
        nEndPos += aViewData.CellsAtY( nEndPos, 1, SC_SPLIT_BOTTOM ); // VisibleCellsY
        if (nEndPos > rDoc.MaxRow())
            nEndPos = lcl_LastVisible( aViewData );

        if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
        {
            SCROW nTopEnd;
            if ( pWhich && *pWhich == SC_SPLIT_TOP && pPosY )
                nTopEnd = *pPosY;
            else
                nTopEnd = aViewData.GetPosY( SC_SPLIT_TOP );
            nTopEnd += aViewData.CellsAtY( nTopEnd, 1, SC_SPLIT_TOP );// VisibleCellsY
            if (nTopEnd > rDoc.MaxRow())
                nTopEnd = lcl_LastVisible( aViewData );

            if ( nTopEnd > nEndPos )
                nEndPos = nTopEnd;
        }
    }

    tools::Long nSmall = pRowBar[SC_SPLIT_BOTTOM]->GetSmallWidth();
    tools::Long nBig   = pRowBar[SC_SPLIT_BOTTOM]->GetBigWidth();
    tools::Long nDiff  = nBig - nSmall;

    if (nEndPos>10000)
        nEndPos = 10000;
    else if (nEndPos<1)     // avoid extra step at 0 (when only one row is visible)
        nEndPos = 1;
    tools::Long nWidth = nBig - ( 10000 - nEndPos ) * nDiff / 10000;

    if (nWidth == pRowBar[SC_SPLIT_BOTTOM]->GetWidth() || bInUpdateHeader)
        return;

    bInUpdateHeader = true;

    pRowBar[SC_SPLIT_BOTTOM]->SetWidth( nWidth );
    if (pRowBar[SC_SPLIT_TOP])
        pRowBar[SC_SPLIT_TOP]->SetWidth( nWidth );

    RepeatResize();

    // on VCL there are endless updates (each Update is valid for all windows)
    //aCornerButton->Update();       // otherwise this never gets an Update

    bInUpdateHeader = false;
}

static void ShowHide( vcl::Window* pWin, bool bShow )
{
    OSL_ENSURE(pWin || !bShow, "window is not present");
    if (pWin)
        pWin->Show(bShow);
}

void ScTabView::UpdateShow()
{
    bool bHScrollMode = aViewData.IsHScrollMode();
    bool bVScrollMode = aViewData.IsVScrollMode();
    bool bTabMode     = aViewData.IsTabMode();
    bool bOutlMode    = aViewData.IsOutlineMode();
    bool bHOutline    = bOutlMode && lcl_HasColOutline(aViewData);
    bool bVOutline    = bOutlMode && lcl_HasRowOutline(aViewData);
    bool bHeader      = aViewData.IsHeaderMode();

    bool bShowH = ( aViewData.GetHSplitMode() != SC_SPLIT_NONE );
    bool bShowV = ( aViewData.GetVSplitMode() != SC_SPLIT_NONE );

    if ( aViewData.GetDocShell()->IsPreview() )
        bHScrollMode = bVScrollMode = bTabMode = bHeader = bHOutline = bVOutline = false;

        // create Windows

    if (bShowH && !pGridWin[SC_SPLIT_BOTTOMRIGHT])
    {
        pGridWin[SC_SPLIT_BOTTOMRIGHT] = VclPtr<ScGridWindow>::Create( pFrameWin, aViewData, SC_SPLIT_BOTTOMRIGHT );
        DoAddWin( pGridWin[SC_SPLIT_BOTTOMRIGHT] );
    }
    if (bShowV && !pGridWin[SC_SPLIT_TOPLEFT])
    {
        pGridWin[SC_SPLIT_TOPLEFT] = VclPtr<ScGridWindow>::Create( pFrameWin, aViewData, SC_SPLIT_TOPLEFT );
        DoAddWin( pGridWin[SC_SPLIT_TOPLEFT] );
    }
    if (bShowH && bShowV && !pGridWin[SC_SPLIT_TOPRIGHT])
    {
        pGridWin[SC_SPLIT_TOPRIGHT] = VclPtr<ScGridWindow>::Create( pFrameWin, aViewData, SC_SPLIT_TOPRIGHT );
        DoAddWin( pGridWin[SC_SPLIT_TOPRIGHT] );
    }

    if (bHOutline && !pColOutline[SC_SPLIT_LEFT])
        pColOutline[SC_SPLIT_LEFT] = VclPtr<ScOutlineWindow>::Create( pFrameWin, SC_OUTLINE_HOR, &aViewData, SC_SPLIT_BOTTOMLEFT );
    if (bShowH && bHOutline && !pColOutline[SC_SPLIT_RIGHT])
        pColOutline[SC_SPLIT_RIGHT] = VclPtr<ScOutlineWindow>::Create( pFrameWin, SC_OUTLINE_HOR, &aViewData, SC_SPLIT_BOTTOMRIGHT );

    if (bVOutline && !pRowOutline[SC_SPLIT_BOTTOM])
        pRowOutline[SC_SPLIT_BOTTOM] = VclPtr<ScOutlineWindow>::Create( pFrameWin, SC_OUTLINE_VER, &aViewData, SC_SPLIT_BOTTOMLEFT );
    if (bShowV && bVOutline && !pRowOutline[SC_SPLIT_TOP])
        pRowOutline[SC_SPLIT_TOP] = VclPtr<ScOutlineWindow>::Create( pFrameWin, SC_OUTLINE_VER, &aViewData, SC_SPLIT_TOPLEFT );

    if (bShowH && bHeader && !pColBar[SC_SPLIT_RIGHT])
        pColBar[SC_SPLIT_RIGHT] = VclPtr<ScColBar>::Create( pFrameWin, SC_SPLIT_RIGHT,
                                                            &aHdrFunc, pHdrSelEng.get(), this );
    if (bShowV && bHeader && !pRowBar[SC_SPLIT_TOP])
        pRowBar[SC_SPLIT_TOP] = VclPtr<ScRowBar>::Create( pFrameWin, SC_SPLIT_TOP,
                                                          &aHdrFunc, pHdrSelEng.get(), this );

        // show Windows

    ShowHide( aHScrollLeft.get(), bHScrollMode );
    ShowHide( aHScrollRight.get(), bShowH && bHScrollMode );
    ShowHide( aVScrollBottom.get(), bVScrollMode );
    ShowHide( aVScrollTop.get(), bShowV && bVScrollMode );

    ShowHide( pHSplitter, bHScrollMode || bShowH );         // always generated
    ShowHide( pVSplitter, bVScrollMode || bShowV );
    ShowHide( pTabControl, bTabMode );

                                                    // from here dynamically generated

    ShowHide( pGridWin[SC_SPLIT_BOTTOMRIGHT], bShowH );
    ShowHide( pGridWin[SC_SPLIT_TOPLEFT], bShowV );
    ShowHide( pGridWin[SC_SPLIT_TOPRIGHT], bShowH && bShowV );

    ShowHide( pColOutline[SC_SPLIT_LEFT], bHOutline );
    ShowHide( pColOutline[SC_SPLIT_RIGHT], bShowH && bHOutline );

    ShowHide( pRowOutline[SC_SPLIT_BOTTOM], bVOutline );
    ShowHide( pRowOutline[SC_SPLIT_TOP], bShowV && bVOutline );

    ShowHide( pColBar[SC_SPLIT_RIGHT], bShowH && bHeader );
    ShowHide( pRowBar[SC_SPLIT_TOP], bShowV && bHeader );

    //! register new Gridwindows
}

bool ScTabView::UpdateVisibleRange()
{
    bool bChanged = false;
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
    {
        if (!pWin || !pWin->IsVisible())
            continue;

        if (pWin->UpdateVisibleRange())
            bChanged = true;
    }

    return bChanged;
}

// ---  Splitter  --------------------------------------------------------

IMPL_LINK( ScTabView, SplitHdl, Splitter*, pSplitter, void )
{
    if ( pSplitter == pHSplitter )
        DoHSplit( pHSplitter->GetSplitPosPixel() );
    else
        DoVSplit( pVSplitter->GetSplitPosPixel() );

    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX || aViewData.GetVSplitMode() == SC_SPLIT_FIX )
        FreezeSplitters( true );

    DoResize( aBorderPos, aFrameSize );
}

void ScTabView::DoHSplit(tools::Long nSplitPos)
{
    //  nSplitPos is the real pixel position on the frame window,
    //  mirroring for RTL has to be done here.

    bool bLayoutRTL = aViewData.GetDocument().IsLayoutRTL( aViewData.GetTabNo() );
    if ( bLayoutRTL )
        nSplitPos = pFrameWin->GetOutputSizePixel().Width() - nSplitPos - 1;

    tools::Long nMinPos;
    tools::Long nMaxPos;

    nMinPos = SPLIT_MARGIN;
    if ( pRowBar[SC_SPLIT_BOTTOM] && pRowBar[SC_SPLIT_BOTTOM]->GetSizePixel().Width() >= nMinPos )
        nMinPos = pRowBar[SC_SPLIT_BOTTOM]->GetSizePixel().Width() + 1;
    nMaxPos = aFrameSize.Width() - SPLIT_MARGIN;

    ScSplitMode aOldMode = aViewData.GetHSplitMode();
    ScSplitMode aNewMode = SC_SPLIT_NORMAL;

    aViewData.SetHSplitPos( nSplitPos );
    if ( nSplitPos < nMinPos || nSplitPos > nMaxPos )
        aNewMode = SC_SPLIT_NONE;

    aViewData.SetHSplitMode( aNewMode );

    if ( aNewMode == aOldMode )
        return;

    UpdateShow();       // before ActivatePart !!

    if ( aNewMode == SC_SPLIT_NONE )
    {
        if (aViewData.GetActivePart() == SC_SPLIT_TOPRIGHT)
            ActivatePart( SC_SPLIT_TOPLEFT );
        if (aViewData.GetActivePart() == SC_SPLIT_BOTTOMRIGHT)
            ActivatePart( SC_SPLIT_BOTTOMLEFT );
    }
    else
    {
        SCCOL nOldDelta = aViewData.GetPosX( SC_SPLIT_LEFT );
        tools::Long nLeftWidth = nSplitPos - pRowBar[SC_SPLIT_BOTTOM]->GetSizePixel().Width();
        if ( nLeftWidth < 0 ) nLeftWidth = 0;
        SCCOL nNewDelta = nOldDelta + aViewData.CellsAtX( nOldDelta, 1, SC_SPLIT_LEFT,
                        static_cast<sal_uInt16>(nLeftWidth) );
        ScDocument& rDoc = aViewData.GetDocument();
        if ( nNewDelta > rDoc.MaxCol() )
            nNewDelta = rDoc.MaxCol();
        aViewData.SetPosX( SC_SPLIT_RIGHT, nNewDelta );
        if ( nNewDelta > aViewData.GetCurX() )
            ActivatePart( (WhichV(aViewData.GetActivePart()) == SC_SPLIT_BOTTOM) ?
                SC_SPLIT_BOTTOMLEFT : SC_SPLIT_TOPLEFT );
        else
            ActivatePart( (WhichV(aViewData.GetActivePart()) == SC_SPLIT_BOTTOM) ?
                SC_SPLIT_BOTTOMRIGHT : SC_SPLIT_TOPRIGHT );
    }

    // Form Layer needs to know the visible part of all windows
    // that is why MapMode must already be correct here
    SyncGridWindowMapModeFromDrawMapMode();
    SetNewVisArea();

    PaintGrid();
    PaintTop();

    InvalidateSplit();
}

void ScTabView::DoVSplit(tools::Long nSplitPos)
{
    tools::Long nMinPos;
    tools::Long nMaxPos;
    SCROW nOldDelta;

    nMinPos = SPLIT_MARGIN;
    if ( pColBar[SC_SPLIT_LEFT] && pColBar[SC_SPLIT_LEFT]->GetSizePixel().Height() >= nMinPos )
        nMinPos = pColBar[SC_SPLIT_LEFT]->GetSizePixel().Height() + 1;
    nMaxPos = aFrameSize.Height() - SPLIT_MARGIN;

    ScSplitMode aOldMode = aViewData.GetVSplitMode();
    ScSplitMode aNewMode = SC_SPLIT_NORMAL;

    aViewData.SetVSplitPos( nSplitPos );
    if ( nSplitPos < nMinPos || nSplitPos > nMaxPos )
        aNewMode = SC_SPLIT_NONE;

    aViewData.SetVSplitMode( aNewMode );

    if ( aNewMode == aOldMode )
        return;

    UpdateShow();       // before ActivatePart !!

    if ( aNewMode == SC_SPLIT_NONE )
    {
        nOldDelta = aViewData.GetPosY( SC_SPLIT_TOP );
        aViewData.SetPosY( SC_SPLIT_BOTTOM, nOldDelta );

        if (aViewData.GetActivePart() == SC_SPLIT_TOPLEFT)
            ActivatePart( SC_SPLIT_BOTTOMLEFT );
        if (aViewData.GetActivePart() == SC_SPLIT_TOPRIGHT)
            ActivatePart( SC_SPLIT_BOTTOMRIGHT );
    }
    else
    {
        if ( aOldMode == SC_SPLIT_NONE )
            nOldDelta = aViewData.GetPosY( SC_SPLIT_BOTTOM );
        else
            nOldDelta = aViewData.GetPosY( SC_SPLIT_TOP );

        aViewData.SetPosY( SC_SPLIT_TOP, nOldDelta );
        tools::Long nTopHeight = nSplitPos - pColBar[SC_SPLIT_LEFT]->GetSizePixel().Height();
        if ( nTopHeight < 0 ) nTopHeight = 0;
        SCROW nNewDelta = nOldDelta + aViewData.CellsAtY( nOldDelta, 1, SC_SPLIT_TOP,
                        static_cast<sal_uInt16>(nTopHeight) );
        ScDocument& rDoc = aViewData.GetDocument();
        if ( nNewDelta > rDoc.MaxRow() )
            nNewDelta = rDoc.MaxRow();
        aViewData.SetPosY( SC_SPLIT_BOTTOM, nNewDelta );
        if ( nNewDelta > aViewData.GetCurY() )
            ActivatePart( (WhichH(aViewData.GetActivePart()) == SC_SPLIT_LEFT) ?
                SC_SPLIT_TOPLEFT : SC_SPLIT_TOPRIGHT );
        else
            ActivatePart( (WhichH(aViewData.GetActivePart()) == SC_SPLIT_LEFT) ?
                SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT );
    }

    // Form Layer needs to know the visible part of all windows
    // that is why MapMode must already be correct here
    SyncGridWindowMapModeFromDrawMapMode();
    SetNewVisArea();

    PaintGrid();
    PaintLeft();

    InvalidateSplit();
}

Point ScTabView::GetInsertPos() const
{
    ScDocument& rDoc = aViewData.GetDocument();
    SCCOL nCol = aViewData.GetCurX();
    SCROW nRow = aViewData.GetCurY();
    SCTAB nTab = aViewData.GetTabNo();
    tools::Long nPosX = 0;
    for (SCCOL i=0; i<nCol; i++)
        nPosX += rDoc.GetColWidth(i,nTab);
    nPosX = o3tl::convert(nPosX, o3tl::Length::twip, o3tl::Length::mm100);
    if ( rDoc.IsNegativePage( nTab ) )
        nPosX = -nPosX;
    tools::Long nPosY = rDoc.GetRowHeight( 0, nRow-1, nTab);
    nPosY = o3tl::convert(nPosY, o3tl::Length::twip, o3tl::Length::mm100);
    return Point(nPosX,nPosY);
}

Point ScTabView::GetChartInsertPos( const Size& rSize, const ScRange& rCellRange )
{
    Point aInsertPos;
    const tools::Long nBorder = 100;   // leave 1mm for border
    tools::Long nNeededWidth = rSize.Width() + 2 * nBorder;
    tools::Long nNeededHeight = rSize.Height() + 2 * nBorder;

    // use the active window, or lower/right if frozen (as in CalcZoom)
    ScSplitPos eUsedPart = aViewData.GetActivePart();
    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
        eUsedPart = (WhichV(eUsedPart)==SC_SPLIT_TOP) ? SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT;
    if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
        eUsedPart = (WhichH(eUsedPart)==SC_SPLIT_LEFT) ? SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT;

    ScGridWindow* pWin = pGridWin[eUsedPart].get();
    OSL_ENSURE( pWin, "Window not found" );
    if (pWin)
    {
        ActivatePart( eUsedPart );

        //  get the visible rectangle in logic units
        bool bLOKActive = comphelper::LibreOfficeKit::isActive();
        MapMode aDrawMode = pWin->GetDrawMapMode();
        tools::Rectangle aVisible(
            bLOKActive ?
            OutputDevice::LogicToLogic( aViewData.getLOKVisibleArea(), MapMode(MapUnit::MapTwip), MapMode(MapUnit::Map100thMM) )
            : pWin->PixelToLogic( tools::Rectangle( Point(0,0), pWin->GetOutputSizePixel() ), aDrawMode ) );

        ScDocument& rDoc = aViewData.GetDocument();
        SCTAB nTab = aViewData.GetTabNo();
        bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );
        tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;

        tools::Long nDocX = o3tl::convert(rDoc.GetColOffset(rDoc.MaxCol() + 1, nTab), o3tl::Length::twip, o3tl::Length::mm100) * nLayoutSign;
        tools::Long nDocY = o3tl::convert(rDoc.GetRowOffset( rDoc.MaxRow() + 1, nTab ), o3tl::Length::twip, o3tl::Length::mm100);

        if ( aVisible.Left() * nLayoutSign > nDocX * nLayoutSign )
            aVisible.SetLeft( nDocX );
        if ( aVisible.Right() * nLayoutSign > nDocX * nLayoutSign )
            aVisible.SetRight( nDocX );
        if ( aVisible.Top() > nDocY )
            aVisible.SetTop( nDocY );
        if ( aVisible.Bottom() > nDocY )
            aVisible.SetBottom( nDocY );

        //  get the logic position of the selection

        tools::Rectangle aSelection = rDoc.GetMMRect( rCellRange.aStart.Col(), rCellRange.aStart.Row(),
                                                rCellRange.aEnd.Col(), rCellRange.aEnd.Row(), nTab );

        if (bLOKActive && bLayoutRTL)
        {
            // In this case we operate in negative X coordinates. The rectangle aSelection already
            // has negative X coordinates. So the x coordinates in the rectangle aVisible(from getLOKVisibleArea)
            // need be negated to match.
            aVisible = tools::Rectangle(-aVisible.Right(), aVisible.Top(), -aVisible.Left(), aVisible.Bottom());
        }

        tools::Long nLeftSpace = aSelection.Left() - aVisible.Left();
        tools::Long nRightSpace = aVisible.Right() - aSelection.Right();
        tools::Long nTopSpace = aSelection.Top() - aVisible.Top();
        tools::Long nBottomSpace = aVisible.Bottom() - aSelection.Bottom();

        bool bFitLeft = ( nLeftSpace >= nNeededWidth );
        bool bFitRight = ( nRightSpace >= nNeededWidth );

        if ( bFitLeft || bFitRight )
        {
            // first preference: completely left or right of the selection

            // if both fit, prefer left in RTL mode, right otherwise
            bool bPutLeft = bFitLeft && ( bLayoutRTL || !bFitRight );

            if ( bPutLeft )
                aInsertPos.setX( aSelection.Left() - nNeededWidth );
            else
                aInsertPos.setX( aSelection.Right() + 1 );

            // align with top of selection (is moved again if it doesn't fit)
            aInsertPos.setY( std::max( aSelection.Top(), aVisible.Top() ) );
        }
        else if ( nTopSpace >= nNeededHeight || nBottomSpace >= nNeededHeight )
        {
            // second preference: completely above or below the selection
            if ( nBottomSpace > nNeededHeight )             // bottom is preferred
                aInsertPos.setY( aSelection.Bottom() + 1 );
            else
                aInsertPos.setY( aSelection.Top() - nNeededHeight );

            // align with (logic) left edge of selection (moved again if it doesn't fit)
            if ( bLayoutRTL )
                aInsertPos.setX( std::min( aSelection.Right(), aVisible.Right() ) - nNeededWidth + 1 );
            else
                aInsertPos.setX( std::max( aSelection.Left(), aVisible.Left() ) );
        }
        else
        {
            // place to the (logic) right of the selection and move so it fits

            if ( bLayoutRTL )
                aInsertPos.setX( aSelection.Left() - nNeededWidth );
            else
                aInsertPos.setX( aSelection.Right() + 1 );
            aInsertPos.setY( std::max( aSelection.Top(), aVisible.Top() ) );
        }

        // move the position if the object doesn't fit in the screen

        tools::Rectangle aCompareRect( aInsertPos, Size( nNeededWidth, nNeededHeight ) );
        if ( aCompareRect.Right() > aVisible.Right() )
            aInsertPos.AdjustX( -(aCompareRect.Right() - aVisible.Right()) );
        if ( aCompareRect.Bottom() > aVisible.Bottom() )
            aInsertPos.AdjustY( -(aCompareRect.Bottom() - aVisible.Bottom()) );

        if ( aInsertPos.X() < aVisible.Left() )
            aInsertPos.setX( aVisible.Left() );
        if ( aInsertPos.Y() < aVisible.Top() )
            aInsertPos.setY( aVisible.Top() );

        // nNeededWidth / nNeededHeight includes all borders - move aInsertPos to the
        // object position, inside the border

        aInsertPos.AdjustX(nBorder );
        aInsertPos.AdjustY(nBorder );
    }
    return aInsertPos;
}

Point ScTabView::GetChartDialogPos( const Size& rDialogSize, const tools::Rectangle& rLogicChart )
{
    // rDialogSize must be in pixels, rLogicChart in 1/100 mm. Return value is in pixels.

    Point aRet;

    // use the active window, or lower/right if frozen (as in CalcZoom)
    ScSplitPos eUsedPart = aViewData.GetActivePart();
    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
        eUsedPart = (WhichV(eUsedPart)==SC_SPLIT_TOP) ? SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT;
    if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
        eUsedPart = (WhichH(eUsedPart)==SC_SPLIT_LEFT) ? SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT;

    ScGridWindow* pWin = pGridWin[eUsedPart].get();
    OSL_ENSURE( pWin, "Window not found" );
    if (pWin)
    {
        MapMode aDrawMode = pWin->GetDrawMapMode();
        tools::Rectangle aObjPixel = pWin->LogicToPixel( rLogicChart, aDrawMode );
        AbsoluteScreenPixelRectangle aObjAbs( pWin->OutputToAbsoluteScreenPixel( aObjPixel.TopLeft() ),
                           pWin->OutputToAbsoluteScreenPixel( aObjPixel.BottomRight() ) );

        AbsoluteScreenPixelRectangle aDesktop = pWin->GetDesktopRectPixel();
        Size aSpace = pWin->LogicToPixel( Size(8, 12), MapMode(MapUnit::MapAppFont));

        ScDocument& rDoc = aViewData.GetDocument();
        SCTAB nTab = aViewData.GetTabNo();
        bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );

        bool bCenterHor = false;

        if ( aDesktop.Bottom() - aObjAbs.Bottom() >= rDialogSize.Height() + aSpace.Height() )
        {
            // first preference: below the chart

            aRet.setY( aObjAbs.Bottom() + aSpace.Height() );
            bCenterHor = true;
        }
        else if ( aObjAbs.Top() - aDesktop.Top() >= rDialogSize.Height() + aSpace.Height() )
        {
            // second preference: above the chart

            aRet.setY( aObjAbs.Top() - rDialogSize.Height() - aSpace.Height() );
            bCenterHor = true;
        }
        else
        {
            bool bFitLeft = ( aObjAbs.Left() - aDesktop.Left() >= rDialogSize.Width() + aSpace.Width() );
            bool bFitRight = ( aDesktop.Right() - aObjAbs.Right() >= rDialogSize.Width() + aSpace.Width() );

            if ( bFitLeft || bFitRight )
            {
                // if both fit, prefer right in RTL mode, left otherwise
                bool bPutRight = bFitRight && ( bLayoutRTL || !bFitLeft );
                if ( bPutRight )
                    aRet.setX( aObjAbs.Right() + aSpace.Width() );
                else
                    aRet.setX( aObjAbs.Left() - rDialogSize.Width() - aSpace.Width() );

                // center vertically
                aRet.setY( aObjAbs.Top() + ( aObjAbs.GetHeight() - rDialogSize.Height() ) / 2 );
            }
            else
            {
                // doesn't fit on any edge - put at the bottom of the screen
                aRet.setY( aDesktop.Bottom() - rDialogSize.Height() );
                bCenterHor = true;
            }
        }
        if ( bCenterHor )
            aRet.setX( aObjAbs.Left() + ( aObjAbs.GetWidth() - rDialogSize.Width() ) / 2 );

        // limit to screen (centering might lead to invalid positions)
        if ( aRet.X() + rDialogSize.Width() - 1 > aDesktop.Right() )
            aRet.setX( aDesktop.Right() - rDialogSize.Width() + 1 );
        if ( aRet.X() < aDesktop.Left() )
            aRet.setX( aDesktop.Left() );
        if ( aRet.Y() + rDialogSize.Height() - 1 > aDesktop.Bottom() )
            aRet.setY( aDesktop.Bottom() - rDialogSize.Height() + 1 );
        if ( aRet.Y() < aDesktop.Top() )
            aRet.setY( aDesktop.Top() );
    }

    return aRet;
}

void ScTabView::LockModifiers( sal_uInt16 nModifiers )
{
    pSelEngine->LockModifiers( nModifiers );
    pHdrSelEng->LockModifiers( nModifiers );
}

sal_uInt16 ScTabView::GetLockedModifiers() const
{
    return pSelEngine->GetLockedModifiers();
}

Point ScTabView::GetMousePosPixel()
{
    Point aPos;
    ScGridWindow* pWin = GetActiveWin();

    if ( pWin )
        aPos = pWin->GetMousePosPixel();

    return aPos;
}

void ScTabView::FreezeSplitters( bool bFreeze, SplitMethod eSplitMethod, SCCOLROW nFreezeIndex)
{
    if ((eSplitMethod == SC_SPLIT_METHOD_COL || eSplitMethod == SC_SPLIT_METHOD_ROW) && nFreezeIndex < 0)
        nFreezeIndex = 0;

    ScSplitMode eOldH = aViewData.GetHSplitMode();
    ScSplitMode eOldV = aViewData.GetVSplitMode();

    ScSplitPos ePos = SC_SPLIT_BOTTOMLEFT;
    if ( eOldV != SC_SPLIT_NONE )
        ePos = SC_SPLIT_TOPLEFT;
    vcl::Window* pWin = pGridWin[ePos];

    bool bLayoutRTL = aViewData.GetDocument().IsLayoutRTL( aViewData.GetTabNo() );
    bool bUpdateFix = false;

    if ( bFreeze )
    {
        Point aWinStart = pWin->GetPosPixel();
        aViewData.GetDocShell()->SetDocumentModified();

        Point aSplit;
        SCCOL nPosX = 1;
        SCROW nPosY = 1;
        if (eOldV != SC_SPLIT_NONE || eOldH != SC_SPLIT_NONE)
        {
            if ( eOldV != SC_SPLIT_NONE && (eSplitMethod == SC_SPLIT_METHOD_ROW || eSplitMethod == SC_SPLIT_METHOD_CURSOR))
                aSplit.setY( aViewData.GetVSplitPos() - aWinStart.Y() );

            if ( eOldH != SC_SPLIT_NONE && (eSplitMethod == SC_SPLIT_METHOD_COL || eSplitMethod == SC_SPLIT_METHOD_CURSOR))
            {
                tools::Long nSplitPos = aViewData.GetHSplitPos();
                if ( bLayoutRTL )
                    nSplitPos = pFrameWin->GetOutputSizePixel().Width() - nSplitPos - 1;
                aSplit.setX( nSplitPos - aWinStart.X() );
            }

            aViewData.GetPosFromPixel( aSplit.X(), aSplit.Y(), ePos, nPosX, nPosY );
            bool bLeft;
            bool bTop;
            aViewData.GetMouseQuadrant( aSplit, ePos, nPosX, nPosY, bLeft, bTop );
            if (eSplitMethod == SC_SPLIT_METHOD_COL)
                nPosX = static_cast<SCCOL>(nFreezeIndex);
            else if (!bLeft)
                ++nPosX;
            if (eSplitMethod == SC_SPLIT_METHOD_ROW)
                nPosY = static_cast<SCROW>(nFreezeIndex);
            else if (!bTop)
                ++nPosY;
        }
        else
        {
            switch(eSplitMethod)
            {
                case SC_SPLIT_METHOD_ROW:
                {
                    nPosX = 0;
                    nPosY = static_cast<SCROW>(nFreezeIndex);
                }
                break;
                case SC_SPLIT_METHOD_COL:
                {
                    nPosX = static_cast<SCCOL>(nFreezeIndex);
                    nPosY = 0;
                }
                break;
                case SC_SPLIT_METHOD_CURSOR:
                {
                    nPosX = aViewData.GetCurX();
                    nPosY = aViewData.GetCurY();
                }
                break;
            }
        }

        SCROW nTopPos = aViewData.GetPosY(SC_SPLIT_BOTTOM);
        SCROW nBottomPos = nPosY;
        SCCOL nLeftPos = aViewData.GetPosX(SC_SPLIT_LEFT);
        SCCOL nRightPos = nPosX;

        if (eSplitMethod == SC_SPLIT_METHOD_ROW || eSplitMethod == SC_SPLIT_METHOD_CURSOR)
        {
            if (eOldV != SC_SPLIT_NONE)
            {
                nTopPos = aViewData.GetPosY(SC_SPLIT_TOP);
                if (aViewData.GetPosY(SC_SPLIT_BOTTOM) > nBottomPos)
                    nBottomPos = aViewData.GetPosY(SC_SPLIT_BOTTOM);
            }
            aSplit = aViewData.GetScrPos(nPosX, nPosY, ePos, true);
            if (aSplit.Y() > 0)
            {
                aViewData.SetVSplitMode(SC_SPLIT_FIX);
                aViewData.SetVSplitPos(aSplit.Y() + aWinStart.Y());
                aViewData.SetFixPosY(nPosY);

                aViewData.SetPosY(SC_SPLIT_TOP, nTopPos);
                aViewData.SetPosY(SC_SPLIT_BOTTOM, nBottomPos);
            }
            else if (nPosY == 1 && eSplitMethod == SC_SPLIT_METHOD_ROW)
            {
                // Freeze first row, but row 1 is not visible on screen now == special handling
                aViewData.SetVSplitMode(SC_SPLIT_FIX);
                aViewData.SetFixPosY(nPosY);

                aViewData.SetPosY(SC_SPLIT_TOP, 0);
                bUpdateFix = true;
            }
            else
                aViewData.SetVSplitMode(SC_SPLIT_NONE);
        }

        if (eSplitMethod == SC_SPLIT_METHOD_COL || eSplitMethod == SC_SPLIT_METHOD_CURSOR)
        {
            if (eOldH != SC_SPLIT_NONE)
            {
                if (aViewData.GetPosX(SC_SPLIT_RIGHT) > nRightPos)
                    nRightPos = aViewData.GetPosX(SC_SPLIT_RIGHT);
            }
            aSplit = aViewData.GetScrPos( nPosX, nPosY, ePos, true );
            if (nPosX > aViewData.GetPosX(SC_SPLIT_LEFT))       // (aSplit.X() > 0) doesn't work for RTL
            {
                tools::Long nSplitPos = aSplit.X() + aWinStart.X();
                if ( bLayoutRTL )
                    nSplitPos = pFrameWin->GetOutputSizePixel().Width() - nSplitPos - 1;

                aViewData.SetHSplitMode( SC_SPLIT_FIX );
                aViewData.SetHSplitPos( nSplitPos );
                aViewData.SetFixPosX( nPosX );

                aViewData.SetPosX(SC_SPLIT_LEFT, nLeftPos);
                aViewData.SetPosX(SC_SPLIT_RIGHT, nRightPos);
            }
            else if (nPosX == 1 && eSplitMethod == SC_SPLIT_METHOD_COL)
            {
                // Freeze first column, but col A is not visible on screen now == special handling
                aViewData.SetHSplitMode(SC_SPLIT_FIX);
                aViewData.SetFixPosX(nPosX);

                aViewData.SetPosX(SC_SPLIT_RIGHT, aViewData.GetPosX(SC_SPLIT_LEFT));
                aViewData.SetPosX(SC_SPLIT_LEFT, 0);
                bUpdateFix = true;
            }
            else
                aViewData.SetHSplitMode( SC_SPLIT_NONE );
        }
    }
    else                        // unfreeze
    {
        if ( eOldH == SC_SPLIT_FIX )
            aViewData.SetHSplitMode( SC_SPLIT_NORMAL );
        if ( eOldV == SC_SPLIT_FIX )
            aViewData.SetVSplitMode( SC_SPLIT_NORMAL );
    }

    // Form Layer needs to know the visible part of all windows
    // that is why MapMode must already be correct here
    SyncGridWindowMapModeFromDrawMapMode();
    SetNewVisArea();

    RepeatResize(bUpdateFix);

    UpdateShow();
    PaintLeft();
    PaintTop();
    PaintGrid();

    //  SC_FOLLOW_NONE: only update active part
    AlignToCursor( aViewData.GetCurX(), aViewData.GetCurY(), SC_FOLLOW_NONE );
    UpdateAutoFillMark();

    InvalidateSplit();
}

void ScTabView::RemoveSplit()
{
    if (aViewData.GetHSplitMode() == SC_SPLIT_FIX || aViewData.GetVSplitMode() == SC_SPLIT_FIX)
        aViewData.GetDocShell()->SetDocumentModified();
    DoHSplit( 0 );
    DoVSplit( 0 );
    RepeatResize();
}

void ScTabView::SplitAtCursor()
{
    ScSplitPos ePos = SC_SPLIT_BOTTOMLEFT;
    if ( aViewData.GetVSplitMode() != SC_SPLIT_NONE )
        ePos = SC_SPLIT_TOPLEFT;
    vcl::Window* pWin = pGridWin[ePos];
    Point aWinStart = pWin->GetPosPixel();

    SCCOL nPosX = aViewData.GetCurX();
    SCROW nPosY = aViewData.GetCurY();
    Point aSplit = aViewData.GetScrPos( nPosX, nPosY, ePos, true );
    if ( nPosX > 0 )
        DoHSplit( aSplit.X() + aWinStart.X() );
    else
        DoHSplit( 0 );
    if ( nPosY > 0 )
        DoVSplit( aSplit.Y() + aWinStart.Y() );
    else
        DoVSplit( 0 );
    RepeatResize();
}

void ScTabView::SplitAtPixel( const Point& rPixel )
{
    // pixel is relative to the entire View, not to the first GridWin

    if ( rPixel.X() > 0 )
        DoHSplit( rPixel.X() );
    else
        DoHSplit( 0 );
    if ( rPixel.Y() > 0 )
        DoVSplit( rPixel.Y() );
    else
        DoVSplit( 0 );
    RepeatResize();
}

void ScTabView::InvalidateSplit()
{
    SfxBindings& rBindings = aViewData.GetBindings();
    rBindings.Invalidate( SID_WINDOW_SPLIT );
    rBindings.Invalidate( SID_WINDOW_FIX );
    rBindings.Invalidate( SID_WINDOW_FIX_COL );
    rBindings.Invalidate( SID_WINDOW_FIX_ROW );

    pHSplitter->SetFixed( aViewData.GetHSplitMode() == SC_SPLIT_FIX );
    pVSplitter->SetFixed( aViewData.GetVSplitMode() == SC_SPLIT_FIX );
}

void ScTabView::SetNewVisArea()
{
    //  Draw-MapMode must be set for Controls when VisAreaChanged
    //  (also when Edit-MapMode is set instead)
    MapMode aOldMode[4];
    MapMode aDrawMode[4];
    sal_uInt16 i;
    for (i=0; i<4; i++)
        if (pGridWin[i])
        {
            aOldMode[i] = pGridWin[i]->GetMapMode();
            aDrawMode[i] = pGridWin[i]->GetDrawMapMode();
            if (aDrawMode[i] != aOldMode[i])
                pGridWin[i]->SetMapMode(aDrawMode[i]);
        }

    vcl::Window* pActive = pGridWin[aViewData.GetActivePart()];
    if (pActive)
        aViewData.GetViewShell()->VisAreaChanged();
    if (pDrawView)
        pDrawView->VisAreaChanged(nullptr);    // no window passed on -> for all windows

    UpdateAllOverlays();                // #i79909# with drawing MapMode set

    for (i=0; i<4; i++)
        if (pGridWin[i] && aDrawMode[i] != aOldMode[i])
        {
            pGridWin[i]->flushOverlayManager();     // #i79909# flush overlays before switching to edit MapMode
            pGridWin[i]->SetMapMode(aOldMode[i]);
        }

    SfxViewFrame& rViewFrame = aViewData.GetViewShell()->GetViewFrame();
    SfxFrame& rFrame = rViewFrame.GetFrame();
    css::uno::Reference<css::frame::XController> xController = rFrame.GetController();
    if (xController.is())
    {
        ScTabViewObj* pImp = dynamic_cast<ScTabViewObj*>( xController.get() );
        if (pImp)
            pImp->VisAreaChanged();
    }

    if (GetViewData().HasEditView(GetViewData().GetActivePart()))
    {
        EditView *pEditView = GetViewData().GetEditView(GetViewData().GetActivePart());
        vcl::Cursor *pInPlaceCrsr = pEditView->GetCursor();
        bool bInPlaceVisCursor = pInPlaceCrsr && pInPlaceCrsr->IsVisible();

        if (bInPlaceVisCursor)
            pInPlaceCrsr->Hide();

        ScGridWindow *pGridWindow = GetViewData().GetActiveWin();
        pGridWindow->DrawEditView(*pGridWindow->GetOutDev(), pEditView);

        pGridWindow->flushOverlayManager();
        pGridWindow->GetOutDev()->SetMapMode(GetViewData().GetLogicMode());

        if (bInPlaceVisCursor)
            pInPlaceCrsr->Show();
    }

    if (aViewData.GetViewShell()->HasAccessibilityObjects())
        aViewData.GetViewShell()->BroadcastAccessibility(SfxHint(SfxHintId::ScAccVisAreaChanged));
}

bool ScTabView::HasPageFieldDataAtCursor() const
{
    ScGridWindow* pWin = pGridWin[aViewData.GetActivePart()].get();
    SCCOL nCol = aViewData.GetCurX();
    SCROW nRow = aViewData.GetCurY();
    if (pWin)
        return pWin->GetDPFieldOrientation( nCol, nRow ) == sheet::DataPilotFieldOrientation_PAGE;

    return false;
}

void ScTabView::StartDataSelect()
{
    ScGridWindow* pWin = pGridWin[aViewData.GetActivePart()].get();
    SCCOL nCol = aViewData.GetCurX();
    SCROW nRow = aViewData.GetCurY();

    if (!pWin)
        return;

    switch (pWin->GetDPFieldOrientation(nCol, nRow))
    {
        case sheet::DataPilotFieldOrientation_PAGE:
            //  #i36598# If the cursor is on a page field's data cell,
            //  no meaningful input is possible anyway, so this function
            //  can be used to select a page field entry.
            pWin->LaunchPageFieldMenu( nCol, nRow );
            return;
        case sheet::DataPilotFieldOrientation_COLUMN:
        case sheet::DataPilotFieldOrientation_ROW:
            pWin->LaunchDPFieldMenu( nCol, nRow );
            return;
        default:
            ;
    }

    // Do autofilter if the current cell has autofilter button.  Otherwise do
    // a normal data select popup.
    const ScMergeFlagAttr* pAttr =
        aViewData.GetDocument().GetAttr(
            nCol, nRow, aViewData.GetTabNo(), ATTR_MERGE_FLAG);

    if (pAttr->HasAutoFilter())
        pWin->LaunchAutoFilterMenu(nCol, nRow);
    else
        pWin->LaunchDataSelectMenu(nCol, nRow);
}

void ScTabView::EnableRefInput(bool bFlag)
{
    aHScrollLeft->EnableInput(bFlag);
    aHScrollRight->EnableInput(bFlag);
    aVScrollBottom->EnableInput(bFlag);
    aVScrollTop->EnableInput(bFlag);

    // from here on dynamically created ones

    if(pTabControl!=nullptr) pTabControl->EnableInput(bFlag);

    for (auto& p : pGridWin)
        if (p)
            p->EnableInput(bFlag, false);
    for (auto& p : pColBar)
        if (p)
            p->EnableInput(bFlag, false);
    for (auto& p : pRowBar)
        if (p)
            p->EnableInput(bFlag, false);
}

void ScTabView::EnableAutoSpell( bool bEnable )
{
    const bool bWasEnabled = IsAutoSpell();
    if (bEnable)
        mpSpellCheckCxt =
            std::make_shared<sc::SpellCheckContext>(&aViewData.GetDocument(),
                                      aViewData.GetTabNo());
    else
        mpSpellCheckCxt.reset();

    for (VclPtr<ScGridWindow> & pWin : pGridWin)
    {
        if (!pWin)
            continue;

        pWin->SetAutoSpellContext(mpSpellCheckCxt);
    }

    if (bWasEnabled != bEnable && comphelper::LibreOfficeKit::isActive())
    {
        if (ScTabViewShell* pViewSh = aViewData.GetViewShell())
        {
            ScModelObj* pModel = comphelper::getFromUnoTunnel<ScModelObj>(pViewSh->GetCurrentDocument());
            SfxLokHelper::notifyViewRenderState(pViewSh, pModel);
        }
    }
}

bool ScTabView::IsAutoSpell() const
{
    return static_cast<bool>(mpSpellCheckCxt);
}

void ScTabView::ResetAutoSpell()
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
    {
        if (!pWin)
            continue;

        pWin->ResetAutoSpell();
    }
}

void ScTabView::ResetAutoSpellForContentChange()
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
    {
        if (!pWin)
            continue;

        pWin->ResetAutoSpellForContentChange();
    }
}

void ScTabView::SetAutoSpellData( SCCOL nPosX, SCROW nPosY, const std::vector<editeng::MisspellRanges>* pRanges )
{
    for (VclPtr<ScGridWindow> & pWin: pGridWin)
    {
        if (!pWin)
            continue;

        pWin->SetAutoSpellData(nPosX, nPosY, pRanges);
    }
}

namespace
{

tools::Long lcl_GetRowHeightPx(const ScViewData &rViewData, SCROW nRow, SCTAB nTab)
{
    const sal_uInt16 nSize = rViewData.GetDocument().GetRowHeight(nRow, nTab);
    return ScViewData::ToPixel(nSize, rViewData.GetPPTY());
}

tools::Long lcl_GetColWidthPx(const ScViewData &rViewData, SCCOL nCol, SCTAB nTab)
{
    const sal_uInt16 nSize = rViewData.GetDocument().GetColWidth(nCol, nTab);
    return ScViewData::ToPixel(nSize, rViewData.GetPPTX());
}

void lcl_getGroupIndexes(const ScOutlineArray& rArray, SCCOLROW nStart, SCCOLROW nEnd, std::vector<size_t>& rGroupIndexes)
{
    rGroupIndexes.clear();
    const size_t nGroupDepth = rArray.GetDepth();
    rGroupIndexes.resize(nGroupDepth);

    // Get first group per each level
    for (size_t nLevel = 0; nLevel < nGroupDepth; ++nLevel)
    {
        if (rArray.GetCount(nLevel))
        {
            // look for a group inside the [nStartRow+1, nEndRow] range
            size_t nIndex;
            bool bFound = rArray.GetEntryIndexInRange(nLevel, nStart + 1, nEnd, nIndex);
            if (bFound)
            {
                if (nIndex > 0)
                {
                    // is there a previous group not inside the range
                    // but anyway intersecting it ?
                    const ScOutlineEntry* pPrevEntry = rArray.GetEntry(nLevel, nIndex - 1);
                    if (pPrevEntry && nStart < pPrevEntry->GetEnd())
                    {
                        --nIndex;
                    }
                }
            }
            else
            {
                // look for a group which contains nStartRow+1
                bFound = rArray.GetEntryIndex(nLevel, nStart + 1, nIndex);
                if (!bFound)
                {
                    // look for a group which contains nEndRow
                    bFound = rArray.GetEntryIndex(nLevel, nEnd, nIndex);
                }
            }

            if (bFound)
            {
                // skip groups with no visible control
                bFound = false;
                while (nIndex < rArray.GetCount(nLevel))
                {
                    const ScOutlineEntry* pEntry = rArray.GetEntry(nLevel, nIndex);
                    if (pEntry && pEntry->IsVisible())
                    {
                        bFound = true;
                        break;
                    }
                    if (pEntry && pEntry->GetStart() > nEnd)
                    {
                        break;
                    }
                    ++nIndex;
                }
            }

            rGroupIndexes[nLevel] = bFound ? nIndex : -1;
        }
    }
}

void lcl_createGroupsData(
        SCCOLROW nHeaderIndex, SCCOLROW nEnd, tools::Long nSizePx, tools::Long nTotalPx,
        const ScOutlineArray& rArray, std::vector<size_t>& rGroupIndexes,
        std::vector<tools::Long>& rGroupStartPositions, OStringBuffer& rGroupsBuffer)
{
    const size_t nGroupDepth = rArray.GetDepth();
    // create string data for group controls
    for (size_t nLevel = nGroupDepth - 1; nLevel != size_t(-1); --nLevel)
    {
        size_t nIndex = rGroupIndexes[nLevel];
        if (nIndex == size_t(-1))
            continue;
        const ScOutlineEntry* pEntry = rArray.GetEntry(nLevel, nIndex);
        if (pEntry)
        {
            if (nHeaderIndex < pEntry->GetStart())
            {
                continue;
            }
            else if (nHeaderIndex == pEntry->GetStart())
            {
                rGroupStartPositions[nLevel] = nTotalPx - nSizePx;
            }
            else if (nHeaderIndex > pEntry->GetStart() && (nHeaderIndex < nEnd && nHeaderIndex < pEntry->GetEnd()))
            {
                // for handling group started before the current view range
                if (rGroupStartPositions[nLevel] < 0)
                    rGroupStartPositions[nLevel] *= -1;
                break;
            }
            if (nHeaderIndex == pEntry->GetEnd() || (nHeaderIndex == nEnd && rGroupStartPositions[nLevel] != -1))
            {
                // nHeaderIndex is the end col/row of a group or is the last col/row and a group started and not yet ended

                // append a new group control data
                auto len = rGroupsBuffer.getLength();
                if (len && rGroupsBuffer[len-1] == '}')
                {
                    rGroupsBuffer.append(", ");
                }

                bool bGroupHidden = pEntry->IsHidden();

                rGroupsBuffer.append(
                    "{ \"level\": " + OString::number(nLevel + 1) + ", "
                    "\"index\": " + OString::number(nIndex) + ", "
                    "\"startPos\": " + OString::number(rGroupStartPositions[nLevel]) + ", "
                    "\"endPos\": " + OString::number(nTotalPx) + ", "
                    "\"hidden\": " + OString::number(bGroupHidden ? 1 : 0) + " }");

                // look for the next visible group control at level nLevel
                bool bFound = false;
                ++nIndex;
                while (nIndex < rArray.GetCount(nLevel))
                {
                    pEntry = rArray.GetEntry(nLevel, nIndex);
                    if (pEntry && pEntry->IsVisible())
                    {
                        bFound = true;
                        break;
                    }
                    if (pEntry && pEntry->GetStart() > nEnd)
                    {
                        break;
                    }
                    ++nIndex;
                }
                rGroupIndexes[nLevel] = bFound ? nIndex : -1;
                rGroupStartPositions[nLevel] = -1;
            }
        }
    }
}

class ScRangeProvider
{
public:
    ScRangeProvider(const tools::Rectangle& rArea, bool bInPixels,
                    ScViewData& rViewData):
        mrViewData(rViewData)
    {
        tools::Rectangle aAreaPx = bInPixels ? rArea :
            tools::Rectangle(rArea.Left() * mrViewData.GetPPTX(),
                             rArea.Top() * mrViewData.GetPPTY(),
                             rArea.Right() * mrViewData.GetPPTX(),
                             rArea.Bottom() * mrViewData.GetPPTY());
        calculateBounds(aAreaPx);
    }

    const ScRange& getCellRange() const
    {
        return maRange;
    }

    void getColPositions(tools::Long& rStartColPos, tools::Long& rEndColPos) const
    {
        rStartColPos = maBoundPositions.Left();
        rEndColPos = maBoundPositions.Right();
    }

    void getRowPositions(tools::Long& rStartRowPos, tools::Long& rEndRowPos) const
    {
        rStartRowPos = maBoundPositions.Top();
        rEndRowPos = maBoundPositions.Bottom();
    }

private:
    void calculateBounds(const tools::Rectangle& rAreaPx)
    {
        tools::Long nLeftPx = 0, nRightPx = 0;
        SCCOLROW nStartCol = -1, nEndCol = -1;
        calculateDimensionBounds(rAreaPx.Left(), rAreaPx.Right(), true,
                                 nStartCol, nEndCol, nLeftPx, nRightPx,
                                 mnEnlargeX, mrViewData);
        tools::Long nTopPx = 0, nBottomPx = 0;
        SCCOLROW nStartRow = -1, nEndRow = -1;
        calculateDimensionBounds(rAreaPx.Top(), rAreaPx.Bottom(), false,
                                 nStartRow, nEndRow, nTopPx, nBottomPx,
                                 mnEnlargeY, mrViewData);

        maRange.aStart.Set(nStartCol, nStartRow, mrViewData.GetTabNo());
        maRange.aEnd.Set(nEndCol, nEndRow, mrViewData.GetTabNo());

        maBoundPositions.SetLeft(nLeftPx);
        maBoundPositions.SetRight(nRightPx);
        maBoundPositions.SetTop(nTopPx);
        maBoundPositions.SetBottom(nBottomPx);
    }

    // All positions are in pixels.
    static void calculateDimensionBounds(const tools::Long nStartPos, const tools::Long nEndPos,
                                         bool bColumns, SCCOLROW& rStartIndex,
                                         SCCOLROW& rEndIndex, tools::Long& rBoundStart,
                                         tools::Long& rBoundEnd, SCCOLROW nEnlarge,
                                         ScViewData& rViewData)
    {
        ScPositionHelper& rPosHelper = bColumns ? rViewData.GetLOKWidthHelper() :
            rViewData.GetLOKHeightHelper();
        const auto& rStartNearest = rPosHelper.getNearestByPosition(nStartPos);
        const auto& rEndNearest = rPosHelper.getNearestByPosition(nEndPos);

        ScBoundsProvider aBoundsProvider(rViewData, rViewData.GetTabNo(), bColumns);
        aBoundsProvider.Compute(rStartNearest, rEndNearest, nStartPos, nEndPos);
        aBoundsProvider.EnlargeBy(nEnlarge);
        if (bColumns)
        {
            SCCOL nStartCol = -1, nEndCol = -1;
            aBoundsProvider.GetStartIndexAndPosition(nStartCol, rBoundStart);
            aBoundsProvider.GetEndIndexAndPosition(nEndCol, rBoundEnd);
            rStartIndex = nStartCol;
            rEndIndex = nEndCol;
        }
        else
        {
            SCROW nStartRow = -1, nEndRow = -1;
            aBoundsProvider.GetStartIndexAndPosition(nStartRow, rBoundStart);
            aBoundsProvider.GetEndIndexAndPosition(nEndRow, rBoundEnd);
            rStartIndex = nStartRow;
            rEndIndex = nEndRow;
        }
    }

private:

    ScRange maRange;
    tools::Rectangle maBoundPositions;
    ScViewData& mrViewData;
    static const SCCOLROW mnEnlargeX = 2;
    static const SCCOLROW mnEnlargeY = 2;
};

void lcl_ExtendTiledDimension(bool bColumn, const SCCOLROW nEnd, const SCCOLROW nExtra,
                              ScTabView& rTabView, ScViewData& rViewData)
{
    ScDocument& rDoc = rViewData.GetDocument();
    // If we are approaching current max tiled row/col, signal a size changed event
    // and invalidate the involved area
    SCCOLROW nMaxTiledIndex = bColumn ? rViewData.GetMaxTiledCol() : rViewData.GetMaxTiledRow();
    SCCOLROW nHardLimit = !bColumn ? MAXTILEDROW : rDoc.MaxCol();

    if (nMaxTiledIndex >= nHardLimit)
        return;

    if (nEnd <= nMaxTiledIndex - nExtra) // No need to extend.
        return;

    ScDocShell* pDocSh = rViewData.GetDocShell();
    ScModelObj* pModelObj = pDocSh ? pDocSh->GetModel() : nullptr;
    Size aOldSize(0, 0);
    if (pModelObj)
        aOldSize = pModelObj->getDocumentSize();

    SCCOLROW nNewMaxTiledIndex = std::min(std::max(nEnd, nMaxTiledIndex) + nExtra, nHardLimit);

    if (bColumn)
        rViewData.SetMaxTiledCol(nNewMaxTiledIndex);
    else
        rViewData.SetMaxTiledRow(nNewMaxTiledIndex);

    Size aNewSize(0, 0);
    if (pModelObj)
        aNewSize = pModelObj->getDocumentSize();

    if (!pDocSh)
        return;

    if (pModelObj)
    {
        ScGridWindow* pGridWindow = rViewData.GetActiveWin();
        if (pGridWindow)
        {
            Size aNewSizePx(aNewSize.Width() * rViewData.GetPPTX(), aNewSize.Height() * rViewData.GetPPTY());
            if (aNewSizePx != pGridWindow->GetOutputSizePixel())
                pGridWindow->SetOutputSizePixel(aNewSizePx);
        }
    }

    if (aOldSize == aNewSize)
        return;

    // New area extended to the right/bottom of the sheet after last col/row
    tools::Rectangle aNewArea(Point(0, 0), aNewSize);
    // excluding overlapping area with aNewArea
    if (bColumn)
        aNewArea.SetLeft(aOldSize.getWidth());
    else
        aNewArea.SetTop(aOldSize.getHeight());

    // Only invalidate if spreadsheet has extended to the right or bottom
    if ((bColumn && aNewArea.getOpenWidth()) || (!bColumn && aNewArea.getOpenHeight()))
    {
        rTabView.UpdateSelectionOverlay();
        SfxLokHelper::notifyInvalidation(rViewData.GetViewShell(), &aNewArea);
    }

    // Provide size in the payload, so clients don't have to query for that.
    std::stringstream ss;
    ss << aNewSize.Width() << ", " << aNewSize.Height();
    OString sSize( ss.str() );
    ScModelObj* pModel = comphelper::getFromUnoTunnel<ScModelObj>(
        rViewData.GetViewShell()->GetCurrentDocument());
    SfxLokHelper::notifyDocumentSizeChanged(rViewData.GetViewShell(), sSize, pModel, false);
}

} // anonymous namespace

void ScTabView::getRowColumnHeaders(const tools::Rectangle& rRectangle, tools::JsonWriter& rJsonWriter)
{
    ScDocument& rDoc = aViewData.GetDocument();

    if (rRectangle.IsEmpty())
        return;

    bool bRangeHeaderSupport = comphelper::LibreOfficeKit::isRangeHeaders();

    rJsonWriter.put("commandName", ".uno:ViewRowColumnHeaders");

    SCTAB nTab = aViewData.GetTabNo();
    SCROW nStartRow = -1;
    SCROW nEndRow = -1;
    tools::Long nStartHeightPx = 0;
    SCCOL nStartCol = -1;
    SCCOL nEndCol = -1;
    tools::Long nStartWidthPx = 0;

    tools::Rectangle aOldVisArea(
            mnLOKStartHeaderCol + 1, mnLOKStartHeaderRow + 1,
            mnLOKEndHeaderCol, mnLOKEndHeaderRow);

    ScRangeProvider aRangeProvider(rRectangle, /* bInPixels */ false, aViewData);
    const ScRange& rCellRange = aRangeProvider.getCellRange();

    /// *** start collecting ROWS ***

    /// 1) compute start and end rows

    if (rRectangle.Top() < rRectangle.Bottom())
    {
        SAL_INFO("sc.lok.header", "Row Header: compute start/end rows.");
        tools::Long nEndHeightPx = 0;
        nStartRow = rCellRange.aStart.Row();
        nEndRow = rCellRange.aEnd.Row();
        aRangeProvider.getRowPositions(nStartHeightPx, nEndHeightPx);

        aViewData.GetLOKHeightHelper().removeByIndex(mnLOKStartHeaderRow);
        aViewData.GetLOKHeightHelper().removeByIndex(mnLOKEndHeaderRow);
        aViewData.GetLOKHeightHelper().insert(nStartRow, nStartHeightPx);
        aViewData.GetLOKHeightHelper().insert(nEndRow, nEndHeightPx);

        mnLOKStartHeaderRow = nStartRow;
        mnLOKEndHeaderRow = nEndRow;
    }

    sal_Int32 nVisibleRows = nEndRow - nStartRow;
    if (nVisibleRows < 25)
        nVisibleRows = 25;

    SAL_INFO("sc.lok.header", "Row Header: visible rows: " << nVisibleRows);


    // Get row groups
    // per each level store the index of the first group intersecting
    // [nStartRow, nEndRow] range

    const ScOutlineTable* pTable = rDoc.GetOutlineTable(nTab);
    const ScOutlineArray* pRowArray = pTable ? &(pTable->GetRowArray()) : nullptr;
    size_t nRowGroupDepth = 0;
    std::vector<size_t> aRowGroupIndexes;
    if (bRangeHeaderSupport && pTable)
    {
        nRowGroupDepth = pRowArray->GetDepth();
        lcl_getGroupIndexes(*pRowArray, nStartRow, nEndRow, aRowGroupIndexes);
    }

    /// 2) if we are approaching current max tiled row, signal a size changed event
    ///    and invalidate the involved area
    lcl_ExtendTiledDimension(/* bColumn */ false, nEndRow, nVisibleRows, *this, aViewData);

    /// 3) create string data for rows

    tools::Long nTotalPixels = nStartHeightPx;
    tools::Long nPrevSizePx = -1;
    OStringBuffer aRowGroupsBuffer = "\"rowGroups\": [\n";
    {
        auto rowsNode = rJsonWriter.startArray("rows");

        SAL_INFO("sc.lok.header", "Row Header: [create string data for rows]: start row: "
                << nStartRow << " start height: " << nTotalPixels);

        if (nStartRow != nEndRow)
        {
            auto node = rJsonWriter.startStruct();
            rJsonWriter.put("text", nStartRow + 1);
            rJsonWriter.put("size", nTotalPixels);
            rJsonWriter.put("groupLevels", static_cast<sal_Int64>(nRowGroupDepth));
        }

        std::vector<tools::Long> aRowGroupStartPositions(nRowGroupDepth, -nTotalPixels);
        for (SCROW nRow = nStartRow + 1; nRow <= nEndRow; ++nRow)
        {
            // nSize will be 0 for hidden rows.
            const tools::Long nSizePx = lcl_GetRowHeightPx(aViewData, nRow, nTab);
            nTotalPixels += nSizePx;

            if (bRangeHeaderSupport && nRowGroupDepth > 0)
            {
                lcl_createGroupsData(nRow, nEndRow, nSizePx, nTotalPixels,
                                     *pRowArray, aRowGroupIndexes, aRowGroupStartPositions,
                                     aRowGroupsBuffer);
            }

            if (bRangeHeaderSupport && nRow < nEndRow && nSizePx == nPrevSizePx)
                continue;
            nPrevSizePx = nSizePx;

            auto node = rJsonWriter.startStruct();
            rJsonWriter.put("text", pRowBar[SC_SPLIT_BOTTOM]->GetEntryText(nRow));
            rJsonWriter.put("size", nTotalPixels);
        }

        aRowGroupsBuffer.append("]");
    }
    if (nRowGroupDepth > 0)
    {
        aRowGroupsBuffer.append(",\n");
        rJsonWriter.putRaw(aRowGroupsBuffer);
    }
    ///  end collecting ROWS


    /// *** start collecting COLS ***

    /// 1) compute start and end columns

    if (rRectangle.Left() < rRectangle.Right())
    {
        SAL_INFO("sc.lok.header", "Column Header: compute start/end columns.");
        tools::Long nEndWidthPx = 0;
        nStartCol = rCellRange.aStart.Col();
        nEndCol = rCellRange.aEnd.Col();
        aRangeProvider.getColPositions(nStartWidthPx, nEndWidthPx);

        aViewData.GetLOKWidthHelper().removeByIndex(mnLOKStartHeaderCol);
        aViewData.GetLOKWidthHelper().removeByIndex(mnLOKEndHeaderCol);
        aViewData.GetLOKWidthHelper().insert(nStartCol, nStartWidthPx);
        aViewData.GetLOKWidthHelper().insert(nEndCol, nEndWidthPx);

        mnLOKStartHeaderCol = nStartCol;
        mnLOKEndHeaderCol = nEndCol;
    }

    sal_Int32 nVisibleCols = nEndCol - nStartCol;
    if (nVisibleCols < 10)
        nVisibleCols = 10;


    // Get column groups
    // per each level store the index of the first group intersecting
    // [nStartCol, nEndCol] range

    const ScOutlineArray* pColArray = pTable ? &(pTable->GetColArray()) : nullptr;
    size_t nColGroupDepth = 0;
    std::vector<size_t> aColGroupIndexes;
    if (bRangeHeaderSupport && pTable)
    {
        nColGroupDepth = pColArray->GetDepth();
        lcl_getGroupIndexes(*pColArray, nStartCol, nEndCol, aColGroupIndexes);
    }

    /// 2) if we are approaching current max tiled column, signal a size changed event
    ///    and invalidate the involved area
    lcl_ExtendTiledDimension(/* bColumn */ true, nEndCol, nVisibleCols, *this, aViewData);

    /// 3) create string data for columns
    OStringBuffer aColGroupsBuffer = "\"columnGroups\": [\n";
    {
        auto columnsNode = rJsonWriter.startArray("columns");

        nTotalPixels = nStartWidthPx;
        SAL_INFO("sc.lok.header", "Col Header: [create string data for cols]: start col: "
                << nStartRow << " start width: " << nTotalPixels);

        if (nStartCol != nEndCol)
        {
            auto node = rJsonWriter.startStruct();
            rJsonWriter.put("text", static_cast<sal_Int64>(nStartCol + 1));
            rJsonWriter.put("size", nTotalPixels);
            rJsonWriter.put("groupLevels", static_cast<sal_Int64>(nColGroupDepth));
        }

        std::vector<tools::Long> aColGroupStartPositions(nColGroupDepth, -nTotalPixels);
        nPrevSizePx = -1;
        for (SCCOL nCol = nStartCol + 1; nCol <= nEndCol; ++nCol)
        {
            // nSize will be 0 for hidden columns.
            const tools::Long nSizePx = lcl_GetColWidthPx(aViewData, nCol, nTab);
            nTotalPixels += nSizePx;

            if (bRangeHeaderSupport && nColGroupDepth > 0)
                lcl_createGroupsData(nCol, nEndCol, nSizePx, nTotalPixels,
                                     *pColArray, aColGroupIndexes,
                                     aColGroupStartPositions, aColGroupsBuffer);

            if (bRangeHeaderSupport && nCol < nEndCol && nSizePx == nPrevSizePx)
                continue;
            nPrevSizePx = nSizePx;

            OUString aText = bRangeHeaderSupport ?
                    OUString::number(nCol + 1) : pColBar[SC_SPLIT_LEFT]->GetEntryText(nCol);

            auto node = rJsonWriter.startStruct();
            rJsonWriter.put("text", aText);
            rJsonWriter.put("size", nTotalPixels);
        }

        aColGroupsBuffer.append("]");
    }
    if (nColGroupDepth > 0)
    {
        aColGroupsBuffer.append(",\n");
        rJsonWriter.putRaw(aColGroupsBuffer);
    }
    ///  end collecting COLs

    vcl::Region aNewVisArea(
            tools::Rectangle(mnLOKStartHeaderCol + 1, mnLOKStartHeaderRow + 1,
                    mnLOKEndHeaderCol, mnLOKEndHeaderRow));
    aNewVisArea.Exclude(aOldVisArea);
    tools::Rectangle aChangedArea = aNewVisArea.GetBoundRect();
    if (!aChangedArea.IsEmpty())
    {
        UpdateVisibleRange();
        UpdateFormulas(aChangedArea.Left(), aChangedArea.Top(), aChangedArea.Right(), aChangedArea.Bottom());
    }
}

OString ScTabView::getSheetGeometryData(bool bColumns, bool bRows, bool bSizes, bool bHidden,
                                        bool bFiltered, bool bGroups)
{
    ScDocument& rDoc = aViewData.GetDocument();

    boost::property_tree::ptree aTree;
    aTree.put("commandName", ".uno:SheetGeometryData");
    aTree.put("maxtiledcolumn", rDoc.MaxCol());
    aTree.put("maxtiledrow", MAXTILEDROW);

    auto getJSONString = [](const boost::property_tree::ptree& rTree) {
        std::stringstream aStream;
        boost::property_tree::write_json(aStream, rTree);
        return aStream.str();
    };

    if ((!bSizes && !bHidden && !bFiltered && !bGroups) ||
        (!bColumns && !bRows))
    {
        return OString(getJSONString(aTree));
    }

    struct GeomEntry
    {
        SheetGeomType eType;
        const char* pKey;
        bool bEnabled;
    };

    const GeomEntry aGeomEntries[] = {
        { SheetGeomType::SIZES,    "sizes",    bSizes    },
        { SheetGeomType::HIDDEN,   "hidden",   bHidden   },
        { SheetGeomType::FILTERED, "filtered", bFiltered },
        { SheetGeomType::GROUPS,   "groups",   bGroups   }
    };

    struct DimensionEntry
    {
        const char* pKey;
        bool bDimIsCol;
        bool bEnabled;
    };

    const DimensionEntry aDimEntries[] = {
        { "columns", true,  bColumns },
        { "rows",    false, bRows    }
    };

    SCTAB nTab = aViewData.GetTabNo();

    for (const auto& rDimEntry : aDimEntries)
    {
        if (!rDimEntry.bEnabled)
            continue;

        bool bDimIsCol = rDimEntry.bDimIsCol;

        boost::property_tree::ptree aDimTree;
        for (const auto& rGeomEntry : aGeomEntries)
        {
            if (!rGeomEntry.bEnabled)
                continue;

            OString aGeomDataEncoding = rDoc.dumpSheetGeomData(nTab, bDimIsCol, rGeomEntry.eType);
            // TODO: Investigate if we can avoid the copy of the 'value' string in put().
            aDimTree.put(rGeomEntry.pKey, aGeomDataEncoding.getStr());
        }

        aTree.add_child(rDimEntry.pKey, aDimTree);
    }

    return OString(getJSONString(aTree));
}

void ScTabView::extendTiledAreaIfNeeded()
{
    SAL_INFO("sc.lok.header",
        "extendTiledAreaIfNeeded: START: ClientView: ColRange["
        << mnLOKStartHeaderCol << "," << mnLOKEndHeaderCol
        << "] RowRange[" << mnLOKStartHeaderRow << "," << mnLOKEndHeaderRow
        << "] MaxTiledCol = " << aViewData.GetMaxTiledCol()
        << " MaxTiledRow = " << aViewData.GetMaxTiledRow());

    const tools::Rectangle rVisArea = aViewData.getLOKVisibleArea();
    if (rVisArea.Top() >= rVisArea.Bottom() ||
        rVisArea.Left() >= rVisArea.Right())
        return;

    // Needed for conditional updating of visible-range/formula.
    tools::Rectangle aOldVisCellRange(mnLOKStartHeaderCol + 1, mnLOKStartHeaderRow + 1,
                                      mnLOKEndHeaderCol, mnLOKEndHeaderRow);

    ScRangeProvider aRangeProvider(rVisArea, /* bInPixels */ false, aViewData);
    // Index bounds.
    const ScRange& rCellRange = aRangeProvider.getCellRange();
    const SCCOL nStartCol = rCellRange.aStart.Col();
    const SCCOL nEndCol = rCellRange.aEnd.Col();
    const SCROW nStartRow = rCellRange.aStart.Row();
    const SCROW nEndRow = rCellRange.aEnd.Row();

    // Column/Row positions.
    tools::Long nStartColPos, nEndColPos, nStartRowPos, nEndRowPos;
    aRangeProvider.getColPositions(nStartColPos, nEndColPos);
    aRangeProvider.getRowPositions(nStartRowPos, nEndRowPos);

    ScPositionHelper& rWidthHelper = aViewData.GetLOKWidthHelper();
    ScPositionHelper& rHeightHelper = aViewData.GetLOKHeightHelper();

    // Update mnLOKStartHeaderCol and mnLOKEndHeaderCol members.
    // These are consulted in some ScGridWindow methods.
    if (mnLOKStartHeaderCol != nStartCol)
    {
        rWidthHelper.removeByIndex(mnLOKStartHeaderCol);
        rWidthHelper.insert(nStartCol, nStartColPos);
        mnLOKStartHeaderCol = nStartCol;
    }

    if (mnLOKEndHeaderCol != nEndCol)
    {
        rWidthHelper.removeByIndex(mnLOKEndHeaderCol);
        rWidthHelper.insert(nEndCol, nEndColPos);
        mnLOKEndHeaderCol = nEndCol;
    }

    // Update mnLOKStartHeaderRow and mnLOKEndHeaderRow members.
    // These are consulted in some ScGridWindow methods.
    if (mnLOKStartHeaderRow != nStartRow)
    {
        rHeightHelper.removeByIndex(mnLOKStartHeaderRow);
        rHeightHelper.insert(nStartRow, nStartRowPos);
        mnLOKStartHeaderRow = nStartRow;
    }

    if (mnLOKEndHeaderRow != nEndRow)
    {
        rHeightHelper.removeByIndex(mnLOKEndHeaderRow);
        rHeightHelper.insert(nEndRow, nEndRowPos);
        mnLOKEndHeaderRow = nEndRow;
    }

    constexpr SCCOL nMinExtraCols = 10;
    SCCOL nExtraCols = std::max<SCCOL>(nMinExtraCols, nEndCol - nStartCol);
    // If we are approaching current max tiled column, signal a size changed event
    // and invalidate the involved area.
    lcl_ExtendTiledDimension(/* bColumn */ true, nEndCol, nExtraCols, *this, aViewData);

    constexpr SCROW nMinExtraRows = 25;
    SCROW nExtraRows = std::max(nMinExtraRows, nEndRow - nStartRow);
    // If we are approaching current max tiled row, signal a size changed event
    // and invalidate the involved area.
    lcl_ExtendTiledDimension(/* bColumn */ false, nEndRow, nExtraRows, *this, aViewData);

    vcl::Region aNewVisCellRange(
            tools::Rectangle(mnLOKStartHeaderCol + 1, mnLOKStartHeaderRow + 1,
                             mnLOKEndHeaderCol, mnLOKEndHeaderRow));
    aNewVisCellRange.Exclude(aOldVisCellRange);
    tools::Rectangle aChangedCellRange = aNewVisCellRange.GetBoundRect();
    if (!aChangedCellRange.IsEmpty())
    {
        UpdateVisibleRange();
        UpdateFormulas(aChangedCellRange.Left(), aChangedCellRange.Top(),
                       aChangedCellRange.Right(), aChangedCellRange.Bottom());
    }

    SAL_INFO("sc.lok.header",
        "extendTiledAreaIfNeeded: END: ClientView: ColRange["
        << mnLOKStartHeaderCol << "," << mnLOKEndHeaderCol
        << "] RowRange[" << mnLOKStartHeaderRow << "," << mnLOKEndHeaderRow
        << "] MaxTiledCol = " << aViewData.GetMaxTiledCol()
        << " MaxTiledRow = " << aViewData.GetMaxTiledRow());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
