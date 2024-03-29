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

#include <memory>

#include <o3tl/safeint.hxx>
#include <sal/config.h>
#include <sal/log.hxx>

#include <vcl/layout.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>
#include <vcl/event.hxx>
#include <vcl/syswin.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/virdev.hxx>

#include <rtl/ustrbuf.hxx>
#include <o3tl/string_view.hxx>

#include <accel.hxx>
#include <salframe.hxx>
#include <svdata.hxx>
#include <brdwin.hxx>
#include <window.h>

using namespace ::com::sun::star::uno;

class SystemWindow::ImplData
{
public:
    ImplData();

    std::unique_ptr<TaskPaneList>
                    mpTaskPaneList;
    Size            maMaxOutSize;
    OUString        maRepresentedURL;
    Link<SystemWindow&,void> maCloseHdl;
};

SystemWindow::ImplData::ImplData()
{
    mpTaskPaneList = nullptr;
    maMaxOutSize = Size( SHRT_MAX, SHRT_MAX );
}

SystemWindow::SystemWindow(WindowType nType, const char* pIdleDebugName)
    : Window(nType)
    , mbDockBtn(false)
    , mbHideBtn(false)
    , mbSysChild(false)
    , mbIsCalculatingInitialLayoutSize(false)
    , mbInitialLayoutSizeCalculated(false)
    , mbPaintComplete(false)
    , mnMenuBarMode(MenuBarMode::Normal)
    , mnIcon(0)
    , mpImplData(new ImplData)
    , maLayoutIdle( pIdleDebugName )
    , mbIsDeferredInit(false)
{
    mpWindowImpl->mbSysWin            = true;
    mpWindowImpl->mnActivateMode      = ActivateModeFlags::GrabFocus;

    //To-Do, reuse maResizeTimer
    maLayoutIdle.SetPriority(TaskPriority::RESIZE);
    maLayoutIdle.SetInvokeHandler( LINK( this, SystemWindow, ImplHandleLayoutTimerHdl ) );
}

void SystemWindow::loadUI(vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription,
    const css::uno::Reference<css::frame::XFrame> &rFrame)
{
    mbIsDeferredInit = true;
    mpDialogParent = pParent; //should be unset in doDeferredInit
    m_pUIBuilder.reset( new VclBuilder(this, AllSettings::GetUIRootDir(), rUIXMLDescription, rID, rFrame) );
}

SystemWindow::~SystemWindow()
{
    disposeOnce();
}

void SystemWindow::dispose()
{
    maLayoutIdle.Stop();
    mpImplData.reset();

    // Hack to make sure code called from base ~Window does not interpret this
    // as a SystemWindow (which it no longer is by then):
    mpWindowImpl->mbSysWin = false;
    disposeBuilder();
    mpDialogParent.clear();
    mpMenuBar.clear();
    Window::dispose();
}

static void ImplHandleControlAccelerator( const vcl::Window* pWindow, bool bShow )
{
    Control *pControl = dynamic_cast<Control*>(pWindow->ImplGetWindow());
    if (pControl && pControl->GetText().indexOf('~') != -1)
    {
        pControl->SetShowAccelerator( bShow );
        pControl->Invalidate(InvalidateFlags::Update);
    }
}

namespace
{
    void processChildren(const vcl::Window *pParent, bool bShowAccel)
    {
        // go through its children
        vcl::Window* pChild = firstLogicalChildOfParent(pParent);
        while (pChild)
        {
            if (pChild->GetType() == WindowType::TABCONTROL)
            {
                // find currently shown tab page
                TabControl* pTabControl = static_cast<TabControl*>(pChild);
                TabPage* pTabPage = pTabControl->GetTabPage( pTabControl->GetCurPageId() );
                processChildren(pTabPage, bShowAccel);
            }
            else if (pChild->GetType() == WindowType::TABPAGE)
            {
                // bare tabpage without tabcontrol parent (options dialog)
                processChildren(pChild, bShowAccel);
            }
            else if ((pChild->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL)
            {
                // special controls that manage their children outside of widget layout
                processChildren(pChild, bShowAccel);
            }
            else
            {
                ImplHandleControlAccelerator(pChild, bShowAccel);
            }
            pChild = nextLogicalChildOfParent(pParent, pChild);
        }
    }
}

namespace
{
    bool ToggleMnemonicsOnHierarchy(const CommandEvent& rCEvent, const vcl::Window *pWindow)
    {
        if (rCEvent.GetCommand() == CommandEventId::ModKeyChange && ImplGetSVData()->maNWFData.mbAutoAccel)
        {
            const CommandModKeyData *pCData = rCEvent.GetModKeyData();
            const bool bShowAccel = pCData && pCData->IsMod2() && pCData->IsDown();
            processChildren(pWindow, bShowAccel);
            return true;
        }
        return false;
    }
}

bool SystemWindow::EventNotify( NotifyEvent& rNEvt )
{
    if (rNEvt.GetType() == NotifyEventType::COMMAND)
        ToggleMnemonicsOnHierarchy(*rNEvt.GetCommandEvent(), this);

    // capture KeyEvents for menu handling
    if (rNEvt.GetType() == NotifyEventType::KEYINPUT ||
        rNEvt.GetType() == NotifyEventType::COMMAND)
    {
        MenuBar* pMBar = mpMenuBar;
        if ( !pMBar && ( GetType() == WindowType::FLOATINGWINDOW ) )
        {
            vcl::Window* pWin = ImplGetFrameWindow()->ImplGetWindow();
            if( pWin && pWin->IsSystemWindow() )
                pMBar = static_cast<SystemWindow*>(pWin)->GetMenuBar();
        }
        bool bDone(false);
        if (pMBar)
        {
            if (rNEvt.GetType() == NotifyEventType::COMMAND)
                bDone = pMBar->ImplHandleCmdEvent(*rNEvt.GetCommandEvent());
            else
                bDone = pMBar->ImplHandleKeyEvent(*rNEvt.GetKeyEvent());
        }
        if (bDone)
            return true;
    }

    return Window::EventNotify( rNEvt );
}

bool SystemWindow::PreNotify( NotifyEvent& rNEvt )
{
    // capture KeyEvents for taskpane cycling
    if ( rNEvt.GetType() == NotifyEventType::KEYINPUT )
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
            TaskPaneList *pTList = mpImplData->mpTaskPaneList.get();
            if( !pTList && ( GetType() == WindowType::FLOATINGWINDOW ) )
            {
                vcl::Window* pWin = ImplGetFrameWindow()->ImplGetWindow();
                if( pWin && pWin->IsSystemWindow() )
                    pTList = static_cast<SystemWindow*>(pWin)->mpImplData->mpTaskPaneList.get();
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
                pTList = pSysWin->mpImplData->mpTaskPaneList.get();
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
        return mpImplData->mpTaskPaneList.get();
    else
    {
        mpImplData->mpTaskPaneList.reset( new TaskPaneList );
        MenuBar* pMBar = mpMenuBar;
        if ( !pMBar && ( GetType() == WindowType::FLOATINGWINDOW ) )
        {
            vcl::Window* pWin = ImplGetFrameWindow()->ImplGetWindow();
            if ( pWin && pWin->IsSystemWindow() )
                pMBar = static_cast<SystemWindow*>(pWin)->GetMenuBar();
        }
        if( pMBar )
            mpImplData->mpTaskPaneList->AddWindow( pMBar->ImplGetWindow() );
        return mpImplData->mpTaskPaneList.get();
    }
}

bool SystemWindow::Close()
{
    VclPtr<vcl::Window> xWindow = this;
    CallEventListeners( VclEventId::WindowClose );
    if ( xWindow->isDisposed() )
        return false;

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
        aSize.setWidth( SHRT_MAX );
    if( aSize.Height() > SHRT_MAX || aSize.Height() <= 0 )
        aSize.setHeight( SHRT_MAX );

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

vcl::WindowData::WindowData(std::u16string_view rStr)
{
    vcl::WindowData& rData = *this;
    vcl::WindowDataMask nValidMask = vcl::WindowDataMask::NONE;
    sal_Int32 nIndex      = 0;

    std::u16string_view aTokenStr = o3tl::getToken(rStr, 0, ',', nIndex);
    if (!aTokenStr.empty())
    {
        rData.setX(o3tl::toInt32(aTokenStr));
        if (rData.x() > -16384 && rData.x() < 16384)
            nValidMask |= vcl::WindowDataMask::X;
        else
            rData.setX(0);
    }
    else
        rData.setX(0);
    aTokenStr = o3tl::getToken(rStr, 0, ',', nIndex);
    if (!aTokenStr.empty())
    {
        rData.setY(o3tl::toInt32(aTokenStr));
        if (rData.y() > -16384 && rData.y() < 16384)
            nValidMask |= vcl::WindowDataMask::Y;
        else
            rData.setY(0);
    }
    else
        rData.setY(0);
    aTokenStr = o3tl::getToken(rStr, 0, ',', nIndex);
    if (!aTokenStr.empty())
    {
        sal_Int32 nWidth = o3tl::toInt32(aTokenStr);
        if (nWidth >= 0)
        {
            rData.setWidth(nWidth);
        }
        if (rData.width() > 0 && rData.width() < 16384)
            nValidMask |= vcl::WindowDataMask::Width;
        else
            rData.setWidth(0);
    }
    else
        rData.setWidth(0);
    aTokenStr = o3tl::getToken(rStr, 0, ';', nIndex);
    if (!aTokenStr.empty())
    {
        sal_Int32 nHeight = o3tl::toInt32(aTokenStr);
        if (nHeight >= 0)
        {
            rData.setHeight(nHeight);
        }
        if (rData.height() > 0 && rData.height() < 16384)
            nValidMask |= vcl::WindowDataMask::Height;
        else
            rData.setHeight(0);
    }
    else
        rData.setHeight(0);
    aTokenStr = o3tl::getToken(rStr, 0, ';', nIndex);
    if (!aTokenStr.empty())
    {
        // #94144# allow Minimize again, should be masked out when read from configuration
        // 91625 - ignore Minimize
        vcl::WindowState nState = static_cast<vcl::WindowState>(o3tl::toInt32(aTokenStr));
        //nState &= ~vcl::WindowState::Minimized;
        rData.setState(nState);
        nValidMask |= vcl::WindowDataMask::State;
    }
    else
        rData.setState(vcl::WindowState::NONE);

    // read maximized pos/size
    aTokenStr = o3tl::getToken(rStr, 0, ',', nIndex);
    if (!aTokenStr.empty())
    {
        rData.SetMaximizedX(o3tl::toInt32(aTokenStr));
        if (rData.GetMaximizedX() > -16384 && rData.GetMaximizedX() < 16384)
            nValidMask |= vcl::WindowDataMask::MaximizedX;
        else
            rData.SetMaximizedX(0);
    }
    else
        rData.SetMaximizedX(0);
    aTokenStr = o3tl::getToken(rStr, 0, ',', nIndex);
    if (!aTokenStr.empty())
    {
        rData.SetMaximizedY(o3tl::toInt32(aTokenStr));
        if (rData.GetMaximizedY() > -16384 && rData.GetMaximizedY() < 16384)
            nValidMask |= vcl::WindowDataMask::MaximizedY;
        else
            rData.SetMaximizedY(0);
    }
    else
        rData.SetMaximizedY(0);
    aTokenStr = o3tl::getToken(rStr, 0, ',', nIndex);
    if (!aTokenStr.empty())
    {
        rData.SetMaximizedWidth(o3tl::toInt32(aTokenStr));
        if (rData.GetMaximizedWidth() > 0 && rData.GetMaximizedWidth() < 16384)
            nValidMask |= vcl::WindowDataMask::MaximizedWidth;
        else
            rData.SetMaximizedWidth(0);
    }
    else
        rData.SetMaximizedWidth(0);
    aTokenStr = o3tl::getToken(rStr, 0, ';', nIndex);
    if (!aTokenStr.empty())
    {
        rData.SetMaximizedHeight(o3tl::toInt32(aTokenStr));
        if (rData.GetMaximizedHeight() > 0 && rData.GetMaximizedHeight() < 16384)
            nValidMask |= vcl::WindowDataMask::MaximizedHeight;
        else
            rData.SetMaximizedHeight(0);
    }
    else
        rData.SetMaximizedHeight(0);

    // mark valid fields
    rData.setMask(nValidMask);
}

OUString vcl::WindowData::toStr() const
{
    const vcl::WindowDataMask nValidMask = mask();
    if ( nValidMask == vcl::WindowDataMask::NONE )
        return {};

    OUStringBuffer rStrBuf(64);

    tools::Rectangle aRect = posSize();

    if (nValidMask & vcl::WindowDataMask::X)
        rStrBuf.append(static_cast<sal_Int32>(aRect.Left()));
    rStrBuf.append(',');
    if (nValidMask & vcl::WindowDataMask::Y)
        rStrBuf.append(static_cast<sal_Int32>(aRect.Top()));
    rStrBuf.append(',');
    if (nValidMask & vcl::WindowDataMask::Width)
        rStrBuf.append(static_cast<sal_Int32>(aRect.GetWidth()));
    rStrBuf.append(',');
    if (nValidMask & vcl::WindowDataMask::Height)
        rStrBuf.append(static_cast<sal_Int32>(aRect.GetHeight()));
    rStrBuf.append( ';' );
    if (nValidMask & vcl::WindowDataMask::State)
    {
        // #94144# allow Minimize again, should be masked out when read from configuration
        // 91625 - ignore Minimize
        rStrBuf.append(static_cast<sal_Int32>(state()));
    }
    rStrBuf.append(';');
    if (nValidMask & vcl::WindowDataMask::MaximizedX)
        rStrBuf.append(static_cast<sal_Int32>(GetMaximizedX()));
    rStrBuf.append(',');
    if (nValidMask & vcl::WindowDataMask::MaximizedY)
        rStrBuf.append(static_cast<sal_Int32>(GetMaximizedY()));
    rStrBuf.append( ',' );
    if (nValidMask & vcl::WindowDataMask::MaximizedWidth)
        rStrBuf.append(static_cast<sal_Int32>(GetMaximizedWidth()));
    rStrBuf.append(',');
    if (nValidMask & vcl::WindowDataMask::MaximizedHeight)
        rStrBuf.append(static_cast<sal_Int32>(GetMaximizedHeight()));
    rStrBuf.append(';');

    return rStrBuf.makeStringAndClear();
}

void SystemWindow::ImplMoveToScreen( tools::Long& io_rX, tools::Long& io_rY, tools::Long i_nWidth, tools::Long i_nHeight, vcl::Window const * i_pConfigureWin )
{
    AbsoluteScreenPixelRectangle aScreenRect = Application::GetScreenPosSizePixel( 0 );
    for( unsigned int i = 1; i < Application::GetScreenCount(); i++ )
        aScreenRect.Union( Application::GetScreenPosSizePixel( i ) );
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
        // calculate absolute screen pos here, since that is what is contained in WindowData
        Point aParentAbsPos( pParent->OutputToAbsoluteScreenPixel( Point(0,0) ) );
        Size aParentSizePixel( pParent->GetOutputSizePixel() );
        Point aPos( (aParentSizePixel.Width() - i_nWidth) / 2,
                    (aParentSizePixel.Height() - i_nHeight) / 2 );
        io_rX = aParentAbsPos.X() + aPos.X();
        io_rY = aParentAbsPos.Y() + aPos.Y();
    }
}

void SystemWindow::SetWindowState(const vcl::WindowData& rData)
{
    const vcl::WindowDataMask nValidMask = rData.mask();
    if ( nValidMask == vcl::WindowDataMask::NONE )
        return;

    if ( mbSysChild )
        return;

    vcl::Window* pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;

    if ( pWindow->mpWindowImpl->mbFrame )
    {
        const vcl::WindowState nState = rData.state();
        vcl::WindowData aState = rData;

        if (rData.mask() & vcl::WindowDataMask::Size)
        {
            // #i43799# adjust window state sizes if a minimal output size was set
            // otherwise the frame and the client might get different sizes
            if (maMinOutSize.Width() > static_cast<tools::Long>(aState.width()))
                aState.setWidth(maMinOutSize.Width());
            if (maMinOutSize.Height() > static_cast<tools::Long>(aState.width()))
                aState.setHeight(maMinOutSize.Height());
        }

        // #94144# allow Minimize again, should be masked out when read from configuration
        // 91625 - ignore Minimize
        //nState &= ~(WindowState::Minimized);
        aState.rState() &= vcl::WindowState::SystemMask;

        // normalize window positions onto screen
        tools::Long nX = aState.x(), nY = aState.y();
        ImplMoveToScreen(nX, nY, aState.width(), aState.height(), pWindow);
        aState.setPos({ nX, nY });
        nX = aState.GetMaximizedX();
        nY = aState.GetMaximizedY();
        ImplMoveToScreen(nX, nY, aState.GetMaximizedWidth(), aState.GetMaximizedHeight(), pWindow);
        aState.SetMaximizedX(nX);
        aState.SetMaximizedY(nY);

        // #96568# avoid having multiple frames at the same screen location
        //  do the check only if not maximized
        if( !((rData.mask() & vcl::WindowDataMask::State) && (nState & vcl::WindowState::Maximized)) )
            if (rData.mask() & vcl::WindowDataMask::PosSize)
            {
                AbsoluteScreenPixelRectangle aDesktop = GetDesktopRectPixel();
                ImplSVData *pSVData = ImplGetSVData();
                vcl::Window *pWin = pSVData->maFrameData.mpFirstFrame;
                bool bWrapped = false;
                while( pWin )
                {
                    if( !pWin->ImplIsRealParentPath( this ) && ( pWin != this ) &&
                        pWin->ImplGetWindow()->IsTopWindow() && pWin->mpWindowImpl->mbReallyVisible )
                    {
                        SalFrameGeometry g = pWin->mpWindowImpl->mpFrame->GetGeometry();
                        if( std::abs(g.x()-aState.x()) < 2 && std::abs(g.y()-aState.y()) < 5 )
                        {
                            tools::Long displacement = g.topDecoration() ? g.topDecoration() : 20;
                            if( static_cast<tools::Long>(aState.x() + displacement + aState.width() + g.rightDecoration()) > aDesktop.Right() ||
                                static_cast<tools::Long>(aState.y() + displacement + aState.height() + g.bottomDecoration()) > aDesktop.Bottom() )
                            {
                                // displacing would leave screen
                                aState.setX(g.leftDecoration() ? g.leftDecoration() : 10); // should result in (0,0)
                                aState.setY(displacement);
                                if( bWrapped ||
                                    static_cast<tools::Long>(aState.x() + displacement + aState.width() + g.rightDecoration()) > aDesktop.Right() ||
                                    static_cast<tools::Long>(aState.y() + displacement + aState.height() + g.bottomDecoration()) > aDesktop.Bottom() )
                                    break;  // further displacement not possible -> break
                                // avoid endless testing
                                bWrapped = true;
                            }
                            else
                                aState.move(displacement, displacement);
                            pWin = pSVData->maFrameData.mpFirstFrame; // check new pos again
                        }
                    }
                    pWin = pWin->mpWindowImpl->mpFrameData->mpNextFrame;
                }
            }

        mpWindowImpl->mpFrame->SetWindowState( &aState );

        // do a synchronous resize for layout reasons
        //  but use rData only when the window is not to be maximized (#i38089#)
        //  otherwise we have no useful size information
        if( (rData.mask() & vcl::WindowDataMask::State) && (nState & vcl::WindowState::Maximized) )
        {
            // query maximized size from frame
            SalFrameGeometry aGeometry = mpWindowImpl->mpFrame->GetGeometry();

            // but use it only if it is different from the restore size (rData)
            // as currently only on windows the exact size of a maximized window
            //  can be computed without actually showing the window
            if (aGeometry.width() != rData.width() || aGeometry.height() != rData.height())
                ImplHandleResize(pWindow, aGeometry.width(), aGeometry.height());
        }
        else
            if (rData.mask() & vcl::WindowDataMask::Size)
                ImplHandleResize(pWindow, aState.width(), aState.height());   // #i43799# use aState and not rData, see above
    }
    else
    {
        PosSizeFlags nPosSize = PosSizeFlags::NONE;
        if ( nValidMask & vcl::WindowDataMask::X )
            nPosSize |= PosSizeFlags::X;
        if ( nValidMask & vcl::WindowDataMask::Y )
            nPosSize |= PosSizeFlags::Y;
        if ( nValidMask & vcl::WindowDataMask::Width )
            nPosSize |= PosSizeFlags::Width;
        if ( nValidMask & vcl::WindowDataMask::Height )
            nPosSize |= PosSizeFlags::Height;

        tools::Long nX         = rData.x();
        tools::Long nY         = rData.y();
        tools::Long nWidth     = rData.width();
        tools::Long nHeight    = rData.height();
        const SalFrameGeometry& rGeom = pWindow->mpWindowImpl->mpFrame->GetGeometry();
        if( nX < 0 )
            nX = 0;
        if( nX + nWidth > static_cast<tools::Long>(rGeom.width()) )
            nX = rGeom.width() - nWidth;
        if( nY < 0 )
            nY = 0;
        if( nY + nHeight > static_cast<tools::Long>(rGeom.height()) )
            nY = rGeom.height() - nHeight;
        setPosSizePixel( nX, nY, nWidth, nHeight, nPosSize );
    }

    // tdf#146648 if an explicit size state was set, then use it as the preferred
    // size for layout
    if (nValidMask & vcl::WindowDataMask::Size)
        mbInitialLayoutSizeCalculated = true;
}

void SystemWindow::GetWindowState(vcl::WindowData& rData) const
{
    vcl::WindowDataMask nValidMask = rData.mask();
    if ( nValidMask == vcl::WindowDataMask::NONE )
        return;

    if ( mbSysChild )
    {
        rData.setMask( vcl::WindowDataMask::NONE );
        return;
    }

    const vcl::Window* pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;

    if ( pWindow->mpWindowImpl->mbFrame )
    {
        vcl::WindowData aState;
        if ( mpWindowImpl->mpFrame->GetWindowState( &aState ) )
        {
            // Limit mask only to what we've received, the rest is not set.
            nValidMask &= aState.mask();
            rData.setMask( nValidMask );
            if ( nValidMask & vcl::WindowDataMask::X )
                rData.setX( aState.x() );
            if ( nValidMask & vcl::WindowDataMask::Y )
                rData.setY( aState.y() );
            if ( nValidMask & vcl::WindowDataMask::Width )
                rData.setWidth( aState.width() );
            if ( nValidMask & vcl::WindowDataMask::Height )
                rData.setHeight( aState.height() );
            if ( nValidMask & vcl::WindowDataMask::MaximizedX )
                rData.SetMaximizedX( aState.GetMaximizedX() );
            if ( nValidMask & vcl::WindowDataMask::MaximizedY )
                rData.SetMaximizedY( aState.GetMaximizedY() );
            if ( nValidMask & vcl::WindowDataMask::MaximizedWidth )
                rData.SetMaximizedWidth( aState.GetMaximizedWidth() );
            if ( nValidMask & vcl::WindowDataMask::MaximizedHeight )
                rData.SetMaximizedHeight( aState.GetMaximizedHeight() );
            if ( nValidMask & vcl::WindowDataMask::State )
            {
                // #94144# allow Minimize again, should be masked out when read from configuration
                // 91625 - ignore Minimize
                if (!(nValidMask & vcl::WindowDataMask::Minimized))
                    aState.rState() &= ~vcl::WindowState::Minimized;
                rData.setState(aState.state());
            }
            rData.setMask( nValidMask );
        }
        else
            rData.setMask(vcl::WindowDataMask::NONE);
    }
    else
    {
        Point   aPos = GetPosPixel();
        Size    aSize = GetSizePixel();
        vcl::WindowState nState = vcl::WindowState::NONE;

        nValidMask &= vcl::WindowDataMask::PosSizeState;
        rData.setMask( nValidMask );
        if (nValidMask & vcl::WindowDataMask::X)
            rData.setX(aPos.X());
        if (nValidMask & vcl::WindowDataMask::Y)
            rData.setY(aPos.Y());
        if (nValidMask & vcl::WindowDataMask::Width)
            rData.setWidth(aSize.Width());
        if (nValidMask & vcl::WindowDataMask::Height)
            rData.setHeight(aSize.Height());
        if (nValidMask & vcl::WindowDataMask::State)
            rData.setState(nState);
    }
}

void SystemWindow::SetWindowState(std::u16string_view rStr)
{
    if (rStr.empty())
        return;
    SetWindowState(vcl::WindowData(rStr));
}

OUString SystemWindow::GetWindowState(vcl::WindowDataMask nMask) const
{
    vcl::WindowData aData;
    aData.setMask(nMask);
    GetWindowState(aData);
    return aData.toStr();
}

void SystemWindow::SetMenuBar(MenuBar* pMenuBar)
{
    if ( mpMenuBar == pMenuBar )
        return;

    MenuBar* pOldMenuBar = mpMenuBar;
    vcl::Window*  pOldWindow = nullptr;
    VclPtr<vcl::Window> pNewWindow;
    mpMenuBar = pMenuBar;

    if ( mpWindowImpl->mpBorderWindow && (mpWindowImpl->mpBorderWindow->GetType() == WindowType::BORDERWINDOW) )
    {
        if ( pOldMenuBar )
            pOldWindow = pOldMenuBar->ImplGetWindow();
        else
            pOldWindow = nullptr;
        if ( pOldWindow )
        {
            CallEventListeners( VclEventId::WindowMenubarRemoved, static_cast<void*>(pOldMenuBar) );
            pOldWindow->SetAccessible( css::uno::Reference< css::accessibility::XAccessible >() );
        }
        if ( pMenuBar )
        {
            SAL_WARN_IF( pMenuBar->pWindow, "vcl", "SystemWindow::SetMenuBar() - MenuBars can only set in one SystemWindow at time" );

            pNewWindow = MenuBar::ImplCreate(mpWindowImpl->mpBorderWindow, pOldWindow, pMenuBar);
            static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetMenuBarWindow(pNewWindow);

            CallEventListeners( VclEventId::WindowMenubarAdded, static_cast<void*>(pMenuBar) );
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

void SystemWindow::SetNotebookBar(const OUString& rUIXMLDescription,
                                  const css::uno::Reference<css::frame::XFrame>& rFrame,
                                  const NotebookBarAddonsItem& aNotebookBarAddonsItem,
                                  bool bReloadNotebookbar)
{
    if (rUIXMLDescription != maNotebookBarUIFile || bReloadNotebookbar)
    {
        static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())
            ->SetNotebookBar(rUIXMLDescription, rFrame, aNotebookBarAddonsItem);
        maNotebookBarUIFile = rUIXMLDescription;
        if(GetNotebookBar())
            GetNotebookBar()->SetSystemWindow(this);
    }
}

void SystemWindow::CloseNotebookBar()
{
    static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->CloseNotebookBar();
    maNotebookBarUIFile.clear();
}

VclPtr<NotebookBar> const & SystemWindow::GetNotebookBar() const
{
    return static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->GetNotebookBar();
}

void SystemWindow::SetMenuBarMode( MenuBarMode nMode )
{
    if ( mnMenuBarMode != nMode )
    {
        mnMenuBarMode = nMode;
        if ( mpWindowImpl->mpBorderWindow && (mpWindowImpl->mpBorderWindow->GetType() == WindowType::BORDERWINDOW) )
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
    return mpWindowImpl->mpFrame->maGeometry.screen();
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

    Window *pBox = GetWindow(GetWindowType::FirstChild);
    // tdf#141318 Do the same as SystemWindow::setOptimalLayoutSize in case we're called before initial layout
    const_cast<SystemWindow*>(this)->settingOptimalLayoutSize(pBox);
    Size aSize = VclContainer::getLayoutRequisition(*pBox);

    sal_Int32 nBorderWidth = get_border_width();

    aSize.AdjustHeight(2 * nBorderWidth );
    aSize.AdjustWidth(2 * nBorderWidth );

    return Window::CalcWindowSize(aSize);
}

void SystemWindow::setPosSizeOnContainee(Size aSize, Window &rBox)
{
    sal_Int32 nBorderWidth = get_border_width();

    aSize.AdjustWidth( -(2 * nBorderWidth) );
    aSize.AdjustHeight( -(2 * nBorderWidth) );

    Point aPos(nBorderWidth, nBorderWidth);
    VclContainer::setLayoutAllocation(rBox, aPos, CalcOutputSize(aSize));
}

IMPL_LINK_NOARG( SystemWindow, ImplHandleLayoutTimerHdl, Timer*, void )
{
    Window *pBox = GetWindow(GetWindowType::FirstChild);
    if (!isLayoutEnabled())
    {
        SAL_WARN_IF(pBox, "vcl.layout", "SystemWindow has become non-layout because extra children have been added directly to it.");
        return;
    }
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

void SystemWindow::setOptimalLayoutSize(bool bAllowWindowShrink)
{
    maLayoutIdle.Stop();

    //resize SystemWindow to fit requisition on initial show
    Window *pBox = GetWindow(GetWindowType::FirstChild);

    settingOptimalLayoutSize(pBox);

    Size aSize = get_preferred_size();

    Size aMax(bestmaxFrameSizeForScreenSize(Size(GetDesktopRectPixel().GetSize())));

    aSize.setWidth( std::min(aMax.Width(), aSize.Width()) );
    aSize.setHeight( std::min(aMax.Height(), aSize.Height()) );

    SetMinOutputSizePixel(aSize);

    if (!bAllowWindowShrink)
    {
        Size aCurrentSize = GetSizePixel();
        aSize.setWidth(std::max(aSize.Width(), aCurrentSize.Width()));
        aSize.setHeight(std::max(aSize.Height(), aCurrentSize.Height()));
    }

    SetSizePixel(aSize);
    setPosSizeOnContainee(aSize, *pBox);
}

void SystemWindow::DoInitialLayout()
{
    if (GetSettings().GetStyleSettings().GetAutoMnemonic())
       GenerateAutoMnemonicsOnHierarchy(this);

    if (isLayoutEnabled())
    {
        mbIsCalculatingInitialLayoutSize = true;
        setDeferredProperties();
        setOptimalLayoutSize(!mbInitialLayoutSizeCalculated);
        mbInitialLayoutSizeCalculated = true;
        mbIsCalculatingInitialLayoutSize = false;
    }
}

void SystemWindow::doDeferredInit(WinBits /*nBits*/)
{
    SAL_WARN("vcl.layout", "SystemWindow in layout without doDeferredInit impl");
}

VclPtr<VirtualDevice> SystemWindow::createScreenshot()
{
    // same prerequisites as in Execute()
    setDeferredProperties();
    ImplAdjustNWFSizes();
    Show();
    ToTop();
    ensureRepaint();

    Size aSize(GetOutputSizePixel());

    VclPtr<VirtualDevice> xOutput(VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA));
    xOutput->SetOutputSizePixel(aSize);

    Point aPos;
    xOutput->DrawOutDev(aPos, aSize, aPos, aSize, *GetOutDev());

    return xOutput;
}

void SystemWindow::PrePaint(vcl::RenderContext& rRenderContext)
{
    Window::PrePaint(rRenderContext);
    mbPaintComplete = false;
}

void SystemWindow::PostPaint(vcl::RenderContext& rRenderContext)
{
    Window::PostPaint(rRenderContext);
    mbPaintComplete = true;
}

void SystemWindow::ensureRepaint()
{
    // ensure repaint
    Invalidate();
    mbPaintComplete = false;

    while (!mbPaintComplete && !Application::IsQuit())
    {
        Application::Yield();
    }
}

void SystemWindow::CollectMenuBarMnemonics(MnemonicGenerator& rMnemonicGenerator) const
{
    if (MenuBar* pMenu = GetMenuBar())
    {
        sal_uInt16 nMenuItems = pMenu->GetItemCount();
        for ( sal_uInt16 i = 0; i < nMenuItems; ++i )
            rMnemonicGenerator.RegisterMnemonic( pMenu->GetItemText( pMenu->GetItemId( i ) ) );
    }
}

int SystemWindow::GetMenuBarHeight() const
{
    if (MenuBar* pMenuBar = GetMenuBar())
        return pMenuBar->GetMenuBarHeight();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
