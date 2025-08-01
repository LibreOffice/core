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
#include <sal/log.hxx>
#include <vcl/dndlistenercontainer.hxx>
#include <vcl/event.hxx>
#include <vcl/toolkit/floatwin.hxx>
#include <vcl/layout.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/settings.hxx>
#include <comphelper/lok.hxx>

#include <accel.hxx>
#include <svdata.hxx>
#include <window.h>
#include <brdwin.hxx>

#include "impldockingwrapper.hxx"

#define DOCKWIN_FLOATSTYLES         (WB_SIZEABLE | WB_MOVEABLE | WB_CLOSEABLE | WB_STANDALONE)

class DockingWindow::ImplData
{
public:
    ImplData();

    VclPtr<vcl::Window> mpParent;
    Size                maMaxOutSize;
};

DockingWindow::ImplData::ImplData()
{
    mpParent = nullptr;
    maMaxOutSize = Size( SHRT_MAX, SHRT_MAX );
}

namespace {

class ImplDockFloatWin : public FloatingWindow
{
private:
    VclPtr<DockingWindow> mpDockWin;
    sal_uInt64      mnLastTicks;
    Idle            maDockIdle;
    Point           maDockPos;
    tools::Rectangle       maDockRect;
    bool            mbInMove;
    ImplSVEvent *   mnLastUserEvent;

    DECL_LINK(DockingHdl, void *, void);
    DECL_LINK(DockTimerHdl, Timer *, void);
public:
    ImplDockFloatWin( vcl::Window* pParent, WinBits nWinBits,
                      DockingWindow* pDockingWin );
    virtual ~ImplDockFloatWin() override;
    virtual void dispose() override;

    virtual void    Move() override;
    virtual void    Resize() override;
    virtual void    Resizing( Size& rSize ) override;
    virtual bool    Close() override;
};

}

ImplDockFloatWin::ImplDockFloatWin( vcl::Window* pParent, WinBits nWinBits,
                                    DockingWindow* pDockingWin ) :
        FloatingWindow( pParent, nWinBits ),
        mpDockWin( pDockingWin ),
        mnLastTicks( tools::Time::GetSystemTicks() ),
        maDockIdle( "vcl::ImplDockFloatWin maDockIdle" ),
        mbInMove( false ),
        mnLastUserEvent( nullptr )
{
    // copy settings of DockingWindow
    if ( pDockingWin )
    {
        GetOutDev()->SetSettings( pDockingWin->GetSettings() );
        Enable( pDockingWin->IsEnabled(), false );
        EnableInput( pDockingWin->IsInputEnabled(), false );
        AlwaysEnableInput( pDockingWin->IsAlwaysEnableInput(), false );
        EnableAlwaysOnTop( pDockingWin->IsAlwaysOnTopEnabled() );
        SetActivateMode( pDockingWin->GetActivateMode() );
    }

    SetBackground();

    maDockIdle.SetInvokeHandler( LINK( this, ImplDockFloatWin, DockTimerHdl ) );
    maDockIdle.SetPriority( TaskPriority::HIGH_IDLE );
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

    mpDockWin.reset();
    FloatingWindow::dispose();
}

IMPL_LINK_NOARG(ImplDockFloatWin, DockTimerHdl, Timer *, void)
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

IMPL_LINK_NOARG(ImplDockFloatWin, DockingHdl, void*, void)
{
    PointerState aState = mpDockWin->GetParent()->GetPointerState();

    mnLastUserEvent = nullptr;
    if( mpDockWin->IsDockable()                             &&
        (tools::Time::GetSystemTicks() - mnLastTicks > 500)        &&
        ( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) &&
        !(aState.mnState & KEY_MOD1) )  // i43499 CTRL disables docking now
    {
        maDockPos = mpDockWin->GetParent()->AbsoluteScreenToOutputPixel( OutputToAbsoluteScreenPixel( Point() ) );
        maDockPos = mpDockWin->GetParent()->OutputToScreenPixel( maDockPos );  // sfx expects screen coordinates

        if( ! mpDockWin->IsDocking() )
            mpDockWin->StartDocking();
        maDockRect = tools::Rectangle( maDockPos, mpDockWin->GetSizePixel() );

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

void ImplDockFloatWin::Resizing( Size& rSize )
{
    FloatingWindow::Resizing( rSize );
    mpDockWin->Resizing( rSize );
}

bool ImplDockFloatWin::Close()
{
    return mpDockWin->Close();
}

void DockingWindow::ImplStartDocking( const Point& rPos )
{
    if ( !mbDockable )
        return;

    maMouseOff      = rPos;
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

    Point   aPos    = OutputToScreenPixel( Point() );
    Size    aSize   = Window::GetOutputSizePixel();
    mnTrackX        = aPos.X();
    mnTrackY        = aPos.Y();
    mnTrackWidth    = aSize.Width();
    mnTrackHeight   = aSize.Height();

    if ( mbLastFloatMode )
    {
        maMouseOff.AdjustX(mnDockLeft );
        maMouseOff.AdjustY(mnDockTop );
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
}

void DockingWindow::ImplInitDockingWindowData()
{
    mpWindowImpl->mbDockWin = true;
    mpFloatWin     = nullptr;
    mpOldBorderWin = nullptr;
    mpImplData.reset(new ImplData);
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
    mbDockBtn      = false;
    mbHideBtn      = false;
    mbIsDeferredInit = false;
    mbIsCalculatingInitialLayoutSize = false;
    mpDialogParent = nullptr;

    //To-Do, reuse maResizeTimer
    maLayoutIdle.SetPriority(TaskPriority::RESIZE);
    maLayoutIdle.SetInvokeHandler( LINK( this, DockingWindow, ImplHandleLayoutTimerHdl ) );
}

void DockingWindow::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;

    mpImplData->mpParent    = pParent;
    mbDockable              = (nStyle & WB_DOCKABLE) != 0;
    mnFloatBits             = WB_BORDER | (nStyle & DOCKWIN_FLOATSTYLES);
    nStyle                 &= ~(DOCKWIN_FLOATSTYLES | WB_BORDER);

    Window::ImplInit( pParent, nStyle, nullptr );

    ImplInitSettings();
}

void DockingWindow::ImplInitSettings()
{
    // Hack: to be able to build DockingWindows w/o background before switching
    // TODO: Hack
    if ( !IsBackground() )
        return;

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

DockingWindow::DockingWindow( WindowType eType, const char* pIdleDebugName ) :
    Window(eType),
    maLayoutIdle( pIdleDebugName )
{
    ImplInitDockingWindowData();
}

DockingWindow::DockingWindow( vcl::Window* pParent, WinBits nStyle, const char* pIdleDebugName ) :
    Window( WindowType::DOCKINGWINDOW ),
    maLayoutIdle( pIdleDebugName )
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
    mbIsDeferredInit = false;
}

void DockingWindow::loadUI(vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription,
    const css::uno::Reference<css::frame::XFrame> &rFrame)
{
    mbIsDeferredInit = true;
    mpDialogParent = pParent; //should be unset in doDeferredInit
    m_pUIBuilder.reset( new VclBuilder(this, AllSettings::GetUIRootDir(), rUIXMLDescription, rID, rFrame) );
}

DockingWindow::DockingWindow(vcl::Window* pParent, const OUString& rID,
    const OUString& rUIXMLDescription, const char* pIdleDebugName,
    const css::uno::Reference<css::frame::XFrame> &rFrame)
    : Window(WindowType::DOCKINGWINDOW),
    maLayoutIdle( pIdleDebugName )
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
    mpImplData.reset();
    mpFloatWin.reset();
    mpOldBorderWin.reset();
    mpDialogParent.reset();
    disposeBuilder();
    Window::dispose();
}

void DockingWindow::Tracking( const TrackingEvent& rTEvt )
{
    if( GetDockingManager()->IsDockable( this ) )   // new docking interface
        return Window::Tracking( rTEvt );

    if ( !mbDocking )
        return;

    if ( rTEvt.IsTrackingEnded() )
    {
        mbDocking = false;
        if ( mbDragFull )
        {
            // reset old state on Cancel
            if ( rTEvt.IsTrackingCanceled() )
            {
                StartDocking();
                tools::Rectangle aRect( Point( mnTrackX, mnTrackY ), Size( mnTrackWidth, mnTrackHeight ) );
                EndDocking( aRect, mbStartFloat );
            }
        }
        else
        {
            HideTracking();
            if ( rTEvt.IsTrackingCanceled() )
            {
                mbDockCanceled = true;
                EndDocking( tools::Rectangle( Point( mnTrackX, mnTrackY ), Size( mnTrackWidth, mnTrackHeight ) ), mbLastFloatMode );
                mbDockCanceled = false;
            }
            else
                EndDocking( tools::Rectangle( Point( mnTrackX, mnTrackY ), Size( mnTrackWidth, mnTrackHeight ) ), mbLastFloatMode );
        }
    }
    // dock only for non-synthetic MouseEvents
    else if ( !rTEvt.GetMouseEvent().IsSynthetic() || rTEvt.GetMouseEvent().IsModifierChanged() )
    {
        Point   aMousePos = rTEvt.GetMouseEvent().GetPosPixel();
        Point   aFrameMousePos = OutputToScreenPixel( aMousePos );
        Size    aFrameSize = mpWindowImpl->mpFrameWindow->GetOutputSizePixel();
        if ( aFrameMousePos.X() < 0 )
            aFrameMousePos.setX( 0 );
        if ( aFrameMousePos.Y() < 0 )
            aFrameMousePos.setY( 0 );
        if ( aFrameMousePos.X() > aFrameSize.Width()-1 )
            aFrameMousePos.setX( aFrameSize.Width()-1 );
        if ( aFrameMousePos.Y() > aFrameSize.Height()-1 )
            aFrameMousePos.setY( aFrameSize.Height()-1 );
        aMousePos = ScreenToOutputPixel( aFrameMousePos );
        aMousePos.AdjustX( -(maMouseOff.X()) );
        aMousePos.AdjustY( -(maMouseOff.Y()) );
        Point aFramePos = OutputToScreenPixel( aMousePos );
        tools::Rectangle aTrackRect( aFramePos, Size( mnTrackWidth, mnTrackHeight ) );
        tools::Rectangle aCompRect = aTrackRect;
        aFramePos.AdjustX(maMouseOff.X() );
        aFramePos.AdjustY(maMouseOff.Y() );
        if ( mbDragFull )
            StartDocking();
        bool bFloatMode = Docking( aFramePos, aTrackRect );
        if ( mbLastFloatMode != bFloatMode )
        {
            if ( bFloatMode )
            {
                aTrackRect.AdjustLeft( -mnDockLeft );
                aTrackRect.AdjustTop( -mnDockTop );
                aTrackRect.AdjustRight(mnDockRight );
                aTrackRect.AdjustBottom(mnDockBottom );
            }
            else
            {
                if ( aCompRect == aTrackRect )
                {
                    aTrackRect.AdjustLeft(mnDockLeft );
                    aTrackRect.AdjustTop(mnDockTop );
                    aTrackRect.AdjustRight( -mnDockRight );
                    aTrackRect.AdjustBottom( -mnDockBottom );
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
            tools::Rectangle aShowTrackRect = aTrackRect;
            aShowTrackRect.SetPos( ScreenToOutputPixel( aShowTrackRect.TopLeft() ) );
            ShowTracking( aShowTrackRect, nTrackStyle );

            // recalculate mouse offset, as the rectangle was changed
            maMouseOff.setX( aFramePos.X() - aTrackRect.Left() );
            maMouseOff.setY( aFramePos.Y() - aTrackRect.Top() );
        }

        mnTrackX        = aTrackRect.Left();
        mnTrackY        = aTrackRect.Top();
        mnTrackWidth    = aTrackRect.GetWidth();
        mnTrackHeight   = aTrackRect.GetHeight();
    }
}

bool DockingWindow::EventNotify( NotifyEvent& rNEvt )
{
    if( GetDockingManager()->IsDockable( this ) )   // new docking interface
        return Window::EventNotify( rNEvt );

    if ( mbDockable )
    {
        const bool bDockingSupportCrippled = !StyleSettings::GetDockingFloatsSupported();

        if ( rNEvt.GetType() == NotifyEventType::MOUSEBUTTONDOWN )
        {
            const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
            if ( pMEvt->IsLeft() )
            {
                if (!bDockingSupportCrippled && pMEvt->IsMod1() && (pMEvt->GetClicks() == 2) )
                {
                    SetFloatingMode( !IsFloatingMode() );
                    if ( IsFloatingMode() )
                        ToTop( ToTopFlags::GrabFocusOnly );
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
        else if( rNEvt.GetType() == NotifyEventType::KEYINPUT )
        {
            const vcl::KeyCode& rKey = rNEvt.GetKeyEvent()->GetKeyCode();
            if( rKey.GetCode() == KEY_F10 && rKey.GetModifier() &&
                rKey.IsShift() && rKey.IsMod1() && !bDockingSupportCrippled )
            {
                SetFloatingMode( !IsFloatingMode() );
                if ( IsFloatingMode() )
                    ToTop( ToTopFlags::GrabFocusOnly );
                return true;
            }
        }
    }

    return Window::EventNotify( rNEvt );
}

void DockingWindow::StartDocking()
{
    mbDocking = true;
}

bool DockingWindow::Docking( const Point&, tools::Rectangle& )
{
    return IsFloatingMode();
}

void DockingWindow::EndDocking( const tools::Rectangle& rRect, bool bFloatMode )
{
    bool bOrigDockCanceled = mbDockCanceled;
    if (bFloatMode && !StyleSettings::GetDockingFloatsSupported())
        mbDockCanceled = true;

    if ( !IsDockingCanceled() )
    {
        if ( bFloatMode != IsFloatingMode() )
        {
            SetFloatingMode( bFloatMode );
            if ( IsFloatingMode() )
                ToTop( ToTopFlags::GrabFocusOnly );
            if ( bFloatMode && mpFloatWin )
                mpFloatWin->SetPosSizePixel( rRect.TopLeft(), rRect.GetSize() );
        }
        if ( !bFloatMode )
        {
            Point aPos = rRect.TopLeft();
            aPos = GetParent()->ScreenToOutputPixel( aPos );
            Window::SetPosSizePixel( aPos, rRect.GetSize() );
        }
    }
    mbDocking = false;
    mbDockCanceled = bOrigDockCanceled;
}

bool DockingWindow::PrepareToggleFloatingMode()
{
    return true;
}

bool DockingWindow::Close()
{
    VclPtr<vcl::Window> xWindow = this;
    CallEventListeners( VclEventId::WindowClose );
    if ( xWindow->isDisposed() )
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
       GenerateAutoMnemonicsOnHierarchy(this);

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
    if ( IsFloatingMode() == bFloatMode )
        return;

    if ( !PrepareToggleFloatingMode() ) // changes to floating mode can be vetoed
        return;

    bool bVisible = IsVisible();

    if ( bFloatMode )
    {
        // set deferred properties early, so border width will end up
        // in our mpWindowImpl->mnBorderWidth, not in mpBorderWindow.
        // (see its usage in setPosSizeOnContainee and GetOptimalSize.)
        setDeferredProperties();

        Show( false, ShowFlags::NoFocusChange );

        maDockPos = Window::GetPosPixel();

        vcl::Window* pRealParent = mpWindowImpl->mpRealParent;
        mpOldBorderWin = mpWindowImpl->mpBorderWindow;

        VclPtrInstance<ImplDockFloatWin> pWin(
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
        pWin->SetMinOutputSizePixel( maMinOutSize );

        pWin->SetMaxOutputSizePixel( mpImplData->maMaxOutSize );
    }
    else
    {
        Show( false, ShowFlags::NoFocusChange );

        // store FloatingData in FloatingWindow
        maFloatPos      = mpFloatWin->GetPosPixel();
        mbDockBtn       = mpFloatWin->IsTitleButtonVisible( TitleButton::Docking );
        mbHideBtn       = mpFloatWin->IsTitleButtonVisible( TitleButton::Hide );
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
    }

    ToggleFloatingMode();

    if (bVisible)
        Show();

    CallEventListeners(VclEventId::WindowToggleFloating);
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

void DockingWindow::setPosSizePixel( tools::Long nX, tools::Long nY,
                                     tools::Long nWidth, tools::Long nHeight,
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
        else if (comphelper::LibreOfficeKit::isActive())
        {
            if ((nFlags & PosSizeFlags::Size) == PosSizeFlags::Size)
                mpFloatWin->SetOutputSizePixel({ nWidth, nHeight });
            if ((nFlags & PosSizeFlags::Pos) == PosSizeFlags::Pos)
                mpFloatWin->SetPosPixel({ nX, nY });
        }
    }

    if (::isLayoutEnabled(this))
        setPosSizeOnContainee();
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
            vcl::WindowData aData;
            aData.setMask(vcl::WindowDataMask::Pos);
            pWrapper->mpFloatWin->GetWindowState( aData );
            AbsoluteScreenPixelPoint aPos(aData.x(), aData.y());
            // LOK needs logic coordinates not absolute screen position for autofilter menu
            if (!comphelper::LibreOfficeKit::isActive())
                return pWrapper->mpFloatWin->GetParent()->ImplGetFrameWindow()->AbsoluteScreenToOutputPixel( aPos );
            return Point(aPos);
        }
        else
            return maFloatPos;
    }

    if ( mpFloatWin )
    {
        vcl::WindowData aData;
        aData.setMask(vcl::WindowDataMask::Pos);
        mpFloatWin->GetWindowState( aData );
        AbsoluteScreenPixelPoint aPos(aData.x(), aData.y());
        return mpFloatWin->GetParent()->ImplGetFrameWindow()->AbsoluteScreenToOutputPixel( aPos );
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
    Size aSize = get_preferred_size();

    Size aMax(bestmaxFrameSizeForScreenSize(Size(GetDesktopRectPixel().GetSize())));

    aSize.setWidth( std::min(aMax.Width(), aSize.Width()) );
    aSize.setHeight( std::min(aMax.Height(), aSize.Height()) );

    SetMinOutputSizePixel(aSize);
    setPosSizeOnContainee();
}

void DockingWindow::setPosSizeOnContainee()
{
    Size aSize = GetOutputSizePixel();

    // Don't make the border width accessible via get_border_width(),
    // otherwise the floating window will handle the border as well.
    sal_Int32 nBorderWidth = mpWindowImpl->mnBorderWidth;

    aSize.AdjustWidth( -(2 * nBorderWidth) );
    aSize.AdjustHeight( -(2 * nBorderWidth) );

    Window* pBox = GetWindow(GetWindowType::FirstChild);
    assert(pBox);
    VclContainer::setLayoutAllocation(*pBox, Point(nBorderWidth, nBorderWidth), aSize);
}

Size DockingWindow::GetOptimalSize() const
{
    if (!isLayoutEnabled())
        return Window::GetOptimalSize();

    Size aSize = VclContainer::getLayoutRequisition(*GetWindow(GetWindowType::FirstChild));

    // Don't make the border width accessible via get_border_width(),
    // otherwise the floating window will handle the border as well.
    sal_Int32 nBorderWidth = mpWindowImpl->mnBorderWidth;

    aSize.AdjustHeight(2 * nBorderWidth );
    aSize.AdjustWidth(2 * nBorderWidth );

    return aSize;
}

void DockingWindow::queue_resize(StateChangedType eReason)
{
    bool bTriggerLayout = true;
    if (maLayoutIdle.IsActive() || mbIsCalculatingInitialLayoutSize)
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

IMPL_LINK_NOARG(DockingWindow, ImplHandleLayoutTimerHdl, Timer*, void)
{
    if (!isLayoutEnabled())
    {
        SAL_WARN_IF(GetWindow(GetWindowType::FirstChild), "vcl.layout", "DockingWindow has become non-layout because extra children have been added directly to it.");
        return;
    }
    setPosSizeOnContainee();
}

void DockingWindow::SetMinOutputSizePixel( const Size& rSize )
{
    if ( mpFloatWin )
        mpFloatWin->SetMinOutputSizePixel( rSize );
    maMinOutSize = rSize;
}

const Size& DockingWindow::GetMinOutputSizePixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetMinOutputSizePixel();
    return maMinOutSize;
}

void DockingWindow::SetFloatingPos( const Point& rNewPos )
{
    if ( mpFloatWin )
        mpFloatWin->SetPosPixel( rNewPos );
    else
        maFloatPos = rNewPos;
}

SystemWindow* DockingWindow::GetFloatingWindow() const
{
    return mpFloatWin;
}

DropdownDockingWindow::DropdownDockingWindow(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rFrame, bool bTearable)
    : DockingWindow(pParent,
                    !bTearable ? u"InterimDockParent"_ustr : u"InterimTearableParent"_ustr,
                    !bTearable ? u"vcl/ui/interimdockparent.ui"_ustr : u"vcl/ui/interimtearableparent.ui"_ustr,
                    "vcl::DropdownDockingWindow maLayoutIdle",
                    rFrame)
    , m_xBox(m_pUIBuilder->get(u"box"))
{
}

DropdownDockingWindow::~DropdownDockingWindow()
{
    disposeOnce();
}

void DropdownDockingWindow::dispose()
{
    m_xBox.reset();
    DockingWindow::dispose();
}

ResizableDockingWindow::ResizableDockingWindow(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rFrame)
    : DockingWindow(pParent, u"DockingWindow"_ustr, u"vcl/ui/dockingwindow.ui"_ustr, "vcl::ResizableDockingWindow maLayoutIdle", rFrame)
    , m_xBox(m_pUIBuilder->get(u"box"))
{
}

ResizableDockingWindow::ResizableDockingWindow(vcl::Window* pParent, WinBits nStyle)
    : DockingWindow(pParent, nStyle, "vcl::ResizableDockingWindow maLayoutIdle")
{
}

// needed to blow away the cached size of the boundary between vcl and hosted child widget
void ResizableDockingWindow::InvalidateChildSizeCache()
{
    // find the bottom vcl::Window of the hierarchy and queue_resize on that
    // one will invalidate all the size caches upwards
    vcl::Window* pChild = GetWindow(GetWindowType::FirstChild);
    while (true)
    {
        vcl::Window* pSubChild = pChild->GetWindow(GetWindowType::FirstChild);
        if (!pSubChild)
            break;
        pChild = pSubChild;
    }
    pChild->queue_resize();
}

ResizableDockingWindow::~ResizableDockingWindow()
{
    disposeOnce();
}

void ResizableDockingWindow::dispose()
{
    m_xBox.reset();
    DockingWindow::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
