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
}

void WorkWindow::ImplInit( vcl::Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData )
{
    sal_uInt16 nFrameStyle = BORDERWINDOW_STYLE_FRAME;
    if ( nStyle & WB_APP )
        nFrameStyle |= BORDERWINDOW_STYLE_APP;

    VclPtrInstance<ImplBorderWindow> pBorderWin( pParent, pSystemParentData, nStyle, nFrameStyle );
    Window::ImplInit( pBorderWin, nStyle & (WB_3DLOOK | WB_CLIPCHILDREN | WB_DIALOGCONTROL | WB_SYSTEMFLOATWIN), nullptr );
    pBorderWin->mpWindowImpl->mpClientWindow = this;
    pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
    mpWindowImpl->mpBorderWindow  = pBorderWin;

    // mpWindowImpl->mpRealParent    = pParent; // should actually be set, but is not set due to errors with the menubar!!

    if ( nStyle & WB_APP )
    {
        ImplSVData* pSVData = ImplGetSVData();
        DBG_ASSERT( !pSVData->maWinData.mpAppWin, "WorkWindow::WorkWindow(): More than one window with style WB_APP" );
        pSVData->maWinData.mpAppWin = this;
    }

    SetActivateMode( ActivateModeFlags::GrabFocus );
}

void WorkWindow::ImplInit( vcl::Window* pParent, WinBits nStyle, const ::com::sun::star::uno::Any& aSystemWorkWindowToken )
{
    if( aSystemWorkWindowToken.hasValue() )
    {
        ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
        aSystemWorkWindowToken >>= aSeq;
        SystemParentData* pData = reinterpret_cast<SystemParentData*>(aSeq.getArray());
        DBG_ASSERT( aSeq.getLength() == sizeof( SystemParentData ) && pData->nSize == sizeof( SystemParentData ), "WorkWindow::WorkWindow( vcl::Window*, const Any&, WinBits ) called with invalid Any" );
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

WorkWindow::WorkWindow( vcl::Window* pParent, const ::com::sun::star::uno::Any& aSystemWorkWindowToken, WinBits nStyle ) :
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
        com::sun::star::uno::Reference< com::sun::star::rendering::XCanvas > xCanvas( mpWindowImpl->mxCanvas );
        if( xCanvas.is() )
        {
            com::sun::star::uno::Reference< com::sun::star::lang::XComponent >
                xCanvasComponent( xCanvas,
                                  com::sun::star::uno::UNO_QUERY );
            if( xCanvasComponent.is() )
                xCanvasComponent->dispose();
        }

        mpWindowImpl->mpFrameWindow->mpWindowImpl->mbWaitSystemResize = true;
        ImplGetFrame()->ShowFullScreen( bFullScreenMode, nDisplayScreen );
    }
}

void WorkWindow::StartPresentationMode( bool bPresentation, PresentationFlags nFlags )
{
    return StartPresentationMode( bPresentation, nFlags, GetScreenNumber());
}

void WorkWindow::StartPresentationMode( bool bPresentation, PresentationFlags nFlags, sal_Int32 nDisplayScreen )
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
    mpWindowImpl->mpFrame->GetWindowState(&aState);
    return (( aState.mnState & WINDOWSTATE_STATE_MINIMIZED ) != 0);
}

bool WorkWindow::SetPluginParent( SystemParentData* pParent )
{
    DBG_ASSERT( ! mbPresentationMode && ! mbFullScreenMode, "SetPluginParent in fullscreen or presentation mode !" );

    bool bWasDnd = Window::ImplStopDnd();

    bool bShown = IsVisible();
    Show( false );
    bool bRet = mpWindowImpl->mpFrame->SetPluginParent( pParent );
    Show( bShown );

    if( bWasDnd )
        Window::ImplStartDnd();

    return bRet;
}

void WorkWindow::ImplSetFrameState( sal_uLong aFrameState )
{
    SalFrameState   aState;
    aState.mnMask   = WINDOWSTATE_MASK_STATE;
    aState.mnState  = aFrameState;
    mpWindowImpl->mpFrame->SetWindowState( &aState );
}

void WorkWindow::Minimize()
{
    ImplSetFrameState( WINDOWSTATE_STATE_MINIMIZED );
}

void WorkWindow::Restore()
{
    ImplSetFrameState( WINDOWSTATE_STATE_NORMAL );
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
    ImplSetFrameState( bMaximize ? WINDOWSTATE_STATE_MAXIMIZED : WINDOWSTATE_STATE_NORMAL );
}

bool WorkWindow::IsMaximized() const
{
    bool bRet = false;

    SalFrameState aState;
    if( mpWindowImpl->mpFrame->GetWindowState( &aState ) )
    {
        if( aState.mnState & (WINDOWSTATE_STATE_MAXIMIZED          |
                              WINDOWSTATE_STATE_MAXIMIZED_HORZ     |
                              WINDOWSTATE_STATE_MAXIMIZED_VERT ) )
            bRet = true;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
