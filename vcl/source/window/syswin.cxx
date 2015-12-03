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

#include <sal/config.h>

#include <cstdlib>

#include <tools/debug.hxx>

#include <vcl/layout.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>
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

class SystemWindow::ImplData
{
public:
    ImplData();
    ~ImplData();

    TaskPaneList*   mpTaskPaneList;
    Size            maMaxOutSize;
    OUString        maRepresentedURL;
    Link<SystemWindow&,void> maCloseHdl;
};

SystemWindow::ImplData::ImplData()
{
    mpTaskPaneList = nullptr;
    maMaxOutSize = Size( SHRT_MAX, SHRT_MAX );
}

SystemWindow::ImplData::~ImplData()
{
    delete mpTaskPaneList;
}

void SystemWindow::Init()
{
    mpImplData          = new ImplData;
    mpWindowImpl->mbSysWin            = true;
    mpWindowImpl->mnActivateMode      = ActivateModeFlags::GrabFocus;

    mpMenuBar           = nullptr;
    mbPinned            = false;
    mbRollUp            = false;
    mbRollFunc          = false;
    mbDockBtn           = false;
    mbHideBtn           = false;
    mbSysChild          = false;
    mbIsCalculatingInitialLayoutSize = false;
    mbInitialLayoutDone = false;
    mnMenuBarMode       = MenuBarMode::Normal;
    mnIcon              = 0;
    mpDialogParent      = nullptr;

    //To-Do, reuse maResizeTimer
    maLayoutIdle.SetPriority(SchedulerPriority::RESIZE);
    maLayoutIdle.SetIdleHdl( LINK( this, SystemWindow, ImplHandleLayoutTimerHdl ) );
}

SystemWindow::SystemWindow(WindowType nType)
    : Window(nType)
    , mbIsDefferedInit(false)
{
    Init();
}

void SystemWindow::loadUI(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
    const css::uno::Reference<css::frame::XFrame> &rFrame)
{
    mbIsDefferedInit = true;
    mpDialogParent = pParent; //should be unset in doDeferredInit
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(), rUIXMLDescription, rID, rFrame);
}

SystemWindow::~SystemWindow()
{
    disposeOnce();
}

void SystemWindow::dispose()
{
    maLayoutIdle.Stop();
    delete mpImplData;
    mpImplData = nullptr;

    // Hack to make sure code called from base ~Window does not interpret this
    // as a SystemWindow (which it no longer is by then):
    mpWindowImpl->mbSysWin = false;
    disposeBuilder();
    mpDialogParent.clear();
    Window::dispose();
}

bool SystemWindow::Notify( NotifyEvent& rNEvt )
{
    // capture KeyEvents for menu handling
    if (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT ||
        rNEvt.GetType() == MouseNotifyEvent::COMMAND)
    {
        MenuBar* pMBar = mpMenuBar;
        if ( !pMBar && ( GetType() == WINDOW_FLOATINGWINDOW ) )
        {
            vcl::Window* pWin = ImplGetFrameWindow()->ImplGetWindow();
            if( pWin && pWin->IsSystemWindow() )
                pMBar = static_cast<SystemWindow*>(pWin)->GetMenuBar();
        }
        bool bDone(false);
        if (pMBar)
        {
            if (rNEvt.GetType() == MouseNotifyEvent::COMMAND)
                bDone = pMBar->ImplHandleCmdEvent(*rNEvt.GetCommandEvent());
            else
                bDone = pMBar->ImplHandleKeyEvent(*rNEvt.GetKeyEvent(), false);
        }
        if (bDone)
            return true;
    }

    return Window::Notify( rNEvt );
}

bool SystemWindow::PreNotify( NotifyEvent& rNEvt )
{
    // capture KeyEvents for taskpane cycling
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        if( rNEvt.GetKeyEvent()->GetKeyCode().GetCode() == KEY_F6 &&
            rNEvt.GetKeyEvent()->GetKeyCode().IsMod1() &&
           !rNEvt.GetKeyEvent()->GetKeyCode().IsShift() )
        {
            // Ctrl-F6 goes directly to the document
            GrabFocusToDocument();
            return true;
        }
        else
        {
            TaskPaneList *pTList = mpImplData->mpTaskPaneList;
            if( !pTList && ( GetType() == WINDOW_FLOATINGWINDOW ) )
            {
                vcl::Window* pWin = ImplGetFrameWindow()->ImplGetWindow();
                if( pWin && pWin->IsSystemWindow() )
                    pTList = static_cast<SystemWindow*>(pWin)->mpImplData->mpTaskPaneList;
            }
            if( !pTList )
            {
                // search topmost system window which is the one to handle dialog/toolbar cycling
                SystemWindow *pSysWin = this;
                vcl::Window *pWin = this;
                while( pWin )
                {
                    pWin = pWin->GetParent();
                    if( pWin && pWin->IsSystemWindow() )
                        pSysWin = static_cast<SystemWindow*>(pWin);
                }
                pTList = pSysWin->mpImplData->mpTaskPaneList;
            }
            if( pTList && pTList->HandleKeyEvent( *rNEvt.GetKeyEvent() ) )
                return true;
        }
    }
    return Window::PreNotify( rNEvt );
}

TaskPaneList* SystemWindow::GetTaskPaneList()
{
    if( !mpImplData )
        return nullptr;
    if( mpImplData->mpTaskPaneList )
        return mpImplData->mpTaskPaneList ;
    else
    {
        mpImplData->mpTaskPaneList = new TaskPaneList();
        MenuBar* pMBar = mpMenuBar;
        if ( !pMBar && ( GetType() == WINDOW_FLOATINGWINDOW ) )
        {
            vcl::Window* pWin = ImplGetFrameWindow()->ImplGetWindow();
            if ( pWin && pWin->IsSystemWindow() )
                pMBar = static_cast<SystemWindow*>(pWin)->GetMenuBar();
        }
        if( pMBar )
            mpImplData->mpTaskPaneList->AddWindow( pMBar->ImplGetWindow() );
        return mpImplData->mpTaskPaneList;
    }
}

bool SystemWindow::Close()
{
    ImplDelData aDelData;
    ImplAddDel( &aDelData );
    CallEventListeners( VCLEVENT_WINDOW_CLOSE );
    if ( aDelData.IsDead() )
        return false;
    ImplRemoveDel( &aDelData );

    if ( mpWindowImpl->mxWindowPeer.is() && IsCreatedWithToolkit() )
        return false;

    // Is Window not closeable, ignore close
    vcl::Window*     pBorderWin = ImplGetBorderWindow();
    WinBits     nStyle;
    if ( pBorderWin )
        nStyle = pBorderWin->GetStyle();
    else
        nStyle = GetStyle();
    if ( !(nStyle & WB_CLOSEABLE) )
        return false;

    Hide();

    return true;
}

void SystemWindow::TitleButtonClick( TitleButton )
{
}

void SystemWindow::Pin()
{
}

void SystemWindow::Roll()
{
}

void SystemWindow::Resizing( Size& )
{
}

void SystemWindow::SetRepresentedURL( const OUString& i_rURL )
{
    bool bChanged = (i_rURL != mpImplData->maRepresentedURL);
    mpImplData->maRepresentedURL = i_rURL;
    if ( !mbSysChild && bChanged )
    {
        const vcl::Window* pWindow = this;
        while ( pWindow->mpWindowImpl->mpBorderWindow )
            pWindow = pWindow->mpWindowImpl->mpBorderWindow;

        if ( pWindow->mpWindowImpl->mbFrame )
            pWindow->mpWindowImpl->mpFrame->SetRepresentedURL( i_rURL );
    }
}

void SystemWindow::SetIcon( sal_uInt16 nIcon )
{
    if ( mnIcon == nIcon )
        return;

    mnIcon = nIcon;

    if ( !mbSysChild )
    {
        const vcl::Window* pWindow = this;
        while ( pWindow->mpWindowImpl->mpBorderWindow )
            pWindow = pWindow->mpWindowImpl->mpBorderWindow;

        if ( pWindow->mpWindowImpl->mbFrame )
            pWindow->mpWindowImpl->mpFrame->SetIcon( nIcon );
    }
}

void SystemWindow::EnableSaveBackground(bool bSave)
{
    if( ImplGetSVData()->maWinData.mbNoSaveBackground )
        bSave = false;

    vcl::Window* pWindow = this;
    while (pWindow->mpWindowImpl->mpBorderWindow)
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;
    if (pWindow->mpWindowImpl->mbOverlapWin && !pWindow->mpWindowImpl->mbFrame)
    {
        pWindow->mpWindowImpl->mpOverlapData->mbSaveBack = bSave;
        if (!bSave)
            pWindow->ImplDeleteOverlapBackground();
    }
}

bool SystemWindow::IsSaveBackgroundEnabled() const
{
    const vcl::Window* pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;
    if ( pWindow->mpWindowImpl->mpOverlapData )
        return pWindow->mpWindowImpl->mpOverlapData->mbSaveBack;
    else
        return false;
}

void SystemWindow::ShowTitleButton( TitleButton nButton, bool bVisible )
{
    if ( nButton == TitleButton::Docking )
    {
        if ( mbDockBtn != bVisible )
        {
            mbDockBtn = bVisible;
            if ( mpWindowImpl->mpBorderWindow )
                static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetDockButton( bVisible );
        }
    }
    else if ( nButton == TitleButton::Hide )
    {
        if ( mbHideBtn != bVisible )
        {
            mbHideBtn = bVisible;
            if ( mpWindowImpl->mpBorderWindow )
                static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetHideButton( bVisible );
        }
    }
    else if ( nButton == TitleButton::Menu )
    {
        if ( mpWindowImpl->mpBorderWindow )
            static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetMenuButton( bVisible );
    }
    else
        return;
}

bool SystemWindow::IsTitleButtonVisible( TitleButton nButton ) const
{
    if ( nButton == TitleButton::Docking )
        return mbDockBtn;
    else /* if ( nButton == TitleButton::Hide ) */
        return mbHideBtn;
}

void SystemWindow::SetPin( bool bPin )
{
    if ( bPin != mbPinned )
    {
        mbPinned = bPin;
        if ( mpWindowImpl->mpBorderWindow )
            static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetPin( bPin );
    }
}

void SystemWindow::RollUp()
{
    if ( !mbRollUp )
    {
        maOrgSize = GetOutputSizePixel();
        mbRollFunc = true;
        Size aSize = maRollUpOutSize;
        if ( !aSize.Width() )
            aSize.Width() = GetOutputSizePixel().Width();
        mbRollUp = true;
        if ( mpWindowImpl->mpBorderWindow )
            static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetRollUp( true, aSize );
        else
            SetOutputSizePixel( aSize );
        mbRollFunc = false;
    }
}

void SystemWindow::RollDown()
{
    if ( mbRollUp )
    {
        mbRollUp = false;
        if ( mpWindowImpl->mpBorderWindow )
            static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetRollUp( false, maOrgSize );
        else
            SetOutputSizePixel( maOrgSize );
    }
}

void SystemWindow::SetMinOutputSizePixel( const Size& rSize )
{
    maMinOutSize = rSize;
    if ( mpWindowImpl->mpBorderWindow )
    {
        static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetMinOutputSize( rSize.Width(), rSize.Height() );
        if ( mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame )
            mpWindowImpl->mpBorderWindow->mpWindowImpl->mpFrame->SetMinClientSize( rSize.Width(), rSize.Height() );
    }
    else if ( mpWindowImpl->mbFrame )
        mpWindowImpl->mpFrame->SetMinClientSize( rSize.Width(), rSize.Height() );
}

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
        static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetMaxOutputSize( aSize.Width(), aSize.Height() );
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

static void ImplWindowStateFromStr(WindowStateData& rData,
    const OString& rStr)
{
    sal_uInt32 nValidMask = 0;
    sal_Int32 nIndex      = 0;
    OString aTokenStr;

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

static OString ImplWindowStateToStr(const WindowStateData& rData)
{
    const sal_uInt32 nValidMask = rData.GetMask();
    if ( !nValidMask )
        return OString();

    OStringBuffer rStrBuf;

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

void SystemWindow::ImplMoveToScreen( long& io_rX, long& io_rY, long i_nWidth, long i_nHeight, vcl::Window* i_pConfigureWin )
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
    vcl::Window* pParent = i_pConfigureWin->GetParent();
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
    const sal_uInt32 nValidMask = rData.GetMask();
    if ( !nValidMask )
        return;

    if ( mbSysChild )
        return;

    vcl::Window* pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;

    if ( pWindow->mpWindowImpl->mbFrame )
    {
        const sal_uInt32 nState     = rData.GetState();
        SalFrameState   aState;
        aState.mnMask               = rData.GetMask();
        aState.mnX                  = rData.GetX();
        aState.mnY                  = rData.GetY();
        aState.mnWidth              = rData.GetWidth();
        aState.mnHeight             = rData.GetHeight();

        if( rData.GetMask() & (WINDOWSTATE_MASK_WIDTH|WINDOWSTATE_MASK_HEIGHT) )
        {
            // #i43799# adjust window state sizes if a minimal output size was set
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
        aState.mnState  = nState & WINDOWSTATE_STATE_SYSTEMMASK;

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
                vcl::Window *pWin = pSVData->maWinData.mpFirstFrame;
                bool bWrapped = false;
                while( pWin )
                {
                    if( !pWin->ImplIsRealParentPath( this ) && ( pWin != this ) &&
                        pWin->ImplGetWindow()->IsTopWindow() && pWin->mpWindowImpl->mbReallyVisible )
                    {
                        SalFrameGeometry g = pWin->mpWindowImpl->mpFrame->GetGeometry();
                        if( std::abs(g.nX-aState.mnX) < 2 && std::abs(g.nY-aState.mnY) < 5 )
                        {
                            long displacement = g.nTopDecoration ? g.nTopDecoration : 20;
                            if( (unsigned long) (aState.mnX + displacement + aState.mnWidth + g.nRightDecoration) > (unsigned long) aDesktop.Right() ||
                                (unsigned long) (aState.mnY + displacement + aState.mnHeight + g.nBottomDecoration) > (unsigned long) aDesktop.Bottom() )
                            {
                                // displacing would leave screen
                                aState.mnX = g.nLeftDecoration ? g.nLeftDecoration : 10; // should result in (0,0)
                                aState.mnY = displacement;
                                if( bWrapped ||
                                    (unsigned long) (aState.mnX + displacement + aState.mnWidth + g.nRightDecoration) > (unsigned long) aDesktop.Right() ||
                                    (unsigned long) (aState.mnY + displacement + aState.mnHeight + g.nBottomDecoration) > (unsigned long) aDesktop.Bottom() )
                                    break;  // further displacement not possible -> break
                                // avoid endless testing
                                bWrapped = true;
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
        PosSizeFlags nPosSize = PosSizeFlags::NONE;
        if ( nValidMask & WINDOWSTATE_MASK_X )
            nPosSize |= PosSizeFlags::X;
        if ( nValidMask & WINDOWSTATE_MASK_Y )
            nPosSize |= PosSizeFlags::Y;
        if ( nValidMask & WINDOWSTATE_MASK_WIDTH )
            nPosSize |= PosSizeFlags::Width;
        if ( nValidMask & WINDOWSTATE_MASK_HEIGHT )
            nPosSize |= PosSizeFlags::Height;

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
            const sal_uInt32 nState = rData.GetState();
            if ( nState & WINDOWSTATE_STATE_ROLLUP )
                RollUp();
            else
                RollDown();
        }
    }
}

void SystemWindow::GetWindowStateData( WindowStateData& rData ) const
{
    sal_uInt32 nValidMask = rData.GetMask();
    if ( !nValidMask )
        return;

    if ( mbSysChild )
        return;

    const vcl::Window* pWindow = this;
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
            if ( aState.mnMask & WINDOWSTATE_MASK_MAXIMIZED_X )
            {
                rData.SetMaximizedX( aState.mnMaximizedX );
                nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_X;
            }
            if ( aState.mnMask & WINDOWSTATE_MASK_MAXIMIZED_Y )
            {
                rData.SetMaximizedY( aState.mnMaximizedY );
                nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_Y;
            }
            if ( aState.mnMask & WINDOWSTATE_MASK_MAXIMIZED_WIDTH )
            {
                rData.SetMaximizedWidth( aState.mnMaximizedWidth );
                nValidMask |= WINDOWSTATE_MASK_MAXIMIZED_WIDTH;
            }
            if ( aState.mnMask & WINDOWSTATE_MASK_MAXIMIZED_HEIGHT )
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
        sal_uInt32 nState = 0;

        if ( IsRollUp() )
        {
            aSize.Height() += maOrgSize.Height();
            nState = WINDOWSTATE_STATE_ROLLUP;
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

void SystemWindow::SetWindowState(const OString& rStr)
{
    if (rStr.isEmpty())
        return;

    WindowStateData aData;
    ImplWindowStateFromStr( aData, rStr );
    SetWindowStateData( aData );
}

OString SystemWindow::GetWindowState( sal_uInt32 nMask ) const
{
    WindowStateData aData;
    aData.SetMask( nMask );
    GetWindowStateData( aData );

    return ImplWindowStateToStr(aData);
}

void SystemWindow::SetMenuBar(MenuBar* pMenuBar)
{
    if ( mpMenuBar != pMenuBar )
    {
        MenuBar* pOldMenuBar = mpMenuBar;
        vcl::Window*  pOldWindow = nullptr;
        vcl::Window*  pNewWindow=nullptr;
        mpMenuBar = pMenuBar;

        if ( mpWindowImpl->mpBorderWindow && (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) )
        {
            if ( pOldMenuBar )
                pOldWindow = pOldMenuBar->ImplGetWindow();
            else
                pOldWindow = nullptr;
            if ( pOldWindow )
            {
                CallEventListeners( VCLEVENT_WINDOW_MENUBARREMOVED, static_cast<void*>(pOldMenuBar) );
                pOldWindow->SetAccessible( css::uno::Reference< css::accessibility::XAccessible >() );
            }
            if ( pMenuBar )
            {
                DBG_ASSERT( !pMenuBar->pWindow, "SystemWindow::SetMenuBar() - MenuBars can only set in one SystemWindow at time" );

                pNewWindow = MenuBar::ImplCreate(mpWindowImpl->mpBorderWindow, pOldWindow, pMenuBar);
                static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetMenuBarWindow(pNewWindow);

                CallEventListeners( VCLEVENT_WINDOW_MENUBARADDED, static_cast<void*>(pMenuBar) );
            }
            else
                static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetMenuBarWindow( nullptr );
            ImplToBottomChild();
            if ( pOldMenuBar )
            {
                bool bDelete = (pMenuBar == nullptr);
                if( bDelete && pOldWindow )
                {
                    if( mpImplData->mpTaskPaneList )
                        mpImplData->mpTaskPaneList->RemoveWindow( pOldWindow );
                }
                MenuBar::ImplDestroy( pOldMenuBar, bDelete );
                if( bDelete )
                    pOldWindow = nullptr;  // will be deleted in MenuBar::ImplDestroy,
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

void SystemWindow::SetMenuBarMode( MenuBarMode nMode )
{
    if ( mnMenuBarMode != nMode )
    {
        mnMenuBarMode = nMode;
        if ( mpWindowImpl->mpBorderWindow && (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) )
        {
            if ( nMode == MenuBarMode::Hide )
                static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetMenuBarMode( true );
            else
                static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetMenuBarMode( false );
        }
    }
}

bool SystemWindow::ImplIsInTaskPaneList( vcl::Window* pWin )
{
    if( mpImplData && mpImplData->mpTaskPaneList )
        return mpImplData->mpTaskPaneList->IsInList( pWin );
    return false;
}

unsigned int SystemWindow::GetScreenNumber() const
{
    return mpWindowImpl->mpFrame->maGeometry.nDisplayScreenNumber;
}

void SystemWindow::SetScreenNumber(unsigned int nDisplayScreen)
{
    mpWindowImpl->mpFrame->SetScreenNumber( nDisplayScreen );
}

void SystemWindow::SetApplicationID(const OUString &rApplicationID)
{
    mpWindowImpl->mpFrame->SetApplicationID( rApplicationID );
}

void SystemWindow::SetCloseHdl(const Link<SystemWindow&,void>& rLink)
{
    mpImplData->maCloseHdl = rLink;
}

const Link<SystemWindow&,void>& SystemWindow::GetCloseHdl() const
{
    return mpImplData->maCloseHdl;
}

void SystemWindow::queue_resize(StateChangedType /*eReason*/)
{
    if (!isLayoutEnabled())
        return;
    if (isCalculatingInitialLayoutSize())
        return;
    InvalidateSizeCache();
    if (hasPendingLayout())
        return;
    maLayoutIdle.Start();
}

void SystemWindow::Resize()
{
    queue_resize();
}

bool SystemWindow::isLayoutEnabled() const
{
    //pre dtor called, and single child is a container => we're layout enabled
    return mpImplData && ::isLayoutEnabled(this);
}

Size SystemWindow::GetOptimalSize() const
{
    if (!isLayoutEnabled())
        return Window::GetOptimalSize();

    Size aSize = VclContainer::getLayoutRequisition(*GetWindow(GetWindowType::FirstChild));

    sal_Int32 nBorderWidth = get_border_width();

    aSize.Height() += mpWindowImpl->mnLeftBorder + mpWindowImpl->mnRightBorder
        + 2*nBorderWidth;
    aSize.Width() += mpWindowImpl->mnTopBorder + mpWindowImpl->mnBottomBorder
        + 2*nBorderWidth;

    return Window::CalcWindowSize(aSize);
}

void SystemWindow::setPosSizeOnContainee(Size aSize, Window &rBox)
{
    sal_Int32 nBorderWidth = get_border_width();

    aSize.Width() -= 2 * nBorderWidth;
    aSize.Height() -= 2 * nBorderWidth;

    Point aPos(nBorderWidth, nBorderWidth);
    VclContainer::setLayoutAllocation(rBox, aPos, aSize);
}

IMPL_LINK_NOARG_TYPED( SystemWindow, ImplHandleLayoutTimerHdl, Idle*, void )
{
    if (!isLayoutEnabled())
    {
        SAL_WARN("vcl.layout", "SystemWindow has become non-layout because extra children have been added directly to it.");
        return;
    }

    Window *pBox = GetWindow(GetWindowType::FirstChild);
    assert(pBox);
    setPosSizeOnContainee(GetSizePixel(), *pBox);
}

void SystemWindow::SetText(const OUString& rStr)
{
    setDeferredProperties();
    Window::SetText(rStr);
}

OUString SystemWindow::GetText() const
{
    const_cast<SystemWindow*>(this)->setDeferredProperties();
    return Window::GetText();
}

void SystemWindow::settingOptimalLayoutSize(Window* /*pBox*/)
{
}

void SystemWindow::setOptimalLayoutSize()
{
    maLayoutIdle.Stop();

    //resize SystemWindow to fit requisition on initial show
    Window *pBox = GetWindow(GetWindowType::FirstChild);

    settingOptimalLayoutSize(pBox);

    Size aSize = get_preferred_size();

    Size aMax(bestmaxFrameSizeForScreenSize(GetDesktopRectPixel().GetSize()));

    aSize.Width() = std::min(aMax.Width(), aSize.Width());
    aSize.Height() = std::min(aMax.Height(), aSize.Height());

    SetMinOutputSizePixel(aSize);
    SetSizePixel(aSize);
    setPosSizeOnContainee(aSize, *pBox);
}

void SystemWindow::DoInitialLayout()
{
    if ( GetSettings().GetStyleSettings().GetAutoMnemonic() )
       ImplWindowAutoMnemonic( this );

    if (isLayoutEnabled())
    {
        mbIsCalculatingInitialLayoutSize = true;
        setDeferredProperties();
        setOptimalLayoutSize();
        mbIsCalculatingInitialLayoutSize = false;
        mbInitialLayoutDone = true;
    }
}

void SystemWindow::doDeferredInit(WinBits /*nBits*/)
{
    SAL_WARN("vcl.layout", "SystemWindow in layout without doDeferredInit impl");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
