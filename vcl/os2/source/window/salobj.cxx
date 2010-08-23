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

#include <svpm.h>

#define _SV_SALOBJ_CXX
#include <saldata.hxx>
#include <salinst.h>
#include <salframe.h>
#include <salobj.h>

// =======================================================================

static BOOL ImplIsSysWindowOrChild( HWND hWndParent, HWND hWndChild )
{
    if ( hWndParent == hWndChild )
        return TRUE;

    HWND hTempWnd = WinQueryWindow( hWndChild, QW_PARENT );
    while ( hTempWnd )
    {
        if ( hTempWnd == hWndParent )
            return TRUE;
        hTempWnd = WinQueryWindow( hTempWnd, QW_PARENT );
    }

    return FALSE;
}

// -----------------------------------------------------------------------

static Os2SalObject* ImplFindOs2SalObject( HWND hWndChild )
{
    SalData*    pSalData = GetSalData();
    Os2SalObject*   pObject = pSalData->mpFirstObject;
    while ( pObject )
    {
        if ( ImplIsSysWindowOrChild( pObject->mhWndChild, hWndChild ) )
            return pObject;

        pObject = pObject->mpNextObject;
    }

    return NULL;
}

// =======================================================================

BOOL EXPENTRY SalSysMsgProc( HAB /* hAB */, QMSG* pMsg, ULONG /* fs */ )
{
    if ( (pMsg->msg == WM_BUTTON1DOWN) ||
         (pMsg->msg == WM_BUTTON2DOWN) ||
         (pMsg->msg == WM_BUTTON3DOWN) )
    {
        SalData*    pSalData = GetSalData();
        Os2SalObject*   pObject = ImplFindOs2SalObject( pMsg->hwnd );
        if ( pObject )
            WinPostMsg( pObject->mhWnd, SALOBJ_MSG_TOTOP, 0, 0 );
    }

    // Focus fangen wir hier nicht ab, da wir erstmal davon ausgehen,
    // das unser Os2SalObject-Fenster immer eine WM_FOCUSCHANGE-Message
    // bekommt.

    return FALSE;
}

// -----------------------------------------------------------------------

MRESULT EXPENTRY SalSysObjWndProc( HWND hWnd, ULONG nMsg,
                                   MPARAM nMP1, MPARAM nMP2 )
{
    Os2SalObject*   pSysObj;
    MRESULT     nRet = 0;
    int         bDef = TRUE;

#if OSL_DEBUG_LEVEL>0
    debug_printf( "SalSysObjWndProc hWnd 0x%x nMsg %d\n", hWnd, nMsg);
#endif

    switch( nMsg )
    {
        case WM_ERASEBACKGROUND:
            nRet = (MRESULT)FALSE;
            bDef = FALSE;
            break;
        case WM_PAINT:
            {
            HPS     hPS;
            RECTL   aRect;
            hPS = WinBeginPaint( hWnd, NULLHANDLE, &aRect );
            WinEndPaint( hPS );
            bDef = FALSE;
            }
            bDef = FALSE;
            break;

        case WM_BUTTON1DOWN:
        case WM_BUTTON2DOWN:
        case WM_BUTTON3DOWN:
        case SALOBJ_MSG_TOTOP:
            if ( ImplSalYieldMutexTryToAcquire() )
            {
                pSysObj = GetSalObjWindowPtr( hWnd );
                pSysObj->mpProc( pSysObj->mpInst, pSysObj,
                                              SALOBJ_EVENT_TOTOP, 0 );
                ImplSalYieldMutexRelease();
            }
            else
                WinPostMsg( hWnd, SALOBJ_MSG_TOTOP, 0, 0 );
            break;

        case WM_FOCUSCHANGE:
        case SALOBJ_MSG_POSTFOCUS:
            if ( ImplSalYieldMutexTryToAcquire() )
            {
                pSysObj = GetSalObjWindowPtr( hWnd );
                if ( SHORT1FROMMP( nMP2 ) )
                {
                    pSysObj->mhLastFocusWnd = WinQueryFocus( HWND_DESKTOP );
                    pSysObj->mpProc( pSysObj->mpInst, pSysObj,
                                                  SALOBJ_EVENT_GETFOCUS, 0 );
                }
                else
                {
                    HWND hWndFocus = HWNDFROMMP( nMP1 );
                    if ( !hWndFocus || !ImplIsSysWindowOrChild( hWnd, hWndFocus ) )
                    {
                        pSysObj->mpProc( pSysObj->mpInst, pSysObj,
                                                      SALOBJ_EVENT_LOSEFOCUS, 0 );
                    }
                }
                ImplSalYieldMutexRelease();
            }
            else
                WinPostMsg( hWnd, SALOBJ_MSG_POSTFOCUS, nMP1, nMP2 );
            break;

        case WM_SIZE:
            {
            pSysObj = GetSalObjWindowPtr( hWnd );
            pSysObj->mnHeight = (short)SHORT2FROMMP( nMP2 );
            WinSetWindowPos( pSysObj->mhWndChild, 0,
                             0, 0,
                             (short)SHORT1FROMMP( nMP2 ), (short)SHORT2FROMMP( nMP2 ),
                             SWP_SIZE | SWP_MOVE );
            bDef = FALSE;
            }
            break;

        case WM_CREATE:
            {
            // Window-Instanz am Windowhandle speichern
            CREATESTRUCT* pStruct = (CREATESTRUCT*)nMP2;
            pSysObj = (Os2SalObject*)pStruct->pPresParams;
            SetSalObjWindowPtr( hWnd, pSysObj );
            bDef = FALSE;
            }
            break;
    }

    if ( bDef )
        nRet = WinDefWindowProc( hWnd, nMsg, nMP1, nMP2 );
    return nRet;
}

// -----------------------------------------------------------------------

MRESULT EXPENTRY SalSysObjChildWndProc( HWND hWnd, ULONG nMsg,
                                        MPARAM nMP1, MPARAM nMP2 )
{
    MRESULT     nRet = 0;
    int         bDef = TRUE;

    debug_printf( "SalSysObjChildWndProc hWnd 0x%x nMsg %d\n", hWnd, nMsg);

    switch( nMsg )
    {
        case WM_ERASEBACKGROUND:
            // Wegen PlugIn's loeschen wir erstmal den Hintergrund
/*
            nRet = (MRESULT)FALSE;
            bDef = FALSE;
*/
            break;
        case WM_PAINT:
            {
            HPS     hPS;
            RECTL   aRect;
            hPS = WinBeginPaint( hWnd, NULLHANDLE, &aRect );
            WinEndPaint( hPS );
            bDef = FALSE;
            }
            break;
    }

    if ( bDef )
        nRet = WinDefWindowProc( hWnd, nMsg, nMP1, nMP2 );
    return nRet;
}

// -----------------------------------------------------------------------

MRESULT EXPENTRY SalSysObjClipWndProc( HWND hWnd, ULONG nMsg,
                                       MPARAM nMP1, MPARAM nMP2 )
{
    MRESULT     nRet = 0;
    int         bDef = TRUE;

    debug_printf( "SalSysObjClipWndProc hWnd 0x%x nMsg %d\n", hWnd, nMsg);

    switch( nMsg )
    {
        case WM_MOUSEMOVE:
        case WM_BUTTON1DOWN:
        case WM_BUTTON2DOWN:
        case WM_BUTTON3DOWN:
        case WM_BUTTON1DBLCLK:
        case WM_BUTTON2DBLCLK:
        case WM_BUTTON3DBLCLK:
        case WM_BUTTON1UP:
        case WM_BUTTON2UP:
        case WM_BUTTON3UP:
            {
            // Alle Events an den Frame weiterreichen, da diese Bereiche
            // dem Frame gehoeren. Dazu muessen die Mouse-Koordinaaten
            // entsprechend umgerechnet werden
            HWND hWndParent = WinQueryWindow( hWnd, QW_PARENT ); // ergibt SysChild-Fenster
            hWndParent = WinQueryWindow( hWndParent, QW_PARENT );
            short nX = (short)SHORT1FROMMP( nMP1 );
            short nY = (short)SHORT2FROMMP( nMP1 );
            POINTL aPos;
            aPos.x = nX;
            aPos.y = nY;
            WinMapWindowPoints( hWnd, hWndParent, &aPos, 1 );
            nMP1 = MPFROM2SHORT( (short)aPos.x, (short)aPos.y );
            bDef = FALSE;
            nRet = WinSendMsg( hWndParent, nMsg, nMP1, nMP2 );
            }
            break;

        case WM_HITTEST:
            // Damit im disablten Zustand die MouseKlicks immer noch
            // an den Frame geschickt werden
            // Dieser Code reicht leider nicht aus, deshalb wir unter
            // OS2 immer das Child-Fenster disablen, im Gegensatz
            // zu Windows, wo immer der Parent disablte wird, da
            // sich das Fenster evtl. anders Darstellen koennte,
            // wenn es disablte wird. Da dieser Fall uns bisher
            // nicht bekannt ist, ignorieren wir das.
            nRet = HT_NORMAL;
            bDef = FALSE;
            break;

        case WM_ERASEBACKGROUND:
            nRet = (MRESULT)FALSE;
            bDef = FALSE;
            break;
        case WM_PAINT:
            {
            HPS     hPS;
            RECTL   aRect;
            hPS = WinBeginPaint( hWnd, NULLHANDLE, &aRect );
            WinEndPaint( hPS );
            bDef = FALSE;
            }
            break;
    }

    if ( bDef )
        nRet = WinDefWindowProc( hWnd, nMsg, nMP1, nMP2 );
    return nRet;
}

// =======================================================================

void ImplDestroyAllClipWindows( HWND hWndLast )
{
    if ( hWndLast == HWND_TOP )
        return;

    HWND hWndPrev;
    while ( hWndLast )
    {
        hWndPrev = WinQueryWindow( hWndLast, QW_PREV );
        WinDestroyWindow( hWndLast );
        hWndLast = hWndPrev;
    }
}

// =======================================================================

SalObject* ImplSalCreateObject( Os2SalInstance* pInst, Os2SalFrame* pParent )
{
    SalData* pSalData = GetSalData();

    if ( !pSalData->mbObjClassInit )
    {
        if ( WinRegisterClass( pSalData->mhAB, (PSZ)SAL_OBJECT_CLASSNAME,
                               (PFNWP)SalSysObjWndProc, CS_MOVENOTIFY,
                               SAL_OBJECT_WNDEXTRA ) )
        {
            if ( WinRegisterClass( pSalData->mhAB, (PSZ)SAL_OBJECT_CLIPCLASSNAME,
                                   (PFNWP)SalSysObjClipWndProc, CS_HITTEST | CS_MOVENOTIFY, 0 ) )
            {
                if ( WinRegisterClass( pSalData->mhAB, (PSZ)SAL_OBJECT_CHILDCLASSNAME,
                                       (PFNWP)SalSysObjChildWndProc, CS_HITTEST | CS_MOVENOTIFY, 32 ) )
                    pSalData->mbObjClassInit = TRUE;
            }
        }
    }

    if ( pSalData->mbObjClassInit )
    {
        Os2SalObject* pObject = new Os2SalObject;
        HWND hWnd = WinCreateWindow( pParent->mhWndClient, SAL_OBJECT_CLASSNAME, "",
                                     0,
                                     0, 0, 0, 0,
                                     pParent->mhWndClient, HWND_TOP,
                                     0, NULL, (void*)pObject );
        HWND hWndChild = WinCreateWindow( hWnd, SAL_OBJECT_CHILDCLASSNAME, "",
                                          WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                                          0, 0, 0, 0,
                                          hWnd, HWND_TOP,
                                          0, NULL, NULL );

        if ( !hWndChild )
        {
            if ( hWnd )
                WinDestroyWindow( hWnd );
            delete pObject;
            return NULL;
        }

        if ( hWnd )
        {
        debug_printf("ImplSalCreateObject hWndChild %x\n", hWndChild);
        debug_printf("ImplSalCreateObject hWnd %x\n", hWnd);
            pObject->mhWnd = hWnd;
            pObject->mhWndChild = hWndChild;
            pObject->maSysData.hWnd = hWndChild;
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

Os2SalObject::Os2SalObject()
{
    SalData* pSalData = GetSalData();

    mhLastClipWnd       = HWND_TOP;

    mhWnd               = 0;
    mhWndChild      = 0;
    mhLastFocusWnd  = 0;
    maSysData.nSize = sizeof( SystemEnvData );
    mnHeight            = 0;
    mpInst          = NULL;
    mpProc          = ImplSalObjCallbackDummy;

    // Hook installieren, wenn es das erste Os2SalObject ist
    if ( !pSalData->mpFirstObject )
    {
        WinSetHook( pSalData->mhAB, pSalData->mhMQ,
                    HK_INPUT, (PFN)SalSysMsgProc, (HMODULE)0 );
    }

    // Insert object in objectlist
    mpNextObject = pSalData->mpFirstObject;
    pSalData->mpFirstObject = this;
}

// -----------------------------------------------------------------------

Os2SalObject::~Os2SalObject()
{
    SalData* pSalData = GetSalData();

    // remove frame from framelist
    if ( this == pSalData->mpFirstObject )
    {
        pSalData->mpFirstObject = mpNextObject;

        // Wenn letztes Os2SalObject, dann Hook wieder entfernen
        if ( !pSalData->mpFirstObject )
        {
            WinReleaseHook( pSalData->mhAB, pSalData->mhMQ,
                            HK_INPUT, (PFN)SalSysMsgProc, (HMODULE)0 );
        }
    }
    else
    {
        Os2SalObject* pTempObject = pSalData->mpFirstObject;
        while ( pTempObject->mpNextObject != this )
            pTempObject = pTempObject->mpNextObject;

        pTempObject->mpNextObject = mpNextObject;
    }

    // Cache-Daten zerstoeren
    ImplDestroyAllClipWindows( mhLastClipWnd );

    if ( mhWndChild )
        WinDestroyWindow( mhWndChild );
    if ( mhWnd )
        WinDestroyWindow( mhWnd );
}

// -----------------------------------------------------------------------

void Os2SalObject::ResetClipRegion()
{
    ImplDestroyAllClipWindows( mhLastClipWnd );
    mhLastClipWnd = HWND_TOP;
}

// -----------------------------------------------------------------------

USHORT Os2SalObject::GetClipRegionType()
{
    return SAL_OBJECT_CLIP_EXCLUDERECTS;
}

// -----------------------------------------------------------------------

void Os2SalObject::BeginSetClipRegion( ULONG nRectCount )
{
    mhOldLastClipWnd = mhLastClipWnd;
}

// -----------------------------------------------------------------------

void Os2SalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    HWND hClipWnd = WinCreateWindow( mhWnd, SAL_OBJECT_CLIPCLASSNAME, "",
                                     WS_VISIBLE,
                                     nX, mnHeight-(nY+nHeight), nWidth, nHeight,
                                     mhWnd, mhLastClipWnd,
                                     0, NULL, NULL );
        debug_printf("Os2SalObject::UnionClipRegion hClipWnd %x\n", hClipWnd);
    mhLastClipWnd = hClipWnd;
}

// -----------------------------------------------------------------------

void Os2SalObject::EndSetClipRegion()
{
    ImplDestroyAllClipWindows( mhOldLastClipWnd );
}

// -----------------------------------------------------------------------

void Os2SalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
    ULONG  nStyle = 0;
    BOOL   bVisible = WinIsWindowVisible( mhWnd );
    if ( bVisible )
    {
        WinShowWindow( mhWnd, FALSE );
        nStyle |= SWP_SHOW;
    }
    SWP aParentSWP;
    WinQueryWindowPos( WinQueryWindow( mhWnd, QW_PARENT ), &aParentSWP );
    WinSetWindowPos( mhWnd, 0, nX, aParentSWP.cy-(nY+nHeight), nWidth, nHeight,
                     SWP_MOVE | SWP_SIZE | nStyle );
}

// -----------------------------------------------------------------------

void Os2SalObject::Show( BOOL bVisible )
{
    WinShowWindow( mhWnd, bVisible );
}

// -----------------------------------------------------------------------

void Os2SalObject::Enable( BOOL bEnable )
{
    // Im Gegensatz zu Windows disablen wir das Childfenster,
    // da ansonsten unser Clippen nicht mehr funktioniert, da
    // wir keine Events mehr bekommen. Dadurch kann sich evtl.
    // das Fenster anders darstellen, was wir eigentlich nicht
    // wollen. Aber da uns bisher kein Fall bekannt ist,
    // ignorieren wir dies. Ansonsten muss ein Fenster dazwischen
    // gezogen werden oder getestet werden, wie wir die
    // Maustransparenz erreichen, wenn mhWnd
    // disablte wird.
    WinEnableWindow( mhWndChild, bEnable );
}

// -----------------------------------------------------------------------

void Os2SalObject::GrabFocus()
{
    if ( mhLastFocusWnd &&
         WinIsWindow( GetSalData()->mhAB, mhLastFocusWnd ) &&
         ImplIsSysWindowOrChild( mhWndChild, mhLastFocusWnd ) )
        WinSetFocus( HWND_DESKTOP, mhLastFocusWnd );
    else
        WinSetFocus( HWND_DESKTOP, mhWndChild );
}

// -----------------------------------------------------------------------

void Os2SalObject::SetBackground()
{
}

// -----------------------------------------------------------------------

void Os2SalObject::SetBackground( SalColor nSalColor )
{
}

// -----------------------------------------------------------------------

const SystemChildData* Os2SalObject::GetSystemData() const
{
    return &maSysData;
}

// -----------------------------------------------------------------------
#if 0
void Os2SalObject::SetCallback( void* pInst, SALOBJECTPROC pProc )
{
    mpInst = pInst;
    if ( pProc )
        mpProc = pProc;
    else
        mpProc = ImplSalObjCallbackDummy;
}
#endif

// -----------------------------------------------------------------------

void Os2SalObject::InterceptChildWindowKeyDown( sal_Bool /*bIntercept*/ )
{
}

