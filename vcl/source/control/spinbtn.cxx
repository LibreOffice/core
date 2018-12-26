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
#include <vcl/spin.hxx>
#include <vcl/settings.hxx>
#include <vcl/vclevent.hxx>

#include <spin.hxx>

void SpinButton::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    mbUpperIn     = false;
    mbLowerIn     = false;
    mbInitialUp   = false;
    mbInitialDown = false;

    mnMinRange  = 0;
    mnMaxRange  = 100;
    mnValue     = 0;
    mnValueStep = 1;

    maRepeatTimer.SetTimeout(MouseSettings::GetButtonStartRepeat());
    maRepeatTimer.SetInvokeHandler(LINK(this, SpinButton, ImplTimeout));

    mbRepeat = 0 != (nStyle & WB_REPEAT);

    if (nStyle & WB_HSCROLL)
        mbHorz = true;
    else
        mbHorz = false;

    Control::ImplInit( pParent, nStyle, nullptr );
}

SpinButton::SpinButton( vcl::Window* pParent, WinBits nStyle )
    : Control(WindowType::SPINBUTTON)
    , mbUpperIsFocused(false)
{
    ImplInit(pParent, nStyle);
}

IMPL_LINK(SpinButton, ImplTimeout, Timer*, pTimer, void)
{
    if (pTimer->GetTimeout() == MouseSettings::GetButtonStartRepeat())
    {
        pTimer->SetTimeout( GetSettings().GetMouseSettings().GetButtonRepeat() );
        pTimer->Start();
    }
    else
    {
        if (mbInitialUp)
            Up();
        else
            Down();
    }
}

void SpinButton::Up()
{
    if (ImplIsUpperEnabled())
    {
        mnValue += mnValueStep;
        CompatStateChanged(StateChangedType::Data);

        ImplMoveFocus(true);
    }

    ImplCallEventListenersAndHandler(VclEventId::SpinbuttonUp, nullptr );
}

void SpinButton::Down()
{
    if (ImplIsLowerEnabled())
    {
        mnValue -= mnValueStep;
        CompatStateChanged(StateChangedType::Data);

        ImplMoveFocus(false);
    }

    ImplCallEventListenersAndHandler(VclEventId::SpinbuttonDown, nullptr );
}

void SpinButton::Resize()
{
    Control::Resize();

    Size aSize(GetOutputSizePixel());
    tools::Rectangle aRect(Point(), aSize);
    if (mbHorz)
    {
        maLowerRect = tools::Rectangle(0, 0, aSize.Width() / 2, aSize.Height() - 1);
        maUpperRect = tools::Rectangle(maLowerRect.TopRight(), aRect.BottomRight());
    }
    else
    {
        maUpperRect = tools::Rectangle(0, 0, aSize.Width() - 1, aSize.Height() / 2);
        maLowerRect = tools::Rectangle(maUpperRect.BottomLeft(), aRect.BottomRight());
    }

    ImplCalcFocusRect(ImplIsUpperEnabled() || !ImplIsLowerEnabled());

    Invalidate();
}

void SpinButton::Draw(OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags)
{
    Point aPos  = pDev->LogicToPixel(rPos);
    Size aSize = pDev->LogicToPixel(rSize);

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

    tools::Rectangle   aRect( Point( 0, 0 ), aSize );
    tools::Rectangle aLowerRect, aUpperRect;
    if ( mbHorz )
    {
        aLowerRect = tools::Rectangle( 0, 0, aSize.Width()/2, aSize.Height()-1 );
        aUpperRect = tools::Rectangle( aLowerRect.TopRight(), aRect.BottomRight() );
    }
    else
    {
        aUpperRect = tools::Rectangle( 0, 0, aSize.Width()-1, aSize.Height()/2 );
        aLowerRect = tools::Rectangle( aUpperRect.BottomLeft(), aRect.BottomRight() );
    }

    aUpperRect += aPos;
    aLowerRect += aPos;

    ImplDrawSpinButton(*pDev, this, aUpperRect, aLowerRect, false, false,
                       IsEnabled() && ImplIsUpperEnabled(),
                       IsEnabled() && ImplIsLowerEnabled(), mbHorz, true);
    pDev->Pop();
}

void SpinButton::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    HideFocus();

    bool bEnable = IsEnabled();
    ImplDrawSpinButton(rRenderContext, this, maUpperRect, maLowerRect, mbUpperIn, mbLowerIn,
                       bEnable && ImplIsUpperEnabled(),
                       bEnable && ImplIsLowerEnabled(), mbHorz, true);

    if (HasFocus())
        ShowFocus(maFocusRect);
}

void SpinButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( maUpperRect.IsInside( rMEvt.GetPosPixel() ) && ( ImplIsUpperEnabled() ) )
    {
        mbUpperIn   = true;
        mbInitialUp = true;
        Invalidate( maUpperRect );
    }
    else if ( maLowerRect.IsInside( rMEvt.GetPosPixel() ) && ( ImplIsLowerEnabled() ) )
    {
        mbLowerIn     = true;
        mbInitialDown = true;
        Invalidate( maLowerRect );
    }

    if ( mbUpperIn || mbLowerIn )
    {
        Update();
        CaptureMouse();
        if ( mbRepeat )
            maRepeatTimer.Start();
    }
}

void SpinButton::MouseButtonUp( const MouseEvent& )
{
    ReleaseMouse();
    if ( mbRepeat )
    {
        maRepeatTimer.Stop();
        maRepeatTimer.SetTimeout(MouseSettings::GetButtonStartRepeat() );
    }

    if ( mbUpperIn )
    {
        mbUpperIn   = false;
        Invalidate( maUpperRect );
        Update();
        Up();
    }
    else if ( mbLowerIn )
    {
        mbLowerIn = false;
        Invalidate( maLowerRect );
        Update();
        Down();
    }

    mbInitialUp = mbInitialDown = false;
}

void SpinButton::MouseMove( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() || (!mbInitialUp && !mbInitialDown) )
        return;

    if ( !maUpperRect.IsInside( rMEvt.GetPosPixel() ) &&
         mbUpperIn && mbInitialUp )
    {
        mbUpperIn = false;
        maRepeatTimer.Stop();
        Invalidate( maUpperRect );
        Update();
    }
    else if ( !maLowerRect.IsInside( rMEvt.GetPosPixel() ) &&
              mbLowerIn && mbInitialDown )
    {
        mbLowerIn = false;
        maRepeatTimer.Stop();
        Invalidate( maLowerRect );
        Update();
    }
    else if ( maUpperRect.IsInside( rMEvt.GetPosPixel() ) &&
              !mbUpperIn && mbInitialUp )
    {
        mbUpperIn = true;
        if ( mbRepeat )
            maRepeatTimer.Start();
        Invalidate( maUpperRect );
        Update();
    }
    else if ( maLowerRect.IsInside( rMEvt.GetPosPixel() ) &&
              !mbLowerIn && mbInitialDown )
    {
        mbLowerIn = true;
        if ( mbRepeat )
            maRepeatTimer.Start();
        Invalidate( maLowerRect );
        Update();
    }
}

void SpinButton::KeyInput( const KeyEvent& rKEvt )
{
    if ( !rKEvt.GetKeyCode().GetModifier() )
    {
        switch ( rKEvt.GetKeyCode().GetCode() )
        {
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            bool bUp = KEY_RIGHT == rKEvt.GetKeyCode().GetCode();
            if ( mbHorz && !ImplMoveFocus( bUp ) )
                bUp ? Up() : Down();
        }
        break;

        case KEY_UP:
        case KEY_DOWN:
        {
            bool bUp = KEY_UP == rKEvt.GetKeyCode().GetCode();
            if ( !mbHorz && !ImplMoveFocus( KEY_UP == rKEvt.GetKeyCode().GetCode() ) )
                bUp ? Up() : Down();
        }
        break;

        case KEY_SPACE:
            mbUpperIsFocused ? Up() : Down();
            break;

        default:
            Control::KeyInput( rKEvt );
            break;
        }
    }
    else
        Control::KeyInput( rKEvt );
}

void SpinButton::StateChanged( StateChangedType nType )
{
    switch ( nType )
    {
    case StateChangedType::Data:
    case StateChangedType::Enable:
        Invalidate();
        break;

    case StateChangedType::Style:
    {
        bool bNewRepeat = 0 != ( GetStyle() & WB_REPEAT );
        if ( bNewRepeat != mbRepeat )
        {
            if ( maRepeatTimer.IsActive() )
            {
                maRepeatTimer.Stop();
                maRepeatTimer.SetTimeout( MouseSettings::GetButtonStartRepeat() );
            }
            mbRepeat = bNewRepeat;
        }

        bool bNewHorz = 0 != ( GetStyle() & WB_HSCROLL );
        if ( bNewHorz != mbHorz )
        {
            mbHorz = bNewHorz;
            Resize();
        }
    }
    break;
    default:;
    }

    Control::StateChanged( nType );
}

void SpinButton::SetRangeMin( long nNewRange )
{
    SetRange( Range( nNewRange, GetRangeMax() ) );
}

void SpinButton::SetRangeMax( long nNewRange )
{
    SetRange( Range( GetRangeMin(), nNewRange ) );
}

void SpinButton::SetRange( const Range& rRange )
{
    // adjust rage
    Range aRange = rRange;
    aRange.Justify();
    long nNewMinRange = aRange.Min();
    long nNewMaxRange = aRange.Max();

    // do something only if old and new range differ
    if ( (mnMinRange != nNewMinRange) ||
         (mnMaxRange != nNewMaxRange) )
    {
        mnMinRange = nNewMinRange;
        mnMaxRange = nNewMaxRange;

        // adjust value to new range, if necessary
        if ( mnValue > mnMaxRange )
            mnValue = mnMaxRange;
        if ( mnValue < mnMinRange )
            mnValue = mnMinRange;

        CompatStateChanged( StateChangedType::Data );
    }
}

void SpinButton::SetValue( long nValue )
{
    // adjust, if necessary
    if ( nValue > mnMaxRange )
        nValue = mnMaxRange;
    if ( nValue < mnMinRange )
        nValue = mnMinRange;

    if ( mnValue != nValue )
    {
        mnValue = nValue;
        CompatStateChanged( StateChangedType::Data );
    }
}

void SpinButton::GetFocus()
{
    ShowFocus( maFocusRect );
    Control::GetFocus();
}

void SpinButton::LoseFocus()
{
    HideFocus();
    Control::LoseFocus();
}

bool SpinButton::ImplMoveFocus( bool _bUpper )
{
    if ( _bUpper == mbUpperIsFocused )
        return false;

    HideFocus();
    ImplCalcFocusRect( _bUpper );
    if ( HasFocus() )
        ShowFocus( maFocusRect );
    return true;
}

void SpinButton::ImplCalcFocusRect( bool _bUpper )
{
    maFocusRect = _bUpper ? maUpperRect : maLowerRect;
    // inflate by some pixels
    maFocusRect.AdjustLeft(2 );
    maFocusRect.AdjustTop(2 );
    maFocusRect.AdjustRight( -2 );
    maFocusRect.AdjustBottom( -2 );
    mbUpperIsFocused = _bUpper;
}

tools::Rectangle* SpinButton::ImplFindPartRect( const Point& rPt )
{
    if( maUpperRect.IsInside( rPt ) )
        return &maUpperRect;
    else if( maLowerRect.IsInside( rPt ) )
        return &maLowerRect;
    else
        return nullptr;
}

bool SpinButton::PreNotify( NotifyEvent& rNEvt )
{
    const MouseEvent* pMouseEvt = nullptr;

    if ((rNEvt.GetType() == MouseNotifyEvent::MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != nullptr)
    {
        if (!pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged())
        {
            // trigger redraw if mouse over state has changed
            if (IsNativeControlSupported(ControlType::Spinbox, ControlPart::Entire) ||
                IsNativeControlSupported(ControlType::Spinbox, ControlPart::AllButtons) )
            {
                tools::Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                tools::Rectangle* pLastRect = ImplFindPartRect( GetLastPointerPosPixel() );
                if (pRect != pLastRect || (pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow()))
                {
                    vcl::Region aRgn(GetActiveClipRegion());
                    if (pLastRect)
                    {
                        SetClipRegion(vcl::Region(*pLastRect));
                        Invalidate(*pLastRect);
                        SetClipRegion( aRgn );
                    }
                    if (pRect)
                    {
                        SetClipRegion(vcl::Region(*pRect));
                        Invalidate(*pRect);
                        SetClipRegion(aRgn);
                    }
                }
            }
        }
    }

    return Control::PreNotify(rNEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
