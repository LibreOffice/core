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
#include <tools/debug.hxx>

#include <vcl/svapp.hxx>

#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salinst.h>
#include <win/salframe.h>
#include <win/salobj.h>

static bool ImplIsSysWindowOrChild( HWND hWndParent, HWND hWndChild )
{
    if ( hWndParent == hWndChild )
        return TRUE;

    HWND hTempWnd = ::GetParent( hWndChild );
    while ( hTempWnd )
    {
        // stop searching if not a child window
        if ( !(GetWindowStyle( hTempWnd ) & WS_CHILD) )
            return FALSE;
        if ( hTempWnd == hWndParent )
            return TRUE;
        hTempWnd = ::GetParent( hTempWnd );
    }

    return FALSE;
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

    return NULL;
}

WinSalFrame* ImplFindSalObjectFrame( HWND hWnd )
{
    WinSalFrame* pFrame = NULL;
    WinSalObject* pObject = ImplFindSalObject( hWnd );
    if ( pObject )
    {
        // Dazugehoerenden Frame suchen
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

LRESULT CALLBACK SalSysMsgProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    // Used for Unicode and none Unicode
    SalData* pSalData = GetSalData();

    if ( (nCode >= 0) && lParam )
    {
        CWPSTRUCT* pData = (CWPSTRUCT*)lParam;
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
                    pObject->CallCallback( SALOBJ_EVENT_GETFOCUS, 0 );
                    ImplSalYieldMutexRelease();
                }
                else
                    PostMessageW( pObject->mhWnd, SALOBJ_MSG_POSTFOCUS, 0, 0 );
            }
        }
        else if ( pData->message == WM_KILLFOCUS )
        {
            pObject = ImplFindSalObject( pData->hwnd );
            if ( pObject && !ImplFindSalObject( (HWND)pData->wParam ) )
            {
                // only call LoseFocus, if truly no child window gets the focus
                if ( !pData->wParam || !ImplFindSalObject( (HWND)pData->wParam ) )
                {
                    if ( ImplSalYieldMutexTryToAcquire() )
                    {
                        pObject->CallCallback( SALOBJ_EVENT_LOSEFOCUS, 0 );
                        ImplSalYieldMutexRelease();
                    }
                    else
                        PostMessageW( pObject->mhWnd, SALOBJ_MSG_POSTFOCUS, 0, 0 );
                }
                else
                    pObject->mhLastFocusWnd = (HWND)pData->wParam;
            }
        }
    }

    return CallNextHookEx( pSalData->mhSalObjMsgHook, nCode, wParam, lParam );
}

bool ImplSalPreDispatchMsg( MSG* pMsg )
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
            PostMessageW( pObject->mhWnd, SALOBJ_MSG_TOTOP, 0, 0 );
        ImplSalYieldMutexRelease();
    }

    if ( (pMsg->message == WM_KEYDOWN) ||
         (pMsg->message == WM_KEYUP) )
    {
        // process KeyEvents even if the control does not process them itself
        // SysKeys are processed as WM_SYSCOMMAND
        // Char-Events are not processed, as they are not accelerator-relevant
        bool bWantedKeyCode = FALSE;
        // A-Z, 0-9 only when combined with the Control-key
        if ( ((pMsg->wParam >= 65) && (pMsg->wParam <= 90)) ||
             ((pMsg->wParam >= 48) && (pMsg->wParam <= 57)) )
        {
            if ( GetKeyState( VK_CONTROL ) & 0x8000 )
                bWantedKeyCode = TRUE;
        }
        else if ( ((pMsg->wParam >= VK_F1) && (pMsg->wParam <= VK_F24)) ||
                  ((pMsg->wParam >= VK_SPACE) && (pMsg->wParam <= VK_HELP)) ||
                  (pMsg->wParam == VK_BACK) || (pMsg->wParam == VK_TAB) ||
                  (pMsg->wParam == VK_CLEAR) || (pMsg->wParam == VK_RETURN) ||
                  (pMsg->wParam == VK_ESCAPE) )
            bWantedKeyCode = TRUE;
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
        // Nur 0-9 und A-Z
        if ( ((nKeyCode >= 48) && (nKeyCode <= 57)) ||
             ((nKeyCode >= 65) && (nKeyCode <= 90)) ||
             ((nKeyCode >= 97) && (nKeyCode <= 122)) )
        {
            bool bRet = FALSE;
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
                            bRet = TRUE;
                    }
                }
            }
            ImplSalYieldMutexRelease();
            if ( bRet )
                return TRUE;
        }
    }
    else
        pSalData->mnSalObjWantKeyEvt = 0;

    return FALSE;
}

void ImplSalPostDispatchMsg( MSG* pMsg, LRESULT /* nDispatchResult */ )
{
    // Used for Unicode and none Unicode
    SalData*        pSalData = GetSalData();

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

LRESULT CALLBACK SalSysObjWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, int& rDef )
{
    WinSalObject*   pSysObj;
    LRESULT         nRet = 0;

    switch( nMsg )
    {
        case WM_ERASEBKGND:
            nRet = 1;
            rDef = FALSE;
            break;
        case WM_PAINT:
            {
            PAINTSTRUCT aPs;
            BeginPaint( hWnd, &aPs );
            EndPaint( hWnd, &aPs );
            rDef = FALSE;
            }
            break;

        case WM_PARENTNOTIFY:
            {
            UINT nNotifyMsg = LOWORD( wParam );
            if ( (nNotifyMsg == WM_LBUTTONDOWN) ||
                 (nNotifyMsg == WM_RBUTTONDOWN) ||
                 (nNotifyMsg == WM_MBUTTONDOWN) )
            {
                ImplSalYieldMutexAcquireWithWait();
                pSysObj = GetSalObjWindowPtr( hWnd );
                if ( pSysObj && !pSysObj->IsMouseTransparent() )
                    pSysObj->CallCallback( SALOBJ_EVENT_TOTOP, 0 );
                ImplSalYieldMutexRelease();
            }
            }
            break;

        case WM_MOUSEACTIVATE:
            {
            ImplSalYieldMutexAcquireWithWait();
            pSysObj = GetSalObjWindowPtr( hWnd );
            if ( pSysObj && !pSysObj->IsMouseTransparent() )
                PostMessageW( hWnd, SALOBJ_MSG_TOTOP, 0, 0 );
            ImplSalYieldMutexRelease();
            }
            break;

        case SALOBJ_MSG_TOTOP:
            if ( ImplSalYieldMutexTryToAcquire() )
            {
                pSysObj = GetSalObjWindowPtr( hWnd );
                pSysObj->CallCallback( SALOBJ_EVENT_TOTOP, 0 );
                ImplSalYieldMutexRelease();
                rDef = FALSE;
            }
            else
                PostMessageW( hWnd, SALOBJ_MSG_TOTOP, 0, 0 );
            break;

        case SALOBJ_MSG_POSTFOCUS:
            if ( ImplSalYieldMutexTryToAcquire() )
            {
                pSysObj = GetSalObjWindowPtr( hWnd );
                HWND    hFocusWnd = ::GetFocus();
                sal_uInt16 nEvent;
                if ( hFocusWnd && ImplIsSysWindowOrChild( hWnd, hFocusWnd ) )
                    nEvent = SALOBJ_EVENT_GETFOCUS;
                else
                    nEvent = SALOBJ_EVENT_LOSEFOCUS;
                pSysObj->CallCallback( nEvent, 0 );
                ImplSalYieldMutexRelease();
            }
            else
                PostMessageW( hWnd, SALOBJ_MSG_POSTFOCUS, 0, 0 );
            rDef = FALSE;
            break;

        case WM_SIZE:
            {
            HWND hWndChild = GetWindow( hWnd, GW_CHILD );
            if ( hWndChild )
            {
                SetWindowPos( hWndChild,
                              0, 0,  0, (int)LOWORD( lParam ), (int)HIWORD( lParam ),
                              SWP_NOZORDER | SWP_NOACTIVATE );
            }
            }
            rDef = FALSE;
            break;

        case WM_CREATE:
            {
            // Window-Instanz am Windowhandle speichern
            // Can also be used for the W-Version, because the struct
            // to access lpCreateParams is the same structure
            CREATESTRUCTA* pStruct = (CREATESTRUCTA*)lParam;
            pSysObj = (WinSalObject*)pStruct->lpCreateParams;
            SetSalObjWindowPtr( hWnd, pSysObj );
            // set HWND already here,
            // as instance data might be used during CreateWindow() events
            pSysObj->mhWnd = hWnd;
            rDef = FALSE;
            }
            break;
    }

    return nRet;
}

LRESULT CALLBACK SalSysObjWndProcA( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = SalSysObjWndProc( hWnd, nMsg, wParam, lParam, bDef );
    if ( bDef )
        nRet = DefWindowProcA( hWnd, nMsg, wParam, lParam );
    return nRet;
}

LRESULT CALLBACK SalSysObjChildWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, int& rDef )
{
    LRESULT nRet = 0;

    switch( nMsg )
    {
        // clear background for plugins
        case WM_ERASEBKGND:
            {
                WinSalObject* pSysObj = GetSalObjWindowPtr( ::GetParent( hWnd ) );

                if( pSysObj && !pSysObj->IsEraseBackgroundEnabled() )
                {
                    // do not erase background
                    nRet = 1;
                    rDef = FALSE;
                }
            }
            break;

        case WM_PAINT:
            {
            PAINTSTRUCT aPs;
            BeginPaint( hWnd, &aPs );
            EndPaint( hWnd, &aPs );
            rDef = FALSE;
            }
            break;

        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            {
                WinSalObject* pSysObj;
                pSysObj = GetSalObjWindowPtr( ::GetParent( hWnd ) );

                if( pSysObj && pSysObj->IsMouseTransparent() )
                {
                    // forward mouse events to parent frame
                    HWND hWndParent = ::GetParent( pSysObj->mhWnd );

                    // transform coordinates
                    POINT pt;
                    pt.x = (long) LOWORD( lParam );
                    pt.y = (long) HIWORD( lParam );
                    MapWindowPoints( hWnd, hWndParent, &pt, 1 );
                    lParam = MAKELPARAM( (WORD) pt.x, (WORD) pt.y );

                    nRet = SendMessageW( hWndParent, nMsg, wParam, lParam );
                    rDef = FALSE;
                }
            }
            break;
    }

    return nRet;
}

LRESULT CALLBACK SalSysObjChildWndProcA( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = SalSysObjChildWndProc( hWnd, nMsg, wParam, lParam, bDef );
    if ( bDef )
        nRet = DefWindowProcA( hWnd, nMsg, wParam, lParam );
    return nRet;
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
        WNDCLASSEXA aWndClassEx;
        aWndClassEx.cbSize          = sizeof( aWndClassEx );
        aWndClassEx.style           = 0;
        aWndClassEx.lpfnWndProc     = SalSysObjWndProcA;
        aWndClassEx.cbClsExtra      = 0;
        aWndClassEx.cbWndExtra      = SAL_OBJECT_WNDEXTRA;
        aWndClassEx.hInstance       = pSalData->mhInst;
        aWndClassEx.hIcon           = 0;
        aWndClassEx.hIconSm         = 0;
        aWndClassEx.hCursor         = LoadCursor( 0, IDC_ARROW );
        aWndClassEx.hbrBackground   = 0;
        aWndClassEx.lpszMenuName    = 0;
        aWndClassEx.lpszClassName   = SAL_OBJECT_CLASSNAMEA;
        if ( RegisterClassExA( &aWndClassEx ) )
        {
            // Clean background first because of plugins.
            aWndClassEx.cbWndExtra      = 0;
            aWndClassEx.hbrBackground   = (HBRUSH)(COLOR_WINDOW+1);
            aWndClassEx.lpfnWndProc     = SalSysObjChildWndProcA;
            aWndClassEx.lpszClassName   = SAL_OBJECT_CHILDCLASSNAMEA;
            if ( RegisterClassExA( &aWndClassEx ) )
                pSalData->mbObjClassInit = TRUE;
        }
    }

    if ( pSalData->mbObjClassInit )
    {
        WinSalObject* pObject = new WinSalObject;

        // #135235# Clip siblings of this
        // SystemChildWindow. Otherwise, DXCanvas (using a hidden
        // SystemChildWindow) clobbers applets/plugins during
        // animations .
        HWND hWnd = CreateWindowExA( 0, SAL_OBJECT_CLASSNAMEA, "",
                                     WS_CHILD | WS_CLIPSIBLINGS, 0, 0, 0, 0,
                                     pParent->mhWnd, 0,
                                     pInst->mhInst, (void*)pObject );

        HWND hWndChild = 0;
        if ( hWnd )
        {
            // #135235# Explicitely stack SystemChildWindows in
            // the order they're created - since there's no notion
            // of zorder.
            SetWindowPos(hWnd,HWND_TOP,0,0,0,0,
                         SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOREDRAW|SWP_NOSIZE);
            hWndChild = CreateWindowExA( 0, SAL_OBJECT_CHILDCLASSNAMEA, "",
                                         WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                                         0, 0, 0, 0,
                                         hWnd, 0,
                                         pInst->mhInst, NULL );
        }

        if ( !hWndChild )
        {
#if OSL_DEBUG_LEVEL > 1
            char *msg = NULL;
            FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER
                          |FORMAT_MESSAGE_IGNORE_INSERTS
                          |FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL, GetLastError(), 0,
                           (LPSTR) &msg, 0, NULL);
            MessageBoxA(NULL, msg, "CreateWindowExA failed", MB_OK);
#endif
            delete pObject;
            return NULL;
        }

        if ( hWnd )
        {
            pObject->mhWnd          = hWnd;
            pObject->mhWndChild     = hWndChild;
            pObject->maSysData.hWnd = hWndChild;
            return pObject;
        }
    }

    return NULL;
}

WinSalObject::WinSalObject()
{
    SalData* pSalData = GetSalData();

    mhWnd           = 0;
    mhWndChild      = 0;
    mhLastFocusWnd  = 0;
    maSysData.nSize = sizeof( SystemEnvData );
    mpStdClipRgnData    = NULL;

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

    // Cache-Daten zerstoeren
    delete [] (BYTE*)mpStdClipRgnData;

    HWND hWndParent = ::GetParent( mhWnd );

    if ( mhWndChild )
        DestroyWindow( mhWndChild );
    if ( mhWnd )
        DestroyWindow( mhWnd );

    // reset palette, if no external child window is left,
    // as they might have overwritten our palette
    if ( hWndParent &&
         ::GetActiveWindow() == hWndParent &&
         !GetWindow( hWndParent, GW_CHILD ) )
        SendMessageW( hWndParent, SAL_MSG_FORCEPALETTE, 0, 0 );
}

void WinSalObject::ResetClipRegion()
{
    SetWindowRgn( mhWnd, 0, TRUE );
}

sal_uInt16 WinSalObject::GetClipRegionType()
{
    return SAL_OBJECT_CLIP_INCLUDERECTS;
}

void WinSalObject::BeginSetClipRegion( sal_uLong nRectCount )
{
    sal_uLong nRectBufSize = sizeof(RECT)*nRectCount;
    if ( nRectCount < SAL_CLIPRECT_COUNT )
    {
        if ( !mpStdClipRgnData )
            mpStdClipRgnData = (RGNDATA*)new BYTE[sizeof(RGNDATA)-1+(SAL_CLIPRECT_COUNT*sizeof(RECT))];
        mpClipRgnData = mpStdClipRgnData;
    }
    else
        mpClipRgnData = (RGNDATA*)new BYTE[sizeof(RGNDATA)-1+nRectBufSize];
    mpClipRgnData->rdh.dwSize     = sizeof( RGNDATAHEADER );
    mpClipRgnData->rdh.iType      = RDH_RECTANGLES;
    mpClipRgnData->rdh.nCount     = nRectCount;
    mpClipRgnData->rdh.nRgnSize  = nRectBufSize;
    SetRectEmpty( &(mpClipRgnData->rdh.rcBound) );
    mpNextClipRect        = (RECT*)(&(mpClipRgnData->Buffer));
    mbFirstClipRect       = TRUE;
}

void WinSalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    RECT*       pRect = mpNextClipRect;
    RECT*       pBoundRect = &(mpClipRgnData->rdh.rcBound);
    long        nRight = nX + nWidth;
    long        nBottom = nY + nHeight;

    if ( mbFirstClipRect )
    {
        pBoundRect->left    = nX;
        pBoundRect->top     = nY;
        pBoundRect->right   = nRight;
        pBoundRect->bottom  = nBottom;
        mbFirstClipRect = FALSE;
    }
    else
    {
        if ( nX < pBoundRect->left )
            pBoundRect->left = (int)nX;

        if ( nY < pBoundRect->top )
            pBoundRect->top = (int)nY;

        if ( nRight > pBoundRect->right )
            pBoundRect->right = (int)nRight;

        if ( nBottom > pBoundRect->bottom )
            pBoundRect->bottom = (int)nBottom;
    }

    pRect->left     = (int)nX;
    pRect->top      = (int)nY;
    pRect->right    = (int)nRight;
    pRect->bottom   = (int)nBottom;
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
        hRegion = ExtCreateRegion( NULL, nSize, mpClipRgnData );
        if ( mpClipRgnData != mpStdClipRgnData )
            delete [] (BYTE*)mpClipRgnData;
    }

    DBG_ASSERT( hRegion, "SalObject::EndSetClipRegion() - Can't create ClipRegion" );
    SetWindowRgn( mhWnd, hRegion, TRUE );
}

void WinSalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
    sal_uLong nStyle = 0;
    bool bVisible = (GetWindowStyle( mhWnd ) & WS_VISIBLE) != 0;
    if ( bVisible )
    {
        ShowWindow( mhWnd, SW_HIDE );
        nStyle |= SWP_SHOWWINDOW;
    }
    SetWindowPos( mhWnd, 0,
                  (int)nX, (int)nY, (int)nWidth, (int)nHeight,
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

const SystemEnvData* WinSalObject::GetSystemData() const
{
    return &maSysData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
