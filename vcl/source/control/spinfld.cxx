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


#include "tools/rc.h"

#include "vcl/event.hxx"
#include "vcl/decoview.hxx"
#include "vcl/spin.h"
#include "vcl/spinfld.hxx"

#include "controldata.hxx"
#include "svdata.hxx"

// =======================================================================

void ImplGetSpinbuttonValue( Window *pWin, const Rectangle& rUpperRect,
                            const Rectangle& rLowerRect,
                            sal_Bool bUpperIn, sal_Bool bLowerIn,
                            sal_Bool bUpperEnabled, sal_Bool bLowerEnabled, sal_Bool bHorz,
                            SpinbuttonValue& rValue )
{
    // convert spinbutton data to a SpinbuttonValue structure for native painting

    rValue.maUpperRect = rUpperRect;
    rValue.maLowerRect = rLowerRect;

    Point aPointerPos = pWin->GetPointerPosPixel();

    ControlState nState = CTRL_STATE_ENABLED;
    if ( bUpperIn )
        nState |= CTRL_STATE_PRESSED;
    if ( !pWin->IsEnabled() || !bUpperEnabled )
        nState &= ~CTRL_STATE_ENABLED;
    if ( pWin->HasFocus() )
        nState |= CTRL_STATE_FOCUSED;
    if( pWin->IsMouseOver() && rUpperRect.IsInside( aPointerPos ) )
        nState |= CTRL_STATE_ROLLOVER;
    rValue.mnUpperState = nState;

    nState = CTRL_STATE_ENABLED;
    if ( bLowerIn )
        nState |= CTRL_STATE_PRESSED;
    if ( !pWin->IsEnabled() || !bLowerEnabled )
        nState &= ~CTRL_STATE_ENABLED;
    if ( pWin->HasFocus() )
        nState |= CTRL_STATE_FOCUSED;
    // for overlapping spins: highlight only one
    if( pWin->IsMouseOver() && rLowerRect.IsInside( aPointerPos ) &&
                              !rUpperRect.IsInside( aPointerPos ) )
        nState |= CTRL_STATE_ROLLOVER;
    rValue.mnLowerState = nState;

    rValue.mnUpperPart = bHorz ? PART_BUTTON_LEFT : PART_BUTTON_UP;
    rValue.mnLowerPart = bHorz ? PART_BUTTON_RIGHT : PART_BUTTON_DOWN;
}


sal_Bool ImplDrawNativeSpinfield( Window *pWin, const SpinbuttonValue& rSpinbuttonValue )
{
    sal_Bool bNativeOK = sal_False;

    if( pWin->IsNativeControlSupported(CTRL_SPINBOX, PART_ENTIRE_CONTROL) &&
        // there is just no useful native support for spinfields with dropdown
        !(pWin->GetStyle() & WB_DROPDOWN) )
    {
        if( pWin->IsNativeControlSupported(CTRL_SPINBOX, rSpinbuttonValue.mnUpperPart) &&
            pWin->IsNativeControlSupported(CTRL_SPINBOX, rSpinbuttonValue.mnLowerPart) )
        {
            // only paint the embedded spin buttons, all buttons are painted at once
            bNativeOK = pWin->DrawNativeControl( CTRL_SPINBOX, PART_ALL_BUTTONS, Rectangle(), CTRL_STATE_ENABLED,
                        rSpinbuttonValue, rtl::OUString() );
        }
        else
        {
            // paint the spinbox as a whole, use borderwindow to have proper clipping
            Window *pBorder = pWin->GetWindow( WINDOW_BORDER );

            // to not overwrite everything, set the button region as clipregion to the border window
            Rectangle aClipRect( rSpinbuttonValue.maLowerRect );
            aClipRect.Union( rSpinbuttonValue.maUpperRect );

            // convert from screen space to borderwin space
            aClipRect.SetPos( pBorder->ScreenToOutputPixel(pWin->OutputToScreenPixel(aClipRect.TopLeft())) );

            Region oldRgn( pBorder->GetClipRegion() );
            pBorder->SetClipRegion( Region( aClipRect ) );

            Point aPt;
            Size aSize( pBorder->GetOutputSizePixel() );    // the size of the border window, i.e., the whole control
            Rectangle aBound, aContent;
            Rectangle aNatRgn( aPt, aSize );
            if( ! ImplGetSVData()->maNWFData.mbCanDrawWidgetAnySize &&
                pBorder->GetNativeControlRegion( CTRL_SPINBOX, PART_ENTIRE_CONTROL,
                                                 aNatRgn, 0, rSpinbuttonValue, rtl::OUString(), aBound, aContent) )
            {
                aSize = aContent.GetSize();
            }

            Rectangle aRgn( aPt, aSize );
            bNativeOK = pBorder->DrawNativeControl( CTRL_SPINBOX, PART_ENTIRE_CONTROL, aRgn, CTRL_STATE_ENABLED,
                        rSpinbuttonValue, rtl::OUString() );

            pBorder->SetClipRegion( oldRgn );
        }
    }
    return bNativeOK;
}

sal_Bool ImplDrawNativeSpinbuttons( Window *pWin, const SpinbuttonValue& rSpinbuttonValue )
{
    sal_Bool bNativeOK = sal_False;

    if( pWin->IsNativeControlSupported(CTRL_SPINBUTTONS, PART_ENTIRE_CONTROL) )
    {
        // only paint the standalone spin buttons, all buttons are painted at once
        bNativeOK = pWin->DrawNativeControl( CTRL_SPINBUTTONS, PART_ALL_BUTTONS, Rectangle(), CTRL_STATE_ENABLED,
                    rSpinbuttonValue, rtl::OUString() );
    }
    return bNativeOK;
}

void ImplDrawSpinButton( OutputDevice* pOutDev,
                         const Rectangle& rUpperRect,
                         const Rectangle& rLowerRect,
                         sal_Bool bUpperIn, sal_Bool bLowerIn,
                         sal_Bool bUpperEnabled, sal_Bool bLowerEnabled, sal_Bool bHorz, sal_Bool bMirrorHorz )
{
    DecorationView aDecoView( pOutDev );

    sal_uInt16 nStyle = BUTTON_DRAW_NOLEFTLIGHTBORDER;
    sal_uInt16 nSymStyle = 0;

    SymbolType eType1, eType2;

    const StyleSettings& rStyleSettings = pOutDev->GetSettings().GetStyleSettings();
    if ( rStyleSettings.GetOptions() & STYLE_OPTION_SPINARROW )
    {
        // arrows are only use in OS/2 look
        if ( bHorz )
        {
            eType1 = bMirrorHorz ? SYMBOL_ARROW_RIGHT : SYMBOL_ARROW_LEFT;
            eType2 = bMirrorHorz ? SYMBOL_ARROW_LEFT : SYMBOL_ARROW_RIGHT;
        }
        else
        {
            eType1 = SYMBOL_ARROW_UP;
            eType2 = SYMBOL_ARROW_DOWN;
        }
    }
    else
    {
        if ( bHorz )
        {
            eType1 = bMirrorHorz ? SYMBOL_SPIN_RIGHT : SYMBOL_SPIN_LEFT;
            eType2 = bMirrorHorz ? SYMBOL_SPIN_LEFT : SYMBOL_SPIN_RIGHT;
        }
        else
        {
            eType1 = SYMBOL_SPIN_UP;
            eType2 = SYMBOL_SPIN_DOWN;
        }
    }

    // Oberen/linken Button malen
    sal_uInt16 nTempStyle = nStyle;
    if ( bUpperIn )
        nTempStyle |= BUTTON_DRAW_PRESSED;

    sal_Bool bNativeOK = sal_False;
    Rectangle aUpRect;

    if( pOutDev->GetOutDevType() == OUTDEV_WINDOW )
    {
        Window *pWin = (Window*) pOutDev;

        // are we drawing standalone spin buttons or members of a spinfield ?
        ControlType aControl = CTRL_SPINBUTTONS;
        switch( pWin->GetType() )
        {
            case WINDOW_EDIT:
            case WINDOW_MULTILINEEDIT:
            case WINDOW_PATTERNFIELD:
            case WINDOW_METRICFIELD:
            case WINDOW_CURRENCYFIELD:
            case WINDOW_DATEFIELD:
            case WINDOW_TIMEFIELD:
            case WINDOW_LONGCURRENCYFIELD:
            case WINDOW_NUMERICFIELD:
            case WINDOW_SPINFIELD:
                aControl = CTRL_SPINBOX;
                break;
            default:
                aControl = CTRL_SPINBUTTONS;
                break;
        }

        SpinbuttonValue aValue;
        ImplGetSpinbuttonValue( pWin, rUpperRect, rLowerRect,
                                bUpperIn, bLowerIn, bUpperEnabled, bLowerEnabled,
                                bHorz, aValue );

        if( aControl == CTRL_SPINBOX )
            bNativeOK = ImplDrawNativeSpinfield( pWin, aValue );
        else if( aControl == CTRL_SPINBUTTONS )
            bNativeOK = ImplDrawNativeSpinbuttons( pWin, aValue );
    }

    if( !bNativeOK )
        aUpRect = aDecoView.DrawButton( rUpperRect, nTempStyle );

    // Unteren/rechten Button malen
    if ( bLowerIn )
        nStyle |= BUTTON_DRAW_PRESSED;
    Rectangle aLowRect;
    if( !bNativeOK )
        aLowRect = aDecoView.DrawButton( rLowerRect, nStyle );

    // Zusaetzliche Default-Kante wollen wir auch ausnutzen
    aUpRect.Left()--;
    aUpRect.Top()--;
    aUpRect.Right()++;
    aUpRect.Bottom()++;
    aLowRect.Left()--;
    aLowRect.Top()--;
    aLowRect.Right()++;
    aLowRect.Bottom()++;

    // Wir malen auch in die Kante rein, damit man etwas erkennen kann,
    // wenn das Rechteck zu klein ist
    if ( aUpRect.GetHeight() < 4 )
    {
        aUpRect.Right()++;
        aUpRect.Bottom()++;
        aLowRect.Right()++;
        aLowRect.Bottom()++;
    }

    // Symbolgroesse berechnen
    long nTempSize1 = aUpRect.GetWidth();
    long nTempSize2 = aLowRect.GetWidth();
    if ( Abs( nTempSize1-nTempSize2 ) == 1 )
    {
        if ( nTempSize1 > nTempSize2 )
            aUpRect.Left()++;
        else
            aLowRect.Left()++;
    }
    nTempSize1 = aUpRect.GetHeight();
    nTempSize2 = aLowRect.GetHeight();
    if ( Abs( nTempSize1-nTempSize2 ) == 1 )
    {
        if ( nTempSize1 > nTempSize2 )
            aUpRect.Top()++;
        else
            aLowRect.Top()++;
    }

    nTempStyle = nSymStyle;
    if ( !bUpperEnabled )
        nTempStyle |= SYMBOL_DRAW_DISABLE;
    if( !bNativeOK )
        aDecoView.DrawSymbol( aUpRect, eType1, rStyleSettings.GetButtonTextColor(), nTempStyle );

    if ( !bLowerEnabled )
        nSymStyle |= SYMBOL_DRAW_DISABLE;
    if( !bNativeOK )
        aDecoView.DrawSymbol( aLowRect, eType2, rStyleSettings.GetButtonTextColor(), nSymStyle );
}

// =======================================================================

void SpinField::ImplInitSpinFieldData()
{
    mpEdit          = NULL;
    mbSpin          = sal_False;
    mbRepeat        = sal_False;
    mbUpperIn       = sal_False;
    mbLowerIn       = sal_False;
    mbInitialUp     = sal_False;
    mbInitialDown   = sal_False;
    mbNoSelect      = sal_False;
    mbInDropDown    = sal_False;
}

// --------------------------------------------------------------------

void SpinField::ImplInit( Window* pParent, WinBits nWinStyle )
{
    Edit::ImplInit( pParent, nWinStyle );

    if ( nWinStyle & (WB_SPIN|WB_DROPDOWN) )
    {
        mbSpin = sal_True;

        // Some themes want external spin buttons, therefore the main
        // spinfield should not overdraw the border between its encapsulated
        // edit field and the spin buttons
        if ( (nWinStyle & WB_SPIN) && ImplUseNativeBorder( nWinStyle ) )
        {
            SetBackground();
            mpEdit = new Edit( this, WB_NOBORDER );
            mpEdit->SetBackground();
        }
        else
            mpEdit = new Edit( this, WB_NOBORDER );

        mpEdit->EnableRTL( sal_False );
        mpEdit->SetPosPixel( Point() );
        mpEdit->Show();
        SetSubEdit( mpEdit );

        maRepeatTimer.SetTimeoutHdl( LINK( this, SpinField, ImplTimeout ) );
        maRepeatTimer.SetTimeout( GetSettings().GetMouseSettings().GetButtonStartRepeat() );
        if ( nWinStyle & WB_REPEAT )
            mbRepeat = sal_True;

        SetCompoundControl( sal_True );
    }
}

// --------------------------------------------------------------------

SpinField::SpinField( WindowType nTyp ) :
    Edit( nTyp )
{
    ImplInitSpinFieldData();
}

// --------------------------------------------------------------------

SpinField::SpinField( Window* pParent, WinBits nWinStyle ) :
    Edit( WINDOW_SPINFIELD )
{
    ImplInitSpinFieldData();
    ImplInit( pParent, nWinStyle );
}

SpinField::SpinField( Window* pParent, const ResId& rResId ) :
    Edit( WINDOW_SPINFIELD )
{
    ImplInitSpinFieldData();
    rResId.SetRT( RSC_SPINFIELD );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

void SpinField::take_properties(Window &rOther)
{
    fprintf(stderr, "SpinField::take_properties\n");
    if (!GetParent())
    {
        ImplInitSpinFieldData();
        ImplInit(rOther.GetParent(), rOther.GetStyle());
    }

    Edit::take_properties(rOther);

    SpinField &rOtherField = static_cast<SpinField&>(rOther);
    assert(mpEdit && rOtherField.mpEdit);
    mpEdit->take_properties(*rOtherField.mpEdit);

    maUpperRect = rOtherField.maUpperRect;
    maLowerRect = rOtherField.maLowerRect;
    maDropDownRect = rOtherField.maDropDownRect;
    mbRepeat = rOtherField.mbRepeat;
    mbSpin = rOtherField.mbSpin;
    mbInitialUp = rOtherField.mbInitialUp;
    mbInitialDown = rOtherField.mbInitialDown;
    mbNoSelect = rOtherField.mbNoSelect;
    mbUpperIn = rOtherField.mbUpperIn;
    mbLowerIn = rOtherField.mbLowerIn;
    mbInDropDown = rOtherField.mbInDropDown;
    fprintf(stderr, "SpinField::take_properties %p %d\n", this, IsVisible());
}



// --------------------------------------------------------------------

SpinField::~SpinField()
{
    delete mpEdit;
}

// --------------------------------------------------------------------

void SpinField::Up()
{
    ImplCallEventListenersAndHandler( VCLEVENT_SPINFIELD_UP, maUpHdlLink, this );
}

// --------------------------------------------------------------------

void SpinField::Down()
{
    ImplCallEventListenersAndHandler( VCLEVENT_SPINFIELD_DOWN, maDownHdlLink, this );
}

// --------------------------------------------------------------------

void SpinField::First()
{
    ImplCallEventListenersAndHandler( VCLEVENT_SPINFIELD_FIRST, maFirstHdlLink, this );
}

// --------------------------------------------------------------------

void SpinField::Last()
{
    ImplCallEventListenersAndHandler( VCLEVENT_SPINFIELD_LAST, maLastHdlLink, this );
}

// --------------------------------------------------------------------

void SpinField::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !HasFocus() && ( !mpEdit || !mpEdit->HasFocus() ) )
    {
        mbNoSelect = sal_True;
        GrabFocus();
    }

    if ( !IsReadOnly() )
    {
        if ( maUpperRect.IsInside( rMEvt.GetPosPixel() ) )
        {
            mbUpperIn   = sal_True;
            mbInitialUp = sal_True;
            Invalidate( maUpperRect );
        }
        else if ( maLowerRect.IsInside( rMEvt.GetPosPixel() ) )
        {
            mbLowerIn    = sal_True;
            mbInitialDown = sal_True;
            Invalidate( maLowerRect );
        }
        else if ( maDropDownRect.IsInside( rMEvt.GetPosPixel() ) )
        {
            // Rechts daneben liegt der DropDownButton:
            mbInDropDown = ShowDropDown( mbInDropDown ? sal_False : sal_True );
            Paint( Rectangle( Point(), GetOutputSizePixel() ) );
        }

        if ( mbUpperIn || mbLowerIn )
        {
            Update();
            CaptureMouse();
            if ( mbRepeat )
                maRepeatTimer.Start();
            return;
        }
    }

    Edit::MouseButtonDown( rMEvt );
}

// --------------------------------------------------------------------

void SpinField::MouseButtonUp( const MouseEvent& rMEvt )
{
    ReleaseMouse();
    mbInitialUp = mbInitialDown = sal_False;
    maRepeatTimer.Stop();
    maRepeatTimer.SetTimeout( GetSettings().GetMouseSettings().GetButtonStartRepeat() );

    if ( mbUpperIn )
    {
        mbUpperIn = sal_False;
        Invalidate( maUpperRect );
        Update();
        Up();
    }
    else if ( mbLowerIn )
    {
        mbLowerIn = sal_False;
        Invalidate( maLowerRect );
        Update();
        Down();
    }

    Edit::MouseButtonUp( rMEvt );
}

// --------------------------------------------------------------------

void SpinField::MouseMove( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        if ( mbInitialUp )
        {
            sal_Bool bNewUpperIn = maUpperRect.IsInside( rMEvt.GetPosPixel() );
            if ( bNewUpperIn != mbUpperIn )
            {
                if ( bNewUpperIn )
                {
                    if ( mbRepeat )
                        maRepeatTimer.Start();
                }
                else
                    maRepeatTimer.Stop();

                mbUpperIn = bNewUpperIn;
                Invalidate( maUpperRect );
                Update();
            }
        }
        else if ( mbInitialDown )
        {
            sal_Bool bNewLowerIn = maLowerRect.IsInside( rMEvt.GetPosPixel() );
            if ( bNewLowerIn != mbLowerIn )
            {
                if ( bNewLowerIn )
                {
                    if ( mbRepeat )
                        maRepeatTimer.Start();
                }
                else
                    maRepeatTimer.Stop();

                mbLowerIn = bNewLowerIn;
                Invalidate( maLowerRect );
                Update();
            }
        }
    }

    Edit::MouseMove( rMEvt );
}

// --------------------------------------------------------------------

long SpinField::Notify( NotifyEvent& rNEvt )
{
    long nDone = 0;
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent& rKEvt = *rNEvt.GetKeyEvent();
        if ( !IsReadOnly() )
        {
            sal_uInt16 nMod = rKEvt.GetKeyCode().GetModifier();
            switch ( rKEvt.GetKeyCode().GetCode() )
            {
                case KEY_UP:
                {
                    if ( !nMod )
                    {
                        Up();
                        nDone = 1;
                    }
                }
                break;
                case KEY_DOWN:
                {
                    if ( !nMod )
                    {
                        Down();
                        nDone = 1;
                    }
                    else if ( ( nMod == KEY_MOD2 ) && !mbInDropDown && ( GetStyle() & WB_DROPDOWN ) )
                    {
                        mbInDropDown = ShowDropDown( sal_True );
                        Paint( Rectangle( Point(), GetOutputSizePixel() ) );
                        nDone = 1;
                    }
                }
                break;
                case KEY_PAGEUP:
                {
                    if ( !nMod )
                    {
                        Last();
                        nDone = 1;
                    }
                }
                break;
                case KEY_PAGEDOWN:
                {
                    if ( !nMod )
                    {
                        First();
                        nDone = 1;
                    }
                }
                break;
            }
        }
    }

    if ( rNEvt.GetType() == EVENT_COMMAND )
    {
        if ( ( rNEvt.GetCommandEvent()->GetCommand() == COMMAND_WHEEL ) && !IsReadOnly() )
        {
            sal_uInt16 nWheelBehavior( GetSettings().GetMouseSettings().GetWheelBehavior() );
            if  (   ( nWheelBehavior == MOUSE_WHEEL_ALWAYS )
                ||  (   ( nWheelBehavior == MOUSE_WHEEL_FOCUS_ONLY )
                    &&  HasChildPathFocus()
                    )
                )
            {
                const CommandWheelData* pData = rNEvt.GetCommandEvent()->GetWheelData();
                if ( pData->GetMode() == COMMAND_WHEEL_SCROLL )
                {
                    if ( pData->GetDelta() < 0L )
                        Down();
                    else
                        Up();
                    nDone = 1;
                }
            }
            else
                nDone = 0;  // don't eat this event, let the default handling happen (i.e. scroll the context)
        }
    }

    return nDone ? nDone : Edit::Notify( rNEvt );
}

// --------------------------------------------------------------------

void SpinField::Command( const CommandEvent& rCEvt )
{
    Edit::Command( rCEvt );
}

// --------------------------------------------------------------------

void SpinField::FillLayoutData() const
{
    if( mbSpin )
    {
        mpControlData->mpLayoutData = new vcl::ControlLayoutData();
        AppendLayoutData( *GetSubEdit() );
        GetSubEdit()->SetLayoutDataParent( this );
    }
    else
        Edit::FillLayoutData();
}

// --------------------------------------------------------------------

void SpinField::Paint( const Rectangle& rRect )
{
    if ( mbSpin )
    {
        sal_Bool    bEnable = IsEnabled();
        ImplDrawSpinButton( this, maUpperRect, maLowerRect,
                            mbUpperIn, mbLowerIn, bEnable, bEnable );
    }

    if ( GetStyle() & WB_DROPDOWN )
    {
        DecorationView aView( this );

        sal_uInt16 nStyle = BUTTON_DRAW_NOLIGHTBORDER;
        if ( mbInDropDown )
            nStyle |= BUTTON_DRAW_PRESSED;
        Rectangle aInnerRect = aView.DrawButton( maDropDownRect, nStyle );

        SymbolType eSymbol = SYMBOL_SPIN_DOWN;
        if ( GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_SPINUPDOWN )
            eSymbol = SYMBOL_SPIN_UPDOWN;

        nStyle = IsEnabled() ? 0 : SYMBOL_DRAW_DISABLE;
        aView.DrawSymbol( aInnerRect, eSymbol, GetSettings().GetStyleSettings().GetButtonTextColor(), nStyle );
    }

    Edit::Paint( rRect );
}

// --------------------------------------------------------------------

void SpinField::ImplCalcButtonAreas( OutputDevice* pDev, const Size& rOutSz, Rectangle& rDDArea, Rectangle& rSpinUpArea, Rectangle& rSpinDownArea )
{
    const StyleSettings& rStyleSettings = pDev->GetSettings().GetStyleSettings();

    Size aSize = rOutSz;
    Size aDropDownSize;

    if ( GetStyle() & WB_DROPDOWN )
    {
        long nW = rStyleSettings.GetScrollBarSize();
        nW = GetDrawPixel( pDev, nW );
        aDropDownSize = Size( CalcZoom( nW ), aSize.Height() );
        aSize.Width() -= aDropDownSize.Width();
        rDDArea = Rectangle( Point( aSize.Width(), 0 ), aDropDownSize );
        rDDArea.Top()--;
    }
    else
        rDDArea.SetEmpty();

    // Je nach Hoehe, die groessen Berechnen
    if ( GetStyle() & WB_SPIN )
    {
        long nBottom1 = aSize.Height()/2;
        long nBottom2 = aSize.Height()-1;
        long nTop2 = nBottom1;
        long nTop1 = 0;
        if ( !(aSize.Height() & 0x01) )
            nBottom1--;

        sal_Bool bNativeRegionOK = sal_False;
        Rectangle aContentUp, aContentDown;

        if ( (pDev->GetOutDevType() == OUTDEV_WINDOW) &&
            // there is just no useful native support for spinfields with dropdown
            ! (GetStyle() & WB_DROPDOWN) &&
            IsNativeControlSupported(CTRL_SPINBOX, PART_ENTIRE_CONTROL) )
        {
            Window *pWin = (Window*) pDev;
            Window *pBorder = pWin->GetWindow( WINDOW_BORDER );

            // get the system's spin button size
            ImplControlValue aControlValue;
            Rectangle aBound;
            Point aPoint;

            // use the full extent of the control
            Rectangle aArea( aPoint, pBorder->GetOutputSizePixel() );

            bNativeRegionOK =
                pWin->GetNativeControlRegion(CTRL_SPINBOX, PART_BUTTON_UP,
                    aArea, 0, aControlValue, rtl::OUString(), aBound, aContentUp) &&
                pWin->GetNativeControlRegion(CTRL_SPINBOX, PART_BUTTON_DOWN,
                    aArea, 0, aControlValue, rtl::OUString(), aBound, aContentDown);

            if( bNativeRegionOK )
            {
                // convert back from border space to local coordinates
                aPoint = pBorder->ScreenToOutputPixel( pWin->OutputToScreenPixel( aPoint ) );
                aContentUp.Move(-aPoint.X(), -aPoint.Y());
                aContentDown.Move(-aPoint.X(), -aPoint.Y());
            }
        }

        if( bNativeRegionOK )
        {
            rSpinUpArea = aContentUp;
            rSpinDownArea = aContentDown;
        }
        else
        {
            aSize.Width() -= CalcZoom( GetDrawPixel( pDev, rStyleSettings.GetSpinSize() ) );

            rSpinUpArea = Rectangle( aSize.Width(), nTop1, rOutSz.Width()-aDropDownSize.Width()-1, nBottom1 );
            rSpinDownArea = Rectangle( rSpinUpArea.Left(), nTop2, rSpinUpArea.Right(), nBottom2 );
        }
    }
    else
    {
        rSpinUpArea.SetEmpty();
        rSpinDownArea.SetEmpty();
    }
}

// --------------------------------------------------------------------

void SpinField::Resize()
{
    if ( mbSpin )
    {
        Control::Resize();
        Size aSize = GetOutputSizePixel();
        bool bSubEditPositioned = false;

        if ( GetStyle() & (WB_SPIN|WB_DROPDOWN) )
        {
            ImplCalcButtonAreas( this, aSize, maDropDownRect, maUpperRect, maLowerRect );

            ImplControlValue aControlValue;
            Point aPoint;
            Rectangle aContent, aBound;

            // use the full extent of the control
            Window *pBorder = GetWindow( WINDOW_BORDER );
            Rectangle aArea( aPoint, pBorder->GetOutputSizePixel() );

            // adjust position and size of the edit field
            if ( GetNativeControlRegion(CTRL_SPINBOX, PART_SUB_EDIT,
                        aArea, 0, aControlValue, rtl::OUString(), aBound, aContent) )
            {
                // convert back from border space to local coordinates
                aPoint = pBorder->ScreenToOutputPixel( OutputToScreenPixel( aPoint ) );
                aContent.Move(-aPoint.X(), -aPoint.Y());

                // use the themes drop down size
                mpEdit->SetPosPixel( aContent.TopLeft() );
                bSubEditPositioned = true;
                aSize = aContent.GetSize();
            }
            else
            {
                if ( maUpperRect.IsEmpty() )
                {
                    DBG_ASSERT( !maDropDownRect.IsEmpty(), "SpinField::Resize: SPIN && DROPDOWN, but all empty rects?" );
                    aSize.Width() = maDropDownRect.Left();
                }
                else
                    aSize.Width() = maUpperRect.Left();
            }
        }

        if( ! bSubEditPositioned )
        {
            // this moves our sub edit if RTL gets switched
            mpEdit->SetPosPixel( Point() );
        }
        mpEdit->SetSizePixel( aSize );

        if ( GetStyle() & WB_SPIN )
            Invalidate( Rectangle( maUpperRect.TopLeft(), maLowerRect.BottomRight() ) );
        if ( GetStyle() & WB_DROPDOWN )
            Invalidate( maDropDownRect );
    }
}

// -----------------------------------------------------------------------

void SpinField::StateChanged( StateChangedType nType )
{
    Edit::StateChanged( nType );

    if ( nType == STATE_CHANGE_ENABLE )
    {
        if ( mbSpin || ( GetStyle() & WB_DROPDOWN ) )
        {
            mpEdit->Enable( IsEnabled() );

            if ( mbSpin )
            {
                Invalidate( maLowerRect );
                Invalidate( maUpperRect );
            }
            if ( GetStyle() & WB_DROPDOWN )
                Invalidate( maDropDownRect );
        }
    }
    else if ( nType == STATE_CHANGE_STYLE )
    {
        if ( GetStyle() & WB_REPEAT )
            mbRepeat = sal_True;
        else
            mbRepeat = sal_False;
    }
    else if ( nType == STATE_CHANGE_ZOOM )
    {
        Resize();
        if ( mpEdit )
            mpEdit->SetZoom( GetZoom() );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFONT )
    {
        if ( mpEdit )
            mpEdit->SetControlFont( GetControlFont() );
        ImplInitSettings( sal_True, sal_False, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        if ( mpEdit )
            mpEdit->SetControlForeground( GetControlForeground() );
        ImplInitSettings( sal_False, sal_True, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        if ( mpEdit )
            mpEdit->SetControlBackground( GetControlBackground() );
        ImplInitSettings( sal_False, sal_False, sal_True );
        Invalidate();
    }
    else if( nType == STATE_CHANGE_MIRRORING )
    {
        if( mpEdit )
            mpEdit->StateChanged( STATE_CHANGE_MIRRORING );
        Resize();
    }
}

// -----------------------------------------------------------------------

void SpinField::DataChanged( const DataChangedEvent& rDCEvt )
{
    Edit::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        Resize();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

Rectangle* SpinField::ImplFindPartRect( const Point& rPt )
{
    if( maUpperRect.IsInside( rPt ) )
        return &maUpperRect;
    else if( maLowerRect.IsInside( rPt ) )
        return &maLowerRect;
    else
        return NULL;
}

long SpinField::PreNotify( NotifyEvent& rNEvt )
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
                    // FIXME: this is currently only on aqua
                    // check for other platforms that need similar handling
                    if( ImplGetSVData()->maNWFData.mbNoFocusRects &&
                        IsNativeWidgetEnabled() &&
                        IsNativeControlSupported( CTRL_EDITBOX, PART_ENTIRE_CONTROL ) )
                    {
                        ImplInvalidateOutermostBorder( this );
                    }
                    else
                    {
                        // paint directly
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
    }

    return nDone ? nDone : Edit::PreNotify(rNEvt);
}

// -----------------------------------------------------------------------

void SpinField::EndDropDown()
{
    mbInDropDown = sal_False;
    Paint( Rectangle( Point(), GetOutputSizePixel() ) );
}

// -----------------------------------------------------------------------

sal_Bool SpinField::ShowDropDown( sal_Bool )
{
    return sal_False;
}

Size SpinField::CalcMinimumSizeForText(const rtl::OUString &rString) const
{
    Size aSz = Edit::CalcMinimumSizeForText(rString);

    if ( GetStyle() & WB_DROPDOWN )
        aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();
    if ( GetStyle() & WB_SPIN )
        aSz.Width() += maUpperRect.GetWidth();

    return aSz;
}

Size SpinField::CalcMinimumSize() const
{
    fprintf(stderr, "SpinField::CalcMinimumSize\n");
    return CalcMinimumSizeForText(GetText());
}

// -----------------------------------------------------------------------

Size SpinField::GetOptimalSize(WindowSizeType eType) const
{
    switch (eType) {
    case WINDOWSIZE_MINIMUM:
        return CalcMinimumSize();
    default:
        return Edit::GetOptimalSize( eType );
    }
}

// -----------------------------------------------------------------------

Size SpinField::CalcSize( sal_uInt16 nChars ) const
{
    Size aSz = Edit::CalcSize( nChars );

    if ( GetStyle() & WB_DROPDOWN )
        aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();
    if ( GetStyle() & WB_SPIN )
        aSz.Width() += GetSettings().GetStyleSettings().GetSpinSize();

    return aSz;
}

// --------------------------------------------------------------------

IMPL_LINK( SpinField, ImplTimeout, Timer*, pTimer )
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

void SpinField::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
    Edit::Draw( pDev, rPos, rSize, nFlags );

    WinBits nFieldStyle = GetStyle();
    if ( !(nFlags & WINDOW_DRAW_NOCONTROLS ) && ( nFieldStyle & (WB_SPIN|WB_DROPDOWN) ) )
    {
        Point aPos = pDev->LogicToPixel( rPos );
        Size aSize = pDev->LogicToPixel( rSize );
        OutDevType eOutDevType = pDev->GetOutDevType();
        AllSettings aOldSettings = pDev->GetSettings();

        pDev->Push();
        pDev->SetMapMode();

        if ( eOutDevType == OUTDEV_PRINTER )
        {
            StyleSettings aStyleSettings = aOldSettings.GetStyleSettings();
            aStyleSettings.SetFaceColor( COL_LIGHTGRAY );
            aStyleSettings.SetButtonTextColor( COL_BLACK );
            AllSettings aSettings( aOldSettings );
            aSettings.SetStyleSettings( aStyleSettings );
            pDev->SetSettings( aSettings );
        }

        Rectangle aDD, aUp, aDown;
        ImplCalcButtonAreas( pDev, aSize, aDD, aUp, aDown );
        aDD.Move( aPos.X(), aPos.Y() );
        aUp.Move( aPos.X(), aPos.Y() );
        aUp.Top()++;
        aDown.Move( aPos.X(), aPos.Y() );

        Color aButtonTextColor;
        if ( ( nFlags & WINDOW_DRAW_MONO ) || ( eOutDevType == OUTDEV_PRINTER ) )
            aButtonTextColor = Color( COL_BLACK );
        else
            aButtonTextColor = GetSettings().GetStyleSettings().GetButtonTextColor();

        if ( GetStyle() & WB_DROPDOWN )
        {
            DecorationView aView( pDev );
            sal_uInt16 nStyle = BUTTON_DRAW_NOLIGHTBORDER;
            Rectangle aInnerRect = aView.DrawButton( aDD, nStyle );
            SymbolType eSymbol = SYMBOL_SPIN_DOWN;
            if ( GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_SPINUPDOWN )
                eSymbol = SYMBOL_SPIN_UPDOWN;

            nStyle = ( IsEnabled() || ( nFlags & WINDOW_DRAW_NODISABLE ) ) ? 0 : SYMBOL_DRAW_DISABLE;
            aView.DrawSymbol( aInnerRect, eSymbol, aButtonTextColor, nStyle );
        }

        if ( GetStyle() & WB_SPIN )
        {
            ImplDrawSpinButton( pDev, aUp, aDown, sal_False, sal_False, sal_True, sal_True );
        }

        pDev->Pop();
        pDev->SetSettings( aOldSettings );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
