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

#include <tools/debug.hxx>

#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/syswin.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/unowrap.hxx>

#include <rtl/strbuf.hxx>

#include <salframe.hxx>
#include <svdata.hxx>
#include <brdwin.hxx>
#include <window.h>

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
    rtl::OUString   maRepresentedURL;
    Link maCloseHdl;
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
    mpWindowImpl->mbSysWin            = sal_True;
    mpWindowImpl->mnActivateMode      = ACTIVATE_MODE_GRABFOCUS;

    mpMenuBar           = NULL;
    mbPined             = sal_False;
    mbRollUp            = sal_False;
    mbRollFunc          = sal_False;
    mbDockBtn           = sal_False;
    mbHideBtn           = sal_False;
    mbSysChild          = sal_False;
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
        if ( pMBar && pMBar->ImplHandleKeyEvent( *rNEvt.GetKeyEvent(), sal_False ) )
            return sal_True;
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
            return sal_True;
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
                return sal_True;
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

sal_Bool SystemWindow::Close()
{
    ImplDelData aDelData;
    ImplAddDel( &aDelData );
    ImplCallEventListeners( VCLEVENT_WINDOW_CLOSE );
    if ( aDelData.IsDead() )
        return sal_False;
    ImplRemoveDel( &aDelData );

    if ( mpWindowImpl->mxWindowPeer.is() && IsCreatedWithToolkit() )
        return sal_False;

    // Is Window not closeable, ignore close
    Window*     pBorderWin = ImplGetBorderWindow();
    WinBits     nStyle;
    if ( pBorderWin )
        nStyle = pBorderWin->GetStyle();
    else
        nStyle = GetStyle();
    if ( !(nStyle & WB_CLOSEABLE) )
        return sal_False;

    Hide();

    return sal_True;
}

// -----------------------------------------------------------------------

void SystemWindow::TitleButtonClick( sal_uInt16 )
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

void SystemWindow::SetRepresentedURL( const rtl::OUString& i_rURL )
{
    bool bChanged = (i_rURL != mpImplData->maRepresentedURL);
    mpImplData->maRepresentedURL = i_rURL;
    if ( !mbSysChild && bChanged )
    {
        const Window* pWindow = this;
        while ( pWindow->mpWindowImpl->mpBorderWindow )
            pWindow = pWindow->mpWindowImpl->mpBorderWindow;

        if ( pWindow->mpWindowImpl->mbFrame )
            pWindow->mpWindowImpl->mpFrame->SetRepresentedURL( i_rURL );
    }
}

// -----------------------------------------------------------------------

void SystemWindow::SetIcon( sal_uInt16 nIcon )
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

void SystemWindow::EnableSaveBackground( sal_Bool bSave )
{
    if( ImplGetSVData()->maWinData.mbNoSaveBackground )
        bSave = false;

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

sal_Bool SystemWindow::IsSaveBackgroundEnabled() const
{
    const Window* pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;
    if ( pWindow->mpWindowImpl->mpOverlapData )
        return pWindow->mpWindowImpl->mpOverlapData->mbSaveBack;
    else
        return sal_False;
}

// -----------------------------------------------------------------------

void SystemWindow::ShowTitleButton( sal_uInt16 nButton, sal_Bool bVisible )
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

sal_Bool SystemWindow::IsTitleButtonVisible( sal_uInt16 nButton ) const
{
    if ( nButton == TITLE_BUTTON_DOCKING )
        return mbDockBtn;
    else /* if ( nButton == TITLE_BUTTON_HIDE ) */
        return mbHideBtn;
}

// -----------------------------------------------------------------------

void SystemWindow::SetPin( sal_Bool bPin )
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
        mbRollFunc = sal_True;
        Size aSize = maRollUpOutSize;
        if ( !aSize.Width() )
            aSize.Width() = GetOutputSizePixel().Width();
        mbRollUp = sal_True;
        if ( mpWindowImpl->mpBorderWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetRollUp( sal_True, aSize );
        else
            SetOutputSizePixel( aSize );
        mbRollFunc = sal_False;
    }
}

// -----------------------------------------------------------------------

void SystemWindow::RollDown()
{
    if ( mbRollUp )
    {
        mbRollUp = sal_False;
        if ( mpWindowImpl->mpBorderWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetRollUp( sal_False, maOrgSize );
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

static void ImplWindowStateFromStr(WindowStateData& rData,
    const rtl::OString& rStr)
{
    sal_uLong       nValidMask  = 0;
    sal_Int32 nIndex      = 0;
    rtl::OString aTokenStr;

    aTokenStr = rStr.getToken(0, ',', nIndex);
    if (!aTokenStr.isEmpty())
    {
        rData.SetX(aTokenStr.toInt32());
        if( rData.GetX() > -16384 && rData.GetX() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_X;
        else
            rData.SetX( 0 );
    }
    else
        rData.SetX( 0 );
    aTokenStr = rStr.getToken(0, ',', nIndex);
    if (!aTokenStr.isEmpty())
    {
        rData.SetY(aTokenStr.toInt32());
        if( rData.GetY() > -16384 && rData.GetY() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_Y;
        else
            rData.SetY( 0 );
    }
    else
        rData.SetY( 0 );
    aTokenStr = rStr.getToken(0, ',', nIndex);
    if (!aTokenStr.isEmpty())
    {
        rData.SetWidth(aTokenStr.toInt32());
        if( rData.GetWidth() > 0 && rData.GetWidth() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_WIDTH;
        else
            rData.SetWidth( 0 );
    }
    else
        rData.SetWidth( 0 );
    aTokenStr = rStr.getToken(0, ';', nIndex);
    if (!aTokenStr.isEmpty())
    {
        rData.SetHeight(aTokenStr.toInt32());
        if( rData.GetHeight() > 0 && rData.GetHeight() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_HEIGHT;
        else
            rData.SetHeight( 0 );
    }
    else
        rData.SetHeight( 0 );
    aTokenStr = rStr.getToken(0, ';', nIndex);
    if (!aTokenStr.isEmpty())
    {
        // #94144# allow Minimize again, should be masked out when read from configuration
        // 91625 - ignore Minimize
        sal_uInt32 nState = (sal_uInt32)aTokenStr.toInt32();
        //nState &= ~(WINDOWSTATE_STATE_MINIMIZED);
        rData.SetState( nState );
        nValidMask |= WINDOWSTATE_MASK_STATE;
    }
    else
        rData.SetState( 0 );

    // read maximized pos/size
    aTokenStr = rStr.getToken(0, ',', nIndex);
    if (!aTokenStr.isEmpty())
    {
        rData.SetMaximizedX(aTokenStr.toInt32());
        if( rData.GetMaximizedX() > -16384 && rData.GetMaximizedX() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_X;
        else
            rData.SetMaximizedX( 0 );
    }
    else
        rData.SetMaximizedX( 0 );
    aTokenStr = rStr.getToken(0, ',', nIndex);
    if (!aTokenStr.isEmpty())
    {
        rData.SetMaximizedY(aTokenStr.toInt32());
        if( rData.GetMaximizedY() > -16384 && rData.GetMaximizedY() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_Y;
        else
            rData.SetMaximizedY( 0 );
    }
    else
        rData.SetMaximizedY( 0 );
    aTokenStr = rStr.getToken(0, ',', nIndex);
    if (!aTokenStr.isEmpty())
    {
        rData.SetMaximizedWidth(aTokenStr.toInt32());
        if( rData.GetMaximizedWidth() > 0 && rData.GetMaximizedWidth() < 16384 )
            nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_WIDTH;
        else
            rData.SetMaximizedWidth( 0 );
    }
    else
        rData.SetMaximizedWidth( 0 );
    aTokenStr = rStr.getToken(0, ';', nIndex);
    if (!aTokenStr.isEmpty())
    {
        rData.SetMaximizedHeight(aTokenStr.toInt32());
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

static rtl::OString ImplWindowStateToStr(const WindowStateData& rData)
{
    sal_uLong nValidMask = rData.GetMask();
    if ( !nValidMask )
        return rtl::OString();

    rtl::OStringBuffer rStrBuf;

    if ( nValidMask & WINDOWSTATE_MASK_X )
        rStrBuf.append(static_cast<sal_Int32>(rData.GetX()));
    rStrBuf.append(',');
    if ( nValidMask & WINDOWSTATE_MASK_Y )
        rStrBuf.append(static_cast<sal_Int32>(rData.GetY()));
    rStrBuf.append(',');
    if ( nValidMask & WINDOWSTATE_MASK_WIDTH )
        rStrBuf.append(static_cast<sal_Int32>(rData.GetWidth()));
    rStrBuf.append(',');
    if ( nValidMask & WINDOWSTATE_MASK_HEIGHT )
        rStrBuf.append(static_cast<sal_Int32>(rData.GetHeight()));
    rStrBuf.append( ';' );
    if ( nValidMask & WINDOWSTATE_MASK_STATE )
    {
        // #94144# allow Minimize again, should be masked out when read from configuration
        // 91625 - ignore Minimize
        sal_uInt32 nState = rData.GetState();
        rStrBuf.append(static_cast<sal_Int32>(nState));
    }
    rStrBuf.append(';');
    if ( nValidMask & WINDOWSTATE_MASK_MAXIMIZED_X )
        rStrBuf.append(static_cast<sal_Int32>(rData.GetMaximizedX()));
    rStrBuf.append(',');
    if ( nValidMask & WINDOWSTATE_MASK_MAXIMIZED_Y )
        rStrBuf.append(static_cast<sal_Int32>(rData.GetMaximizedY()));
    rStrBuf.append( ',' );
    if ( nValidMask & WINDOWSTATE_MASK_MAXIMIZED_WIDTH )
        rStrBuf.append(static_cast<sal_Int32>(rData.GetMaximizedWidth()));
    rStrBuf.append(',');
    if ( nValidMask & WINDOWSTATE_MASK_MAXIMIZED_HEIGHT )
        rStrBuf.append(static_cast<sal_Int32>(rData.GetMaximizedHeight()));
    rStrBuf.append(';');

    return rStrBuf.makeStringAndClear();
}

// -----------------------------------------------------------------------

void SystemWindow::ImplMoveToScreen( long& io_rX, long& io_rY, long i_nWidth, long i_nHeight, Window* i_pConfigureWin )
{
    Rectangle aScreenRect;
    if( !Application::IsUnifiedDisplay() )
        aScreenRect = Application::GetScreenPosSizePixel( GetScreenNumber() );
    else
    {
        aScreenRect = Application::GetScreenPosSizePixel( 0 );
        for( unsigned int i = 1; i < Application::GetScreenCount(); i++ )
            aScreenRect.Union( Application::GetScreenPosSizePixel( i ) );
    }
    // unfortunately most of the time width and height are not really known
    if( i_nWidth < 1 )
        i_nWidth = 50;
    if( i_nHeight < 1 )
        i_nHeight = 50;

    // check left border
    bool bMove = false;
    if( io_rX + i_nWidth < aScreenRect.Left() )
    {
        bMove = true;
        io_rX = aScreenRect.Left();
    }
    // check right border
    if( io_rX > aScreenRect.Right() - i_nWidth )
    {
        bMove = true;
        io_rX = aScreenRect.Right() - i_nWidth;
    }
    // check top border
    if( io_rY + i_nHeight < aScreenRect.Top() )
    {
        bMove = true;
        io_rY = aScreenRect.Top();
    }
    // check bottom border
    if( io_rY > aScreenRect.Bottom() - i_nHeight )
    {
        bMove = true;
        io_rY = aScreenRect.Bottom() - i_nHeight;
    }
    Window* pParent = i_pConfigureWin->GetParent();
    if( bMove && pParent )
    {
        // calculate absolute screen pos here, since that is what is contained in WindowState
        Point aParentAbsPos( pParent->OutputToAbsoluteScreenPixel( Point(0,0) ) );
        Size aParentSizePixel( pParent->GetOutputSizePixel() );
        Point aPos( (aParentSizePixel.Width() - i_nWidth) / 2,
                    (aParentSizePixel.Height() - i_nHeight) / 2 );
        io_rX = aParentAbsPos.X() + aPos.X();
        io_rY = aParentAbsPos.Y() + aPos.Y();
    }
}

void SystemWindow::SetWindowStateData( const WindowStateData& rData )
{
    sal_uLong nValidMask = rData.GetMask();
    if ( !nValidMask )
        return;

    if ( mbSysChild )
        return;

    Window* pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;

    if ( pWindow->mpWindowImpl->mbFrame )
    {
        sal_uLong           nState      = rData.GetState();
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

        // normalize window positions onto screen
        ImplMoveToScreen( aState.mnX, aState.mnY, aState.mnWidth, aState.mnHeight, pWindow );
        ImplMoveToScreen( aState.mnMaximizedX, aState.mnMaximizedY, aState.mnMaximizedWidth, aState.mnMaximizedHeight, pWindow );

        // #96568# avoid having multiple frames at the same screen location
        //  do the check only if not maximized
        if( !((rData.GetMask() & WINDOWSTATE_MASK_STATE) && (nState & WINDOWSTATE_STATE_MAXIMIZED)) )
            if( rData.GetMask() & (WINDOWSTATE_MASK_POS|WINDOWSTATE_MASK_WIDTH|WINDOWSTATE_MASK_HEIGHT) )
            {
                Rectangle aDesktop = GetDesktopRectPixel();
                ImplSVData *pSVData = ImplGetSVData();
                Window *pWin = pSVData->maWinData.mpFirstFrame;
                sal_Bool bWrapped = sal_False;
                while( pWin )
                {
                    if( !pWin->ImplIsRealParentPath( this ) && ( pWin != this ) &&
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
                                bWrapped = sal_True;
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
        sal_uInt16 nPosSize = 0;
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
        setPosSizePixel( nX, nY, nWidth, nHeight, nPosSize );
        maOrgSize = Size( nWidth, nHeight );

        // 91625 - ignore Minimize
        if ( nValidMask & WINDOWSTATE_MASK_STATE )
        {
            sal_uLong nState = rData.GetState();
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
    sal_uLong nValidMask = rData.GetMask();
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
        sal_uLong   nState = 0;

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

void SystemWindow::SetWindowState(const rtl::OString& rStr)
{
    if (rStr.isEmpty())
        return;

    WindowStateData aData;
    ImplWindowStateFromStr( aData, rStr );
    SetWindowStateData( aData );
}

// -----------------------------------------------------------------------

rtl::OString SystemWindow::GetWindowState( sal_uLong nMask ) const
{
    WindowStateData aData;
    aData.SetMask( nMask );
    GetWindowStateData( aData );

    return ImplWindowStateToStr(aData);
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
                sal_Bool bDelete = (pMenuBar == 0) ? sal_True : sal_False;
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

void SystemWindow::SetMenuBarMode( sal_uInt16 nMode )
{
    if ( mnMenuBarMode != nMode )
    {
        mnMenuBarMode = nMode;
        if ( mpWindowImpl->mpBorderWindow && (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) )
        {
            if ( nMode == MENUBAR_MODE_HIDE )
                ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetMenuBarMode( sal_True );
            else
                ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetMenuBarMode( sal_False );
        }
    }
}

// -----------------------------------------------------------------------

sal_Bool SystemWindow::ImplIsInTaskPaneList( Window* pWin )
{
    if( mpImplData && mpImplData->mpTaskPaneList )
        return mpImplData->mpTaskPaneList->IsInList( pWin );
    return sal_False;
}

unsigned int SystemWindow::GetScreenNumber() const
{
    return mpWindowImpl->mpFrame->maGeometry.nDisplayScreenNumber;
}

void SystemWindow::SetScreenNumber(unsigned int nDisplayScreen)
{
    mpWindowImpl->mpFrame->SetScreenNumber( nDisplayScreen );
}

void SystemWindow::SetApplicationID(const rtl::OUString &rApplicationID)
{
    mpWindowImpl->mpFrame->SetApplicationID( rApplicationID );
}

void SystemWindow::SetCloseHdl(const Link& rLink)
{
    mpImplData->maCloseHdl = rLink;
}

const Link& SystemWindow::GetCloseHdl() const
{
    return mpImplData->maCloseHdl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
