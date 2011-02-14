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

// --------------------
// - Standard-Defines -
// --------------------

#define MAX_STOCKPEN            4
#define MAX_STOCKBRUSH          4
#define SAL_CLIPRECT_COUNT      16

// --------------------
// - Icon cache       -
// --------------------

struct SalIcon
{
    int     nId;
    HICON   hIcon;
    HICON   hSmallIcon;
    SalIcon *pNext;
};

// -----------
// - SalData -
// -----------

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
    sal_uInt16                  mnStockPenCount;        // count of static pens
    sal_uInt16                  mnStockBrushCount;      // count of static brushes
    WPARAM                  mnSalObjWantKeyEvt;     // KeyEvent, welcher vom SalObj-Hook verarbeitet werden soll
    BYTE                    mnCacheDCInUse;         // count of CacheDC in use
    sal_Bool                    mbObjClassInit;         // is SALOBJECTCLASS initialised
    sal_Bool                    mbInPalChange;          // is in WM_QUERYNEWPALETTE
    DWORD                   mnAppThreadId;          // Id from Applikation-Thread
    BOOL                mbScrSvrEnabled;        // ScreenSaver enabled
    int                     mnSageStatus;           // status of Sage-DLL (DISABLE_AGENT == nicht vorhanden)
    SysAgt_Enable_PROC      mpSageEnableProc;       // funktion to deactivate the system agent
    SalIcon*                mpFirstIcon;            // icon cache, points to first icon, NULL if none
    TempFontItem*           mpTempFontItem;
    sal_Bool                mbThemeChanged;         // true if visual theme was changed: throw away theme handles
    sal_Bool                mbThemeMenuSupport;

    // for GdiPlus GdiplusStartup/GdiplusShutdown
    ULONG_PTR               gdiplusToken;

    std::set< HMENU >       mhMenuSet;              // keeps track of menu handles created by VCL, used by IsKnownMenuHandle()
    std::map< UINT,USHORT > maVKMap;      // map some dynamic VK_* entries
    oslModule               maDwmLib;
    DwmIsCompositionEnabled_ptr mpDwmIsCompositionEnabled;
};

inline void SetSalData( SalData* pData ) { ImplGetSVData()->mpSalData = (void*)pData; }
inline SalData* GetSalData() { return (SalData*)ImplGetSVData()->mpSalData; }
inline SalData* GetAppSalData() { return (SalData*)ImplGetAppSVData()->mpSalData; }

// --------------
// - SalShlData -
// --------------

struct SalShlData
{
    HINSTANCE               mhInst;                 // Instance of SAL-DLL
    UINT                    mnWheelScrollLines;     // WheelScrollLines
    UINT                    mnWheelScrollChars;     // WheelScrollChars
    UINT                    mnWheelMsgId;           // Wheel-Message-Id fuer W95
    WORD                    mnVersion;              // System-Version (311 == 3.11)
    BOOL                    mbW40;                  // Is System-Version >= 4.0
    BOOL                    mbWXP;                  // Windows XP
    BOOL                    mbWPrinter;             // true: use unicode printer functions
                                                    // false: use anis compat printer functions
    OSVERSIONINFO           maVersionInfo;
};

extern SalShlData aSalShlData;

// ------------
// - GDICache -
// ------------

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

bool ImplAddTempFont( SalData&, const String& rFontFileURL );
void ImplReleaseTempFonts( SalData& );

// --------------------------------------------
// - SALSHL.CXX - for accessing DLL resources -
// --------------------------------------------

HCURSOR ImplLoadSalCursor( int nId );
HBITMAP ImplLoadSalBitmap( int nId );
sal_Bool ImplLoadSalIcon( int nId, HICON& rIcon, HICON& rSmallIcon );

// SALGDI.CXX
void ImplInitSalGDI();
void ImplFreeSalGDI();

// --------------
// - Prototypes -
// --------------

// \\WIN\SOURCE\APP\SALINST.CXX
void ImplSalYieldMutexAcquireWithWait();
sal_Bool ImplSalYieldMutexTryToAcquire();
void ImplSalYieldMutexAcquire();
void ImplSalYieldMutexRelease();
sal_uLong ImplSalReleaseYieldMutex();
void ImplSalAcquireYieldMutex( sal_uLong nCount );
sal_Bool ImplInterceptChildWindowKeyDown( MSG& rMsg );

// \\WIN\SOURCE\WINDOW\SALFRAME.CXX
LRESULT CALLBACK SalFrameWndProcA( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK SalFrameWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
// \SV\WIN\SOURCE\APP\SALTIMER.CXX
#define SALTIMERPROC_RECURSIVE 0xffffffff
void    CALLBACK SalTimerProc( HWND hWnd, UINT nMsg, UINT_PTR nId, DWORD nTime );

// \WIN\SOURCE\WINDOW\SALFRAME.CXX
void SalTestMouseLeave();
sal_Bool ImplWriteLastError( DWORD lastError, const char *szApiCall );

// \WIN\SOURCE\WINDOW\SALFRAME.CXX
long ImplHandleSalObjKeyMsg( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
long ImplHandleSalObjSysCharMsg( HWND hWnd, WPARAM wParam, LPARAM lParam );
sal_Bool ImplHandleGlobalMsg( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT& rlResult );

// \WIN\SOURCE\WINDOW\SALOBJ.CXX
WinSalObject* ImplFindSalObject( HWND hWndChild );
sal_Bool ImplSalPreDispatchMsg( MSG* pMsg );
void ImplSalPostDispatchMsg( MSG* pMsg, LRESULT nDispatchResult );

// \WIN\SOURCE\GDI\SALGDI3.CXX
void ImplSalLogFontToFontA( HDC hDC, const LOGFONTA& rLogFont, Font& rFont );
void ImplSalLogFontToFontW( HDC hDC, const LOGFONTW& rLogFont, Font& rFont );
bool ImplIsFontAvailable( HDC hDC, const UniString& rName );

// \WIN\SOURCE\APP\SALDATA.CXX
rtl_TextEncoding ImplSalGetSystemEncoding();
ByteString ImplSalGetWinAnsiString( const UniString& rStr, sal_Bool bFileName = FALSE );
UniString ImplSalGetUniString( const sal_Char* pStr, xub_StrLen nLen = STRING_LEN );
int ImplSalWICompareAscii( const wchar_t* pStr1, const char* pStr2 );

// -----------
// - Defines -
// -----------

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
// NULL-Message, soll nicht verarbeitet werden
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


// -----------------
// - Helpfunctions -
// -----------------

// A/W-Wrapper
LONG        ImplSetWindowLong( HWND hWnd, int nIndex, DWORD dwNewLong );
LONG        ImplGetWindowLong( HWND hWnd, int nIndex );
BOOL    ImplPostMessage( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
BOOL    ImplSendMessage( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
BOOL    ImplGetMessage( LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax );
BOOL    ImplPeekMessage( LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg );
LONG        ImplDispatchMessage( CONST MSG *lpMsg );

inline void SetWindowPtr( HWND hWnd, WinSalFrame* pThis )
{
    ImplSetWindowLong( hWnd, SAL_FRAME_THIS, (LONG)pThis );
}

inline WinSalFrame* GetWindowPtr( HWND hWnd )
{
    return (WinSalFrame*)ImplGetWindowLong( hWnd, SAL_FRAME_THIS );
}

inline void SetSalObjWindowPtr( HWND hWnd, WinSalObject* pThis )
{
    ImplSetWindowLong( hWnd, SAL_OBJECT_THIS, (LONG)pThis );
}

inline WinSalObject* GetSalObjWindowPtr( HWND hWnd )
{
    return (WinSalObject*)ImplGetWindowLong( hWnd, SAL_OBJECT_THIS );
}

#endif  // _SV_SALDATA_HXX
