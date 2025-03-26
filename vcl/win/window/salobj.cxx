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

#include <string.h>

#include <svsys.h>

#include <vcl/svapp.hxx>
#include <sal/log.hxx>

#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salinst.h>
#include <win/salframe.h>
#include <win/salobj.h>

#include <comphelper/windowserrorstring.hxx>

static bool ImplIsSysWindowOrChild( HWND hWndParent, HWND hWndChild )
{
    if ( hWndParent == hWndChild )
        return true;

    HWND hTempWnd = ::GetParent( hWndChild );
    while ( hTempWnd )
    {
        // stop searching if not a child window
        if ( !(GetWindowStyle( hTempWnd ) & WS_CHILD) )
            return false;
        if ( hTempWnd == hWndParent )
            return true;
        hTempWnd = ::GetParent( hTempWnd );
    }

    return false;
}

WinSalObject* ImplFindSalObject( HWND hWndChild )
{
    SalData*        pSalData = GetSalData();
    WinSalObject*   pObject = pSalData->mpFirstObject;
    while ( pObject )
    {
        if ( ImplIsSysWindowOrChild( pObject->mhWndChild, hWndChild ) )
            return pObject;

        pObject = pObject->mpNextObject;
    }

    return nullptr;
}

static WinSalFrame* ImplFindSalObjectFrame( HWND hWnd )
{
    WinSalFrame* pFrame = nullptr;
    WinSalObject* pObject = ImplFindSalObject( hWnd );
    if ( pObject )
    {
        // find matching frame
        HWND hWnd2 = ::GetParent( pObject->mhWnd );
        pFrame = GetSalData()->mpFirstFrame;
        while ( pFrame )
        {
            if ( pFrame->mhWnd == hWnd2 )
                break;

            pFrame = pFrame->mpNextFrame;
        }
    }

    return pFrame;
}

static LRESULT CALLBACK SalSysMsgProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    // Used for Unicode and none Unicode
    SalData* pSalData = GetSalData();

    if ( (nCode >= 0) && lParam )
    {
        CWPSTRUCT* pData = reinterpret_cast<CWPSTRUCT*>(lParam);
        if ( (pData->message != WM_KEYDOWN) &&
             (pData->message != WM_KEYUP) )
            pSalData->mnSalObjWantKeyEvt = 0;


        // check if we need to process data for a SalObject-window
        WinSalObject* pObject;
        if ( pData->message == WM_SETFOCUS )
        {
            pObject = ImplFindSalObject( pData->hwnd );
            if ( pObject )
            {
                pObject->mhLastFocusWnd = pData->hwnd;
                if ( ImplSalYieldMutexTryToAcquire() )
                {
                    pObject->CallCallback( SalObjEvent::GetFocus );
                    ImplSalYieldMutexRelease();
                }
                else
                {
                    bool const ret = PostMessageW(pObject->mhWnd, SALOBJ_MSG_POSTFOCUS, 0, 0);
                    SAL_WARN_IF(!ret, "vcl", "ERROR: PostMessage() failed!");
                }
            }
        }
        else if ( pData->message == WM_KILLFOCUS )
        {
            pObject = ImplFindSalObject( pData->hwnd );
            if ( pObject && !ImplFindSalObject( reinterpret_cast<HWND>(pData->wParam) ) )
            {
                // only call LoseFocus, if truly no child window gets the focus
                if ( !pData->wParam || !ImplFindSalObject( reinterpret_cast<HWND>(pData->wParam) ) )
                {
                    if ( ImplSalYieldMutexTryToAcquire() )
                    {
                        pObject->CallCallback( SalObjEvent::LoseFocus );
                        ImplSalYieldMutexRelease();
                    }
                    else
                    {
                        bool const ret = PostMessageW(pObject->mhWnd, SALOBJ_MSG_POSTFOCUS, 0, 0);
                        SAL_WARN_IF(!ret, "vcl", "ERROR: PostMessage() failed!");
                    }
                }
                else
                    pObject->mhLastFocusWnd = reinterpret_cast<HWND>(pData->wParam);
            }
        }
    }

    return CallNextHookEx( pSalData->mhSalObjMsgHook, nCode, wParam, lParam );
}

bool ImplSalPreDispatchMsg( const MSG* pMsg )
{
    // Used for Unicode and none Unicode
    SalData*        pSalData = GetSalData();
    WinSalObject*   pObject;

    if ( (pMsg->message == WM_LBUTTONDOWN) ||
         (pMsg->message == WM_RBUTTONDOWN) ||
         (pMsg->message == WM_MBUTTONDOWN) )
    {
        ImplSalYieldMutexAcquireWithWait();
        pObject = ImplFindSalObject( pMsg->hwnd );
        if ( pObject && !pObject->IsMouseTransparent() )
        {
            bool const ret = PostMessageW(pObject->mhWnd, SALOBJ_MSG_TOTOP, 0, 0);
            SAL_WARN_IF(!ret, "vcl", "ERROR: PostMessage() failed!");
        }
        ImplSalYieldMutexRelease();
    }

    if ( (pMsg->message == WM_KEYDOWN) ||
         (pMsg->message == WM_KEYUP) )
    {
        // process KeyEvents even if the control does not process them itself
        // SysKeys are processed as WM_SYSCOMMAND
        // Char-Events are not processed, as they are not accelerator-relevant
        bool bWantedKeyCode = false;
        // A-Z, 0-9 only when combined with the Control-key
        if ( ((pMsg->wParam >= 65) && (pMsg->wParam <= 90)) ||
             ((pMsg->wParam >= 48) && (pMsg->wParam <= 57)) )
        {
            if ( GetKeyState( VK_CONTROL ) & 0x8000 )
                bWantedKeyCode = true;
        }
        else if ( ((pMsg->wParam >= VK_F1) && (pMsg->wParam <= VK_F24)) ||
                  ((pMsg->wParam >= VK_SPACE) && (pMsg->wParam <= VK_HELP)) ||
                  (pMsg->wParam == VK_BACK) || (pMsg->wParam == VK_TAB) ||
                  (pMsg->wParam == VK_CLEAR) || (pMsg->wParam == VK_RETURN) ||
                  (pMsg->wParam == VK_ESCAPE) )
            bWantedKeyCode = true;
        if ( bWantedKeyCode )
        {
            ImplSalYieldMutexAcquireWithWait();
            pObject = ImplFindSalObject( pMsg->hwnd );
            if ( pObject )
                pSalData->mnSalObjWantKeyEvt = pMsg->wParam;
            ImplSalYieldMutexRelease();
        }
    }
    // check WM_SYSCHAR, to activate menu with Alt key
    else if ( pMsg->message == WM_SYSCHAR )
    {
        pSalData->mnSalObjWantKeyEvt = 0;

        sal_uInt16 nKeyCode = LOWORD( pMsg->wParam );
        // only 0-9 and A-Z
        if ( ((nKeyCode >= 48) && (nKeyCode <= 57)) ||
             ((nKeyCode >= 65) && (nKeyCode <= 90)) ||
             ((nKeyCode >= 97) && (nKeyCode <= 122)) )
        {
            bool bRet = false;
            ImplSalYieldMutexAcquireWithWait();
            pObject = ImplFindSalObject( pMsg->hwnd );
            if ( pObject )
            {
                if ( pMsg->hwnd == ::GetFocus() )
                {
                    WinSalFrame* pFrame = ImplFindSalObjectFrame( pMsg->hwnd );
                    if ( pFrame )
                    {
                        if ( ImplHandleSalObjSysCharMsg( pFrame->mhWnd, pMsg->wParam, pMsg->lParam ) )
                            bRet = true;
                    }
                }
            }
            ImplSalYieldMutexRelease();
            if ( bRet )
                return true;
        }
    }
    else
        pSalData->mnSalObjWantKeyEvt = 0;

    return false;
}

void ImplSalPostDispatchMsg( const MSG* pMsg )
{
    // Used for Unicode and none Unicode
    SalData *pSalData = GetSalData();

    if ( (pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP) )
    {
        if ( pSalData->mnSalObjWantKeyEvt == pMsg->wParam )
        {
            pSalData->mnSalObjWantKeyEvt = 0;
            if ( pMsg->hwnd == ::GetFocus() )
            {
                ImplSalYieldMutexAcquireWithWait();
                WinSalFrame* pFrame = ImplFindSalObjectFrame( pMsg->hwnd );
                if ( pFrame )
                    ImplHandleSalObjKeyMsg( pFrame->mhWnd, pMsg->message, pMsg->wParam, pMsg->lParam );
                ImplSalYieldMutexRelease();
            }
        }
    }

    pSalData->mnSalObjWantKeyEvt = 0;
}

static LRESULT CALLBACK SalSysObjWndProcW(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    switch( nMsg )
    {
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            {
            PAINTSTRUCT aPs;
            BeginPaint( hWnd, &aPs );
            EndPaint( hWnd, &aPs );
            }
            return 0;

        case WM_PARENTNOTIFY:
            if (UINT nNotifyMsg = LOWORD(wParam);
                 (nNotifyMsg == WM_LBUTTONDOWN) ||
                 (nNotifyMsg == WM_RBUTTONDOWN) ||
                 (nNotifyMsg == WM_MBUTTONDOWN) )
            {
                ImplSalYieldMutexAcquireWithWait();
                WinSalObject* pSysObj = GetSalObjWindowPtr(hWnd);
                if ( pSysObj && !pSysObj->IsMouseTransparent() )
                    pSysObj->CallCallback( SalObjEvent::ToTop );
                ImplSalYieldMutexRelease();
            }
            break;

        case WM_MOUSEACTIVATE:
            ImplSalYieldMutexAcquireWithWait();
            if (WinSalObject* pSysObj = GetSalObjWindowPtr(hWnd);
                pSysObj && !pSysObj->IsMouseTransparent())
            {
                bool const ret = PostMessageW( hWnd, SALOBJ_MSG_TOTOP, 0, 0 );
                SAL_WARN_IF(!ret, "vcl", "ERROR: PostMessage() failed!");
            }
            ImplSalYieldMutexRelease();
            break;

        case SALOBJ_MSG_TOTOP:
            if ( ImplSalYieldMutexTryToAcquire() )
            {
                if (WinSalObject* pSysObj = GetSalObjWindowPtr(hWnd))
                    pSysObj->CallCallback(SalObjEvent::ToTop);
                ImplSalYieldMutexRelease();
                return 0;
            }
            else
            {
                bool const ret = PostMessageW( hWnd, SALOBJ_MSG_TOTOP, 0, 0 );
                SAL_WARN_IF(!ret, "vcl", "ERROR: PostMessage() failed!");
            }
            break;

        case SALOBJ_MSG_POSTFOCUS:
            if ( ImplSalYieldMutexTryToAcquire() )
            {
                WinSalObject* pSysObj = GetSalObjWindowPtr(hWnd);
                HWND    hFocusWnd = ::GetFocus();
                SalObjEvent nEvent;
                if ( hFocusWnd && ImplIsSysWindowOrChild( hWnd, hFocusWnd ) )
                    nEvent = SalObjEvent::GetFocus;
                else
                    nEvent = SalObjEvent::LoseFocus;
                pSysObj->CallCallback( nEvent );
                ImplSalYieldMutexRelease();
            }
            else
            {
                bool const ret = PostMessageW(hWnd, SALOBJ_MSG_POSTFOCUS, 0, 0);
                SAL_WARN_IF(!ret, "vcl", "ERROR: PostMessage() failed!");
            }
            return 0;

        case WM_SIZE:
            if (HWND hWndChild = GetWindow(hWnd, GW_CHILD))
            {
                SetWindowPos( hWndChild,
                              nullptr, 0,  0, static_cast<int>(LOWORD( lParam )), static_cast<int>(HIWORD( lParam )),
                              SWP_NOZORDER | SWP_NOACTIVATE );
            }
            return 0;

        case WM_CREATE:
            {
            // Save the window instance at the window handle.
            CREATESTRUCTW* pStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
            WinSalObject* pSysObj = static_cast<WinSalObject*>(pStruct->lpCreateParams);
            SetSalObjWindowPtr( hWnd, pSysObj );
            // set HWND already here,
            // as instance data might be used during CreateWindow() events
            pSysObj->mhWnd = hWnd;
            return 0;
            }
    }

    return DefWindowProcW(hWnd, nMsg, wParam, lParam);
}

static LRESULT CALLBACK SalSysObjChildWndProcW(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    switch( nMsg )
    {
        // clear background for plugins
        case WM_ERASEBKGND:
            if (WinSalObject* pSysObj = GetSalObjWindowPtr(GetParent(hWnd));
                pSysObj && !pSysObj->IsEraseBackgroundEnabled())
            {
                // do not erase background
                return 1;
            }
            break;

        case WM_PAINT:
            {
            PAINTSTRUCT aPs;
            BeginPaint( hWnd, &aPs );
            EndPaint( hWnd, &aPs );
            }
            return 0;

        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            if (WinSalObject* pSysObj = GetSalObjWindowPtr(GetParent(hWnd));
                pSysObj && pSysObj->IsMouseTransparent())
            {
                // forward mouse events to parent frame
                HWND hWndParent = GetParent(pSysObj->mhWnd);

                // transform coordinates
                POINT pt;
                pt.x = static_cast<tools::Long>(LOWORD(lParam));
                pt.y = static_cast<tools::Long>(HIWORD(lParam));
                MapWindowPoints(hWnd, hWndParent, &pt, 1);
                lParam = MAKELPARAM(static_cast<WORD>(pt.x), static_cast<WORD>(pt.y));

                return SendMessageW(hWndParent, nMsg, wParam, lParam);
            }
            break;
    }

    return DefWindowProcW(hWnd, nMsg, wParam, lParam);
}

SalObject* ImplSalCreateObject( WinSalInstance* pInst, WinSalFrame* pParent )
{
    SalData* pSalData = GetSalData();

    // install hook, if it is the first SalObject
    if ( !pSalData->mpFirstObject )
    {
        pSalData->mhSalObjMsgHook = SetWindowsHookExW( WH_CALLWNDPROC,
                                                       SalSysMsgProc,
                                                       pSalData->mhInst,
                                                       pSalData->mnAppThreadId );
    }

    if ( !pSalData->mbObjClassInit )
    {
        WNDCLASSEXW aWndClassEx;
        aWndClassEx.cbSize          = sizeof( aWndClassEx );
        aWndClassEx.style           = 0;
        aWndClassEx.lpfnWndProc     = SalSysObjWndProcW;
        aWndClassEx.cbClsExtra      = 0;
        aWndClassEx.cbWndExtra      = SAL_OBJECT_WNDEXTRA;
        aWndClassEx.hInstance       = pSalData->mhInst;
        aWndClassEx.hIcon           = nullptr;
        aWndClassEx.hIconSm         = nullptr;
        aWndClassEx.hCursor         = LoadCursor( nullptr, IDC_ARROW );
        aWndClassEx.hbrBackground   = nullptr;
        aWndClassEx.lpszMenuName    = nullptr;
        aWndClassEx.lpszClassName   = SAL_OBJECT_CLASSNAMEW;
        if ( RegisterClassExW( &aWndClassEx ) )
        {
            // Clean background first because of plugins.
            aWndClassEx.cbWndExtra      = 0;
            aWndClassEx.hbrBackground   = reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
            aWndClassEx.lpfnWndProc     = SalSysObjChildWndProcW;
            aWndClassEx.lpszClassName   = SAL_OBJECT_CHILDCLASSNAMEW;
            if ( RegisterClassExW( &aWndClassEx ) )
                pSalData->mbObjClassInit = true;
        }
    }

    if ( pSalData->mbObjClassInit )
    {
        WinSalObject* pObject = new WinSalObject;

        // #135235# Clip siblings of this
        // SystemChildWindow. Otherwise, DXCanvas (using a hidden
        // SystemChildWindow) clobbers applets/plugins during
        // animations .
        HWND hWnd = CreateWindowExW( 0, SAL_OBJECT_CLASSNAMEW, L"",
                                     WS_CHILD | WS_CLIPSIBLINGS, 0, 0, 0, 0,
                                     pParent->mhWnd, nullptr,
                                     pInst->mhInst, pObject );

        HWND hWndChild = nullptr;
        if ( hWnd )
        {
            // #135235# Explicitly stack SystemChildWindows in
            // the order they're created - since there's no notion
            // of zorder.
            SetWindowPos(hWnd,HWND_TOP,0,0,0,0,
                         SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOREDRAW|SWP_NOSIZE);
            hWndChild = CreateWindowExW( 0, SAL_OBJECT_CHILDCLASSNAMEW, L"",
                                         WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                                         0, 0, 0, 0,
                                         hWnd, nullptr,
                                         pInst->mhInst, nullptr );
        }

        if ( !hWndChild )
        {
            SAL_WARN("vcl", "CreateWindowExW failed: " << comphelper::WindowsErrorString(GetLastError()));

            delete pObject;
            return nullptr;
        }

        if ( hWnd )
        {
            pObject->mhWnd          = hWnd;
            pObject->mhWndChild     = hWndChild;
            pObject->maSysData.hWnd = hWndChild;
            return pObject;
        }
    }

    return nullptr;
}

WinSalObject::WinSalObject()
{
    SalData* pSalData = GetSalData();

    mhWnd           = nullptr;
    mhWndChild      = nullptr;
    mhLastFocusWnd  = nullptr;
    mpStdClipRgnData    = nullptr;

    // Insert object in objectlist
    mpNextObject = pSalData->mpFirstObject;
    pSalData->mpFirstObject = this;
}

WinSalObject::~WinSalObject()
{
    SalData* pSalData = GetSalData();

    // remove frame from framelist
    if ( this == pSalData->mpFirstObject )
    {
        pSalData->mpFirstObject = mpNextObject;

        // remove hook, if it is the last SalObject
        if ( !pSalData->mpFirstObject )
            UnhookWindowsHookEx( pSalData->mhSalObjMsgHook );
    }
    else
    {
        WinSalObject* pTempObject = pSalData->mpFirstObject;
        while ( pTempObject->mpNextObject != this )
            pTempObject = pTempObject->mpNextObject;

        pTempObject->mpNextObject = mpNextObject;
    }

    // destroy cache data
    delete [] reinterpret_cast<BYTE*>(mpStdClipRgnData);

    if ( mhWndChild )
        DestroyWindow( mhWndChild );
    if ( mhWnd )
        DestroyWindow( mhWnd );
}

void WinSalObject::ResetClipRegion()
{
    SetWindowRgn( mhWnd, nullptr, TRUE );
}

void WinSalObject::BeginSetClipRegion( sal_uInt32 nRectCount )
{
    sal_uLong nRectBufSize = sizeof(RECT)*nRectCount;
    if ( nRectCount < SAL_CLIPRECT_COUNT )
    {
        if ( !mpStdClipRgnData )
            mpStdClipRgnData = reinterpret_cast<RGNDATA*>(new BYTE[sizeof(RGNDATA)-1+(SAL_CLIPRECT_COUNT*sizeof(RECT))]);
        mpClipRgnData = mpStdClipRgnData;
    }
    else
        mpClipRgnData = reinterpret_cast<RGNDATA*>(new BYTE[sizeof(RGNDATA)-1+nRectBufSize]);
    mpClipRgnData->rdh.dwSize     = sizeof( RGNDATAHEADER );
    mpClipRgnData->rdh.iType      = RDH_RECTANGLES;
    mpClipRgnData->rdh.nCount     = nRectCount;
    mpClipRgnData->rdh.nRgnSize  = nRectBufSize;
    SetRectEmpty( &(mpClipRgnData->rdh.rcBound) );
    mpNextClipRect        = reinterpret_cast<RECT*>(&(mpClipRgnData->Buffer));
    mbFirstClipRect       = true;
}

void WinSalObject::UnionClipRegion( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight )
{
    RECT*       pRect = mpNextClipRect;
    RECT*       pBoundRect = &(mpClipRgnData->rdh.rcBound);
    tools::Long        nRight = nX + nWidth;
    tools::Long        nBottom = nY + nHeight;

    if ( mbFirstClipRect )
    {
        pBoundRect->left    = nX;
        pBoundRect->top     = nY;
        pBoundRect->right   = nRight;
        pBoundRect->bottom  = nBottom;
        mbFirstClipRect = false;
    }
    else
    {
        if ( nX < pBoundRect->left )
            pBoundRect->left = static_cast<int>(nX);

        if ( nY < pBoundRect->top )
            pBoundRect->top = static_cast<int>(nY);

        if ( nRight > pBoundRect->right )
            pBoundRect->right = static_cast<int>(nRight);

        if ( nBottom > pBoundRect->bottom )
            pBoundRect->bottom = static_cast<int>(nBottom);
    }

    pRect->left     = static_cast<int>(nX);
    pRect->top      = static_cast<int>(nY);
    pRect->right    = static_cast<int>(nRight);
    pRect->bottom   = static_cast<int>(nBottom);
    mpNextClipRect++;
}

void WinSalObject::EndSetClipRegion()
{
    HRGN hRegion;

    // create a ClipRegion from the vcl::Region data
    if ( mpClipRgnData->rdh.nCount == 1 )
    {
        RECT* pRect = &(mpClipRgnData->rdh.rcBound);
        hRegion = CreateRectRgn( pRect->left, pRect->top,
                                 pRect->right, pRect->bottom );
    }
    else
    {
        sal_uLong nSize = mpClipRgnData->rdh.nRgnSize+sizeof(RGNDATAHEADER);
        hRegion = ExtCreateRegion( nullptr, nSize, mpClipRgnData );
        if ( mpClipRgnData != mpStdClipRgnData )
            delete [] reinterpret_cast<BYTE*>(mpClipRgnData);
    }

    SAL_WARN_IF( !hRegion, "vcl", "SalObject::EndSetClipRegion() - Can't create ClipRegion" );
    SetWindowRgn( mhWnd, hRegion, TRUE );
}

void WinSalObject::SetPosSize( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight )
{
    sal_uLong nStyle = 0;
    bool bVisible = (GetWindowStyle( mhWnd ) & WS_VISIBLE) != 0;
    if ( bVisible )
    {
        ShowWindow( mhWnd, SW_HIDE );
        nStyle |= SWP_SHOWWINDOW;
    }
    SetWindowPos( mhWnd, nullptr,
                  static_cast<int>(nX), static_cast<int>(nY), static_cast<int>(nWidth), static_cast<int>(nHeight),
                  SWP_NOZORDER | SWP_NOACTIVATE | nStyle );
}

void WinSalObject::Show( bool bVisible )
{
    if ( bVisible )
        ShowWindow( mhWnd, SW_SHOWNORMAL );
    else
        ShowWindow( mhWnd, SW_HIDE );
}

void WinSalObject::Enable( bool bEnable )
{
    EnableWindow( mhWnd, bEnable );
}

void WinSalObject::GrabFocus()
{
    if ( mhLastFocusWnd &&
         IsWindow( mhLastFocusWnd ) &&
         ImplIsSysWindowOrChild( mhWndChild, mhLastFocusWnd ) )
        ::SetFocus( mhLastFocusWnd );
    else
        ::SetFocus( mhWndChild );
}

const SystemEnvData& WinSalObject::GetSystemData() const
{
    return maSysData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
