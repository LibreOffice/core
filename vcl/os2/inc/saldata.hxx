/*************************************************************************
 *
 *  $RCSfile: saldata.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:34 $
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

class SalFrame;
class SalObject;

// --------------
// - SalIMEData -
// --------------

#define ENABLE_IME

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
    PM_ULONG                mnTimerId;              // os2 timer id
    SALTIMERPROC            mpTimerProc;            // timer callback proc
    HWND                    mhWantLeaveMsg;         // window handle, that want a MOUSELEAVE message
    AutoTimer*              mpMouseLeaveTimer;      // Timer for MouseLeave Test
    SalInstance*            mpFirstInstance;        // pointer of first instance
    SalFrame*               mpFirstFrame;           // pointer of first frame
    SalFrame*               mpDummyFrame;           // Dummy-Frame
    SalFrame*               mpCreateFrame;          // Create-Frame for WM_CREATE
    SalFrame*               mpDefaultFrame;         // Default-Frame (App-Fenster)
    SalObject*              mpFirstObject;          // pointer of first object window
    ULONG                   mnAppThreadId;          // Id from Applikation-Thread
    ULONG                   mnFontMetricCount;      // number of entries in the font list
    PFONTMETRICS            mpFontMetrics;          // cached font list
    BOOL                    mbObjClassInit;         // Ist SALOBJECTCLASS initialised
#ifdef ENABLE_IME
    SalIMEData*             mpIMEData;              // SalIME-Data
    BOOL                    mbIMEInit;              // SalIME-Data-Init
#endif
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

BOOL SalImplHandleProcessMenu( HWND hWnd, PM_ULONG nMsg, MPARAM nMP1, MPARAM nMP2 );

// SALSHL.CXX - Funktionen fuer DLL-Resource-Zugriffe
HPOINTER ImplLoadPointer( ULONG nId );

// --------------
// - Prototypes -
// --------------

// \\OS2\SOURCE\APP\SALINST.CXX
ULONG ImplSalGetCurrentThreadId();
BOOL ImplSalYieldMutexTryToAcquire();
void ImplSalYieldMutexAcquire();
void ImplSalYieldMutexRelease();

// \\OS2\SOURCE\WINDOW\SALFRAME.CXX
MRESULT EXPENTRY SalFrameWndProc( HWND hWnd, PM_ULONG nMsg, MPARAM nMP1, MPARAM nMP2 );
MRESULT EXPENTRY SalFrameFrameProc( HWND hWnd, PM_ULONG nMsg, MPARAM nMP1, MPARAM nMP2 );

// \\OS2\SOURCE\WINDOW\SALFRAME.CXX
// return Frame for Message-Handling
SalFrame* GetSalDefaultFrame();

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

#define SAL_FRAME_WNDEXTRA          sizeof(PM_ULONG)
#define SAL_FRAME_THIS              0
#define SAL_FRAME_CLASSNAME         "SALFRAME"
#define SAL_OBJECT_WNDEXTRA         sizeof(PM_ULONG)
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

// SysChild-ToTop; nMP1 = 0; nMP2 = 0
#define SALOBJ_MSG_TOTOP            (WM_USER+150)
// POSTFOCUS-Message; MP1 == nMP1; MP2 == nMP2 (SHORT1( bFocus ), 0)
#define SALOBJ_MSG_POSTFOCUS        (WM_USER+151)

// -----------------
// - Helpfunctions -
// -----------------

inline void SetWindowPtr( HWND hWnd, SalFrame* pThis )
{
    WinSetWindowULong( hWnd, SAL_FRAME_THIS, (PM_ULONG)pThis );
}

inline SalFrame* GetWindowPtr( HWND hWnd )
{
    return (SalFrame*)WinQueryWindowULong( hWnd, SAL_FRAME_THIS );
}

inline void SetSalObjWindowPtr( HWND hWnd, SalObject* pThis )
{
    WinSetWindowULong( hWnd, SAL_OBJECT_THIS, (PM_ULONG)pThis );
}

inline SalObject* GetSalObjWindowPtr( HWND hWnd )
{
    return (SalObject*)WinQueryWindowULong( hWnd, SAL_OBJECT_THIS );
}

#endif  // _SV_SALDATA_HXX
