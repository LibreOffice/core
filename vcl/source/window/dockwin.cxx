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
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/settings.hxx>

#include <svdata.hxx>
#include <window.h>
#include <brdwin.hxx>
#include <salframe.hxx>

#define DOCKWIN_FLOATSTYLES         (WB_SIZEABLE | WB_MOVEABLE | WB_CLOSEABLE | WB_STANDALONE | WB_PINABLE | WB_ROLLABLE )

class DockingWindow::ImplData
{
public:
    ImplData();
    ~ImplData();

    VclPtr<vcl::Window> mpParent;
    Size                maMaxOutSize;
};

DockingWindow::ImplData::ImplData()
{
    mpParent = nullptr;
    maMaxOutSize = Size( SHRT_MAX, SHRT_MAX );
}

DockingWindow::ImplData::~ImplData()
{
}

class ImplDockFloatWin : public FloatingWindow
{
private:
    VclPtr<DockingWindow> mpDockWin;
    sal_uInt64      mnLastTicks;
    Idle            maDockIdle;
    Point           maDockPos;
    Rectangle       maDockRect;
    bool            mbInMove;
    ImplSVEvent *   mnLastUserEvent;

    DECL_LINK_TYPED(DockingHdl, void *, void);
    DECL_LINK_TYPED(DockTimerHdl, Idle *, void);
public:
    ImplDockFloatWin( vcl::Window* pParent, WinBits nWinBits,
                      DockingWindow* pDockingWin );
    virtual ~ImplDockFloatWin() override;
    virtual void dispose() override;

    virtual void    Move() override;
    virtual void    Resize() override;
    virtual void    TitleButtonClick( TitleButton nButton ) override;
    virtual void    Pin() override;
    virtual void    Roll() override;
    virtual void    Resizing( Size& rSize ) override;
    virtual bool    Close() override;
};

ImplDockFloatWin::ImplDockFloatWin( vcl::Window* pParent, WinBits nWinBits,
                                    DockingWindow* pDockingWin ) :
        FloatingWindow( pParent, nWinBits ),
        mpDockWin( pDockingWin ),
        mnLastTicks( tools::Time::GetSystemTicks() ),
        mbInMove( false ),
        mnLastUserEvent( nullptr )
{
    // copy settings of DockingWindow
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

    maDockIdle.SetIdleHdl( LINK( this, ImplDockFloatWin, DockTimerHdl ) );
    maDockIdle.SetPriority( SchedulerPriority::MEDIUM );
    maDockIdle.SetDebugName( "vcl::ImplDockFloatWin maDockIdle" );
}

ImplDockFloatWin::~ImplDockFloatWin()
{
    disposeOnce();
}

void ImplDockFloatWin::dispose()
{
    if( mnLastUserEvent )
        Application::RemoveUserEvent( mnLastUserEvent );

    disposeBuilder();

    mpDockWin.clear();
    FloatingWindow::dispose();
}

IMPL_LINK_NOARG_TYPED(ImplDockFloatWin, DockTimerHdl, Idle *, void)
{
    SAL_WARN_IF( !mpDockWin->IsFloatingMode(), "vcl", "docktimer called but not floating" );

    maDockIdle.Stop();
    PointerState aState = GetPointerState();

    if( aState.mnState & KEY_MOD1 )
    {
        // i43499 CTRL disables docking now
        mpDockWin->GetParent()->ImplGetFrameWindow()->HideTracking();
        mpDockWin->EndDocking( maDockRect, true );
        if( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) )
            maDockIdle.Start();
    }
    else if( ! ( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) )
    {
        mpDockWin->GetParent()->ImplGetFrameWindow()->HideTracking();
        mpDockWin->EndDocking( maDockRect, false );
    }
    else
    {
        mpDockWin->GetParent()->ImplGetFrameWindow()->ShowTracking( maDockRect, ShowTrackFlags::Big | ShowTrackFlags::TrackWindow );
        maDockIdle.Start();
    }
}

IMPL_LINK_NOARG_TYPED(ImplDockFloatWin, DockingHdl, void*, void)
{
    PointerState aState = mpDockWin->GetParent()->GetPointerState();

    mnLastUserEvent = nullptr;
    if( mpDockWin->IsDockable()                             &&
        (tools::Time::GetSystemTicks() - mnLastTicks > 500)        &&
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
            mpDockWin->GetParent()->ImplGetFrameWindow()->ShowTracking( maDockRect, ShowTrackFlags::Object | ShowTrackFlags::TrackWindow );
            DockTimerHdl( nullptr );
        }
        else
        {
            mpDockWin->GetParent()->ImplGetFrameWindow()->HideTracking();
            maDockIdle.Stop();
            mpDockWin->EndDocking( maDockRect, true );
        }
    }
    mbInMove = false;
}

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
        mnLastUserEvent = Application::PostUserEvent( LINK( this, ImplDockFloatWin, DockingHdl ), nullptr, true );
}

void ImplDockFloatWin::Resize()
{
    FloatingWindow::Resize();
    Size aSize( GetSizePixel() );
    mpDockWin->ImplPosSizeWindow( 0, 0, aSize.Width(), aSize.Height(), PosSizeFlags::PosSize );
}

void ImplDockFloatWin::TitleButtonClick( TitleButton nButton )
{
    FloatingWindow::TitleButtonClick( nButton );
}

void ImplDockFloatWin::Pin()
{
    FloatingWindow::Pin();
}

void ImplDockFloatWin::Roll()
{
    FloatingWindow::Roll();
}

void ImplDockFloatWin::Resizing( Size& rSize )
{
    FloatingWindow::Resizing( rSize );
    mpDockWin->Resizing( rSize );
}

bool ImplDockFloatWin::Close()
{
    return mpDockWin->Close();
}

bool DockingWindow::ImplStartDocking( const Point& rPos )
{
    if ( !mbDockable )
        return false;

    maMouseOff      = rPos;
    maMouseStart    = maMouseOff;
    mbDocking       = true;
    mbLastFloatMode = IsFloatingMode();
    mbStartFloat    = mbLastFloatMode;

    // calculate FloatingBorder
    VclPtr<FloatingWindow> pWin;
    if ( mpFloatWin )
        pWin = mpFloatWin;
    else
        pWin = VclPtr<ImplDockFloatWin>::Create( mpImplData->mpParent, mnFloatBits, nullptr );
    pWin->GetBorder( mnDockLeft, mnDockTop, mnDockRight, mnDockBottom );
    if ( !mpFloatWin )
        pWin.disposeAndClear();

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

    if ( GetSettings().GetStyleSettings().GetDragFullOptions() & DragFullOptions::Docking &&
        !( mnFloatBits & ( WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE ) ) ) // no full drag when migrating to system window
        mbDragFull = true;
    else
    {
        StartDocking();
        mbDragFull = false;
        ImplUpdateAll();
        ImplGetFrameWindow()->ImplUpdateAll();
    }

    StartTracking( StartTrackingFlags::KeyMod );
    return true;
}

void DockingWindow::ImplInitDockingWindowData()
{
    mpWindowImpl->mbDockWin = true;
    mpFloatWin     = nullptr;
    mpOldBorderWin = nullptr;
    mpImplData     = new ImplData;
    mnTrackX       = 0;
    mnTrackY       = 0;
    mnTrackWidth   = 0;
    mnTrackHeight  = 0;
    mnDockLeft     = 0;
    mnDockTop      = 0;
    mnDockRight    = 0;
    mnDockBottom   = 0;
    mnFloatBits    = 0;
    mbDockCanceled  = false;
    mbDockable     = false;
    mbDocking      = false;
    mbDragFull     = false;
    mbLastFloatMode = false;
    mbStartFloat   = false;
    mbPinned       = false;
    mbRollUp       = false;
    mbDockBtn      = false;
    mbHideBtn      = false;
    mbIsDefferedInit = false;
    mbIsCalculatingInitialLayoutSize = false;
    mpDialogParent = nullptr;

    //To-Do, reuse maResizeTimer
    maLayoutIdle.SetPriority(SchedulerPriority::RESIZE);
    maLayoutIdle.SetIdleHdl( LINK( this, DockingWindow, ImplHandleLayoutTimerHdl ) );
    maLayoutIdle.SetDebugName( "vcl::DockingWindow maLayoutIdle" );
}

void DockingWindow::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;

    mpImplData->mpParent    = pParent;
    mbDockable              = (nStyle & WB_DOCKABLE) != 0;
    mnFloatBits             = WB_BORDER | (nStyle & DOCKWIN_FLOATSTYLES);
    nStyle                 &= ~(DOCKWIN_FLOATSTYLES | WB_BORDER);
    if ( nStyle & WB_DOCKBORDER )
        nStyle |= WB_BORDER;

    Window::ImplInit( pParent, nStyle, nullptr );

    ImplInitSettings();
}

void DockingWindow::ImplInitSettings()
{
    // Hack: to be able to build DockingWindows w/o background before switching
    // TODO: Hack
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

void DockingWindow::ImplLoadRes( const ResId& rResId )
{
    Window::ImplLoadRes( rResId );

    const sal_uInt32 nMask = ReadLongRes();

    if ( (RSC_DOCKINGWINDOW_XYMAPMODE | RSC_DOCKINGWINDOW_X |
          RSC_DOCKINGWINDOW_Y) & nMask )
    {
        // use Sizes of the Resource
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

DockingWindow::DockingWindow( WindowType nType ) :
    Window(nType)
{
    ImplInitDockingWindowData();
}

DockingWindow::DockingWindow( vcl::Window* pParent, WinBits nStyle ) :
    Window( WINDOW_DOCKINGWINDOW )
{
    ImplInitDockingWindowData();
    ImplInit( pParent, nStyle );
}

//Find the real parent stashed in mpDialogParent.
void DockingWindow::doDeferredInit(WinBits nBits)
{
    vcl::Window *pParent = mpDialogParent;
    mpDialogParent = nullptr;
    ImplInit(pParent, nBits);
    mbIsDefferedInit = false;
}

void DockingWindow::loadUI(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
    const css::uno::Reference<css::frame::XFrame> &rFrame)
{
    mbIsDefferedInit = true;
    mpDialogParent = pParent; //should be unset in doDeferredInit
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(), rUIXMLDescription, rID, rFrame);
}

DockingWindow::DockingWindow(vcl::Window* pParent, const OString& rID,
    const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame)
    : Window(WINDOW_DOCKINGWINDOW)
{
    ImplInitDockingWindowData();

    loadUI(pParent, rID, rUIXMLDescription, rFrame);
}

DockingWindow::~DockingWindow()
{
    disposeOnce();
}

void DockingWindow::dispose()
{
    if ( IsFloatingMode() )
    {
        Show( false, ShowFlags::NoFocusChange );
        SetFloatingMode(false);
    }
    delete mpImplData;
    mpImplData = nullptr;
    mpFloatWin.clear();
    mpOldBorderWin.clear();
    mpDialogParent.clear();
    disposeBuilder();
    Window::dispose();
}

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
                // reset old state on Cancel
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
        // dock only for non-synthetic MouseEvents
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
                Point aOldPos = OutputToScreenPixel( Point() );
                EndDocking( aTrackRect, mbLastFloatMode );
                // repaint if state or position has changed
                if ( aOldPos != OutputToScreenPixel( Point() ) )
                {
                    ImplUpdateAll();
                    ImplGetFrameWindow()->ImplUpdateAll();
                }
//                EndDocking( aTrackRect, mbLastFloatMode );
            }
            else
            {
                ShowTrackFlags nTrackStyle;
                if ( bFloatMode )
                    nTrackStyle = ShowTrackFlags::Big;
                else
                    nTrackStyle = ShowTrackFlags::Object;
                Rectangle aShowTrackRect = aTrackRect;
                aShowTrackRect.SetPos( ImplFrameToOutput( aShowTrackRect.TopLeft() ) );
                ShowTracking( aShowTrackRect, nTrackStyle );

                // recalculate mouse offset, as the rectangle was changed
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

bool DockingWindow::Notify( NotifyEvent& rNEvt )
{
    if( GetDockingManager()->IsDockable( this ) )   // new docking interface
        return Window::Notify( rNEvt );

    if ( mbDockable )
    {
        if ( rNEvt.GetType() == MouseNotifyEvent::MOUSEBUTTONDOWN )
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
                        vcl::Window* pWindow = rNEvt.GetWindow();
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
        else if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        {
            const vcl::KeyCode& rKey = rNEvt.GetKeyEvent()->GetKeyCode();
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

void DockingWindow::StartDocking()
{
    mbDocking = true;
}

bool DockingWindow::Docking( const Point&, Rectangle& )
{
    return IsFloatingMode();
}

void DockingWindow::EndDocking( const Rectangle& rRect, bool bFloatMode )
{
    if ( !IsDockingCanceled() )
    {
        bool bShow = false;
        if ( bool(bFloatMode) != IsFloatingMode() )
        {
            Show( false, ShowFlags::NoFocusChange );
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

bool DockingWindow::PrepareToggleFloatingMode()
{
    return true;
}

bool DockingWindow::Close()
{
    VclPtr<vcl::Window> xWindow = this;
    CallEventListeners( VCLEVENT_WINDOW_CLOSE );
    if ( xWindow->IsDisposed() )
        return false;

    if ( mpWindowImpl->mxWindowPeer.is() && IsCreatedWithToolkit() )
        return false;

    Show( false, ShowFlags::NoFocusChange );
    return true;
}

void DockingWindow::ToggleFloatingMode()
{
}

void DockingWindow::Resizing( Size& )
{
}

void DockingWindow::DoInitialLayout()
{
    if (GetSettings().GetStyleSettings().GetAutoMnemonic())
       Accelerator::GenerateAutoMnemonicsOnHierarchy(this);

    if (isLayoutEnabled())
    {
        mbIsCalculatingInitialLayoutSize = true;
        setDeferredProperties();
        if (IsFloatingMode())
            setOptimalLayoutSize();
        mbIsCalculatingInitialLayoutSize = false;
    }
}

void DockingWindow::StateChanged( StateChangedType nType )
{
    switch(nType)
    {
        case StateChangedType::InitShow:
            DoInitialLayout();
            break;

        case StateChangedType::ControlBackground:
            ImplInitSettings();
            Invalidate();
            break;

        case StateChangedType::Style:
            mbDockable = (GetStyle() & WB_DOCKABLE) != 0;
            break;

        default:
            break;
    }

    Window::StateChanged( nType );
}

void DockingWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
    else
        Window::DataChanged( rDCEvt );
}

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
                Show( false, ShowFlags::NoFocusChange );

                sal_Int32 nBorderWidth = get_border_width();

                maDockPos = Window::GetPosPixel();

                vcl::Window* pRealParent = mpWindowImpl->mpRealParent;
                mpOldBorderWin = mpWindowImpl->mpBorderWindow;

                ImplDockFloatWin* pWin =
                    VclPtr<ImplDockFloatWin>::Create(

                                         mpImplData->mpParent,
                                         mnFloatBits & ( WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE ) ?  mnFloatBits | WB_SYSTEMWINDOW : mnFloatBits,
                                         this );
                mpFloatWin      = pWin;
                mpWindowImpl->mpBorderWindow  = nullptr;
                mpWindowImpl->mnLeftBorder    = 0;
                mpWindowImpl->mnTopBorder     = 0;
                mpWindowImpl->mnRightBorder   = 0;
                mpWindowImpl->mnBottomBorder  = 0;
                // if the parent gets destroyed, we also have to reset the parent of the BorderWindow
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
                Size aSize(Window::GetSizePixel());
                pWin->SetOutputSizePixel(aSize);
                pWin->SetPosPixel( maFloatPos );
                // pass on DockingData to FloatingWindow
                pWin->ShowTitleButton( TitleButton::Docking, mbDockBtn );
                pWin->ShowTitleButton( TitleButton::Hide, mbHideBtn );
                pWin->SetPin( mbPinned );
                if ( mbRollUp )
                    pWin->RollUp();
                else
                    pWin->RollDown();
                pWin->SetRollUpOutputSizePixel( maRollUpOutSize );
                pWin->SetMinOutputSizePixel( maMinOutSize );

                pWin->SetMaxOutputSizePixel( mpImplData->maMaxOutSize );

                ToggleFloatingMode();

                set_border_width(nBorderWidth);

                if ( bVisible )
                    Show();

                mpFloatWin->queue_resize();
            }
            else
            {
                Show( false, ShowFlags::NoFocusChange );

                sal_Int32 nBorderWidth = get_border_width();

                // store FloatingData in FloatingWindow
                maFloatPos      = mpFloatWin->GetPosPixel();
                mbDockBtn       = mpFloatWin->IsTitleButtonVisible( TitleButton::Docking );
                mbHideBtn       = mpFloatWin->IsTitleButtonVisible( TitleButton::Hide );
                mbPinned        = mpFloatWin->IsPinned();
                mbRollUp        = mpFloatWin->IsRollUp();
                maRollUpOutSize = mpFloatWin->GetRollUpOutputSizePixel();
                maMinOutSize    = mpFloatWin->GetMinOutputSizePixel();
                mpImplData->maMaxOutSize = mpFloatWin->GetMaxOutputSizePixel();

                vcl::Window* pRealParent = mpWindowImpl->mpRealParent;
                mpWindowImpl->mpBorderWindow = nullptr;
                if ( mpOldBorderWin )
                {
                    SetParent( mpOldBorderWin );
                    static_cast<ImplBorderWindow*>(mpOldBorderWin.get())->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
                    mpOldBorderWin->Resize();
                }
                mpWindowImpl->mpBorderWindow = mpOldBorderWin;
                SetParent( pRealParent );
                mpWindowImpl->mpRealParent = pRealParent;
                mpFloatWin.disposeAndClear();
                SetPosPixel( maDockPos );

                ToggleFloatingMode();

                set_border_width(nBorderWidth);

                if ( bVisible )
                    Show();
            }
        }
    }
}

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

WinBits DockingWindow::GetFloatStyle() const
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
    {
        return pWrapper->GetFloatStyle();
    }

    return mnFloatBits;
}

void DockingWindow::setPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight,
                                     PosSizeFlags nFlags )
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if (pWrapper)
    {
        if (!pWrapper->mpFloatWin)
            Window::setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
    }
    else
    {
        if (!mpFloatWin)
            Window::setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
    }

    if (::isLayoutEnabled(this))
    {
        Size aSize(GetSizePixel());

        sal_Int32 nBorderWidth = get_border_width();

        aSize.Width() -= 2 * nBorderWidth;
        aSize.Height() -= 2 * nBorderWidth;

        Point aPos(nBorderWidth, nBorderWidth);
        Window *pBox = GetWindow(GetWindowType::FirstChild);
        assert(pBox);
        VclContainer::setLayoutAllocation(*pBox, aPos, aSize);
    }
}

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
            aData.SetMask( WindowStateMask::Pos );
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
        aData.SetMask( WindowStateMask::Pos );
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
        return (mpFloatWin != nullptr);
}

void DockingWindow::SetMaxOutputSizePixel( const Size& rSize )
{
    if ( mpFloatWin )
        mpFloatWin->SetMaxOutputSizePixel( rSize );
    mpImplData->maMaxOutSize = rSize;
}

void DockingWindow::SetText(const OUString& rStr)
{
    setDeferredProperties();
    Window::SetText(rStr);
}

OUString DockingWindow::GetText() const
{
    const_cast<DockingWindow*>(this)->setDeferredProperties();
    return Window::GetText();
}

bool DockingWindow::isLayoutEnabled() const
{
    //pre dtor called, and single child is a container => we're layout enabled
    return mpImplData && ::isLayoutEnabled(this);
}

void DockingWindow::setOptimalLayoutSize()
{
    maLayoutIdle.Stop();

    //resize DockingWindow to fit requisition on initial show
    Window *pBox = GetWindow(GetWindowType::FirstChild);

    Size aSize = get_preferred_size();

    Size aMax(bestmaxFrameSizeForScreenSize(GetDesktopRectPixel().GetSize()));

    aSize.Width() = std::min(aMax.Width(), aSize.Width());
    aSize.Height() = std::min(aMax.Height(), aSize.Height());

    SetMinOutputSizePixel(aSize);
    SetSizePixel(aSize);
    setPosSizeOnContainee(aSize, *pBox);
}

void DockingWindow::setPosSizeOnContainee(Size aSize, Window &rBox)
{
    sal_Int32 nBorderWidth = get_border_width();

    aSize.Width() -= 2 * nBorderWidth;
    aSize.Height() -= 2 * nBorderWidth;

    Point aPos(nBorderWidth, nBorderWidth);
    VclContainer::setLayoutAllocation(rBox, aPos, aSize);
}

Size DockingWindow::GetOptimalSize() const
{
    if (!isLayoutEnabled())
        return Window::GetOptimalSize();

    Size aSize = VclContainer::getLayoutRequisition(*GetWindow(GetWindowType::FirstChild));

    sal_Int32 nBorderWidth = get_border_width();

    aSize.Height() += mpWindowImpl->mnLeftBorder + mpWindowImpl->mnRightBorder
        + 2*nBorderWidth;
    aSize.Width() += mpWindowImpl->mnTopBorder + mpWindowImpl->mnBottomBorder
        + 2*nBorderWidth;

    return Window::CalcWindowSize(aSize);
}

void DockingWindow::queue_resize(StateChangedType eReason)
{
    bool bTriggerLayout = true;
    if (hasPendingLayout() || isCalculatingInitialLayoutSize())
    {
        bTriggerLayout = false;
    }
    if (!isLayoutEnabled())
    {
        bTriggerLayout = false;
    }
    if (bTriggerLayout)
    {
        InvalidateSizeCache();
        maLayoutIdle.Start();
    }
    vcl::Window::queue_resize(eReason);
}

IMPL_LINK_NOARG_TYPED(DockingWindow, ImplHandleLayoutTimerHdl, Idle*, void)
{
    if (!isLayoutEnabled())
    {
        SAL_WARN("vcl.layout", "DockingWindow has become non-layout because extra children have been added directly to it.");
        return;
    }

    Window *pBox = GetWindow(GetWindowType::FirstChild);
    assert(pBox);
    setPosSizeOnContainee(GetSizePixel(), *pBox);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
