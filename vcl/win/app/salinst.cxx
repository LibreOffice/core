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
#include <tools/time.hxx>
#include <comphelper/solarmutex.hxx>

#include <vcl/inputtypes.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/timer.hxx>

#include "opengl/salbmp.hxx"
#include "opengl/win/gdiimpl.hxx"
#include "win/wincomp.hxx"
#include "win/salids.hrc"
#include "win/saldata.hxx"
#include "win/salinst.h"
#include "win/salframe.h"
#include "win/salobj.h"
#include "win/saltimer.h"
#include "win/salbmp.h"
#include "win/winlayout.hxx"

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
        FatalAppExitW( 0, SAL_W(rErrorText.getStr()) );
    }
}

LRESULT CALLBACK SalComWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );

class SalYieldMutex : public comphelper::GenericSolarMutex
{
public: // for ImplSalYield() and ImplSalYieldMutexAcquireWithWait()
    osl::Condition            m_condition; /// for MsgWaitForMultipleObjects()

protected:
    virtual void              doAcquire( sal_uInt32 nLockCount ) override;
    virtual sal_uInt32        doRelease( bool bUnlockAll ) override;

    static void               BeforeReleaseHandler();

public:
    explicit SalYieldMutex();

    virtual bool              IsCurrentThread() const override;
    virtual bool              tryToAcquire() override;
};

SalYieldMutex::SalYieldMutex()
{
    SetBeforeReleaseHandler( &SalYieldMutex::BeforeReleaseHandler );
}

void SalYieldMutex::BeforeReleaseHandler()
{
    OpenGLContext::prepareForYield();

    if ( GetSalData()->mnAppThreadId != GetCurrentThreadId() )
    {
        // If we don't call these message, the Output from the
        // Java clients doesn't come in the right order
        GdiFlush();
    }
}

/// note: while VCL is fully up and running (other threads started and
/// before shutdown), the main thread must acquire SolarMutex only via
/// this function to avoid deadlock
void SalYieldMutex::doAcquire( sal_uInt32 nLockCount )
{
    WinSalInstance* pInst = GetSalData()->mpInstance;
    if ( pInst && pInst->IsMainThread() )
    {
        if ( pInst->mbNoYieldLock )
            return;
        // tdf#96887 If this is the main thread, then we must wait for two things:
        // - the mpSalYieldMutex being freed
        // - SendMessage() being triggered
        // This can nicely be done using MsgWaitForMultipleObjects. The 2nd one is
        // needed because if we don't reschedule, then we create deadlocks if a
        // Window's create/destroy is called via SendMessage() from another thread.
        // Have a look at the osl_waitCondition implementation for more info.
        do {
            // reset condition *before* acquiring!
            m_condition.reset();
            if (m_aMutex.tryToAcquire())
                break;
            // wait for SalYieldMutex::release() to set the condition
            osl::Condition::Result res = m_condition.wait();
            assert(osl::Condition::Result::result_ok == res);
        }
        while ( true );
    }
    else
        m_aMutex.acquire();
    ++m_nCount;
    --nLockCount;

    comphelper::GenericSolarMutex::doAcquire( nLockCount );
}

sal_uInt32 SalYieldMutex::doRelease( const bool bUnlockAll )
{
    WinSalInstance* pInst = GetSalData()->mpInstance;
    if ( pInst && pInst->mbNoYieldLock && pInst->IsMainThread() )
        return 1;

    sal_uInt32 nCount = comphelper::GenericSolarMutex::doRelease( bUnlockAll );
    // wake up ImplSalYieldMutexAcquireWithWait() after release
    if ( 0 == m_nCount )
        m_condition.set();
    return nCount;
}

bool SalYieldMutex::tryToAcquire()
{
    WinSalInstance* pInst = GetSalData()->mpInstance;
    if ( pInst )
    {
        if ( pInst->mbNoYieldLock && pInst->IsMainThread() )
            return true;
        else
            return comphelper::GenericSolarMutex::tryToAcquire();
    }
    else
        return false;
}

void ImplSalYieldMutexAcquireWithWait( sal_uInt32 nCount )
{
    WinSalInstance* pInst = GetSalData()->mpInstance;
    if ( pInst )
        pInst->mpSalYieldMutex->acquire( nCount );
}

bool ImplSalYieldMutexTryToAcquire()
{
    WinSalInstance* pInst = GetSalData()->mpInstance;
    return pInst && pInst->mpSalYieldMutex->tryToAcquire();
}

void ImplSalYieldMutexRelease()
{
    WinSalInstance* pInst = GetSalData()->mpInstance;
    if ( pInst )
    {
        GdiFlush();
        pInst->mpSalYieldMutex->release();
    }
}

bool SalYieldMutex::IsCurrentThread() const
{
    if ( !GetSalData()->mpInstance->mbNoYieldLock )
        // For the Windows backend, the LO identifier is the system thread ID
        return m_nThreadId == GetCurrentThreadId();
    else
        return GetSalData()->mpInstance->IsMainThread();
}

void SalData::initKeyCodeMap()
{
    UINT nKey;
    #define initKey( a, b )\
        nKey = LOWORD( VkKeyScanW( a ) );\
        if( nKey < 0xffff )\
            maVKMap[ nKey ] = b;

    maVKMap.clear();

    initKey( L'+', KEY_ADD );
    initKey( L'-', KEY_SUBTRACT );
    initKey( L'*', KEY_MULTIPLY );
    initKey( L'/', KEY_DIVIDE );
    initKey( L'.', KEY_POINT );
    initKey( L',', KEY_COMMA );
    initKey( L'<', KEY_LESS );
    initKey( L'>', KEY_GREATER );
    initKey( L'=', KEY_EQUAL );
    initKey( L'~', KEY_TILDE );
    initKey( L'`', KEY_QUOTELEFT );
    initKey( L'[', KEY_BRACKETLEFT );
    initKey( L']', KEY_BRACKETRIGHT );
    initKey( L';', KEY_SEMICOLON );
    initKey( L'\'', KEY_QUOTERIGHT );
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
    mhSalObjMsgHook = nullptr;  // hook to get interesting msg for SalObject
    mhWantLeaveMsg = nullptr;   // window handle, that want a MOUSELEAVE message
    mpMouseLeaveTimer = nullptr; // Timer for MouseLeave Test
    mpInstance = nullptr;  // pointer of first instance
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
        STARTUPINFOW aSI;
        aSI.cb = sizeof( aSI );
        GetStartupInfoW( &aSI );
        pData->mhInst    = GetModuleHandleW( nullptr );
        pData->mnCmdShow = aSI.wShowWindow;
    }
}

SalInstance* CreateSalInstance()
{
    SalData* pSalData = GetSalData();

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
    aWndClassEx.style          |= CS_DROPSHADOW;
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
    pSalData->mpInstance   = pInst;
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
    if ( pSalData->mpInstance == pInst )
        pSalData->mpInstance = nullptr;

    delete pInst;
}

WinSalInstance::WinSalInstance()
    : mhComWnd( nullptr )
    , mbNoYieldLock( false )
{
    mpSalYieldMutex = new SalYieldMutex();
    mpSalYieldMutex->acquire();
}

WinSalInstance::~WinSalInstance()
{
    mpSalYieldMutex->release();
    delete mpSalYieldMutex;
    DestroyWindow( mhComWnd );
}

comphelper::SolarMutex* WinSalInstance::GetYieldMutex()
{
    return mpSalYieldMutex;
}

sal_uInt32 WinSalInstance::ReleaseYieldMutex( bool bUnlockAll )
{
    return mpSalYieldMutex->release( bUnlockAll );
}

void WinSalInstance::AcquireYieldMutex( sal_uInt32 nCount )
{
    mpSalYieldMutex->acquire( nCount );
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

static bool ImplSalYield( bool bWait, bool bHandleAllCurrentEvents )
{
    static sal_uInt32 nLastTicks = 0;
    MSG aMsg;
    bool bWasMsg = false, bOneEvent = false;
    ImplSVData *const pSVData = ImplGetSVData();
    WinSalTimer* pTimer = static_cast<WinSalTimer*>( pSVData->maSchedCtx.mpSalTimer );

    sal_uInt32 nCurTicks = 0;
    if ( bHandleAllCurrentEvents )
        nCurTicks = GetTickCount();

    bool bHadNewerEvent = false;
    do
    {
        bOneEvent = PeekMessageW( &aMsg, nullptr, 0, 0, PM_REMOVE );
        if ( bOneEvent )
        {
            bWasMsg = true;
            TranslateMessage( &aMsg );
            ImplSalDispatchMessage( &aMsg );
            if ( bHandleAllCurrentEvents
                    && !bHadNewerEvent && aMsg.time > nCurTicks
                    && (nLastTicks <= nCurTicks || aMsg.time < nLastTicks) )
                bHadNewerEvent = true;
            bOneEvent = !bHadNewerEvent;
        }
        // busy loop to catch a message, eventually the 0ms timer.
        // we don't need to loop, if we wait anyway.
        if ( !bWait && !bWasMsg && pTimer && pTimer->PollForMessage() )
        {
            SwitchToThread();
            continue;
        }
        if ( !(bHandleAllCurrentEvents && bOneEvent) )
            break;
    }
    while( true );

    if ( bHandleAllCurrentEvents )
        nLastTicks = nCurTicks;

    // Also check that we don't wait when application already has quit
    if ( bWait && !bWasMsg && !pSVData->maAppData.mbAppQuit )
    {
        if ( GetMessageW( &aMsg, nullptr, 0, 0 ) )
        {
            bWasMsg = true;
            TranslateMessage( &aMsg );
            ImplSalDispatchMessage( &aMsg );
        }
    }

    return bWasMsg;
}

bool WinSalInstance::IsMainThread() const
{
    const SalData* pSalData = GetSalData();
    return pSalData->mnAppThreadId == GetCurrentThreadId();
}

bool WinSalInstance::DoYield(bool bWait, bool bHandleAllCurrentEvents)
{
    bool bDidWork = false;
    SolarMutexReleaser aReleaser;
    if ( !IsMainThread() )
    {
        // If you change the SendMessageW function, you might need to update
        // the PeekMessage( ... PM_QS_POSTMESSAGE) calls!
        bDidWork = SendMessageW( mhComWnd, SAL_MSG_THREADYIELD,
                                 (WPARAM) false, (LPARAM) bHandleAllCurrentEvents );
        if ( !bDidWork && bWait )
        {
            maWaitingYieldCond.reset();
            maWaitingYieldCond.wait();
            bDidWork = true;
        }
    }
    else
    {
        bDidWork = ImplSalYield( bWait, bHandleAllCurrentEvents );
        if ( bDidWork )
            maWaitingYieldCond.set();
    }

    return bDidWork;
}

LRESULT CALLBACK SalComWndProc( HWND, UINT nMsg, WPARAM wParam, LPARAM lParam, int& rDef )
{
    LRESULT nRet = 0;
    WinSalInstance *pInst = GetSalData()->mpInstance;

    switch ( nMsg )
    {
        case SAL_MSG_THREADYIELD:
            assert( !(bool)wParam );
            nRet = static_cast<LRESULT>(ImplSalYield( false, (bool)lParam ));
            rDef = FALSE;
            break;
        case SAL_MSG_STARTTIMER:
        {
            sal_uInt64 nTime = tools::Time::GetSystemTicks();
            if ( nTime < (sal_uInt64) lParam )
                nTime = (sal_uInt64) lParam - nTime;
            else
                nTime = 0;
            static_cast<WinSalTimer*>(ImplGetSVData()->maSchedCtx.mpSalTimer)->ImplStart( nTime );
            rDef = FALSE;
            break;
        }
        case SAL_MSG_STOPTIMER:
            static_cast<WinSalTimer*>(ImplGetSVData()->maSchedCtx.mpSalTimer)->ImplStop();
            break;
        case SAL_MSG_CREATEFRAME:
            assert( !pInst->mbNoYieldLock );
            pInst->mbNoYieldLock = true;
            nRet = reinterpret_cast<LRESULT>(ImplSalCreateFrame( GetSalData()->mpInstance, reinterpret_cast<HWND>(lParam), (SalFrameStyleFlags)wParam ));
            pInst->mbNoYieldLock = false;
            rDef = FALSE;
            break;
        case SAL_MSG_RECREATEHWND:
            assert( !pInst->mbNoYieldLock );
            pInst->mbNoYieldLock = true;
            nRet = reinterpret_cast<LRESULT>(ImplSalReCreateHWND( reinterpret_cast<HWND>(wParam), reinterpret_cast<HWND>(lParam), false ));
            pInst->mbNoYieldLock = false;
            rDef = FALSE;
            break;
        case SAL_MSG_RECREATECHILDHWND:
            assert( !pInst->mbNoYieldLock );
            pInst->mbNoYieldLock = true;
            nRet = reinterpret_cast<LRESULT>(ImplSalReCreateHWND( reinterpret_cast<HWND>(wParam), reinterpret_cast<HWND>(lParam), true ));
            pInst->mbNoYieldLock = false;
            rDef = FALSE;
            break;
        case SAL_MSG_DESTROYFRAME:
            assert( !pInst->mbNoYieldLock );
            pInst->mbNoYieldLock = true;
            delete reinterpret_cast<SalFrame*>(lParam);
            pInst->mbNoYieldLock = false;
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
            assert( !pInst->mbNoYieldLock );
            pInst->mbNoYieldLock = true;
            nRet = reinterpret_cast<LRESULT>(ImplSalCreateObject( GetSalData()->mpInstance, reinterpret_cast<WinSalFrame*>(lParam) ));
            pInst->mbNoYieldLock = false;
            rDef = FALSE;
            break;
        case SAL_MSG_DESTROYOBJECT:
            assert( !pInst->mbNoYieldLock );
            pInst->mbNoYieldLock = true;
            delete reinterpret_cast<SalObject*>(lParam);
            pInst->mbNoYieldLock = false;
            rDef = FALSE;
            break;
        case SAL_MSG_GETDC:
            assert( !pInst->mbNoYieldLock );
            pInst->mbNoYieldLock = true;
            nRet = reinterpret_cast<LRESULT>(GetDCEx( reinterpret_cast<HWND>(wParam), nullptr, DCX_CACHE ));
            pInst->mbNoYieldLock = false;
            rDef = FALSE;
            break;
        case SAL_MSG_RELEASEDC:
            assert( !pInst->mbNoYieldLock );
            pInst->mbNoYieldLock = true;
            ReleaseDC( reinterpret_cast<HWND>(wParam), reinterpret_cast<HDC>(lParam) );
            pInst->mbNoYieldLock = false;
            rDef = FALSE;
            break;
        case SAL_MSG_TIMER_CALLBACK:
        {
            WinSalTimer *const pTimer = static_cast<WinSalTimer*>( ImplGetSVData()->maSchedCtx.mpSalTimer );
            assert( pTimer != nullptr );
            MSG aMsg;
            bool bValidMSG = pTimer->IsValidWPARAM( wParam );
            // PM_QS_POSTMESSAGE is needed, so we don't process the SendMessage from DoYield!
            while ( PeekMessageW(&aMsg, pInst->mhComWnd, SAL_MSG_TIMER_CALLBACK,
                                 SAL_MSG_TIMER_CALLBACK, PM_REMOVE | PM_NOYIELD | PM_QS_POSTMESSAGE) )
            {
                assert( !bValidMSG && "Unexpected non-last valid message" );
                bValidMSG = pTimer->IsValidWPARAM( aMsg.wParam );
            }
            if ( bValidMSG )
                pTimer->ImplEmitTimerCallback();
            break;
        }
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
    return OUString();
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
        IShellItem* pShellItem = nullptr;

        HRESULT hr = SHCreateItemFromParsingName(SAL_W(system_path.getStr()), nullptr, IID_PPV_ARGS(&pShellItem));

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

                SHARDAPPIDINFO info;
                info.psi = pShellItem;
                info.pszAppID = SAL_W(sApplicationID.getStr());

                SHAddToRecentDocs ( SHARD_APPIDINFO, &info );
                return;
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
    // GetVersion(Ex) and VersionHelpers (based on VerifyVersionInfo) API are
    // subject to manifest-based behavior since Windows 8.1, so give wrong results.
    // Another approach would be to use NetWkstaGetInfo, but that has some small
    // reported delays (some milliseconds), and might get slower in domains with
    // poor network connections.
    // So go with a solution described at https://msdn.microsoft.com/en-us/library/ms724429
    HINSTANCE hLibrary = LoadLibraryW(L"kernel32.dll");
    if (hLibrary != nullptr)
    {
        wchar_t szPath[MAX_PATH];
        DWORD dwCount = GetModuleFileNameW(hLibrary, szPath, SAL_N_ELEMENTS(szPath));
        FreeLibrary(hLibrary);
        if (dwCount != 0 && dwCount < SAL_N_ELEMENTS(szPath))
        {
            dwCount = GetFileVersionInfoSizeW(szPath, NULL);
            if (dwCount != 0)
            {
                std::unique_ptr<char> ver(new char[dwCount]);
                if (GetFileVersionInfoW(szPath, 0, dwCount, ver.get()) != FALSE)
                {
                    void* pBlock = nullptr;
                    UINT dwBlockSz = 0;
                    if (VerQueryValueW(ver.get(), L"\\", &pBlock, &dwBlockSz) != FALSE && dwBlockSz >= sizeof(VS_FIXEDFILEINFO))
                    {
                        VS_FIXEDFILEINFO *vinfo = reinterpret_cast<VS_FIXEDFILEINFO *>(pBlock);
                        OUStringBuffer aVer;
                        aVer.append("Windows ");
                        aVer.append((sal_Int32)HIWORD(vinfo->dwProductVersionMS));
                        aVer.append(".");
                        aVer.append((sal_Int32)LOWORD(vinfo->dwProductVersionMS));
                        return aVer.makeStringAndClear();
                    }
                }
            }
        }
    }
    return "unknown";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
