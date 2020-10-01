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
#include <slider.hxx>
#include <vcl/settings.hxx>

#include "thumbpos.hxx"

#define SLIDER_STATE_CHANNEL1_DOWN  (sal_uInt16(0x0001))
#define SLIDER_STATE_CHANNEL2_DOWN  (sal_uInt16(0x0002))
#define SLIDER_STATE_THUMB_DOWN     (sal_uInt16(0x0004))

#define SLIDER_THUMB_SIZE           9
#define SLIDER_CHANNEL_SIZE         4
#define SLIDER_CHANNEL_HALFSIZE     2

#define SLIDER_HEIGHT               16

#define SLIDER_VIEW_STYLE           (WB_3DLOOK | WB_HORZ | WB_VERT)

void Slider::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    mnThumbPixOffset    = 0;
    mnThumbPixRange     = 0;
    mnThumbPixPos       = 0;    // between mnThumbPixOffset and mnThumbPixOffset+mnThumbPixRange
    mnThumbSize         = SLIDER_THUMB_SIZE;
    mnChannelPixRange   = 0;
    mnChannelPixTop     = 0;
    mnChannelPixBottom  = 0;

    mnMinRange          = 0;
    mnMaxRange          = 100;
    mnThumbPos          = 0;
    mnLineSize          = 1;
    mnPageSize          = 1;
    mnStateFlags        = 0;
    meScrollType        = ScrollType::DontKnow;
    mbCalcSize          = true;

    Control::ImplInit( pParent, nStyle, nullptr );

    ImplInitSettings();
    SetSizePixel( CalcWindowSizePixel() );
}

Slider::Slider( vcl::Window* pParent, WinBits nStyle ) :
    Control(WindowType::SLIDER)
{
    ImplInit( pParent, nStyle );
}

Slider::~Slider()
{
    disposeOnce();
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
    tools::Rectangle aOldThumbRect = maThumbRect;
    bool bInvalidateAll = false;

    if ( mnThumbPixRange )
    {
        if ( GetStyle() & WB_HORZ )
        {
            maThumbRect.SetLeft(mnThumbPixPos - (mnThumbSize / 2));
            maThumbRect.SetRight(maThumbRect.Left() + mnThumbSize - 1);
            if ( 0 < maThumbRect.Left() )
            {
                maChannel1Rect.SetLeft( 0 );
                maChannel1Rect.SetRight( maThumbRect.Left()-1 );
                maChannel1Rect.SetTop( mnChannelPixTop );
                maChannel1Rect.SetBottom( mnChannelPixBottom );
            }
            else
                maChannel1Rect.SetEmpty();
            if ( mnChannelPixRange-1 > maThumbRect.Right() )
            {
                maChannel2Rect.SetLeft( maThumbRect.Right()+1 );
                maChannel2Rect.SetRight( mnChannelPixRange-1 );
                maChannel2Rect.SetTop( mnChannelPixTop );
                maChannel2Rect.SetBottom( mnChannelPixBottom );
            }
            else
                maChannel2Rect.SetEmpty();

            const tools::Rectangle aControlRegion(tools::Rectangle(Point(), Size(mnThumbSize, 10)));
            tools::Rectangle aThumbBounds, aThumbContent;
            if ( GetNativeControlRegion( ControlType::Slider, ControlPart::ThumbHorz,
                                         aControlRegion, ControlState::NONE, ImplControlValue(),
                                         aThumbBounds, aThumbContent ) )
            {
                maThumbRect.SetLeft( mnThumbPixPos - aThumbBounds.GetWidth()/2 );
                maThumbRect.SetRight( maThumbRect.Left() + aThumbBounds.GetWidth() - 1 );
                bInvalidateAll = true;
            }
        }
        else
        {
            maThumbRect.SetTop( mnThumbPixPos - (mnThumbSize / 2));
            maThumbRect.SetBottom( maThumbRect.Top() + mnThumbSize - 1);
            if ( 0 < maThumbRect.Top() )
            {
                maChannel1Rect.SetTop( 0 );
                maChannel1Rect.SetBottom( maThumbRect.Top()-1 );
                maChannel1Rect.SetLeft( mnChannelPixTop );
                maChannel1Rect.SetRight( mnChannelPixBottom );
            }
            else
                maChannel1Rect.SetEmpty();
            if ( mnChannelPixRange-1 > maThumbRect.Bottom() )
            {
                maChannel2Rect.SetTop( maThumbRect.Bottom()+1 );
                maChannel2Rect.SetBottom( mnChannelPixRange-1 );
                maChannel2Rect.SetLeft( mnChannelPixTop );
                maChannel2Rect.SetRight( mnChannelPixBottom );
            }
            else
                maChannel2Rect.SetEmpty();

            const tools::Rectangle aControlRegion(tools::Rectangle(Point(), Size(10, mnThumbSize)));
            tools::Rectangle aThumbBounds, aThumbContent;
            if ( GetNativeControlRegion( ControlType::Slider, ControlPart::ThumbVert,
                                         aControlRegion, ControlState::NONE, ImplControlValue(),
                                         aThumbBounds, aThumbContent ) )
            {
                maThumbRect.SetTop( mnThumbPixPos - aThumbBounds.GetHeight()/2 );
                maThumbRect.SetBottom( maThumbRect.Top() + aThumbBounds.GetHeight() - 1 );
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

    if ( !bUpdate )
        return;

    if ( aOldThumbRect == maThumbRect )
        return;

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

    if (mbCalcSize)
    {
        if (GetStyle() & WB_HORZ)
        {
            const tools::Rectangle aControlRegion(tools::Rectangle(Point(), Size(SLIDER_THUMB_SIZE, 10)));
            tools::Rectangle aThumbBounds, aThumbContent;
            if (GetNativeControlRegion(ControlType::Slider, ControlPart::ThumbHorz,
                                       aControlRegion, ControlState::NONE, ImplControlValue(),
                                       aThumbBounds, aThumbContent))
            {
                mnThumbSize = aThumbBounds.GetWidth();
            }
            else
            {
                mnThumbSize = SLIDER_THUMB_SIZE;
            }
        }
        else
        {
            const tools::Rectangle aControlRegion(tools::Rectangle(Point(), Size(10, SLIDER_THUMB_SIZE)));
            tools::Rectangle aThumbBounds, aThumbContent;
            if (GetNativeControlRegion( ControlType::Slider, ControlPart::ThumbVert,
                                         aControlRegion, ControlState::NONE, ImplControlValue(),
                                         aThumbBounds, aThumbContent))
            {
                mnThumbSize = aThumbBounds.GetHeight();
            }
            else
            {
                mnThumbSize = SLIDER_THUMB_SIZE;
            }
        }

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
            maThumbRect.SetTop( 0 );
            maThumbRect.SetBottom( aSize.Height()-1 );
        }
        else
        {
            nCalcWidth          = aSize.Height();
            nCalcHeight         = aSize.Width();
            maThumbRect.SetLeft( 0 );
            maThumbRect.SetRight( aSize.Width()-1 );
        }

        if (nCalcWidth >= mnThumbSize)
        {
            mnThumbPixOffset    = mnThumbSize / 2;
            mnThumbPixRange     = nCalcWidth - mnThumbSize;
            mnThumbPixPos       = 0;
            mnChannelPixRange   = nCalcWidth;
            mnChannelPixTop     = (nCalcHeight/2)-SLIDER_CHANNEL_HALFSIZE;
            mnChannelPixBottom  = mnChannelPixTop+SLIDER_CHANNEL_SIZE-1;
        }
        else
        {
            mnThumbPixRange = 0;
            mnChannelPixRange = 0;
        }

        if ( (nOldChannelPixRange != mnChannelPixRange) ||
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
    // do missing calculations
    if (mbCalcSize)
        ImplCalc(false);

    ControlPart nPart = (GetStyle() & WB_HORZ) ? ControlPart::TrackHorzArea : ControlPart::TrackVertArea;

    if (rRenderContext.IsNativeControlSupported(ControlType::Slider, nPart))
    {
        ControlState nState = (IsEnabled() ? ControlState::ENABLED : ControlState::NONE);
        nState |= (HasFocus() ? ControlState::FOCUSED : ControlState::NONE);

        SliderValue aSliderValue;
        aSliderValue.mnMin = mnMinRange;
        aSliderValue.mnMax = mnMaxRange;
        aSliderValue.mnCur = mnThumbPos;
        aSliderValue.maThumbRect = maThumbRect;

        if (IsMouseOver())
        {
            if (maThumbRect.IsInside(GetPointerPosPixel()))
                aSliderValue.mnThumbState |= ControlState::ROLLOVER;
        }

        const tools::Rectangle aCtrlRegion(Point(0,0), GetOutputSizePixel());

        if (rRenderContext.DrawNativeControl(ControlType::Slider, nPart, aCtrlRegion, nState, aSliderValue, OUString()))
            return;
    }

    DecorationView aDecoView(&rRenderContext);
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    bool bEnabled = IsEnabled();

    if (!maChannel1Rect.IsEmpty())
    {
        long        nRectSize;
        tools::Rectangle   aRect = maChannel1Rect;
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
            aRect.AdjustLeft( 1 );
            aRect.AdjustTop( 1 );
            if (GetStyle() & WB_HORZ)
                aRect.AdjustBottom( -1 );
            else
                aRect.AdjustRight( -1 );
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
        tools::Rectangle aRect = maChannel2Rect;
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

            aRect.AdjustRight( -1 );
            aRect.AdjustBottom( -1 );
            if (GetStyle() & WB_HORZ)
                aRect.AdjustTop( 1 );
            else
                aRect.AdjustLeft( 1 );
            rRenderContext.SetLineColor();
            if (mnStateFlags & SLIDER_STATE_CHANNEL2_DOWN)
                rRenderContext.SetFillColor(rStyleSettings.GetShadowColor());
            else
                rRenderContext.SetFillColor(rStyleSettings.GetCheckedColor());
            rRenderContext.DrawRect(aRect);
        }
    }

    if (maThumbRect.IsEmpty())
        return;

    if (bEnabled)
    {
        DrawButtonFlags nStyle = DrawButtonFlags::NONE;
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

bool Slider::ImplIsPageUp( const Point& rPos )
{
    Size aSize = GetOutputSizePixel();
    tools::Rectangle aRect = maChannel1Rect;
    if ( GetStyle() & WB_HORZ )
    {
        aRect.SetTop( 0 );
        aRect.SetBottom( aSize.Height()-1 );
    }
    else
    {
        aRect.SetLeft( 0 );
        aRect.SetRight( aSize.Width()-1 );
    }
    return aRect.IsInside( rPos );
}

bool Slider::ImplIsPageDown( const Point& rPos )
{
    Size aSize = GetOutputSizePixel();
    tools::Rectangle aRect = maChannel2Rect;
    if ( GetStyle() & WB_HORZ )
    {
        aRect.SetTop( 0 );
        aRect.SetBottom( aSize.Height()-1 );
    }
    else
    {
        aRect.SetLeft( 0 );
        aRect.SetRight( aSize.Width()-1 );
    }
    return aRect.IsInside( rPos );
}

long Slider::ImplSlide( long nNewPos )
{
    long nOldPos = mnThumbPos;
    SetThumbPos( nNewPos );
    long nDelta = mnThumbPos-nOldPos;
    if ( nDelta )
    {
        Slide();
    }
    return nDelta;
}

long Slider::ImplDoAction()
{
    long nDelta = 0;

    switch ( meScrollType )
    {
        case ScrollType::LineUp:
            nDelta = ImplSlide( mnThumbPos-mnLineSize );
            break;

        case ScrollType::LineDown:
            nDelta = ImplSlide( mnThumbPos+mnLineSize );
            break;

        case ScrollType::PageUp:
            nDelta = ImplSlide( mnThumbPos-mnPageSize );
            break;

        case ScrollType::PageDown:
            nDelta = ImplSlide( mnThumbPos+mnPageSize );
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
        case ScrollType::PageUp:
            if ( ImplIsPageUp( rMousePos ) )
            {
                bAction = bCallAction;
                mnStateFlags |= SLIDER_STATE_CHANNEL1_DOWN;
            }
            else
                mnStateFlags &= ~SLIDER_STATE_CHANNEL1_DOWN;
            break;

        case ScrollType::PageDown:
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
        if ( ImplDoAction() )
        {
            Invalidate();
        }
    }
    else if ( nOldStateFlags != mnStateFlags )
    {
        Invalidate();
    }
}

void Slider::ImplDoSlide( long nNewPos )
{
    if ( meScrollType != ScrollType::DontKnow )
        return;

    meScrollType = ScrollType::Drag;
    ImplSlide( nNewPos );
    meScrollType = ScrollType::DontKnow;
}

void Slider::ImplDoSlideAction( ScrollType eScrollType )
{
    if ( (meScrollType != ScrollType::DontKnow) ||
         (eScrollType == ScrollType::DontKnow) ||
         (eScrollType == ScrollType::Drag) )
        return;

    meScrollType = eScrollType;
    ImplDoAction();
    meScrollType = ScrollType::DontKnow;
}

void Slider::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() )
        return;

    const Point&       rMousePos = rMEvt.GetPosPixel();
    StartTrackingFlags nTrackFlags = StartTrackingFlags::NONE;

    if ( maThumbRect.IsInside( rMousePos ) )
    {
        meScrollType    = ScrollType::Drag;

        // calculate additional values
        Point aCenterPos = maThumbRect.Center();
        if ( GetStyle() & WB_HORZ )
            mnMouseOff = rMousePos.X()-aCenterPos.X();
        else
            mnMouseOff = rMousePos.Y()-aCenterPos.Y();
    }
    else if ( ImplIsPageUp( rMousePos ) )
    {
        nTrackFlags = StartTrackingFlags::ButtonRepeat;
        meScrollType = ScrollType::PageUp;
    }
    else if ( ImplIsPageDown( rMousePos ) )
    {
        nTrackFlags = StartTrackingFlags::ButtonRepeat;
        meScrollType = ScrollType::PageDown;
    }

    // Shall we start Tracking?
    if( meScrollType != ScrollType::DontKnow )
    {
        // store Start position for cancel and EndScroll delta
        mnStartPos = mnThumbPos;
        ImplDoMouseAction( rMousePos, /*bCallAction*/true );
        PaintImmediately();

        StartTracking( nTrackFlags );
    }
}

void Slider::MouseButtonUp( const MouseEvent& )
{
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
            SetThumbPos( mnStartPos );
            Slide();
        }

        if ( meScrollType == ScrollType::Drag )
        {
            // after dragging, recalculate to a rounded Thumb position
            ImplCalc();
            PaintImmediately();
        }

        meScrollType = ScrollType::DontKnow;
    }
    else
    {
        const Point rMousePos = rTEvt.GetMouseEvent().GetPosPixel();

        // special handling for dragging
        if ( meScrollType == ScrollType::Drag )
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
                    PaintImmediately();
                    if ( nOldPos != mnThumbPos )
                    {
                        Slide();
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
                ImplDoSlideAction( ScrollType::LineUp );
                break;

            case KEY_RIGHT:
            case KEY_DOWN:
                ImplDoSlideAction( ScrollType::LineDown );
                break;

            case KEY_PAGEUP:
                ImplDoSlideAction( ScrollType::PageUp );
                break;

            case KEY_PAGEDOWN:
                ImplDoSlideAction( ScrollType::PageDown );
                break;

            default:
                Control::KeyInput( rKEvt );
                break;
        }
    }
    else
        Control::KeyInput( rKEvt );
}

void Slider::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
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
        CompatStateChanged( StateChangedType::Data );
    }
}

Size Slider::CalcWindowSizePixel()
{
    long nWidth = mnMaxRange - mnMinRange + mnThumbSize + 1;
    long nHeight = SLIDER_HEIGHT;
    Size aSize;
    if ( GetStyle() & WB_HORZ )
    {
        aSize.setWidth( nWidth );
        aSize.setHeight( nHeight );
    }
    else
    {
        aSize.setHeight( nWidth );
        aSize.setWidth( nHeight );
    }
    return aSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
