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


#include <tools/time.hxx>
#include <tools/rc.h>
#include <vcl/event.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/settings.hxx>

#include <svdata.hxx>
#include <window.h>
#include <brdwin.hxx>
#include <salframe.hxx>



// =======================================================================

#define DOCKWIN_FLOATSTYLES         (WB_SIZEABLE | WB_MOVEABLE | WB_CLOSEABLE | WB_STANDALONE | WB_PINABLE | WB_ROLLABLE )

// =======================================================================

// -----------------------------------------------------------------------

class DockingWindow::ImplData
{
public:
    ImplData();
    ~ImplData();

    Window*         mpParent;
    Size            maMaxOutSize;
};

DockingWindow::ImplData::ImplData()
{
    mpParent = NULL;
    maMaxOutSize = Size( SHRT_MAX, SHRT_MAX );
}

DockingWindow::ImplData::~ImplData()
{
}

// -----------------------------------------------------------------------

class ImplDockFloatWin : public FloatingWindow
{
private:
    DockingWindow*  mpDockWin;
    sal_uLong           mnLastTicks;
    Timer           maDockTimer;
    Point           maDockPos;
    Rectangle       maDockRect;
    bool            mbInMove;
    sal_uLong           mnLastUserEvent;

    DECL_LINK(DockingHdl, void *);
    DECL_LINK(DockTimerHdl, void *);
public:
    ImplDockFloatWin( Window* pParent, WinBits nWinBits,
                      DockingWindow* pDockingWin );
    ~ImplDockFloatWin();

    virtual void    Move();
    virtual void    Resize();
    virtual void    TitleButtonClick( sal_uInt16 nButton );
    virtual void    Pin();
    virtual void    Roll();
    virtual void    PopupModeEnd();
    virtual void    Resizing( Size& rSize );
    virtual bool    Close();

    sal_uLong GetLastTicks() const { return mnLastTicks; }
};


ImplDockFloatWin::ImplDockFloatWin( Window* pParent, WinBits nWinBits,
                                    DockingWindow* pDockingWin ) :
        FloatingWindow( pParent, nWinBits ),
        mpDockWin( pDockingWin ),
        mnLastTicks( Time::GetSystemTicks() ),
        mbInMove( false ),
        mnLastUserEvent( 0 )
{
    // Daten vom DockingWindow uebernehmen
    if ( pDockingWin )
    {
        SetSettings( pDockingWin->GetSettings() );
        Enable( pDockingWin->IsEnabled(), false );
        EnableInput( pDockingWin->IsInputEnabled(), false );
        AlwaysEnableInput( pDockingWin->IsAlwaysEnableInput(), false );
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

IMPL_LINK_NOARG(ImplDockFloatWin, DockTimerHdl)
{
    DBG_ASSERT( mpDockWin->IsFloatingMode(), "docktimer called but not floating" );

    maDockTimer.Stop();
    PointerState aState = GetPointerState();

    if( aState.mnState & KEY_MOD1 )
    {
        // i43499 CTRL disables docking now
        mpDockWin->GetParent()->ImplGetFrameWindow()->HideTracking();
        mpDockWin->EndDocking( maDockRect, sal_True );
        if( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) )
            maDockTimer.Start();
    }
    else if( ! ( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) )
    {
        mpDockWin->GetParent()->ImplGetFrameWindow()->HideTracking();
        mpDockWin->EndDocking( maDockRect, sal_False );
    }
    else
    {
        mpDockWin->GetParent()->ImplGetFrameWindow()->ShowTracking( maDockRect, SHOWTRACK_BIG | SHOWTRACK_WINDOW );
        maDockTimer.Start();
    }

    return 0;
}

IMPL_LINK_NOARG(ImplDockFloatWin, DockingHdl)
{
    PointerState aState = mpDockWin->GetParent()->GetPointerState();

    mnLastUserEvent = 0;
    if( mpDockWin->IsDockable()                             &&
        (Time::GetSystemTicks() - mnLastTicks > 500)        &&
        ( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) &&
        !(aState.mnState & KEY_MOD1) )  // i43499 CTRL disables docking now
    {
        maDockPos = Point( mpDockWin->GetParent()->AbsoluteScreenToOutputPixel( OutputToAbsoluteScreenPixel( Point() ) ) );
        maDockPos = mpDockWin->GetParent()->OutputToScreenPixel( maDockPos );  // sfx expects screen coordinates

        if( ! mpDockWin->IsDocking() )
            mpDockWin->StartDocking();
        maDockRect = Rectangle( maDockPos, mpDockWin->GetSizePixel() );

        // mouse pos also in screen pixels
        Point aMousePos = mpDockWin->GetParent()->OutputToScreenPixel( aState.maPos );

        bool bFloatMode = mpDockWin->Docking( aMousePos, maDockRect );
        if( ! bFloatMode )
        {
            mpDockWin->GetParent()->ImplGetFrameWindow()->ShowTracking( maDockRect, SHOWTRACK_OBJECT | SHOWTRACK_WINDOW );
            DockTimerHdl( this );
        }
        else
        {
            mpDockWin->GetParent()->ImplGetFrameWindow()->HideTracking();
            maDockTimer.Stop();
            mpDockWin->EndDocking( maDockRect, sal_True );
        }
    }
    mbInMove = false;
    return 0;
}
// -----------------------------------------------------------------------

void ImplDockFloatWin::Move()
{
    if( mbInMove )
        return;

    mbInMove = true;
    FloatingWindow::Move();
    mpDockWin->Move();

    /*
     *  note: the window should only dock if
     *  the user releases all mouse buttons. The real problem here
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

void ImplDockFloatWin::TitleButtonClick( sal_uInt16 nButton )
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

bool ImplDockFloatWin::Close()
{
    return mpDockWin->Close();
}

// =======================================================================

bool DockingWindow::ImplStartDocking( const Point& rPos )
{
    if ( !mbDockable )
        return false;

    maMouseOff      = rPos;
    maMouseStart    = maMouseOff;
    mbDocking       = true;
    mbLastFloatMode = IsFloatingMode();
    mbStartFloat    = mbLastFloatMode;

    // FloatingBorder berechnen
    FloatingWindow* pWin;
    if ( mpFloatWin )
        pWin = mpFloatWin;
    else
        pWin = new ImplDockFloatWin( mpImplData->mpParent, mnFloatBits, NULL );
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
        mbDragFull = true;
    else
    {
        StartDocking();
        mbDragFull = false;
        ImplUpdateAll();
        ImplGetFrameWindow()->ImplUpdateAll();
    }

    StartTracking( STARTTRACK_KEYMOD );
    return true;
}

// =======================================================================

void DockingWindow::ImplInitDockingWindowData()
{
    mpImplData              = new ImplData;
    mpWindowImpl->mbDockWin               = true;

    mpFloatWin              = NULL;
    mbDockCanceled          = false;
    mbDockPrevented         = false;
    mbFloatPrevented        = false;
    mbDocking               = false;
    mbPined                 = false;
    mbRollUp                = false;
    mbDockBtn               = false;
    mbHideBtn               = false;
}

// -----------------------------------------------------------------------

void DockingWindow::ImplInit( Window* pParent, WinBits nStyle )
{
    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;

    mpImplData->mpParent    = pParent;
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

    sal_uLong  nMask = ReadLongRes();

    if ( (RSC_DOCKINGWINDOW_XYMAPMODE | RSC_DOCKINGWINDOW_X |
          RSC_DOCKINGWINDOW_Y) & nMask )
    {
        // Groessenangabe aus der Resource verwenden
        Point   aPos;
        MapUnit ePosMap = MAP_PIXEL;

        if ( RSC_DOCKINGWINDOW_XYMAPMODE & nMask )
            ePosMap = (MapUnit)ReadLongRes();

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
        if ( ReadShortRes() != 0 )
            SetFloatingMode( true );
    }
}

// -----------------------------------------------------------------------

DockingWindow::DockingWindow( WindowType nType ) :
    Window( nType )
{
    ImplInitDockingWindowData();
}

// -----------------------------------------------------------------------

DockingWindow::DockingWindow( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_DOCKINGWINDOW )
{
    ImplInitDockingWindowData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

DockingWindow::DockingWindow( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_DOCKINGWINDOW )
{
    ImplInitDockingWindowData();
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
        Show( false, SHOW_NOFOCUSCHANGE );
        SetFloatingMode( false );
    }
    delete mpImplData;
}

// -----------------------------------------------------------------------

void DockingWindow::Tracking( const TrackingEvent& rTEvt )
{
    if( GetDockingManager()->IsDockable( this ) )   // new docking interface
        return Window::Tracking( rTEvt );

    if ( mbDocking )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbDocking = false;
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
                    mbDockCanceled = true;
                    EndDocking( Rectangle( Point( mnTrackX, mnTrackY ), Size( mnTrackWidth, mnTrackHeight ) ), mbLastFloatMode );
                    mbDockCanceled = false;
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
            Size    aFrameSize = mpWindowImpl->mpFrameWindow->GetOutputSizePixel();
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
            Point aFramePos = ImplOutputToFrame( aMousePos );
            Rectangle aTrackRect( aFramePos, Size( mnTrackWidth, mnTrackHeight ) );
            Rectangle aCompRect = aTrackRect;
            aFramePos.X()    += maMouseOff.X();
            aFramePos.Y()    += maMouseOff.Y();
            if ( mbDragFull )
                StartDocking();
            bool bFloatMode = Docking( aFramePos, aTrackRect );
            mbDockPrevented = false;
            mbFloatPrevented = false;
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
                sal_uInt16 nTrackStyle;
                if ( bFloatMode )
                    nTrackStyle = SHOWTRACK_BIG;
                else
                    nTrackStyle = SHOWTRACK_OBJECT;
                Rectangle aShowTrackRect = aTrackRect;
                aShowTrackRect.SetPos( ImplFrameToOutput( aShowTrackRect.TopLeft() ) );
                ShowTracking( aShowTrackRect, nTrackStyle );

                // Maus-Offset neu berechnen, da Rechteck veraendert werden
                // konnte
                maMouseOff.X()  = aFramePos.X() - aTrackRect.Left();
                maMouseOff.Y()  = aFramePos.Y() - aTrackRect.Top();
            }

            mnTrackX        = aTrackRect.Left();
            mnTrackY        = aTrackRect.Top();
            mnTrackWidth    = aTrackRect.GetWidth();
            mnTrackHeight   = aTrackRect.GetHeight();
        }
    }
}

// -----------------------------------------------------------------------

bool DockingWindow::Notify( NotifyEvent& rNEvt )
{
    if( GetDockingManager()->IsDockable( this ) )   // new docking interface
        return Window::Notify( rNEvt );

    if ( mbDockable )
    {
        if ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
        {
            const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
            if ( pMEvt->IsLeft() )
            {
                if ( pMEvt->IsMod1() && (pMEvt->GetClicks() == 2) )
                {
                    SetFloatingMode( !IsFloatingMode() );
                    return true;
                }
                else if ( pMEvt->GetClicks() == 1 )
                {
                    // check if window is floating standalone (IsFloating())
                    // or only partially floating and still docked with one border
                    // ( !mpWindowImpl->mbFrame)
                    if( ! IsFloatingMode() || ! mpFloatWin->mpWindowImpl->mbFrame )
                    {
                        Point   aPos = pMEvt->GetPosPixel();
                        Window* pWindow = rNEvt.GetWindow();
                        if ( pWindow != this )
                        {
                            aPos = pWindow->OutputToScreenPixel( aPos );
                            aPos = ScreenToOutputPixel( aPos );
                        }
                        ImplStartDocking( aPos );
                    }
                    return true;
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
                return true;
            }
        }
    }

    return Window::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void DockingWindow::StartDocking()
{
    mbDocking = true;
}

// -----------------------------------------------------------------------

bool DockingWindow::Docking( const Point&, Rectangle& )
{
    return IsFloatingMode();
}

// -----------------------------------------------------------------------

void DockingWindow::EndDocking( const Rectangle& rRect, sal_Bool bFloatMode )
{
    if ( !IsDockingCanceled() )
    {
        bool bShow = false;
        if ( bool(bFloatMode) != IsFloatingMode() )
        {
            Show( false, SHOW_NOFOCUSCHANGE );
            SetFloatingMode( bFloatMode );
            bShow = true;
            if ( bFloatMode && mpFloatWin )
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
    mbDocking = false;
}

// -----------------------------------------------------------------------

bool DockingWindow::PrepareToggleFloatingMode()
{
    return true;
}

// -----------------------------------------------------------------------

bool DockingWindow::Close()
{
    ImplDelData aDelData;
    ImplAddDel( &aDelData );
    ImplCallEventListeners( VCLEVENT_WINDOW_CLOSE );
    if ( aDelData.IsDead() )
        return false;
    ImplRemoveDel( &aDelData );

    if ( mpWindowImpl->mxWindowPeer.is() && IsCreatedWithToolkit() )
        return false;

    Show( false, SHOW_NOFOCUSCHANGE );
    return true;
}

// -----------------------------------------------------------------------

void DockingWindow::ToggleFloatingMode()
{
}

// -----------------------------------------------------------------------

void DockingWindow::TitleButtonClick( sal_uInt16 )
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
    switch(nType)
    {
        case STATE_CHANGE_CONTROLBACKGROUND:
            ImplInitSettings();
            Invalidate();
            break;

        case STATE_CHANGE_STYLE:
            mbDockable = (GetStyle() & WB_DOCKABLE) != 0;
            break;

        default:
            break;
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

void DockingWindow::SetFloatingMode( bool bFloatMode )
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
    {
        pWrapper->SetFloatingMode( bFloatMode );
        return;
    }
    if ( IsFloatingMode() != bFloatMode )
    {
        if ( PrepareToggleFloatingMode() ) // changes to floating mode can be vetoed
        {
            bool bVisible = IsVisible();

            if ( bFloatMode )
            {
                Show( false, SHOW_NOFOCUSCHANGE );

                maDockPos = Window::GetPosPixel();

                Window* pRealParent = mpWindowImpl->mpRealParent;
                mpOldBorderWin = mpWindowImpl->mpBorderWindow;

                ImplDockFloatWin* pWin =
                    new ImplDockFloatWin(
                                         mpImplData->mpParent,
                                         mnFloatBits & ( WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE ) ?  mnFloatBits | WB_SYSTEMWINDOW : mnFloatBits,
                                         this );
                mpFloatWin      = pWin;
                mpWindowImpl->mpBorderWindow  = NULL;
                mpWindowImpl->mnLeftBorder    = 0;
                mpWindowImpl->mnTopBorder     = 0;
                mpWindowImpl->mnRightBorder   = 0;
                mpWindowImpl->mnBottomBorder  = 0;
                // Falls Parent zerstoert wird, muessen wir auch vom
                // BorderWindow den Parent umsetzen
                if ( mpOldBorderWin )
                    mpOldBorderWin->SetParent( pWin );

                // #i123765# reset the buffered DropTargets when undocking, else it may not
                // be correctly initialized
                mpWindowImpl->mxDNDListenerContainer.clear();

                SetParent( pWin );
                SetPosPixel( Point() );
                mpWindowImpl->mpBorderWindow = pWin;
                pWin->mpWindowImpl->mpClientWindow = this;
                mpWindowImpl->mpRealParent = pRealParent;
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
                pWin->SetMaxOutputSizePixel( mpImplData->maMaxOutSize );

                ToggleFloatingMode();

                if ( bVisible )
                    Show();
            }
            else
            {
                Show( false, SHOW_NOFOCUSCHANGE );

                // FloatingDaten wird im FloatingWindow speichern
                maFloatPos      = mpFloatWin->GetPosPixel();
                mbDockBtn       = mpFloatWin->IsTitleButtonVisible( TITLE_BUTTON_DOCKING );
                mbHideBtn       = mpFloatWin->IsTitleButtonVisible( TITLE_BUTTON_HIDE );
                mbPined         = mpFloatWin->IsPined();
                mbRollUp        = mpFloatWin->IsRollUp();
                maRollUpOutSize = mpFloatWin->GetRollUpOutputSizePixel();
                maMinOutSize    = mpFloatWin->GetMinOutputSizePixel();
                mpImplData->maMaxOutSize = mpFloatWin->GetMaxOutputSizePixel();

                Window* pRealParent = mpWindowImpl->mpRealParent;
                mpWindowImpl->mpBorderWindow = NULL;
                if ( mpOldBorderWin )
                {
                    SetParent( mpOldBorderWin );
                    ((ImplBorderWindow*)mpOldBorderWin)->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
                    mpOldBorderWin->Resize();
                }
                mpWindowImpl->mpBorderWindow = mpOldBorderWin;
                SetParent( pRealParent );
                mpWindowImpl->mpRealParent = pRealParent;
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
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
    {
        pWrapper->SetFloatStyle( nStyle );
        return;
    }

    mnFloatBits = nStyle;
}

// -----------------------------------------------------------------------

WinBits DockingWindow::GetFloatStyle() const
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
    {
        return pWrapper->GetFloatStyle();
    }

    return mnFloatBits;
}

// -----------------------------------------------------------------------

void DockingWindow::setPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight,
                                     sal_uInt16 nFlags )
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
    {
        if ( pWrapper->mpFloatWin )
            pWrapper->mpFloatWin->setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
        else
            Window::setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
        return;
    }

    if ( mpFloatWin )
        mpFloatWin->setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
    else
        Window::setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

// -----------------------------------------------------------------------

Point DockingWindow::GetPosPixel() const
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
    {
        if ( pWrapper->mpFloatWin )
            return pWrapper->mpFloatWin->GetPosPixel();
        else
            return Window::GetPosPixel();
    }

    if ( mpFloatWin )
        return mpFloatWin->GetPosPixel();
    else
        return Window::GetPosPixel();
}

// -----------------------------------------------------------------------

Size DockingWindow::GetSizePixel() const
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
    {
        if ( pWrapper->mpFloatWin )
            return pWrapper->mpFloatWin->GetSizePixel();
        else
            return Window::GetSizePixel();
    }

    if ( mpFloatWin )
        return mpFloatWin->GetSizePixel();
    else
        return Window::GetSizePixel();
}

// -----------------------------------------------------------------------

void DockingWindow::SetOutputSizePixel( const Size& rNewSize )
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
    {
        if ( pWrapper->mpFloatWin )
            pWrapper->mpFloatWin->SetOutputSizePixel( rNewSize );
        else
            Window::SetOutputSizePixel( rNewSize );
        return;
    }

    if ( mpFloatWin )
        mpFloatWin->SetOutputSizePixel( rNewSize );
    else
        Window::SetOutputSizePixel( rNewSize );
}

// -----------------------------------------------------------------------

Size DockingWindow::GetOutputSizePixel() const
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
    {
        if ( pWrapper->mpFloatWin )
            return pWrapper->mpFloatWin->GetOutputSizePixel();
        else
            return Window::GetOutputSizePixel();
    }

    if ( mpFloatWin )
        return mpFloatWin->GetOutputSizePixel();
    else
        return Window::GetOutputSizePixel();
}

Point DockingWindow::GetFloatingPos() const
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
    {
        if ( pWrapper->mpFloatWin )
        {
            WindowStateData aData;
            aData.SetMask( WINDOWSTATE_MASK_POS );
            pWrapper->mpFloatWin->GetWindowStateData( aData );
            Point aPos( aData.GetX(), aData.GetY() );
            aPos = pWrapper->mpFloatWin->GetParent()->ImplGetFrameWindow()->AbsoluteScreenToOutputPixel( aPos );
            return aPos;
        }
        else
            return maFloatPos;
    }

    if ( mpFloatWin )
    {
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

bool DockingWindow::IsFloatingMode() const
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
        return pWrapper->IsFloatingMode();
    else
        return (mpFloatWin != NULL);
}

void DockingWindow::SetMaxOutputSizePixel( const Size& rSize )
{
    if ( mpFloatWin )
        mpFloatWin->SetMaxOutputSizePixel( rSize );
    mpImplData->maMaxOutSize = rSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
