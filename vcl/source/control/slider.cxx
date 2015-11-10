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

#include <tools/rc.h>
#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/slider.hxx>
#include <vcl/settings.hxx>

#include "thumbpos.hxx"

#define SLIDER_STATE_CHANNEL1_DOWN  ((sal_uInt16)0x0001)
#define SLIDER_STATE_CHANNEL2_DOWN  ((sal_uInt16)0x0002)
#define SLIDER_STATE_THUMB_DOWN     ((sal_uInt16)0x0004)

#define SLIDER_THUMB_SIZE           9
#define SLIDER_THUMB_HALFSIZE       4
#define SLIDER_CHANNEL_OFFSET       0
#define SLIDER_CHANNEL_SIZE         4
#define SLIDER_CHANNEL_HALFSIZE     2

#define SLIDER_HEIGHT               16

#define SLIDER_VIEW_STYLE           (WB_3DLOOK | WB_HORZ | WB_VERT)

void Slider::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    mnThumbPixOffset    = 0;
    mnThumbPixRange     = 0;
    mnThumbPixPos       = 0;    // between mnThumbPixOffset and mnThumbPixOffset+mnThumbPixRange
    mnChannelPixOffset  = 0;
    mnChannelPixRange   = 0;
    mnChannelPixTop     = 0;
    mnChannelPixBottom  = 0;

    mnMinRange          = 0;
    mnMaxRange          = 100;
    mnThumbPos          = 0;
    mnLineSize          = 1;
    mnPageSize          = 1;
    mnDelta             = 0;
    mnDragDraw          = 0;
    mnStateFlags        = 0;
    meScrollType        = SCROLL_DONTKNOW;
    mbCalcSize          = true;
    mbFullDrag          = true;

    mpLinkedField       = nullptr;

    Control::ImplInit( pParent, nStyle, nullptr );

    ImplInitSettings();
    SetSizePixel( CalcWindowSizePixel() );
}

Slider::Slider( vcl::Window* pParent, WinBits nStyle ) :
    Control(WINDOW_SLIDER)
{
    ImplInit( pParent, nStyle );
}

Slider::~Slider()
{
    disposeOnce();
}

void Slider::dispose()
{
    mpLinkedField.clear();
    Control::dispose();
}

void Slider::ImplInitSettings()
{
    vcl::Window* pParent = GetParent();
    if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
    {
        EnableChildTransparentMode();
        SetParentClipMode( ParentClipMode::NoClip );
        SetPaintTransparent( true );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( false );
        SetParentClipMode();
        SetPaintTransparent( false );

        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( pParent->GetBackground() );
    }
}

void Slider::ImplUpdateRects( bool bUpdate )
{
    Rectangle aOldThumbRect = maThumbRect;
    bool bInvalidateAll = false;

    if ( mnThumbPixRange )
    {
        if ( GetStyle() & WB_HORZ )
        {
            maThumbRect.Left()      = mnThumbPixPos-SLIDER_THUMB_HALFSIZE;
            maThumbRect.Right()     = maThumbRect.Left()+SLIDER_THUMB_SIZE-1;
            if ( mnChannelPixOffset < maThumbRect.Left() )
            {
                maChannel1Rect.Left()   = mnChannelPixOffset;
                maChannel1Rect.Right()  = maThumbRect.Left()-1;
                maChannel1Rect.Top()    = mnChannelPixTop;
                maChannel1Rect.Bottom() = mnChannelPixBottom;
            }
            else
                maChannel1Rect.SetEmpty();
            if ( mnChannelPixOffset+mnChannelPixRange-1 > maThumbRect.Right() )
            {
                maChannel2Rect.Left()   = maThumbRect.Right()+1;
                maChannel2Rect.Right()  = mnChannelPixOffset+mnChannelPixRange-1;
                maChannel2Rect.Top()    = mnChannelPixTop;
                maChannel2Rect.Bottom() = mnChannelPixBottom;
            }
            else
                maChannel2Rect.SetEmpty();

            const Rectangle aControlRegion( Rectangle( Point(0,0), Size( SLIDER_THUMB_SIZE, 10 ) ) );
            Rectangle aThumbBounds, aThumbContent;
            if ( GetNativeControlRegion( CTRL_SLIDER, PART_THUMB_HORZ,
                                         aControlRegion, ControlState::NONE, ImplControlValue(), OUString(),
                                         aThumbBounds, aThumbContent ) )
            {
                maThumbRect.Left() = mnThumbPixPos - aThumbBounds.GetWidth()/2;
                maThumbRect.Right() = maThumbRect.Left() + aThumbBounds.GetWidth() - 1;
                bInvalidateAll = true;
            }
        }
        else
        {
            maThumbRect.Top()       = mnThumbPixPos-SLIDER_THUMB_HALFSIZE;
            maThumbRect.Bottom()    = maThumbRect.Top()+SLIDER_THUMB_SIZE-1;
            if ( mnChannelPixOffset < maThumbRect.Top() )
            {
                maChannel1Rect.Top()    = mnChannelPixOffset;
                maChannel1Rect.Bottom() = maThumbRect.Top()-1;
                maChannel1Rect.Left()   = mnChannelPixTop;
                maChannel1Rect.Right()  = mnChannelPixBottom;
            }
            else
                maChannel1Rect.SetEmpty();
            if ( mnChannelPixOffset+mnChannelPixRange-1 > maThumbRect.Bottom() )
            {
                maChannel2Rect.Top()    = maThumbRect.Bottom()+1;
                maChannel2Rect.Bottom() = mnChannelPixOffset+mnChannelPixRange-1;
                maChannel2Rect.Left()   = mnChannelPixTop;
                maChannel2Rect.Right()  = mnChannelPixBottom;
            }
            else
                maChannel2Rect.SetEmpty();

            const Rectangle aControlRegion( Rectangle( Point(0,0), Size( 10, SLIDER_THUMB_SIZE ) ) );
            Rectangle aThumbBounds, aThumbContent;
            if ( GetNativeControlRegion( CTRL_SLIDER, PART_THUMB_VERT,
                                         aControlRegion, ControlState::NONE, ImplControlValue(), OUString(),
                                         aThumbBounds, aThumbContent ) )
            {
                maThumbRect.Top() = mnThumbPixPos - aThumbBounds.GetHeight()/2;
                maThumbRect.Bottom() = maThumbRect.Top() + aThumbBounds.GetHeight() - 1;
                bInvalidateAll = true;
            }
        }
    }
    else
    {
        maChannel1Rect.SetEmpty();
        maChannel2Rect.SetEmpty();
        maThumbRect.SetEmpty();
    }

    if ( bUpdate )
    {
        if ( aOldThumbRect != maThumbRect )
        {
            if( bInvalidateAll )
                Invalidate(InvalidateFlags::NoChildren | InvalidateFlags::NoErase);
            else
            {
                vcl::Region aInvalidRegion( aOldThumbRect );
                aInvalidRegion.Union( maThumbRect );

                if( !IsBackground() && GetParent() )
                {
                    const Point aPos( GetPosPixel() );
                    aInvalidRegion.Move( aPos.X(), aPos.Y() );
                    GetParent()->Invalidate( aInvalidRegion, InvalidateFlags::Transparent | InvalidateFlags::Update );
                }
                else
                    Invalidate( aInvalidRegion );
            }
        }
    }
}

void Slider::ImplUpdateLinkedField()
{
    if (mpLinkedField)
        mpLinkedField->SetValue(mnThumbPos);
}

long Slider::ImplCalcThumbPos( long nPixPos )
{
    // calculate position
    long nCalcThumbPos;
    nCalcThumbPos = ImplMulDiv( nPixPos-mnThumbPixOffset, mnMaxRange-mnMinRange, mnThumbPixRange-1 );
    nCalcThumbPos += mnMinRange;
    return nCalcThumbPos;
}

long Slider::ImplCalcThumbPosPix( long nPos )
{
    // calculate position
    long nCalcThumbPos;
    nCalcThumbPos = ImplMulDiv( nPos-mnMinRange, mnThumbPixRange-1, mnMaxRange-mnMinRange );
    // at the beginning and end we try to display Slider correctly
    if ( !nCalcThumbPos && (mnThumbPos > mnMinRange) )
        nCalcThumbPos = 1;
    if ( nCalcThumbPos &&
         (nCalcThumbPos == mnThumbPixRange-1) &&
         (mnThumbPos < mnMaxRange) )
        nCalcThumbPos--;
    return nCalcThumbPos+mnThumbPixOffset;
}

void Slider::ImplCalc( bool bUpdate )
{
    bool bInvalidateAll = false;

    if ( mbCalcSize )
    {
        long nOldChannelPixOffset   = mnChannelPixOffset;
        long nOldChannelPixRange    = mnChannelPixRange;
        long nOldChannelPixTop      = mnChannelPixTop;
        long nOldChannelPixBottom   = mnChannelPixBottom;
        long nCalcWidth;
        long nCalcHeight;

        maChannel1Rect.SetEmpty();
        maChannel2Rect.SetEmpty();
        maThumbRect.SetEmpty();

        Size aSize = GetOutputSizePixel();
        if ( GetStyle() & WB_HORZ )
        {
            nCalcWidth          = aSize.Width();
            nCalcHeight         = aSize.Height();
            maThumbRect.Top()   = 0;
            maThumbRect.Bottom()= aSize.Height()-1;
        }
        else
        {
            nCalcWidth          = aSize.Height();
            nCalcHeight         = aSize.Width();
            maThumbRect.Left()  = 0;
            maThumbRect.Right() = aSize.Width()-1;
        }

        if ( nCalcWidth >= SLIDER_THUMB_SIZE )
        {
            mnThumbPixOffset    = SLIDER_THUMB_HALFSIZE;
            mnThumbPixRange     = nCalcWidth-(SLIDER_THUMB_HALFSIZE*2);
            mnThumbPixPos       = 0;
            mnChannelPixOffset  = SLIDER_CHANNEL_OFFSET;
            mnChannelPixRange   = nCalcWidth-(SLIDER_CHANNEL_OFFSET*2);
            mnChannelPixTop     = (nCalcHeight/2)-SLIDER_CHANNEL_HALFSIZE;
            mnChannelPixBottom  = mnChannelPixTop+SLIDER_CHANNEL_SIZE-1;
        }
        else
        {
            mnThumbPixRange = 0;
            mnChannelPixRange = 0;
        }

        if ( (nOldChannelPixOffset != mnChannelPixOffset) ||
             (nOldChannelPixRange != mnChannelPixRange) ||
             (nOldChannelPixTop != mnChannelPixTop) ||
             (nOldChannelPixBottom != mnChannelPixBottom) )
            bInvalidateAll = true;

        mbCalcSize = false;
    }

    if ( mnThumbPixRange )
        mnThumbPixPos = ImplCalcThumbPosPix( mnThumbPos );

    if ( bUpdate && bInvalidateAll )
    {
        Invalidate();
        bUpdate = false;
    }
    ImplUpdateRects( bUpdate );
}

void Slider::ImplDraw(vcl::RenderContext& rRenderContext)
{
    DecorationView aDecoView(&rRenderContext);
    DrawButtonFlags nStyle;
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    bool bEnabled = IsEnabled();

    // do missing calculations
    if (mbCalcSize)
        ImplCalc(false);

    ControlPart nPart = (GetStyle() & WB_HORZ) ? PART_TRACK_HORZ_AREA : PART_TRACK_VERT_AREA;
    ControlState nState = (IsEnabled() ? ControlState::ENABLED : ControlState::NONE);
    nState |= (HasFocus() ? ControlState::FOCUSED : ControlState::NONE);
    SliderValue sldValue;

    sldValue.mnMin = mnMinRange;
    sldValue.mnMax = mnMaxRange;
    sldValue.mnCur = mnThumbPos;
    sldValue.maThumbRect = maThumbRect;

    if (IsMouseOver())
    {
        if (maThumbRect.IsInside(GetPointerPosPixel()))
            sldValue.mnThumbState |= ControlState::ROLLOVER;
    }

    const Rectangle aCtrlRegion(Point(0,0), GetOutputSizePixel());
    bool bNativeOK = rRenderContext.DrawNativeControl(CTRL_SLIDER, nPart, aCtrlRegion, nState, sldValue, OUString());
    if (bNativeOK)
        return;

    if (!maChannel1Rect.IsEmpty())
    {
        long        nRectSize;
        Rectangle   aRect = maChannel1Rect;
        rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
        if (GetStyle() & WB_HORZ)
        {
            rRenderContext.DrawLine(aRect.TopLeft(), Point(aRect.Left(), aRect.Bottom() - 1));
            rRenderContext.DrawLine(aRect.TopLeft(), aRect.TopRight());
        }
        else
        {
            rRenderContext.DrawLine(aRect.TopLeft(), Point(aRect.Right() - 1, aRect.Top()));
            rRenderContext.DrawLine(aRect.TopLeft(), aRect.BottomLeft());
        }
        rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
        if (GetStyle() & WB_HORZ)
        {
            rRenderContext.DrawLine(aRect.BottomLeft(), aRect.BottomRight());
            nRectSize = aRect.GetWidth();
        }
        else
        {
            rRenderContext.DrawLine(aRect.TopRight(), aRect.BottomRight());
            nRectSize = aRect.GetHeight();
        }

        if (nRectSize > 1)
        {
            aRect.Left()++;
            aRect.Top()++;
            if (GetStyle() & WB_HORZ)
                aRect.Bottom()--;
            else
                aRect.Right()--;
            rRenderContext.SetLineColor();
            if (mnStateFlags & SLIDER_STATE_CHANNEL1_DOWN)
                rRenderContext.SetFillColor(rStyleSettings.GetShadowColor());
            else
                rRenderContext.SetFillColor(rStyleSettings.GetCheckedColor());
            rRenderContext.DrawRect(aRect);
        }
    }

    if (!maChannel2Rect.IsEmpty())
    {
        long nRectSize;
        Rectangle aRect = maChannel2Rect;
        rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
        if (GetStyle() & WB_HORZ)
        {
            rRenderContext.DrawLine(aRect.TopRight(), aRect.BottomRight());
            rRenderContext.DrawLine(aRect.BottomLeft(), aRect.BottomRight());
            nRectSize = aRect.GetWidth();
        }
        else
        {
            rRenderContext.DrawLine(aRect.BottomLeft(), aRect.BottomRight());
            rRenderContext.DrawLine(aRect.TopRight(), aRect.BottomRight());
            nRectSize = aRect.GetHeight();
        }

        if (nRectSize > 1)
        {
            rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
            if (GetStyle() & WB_HORZ)
                rRenderContext.DrawLine(aRect.TopLeft(), Point(aRect.Right() - 1, aRect.Top()));
            else
                rRenderContext.DrawLine(aRect.TopLeft(), Point(aRect.Left(), aRect.Bottom() - 1));

            aRect.Right()--;
            aRect.Bottom()--;
            if (GetStyle() & WB_HORZ)
                aRect.Top()++;
            else
                aRect.Left()++;
            rRenderContext.SetLineColor();
            if (mnStateFlags & SLIDER_STATE_CHANNEL2_DOWN)
                rRenderContext.SetFillColor(rStyleSettings.GetShadowColor());
            else
                rRenderContext.SetFillColor(rStyleSettings.GetCheckedColor());
            rRenderContext.DrawRect(aRect);
        }
    }

    if (!maThumbRect.IsEmpty())
    {
        if (bEnabled)
        {
            nStyle = DrawButtonFlags::NONE;
            if (mnStateFlags & SLIDER_STATE_THUMB_DOWN)
                nStyle |= DrawButtonFlags::Pressed;
            aDecoView.DrawButton(maThumbRect, nStyle);
        }
        else
        {
            rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
            rRenderContext.SetFillColor(rStyleSettings.GetCheckedColor());
            rRenderContext.DrawRect(maThumbRect);
        }
    }
}

bool Slider::ImplIsPageUp( const Point& rPos )
{
    Size aSize = GetOutputSizePixel();
    Rectangle aRect = maChannel1Rect;
    if ( GetStyle() & WB_HORZ )
    {
        aRect.Top()     = 0;
        aRect.Bottom()  = aSize.Height()-1;
    }
    else
    {
        aRect.Left()    = 0;
        aRect.Right()   = aSize.Width()-1;
    }
    return aRect.IsInside( rPos );
}

bool Slider::ImplIsPageDown( const Point& rPos )
{
    Size aSize = GetOutputSizePixel();
    Rectangle aRect = maChannel2Rect;
    if ( GetStyle() & WB_HORZ )
    {
        aRect.Top()     = 0;
        aRect.Bottom()  = aSize.Height()-1;
    }
    else
    {
        aRect.Left()    = 0;
        aRect.Right()   = aSize.Width()-1;
    }
    return aRect.IsInside( rPos );
}

long Slider::ImplSlide( long nNewPos, bool bCallEndSlide )
{
    long nOldPos = mnThumbPos;
    SetThumbPos( nNewPos );
    long nDelta = mnThumbPos-nOldPos;
    if ( nDelta )
    {
        mnDelta = nDelta;
        Slide();
        if ( bCallEndSlide )
            EndSlide();
        mnDelta = 0;
    }
    return nDelta;
}

long Slider::ImplDoAction( bool bCallEndSlide )
{
    long nDelta = 0;

    switch ( meScrollType )
    {
        case SCROLL_LINEUP:
            nDelta = ImplSlide( mnThumbPos-mnLineSize, bCallEndSlide );
            break;

        case SCROLL_LINEDOWN:
            nDelta = ImplSlide( mnThumbPos+mnLineSize, bCallEndSlide );
            break;

        case SCROLL_PAGEUP:
            nDelta = ImplSlide( mnThumbPos-mnPageSize, bCallEndSlide );
            break;

        case SCROLL_PAGEDOWN:
            nDelta = ImplSlide( mnThumbPos+mnPageSize, bCallEndSlide );
            break;

        case SCROLL_SET:
            nDelta = ImplSlide( ImplCalcThumbPos( GetPointerPosPixel().X() ), bCallEndSlide );
            break;
        default:
            break;
    }

    return nDelta;
}

void Slider::ImplDoMouseAction( const Point& rMousePos, bool bCallAction )
{
    sal_uInt16  nOldStateFlags = mnStateFlags;
    bool    bAction = false;

    switch ( meScrollType )
    {
        case( SCROLL_SET ):
        {
            const bool bUp = ImplIsPageUp( rMousePos ), bDown = ImplIsPageDown( rMousePos );

            if ( bUp || bDown )
            {
                bAction = bCallAction;
                mnStateFlags |= ( bUp ? SLIDER_STATE_CHANNEL1_DOWN : SLIDER_STATE_CHANNEL2_DOWN );
            }
            else
                mnStateFlags &= ~( SLIDER_STATE_CHANNEL1_DOWN | SLIDER_STATE_CHANNEL2_DOWN );
            break;
        }

        case SCROLL_PAGEUP:
            if ( ImplIsPageUp( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SLIDER_STATE_CHANNEL1_DOWN;
            }
            else
                mnStateFlags &= ~SLIDER_STATE_CHANNEL1_DOWN;
            break;

        case SCROLL_PAGEDOWN:
            if ( ImplIsPageDown( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SLIDER_STATE_CHANNEL2_DOWN;
            }
            else
                mnStateFlags &= ~SLIDER_STATE_CHANNEL2_DOWN;
            break;
        default:
            break;
    }

    if ( bAction )
    {
        if ( ImplDoAction( false ) )
        {
            Update();
            Invalidate();
        }
    }
    else if ( nOldStateFlags != mnStateFlags )
    {
        Invalidate();
    }
}

long Slider::ImplDoSlide( long nNewPos )
{
    if ( meScrollType != SCROLL_DONTKNOW )
        return 0;

    meScrollType = SCROLL_DRAG;
    long nDelta = ImplSlide( nNewPos, true );
    meScrollType = SCROLL_DONTKNOW;
    return nDelta;
}

long Slider::ImplDoSlideAction( ScrollType eScrollType )
{
    if ( (meScrollType != SCROLL_DONTKNOW) ||
         (eScrollType == SCROLL_DONTKNOW) ||
         (eScrollType == SCROLL_DRAG) )
        return 0;

    meScrollType = eScrollType;
    long nDelta = ImplDoAction( true );
    meScrollType = SCROLL_DONTKNOW;
    return nDelta;
}

void Slider::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        const Point&       rMousePos = rMEvt.GetPosPixel();
        StartTrackingFlags nTrackFlags = StartTrackingFlags::NONE;

        if ( maThumbRect.IsInside( rMousePos ) )
        {
            meScrollType    = SCROLL_DRAG;

            // calculate additional values
            Point aCenterPos = maThumbRect.Center();
            if ( GetStyle() & WB_HORZ )
                mnMouseOff = rMousePos.X()-aCenterPos.X();
            else
                mnMouseOff = rMousePos.Y()-aCenterPos.Y();
        }
        else if ( ImplIsPageUp( rMousePos ) )
        {
            if( GetStyle() & WB_SLIDERSET )
                meScrollType = SCROLL_SET;
            else
            {
                nTrackFlags = StartTrackingFlags::ButtonRepeat;
                meScrollType = SCROLL_PAGEUP;
            }
        }
        else if ( ImplIsPageDown( rMousePos ) )
        {
            if( GetStyle() & WB_SLIDERSET )
                meScrollType = SCROLL_SET;
            else
            {
                nTrackFlags = StartTrackingFlags::ButtonRepeat;
                meScrollType = SCROLL_PAGEDOWN;
            }
        }

        // Shall we start Tracking?
        if( meScrollType != SCROLL_DONTKNOW )
        {
            // store Start position for cancel and EndScroll delta
            mnStartPos = mnThumbPos;
            ImplDoMouseAction( rMousePos, meScrollType != SCROLL_SET );
            Update();

            if( meScrollType != SCROLL_SET )
                StartTracking( nTrackFlags );
        }
    }
}

void Slider::MouseButtonUp( const MouseEvent& )
{
    if( SCROLL_SET == meScrollType )
    {
        // reset Button and PageRect state
        const sal_uInt16 nOldStateFlags = mnStateFlags;

        mnStateFlags &= ~( SLIDER_STATE_CHANNEL1_DOWN | SLIDER_STATE_CHANNEL2_DOWN | SLIDER_STATE_THUMB_DOWN );

        if ( nOldStateFlags != mnStateFlags )
        {
            Invalidate(InvalidateFlags::NoChildren | InvalidateFlags::NoErase);
        }
        ImplDoAction( true );
        meScrollType = SCROLL_DONTKNOW;
    }
}

void Slider::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        // reset Button and PageRect state
        sal_uInt16 nOldStateFlags = mnStateFlags;
        mnStateFlags &= ~(SLIDER_STATE_CHANNEL1_DOWN | SLIDER_STATE_CHANNEL2_DOWN |
                          SLIDER_STATE_THUMB_DOWN);
        if ( nOldStateFlags != mnStateFlags )
        {
            Invalidate(InvalidateFlags::NoChildren | InvalidateFlags::NoErase);
        }

        // on cancel, reset the previous Thumb position
        if ( rTEvt.IsTrackingCanceled() )
        {
            long nOldPos = mnThumbPos;
            SetThumbPos( mnStartPos );
            mnDelta = mnThumbPos-nOldPos;
            Slide();
        }

        if ( meScrollType == SCROLL_DRAG )
        {
            // after dragging, recalculate to a rounded Thumb position
            ImplCalc();
            Update();

            if ( !mbFullDrag && (mnStartPos != mnThumbPos) )
            {
                mnDelta = mnThumbPos-mnStartPos;
                Slide();
                mnDelta = 0;
            }
        }

        mnDelta = mnThumbPos-mnStartPos;
        EndSlide();
        mnDelta = 0;
        meScrollType = SCROLL_DONTKNOW;
    }
    else
    {
        const Point rMousePos = rTEvt.GetMouseEvent().GetPosPixel();

        // special handling for dragging
        if ( meScrollType == SCROLL_DRAG )
        {
            long nMovePix;
            Point aCenterPos = maThumbRect.Center();
            if ( GetStyle() & WB_HORZ )
                nMovePix = rMousePos.X()-(aCenterPos.X()+mnMouseOff);
            else
                nMovePix = rMousePos.Y()-(aCenterPos.Y()+mnMouseOff);
            // only if the mouse moves in Scroll direction we have to act
            if ( nMovePix )
            {
                mnThumbPixPos += nMovePix;
                if ( mnThumbPixPos < mnThumbPixOffset )
                    mnThumbPixPos = mnThumbPixOffset;
                if ( mnThumbPixPos > (mnThumbPixOffset+mnThumbPixRange-1) )
                    mnThumbPixPos = mnThumbPixOffset+mnThumbPixRange-1;
                long nOldPos = mnThumbPos;
                mnThumbPos = ImplCalcThumbPos( mnThumbPixPos );
                if ( nOldPos != mnThumbPos )
                {
                    ImplUpdateRects();
                    Update();
                    ImplUpdateLinkedField();
                    if ( mbFullDrag && (nOldPos != mnThumbPos) )
                    {
                        mnDelta = mnThumbPos-nOldPos;
                        Slide();
                        mnDelta = 0;
                    }
                }
            }
        }
        else
            ImplDoMouseAction( rMousePos, rTEvt.IsTrackingRepeat() );

        // end tracking if ScrollBar values indicate we are done
        if ( !IsVisible() )
            EndTracking();
    }
}

void Slider::KeyInput( const KeyEvent& rKEvt )
{
    if ( !rKEvt.GetKeyCode().GetModifier() )
    {
        switch ( rKEvt.GetKeyCode().GetCode() )
        {
            case KEY_HOME:
                ImplDoSlide( GetRangeMin() );
                break;
            case KEY_END:
                ImplDoSlide( GetRangeMax() );
                break;

            case KEY_LEFT:
            case KEY_UP:
                ImplDoSlideAction( SCROLL_LINEUP );
                break;

            case KEY_RIGHT:
            case KEY_DOWN:
                ImplDoSlideAction( SCROLL_LINEDOWN );
                break;

            case KEY_PAGEUP:
                ImplDoSlideAction( SCROLL_PAGEUP );
                break;

            case KEY_PAGEDOWN:
                ImplDoSlideAction( SCROLL_PAGEDOWN );
                break;

            default:
                Control::KeyInput( rKEvt );
                break;
        }
    }
    else
        Control::KeyInput( rKEvt );
}

void Slider::Paint(vcl::RenderContext& rRenderContext, const Rectangle& /*rRect*/)
{
    ImplDraw(rRenderContext);
}

void Slider::Resize()
{
    Control::Resize();
    mbCalcSize = true;
    if ( IsReallyVisible() )
        ImplCalc( false );
    Invalidate(InvalidateFlags::NoChildren | InvalidateFlags::NoErase);
}

void Slider::SetLinkedField(VclPtr<NumericField> pField)
{
    if (mpLinkedField)
    {
        mpLinkedField->SetModifyHdl(Link<Edit&,void>());
        mpLinkedField->SetUpHdl(Link<SpinField&,void>());
        mpLinkedField->SetDownHdl(Link<SpinField&,void>());
        mpLinkedField->SetFirstHdl(Link<SpinField&,void>());
        mpLinkedField->SetLastHdl(Link<SpinField&,void>());
        mpLinkedField->SetLoseFocusHdl(Link<Control&,void>());
    }
    mpLinkedField = pField;
    if (mpLinkedField)
    {
        mpLinkedField->SetModifyHdl(LINK(this, Slider, LinkedFieldModifyHdl));
        mpLinkedField->SetUpHdl(LINK(this, Slider, LinkedFieldSpinnerHdl));
        mpLinkedField->SetDownHdl(LINK(this, Slider, LinkedFieldSpinnerHdl));
        mpLinkedField->SetFirstHdl(LINK(this, Slider, LinkedFieldSpinnerHdl));
        mpLinkedField->SetLastHdl(LINK(this, Slider, LinkedFieldSpinnerHdl));
        mpLinkedField->SetLoseFocusHdl(LINK(this, Slider, LinkedFieldLoseFocusHdl));
    }
}

IMPL_LINK_NOARG_TYPED(Slider, LinkedFieldSpinnerHdl, SpinField&, void)
{
    if (mpLinkedField)
        SetThumbPos(mpLinkedField->GetValue());
}
IMPL_LINK_NOARG_TYPED(Slider, LinkedFieldLoseFocusHdl, Control&, void)
{
    if (mpLinkedField)
        SetThumbPos(mpLinkedField->GetValue());
}
IMPL_LINK_NOARG_TYPED(Slider, LinkedFieldModifyHdl, Edit&, void)
{
    if (mpLinkedField)
        SetThumbPos(mpLinkedField->GetValue());
}

void Slider::RequestHelp( const HelpEvent& rHEvt )
{
    Control::RequestHelp( rHEvt );
}

void Slider::StateChanged( StateChangedType nType )
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
        {
            Invalidate();
        }
    }
    else if ( nType == StateChangedType::Style )
    {
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            if ( (GetPrevStyle() & SLIDER_VIEW_STYLE) !=
                 (GetStyle() & SLIDER_VIEW_STYLE) )
            {
                mbCalcSize = true;
                ImplCalc( false );
                Invalidate();
            }
        }
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void Slider::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void Slider::Slide()
{
    maSlideHdl.Call( this );
}

void Slider::EndSlide()
{
    maEndSlideHdl.Call( this );
}

void Slider::SetRangeMin(long nNewRange)
{
    SetRange(Range(nNewRange, GetRangeMax()));
}

void Slider::SetRangeMax(long nNewRange)
{
    SetRange(Range(GetRangeMin(), nNewRange));
}

void Slider::SetRange( const Range& rRange )
{
    // adjust Range
    Range aRange = rRange;
    aRange.Justify();
    long nNewMinRange = aRange.Min();
    long nNewMaxRange = aRange.Max();

    // reset Range if different
    if ( (mnMinRange != nNewMinRange) ||
         (mnMaxRange != nNewMaxRange) )
    {
        mnMinRange = nNewMinRange;
        mnMaxRange = nNewMaxRange;

        // adjust Thumb
        if ( mnThumbPos > mnMaxRange )
            mnThumbPos = mnMaxRange;
        if ( mnThumbPos < mnMinRange )
            mnThumbPos = mnMinRange;
        ImplUpdateLinkedField();
        CompatStateChanged( StateChangedType::Data );
    }
}

void Slider::SetThumbPos( long nNewThumbPos )
{
    if ( nNewThumbPos < mnMinRange )
        nNewThumbPos = mnMinRange;
    if ( nNewThumbPos > mnMaxRange )
        nNewThumbPos = mnMaxRange;

    if ( mnThumbPos != nNewThumbPos )
    {
        mnThumbPos = nNewThumbPos;
        ImplUpdateLinkedField();
        CompatStateChanged( StateChangedType::Data );
    }
}

Size Slider::CalcWindowSizePixel()
{
    long nWidth = mnMaxRange-mnMinRange+(SLIDER_THUMB_HALFSIZE*2)+1;
    long nHeight = SLIDER_HEIGHT;
    Size aSize;
    if ( GetStyle() & WB_HORZ )
    {
        aSize.Width()   = nWidth;
        aSize.Height()  = nHeight;
    }
    else
    {
        aSize.Height()  = nWidth;
        aSize.Width()   = nHeight;
    }
    return aSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
