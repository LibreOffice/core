/*************************************************************************
 *
 *  $RCSfile: salobj.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:53:06 $
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

#include <string.h>

#ifndef _SVWIN_HXX
#include <tools/svwin.h>
#endif

#define _SV_SALOBJ_CXX

#ifndef _SV_WINCOMP_HXX
#include <wincomp.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SALOBJ_H
#include <salobj.h>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

// =======================================================================

static BOOL ImplIsSysWindowOrChild( HWND hWndParent, HWND hWndChild )
{
    if ( hWndParent == hWndChild )
        return TRUE;

    HWND hTempWnd = ::GetParent( hWndChild );
    while ( hTempWnd )
    {
        // Ab nicht Child-Fenstern hoeren wir auf zu suchen
        if ( !(GetWindowStyle( hTempWnd ) & WS_CHILD) )
            return FALSE;
        if ( hTempWnd == hWndParent )
            return TRUE;
        hTempWnd = ::GetParent( hTempWnd );
    }

    return FALSE;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

WinSalFrame* ImplFindSalObjectFrame( HWND hWnd )
{
    WinSalFrame* pFrame = NULL;
    WinSalObject* pObject = ImplFindSalObject( hWnd );
    if ( pObject )
    {
        // Dazugehoerenden Frame suchen
        HWND hWnd = ::GetParent( pObject->mhWnd );
        pFrame = GetSalData()->mpFirstFrame;
        while ( pFrame )
        {
            if ( pFrame->mhWnd == hWnd )
                break;

            pFrame = pFrame->mpNextFrame;
        }
    }

    return pFrame;
}

// -----------------------------------------------------------------------

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

        // Testen, ob wir Daten fuer ein SalObject-Fenster behandeln
        // muessen
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
                    ImplPostMessage( pObject->mhWnd, SALOBJ_MSG_POSTFOCUS, 0, 0 );
            }
        }
        else if ( pData->message == WM_KILLFOCUS )
        {
            pObject = ImplFindSalObject( pData->hwnd );
            if ( pObject && !ImplFindSalObject( (HWND)pData->wParam ) )
            {
                // LoseFocus nur rufen, wenn wirklich kein ChildFenster
                // den Focus bekommt
                if ( !pData->wParam || !ImplFindSalObject( (HWND)pData->wParam ) )
                {
                    if ( ImplSalYieldMutexTryToAcquire() )
                    {
                        pObject->CallCallback( SALOBJ_EVENT_LOSEFOCUS, 0 );
                        ImplSalYieldMutexRelease();
                    }
                    else
                        ImplPostMessage( pObject->mhWnd, SALOBJ_MSG_POSTFOCUS, 0, 0 );
                }
                else
                    pObject->mhLastFocusWnd = (HWND)pData->wParam;
            }
        }
    }

    return CallNextHookEx( pSalData->mhSalObjMsgHook, nCode, wParam, lParam );
}

// -----------------------------------------------------------------------

BOOL ImplSalPreDispatchMsg( MSG* pMsg )
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
        if ( pObject )
            ImplPostMessage( pObject->mhWnd, SALOBJ_MSG_TOTOP, 0, 0 );
        ImplSalYieldMutexRelease();
    }

    if ( (pMsg->message == WM_KEYDOWN) ||
         (pMsg->message == WM_KEYUP) )
    {
        // KeyEvents wollen wir nach Moeglichkeit auch abarbeiten,
        // wenn das Control diese nicht selber auswertet
        // SysKeys werden als WM_SYSCOMMAND verarbeitet
        // Char-Events verarbeiten wir nicht, da wir nur
        // Accelerator relevante Keys verarbeiten wollen
        BOOL bWantedKeyCode = FALSE;
        // A-Z, 0-9 nur in Verbindung mit Control-Taste
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
    // Hier WM_SYSCHAR abfangen, um mit Alt+Taste evtl. Menu zu aktivieren
    else if ( pMsg->message == WM_SYSCHAR )
    {
        pSalData->mnSalObjWantKeyEvt = 0;

        USHORT nKeyCode = LOWORD( pMsg->wParam );
        // Nur 0-9 und A-Z
        if ( ((nKeyCode >= 48) && (nKeyCode <= 57)) ||
             ((nKeyCode >= 65) && (nKeyCode <= 90)) ||
             ((nKeyCode >= 97) && (nKeyCode <= 122)) )
        {
            BOOL bRet = FALSE;
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

// -----------------------------------------------------------------------

void ImplSalPostDispatchMsg( MSG* pMsg, LRESULT /* nDispatchResult */ )
{
    // Used for Unicode and none Unicode
    SalData*        pSalData = GetSalData();
    WinSalFrame*    pFrame;

    if ( (pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP) )
    {
        if ( pSalData->mnSalObjWantKeyEvt == pMsg->wParam )
        {
            pSalData->mnSalObjWantKeyEvt = 0;
            if ( pMsg->hwnd == ::GetFocus() )
            {
                ImplSalYieldMutexAcquireWithWait();
                pFrame = ImplFindSalObjectFrame( pMsg->hwnd );
                if ( pFrame )
                    ImplHandleSalObjKeyMsg( pFrame->mhWnd, pMsg->message, pMsg->wParam, pMsg->lParam );
                ImplSalYieldMutexRelease();
            }
        }
    }

    pSalData->mnSalObjWantKeyEvt = 0;
}

// =======================================================================

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
                if ( pSysObj )
                    pSysObj->CallCallback( SALOBJ_EVENT_TOTOP, 0 );
                ImplSalYieldMutexRelease();
            }
            }
            break;

        case WM_MOUSEACTIVATE:
            ImplPostMessage( hWnd, SALOBJ_MSG_TOTOP, 0, 0 );
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
                ImplPostMessage( hWnd, SALOBJ_MSG_TOTOP, 0, 0 );
            break;

        case SALOBJ_MSG_POSTFOCUS:
            if ( ImplSalYieldMutexTryToAcquire() )
            {
                pSysObj = GetSalObjWindowPtr( hWnd );
                HWND    hFocusWnd = ::GetFocus();
                USHORT nEvent;
                if ( hFocusWnd && ImplIsSysWindowOrChild( hWnd, hFocusWnd ) )
                    nEvent = SALOBJ_EVENT_GETFOCUS;
                else
                    nEvent = SALOBJ_EVENT_LOSEFOCUS;
                pSysObj->CallCallback( nEvent, 0 );
                ImplSalYieldMutexRelease();
            }
            else
                ImplPostMessage( hWnd, SALOBJ_MSG_POSTFOCUS, 0, 0 );
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
            // HWND schon hier setzen, da schon auf den Instanzdaten
            // gearbeitet werden kann, wenn Messages waehrend
            // CreateWindow() gesendet werden
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

LRESULT CALLBACK SalSysObjWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = SalSysObjWndProc( hWnd, nMsg, wParam, lParam, bDef );
    if ( bDef )
        nRet = DefWindowProcW( hWnd, nMsg, wParam, lParam );
    return nRet;
}

// -----------------------------------------------------------------------

LRESULT CALLBACK SalSysObjChildWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, int& rDef )
{
    LRESULT nRet = 0;

    switch( nMsg )
    {
        // Wegen PlugIn's loeschen wir erstmal den Hintergrund
/*
        case WM_ERASEBKGND:
            nRet = 1;
            rDef = FALSE;
            break;
*/
        case WM_PAINT:
            {
            PAINTSTRUCT aPs;
            BeginPaint( hWnd, &aPs );
            EndPaint( hWnd, &aPs );
            rDef = FALSE;
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

LRESULT CALLBACK SalSysObjChildWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = SalSysObjChildWndProc( hWnd, nMsg, wParam, lParam, bDef );
    if ( bDef )
        nRet = DefWindowProcW( hWnd, nMsg, wParam, lParam );
    return nRet;
}

// =======================================================================

SalObject* ImplSalCreateObject( WinSalInstance* pInst, WinSalFrame* pParent )
{
    SalData* pSalData = GetSalData();

    // Hook installieren, wenn es das erste SalObject ist
    if ( !pSalData->mpFirstObject )
    {
        if ( aSalShlData.mbWNT )
        {
            pSalData->mhSalObjMsgHook = SetWindowsHookExW( WH_CALLWNDPROC,
                                                           SalSysMsgProc,
                                                           pSalData->mhInst,
                                                           pSalData->mnAppThreadId );
        }
        else
        {
            pSalData->mhSalObjMsgHook = SetWindowsHookExA( WH_CALLWNDPROC,
                                                           SalSysMsgProc,
                                                           pSalData->mhInst,
                                                           pSalData->mnAppThreadId );
        }
    }

    if ( !pSalData->mbObjClassInit )
    {
        // #95301# shockwave plugin has bug; expects ASCII functions to be used
        if ( false )//aSalShlData.mbWNT )
        {
            WNDCLASSEXW aWndClassEx;
            aWndClassEx.cbSize          = sizeof( aWndClassEx );
            aWndClassEx.style           = 0;
            aWndClassEx.lpfnWndProc     = SalSysObjWndProcW;
            aWndClassEx.cbClsExtra      = 0;
            aWndClassEx.cbWndExtra      = SAL_OBJECT_WNDEXTRA;
            aWndClassEx.hInstance       = pSalData->mhInst;
            aWndClassEx.hIcon           = 0;
            aWndClassEx.hIconSm         = 0;
            aWndClassEx.hCursor         = LoadCursor( 0, IDC_ARROW );
            aWndClassEx.hbrBackground   = 0;
            aWndClassEx.lpszMenuName    = 0;
            aWndClassEx.lpszClassName   = SAL_OBJECT_CLASSNAMEW;
            if ( RegisterClassExW( &aWndClassEx ) )
            {
                // Wegen PlugIn's loeschen wir erstmal den Hintergrund
                aWndClassEx.cbWndExtra      = 0;
                aWndClassEx.hbrBackground   = (HBRUSH)(COLOR_WINDOW+1);
                aWndClassEx.lpfnWndProc     = SalSysObjChildWndProcW;
                aWndClassEx.lpszClassName   = SAL_OBJECT_CHILDCLASSNAMEW;
                if ( RegisterClassExW( &aWndClassEx ) )
                    pSalData->mbObjClassInit = TRUE;
            }
        }
        else
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
                // Wegen PlugIn's loeschen wir erstmal den Hintergrund
                aWndClassEx.cbWndExtra      = 0;
                aWndClassEx.hbrBackground   = (HBRUSH)(COLOR_WINDOW+1);
                aWndClassEx.lpfnWndProc     = SalSysObjChildWndProcA;
                aWndClassEx.lpszClassName   = SAL_OBJECT_CHILDCLASSNAMEA;
                if ( RegisterClassExA( &aWndClassEx ) )
                    pSalData->mbObjClassInit = TRUE;
            }
        }
    }

    if ( pSalData->mbObjClassInit )
    {
        WinSalObject* pObject = new WinSalObject;
        HWND        hWnd;
        HWND        hWndChild = 0;
        // #95301# shockwave plugin has bug; expects ASCII functions to be used
        if ( false )//aSalShlData.mbWNT )
        {
            hWnd = CreateWindowExW( 0, SAL_OBJECT_CLASSNAMEW, L"",
                                    WS_CHILD, 0, 0, 0, 0,
                                    pParent->mhWnd, 0,
                                    pInst->mhInst, (void*)pObject );
            if ( hWnd )
            {
                hWndChild = CreateWindowExW( 0, SAL_OBJECT_CHILDCLASSNAMEW, L"",
                                             WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                                             0, 0, 0, 0,
                                             hWnd, 0,
                                             pInst->mhInst, NULL );
            }
        }
        else
        {
            hWnd = CreateWindowExA( 0, SAL_OBJECT_CLASSNAMEA, "",
                                    WS_CHILD, 0, 0, 0, 0,
                                    pParent->mhWnd, 0,
                                    pInst->mhInst, (void*)pObject );
            if ( hWnd )
            {
                hWndChild = CreateWindowExA( 0, SAL_OBJECT_CHILDCLASSNAMEA, "",
                                             WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                                             0, 0, 0, 0,
                                             hWnd, 0,
                                             pInst->mhInst, NULL );
            }
        }
        if ( !hWndChild )
        {
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

// =======================================================================

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

// -----------------------------------------------------------------------

WinSalObject::~WinSalObject()
{
    SalData* pSalData = GetSalData();

    // remove frame from framelist
    if ( this == pSalData->mpFirstObject )
    {
        pSalData->mpFirstObject = mpNextObject;

        // Wenn letztes SalObject, dann Hook wieder entfernen
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
    if ( mpStdClipRgnData )
        delete mpStdClipRgnData;

    HWND hWndParent = ::GetParent( mhWnd );

    if ( mhWndChild )
        DestroyWindow( mhWndChild );
    if ( mhWnd )
        DestroyWindow( mhWnd );

    // Palette wieder zuruecksetzen, wenn kein externes Child-Fenster
    // mehr vorhanden ist, da diese unsere Palette ueberschrieben haben
    // koennen
    if ( hWndParent &&
         ::GetActiveWindow() == hWndParent &&
         !GetWindow( hWndParent, GW_CHILD ) )
        ImplSendMessage( hWndParent, SAL_MSG_FORCEPALETTE, 0, 0 );
}

// -----------------------------------------------------------------------

void WinSalObject::ResetClipRegion()
{
    SetWindowRgn( mhWnd, 0, TRUE );
}

// -----------------------------------------------------------------------

USHORT WinSalObject::GetClipRegionType()
{
    return SAL_OBJECT_CLIP_INCLUDERECTS;
}

// -----------------------------------------------------------------------

void WinSalObject::BeginSetClipRegion( ULONG nRectCount )
{
    ULONG nRectBufSize = sizeof(RECT)*nRectCount;
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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void WinSalObject::EndSetClipRegion()
{
    HRGN hRegion;

    // Aus den Region-Daten muessen wir jetzt eine ClipRegion erzeugen
    if ( mpClipRgnData->rdh.nCount == 1 )
    {
        RECT* pRect = &(mpClipRgnData->rdh.rcBound);
        hRegion = CreateRectRgn( pRect->left, pRect->top,
                                 pRect->right, pRect->bottom );
    }
    else
    {
        ULONG nSize = mpClipRgnData->rdh.nRgnSize+sizeof(RGNDATAHEADER);
        hRegion = ExtCreateRegion( NULL, nSize, mpClipRgnData );
        if ( mpClipRgnData != mpStdClipRgnData )
            delete mpClipRgnData;
    }

    DBG_ASSERT( hRegion, "SalObject::EndSetClipRegion() - Can't create ClipRegion" );
    SetWindowRgn( mhWnd, hRegion, TRUE );
}

// -----------------------------------------------------------------------

void WinSalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
    ULONG nStyle = 0;
    BOOL bVisible = (GetWindowStyle( mhWnd ) & WS_VISIBLE) != 0;
    if ( bVisible )
    {
        ShowWindow( mhWnd, SW_HIDE );
        nStyle |= SWP_SHOWWINDOW;
    }
    SetWindowPos( mhWnd, 0,
                  (int)nX, (int)nY, (int)nWidth, (int)nHeight,
                  SWP_NOZORDER | SWP_NOACTIVATE | nStyle );
}

// -----------------------------------------------------------------------

void WinSalObject::Show( BOOL bVisible )
{
    if ( bVisible )
        ShowWindow( mhWnd, SW_SHOWNORMAL );
    else
        ShowWindow( mhWnd, SW_HIDE );
}

// -----------------------------------------------------------------------

void WinSalObject::Enable( BOOL bEnable )
{
    EnableWindow( mhWnd, bEnable );
}

// -----------------------------------------------------------------------

void WinSalObject::GrabFocus()
{
    if ( mhLastFocusWnd &&
         IsWindow( mhLastFocusWnd ) &&
         ImplIsSysWindowOrChild( mhWndChild, mhLastFocusWnd ) )
        ::SetFocus( mhLastFocusWnd );
    else
        ::SetFocus( mhWndChild );
}

// -----------------------------------------------------------------------

void WinSalObject::SetBackground()
{
}

// -----------------------------------------------------------------------

void WinSalObject::SetBackground( SalColor nSalColor )
{
}

// -----------------------------------------------------------------------

const SystemEnvData* WinSalObject::GetSystemData() const
{
    return &maSysData;
}
