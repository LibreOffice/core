/*************************************************************************
 *
 *  $RCSfile: syswin.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:32:28 $
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
#ifndef _SV_TASKPANELIST_HXX
#include <taskpanelist.hxx>
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
    mpTaskPaneList      = NULL;
}

// -----------------------------------------------------------------------

long SystemWindow::Notify( NotifyEvent& rNEvt )
{
    // capture KeyEvents for menu handling
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        MenuBar* pMBar = mpMenuBar;
        if ( !pMBar && ( GetType() == WINDOW_FLOATINGWINDOW ) )
        {
            Window* pWin = ImplGetFrameWindow()->ImplGetWindow();
            if( pWin && pWin->IsSystemWindow() )
                pMBar = ((SystemWindow*)pWin)->GetMenuBar();
        }
        if ( pMBar && pMBar->ImplHandleKeyEvent( *rNEvt.GetKeyEvent(), FALSE ) )
            return TRUE;
    }

    return Window::Notify( rNEvt );
}

// -----------------------------------------------------------------------

long SystemWindow::PreNotify( NotifyEvent& rNEvt )
{
    // capture KeyEvents for taskpane cycling
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if( rNEvt.GetKeyEvent()->GetKeyCode().GetCode() == KEY_F6 &&
            rNEvt.GetKeyEvent()->GetKeyCode().IsMod1() &&
           !rNEvt.GetKeyEvent()->GetKeyCode().IsShift() )
        {
            // Ctrl-F6 goes directly to the document
            GrabFocusToDocument();
            return TRUE;
        }
        else
        {
            TaskPaneList *pTList = mpTaskPaneList;
            if( !pTList && ( GetType() == WINDOW_FLOATINGWINDOW ) )
            {
                Window* pWin = ImplGetFrameWindow()->ImplGetWindow();
                if( pWin && pWin->IsSystemWindow() )
                    pTList = ((SystemWindow*)pWin)->mpTaskPaneList;
            }
            if( !pTList )
            {
                // search topmost system window which is the one to handle dialog/toolbar cycling
                SystemWindow *pSysWin = this;
                Window *pWin = this;
                while( pWin )
                {
                    pWin = pWin->GetParent();
                    if( pWin && pWin->IsSystemWindow() )
                        pSysWin = (SystemWindow*) pWin;
                }
                pTList = pSysWin->mpTaskPaneList;
            }
            if( pTList && pTList->HandleKeyEvent( *rNEvt.GetKeyEvent() ) )
                return TRUE;
        }
    }
    return Window::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

TaskPaneList* SystemWindow::GetTaskPaneList()
{
    if( mpTaskPaneList )
        return mpTaskPaneList ;
    else
    {
        mpTaskPaneList = new TaskPaneList();
        MenuBar* pMBar = mpMenuBar;
        if ( !pMBar && ( GetType() == WINDOW_FLOATINGWINDOW ) )
        {
            Window* pWin = ImplGetFrameWindow()->ImplGetWindow();
            if ( pWin && pWin->IsSystemWindow() )
                pMBar = ((SystemWindow*)pWin)->GetMenuBar();
        }
        if( pMBar )
            mpTaskPaneList->AddWindow( pMBar->ImplGetWindow() );
        return mpTaskPaneList;
    }
}

// -----------------------------------------------------------------------

BOOL SystemWindow::Close()
{
    ImplDelData aDelData;
    ImplAddDel( &aDelData );
    ImplCallEventListeners( VCLEVENT_WINDOW_CLOSE );
    if ( aDelData.IsDelete() )
        return FALSE;
    ImplRemoveDel( &aDelData );

    if ( mxWindowPeer.is() && IsCreatedWithToolkit() )
        return FALSE;

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
        // #94144# allow Minimize again, should be masked out when read from configuration
        // 91625 - ignore Minimize
        ULONG nState = (ULONG)aTokenStr.ToInt32();
        //nState &= ~(WINDOWSTATE_STATE_MINIMIZED);
        rData.SetState( nState );
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
    {
        // #94144# allow Minimize again, should be masked out when read from configuration
        // 91625 - ignore Minimize
        ULONG nState = rData.GetState();
        //nState &= ~(WINDOWSTATE_STATE_MINIMIZED);
        rStr.Append( ByteString::CreateFromInt32( (long)nState ) );
    }
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
        // #94144# allow Minimize again, should be masked out when read from configuration
        // 91625 - ignore Minimize
        //nState &= ~(WINDOWSTATE_STATE_MINIMIZED);
        aState.mnState  = nState & SAL_FRAMESTATE_SYSTEMMASK;

        // #96568# avoid having multiple frames at the same screen location
        if( rData.GetMask() & (WINDOWSTATE_MASK_POS|WINDOWSTATE_MASK_WIDTH|WINDOWSTATE_MASK_HEIGHT) )
        {
            Rectangle aDesktop = GetDesktopRectPixel();
            ImplSVData *pSVData = ImplGetSVData();
            Window *pWin = pSVData->maWinData.mpFirstFrame;
            BOOL bWrapped = FALSE;
            while( pWin )
            {
                if( !pWin->ImplIsRealParentPath( this ) &&
                    pWin->ImplGetWindow()->IsTopWindow() && pWin->mbReallyVisible )
                {
                    SalFrameGeometry g = pWin->mpFrame->GetGeometry();
                    if( abs(g.nX-aState.mnX) < 2 && abs(g.nY-aState.mnY) < 5 )
                    {
                        long displacement = g.nTopDecoration ? g.nTopDecoration : 20;
                        if( aState.mnX + displacement + aState.mnWidth + g.nRightDecoration > aDesktop.nRight ||
                            aState.mnY + displacement + aState.mnHeight + g.nBottomDecoration > aDesktop.nBottom )
                        {
                            // displacing would leave screen
                            aState.mnX = g.nLeftDecoration ? g.nLeftDecoration : 10; // should result in (0,0)
                            aState.mnY = displacement;
                            if( bWrapped ||
                                aState.mnX + displacement + aState.mnWidth + g.nRightDecoration > aDesktop.nRight ||
                                aState.mnY + displacement + aState.mnHeight + g.nBottomDecoration > aDesktop.nBottom )
                                break;  // further displacement not possible -> break
                            // avoid endless testing
                            bWrapped = TRUE;
                        }
                        else
                        {
                            // displace
                            aState.mnX += displacement;
                            aState.mnY += displacement;
                        }
                    pWin = pSVData->maWinData.mpFirstFrame; // check new pos again
                    }
                }
                pWin = pWin->mpFrameData->mpNextFrame;
            }

        }

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

        if( IsRollUp() )
            RollDown();

        long nX         = rData.GetX();
        long nY         = rData.GetY();
        long nWidth     = rData.GetWidth();
        long nHeight    = rData.GetHeight();
        const SalFrameGeometry& rGeom = pWindow->mpFrame->GetGeometry();
        if( nX < 0 )
            nX = 0;
        if( nX + nWidth > rGeom.nWidth )
            nX = rGeom.nWidth - nWidth;
        if( nY < 0 )
            nY = 0;
        if( nY + nHeight > rGeom.nHeight )
            nY = rGeom.nHeight - nHeight;
        SetPosSizePixel( nX, nY, nWidth, nHeight, nPosSize );
        maOrgSize = Size( nWidth, nHeight );

        // 91625 - ignore Minimize
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
            {
                // #94144# allow Minimize again, should be masked out when read from configuration
                // 91625 - ignore Minimize
                if ( !(nValidMask&WINDOWSTATE_MASK_MINIMIZED) )
                    aState.mnState &= ~(WINDOWSTATE_STATE_MINIMIZED);
                rData.SetState( aState.mnState );
            }
        }
        else
            rData.SetMask( 0 );
#else
        ::rtl::OUString aStr( mpFrame->GetWindowState() );
        ByteString aByteStr( aStr.getStr(), aStr.getLength(), RTL_TEXTENCODING_ASCII_US );
        WindowStateData aData;
        ImplWindowStateFromStr( aData, aByteStr );
        rData = aData;
#endif
    }
    else
    {
        Point   aPos = GetPosPixel();
        Size    aSize = GetSizePixel();
        ULONG   nState = 0;

        if ( IsRollUp() )
        {
            aSize.Height() += maOrgSize.Height();
            nState |= WINDOWSTATE_STATE_ROLLUP;
        }

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
        Window*  pNewWindow=NULL;
        mpMenuBar = pMenuBar;

        if ( mpBorderWindow && (mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) )
        {
            if ( pOldMenuBar )
                pOldWindow = pOldMenuBar->ImplGetWindow();
            else
                pOldWindow = NULL;
            if ( pOldWindow )
            {
                ImplCallEventListeners( VCLEVENT_WINDOW_MENUBARREMOVED, (void*) pOldMenuBar );
                pOldWindow->SetAccessible( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >() );
            }
            if ( pMenuBar )
            {
                DBG_ASSERT( !pMenuBar->pWindow, "SystemWindow::SetMenuBar() - MenuBars can only set in one SystemWindow at time" );
                ((ImplBorderWindow*)mpBorderWindow)->SetMenuBarWindow( pNewWindow = MenuBar::ImplCreate( mpBorderWindow, pOldWindow, pMenuBar ) );
                ImplCallEventListeners( VCLEVENT_WINDOW_MENUBARADDED, (void*) pMenuBar );
            }
            else
                ((ImplBorderWindow*)mpBorderWindow)->SetMenuBarWindow( NULL );
            ImplToBottomChild();
            if ( pOldMenuBar )
                MenuBar::ImplDestroy( pOldMenuBar, pMenuBar == 0 );

        }
        else
        {
            if( pMenuBar )
                pNewWindow = pMenuBar->ImplGetWindow();
            if( pOldMenuBar )
                pOldWindow = pOldMenuBar->ImplGetWindow();
        }

        // update taskpane list to make menubar accessible
        if( mpTaskPaneList )
        {
            if( pOldWindow )
                mpTaskPaneList->RemoveWindow( pOldWindow );
            if( pNewWindow )
                mpTaskPaneList->AddWindow( pNewWindow );
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
