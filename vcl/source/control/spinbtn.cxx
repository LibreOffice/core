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

#include <tools/rcid.h>
#include <vcl/spin.h>
#include <vcl/event.hxx>
#include <vcl/spin.hxx>
#include <vcl/settings.hxx>

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

    maRepeatTimer.SetTimeout( GetSettings().GetMouseSettings().GetButtonStartRepeat() );
    maRepeatTimer.SetTimeoutHdl( LINK( this, SpinButton, ImplTimeout ) );

    mbRepeat = 0 != ( nStyle & WB_REPEAT );

    if ( nStyle & WB_HSCROLL )
        mbHorz = true;
    else
        mbHorz = false;

    Control::ImplInit( pParent, nStyle, NULL );
}

SpinButton::SpinButton( vcl::Window* pParent, WinBits nStyle )
    :Control( WINDOW_SPINBUTTON )
    ,mbUpperIsFocused( false )
{
    ImplInit( pParent, nStyle );
}

SpinButton::~SpinButton()
{
}

IMPL_LINK( SpinButton, ImplTimeout, Timer*, pTimer )
{
    if ( pTimer->GetTimeout() == GetSettings().GetMouseSettings().GetButtonStartRepeat() )
    {
        pTimer->SetTimeout( GetSettings().GetMouseSettings().GetButtonRepeat() );
        pTimer->Start();
    }
    else
    {
        if ( mbInitialUp )
            Up();
        else
            Down();
    }

    return 0;
}

void SpinButton::Up()
{
    if ( ImplIsUpperEnabled() )
    {
        mnValue += mnValueStep;
        StateChanged( StateChangedType::DATA );

        ImplMoveFocus( true );
    }

    ImplCallEventListenersAndHandler( VCLEVENT_SPINBUTTON_UP, maUpHdlLink, this );
}

void SpinButton::Down()
{
    if ( ImplIsLowerEnabled() )
    {
        mnValue -= mnValueStep;
        StateChanged( StateChangedType::DATA );

        ImplMoveFocus( false );
    }

    ImplCallEventListenersAndHandler( VCLEVENT_SPINBUTTON_DOWN, maDownHdlLink, this );
}

void SpinButton::Resize()
{
    Control::Resize();

    Size aSize( GetOutputSizePixel() );
    Point aTmpPoint;
    Rectangle aRect( aTmpPoint, aSize );
    if ( mbHorz )
    {
        maLowerRect = Rectangle( 0, 0, aSize.Width()/2, aSize.Height()-1 );
        maUpperRect = Rectangle( maLowerRect.TopRight(), aRect.BottomRight() );
    }
    else
    {
        maUpperRect = Rectangle( 0, 0, aSize.Width()-1, aSize.Height()/2 );
        maLowerRect = Rectangle( maUpperRect.BottomLeft(), aRect.BottomRight() );
    }

    ImplCalcFocusRect( ImplIsUpperEnabled() || !ImplIsLowerEnabled() );

    Invalidate();
}

void SpinButton::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
    Point       aPos  = pDev->LogicToPixel( rPos );
    Size        aSize = pDev->LogicToPixel( rSize );

    pDev->Push();
    pDev->SetMapMode();
    if ( !(nFlags & WINDOW_DRAW_MONO) )
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

    Rectangle   aRect( Point( 0, 0 ), aSize );
    Rectangle aLowerRect, aUpperRect;
    if ( mbHorz )
    {
        aLowerRect = Rectangle( 0, 0, aSize.Width()/2, aSize.Height()-1 );
        aUpperRect = Rectangle( aLowerRect.TopRight(), aRect.BottomRight() );
    }
    else
    {
        aUpperRect = Rectangle( 0, 0, aSize.Width()-1, aSize.Height()/2 );
        aLowerRect = Rectangle( aUpperRect.BottomLeft(), aRect.BottomRight() );
    }

    aUpperRect += aPos;
    aLowerRect += aPos;

    ImplDrawSpinButton( pDev, aUpperRect, aLowerRect, false, false,
                        IsEnabled() && ImplIsUpperEnabled(),
                        IsEnabled() && ImplIsLowerEnabled(), mbHorz, true );
    pDev->Pop();
}

void SpinButton::Paint( const Rectangle& )
{
    HideFocus();

    bool bEnable = IsEnabled();
    ImplDrawSpinButton( this, maUpperRect, maLowerRect, mbUpperIn, mbLowerIn,
                        bEnable && ImplIsUpperEnabled(),
                        bEnable && ImplIsLowerEnabled(), mbHorz, true );

    if ( HasFocus() )
        ShowFocus( maFocusRect );
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
        maRepeatTimer.SetTimeout(GetSettings().GetMouseSettings().GetButtonStartRepeat() );
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
    case StateChangedType::DATA:
    case StateChangedType::ENABLE:
        Invalidate();
        break;

    case StateChangedType::STYLE:
    {
        bool bNewRepeat = 0 != ( GetStyle() & WB_REPEAT );
        if ( bNewRepeat != mbRepeat )
        {
            if ( maRepeatTimer.IsActive() )
            {
                maRepeatTimer.Stop();
                maRepeatTimer.SetTimeout( GetSettings().GetMouseSettings().GetButtonStartRepeat() );
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

        StateChanged( StateChangedType::DATA );
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
        StateChanged( StateChangedType::DATA );
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
    maFocusRect.Left() += 2;
    maFocusRect.Top() += 2;
    maFocusRect.Right() -= 2;
    maFocusRect.Bottom() -= 2;
    mbUpperIsFocused = _bUpper;
}

Rectangle* SpinButton::ImplFindPartRect( const Point& rPt )
{
    if( maUpperRect.IsInside( rPt ) )
        return &maUpperRect;
    else if( maLowerRect.IsInside( rPt ) )
        return &maLowerRect;
    else
        return NULL;
}

bool SpinButton::PreNotify( NotifyEvent& rNEvt )
{
    const MouseEvent* pMouseEvt = NULL;

    if( (rNEvt.GetType() == EVENT_MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != NULL )
    {
        if( !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            // trigger redraw if mouse over state has changed
            if( IsNativeControlSupported(CTRL_SPINBOX, PART_ENTIRE_CONTROL) ||
                IsNativeControlSupported(CTRL_SPINBOX, PART_ALL_BUTTONS) )
            {
                Rectangle* pRect = ImplFindPartRect( GetPointerPosPixel() );
                Rectangle* pLastRect = ImplFindPartRect( GetLastPointerPosPixel() );
                if( pRect != pLastRect || (pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow()) )
                {
                    vcl::Region aRgn( GetActiveClipRegion() );
                    if( pLastRect )
                    {
                        SetClipRegion(vcl::Region(*pLastRect));
                        Paint( *pLastRect );
                        SetClipRegion( aRgn );
                    }
                    if( pRect )
                    {
                        SetClipRegion(vcl::Region(*pRect));
                        Paint( *pRect );
                        SetClipRegion( aRgn );
                    }
                }
            }
        }
    }

    return Control::PreNotify(rNEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
