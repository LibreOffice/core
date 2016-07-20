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

#include <tools/debug.hxx>
#include <tools/rc.h>

#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
// declare system types in sysdata.hxx
#include <vcl/sysdata.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>

#include <svdata.hxx>
#include <salframe.hxx>
#include <brdwin.hxx>
#include <window.h>

void WorkWindow::ImplInitWorkWindowData()
{
    mnIcon                  = 0; // Should be removed in the next top level update - now in SystemWindow

    mnPresentationFlags     = PresentationFlags::NONE;
    mbPresentationMode      = false;
    mbPresentationVisible   = false;
    mbPresentationFull      = false;
    mbFullScreenMode        = false;

    maLayoutIdle.SetDebugName( "vcl::WorkWindow maLayoutIdle" );
}

void WorkWindow::ImplInit( vcl::Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData )
{
    BorderWindowStyle nFrameStyle = BorderWindowStyle::Frame;
    if ( nStyle & WB_APP )
        nFrameStyle |= BorderWindowStyle::App;

    VclPtrInstance<ImplBorderWindow> pBorderWin( pParent, pSystemParentData, nStyle, nFrameStyle );
    Window::ImplInit( pBorderWin, nStyle & (WB_3DLOOK | WB_CLIPCHILDREN | WB_DIALOGCONTROL | WB_SYSTEMFLOATWIN), nullptr );
    pBorderWin->mpWindowImpl->mpClientWindow = this;
    pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
    mpWindowImpl->mpBorderWindow  = pBorderWin;

    // mpWindowImpl->mpRealParent    = pParent; // should actually be set, but is not set due to errors with the menubar!!

    if ( nStyle & WB_APP )
    {
        ImplSVData* pSVData = ImplGetSVData();
        SAL_WARN_IF( pSVData->maWinData.mpAppWin, "vcl", "WorkWindow::WorkWindow(): More than one window with style WB_APP" );
        pSVData->maWinData.mpAppWin = this;
    }

    SetActivateMode( ActivateModeFlags::GrabFocus );
}

void WorkWindow::ImplInit( vcl::Window* pParent, WinBits nStyle, const css::uno::Any& aSystemWorkWindowToken )
{
    if( aSystemWorkWindowToken.hasValue() )
    {
        css::uno::Sequence< sal_Int8 > aSeq;
        aSystemWorkWindowToken >>= aSeq;
        SystemParentData* pData = reinterpret_cast<SystemParentData*>(aSeq.getArray());
        SAL_WARN_IF( aSeq.getLength() != sizeof( SystemParentData ) || pData->nSize != sizeof( SystemParentData ), "vcl", "WorkWindow::WorkWindow( vcl::Window*, const Any&, WinBits ) called with invalid Any" );
        // init with style 0 as does WorkWindow::WorkWindow( SystemParentData* );
        ImplInit( pParent, 0, pData );
    }
    else
        ImplInit( pParent, nStyle );
}

WorkWindow::WorkWindow( WindowType nType ) :
    SystemWindow( nType )
{
    ImplInitWorkWindowData();
}

WorkWindow::WorkWindow( vcl::Window* pParent, WinBits nStyle ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitWorkWindowData();
    ImplInit( pParent, nStyle );
}

WorkWindow::WorkWindow( vcl::Window* pParent, const css::uno::Any& aSystemWorkWindowToken, WinBits nStyle ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitWorkWindowData();
    mbSysChild = true;
    ImplInit( pParent, nStyle, aSystemWorkWindowToken );
}

WorkWindow::WorkWindow( SystemParentData* pParent ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitWorkWindowData();
    mbSysChild = true;
    ImplInit( nullptr, 0, pParent );
}

WorkWindow::~WorkWindow()
{
    disposeOnce();
}

void WorkWindow::dispose()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpAppWin == this )
    {
        pSVData->maWinData.mpAppWin = nullptr;
        Application::Quit();
    }
    SystemWindow::dispose();
}

void WorkWindow::ShowFullScreenMode( bool bFullScreenMode )
{
    return ShowFullScreenMode( bFullScreenMode, GetScreenNumber());
}

void WorkWindow::ShowFullScreenMode( bool bFullScreenMode, sal_Int32 nDisplayScreen )
{
    if ( !mbFullScreenMode == !bFullScreenMode )
        return;

    mbFullScreenMode = bFullScreenMode;
    if ( !mbSysChild )
    {
        // Dispose of the canvas implementation, which might rely on
        // screen-specific system data.
        css::uno::Reference< css::rendering::XCanvas > xCanvas( mpWindowImpl->mxCanvas );
        if( xCanvas.is() )
        {
            css::uno::Reference< css::lang::XComponent >  xCanvasComponent( xCanvas, css::uno::UNO_QUERY );
            if( xCanvasComponent.is() )
                xCanvasComponent->dispose();
        }

        mpWindowImpl->mpFrameWindow->mpWindowImpl->mbWaitSystemResize = true;
        ImplGetFrame()->ShowFullScreen( bFullScreenMode, nDisplayScreen );
    }
}

void WorkWindow::StartPresentationMode( PresentationFlags nFlags )
{
    return StartPresentationMode( false/*bPresentation*/, nFlags, GetScreenNumber());
}

void WorkWindow::StartPresentationMode( bool bPresentation, PresentationFlags nFlags, sal_uInt32 nDisplayScreen )
{
    if ( !bPresentation == !mbPresentationMode )
        return;

    if ( bPresentation )
    {
        mbPresentationMode      = true;
        mbPresentationVisible   = IsVisible();
        mbPresentationFull      = mbFullScreenMode;
        mnPresentationFlags     = nFlags;

        if ( !(mnPresentationFlags & PresentationFlags::NoFullScreen) )
            ShowFullScreenMode( true, nDisplayScreen );
        if ( !mbSysChild )
        {
            if ( mnPresentationFlags & PresentationFlags::HideAllApps )
                mpWindowImpl->mpFrame->SetAlwaysOnTop( true );
            if ( !(mnPresentationFlags & PresentationFlags::NoAutoShow) )
                ToTop();
            mpWindowImpl->mpFrame->StartPresentation( true );
        }

        if ( !(mnPresentationFlags & PresentationFlags::NoAutoShow) )
            Show();
    }
    else
    {
        Show( mbPresentationVisible );
        if ( !mbSysChild )
        {
            mpWindowImpl->mpFrame->StartPresentation( false );
            if ( mnPresentationFlags & PresentationFlags::HideAllApps )
                mpWindowImpl->mpFrame->SetAlwaysOnTop( false );
        }
        ShowFullScreenMode( mbPresentationFull, nDisplayScreen );

        mbPresentationMode      = false;
        mbPresentationVisible   = false;
        mbPresentationFull      = false;
        mnPresentationFlags     = PresentationFlags::NONE;
    }
}

bool WorkWindow::IsMinimized() const
{
    //return mpWindowImpl->mpFrameData->mbMinimized;
    SalFrameState aState;
    if (mpWindowImpl->mpFrame->GetWindowState(&aState))
        return bool(aState.mnState & WindowStateState::Minimized);
    else
        return false;
}

bool WorkWindow::SetPluginParent( SystemParentData* pParent )
{
    SAL_WARN_IF( mbPresentationMode || mbFullScreenMode, "vcl", "SetPluginParent in fullscreen or presentation mode !" );

    bool bWasDnd = Window::ImplStopDnd();

    bool bShown = IsVisible();
    Show( false );
    bool bRet = mpWindowImpl->mpFrame->SetPluginParent( pParent );
    Show( bShown );

    if( bWasDnd )
        Window::ImplStartDnd();

    return bRet;
}

void WorkWindow::ImplSetFrameState( WindowStateState aFrameState )
{
    SalFrameState   aState;
    aState.mnMask   = WindowStateMask::State;
    aState.mnState  = aFrameState;
    mpWindowImpl->mpFrame->SetWindowState( &aState );
}

void WorkWindow::Minimize()
{
    ImplSetFrameState( WindowStateState::Minimized );
}

void WorkWindow::Restore()
{
    ImplSetFrameState( WindowStateState::Normal );
}

bool WorkWindow::Close()
{
    bool bCanClose = SystemWindow::Close();

    // Ist es das Applikationsfenster, dann beende die Applikation
    if ( bCanClose && ( ImplGetSVData()->maWinData.mpAppWin == this ) )
        Application::Quit();

    return bCanClose;
}

void WorkWindow::Maximize( bool bMaximize )
{
    ImplSetFrameState( bMaximize ? WindowStateState::Maximized : WindowStateState::Normal );
}

bool WorkWindow::IsMaximized() const
{
    bool bRet = false;

    SalFrameState aState;
    if( mpWindowImpl->mpFrame->GetWindowState( &aState ) )
    {
        if( aState.mnState & (WindowStateState::Maximized          |
                              WindowStateState::MaximizedHorz     |
                              WindowStateState::MaximizedVert ) )
            bRet = true;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
