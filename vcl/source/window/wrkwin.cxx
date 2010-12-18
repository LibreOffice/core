/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <svsys.h>
#include <vcl/salframe.hxx>
#include <tools/debug.hxx>

#include <tools/rc.h>
#include <vcl/svdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/brdwin.hxx>
#include <vcl/window.h>
#include <vcl/wrkwin.hxx>
#include <vcl/sysdata.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>

// =======================================================================

#define WORKWIN_WINDOWSTATE_FULLSCREEN          ((ULONG)0x00010000)
#define WORKWIN_WINDOWSTATE_ALL                 ((ULONG)0x00FF0000)

// =======================================================================

void WorkWindow::ImplInitWorkWindowData()
{
    mnIcon                  = 0; // Should be removed in the next top level update - now in SystemWindow

    mnPresentationFlags     = 0;
    mbPresentationMode      = FALSE;
    mbPresentationVisible   = FALSE;
    mbPresentationFull      = FALSE;
    mbFullScreenMode        = FALSE;
}

// -----------------------------------------------------------------------

void WorkWindow::ImplInit( Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData )
{
    USHORT nFrameStyle = BORDERWINDOW_STYLE_FRAME;
    if ( nStyle & WB_APP )
        nFrameStyle |= BORDERWINDOW_STYLE_APP;

    ImplBorderWindow* pBorderWin = new ImplBorderWindow( pParent, pSystemParentData, nStyle, nFrameStyle );
    Window::ImplInit( pBorderWin, nStyle & (WB_3DLOOK | WB_CLIPCHILDREN | WB_DIALOGCONTROL | WB_SYSTEMFLOATWIN), NULL );
    pBorderWin->mpWindowImpl->mpClientWindow = this;
    pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
    mpWindowImpl->mpBorderWindow  = pBorderWin;
//        mpWindowImpl->mpRealParent    = pParent; // !!! Muesste eigentlich gesetzt werden, aber wegen Fehlern mit dem MenuBar erstmal nicht gesetzt !!!

    if ( nStyle & WB_APP )
    {
        ImplSVData* pSVData = ImplGetSVData();
        DBG_ASSERT( !pSVData->maWinData.mpAppWin, "WorkWindow::WorkWindow(): More than one window with style WB_APP" );
        pSVData->maWinData.mpAppWin = this;
    }

    SetActivateMode( ACTIVATE_MODE_GRABFOCUS );
}

// -----------------------------------------------------------------------

void WorkWindow::ImplInit( Window* pParent, WinBits nStyle, const ::com::sun::star::uno::Any& aSystemWorkWindowToken )
{
    if( aSystemWorkWindowToken.hasValue() )
    {
        ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
        aSystemWorkWindowToken >>= aSeq;
        SystemParentData* pData = (SystemParentData*)aSeq.getArray();
        DBG_ASSERT( aSeq.getLength() == sizeof( SystemParentData ) && pData->nSize == sizeof( SystemParentData ), "WorkWindow::WorkWindow( Window*, const Any&, WinBits ) called with invalid Any" );
        // init with style 0 as does WorkWindow::WorkWindow( SystemParentData* );
        ImplInit( pParent, 0, pData );
    }
    else
        ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( WindowType nType ) :
    SystemWindow( nType )
{
    ImplInitWorkWindowData();
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( Window* pParent, WinBits nStyle ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitWorkWindowData();
    ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( Window* pParent, const ResId& rResId ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitWorkWindowData();
    rResId.SetRT( RSC_WORKWIN );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( Window* pParent, const ::com::sun::star::uno::Any& aSystemWorkWindowToken, WinBits nStyle ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitWorkWindowData();
    mbSysChild = TRUE;
    ImplInit( pParent, nStyle, aSystemWorkWindowToken );
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( SystemParentData* pParent ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitWorkWindowData();
    mbSysChild = TRUE;
    ImplInit( NULL, 0, pParent );
}

// -----------------------------------------------------------------------

void WorkWindow::ImplLoadRes( const ResId& rResId )
{
    SystemWindow::ImplLoadRes( rResId );

    ReadLongRes();
    if ( !(rResId.GetWinBits() & WB_HIDE) && (RSC_WORKWIN == rResId.GetRT()) )
        Show();
}

// -----------------------------------------------------------------------

WorkWindow::~WorkWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpAppWin == this )
    {
        pSVData->maWinData.mpAppWin = NULL;
        Application::Quit();
    }
}

// -----------------------------------------------------------------------

void WorkWindow::ShowFullScreenMode( BOOL bFullScreenMode, sal_Int32 nDisplay )
{
    if ( !mbFullScreenMode == !bFullScreenMode )
        return;

    if( (nDisplay < 0)
    || (nDisplay >= static_cast<sal_Int32>(Application::GetScreenCount()) ) )
    {
        nDisplay = GetScreenNumber();
    }

    mbFullScreenMode = bFullScreenMode != 0;
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

        mpWindowImpl->mpFrameWindow->mpWindowImpl->mbWaitSystemResize = TRUE;
        ImplGetFrame()->ShowFullScreen( bFullScreenMode, nDisplay );
    }
}

// -----------------------------------------------------------------------

void WorkWindow::StartPresentationMode( BOOL bPresentation, USHORT nFlags, sal_Int32 nDisplay )
{
    if ( !bPresentation == !mbPresentationMode )
        return;

    if ( bPresentation )
    {
        mbPresentationMode      = TRUE;
        mbPresentationVisible   = IsVisible();
        mbPresentationFull      = mbFullScreenMode;
        mnPresentationFlags     = nFlags;

        if ( !(mnPresentationFlags & PRESENTATION_NOFULLSCREEN) )
            ShowFullScreenMode( TRUE, nDisplay );
        if ( !mbSysChild )
        {
            if ( mnPresentationFlags & PRESENTATION_HIDEALLAPPS )
                mpWindowImpl->mpFrame->SetAlwaysOnTop( TRUE );
            if ( !(mnPresentationFlags & PRESENTATION_NOAUTOSHOW) )
                ToTop();
            mpWindowImpl->mpFrame->StartPresentation( TRUE );
        }

        if ( !(mnPresentationFlags & PRESENTATION_NOAUTOSHOW) )
            Show();
    }
    else
    {
        Show( mbPresentationVisible );
        if ( !mbSysChild )
        {
            mpWindowImpl->mpFrame->StartPresentation( FALSE );
            if ( mnPresentationFlags & PRESENTATION_HIDEALLAPPS )
                mpWindowImpl->mpFrame->SetAlwaysOnTop( FALSE );
        }
        ShowFullScreenMode( mbPresentationFull, nDisplay );

        mbPresentationMode      = FALSE;
        mbPresentationVisible   = FALSE;
        mbPresentationFull      = FALSE;
        mnPresentationFlags     = 0;
    }
}

// -----------------------------------------------------------------------

BOOL WorkWindow::IsMinimized() const
{
    //return mpWindowImpl->mpFrameData->mbMinimized;
    SalFrameState aState;
    mpWindowImpl->mpFrame->GetWindowState(&aState);
    return (( aState.mnState & SAL_FRAMESTATE_MINIMIZED ) != 0);
}

// -----------------------------------------------------------------------

BOOL WorkWindow::SetPluginParent( SystemParentData* pParent )
{
    DBG_ASSERT( ! mbPresentationMode && ! mbFullScreenMode, "SetPluginParent in fullscreen or presentation mode !" );

    bool bWasDnd = Window::ImplStopDnd();

    BOOL bShown = IsVisible();
    Show( FALSE );
    BOOL bRet = mpWindowImpl->mpFrame->SetPluginParent( pParent );
    Show( bShown );

    if( bWasDnd )
        Window::ImplStartDnd();

    return bRet;
}

void WorkWindow::ImplSetFrameState( ULONG aFrameState )
{
    SalFrameState   aState;
    aState.mnMask   = SAL_FRAMESTATE_MASK_STATE;
    aState.mnState  = aFrameState;
    mpWindowImpl->mpFrame->SetWindowState( &aState );
}


void WorkWindow::Minimize()
{
    ImplSetFrameState( SAL_FRAMESTATE_MINIMIZED );
}

void WorkWindow::Restore()
{
    ImplSetFrameState( SAL_FRAMESTATE_NORMAL );
}

BOOL WorkWindow::Close()
{
    BOOL bCanClose = SystemWindow::Close();

    // Ist es das Applikationsfenster, dann beende die Applikation
    if ( bCanClose && ( ImplGetSVData()->maWinData.mpAppWin == this ) )
        GetpApp()->Quit();

    return bCanClose;
}

void WorkWindow::Maximize( BOOL bMaximize )
{
    ImplSetFrameState( bMaximize ? SAL_FRAMESTATE_MAXIMIZED : SAL_FRAMESTATE_NORMAL );
}

BOOL WorkWindow::IsMaximized() const
{
    BOOL bRet = FALSE;

    SalFrameState aState;
    if( mpWindowImpl->mpFrame->GetWindowState( &aState ) )
    {
        if( aState.mnState & (SAL_FRAMESTATE_MAXIMIZED          |
                              SAL_FRAMESTATE_MAXIMIZED_HORZ     |
                              SAL_FRAMESTATE_MAXIMIZED_VERT ) )
            bRet = TRUE;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
