/*************************************************************************
 *
 *  $RCSfile: wrkwin.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-20 14:12:10 $
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

#define _SV_WRKWIN_CXX

#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#else
#include <rmwindow.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_BRDWIN_HXX
#include <brdwin.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_OPENGL_HXX
#include <opengl.hxx>
#endif
#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif

#include <rvp.hxx>

#pragma hdrstop

// =======================================================================

#define WORKWIN_WINDOWSTATE_FULLSCREEN          ((ULONG)0x00010000)
#define WORKWIN_WINDOWSTATE_ALL                 ((ULONG)0x00FF0000)

// =======================================================================

void WorkWindow::ImplInitData()
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
#ifdef REMOTE_APPSERVER
    static ::com::sun::star::uno::Any aVoid;
    DBG_ASSERT( ! pSystemParentData, "SystemParentData not implemented in remote vcl" );
    ImplInit( pParent, nStyle, aVoid );
#else
#if defined WNT
    /*
     * #98153# since SystemParentData typically contains a HWND from
     * another process and the OpenGL implementation does not like
     * our child window of another processes frame we disable it here.
     */
    if( pSystemParentData )
        OpenGL::Invalidate();
#endif
    USHORT nFrameStyle = BORDERWINDOW_STYLE_FRAME;
    if ( nStyle & WB_APP )
        nFrameStyle |= BORDERWINDOW_STYLE_APP;

    ImplBorderWindow* pBorderWin = new ImplBorderWindow( pParent, pSystemParentData, nStyle, nFrameStyle );
    Window::ImplInit( pBorderWin, nStyle & (WB_3DLOOK | WB_CLIPCHILDREN | WB_DIALOGCONTROL | WB_SYSTEMFLOATWIN), NULL );
    pBorderWin->mpClientWindow = this;
    pBorderWin->GetBorder( mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder );
    mpBorderWindow  = pBorderWin;
//        mpRealParent    = pParent; // !!! Muesste eigentlich gesetzt werden, aber wegen Fehlern mit dem MenuBar erstmal nicht gesetzt !!!

    if ( nStyle & WB_APP )
    {
        ImplSVData* pSVData = ImplGetSVData();
        DBG_ASSERT( !pSVData->maWinData.mpAppWin, "WorkWindow::WorkWindow(): More than one window with style WB_APP" );
        pSVData->maWinData.mpAppWin = this;
    }

    SetActivateMode( ACTIVATE_MODE_GRABFOCUS );
#endif
}

// -----------------------------------------------------------------------

void WorkWindow::ImplInit( Window* pParent, WinBits nStyle, const ::com::sun::star::uno::Any& aSystemWorkWindowToken )
{
#ifndef REMOTE_APPSERVER
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
#else
    USHORT nFrameStyle = BORDERWINDOW_STYLE_FRAME;
    if ( nStyle & WB_APP )
        nFrameStyle |= BORDERWINDOW_STYLE_APP;
    ImplBorderWindow* pBorderWin  = new ImplBorderWindow( pParent, nStyle, nFrameStyle, aSystemWorkWindowToken );
    Window::ImplInit( pBorderWin, nStyle & (WB_3DLOOK | WB_CLIPCHILDREN | WB_DIALOGCONTROL), aSystemWorkWindowToken );
    pBorderWin->mpClientWindow = this;
    pBorderWin->GetBorder( mnLeftBorder, mnTopBorder, mnRightBorder, mnBottomBorder );
    mpBorderWindow  = pBorderWin;
//        mpRealParent    = pParent; // !!! Muesste eigentlich gesetzt werden, aber wegen Fehlern mit dem MenuBar erstmal nicht gesetzt !!!

    if ( nStyle & WB_APP )
    {
        ImplSVData* pSVData = ImplGetSVData();
        DBG_ASSERT( !pSVData->maWinData.mpAppWin, "WorkWindow::WorkWindow(): More than one window with style WB_APP" );
        pSVData->maWinData.mpAppWin = this;
    }

    SetActivateMode( ACTIVATE_MODE_GRABFOCUS );
#endif
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( WindowType nType ) :
    SystemWindow( nType )
{
    ImplInitData();
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( Window* pParent, WinBits nStyle ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitData();
    ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( Window* pParent, const ResId& rResId ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitData();
    rResId.SetRT( RSC_WORKWIN );
    ImplInit( pParent, ImplInitRes( rResId ) );
    ImplLoadRes( rResId );
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( Window* pParent, const ::com::sun::star::uno::Any& aSystemWorkWindowToken, WinBits nStyle ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitData();
    mbSysChild = TRUE;
    ImplInit( pParent, nStyle, aSystemWorkWindowToken );
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( SystemParentData* pParent ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitData();
    mbSysChild = TRUE;
    ImplInit( NULL, 0, pParent );
}

// -----------------------------------------------------------------------

void WorkWindow::ImplLoadRes( const ResId& rResId )
{
    SystemWindow::ImplLoadRes( rResId );

    USHORT nShowStyle   = ReadShortRes();
    if ( !(rResId.aWinBits & WB_HIDE) && (RSC_WORKWIN == rResId.GetRT()) )
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

void WorkWindow::ShowFullScreenMode( BOOL bFullScreenMode )
{
    if ( !mbFullScreenMode == !bFullScreenMode )
        return;

    mbFullScreenMode = bFullScreenMode != 0;
    if ( !mbSysChild )
    {
        mpFrameWindow->mbWaitSystemResize = TRUE;
        ImplGetFrame()->ShowFullScreen( bFullScreenMode );
    }
}

// -----------------------------------------------------------------------

void WorkWindow::StartPresentationMode( BOOL bPresentation, USHORT nFlags )
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
            ShowFullScreenMode( TRUE );
#ifndef REMOTE_APPSERVER
        if ( !mbSysChild )
        {
            if ( mnPresentationFlags & PRESENTATION_HIDEALLAPPS )
                mpFrame->SetAlwaysOnTop( TRUE );
            if ( !(mnPresentationFlags & PRESENTATION_NOAUTOSHOW) )
                ToTop();
            mpFrame->StartPresentation( TRUE );
        }
#else
        if ( !mbSysChild )
            mpFrame->StartPresentation( TRUE, nFlags | PRESENTATION_NOFULLSCREEN | PRESENTATION_NOAUTOSHOW );
#endif

        if ( !(mnPresentationFlags & PRESENTATION_NOAUTOSHOW) )
            Show();
    }
    else
    {
        Show( mbPresentationVisible );
#ifndef REMOTE_APPSERVER
        if ( !mbSysChild )
        {
            mpFrame->StartPresentation( FALSE );
            if ( mnPresentationFlags & PRESENTATION_HIDEALLAPPS )
                mpFrame->SetAlwaysOnTop( FALSE );
        }
#else
        if ( !mbSysChild )
            mpFrame->StartPresentation( FALSE, mnPresentationFlags | PRESENTATION_NOFULLSCREEN | PRESENTATION_NOAUTOSHOW );
#endif
        ShowFullScreenMode( mbPresentationFull );

        mbPresentationMode      = FALSE;
        mbPresentationVisible   = FALSE;
        mbPresentationFull      = FALSE;
        mnPresentationFlags     = 0;
    }
}

// -----------------------------------------------------------------------

BOOL WorkWindow::IsMinimized() const
{
    //return mpFrameData->mbMinimized;
    SalFrameState aState;
    mpFrame->GetWindowState(&aState);
    return (( aState.mnState & SAL_FRAMESTATE_MINIMIZED ) != 0);
}

// -----------------------------------------------------------------------

BOOL WorkWindow::SetPluginParent( SystemParentData* pParent )
{
    DBG_ASSERT( ! mbPresentationMode && ! mbFullScreenMode, "SetPluginParent in fullscreen or presentation mode !" );

    bool bWasDnd = Window::ImplStopDnd();

    BOOL bShown = IsVisible();
    Show( FALSE );
    BOOL bRet = mpFrame->SetPluginParent( pParent );
    Show( bShown );

    if( bWasDnd )
        Window::ImplStartDnd();

    return bRet;
}
