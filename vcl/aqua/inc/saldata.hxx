/*************************************************************************
 *
 *  $RCSfile: saldata.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:25 $
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

#ifndef _SV_SALDATA_HXX
#define _SV_SALDATA_HXX

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SALWTYPE_HXX
#include <salwtype.hxx>
#endif
#ifndef _SV_SALAQUA_HXX
#include <salaqua.hxx>
#endif

class AutoTimer;
class SalInstance;
class SalObject;
class SalFrame;
class SalVirtualDevice;
class SalPrinter;
class Font;
struct HDCCache;

// --------------------
// - Standard-Defines -
// --------------------

#define MAX_STOCKPEN            4
#define MAX_STOCKBRUSH          4
#define SAL_CLIPRECT_COUNT      16

// -----------
// - SalData -
// -----------

struct SalData
{
    HINSTANCE               mhInst;                 // default instance handle
    HINSTANCE               mhPrevInst;             // previous instance handle
    int                     mnCmdShow;              // default frame show style
    // Erst hier koennen Daten kompatible eingefuegt werden, da die
    // oberen Daten in salmain.cxx modifiziert werden
    HPALETTE                mhDitherPal;            // dither palette
    HGLOBAL                 mhDitherDIB;            // dither memory handle
    BYTE*                   mpDitherDIB;            // dither memory
    BYTE*                   mpDitherDIBData;        // beginning of DIB data
    long*                   mpDitherDiff;           // Dither mapping table
    BYTE*                   mpDitherLow;            // Dither mapping table
    BYTE*                   mpDitherHigh;           // Dither mapping table
    ULONG                   mnTimerMS;              // Current Time (in MS) of the Timer
    ULONG                   mnTimerOrgMS;           // Current Original Time (in MS)
    UINT                    mnTimerId;              // windows timer id
    SALTIMERPROC            mpTimerProc;            // timer callback proc
    HHOOK                   mhSalObjMsgHook;        // hook um SalObject relevante Message mitzubekommen
    HWND                    mhWantLeaveMsg;         // window handle, that want a MOUSELEAVE message
    AutoTimer*              mpMouseLeaveTimer;      // Timer for MouseLeave Test
    SalInstance*            mpFirstInstance;        // pointer of first instance
    SalFrame*               mpFirstFrame;           // pointer of first frame
    SalObject*              mpFirstObject;          // pointer of first object window
    SalVirtualDevice*       mpFirstVD;              // first VirDev
    SalPrinter*             mpFirstPrinter;         // first printing printer
    HDCCache*               mpHDCCache;             // Cache for three DC's
    HBITMAP                 mh50Bmp;                // 50% Bitmap
    HBRUSH                  mh50Brush;              // 50% Brush
    COLORREF                maStockPenColorAry[MAX_STOCKPEN];
    COLORREF                maStockBrushColorAry[MAX_STOCKBRUSH];
    HPEN                    mhStockPenAry[MAX_STOCKPEN];
    HBRUSH                  mhStockBrushAry[MAX_STOCKBRUSH];
    USHORT                  mnStockPenCount;        // Anzahl statischer Pens
    USHORT                  mnStockBrushCount;      // Anzahl statischer Brushes
    WPARAM                  mnSalObjWantKeyEvt;     // KeyEvent, welcher vom SalObj-Hook verarbeitet werden soll
    BOOL                    mbObjClassInit;         // Ist SALOBJECTCLASS initialised
    BOOL                    mbInPalChange;          // is in WM_QUERYNEWPALETTE
    DWORD                   mnAppThreadId;          // Id from Applikation-Thread
    WIN_BOOL                mbScrSvrEnabled;        // ScreenSaver enabled
    int                     mnSageStatus;           // Status der Sage-DLL (DISABLE_AGENT == nicht vorhanden)
    HINSTANCE               mhSageInst;             // Instance-Handle zur Sage-DLL
    SysAgt_Enable_PROC      mpSageEnableProc;       // Funktion zum deaktivieren des Systemagenten
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
    UINT                    mnVKAdd;                // VK-Code von KEY_ADD
    UINT                    mnVKSubtract;           // VK-Code von KEY_SUBTRACT
    UINT                    mnVKMultiply;           // VK-Code von KEY_MULTIPLY
    UINT                    mnVKDivide;             // VK-Code von KEY_DIVIDE
    UINT                    mnVKPoint;              // VK-Code von KEY_POINT
    UINT                    mnVKComma;              // VK-Code von KEY_KOMMA
    UINT                    mnVKLess;               // VK-Code von KEY_LESS
    UINT                    mnVKGreater;            // VK-Code von KEY_GREATER
    UINT                    mnVKEqual;              // VK-Code von KEY_EQUAL
    UINT                    mnWheelScrollLines;     // WheelScrollLines
    UINT                    mnWheelMsgId;           // Wheel-Message-Id fuer W95
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
HDC ImplGetCachedDC( ULONG nID, HBITMAP hBmp = 0 );
void ImplReleaseCachedDC( ULONG nID );

// ------------------------------------------------------
// - salshl.cxx - Funktionen fuer DLL-Resource-Zugriffe -
// ------------------------------------------------------

HCURSOR ImplLoadSalCursor( int nId );
HBITMAP ImplLoadSalBitmap( int nId );
BOOL ImplLoadSalIcon( int nId, HICON& rIcon, HICON& rSmallIcon );

void ImplInitSalGDI();
void ImplFreeSalGDI();

// --------------
// - Prototypes -
// --------------

void ImplSalYieldMutexAcquireWithWait();
BOOL ImplSalYieldMutexTryToAcquire();
void ImplSalYieldMutexAcquire();
void ImplSalYieldMutexRelease();
ULONG ImplSalReleaseYieldMutex();
void ImplSalAcquireYieldMutex( ULONG nCount );

LRESULT CALLBACK SalFrameWndProcA( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK SalFrameWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
void    CALLBACK SalTimerProc( HWND hWnd, UINT nMsg, UINT nId, DWORD nTime );

void SalTestMouseLeave();

long ImplHandleSalObjKeyMsg( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
long ImplHandleSalObjSysCharMsg( HWND hWnd, WPARAM wParam, LPARAM lParam );
BOOL ImplHandleGlobalMsg( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT& rlResult );

SalObject* ImplFindSalObject( HWND hWndChild );
#ifdef WIN
BOOL ImplSalPreDispatchMsg( MSG* pMsg );
void ImplSalPostDispatchMsg( MSG* pMsg, LRESULT nDispatchResult );

void ImplSalLogFontToFontA( const LOGFONTA& rLogFont, Font& rFont );
void ImplSalLogFontToFontW( const LOGFONTW& rLogFont, Font& rFont );
#endif

rtl_TextEncoding ImplSalGetSystemEncoding();
ByteString ImplSalGetWinAnsiString( const UniString& rStr, BOOL bFileName = FALSE );
UniString ImplSalGetUniString( const sal_Char* pStr, xub_StrLen nLen = STRING_LEN );
int ImplSalWICompareAscii( const wchar_t* pStr1, const char* pStr2 );

// -----------
// - Defines -
// -----------

#define SAL_FRAME_WNDEXTRA          sizeof( DWORD )
#define SAL_FRAME_THIS              0
#define SAL_FRAME_CLASSNAMEA        "SALFRAME"
#define SAL_FRAME_CLASSNAMEW        L"SALFRAME"
#define SAL_FRAME_CLASSNAME_SBA     "SALFRAMESB"
#define SAL_FRAME_CLASSNAME_SBW     L"SALFRAMESB"
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
// wParam == 0; lParam == this; lResult == bRet
#define SAL_MSG_CREATESOUND         (WM_USER+118)
// wParam == 0; lParam == this
#define SAL_MSG_DESTROYSOUND        (WM_USER+119)

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
// wParam == pRECT
#define SAL_MSG_POSTPAINT           (WM_USER+138)
// wParam == 0; lParam == pFrame; lResult 0
#define SAL_MSG_FORCEPALETTE        (WM_USER+139)
// wParam == 0; lParam == 0;
#define SAL_MSG_CAPTUREMOUSE        (WM_USER+140)
// wParam == 0; lParam == 0
#define SAL_MSG_RELEASEMOUSE        (WM_USER+141)
// wParam == nFlags; lParam == 0
#define SAL_MSG_TOTOP               (WM_USER+142)
// wParam == bVisible; lParam == 0
#define SAL_MSG_SHOW                (WM_USER+143)

// SysChild-ToTop; wParam = 0; lParam = 0
#define SALOBJ_MSG_TOTOP            (WM_USER+160)
// POSTFOCUS-Message; wParam == bFocus; lParam == 0
#define SALOBJ_MSG_POSTFOCUS        (WM_USER+161)

// -----------------
// - Helpfunctions -
// -----------------

// A/W-Wrapper
#ifdef WIN
LONG        ImplSetWindowLong( HWND hWnd, int nIndex, DWORD dwNewLong );
LONG        ImplGetWindowLong( HWND hWnd, int nIndex );
WIN_BOOL    ImplPostMessage( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
WIN_BOOL    ImplSendMessage( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
WIN_BOOL    ImplGetMessage( LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax );
WIN_BOOL    ImplPeekMessage( LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg );
LONG        ImplDispatchMessage( const MSG *lpMsg );
#endif

inline void SetWindowPtr( HWND hWnd, SalFrame* pThis )
{
#ifdef WIN
    ImplSetWindowLong( hWnd, SAL_FRAME_THIS, (LONG)pThis );
#endif
}

inline SalFrame* GetWindowPtr( HWND hWnd )
{
#ifdef WIN
    return (SalFrame*)ImplGetWindowLong( hWnd, SAL_FRAME_THIS );
#else
    return NULL;
#endif
}

inline void SetSalObjWindowPtr( HWND hWnd, SalObject* pThis )
{
#ifdef WIN
    ImplSetWindowLong( hWnd, SAL_OBJECT_THIS, (LONG)pThis );
#endif
}

inline SalObject* GetSalObjWindowPtr( HWND hWnd )
{
#ifdef WIN
    return (SalObject*)ImplGetWindowLong( hWnd, SAL_OBJECT_THIS );
#else
    return NULL;
#endif
}

#endif  // _SV_SALDATA_HXX
