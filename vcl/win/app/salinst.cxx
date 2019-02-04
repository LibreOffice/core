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
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <tools/time.hxx>
#include <comphelper/solarmutex.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <vcl/inputtypes.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/timer.hxx>
#include <vclpluginapi.h>

#include <opengl/salbmp.hxx>
#include <opengl/win/gdiimpl.hxx>
#include <win/wincomp.hxx>
#include <win/salids.hrc>
#include <win/saldata.hxx>
#include <win/salinst.h>
#include <win/salframe.h>
#include <win/salobj.h>
#include <win/saltimer.h>
#include <win/salbmp.h>
#include <win/winlayout.hxx>

#include <salimestatus.hxx>
#include <salsys.hxx>

#include <desktop/crashreport.hxx>

#if defined _MSC_VER
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#endif

#include <prewin.h>

#include <gdiplus.h>
#include <shlobj.h>

#include <postwin.h>

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
        FatalAppExitW( 0, o3tl::toW(rErrorText.getStr()) );
    }
}

static LRESULT CALLBACK SalComWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );

class SalYieldMutex : public comphelper::SolarMutex
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
        if ( pInst->m_nNoYieldLock )
            return;
        // tdf#96887 If this is the main thread, then we must wait for two things:
        // - the yield mutex being unlocked
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

    comphelper::SolarMutex::doAcquire( nLockCount );
}

sal_uInt32 SalYieldMutex::doRelease( const bool bUnlockAll )
{
    WinSalInstance* pInst = GetSalData()->mpInstance;
    if ( pInst && pInst->m_nNoYieldLock && pInst->IsMainThread() )
        return 1;

    sal_uInt32 nCount = comphelper::SolarMutex::doRelease( bUnlockAll );
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
        if ( pInst->m_nNoYieldLock && pInst->IsMainThread() )
            return true;
        else
            return comphelper::SolarMutex::tryToAcquire();
    }
    else
        return false;
}

void ImplSalYieldMutexAcquireWithWait( sal_uInt32 nCount )
{
    WinSalInstance* pInst = GetSalData()->mpInstance;
    if ( pInst )
        pInst->GetYieldMutex()->acquire( nCount );
}

bool ImplSalYieldMutexTryToAcquire()
{
    WinSalInstance* pInst = GetSalData()->mpInstance;
    return pInst && pInst->GetYieldMutex()->tryToAcquire();
}

void ImplSalYieldMutexRelease()
{
    WinSalInstance* pInst = GetSalData()->mpInstance;
    if ( pInst )
    {
        GdiFlush();
        pInst->GetYieldMutex()->release();
    }
}

bool SalYieldMutex::IsCurrentThread() const
{
    if ( !GetSalData()->mpInstance->m_nNoYieldLock )
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
    mnAppThreadId = 0;          // Id from Application-Thread
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

    CoInitialize(nullptr); // put main thread in Single Threaded Apartment (STA)
    static Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

SalData::~SalData()
{
    deInitNWF();
    SetSalData( nullptr );

    CoUninitialize();

    if (gdiplusToken)
        Gdiplus::GdiplusShutdown(gdiplusToken);
}

extern "C" {
VCLPLUG_WIN_PUBLIC SalInstance* create_SalInstance()
{
    SalData* pSalData = new SalData();

    STARTUPINFOW aSI;
    aSI.cb = sizeof( aSI );
    GetStartupInfoW( &aSI );
    pSalData->mhInst = GetModuleHandleW( nullptr );
    pSalData->mnCmdShow = aSI.wShowWindow;

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
}

WinSalInstance::WinSalInstance()
    : SalInstance(std::make_unique<SalYieldMutex>())
    , mhInst( nullptr )
    , mhComWnd( nullptr )
    , m_nNoYieldLock( 0 )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mxToolkitName = OUString("win");
}

WinSalInstance::~WinSalInstance()
{
    ImplFreeSalGDI();
    DestroyWindow( mhComWnd );
}

static LRESULT ImplSalDispatchMessage( const MSG* pMsg )
{
    SalData* pSalData = GetSalData();
    if ( pSalData->mpFirstObject && ImplSalPreDispatchMsg( pMsg ) )
        return 0;
    LRESULT lResult = DispatchMessageW( pMsg );
    if ( pSalData->mpFirstObject )
        ImplSalPostDispatchMsg( pMsg );
    return lResult;
}

bool ImplSalYield( bool bWait, bool bHandleAllCurrentEvents )
{
    static sal_uInt32 nLastTicks = 0;
    MSG aMsg;
    bool bWasMsg = false, bOneEvent = false, bWasTimeoutMsg = false;
    ImplSVData *const pSVData = ImplGetSVData();
    WinSalTimer* pTimer = static_cast<WinSalTimer*>( pSVData->maSchedCtx.mpSalTimer );
    const bool bNoYieldLock = (GetSalData()->mpInstance->m_nNoYieldLock > 0);

    assert( !bNoYieldLock );
    if ( bNoYieldLock )
        return false;

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
            LRESULT nRet = ImplSalDispatchMessage( &aMsg );

            if ( !bWasTimeoutMsg )
                bWasTimeoutMsg = (SAL_MSG_TIMER_CALLBACK == aMsg.message)
                    && static_cast<bool>( nRet );

            if ( bHandleAllCurrentEvents
                    && !bHadNewerEvent && aMsg.time > nCurTicks
                    && (nLastTicks <= nCurTicks || aMsg.time < nLastTicks) )
                bHadNewerEvent = true;
            bOneEvent = !bHadNewerEvent;
        }

        if ( !(bHandleAllCurrentEvents && bOneEvent) )
            break;
    }
    while( true );

    // 0ms timeouts are handled out-of-bounds to prevent busy-locking the
    // event loop with timeout messages.
    // We ensure we never handle more then one timeout per call.
    // This way we'll always process a normal system message.
    if ( !bWasTimeoutMsg && pTimer && pTimer->IsDirectTimeout() )
    {
        pTimer->ImplHandleElapsedTimer();
        bWasMsg = true;
    }

    if ( bHandleAllCurrentEvents )
        nLastTicks = nCurTicks;

    if ( bWait && !bWasMsg )
    {
        if ( GetMessageW( &aMsg, nullptr, 0, 0 ) )
        {
            bWasMsg = true;
            TranslateMessage( &aMsg );
            ImplSalDispatchMessage( &aMsg );
        }
    }

    // we're back in the main loop after resize or move
    if ( pTimer )
        pTimer->SetForceRealTimer( false );

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
        bDidWork = SendMessageW( mhComWnd, SAL_MSG_THREADYIELD,
                                 WPARAM(false), static_cast<LPARAM>(bHandleAllCurrentEvents) );
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

#define CASE_NOYIELDLOCK( salmsg, function ) \
    case salmsg: \
        if (bIsOtherThreadMessage) \
        { \
            ++pInst->m_nNoYieldLock; \
            function; \
            --pInst->m_nNoYieldLock; \
        } \
        else \
        { \
            DBG_TESTSOLARMUTEX(); \
            function; \
        } \
        break;

#define CASE_NOYIELDLOCK_RESULT( salmsg, function ) \
    case salmsg: \
        if (bIsOtherThreadMessage) \
        { \
            ++pInst->m_nNoYieldLock; \
            nRet = reinterpret_cast<LRESULT>( function ); \
            --pInst->m_nNoYieldLock; \
        } \
        else \
        { \
            DBG_TESTSOLARMUTEX(); \
            nRet = reinterpret_cast<LRESULT>( function ); \
        } \
        break;

LRESULT CALLBACK SalComWndProc( HWND, UINT nMsg, WPARAM wParam, LPARAM lParam, bool& rDef )
{
    const BOOL bIsOtherThreadMessage = InSendMessage();
    LRESULT nRet = 0;
    WinSalInstance *pInst = GetSalData()->mpInstance;
    WinSalTimer *const pTimer = static_cast<WinSalTimer*>( ImplGetSVData()->maSchedCtx.mpSalTimer );

    SAL_INFO("vcl.gdi.wndproc", "SalComWndProc(nMsg=" << nMsg << ", wParam=" << wParam
                                << ", lParam=" << lParam << "); inSendMsg: " << bIsOtherThreadMessage);

    switch ( nMsg )
    {
        case SAL_MSG_THREADYIELD:
            assert( !static_cast<bool>(wParam) );
            nRet = static_cast<LRESULT>(ImplSalYield(
                false, static_cast<bool>( lParam ) ));
            break;

        case SAL_MSG_STARTTIMER:
        {
            sal_uInt64 nTime = tools::Time::GetSystemTicks();
            if ( nTime < static_cast<sal_uInt64>( lParam ) )
                nTime = static_cast<sal_uInt64>( lParam ) - nTime;
            else
                nTime = 0;
            assert( pTimer != nullptr );
            pTimer->ImplStart( nTime );
            break;
        }

        case SAL_MSG_STOPTIMER:
            assert( pTimer != nullptr );
            pTimer->ImplStop();
            break;

        CASE_NOYIELDLOCK_RESULT( SAL_MSG_CREATEFRAME, ImplSalCreateFrame( GetSalData()->mpInstance,
            reinterpret_cast<HWND>(lParam), static_cast<SalFrameStyleFlags>(wParam)) )
        CASE_NOYIELDLOCK_RESULT( SAL_MSG_RECREATEHWND, ImplSalReCreateHWND(
            reinterpret_cast<HWND>(wParam), reinterpret_cast<HWND>(lParam), false) )
        CASE_NOYIELDLOCK_RESULT( SAL_MSG_RECREATECHILDHWND, ImplSalReCreateHWND(
            reinterpret_cast<HWND>(wParam), reinterpret_cast<HWND>(lParam), true) )
        CASE_NOYIELDLOCK( SAL_MSG_DESTROYFRAME, delete reinterpret_cast<SalFrame*>(lParam) )

        case SAL_MSG_DESTROYHWND:
            // We only destroy the native window here. We do NOT destroy the SalFrame contained
            // in the structure (GetWindowPtr()).
            if (DestroyWindow(reinterpret_cast<HWND>(lParam)) == 0)
            {
                OSL_FAIL("DestroyWindow failed!");
                // Failure: We remove the SalFrame from the window structure. So we avoid that
                // the window structure may contain an invalid pointer, once the SalFrame is deleted.
                SetWindowPtr(reinterpret_cast<HWND>(lParam), nullptr);
            }
            break;

        CASE_NOYIELDLOCK_RESULT( SAL_MSG_CREATEOBJECT, ImplSalCreateObject(
            GetSalData()->mpInstance, reinterpret_cast<WinSalFrame*>(lParam)) )
        CASE_NOYIELDLOCK( SAL_MSG_DESTROYOBJECT, delete reinterpret_cast<SalObject*>(lParam) )
        CASE_NOYIELDLOCK_RESULT( SAL_MSG_GETCACHEDDC, GetDCEx(
            reinterpret_cast<HWND>(wParam), nullptr, DCX_CACHE) )
        CASE_NOYIELDLOCK( SAL_MSG_RELEASEDC, ReleaseDC(
            reinterpret_cast<HWND>(wParam), reinterpret_cast<HDC>(lParam)) )

        case SAL_MSG_TIMER_CALLBACK:
            assert( pTimer != nullptr );
            pTimer->ImplHandleTimerEvent( wParam );
            break;

        case WM_TIMER:
            assert( pTimer != nullptr );
            pTimer->ImplHandle_WM_TIMER( wParam );
            break;

        case SAL_MSG_FORCE_REAL_TIMER:
            assert(pTimer != nullptr);
            pTimer->SetForceRealTimer(true);
            break;

        case SAL_MSG_DUMMY:
            break;

        default:
            rDef = true;
            break;
    }

    return nRet;
}

#undef CASE_NOYIELDLOCK
#undef CASE_NOYIELDLOCK_RESULT

LRESULT CALLBACK SalComWndProcW( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    bool bDef = false;
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

struct MsgRange
{
    UINT nStart;
    UINT nEnd;
};

static std::vector<MsgRange> GetOtherRanges( VclInputFlags nType )
{
    assert( nType != VCL_INPUT_ANY );

    // this array must be kept sorted!
    const UINT nExcludeMsgIds[] =
    {
        0,

        WM_MOVE, // 3
        WM_SIZE, // 5
        WM_PAINT, // 15
        WM_KEYDOWN, // 256
        WM_TIMER, // 275

        WM_MOUSEFIRST, // 512
        513,
        514,
        515,
        516,
        517,
        518,
        519,
        520,
        WM_MOUSELAST, // 521

        SAL_MSG_POSTMOVE, // WM_USER+136
        SAL_MSG_POSTCALLSIZE, // WM_USER+137

        SAL_MSG_TIMER_CALLBACK, // WM_USER+162

        UINT_MAX
    };
    const unsigned MAX_EXCL = SAL_N_ELEMENTS( nExcludeMsgIds );

    bool aExcludeMsgList[ MAX_EXCL ] = { false, };
    std::vector<MsgRange> aResult;

    // set the excluded values
    if ( !(nType & VclInputFlags::MOUSE) )
    {
        for ( unsigned i = 0; nExcludeMsgIds[ 6 + i ] <= WM_MOUSELAST; ++i )
            aExcludeMsgList[ 6 + i ] = true;
    }

    if ( !(nType & VclInputFlags::KEYBOARD) )
        aExcludeMsgList[ 4 ] = true;

    if ( !(nType & VclInputFlags::PAINT) )
    {
        aExcludeMsgList[ 1 ] = true;
        aExcludeMsgList[ 2 ] = true;
        aExcludeMsgList[ 3 ] = true;
        aExcludeMsgList[ 16 ] = true;
        aExcludeMsgList[ 17 ] = true;
    }

    if ( !(nType & VclInputFlags::TIMER) )
    {
        aExcludeMsgList[ 5 ]  = true;
        aExcludeMsgList[ 18 ]  = true;
    }

    // build the message ranges to check
    MsgRange aRange = { 0, 0 };
    bool doEnd = true;
    for ( unsigned i = 1; i < MAX_EXCL; ++i )
    {
        if ( aExcludeMsgList[ i ] )
        {
            if ( !doEnd )
            {
                if ( nExcludeMsgIds[ i ] == aRange.nStart )
                    ++aRange.nStart;
                else
                    doEnd = true;
            }
            if ( doEnd )
            {
                aRange.nEnd = nExcludeMsgIds[ i ] - 1;
                aResult.push_back( aRange );
                doEnd = false;
                aRange.nStart = aRange.nEnd + 2;
            }
        }
    }

    if ( aRange.nStart != UINT_MAX )
    {
        aRange.nEnd = UINT_MAX;
        aResult.push_back( aRange );
    }

    return aResult;
}

bool WinSalInstance::AnyInput( VclInputFlags nType )
{
    MSG aMsg;

    if ( nType & VclInputFlags::TIMER )
    {
        const WinSalTimer* pTimer = static_cast<WinSalTimer*>( ImplGetSVData()->maSchedCtx.mpSalTimer );
        if ( pTimer && pTimer->HasTimerElapsed() )
            return true;
    }

    if ( (nType & VCL_INPUT_ANY) == VCL_INPUT_ANY )
    {
        // revert bugfix for #108919# which never reported timeouts when called from the timer handler
        // which made the application completely unresponsive during background formatting
        if ( PeekMessageW( &aMsg, nullptr, 0, 0, PM_NOREMOVE | PM_NOYIELD ) )
            return true;
    }
    else
    {
        const bool bCheck_KEYBOARD (nType & VclInputFlags::KEYBOARD);
        const bool bCheck_OTHER    (nType & VclInputFlags::OTHER);

        // If there is a modifier key event, it counts as OTHER
        // Previously we were simply ignoring these events...
        if ( bCheck_KEYBOARD || bCheck_OTHER )
        {
            if ( PeekMessageW( &aMsg, nullptr, WM_KEYDOWN, WM_KEYDOWN,
                                  PM_NOREMOVE | PM_NOYIELD ) )
            {
                const bool bIsModifier = ( (aMsg.wParam == VK_SHIFT) ||
                    (aMsg.wParam == VK_CONTROL) || (aMsg.wParam == VK_MENU) );
                if ( bCheck_KEYBOARD && !bIsModifier )
                    return true;
                if ( bCheck_OTHER && bIsModifier )
                    return true;
            }
        }

        // Other checks for all messages not excluded.
        // The less we exclude, the less ranges have to be checked!
        if ( bCheck_OTHER )
        {
            // TIMER and KEYBOARD are already handled, so always exclude them!
            VclInputFlags nOtherType = nType &
                ~VclInputFlags(VclInputFlags::KEYBOARD | VclInputFlags::TIMER);

            std::vector<MsgRange> aMsgRangeList( GetOtherRanges( nOtherType ) );
            for ( MsgRange const & aRange : aMsgRangeList )
                if ( PeekMessageW( &aMsg, nullptr, aRange.nStart,
                                   aRange.nEnd, PM_NOREMOVE | PM_NOYIELD ) )
                    return true;

            // MOUSE and PAINT already handled, so skip further checks
            return false;
        }

        if ( nType & VclInputFlags::MOUSE )
        {
            if ( PeekMessageW( &aMsg, nullptr, WM_MOUSEFIRST, WM_MOUSELAST,
                                  PM_NOREMOVE | PM_NOYIELD ) )
                return true;
        }

        if ( nType & VclInputFlags::PAINT )
        {
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
    }

    return false;
}

SalFrame* WinSalInstance::CreateChildFrame( SystemParentData* pSystemParentData, SalFrameStyleFlags nSalFrameStyle )
{
    // to switch to Main-Thread
    return reinterpret_cast<SalFrame*>(static_cast<sal_IntPtr>(SendMessageW( mhComWnd, SAL_MSG_CREATEFRAME, static_cast<WPARAM>(nSalFrameStyle), reinterpret_cast<LPARAM>(pSystemParentData->hWnd) )));
}

SalFrame* WinSalInstance::CreateFrame( SalFrame* pParent, SalFrameStyleFlags nSalFrameStyle )
{
    // to switch to Main-Thread
    HWND hWndParent;
    if ( pParent )
        hWndParent = static_cast<WinSalFrame*>(pParent)->mhWnd;
    else
        hWndParent = nullptr;
    return reinterpret_cast<SalFrame*>(static_cast<sal_IntPtr>(SendMessageW( mhComWnd, SAL_MSG_CREATEFRAME, static_cast<WPARAM>(nSalFrameStyle), reinterpret_cast<LPARAM>(hWndParent) )));
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
    return reinterpret_cast<SalObject*>(static_cast<sal_IntPtr>(SendMessageW( mhComWnd, SAL_MSG_CREATEOBJECT, 0, reinterpret_cast<LPARAM>(static_cast<WinSalFrame*>(pParent)) )));
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
    if (Application::IsHeadlessModeEnabled())
        return;

    OUString system_path;
    osl::FileBase::RC rc = osl::FileBase::getSystemPathFromFileURL(rFileUrl, system_path);

    OSL_ENSURE(osl::FileBase::E_None == rc, "Invalid file url");

    if (osl::FileBase::E_None == rc)
    {
        IShellItem* pShellItem = nullptr;

        HRESULT hr = SHCreateItemFromParsingName(o3tl::toW(system_path.getStr()), nullptr, IID_PPV_ARGS(&pShellItem));

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
                info.pszAppID = o3tl::toW(sApplicationID.getStr());

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

std::shared_ptr<SalBitmap> WinSalInstance::CreateSalBitmap()
{
    if (OpenGLHelper::isVCLOpenGLEnabled())
        return std::make_shared<OpenGLSalBitmap>();
    else
        return std::make_shared<WinSalBitmap>();
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
            dwCount = GetFileVersionInfoSizeW(szPath, nullptr);
            if (dwCount != 0)
            {
                std::unique_ptr<char[]> ver(new char[dwCount]);
                if (GetFileVersionInfoW(szPath, 0, dwCount, ver.get()) != FALSE)
                {
                    void* pBlock = nullptr;
                    UINT dwBlockSz = 0;
                    if (VerQueryValueW(ver.get(), L"\\", &pBlock, &dwBlockSz) != FALSE && dwBlockSz >= sizeof(VS_FIXEDFILEINFO))
                    {
                        VS_FIXEDFILEINFO *vinfo = static_cast<VS_FIXEDFILEINFO *>(pBlock);
                        OUStringBuffer aVer;
                        aVer.append("Windows ");
                        aVer.append(static_cast<sal_Int32>(HIWORD(vinfo->dwProductVersionMS)));
                        aVer.append(".");
                        aVer.append(static_cast<sal_Int32>(LOWORD(vinfo->dwProductVersionMS)));
                        return aVer.makeStringAndClear();
                    }
                }
            }
        }
    }
    return "unknown";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
