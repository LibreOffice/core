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

#include <accessibility/vclxaccessiblescrollbar.hxx>

#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/timer.hxx>
#include <vcl/settings.hxx>
#include <vcl/toolkit/scrbar.hxx>
#include <vcl/vclevent.hxx>

#include <sal/log.hxx>
#include <scrollbarvalue.hxx>

/*  #i77549#
    HACK: for scrollbars in case of thumb rect, page up and page down rect we
    abuse the HitTestNativeScrollbar interface. All theming engines but macOS
    are actually able to draw the thumb according to our internal representation.
    However macOS draws a little outside. The canonical way would be to enhance the
    HitTestNativeScrollbar passing a ScrollbarValue additionally so all necessary
    information is available in the call.
    .
    However since there is only this one small exception we will deviate a little and
    instead pass the respective rect as control region to allow for a small correction.

    So all places using HitTestNativeScrollbar on ControlPart::ThumbHorz, ControlPart::ThumbVert,
    ControlPart::TrackHorzLeft, ControlPart::TrackHorzRight, ControlPart::TrackVertUpper, ControlPart::TrackVertLower
    do not use the control rectangle as region but the actual part rectangle, making
    only small deviations feasible.
*/

#include "thumbpos.hxx"

#define SCRBAR_DRAW_BTN1            (sal_uInt16(0x0001))
#define SCRBAR_DRAW_BTN2            (sal_uInt16(0x0002))
#define SCRBAR_DRAW_PAGE1           (sal_uInt16(0x0004))
#define SCRBAR_DRAW_PAGE2           (sal_uInt16(0x0008))
#define SCRBAR_DRAW_THUMB           (sal_uInt16(0x0010))
#define SCRBAR_DRAW_BACKGROUND      (sal_uInt16(0x0020))

#define SCRBAR_STATE_BTN1_DOWN      (sal_uInt16(0x0001))
#define SCRBAR_STATE_BTN1_DISABLE   (sal_uInt16(0x0002))
#define SCRBAR_STATE_BTN2_DOWN      (sal_uInt16(0x0004))
#define SCRBAR_STATE_BTN2_DISABLE   (sal_uInt16(0x0008))
#define SCRBAR_STATE_PAGE1_DOWN     (sal_uInt16(0x0010))
#define SCRBAR_STATE_PAGE2_DOWN     (sal_uInt16(0x0020))
#define SCRBAR_STATE_THUMB_DOWN     (sal_uInt16(0x0040))

#define SCRBAR_VIEW_STYLE           (WB_3DLOOK | WB_HORZ | WB_VERT)

struct ImplScrollBarData
{
    AutoTimer       maTimer { "vcl::ScrollBar mpData->maTimer" };
    bool            mbHide;
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
    mnStateFlags        = 0;
    meScrollType        = ScrollType::DontKnow;
    mbCalcSize          = true;
    mbFullDrag          = false;
    mbSwapArrows        = false;

    ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, nullptr );

    tools::Long nScrollSize = GetSettings().GetStyleSettings().GetScrollBarSize();
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
    Control( WindowType::SCROLLBAR )
{
    ImplInit( pParent, nStyle );
}

ScrollBar::~ScrollBar()
{
    disposeOnce();
}

void ScrollBar::dispose()
{
    mpData.reset();
    Control::dispose();
}

void ScrollBar::ImplUpdateRects( bool bUpdate )
{
    mnStateFlags  &= ~SCRBAR_STATE_BTN1_DISABLE;
    mnStateFlags  &= ~SCRBAR_STATE_BTN2_DISABLE;

    if ( mnThumbPixRange )
    {
        if ( GetStyle() & WB_HORZ )
        {
            maThumbRect.SetLeft( maTrackRect.Left()+mnThumbPixPos );
            maThumbRect.SetRight( maThumbRect.Left()+mnThumbPixSize-1 );
            if ( !mnThumbPixPos )
                maPage1Rect.SetWidthEmpty();
            else
                maPage1Rect.SetRight( maThumbRect.Left()-1 );
            if ( mnThumbPixPos >= (mnThumbPixRange-mnThumbPixSize) )
                maPage2Rect.SetWidthEmpty();
            else
            {
                maPage2Rect.SetLeft( maThumbRect.Right()+1 );
                maPage2Rect.SetRight( maTrackRect.Right() );
            }
        }
        else
        {
            maThumbRect.SetTop( maTrackRect.Top()+mnThumbPixPos );
            maThumbRect.SetBottom( maThumbRect.Top()+mnThumbPixSize-1 );
            if ( !mnThumbPixPos )
                maPage1Rect.SetHeightEmpty();
            else
                maPage1Rect.SetBottom( maThumbRect.Top()-1 );
            if ( mnThumbPixPos >= (mnThumbPixRange-mnThumbPixSize) )
                maPage2Rect.SetHeightEmpty();
            else
            {
                maPage2Rect.SetTop( maThumbRect.Bottom()+1 );
                maPage2Rect.SetBottom( maTrackRect.Bottom() );
            }
        }
    }
    else
    {
        if ( GetStyle() & WB_HORZ )
        {
            const tools::Long nSpace = maTrackRect.Right() - maTrackRect.Left();
            if ( nSpace > 0 )
            {
                maPage1Rect.SetLeft( maTrackRect.Left() );
                maPage1Rect.SetRight( maTrackRect.Left() + (nSpace/2) );
                maPage2Rect.SetLeft( maPage1Rect.Right() + 1 );
                maPage2Rect.SetRight( maTrackRect.Right() );
            }
        }
        else
        {
            const tools::Long nSpace = maTrackRect.Bottom() - maTrackRect.Top();
            if ( nSpace > 0 )
            {
                maPage1Rect.SetTop( maTrackRect.Top() );
                maPage1Rect.SetBottom( maTrackRect.Top() + (nSpace/2) );
                maPage2Rect.SetTop( maPage1Rect.Bottom() + 1 );
                maPage2Rect.SetBottom( maTrackRect.Bottom() );
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

tools::Long ScrollBar::ImplCalcThumbPos( tools::Long nPixPos ) const
{
    // Calculate position
    tools::Long nCalcThumbPos;
    nCalcThumbPos = ImplMulDiv( nPixPos, mnMaxRange-mnVisibleSize-mnMinRange,
                                mnThumbPixRange-mnThumbPixSize );
    nCalcThumbPos += mnMinRange;
    return nCalcThumbPos;
}

tools::Long ScrollBar::ImplCalcThumbPosPix( tools::Long nPos ) const
{
    tools::Long nCalcThumbPos;

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
    const tools::Long nMinThumbSize = GetSettings().GetStyleSettings().GetMinThumbSize();

    if ( mbCalcSize )
    {
        Size aOldSize = getCurrentCalcSize();

        tools::Rectangle aBtn1Region, aBtn2Region, aTrackRegion, aBoundingRegion;
        const bool bSwapArrows = mbSwapArrows || IsRTLEnabled();

        // reset rectangles to empty *and* (0,0) position
        maThumbRect = tools::Rectangle();
        maPage1Rect = tools::Rectangle();
        maPage2Rect = tools::Rectangle();

        const tools::Rectangle aScrollbarRegion = GetScrollbarRegion();

        if ( GetStyle() & WB_HORZ )
        {
            if ( GetNativeControlRegion( ControlType::Scrollbar, bSwapArrows? ControlPart::ButtonRight: ControlPart::ButtonLeft,
                        aScrollbarRegion, ControlState::NONE, ImplControlValue(), aBoundingRegion, aBtn1Region ) &&
                 GetNativeControlRegion( ControlType::Scrollbar, bSwapArrows? ControlPart::ButtonLeft: ControlPart::ButtonRight,
                        aScrollbarRegion, ControlState::NONE, ImplControlValue(), aBoundingRegion, aBtn2Region ) )
            {
                maBtn1Rect = aBtn1Region;
                maBtn2Rect = aBtn2Region;
            }
            else
            {
                Size aBtnSize( aSize.Height(), aSize.Height() );
                maBtn2Rect.SetTop( maBtn1Rect.Top() );
                maBtn2Rect.SetLeft( aSize.Width()-aSize.Height() );
                maBtn1Rect.SetSize( aBtnSize );
                maBtn2Rect.SetSize( aBtnSize );
            }

            if (GetNativeControlRegion( ControlType::Scrollbar, ControlPart::TrackHorzArea,
                     aScrollbarRegion, ControlState::NONE, ImplControlValue(), aBoundingRegion, aTrackRegion ) )
                maTrackRect = aTrackRegion;
            else
                maTrackRect = tools::Rectangle::Normalize( maBtn1Rect.TopRight(), maBtn2Rect.BottomLeft() );

            // Check if available space is big enough for thumb ( min thumb size = ScrBar width/height )
            mnThumbPixRange = maTrackRect.Right() - maTrackRect.Left();
            if( mnThumbPixRange > 0 )
            {
                maPage1Rect.SetLeft( maTrackRect.Left() );
                maPage1Rect.SetBottom( maTrackRect.Bottom() );
                maPage2Rect.SetBottom (maTrackRect.Bottom() );
                maThumbRect.SetBottom( maTrackRect.Bottom() );
            }
            else
                mnThumbPixRange = 0;
        }
        else // WB_VERT
        {
            if ( GetNativeControlRegion( ControlType::Scrollbar, ControlPart::ButtonUp,
                        aScrollbarRegion, ControlState::NONE, ImplControlValue(), aBoundingRegion, aBtn1Region ) &&
                 GetNativeControlRegion( ControlType::Scrollbar, ControlPart::ButtonDown,
                        aScrollbarRegion, ControlState::NONE, ImplControlValue(), aBoundingRegion, aBtn2Region ) )
            {
                maBtn1Rect = aBtn1Region;
                maBtn2Rect = aBtn2Region;
            }
            else
            {
                const Size aBtnSize( aSize.Width(), aSize.Width() );
                maBtn2Rect.SetLeft( maBtn1Rect.Left() );
                maBtn2Rect.SetTop( aSize.Height()-aSize.Width() );
                maBtn1Rect.SetSize( aBtnSize );
                maBtn2Rect.SetSize( aBtnSize );
            }

            if ( GetNativeControlRegion( ControlType::Scrollbar, ControlPart::TrackVertArea,
                     aScrollbarRegion, ControlState::NONE, ImplControlValue(), aBoundingRegion, aTrackRegion ) )
                maTrackRect = aTrackRegion;
            else
                maTrackRect = tools::Rectangle::Normalize( maBtn1Rect.BottomLeft()+Point(0,1), maBtn2Rect.TopRight() );

            // Check if available space is big enough for thumb
            mnThumbPixRange = maTrackRect.Bottom() - maTrackRect.Top();
            if( mnThumbPixRange > 0 )
            {
                maPage1Rect.SetTop( maTrackRect.Top() );
                maPage1Rect.SetRight( maTrackRect.Right() );
                maPage2Rect.SetRight( maTrackRect.Right() );
                maThumbRect.SetRight( maTrackRect.Right() );
            }
            else
                mnThumbPixRange = 0;
        }

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

    // If we're ought to output again and we have been triggered
    // a Paint event via an Action, we don't output directly,
    // but invalidate everything
    if ( bUpdate && HasPaintEvent() )
    {
        Invalidate();
        bUpdate = false;
    }
    ImplUpdateRects( bUpdate );
}

void ScrollBar::Draw( OutputDevice* pDev, const Point& rPos, SystemTextColorFlags nFlags )
{
    Point aPos  = pDev->LogicToPixel( rPos );

    pDev->Push();
    pDev->SetMapMode();
    if ( !(nFlags & SystemTextColorFlags::Mono) )
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
    maTrackRect+=aPos;
    maPage1Rect+=aPos;
    maPage2Rect+=aPos;

    ImplDraw(*pDev);
    pDev->Pop();

    mbCalcSize = true;
}

bool ScrollBar::ImplDrawNative(vcl::RenderContext& rRenderContext, sal_uInt16 nSystemTextColorFlags)
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

        if (IsMouseOver())
        {
            tools::Rectangle* pRect = ImplFindPartRect(GetPointerPosPixel());
            if (pRect)
            {
                if (pRect == &maThumbRect)
                    scrValue.mnThumbState |= ControlState::ROLLOVER;
                else if (pRect == &maBtn1Rect)
                    scrValue.mnButton1State |= ControlState::ROLLOVER;
                else if (pRect == &maBtn2Rect)
                    scrValue.mnButton2State |= ControlState::ROLLOVER;
            }
        }

        tools::Rectangle aCtrlRegion;
        aCtrlRegion.Union(maBtn1Rect);
        aCtrlRegion.Union(maBtn2Rect);
        aCtrlRegion.Union(maPage1Rect);
        aCtrlRegion.Union(maPage2Rect);
        aCtrlRegion.Union(maThumbRect);

        tools::Rectangle aRequestedRegion(Point(0,0), GetOutputSizePixel());
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
        if ((nSystemTextColorFlags & SCRBAR_DRAW_PAGE1) || (nSystemTextColorFlags & SCRBAR_DRAW_PAGE2))
        {
            ControlPart part1 = bHorz ? ControlPart::TrackHorzLeft : ControlPart::TrackVertUpper;
            ControlPart part2 = bHorz ? ControlPart::TrackHorzRight : ControlPart::TrackVertLower;
            tools::Rectangle aCtrlRegion1(maPage1Rect);
            tools::Rectangle aCtrlRegion2(maPage2Rect);
            ControlState nState1 = (IsEnabled() ? ControlState::ENABLED : ControlState::NONE)
                                 | (HasFocus() ? ControlState::FOCUSED : ControlState::NONE);
            ControlState nState2 = nState1;

            nState1 |= ((mnStateFlags & SCRBAR_STATE_PAGE1_DOWN) ? ControlState::PRESSED : ControlState::NONE);
            nState2 |= ((mnStateFlags & SCRBAR_STATE_PAGE2_DOWN) ? ControlState::PRESSED : ControlState::NONE);

            if (IsMouseOver())
            {
                tools::Rectangle* pRect = ImplFindPartRect(GetPointerPosPixel());
                if (pRect)
                {
                    if (pRect == &maPage1Rect)
                        nState1 |= ControlState::ROLLOVER;
                    else if (pRect == &maPage2Rect)
                        nState2 |= ControlState::ROLLOVER;
                }
            }

            if (nSystemTextColorFlags & SCRBAR_DRAW_PAGE1)
                bNativeOK = rRenderContext.DrawNativeControl(ControlType::Scrollbar, part1, aCtrlRegion1, nState1, scrValue, OUString());

            if (nSystemTextColorFlags & SCRBAR_DRAW_PAGE2)
                bNativeOK = rRenderContext.DrawNativeControl(ControlType::Scrollbar, part2, aCtrlRegion2, nState2, scrValue, OUString());
        }
        if ((nSystemTextColorFlags & SCRBAR_DRAW_BTN1) || (nSystemTextColorFlags & SCRBAR_DRAW_BTN2))
        {
            ControlPart part1 = bHorz ? ControlPart::ButtonLeft : ControlPart::ButtonUp;
            ControlPart part2 = bHorz ? ControlPart::ButtonRight : ControlPart::ButtonDown;
            tools::Rectangle aCtrlRegion1(maBtn1Rect);
            tools::Rectangle aCtrlRegion2(maBtn2Rect);
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
                tools::Rectangle* pRect = ImplFindPartRect(GetPointerPosPixel());
                if (pRect)
                {
                    if (pRect == &maBtn1Rect)
                        nState1 |= ControlState::ROLLOVER;
                    else if (pRect == &maBtn2Rect)
                        nState2 |= ControlState::ROLLOVER;
                }
            }

            if (nSystemTextColorFlags & SCRBAR_DRAW_BTN1)
                bNativeOK = rRenderContext.DrawNativeControl(ControlType::Scrollbar, part1, aCtrlRegion1, nState1, scrValue, OUString());

            if (nSystemTextColorFlags & SCRBAR_DRAW_BTN2)
                bNativeOK = rRenderContext.DrawNativeControl(ControlType::Scrollbar, part2, aCtrlRegion2, nState2, scrValue, OUString());
        }
        if ((nSystemTextColorFlags & SCRBAR_DRAW_THUMB) && !maThumbRect.IsEmpty())
        {
            ControlState nState = IsEnabled() ? ControlState::ENABLED : ControlState::NONE;
            tools::Rectangle aCtrlRegion(maThumbRect);

            if (mnStateFlags & SCRBAR_STATE_THUMB_DOWN)
                nState |= ControlState::PRESSED;

            if (HasFocus())
                nState |= ControlState::FOCUSED;

            if (IsMouseOver())
            {
                tools::Rectangle* pRect = ImplFindPartRect(GetPointerPosPixel());
                if (pRect && pRect == &maThumbRect)
                    nState |= ControlState::ROLLOVER;
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
    tools::Rectangle aTempRect;
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

tools::Long ScrollBar::ImplScroll( tools::Long nNewPos, bool bCallEndScroll )
{
    tools::Long nOldPos = mnThumbPos;
    SetThumbPos( nNewPos );
    tools::Long nDelta = mnThumbPos-nOldPos;
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

tools::Long ScrollBar::ImplDoAction( bool bCallEndScroll )
{
    tools::Long nDelta = 0;

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

    switch ( meScrollType )
    {
        case ScrollType::LineUp:
            if (ImplHitTestBtn1(rMousePos))
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_BTN1_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_BTN1_DOWN;
            break;

        case ScrollType::LineDown:
            if (ImplHitTestBtn2(rMousePos))
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_BTN2_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_BTN2_DOWN;
            break;

        case ScrollType::PageUp:
            // HitTestNativeScrollbar, see remark at top of file
            if (ImplHitTestPageUp(rMousePos))
            {
                bAction = bCallAction;
                mnStateFlags |= SCRBAR_STATE_PAGE1_DOWN;
            }
            else
                mnStateFlags &= ~SCRBAR_STATE_PAGE1_DOWN;
            break;

        case ScrollType::PageDown:
            // HitTestNativeScrollbar, see remark at top of file
            if (ImplHitTestPageDown(rMousePos))
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

bool ScrollBar::ImplHitTestBtn1(const Point& rPt) const
{
    ControlPart ePart;

    if (IsHorizontal())
    {
        if (IsRTLEnabled())
            ePart = ControlPart::ButtonRight;
        else
            ePart = ControlPart::ButtonLeft;
    }
    else
    {
        ePart = ControlPart::ButtonUp;
    }

    bool bIsInside = false;

    if (GetOutDev()->HitTestNativeScrollbar(ePart, GetScrollbarRegion(), rPt, bIsInside))
        return bIsInside;

    return maBtn1Rect.Contains(rPt);
}

bool ScrollBar::ImplHitTestBtn2(const Point& rPt) const
{
    ControlPart ePart;

    if (IsHorizontal())
    {
        if (IsRTLEnabled())
            ePart = ControlPart::ButtonLeft;
        else
            ePart = ControlPart::ButtonRight;
    }
    else
    {
        ePart = ControlPart::ButtonDown;
    }

    bool bIsInside = false;

    if (GetOutDev()->HitTestNativeScrollbar(ePart, GetScrollbarRegion(), rPt, bIsInside))
        return bIsInside;

    return maBtn2Rect.Contains(rPt);
}

bool ScrollBar::ImplHitTestPageUp(const Point& rPt) const
{
    ControlPart ePart;

    if (IsHorizontal())
        ePart = ControlPart::TrackHorzLeft;
    else
        ePart = ControlPart::TrackVertUpper;

    bool bIsInside = false;

    if (GetOutDev()->HitTestNativeScrollbar(ePart, maPage1Rect, rPt, bIsInside))
        return bIsInside;

    return maPage1Rect.Contains(rPt);
}

bool ScrollBar::ImplHitTestPageDown(const Point& rPt) const
{
    ControlPart ePart;

    if (IsHorizontal())
        ePart = ControlPart::TrackHorzRight;
    else
        ePart = ControlPart::TrackVertLower;

    bool bIsInside = false;

    if (GetOutDev()->HitTestNativeScrollbar(ePart, maPage2Rect, rPt, bIsInside))
        return bIsInside;

    return maPage2Rect.Contains(rPt);
}

bool ScrollBar::ImplHitTestThumb(const Point& rPt) const
{
    ControlPart ePart;

    if (IsHorizontal())
        ePart = ControlPart::ThumbHorz;
    else
        ePart = ControlPart::ThumbVert;

    bool bIsInside = false;

    if (GetOutDev()->HitTestNativeScrollbar(ePart, maThumbRect, rPt, bIsInside))
        return bIsInside;

    return maThumbRect.Contains(rPt);
}

void ScrollBar::ImplDragThumb( const Point& rMousePos )
{
    tools::Long nMovePix;
    if ( GetStyle() & WB_HORZ )
        nMovePix = rMousePos.X()-(maThumbRect.Left()+mnMouseOff);
    else
        nMovePix = rMousePos.Y()-(maThumbRect.Top()+mnMouseOff);

    // Move thumb if necessary
    if ( !nMovePix )
        return;

    mnThumbPixPos += nMovePix;
    if ( mnThumbPixPos < 0 )
        mnThumbPixPos = 0;
    if ( mnThumbPixPos > (mnThumbPixRange-mnThumbPixSize) )
        mnThumbPixPos = mnThumbPixRange-mnThumbPixSize;
    tools::Long nOldPos = mnThumbPos;
    mnThumbPos = ImplCalcThumbPos( mnThumbPixPos );
    ImplUpdateRects();
    if ( !(mbFullDrag && (nOldPos != mnThumbPos)) )
        return;

    // When dragging in windows the repaint request gets starved so dragging
    // the scrollbar feels slower than it actually is. Let's force an immediate
    // repaint of the scrollbar.
    if (SupportsDoubleBuffering())
    {
        Invalidate();
        PaintImmediately();
    }
    else
        ImplDraw(*GetOutDev());

    mnDelta = mnThumbPos-nOldPos;
    Scroll();
    mnDelta = 0;
}

bool ScrollBar::IsHorizontal() const
{
    return (GetStyle() & WB_HORZ) != 0;
}

tools::Rectangle ScrollBar::GetScrollbarRegion() const
{
    return tools::Rectangle(Point(0, 0), GetOutputSizePixel());
}

rtl::Reference<comphelper::OAccessible> ScrollBar::CreateAccessible()
{
    return new VCLXAccessibleScrollBar(this);
}

void ScrollBar::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bPrimaryWarps = GetSettings().GetStyleSettings().GetPrimaryButtonWarpsSlider();
#ifdef MACOSX
    // tdf#147067 Jump to clicked spot if left mouse click with Option key
    // Most macOS applications such as TextEdit and Safari jump to the
    // clicked spot if the Option key is pressed during a left mouse click.
    if (!bPrimaryWarps && rMEvt.IsLeft() && rMEvt.GetModifier() == KEY_MOD2)
        bPrimaryWarps = true;
#endif
    bool bWarp = bPrimaryWarps ? rMEvt.IsLeft() : rMEvt.IsMiddle();
    bool bPrimaryWarping = bWarp && rMEvt.IsLeft();
    bool bPage = bPrimaryWarps ? rMEvt.IsRight() : rMEvt.IsLeft();

    if (!rMEvt.IsLeft() && !rMEvt.IsMiddle() && !rMEvt.IsRight())
        return;

    Point aPosPixel;
    if (!IsMapModeEnabled() && GetMapMode().GetMapUnit() == MapUnit::MapTwip)
    {
        // rMEvt coordinates are in twips.
        GetOutDev()->Push(vcl::PushFlags::MAPMODE);
        EnableMapMode();
        MapMode aMapMode = GetMapMode();
        aMapMode.SetOrigin(Point(0, 0));
        SetMapMode(aMapMode);
        aPosPixel = LogicToPixel(rMEvt.GetPosPixel());
        GetOutDev()->Pop();
    }
    const Point&        rMousePos = (GetMapMode().GetMapUnit() != MapUnit::MapTwip ? rMEvt.GetPosPixel() : aPosPixel);
    StartTrackingFlags  nTrackFlags = StartTrackingFlags::NONE;
    bool                bIsInside = false;
    bool                bDragToMouse = false;

    if (ImplHitTestBtn1(rMousePos))
    {
        if (rMEvt.IsLeft() && !(mnStateFlags & SCRBAR_STATE_BTN1_DISABLE) )
        {
            nTrackFlags     = StartTrackingFlags::ButtonRepeat;
            meScrollType    = ScrollType::LineUp;
        }
    }
    else if (ImplHitTestBtn2(rMousePos))
    {
        if (rMEvt.IsLeft() && !(mnStateFlags & SCRBAR_STATE_BTN2_DISABLE) )
        {
            nTrackFlags     = StartTrackingFlags::ButtonRepeat;
            meScrollType    = ScrollType::LineDown;
        }
    }
    else
    {
        bool bThumbHit = ImplHitTestThumb(rMousePos);
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
        else if(bPage && (!GetOutDev()->HitTestNativeScrollbar(IsHorizontal() ? ControlPart::TrackHorzArea : ControlPart::TrackVertArea,
                                       GetScrollbarRegion(), rMousePos, bIsInside ) ||
            bIsInside) )
        {
            nTrackFlags = StartTrackingFlags::ButtonRepeat;

            // HitTestNativeScrollbar, see remark at top of file
            if (ImplHitTestPageUp(rMousePos))
            {
                meScrollType    = ScrollType::PageUp;
            }
            else
            {
                meScrollType    = ScrollType::PageDown;
            }
        }
    }

    // Should we start Tracking?
    if ( meScrollType == ScrollType::DontKnow )
        return;

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

        // Restore the old ThumbPosition when canceled
        if ( rTEvt.IsTrackingCanceled() )
        {
            tools::Long nOldPos = mnThumbPos;
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
        if (!IsMapModeEnabled() && GetMapMode().GetMapUnit() == MapUnit::MapTwip)
        {
            // rTEvt coordinates are in twips.
            GetOutDev()->Push(vcl::PushFlags::MAPMODE);
            EnableMapMode();
            MapMode aMapMode = GetMapMode();
            aMapMode.SetOrigin(Point(0, 0));
            SetMapMode(aMapMode);
            aPosPixel = LogicToPixel(rTEvt.GetMouseEvent().GetPosPixel());
            GetOutDev()->Pop();
        }
        const Point rMousePos = (GetMapMode().GetMapUnit() != MapUnit::MapTwip ? rTEvt.GetMouseEvent().GetPosPixel() : aPosPixel);

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

void ScrollBar::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    ImplDraw(rRenderContext);
}

void ScrollBar::Move()
{
    Control::Move();
    mbCalcSize = true;
    if (IsReallyVisible())
        ImplCalc(false);
    Invalidate();
}

void ScrollBar::Resize()
{
    Control::Resize();
    mbCalcSize = true;
    if ( IsReallyVisible() )
        ImplCalc( false );
    Invalidate();
}

IMPL_LINK_NOARG(ScrollBar, ImplAutoTimerHdl, Timer *, void)
{
    if( mpData && mpData->mbHide )
        return;
    ImplInvert();
}

void ScrollBar::ImplInvert()
{
    tools::Rectangle aRect( maThumbRect );
    if( aRect.GetWidth() > 5 )
    {
        aRect.AdjustLeft(2 );
        aRect.AdjustRight( -2 );
    }
    if( aRect.GetHeight() > 5 )
    {
        aRect.AdjustTop(2 );
        aRect.AdjustBottom( -2 );
    }

    GetOutDev()->Invert( aRect );
}

void ScrollBar::GetFocus()
{
    if( !mpData )
    {
        mpData.reset(new ImplScrollBarData);
        mpData->maTimer.SetInvokeHandler( LINK( this, ScrollBar, ImplAutoTimerHdl ) );
        mpData->mbHide = false;
    }
    ImplInvert(); // react immediately
    mpData->maTimer.SetTimeout( GetSettings().GetStyleSettings().GetCursorBlinkTime() );
    if (mpData->maTimer.GetTimeout() != STYLE_CURSOR_NOBLINKTIME)
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

tools::Rectangle* ScrollBar::ImplFindPartRect( const Point& rPt )
{
    // HitTestNativeScrollbar, see remark at top of file

    if (ImplHitTestBtn1(rPt))
        return &maBtn1Rect;

    if (ImplHitTestBtn2(rPt))
        return &maBtn2Rect;

    if (ImplHitTestPageUp(rPt))
        return &maPage1Rect;

    if (ImplHitTestPageDown(rPt))
        return &maPage2Rect;

    if (ImplHitTestThumb(rPt))
        return &maThumbRect;

    return nullptr;
}

bool ScrollBar::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == NotifyEventType::MOUSEMOVE )
    {
        const MouseEvent* pMouseEvt = rNEvt.GetMouseEvent();
        if( pMouseEvt && !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            // Trigger a redraw if mouse over state has changed
            if( IsNativeControlSupported(ControlType::Scrollbar, ControlPart::Entire) )
            {
                tools::Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                tools::Rectangle* pLastRect = ImplFindPartRect( GetLastPointerPosPixel() );
                if( pRect != pLastRect || pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow() )
                {
                    vcl::Region aRgn( GetOutDev()->GetActiveClipRegion() );
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

                    GetOutDev()->SetClipRegion( aClipRegion );
                    Invalidate(aClipRegion.GetBoundRect());

                    GetOutDev()->SetClipRegion( aRgn );
                }
            }
        }
    }

    return Control::PreNotify(rNEvt);
}

void ScrollBar::Scroll()
{
    ImplCallEventListenersAndHandler( VclEventId::ScrollbarScroll, [this] () { maScrollHdl.Call(this); } );
}

void ScrollBar::EndScroll()
{
    ImplCallEventListenersAndHandler( VclEventId::ScrollbarEndScroll, [this] () { maEndScrollHdl.Call(this); } );
}

tools::Long ScrollBar::DoScroll( tools::Long nNewPos )
{
    if ( meScrollType != ScrollType::DontKnow )
        return 0;

    SAL_INFO("vcl.scrollbar", "DoScroll(" << nNewPos << ")");
    meScrollType = ScrollType::Drag;
    tools::Long nDelta = ImplScroll( nNewPos, true );
    meScrollType = ScrollType::DontKnow;
    return nDelta;
}

tools::Long ScrollBar::DoScrollAction( ScrollType eScrollType )
{
    if ( (meScrollType != ScrollType::DontKnow) ||
         (eScrollType == ScrollType::DontKnow) ||
         (eScrollType == ScrollType::Drag) )
        return 0;

    meScrollType = eScrollType;
    tools::Long nDelta = ImplDoAction( true );
    meScrollType = ScrollType::DontKnow;
    return nDelta;
}

void ScrollBar::SetRangeMin( tools::Long nNewRange )
{
    SetRange( Range( nNewRange, GetRangeMax() ) );
}

void ScrollBar::SetRangeMax( tools::Long nNewRange )
{
    SetRange( Range( GetRangeMin(), nNewRange ) );
}

void ScrollBar::SetRange( const Range& rRange )
{
    // Adapt Range
    Range aRange = rRange;
    aRange.Normalize();
    tools::Long nNewMinRange = aRange.Min();
    tools::Long nNewMaxRange = aRange.Max();

    // If Range differs, set a new one
    if ( (mnMinRange == nNewMinRange) && (mnMaxRange == nNewMaxRange))
        return;

    mnMinRange = nNewMinRange;
    mnMaxRange = nNewMaxRange;

    // Adapt Thumb
    if ( mnThumbPos > mnMaxRange-mnVisibleSize )
        mnThumbPos = mnMaxRange-mnVisibleSize;
    if ( mnThumbPos < mnMinRange )
        mnThumbPos = mnMinRange;

    CompatStateChanged( StateChangedType::Data );
}

void ScrollBar::SetThumbPos( tools::Long nNewThumbPos )
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

void ScrollBar::SetVisibleSize( tools::Long nNewSize )
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

    const tools::Long nMinThumbSize = GetSettings().GetStyleSettings().GetMinThumbSize();

    if (GetStyle() & WB_HORZ)
    {
        aRet.setWidth( maBtn1Rect.GetWidth() + nMinThumbSize + maBtn2Rect.GetWidth() );
    }
    else
    {
        aRet.setHeight( maBtn1Rect.GetHeight() + nMinThumbSize + maBtn2Rect.GetHeight() );
    }

    return aRet;
}

Size ScrollBar::getCurrentCalcSize() const
{
    tools::Rectangle aCtrlRegion;
    aCtrlRegion.Union(maBtn1Rect);
    aCtrlRegion.Union(maBtn2Rect);
    aCtrlRegion.Union(maPage1Rect);
    aCtrlRegion.Union(maPage2Rect);
    aCtrlRegion.Union(maThumbRect);
    return aCtrlRegion.GetSize();
}

bool ScrollBar::Inactive() const
{
    return !IsEnabled() || !IsInputEnabled() || IsInModalMode();
}

void ScrollBarBox::ImplInit(vcl::Window* pParent, WinBits nStyle)
{
    Window::ImplInit( pParent, nStyle, nullptr );

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    tools::Long nScrollSize = rStyleSettings.GetScrollBarSize();
    SetSizePixel(Size(nScrollSize, nScrollSize));
}

ScrollBarBox::ScrollBarBox( vcl::Window* pParent, WinBits nStyle ) :
    Window( WindowType::SCROLLBARBOX )
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
