/*************************************************************************
 *
 *  $RCSfile: spinfld.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:47:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_SPIN_H
#include <spin.h>
#endif
#ifndef _SV_SPINFLD_HXX
#include <spinfld.hxx>
#endif
#ifndef _VCL_CONTROLLAYOUT_HXX
#include <controllayout.hxx>
#endif

// =======================================================================

void ImplGetSpinbuttonValue( Window *pWin, const Rectangle& rUpperRect,
                            const Rectangle& rLowerRect,
                            BOOL bUpperIn, BOOL bLowerIn,
                            BOOL bUpperEnabled, BOOL bLowerEnabled, BOOL bHorz,
                            SpinbuttonValue& rValue )
{
    // convert spinbutton data to a SpinbuttonValue structure for native painting

    rValue.maUpperRect = rUpperRect;
    rValue.maLowerRect = rLowerRect;

    // convert rectangles to screen coordinates
    Point aPt = pWin->OutputToScreenPixel( rValue.maUpperRect.TopLeft() );
    rValue.maUpperRect.SetPos( aPt );
    aPt = pWin->OutputToScreenPixel( rValue.maLowerRect.TopLeft() );
    rValue.maLowerRect.SetPos( aPt );

    ControlState nState = CTRL_STATE_ENABLED;
    if ( bUpperIn )
        nState |= CTRL_STATE_PRESSED;
    if ( !pWin->IsEnabled() || !bUpperEnabled )
        nState &= ~CTRL_STATE_ENABLED;
    if ( pWin->HasFocus() )
        nState |= CTRL_STATE_FOCUSED;
    if( pWin->IsMouseOver() && rUpperRect.IsInside( pWin->GetPointerPosPixel() ) )
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
    if( pWin->IsMouseOver() && rLowerRect.IsInside( pWin->GetPointerPosPixel() ) &&
                              !rUpperRect.IsInside( pWin->GetPointerPosPixel() ) )
        nState |= CTRL_STATE_ROLLOVER;
    rValue.mnLowerState = nState;

    rValue.mnUpperPart = bHorz ? PART_BUTTON_LEFT : PART_BUTTON_UP;
    rValue.mnLowerPart = bHorz ? PART_BUTTON_RIGHT : PART_BUTTON_DOWN;
}


BOOL ImplDrawNativeSpinfield( Window *pWin, const SpinbuttonValue& rSpinbuttonValue )
{
    BOOL bNativeOK = FALSE;

    if( pWin->IsNativeControlSupported(CTRL_SPINBOX, PART_ENTIRE_CONTROL) )
    {
        ImplControlValue aControlValue;
        aControlValue.setOptionalVal( (void*) &rSpinbuttonValue );

        if( pWin->IsNativeControlSupported(CTRL_SPINBOX, rSpinbuttonValue.mnUpperPart) &&
            pWin->IsNativeControlSupported(CTRL_SPINBOX, rSpinbuttonValue.mnLowerPart) )
        {
            // only paint the embedded spin buttons, all buttons are painted at once
            bNativeOK = pWin->DrawNativeControl( CTRL_SPINBOX, PART_ALL_BUTTONS, Region(), CTRL_STATE_ENABLED,
                        aControlValue, rtl::OUString() );
        }
        else
        {
            // paint the spinbox as a whole, use borderwindow to have proper clipping
            Window *pBorder = pWin->GetWindow( WINDOW_BORDER );

            // to not overwrite everything, set the button region as clipregion to the border window
            Rectangle aClipRect( rSpinbuttonValue.maLowerRect );
            aClipRect.Union( rSpinbuttonValue.maUpperRect );

            // convert from screen space to borderwin space
            aClipRect.SetPos( pBorder->ScreenToOutputPixel(aClipRect.TopLeft()) );

            Region oldRgn( pBorder->GetClipRegion() );
            pBorder->SetClipRegion( Region( aClipRect ) );

            Point aPt;
            Size aSize( pBorder->GetOutputSizePixel() );    // the size of the border window, i.e., the whole control
            Region aRgn( Rectangle( aPt, aSize ) );
            bNativeOK = pBorder->DrawNativeControl( CTRL_SPINBOX, PART_ENTIRE_CONTROL, aRgn, CTRL_STATE_ENABLED,
                        aControlValue, rtl::OUString() );

            pBorder->SetClipRegion( oldRgn );
        }
    }
    return bNativeOK;
}

BOOL ImplDrawNativeSpinbuttons( Window *pWin, const SpinbuttonValue& rSpinbuttonValue )
{
    BOOL bNativeOK = FALSE;

    if( pWin->IsNativeControlSupported(CTRL_SPINBUTTONS, PART_ENTIRE_CONTROL) )
    {
        ImplControlValue aControlValue;
        aControlValue.setOptionalVal( (void*) &rSpinbuttonValue );

        // only paint the standalone spin buttons, all buttons are painted at once
        bNativeOK = pWin->DrawNativeControl( CTRL_SPINBUTTONS, PART_ALL_BUTTONS, Region(), CTRL_STATE_ENABLED,
                    aControlValue, rtl::OUString() );
    }
    return bNativeOK;
}

void ImplDrawSpinButton( OutputDevice* pOutDev,
                         const Rectangle& rUpperRect,
                         const Rectangle& rLowerRect,
                         BOOL bUpperIn, BOOL bLowerIn,
                         BOOL bUpperEnabled, BOOL bLowerEnabled, BOOL bHorz, BOOL bMirrorHorz )
{
    DecorationView aDecoView( pOutDev );

    USHORT nStyle = BUTTON_DRAW_NOLEFTLIGHTBORDER;
    USHORT nSymStyle = 0;

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
    USHORT nTempStyle = nStyle;
    if ( bUpperIn )
        nTempStyle |= BUTTON_DRAW_PRESSED;

    BOOL bNativeOK = FALSE;
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

void SpinField::ImplInitData()
{
    mpEdit          = NULL;
    mbSpin          = FALSE;
    mbRepeat        = FALSE;
    mbUpperIn       = FALSE;
    mbLowerIn       = FALSE;
    mbInitialUp     = FALSE;
    mbInitialDown   = FALSE;
    mbNoSelect      = FALSE;
    mbInDropDown    = FALSE;
    EnableRTL( TRUE );
}

// --------------------------------------------------------------------

void SpinField::ImplInit( Window* pParent, WinBits nWinStyle )
{
    Edit::ImplInit( pParent, nWinStyle );

    if ( nWinStyle & (WB_SPIN|WB_DROPDOWN) )
    {
        mbSpin = TRUE;

        // Some themes want external spin buttons, therefore the main
        // spinfield should not overdraw the border between its encapsulated
        // edit field and the spin buttons
        if ( (nWinStyle & WB_SPIN) &&
            IsNativeControlSupported(CTRL_SPINBOX, HAS_BACKGROUND_TEXTURE) )
        {
            SetBackground();
            mpEdit = new Edit( this, WB_NOBORDER );
            mpEdit->SetBackground();
        }
        else
            mpEdit = new Edit( this, WB_NOBORDER );

        mpEdit->EnableRTL( FALSE );
        mpEdit->SetPosPixel( Point() );
        mpEdit->Show();
        SetSubEdit( mpEdit );

        maRepeatTimer.SetTimeoutHdl( LINK( this, SpinField, ImplTimeout ) );
        maRepeatTimer.SetTimeout( GetSettings().GetMouseSettings().GetButtonStartRepeat() );
        if ( nWinStyle & WB_REPEAT )
            mbRepeat = TRUE;

        SetCompoundControl( TRUE );
    }
}

// --------------------------------------------------------------------

SpinField::SpinField( WindowType nTyp ) :
    Edit( nTyp )
{
    ImplInitData();
}

// --------------------------------------------------------------------

SpinField::SpinField( Window* pParent, WinBits nWinStyle ) :
    Edit( WINDOW_SPINFIELD )
{
    ImplInitData();
    ImplInit( pParent, nWinStyle );
}

// --------------------------------------------------------------------

SpinField::SpinField( Window* pParent, const ResId& rResId ) :
    Edit( WINDOW_SPINFIELD )
{
    ImplInitData();
    rResId.SetRT( RSC_SPINFIELD );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// --------------------------------------------------------------------

SpinField::~SpinField()
{
    delete mpEdit;
}

// --------------------------------------------------------------------

void SpinField::Up()
{
    ImplCallEventListeners( VCLEVENT_SPINFIELD_UP );
    maUpHdlLink.Call( this );
}

// --------------------------------------------------------------------

void SpinField::Down()
{
    ImplCallEventListeners( VCLEVENT_SPINFIELD_DOWN );
    maDownHdlLink.Call( this );
}

// --------------------------------------------------------------------

void SpinField::First()
{
    ImplCallEventListeners( VCLEVENT_SPINFIELD_FIRST );
    maFirstHdlLink.Call( this );
}

// --------------------------------------------------------------------

void SpinField::Last()
{
    ImplCallEventListeners( VCLEVENT_SPINFIELD_LAST );
    maLastHdlLink.Call( this );
}

// --------------------------------------------------------------------

void SpinField::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !HasFocus() && ( !mpEdit || !mpEdit->HasFocus() ) )
    {
        mbNoSelect = TRUE;
        GrabFocus();
    }

    if ( !IsReadOnly() )
    {
        if ( maUpperRect.IsInside( rMEvt.GetPosPixel() ) )
        {
            mbUpperIn   = TRUE;
            mbInitialUp = TRUE;
            Invalidate( maUpperRect );
        }
        else if ( maLowerRect.IsInside( rMEvt.GetPosPixel() ) )
        {
            mbLowerIn    = TRUE;
            mbInitialDown = TRUE;
            Invalidate( maLowerRect );
        }
        else if ( maDropDownRect.IsInside( rMEvt.GetPosPixel() ) )
        {
            // Rechts daneben liegt der DropDownButton:
            mbInDropDown = ShowDropDown( mbInDropDown ? FALSE : TRUE );
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
    mbInitialUp = mbInitialDown = FALSE;
    maRepeatTimer.Stop();
    maRepeatTimer.SetTimeout( GetSettings().GetMouseSettings().GetButtonStartRepeat() );

    if ( mbUpperIn )
    {
        mbUpperIn = FALSE;
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

    Edit::MouseButtonUp( rMEvt );
}

// --------------------------------------------------------------------

void SpinField::MouseMove( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        if ( mbInitialUp )
        {
            BOOL bNewUpperIn = maUpperRect.IsInside( rMEvt.GetPosPixel() );
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
            BOOL bNewLowerIn = maLowerRect.IsInside( rMEvt.GetPosPixel() );
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
            USHORT nMod = rKEvt.GetKeyCode().GetModifier();
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
                        mbInDropDown = ShowDropDown( TRUE );
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
        mpLayoutData = new vcl::ControlLayoutData();
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
        BOOL    bEnable = IsEnabled();
        ImplDrawSpinButton( this, maUpperRect, maLowerRect,
                            mbUpperIn, mbLowerIn, bEnable, bEnable );
    }

    if ( GetStyle() & WB_DROPDOWN )
    {
        DecorationView aView( this );

        USHORT nStyle = BUTTON_DRAW_NOLIGHTBORDER;
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

    DBG_ASSERT( !(GetStyle() & WB_DROPDOWN), "SpinField::ImplCalcButtonAreas(): SpinField with drop down not supported on all (native) platforms !" );

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

        BOOL bNativeRegionOK = FALSE;
        Region aContentUp, aContentDown;

        if ( (pDev->GetOutDevType() == OUTDEV_WINDOW) &&
            IsNativeControlSupported(CTRL_SPINBOX, PART_ENTIRE_CONTROL) )
        {
            Window *pWin = (Window*) pDev;
            Window *pBorder = pWin->GetWindow( WINDOW_BORDER );

            // get the system's spin button size
            ImplControlValue aControlValue;
            Region aBound;
            Point aPoint;

            // use the full extent of the control
            Region aArea( Rectangle( aPoint, pBorder->GetOutputSizePixel() ) );

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
            rSpinUpArea = aContentUp.GetBoundRect();
            rSpinDownArea = aContentDown.GetBoundRect();
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

        if ( GetStyle() & (WB_SPIN|WB_DROPDOWN) )
        {
            ImplCalcButtonAreas( this, aSize, maDropDownRect, maUpperRect, maLowerRect );
            aSize.Width() = maUpperRect.Left();
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
            mbRepeat = TRUE;
        else
            mbRepeat = FALSE;
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
        ImplInitSettings( TRUE, FALSE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        if ( mpEdit )
            mpEdit->SetControlForeground( GetControlForeground() );
        ImplInitSettings( FALSE, TRUE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        if ( mpEdit )
            mpEdit->SetControlBackground( GetControlBackground() );
        ImplInitSettings( FALSE, FALSE, TRUE );
        Invalidate();
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

    if( (rNEvt.GetType() == EVENT_MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) )
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

    return nDone ? nDone : Edit::PreNotify(rNEvt);
}

// -----------------------------------------------------------------------

void SpinField::EndDropDown()
{
    mbInDropDown = FALSE;
    Paint( Rectangle( Point(), GetOutputSizePixel() ) );
}

// -----------------------------------------------------------------------

BOOL SpinField::ShowDropDown( BOOL bShow )
{
    return FALSE;
}

// -----------------------------------------------------------------------

Size SpinField::CalcMinimumSize() const
{
    Size aSz = Edit::CalcMinimumSize();

    if ( GetStyle() & WB_DROPDOWN )
        aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();
    if ( GetStyle() & WB_SPIN )
        aSz.Width() += GetSettings().GetStyleSettings().GetSpinSize();

    return aSz;
}

// -----------------------------------------------------------------------

Size SpinField::CalcSize( USHORT nChars ) const
{
    Size aSz = Edit::CalcSize( nChars );

    if ( GetStyle() & WB_DROPDOWN )
        aSz.Width() += GetSettings().GetStyleSettings().GetScrollBarSize();
    if ( GetStyle() & WB_SPIN )
        aSz.Width() += GetSettings().GetStyleSettings().GetSpinSize();

    return aSz;
}

// -----------------------------------------------------------------------

USHORT SpinField::GetMaxVisChars() const
{
    long nOutWidth = mpEdit->GetOutputSizePixel().Width();
    long nCharWidth = GetTextWidth( XubString( 'x' ) );
    return nCharWidth ? (USHORT)(nOutWidth/nCharWidth) : 0;
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

void SpinField::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags )
{
    Edit::Draw( pDev, rPos, rSize, nFlags );

    WinBits nStyle = GetStyle();
    if ( !(nFlags & WINDOW_DRAW_NOCONTROLS ) && ( nStyle & (WB_SPIN|WB_DROPDOWN) ) )
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
            USHORT nStyle = BUTTON_DRAW_NOLIGHTBORDER;
            Rectangle aInnerRect = aView.DrawButton( aDD, nStyle );
            SymbolType eSymbol = SYMBOL_SPIN_DOWN;
            if ( GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_SPINUPDOWN )
                eSymbol = SYMBOL_SPIN_UPDOWN;

            nStyle = ( IsEnabled() || ( nFlags & WINDOW_DRAW_NODISABLE ) ) ? 0 : SYMBOL_DRAW_DISABLE;
            aView.DrawSymbol( aInnerRect, eSymbol, aButtonTextColor, nStyle );
        }

        if ( GetStyle() & WB_SPIN )
        {
            ImplDrawSpinButton( pDev, aUp, aDown, FALSE, FALSE, TRUE, TRUE );
        }

        pDev->Pop();
        pDev->SetSettings( aOldSettings );
    }
}
