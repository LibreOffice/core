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

#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/timer.hxx>
#include <vcl/settings.hxx>

#include "svdata.hxx"

#include "rtl/string.hxx"
#include "tools/rc.h"

/*  #i77549#
    HACK: for scrollbars in case of thumb rect, page up and page down rect we
    abuse the HitTestNativeScrollbar interface. All theming engines but OS X
    are actually able to draw the thumb according to our internal representation.
    However OS X draws a little outside. The canonical way would be to enhance the
    HitTestNativeScrollbar passing a ScrollbarValue additionally so all necessary
    information is available in the call.
    .
    However since there is only this one small exception we will deviate a little and
    instead pass the respective rect as control region to allow for a small correction.

    So all places using HitTestNativeScrollbar on ControlPart::ThumbHorz, ControlPart::ThumbVert,
    ControlPart::TrackHorzLeft, ControlPart::TrackHorzRight, ControlPart::TrackVertUpper, ControlPart::TrackVertLower
    do not use the control rectangle as region but the actuall part rectangle, making
    only small deviations feasible.
*/

#include "thumbpos.hxx"

#define SCRBAR_DRAW_BTN1            ((sal_uInt16)0x0001)
#define SCRBAR_DRAW_BTN2            ((sal_uInt16)0x0002)
#define SCRBAR_DRAW_PAGE1           ((sal_uInt16)0x0004)
#define SCRBAR_DRAW_PAGE2           ((sal_uInt16)0x0008)
#define SCRBAR_DRAW_THUMB           ((sal_uInt16)0x0010)
#define SCRBAR_DRAW_BACKGROUND      ((sal_uInt16)0x0020)

#define SCRBAR_STATE_BTN1_DOWN      ((sal_uInt16)0x0001)
#define SCRBAR_STATE_BTN1_DISABLE   ((sal_uInt16)0x0002)
#define SCRBAR_STATE_BTN2_DOWN      ((sal_uInt16)0x0004)
#define SCRBAR_STATE_BTN2_DISABLE   ((sal_uInt16)0x0008)
#define SCRBAR_STATE_PAGE1_DOWN     ((sal_uInt16)0x0010)
#define SCRBAR_STATE_PAGE2_DOWN     ((sal_uInt16)0x0020)
#define SCRBAR_STATE_THUMB_DOWN     ((sal_uInt16)0x0040)

#define SCRBAR_VIEW_STYLE           (WB_3DLOOK | WB_HORZ | WB_VERT)

struct ImplScrollBarData
{
    AutoTimer       maTimer; // Timer
    bool            mbHide;
    Rectangle       maTrackRect; // TODO: move to ScrollBar class when binary incompatibility of ScrollBar class is no longer problematic
};

void ScrollBar::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    mpData              = nullptr;
    mnThumbPixRange     = 0;
    mnThumbPixPos       = 0;
    mnThumbPixSize      = 0;
    mnMinRange          = 0;
    mnMaxRange          = 100;
    mnThumbPos          = 0;
    mnVisibleSize       = 0;
    mnLineSize          = 1;
    mnPageSize          = 1;
    mnDelta             = 0;
    mnDragDraw          = 0;
    mnStateFlags        = 0;
    meScrollType        = ScrollType::DontKnow;
    mbCalcSize          = true;
    mbFullDrag          = false;

    if( !mpData )  // TODO: remove when maTrackRect is no longer in mpData
    {
        mpData = new ImplScrollBarData;
        mpData->maTimer.SetTimeoutHdl( LINK( this, ScrollBar, ImplAutoTimerHdl ) );
        mpData->maTimer.SetDebugName( "vcl::ScrollBar mpData->maTimer" );
        mpData->mbHide = false;
    }

    ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, nullptr );

    long nScrollSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    SetSizePixel( Size( nScrollSize, nScrollSize ) );
}

void ScrollBar::ImplInitStyle( WinBits nStyle )
{
    if ( nStyle & WB_DRAG )
        mbFullDrag = true;
    else
        mbFullDrag = bool(GetSettings().GetStyleSettings().GetDragFullOptions() & DragFullOptions::Scroll);
}

ScrollBar::ScrollBar( vcl::Window* pParent, WinBits nStyle ) :
    Control( WINDOW_SCROLLBAR )
{
    ImplInit( pParent, nStyle );
}

ScrollBar::~ScrollBar()
{
    disposeOnce();
}

void ScrollBar::dispose()
{
    delete mpData; mpData = nullptr;
    Control::dispose();
}

void ScrollBar::ImplUpdateRects( bool bUpdate )
{
    mnStateFlags  &= ~SCRBAR_STATE_BTN1_DISABLE;
    mnStateFlags  &= ~SCRBAR_STATE_BTN2_DISABLE;

    Rectangle& aTrackRect = mpData->maTrackRect; // TODO: remove when maTrackRect is no longer in mpData
    if ( mnThumbPixRange )
    {
        if ( GetStyle() & WB_HORZ )
        {
            maThumbRect.Left()      = aTrackRect.Left()+mnThumbPixPos;
            maThumbRect.Right()     = maThumbRect.Left()+mnThumbPixSize-1;
            if ( !mnThumbPixPos )
                maPage1Rect.Right()     = RECT_EMPTY;
            else
                maPage1Rect.Right()     = maThumbRect.Left()-1;
            if ( mnThumbPixPos >= (mnThumbPixRange-mnThumbPixSize) )
                maPage2Rect.Right()     = RECT_EMPTY;
            else
            {
                maPage2Rect.Left()      = maThumbRect.Right()+1;
                maPage2Rect.Right()     = aTrackRect.Right();
            }
        }
        else
        {
            maThumbRect.Top()       = aTrackRect.Top()+mnThumbPixPos;
            maThumbRect.Bottom()    = maThumbRect.Top()+mnThumbPixSize-1;
            if ( !mnThumbPixPos )
                maPage1Rect.Bottom()    = RECT_EMPTY;
            else
                maPage1Rect.Bottom()    = maThumbRect.Top()-1;
            if ( mnThumbPixPos >= (mnThumbPixRange-mnThumbPixSize) )
                maPage2Rect.Bottom()    = RECT_EMPTY;
            else
            {
                maPage2Rect.Top()       = maThumbRect.Bottom()+1;
                maPage2Rect.Bottom()    = aTrackRect.Bottom();
            }
        }
    }
    else
    {
        if ( GetStyle() & WB_HORZ )
        {
            const long nSpace = aTrackRect.Right() - aTrackRect.Left();
            if ( nSpace > 0 )
            {
                maPage1Rect.Left()   = aTrackRect.Left();
                maPage1Rect.Right()  = aTrackRect.Left() + (nSpace/2);
                maPage2Rect.Left()   = maPage1Rect.Right() + 1;
                maPage2Rect.Right()  = aTrackRect.Right();
            }
        }
        else
        {
            const long nSpace = aTrackRect.Bottom() - aTrackRect.Top();
            if ( nSpace > 0 )
            {
                maPage1Rect.Top()    = aTrackRect.Top();
                maPage1Rect.Bottom() = aTrackRect.Top() + (nSpace/2);
                maPage2Rect.Top()    = maPage1Rect.Bottom() + 1;
                maPage2Rect.Bottom() = aTrackRect.Bottom();
            }
        }
    }

    if( !IsNativeControlSupported(ControlType::Scrollbar, ControlPart::Entire) )
    {
        // disable scrollbar buttons only in VCL's own 'theme'
        // as it is uncommon on other platforms
        if ( mnThumbPos == mnMinRange )
            mnStateFlags |= SCRBAR_STATE_BTN1_DISABLE;
        if ( mnThumbPos >= (mnMaxRange-mnVisibleSize) )
            mnStateFlags |= SCRBAR_STATE_BTN2_DISABLE;
    }

    if ( bUpdate )
    {
        Invalidate();
    }
}

long ScrollBar::ImplCalcThumbPos( long nPixPos )
{
    // Calculate position
    long nCalcThumbPos;
    nCalcThumbPos = ImplMulDiv( nPixPos, mnMaxRange-mnVisibleSize-mnMinRange,
                                mnThumbPixRange-mnThumbPixSize );
    nCalcThumbPos += mnMinRange;
    return nCalcThumbPos;
}

long ScrollBar::ImplCalcThumbPosPix( long nPos )
{
    long nCalcThumbPos;

    // Calculate position
    nCalcThumbPos = ImplMulDiv( nPos-mnMinRange, mnThumbPixRange-mnThumbPixSize,
                                mnMaxRange-mnVisibleSize-mnMinRange );

    // At the start and end of the ScrollBar, we try to show the display correctly
    if ( !nCalcThumbPos && (mnThumbPos > mnMinRange) )
        nCalcThumbPos = 1;
    if ( nCalcThumbPos &&
         ((nCalcThumbPos+mnThumbPixSize) >= mnThumbPixRange) &&
         (mnThumbPos < (mnMaxRange-mnVisibleSize)) )
        nCalcThumbPos--;

    return nCalcThumbPos;
}

void ScrollBar::ImplCalc( bool bUpdate )
{
    const Size aSize = GetOutputSizePixel();
    const long nMinThumbSize = GetSettings().GetStyleSettings().GetMinThumbSize();

    Rectangle& aTrackRect = mpData->maTrackRect;  // TODO: remove when maTrackRect is no longer in mpData
    if ( mbCalcSize )
    {
        Size aOldSize = getCurrentCalcSize();

        const Rectangle aControlRegion( Point(0,0), aSize );
        Rectangle aBtn1Region, aBtn2Region, aTrackRegion, aBoundingRegion;

        if ( GetStyle() & WB_HORZ )
        {
            if ( GetNativeControlRegion( ControlType::Scrollbar, IsRTLEnabled()? ControlPart::ButtonRight: ControlPart::ButtonLeft,
                        aControlRegion, ControlState::NONE, ImplControlValue(), OUString(), aBoundingRegion, aBtn1Region ) &&
                 GetNativeControlRegion( ControlType::Scrollbar, IsRTLEnabled()? ControlPart::ButtonLeft: ControlPart::ButtonRight,
                        aControlRegion, ControlState::NONE, ImplControlValue(), OUString(), aBoundingRegion, aBtn2Region ) )
            {
                maBtn1Rect = aBtn1Region;
                maBtn2Rect = aBtn2Region;
            }
            else
            {
                Size aBtnSize( aSize.Height(), aSize.Height() );
                maBtn2Rect.Top()    = maBtn1Rect.Top();
                maBtn2Rect.Left()   = aSize.Width()-aSize.Height();
                maBtn1Rect.SetSize( aBtnSize );
                maBtn2Rect.SetSize( aBtnSize );
            }

            if ( GetNativeControlRegion( ControlType::Scrollbar, ControlPart::TrackHorzArea,
                     aControlRegion, ControlState::NONE, ImplControlValue(), OUString(), aBoundingRegion, aTrackRegion ) )
                aTrackRect = aTrackRegion;
            else
                aTrackRect = Rectangle( maBtn1Rect.TopRight(), maBtn2Rect.BottomLeft() );

            // Check if available space is big enough for thumb ( min thumb size = ScrBar width/height )
            mnThumbPixRange = aTrackRect.Right() - aTrackRect.Left();
            if( mnThumbPixRange > 0 )
            {
                maPage1Rect.Left()      = aTrackRect.Left();
                maPage1Rect.Bottom()    =
                maPage2Rect.Bottom()    =
                maThumbRect.Bottom()    = aTrackRect.Bottom();
            }
            else
            {
                mnThumbPixRange = 0;
                maPage1Rect.SetEmpty();
                maPage2Rect.SetEmpty();
            }
        }
        else
        {
            if ( GetNativeControlRegion( ControlType::Scrollbar, ControlPart::ButtonUp,
                        aControlRegion, ControlState::NONE, ImplControlValue(), OUString(), aBoundingRegion, aBtn1Region ) &&
                 GetNativeControlRegion( ControlType::Scrollbar, ControlPart::ButtonDown,
                        aControlRegion, ControlState::NONE, ImplControlValue(), OUString(), aBoundingRegion, aBtn2Region ) )
            {
                maBtn1Rect = aBtn1Region;
                maBtn2Rect = aBtn2Region;
            }
            else
            {
                const Size aBtnSize( aSize.Width(), aSize.Width() );
                maBtn2Rect.Left()   = maBtn1Rect.Left();
                maBtn2Rect.Top()    = aSize.Height()-aSize.Width();
                maBtn1Rect.SetSize( aBtnSize );
                maBtn2Rect.SetSize( aBtnSize );
            }

            if ( GetNativeControlRegion( ControlType::Scrollbar, ControlPart::TrackVertArea,
                     aControlRegion, ControlState::NONE, ImplControlValue(), OUString(), aBoundingRegion, aTrackRegion ) )
                aTrackRect = aTrackRegion;
            else
                aTrackRect = Rectangle( maBtn1Rect.BottomLeft()+Point(0,1), maBtn2Rect.TopRight() );

            // Check if available space is big enough for thumb
            mnThumbPixRange = aTrackRect.Bottom() - aTrackRect.Top();
            if( mnThumbPixRange > 0 )
            {
                maPage1Rect.Top()       = aTrackRect.Top();
                maPage1Rect.Right()     =
                maPage2Rect.Right()     =
                maThumbRect.Right()     = aTrackRect.Right();
            }
            else
            {
                mnThumbPixRange = 0;
                maPage1Rect.SetEmpty();
                maPage2Rect.SetEmpty();
            }
        }

        if ( !mnThumbPixRange )
            maThumbRect.SetEmpty();

        mbCalcSize = false;

        Size aNewSize = getCurrentCalcSize();
        if (aOldSize != aNewSize)
        {
            queue_resize();
        }
    }

    if ( mnThumbPixRange )
    {
        // Calculate values
        if ( (mnVisibleSize >= (mnMaxRange-mnMinRange)) ||
             ((mnMaxRange-mnMinRange) <= 0) )
        {
            mnThumbPos      = mnMinRange;
            mnThumbPixPos   = 0;
            mnThumbPixSize  = mnThumbPixRange;
        }
        else
        {
            if ( mnVisibleSize )
                mnThumbPixSize = ImplMulDiv( mnThumbPixRange, mnVisibleSize, mnMaxRange-mnMinRange );
            else
            {
                if ( GetStyle() & WB_HORZ )
                    mnThumbPixSize = maThumbRect.GetWidth();
                else
                    mnThumbPixSize = maThumbRect.GetHeight();
            }
            if ( mnThumbPixSize < nMinThumbSize )
                mnThumbPixSize = nMinThumbSize;
            if ( mnThumbPixSize > mnThumbPixRange )
                mnThumbPixSize = mnThumbPixRange;
            mnThumbPixPos = ImplCalcThumbPosPix( mnThumbPos );
        }
    }

    // If we're ought to ouput again and we have been triggered
    // a Paint event via an Action, we don't output directly,
    // but invalidate everything
    if ( bUpdate && HasPaintEvent() )
    {
        Invalidate();
        bUpdate = false;
    }
    ImplUpdateRects( bUpdate );
}

void ScrollBar::Draw( OutputDevice* pDev, const Point& rPos, const Size& /* rSize */, DrawFlags nFlags )
{
    Point aPos  = pDev->LogicToPixel( rPos );

    pDev->Push();
    pDev->SetMapMode();
    if ( !(nFlags & DrawFlags::Mono) )
    {
        // DecoView uses the FaceColor...
        AllSettings aSettings = pDev->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        if ( IsControlBackground() )
            aStyleSettings.SetFaceColor( GetControlBackground() );
        else
            aStyleSettings.SetFaceColor( GetSettings().GetStyleSettings().GetFaceColor() );

        aSettings.SetStyleSettings( aStyleSettings );
        pDev->SetSettings( aSettings );
    }

    // For printing:
    // - calculate the size of the rects
    // - because this is zero-based add the correct offset
    // - print
    // - force recalculate

    if ( mbCalcSize )
        ImplCalc( false );

    maBtn1Rect+=aPos;
    maBtn2Rect+=aPos;
    maThumbRect+=aPos;
    mpData->maTrackRect+=aPos; // TODO: update when maTrackRect is no longer in mpData
    maPage1Rect+=aPos;
    maPage2Rect+=aPos;

    ImplDraw(*pDev);
    pDev->Pop();

    mbCalcSize = true;
}

bool ScrollBar::ImplDrawNative(vcl::RenderContext& rRenderContext, sal_uInt16 nDrawFlags)
{
    ScrollbarValue scrValue;

    bool bNativeOK = rRenderContext.IsNativeControlSupported(ControlType::Scrollbar, ControlPart::Entire);
    if (!bNativeOK)
        return false;

    bool bHorz = (GetStyle() & WB_HORZ) != 0;

    // Draw the entire background if the control supports it
    if (rRenderContext.IsNativeControlSupported(ControlType::Scrollbar, bHorz ? ControlPart::DrawBackgroundHorz : ControlPart::DrawBackgroundVert))
    {
        ControlState nState = (IsEnabled() ? ControlState::ENABLED : ControlState::NONE)
                            | (HasFocus() ? ControlState::FOCUSED : ControlState::NONE);

        scrValue.mnMin = mnMinRange;
        scrValue.mnMax = mnMaxRange;
        scrValue.mnCur = mnThumbPos;
        scrValue.mnVisibleSize = mnVisibleSize;
        scrValue.maThumbRect = maThumbRect;
        scrValue.maButton1Rect = maBtn1Rect;
        scrValue.maButton2Rect = maBtn2Rect;
        scrValue.mnButton1State = ((mnStateFlags & SCRBAR_STATE_BTN1_DOWN) ? ControlState::PRESSED : ControlState::NONE) |
                            ((!(mnStateFlags & SCRBAR_STATE_BTN1_DISABLE)) ? ControlState::ENABLED : ControlState::NONE);
        scrValue.mnButton2State = ((mnStateFlags & SCRBAR_STATE_BTN2_DOWN) ? ControlState::PRESSED : ControlState::NONE) |
                            ((!(mnStateFlags & SCRBAR_STATE_BTN2_DISABLE)) ? ControlState::ENABLED : ControlState::NONE);
        scrValue.mnThumbState = nState | ((mnStateFlags & SCRBAR_STATE_THUMB_DOWN) ? ControlState::PRESSED : ControlState::NONE);
        scrValue.mnPage1State = nState | ((mnStateFlags & SCRBAR_STATE_PAGE1_DOWN) ? ControlState::PRESSED : ControlState::NONE);
        scrValue.mnPage2State = nState | ((mnStateFlags & SCRBAR_STATE_PAGE2_DOWN) ? ControlState::PRESSED : ControlState::NONE);

        if (IsMouseOver())
        {
            Rectangle* pRect = ImplFindPartRect(GetPointerPosPixel());
            if (pRect)
            {
                if (pRect == &maThumbRect)
                    scrValue.mnThumbState |= ControlState::ROLLOVER;
                else if (pRect == &maBtn1Rect)
                    scrValue.mnButton1State |= ControlState::ROLLOVER;
                else if (pRect == &maBtn2Rect)
                    scrValue.mnButton2State |= ControlState::ROLLOVER;
                else if (pRect == &maPage1Rect)
                    scrValue.mnPage1State |= ControlState::ROLLOVER;
                else if (pRect == &maPage2Rect)
                    scrValue.mnPage2State |= ControlState::ROLLOVER;
            }
        }

        Rectangle aCtrlRegion;
        aCtrlRegion.Union(maBtn1Rect);
        aCtrlRegion.Union(maBtn2Rect);
        aCtrlRegion.Union(maPage1Rect);
        aCtrlRegion.Union(maPage2Rect);
        aCtrlRegion.Union(maThumbRect);

        Rectangle aRequestedRegion(Point(0,0), GetOutputSizePixel());
        // if the actual native control region is smaller then the region that
        // we requested the control to draw in, then draw a background rectangle
        // to avoid drawing artifacts in the uncovered region
        if (aCtrlRegion.GetWidth() < aRequestedRegion.GetWidth() ||
            aCtrlRegion.GetHeight() < aRequestedRegion.GetHeight())
        {
            Color aFaceColor = rRenderContext.GetSettings().GetStyleSettings().GetFaceColor();
            rRenderContext.SetFillColor(aFaceColor);
            rRenderContext.SetLineColor(aFaceColor);
            rRenderContext.DrawRect(aRequestedRegion);
        }

        bNativeOK = rRenderContext.DrawNativeControl(ControlType::Scrollbar, (bHorz ? ControlPart::DrawBackgroundHorz : ControlPart::DrawBackgroundVert),
                                                    aCtrlRegion, nState, scrValue, OUString());
    }
    else
    {
        if ((nDrawFlags & SCRBAR_DRAW_PAGE1) || (nDrawFlags & SCRBAR_DRAW_PAGE2))
        {
            ControlPart part1 = bHorz ? ControlPart::TrackHorzLeft : ControlPart::TrackVertUpper;
            ControlPart part2 = bHorz ? ControlPart::TrackHorzRight : ControlPart::TrackVertLower;
            Rectangle aCtrlRegion1(maPage1Rect);
            Rectangle aCtrlRegion2(maPage2Rect);
            ControlState nState1 = (IsEnabled() ? ControlState::ENABLED : ControlState::NONE)
                                 | (HasFocus() ? ControlState::FOCUSED : ControlState::NONE);
            ControlState nState2 = nState1;

            nState1 |= ((mnStateFlags & SCRBAR_STATE_PAGE1_DOWN) ? ControlState::PRESSED : ControlState::NONE);
            nState2 |= ((mnStateFlags & SCRBAR_STATE_PAGE2_DOWN) ? ControlState::PRESSED : ControlState::NONE);

            if (IsMouseOver())
            {
                Rectangle* pRect = ImplFindPartRect(GetPointerPosPixel());
                if (pRect)
                {
                    if (pRect == &maPage1Rect)
                        nState1 |= ControlState::ROLLOVER;
                    else if (pRect == &maPage2Rect)
                        nState2 |= ControlState::ROLLOVER;
                }
            }

            if (nDrawFlags & SCRBAR_DRAW_PAGE1)
                bNativeOK = rRenderContext.DrawNativeControl(ControlType::Scrollbar, part1, aCtrlRegion1, nState1, scrValue, OUString());

            if (nDrawFlags & SCRBAR_DRAW_PAGE2)
                bNativeOK = rRenderContext.DrawNativeControl(ControlType::Scrollbar, part2, aCtrlRegion2, nState2, scrValue, OUString());
        }
        if ((nDrawFlags & SCRBAR_DRAW_BTN1) || (nDrawFlags & SCRBAR_DRAW_BTN2))
        {
            ControlPart part1 = bHorz ? ControlPart::ButtonLeft : ControlPart::ButtonUp;
            ControlPart part2 = bHorz ? ControlPart::ButtonRight : ControlPart::ButtonDown;
            Rectangle aCtrlRegion1(maBtn1Rect);
            Rectangle aCtrlRegion2(maBtn2Rect);
            ControlState nState1 = HasFocus() ? ControlState::FOCUSED : ControlState::NONE;
            ControlState nState2 = nState1;

            if (!Window::IsEnabled() || !IsEnabled())
                nState1 = (nState2 &= ~ControlState::ENABLED);
            else
                nState1 = (nState2 |= ControlState::ENABLED);

            nState1 |= ((mnStateFlags & SCRBAR_STATE_BTN1_DOWN) ? ControlState::PRESSED : ControlState::NONE);
            nState2 |= ((mnStateFlags & SCRBAR_STATE_BTN2_DOWN) ? ControlState::PRESSED : ControlState::NONE);

            if (mnStateFlags & SCRBAR_STATE_BTN1_DISABLE)
                nState1 &= ~ControlState::ENABLED;
            if (mnStateFlags & SCRBAR_STATE_BTN2_DISABLE)
                nState2 &= ~ControlState::ENABLED;

            if (IsMouseOver())
            {
                Rectangle* pRect = ImplFindPartRect(GetPointerPosPixel());
                if (pRect)
                {
                    if (pRect == &maBtn1Rect)
                        nState1 |= ControlState::ROLLOVER;
                    else if (pRect == &maBtn2Rect)
                        nState2 |= ControlState::ROLLOVER;
                }
            }

            if (nDrawFlags & SCRBAR_DRAW_BTN1)
                bNativeOK = rRenderContext.DrawNativeControl(ControlType::Scrollbar, part1, aCtrlRegion1, nState1, scrValue, OUString());

            if (nDrawFlags & SCRBAR_DRAW_BTN2)
                bNativeOK = rRenderContext.DrawNativeControl(ControlType::Scrollbar, part2, aCtrlRegion2, nState2, scrValue, OUString());
        }
        if ((nDrawFlags & SCRBAR_DRAW_THUMB) && !maThumbRect.IsEmpty())
        {
            ControlState nState = IsEnabled() ? ControlState::ENABLED : ControlState::NONE;
            Rectangle aCtrlRegion(maThumbRect);

            if (mnStateFlags & SCRBAR_STATE_THUMB_DOWN)
                nState |= ControlState::PRESSED;

            if (HasFocus())
                nState |= ControlState::FOCUSED;

            if (IsMouseOver())
            {
                Rectangle* pRect = ImplFindPartRect(GetPointerPosPixel());
                if (pRect)
                {
                    if (pRect == &maThumbRect)
                        nState |= ControlState::ROLLOVER;
                }
            }

            bNativeOK = rRenderContext.DrawNativeControl(ControlType::Scrollbar, (bHorz ? ControlPart::ThumbHorz : ControlPart::ThumbVert),
                                                         aCtrlRegion, nState, scrValue, OUString());
        }
    }
    return bNativeOK;
}

void ScrollBar::ImplDraw(vcl::RenderContext& rRenderContext)
{
    DecorationView aDecoView(&rRenderContext);
    Rectangle aTempRect;
    DrawButtonFlags nStyle;
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    SymbolType eSymbolType;
    bool bEnabled = IsEnabled();

    // Finish some open calculations (if any)
    if (mbCalcSize)
        ImplCalc(false);

    //vcl::Window *pWin = NULL;
    //if (rRenderContext.GetOutDevType() == OUTDEV_WINDOW)
    //    pWin = static_cast<vcl::Window*>(&rRenderContext);

    // Draw the entire control if the native theme engine needs it
    if (rRenderContext.IsNativeControlSupported(ControlType::Scrollbar, ControlPart::DrawBackgroundHorz))
    {
        ImplDrawNative(rRenderContext, SCRBAR_DRAW_BACKGROUND);
        return;
    }

    if (!ImplDrawNative(rRenderContext, SCRBAR_DRAW_BTN1))
    {
        nStyle = DrawButtonFlags::NoLightBorder;
        if (mnStateFlags & SCRBAR_STATE_BTN1_DOWN)
            nStyle |= DrawButtonFlags::Pressed;
        aTempRect = aDecoView.DrawButton( PixelToLogic(maBtn1Rect), nStyle );
        ImplCalcSymbolRect( aTempRect );
        DrawSymbolFlags nSymbolStyle = DrawSymbolFlags::NONE;
        if ((mnStateFlags & SCRBAR_STATE_BTN1_DISABLE) || !bEnabled)
            nSymbolStyle |= DrawSymbolFlags::Disable;
        if (GetStyle() & WB_HORZ)
            eSymbolType = SymbolType::SPIN_LEFT;
        else
            eSymbolType = SymbolType::SPIN_UP;
        aDecoView.DrawSymbol(aTempRect, eSymbolType, rStyleSettings.GetButtonTextColor(), nSymbolStyle);
    }

    if (!ImplDrawNative(rRenderContext, SCRBAR_DRAW_BTN2))
    {
        nStyle = DrawButtonFlags::NoLightBorder;
        if (mnStateFlags & SCRBAR_STATE_BTN2_DOWN)
            nStyle |= DrawButtonFlags::Pressed;
        aTempRect = aDecoView.DrawButton(PixelToLogic(maBtn2Rect), nStyle);
        ImplCalcSymbolRect(aTempRect);
        DrawSymbolFlags nSymbolStyle = DrawSymbolFlags::NONE;
        if ((mnStateFlags & SCRBAR_STATE_BTN2_DISABLE) || !bEnabled)
            nSymbolStyle |= DrawSymbolFlags::Disable;
        if (GetStyle() & WB_HORZ)
            eSymbolType = SymbolType::SPIN_RIGHT;
        else
            eSymbolType = SymbolType::SPIN_DOWN;
        aDecoView.DrawSymbol(aTempRect, eSymbolType, rStyleSettings.GetButtonTextColor(), nSymbolStyle);
    }

    rRenderContext.SetLineColor();

    if (!ImplDrawNative(rRenderContext, SCRBAR_DRAW_THUMB))
    {
        if (!maThumbRect.IsEmpty())
        {
            if (bEnabled)
            {
                nStyle = DrawButtonFlags::NoLightBorder;
                aTempRect = aDecoView.DrawButton(PixelToLogic(maThumbRect), nStyle);
            }
            else
            {
                rRenderContext.SetFillColor(rStyleSettings.GetCheckedColor());
                rRenderContext.DrawRect(PixelToLogic(maThumbRect));
            }
        }
    }

    if (!ImplDrawNative(rRenderContext, SCRBAR_DRAW_PAGE1))
    {
        if (mnStateFlags & SCRBAR_STATE_PAGE1_DOWN)
            rRenderContext.SetFillColor(rStyleSettings.GetShadowColor());
        else
            rRenderContext.SetFillColor(rStyleSettings.GetCheckedColor());
        rRenderContext.DrawRect(PixelToLogic(maPage1Rect));
    }
    if (!ImplDrawNative(rRenderContext, SCRBAR_DRAW_PAGE2))
    {
        if (mnStateFlags & SCRBAR_STATE_PAGE2_DOWN)
            rRenderContext.SetFillColor(rStyleSettings.GetShadowColor());
        else
            rRenderContext.SetFillColor(rStyleSettings.GetCheckedColor());
        rRenderContext.DrawRect(PixelToLogic(maPage2Rect));
    }
}

long ScrollBar::ImplScroll( long nNewPos, bool bCallEndScroll )
{
    long nOldPos = mnThumbPos;
    SetThumbPos( nNewPos );
    long nDelta = mnThumbPos-nOldPos;
    if ( nDelta )
    {
        mnDelta = nDelta;
        Scroll();
        if ( bCallEndScroll )
            EndScroll();
        mnDelta = 0;
    }
    return nDelta;
}

long ScrollBar::ImplDoAction( bool bCallEndScroll )
{
    long nDelta = 0;

    switch ( meScrollType )
    {
        case ScrollType::LineUp:
            nDelta = ImplScroll( mnThumbPos-mnLineSize, bCallEndScroll );
            break;

        case ScrollType::LineDown:
            nDelta = ImplScroll( mnThumbPos+mnLineSize, bCallEndScroll );
            break;

        case ScrollType::PageUp:
            nDelta = ImplScroll( mnThumbPos-mnPageSize, bCallEndScroll );
            break;

        case ScrollType::PageDown:
            nDelta = ImplScroll( mnThumbPos+mnPageSize, bCallEndScroll );
            break;
        default:
            ;
    }

    return nDelta;
}

void ScrollBar::ImplDoMouseAction( const Point& rMousePos, bool bCallAction )
{
    sal_uInt16  nOldStateFlags = mnStateFlags;
    bool    bAction = false;
    bool        bHorizontal = ( GetStyle() & WB_HORZ ) != 0;
    bool        bIsInside = false;

    Point aPoint( 0, 0 );
    Rectangle aControlRegion( aPoint, GetOutputSizePixel() );

    switch ( meScrollType )
    {
        case ScrollType::LineUp:
            if ( HitTestNativeScrollbar( bHorizontal? (IsRTLEnabled()? ControlPart::ButtonRight: ControlPart::ButtonLeft): ControlPart::ButtonUp,
                        aControlRegion, rMousePos, bIsInside )?
                    bIsInside:
                    maBtn1Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_BTN1_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_BTN1_DOWN;
            break;

        case ScrollType::LineDown:
            if ( HitTestNativeScrollbar( bHorizontal? (IsRTLEnabled()? ControlPart::ButtonLeft: ControlPart::ButtonRight): ControlPart::ButtonDown,
                        aControlRegion, rMousePos, bIsInside )?
                    bIsInside:
                    maBtn2Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_BTN2_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_BTN2_DOWN;
            break;

        case ScrollType::PageUp:
            // HitTestNativeScrollbar, see remark at top of file
            if ( HitTestNativeScrollbar( bHorizontal? ControlPart::TrackHorzLeft: ControlPart::TrackVertUpper,
                                       maPage1Rect, rMousePos, bIsInside )?
                    bIsInside:
                    maPage1Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_PAGE1_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_PAGE1_DOWN;
            break;

        case ScrollType::PageDown:
            // HitTestNativeScrollbar, see remark at top of file
            if ( HitTestNativeScrollbar( bHorizontal? ControlPart::TrackHorzRight: ControlPart::TrackVertLower,
                                       maPage2Rect, rMousePos, bIsInside )?
                    bIsInside:
                    maPage2Rect.IsInside( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_PAGE2_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_PAGE2_DOWN;
            break;
        default:
            ;
    }

    if ( nOldStateFlags != mnStateFlags )
        Invalidate();
    if ( bAction )
        ImplDoAction( false );
}

void ScrollBar::ImplDragThumb( const Point& rMousePos )
{
    long nMovePix;
    if ( GetStyle() & WB_HORZ )
        nMovePix = rMousePos.X()-(maThumbRect.Left()+mnMouseOff);
    else
        nMovePix = rMousePos.Y()-(maThumbRect.Top()+mnMouseOff);

    // Move thumb if necessary
    if ( nMovePix )
    {
        mnThumbPixPos += nMovePix;
        if ( mnThumbPixPos < 0 )
            mnThumbPixPos = 0;
        if ( mnThumbPixPos > (mnThumbPixRange-mnThumbPixSize) )
            mnThumbPixPos = mnThumbPixRange-mnThumbPixSize;
        long nOldPos = mnThumbPos;
        mnThumbPos = ImplCalcThumbPos( mnThumbPixPos );
        ImplUpdateRects();
        if ( mbFullDrag && (nOldPos != mnThumbPos) )
        {
            // When dragging in windows the repaint request gets starved so dragging
            // the scrollbar feels slower than it actually is. Let's force an immediate
            // repaint of the scrollbar.
            ImplDraw(*this);

            mnDelta = mnThumbPos-nOldPos;
            Scroll();
            mnDelta = 0;
        }
    }
}

void ScrollBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bPrimaryWarps = GetSettings().GetStyleSettings().GetPrimaryButtonWarpsSlider();
    bool bWarp = bPrimaryWarps ? rMEvt.IsLeft() : rMEvt.IsMiddle();
    bool bPrimaryWarping = bWarp && rMEvt.IsLeft();
    bool bPage = bPrimaryWarps ? rMEvt.IsRight() : rMEvt.IsLeft();

    if (rMEvt.IsLeft() || rMEvt.IsMiddle() || rMEvt.IsRight())
    {
        Point aPosPixel;
        if (!IsMapModeEnabled() && GetMapMode().GetMapUnit() == MAP_TWIP)
        {
            // rMEvt coordinates are in twips.
            Push(PushFlags::MAPMODE);
            EnableMapMode();
            MapMode aMapMode = GetMapMode();
            aMapMode.SetOrigin(Point(0, 0));
            SetMapMode(aMapMode);
            aPosPixel = LogicToPixel(rMEvt.GetPosPixel());
            Pop();
        }
        const Point&        rMousePos = (GetMapMode().GetMapUnit() != MAP_TWIP ? rMEvt.GetPosPixel() : aPosPixel);
        StartTrackingFlags  nTrackFlags = StartTrackingFlags::NONE;
        bool                bHorizontal = ( GetStyle() & WB_HORZ ) != 0;
        bool                bIsInside = false;
        bool                bDragToMouse = false;

        Point aPoint( 0, 0 );
        Rectangle aControlRegion( aPoint, GetOutputSizePixel() );

        if ( HitTestNativeScrollbar( bHorizontal? (IsRTLEnabled()? ControlPart::ButtonRight: ControlPart::ButtonLeft): ControlPart::ButtonUp,
                    aControlRegion, rMousePos, bIsInside )?
                bIsInside:
                maBtn1Rect.IsInside( rMousePos ) )
        {
            if (rMEvt.IsLeft() && !(mnStateFlags & SCRBAR_STATE_BTN1_DISABLE) )
            {
                nTrackFlags     = StartTrackingFlags::ButtonRepeat;
                meScrollType    = ScrollType::LineUp;
                mnDragDraw      = SCRBAR_DRAW_BTN1;
            }
        }
        else if ( HitTestNativeScrollbar( bHorizontal? (IsRTLEnabled()? ControlPart::ButtonLeft: ControlPart::ButtonRight): ControlPart::ButtonDown,
                    aControlRegion, rMousePos, bIsInside )?
                bIsInside:
                maBtn2Rect.IsInside( rMousePos ) )
        {
            if (rMEvt.IsLeft() && !(mnStateFlags & SCRBAR_STATE_BTN2_DISABLE) )
            {
                nTrackFlags     = StartTrackingFlags::ButtonRepeat;
                meScrollType    = ScrollType::LineDown;
                mnDragDraw      = SCRBAR_DRAW_BTN2;
            }
        }
        else
        {
            bool bThumbHit = HitTestNativeScrollbar( bHorizontal? ControlPart::ThumbHorz : ControlPart::ThumbVert,
                                                   maThumbRect, rMousePos, bIsInside )
                             ? bIsInside : maThumbRect.IsInside( rMousePos );

            bool bThumbAction = bWarp || bPage;

            bool bDragHandling = bWarp || (bThumbHit && bThumbAction);
            if( bDragHandling )
            {
                if( mpData )
                {
                    mpData->mbHide = true; // disable focus blinking
                    if (HasFocus())
                    {
                        mnStateFlags |= SCRBAR_DRAW_THUMB; // paint without focus
                        Invalidate();
                    }
                }

                if ( mnVisibleSize < mnMaxRange-mnMinRange )
                {
                    nTrackFlags     = StartTrackingFlags::NONE;
                    meScrollType    = ScrollType::Drag;
                    mnDragDraw      = SCRBAR_DRAW_THUMB;

                    // calculate mouse offset
                    if (bWarp && (!bThumbHit || !bPrimaryWarping))
                    {
                        bDragToMouse = true;
                        if ( GetStyle() & WB_HORZ )
                            mnMouseOff = maThumbRect.GetWidth()/2;
                        else
                            mnMouseOff = maThumbRect.GetHeight()/2;
                    }
                    else
                    {
                        if ( GetStyle() & WB_HORZ )
                            mnMouseOff = rMousePos.X()-maThumbRect.Left();
                        else
                            mnMouseOff = rMousePos.Y()-maThumbRect.Top();
                    }

                    mnStateFlags |= SCRBAR_STATE_THUMB_DOWN;
                    Invalidate();
                }
            }
            else if(bPage && (!HitTestNativeScrollbar( bHorizontal? ControlPart::TrackHorzArea : ControlPart::TrackVertArea,
                                           aControlRegion, rMousePos, bIsInside ) ||
                bIsInside) )
            {
                nTrackFlags = StartTrackingFlags::ButtonRepeat;

                // HitTestNativeScrollbar, see remark at top of file
                if ( HitTestNativeScrollbar( bHorizontal? ControlPart::TrackHorzLeft : ControlPart::TrackVertUpper,
                                           maPage1Rect, rMousePos, bIsInside )?
                    bIsInside:
                    maPage1Rect.IsInside( rMousePos ) )
                {
                    meScrollType    = ScrollType::PageUp;
                    mnDragDraw      = SCRBAR_DRAW_PAGE1;
                }
                else
                {
                    meScrollType    = ScrollType::PageDown;
                    mnDragDraw      = SCRBAR_DRAW_PAGE2;
                }
            }
        }

        // Should we start Tracking?
        if ( meScrollType != ScrollType::DontKnow )
        {
            // store original position for cancel and EndScroll delta
            mnStartPos = mnThumbPos;
            // #92906# Call StartTracking() before ImplDoMouseAction(), otherwise
            // MouseButtonUp() / EndTracking() may be called if somebody is spending
            // a lot of time in the scroll handler
            StartTracking( nTrackFlags );
            ImplDoMouseAction( rMousePos );

            if( bDragToMouse )
                ImplDragThumb( rMousePos );
        }
    }
}

void ScrollBar::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        // Restore Button and PageRect status
        sal_uInt16 nOldStateFlags = mnStateFlags;
        mnStateFlags &= ~(SCRBAR_STATE_BTN1_DOWN | SCRBAR_STATE_BTN2_DOWN |
                          SCRBAR_STATE_PAGE1_DOWN | SCRBAR_STATE_PAGE2_DOWN |
                          SCRBAR_STATE_THUMB_DOWN);
        if ( nOldStateFlags != mnStateFlags )
            Invalidate();
        mnDragDraw = 0;

        // Restore the old ThumbPosition when canceled
        if ( rTEvt.IsTrackingCanceled() )
        {
            long nOldPos = mnThumbPos;
            SetThumbPos( mnStartPos );
            mnDelta = mnThumbPos-nOldPos;
            Scroll();
        }

        if ( meScrollType == ScrollType::Drag )
        {
            // On a SCROLLDRAG we recalculate the Thumb, so that it's back to a
            // rounded ThumbPosition
            ImplCalc();

            if ( !mbFullDrag && (mnStartPos != mnThumbPos) )
            {
                mnDelta = mnThumbPos-mnStartPos;
                Scroll();
                mnDelta = 0;
            }
        }

        mnDelta = mnThumbPos-mnStartPos;
        EndScroll();
        mnDelta = 0;
        meScrollType = ScrollType::DontKnow;

        if( mpData )
            mpData->mbHide = false; // re-enable focus blinking
    }
    else
    {
        Point aPosPixel;
        if (!IsMapModeEnabled() && GetMapMode().GetMapUnit() == MAP_TWIP)
        {
            // rTEvt coordinates are in twips.
            Push(PushFlags::MAPMODE);
            EnableMapMode();
            MapMode aMapMode = GetMapMode();
            aMapMode.SetOrigin(Point(0, 0));
            SetMapMode(aMapMode);
            aPosPixel = LogicToPixel(rTEvt.GetMouseEvent().GetPosPixel());
            Pop();
        }
        const Point rMousePos = (GetMapMode().GetMapUnit() != MAP_TWIP ? rTEvt.GetMouseEvent().GetPosPixel() : aPosPixel);

        // Dragging is treated in a special way
        if ( meScrollType == ScrollType::Drag )
            ImplDragThumb( rMousePos );
        else
            ImplDoMouseAction( rMousePos, rTEvt.IsTrackingRepeat() );

        // If ScrollBar values are translated in a way that there's
        // nothing left to track, we cancel here
        if ( !IsVisible() || (mnVisibleSize >= (mnMaxRange-mnMinRange)) )
            EndTracking();
    }
}

void ScrollBar::KeyInput( const KeyEvent& rKEvt )
{
    if ( !rKEvt.GetKeyCode().GetModifier() )
    {
        switch ( rKEvt.GetKeyCode().GetCode() )
        {
            case KEY_HOME:
                DoScroll( 0 );
                break;

            case KEY_END:
                DoScroll( GetRangeMax() );
                break;

            case KEY_LEFT:
            case KEY_UP:
                DoScrollAction( ScrollType::LineUp );
                break;

            case KEY_RIGHT:
            case KEY_DOWN:
                DoScrollAction( ScrollType::LineDown );
                break;

            case KEY_PAGEUP:
                DoScrollAction( ScrollType::PageUp );
                break;

            case KEY_PAGEDOWN:
                DoScrollAction( ScrollType::PageDown );
                break;

            default:
                Control::KeyInput( rKEvt );
                break;
        }
    }
    else
        Control::KeyInput( rKEvt );
}

void ScrollBar::ApplySettings(vcl::RenderContext& rRenderContext)
{
    rRenderContext.SetBackground();
}

void ScrollBar::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    ImplDraw(rRenderContext);
}

void ScrollBar::Resize()
{
    Control::Resize();
    mbCalcSize = true;
    if ( IsReallyVisible() )
        ImplCalc( false );
    Invalidate();
}

IMPL_LINK_NOARG_TYPED(ScrollBar, ImplAutoTimerHdl, Timer *, void)
{
    if( mpData && mpData->mbHide )
        return;
    ImplInvert();
}

void ScrollBar::ImplInvert()
{
    Rectangle aRect( maThumbRect );
    if( aRect.getWidth() > 4 )
    {
        aRect.Left() += 2;
        aRect.Right() -= 2;
    }
    if( aRect.getHeight() > 4 )
    {
        aRect.Top() += 2;
        aRect.Bottom() -= 2;
    }

    Invert( aRect );
}

void ScrollBar::GetFocus()
{
    if( !mpData )
    {
        mpData = new ImplScrollBarData;
        mpData->maTimer.SetTimeoutHdl( LINK( this, ScrollBar, ImplAutoTimerHdl ) );
        mpData->mbHide = false;
    }
    ImplInvert(); // react immediately
    mpData->maTimer.SetTimeout( GetSettings().GetStyleSettings().GetCursorBlinkTime() );
    mpData->maTimer.Start();
    Control::GetFocus();
}

void ScrollBar::LoseFocus()
{
    if( mpData )
        mpData->maTimer.Stop();
    Invalidate();

    Control::LoseFocus();
}

void ScrollBar::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( nType == StateChangedType::InitShow )
        ImplCalc( false );
    else if ( nType == StateChangedType::Data )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            ImplCalc();
    }
    else if ( nType == StateChangedType::UpdateMode )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            ImplCalc( false );
            Invalidate();
        }
    }
    else if ( nType == StateChangedType::Enable )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == StateChangedType::Style )
    {
        ImplInitStyle( GetStyle() );
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            if ( (GetPrevStyle() & SCRBAR_VIEW_STYLE) !=
                 (GetStyle() & SCRBAR_VIEW_STYLE) )
            {
                mbCalcSize = true;
                ImplCalc( false );
                Invalidate();
            }
        }
    }
}

void ScrollBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        mbCalcSize = true;
        ImplCalc( false );
        Invalidate();
    }
}

Rectangle* ScrollBar::ImplFindPartRect( const Point& rPt )
{
    bool    bHorizontal = ( GetStyle() & WB_HORZ ) != 0;
    bool    bIsInside = false;

    Point aPoint( 0, 0 );
    Rectangle aControlRegion( aPoint, GetOutputSizePixel() );

    if( HitTestNativeScrollbar( bHorizontal? (IsRTLEnabled()? ControlPart::ButtonRight: ControlPart::ButtonLeft): ControlPart::ButtonUp,
                aControlRegion, rPt, bIsInside )?
            bIsInside:
            maBtn1Rect.IsInside( rPt ) )
        return &maBtn1Rect;
    else if( HitTestNativeScrollbar( bHorizontal? (IsRTLEnabled()? ControlPart::ButtonLeft: ControlPart::ButtonRight): ControlPart::ButtonDown,
                aControlRegion, rPt, bIsInside )?
            bIsInside:
            maBtn2Rect.IsInside( rPt ) )
        return &maBtn2Rect;
    // HitTestNativeScrollbar, see remark at top of file
    else if( HitTestNativeScrollbar( bHorizontal ? ControlPart::TrackHorzLeft : ControlPart::TrackVertUpper,
                maPage1Rect, rPt, bIsInside)?
            bIsInside:
            maPage1Rect.IsInside( rPt ) )
        return &maPage1Rect;
    // HitTestNativeScrollbar, see remark at top of file
    else if( HitTestNativeScrollbar( bHorizontal ? ControlPart::TrackHorzRight : ControlPart::TrackVertLower,
                maPage2Rect, rPt, bIsInside)?
            bIsInside:
            maPage2Rect.IsInside( rPt ) )
        return &maPage2Rect;
    // HitTestNativeScrollbar, see remark at top of file
    else if( HitTestNativeScrollbar( bHorizontal ? ControlPart::ThumbHorz : ControlPart::ThumbVert,
                maThumbRect, rPt, bIsInside)?
             bIsInside:
             maThumbRect.IsInside( rPt ) )
        return &maThumbRect;
    else
        return nullptr;
}

bool ScrollBar::PreNotify( NotifyEvent& rNEvt )
{
    const MouseEvent* pMouseEvt = nullptr;

    if( (rNEvt.GetType() == MouseNotifyEvent::MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != nullptr )
    {
        if( !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            // Trigger a redraw if mouse over state has changed
            if( IsNativeControlSupported(ControlType::Scrollbar, ControlPart::Entire) )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                Rectangle* pLastRect = ImplFindPartRect( GetLastPointerPosPixel() );
                if( pRect != pLastRect || pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow() )
                {
                    vcl::Region aRgn( GetActiveClipRegion() );
                    vcl::Region aClipRegion;

                    if ( pRect )
                        aClipRegion.Union( *pRect );
                    if ( pLastRect )
                        aClipRegion.Union( *pLastRect );

                    // Support for 3-button scroll bars
                    bool bHas3Buttons = IsNativeControlSupported( ControlType::Scrollbar, ControlPart::HasThreeButtons );
                    if ( bHas3Buttons && ( pRect == &maBtn1Rect || pLastRect == &maBtn1Rect ) )
                    {
                        aClipRegion.Union( maBtn2Rect );
                    }

                    SetClipRegion( aClipRegion );
                    Invalidate(aClipRegion.GetBoundRect());

                    SetClipRegion( aRgn );
                }
            }
        }
    }

    return Control::PreNotify(rNEvt);
}

void ScrollBar::Scroll()
{
    ImplCallEventListenersAndHandler( VCLEVENT_SCROLLBAR_SCROLL, [this] () { maScrollHdl.Call(this); } );
}

void ScrollBar::EndScroll()
{
    ImplCallEventListenersAndHandler( VCLEVENT_SCROLLBAR_ENDSCROLL, [this] () { maEndScrollHdl.Call(this); } );
}

long ScrollBar::DoScroll( long nNewPos )
{
    if ( meScrollType != ScrollType::DontKnow )
        return 0;

    SAL_INFO("vcl.scrollbar", "DoScroll(" << nNewPos << ")");
    meScrollType = ScrollType::Drag;
    long nDelta = ImplScroll( nNewPos, true );
    meScrollType = ScrollType::DontKnow;
    return nDelta;
}

long ScrollBar::DoScrollAction( ScrollType eScrollType )
{
    if ( (meScrollType != ScrollType::DontKnow) ||
         (eScrollType == ScrollType::DontKnow) ||
         (eScrollType == ScrollType::Drag) )
        return 0;

    meScrollType = eScrollType;
    long nDelta = ImplDoAction( true );
    meScrollType = ScrollType::DontKnow;
    return nDelta;
}

void ScrollBar::SetRangeMin( long nNewRange )
{
    SetRange( Range( nNewRange, GetRangeMax() ) );
}

void ScrollBar::SetRangeMax( long nNewRange )
{
    SetRange( Range( GetRangeMin(), nNewRange ) );
}

void ScrollBar::SetRange( const Range& rRange )
{
    // Adapt Range
    Range aRange = rRange;
    aRange.Justify();
    long nNewMinRange = aRange.Min();
    long nNewMaxRange = aRange.Max();

    // If Range differs, set a new one
    if ( (mnMinRange != nNewMinRange) ||
         (mnMaxRange != nNewMaxRange) )
    {
        mnMinRange = nNewMinRange;
        mnMaxRange = nNewMaxRange;

        // Adapt Thumb
        if ( mnThumbPos > mnMaxRange-mnVisibleSize )
            mnThumbPos = mnMaxRange-mnVisibleSize;
        if ( mnThumbPos < mnMinRange )
            mnThumbPos = mnMinRange;

        CompatStateChanged( StateChangedType::Data );
    }
}

void ScrollBar::SetThumbPos( long nNewThumbPos )
{
    if ( nNewThumbPos > mnMaxRange-mnVisibleSize )
        nNewThumbPos = mnMaxRange-mnVisibleSize;
    if ( nNewThumbPos < mnMinRange )
        nNewThumbPos = mnMinRange;

    if ( mnThumbPos != nNewThumbPos )
    {
        mnThumbPos = nNewThumbPos;
        CompatStateChanged( StateChangedType::Data );
    }
}

void ScrollBar::SetVisibleSize( long nNewSize )
{
    if ( mnVisibleSize != nNewSize )
    {
        mnVisibleSize = nNewSize;

        // Adapt Thumb
        if ( mnThumbPos > mnMaxRange-mnVisibleSize )
            mnThumbPos = mnMaxRange-mnVisibleSize;
        if ( mnThumbPos < mnMinRange )
            mnThumbPos = mnMinRange;
        CompatStateChanged( StateChangedType::Data );
    }
}

Size ScrollBar::GetOptimalSize() const
{
    if (mbCalcSize)
        const_cast<ScrollBar*>(this)->ImplCalc(false);

    Size aRet = getCurrentCalcSize();

    const long nMinThumbSize = GetSettings().GetStyleSettings().GetMinThumbSize();

    if (GetStyle() & WB_HORZ)
    {
        aRet.Width() = maBtn1Rect.GetWidth() + nMinThumbSize + maBtn2Rect.GetWidth();
    }
    else
    {
        aRet.Height() = maBtn1Rect.GetHeight() + nMinThumbSize + maBtn2Rect.GetHeight();
    }

    return aRet;
}

Size ScrollBar::getCurrentCalcSize() const
{
    Rectangle aCtrlRegion;
    aCtrlRegion.Union(maBtn1Rect);
    aCtrlRegion.Union(maBtn2Rect);
    aCtrlRegion.Union(maPage1Rect);
    aCtrlRegion.Union(maPage2Rect);
    aCtrlRegion.Union(maThumbRect);
    return aCtrlRegion.GetSize();
}

void ScrollBarBox::ImplInit(vcl::Window* pParent, WinBits nStyle)
{
    Window::ImplInit( pParent, nStyle, nullptr );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    long nScrollSize = rStyleSettings.GetScrollBarSize();
    SetSizePixel(Size(nScrollSize, nScrollSize));
}

ScrollBarBox::ScrollBarBox( vcl::Window* pParent, WinBits nStyle ) :
    Window( WINDOW_SCROLLBARBOX )
{
    ImplInit( pParent, nStyle );
}

void ScrollBarBox::ApplySettings(vcl::RenderContext& rRenderContext)
{
    if (rRenderContext.IsBackground())
    {
        Color aColor = rRenderContext.GetSettings().GetStyleSettings().GetFaceColor();
        ApplyControlBackground(rRenderContext, aColor);
    }
}

void ScrollBarBox::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if (nType == StateChangedType::ControlBackground)
    {
        Invalidate();
    }
}

void ScrollBarBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
        (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        Invalidate();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
