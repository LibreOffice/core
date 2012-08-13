/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <string.h>
#include <svsys.h>
#include <process.h>

#include <osl/file.hxx>
#include <osl/mutex.hxx>

#include <tools/solarmutex.hxx>

#include <vcl/timer.hxx>
#include <vcl/apptypes.hxx>

#include <vcl/solarmutex.hxx>
#include <win/wincomp.hxx>
#include <win/salids.hrc>
#include <win/saldata.hxx>
#include <win/salinst.h>
#include <win/salframe.h>
#include <win/salobj.h>
#include <win/saltimer.h>
#include <win/salbmp.h>

#include <salimestatus.hxx>
#include <salsys.hxx>

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning( disable: 4917 )
#endif

#include <gdiplus.h>
#include <gdiplusenums.h>
#include <gdipluscolor.h>
#include <shlobj.h>

#if defined _MSC_VER
#pragma warning(pop)
#endif

#ifdef __MINGW32__
#include <sehandler.hxx>
#endif

// =======================================================================

void SalAbort( const rtl::OUString& rErrorText, bool )
{
    ImplFreeSalGDI();

    if ( rErrorText.isEmpty() )
    {
        // make sure crash reporter is triggered
        RaiseException( 0, EXCEPTION_NONCONTINUABLE, 0, NULL );
        FatalAppExitW( 0, L"Application Error" );
    }
    else
    {
        // make sure crash reporter is triggered
        RaiseException( 0, EXCEPTION_NONCONTINUABLE, 0, NULL );
        FatalAppExitW( 0, reinterpret_cast<LPCWSTR>(rErrorText.getStr()) );
    }
}

// =======================================================================

LRESULT CALLBACK SalComWndProcA( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK SalComWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );

// =======================================================================

class SalYieldMutex : public vcl::SolarMutexObject
{
public: // for ImplSalYield()
    WinSalInstance*             mpInstData;
    sal_uLong                       mnCount;
    DWORD                       mnThreadId;

public:
                                SalYieldMutex( WinSalInstance* pInstData );

    virtual void SAL_CALL       acquire();
    virtual void SAL_CALL       release();
    virtual sal_Bool SAL_CALL   tryToAcquire();

    sal_uLong                       GetAcquireCount( sal_uLong nThreadId );
};

// -----------------------------------------------------------------------

SalYieldMutex::SalYieldMutex( WinSalInstance* pInstData )
{
    mpInstData  = pInstData;
    mnCount     = 0;
    mnThreadId  = 0;
}

// -----------------------------------------------------------------------

void SAL_CALL SalYieldMutex::acquire()
{
    SolarMutexObject::acquire();
    mnCount++;
    mnThreadId = GetCurrentThreadId();
}

// -----------------------------------------------------------------------

void SAL_CALL SalYieldMutex::release()
{
    DWORD nThreadId = GetCurrentThreadId();
    if ( mnThreadId != nThreadId )
        SolarMutexObject::release();
    else
    {
        SalData* pSalData = GetSalData();
        if ( pSalData->mnAppThreadId != nThreadId )
        {
            if ( mnCount == 1 )
            {
                // If we don't call these message, the Output from the
                // Java clients doesn't come in the right order
                GdiFlush();

                mpInstData->mpSalWaitMutex->acquire();
                if ( mpInstData->mnYieldWaitCount )
                    ImplPostMessage( mpInstData->mhComWnd, SAL_MSG_RELEASEWAITYIELD, 0, 0 );
                mnThreadId = 0;
                mnCount--;
                SolarMutexObject::release();
                mpInstData->mpSalWaitMutex->release();
            }
            else
            {
                mnCount--;
                SolarMutexObject::release();
            }
        }
        else
        {
            if ( mnCount == 1 )
                mnThreadId = 0;
            mnCount--;
            SolarMutexObject::release();
        }
    }
}

// -----------------------------------------------------------------------

sal_Bool SAL_CALL SalYieldMutex::tryToAcquire()
{
    if( SolarMutexObject::tryToAcquire() )
    {
        mnCount++;
        mnThreadId = GetCurrentThreadId();
        return sal_True;
    }
    else
        return sal_False;
}

// -----------------------------------------------------------------------

sal_uLong SalYieldMutex::GetAcquireCount( sal_uLong nThreadId )
{
    if ( nThreadId == mnThreadId )
        return mnCount;
    else
        return 0;
}

// -----------------------------------------------------------------------

void ImplSalYieldMutexAcquireWithWait()
{
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( !pInst )
        return;

    // If we are the main thread, then we must wait with wait, because
    // in if we don't reschedule, then we create deadlocks if a Windows
    // Function is called from another thread. If we arn't the main thread,
    // than we call qcquire directly.
    DWORD nThreadId = GetCurrentThreadId();
    SalData* pSalData = GetSalData();
    if ( pSalData->mnAppThreadId == nThreadId )
    {
        // Wenn wir den Mutex nicht bekommen, muessen wir solange
        // warten, bis wir Ihn bekommen
        sal_Bool bAcquire = FALSE;
        do
        {
            if ( pInst->mpSalYieldMutex->tryToAcquire() )
                bAcquire = TRUE;
            else
            {
                pInst->mpSalWaitMutex->acquire();
                if ( pInst->mpSalYieldMutex->tryToAcquire() )
                {
                    bAcquire = TRUE;
                    pInst->mpSalWaitMutex->release();
                }
                else
                {
                    pInst->mnYieldWaitCount++;
                    pInst->mpSalWaitMutex->release();
                    MSG aTmpMsg;
                    ImplGetMessage( &aTmpMsg, pInst->mhComWnd, SAL_MSG_RELEASEWAITYIELD, SAL_MSG_RELEASEWAITYIELD );
                    pInst->mnYieldWaitCount--;
                    if ( pInst->mnYieldWaitCount )
                        ImplPostMessage( pInst->mhComWnd, SAL_MSG_RELEASEWAITYIELD, 0, 0 );
                }
            }
        }
        while ( !bAcquire );
    }
    else
        pInst->mpSalYieldMutex->acquire();
}

// -----------------------------------------------------------------------

sal_Bool ImplSalYieldMutexTryToAcquire()
{
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
        return pInst->mpSalYieldMutex->tryToAcquire();
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void ImplSalYieldMutexAcquire()
{
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
        pInst->mpSalYieldMutex->acquire();
}

// -----------------------------------------------------------------------

void ImplSalYieldMutexRelease()
{
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
    {
        GdiFlush();
        pInst->mpSalYieldMutex->release();
    }
}

// -----------------------------------------------------------------------

sal_uLong ImplSalReleaseYieldMutex()
{
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( !pInst )
        return 0;

    SalYieldMutex*  pYieldMutex = pInst->mpSalYieldMutex;
    sal_uLong           nCount = pYieldMutex->GetAcquireCount( GetCurrentThreadId() );
    sal_uLong           n = nCount;
    while ( n )
    {
        pYieldMutex->release();
        n--;
    }

    return nCount;
}

// -----------------------------------------------------------------------

void ImplSalAcquireYieldMutex( sal_uLong nCount )
{
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( !pInst )
        return;

    SalYieldMutex*  pYieldMutex = pInst->mpSalYieldMutex;
    while ( nCount )
    {
        pYieldMutex->acquire();
        nCount--;
    }
}

// -----------------------------------------------------------------------

bool WinSalInstance::CheckYieldMutex()
{
    SalData*    pSalData = GetSalData();
    if ( pSalData->mpFirstInstance )
    {
        SalYieldMutex* pYieldMutex = pSalData->mpFirstInstance->mpSalYieldMutex;
        return (pYieldMutex->mnThreadId == (GetCurrentThreadId()));
    }
    return true;
}

// =======================================================================

void SalData::initKeyCodeMap()
{
    UINT nKey = 0xffffffff;
    #define initKey( a, b )\
        nKey = LOWORD( VkKeyScan( a ) );\
        if( nKey < 0xffff )\
            maVKMap[ nKey ] = b;

    initKey( '+', KEY_ADD );
    initKey( '-', KEY_SUBTRACT );
    initKey( '*', KEY_MULTIPLY );
    initKey( '/', KEY_DIVIDE );
    initKey( '.', KEY_POINT );
    initKey( ',', KEY_COMMA );
    initKey( '<', KEY_LESS );
    initKey( '>', KEY_GREATER );
    initKey( '=', KEY_EQUAL );
    initKey( '~', KEY_TILDE );
    initKey( '`', KEY_QUOTELEFT );
    initKey( '[', KEY_BRACKETLEFT );
    initKey( ']', KEY_BRACKETRIGHT );
    initKey( ';', KEY_SEMICOLON );
}

// =======================================================================
// -------
// SalData
// -------

SalData::SalData()
{
    mhInst = 0;                 // default instance handle
    mhPrevInst = 0;             // previous instance handle
    mnCmdShow = 0;              // default frame show style
    mhDitherPal = 0;            // dither palette
    mhDitherDIB = 0;            // dither memory handle
    mpDitherDIB = 0;            // dither memory
    mpDitherDIBData = 0;        // beginning of DIB data
    mpDitherDiff = 0;           // Dither mapping table
    mpDitherLow = 0;            // Dither mapping table
    mpDitherHigh = 0;           // Dither mapping table
    mnTimerMS = 0;              // Current Time (in MS) of the Timer
    mnTimerOrgMS = 0;           // Current Original Time (in MS)
    mnNextTimerTime = 0;
    mnLastEventTime = 0;
    mnTimerId = 0;              // windows timer id
    mbInTimerProc = FALSE;      // timer event is currently being dispatched
    mhSalObjMsgHook = 0;        // hook to get interesting msg for SalObject
    mhWantLeaveMsg = 0;         // window handle, that want a MOUSELEAVE message
    mpMouseLeaveTimer = 0;      // Timer for MouseLeave Test
    mpFirstInstance = 0;        // pointer of first instance
    mpFirstFrame = 0;           // pointer of first frame
    mpFirstObject = 0;          // pointer of first object window
    mpFirstVD = 0;              // first VirDev
    mpFirstPrinter = 0;         // first printing printer
    mpHDCCache = 0;             // Cache for three DC's
    mh50Bmp = 0;                // 50% Bitmap
    mh50Brush = 0;              // 50% Brush
    int i;
    for(i=0; i<MAX_STOCKPEN; i++)
    {
        maStockPenColorAry[i] = 0;
        mhStockPenAry[i] = 0;
    }
    for(i=0; i<MAX_STOCKBRUSH; i++)
    {
        maStockBrushColorAry[i] = 0;
        mhStockBrushAry[i] = 0;
    }
    mnStockPenCount = 0;        // count of static pens
    mnStockBrushCount = 0;      // count of static brushes
    mnSalObjWantKeyEvt = 0;     // KeyEvent, welcher vom SalObj-Hook verarbeitet werden soll
    mnCacheDCInUse = 0;         // count of CacheDC in use
    mbObjClassInit = FALSE;     // is SALOBJECTCLASS initialised
    mbInPalChange = FALSE;      // is in WM_QUERYNEWPALETTE
    mnAppThreadId = 0;          // Id from Applikation-Thread
    mbScrSvrEnabled = FALSE;    // ScreenSaver enabled
    mnSageStatus = 0;           // status of Sage-DLL (DISABLE_AGENT == nicht vorhanden)
    mpSageEnableProc = 0;       // funktion to deactivate the system agent
    mpFirstIcon = 0;            // icon cache, points to first icon, NULL if none
    mpTempFontItem = 0;
    mbThemeChanged = FALSE;     // true if visual theme was changed: throw away theme handles
    mbThemeMenuSupport = FALSE;

    // init with NULL
    gdiplusToken = 0;
    maDwmLib     = 0;
    mpDwmIsCompositionEnabled = 0;

    initKeyCodeMap();

    SetSalData( this );
    initNWF();
}

SalData::~SalData()
{
    deInitNWF();
    SetSalData( NULL );
}

void InitSalData()
{
    SalData* pSalData = new SalData;
    CoInitialize(0);

    // init GDIPlus
    static Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&pSalData->gdiplusToken, &gdiplusStartupInput, NULL);
}


void DeInitSalData()
{
    CoUninitialize();
    SalData* pSalData = GetSalData();

    // deinit GDIPlus
    if(pSalData)
    {
        Gdiplus::GdiplusShutdown(pSalData->gdiplusToken);
    }

    delete pSalData;
}

// -----------------------------------------------------------------------

void InitSalMain()
{
    // remember data, copied from WinMain
    SalData* pData = GetSalData();
    if ( pData )    // Im AppServer NULL
    {
        STARTUPINFO aSI;
        aSI.cb = sizeof( aSI );
        GetStartupInfo( &aSI );
        pData->mhInst                   = GetModuleHandle( NULL );
        pData->mhPrevInst               = NULL;
        pData->mnCmdShow                = aSI.wShowWindow;
    }
}

void DeInitSalMain()
{
}

// -----------------------------------------------------------------------

SalInstance* CreateSalInstance()
{
    SalData* pSalData = GetSalData();

    // determine the windows version
    aSalShlData.mbWXP        = 0;
    memset( &aSalShlData.maVersionInfo, 0, sizeof(aSalShlData.maVersionInfo) );
    aSalShlData.maVersionInfo.dwOSVersionInfoSize = sizeof( aSalShlData.maVersionInfo );
    if ( GetVersionEx( &aSalShlData.maVersionInfo ) )
    {
        // Windows XP ?
        if ( aSalShlData.maVersionInfo.dwMajorVersion > 5 ||
           ( aSalShlData.maVersionInfo.dwMajorVersion == 5 && aSalShlData.maVersionInfo.dwMinorVersion >= 1 ) )
            aSalShlData.mbWXP = 1;
    }

    pSalData->mnAppThreadId = GetCurrentThreadId();

    // register frame class
    if ( !pSalData->mhPrevInst )
    {
        WNDCLASSEXW aWndClassEx;
        aWndClassEx.cbSize          = sizeof( aWndClassEx );
        aWndClassEx.style           = CS_OWNDC;
        aWndClassEx.lpfnWndProc     = SalFrameWndProcW;
        aWndClassEx.cbClsExtra      = 0;
        aWndClassEx.cbWndExtra      = SAL_FRAME_WNDEXTRA;
        aWndClassEx.hInstance       = pSalData->mhInst;
        aWndClassEx.hCursor         = 0;
        aWndClassEx.hbrBackground   = 0;
        aWndClassEx.lpszMenuName    = 0;
        aWndClassEx.lpszClassName   = SAL_FRAME_CLASSNAMEW;
        ImplLoadSalIcon( SAL_RESID_ICON_DEFAULT, aWndClassEx.hIcon, aWndClassEx.hIconSm );
        if ( !RegisterClassExW( &aWndClassEx ) )
            return NULL;

        aWndClassEx.hIcon           = 0;
        aWndClassEx.hIconSm         = 0;
        aWndClassEx.style          |= CS_SAVEBITS;
        aWndClassEx.lpszClassName   = SAL_SUBFRAME_CLASSNAMEW;
        if ( !RegisterClassExW( &aWndClassEx ) )
            return NULL;

        // shadow effect for popups on XP
        if( aSalShlData.mbWXP )
            aWndClassEx.style       |= CS_DROPSHADOW;
        aWndClassEx.lpszClassName   = SAL_TMPSUBFRAME_CLASSNAMEW;
        if ( !RegisterClassExW( &aWndClassEx ) )
            return NULL;

        aWndClassEx.style           = 0;
        aWndClassEx.lpfnWndProc     = SalComWndProcW;
        aWndClassEx.cbWndExtra      = 0;
        aWndClassEx.lpszClassName   = SAL_COM_CLASSNAMEW;
        if ( !RegisterClassExW( &aWndClassEx ) )
            return NULL;
    }

    HWND hComWnd = CreateWindowExW( WS_EX_TOOLWINDOW, SAL_COM_CLASSNAMEW,
                               L"", WS_POPUP, 0, 0, 0, 0, 0, 0,
                               pSalData->mhInst, NULL );
    if ( !hComWnd )
        return NULL;

    WinSalInstance* pInst = new WinSalInstance;

    // init instance (only one instance in this version !!!)
    pSalData->mpFirstInstance   = pInst;
    pInst->mhInst    = pSalData->mhInst;
    pInst->mhComWnd  = hComWnd;

    // init static GDI Data
    ImplInitSalGDI();

    return pInst;
}

// -----------------------------------------------------------------------

void DestroySalInstance( SalInstance* pInst )
{
    SalData* pSalData = GetSalData();

    //  (only one instance in this version !!!)

    ImplFreeSalGDI();

    // reset instance
    if ( pSalData->mpFirstInstance == pInst )
        pSalData->mpFirstInstance = NULL;

    delete pInst;
}

// -----------------------------------------------------------------------

WinSalInstance::WinSalInstance()
{
    mhComWnd                 = 0;
    mpSalYieldMutex          = new SalYieldMutex( this );
    mpSalWaitMutex           = new osl::Mutex;
    mnYieldWaitCount         = 0;
    mpSalYieldMutex->acquire();
    ::tools::SolarMutex::SetSolarMutex( mpSalYieldMutex );
}

// -----------------------------------------------------------------------

WinSalInstance::~WinSalInstance()
{
    ::tools::SolarMutex::SetSolarMutex( 0 );
    mpSalYieldMutex->release();
    delete mpSalYieldMutex;
    delete mpSalWaitMutex;
    DestroyWindow( mhComWnd );
}

// -----------------------------------------------------------------------

osl::SolarMutex* WinSalInstance::GetYieldMutex()
{
    return mpSalYieldMutex;
}

// -----------------------------------------------------------------------

sal_uLong WinSalInstance::ReleaseYieldMutex()
{
    return ImplSalReleaseYieldMutex();
}

// -----------------------------------------------------------------------

void WinSalInstance::AcquireYieldMutex( sal_uLong nCount )
{
    ImplSalAcquireYieldMutex( nCount );
}

// -----------------------------------------------------------------------

static void ImplSalDispatchMessage( MSG* pMsg )
{
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstObject )
    {
        if ( ImplSalPreDispatchMsg( pMsg ) )
            return;
    }
    LRESULT lResult = ImplDispatchMessage( pMsg );
    if ( pSalData->mpFirstObject )
        ImplSalPostDispatchMsg( pMsg, lResult );
}

// -----------------------------------------------------------------------

void ImplSalYield( sal_Bool bWait, sal_Bool bHandleAllCurrentEvents )
{
    MSG aMsg;
    bool bWasMsg = false, bOneEvent = false;

    int nMaxEvents = bHandleAllCurrentEvents ? 100 : 1;
    do
    {
        if ( ImplPeekMessage( &aMsg, 0, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &aMsg );
            ImplSalDispatchMessage( &aMsg );

            bOneEvent = bWasMsg = true;
        }
        else
            bOneEvent = false;
    } while( --nMaxEvents && bOneEvent );

    if ( bWait && ! bWasMsg )
    {
        if ( ImplGetMessage( &aMsg, 0, 0, 0 ) )
        {
            TranslateMessage( &aMsg );
            ImplSalDispatchMessage( &aMsg );
        }
    }
}

// -----------------------------------------------------------------------

void WinSalInstance::Yield( bool bWait, bool bHandleAllCurrentEvents )
{
    SalYieldMutex*  pYieldMutex = mpSalYieldMutex;
    SalData*        pSalData = GetSalData();
    DWORD           nCurThreadId = GetCurrentThreadId();
    sal_uLong           nCount = pYieldMutex->GetAcquireCount( nCurThreadId );
    sal_uLong           n = nCount;
    while ( n )
    {
        pYieldMutex->release();
        n--;
    }
    if ( pSalData->mnAppThreadId != nCurThreadId )
    {
        // #97739# A SendMessage call blocks until the called thread (here: the main thread)
        // returns. During a yield however, messages are processed in the main thread that might
        // result in a new message loop due to opening a dialog. Thus, SendMessage would not
        // return which will block this thread!
        // Solution: just give up the time slice and hope that messages are processed
        // by the main thread anyway (where all windows are created)
        // If the mainthread is not currently handling messages, then our SendMessage would
        // also do nothing, so this seems to be reasonable.

        // #i18883# only sleep if potential deadlock scenario, ie, when a dialog is open
        if( ImplGetSVData()->maAppData.mnModalMode )
            Sleep(1);
        else
            ImplSendMessage( mhComWnd, SAL_MSG_THREADYIELD, (WPARAM)bWait, (LPARAM)bHandleAllCurrentEvents );

        n = nCount;
        while ( n )
        {
            pYieldMutex->acquire();
            n--;
        }
    }
    else
    {
        ImplSalYield( bWait, bHandleAllCurrentEvents );

        n = nCount;
        while ( n )
        {
            ImplSalYieldMutexAcquireWithWait();
            n--;
        }
    }
}

// -----------------------------------------------------------------------

LRESULT CALLBACK SalComWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam, int& rDef )
{
    LRESULT nRet = 0;


    switch ( nMsg )
    {
        case SAL_MSG_PRINTABORTJOB:
            ImplSalPrinterAbortJobAsync( (HDC)wParam );
            rDef = FALSE;
            break;
        case SAL_MSG_THREADYIELD:
            ImplSalYield( (sal_Bool)wParam, (sal_Bool)lParam );
            rDef = FALSE;
            break;
        // If we get this message, because another GetMessage() call
        // has recieved this message, we must post this message to
        // us again, because in the other case we wait forever.
        case SAL_MSG_RELEASEWAITYIELD:
            {
            WinSalInstance* pInst = GetSalData()->mpFirstInstance;
            if ( pInst && pInst->mnYieldWaitCount )
                ImplPostMessage( hWnd, SAL_MSG_RELEASEWAITYIELD, wParam, lParam );
            }
            rDef = FALSE;
            break;
        case SAL_MSG_STARTTIMER:
            ImplSalStartTimer( (sal_uLong) lParam, FALSE );
            rDef = FALSE;
            break;
        case SAL_MSG_CREATEFRAME:
            nRet = (LRESULT)ImplSalCreateFrame( GetSalData()->mpFirstInstance, (HWND)lParam, (sal_uLong)wParam );
            rDef = FALSE;
            break;
        case SAL_MSG_RECREATEHWND:
            nRet = (LRESULT)ImplSalReCreateHWND( (HWND)wParam, (HWND)lParam, FALSE );
            rDef = FALSE;
            break;
        case SAL_MSG_RECREATECHILDHWND:
            nRet = (LRESULT)ImplSalReCreateHWND( (HWND)wParam, (HWND)lParam, TRUE );
            rDef = FALSE;
            break;
        case SAL_MSG_DESTROYFRAME:
            delete (SalFrame*)lParam;
            rDef = FALSE;
            break;
        case SAL_MSG_DESTROYHWND:
            //We only destroy the native window here. We do NOT destroy the SalFrame contained
            //in the structure (GetWindowPtr()).
            if (DestroyWindow((HWND)lParam) == 0)
            {
                OSL_FAIL("DestroyWindow failed!");
                //Failure: We remove the SalFrame from the window structure. So we avoid that
                // the window structure may contain an invalid pointer, once the SalFrame is deleted.
               SetWindowPtr((HWND)lParam, 0);
            }
            rDef = FALSE;
            break;
        case SAL_MSG_CREATEOBJECT:
            nRet = (LRESULT)ImplSalCreateObject( GetSalData()->mpFirstInstance, (WinSalFrame*)lParam );
            rDef = FALSE;
            break;
        case SAL_MSG_DESTROYOBJECT:
            delete (SalObject*)lParam;
            rDef = FALSE;
            break;
        case SAL_MSG_GETDC:
            nRet = (LRESULT)GetDCEx( (HWND)wParam, 0, DCX_CACHE );
            rDef = FALSE;
            break;
        case SAL_MSG_RELEASEDC:
            ReleaseDC( (HWND)wParam, (HDC)lParam );
            rDef = FALSE;
            break;
        case SAL_MSG_POSTTIMER:
            SalTimerProc( 0, 0, SALTIMERPROC_RECURSIVE, lParam );
            break;
    }

    return nRet;
}

LRESULT CALLBACK SalComWndProcA( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = 0;
#ifdef __MINGW32__
    jmp_buf jmpbuf;
    __SEHandler han;
    if (__builtin_setjmp(jmpbuf) == 0)
    {
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
    __try
    {
#endif
        nRet = SalComWndProc( hWnd, nMsg, wParam, lParam, bDef );
    }
#ifdef __MINGW32__
    han.Reset();
#else
    __except(WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(GetExceptionCode(), GetExceptionInformation()))
    {
    }
#endif
    if ( bDef )
    {
        if ( !ImplHandleGlobalMsg( hWnd, nMsg, wParam, lParam, nRet ) )
            nRet = DefWindowProcA( hWnd, nMsg, wParam, lParam );
    }
    return nRet;
}

LRESULT CALLBACK SalComWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = 0;
#ifdef __MINGW32__
    jmp_buf jmpbuf;
    __SEHandler han;
    if (__builtin_setjmp(jmpbuf) == 0)
    {
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
    __try
    {
#endif
        nRet = SalComWndProc( hWnd, nMsg, wParam, lParam, bDef );
    }
#ifdef __MINGW32__
    han.Reset();
#else
    __except(WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(GetExceptionCode(), GetExceptionInformation()))
    {
    }
#endif
    if ( bDef )
    {
        if ( !ImplHandleGlobalMsg( hWnd, nMsg, wParam, lParam, nRet ) )
            nRet = DefWindowProcW( hWnd, nMsg, wParam, lParam );
    }
    return nRet;
}

// -----------------------------------------------------------------------

bool WinSalInstance::AnyInput( sal_uInt16 nType )
{
    MSG aMsg;

    if ( (nType & (VCL_INPUT_ANY)) == (VCL_INPUT_ANY) )
    {
        // revert bugfix for #108919# which never reported timeouts when called from the timer handler
        // which made the application completely unresponsive during background formatting
        if ( ImplPeekMessage( &aMsg, 0, 0, 0, PM_NOREMOVE | PM_NOYIELD ) )
            return true;
    }
    else
    {
        if ( nType & VCL_INPUT_MOUSE )
        {
            // Test for mouse input
            if ( ImplPeekMessage( &aMsg, 0, WM_MOUSEFIRST, WM_MOUSELAST,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;
        }

        if ( nType & VCL_INPUT_KEYBOARD )
        {
            // Test for key input
            if ( ImplPeekMessage( &aMsg, 0, WM_KEYDOWN, WM_KEYDOWN,
                                  PM_NOREMOVE | PM_NOYIELD ) )
            {
                if ( (aMsg.wParam == VK_SHIFT)   ||
                     (aMsg.wParam == VK_CONTROL) ||
                     (aMsg.wParam == VK_MENU) )
                    return false;
                else
                    return true;
            }
        }

        if ( nType & VCL_INPUT_PAINT )
        {
            // Test for paint input
            if ( ImplPeekMessage( &aMsg, 0, WM_PAINT, WM_PAINT,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;

            if ( ImplPeekMessage( &aMsg, 0, WM_SIZE, WM_SIZE,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;

            if ( ImplPeekMessage( &aMsg, 0, SAL_MSG_POSTCALLSIZE, SAL_MSG_POSTCALLSIZE,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;

            if ( ImplPeekMessage( &aMsg, 0, WM_MOVE, WM_MOVE,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;

            if ( ImplPeekMessage( &aMsg, 0, SAL_MSG_POSTMOVE, SAL_MSG_POSTMOVE,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;
        }

        if ( nType & VCL_INPUT_TIMER )
        {
            // Test for timer input
            if ( ImplPeekMessage( &aMsg, 0, WM_TIMER, WM_TIMER,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;

        }

        if ( nType & VCL_INPUT_OTHER )
        {
            // Test for any input
            if ( ImplPeekMessage( &aMsg, 0, 0, 0, PM_NOREMOVE | PM_NOYIELD ) )
                return true;
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void SalTimer::Start( sal_uLong nMS )
{
    // Um auf Main-Thread umzuschalten
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstInstance )
    {
        if ( pSalData->mnAppThreadId != GetCurrentThreadId() )
            ImplPostMessage( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
        else
            ImplSendMessage( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
    }
    else
        ImplSalStartTimer( nMS, FALSE );
}

// -----------------------------------------------------------------------

SalFrame* WinSalInstance::CreateChildFrame( SystemParentData* pSystemParentData, sal_uLong nSalFrameStyle )
{
    // Um auf Main-Thread umzuschalten
    return (SalFrame*)ImplSendMessage( mhComWnd, SAL_MSG_CREATEFRAME, nSalFrameStyle, (LPARAM)pSystemParentData->hWnd );
}

// -----------------------------------------------------------------------

SalFrame* WinSalInstance::CreateFrame( SalFrame* pParent, sal_uLong nSalFrameStyle )
{
    // Um auf Main-Thread umzuschalten
    HWND hWndParent;
    if ( pParent )
        hWndParent = static_cast<WinSalFrame*>(pParent)->mhWnd;
    else
        hWndParent = 0;
    return (SalFrame*)ImplSendMessage( mhComWnd, SAL_MSG_CREATEFRAME, nSalFrameStyle, (LPARAM)hWndParent );
}

// -----------------------------------------------------------------------

void WinSalInstance::DestroyFrame( SalFrame* pFrame )
{
    ImplSendMessage( mhComWnd, SAL_MSG_DESTROYFRAME, 0, (LPARAM)pFrame );
}

// -----------------------------------------------------------------------

SalObject* WinSalInstance::CreateObject( SalFrame* pParent,
                                         SystemWindowData* /*pWindowData*/, // SystemWindowData meaningless on Windows
                                         sal_Bool /*bShow*/ )
{
    // Um auf Main-Thread umzuschalten
    return (SalObject*)ImplSendMessage( mhComWnd, SAL_MSG_CREATEOBJECT, 0, (LPARAM)static_cast<WinSalFrame*>(pParent) );
}

// -----------------------------------------------------------------------

void WinSalInstance::DestroyObject( SalObject* pObject )
{
    ImplSendMessage( mhComWnd, SAL_MSG_DESTROYOBJECT, 0, (LPARAM)pObject );
}

// -----------------------------------------------------------------------

void* WinSalInstance::GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes )
{
    rReturnedBytes  = 1;
    rReturnedType   = AsciiCString;
    return const_cast<char *>("");
}

// -----------------------------------------------------------------------

/** Add a file to the system shells recent document list if there is any.
      This function may have no effect under Unix because there is no
      standard API among the different desktop managers.

      @param aFileUrl
                The file url of the document.
*/
void WinSalInstance::AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& /*rMimeType*/)
{
    rtl::OUString system_path;
    osl::FileBase::RC rc = osl::FileBase::getSystemPathFromFileURL(rFileUrl, system_path);

    OSL_ENSURE(osl::FileBase::E_None == rc, "Invalid file url");

    if (osl::FileBase::E_None == rc)
        SHAddToRecentDocs(SHARD_PATHW, system_path.getStr());
}

// -----------------------------------------------------------------------

SalTimer* WinSalInstance::CreateSalTimer()
{
    return new WinSalTimer();
}

// -----------------------------------------------------------------------

SalBitmap* WinSalInstance::CreateSalBitmap()
{
    return new WinSalBitmap();
}

class WinImeStatus : public SalI18NImeStatus
{
  public:
    WinImeStatus() {}
    virtual ~WinImeStatus() {}

    // asks whether there is a status window available
    // to toggle into menubar
    virtual bool canToggle() { return false; }
    virtual void toggle() {}
};

SalI18NImeStatus* WinSalInstance::CreateI18NImeStatus()
{
    return new WinImeStatus();
}

// -----------------------------------------------------------------------

const ::rtl::OUString& SalGetDesktopEnvironment()
{
    static ::rtl::OUString aDesktopEnvironment( RTL_CONSTASCII_USTRINGPARAM( "Windows" ) );
    return aDesktopEnvironment;
}

SalSession* WinSalInstance::CreateSalSession()
{
    return NULL;
}

#ifndef __MINGW32__
// -----------------------------------------------------------------------
int WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(int, LPEXCEPTION_POINTERS pExceptionInfo)
{
    // Decide if an exception is a c++ (mostly UNO) exception or a process violation.
    // Depending on this information we pass process violations directly to our signal handler ...
    // and c++ (UNO) exceptions are sended to the following code on the current stack.
    // Problem behind: user32.dll sometime consumes exceptions/process violations .-)
    // see also #112221#

    static DWORD EXCEPTION_MSC_CPP_EXCEPTION = 0xE06D7363;

    if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_MSC_CPP_EXCEPTION)
        return EXCEPTION_CONTINUE_SEARCH;

    return UnhandledExceptionFilter( pExceptionInfo );
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
