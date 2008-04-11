/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: saldata.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

#include <vcl/sv.h>
#include <vcl/svdata.hxx>
#include <vcl/salwtype.hxx>

class Os2SalInstance;
class Os2SalFrame;
class Os2SalObject;

extern "C" int debug_printf(const char *f, ...);

// --------------
// - SalIMEData -
// --------------

// YD FIXME #define ENABLE_IME

#ifdef ENABLE_IME

struct SalIMEData;

#ifdef OS2IM_INCLUDED

typedef APIRET (APIENTRY ImAssociateInstanceFunc)( HWND hwnd, HIMI himi, PHIMI phimiPrev );
typedef APIRET (APIENTRY ImGetInstanceFunc)( HWND hwnd, PHIMI phimi );
typedef APIRET (APIENTRY ImReleaseInstanceFunc)( HWND hwnd, HIMI himi );
typedef APIRET (APIENTRY ImSetConversionFontFunc)( HIMI himi, PFATTRS pFontAttrs );
typedef APIRET (APIENTRY ImSetConversionFontSizeFunc)( HIMI himi, PSIZEF psizfxBox );
typedef APIRET (APIENTRY ImGetConversionStringFunc)( HIMI himi, ULONG ulIndex, PVOID pBuf, PULONG pulBufLen );
typedef APIRET (APIENTRY ImGetResultStringFunc)( HIMI himi, ULONG ulIndex, PVOID pBuf, PULONG pulBufLen );
typedef APIRET (APIENTRY ImSetCandidateWindowPosFunc)( HIMI himi, PCANDIDATEPOS pCandidatePos );
typedef APIRET (APIENTRY ImQueryIMEPropertyFunc)( HIMI himi, ULONG ulIndex, PULONG pulProp );
typedef APIRET (APIENTRY ImRequestIMEFunc)( HIMI himi, ULONG ulAction, ULONG ulIndex, ULONG ulValue );
typedef APIRET (APIENTRY ImSetIMModeFunc)( HIMI himi, ULONG ulInputMode, ULONG ulConversionMode );
typedef APIRET (APIENTRY ImQueryIMModeFunc)( HIMI himi, PULONG pulInputMode, PULONG pulConversionMode );

struct SalIMEData
{
    HMODULE                         mhModIME;
    ImAssociateInstanceFunc*        mpAssocIME;
    ImGetInstanceFunc*              mpGetIME;
    ImReleaseInstanceFunc*          mpReleaseIME;
    ImSetConversionFontFunc*        mpSetConversionFont;
    ImSetConversionFontSizeFunc*    mpSetConversionFontSize;
    ImGetConversionStringFunc*      mpGetConversionString;
    ImGetResultStringFunc*          mpGetResultString;
    ImSetCandidateWindowPosFunc*    mpSetCandidateWin;
    ImQueryIMEPropertyFunc*         mpQueryIMEProperty;
    ImRequestIMEFunc*               mpRequestIME;
    ImSetIMModeFunc*                mpSetIMEMode;
    ImQueryIMModeFunc*              mpQueryIMEMode;
};

#endif

#endif

// --------------------
// - Icon cache       -
// --------------------

struct SalIcon
{
    int     nId;
    HPOINTER hIcon;
    SalIcon *pNext;
};

// -----------
// - SalData -
// -----------

struct SalData
{
    HAB                     mhAB;                   // anchor block handle
    HMQ                     mhMQ;                   // handle of os2 message queue
    int                     mnArgc;                 // commandline param count
    char**                  mpArgv;                 // commandline
    ULONG                   mnNewTimerMS;           // Neue Zeit, mit dem der Timer gestartet werden soll
    ULONG                           mnTimerMS;                  // Current Time (in MS) of the Timer
    ULONG                           mnTimerOrgMS;               // Current Original Time (in MS)
    ULONG                   mnNextTimerTime;
    ULONG                   mnLastEventTime;
    ULONG                   mnTimerId;          // os2 timer id
    BOOL                    mbInTimerProc;          // timer event is currently being dispatched
    //SALTIMERPROC              mpTimerProc;            // timer callback proc
    HWND                    mhWantLeaveMsg;         // window handle, that want a MOUSELEAVE message
    AutoTimer*              mpMouseLeaveTimer;      // Timer for MouseLeave Test
    Os2SalInstance*             mpFirstInstance;        // pointer of first instance
    Os2SalFrame*                mpFirstFrame;           // pointer of first frame
    Os2SalFrame*                mpCreateFrame;          // Create-Frame for WM_CREATE
    Os2SalObject*               mpFirstObject;          // pointer of first object window
    ULONG                   mnAppThreadId;          // Id from Applikation-Thread
    ULONG                   mnFontMetricCount;      // number of entries in the font list
    PFONTMETRICS                mpFontMetrics;          // cached font list
    BOOL                    mbObjClassInit;         // Ist SALOBJECTCLASS initialised
#ifdef ENABLE_IME
    SalIMEData*                 mpIMEData;              // SalIME-Data
    BOOL                    mbIMEInit;              // SalIME-Data-Init
#endif

    SalIcon*                mpFirstIcon;            // icon cache, points to first icon, NULL if none

};

inline void SetSalData( SalData* pData ) { ImplGetSVData()->mpSalData = (void*)pData; }
inline SalData* GetSalData() { return (SalData*)ImplGetSVData()->mpSalData; }
inline SalData* GetAppSalData() { return (SalData*)ImplGetAppSVData()->mpSalData; }

// --------------
// - SalShlData -
// --------------

#define OS2_VER_211     211
#define OS2_VER_WARP3   230
#define OS2_VER_WARP4   240

struct SalShlData
{
    HMODULE                 mhMod;                  // Module handle of SAL-DLL
    USHORT                  mnVersion;              // 211 = OS2 2.11; 230 = OS2 3.0; 240 = OS2 4.0
    PFNWP                   mpOldFrameProc;         // old frame proc
};

extern SalShlData aSalShlData;

BOOL SalImplHandleProcessMenu( HWND hWnd, ULONG nMsg, MPARAM nMP1, MPARAM nMP2 );

// --------------------------------------------
// - SALSHL.CXX - for accessing DLL resources -
// --------------------------------------------

HPOINTER ImplLoadSalCursor( int nId );
HBITMAP ImplLoadSalBitmap( int nId );
BOOL ImplLoadSalIcon( int nId, HPOINTER& rIcon);

// SALGDI.CXX
void ImplInitSalGDI();
void ImplFreeSalGDI();

// --------------
// - Prototypes -
// --------------

// \\OS2\SOURCE\APP\SALINST.CXX
void ImplSalYieldMutexAcquireWithWait();
ULONG ImplSalReleaseYieldMutex();
void ImplSalAcquireYieldMutex( ULONG nCount );
ULONG GetCurrentThreadId();
BOOL ImplSalYieldMutexTryToAcquire();
void ImplSalYieldMutexAcquire();
void ImplSalYieldMutexRelease();

// \\OS2\SOURCE\WINDOW\SALFRAME.CXX
MRESULT EXPENTRY SalFrameWndProc( HWND hWnd, ULONG nMsg, MPARAM nMP1, MPARAM nMP2 );
MRESULT EXPENTRY SalFrameFrameProc( HWND hWnd, ULONG nMsg, MPARAM nMP1, MPARAM nMP2 );
// \SV\WIN\SOURCE\APP\SALTIMER.CXX
#define SALTIMERPROC_RECURSIVE 0xffffffff
void SalTimerProc( HWND hWnd, UINT nMsg, UINT nId, ULONG nTime );

// \WIN\SOURCE\WINDOW\SALFRAME.CXX
void SalTestMouseLeave();

// \\OS2\SOURCE\WINDOW\SALFRAME.CXX
// return Frame for Message-Handling
Os2SalFrame* GetSalDefaultFrame();

// \\OS2\SOURCE\WINDOW\SALFRAME.CXX
// IME-Daten wieder freigeben
#ifdef ENABLE_IME
void ImplReleaseSALIMEData();
#endif

// -----------
// - Defines -
// -----------

#define SAL_PROFILE_APPNAME         ((PSZ)"StarOffice")
#define SAL_PROFILE_USEDJP          ((PSZ)"UseDJP")
#define SAL_PROFILE_PRINTDJP        ((PSZ)"PrintDJP")
#define SAL_PROFILE_PRINTRAW        ((PSZ)"PrintRAW")

#define SAL_FRAME_WNDEXTRA          sizeof(ULONG)
#define SAL_FRAME_THIS              0
#define SAL_FRAME_CLASSNAME         "SALFRAME"
#define SAL_SUBFRAME_CLASSNAME      "SALSUBFRAME"
#define SAL_OBJECT_WNDEXTRA         sizeof(ULONG)
#define SAL_OBJECT_THIS             0
#define SAL_OBJECT_CLASSNAME        "SALOBJECT"
#define SAL_OBJECT_CHILDCLASSNAME   "SALOBJECTCHILD"
#define SAL_OBJECT_CLIPCLASSNAME    "SALOBJECTCLIP"
#define SAL_COM_CLASSNAME           "SALCOMWND"

#define SAL_MOUSELEAVE_TIMEOUT      300

// MP1 == 0; MP2 == pData
#define SAL_MSG_USEREVENT           (WM_USER+111)
// MP1 == 0; MP2 == MousePosition relativ to upper left of screen
#define SAL_MSG_MOUSELEAVE          (WM_USER+112)
// MP1 == hDC; MP2 == 0
#define SAL_MSG_PRINTABORTJOB       (WM_USER+113)
// MP1 == 0; MP2 == 0
#define SAL_MSG_STARTTIMER          (WM_USER+114)
// MP1 == nFrameStyle; MP2 == pParent; lResult pFrame
#define SAL_MSG_CREATEFRAME         (WM_USER+115)
// MP1 == 0; MP2 == pParent; lResult pObject
#define SAL_MSG_CREATEOBJECT        (WM_USER+116)
// MP1 == bWait; MP2 == pMutex
#define SAL_MSG_THREADYIELD         (WM_USER+117)
// MP1 == 0; MP2 == 0
#define SAL_MSG_RELEASEWAITYIELD    (WM_USER+118)
// MP1 == 0; MP2 == pData
#define SAL_MSG_SYSPROCESSMENU      (WM_USER+119)
// POSTFOCUS-Message; MP1 == nMP1; MP2 == nMP2 (SHORT1( bFocus ), 0)
#define SAL_MSG_POSTFOCUS           (WM_USER+120)
// POSTSIZE-Message; MP1 == nMP1; MP2 == nMP2
#define SAL_MSG_POSTSIZE            (WM_USER+121)

// wParam == wParam; lParam == lParam
#define SAL_MSG_POSTMOVE            (WM_USER+136)
// wParam == pRECT; lParam == 0
#define SAL_MSG_POSTPAINT           (WM_USER+137)
// wParam == nFlags; lParam == 0
#define SAL_MSG_TOTOP               (WM_USER+142)
// wParam == bVisible; lParam == 0
#define SAL_MSG_SHOW                (WM_USER+143)

// SysChild-ToTop; nMP1 = 0; nMP2 = 0
#define SALOBJ_MSG_TOTOP            (WM_USER+150)
// POSTFOCUS-Message; MP1 == nMP1; MP2 == nMP2 (SHORT1( bFocus ), 0)
#define SALOBJ_MSG_POSTFOCUS        (WM_USER+151)

// wParam == 0; lParam == 0
#define SAL_MSG_DESTROYFRAME        (WM_USER+160)
// wParam == 0; lParam == pObject;
#define SAL_MSG_DESTROYOBJECT       (WM_USER+161)
// wParam == 0; lParam == this; lResult == bRet
#define SAL_MSG_CREATESOUND         (WM_USER+162)
// wParam == 0; lParam == this
#define SAL_MSG_DESTROYSOUND        (WM_USER+163)
// wParam == hWnd; lParam == 0; lResult == hDC
#define SAL_MSG_GETDC               (WM_USER+164)
// wParam == hWnd; lParam == 0
#define SAL_MSG_RELEASEDC           (WM_USER+165)
// wParam == newParentHwnd; lParam == oldHwnd; lResult == newhWnd
#define SAL_MSG_RECREATEHWND         (WM_USER+166)
// wParam == newParentHwnd; lParam == oldHwnd; lResult == newhWnd
#define SAL_MSG_RECREATECHILDHWND    (WM_USER+167)
// wParam == 0; lParam == HWND;
#define SAL_MSG_DESTROYHWND         (WM_USER+168)
// POSTTIMER-Message; wparam = 0, lParam == time
#define SAL_MSG_POSTTIMER           (WM_USER+169)

// -----------------
// - Helpfunctions -
// -----------------

inline void SetWindowPtr( HWND hWnd, SalFrame* pThis )
{
    WinSetWindowULong( hWnd, SAL_FRAME_THIS, (ULONG)pThis );
}

inline Os2SalFrame* GetWindowPtr( HWND hWnd )
{
    return (Os2SalFrame*)WinQueryWindowULong( hWnd, SAL_FRAME_THIS );
}

inline void SetSalObjWindowPtr( HWND hWnd, SalObject* pThis )
{
    WinSetWindowULong( hWnd, SAL_OBJECT_THIS, (ULONG)pThis );
}

inline Os2SalObject* GetSalObjWindowPtr( HWND hWnd )
{
    return (Os2SalObject*)WinQueryWindowULong( hWnd, SAL_OBJECT_THIS );
}

#endif  // _SV_SALDATA_HXX
