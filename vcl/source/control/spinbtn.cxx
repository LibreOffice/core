/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: spinbtn.cxx,v $
 * $Revision: 1.13 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/rcid.h>
#include <vcl/spin.h>
#include <vcl/event.hxx>
#include <vcl/spin.hxx>

// =======================================================================

void SpinButton::ImplInit( Window* pParent, WinBits nStyle )
{
    mbUpperIn     = FALSE;
    mbLowerIn     = FALSE;
    mbInitialUp   = FALSE;
    mbInitialDown = FALSE;

    mnMinRange  = 0;
    mnMaxRange  = 100;
    mnValue     = 0;
    mnValueStep = 1;

    maRepeatTimer.SetTimeout( GetSettings().GetMouseSettings().GetButtonStartRepeat() );
    maRepeatTimer.SetTimeoutHdl( LINK( this, SpinButton, ImplTimeout ) );

    mbRepeat = 0 != ( nStyle & WB_REPEAT );

    if ( nStyle & WB_HSCROLL )
        mbHorz = TRUE;
    else
        mbHorz = FALSE;

    Control::ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

SpinButton::SpinButton( Window* pParent, WinBits nStyle )
    :Control( WINDOW_SPINBUTTON )
    ,mbUpperIsFocused( FALSE )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

SpinButton::SpinButton( Window* pParent, const ResId& rResId )
    :Control( WINDOW_SPINBUTTON )
    ,mbUpperIsFocused( FALSE )
{
    rResId.SetRT( RSC_SPINBUTTON );
    ImplInit( pParent, ImplInitRes( rResId ) );
    ImplLoadRes( rResId );
    Resize();
}

// -----------------------------------------------------------------------

SpinButton::~SpinButton()
{
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void SpinButton::Up()
{
    if ( ImplIsUpperEnabled() )
    {
        mnValue += mnValueStep;
        StateChanged( STATE_CHANGE_DATA );

        ImplMoveFocus( TRUE );
    }

    ImplCallEventListenersAndHandler( VCLEVENT_SPINBUTTON_UP, maUpHdlLink, this );
}

// -----------------------------------------------------------------------

void SpinButton::Down()
{
    if ( ImplIsLowerEnabled() )
    {
        mnValue -= mnValueStep;
        StateChanged( STATE_CHANGE_DATA );

        ImplMoveFocus( FALSE );
    }

    ImplCallEventListenersAndHandler( VCLEVENT_SPINBUTTON_DOWN, maDownHdlLink, this );
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void SpinButton::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags )
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

    ImplDrawSpinButton( pDev, aUpperRect, aLowerRect, FALSE, FALSE,
                        IsEnabled() && ImplIsUpperEnabled(),
                        IsEnabled() && ImplIsLowerEnabled(), mbHorz, TRUE );
    pDev->Pop();
}


void SpinButton::Paint( const Rectangle& )
{
    HideFocus();

    BOOL bEnable = IsEnabled();
    ImplDrawSpinButton( this, maUpperRect, maLowerRect, mbUpperIn, mbLowerIn,
                        bEnable && ImplIsUpperEnabled(),
                        bEnable && ImplIsLowerEnabled(), mbHorz, TRUE );

    if ( HasFocus() )
        ShowFocus( maFocusRect );
}

// -----------------------------------------------------------------------

void SpinButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( maUpperRect.IsInside( rMEvt.GetPosPixel() ) && ( ImplIsUpperEnabled() ) )
    {
        mbUpperIn   = TRUE;
        mbInitialUp = TRUE;
        Invalidate( maUpperRect );
    }
    else if ( maLowerRect.IsInside( rMEvt.GetPosPixel() ) && ( ImplIsLowerEnabled() ) )
    {
        mbLowerIn     = TRUE;
        mbInitialDown = TRUE;
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

// -----------------------------------------------------------------------

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
        mbUpperIn   = FALSE;
        Invalidate( maUpperRect );
        Update();
        Up();
    }
    else if ( mbLowerIn )
    {
        mbLowerIn = FALSE;
        Invalidate( maLowerRect );
        Update();
        Down();
    }

    mbInitialUp = mbInitialDown = FALSE;
}

// -----------------------------------------------------------------------

void SpinButton::MouseMove( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() || (!mbInitialUp && !mbInitialDown) )
        return;

    if ( !maUpperRect.IsInside( rMEvt.GetPosPixel() ) &&
         mbUpperIn && mbInitialUp )
    {
        mbUpperIn = FALSE;
        maRepeatTimer.Stop();
        Invalidate( maUpperRect );
        Update();
    }
    else if ( !maLowerRect.IsInside( rMEvt.GetPosPixel() ) &&
              mbLowerIn & mbInitialDown )
    {
        mbLowerIn = FALSE;
        maRepeatTimer.Stop();
        Invalidate( maLowerRect );
        Update();
    }
    else if ( maUpperRect.IsInside( rMEvt.GetPosPixel() ) &&
              !mbUpperIn && mbInitialUp )
    {
        mbUpperIn = TRUE;
        if ( mbRepeat )
            maRepeatTimer.Start();
        Invalidate( maUpperRect );
        Update();
    }
    else if ( maLowerRect.IsInside( rMEvt.GetPosPixel() ) &&
              !mbLowerIn && mbInitialDown )
    {
        mbLowerIn = TRUE;
        if ( mbRepeat )
            maRepeatTimer.Start();
        Invalidate( maLowerRect );
        Update();
    }
}

// -----------------------------------------------------------------------

void SpinButton::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aCode = rKEvt.GetKeyCode();

    if ( !rKEvt.GetKeyCode().GetModifier() )
    {
        switch ( rKEvt.GetKeyCode().GetCode() )
        {
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            BOOL bUp = KEY_RIGHT == rKEvt.GetKeyCode().GetCode();
            if ( mbHorz && !ImplMoveFocus( bUp ) )
                bUp ? Up() : Down();
        }
        break;

        case KEY_UP:
        case KEY_DOWN:
        {
            BOOL bUp = KEY_UP == rKEvt.GetKeyCode().GetCode();
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

// -----------------------------------------------------------------------

void SpinButton::StateChanged( StateChangedType nType )
{
    switch ( nType )
    {
    case STATE_CHANGE_DATA:
    case STATE_CHANGE_ENABLE:
        Invalidate();
        break;

    case STATE_CHANGE_STYLE:
    {
        BOOL bNewRepeat = 0 != ( GetStyle() & WB_REPEAT );
        if ( bNewRepeat != mbRepeat )
        {
            if ( maRepeatTimer.IsActive() )
            {
                maRepeatTimer.Stop();
                maRepeatTimer.SetTimeout( GetSettings().GetMouseSettings().GetButtonStartRepeat() );
            }
            mbRepeat = bNewRepeat;
        }

        BOOL bNewHorz = 0 != ( GetStyle() & WB_HSCROLL );
        if ( bNewHorz != mbHorz )
        {
            mbHorz = bNewHorz;
            Resize();
        }
    }
    break;
    }

    Control::StateChanged( nType );
}

// -----------------------------------------------------------------------

void SpinButton::SetRangeMin( long nNewRange )
{
    SetRange( Range( nNewRange, GetRangeMax() ) );
}

// -----------------------------------------------------------------------

void SpinButton::SetRangeMax( long nNewRange )
{
    SetRange( Range( GetRangeMin(), nNewRange ) );
}

// -----------------------------------------------------------------------

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

        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

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
        StateChanged( STATE_CHANGE_DATA );
    }
}

// -----------------------------------------------------------------------

void SpinButton::GetFocus()
{
    ShowFocus( maFocusRect );
    Control::GetFocus();
}

// -----------------------------------------------------------------------

void SpinButton::LoseFocus()
{
    HideFocus();
    Control::LoseFocus();
}

// -----------------------------------------------------------------------

BOOL SpinButton::ImplMoveFocus( BOOL _bUpper )
{
    if ( _bUpper == mbUpperIsFocused )
        return FALSE;

    HideFocus();
    ImplCalcFocusRect( _bUpper );
    if ( HasFocus() )
        ShowFocus( maFocusRect );
    return TRUE;
}

// -----------------------------------------------------------------------

void SpinButton::ImplCalcFocusRect( BOOL _bUpper )
{
    maFocusRect = _bUpper ? maUpperRect : maLowerRect;
    // inflate by some pixels
    maFocusRect.Left() += 2;
    maFocusRect.Top() += 2;
    maFocusRect.Right() -= 2;
    maFocusRect.Bottom() -= 2;
    mbUpperIsFocused = _bUpper;
}

// -----------------------------------------------------------------------

Rectangle* SpinButton::ImplFindPartRect( const Point& rPt )
{
    if( maUpperRect.IsInside( rPt ) )
        return &maUpperRect;
    else if( maLowerRect.IsInside( rPt ) )
        return &maLowerRect;
    else
        return NULL;
}

long SpinButton::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0;
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
                    Region aRgn( GetActiveClipRegion() );
                    if( pLastRect )
                    {
                        SetClipRegion( *pLastRect );
                        Paint( *pLastRect );
                        SetClipRegion( aRgn );
                    }
                    if( pRect )
                    {
                        SetClipRegion( *pRect );
                        Paint( *pRect );
                        SetClipRegion( aRgn );
                    }
                }
            }
        }
    }

    return nDone ? nDone : Control::PreNotify(rNEvt);
}

// -----------------------------------------------------------------------
