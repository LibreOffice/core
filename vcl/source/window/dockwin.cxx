/*************************************************************************
 *
 *  $RCSfile: dockwin.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:58:21 $
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

#define _SV_DOCKWIN_CXX

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_BRDWIN_HXX
#include <brdwin.hxx>
#endif
#ifndef _SV_FLOATWIN_HXX
#include <floatwin.hxx>
#endif
#ifndef _SV_DOCKWIN_HXX
#include <dockwin.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif
#include <unowrap.hxx>
#include <salframe.hxx>

#pragma hdrstop

// =======================================================================

#define DOCKWIN_FLOATSTYLES         (WB_SIZEABLE | WB_MOVEABLE | WB_CLOSEABLE | WB_STANDALONE | WB_PINABLE | WB_ROLLABLE )

// =======================================================================

class ImplDockFloatWin : public FloatingWindow
{
private:
    DockingWindow*  mpDockWin;
    ULONG           mnLastTicks;
    Timer           maDockTimer;
    Point           maDockPos;
    Rectangle       maDockRect;
    BOOL            mbInMove;
    ULONG           mnLastUserEvent;

    DECL_LINK( DockingHdl, ImplDockFloatWin* );
    DECL_LINK( DockTimerHdl, ImplDockFloatWin* );
public:
    ImplDockFloatWin( Window* pParent, WinBits nWinBits,
                      DockingWindow* pDockingWin );
    ~ImplDockFloatWin();

    virtual void    Move();
    virtual void    Resize();
    virtual void    TitleButtonClick( USHORT nButton );
    virtual void    Pin();
    virtual void    Roll();
    virtual void    PopupModeEnd();
    virtual void    Resizing( Size& rSize );
    virtual BOOL    Close();

    ULONG GetLastTicks() const { return mnLastTicks; }
};

// -----------------------------------------------------------------------

ImplDockFloatWin::ImplDockFloatWin( Window* pParent, WinBits nWinBits,
                                    DockingWindow* pDockingWin ) :
        FloatingWindow( pParent, nWinBits ),
        mpDockWin( pDockingWin ),
        mnLastTicks( Time::GetSystemTicks() ),
        mbInMove( FALSE ),
        mnLastUserEvent( 0 )
{
    // Daten vom DockingWindow uebernehmen
    if ( pDockingWin )
    {
        SetSettings( pDockingWin->GetSettings() );
        Enable( pDockingWin->IsEnabled(), FALSE );
        EnableInput( pDockingWin->IsInputEnabled(), FALSE );
        AlwaysEnableInput( pDockingWin->IsAlwaysEnableInput(), FALSE );
        EnableAlwaysOnTop( pDockingWin->IsAlwaysOnTopEnabled() );
        SetActivateMode( pDockingWin->GetActivateMode() );
    }

    SetBackground();

    maDockTimer.SetTimeoutHdl( LINK( this, ImplDockFloatWin, DockTimerHdl ) );
    maDockTimer.SetTimeout( 50 );
}

// -----------------------------------------------------------------------

ImplDockFloatWin::~ImplDockFloatWin()
{
    if( mnLastUserEvent )
        Application::RemoveUserEvent( mnLastUserEvent );
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplDockFloatWin, DockTimerHdl, ImplDockFloatWin*, pWin )
{
    DBG_ASSERT( mpDockWin->IsFloatingMode(), "docktimer called but not floating" );

    maDockTimer.Stop();
    ULONG nModes = GetCurrentModButtons();
    if( ! ( nModes & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) )
    {
        BOOL bFloatMode = ! ( nModes & KEY_MOD1 );
        mpDockWin->GetParent()->ImplGetFrameWindow()->HideTracking();
        mpDockWin->EndDocking( maDockRect, bFloatMode );
    }
    else
    {
        if( !(nModes & KEY_MOD1) )
            mpDockWin->GetParent()->ImplGetFrameWindow()->HideTracking();
        else
            mpDockWin->GetParent()->ImplGetFrameWindow()->ShowTracking( maDockRect, SHOWTRACK_BIG | SHOWTRACK_WINDOW );

        maDockTimer.Start();
    }

    return 0;
}

IMPL_LINK( ImplDockFloatWin, DockingHdl, ImplDockFloatWin*, pWindow )
{
    mnLastUserEvent = 0;
    if( mpDockWin->IsDockable()                             &&
        Time::GetSystemTicks() - mnLastTicks > 500          &&
        (GetCurrentModButtons() & KEY_MOD1)
        )
    {
        maDockPos = Point( mpDockWin->GetParent()->AbsoluteScreenToOutputPixel( OutputToAbsoluteScreenPixel( GetPosPixel() ) ) );

        if( ! mpDockWin->IsDocking() )
            mpDockWin->StartDocking();
        maDockRect = Rectangle( maDockPos, mpDockWin->GetSizePixel() );
        BOOL bFloatMode = mpDockWin->Docking( maDockPos, maDockRect );
        if( ! bFloatMode )
        {
            mpDockWin->GetParent()->ImplGetFrameWindow()->ShowTracking( maDockRect, SHOWTRACK_BIG | SHOWTRACK_WINDOW );
            DockTimerHdl( this );
        }
        else
        {
            mpDockWin->GetParent()->ImplGetFrameWindow()->HideTracking();
            maDockTimer.Stop();
            mpDockWin->EndDocking( maDockRect, TRUE );
        }
    }
    mbInMove = FALSE;
    return 0;
}
// -----------------------------------------------------------------------

void ImplDockFloatWin::Move()
{
    if( mbInMove )
        return;

    mbInMove = TRUE;
    FloatingWindow::Move();
    mpDockWin->Move();

    /*
     *  note: the window should only dock if KEY_MOD1 is pressed
     *  and the user releases all mouse buttons. The real problem here
     *  is that we don't get mouse events (at least not on X)
     *  if the mouse is on the decoration. So we have to start an
     *  awkward timer based process that polls the modifier/buttons
     *  to see whether they are in the right condition shortly after the
     *  last Move message.
     */
    if( ! mnLastUserEvent )
        mnLastUserEvent = Application::PostUserEvent( LINK( this, ImplDockFloatWin, DockingHdl ) );
}

// -----------------------------------------------------------------------

void ImplDockFloatWin::Resize()
{
    FloatingWindow::Resize();
    Size aSize( GetSizePixel() );
    mpDockWin->ImplPosSizeWindow( 0, 0, aSize.Width(), aSize.Height(), WINDOW_POSSIZE_POSSIZE );
}

// -----------------------------------------------------------------------

void ImplDockFloatWin::TitleButtonClick( USHORT nButton )
{
    FloatingWindow::TitleButtonClick( nButton );
    mpDockWin->TitleButtonClick( nButton );
}

// -----------------------------------------------------------------------

void ImplDockFloatWin::Pin()
{
    FloatingWindow::Pin();
    mpDockWin->Pin();
}

// -----------------------------------------------------------------------

void ImplDockFloatWin::Roll()
{
    FloatingWindow::Roll();
    mpDockWin->Roll();
}

// -----------------------------------------------------------------------

void ImplDockFloatWin::PopupModeEnd()
{
    FloatingWindow::PopupModeEnd();
    mpDockWin->PopupModeEnd();
}

// -----------------------------------------------------------------------

void ImplDockFloatWin::Resizing( Size& rSize )
{
    FloatingWindow::Resizing( rSize );
    mpDockWin->Resizing( rSize );
}

// -----------------------------------------------------------------------

BOOL ImplDockFloatWin::Close()
{
    return mpDockWin->Close();
}

// =======================================================================

BOOL DockingWindow::ImplStartDocking( const Point& rPos )
{
    if ( !mbDockable )
        return FALSE;

    maMouseOff      = rPos;
    maMouseStart    = maMouseOff;
    mbDocking       = TRUE;
    mbLastFloatMode = IsFloatingMode();
    mbStartFloat    = mbLastFloatMode;

    // FloatingBorder berechnen
    FloatingWindow* pWin;
    if ( mpFloatWin )
        pWin = mpFloatWin;
    else
        pWin = new ImplDockFloatWin( mpParent, mnFloatBits, NULL );
    pWin->GetBorder( mnDockLeft, mnDockTop, mnDockRight, mnDockBottom );
    if ( !mpFloatWin )
        delete pWin;

    Point   aPos    = ImplOutputToFrame( Point() );
    Size    aSize   = Window::GetOutputSizePixel();
    mnTrackX        = aPos.X();
    mnTrackY        = aPos.Y();
    mnTrackWidth    = aSize.Width();
    mnTrackHeight   = aSize.Height();

    if ( mbLastFloatMode )
    {
        maMouseOff.X()  += mnDockLeft;
        maMouseOff.Y()  += mnDockTop;
        mnTrackX        -= mnDockLeft;
        mnTrackY        -= mnDockTop;
        mnTrackWidth    += mnDockLeft+mnDockRight;
        mnTrackHeight   += mnDockTop+mnDockBottom;
    }

    if ( GetSettings().GetStyleSettings().GetDragFullOptions() & DRAGFULL_OPTION_DOCKING &&
        !( mnFloatBits & ( WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE ) ) ) // no full drag when migrating to system window
        mbDragFull = TRUE;
    else
    {
        StartDocking();
        mbDragFull = FALSE;
        ImplUpdateAll();
        ImplGetFrameWindow()->ImplUpdateAll();
    }

    StartTracking( STARTTRACK_KEYMOD );
    return TRUE;
}

// =======================================================================

void DockingWindow::ImplInitData()
{
    mbDockWin               = TRUE;

    mpFloatWin              = NULL;
    mbDockCanceled          = FALSE;
    mbDockPrevented         = FALSE;
    mbFloatPrevented        = FALSE;
    mbDocking               = FALSE;
    mbPined                 = FALSE;
    mbRollUp                = FALSE;
    mbDockBtn               = FALSE;
    mbHideBtn               = FALSE;
}

// -----------------------------------------------------------------------

void DockingWindow::ImplInit( Window* pParent, WinBits nStyle )
{
    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;

    mpParent                = pParent;
    mbDockable              = (nStyle & WB_DOCKABLE) != 0;
    mnFloatBits             = WB_BORDER | (nStyle & DOCKWIN_FLOATSTYLES);
    nStyle                 &= ~(DOCKWIN_FLOATSTYLES | WB_BORDER);
    if ( nStyle & WB_DOCKBORDER )
        nStyle |= WB_BORDER;

    Window::ImplInit( pParent, nStyle, NULL );

    ImplInitSettings();
}

// -----------------------------------------------------------------------

void DockingWindow::ImplInitSettings()
{
    // Hack, damit man auch DockingWindows ohne Hintergrund bauen kann
    // und noch nicht alles umgestellt ist
    if ( IsBackground() )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else if ( Window::GetStyle() & WB_3DLOOK )
            aColor = rStyleSettings.GetFaceColor();
        else
            aColor = rStyleSettings.GetWindowColor();
        SetBackground( aColor );
    }
}

// -----------------------------------------------------------------------

void DockingWindow::ImplLoadRes( const ResId& rResId )
{
    Window::ImplLoadRes( rResId );

    USHORT  nMask = ReadShortRes();

    if ( (RSC_DOCKINGWINDOW_XYMAPMODE | RSC_DOCKINGWINDOW_X |
          RSC_DOCKINGWINDOW_Y) & nMask )
    {
        // Groessenangabe aus der Resource verwenden
        Point   aPos;
        MapUnit ePosMap = MAP_PIXEL;

        if ( RSC_DOCKINGWINDOW_XYMAPMODE & nMask )
            ePosMap = (MapUnit)ReadShortRes();

        if ( RSC_DOCKINGWINDOW_X & nMask )
        {
            aPos.X() = ReadShortRes();
            aPos.X() = ImplLogicUnitToPixelX( aPos.X(), ePosMap );
        }

        if ( RSC_DOCKINGWINDOW_Y & nMask )
        {
            aPos.Y() = ReadShortRes();
            aPos.Y() = ImplLogicUnitToPixelY( aPos.Y(), ePosMap );
        }

        SetFloatingPos( aPos );
    }

    if ( nMask & RSC_DOCKINGWINDOW_FLOATING )
    {
        if ( (BOOL)ReadShortRes() )
            SetFloatingMode( TRUE );
    }
}

// -----------------------------------------------------------------------

DockingWindow::DockingWindow( WindowType nType ) :
    Window( nType )
{
    ImplInitData();
}

// -----------------------------------------------------------------------

DockingWindow::DockingWindow( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_DOCKINGWINDOW )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

DockingWindow::DockingWindow( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_DOCKINGWINDOW )
{
    ImplInitData();
    rResId.SetRT( RSC_DOCKINGWINDOW );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

DockingWindow::~DockingWindow()
{
    if ( IsFloatingMode() )
    {
        Show( FALSE, SHOW_NOFOCUSCHANGE );
        SetFloatingMode( FALSE );
    }
}

// -----------------------------------------------------------------------

void DockingWindow::Tracking( const TrackingEvent& rTEvt )
{
    if ( mbDocking )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbDocking = FALSE;
            if ( mbDragFull )
            {
                // Bei Abbruch alten Zustand wieder herstellen
                if ( rTEvt.IsTrackingCanceled() )
                {
                    StartDocking();
                    Rectangle aRect( Point( mnTrackX, mnTrackY ), Size( mnTrackWidth, mnTrackHeight ) );
                    EndDocking( aRect, mbStartFloat );
                }
            }
            else
            {
                HideTracking();
                if ( rTEvt.IsTrackingCanceled() )
                {
                    mbDockCanceled = TRUE;
                    EndDocking( Rectangle( Point( mnTrackX, mnTrackY ), Size( mnTrackWidth, mnTrackHeight ) ), mbLastFloatMode );
                    mbDockCanceled = FALSE;
                }
                else
                    EndDocking( Rectangle( Point( mnTrackX, mnTrackY ), Size( mnTrackWidth, mnTrackHeight ) ), mbLastFloatMode );
            }
        }
        // Docking nur bei nicht synthetischen MouseEvents
        else if ( !rTEvt.GetMouseEvent().IsSynthetic() || rTEvt.GetMouseEvent().IsModifierChanged() )
        {
            Point   aMousePos = rTEvt.GetMouseEvent().GetPosPixel();
            Point   aFrameMousePos = ImplOutputToFrame( aMousePos );
            Size    aFrameSize = mpFrameWindow->GetOutputSizePixel();
            if ( aFrameMousePos.X() < 0 )
                aFrameMousePos.X() = 0;
            if ( aFrameMousePos.Y() < 0 )
                aFrameMousePos.Y() = 0;
            if ( aFrameMousePos.X() > aFrameSize.Width()-1 )
                aFrameMousePos.X() = aFrameSize.Width()-1;
            if ( aFrameMousePos.Y() > aFrameSize.Height()-1 )
                aFrameMousePos.Y() = aFrameSize.Height()-1;
            aMousePos = ImplFrameToOutput( aFrameMousePos );
            aMousePos.X() -= maMouseOff.X();
            aMousePos.Y() -= maMouseOff.Y();
            Point aPos = ImplOutputToFrame( aMousePos );
            Rectangle aTrackRect( aPos, Size( mnTrackWidth, mnTrackHeight ) );
            Rectangle aCompRect = aTrackRect;
            aPos.X()    += maMouseOff.X();
            aPos.Y()    += maMouseOff.Y();
            if ( mbDragFull )
                StartDocking();
            if ( !rTEvt.GetMouseEvent().IsMod1() )
                mbDockPrevented = TRUE;
            BOOL bFloatMode = Docking( aPos, aTrackRect );
            mbDockPrevented = FALSE;
            mbFloatPrevented = FALSE;
            if ( mbLastFloatMode != bFloatMode )
            {
                if ( bFloatMode )
                {
                    aTrackRect.Left()   -= mnDockLeft;
                    aTrackRect.Top()    -= mnDockTop;
                    aTrackRect.Right()  += mnDockRight;
                    aTrackRect.Bottom() += mnDockBottom;
                }
                else
                {
                    if ( aCompRect == aTrackRect )
                    {
                        aTrackRect.Left()   += mnDockLeft;
                        aTrackRect.Top()    += mnDockTop;
                        aTrackRect.Right()  -= mnDockRight;
                        aTrackRect.Bottom() -= mnDockBottom;
                    }
                }
                mbLastFloatMode = bFloatMode;
            }
            if ( mbDragFull )
            {
                Point aPos;
                Point aOldPos = OutputToScreenPixel( aPos );
                EndDocking( aTrackRect, mbLastFloatMode );
                // Wenn der Status bzw. die Position sich
                // geaendert hat, dann neu ausgeben
                if ( aOldPos != OutputToScreenPixel( aPos ) )
                {
                    ImplUpdateAll();
                    ImplGetFrameWindow()->ImplUpdateAll();
                }
//                EndDocking( aTrackRect, mbLastFloatMode );
            }
            else
            {
                USHORT nTrackStyle;
                if ( bFloatMode )
                    nTrackStyle = SHOWTRACK_BIG;
                else
                    nTrackStyle = SHOWTRACK_SMALL;
                Rectangle aShowTrackRect = aTrackRect;
                aShowTrackRect.SetPos( ImplFrameToOutput( aShowTrackRect.TopLeft() ) );
                ShowTracking( aShowTrackRect, nTrackStyle );

                // Maus-Offset neu berechnen, da Rechteck veraendert werden
                // konnte
                maMouseOff.X()  = aPos.X() - aTrackRect.Left();
                maMouseOff.Y()  = aPos.Y() - aTrackRect.Top();
            }

            mnTrackX        = aTrackRect.Left();
            mnTrackY        = aTrackRect.Top();
            mnTrackWidth    = aTrackRect.GetWidth();
            mnTrackHeight   = aTrackRect.GetHeight();
        }
    }
}

// -----------------------------------------------------------------------

long DockingWindow::Notify( NotifyEvent& rNEvt )
{
    if ( mbDockable )
    {
        if ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
        {
            const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
            if ( pMEvt->IsLeft() )
            {
                if ( pMEvt->GetClicks() == 2 )
                {
                    if ( pMEvt->IsMod1() )
                        SetFloatingMode( !IsFloatingMode() );
                    return TRUE;
                }
                else if ( pMEvt->GetClicks() == 1 )
                {
                    if( ! IsFloatingMode() || ! mpFloatWin->mbFrame )
                    {
                        Point   aPos = pMEvt->GetPosPixel();
                        Window* pWindow = rNEvt.GetWindow();
                        if ( pWindow != this )
                        {
                            aPos = pWindow->OutputToScreenPixel( aPos );
                            aPos = ScreenToOutputPixel( aPos );
                        }
                        if ( IsFloatingMode() || pMEvt->IsMod1() )
                            ImplStartDocking( aPos );
                    }
                    return TRUE;
                }
            }
        }
        else if( rNEvt.GetType() == EVENT_KEYINPUT )
        {
            const KeyCode& rKey = rNEvt.GetKeyEvent()->GetKeyCode();
            if( rKey.GetCode() == KEY_F10 && rKey.GetModifier() &&
                rKey.IsShift() && rKey.IsMod1() )
            {
                SetFloatingMode( !IsFloatingMode() );
                return TRUE;
            }
        }
    }

    return Window::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void DockingWindow::StartDocking()
{
    mbDocking = TRUE;
}

// -----------------------------------------------------------------------

BOOL DockingWindow::Docking( const Point&, Rectangle& )
{
    return IsFloatingMode();
}

// -----------------------------------------------------------------------

void DockingWindow::EndDocking( const Rectangle& rRect, BOOL bFloatMode )
{
    if ( !IsDockingCanceled() )
    {
        BOOL bShow = FALSE;
        if ( bFloatMode != IsFloatingMode() )
        {
            Show( FALSE, SHOW_NOFOCUSCHANGE );
            SetFloatingMode( bFloatMode );
            bShow = TRUE;
            if ( bFloatMode )
                mpFloatWin->SetPosSizePixel( rRect.TopLeft(), rRect.GetSize() );
        }
        if ( !bFloatMode )
        {
            Point aPos = rRect.TopLeft();
            aPos = GetParent()->ScreenToOutputPixel( aPos );
            Window::SetPosSizePixel( aPos, rRect.GetSize() );
        }

        if ( bShow )
            Show();
    }
    mbDocking = FALSE;
}

// -----------------------------------------------------------------------

BOOL DockingWindow::PrepareToggleFloatingMode()
{
    return TRUE;
}

// -----------------------------------------------------------------------

BOOL DockingWindow::Close()
{
    ImplDelData aDelData;
    ImplAddDel( &aDelData );
    ImplCallEventListeners( VCLEVENT_WINDOW_CLOSE );
    if ( aDelData.IsDelete() )
        return FALSE;
    ImplRemoveDel( &aDelData );

    if ( mxWindowPeer.is() && IsCreatedWithToolkit() )
        return FALSE;

    Show( FALSE, SHOW_NOFOCUSCHANGE );
    return TRUE;
}

// -----------------------------------------------------------------------

void DockingWindow::ToggleFloatingMode()
{
}

// -----------------------------------------------------------------------

void DockingWindow::TitleButtonClick( USHORT )
{
}

// -----------------------------------------------------------------------

void DockingWindow::Pin()
{
}

// -----------------------------------------------------------------------

void DockingWindow::Roll()
{
}

// -----------------------------------------------------------------------

void DockingWindow::PopupModeEnd()
{
}

// -----------------------------------------------------------------------

void DockingWindow::Resizing( Size& )
{
}

// -----------------------------------------------------------------------

void DockingWindow::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }

    Window::StateChanged( nType );
}

// -----------------------------------------------------------------------

void DockingWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
    else
        Window::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

void DockingWindow::ShowTitleButton( USHORT nButton, BOOL bVisible )
{
    if ( mpFloatWin )
        mpFloatWin->ShowTitleButton( nButton, bVisible );
    else
    {
        if ( nButton == TITLE_BUTTON_DOCKING )
            mbDockBtn = bVisible;
        else /* if ( nButton == TITLE_BUTTON_HIDE ) */
            mbHideBtn = bVisible;
    }
}

// -----------------------------------------------------------------------

BOOL DockingWindow::IsTitleButtonVisible( USHORT nButton ) const
{
    if ( mpFloatWin )
        return mpFloatWin->IsTitleButtonVisible( nButton );
    else
    {
        if ( nButton == TITLE_BUTTON_DOCKING )
            return mbDockBtn;
        else /* if ( nButton == TITLE_BUTTON_HIDE ) */
            return mbHideBtn;
    }
}

// -----------------------------------------------------------------------

void DockingWindow::SetFloatingMode( BOOL bFloatMode )
{
    if ( IsFloatingMode() != bFloatMode )
    {
        if ( PrepareToggleFloatingMode() )
        {
            BOOL bVisible = IsVisible();

            if ( bFloatMode )
            {
                Show( FALSE, SHOW_NOFOCUSCHANGE );

                maDockPos = Window::GetPosPixel();

                Window* pRealParent = mpRealParent;
                mpOldBorderWin = mpBorderWindow;

                ImplDockFloatWin* pWin =
                    new ImplDockFloatWin(
                                         mpParent,
                                         mnFloatBits & ( WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE ) ?  mnFloatBits | WB_SYSTEMWINDOW : mnFloatBits,
                                         this );
                mpFloatWin      = pWin;
                mpBorderWindow  = NULL;
                mnLeftBorder    = 0;
                mnTopBorder     = 0;
                mnRightBorder   = 0;
                mnBottomBorder  = 0;
                // Falls Parent zerstoert wird, muessen wir auch vom
                // BorderWindow den Parent umsetzen
                if ( mpOldBorderWin )
                    mpOldBorderWin->SetParent( pWin );
                SetParent( pWin );
                SetPosPixel( Point() );
                mpBorderWindow = pWin;
                pWin->mpClientWindow = this;
                mpRealParent = pRealParent;
                pWin->SetText( Window::GetText() );
                pWin->SetOutputSizePixel( Window::GetSizePixel() );
                pWin->SetPosPixel( maFloatPos );
                // DockingDaten ans FloatingWindow weiterreichen
                pWin->ShowTitleButton( TITLE_BUTTON_DOCKING, mbDockBtn );
                pWin->ShowTitleButton( TITLE_BUTTON_HIDE, mbHideBtn );
                pWin->SetPin( mbPined );
                if ( mbRollUp )
                    pWin->RollUp();
                else
                    pWin->RollDown();
                pWin->SetRollUpOutputSizePixel( maRollUpOutSize );
                pWin->SetMinOutputSizePixel( maMinOutSize );

                ToggleFloatingMode();

                if ( bVisible )
                    Show();
            }
            else
            {
                Show( FALSE, SHOW_NOFOCUSCHANGE );

                // FloatingDaten wird im FloatingWindow speichern
                maFloatPos      = mpFloatWin->GetPosPixel();
                mbDockBtn       = mpFloatWin->IsTitleButtonVisible( TITLE_BUTTON_DOCKING );
                mbHideBtn       = mpFloatWin->IsTitleButtonVisible( TITLE_BUTTON_HIDE );
                mbPined         = mpFloatWin->IsPined();
                mbRollUp        = mpFloatWin->IsRollUp();
                maRollUpOutSize = mpFloatWin->GetRollUpOutputSizePixel();
                maMinOutSize    = mpFloatWin->GetMinOutputSizePixel();

                Window* pRealParent = mpRealParent;
                mpBorderWindow = NULL;
                if ( mpOldBorderWin )
                {
                    SetParent( mpOldBorderWin );
                    ((ImplBorderWindow*)mpOldBorderWin)->GetBorder( mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder );
                    mpOldBorderWin->Resize();
                }
                mpBorderWindow = mpOldBorderWin;
                SetParent( pRealParent );
                mpRealParent = pRealParent;
                delete static_cast<ImplDockFloatWin*>(mpFloatWin);
                mpFloatWin = NULL;
                SetPosPixel( maDockPos );

                ToggleFloatingMode();

                if ( bVisible )
                    Show();
            }
        }
    }
}

// -----------------------------------------------------------------------

void DockingWindow::SetFloatStyle( WinBits nStyle )
{
    mnFloatBits = nStyle;
}

// -----------------------------------------------------------------------

WinBits DockingWindow::GetFloatStyle() const
{
    return mnFloatBits;
}

// -----------------------------------------------------------------------

void DockingWindow::SetTabStop()
{
    mnStyle |= WB_GROUP | WB_TABSTOP;
}

// -----------------------------------------------------------------------

void DockingWindow::SetPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight,
                                     USHORT nFlags )
{
    if ( mpFloatWin )
        mpFloatWin->SetPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
    else
        Window::SetPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

// -----------------------------------------------------------------------

Point DockingWindow::GetPosPixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetPosPixel();
    else
        return Window::GetPosPixel();
}

// -----------------------------------------------------------------------

Size DockingWindow::GetSizePixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetSizePixel();
    else
        return Window::GetSizePixel();
}

// -----------------------------------------------------------------------

void DockingWindow::SetOutputSizePixel( const Size& rNewSize )
{
    if ( mpFloatWin )
        mpFloatWin->SetOutputSizePixel( rNewSize );
    else
        Window::SetOutputSizePixel( rNewSize );
}

// -----------------------------------------------------------------------

Size DockingWindow::GetOutputSizePixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetOutputSizePixel();
    else
        return Window::GetOutputSizePixel();
}

Point DockingWindow::GetFloatingPos() const
{
    if ( mpFloatWin )
    {
        //Rectangle aRect = mpFloatWin->GetWindow( WINDOW_CLIENT)->GetWindowExtentsRelative( mpFloatWin->GetParent() );
        WindowStateData aData;
        aData.SetMask( WINDOWSTATE_MASK_POS );
        mpFloatWin->GetWindowStateData( aData );
        Point aPos( aData.GetX(), aData.GetY() );
        aPos = mpFloatWin->GetParent()->ImplGetFrameWindow()->AbsoluteScreenToOutputPixel( aPos );
        return aPos;
    }
    else
        return maFloatPos;
}
