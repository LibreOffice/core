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

#ifndef INCLUDED_VCL_INC_WIN_SALDATA_HXX
#define INCLUDED_VCL_INC_WIN_SALDATA_HXX

#include <memory>
#include "osl/module.h"

#include <svdata.hxx>
#include <salwtype.hxx>

#include <win/wincomp.hxx>

#include <set>
#include <map>

class AutoTimer;
class WinSalInstance;
class WinSalObject;
class WinSalFrame;
class WinSalVirtualDevice;
class WinSalPrinter;
namespace vcl { class Font; }
struct GlobalGlyphCache;
struct HDCCache;
struct TempFontItem;
class TextOutRenderer;
class TheTextureCache;

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
    static void initNWF();
    static void deInitNWF();

    // fill maVKMap;
    void initKeyCodeMap();

    // checks if the menuhandle was created by VCL
    bool    IsKnownMenuHandle( HMENU hMenu );

    bool                    mbResourcesAlreadyFreed;

public:
    HINSTANCE               mhInst;                 // default instance handle
    int                     mnCmdShow;              // default frame show style
    HPALETTE                mhDitherPal;            // dither palette
    HGLOBAL                 mhDitherDIB;            // dither memory handle
    BYTE*                   mpDitherDIB;            // dither memory
    BYTE*                   mpDitherDIBData;        // beginning of DIB data
    long*                   mpDitherDiff;           // Dither mapping table
    BYTE*                   mpDitherLow;            // Dither mapping table
    BYTE*                   mpDitherHigh;           // Dither mapping table
    HHOOK                   mhSalObjMsgHook;        // hook to get interesting msg for SalObject
    HWND                    mhWantLeaveMsg;         // window handle, that want a MOUSELEAVE message
    AutoTimer*              mpMouseLeaveTimer;      // Timer for MouseLeave Test
    WinSalInstance*         mpInstance;
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
    bool                    mbObjClassInit;         // is SALOBJECTCLASS initialised
    bool                    mbInPalChange;          // is in WM_QUERYNEWPALETTE
    DWORD                   mnAppThreadId;          // Id from Application-Thread
    BOOL                    mbScrSvrEnabled;        // ScreenSaver enabled
    SalIcon*                mpFirstIcon;            // icon cache, points to first icon, NULL if none
    TempFontItem*           mpTempFontItem;
    bool                    mbThemeChanged;         // true if visual theme was changed: throw away theme handles
    bool                    mbThemeMenuSupport;

    // for GdiPlus GdiplusStartup/GdiplusShutdown
    ULONG_PTR               gdiplusToken;

    std::set< HMENU >       mhMenuSet;              // keeps track of menu handles created by VCL, used by IsKnownMenuHandle()
    std::map< UINT,sal_uInt16 > maVKMap;      // map some dynamic VK_* entries

    // must be deleted before exit(), so delete it in DeInitSalData()
    std::unique_ptr<TextOutRenderer> m_pD2DWriteTextOutRenderer;
    // tdf#107205 need 2 instances because D2DWrite can't rotate text
    std::unique_ptr<TextOutRenderer> m_pExTextOutRenderer;
    std::unique_ptr<GlobalGlyphCache> m_pGlobalGlyphCache;
    std::unique_ptr<TheTextureCache> m_pTextureCache;
};

inline void SetSalData( SalData* pData ) { ImplGetSVData()->mpSalData = pData; }
inline SalData* GetSalData() { return ImplGetSVData()->mpSalData; }

struct SalShlData
{
    HINSTANCE               mhInst;                 // Instance of SAL-DLL
    UINT                    mnWheelScrollLines;     // WheelScrollLines
    UINT                    mnWheelScrollChars;     // WheelScrollChars
    BOOL                    mbWXP;                  // Windows XP
    BOOL                    mbWVista;               // Windows Vista
    BOOL                    mbW7;                   // Windows 7
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
HDC ImplGetCachedDC( sal_uLong nID, HBITMAP hBmp = nullptr );
void ImplReleaseCachedDC( sal_uLong nID );

bool ImplAddTempFont( SalData&, const OUString& rFontFileURL );
void ImplReleaseTempFonts( SalData& );

HCURSOR ImplLoadSalCursor( int nId );
HBITMAP ImplLoadSalBitmap( int nId );
bool ImplLoadSalIcon( int nId, HICON& rIcon, HICON& rSmallIcon );

void ImplInitSalGDI();
void ImplFreeSalGDI();

void ImplSalYieldMutexAcquireWithWait( sal_uInt32 nCount = 1 );
bool ImplSalYieldMutexTryToAcquire();
void ImplSalYieldMutexRelease();

LRESULT CALLBACK SalFrameWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );

void SalTestMouseLeave();

bool ImplHandleSalObjKeyMsg( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
bool ImplHandleSalObjSysCharMsg( HWND hWnd, WPARAM wParam, LPARAM lParam );
bool ImplHandleGlobalMsg( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT& rlResult );

WinSalObject* ImplFindSalObject( HWND hWndChild );
bool ImplSalPreDispatchMsg( MSG* pMsg );
void ImplSalPostDispatchMsg( MSG* pMsg, LRESULT nDispatchResult );

void ImplSalLogFontToFontW( HDC hDC, const LOGFONTW& rLogFont, vcl::Font& rFont );

rtl_TextEncoding ImplSalGetSystemEncoding();
OUString ImplSalGetUniString(const sal_Char* pStr, sal_Int32 nLen = -1);
int ImplSalWICompareAscii( const wchar_t* pStr1, const char* pStr2 );

#define SAL_FRAME_WNDEXTRA          sizeof( DWORD )
#define SAL_FRAME_THIS              GWLP_USERDATA
#define SAL_FRAME_CLASSNAMEW        L"SALFRAME"
#define SAL_SUBFRAME_CLASSNAMEW     L"SALSUBFRAME"
#define SAL_TMPSUBFRAME_CLASSNAMEW  L"SALTMPSUBFRAME"
#define SAL_OBJECT_WNDEXTRA         sizeof( DWORD )
#define SAL_OBJECT_THIS             GWLP_USERDATA
#define SAL_OBJECT_CLASSNAMEA       "SALOBJECT"
#define SAL_OBJECT_CHILDCLASSNAMEA  "SALOBJECTCHILD"
#define SAL_COM_CLASSNAMEW          L"SALCOMWND"

#define SAL_MOUSELEAVE_TIMEOUT      300

// wParam == bWait; lParam == 0
#define SAL_MSG_THREADYIELD         (WM_USER+111)
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
#define SAL_MSG_RECREATEHWND        (WM_USER+122)
// wParam == newParentHwnd; lParam == oldHwnd; lResult == newhWnd
#define SAL_MSG_RECREATECHILDHWND   (WM_USER+123)
// wParam == 0; lParam == HWND;
#define SAL_MSG_DESTROYHWND         (WM_USER+124)

// wParam == 0; lParam == pData
#define SAL_MSG_USEREVENT           (WM_USER+130)
// wParam == 0; lParam == MousePosition relative to upper left of screen
#define SAL_MSG_MOUSELEAVE          (WM_USER+131)
// NULL-Message, should not be processed
#define SAL_MSG_DUMMY               (WM_USER+132)
// Used for SETFOCUS and KILLFOCUS
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

// SysChild-ToTop; wParam = 0; lParam = 0
#define SALOBJ_MSG_TOTOP            (WM_USER+160)
// Used for SETFOCUS and KILLFOCUS
// POSTFOCUS-Message; wParam == bFocus; lParam == 0
#define SALOBJ_MSG_POSTFOCUS        (WM_USER+161)

// Call the Timer's callback from the main thread
// wParam = 1 == run when yield is idle instead of direct
#define SAL_MSG_TIMER_CALLBACK      (WM_USER+162)
// Stop the timer from the main thread; wParam = 0, lParam = 0
#define SAL_MSG_STOPTIMER           (WM_USER+163)

inline void SetWindowPtr( HWND hWnd, WinSalFrame* pThis )
{
    SetWindowLongPtr( hWnd, SAL_FRAME_THIS, reinterpret_cast<LONG_PTR>(pThis) );
}

inline WinSalFrame* GetWindowPtr( HWND hWnd )
{
    return reinterpret_cast<WinSalFrame*>(GetWindowLongPtrW( hWnd, SAL_FRAME_THIS ));
}

inline void SetSalObjWindowPtr( HWND hWnd, WinSalObject* pThis )
{
    SetWindowLongPtr( hWnd, SAL_OBJECT_THIS, reinterpret_cast<LONG_PTR>(pThis) );
}

inline WinSalObject* GetSalObjWindowPtr( HWND hWnd )
{
    return reinterpret_cast<WinSalObject*>(GetWindowLongPtr( hWnd, SAL_OBJECT_THIS ));
}

#endif // INCLUDED_VCL_INC_WIN_SALDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
