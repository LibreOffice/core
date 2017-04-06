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

#include <brdwin.hxx>
#include <svdata.hxx>
#include <salframe.hxx>
#include <window.h>

#include <vcl/event.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/settings.hxx>

#define DOCKWIN_FLOATSTYLES         (WB_SIZEABLE | WB_MOVEABLE | WB_CLOSEABLE | WB_STANDALONE | WB_ROLLABLE )

class ImplDockFloatWin2 : public FloatingWindow
{
private:
    ImplDockingWindowWrapper*  mpDockWin;
    sal_uInt64      mnLastTicks;
    Idle            maDockIdle;
    Idle            maEndDockIdle;
    Point           maDockPos;
    tools::Rectangle       maDockRect;
    bool            mbInMove;
    ImplSVEvent *   mnLastUserEvent;

    DECL_LINK(DockingHdl, void *, void);
    DECL_LINK(DockTimerHdl, Timer *, void);
    DECL_LINK(EndDockTimerHdl, Timer *, void);
public:
    ImplDockFloatWin2( vcl::Window* pParent, WinBits nWinBits,
                      ImplDockingWindowWrapper* pDockingWin );
    virtual ~ImplDockFloatWin2() override;
    virtual void dispose() override;

    virtual void    Move() override;
    virtual void    Resize() override;
    virtual void    TitleButtonClick( TitleButton nButton ) override;
    virtual void    Resizing( Size& rSize ) override;
    virtual bool    Close() override;
};

ImplDockFloatWin2::ImplDockFloatWin2( vcl::Window* pParent, WinBits nWinBits,
                                    ImplDockingWindowWrapper* pDockingWin ) :
        FloatingWindow( pParent, nWinBits ),
        mpDockWin( pDockingWin ),
        mnLastTicks( tools::Time::GetSystemTicks() ),
        mbInMove( false ),
        mnLastUserEvent( nullptr )
{
    // copy state of DockingWindow
    if ( pDockingWin )
    {
        SetSettings( pDockingWin->GetWindow()->GetSettings() );
        Enable( pDockingWin->GetWindow()->IsEnabled(), false );
        EnableInput( pDockingWin->GetWindow()->IsInputEnabled(), false );
        AlwaysEnableInput( pDockingWin->GetWindow()->IsAlwaysEnableInput(), false );
        EnableAlwaysOnTop( pDockingWin->GetWindow()->IsAlwaysOnTopEnabled() );
        SetActivateMode( pDockingWin->GetWindow()->GetActivateMode() );
    }

    SetBackground( GetSettings().GetStyleSettings().GetFaceColor() );

    maDockIdle.SetInvokeHandler( LINK( this, ImplDockFloatWin2, DockTimerHdl ) );
    maDockIdle.SetPriority( TaskPriority::HIGH_IDLE );
    maDockIdle.SetDebugName( "vcl::ImplDockFloatWin2 maDockIdle" );

    maEndDockIdle.SetInvokeHandler( LINK( this, ImplDockFloatWin2, EndDockTimerHdl ) );
    maDockIdle.SetPriority( TaskPriority::HIGH_IDLE );
    maEndDockIdle.SetDebugName( "vcl::ImplDockFloatWin2 maEndDockIdle" );
}

ImplDockFloatWin2::~ImplDockFloatWin2()
{
    disposeOnce();
}

void ImplDockFloatWin2::dispose()
{
    if( mnLastUserEvent )
        Application::RemoveUserEvent( mnLastUserEvent );
    FloatingWindow::dispose();
}

IMPL_LINK_NOARG(ImplDockFloatWin2, DockTimerHdl, Timer *, void)
{
    SAL_WARN_IF( !mpDockWin->IsFloatingMode(), "vcl", "docktimer called but not floating" );

    maDockIdle.Stop();
    PointerState aState = GetPointerState();

    if( aState.mnState & KEY_MOD1 )
    {
        // i43499 CTRL disables docking now
        mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->HideTracking();
        if( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) )
            maDockIdle.Start();
    }
    else if( ! ( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) )
    {
        mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->HideTracking();
        mpDockWin->EndDocking( maDockRect, false );
    }
    else
    {
        mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->ShowTracking( maDockRect, ShowTrackFlags::Big | ShowTrackFlags::TrackWindow );
        maDockIdle.Start();
    }
}

IMPL_LINK_NOARG(ImplDockFloatWin2, EndDockTimerHdl, Timer *, void)
{
    SAL_WARN_IF( !mpDockWin->IsFloatingMode(), "vcl", "enddocktimer called but not floating" );

    maEndDockIdle.Stop();
    PointerState aState = GetPointerState();
    if( ! ( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) )
    {
        mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->HideTracking();
        mpDockWin->EndDocking( maDockRect, true );
    }
    else
    {
        maEndDockIdle.Start();
    }
}

IMPL_LINK_NOARG(ImplDockFloatWin2, DockingHdl, void*, void)
{
    // called during move of a floating window
    mnLastUserEvent = nullptr;

    vcl::Window *pDockingArea = mpDockWin->GetWindow()->GetParent();
    PointerState aState = pDockingArea->GetPointerState();

    bool bRealMove = true;
    if( GetStyle() & WB_OWNERDRAWDECORATION )
    {
        // for windows with ownerdraw decoration
        // we allow docking only when the window was moved
        // by dragging its caption
        // and ignore move request due to resizing
        vcl::Window *pBorder = GetWindow( GetWindowType::Border );
        if( pBorder != this )
        {
            tools::Rectangle aBorderRect( Point(), pBorder->GetSizePixel() );
            sal_Int32 nLeft, nTop, nRight, nBottom;
            GetBorder( nLeft, nTop, nRight, nBottom );
            // limit borderrect to the caption part only and without the resizing borders
            aBorderRect.Bottom() = aBorderRect.Top() + nTop;
            aBorderRect.Left() += nLeft;
            aBorderRect.Right() -= nRight;

            PointerState aBorderState = pBorder->GetPointerState();
            if( aBorderRect.IsInside( aBorderState.maPos ) )
                bRealMove = true;
            else
                bRealMove = false;
        }
    }

    if( mpDockWin->GetWindow()->IsVisible() &&
        (tools::Time::GetSystemTicks() - mnLastTicks > 500) &&
        ( aState.mnState & ( MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT ) ) &&
        !(aState.mnState & KEY_MOD1) && // i43499 CTRL disables docking now
        bRealMove )
    {
        maDockPos = Point( pDockingArea->OutputToScreenPixel( pDockingArea->AbsoluteScreenToOutputPixel( OutputToAbsoluteScreenPixel( Point() ) ) ) );
        maDockRect = tools::Rectangle( maDockPos, mpDockWin->GetSizePixel() );

        // mouse pos in screen pixels
        Point aMousePos = pDockingArea->OutputToScreenPixel( aState.maPos );

        if( ! mpDockWin->IsDocking() )
            mpDockWin->StartDocking( aMousePos, maDockRect );

        bool bFloatMode = mpDockWin->Docking( aMousePos, maDockRect );

        if( ! bFloatMode )
        {
            // indicates that the window could be docked at maDockRect
            maDockRect.SetPos( mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->ScreenToOutputPixel(
                 maDockRect.TopLeft() ) );
            mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->ShowTracking( maDockRect, ShowTrackFlags::Big | ShowTrackFlags::TrackWindow );
            maEndDockIdle.Stop();
            DockTimerHdl( nullptr );
        }
        else
        {
            mpDockWin->GetWindow()->GetParent()->ImplGetFrameWindow()->HideTracking();
            maDockIdle.Stop();
            EndDockTimerHdl( nullptr );
        }
    }
    mbInMove = false;
}

void ImplDockFloatWin2::Move()
{
    if( mbInMove )
        return;

    mbInMove = true;
    FloatingWindow::Move();
    mpDockWin->GetWindow()->Move();

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
        mnLastUserEvent = Application::PostUserEvent( LINK( this, ImplDockFloatWin2, DockingHdl ), nullptr, true );
}

void ImplDockFloatWin2::Resize()
{
    // forwarding of resize only required if we have no borderwindow ( GetWindow() then returns 'this' )
    if( GetWindow( GetWindowType::Border ) == this )
    {
        FloatingWindow::Resize();
        Size aSize( GetSizePixel() );
        mpDockWin->GetWindow()->ImplPosSizeWindow( 0, 0, aSize.Width(), aSize.Height(), PosSizeFlags::PosSize ); // TODO: is this needed ???
    }
}

void ImplDockFloatWin2::TitleButtonClick( TitleButton nButton )
{
    FloatingWindow::TitleButtonClick( nButton );
    mpDockWin->TitleButtonClick( nButton );
}

void ImplDockFloatWin2::Resizing( Size& rSize )
{
    FloatingWindow::Resizing( rSize );
    mpDockWin->Resizing( rSize );
}

bool ImplDockFloatWin2::Close()
{
    return true;
}

DockingManager::DockingManager()
{
}

DockingManager::~DockingManager()
{
    ::std::vector< ImplDockingWindowWrapper* >::iterator p;
    p = mDockingWindows.begin();
    for(; p != mDockingWindows.end(); ++p )
    {
        delete (*p);
    }
    mDockingWindows.clear();
}

ImplDockingWindowWrapper* DockingManager::GetDockingWindowWrapper( const vcl::Window *pWindow )
{
    ::std::vector< ImplDockingWindowWrapper* >::iterator p;
    p = mDockingWindows.begin();
    while( p != mDockingWindows.end() )
    {
        if( (*p)->mpDockingWindow == pWindow )
            return (*p);
        else
            ++p;
    }
    return nullptr;
}

bool DockingManager::IsDockable( const vcl::Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );

    /*
    if( pWindow->HasDockingHandler() )
        return true;
    */
    return (pWrapper != nullptr);
}

bool DockingManager::IsFloating( const vcl::Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        return pWrapper->IsFloatingMode();
    else
        return false;
}

bool DockingManager::IsLocked( const vcl::Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper && pWrapper->IsLocked() )
        return true;
    else
        return false;
}

void DockingManager::Lock( const vcl::Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        pWrapper->Lock();
}

void DockingManager::Unlock( const vcl::Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        pWrapper->Unlock();
}

void DockingManager::SetFloatingMode( const vcl::Window *pWindow, bool bFloating )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        pWrapper->SetFloatingMode( bFloating );
}

void DockingManager::StartPopupMode( ToolBox *pParentToolBox, const vcl::Window *pWindow, FloatWinPopupFlags nFlags )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        pWrapper->StartPopupMode( pParentToolBox, nFlags );
}

void DockingManager::StartPopupMode( ToolBox *pParentToolBox, const vcl::Window *pWindow )
{
    StartPopupMode( pParentToolBox, pWindow, FloatWinPopupFlags::AllowTearOff         |
                    FloatWinPopupFlags::AllMouseButtonClose  |
                    FloatWinPopupFlags::NoMouseUpClose );
}

bool DockingManager::IsInPopupMode( const vcl::Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper && pWrapper->IsInPopupMode() )
        return true;
    else
        return false;
}

void DockingManager::EndPopupMode( const vcl::Window *pWin )
{
    ImplDockingWindowWrapper *pWrapper = GetDockingWindowWrapper( pWin );
    if( pWrapper && pWrapper->GetFloatingWindow() && pWrapper->GetFloatingWindow()->IsInPopupMode() )
        pWrapper->GetFloatingWindow()->EndPopupMode();
}

void DockingManager::AddWindow( const vcl::Window *pWindow )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        return;
    else
        pWrapper = new ImplDockingWindowWrapper( pWindow );

    mDockingWindows.push_back( pWrapper );
}

void DockingManager::RemoveWindow( const vcl::Window *pWindow )
{
    ::std::vector< ImplDockingWindowWrapper* >::iterator p;
    p = mDockingWindows.begin();
    while( p != mDockingWindows.end() )
    {
        if( (*p)->mpDockingWindow == pWindow )
        {
            delete (*p);
            mDockingWindows.erase( p );
            break;
        }
        else
            ++p;
    }
}

void DockingManager::SetPosSizePixel( vcl::Window *pWindow, long nX, long nY,
                                    long nWidth, long nHeight,
                                    PosSizeFlags nFlags )
{
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        pWrapper->setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

tools::Rectangle DockingManager::GetPosSizePixel( const vcl::Window *pWindow )
{
    tools::Rectangle aRect;
    ImplDockingWindowWrapper* pWrapper = GetDockingWindowWrapper( pWindow );
    if( pWrapper )
        aRect = tools::Rectangle( pWrapper->GetPosPixel(), pWrapper->GetSizePixel() );

    return aRect;
}

class ImplPopupFloatWin : public FloatingWindow
{
private:
    ImplDockingWindowWrapper*   mpDockingWin;
    bool                        mbMoving;
    bool                        mbTrackingEnabled;
    Point                       maDelta;
    bool                        mbHasGrip;
    void                        ImplSetBorder();

public:
    ImplPopupFloatWin( vcl::Window* pParent, ImplDockingWindowWrapper* pDockingWin, bool bHasGrip );
    virtual ~ImplPopupFloatWin() override;
    virtual void dispose() override;

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;
    virtual void        Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void        MouseMove( const MouseEvent& rMEvt ) override;
    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void        Tracking( const TrackingEvent& rTEvt ) override;
    virtual void        Resize() override;

    tools::Rectangle           GetDragRect() const;
    Point               GetToolboxPosition() const;
    void                DrawGrip(vcl::RenderContext& rRenderContext);
    void                DrawBorder(vcl::RenderContext& rRenderContext);

    bool                hasGrip() const { return mbHasGrip; }
};

ImplPopupFloatWin::ImplPopupFloatWin( vcl::Window* pParent, ImplDockingWindowWrapper* pDockingWin, bool bHasGrip ) :
    FloatingWindow( pParent, WB_NOBORDER | WB_SYSTEMWINDOW | WB_NOSHADOW )
{
    mpWindowImpl->mbToolbarFloatingWindow = true;   // indicate window type, required for accessibility
                                                    // which should not see this window as a toplevel window
    mpDockingWin = pDockingWin;
    mbMoving = false;
    mbTrackingEnabled = false;
    mbHasGrip = bHasGrip;

    ImplSetBorder();
}

ImplPopupFloatWin::~ImplPopupFloatWin()
{
    disposeOnce();
}

void ImplPopupFloatWin::dispose()
{
    mpDockingWin = nullptr;
    FloatingWindow::dispose();
}

css::uno::Reference< css::accessibility::XAccessible > ImplPopupFloatWin::CreateAccessible()
{
    // switch off direct accessibility support for this window

    // this is to avoid appearance of this window as standalone window in the accessibility hierarchy
    // as this window is only used as a helper for subtoolbars that are not teared-off, the parent toolbar
    // has to provide accessibility support (as implemented in the toolkit)
    // so the contained toolbar should appear as child of the corresponding toolbar item of the parent toolbar
    return css::uno::Reference< css::accessibility::XAccessible >();
}

void ImplPopupFloatWin::ImplSetBorder()
{
    // although we have no border in the sense of a borderwindow
    //  we're using a special border for the grip
    // by setting those members the method SetOutputSizePixel() can
    //  be used to set the proper window size
    mpWindowImpl->mnTopBorder     = 1;
    if( hasGrip() )
        mpWindowImpl->mnTopBorder += 1 + ToolBox::ImplGetDragWidth( *this, false );
    mpWindowImpl->mnBottomBorder  = 1;
    mpWindowImpl->mnLeftBorder    = 1;
    mpWindowImpl->mnRightBorder   = 1;
}

void ImplPopupFloatWin::Resize()
{
    // the borderview overwrites the border during resize so restore it
    ImplSetBorder();
}

tools::Rectangle ImplPopupFloatWin::GetDragRect() const
{
    if( !hasGrip() )
        return tools::Rectangle();
    return tools::Rectangle( 1, 1, GetOutputSizePixel().Width() - 1,
                      2 + ToolBox::ImplGetDragWidth( *this, false ) );
}

Point ImplPopupFloatWin::GetToolboxPosition() const
{
    // return inner position where a toolbox could be placed
    return Point( 1, 1 + ( hasGrip() ? GetDragRect().getHeight() : 0 ) );    // grip + border
}

void ImplPopupFloatWin::DrawBorder(vcl::RenderContext& rRenderContext)
{
    rRenderContext.SetFillColor();
    tools::Rectangle aRect( Point(), GetOutputSizePixel() );

    vcl::Region oldClipRgn( GetClipRegion( ) );
    vcl::Region aClipRgn( aRect );
    tools::Rectangle aItemClipRect( ImplGetItemEdgeClipRect() );
    if( !aItemClipRect.IsEmpty() )
    {
        aItemClipRect.SetPos( AbsoluteScreenToOutputPixel( aItemClipRect.TopLeft() ) );

        // draw the excluded border part with the background color of a toolbox
        rRenderContext.SetClipRegion( vcl::Region( aItemClipRect ) );
        rRenderContext.SetLineColor( GetSettings().GetStyleSettings().GetFaceColor() );
        rRenderContext.DrawRect( aRect );

        aClipRgn.Exclude( aItemClipRect );
        SetClipRegion( aClipRgn );
    }
    rRenderContext.SetLineColor( rRenderContext.GetSettings().GetStyleSettings().GetShadowColor() );
    rRenderContext.DrawRect( aRect );
    rRenderContext.SetClipRegion( oldClipRgn );
}

void ImplPopupFloatWin::DrawGrip(vcl::RenderContext& rRenderContext)
{
    bool bLinecolor     = rRenderContext.IsLineColor();
    Color aLinecolor    = rRenderContext.GetLineColor();
    bool bFillcolor     = rRenderContext.IsFillColor();
    Color aFillcolor    = rRenderContext.GetFillColor();

    if (!ToolBox::AlwaysLocked())  // no grip if toolboxes are locked
    {
        ToolBox::ImplDrawGrip(rRenderContext, GetDragRect(),
                              ToolBox::ImplGetDragWidth( *this, false ), WindowAlign::Left, false );
    }

    if (bLinecolor)
        rRenderContext.SetLineColor(aLinecolor);
    else
        rRenderContext.SetLineColor();
    if (bFillcolor)
        rRenderContext.SetFillColor(aFillcolor);
    else
        rRenderContext.SetFillColor();
}

void ImplPopupFloatWin::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    DrawBorder(rRenderContext);
    if (hasGrip())
        DrawGrip(rRenderContext);
}

void ImplPopupFloatWin::MouseMove( const MouseEvent& rMEvt )
{
    Point aMousePos = rMEvt.GetPosPixel();

    if( !ToolBox::AlwaysLocked() )  // no tear off if locking is enabled
    {
        if( mbTrackingEnabled && rMEvt.IsLeft() && GetDragRect().IsInside( aMousePos ) )
        {
            // start window move
            mbMoving = true;
            StartTracking( StartTrackingFlags::NoKeyCancel );
            return;
        }
        if( GetDragRect().IsInside( aMousePos ) )
        {
            if( GetPointer().GetStyle() != PointerStyle::Move )
                SetPointer( Pointer( PointerStyle::Move ) );
        }
        if( rMEvt.IsLeaveWindow() || !GetDragRect().IsInside( aMousePos ) )
        {
            if( GetPointer().GetStyle() != PointerStyle::Arrow )
                SetPointer( Pointer( PointerStyle::Arrow ) );
        }
    }
}

void ImplPopupFloatWin::MouseButtonUp( const MouseEvent& rMEvt )
{
    mbTrackingEnabled = false;
    FloatingWindow::MouseButtonUp( rMEvt );
}

void ImplPopupFloatWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aMousePos = rMEvt.GetPosPixel();
    if( GetDragRect().IsInside( aMousePos ) )
    {
        // get mouse pos at a static window to have a fixed reference point
        PointerState aState = GetParent()->GetPointerState();
        if (HasMirroredGraphics() && IsRTLEnabled())
            ImplMirrorFramePos(aState.maPos);
        maDelta = aState.maPos - GetWindow( GetWindowType::Border )->GetPosPixel();
        mbTrackingEnabled = true;
    }
    else
    {
        mbTrackingEnabled = false;
    }
}

void ImplPopupFloatWin::Tracking( const TrackingEvent& rTEvt )
{
    if( mbMoving )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbMoving = false;
            EndPopupMode( FloatWinPopupEndFlags::TearOff );
        }
        else if ( !rTEvt.GetMouseEvent().IsSynthetic() )
        {
            // move the window according to mouse pos
            PointerState aState = GetParent()->GetPointerState();
            const OutputDevice *pOutDev = GetOutDev();
            if (pOutDev->HasMirroredGraphics() && IsRTLEnabled())
                ImplMirrorFramePos(aState.maPos);
            GetWindow( GetWindowType::Border )->SetPosPixel( aState.maPos - maDelta );
        }
    }
}

ImplDockingWindowWrapper::ImplDockingWindowWrapper( const vcl::Window *pWindow )
    : mpDockingWindow(const_cast<vcl::Window*>(pWindow))
    , mpFloatWin(nullptr)
    , mpOldBorderWin(nullptr)
    , mpParent(pWindow->GetParent())
    , maMaxOutSize( SHRT_MAX, SHRT_MAX )
    , mnTrackX(0)
    , mnTrackY(0)
    , mnTrackWidth(0)
    , mnTrackHeight(0)
    , mnDockLeft(0)
    , mnDockTop(0)
    , mnDockRight(0)
    , mnDockBottom(0)
    , mnFloatBits(WB_BORDER | WB_CLOSEABLE | WB_SIZEABLE | (pWindow->GetStyle() & DOCKWIN_FLOATSTYLES))
    , mbDockCanceled(false)
    , mbDocking(false)
    , mbLastFloatMode(false)
    , mbStartFloat(false)
    , mbRollUp(false)
    , mbDockBtn(false)
    , mbHideBtn(false)
    // must be enabled in Window::Notify to prevent permanent docking during mouse move
    , mbStartDockingEnabled(false)
    , mbLocked(false)
{
    DockingWindow *pDockWin = dynamic_cast< DockingWindow* > ( mpDockingWindow.get() );
    if( pDockWin )
        mnFloatBits = pDockWin->GetFloatStyle();
}

ImplDockingWindowWrapper::~ImplDockingWindowWrapper()
{
    if ( IsFloatingMode() )
    {
        GetWindow()->Show( false, ShowFlags::NoFocusChange );
        SetFloatingMode(false);
    }
}

bool ImplDockingWindowWrapper::ImplStartDocking( const Point& rPos )
{
    if( !mbStartDockingEnabled )
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
        pWin = VclPtr<ImplDockFloatWin2>::Create( mpParent, mnFloatBits, nullptr );
    pWin->GetBorder( mnDockLeft, mnDockTop, mnDockRight, mnDockBottom );
    if ( !mpFloatWin )
        pWin.disposeAndClear();

    Point   aPos    = GetWindow()->ImplOutputToFrame( Point() );
    Size    aSize   = GetWindow()->GetOutputSizePixel();
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

    vcl::Window *pDockingArea = GetWindow()->GetParent();
    vcl::Window::PointerState aState = pDockingArea->GetPointerState();

    // mouse pos in screen pixels
    Point aMousePos = pDockingArea->OutputToScreenPixel( aState.maPos );
    Point aDockPos = Point( pDockingArea->AbsoluteScreenToOutputPixel( GetWindow()->OutputToAbsoluteScreenPixel( GetWindow()->GetPosPixel() ) ) );
    tools::Rectangle aDockRect( aDockPos, GetWindow()->GetSizePixel() );
    StartDocking( aMousePos, aDockRect );

    GetWindow()->ImplUpdateAll();
    GetWindow()->ImplGetFrameWindow()->ImplUpdateAll();

    GetWindow()->StartTracking( StartTrackingFlags::KeyMod );
    return true;
}

void ImplDockingWindowWrapper::Tracking( const TrackingEvent& rTEvt )
{
    // used during docking of a currently docked window
    if ( mbDocking )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbDocking = false;
            GetWindow()->HideTracking();
            if ( rTEvt.IsTrackingCanceled() )
            {
                mbDockCanceled = true;
                EndDocking( tools::Rectangle( Point( mnTrackX, mnTrackY ), Size( mnTrackWidth, mnTrackHeight ) ), mbLastFloatMode );
                mbDockCanceled = false;
            }
            else
                EndDocking( tools::Rectangle( Point( mnTrackX, mnTrackY ), Size( mnTrackWidth, mnTrackHeight ) ), mbLastFloatMode );
        }
        // Docking only upon non-synthetic MouseEvents
        else if ( !rTEvt.GetMouseEvent().IsSynthetic() || rTEvt.GetMouseEvent().IsModifierChanged() )
        {
            Point   aMousePos = rTEvt.GetMouseEvent().GetPosPixel();
            Point   aFrameMousePos = GetWindow()->ImplOutputToFrame( aMousePos );
            Size    aFrameSize = GetWindow()->ImplGetFrameWindow()->GetOutputSizePixel();
            if ( aFrameMousePos.X() < 0 )
                aFrameMousePos.X() = 0;
            if ( aFrameMousePos.Y() < 0 )
                aFrameMousePos.Y() = 0;
            if ( aFrameMousePos.X() > aFrameSize.Width()-1 )
                aFrameMousePos.X() = aFrameSize.Width()-1;
            if ( aFrameMousePos.Y() > aFrameSize.Height()-1 )
                aFrameMousePos.Y() = aFrameSize.Height()-1;
            aMousePos = GetWindow()->ImplFrameToOutput( aFrameMousePos );
            aMousePos.X() -= maMouseOff.X();
            aMousePos.Y() -= maMouseOff.Y();
            Point aPos = GetWindow()->ImplOutputToFrame( aMousePos );
            tools::Rectangle aTrackRect( aPos, Size( mnTrackWidth, mnTrackHeight ) );
            tools::Rectangle aCompRect = aTrackRect;
            aPos.X()    += maMouseOff.X();
            aPos.Y()    += maMouseOff.Y();

            bool bFloatMode = Docking( aPos, aTrackRect );

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

            ShowTrackFlags nTrackStyle;
            if ( bFloatMode )
                nTrackStyle = ShowTrackFlags::Object;
            else
                nTrackStyle = ShowTrackFlags::Big;
            tools::Rectangle aShowTrackRect = aTrackRect;
            aShowTrackRect.SetPos( GetWindow()->ImplFrameToOutput( aShowTrackRect.TopLeft() ) );

            GetWindow()->ShowTracking( aShowTrackRect, nTrackStyle );

            // calculate mouse offset again, as the rectangle was changed
            maMouseOff.X()  = aPos.X() - aTrackRect.Left();
            maMouseOff.Y()  = aPos.Y() - aTrackRect.Top();

            mnTrackX        = aTrackRect.Left();
            mnTrackY        = aTrackRect.Top();
            mnTrackWidth    = aTrackRect.GetWidth();
            mnTrackHeight   = aTrackRect.GetHeight();
        }
    }
}

void ImplDockingWindowWrapper::StartDocking( const Point& rPoint, tools::Rectangle& rRect )
{
    DockingData data( rPoint, rRect, IsFloatingMode() );

    GetWindow()->CallEventListeners( VclEventId::WindowStartDocking, &data );
    mbDocking = true;
}

bool ImplDockingWindowWrapper::Docking( const Point& rPoint, tools::Rectangle& rRect )
{
    DockingData data( rPoint, rRect, IsFloatingMode() );

    GetWindow()->CallEventListeners( VclEventId::WindowDocking, &data );
    rRect = data.maTrackRect;
    return data.mbFloating;
}

void ImplDockingWindowWrapper::EndDocking( const tools::Rectangle& rRect, bool bFloatMode )
{
    tools::Rectangle aRect( rRect );

    bool bOrigDockCanceled = mbDockCanceled;
    if (bFloatMode && !StyleSettings::GetDockingFloatsSupported())
        mbDockCanceled = true;

    if ( !IsDockingCanceled() )
    {
        bool bShow = false;
        if ( bFloatMode != IsFloatingMode() )
        {
            GetWindow()->Show( false, ShowFlags::NoFocusChange );
            SetFloatingMode( bFloatMode );
            bShow = true;
            if ( bFloatMode )
            {
                // #i44800# always use outputsize - as in all other places
                mpFloatWin->SetOutputSizePixel( aRect.GetSize() );
                mpFloatWin->SetPosPixel( aRect.TopLeft() );
            }
        }
        if ( !bFloatMode )
        {
            Point aPos = aRect.TopLeft();
            aPos = GetWindow()->GetParent()->ScreenToOutputPixel( aPos );
            GetWindow()->SetPosSizePixel( aPos, aRect.GetSize() );
        }

        if ( bShow )
            GetWindow()->Show( true, ShowFlags::NoFocusChange | ShowFlags::NoActivate );
    }

    EndDockingData data( aRect, IsFloatingMode(), IsDockingCanceled() );
    GetWindow()->CallEventListeners( VclEventId::WindowEndDocking, &data );

    mbDocking = false;

    // must be enabled in Window::Notify to prevent permanent docking during mouse move
    mbStartDockingEnabled = false;

    mbDockCanceled = bOrigDockCanceled;
}

bool ImplDockingWindowWrapper::PrepareToggleFloatingMode()
{
    bool bFloating = true;
    GetWindow()->CallEventListeners( VclEventId::WindowPrepareToggleFloating, &bFloating );
    return bFloating;
}

void ImplDockingWindowWrapper::ToggleFloatingMode()
{
    // notify dockingwindow/toolbox
    // note: this must be done *before* notifying the
    //       listeners to have the toolbox in the proper state
    if( GetWindow()->IsDockingWindow() )
        static_cast<DockingWindow*>(GetWindow())->ToggleFloatingMode();

    // now notify listeners
    GetWindow()->CallEventListeners( VclEventId::WindowToggleFloating );

    // must be enabled in Window::Notify to prevent permanent docking during mouse move
    mbStartDockingEnabled = false;
}

void ImplDockingWindowWrapper::TitleButtonClick( TitleButton nType )
{
    if( nType == TitleButton::Menu )
    {
        ToolBox *pToolBox = dynamic_cast< ToolBox* >( GetWindow() );
        if( pToolBox )
        {
            pToolBox->ExecuteCustomMenu();
        }
    }
    if( nType == TitleButton::Docking )
    {
        SetFloatingMode( !IsFloatingMode() );
    }
}

void ImplDockingWindowWrapper::Resizing( Size& rSize )
{
    // TODO: add virtual Resizing() to class Window, so we can get rid of class DockingWindow
    DockingWindow *pDockingWindow = dynamic_cast< DockingWindow* >( GetWindow() );
    if( pDockingWindow )
        pDockingWindow->Resizing( rSize );
}

void ImplDockingWindowWrapper::ShowTitleButton( TitleButton nButton, bool bVisible )
{
    if ( mpFloatWin )
        mpFloatWin->ShowTitleButton( nButton, bVisible );
    else
    {
        if ( nButton == TitleButton::Docking )
            mbDockBtn = bVisible;
        else // if ( nButton == TitleButton::Hide )
            mbHideBtn = bVisible;
    }
}

void ImplDockingWindowWrapper::StartPopupMode( ToolBox *pParentToolBox, FloatWinPopupFlags nFlags )
{
    // do nothing if window is floating
    if( IsFloatingMode() )
        return;

    GetWindow()->Show( false, ShowFlags::NoFocusChange );

    // prepare reparenting
    vcl::Window* pRealParent = GetWindow()->GetWindow( GetWindowType::Parent );
    mpOldBorderWin = GetWindow()->GetWindow( GetWindowType::Border );
    if( mpOldBorderWin.get() == GetWindow() )
        mpOldBorderWin = nullptr;  // no border window found

    bool bAllowTearOff = bool( nFlags & FloatWinPopupFlags::AllowTearOff );
    bool bUseStdPopup = bAllowTearOff && GetWindow()->GetType() != WindowType::TOOLBOX;

    // the new parent for popup mode
    VclPtr<FloatingWindow> pWin;
    if ( bUseStdPopup )
        pWin = VclPtr<FloatingWindow>::Create( mpParent, WB_STDPOPUP );
    else
        pWin = VclPtr<ImplPopupFloatWin>::Create( mpParent, this, bAllowTearOff );
    pWin->SetPopupModeEndHdl( LINK( this, ImplDockingWindowWrapper, PopupModeEnd ) );
    pWin->SetText( GetWindow()->GetText() );

    pWin->SetOutputSizePixel( GetWindow()->GetSizePixel() );

    GetWindow()->mpWindowImpl->mpBorderWindow  = nullptr;
    GetWindow()->mpWindowImpl->mnLeftBorder    = 0;
    GetWindow()->mpWindowImpl->mnTopBorder     = 0;
    GetWindow()->mpWindowImpl->mnRightBorder   = 0;
    GetWindow()->mpWindowImpl->mnBottomBorder  = 0;

    // position toolbox below the drag grip
    if ( !bUseStdPopup )
        GetWindow()->SetPosPixel( static_cast<ImplPopupFloatWin*>( pWin.get() )->GetToolboxPosition() );

    // reparent borderwindow and window
    if ( mpOldBorderWin )
        mpOldBorderWin->SetParent( pWin );
    GetWindow()->SetParent( pWin );

    // correct border window pointers
    GetWindow()->mpWindowImpl->mpBorderWindow = pWin;
    pWin->mpWindowImpl->mpClientWindow = GetWindow();
    GetWindow()->mpWindowImpl->mpRealParent = pRealParent;

    // set mpFloatWin not until all window positioning is done !!!
    // (SetPosPixel etc. check for valid mpFloatWin pointer)
    mpFloatWin = pWin;

    // if the subtoolbar was opened via keyboard make sure that key events
    // will go into subtoolbar
    if( pParentToolBox->IsKeyEvent() )
        nFlags |= FloatWinPopupFlags::GrabFocus;

    mpFloatWin->StartPopupMode( pParentToolBox, nFlags );
    GetWindow()->Show();

    if( pParentToolBox->IsKeyEvent() )
    {
        // send HOME key to subtoolbar in order to select first item
        KeyEvent aEvent( 0, vcl::KeyCode( KEY_HOME ) );
        GetWindow()->KeyInput(aEvent);
    }
}

IMPL_LINK_NOARG(ImplDockingWindowWrapper, PopupModeEnd, FloatingWindow*, void)
{
    GetWindow()->Show( false, ShowFlags::NoFocusChange );

    // set parameter for handler before destroying floating window
    EndPopupModeData aData( mpFloatWin->GetWindow( GetWindowType::Border )->GetPosPixel(), mpFloatWin->IsPopupModeTearOff() );

    // before deleting change parent back, so we can delete the floating window alone
    vcl::Window* pRealParent = GetWindow()->GetWindow( GetWindowType::Parent );
    GetWindow()->mpWindowImpl->mpBorderWindow = nullptr;
    if ( mpOldBorderWin )
    {
        GetWindow()->SetParent( mpOldBorderWin );
        static_cast<ImplBorderWindow*>(mpOldBorderWin.get())->GetBorder(
            GetWindow()->mpWindowImpl->mnLeftBorder, GetWindow()->mpWindowImpl->mnTopBorder,
            GetWindow()->mpWindowImpl->mnRightBorder, GetWindow()->mpWindowImpl->mnBottomBorder );
        mpOldBorderWin->Resize();
    }
    GetWindow()->mpWindowImpl->mpBorderWindow = mpOldBorderWin;
    GetWindow()->SetParent( pRealParent );
    GetWindow()->mpWindowImpl->mpRealParent = pRealParent;

    mpFloatWin.disposeAndClear();

    // call handler - which will destroy the window and thus the wrapper as well !
    GetWindow()->CallEventListeners( VclEventId::WindowEndPopupMode, &aData );
}

bool ImplDockingWindowWrapper::IsInPopupMode() const
{
    if( GetFloatingWindow() )
        return GetFloatingWindow()->IsInPopupMode();
    else
        return false;
}

void ImplDockingWindowWrapper::SetFloatingMode( bool bFloatMode )
{
    // do nothing if window is docked and locked
    if( !IsFloatingMode() && IsLocked() )
        return;

    if ( IsFloatingMode() != bFloatMode )
    {
        if ( PrepareToggleFloatingMode() )
        {
            bool bVisible = GetWindow()->IsVisible();

            if ( bFloatMode )
            {
                GetWindow()->Show( false, ShowFlags::NoFocusChange );

                maDockPos = GetWindow()->GetPosPixel();

                vcl::Window* pRealParent = GetWindow()->GetWindow( GetWindowType::Parent );
                mpOldBorderWin = GetWindow()->GetWindow( GetWindowType::Border );
                if( mpOldBorderWin == mpDockingWindow )
                    mpOldBorderWin = nullptr;  // no border window found

                VclPtrInstance<ImplDockFloatWin2> pWin(
                                         mpParent,
                                         mnFloatBits & ( WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE ) ?
                                          mnFloatBits | WB_SYSTEMWINDOW
                                          | WB_OWNERDRAWDECORATION
                                          : mnFloatBits,
                                         this );

                GetWindow()->mpWindowImpl->mpBorderWindow  = nullptr;
                GetWindow()->mpWindowImpl->mnLeftBorder    = 0;
                GetWindow()->mpWindowImpl->mnTopBorder     = 0;
                GetWindow()->mpWindowImpl->mnRightBorder   = 0;
                GetWindow()->mpWindowImpl->mnBottomBorder  = 0;

                // if the parent gets destroyed, we also have to reset the parent of the BorderWindow
                if ( mpOldBorderWin )
                    mpOldBorderWin->SetParent( pWin );
                GetWindow()->SetParent( pWin );
                pWin->SetPosPixel( Point() );

                GetWindow()->mpWindowImpl->mpBorderWindow = pWin;
                pWin->mpWindowImpl->mpClientWindow = mpDockingWindow;
                GetWindow()->mpWindowImpl->mpRealParent = pRealParent;

                pWin->SetText( GetWindow()->GetText() );
                pWin->SetOutputSizePixel( GetWindow()->GetSizePixel() );
                pWin->SetPosPixel( maFloatPos );
                // pass on DockingData to FloatingWindow
                pWin->ShowTitleButton( TitleButton::Docking, mbDockBtn );
                pWin->ShowTitleButton( TitleButton::Hide, mbHideBtn );
                if ( mbRollUp )
                    pWin->RollUp();
                else
                    pWin->RollDown();
                pWin->SetRollUpOutputSizePixel( maRollUpOutSize );
                pWin->SetMinOutputSizePixel( maMinOutSize );
                pWin->SetMaxOutputSizePixel( maMaxOutSize );

                mpFloatWin      = pWin;

                if ( bVisible )
                    GetWindow()->Show( true, ShowFlags::NoFocusChange | ShowFlags::NoActivate );

                ToggleFloatingMode();
            }
            else
            {
                GetWindow()->Show( false, ShowFlags::NoFocusChange );

                // store FloatingData in FloatingWindow
                maFloatPos      = mpFloatWin->GetPosPixel();
                mbDockBtn       = mpFloatWin->IsTitleButtonVisible( TitleButton::Docking );
                mbHideBtn       = mpFloatWin->IsTitleButtonVisible( TitleButton::Hide );
                mbRollUp        = mpFloatWin->IsRollUp();
                maRollUpOutSize = mpFloatWin->GetRollUpOutputSizePixel();
                maMinOutSize    = mpFloatWin->GetMinOutputSizePixel();
                maMaxOutSize    = mpFloatWin->GetMaxOutputSizePixel();

                vcl::Window* pRealParent = GetWindow()->GetWindow( GetWindowType::Parent ); //mpWindowImpl->mpRealParent;
                GetWindow()->mpWindowImpl->mpBorderWindow = nullptr;
                if ( mpOldBorderWin )
                {
                    GetWindow()->SetParent( mpOldBorderWin );
                    static_cast<ImplBorderWindow*>(mpOldBorderWin.get())->GetBorder(
                        GetWindow()->mpWindowImpl->mnLeftBorder, GetWindow()->mpWindowImpl->mnTopBorder,
                        GetWindow()->mpWindowImpl->mnRightBorder, GetWindow()->mpWindowImpl->mnBottomBorder );
                    mpOldBorderWin->Resize();
                }
                GetWindow()->mpWindowImpl->mpBorderWindow = mpOldBorderWin;
                GetWindow()->SetParent( pRealParent );
                GetWindow()->mpWindowImpl->mpRealParent = pRealParent;

                mpFloatWin.disposeAndClear();
                GetWindow()->SetPosPixel( maDockPos );

                if ( bVisible )
                    GetWindow()->Show();

                ToggleFloatingMode();

            }
        }
    }
}

void ImplDockingWindowWrapper::SetFloatStyle( WinBits nStyle )
{
    mnFloatBits = nStyle;
}


void ImplDockingWindowWrapper::setPosSizePixel( long nX, long nY,
                                     long nWidth, long nHeight,
                                     PosSizeFlags nFlags )
{
    if ( mpFloatWin )
        mpFloatWin->setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
    else
        GetWindow()->setPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

Point ImplDockingWindowWrapper::GetPosPixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetPosPixel();
    else
        return mpDockingWindow->GetPosPixel();
}

Size ImplDockingWindowWrapper::GetSizePixel() const
{
    if ( mpFloatWin )
        return mpFloatWin->GetSizePixel();
    else
        return mpDockingWindow->GetSizePixel();
}

// old inlines from DockingWindow

void ImplDockingWindowWrapper::SetMinOutputSizePixel( const Size& rSize )
{
    if ( mpFloatWin )
        mpFloatWin->SetMinOutputSizePixel( rSize );
    maMinOutSize = rSize;
}

void ImplDockingWindowWrapper::SetMaxOutputSizePixel( const Size& rSize )
{
    if ( mpFloatWin )
        mpFloatWin->SetMaxOutputSizePixel( rSize );
    maMaxOutSize = rSize;
}

bool ImplDockingWindowWrapper::IsFloatingMode() const
{
    return (mpFloatWin != nullptr);
}

void    ImplDockingWindowWrapper::SetDragArea( const tools::Rectangle& rRect )
{
    maDragArea = rRect;
}


void ImplDockingWindowWrapper::Lock()
{
    mbLocked = true;
    // only toolbars support locking
    ToolBox *pToolBox = dynamic_cast< ToolBox * >( GetWindow() );
    if( pToolBox )
        pToolBox->Lock( mbLocked );
}

void ImplDockingWindowWrapper::Unlock()
{
    mbLocked = false;
    // only toolbars support locking
    ToolBox *pToolBox = dynamic_cast< ToolBox * >( GetWindow() );
    if( pToolBox )
        pToolBox->Lock( mbLocked );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
