/*************************************************************************
 *
 *  $RCSfile: syswin.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: th $ $Date: 2001-08-23 13:40:45 $
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

#define _SV_SYSWIN_CXX

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
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
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#ifndef _SV_ACCESS_HXX
#include <access.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_BRDWIN_HXX
#include <brdwin.hxx>
#endif
#ifndef _SV_SOUND_HXX
#include <sound.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SYSWIN_HXX
#include <syswin.hxx>
#endif

#include <unowrap.hxx>

#ifdef REMOTE_APPSERVER
#include "rmwindow.hxx"
#endif

#pragma hdrstop

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

// =======================================================================

SystemWindow::SystemWindow( WindowType nType ) :
    Window( nType )
{
    mbSysWin            = TRUE;
    mnActivateMode      = ACTIVATE_MODE_GRABFOCUS;

    mpMenuBar           = NULL;
    mbPined             = FALSE;
    mbRollUp            = FALSE;
    mbRollFunc          = FALSE;
    mbDockBtn           = FALSE;
    mbHideBtn           = FALSE;
    mbSysChild          = FALSE;
    mnMenuBarMode       = MENUBAR_MODE_NORMAL;
    mnIcon              = 0;
}

// -----------------------------------------------------------------------

long SystemWindow::Notify( NotifyEvent& rNEvt )
{
    // Abfangen von KeyEvents fuer Menu-Steuerung
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        MenuBar* pMBar = mpMenuBar;
        if ( !pMBar && ( GetType() == WINDOW_FLOATINGWINDOW ) )
        {
            SystemWindow* pW = (SystemWindow*)ImplGetFrameWindow()->ImplGetWindow();
            if ( pW )
                pMBar = pW->GetMenuBar();
        }
        if ( pMBar && pMBar->ImplHandleKeyEvent( *rNEvt.GetKeyEvent(), FALSE ) )
            return TRUE;
    }

    return Window::Notify( rNEvt );
}

// -----------------------------------------------------------------------

BOOL SystemWindow::Close()
{
    if ( mxWindowPeer.is() )
    {
        // #76482# This window can be destroyed in WindowEvent_Close.
        // => Don't use members after calling WindowEvent_Close
        BOOL bCreatedWithToolkit = IsCreatedWithToolkit();
        Application::GetUnoWrapper()->WindowEvent_Close( this );
        if ( bCreatedWithToolkit )
            return FALSE;
    }

    // Is Window not closeable, ignore close
    Window*     pBorderWin = ImplGetBorderWindow();
    WinBits     nStyle;
    if ( pBorderWin )
        nStyle = pBorderWin->GetStyle();
    else
        nStyle = GetStyle();
    if ( !(nStyle & WB_CLOSEABLE) )
    {
        Sound::Beep( SOUND_DISABLE, this );
        return FALSE;
    }

    Hide();

    // Ist es das Applikationsfenster, dann beende die Applikation
    if ( Application::GetAppWindow() == (const WorkWindow*)this )
        GetpApp()->Quit();

    return TRUE;
}

// -----------------------------------------------------------------------

void SystemWindow::TitleButtonClick( USHORT )
{
}

// -----------------------------------------------------------------------

void SystemWindow::Pin()
{
}

// -----------------------------------------------------------------------

void SystemWindow::Roll()
{
}

// -----------------------------------------------------------------------

void SystemWindow::Resizing( Size& )
{
}

// -----------------------------------------------------------------------

void SystemWindow::SetZLevel( BYTE nLevel )
{
    Window* pWindow = this;
    while ( pWindow->mpBorderWindow )
        pWindow = pWindow->mpBorderWindow;
    if ( pWindow->mbOverlapWin && !pWindow->mbFrame )
    {
        BYTE nOldLevel = pWindow->mpOverlapData->mnTopLevel;
        pWindow->mpOverlapData->mnTopLevel = nLevel;
        // Wenn der neue Level groesser als der alte ist, schieben
        // wir das Fenster nach hinten
        if ( !IsReallyVisible() && (nLevel > nOldLevel) && pWindow->mpNext )
        {
            // Fenster aus der Liste entfernen
            if ( pWindow->mpPrev )
                pWindow->mpPrev->mpNext = pWindow->mpNext;
            else
                pWindow->mpOverlapWindow->mpFirstOverlap = pWindow->mpNext;
            pWindow->mpNext->mpPrev = pWindow->mpPrev;
            pWindow->mpNext = NULL;
            // und Fenster wieder in die Liste am Ende eintragen
            pWindow->mpPrev = pWindow->mpOverlapWindow->mpLastOverlap;
            pWindow->mpOverlapWindow->mpLastOverlap = pWindow;
            pWindow->mpPrev->mpNext = pWindow;
        }
    }
}

// -----------------------------------------------------------------------

void SystemWindow::SetIcon( USHORT nIcon )
{
    if ( mnIcon == nIcon )
        return;

    mnIcon = nIcon;

    if ( !mbSysChild )
    {
        const Window* pWindow = this;
        while ( pWindow->mpBorderWindow )
            pWindow = pWindow->mpBorderWindow;

        if ( pWindow->mbFrame )
            pWindow->mpFrame->SetIcon( nIcon );
    }
}

// -----------------------------------------------------------------------

BYTE SystemWindow::GetZLevel() const
{
    const Window* pWindow = this;
    while ( pWindow->mpBorderWindow )
        pWindow = pWindow->mpBorderWindow;
    if ( pWindow->mpOverlapData )
        return pWindow->mpOverlapData->mnTopLevel;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void SystemWindow::EnableSaveBackground( BOOL bSave )
{
    Window* pWindow = this;
    while ( pWindow->mpBorderWindow )
        pWindow = pWindow->mpBorderWindow;
    if ( pWindow->mbOverlapWin && !pWindow->mbFrame )
    {
        pWindow->mpOverlapData->mbSaveBack = bSave;
        if ( !bSave )
            pWindow->ImplDeleteOverlapBackground();
    }
}

// -----------------------------------------------------------------------

BOOL SystemWindow::IsSaveBackgroundEnabled() const
{
    const Window* pWindow = this;
    while ( pWindow->mpBorderWindow )
        pWindow = pWindow->mpBorderWindow;
    if ( pWindow->mpOverlapData )
        return pWindow->mpOverlapData->mbSaveBack;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void SystemWindow::ShowTitleButton( USHORT nButton, BOOL bVisible )
{
    if ( nButton == TITLE_BUTTON_DOCKING )
    {
        if ( mbDockBtn != bVisible )
        {
            mbDockBtn = bVisible;
            if ( mpBorderWindow )
                ((ImplBorderWindow*)mpBorderWindow)->SetDockButton( bVisible );
        }
    }
    else /* if ( nButton == TITLE_BUTTON_HIDE ) */
    {
        if ( mbHideBtn != bVisible )
        {
            mbHideBtn = bVisible;
            if ( mpBorderWindow )
                ((ImplBorderWindow*)mpBorderWindow)->SetHideButton( bVisible );
        }
    }
}

// -----------------------------------------------------------------------

BOOL SystemWindow::IsTitleButtonVisible( USHORT nButton ) const
{
    if ( nButton == TITLE_BUTTON_DOCKING )
        return mbDockBtn;
    else /* if ( nButton == TITLE_BUTTON_HIDE ) */
        return mbHideBtn;
}

// -----------------------------------------------------------------------

void SystemWindow::SetPin( BOOL bPin )
{
    if ( bPin != mbPined )
    {
        mbPined = bPin;
        if ( mpBorderWindow )
            ((ImplBorderWindow*)mpBorderWindow)->SetPin( bPin );
    }
}

// -----------------------------------------------------------------------

void SystemWindow::RollUp()
{
    if ( !mbRollUp )
    {
        maOrgSize = GetOutputSizePixel();
        mbRollFunc = TRUE;
        Size aSize = maRollUpOutSize;
        if ( !aSize.Width() )
            aSize.Width() = GetOutputSizePixel().Width();
        mbRollUp = TRUE;
        if ( mpBorderWindow )
            ((ImplBorderWindow*)mpBorderWindow)->SetRollUp( TRUE, aSize );
        else
            SetOutputSizePixel( aSize );
        mbRollFunc = FALSE;
    }
}

// -----------------------------------------------------------------------

void SystemWindow::RollDown()
{
    if ( mbRollUp )
    {
        mbRollUp = FALSE;
        if ( mpBorderWindow )
            ((ImplBorderWindow*)mpBorderWindow)->SetRollUp( FALSE, maOrgSize );
        else
            SetOutputSizePixel( maOrgSize );
    }
}

// -----------------------------------------------------------------------

void SystemWindow::SetMinOutputSizePixel( const Size& rSize )
{
    maMinOutSize = rSize;
    if ( mpBorderWindow )
    {
        ((ImplBorderWindow*)mpBorderWindow)->SetMinOutputSize( rSize.Width(), rSize.Height() );
        if ( mpBorderWindow->mbFrame )
            mpBorderWindow->mpFrame->SetMinClientSize( rSize.Width(), rSize.Height() );
    }
    else if ( mbFrame )
        mpFrame->SetMinClientSize( rSize.Width(), rSize.Height() );
}

// -----------------------------------------------------------------------

Size SystemWindow::GetResizeOutputSizePixel() const
{
    Size aSize = GetOutputSizePixel();
    if ( aSize.Width() < maMinOutSize.Width() )
        aSize.Width() = maMinOutSize.Width();
    if ( aSize.Height() < maMinOutSize.Height() )
        aSize.Height() = maMinOutSize.Height();
    return aSize;
}

// -----------------------------------------------------------------------

static void ImplWindowStateFromStr( WindowStateData& rData, const ByteString& rStr )
{
    ULONG       nValidMask  = 0;
    xub_StrLen  nIndex      = 0;
    ByteString  aTokenStr;

    aTokenStr = rStr.GetToken( 0, ',', nIndex );
    if ( aTokenStr.Len() )
    {
        rData.SetX( aTokenStr.ToInt32() );
        nValidMask |= WINDOWSTATE_MASK_X;
    }
    else
        rData.SetX( 0 );
    aTokenStr = rStr.GetToken( 0, ',', nIndex );
    if ( aTokenStr.Len() )
    {
        rData.SetY( aTokenStr.ToInt32() );
        nValidMask |= WINDOWSTATE_MASK_Y;
    }
    else
        rData.SetY( 0 );
    aTokenStr = rStr.GetToken( 0, ',', nIndex );
    if ( aTokenStr.Len() )
    {
        rData.SetWidth( aTokenStr.ToInt32() );
        nValidMask |= WINDOWSTATE_MASK_WIDTH;
    }
    else
        rData.SetWidth( 0 );
    aTokenStr = rStr.GetToken( 0, ';', nIndex );
    if ( aTokenStr.Len() )
    {
        rData.SetHeight( aTokenStr.ToInt32() );
        nValidMask |= WINDOWSTATE_MASK_HEIGHT;
    }
    else
        rData.SetHeight( 0 );
    aTokenStr = rStr.GetToken( 0, ';', nIndex );
    if ( aTokenStr.Len() )
    {
        rData.SetState( (ULONG)aTokenStr.ToInt32() );
        nValidMask |= WINDOWSTATE_MASK_STATE;
    }
    else
        rData.SetState( 0 );
    rData.SetMask( nValidMask );
}

// -----------------------------------------------------------------------

static void ImplWindowStateToStr( const WindowStateData& rData, ByteString& rStr )
{
    ULONG nValidMask = rData.GetMask();
    if ( !nValidMask )
        return;

    if ( nValidMask & WINDOWSTATE_MASK_X )
        rStr.Append( ByteString::CreateFromInt32( rData.GetX() ) );
    rStr.Append( ',' );
    if ( nValidMask & WINDOWSTATE_MASK_Y )
        rStr.Append( ByteString::CreateFromInt32( rData.GetY() ) );
    rStr.Append( ',' );
    if ( nValidMask & WINDOWSTATE_MASK_WIDTH )
        rStr.Append( ByteString::CreateFromInt32( rData.GetWidth() ) );
    rStr.Append( ',' );
    if ( nValidMask & WINDOWSTATE_MASK_HEIGHT )
        rStr.Append( ByteString::CreateFromInt32( rData.GetHeight() ) );
    rStr.Append( ';' );
    if ( nValidMask & WINDOWSTATE_MASK_STATE )
        rStr.Append( ByteString::CreateFromInt32( (long)rData.GetState() ) );
    rStr.Append( ';' );
}

// -----------------------------------------------------------------------

void SystemWindow::SetWindowStateData( const WindowStateData& rData )
{
    ULONG nValidMask = rData.GetMask();
    if ( !nValidMask )
        return;

    if ( mbSysChild )
        return;

    Window* pWindow = this;
    while ( pWindow->mpBorderWindow )
        pWindow = pWindow->mpBorderWindow;

    if ( pWindow->mbFrame )
    {
#ifndef REMOTE_APPSERVER
        ULONG           nState = rData.GetState();
        SalFrameState   aState;
        aState.mnMask   = rData.GetMask();
        aState.mnX      = rData.GetX();
        aState.mnY      = rData.GetY();
        aState.mnWidth  = rData.GetWidth();
        aState.mnHeight = rData.GetHeight();
        aState.mnState  = nState & SAL_FRAMESTATE_SYSTEMMASK;
        mpFrame->SetWindowState( &aState );
#else
        ByteString aStr;
        ImplWindowStateToStr( rData, aStr );
        mpFrame->SetWindowState( ::rtl::OUString( aStr.GetBuffer(), aStr.Len(), RTL_TEXTENCODING_ASCII_US ) );
#endif

        // Syncrones Resize ausloesen, damit wir nach Moeglichkeit gleich
        // mit der richtigen Groesse rechnen
        long nNewWidth;
        long nNewHeight;
        pWindow->mpFrame->GetClientSize( nNewWidth, nNewHeight );
        ImplHandleResize( pWindow, nNewWidth, nNewHeight );
    }
    else
    {
        USHORT nPosSize = 0;
        if ( nValidMask & WINDOWSTATE_MASK_X )
            nPosSize |= WINDOW_POSSIZE_X;
        if ( nValidMask & WINDOWSTATE_MASK_Y )
            nPosSize |= WINDOW_POSSIZE_Y;
        if ( nValidMask & WINDOWSTATE_MASK_WIDTH )
            nPosSize |= WINDOW_POSSIZE_WIDTH;
        if ( nValidMask & WINDOWSTATE_MASK_HEIGHT )
            nPosSize |= WINDOW_POSSIZE_HEIGHT;
        SetPosSizePixel( rData.GetX(), rData.GetY(), rData.GetWidth(), rData.GetHeight(), nPosSize );

        if ( nValidMask & WINDOWSTATE_MASK_STATE )
        {
            ULONG nState = rData.GetState();
            if ( nState & WINDOWSTATE_STATE_ROLLUP )
                RollUp();
            else
                RollDown();
        }
    }
}

// -----------------------------------------------------------------------

void SystemWindow::GetWindowStateData( WindowStateData& rData ) const
{
    ULONG nValidMask = rData.GetMask();
    if ( !nValidMask )
        return;

    if ( mbSysChild )
        return;

    const Window* pWindow = this;
    while ( pWindow->mpBorderWindow )
        pWindow = pWindow->mpBorderWindow;

    if ( pWindow->mbFrame )
    {
#ifndef REMOTE_APPSERVER
        SalFrameState aState;
        aState.mnMask = 0xFFFFFFFF;
        if ( mpFrame->GetWindowState( &aState ) )
        {
            if ( nValidMask & WINDOWSTATE_MASK_X )
                rData.SetX( aState.mnX );
            if ( nValidMask & WINDOWSTATE_MASK_Y )
                rData.SetY( aState.mnY );
            if ( nValidMask & WINDOWSTATE_MASK_WIDTH )
                rData.SetWidth( aState.mnWidth );
            if ( nValidMask & WINDOWSTATE_MASK_HEIGHT )
                rData.SetHeight( aState.mnHeight );
            if ( nValidMask & WINDOWSTATE_MASK_STATE )
                rData.SetState( aState.mnState );
        }
        else
            rData.SetMask( 0 );
#else
        ::rtl::OUString aStr( mpFrame->GetWindowState() );
        ByteString aByteStr( aStr.getStr(), aStr.getLength(), RTL_TEXTENCODING_ASCII_US );
        WindowStateData aData;
        ImplWindowStateFromStr( aData, rStr );
        rData = aData;
#endif
    }
    else
    {
        Point   aPos = GetPosPixel();
        Size    aSize = GetSizePixel();
        ULONG   nState = 0;

        if ( IsRollUp() )
            nState |= WINDOWSTATE_STATE_ROLLUP;

        if ( nValidMask & WINDOWSTATE_MASK_X )
            rData.SetX( aPos.X() );
        if ( nValidMask & WINDOWSTATE_MASK_Y )
            rData.SetY( aPos.Y() );
        if ( nValidMask & WINDOWSTATE_MASK_WIDTH )
            rData.SetWidth( aSize.Width() );
        if ( nValidMask & WINDOWSTATE_MASK_HEIGHT )
            rData.SetHeight( aSize.Height() );
        if ( nValidMask & WINDOWSTATE_MASK_STATE )
            rData.SetState( nState );
    }
}

// -----------------------------------------------------------------------

void SystemWindow::SetWindowState( const ByteString& rStr )
{
    if ( !rStr.Len() )
        return;

    WindowStateData aData;
    ImplWindowStateFromStr( aData, rStr );
    SetWindowStateData( aData );
}

// -----------------------------------------------------------------------

ByteString SystemWindow::GetWindowState( ULONG nMask ) const
{
    WindowStateData aData;
    aData.SetMask( nMask );
    GetWindowStateData( aData );

    ByteString aStr;
    ImplWindowStateToStr( aData, aStr );
    return aStr;
}

// -----------------------------------------------------------------------

void SystemWindow::SetMenuBar( MenuBar* pMenuBar )
{
    if ( mpMenuBar != pMenuBar )
    {
        MenuBar* pOldMenuBar = mpMenuBar;
        Window*  pOldWindow;
        mpMenuBar = pMenuBar;

        if ( mpBorderWindow && (mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) )
        {
            if ( pOldMenuBar )
                pOldWindow = pOldMenuBar->ImplGetWindow();
            else
                pOldWindow = NULL;
            if ( pMenuBar )
            {
                DBG_ASSERT( !pMenuBar->pWindow, "SystemWindow::SetMenuBar() - MenuBars can only set in one SystemWindow at time" );
                ((ImplBorderWindow*)mpBorderWindow)->SetMenuBarWindow( MenuBar::ImplCreate( mpBorderWindow, pOldWindow, pMenuBar ) );
            }
            else
                ((ImplBorderWindow*)mpBorderWindow)->SetMenuBarWindow( NULL );
            ImplToBottomChild();
            if ( pOldMenuBar )
                MenuBar::ImplDestroy( pOldMenuBar, pMenuBar == 0 );
        }

        Application::GenerateAccessEvent( ACCESS_EVENT_MENUBAR );
    }
}

// -----------------------------------------------------------------------

void SystemWindow::SetMenuBarMode( USHORT nMode )
{
    if ( mnMenuBarMode != nMode )
    {
        mnMenuBarMode = nMode;
        if ( mpBorderWindow && (mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) )
        {
            if ( nMode == MENUBAR_MODE_HIDE )
                ((ImplBorderWindow*)mpBorderWindow)->SetMenuBarMode( TRUE );
            else
                ((ImplBorderWindow*)mpBorderWindow)->SetMenuBarMode( FALSE );
        }
    }
}
