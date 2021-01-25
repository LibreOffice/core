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

#include <vcl/window.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>

#include <window.h>
#include <salinst.hxx>
#include <salobj.hxx>
#include <svdata.hxx>

using namespace ::com::sun::star;

static void ImplSysChildProc( SystemChildWindow* pInst, SalObjEvent nEvent )
{
    VclPtr<SystemChildWindow> pWindow = pInst;

    switch ( nEvent )
    {
        case SalObjEvent::GetFocus:
            // get focus, such that all handlers are called,
            // as if this window gets the focus assuring
            // that the frame does not steal it
            pWindow->ImplGetFrameData()->mbSysObjFocus = true;
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = true;
            pWindow->ToTop( ToTopFlags::NoGrabFocus );
            if( pWindow->IsDisposed() )
                break;
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = false;
            pWindow->ImplGetFrameData()->mbInSysObjFocusHdl = true;
            pWindow->GrabFocus();
            if( pWindow->IsDisposed() )
                break;
            pWindow->ImplGetFrameData()->mbInSysObjFocusHdl = false;
            break;

        case SalObjEvent::LoseFocus:
            // trigger a LoseFocus which matches the status
            // of the window with matching Activate-Status
            if (pWindow->IsDisposed())
                break;
            pWindow->ImplGetFrameData()->mbSysObjFocus = false;
            if ( !pWindow->ImplGetFrameData()->mnFocusId )
            {
                pWindow->ImplGetFrameData()->mbStartFocusState = true;
                pWindow->ImplGetFrameData()->mnFocusId = Application::PostUserEvent( LINK( pWindow->ImplGetFrameWindow(), vcl::Window, ImplAsyncFocusHdl ), nullptr, true );
            }
            break;

        case SalObjEvent::ToTop:
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = true;
            if ( !Application::GetFocusWindow() || pWindow->HasChildPathFocus() )
                pWindow->ToTop( ToTopFlags::NoGrabFocus );
            else
                pWindow->ToTop();
            if( pWindow->IsDisposed() )
                break;
            pWindow->GrabFocus();
            if( pWindow->IsDisposed() )
                break;
            pWindow->ImplGetFrameData()->mbInSysObjToTopHdl = false;
            break;

        default: break;
    }
}

void SystemChildWindow::ImplInitSysChild( vcl::Window* pParent, WinBits nStyle, SystemWindowData *pData, bool bShow )
{
    mpWindowImpl->mpSysObj = ImplGetSVData()->mpDefInst->CreateObject( pParent->ImplGetFrame(), pData, bShow );

    Window::ImplInit( pParent, nStyle, nullptr );

    // we do not paint if it is the right SysChild
    if ( GetSystemData() )
    {
        mpWindowImpl->mpSysObj->SetCallback( this, ImplSysChildProc );
        SetParentClipMode( ParentClipMode::Clip );
        SetBackground();
    }
}

SystemChildWindow::SystemChildWindow( vcl::Window* pParent, WinBits nStyle ) :
    Window( WindowType::SYSTEMCHILDWINDOW )
{
    ImplInitSysChild( pParent, nStyle, nullptr );
}

SystemChildWindow::SystemChildWindow( vcl::Window* pParent, WinBits nStyle, SystemWindowData *pData, bool bShow ) :
    Window( WindowType::SYSTEMCHILDWINDOW )
{
    ImplInitSysChild( pParent, nStyle, pData, bShow );
}

SystemChildWindow::~SystemChildWindow()
{
    disposeOnce();
}

void SystemChildWindow::dispose()
{
    Hide();
    if ( mpWindowImpl && mpWindowImpl->mpSysObj )
    {
        ImplGetSVData()->mpDefInst->DestroyObject( mpWindowImpl->mpSysObj );
        mpWindowImpl->mpSysObj = nullptr;
    }
    Window::dispose();
}

const SystemEnvData* SystemChildWindow::GetSystemData() const
{
    if ( mpWindowImpl->mpSysObj )
        return mpWindowImpl->mpSysObj->GetSystemData();
    else
        return nullptr;
}

void SystemChildWindow::EnableEraseBackground( bool bEnable )
{
    if ( mpWindowImpl->mpSysObj )
        mpWindowImpl->mpSysObj->EnableEraseBackground( bEnable );
}

Size SystemChildWindow::GetOptimalSize() const
{
    if (mpWindowImpl->mpSysObj)
        return mpWindowImpl->mpSysObj->GetOptimalSize();
    return vcl::Window::GetOptimalSize();
}

void SystemChildWindow::SetLeaveEnterBackgrounds(const css::uno::Sequence<css::uno::Any>& rLeaveArgs, const css::uno::Sequence<css::uno::Any>& rEnterArgs)
{
    if (mpWindowImpl->mpSysObj)
        mpWindowImpl->mpSysObj->SetLeaveEnterBackgrounds(rLeaveArgs, rEnterArgs);
}

void SystemChildWindow::SetForwardKey( bool bEnable )
{
    if ( mpWindowImpl->mpSysObj )
        mpWindowImpl->mpSysObj->SetForwardKey( bEnable );
}

sal_IntPtr SystemChildWindow::GetParentWindowHandle() const
{
    sal_IntPtr nRet = 0;

#if defined(_WIN32)
    nRet = reinterpret_cast< sal_IntPtr >( GetSystemData()->hWnd );
#elif defined MACOSX
    // FIXME: this is wrong
    nRet = reinterpret_cast< sal_IntPtr >( GetSystemData()->mpNSView );
#elif defined ANDROID
    // Nothing
#elif defined IOS
    // Nothing
#elif defined UNX
    nRet = GetSystemData()->GetWindowHandle();
#endif

    return nRet;
}

void* SystemChildWindow::CreateGStreamerSink()
{
    return ImplGetSVData()->mpDefInst->CreateGStreamerSink(this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
