/*************************************************************************
 *
 *  $RCSfile: salobj.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:35 $
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

#include <tools/svpm.h>

#define _SV_SALOBJ_CXX

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

static SalObject* ImplFindSalObject( HWND hWndChild )
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

// =======================================================================

PM_BOOL EXPENTRY SalSysMsgProc( HAB /* hAB */, QMSG* pMsg, PM_ULONG /* fs */ )
{
    if ( (pMsg->msg == WM_BUTTON1DOWN) ||
         (pMsg->msg == WM_BUTTON2DOWN) ||
         (pMsg->msg == WM_BUTTON3DOWN) )
    {
        SalData*    pSalData = GetSalData();
        SalObject*  pObject = ImplFindSalObject( pMsg->hwnd );
        if ( pObject )
            WinPostMsg( pObject->maObjectData.mhWnd, SALOBJ_MSG_TOTOP, 0, 0 );
    }

    // Focus fangen wir hier nicht ab, da wir erstmal davon ausgehen,
    // das unser SalObject-Fenster immer eine WM_FOCUSCHANGE-Message
    // bekommt.

    return FALSE;
}

// -----------------------------------------------------------------------

MRESULT EXPENTRY SalSysObjWndProc( HWND hWnd, PM_ULONG nMsg,
                                   MPARAM nMP1, MPARAM nMP2 )
{
    SalObject*  pSysObj;
    MRESULT     nRet = 0;
    int         bDef = TRUE;

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
                pSysObj->maObjectData.mpProc( pSysObj->maObjectData.mpInst, pSysObj,
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
                    pSysObj->maObjectData.mhLastFocusWnd = WinQueryFocus( HWND_DESKTOP );
                    pSysObj->maObjectData.mpProc( pSysObj->maObjectData.mpInst, pSysObj,
                                                  SALOBJ_EVENT_GETFOCUS, 0 );
                }
                else
                {
                    HWND hWndFocus = HWNDFROMMP( nMP1 );
                    if ( !hWndFocus || !ImplIsSysWindowOrChild( hWnd, hWndFocus ) )
                    {
                        pSysObj->maObjectData.mpProc( pSysObj->maObjectData.mpInst, pSysObj,
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
            pSysObj->maObjectData.mnHeight = (short)SHORT2FROMMP( nMP2 );
            WinSetWindowPos( pSysObj->maObjectData.mhWndChild, 0,
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
            pSysObj = (SalObject*)pStruct->pPresParams;
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

MRESULT EXPENTRY SalSysObjChildWndProc( HWND hWnd, PM_ULONG nMsg,
                                        MPARAM nMP1, MPARAM nMP2 )
{
    MRESULT     nRet = 0;
    int         bDef = TRUE;

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

MRESULT EXPENTRY SalSysObjClipWndProc( HWND hWnd, PM_ULONG nMsg,
                                       MPARAM nMP1, MPARAM nMP2 )
{
    MRESULT     nRet = 0;
    int         bDef = TRUE;

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

SalObject* ImplSalCreateObject( SalInstance* pInst, SalFrame* pParent )
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
        SalObject* pObject = new SalObject;
        HWND hWnd = WinCreateWindow( pParent->maFrameData.mhWndClient, SAL_OBJECT_CLASSNAME, "",
                                     0,
                                     0, 0, 0, 0,
                                     pParent->maFrameData.mhWndClient, HWND_TOP,
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
            pObject->maObjectData.mhWnd = hWnd;
            pObject->maObjectData.mhWndChild = hWndChild;
            pObject->maObjectData.maSysData.hWnd = hWndChild;
            return pObject;
        }
    }

    return NULL;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyObject( SalObject* pObject )
{
    delete pObject;
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

    maObjectData.mhLastClipWnd      = HWND_TOP;

    maObjectData.mhWnd              = 0;
    maObjectData.mhWndChild         = 0;
    maObjectData.mhLastFocusWnd     = 0;
    maObjectData.maSysData.nSize    = sizeof( SystemEnvData );
    maObjectData.mnHeight           = 0;
    maObjectData.mpInst             = NULL;
    maObjectData.mpProc             = ImplSalObjCallbackDummy;

    // Hook installieren, wenn es das erste SalObject ist
    if ( !pSalData->mpFirstObject )
    {
        WinSetHook( pSalData->mhAB, pSalData->mhMQ,
                    HK_INPUT, (PFN)SalSysMsgProc, (HMODULE)0 );
    }

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
        {
            WinReleaseHook( pSalData->mhAB, pSalData->mhMQ,
                            HK_INPUT, (PFN)SalSysMsgProc, (HMODULE)0 );
        }
    }
    else
    {
        SalObject* pTempObject = pSalData->mpFirstObject;
        while ( pTempObject->maObjectData.mpNextObject != this )
            pTempObject = pTempObject->maObjectData.mpNextObject;

        pTempObject->maObjectData.mpNextObject = maObjectData.mpNextObject;
    }

    // Cache-Daten zerstoeren
    ImplDestroyAllClipWindows( maObjectData.mhLastClipWnd );

    if ( maObjectData.mhWndChild )
        WinDestroyWindow( maObjectData.mhWndChild );
    if ( maObjectData.mhWnd )
        WinDestroyWindow( maObjectData.mhWnd );
}

// -----------------------------------------------------------------------

void SalObject::ResetClipRegion()
{
    ImplDestroyAllClipWindows( maObjectData.mhLastClipWnd );
    maObjectData.mhLastClipWnd = HWND_TOP;
}

// -----------------------------------------------------------------------

USHORT SalObject::GetClipRegionType()
{
    return SAL_OBJECT_CLIP_EXCLUDERECTS;
}

// -----------------------------------------------------------------------

void SalObject::BeginSetClipRegion( ULONG nRectCount )
{
    maObjectData.mhOldLastClipWnd = maObjectData.mhLastClipWnd;
}

// -----------------------------------------------------------------------

void SalObject::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    HWND hClipWnd = WinCreateWindow( maObjectData.mhWnd, SAL_OBJECT_CLIPCLASSNAME, "",
                                     WS_VISIBLE,
                                     nX, maObjectData.mnHeight-(nY+nHeight), nWidth, nHeight,
                                     maObjectData.mhWnd, maObjectData.mhLastClipWnd,
                                     0, NULL, NULL );
    maObjectData.mhLastClipWnd = hClipWnd;
}

// -----------------------------------------------------------------------

void SalObject::EndSetClipRegion()
{
    ImplDestroyAllClipWindows( maObjectData.mhOldLastClipWnd );
}

// -----------------------------------------------------------------------

void SalObject::SetPosSize( long nX, long nY, long nWidth, long nHeight )
{
    PM_ULONG  nStyle = 0;
    PM_BOOL   bVisible = WinIsWindowVisible( maObjectData.mhWnd );
    if ( bVisible )
    {
        WinShowWindow( maObjectData.mhWnd, FALSE );
        nStyle |= SWP_SHOW;
    }
    SWP aParentSWP;
    WinQueryWindowPos( WinQueryWindow( maObjectData.mhWnd, QW_PARENT ), &aParentSWP );
    WinSetWindowPos( maObjectData.mhWnd, 0, nX, aParentSWP.cy-(nY+nHeight), nWidth, nHeight,
                     SWP_MOVE | SWP_SIZE | nStyle );
}

// -----------------------------------------------------------------------

void SalObject::Show( BOOL bVisible )
{
    WinShowWindow( maObjectData.mhWnd, bVisible );
}

// -----------------------------------------------------------------------

void SalObject::Enable( BOOL bEnable )
{
    // Im Gegensatz zu Windows disablen wir das Childfenster,
    // da ansonsten unser Clippen nicht mehr funktioniert, da
    // wir keine Events mehr bekommen. Dadurch kann sich evtl.
    // das Fenster anders darstellen, was wir eigentlich nicht
    // wollen. Aber da uns bisher kein Fall bekannt ist,
    // ignorieren wir dies. Ansonsten muss ein Fenster dazwischen
    // gezogen werden oder getestet werden, wie wir die
    // Maustransparenz erreichen, wenn maObjectData.mhWnd
    // disablte wird.
    WinEnableWindow( maObjectData.mhWndChild, bEnable );
}

// -----------------------------------------------------------------------

void SalObject::GrabFocus()
{
    if ( maObjectData.mhLastFocusWnd &&
         WinIsWindow( GetSalData()->mhAB, maObjectData.mhLastFocusWnd ) &&
         ImplIsSysWindowOrChild( maObjectData.mhWndChild, maObjectData.mhLastFocusWnd ) )
        WinSetFocus( HWND_DESKTOP, maObjectData.mhLastFocusWnd );
    else
        WinSetFocus( HWND_DESKTOP, maObjectData.mhWndChild );
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

const SystemChildData* SalObject::GetSystemData() const
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
