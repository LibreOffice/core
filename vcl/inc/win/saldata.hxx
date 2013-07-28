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

#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

#include "osl/module.h"

#include <svdata.hxx>
#include <salwtype.hxx>

#include <win/wincomp.hxx>

#include <set>  // for hMenu validation
#include <map>

class AutoTimer;
class WinSalInstance;
class WinSalObject;
class WinSalFrame;
class WinSalVirtualDevice;
class WinSalPrinter;
class Font;
struct HDCCache;
struct TempFontItem;

typedef HRESULT (WINAPI  *DwmIsCompositionEnabled_ptr)(BOOL*);

#define MAX_STOCKPEN            4
#define MAX_STOCKBRUSH          4
#define SAL_CLIPRECT_COUNT      16

struct SalIcon
{
    int     nId;
    HICON   hIcon;
    HICON   hSmallIcon;
    SalIcon *pNext;
};

class SalData
{
public:
    SalData();
    ~SalData();

    // native widget framework
    void    initNWF();
    void    deInitNWF();

    // fill maVKMap;
    void initKeyCodeMap();

    // checks if the menuhandle was created by VCL
    sal_Bool    IsKnownMenuHandle( HMENU hMenu );

public:
    HINSTANCE               mhInst;                 // default instance handle
    HINSTANCE               mhPrevInst;             // previous instance handle
    int                     mnCmdShow;              // default frame show style
    HPALETTE                mhDitherPal;            // dither palette
    HGLOBAL                 mhDitherDIB;            // dither memory handle
    BYTE*                   mpDitherDIB;            // dither memory
    BYTE*                   mpDitherDIBData;        // beginning of DIB data
    long*                   mpDitherDiff;           // Dither mapping table
    BYTE*                   mpDitherLow;            // Dither mapping table
    BYTE*                   mpDitherHigh;           // Dither mapping table
    sal_uLong                   mnTimerMS;              // Current Time (in MS) of the Timer
    sal_uLong                   mnTimerOrgMS;           // Current Original Time (in MS)
    DWORD                   mnNextTimerTime;
    DWORD                   mnLastEventTime;
    UINT                    mnTimerId;              // windows timer id
    sal_Bool                    mbInTimerProc;          // timer event is currently being dispatched
    HHOOK                   mhSalObjMsgHook;        // hook to get interesting msg for SalObject
    HWND                    mhWantLeaveMsg;         // window handle, that want a MOUSELEAVE message
    AutoTimer*              mpMouseLeaveTimer;      // Timer for MouseLeave Test
    WinSalInstance*         mpFirstInstance;        // pointer of first instance
    WinSalFrame*            mpFirstFrame;           // pointer of first frame
    WinSalObject*           mpFirstObject;          // pointer of first object window
    WinSalVirtualDevice*    mpFirstVD;              // first VirDev
    WinSalPrinter*          mpFirstPrinter;         // first printing printer
    HDCCache*               mpHDCCache;             // Cache for three DC's
    HBITMAP                 mh50Bmp;                // 50% Bitmap
    HBRUSH                  mh50Brush;              // 50% Brush
    COLORREF                maStockPenColorAry[MAX_STOCKPEN];
    COLORREF                maStockBrushColorAry[MAX_STOCKBRUSH];
    HPEN                    mhStockPenAry[MAX_STOCKPEN];
    HBRUSH                  mhStockBrushAry[MAX_STOCKBRUSH];
    sal_uInt16              mnStockPenCount;        // count of static pens
    sal_uInt16              mnStockBrushCount;      // count of static brushes
    WPARAM                  mnSalObjWantKeyEvt;     // KeyEvent that should be processed by SalObj-Hook
    BYTE                    mnCacheDCInUse;         // count of CacheDC in use
    sal_Bool                mbObjClassInit;         // is SALOBJECTCLASS initialised
    sal_Bool                mbInPalChange;          // is in WM_QUERYNEWPALETTE
    DWORD                   mnAppThreadId;          // Id from Applikation-Thread
    BOOL                    mbScrSvrEnabled;        // ScreenSaver enabled
    int                     mnSageStatus;           // status of Sage-DLL (DISABLE_AGENT == does not exist)
    SysAgt_Enable_PROC      mpSageEnableProc;       // funktion to deactivate the system agent
    SalIcon*                mpFirstIcon;            // icon cache, points to first icon, NULL if none
    TempFontItem*           mpTempFontItem;
    sal_Bool                mbThemeChanged;         // true if visual theme was changed: throw away theme handles
    sal_Bool                mbThemeMenuSupport;

    // for GdiPlus GdiplusStartup/GdiplusShutdown
    ULONG_PTR               gdiplusToken;

    std::set< HMENU >       mhMenuSet;              // keeps track of menu handles created by VCL, used by IsKnownMenuHandle()
    std::map< UINT,sal_uInt16 > maVKMap;      // map some dynamic VK_* entries
    oslModule               maDwmLib;
    DwmIsCompositionEnabled_ptr mpDwmIsCompositionEnabled;
};

inline void SetSalData( SalData* pData ) { ImplGetSVData()->mpSalData = pData; }
inline SalData* GetSalData() { return ImplGetSVData()->mpSalData; }

struct SalShlData
{
    HINSTANCE               mhInst;                 // Instance of SAL-DLL
    UINT                    mnWheelScrollLines;     // WheelScrollLines
    UINT                    mnWheelScrollChars;     // WheelScrollChars
    UINT                    mnWheelMsgId;           // Wheel-Message-Id fuer W95
    BOOL                    mbWXP;                  // Windows XP
    BOOL                    mbW7;                   // Windows 7
    OSVERSIONINFO           maVersionInfo;
};

extern SalShlData aSalShlData;

#define CACHESIZE_HDC       3
#define CACHED_HDC_1        0
#define CACHED_HDC_2        1
#define CACHED_HDC_DRAW     2
#define CACHED_HDC_DEFEXT   64

struct HDCCache
{
    HDC         mhDC;
    HPALETTE    mhDefPal;
    HBITMAP     mhDefBmp;
    HBITMAP     mhSelBmp;
    HBITMAP     mhActBmp;
};

void ImplClearHDCCache( SalData* pData );
HDC ImplGetCachedDC( sal_uLong nID, HBITMAP hBmp = 0 );
void ImplReleaseCachedDC( sal_uLong nID );

bool ImplAddTempFont( SalData&, const OUString& rFontFileURL );
void ImplReleaseTempFonts( SalData& );

HCURSOR ImplLoadSalCursor( int nId );
HBITMAP ImplLoadSalBitmap( int nId );
sal_Bool ImplLoadSalIcon( int nId, HICON& rIcon, HICON& rSmallIcon );

void ImplInitSalGDI();
void ImplFreeSalGDI();

void ImplSalYieldMutexAcquireWithWait();
sal_Bool ImplSalYieldMutexTryToAcquire();
void ImplSalYieldMutexAcquire();
void ImplSalYieldMutexRelease();
sal_uLong ImplSalReleaseYieldMutex();
void ImplSalAcquireYieldMutex( sal_uLong nCount );

LRESULT CALLBACK SalFrameWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );

#define SALTIMERPROC_RECURSIVE 0xffffffff
void    CALLBACK SalTimerProc( HWND hWnd, UINT nMsg, UINT_PTR nId, DWORD nTime );

void SalTestMouseLeave();
sal_Bool ImplWriteLastError( DWORD lastError, const char *szApiCall );

long ImplHandleSalObjKeyMsg( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
long ImplHandleSalObjSysCharMsg( HWND hWnd, WPARAM wParam, LPARAM lParam );
sal_Bool ImplHandleGlobalMsg( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT& rlResult );

WinSalObject* ImplFindSalObject( HWND hWndChild );
sal_Bool ImplSalPreDispatchMsg( MSG* pMsg );
void ImplSalPostDispatchMsg( MSG* pMsg, LRESULT nDispatchResult );

void ImplSalLogFontToFontW( HDC hDC, const LOGFONTW& rLogFont, Font& rFont );

rtl_TextEncoding ImplSalGetSystemEncoding();
OUString ImplSalGetUniString(const sal_Char* pStr, sal_Int32 nLen = -1);
int ImplSalWICompareAscii( const wchar_t* pStr1, const char* pStr2 );

#define SAL_FRAME_WNDEXTRA          sizeof( DWORD )
#define SAL_FRAME_THIS              0
#define SAL_FRAME_CLASSNAMEA        "SALFRAME"
#define SAL_FRAME_CLASSNAMEW        L"SALFRAME"
#define SAL_SUBFRAME_CLASSNAMEA     "SALSUBFRAME"
#define SAL_SUBFRAME_CLASSNAMEW     L"SALSUBFRAME"
#define SAL_TMPSUBFRAME_CLASSNAMEW  L"SALTMPSUBFRAME"
#define SAL_OBJECT_WNDEXTRA         sizeof( DWORD )
#define SAL_OBJECT_THIS             0
#define SAL_OBJECT_CLASSNAMEA       "SALOBJECT"
#define SAL_OBJECT_CLASSNAMEW       L"SALOBJECT"
#define SAL_OBJECT_CHILDCLASSNAMEA  "SALOBJECTCHILD"
#define SAL_OBJECT_CHILDCLASSNAMEW  L"SALOBJECTCHILD"
#define SAL_COM_CLASSNAMEA          "SALCOMWND"
#define SAL_COM_CLASSNAMEW          L"SALCOMWND"

#define SAL_MOUSELEAVE_TIMEOUT      300

// wParam == hDC; lParam == 0
#define SAL_MSG_PRINTABORTJOB       (WM_USER+110)
// wParam == bWait; lParam == 0
#define SAL_MSG_THREADYIELD         (WM_USER+111)
// wParam == 0; lParam == 0
#define SAL_MSG_RELEASEWAITYIELD    (WM_USER+112)
// wParam == 0; lParam == nMS
#define SAL_MSG_STARTTIMER          (WM_USER+113)
// wParam == nFrameStyle; lParam == pParent; lResult == pFrame
#define SAL_MSG_CREATEFRAME         (WM_USER+114)
// wParam == 0; lParam == 0
#define SAL_MSG_DESTROYFRAME        (WM_USER+115)
// wParam == 0; lParam == pParent; lResult == pObject
#define SAL_MSG_CREATEOBJECT        (WM_USER+116)
// wParam == 0; lParam == pObject;
#define SAL_MSG_DESTROYOBJECT       (WM_USER+117)
// wParam == hWnd; lParam == 0; lResult == hDC
#define SAL_MSG_GETDC               (WM_USER+120)
// wParam == hWnd; lParam == 0
#define SAL_MSG_RELEASEDC           (WM_USER+121)
// wParam == newParentHwnd; lParam == oldHwnd; lResult == newhWnd
#define SAL_MSG_RECREATEHWND         (WM_USER+122)
// wParam == newParentHwnd; lParam == oldHwnd; lResult == newhWnd
#define SAL_MSG_RECREATECHILDHWND    (WM_USER+123)
// wParam == 0; lParam == HWND;
#define SAL_MSG_DESTROYHWND         (WM_USER+124)

// wParam == 0; lParam == pData
#define SAL_MSG_USEREVENT           (WM_USER+130)
// wParam == 0; lParam == MousePosition relativ to upper left of screen
#define SAL_MSG_MOUSELEAVE          (WM_USER+131)
// NULL-Message, should not be processed
#define SAL_MSG_DUMMY               (WM_USER+132)
// wParam == 0; lParam == 0
#define SAL_MSG_POSTFOCUS           (WM_USER+133)
// wParam == wParam; lParam == lParam
#define SAL_MSG_POSTQUERYNEWPAL     (WM_USER+134)
// wParam == wParam; lParam == lParam
#define SAL_MSG_POSTPALCHANGED      (WM_USER+135)
// wParam == wParam; lParam == lParam
#define SAL_MSG_POSTMOVE            (WM_USER+136)
// wParam == wParam; lParam == lParam
#define SAL_MSG_POSTCALLSIZE        (WM_USER+137)
// wParam == pRECT; lParam == 0
#define SAL_MSG_POSTPAINT           (WM_USER+138)
// wParam == 0; lParam == pFrame; lResult 0
#define SAL_MSG_FORCEPALETTE        (WM_USER+139)
// wParam == 0; lParam == 0
#define SAL_MSG_CAPTUREMOUSE        (WM_USER+140)
// wParam == 0; lParam == 0
#define SAL_MSG_RELEASEMOUSE        (WM_USER+141)
// wParam == nFlags; lParam == 0
#define SAL_MSG_TOTOP               (WM_USER+142)
// wParam == bVisible; lParam == 0
#define SAL_MSG_SHOW                (WM_USER+143)
// wParam == 0; lParam == SalInputContext
#define SAL_MSG_SETINPUTCONTEXT     (WM_USER+144)
// wParam == nFlags; lParam == 0
#define SAL_MSG_ENDEXTTEXTINPUT     (WM_USER+145)
// POSTTIMER-Message; wparam = 0, lParam == time
#define SAL_MSG_POSTTIMER        (WM_USER+161)

// SysChild-ToTop; wParam = 0; lParam = 0
#define SALOBJ_MSG_TOTOP            (WM_USER+160)
// POSTFOCUS-Message; wParam == bFocus; lParam == 0
#define SALOBJ_MSG_POSTFOCUS        (WM_USER+161)


// A/W-Wrapper
BOOL    ImplPostMessage( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
BOOL    ImplSendMessage( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
BOOL    ImplGetMessage( LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax );
BOOL    ImplPeekMessage( LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg );
LONG        ImplDispatchMessage( CONST MSG *lpMsg );

inline void SetWindowPtr( HWND hWnd, WinSalFrame* pThis )
{
    SetWindowLongPtr( hWnd, SAL_FRAME_THIS, (LONG_PTR)pThis );
}

inline WinSalFrame* GetWindowPtr( HWND hWnd )
{
    return (WinSalFrame*)GetWindowLongPtrW( hWnd, SAL_FRAME_THIS );
}

inline void SetSalObjWindowPtr( HWND hWnd, WinSalObject* pThis )
{
    SetWindowLongPtr( hWnd, SAL_OBJECT_THIS, (LONG_PTR)pThis );
}

inline WinSalObject* GetSalObjWindowPtr( HWND hWnd )
{
    return (WinSalObject*)GetWindowLongPtr( hWnd, SAL_OBJECT_THIS );
}

#endif  // _SV_SALDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
