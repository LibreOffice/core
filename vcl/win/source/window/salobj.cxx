/*************************************************************************
 *
 *  $RCSfile: salobj.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:50 $
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
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALOBJ_HXX
#include <salobj.hxx>
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

SalObject* ImplFindSalObject( HWND hWndChild )
{
    SalData*    pSalData = GetSalData();
    SalObject*  pObject = pSalData->mpFirstObject;
    while ( pObject )
    {
        if ( ImplIsSysWindowOrChild( pObject->maObjectData.mhWndChild, hWndChild ) )
            return pObject;

        pObject = pObject->maObjectData.mpNextObject;
    }

    return NULL;
}

// -----------------------------------------------------------------------

SalFrame* ImplFindSalObjectFrame( HWND hWnd )
{
    SalFrame* pFrame = NULL;
    SalObject* pObject = ImplFindSalObject( hWnd );
    if ( pObject )
    {
        // Dazugehoerenden Frame suchen
        HWND hWnd = ::GetParent( pObject->maObjectData.mhWnd );
        pFrame = GetSalData()->mpFirstFrame;
        while ( pFrame )
        {
            if ( pFrame->maFrameData.mhWnd == hWnd )
                break;

            pFrame = pFrame->maFrameData.mpNextFrame;
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
        SalObject* pObject;
        if ( pData->message == WM_SETFOCUS )
        {
            pObject = ImplFindSalObject( pData->hwnd );
            if ( pObject )
            {
                pObject->maObjectData.mhLastFocusWnd = pData->hwnd;
                if ( ImplSalYieldMutexTryToAcquire() )
                {
                    pObject->maObjectData.mpProc( pObject->maObjectData.mpInst, pObject,
                                                  SALOBJ_EVENT_GETFOCUS, 0 );
                    ImplSalYieldMutexRelease();
                }
                else
                    ImplPostMessage( pObject->maObjectData.mhWnd, SALOBJ_MSG_POSTFOCUS, 0, 0 );
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
                        pObject->maObjectData.mpProc( pObject->maObjectData.mpInst, pObject,
                                                      SALOBJ_EVENT_LOSEFOCUS, 0 );
                        ImplSalYieldMutexRelease();
                    }
                    else
                        ImplPostMessage( pObject->maObjectData.mhWnd, SALOBJ_MSG_POSTFOCUS, 0, 0 );
                }
                else
                    pObject->maObjectData.mhLastFocusWnd = (HWND)pData->wParam;
            }
        }
    }

    return CallNextHookEx( pSalData->mhSalObjMsgHook, nCode, wParam, lParam );
}

// -----------------------------------------------------------------------

BOOL ImplSalPreDispatchMsg( MSG* pMsg )
{
    // Used for Unicode and none Unicode
    SalData*    pSalData = GetSalData();
    SalObject*  pObject;

    if ( (pMsg->message == WM_LBUTTONDOWN) ||
         (pMsg->message == WM_RBUTTONDOWN) ||
         (pMsg->message == WM_MBUTTONDOWN) )
    {
        ImplSalYieldMutexAcquireWithWait();
        pObject = ImplFindSalObject( pMsg->hwnd );
        if ( pObject )
            ImplPostMessage( pObject->maObjectData.mhWnd, SALOBJ_MSG_TOTOP, 0, 0 );
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
                    SalFrame* pFrame = ImplFindSalObjectFrame( pMsg->hwnd );
                    if ( pFrame )
                    {
                        if ( ImplHandleSalObjSysCharMsg( pFrame->maFrameData.mhWnd, pMsg->wParam, pMsg->lParam ) )
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
    SalData*    pSalData = GetSalData();
    SalFrame*   pFrame;

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
                    ImplHandleSalObjKeyMsg( pFrame->maFrameData.mhWnd, pMsg->message, pMsg->wParam, pMsg->lParam );
                ImplSalYieldMutexRelease();
            }
        }
    }

    pSalData->mnSalObjWantKeyEvt = 0;
}

// =======================================================================

LRESULT CALLBACK SalSysObjWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, int& rDef )
{
    SalObject*  pSysObj;
    LRESULT     nRet = 0;

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
                    pSysObj->maObjectData.mpProc( pSysObj->maObjectData.mpInst, pSysObj, SALOBJ_EVENT_TOTOP, 0 );
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
                pSysObj->maObjectData.mpProc( pSysObj->maObjectData.mpInst, pSysObj,
                                              SALOBJ_EVENT_TOTOP, 0 );
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
                pSysObj->maObjectData.mpProc( pSysObj->maObjectData.mpInst, pSysObj,
                                              nEvent, 0 );
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
            pSysObj = (SalObject*)pStruct->lpCreateParams;
            SetSalObjWindowPtr( hWnd, pSysObj );
            // HWND schon hier setzen, da schon auf den Instanzdaten
            // gearbeitet werden kann, wenn Messages waehrend
            // CreateWindow() gesendet werden
            pSysObj->maObjectData.mhWnd = hWnd;
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

SalObject* ImplSalCreateObject( SalInstance* pInst, SalFrame* pParent )
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
        if ( aSalShlData.mbWNT )
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
        SalObject* pObject = new SalObject;
        HWND        hWnd;
        HWND        hWndChild = 0;
        if ( aSalShlData.mbWNT )
        {
            hWnd = CreateWindowExW( 0, SAL_OBJECT_CLASSNAMEW, L"",
                                    WS_CHILD, 0, 0, 0, 0,
                                    pParent->maFrameData.mhWnd, 0,
                                    pInst->maInstData.mhInst, (void*)pObject );
            if ( hWnd )
            {
                hWndChild = CreateWindowExW( 0, SAL_OBJECT_CHILDCLASSNAMEW, L"",
                                             WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                                             0, 0, 0, 0,
                                             hWnd, 0,
                                             pInst->maInstData.mhInst, NULL );
            }
        }
        else
        {
            hWnd = CreateWindowExA( 0, SAL_OBJECT_CLASSNAMEA, "",
                                    WS_CHILD, 0, 0, 0, 0,
                                    pParent->maFrameData.mhWnd, 0,
                                    pInst->maInstData.mhInst, (void*)pObject );
            if ( hWnd )
            {
                hWndChild = CreateWindowExA( 0, SAL_OBJECT_CHILDCLASSNAMEA, "",
                                             WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                                             0, 0, 0, 0,
                                             hWnd, 0,
                                             pInst->maInstData.mhInst, NULL );
            }
        }
        if ( !hWndChild )
        {
            delete pObject;
            return NULL;
        }

        if ( hWnd )
        {
            pObject->maObjectData.mhWnd             = hWnd;
            pObject->maObjectData.mhWndChild        = hWndChild;
            pObject->maObjectData.maSysData.hWnd    = hWndChild;
            return pObject;
        }
    }

    return NULL;
}

// =======================================================================

long ImplSalObjCallbackDummy( void*, SalObject*, USHORT, const void* )
{
    return 0;
}

// =======================================================================

SalObject::SalObject()
{
    SalData* pSalData = GetSalData();

    maObjectData.mhWnd              = 0;
    maObjectData.mhWndChild         = 0;
    maObjectData.mhLastFocusWnd     = 0;
    maObjectData.maSysData.nSize    = sizeof( SystemEnvData );
    maObjectData.mpInst             = NULL;
    maObjectData.mpProc             = ImplSalObjCallbackDummy;
    maObjectData.mpStdClipRgnData   = NULL;

    // Insert object in objectlist
    maObjectData.mpNextObject = pSalData->mpFirstObject;
    pSalData->mpFirstObject = this;
}

// -----------------------------------------------------------------------

SalObject::~SalObject()
{
    SalData* pSalData = GetSalData();

    // remove frame from framelist
    if ( this == pSalData->mpFirstObject )
    {
        pSalData->mpFirstObject = maObjectData.mpNextObject;

        // Wenn letztes SalObject, dann Hook wieder entfernen
        if ( !pSalData->mpFirstObject )
            UnhookWindowsHookEx( pSalData->mhSalObjMsgHook );
    }
    else
    {
        SalObject* pTempObject = pSalData->mpFirstObject;
        while ( pTempObject->maObjectData.mpNextObject != this )
            pTempObject = pTempObject->maObjectData.mpNextObject;

        pTempObject->maObjectData.mpNextObject = maObjectData.mpNextObject;
    }

    // Cache-Daten zerstoeren
    if ( maObjectData.mpStdClipRgnData )
        delete maObjectData.mpStdClipRgnData;

    HWND hWndParent = ::GetParent( maObjectData.mhWnd );

    if ( maObjectData.mhWndChild )
        DestroyWindow( maObjectData.mhWndChild );
    if ( maObjectData.mhWnd )
        DestroyWindow( maObjectData.mhWnd );

    // Palette wieder zuruecksetzen, wenn kein externes Child-Fenster
    // mehr vorhanden ist, da diese unsere Palette ueberschrieben haben
    // koennen
    if ( hWndParent &&
         ::GetActiveWindow() == hWndParent &&
         !GetWindow( hWndParent, GW_CHILD ) )
        ImplSendMessage( hWndParent, SAL_MSG_FORCEPALETTE, 0, 0 );
}

// -----------------------------------------------------------------------

void SalObject::ResetClipRegion()
{
    SetWindowRgn( maObjectData.mhWnd, 0, TRUE );
}

// -----------------------------------------------------------------------

USHORT SalObject::GetClipRegionType()
{
    return SAL_OBJECT_CLIP_INCLUDERECTS;
}

// -----------------------------------------------------------------------

void SalObject::BeginSetClipRegion( ULONG nRectCount )
{
    ULONG nRectBufSize = sizeof(RECT)*nRectCount;
    if ( nRectCount < SAL_CLIPRECT_COUNT )
    {
        if ( !maObjectData.mpStdClipRgnData )
            maObjectData.mpStdClipRgnData = (RGNDATA*)new BYTE[sizeof(RGNDATA)-1+(SAL_CLIPRECT_COUNT*sizeof(RECT))];
        maObjectData.mpClipRgnData = maObjectData.mpStdClipRgnData;
    }
    else
        maObjectData.mpClipRgnData = (RGNDATA*)new BYTE[sizeof(RGNDATA)-1+nRectBufSize];
    maObjectData.mpClipRgnData->rdh.dwSize    = sizeof( RGNDATAHEADER );
    maObjectData.mpClipRgnData->rdh.iType     = RDH_RECTANGLES;
    maObjectData.mpClipRgnData->rdh.nCount    = nRectCount;
    maObjectData.mpClipRgnData->rdh.nRgnSize  = nRectBufSize;
    SetRectEmpty( &(maObjectData.mpClipRgnData->rdh.rcBound) );
    maObjectData.mpNextClipRect           = (RECT*)(&(maObjectData.mpClipRgnData->Buffer));
    maObjectData.mbFirstClipRect          = TRUE;
}

// -----------------------------------------------------------------------

void SalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    RECT*       pRect = maObjectData.mpNextClipRect;
    RECT*       pBoundRect = &(maObjectData.mpClipRgnData->rdh.rcBound);
    long        nRight = nX + nWidth;
    long        nBottom = nY + nHeight;

    if ( maObjectData.mbFirstClipRect )
    {
        pBoundRect->left    = nX;
        pBoundRect->top     = nY;
        pBoundRect->right   = nRight;
        pBoundRect->bottom  = nBottom;
        maObjectData.mbFirstClipRect = FALSE;
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
    maObjectData.mpNextClipRect++;
}

// -----------------------------------------------------------------------

void SalObject::EndSetClipRegion()
{
    HRGN hRegion;

    // Aus den Region-Daten muessen wir jetzt eine ClipRegion erzeugen
    if ( maObjectData.mpClipRgnData->rdh.nCount == 1 )
    {
        RECT* pRect = &(maObjectData.mpClipRgnData->rdh.rcBound);
        hRegion = CreateRectRgn( pRect->left, pRect->top,
                                 pRect->right, pRect->bottom );
    }
    else
    {
        ULONG nSize = maObjectData.mpClipRgnData->rdh.nRgnSize+sizeof(RGNDATAHEADER);
        hRegion = ExtCreateRegion( NULL, nSize, maObjectData.mpClipRgnData );
        if ( maObjectData.mpClipRgnData != maObjectData.mpStdClipRgnData )
            delete maObjectData.mpClipRgnData;
    }

    DBG_ASSERT( hRegion, "SalObject::EndSetClipRegion() - Can't create ClipRegion" );
    SetWindowRgn( maObjectData.mhWnd, hRegion, TRUE );
}

// -----------------------------------------------------------------------

void SalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
    ULONG nStyle = 0;
    BOOL bVisible = (GetWindowStyle( maObjectData.mhWnd ) & WS_VISIBLE) != 0;
    if ( bVisible )
    {
        ShowWindow( maObjectData.mhWnd, SW_HIDE );
        nStyle |= SWP_SHOWWINDOW;
    }
    SetWindowPos( maObjectData.mhWnd, 0,
                  (int)nX, (int)nY, (int)nWidth, (int)nHeight,
                  SWP_NOZORDER | SWP_NOACTIVATE | nStyle );
}

// -----------------------------------------------------------------------

void SalObject::Show( BOOL bVisible )
{
    if ( bVisible )
        ShowWindow( maObjectData.mhWnd, SW_SHOWNORMAL );
    else
        ShowWindow( maObjectData.mhWnd, SW_HIDE );
}

// -----------------------------------------------------------------------

void SalObject::Enable( BOOL bEnable )
{
    EnableWindow( maObjectData.mhWnd, bEnable );
}

// -----------------------------------------------------------------------

void SalObject::GrabFocus()
{
    if ( maObjectData.mhLastFocusWnd &&
         IsWindow( maObjectData.mhLastFocusWnd ) &&
         ImplIsSysWindowOrChild( maObjectData.mhWndChild, maObjectData.mhLastFocusWnd ) )
        ::SetFocus( maObjectData.mhLastFocusWnd );
    else
        ::SetFocus( maObjectData.mhWndChild );
}

// -----------------------------------------------------------------------

void SalObject::SetBackground()
{
}

// -----------------------------------------------------------------------

void SalObject::SetBackground( SalColor nSalColor )
{
}

// -----------------------------------------------------------------------

const SystemEnvData* SalObject::GetSystemData() const
{
    return &maObjectData.maSysData;
}

// -----------------------------------------------------------------------

void SalObject::SetCallback( void* pInst, SALOBJECTPROC pProc )
{
    maObjectData.mpInst = pInst;
    if ( pProc )
        maObjectData.mpProc = pProc;
    else
        maObjectData.mpProc = ImplSalObjCallbackDummy;
}
