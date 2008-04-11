/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: syswin.cxx,v $
 * $Revision: 1.53 $
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
#include <tools/debug.hxx>

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#include <vcl/salframe.hxx>
#include <vcl/svdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/brdwin.hxx>
#include <vcl/menu.hxx>
#include <vcl/window.h>
#include <vcl/brdwin.hxx>
#include <vcl/sound.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/syswin.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/unowrap.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

// =======================================================================
class SystemWindow::ImplData
{
public:
    ImplData();
    ~ImplData();

    TaskPaneList*   mpTaskPaneList;
    Size            maMaxOutSize;
};

SystemWindow::ImplData::ImplData()
{
    mpTaskPaneList = NULL;
    maMaxOutSize = Size( SHRT_MAX, SHRT_MAX );
}

SystemWindow::ImplData::~ImplData()
{
    if( mpTaskPaneList )
        delete mpTaskPaneList;
}

// =======================================================================

SystemWindow::SystemWindow( WindowType nType ) :
    Window( nType )
{
    mpImplData          = new ImplData;
    mpWindowImpl->mbSysWin            = TRUE;
    mpWindowImpl->mnActivateMode      = ACTIVATE_MODE_GRABFOCUS;

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

SystemWindow::~SystemWindow()
{
    delete mpImplData;
    mpImplData = NULL;
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
            TaskPaneList *pTList = mpImplData->mpTaskPaneList;
            if( !pTList && ( GetType() == WINDOW_FLOATINGWINDOW ) )
            {
                Window* pWin = ImplGetFrameWindow()->ImplGetWindow();
                if( pWin && pWin->IsSystemWindow() )
                    pTList = ((SystemWindow*)pWin)->mpImplData->mpTaskPaneList;
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
                pTList = pSysWin->mpImplData->mpTaskPaneList;
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
    if( mpImplData->mpTaskPaneList )
        return mpImplData->mpTaskPaneList ;
    else
    {
        mpImplData->mpTaskPaneList = new TaskPaneList();
        MenuBar* pMBar = mpMenuBar;
        if ( !pMBar && ( GetType() == WINDOW_FLOATINGWINDOW ) )
        {
            Window* pWin = ImplGetFrameWindow()->ImplGetWindow();
            if ( pWin && pWin->IsSystemWindow() )
                pMBar = ((SystemWindow*)pWin)->GetMenuBar();
        }
        if( pMBar )
            mpImplData->mpTaskPaneList->AddWindow( pMBar->ImplGetWindow() );
        return mpImplData->mpTaskPaneList;
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

    if ( mpWindowImpl->mxWindowPeer.is() && IsCreatedWithToolkit() )
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
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;
    if ( pWindow->mpWindowImpl->mbOverlapWin && !pWindow->mpWindowImpl->mbFrame )
    {
        BYTE nOldLevel = pWindow->mpWindowImpl->mpOverlapData->mnTopLevel;
        pWindow->mpWindowImpl->mpOverlapData->mnTopLevel = nLevel;
        // Wenn der neue Level groesser als der alte ist, schieben
        // wir das Fenster nach hinten
        if ( !IsReallyVisible() && (nLevel > nOldLevel) && pWindow->mpWindowImpl->mpNext )
        {
            // Fenster aus der Liste entfernen
            if ( pWindow->mpWindowImpl->mpPrev )
                pWindow->mpWindowImpl->mpPrev->mpWindowImpl->mpNext = pWindow->mpWindowImpl->mpNext;
            else
                pWindow->mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap = pWindow->mpWindowImpl->mpNext;
            pWindow->mpWindowImpl->mpNext->mpWindowImpl->mpPrev = pWindow->mpWindowImpl->mpPrev;
            pWindow->mpWindowImpl->mpNext = NULL;
            // und Fenster wieder in die Liste am Ende eintragen
            pWindow->mpWindowImpl->mpPrev = pWindow->mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap;
            pWindow->mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = pWindow;
            pWindow->mpWindowImpl->mpPrev->mpWindowImpl->mpNext = pWindow;
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
        while ( pWindow->mpWindowImpl->mpBorderWindow )
            pWindow = pWindow->mpWindowImpl->mpBorderWindow;

        if ( pWindow->mpWindowImpl->mbFrame )
            pWindow->mpWindowImpl->mpFrame->SetIcon( nIcon );
    }
}

// -----------------------------------------------------------------------

BYTE SystemWindow::GetZLevel() const
{
    const Window* pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;
    if ( pWindow->mpWindowImpl->mpOverlapData )
        return pWindow->mpWindowImpl->mpOverlapData->mnTopLevel;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void SystemWindow::EnableSaveBackground( BOOL bSave )
{
    Window* pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;
    if ( pWindow->mpWindowImpl->mbOverlapWin && !pWindow->mpWindowImpl->mbFrame )
    {
        pWindow->mpWindowImpl->mpOverlapData->mbSaveBack = bSave;
        if ( !bSave )
            pWindow->ImplDeleteOverlapBackground();
    }
}

// -----------------------------------------------------------------------

BOOL SystemWindow::IsSaveBackgroundEnabled() const
{
    const Window* pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;
    if ( pWindow->mpWindowImpl->mpOverlapData )
        return pWindow->mpWindowImpl->mpOverlapData->mbSaveBack;
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
            if ( mpWindowImpl->mpBorderWindow )
                ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetDockButton( bVisible );
        }
    }
    else if ( nButton == TITLE_BUTTON_HIDE )
    {
        if ( mbHideBtn != bVisible )
        {
            mbHideBtn = bVisible;
            if ( mpWindowImpl->mpBorderWindow )
                ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetHideButton( bVisible );
        }
    }
    else if ( nButton == TITLE_BUTTON_MENU )
    {
        if ( mpWindowImpl->mpBorderWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetMenuButton( bVisible );
    }
    else
        return;
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
        if ( mpWindowImpl->mpBorderWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetPin( bPin );
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
        if ( mpWindowImpl->mpBorderWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetRollUp( TRUE, aSize );
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
        if ( mpWindowImpl->mpBorderWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetRollUp( FALSE, maOrgSize );
        else
            SetOutputSizePixel( maOrgSize );
    }
}

// -----------------------------------------------------------------------

void SystemWindow::SetMinOutputSizePixel( const Size& rSize )
{
    maMinOutSize = rSize;
    if ( mpWindowImpl->mpBorderWindow )
    {
        ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetMinOutputSize( rSize.Width(), rSize.Height() );
        if ( mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame )
            mpWindowImpl->mpBorderWindow->mpWindowImpl->mpFrame->SetMinClientSize( rSize.Width(), rSize.Height() );
    }
    else if ( mpWindowImpl->mbFrame )
        mpWindowImpl->mpFrame->SetMinClientSize( rSize.Width(), rSize.Height() );
}

// -----------------------------------------------------------------------

void SystemWindow::SetMaxOutputSizePixel( const Size& rSize )
{
    Size aSize( rSize );
    if( aSize.Width() > SHRT_MAX || aSize.Width() <= 0 )
        aSize.Width() = SHRT_MAX;
    if( aSize.Height() > SHRT_MAX || aSize.Height() <= 0 )
        aSize.Height() = SHRT_MAX;

    mpImplData->maMaxOutSize = aSize;
    if ( mpWindowImpl->mpBorderWindow )
    {
        ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetMaxOutputSize( aSize.Width(), aSize.Height() );
        if ( mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame )
            mpWindowImpl->mpBorderWindow->mpWindowImpl->mpFrame->SetMaxClientSize( aSize.Width(), aSize.Height() );
    }
    else if ( mpWindowImpl->mbFrame )
        mpWindowImpl->mpFrame->SetMaxClientSize( aSize.Width(), aSize.Height() );
}

const Size& SystemWindow::GetMaxOutputSizePixel() const
{
    return mpImplData->maMaxOutSize;
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
        if( rData.GetX() > -16384 && rData.GetX() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_X;
        else
            rData.SetX( 0 );
    }
    else
        rData.SetX( 0 );
    aTokenStr = rStr.GetToken( 0, ',', nIndex );
    if ( aTokenStr.Len() )
    {
        rData.SetY( aTokenStr.ToInt32() );
        if( rData.GetY() > -16384 && rData.GetY() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_Y;
        else
            rData.SetY( 0 );
    }
    else
        rData.SetY( 0 );
    aTokenStr = rStr.GetToken( 0, ',', nIndex );
    if ( aTokenStr.Len() )
    {
        rData.SetWidth( aTokenStr.ToInt32() );
        if( rData.GetWidth() > 0 && rData.GetWidth() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_WIDTH;
        else
            rData.SetWidth( 0 );
    }
    else
        rData.SetWidth( 0 );
    aTokenStr = rStr.GetToken( 0, ';', nIndex );
    if ( aTokenStr.Len() )
    {
        rData.SetHeight( aTokenStr.ToInt32() );
        if( rData.GetHeight() > 0 && rData.GetHeight() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_HEIGHT;
        else
            rData.SetHeight( 0 );
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

    // read maximized pos/size
    aTokenStr = rStr.GetToken( 0, ',', nIndex );
    if ( aTokenStr.Len() )
    {
        rData.SetMaximizedX( aTokenStr.ToInt32() );
        if( rData.GetMaximizedX() > -16384 && rData.GetMaximizedX() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_X;
        else
            rData.SetMaximizedX( 0 );
    }
    else
        rData.SetMaximizedX( 0 );
    aTokenStr = rStr.GetToken( 0, ',', nIndex );
    if ( aTokenStr.Len() )
    {
        rData.SetMaximizedY( aTokenStr.ToInt32() );
        if( rData.GetMaximizedY() > -16384 && rData.GetMaximizedY() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_Y;
        else
            rData.SetMaximizedY( 0 );
    }
    else
        rData.SetMaximizedY( 0 );
    aTokenStr = rStr.GetToken( 0, ',', nIndex );
    if ( aTokenStr.Len() )
    {
        rData.SetMaximizedWidth( aTokenStr.ToInt32() );
        if( rData.GetMaximizedWidth() > 0 && rData.GetMaximizedWidth() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_WIDTH;
        else
            rData.SetMaximizedWidth( 0 );
    }
    else
        rData.SetMaximizedWidth( 0 );
    aTokenStr = rStr.GetToken( 0, ';', nIndex );
    if ( aTokenStr.Len() )
    {
        rData.SetMaximizedHeight( aTokenStr.ToInt32() );
        if( rData.GetMaximizedHeight() > 0 && rData.GetMaximizedHeight() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_HEIGHT;
        else
            rData.SetMaximizedHeight( 0 );
    }
    else
        rData.SetMaximizedHeight( 0 );

    // mark valid fields
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
    if ( nValidMask & WINDOWSTATE_MASK_MAXIMIZED_X )
        rStr.Append( ByteString::CreateFromInt32( rData.GetMaximizedX() ) );
    rStr.Append( ',' );
    if ( nValidMask & WINDOWSTATE_MASK_MAXIMIZED_Y )
        rStr.Append( ByteString::CreateFromInt32( rData.GetMaximizedY() ) );
    rStr.Append( ',' );
    if ( nValidMask & WINDOWSTATE_MASK_MAXIMIZED_WIDTH )
        rStr.Append( ByteString::CreateFromInt32( rData.GetMaximizedWidth() ) );
    rStr.Append( ',' );
    if ( nValidMask & WINDOWSTATE_MASK_MAXIMIZED_HEIGHT )
        rStr.Append( ByteString::CreateFromInt32( rData.GetMaximizedHeight() ) );
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
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;

    if ( pWindow->mpWindowImpl->mbFrame )
    {
        ULONG           nState      = rData.GetState();
        SalFrameState   aState;
        aState.mnMask               = rData.GetMask();
        aState.mnX                  = rData.GetX();
        aState.mnY                  = rData.GetY();
        aState.mnWidth              = rData.GetWidth();
        aState.mnHeight             = rData.GetHeight();

        if( rData.GetMask() & (WINDOWSTATE_MASK_WIDTH|WINDOWSTATE_MASK_HEIGHT) )
        {
            // #i43799# adjust window state sizes if a minimial output size was set
            // otherwise the frame and the client might get different sizes
            if( maMinOutSize.Width() > aState.mnWidth )
                aState.mnWidth = maMinOutSize.Width();
            if( maMinOutSize.Height() > aState.mnHeight )
                aState.mnHeight = maMinOutSize.Height();
        }

        aState.mnMaximizedX         = rData.GetMaximizedX();
        aState.mnMaximizedY         = rData.GetMaximizedY();
        aState.mnMaximizedWidth     = rData.GetMaximizedWidth();
        aState.mnMaximizedHeight    = rData.GetMaximizedHeight();
        // #94144# allow Minimize again, should be masked out when read from configuration
        // 91625 - ignore Minimize
        //nState &= ~(WINDOWSTATE_STATE_MINIMIZED);
        aState.mnState  = nState & SAL_FRAMESTATE_SYSTEMMASK;

        // #96568# avoid having multiple frames at the same screen location
        //  do the check only if not maximized
        if( !((rData.GetMask() & WINDOWSTATE_MASK_STATE) && (nState & WINDOWSTATE_STATE_MAXIMIZED)) )
            if( rData.GetMask() & (WINDOWSTATE_MASK_POS|WINDOWSTATE_MASK_WIDTH|WINDOWSTATE_MASK_HEIGHT) )
            {
                Rectangle aDesktop = GetDesktopRectPixel();
                ImplSVData *pSVData = ImplGetSVData();
                Window *pWin = pSVData->maWinData.mpFirstFrame;
                BOOL bWrapped = FALSE;
                while( pWin )
                {
                    if( !pWin->ImplIsRealParentPath( this ) &&
                        pWin->ImplGetWindow()->IsTopWindow() && pWin->mpWindowImpl->mbReallyVisible )
                    {
                        SalFrameGeometry g = pWin->mpWindowImpl->mpFrame->GetGeometry();
                        if( abs(g.nX-aState.mnX) < 2 && abs(g.nY-aState.mnY) < 5 )
                        {
                            long displacement = g.nTopDecoration ? g.nTopDecoration : 20;
                            if( (unsigned long) (aState.mnX + displacement + aState.mnWidth + g.nRightDecoration) > (unsigned long) aDesktop.nRight ||
                                (unsigned long) (aState.mnY + displacement + aState.mnHeight + g.nBottomDecoration) > (unsigned long) aDesktop.nBottom )
                            {
                                // displacing would leave screen
                                aState.mnX = g.nLeftDecoration ? g.nLeftDecoration : 10; // should result in (0,0)
                                aState.mnY = displacement;
                                if( bWrapped ||
                                    (unsigned long) (aState.mnX + displacement + aState.mnWidth + g.nRightDecoration) > (unsigned long) aDesktop.nRight ||
                                    (unsigned long) (aState.mnY + displacement + aState.mnHeight + g.nBottomDecoration) > (unsigned long) aDesktop.nBottom )
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
                    pWin = pWin->mpWindowImpl->mpFrameData->mpNextFrame;
                }
            }

        mpWindowImpl->mpFrame->SetWindowState( &aState );

        // do a synchronous resize for layout reasons
        //  but use rData only when the window is not to be maximized (#i38089#)
        //  otherwise we have no useful size information
        if( (rData.GetMask() & WINDOWSTATE_MASK_STATE) && (nState & WINDOWSTATE_STATE_MAXIMIZED) )
        {
            // query maximized size from frame
            SalFrameGeometry aGeometry = mpWindowImpl->mpFrame->GetGeometry();

            // but use it only if it is different from the restore size (rData)
            // as currently only on windows the exact size of a maximized window
            //  can be computed without actually showing the window
            if( aGeometry.nWidth != rData.GetWidth() || aGeometry.nHeight != rData.GetHeight() )
                ImplHandleResize( pWindow, aGeometry.nWidth, aGeometry.nHeight );
        }
        else
            if( rData.GetMask() & (WINDOWSTATE_MASK_WIDTH|WINDOWSTATE_MASK_HEIGHT) )
                ImplHandleResize( pWindow, aState.mnWidth, aState.mnHeight );   // #i43799# use aState and not rData, see above
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
        const SalFrameGeometry& rGeom = pWindow->mpWindowImpl->mpFrame->GetGeometry();
        if( nX < 0 )
            nX = 0;
        if( nX + nWidth > (long) rGeom.nWidth )
            nX = rGeom.nWidth - nWidth;
        if( nY < 0 )
            nY = 0;
        if( nY + nHeight > (long) rGeom.nHeight )
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
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;

    if ( pWindow->mpWindowImpl->mbFrame )
    {
        SalFrameState aState;
        aState.mnMask = 0xFFFFFFFF;
        if ( mpWindowImpl->mpFrame->GetWindowState( &aState ) )
        {
            if ( nValidMask & WINDOWSTATE_MASK_X )
                rData.SetX( aState.mnX );
            if ( nValidMask & WINDOWSTATE_MASK_Y )
                rData.SetY( aState.mnY );
            if ( nValidMask & WINDOWSTATE_MASK_WIDTH )
                rData.SetWidth( aState.mnWidth );
            if ( nValidMask & WINDOWSTATE_MASK_HEIGHT )
                rData.SetHeight( aState.mnHeight );
            if ( aState.mnMask & SAL_FRAMESTATE_MASK_MAXIMIZED_X )
            {
                rData.SetMaximizedX( aState.mnMaximizedX );
                nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_X;
            }
            if ( aState.mnMask & SAL_FRAMESTATE_MASK_MAXIMIZED_Y )
            {
                rData.SetMaximizedY( aState.mnMaximizedY );
                nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_Y;
            }
            if ( aState.mnMask & SAL_FRAMESTATE_MASK_MAXIMIZED_WIDTH )
            {
                rData.SetMaximizedWidth( aState.mnMaximizedWidth );
                nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_WIDTH;
            }
            if ( aState.mnMask & SAL_FRAMESTATE_MASK_MAXIMIZED_HEIGHT )
            {
                rData.SetMaximizedHeight( aState.mnMaximizedHeight );
                nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_HEIGHT;
            }
            if ( nValidMask & WINDOWSTATE_MASK_STATE )
            {
                // #94144# allow Minimize again, should be masked out when read from configuration
                // 91625 - ignore Minimize
                if ( !(nValidMask&WINDOWSTATE_MASK_MINIMIZED) )
                    aState.mnState &= ~(WINDOWSTATE_STATE_MINIMIZED);
                rData.SetState( aState.mnState );
            }
            rData.SetMask( nValidMask );
        }
        else
            rData.SetMask( 0 );
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
        Window*  pOldWindow = NULL;
        Window*  pNewWindow=NULL;
        mpMenuBar = pMenuBar;

        if ( mpWindowImpl->mpBorderWindow && (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) )
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
                ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetMenuBarWindow( pNewWindow = MenuBar::ImplCreate( mpWindowImpl->mpBorderWindow, pOldWindow, pMenuBar ) );
                ImplCallEventListeners( VCLEVENT_WINDOW_MENUBARADDED, (void*) pMenuBar );
            }
            else
                ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetMenuBarWindow( NULL );
            ImplToBottomChild();
            if ( pOldMenuBar )
            {
                BOOL bDelete = (pMenuBar == 0) ? TRUE : FALSE;
                if( bDelete && pOldWindow )
                {
                    if( mpImplData->mpTaskPaneList )
                        mpImplData->mpTaskPaneList->RemoveWindow( pOldWindow );
                }
                MenuBar::ImplDestroy( pOldMenuBar, bDelete );
                if( bDelete )
                    pOldWindow = NULL;  // will be deleted in MenuBar::ImplDestroy,
            }

        }
        else
        {
            if( pMenuBar )
                pNewWindow = pMenuBar->ImplGetWindow();
            if( pOldMenuBar )
                pOldWindow = pOldMenuBar->ImplGetWindow();
        }

        // update taskpane list to make menubar accessible
        if( mpImplData->mpTaskPaneList )
        {
            if( pOldWindow )
                mpImplData->mpTaskPaneList->RemoveWindow( pOldWindow );
            if( pNewWindow )
                mpImplData->mpTaskPaneList->AddWindow( pNewWindow );
        }
    }
}

// -----------------------------------------------------------------------

void SystemWindow::SetMenuBarMode( USHORT nMode )
{
    if ( mnMenuBarMode != nMode )
    {
        mnMenuBarMode = nMode;
        if ( mpWindowImpl->mpBorderWindow && (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) )
        {
            if ( nMode == MENUBAR_MODE_HIDE )
                ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetMenuBarMode( TRUE );
            else
                ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetMenuBarMode( FALSE );
        }
    }
}

// -----------------------------------------------------------------------

BOOL SystemWindow::ImplIsInTaskPaneList( Window* pWin )
{
    if( mpImplData && mpImplData->mpTaskPaneList )
        return mpImplData->mpTaskPaneList->IsInList( pWin );
    return FALSE;
}

// -----------------------------------------------------------------------

unsigned int SystemWindow::GetScreenNumber() const
{
    return mpWindowImpl->mpFrame->maGeometry.nScreenNumber;
}

// -----------------------------------------------------------------------

void SystemWindow::SetScreenNumber( unsigned int nScreen)
{
    mpWindowImpl->mpFrame->SetScreenNumber( nScreen );
}
