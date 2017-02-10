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

#include <string.h>
#include <svsys.h>
#include <process.h>

#include <osl/conditn.hxx>
#include <osl/file.hxx>
#include <comphelper/solarmutex.hxx>

#include <vcl/inputtypes.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/timer.hxx>

#include "opengl/salbmp.hxx"
#include "win/wincomp.hxx"
#include "win/salids.hrc"
#include "win/saldata.hxx"
#include "win/salinst.h"
#include "win/salframe.h"
#include "win/salobj.h"
#include "win/saltimer.h"
#include "win/salbmp.h"

#include "salimestatus.hxx"
#include "salsys.hxx"

#include <desktop/crashreport.hxx>

#if defined _MSC_VER
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning( disable: 4917 )
#endif

#include "prewin.h"

#include <gdiplus.h>
#include <shlobj.h>

#ifdef _WIN32_WINNT_WINBLUE
#include <VersionHelpers.h>
#endif
#include "postwin.h"

#if defined _MSC_VER
#pragma warning(pop)
#endif

void SalAbort( const OUString& rErrorText, bool )
{
    ImplFreeSalGDI();

    if ( rErrorText.isEmpty() )
    {
        // make sure crash reporter is triggered
        RaiseException( 0, EXCEPTION_NONCONTINUABLE, 0, nullptr );
        FatalAppExitW( 0, L"Application Error" );
    }
    else
    {
        CrashReporter::AddKeyValue("AbortMessage", rErrorText);
        // make sure crash reporter is triggered
        RaiseException( 0, EXCEPTION_NONCONTINUABLE, 0, nullptr );
        FatalAppExitW( 0, reinterpret_cast<LPCWSTR>(rErrorText.getStr()) );
    }
}

LRESULT CALLBACK SalComWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );

class SalYieldMutex : public comphelper::SolarMutex
{
public: // for ImplSalYield() and ImplSalYieldMutexAcquireWithWait()
    osl::Mutex                  m_mutex;
    osl::Condition              m_condition; /// for MsgWaitForMultipleObjects()
    WinSalInstance*             mpInstData;
    sal_uLong                   mnCount;
    DWORD                       mnThreadId;

public:
    explicit SalYieldMutex( WinSalInstance* pInstData );

    virtual void                acquire() override;
    virtual void                release() override;
    virtual bool                tryToAcquire() override;

    sal_uLong                   GetAcquireCount( sal_uLong nThreadId );
};

SalYieldMutex::SalYieldMutex( WinSalInstance* pInstData )
{
    mpInstData  = pInstData;
    mnCount     = 0;
    mnThreadId  = 0;
}

void SalYieldMutex::acquire()
{
    m_mutex.acquire();
    mnCount++;
    mnThreadId = GetCurrentThreadId();
}

void SalYieldMutex::release()
{
    DWORD nThreadId = GetCurrentThreadId();
    assert(mnThreadId == nThreadId);

    bool const isRelease(1 == mnCount);
    if ( isRelease )
    {
        OpenGLContext::prepareForYield();

        SalData* pSalData = GetSalData();
        if ( pSalData->mnAppThreadId != nThreadId )
        {
            // If we don't call these message, the Output from the
            // Java clients doesn't come in the right order
            GdiFlush();

        }
        mnThreadId = 0;
    }

    mnCount--;
    m_mutex.release();

    if ( isRelease )
    {   // do this *after* release
        m_condition.set(); // wake up ImplSalYieldMutexAcquireWithWait()
    }
}

bool SalYieldMutex::tryToAcquire()
{
    if( m_mutex.tryToAcquire() )
    {
        mnCount++;
        mnThreadId = GetCurrentThreadId();
        return true;
    }
    else
        return false;
}

sal_uLong SalYieldMutex::GetAcquireCount( sal_uLong nThreadId )
{
    if ( nThreadId == mnThreadId )
        return mnCount;
    else
        return 0;
}

/// note: while VCL is fully up and running (other threads started and
/// before shutdown), the main thread must acquire SolarMutex only via
/// this function to avoid deadlock
void ImplSalYieldMutexAcquireWithWait()
{
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( !pInst )
        return;

    DWORD nThreadId = GetCurrentThreadId();
    SalData* pSalData = GetSalData();

    if ( pSalData->mnAppThreadId == nThreadId )
    {
        // tdf#96887 If this is the main thread, then we must wait for two things:
        // - the mpSalYieldMutex being freed
        // - SendMessage() being triggered
        // This can nicely be done using MsgWaitForMultipleObjects. The 2nd one is
        // needed because if we don't reschedule, then we create deadlocks if a
        // Window's create/destroy is called via SendMessage() from another thread.
        // Have a look at the osl_waitCondition implementation for more info.
        pInst->mpSalYieldMutex->m_condition.reset();
        while (!pInst->mpSalYieldMutex->tryToAcquire())
        {
            // wait for SalYieldMutex::release() to set the condition
            osl::Condition::Result res = pInst->mpSalYieldMutex->m_condition.wait();
            assert(osl::Condition::Result::result_ok == res);
            // reset condition *before* acquiring!
            pInst->mpSalYieldMutex->m_condition.reset();
        }
    }
    else
    {
        // If this is not the main thread, call acquire directly.
        pInst->mpSalYieldMutex->acquire();
    }
}

bool ImplSalYieldMutexTryToAcquire()
{
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
        return pInst->mpSalYieldMutex->tryToAcquire();
    else
        return false;
}

void ImplSalYieldMutexRelease()
{
    WinSalInstance* pInst = GetSalData()->mpFirstInstance;
    if ( pInst )
    {
        GdiFlush();
        pInst->mpSalYieldMutex->release();
    }
}

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

void SalData::initKeyCodeMap()
{
    UINT nKey;
    #define initKey( a, b )\
        nKey = LOWORD( VkKeyScan( a ) );\
        if( nKey < 0xffff )\
            maVKMap[ nKey ] = b;

    maVKMap.clear();

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
    initKey( '\'', KEY_QUOTERIGHT );
}

// SalData

SalData::SalData()
{
    mhInst = nullptr;           // default instance handle
    mnCmdShow = 0;              // default frame show style
    mhDitherPal = nullptr;      // dither palette
    mhDitherDIB = nullptr;      // dither memory handle
    mpDitherDIB = nullptr;      // dither memory
    mpDitherDIBData = nullptr;  // beginning of DIB data
    mpDitherDiff = nullptr;     // Dither mapping table
    mpDitherLow = nullptr;      // Dither mapping table
    mpDitherHigh = nullptr;     // Dither mapping table
    mnTimerMS = 0;              // Current Time (in MS) of the Timer
    mnTimerOrgMS = 0;           // Current Original Time (in MS)
    mnNextTimerTime = 0;
    mnLastEventTime = 0;
    mnTimerId = nullptr;        // windows timer id
    mhSalObjMsgHook = nullptr;  // hook to get interesting msg for SalObject
    mhWantLeaveMsg = nullptr;   // window handle, that want a MOUSELEAVE message
    mpMouseLeaveTimer = nullptr; // Timer for MouseLeave Test
    mpFirstInstance = nullptr;  // pointer of first instance
    mpFirstFrame = nullptr;     // pointer of first frame
    mpFirstObject = nullptr;    // pointer of first object window
    mpFirstVD = nullptr;        // first VirDev
    mpFirstPrinter = nullptr;   // first printing printer
    mpHDCCache = nullptr;       // Cache for three DC's
    mh50Bmp = nullptr;          // 50% Bitmap
    mh50Brush = nullptr;        // 50% Brush
    int i;
    for(i=0; i<MAX_STOCKPEN; i++)
    {
        maStockPenColorAry[i] = 0;
        mhStockPenAry[i] = nullptr;
    }
    for(i=0; i<MAX_STOCKBRUSH; i++)
    {
        maStockBrushColorAry[i] = 0;
        mhStockBrushAry[i] = nullptr;
    }
    mnStockPenCount = 0;        // count of static pens
    mnStockBrushCount = 0;      // count of static brushes
    mnSalObjWantKeyEvt = 0;     // KeyEvent for the SalObj hook
    mnCacheDCInUse = 0;         // count of CacheDC in use
    mbObjClassInit = false;     // is SALOBJECTCLASS initialised
    mbInPalChange = false;      // is in WM_QUERYNEWPALETTE
    mnAppThreadId = 0;          // Id from Applikation-Thread
    mbScrSvrEnabled = FALSE;    // ScreenSaver enabled
    mpFirstIcon = nullptr;      // icon cache, points to first icon, NULL if none
    mpTempFontItem = nullptr;
    mbThemeChanged = false;     // true if visual theme was changed: throw away theme handles
    mbThemeMenuSupport = false;

    // init with NULL
    gdiplusToken = 0;

    initKeyCodeMap();

    SetSalData( this );
    initNWF();
}

SalData::~SalData()
{
    deInitNWF();
    SetSalData( nullptr );
}

void InitSalData()
{
    SalData* pSalData = new SalData;
    CoInitialize(nullptr); // put main thread in Single Threaded Apartment (STA)

    // init GDIPlus
    static Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&pSalData->gdiplusToken, &gdiplusStartupInput, nullptr);
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

void InitSalMain()
{
    // remember data, copied from WinMain
    SalData* pData = GetSalData();
    if ( pData )    // Im AppServer NULL
    {
        STARTUPINFO aSI;
        aSI.cb = sizeof( aSI );
        GetStartupInfo( &aSI );
        pData->mhInst                   = GetModuleHandle( nullptr );
        pData->mnCmdShow                = aSI.wShowWindow;
    }
}

SalInstance* CreateSalInstance()
{
    SalData* pSalData = GetSalData();

    // determine the windows version
    aSalShlData.mbWXP        = 0;
    aSalShlData.mbWVista     = 0;
    aSalShlData.mbW7         = 0;
// the Win32 SDK 8.1 deprecates GetVersionEx()
#ifdef _WIN32_WINNT_WINBLUE
    aSalShlData.mbWXP = IsWindowsXPOrGreater() ? 1 : 0;
    aSalShlData.mbWVista = IsWindowsVistaOrGreater() ? 1 : 0;
    aSalShlData.mbW7 = IsWindows7OrGreater() ? 1 : 0;
#else
    OSVERSIONINFO aVersionInfo;
    memset( &aVersionInfo, 0, sizeof(aVersionInfo) );
    aVersionInfo.dwOSVersionInfoSize = sizeof( aVersionInfo );
    if (GetVersionEx( &aVersionInfo ))
    {
        // Windows XP ?
        if (aVersionInfo.dwMajorVersion > 5 ||
           (aVersionInfo.dwMajorVersion == 5 && aVersionInfo.dwMinorVersion >= 1))
            aSalShlData.mbWXP = 1;
        // Windows Vista ?
        if (aVersionInfo.dwMajorVersion >= 6)
            aSalShlData.mbWVista = 1;
        // Windows 7 ?
        if (aVersionInfo.dwMajorVersion > 6 ||
           (aVersionInfo.dwMajorVersion == 6 && aVersionInfo.dwMinorVersion >= 1))
            aSalShlData.mbW7 = 1;
    }
#endif

    pSalData->mnAppThreadId = GetCurrentThreadId();

    // register frame class
    WNDCLASSEXW aWndClassEx;
    aWndClassEx.cbSize          = sizeof( aWndClassEx );
    aWndClassEx.style           = CS_OWNDC;
    aWndClassEx.lpfnWndProc     = SalFrameWndProcW;
    aWndClassEx.cbClsExtra      = 0;
    aWndClassEx.cbWndExtra      = SAL_FRAME_WNDEXTRA;
    aWndClassEx.hInstance       = pSalData->mhInst;
    aWndClassEx.hCursor         = nullptr;
    aWndClassEx.hbrBackground   = nullptr;
    aWndClassEx.lpszMenuName    = nullptr;
    aWndClassEx.lpszClassName   = SAL_FRAME_CLASSNAMEW;
    ImplLoadSalIcon( SAL_RESID_ICON_DEFAULT, aWndClassEx.hIcon, aWndClassEx.hIconSm );
    if ( !RegisterClassExW( &aWndClassEx ) )
        return nullptr;

    aWndClassEx.hIcon           = nullptr;
    aWndClassEx.hIconSm         = nullptr;
    aWndClassEx.style          |= CS_SAVEBITS;
    aWndClassEx.lpszClassName   = SAL_SUBFRAME_CLASSNAMEW;
    if ( !RegisterClassExW( &aWndClassEx ) )
        return nullptr;

    // shadow effect for popups on XP
    if( aSalShlData.mbWXP )
        aWndClassEx.style       |= CS_DROPSHADOW;
    aWndClassEx.lpszClassName   = SAL_TMPSUBFRAME_CLASSNAMEW;
    if ( !RegisterClassExW( &aWndClassEx ) )
        return nullptr;

    aWndClassEx.style           = 0;
    aWndClassEx.lpfnWndProc     = SalComWndProcW;
    aWndClassEx.cbWndExtra      = 0;
    aWndClassEx.lpszClassName   = SAL_COM_CLASSNAMEW;
    if ( !RegisterClassExW( &aWndClassEx ) )
        return nullptr;

    HWND hComWnd = CreateWindowExW( WS_EX_TOOLWINDOW, SAL_COM_CLASSNAMEW,
                               L"", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr,
                               pSalData->mhInst, nullptr );
    if ( !hComWnd )
        return nullptr;

    WinSalInstance* pInst = new WinSalInstance;

    // init instance (only one instance in this version !!!)
    pSalData->mpFirstInstance   = pInst;
    pInst->mhInst    = pSalData->mhInst;
    pInst->mhComWnd  = hComWnd;

    // init static GDI Data
    ImplInitSalGDI();

    return pInst;
}

void DestroySalInstance( SalInstance* pInst )
{
    SalData* pSalData = GetSalData();

    //  (only one instance in this version !!!)

    ImplFreeSalGDI();

    // reset instance
    if ( pSalData->mpFirstInstance == pInst )
        pSalData->mpFirstInstance = nullptr;

    delete pInst;
}

WinSalInstance::WinSalInstance()
{
    mhComWnd                 = nullptr;
    mpSalYieldMutex          = new SalYieldMutex( this );
    mpSalYieldMutex->acquire();
    ::comphelper::SolarMutex::setSolarMutex( mpSalYieldMutex );
}

WinSalInstance::~WinSalInstance()
{
    ::comphelper::SolarMutex::setSolarMutex( nullptr );
    mpSalYieldMutex->release();
    delete mpSalYieldMutex;
    DestroyWindow( mhComWnd );
}

comphelper::SolarMutex* WinSalInstance::GetYieldMutex()
{
    return mpSalYieldMutex;
}

sal_uLong WinSalInstance::ReleaseYieldMutex()
{
    return ImplSalReleaseYieldMutex();
}

void WinSalInstance::AcquireYieldMutex( sal_uLong nCount )
{
    ImplSalAcquireYieldMutex( nCount );
}

static void ImplSalDispatchMessage( MSG* pMsg )
{
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstObject )
    {
        if ( ImplSalPreDispatchMsg( pMsg ) )
            return;
    }
    LRESULT lResult = DispatchMessageW( pMsg );
    if ( pSalData->mpFirstObject )
        ImplSalPostDispatchMsg( pMsg, lResult );
}

SalYieldResult
ImplSalYield( bool bWait, bool bHandleAllCurrentEvents )
{
    MSG aMsg;
    bool bWasMsg = false, bOneEvent = false;

    int nMaxEvents = bHandleAllCurrentEvents ? 100 : 1;
    do
    {
        if ( PeekMessageW( &aMsg, nullptr, 0, 0, PM_REMOVE ) )
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
        if ( GetMessageW( &aMsg, nullptr, 0, 0 ) )
        {
            TranslateMessage( &aMsg );
            ImplSalDispatchMessage( &aMsg );
        }
    }
    return bWasMsg ? SalYieldResult::EVENT :
                     SalYieldResult::TIMEOUT;
}

SalYieldResult WinSalInstance::DoYield(bool bWait, bool bHandleAllCurrentEvents, sal_uLong const nReleased)
{
    SalYieldResult eDidWork = SalYieldResult::TIMEOUT;
    // NOTE: if nReleased != 0 this will be called without SolarMutex
    //       so don't do anything dangerous before releasing it here
    SalYieldMutex*  pYieldMutex = mpSalYieldMutex;
    SalData*        pSalData = GetSalData();
    DWORD           nCurThreadId = GetCurrentThreadId();
    sal_uLong const nCount = (nReleased != 0)
                                ? nReleased
                                : pYieldMutex->GetAcquireCount(nCurThreadId);
    sal_uLong       n = (nReleased != 0) ? 0 : nCount;
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
            SendMessageW( mhComWnd, SAL_MSG_THREADYIELD, (WPARAM)bWait, (LPARAM)bHandleAllCurrentEvents );

        n = nCount;
        while ( n )
        {
            pYieldMutex->acquire();
            n--;
        }
    }
    else
    {
        if (nReleased == 0) // tdf#99383 ReAcquireSolarMutex shouldn't Yield
        {
            eDidWork = ImplSalYield( bWait, bHandleAllCurrentEvents );
        }

        n = nCount;
        while ( n )
        {
            ImplSalYieldMutexAcquireWithWait();
            n--;
        }
    }
    return eDidWork;
}

LRESULT CALLBACK SalComWndProc( HWND, UINT nMsg, WPARAM wParam, LPARAM lParam, int& rDef )
{
    LRESULT nRet = 0;

    switch ( nMsg )
    {
        case SAL_MSG_THREADYIELD:
            ImplSalYield( (bool)wParam, (bool)lParam );
            rDef = FALSE;
            break;
        case SAL_MSG_STARTTIMER:
            ImplSalStartTimer( (sal_uLong) lParam );
            rDef = FALSE;
            break;
        case SAL_MSG_STOPTIMER:
            ImplSalStopTimer();
            rDef = FALSE;
            break;
        case SAL_MSG_CREATEFRAME:
            nRet = reinterpret_cast<LRESULT>(ImplSalCreateFrame( GetSalData()->mpFirstInstance, reinterpret_cast<HWND>(lParam), (SalFrameStyleFlags)wParam ));
            rDef = FALSE;
            break;
        case SAL_MSG_RECREATEHWND:
            nRet = reinterpret_cast<LRESULT>(ImplSalReCreateHWND( reinterpret_cast<HWND>(wParam), reinterpret_cast<HWND>(lParam), false ));
            rDef = FALSE;
            break;
        case SAL_MSG_RECREATECHILDHWND:
            nRet = reinterpret_cast<LRESULT>(ImplSalReCreateHWND( reinterpret_cast<HWND>(wParam), reinterpret_cast<HWND>(lParam), true ));
            rDef = FALSE;
            break;
        case SAL_MSG_DESTROYFRAME:
            delete reinterpret_cast<SalFrame*>(lParam);
            rDef = FALSE;
            break;
        case SAL_MSG_DESTROYHWND:
            //We only destroy the native window here. We do NOT destroy the SalFrame contained
            //in the structure (GetWindowPtr()).
            if (DestroyWindow(reinterpret_cast<HWND>(lParam)) == 0)
            {
                OSL_FAIL("DestroyWindow failed!");
                //Failure: We remove the SalFrame from the window structure. So we avoid that
                // the window structure may contain an invalid pointer, once the SalFrame is deleted.
               SetWindowPtr(reinterpret_cast<HWND>(lParam), nullptr);
            }
            rDef = FALSE;
            break;
        case SAL_MSG_CREATEOBJECT:
            nRet = reinterpret_cast<LRESULT>(ImplSalCreateObject( GetSalData()->mpFirstInstance, reinterpret_cast<WinSalFrame*>(lParam) ));
            rDef = FALSE;
            break;
        case SAL_MSG_DESTROYOBJECT:
            delete reinterpret_cast<SalObject*>(lParam);
            rDef = FALSE;
            break;
        case SAL_MSG_GETDC:
            nRet = reinterpret_cast<LRESULT>(GetDCEx( reinterpret_cast<HWND>(wParam), nullptr, DCX_CACHE ));
            rDef = FALSE;
            break;
        case SAL_MSG_RELEASEDC:
            ReleaseDC( reinterpret_cast<HWND>(wParam), reinterpret_cast<HDC>(lParam) );
            rDef = FALSE;
            break;
        case SAL_MSG_POSTTIMER:
            EmitTimerCallback();
            break;
        case SAL_MSG_TIMER_CALLBACK:
            EmitTimerCallback();
            MSG aMsg;
            while (PeekMessageW(&aMsg, nullptr, SAL_MSG_TIMER_CALLBACK, SAL_MSG_TIMER_CALLBACK, PM_REMOVE))
            {
                // nothing; just remove all the SAL_MSG_TIMER_CALLBACKs that
                // accumulated in the queue during the EmitTimerCallback(),
                // otherwise it happens with short timeouts and long callbacks
                // that no other events will ever be processed, as the queue
                // is full of SAL_MSG_TIMER_CALLBACKs.
                // It is impossible to limit the amount of them being emitted
                // in the first place, as they are emitted asynchronously, but
                // here we are already fully synchronized.
            }
            break;
    }

    return nRet;
}

LRESULT CALLBACK SalComWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    int bDef = TRUE;
    LRESULT nRet = 0;
    __try
    {
        nRet = SalComWndProc( hWnd, nMsg, wParam, lParam, bDef );
    }
    __except(WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(GetExceptionCode(), GetExceptionInformation()))
    {
    }
    if ( bDef )
    {
        if ( !ImplHandleGlobalMsg( hWnd, nMsg, wParam, lParam, nRet ) )
            nRet = DefWindowProcW( hWnd, nMsg, wParam, lParam );
    }
    return nRet;
}

bool WinSalInstance::AnyInput( VclInputFlags nType )
{
    MSG aMsg;

    if ( (nType & VCL_INPUT_ANY) == VCL_INPUT_ANY )
    {
        // revert bugfix for #108919# which never reported timeouts when called from the timer handler
        // which made the application completely unresponsive during background formatting
        if ( PeekMessageW( &aMsg, nullptr, 0, 0, PM_NOREMOVE | PM_NOYIELD ) )
            return true;
    }
    else
    {
        if ( nType & VclInputFlags::MOUSE )
        {
            // Test for mouse input
            if ( PeekMessageW( &aMsg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;
        }

        if ( nType & VclInputFlags::KEYBOARD )
        {
            // Test for key input
            if ( PeekMessageW( &aMsg, nullptr, WM_KEYDOWN, WM_KEYDOWN,
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

        if ( nType & VclInputFlags::PAINT )
        {
            // Test for paint input
            if ( PeekMessageW( &aMsg, nullptr, WM_PAINT, WM_PAINT,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;

            if ( PeekMessageW( &aMsg, nullptr, WM_SIZE, WM_SIZE,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;

            if ( PeekMessageW( &aMsg, nullptr, SAL_MSG_POSTCALLSIZE, SAL_MSG_POSTCALLSIZE,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;

            if ( PeekMessageW( &aMsg, nullptr, WM_MOVE, WM_MOVE,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;

            if ( PeekMessageW( &aMsg, nullptr, SAL_MSG_POSTMOVE, SAL_MSG_POSTMOVE,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;
        }

        if ( nType & VclInputFlags::TIMER )
        {
            // Test for timer input
            if ( PeekMessageW( &aMsg, nullptr, WM_TIMER, WM_TIMER,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;

        }

        if ( nType & VclInputFlags::OTHER )
        {
            // Test for any input
            if ( PeekMessageW( &aMsg, nullptr, 0, 0, PM_NOREMOVE | PM_NOYIELD ) )
                return true;
        }
    }

    return false;
}

void SalTimer::Start( sal_uLong nMS )
{
    // to switch to Main-Thread
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstInstance )
    {
        if ( pSalData->mnAppThreadId != GetCurrentThreadId() )
        {
            BOOL const ret = PostMessageW(pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS);
            SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");
        }
        else
            SendMessageW( pSalData->mpFirstInstance->mhComWnd, SAL_MSG_STARTTIMER, 0, (LPARAM)nMS );
    }
    else
        ImplSalStartTimer( nMS );
}

SalFrame* WinSalInstance::CreateChildFrame( SystemParentData* pSystemParentData, SalFrameStyleFlags nSalFrameStyle )
{
    // to switch to Main-Thread
    return reinterpret_cast<SalFrame*>((sal_IntPtr)SendMessageW( mhComWnd, SAL_MSG_CREATEFRAME, static_cast<WPARAM>(nSalFrameStyle), reinterpret_cast<LPARAM>(pSystemParentData->hWnd) ));
}

SalFrame* WinSalInstance::CreateFrame( SalFrame* pParent, SalFrameStyleFlags nSalFrameStyle )
{
    // to switch to Main-Thread
    HWND hWndParent;
    if ( pParent )
        hWndParent = static_cast<WinSalFrame*>(pParent)->mhWnd;
    else
        hWndParent = nullptr;
    return reinterpret_cast<SalFrame*>((sal_IntPtr)SendMessageW( mhComWnd, SAL_MSG_CREATEFRAME, static_cast<WPARAM>(nSalFrameStyle), reinterpret_cast<LPARAM>(hWndParent) ));
}

void WinSalInstance::DestroyFrame( SalFrame* pFrame )
{
    OpenGLContext::prepareForYield();
    SendMessageW( mhComWnd, SAL_MSG_DESTROYFRAME, 0, reinterpret_cast<LPARAM>(pFrame) );
}

SalObject* WinSalInstance::CreateObject( SalFrame* pParent,
                                         SystemWindowData* /*pWindowData*/, // SystemWindowData meaningless on Windows
                                         bool /*bShow*/ )
{
    // to switch to Main-Thread
    return reinterpret_cast<SalObject*>((sal_IntPtr)SendMessageW( mhComWnd, SAL_MSG_CREATEOBJECT, 0, reinterpret_cast<LPARAM>(static_cast<WinSalFrame*>(pParent)) ));
}

void WinSalInstance::DestroyObject( SalObject* pObject )
{
    SendMessageW( mhComWnd, SAL_MSG_DESTROYOBJECT, 0, reinterpret_cast<LPARAM>(pObject) );
}

OUString WinSalInstance::GetConnectionIdentifier()
{
    return OUString("");
}

/** Add a file to the system shells recent document list if there is any.
      This function may have no effect under Unix because there is no
      standard API among the different desktop managers.

      @param aFileUrl
                The file url of the document.
*/
void WinSalInstance::AddToRecentDocumentList(const OUString& rFileUrl, const OUString& /*rMimeType*/, const OUString& rDocumentService)
{
    OUString system_path;
    osl::FileBase::RC rc = osl::FileBase::getSystemPathFromFileURL(rFileUrl, system_path);

    OSL_ENSURE(osl::FileBase::E_None == rc, "Invalid file url");

    if (osl::FileBase::E_None == rc)
    {
        if ( aSalShlData.mbW7 )
        {
            typedef HRESULT ( WINAPI *SHCREATEITEMFROMPARSINGNAME )( PCWSTR, IBindCtx*, REFIID, void **ppv );
            SHCREATEITEMFROMPARSINGNAME pSHCreateItemFromParsingName =
                                        reinterpret_cast<SHCREATEITEMFROMPARSINGNAME>(GetProcAddress(
                                        GetModuleHandleW (L"shell32.dll"), "SHCreateItemFromParsingName" ));

            if( pSHCreateItemFromParsingName )
            {
                IShellItem* pShellItem = nullptr;

                HRESULT hr = pSHCreateItemFromParsingName ( system_path.getStr(), nullptr, IID_PPV_ARGS(&pShellItem) );

                if ( SUCCEEDED(hr) && pShellItem )
                {
                    OUString sApplicationName;

                    if ( rDocumentService == "com.sun.star.text.TextDocument" ||
                         rDocumentService == "com.sun.star.text.GlobalDocument" ||
                         rDocumentService == "com.sun.star.text.WebDocument" ||
                         rDocumentService == "com.sun.star.xforms.XMLFormDocument" )
                        sApplicationName = "Writer";
                    else if ( rDocumentService == "com.sun.star.sheet.SpreadsheetDocument" ||
                         rDocumentService == "com.sun.star.chart2.ChartDocument" )
                        sApplicationName = "Calc";
                    else if ( rDocumentService == "com.sun.star.presentation.PresentationDocument" )
                        sApplicationName = "Impress";
                    else if ( rDocumentService == "com.sun.star.drawing.DrawingDocument" )
                        sApplicationName = "Draw";
                    else if ( rDocumentService == "com.sun.star.formula.FormulaProperties" )
                        sApplicationName = "Math";
                    else if ( rDocumentService == "com.sun.star.sdb.DatabaseDocument" ||
                         rDocumentService == "com.sun.star.sdb.OfficeDatabaseDocument" ||
                         rDocumentService == "com.sun.star.sdb.RelationDesign" ||
                         rDocumentService == "com.sun.star.sdb.QueryDesign" ||
                         rDocumentService == "com.sun.star.sdb.TableDesign" ||
                         rDocumentService == "com.sun.star.sdb.DataSourceBrowser" )
                        sApplicationName = "Base";

                    if ( !sApplicationName.isEmpty() )
                    {
                        OUString sApplicationID("TheDocumentFoundation.LibreOffice.");
                        sApplicationID += sApplicationName;

#if _WIN32_WINNT < _WIN32_WINNT_WIN7
// just define Windows 7 only constant locally...
#define SHARD_APPIDINFO  0x00000004
#endif

                        typedef struct {
                            IShellItem *psi;
                            PCWSTR     pszAppID;
                        } DummyShardAppIDInfo;

                        DummyShardAppIDInfo info;
                        info.psi = pShellItem;
                        info.pszAppID = sApplicationID.getStr();

                        SHAddToRecentDocs ( SHARD_APPIDINFO, &info );
                        return;
                    }
                }
            }
        }
        // For whatever reason, we could not use the SHARD_APPIDINFO semantics
        SHAddToRecentDocs(SHARD_PATHW, system_path.getStr());
    }
}

SalTimer* WinSalInstance::CreateSalTimer()
{
    return new WinSalTimer();
}

SalBitmap* WinSalInstance::CreateSalBitmap()
{
    if (OpenGLHelper::isVCLOpenGLEnabled())
        return new OpenGLSalBitmap();
    else
        return new WinSalBitmap();
}

const OUString& SalGetDesktopEnvironment()
{
    static OUString aDesktopEnvironment( "Windows" );
    return aDesktopEnvironment;
}

SalSession* WinSalInstance::CreateSalSession()
{
    return nullptr;
}

int WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(int, LPEXCEPTION_POINTERS pExceptionInfo)
{
    // Decide if an exception is a c++ (mostly UNO) exception or a process violation.
    // Depending on this information we pass process violations directly to our signal handler ...
    // and c++ (UNO) exceptions are sended to the following code on the current stack.
    // Problem behind: user32.dll sometime consumes exceptions/process violations .-)
    // see also #112221#

    static const DWORD EXCEPTION_MSC_CPP_EXCEPTION = 0xE06D7363;

    if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_MSC_CPP_EXCEPTION)
        return EXCEPTION_CONTINUE_SEARCH;

    return UnhandledExceptionFilter( pExceptionInfo );
}

OUString WinSalInstance::getOSVersion()
{
    SalData* pSalData = GetSalData();
    if ( !pSalData )
        return OUString("unknown");

    WORD nMajor = 0, nMinor = 0;
#ifdef _WIN32_WINNT_WINBLUE
    // Trying to hide the real version info behind an
    // uber-lame non-forward-compatible, 'compatibility' API
    // seems unlikely to help OS designers, or API users.
    nMajor = 30;
    while( !IsWindowsVersionOrGreater( nMajor, 0, 0 ) && nMajor > 0)
        nMajor--;
    nMinor = 30;
    while( !IsWindowsVersionOrGreater( nMajor, nMinor, 0 ) && nMinor > 0)
        nMinor--;
#else
    OSVERSIONINFO aVersionInfo;
    memset( &aVersionInfo, 0, sizeof( aVersionInfo ) );
    aVersionInfo.dwOSVersionInfoSize = sizeof( aVersionInfo );
    if ( GetVersionEx( &aVersionInfo ) )
    {
        nMajor = aVersionInfo.dwMajorVersion;
        nMinor = aVersionInfo.dwMinorVersion;
    }
#endif
    OUStringBuffer aVer;
    aVer.append( "Windows " );
    aVer.append( (sal_Int32)nMajor );
    aVer.append( "." );
    aVer.append( (sal_Int32)nMinor );

    return aVer.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
