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
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <tools/time.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/solarmutex.hxx>
#include <comphelper/windowserrorstring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <o3tl/char16_t2wchar_t.hxx>

#include <dndhelper.hxx>
#include <vcl/inputtypes.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/timer.hxx>
#include <vclpluginapi.h>

#include <win/dnd_source.hxx>
#include <win/dnd_target.hxx>
#include <win/wincomp.hxx>
#include <win/salids.hrc>
#include <win/saldata.hxx>
#include <win/salinst.h>
#include <win/salframe.h>
#include <win/salobj.h>
#include <win/saltimer.h>
#include <win/salbmp.h>
#include <win/winlayout.hxx>

#include <config_features.h>
#include <vcl/skia/SkiaHelper.hxx>
#if HAVE_FEATURE_SKIA
#include <config_skia.h>
#include <skia/salbmp.hxx>
#include <skia/win/gdiimpl.hxx>
#endif

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
        // This can nicely be done using MsgWaitForMultipleObjects, which is called in
        // m_condition.wait(). The 2nd one is
        // needed because if we don't reschedule, then we create deadlocks if a
        // Window's create/destroy is called via SendMessage() from another thread.
        // Have a look at the osl_waitCondition implementation for more info.
        do {
            // Calling Condition::reset frequently turns out to be a little expensive,
            // and the vast majority of the time there is no contention, so first
            // try just acquiring the mutex.
            if (m_aMutex.tryToAcquire())
                break;
            // reset condition *before* acquiring!
            m_condition.reset();
            if (m_aMutex.tryToAcquire())
                break;
            // wait for SalYieldMutex::release() to set the condition
            osl::Condition::Result res = m_condition.wait();
            assert(osl::Condition::Result::result_ok == res);
            (void) res;
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
        return SolarMutex::IsCurrentThread();
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
    : sal::systools::CoInitializeGuard(COINIT_APARTMENTTHREADED, false,
                                       sal::systools::CoInitializeGuard::WhenFailed::NoThrow)
         // put main thread in Single Threaded Apartment (STA)
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
    mpSharedTempFontItem = nullptr;
    mpOtherTempFontItem = nullptr;
    mbThemeChanged = false;     // true if visual theme was changed: throw away theme handles
    mbThemeMenuSupport = false;

    // init with NULL
    gdiplusToken = 0;

    initKeyCodeMap();

    SetSalData( this );
    initNWF();

    static Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

SalData::~SalData()
{
    deInitNWF();
    SetSalData( nullptr );

    if (gdiplusToken)
        Gdiplus::GdiplusShutdown(gdiplusToken);
}

bool OSSupportsDarkMode()
{
    bool bRet = false;
    if (HMODULE h_ntdll = GetModuleHandleW(L"ntdll.dll"))
    {
        typedef LONG(WINAPI* RtlGetVersion_t)(PRTL_OSVERSIONINFOW);
        if (auto RtlGetVersion
            = reinterpret_cast<RtlGetVersion_t>(GetProcAddress(h_ntdll, "RtlGetVersion")))
        {
            RTL_OSVERSIONINFOW vi2{};
            vi2.dwOSVersionInfoSize = sizeof(vi2);
            if (RtlGetVersion(&vi2) == 0)
            {
                if (vi2.dwMajorVersion > 10)
                    bRet = true;
                else if (vi2.dwMajorVersion == 10)
                {
                    if (vi2.dwMinorVersion > 0)
                        bRet = true;
                    else if (vi2.dwBuildNumber >= 18362)
                        bRet = true;
                }
            }
        }
    }
    return bRet;
}

enum PreferredAppMode
{
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
};

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

    static bool bSetAllowDarkMode = OSSupportsDarkMode(); // too early to additionally check LibreOffice's config
    if (bSetAllowDarkMode)
    {
        typedef PreferredAppMode(WINAPI* SetPreferredAppMode_t)(PreferredAppMode);
        if (HINSTANCE hUxthemeLib = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32))
        {
            if (auto SetPreferredAppMode = reinterpret_cast<SetPreferredAppMode_t>(GetProcAddress(hUxthemeLib, MAKEINTRESOURCEA(135))))
                SetPreferredAppMode(AllowDark);
            FreeLibrary(hUxthemeLib);
        }
    }

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
    m_bSupportsOpenGL = true;
#if HAVE_FEATURE_SKIA
    WinSkiaSalGraphicsImpl::prepareSkia();
#if SKIA_USE_BITMAP32
    if (SkiaHelper::isVCLSkiaEnabled())
        m_bSupportsBitmap32 = true;
#endif
#endif
}

WinSalInstance::~WinSalInstance()
{
    ImplFreeSalGDI();
    DestroyWindow( mhComWnd );
#if HAVE_FEATURE_SKIA
    SkiaHelper::cleanup();
#endif
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

// probably can't be static, because of SalTimer friend? (static gives C4211)
bool ImplSalYield(const bool bWait, const bool bHandleAllCurrentEvents)
{
    // used to abort further message processing on tick count wraps
    static sal_uInt32 nLastTicks = 0;

    // we should never yield in m_nNoYieldLock mode!
    const bool bNoYieldLock = (GetSalData()->mpInstance->m_nNoYieldLock > 0);
    assert(!bNoYieldLock);
    if (bNoYieldLock)
        return false;

    MSG aMsg;
    bool bWasMsg = false, bWasTimeoutMsg = false;
    WinSalTimer* pTimer = static_cast<WinSalTimer*>(ImplGetSVData()->maSchedCtx.mpSalTimer);

    sal_uInt32 nCurTicks = GetTickCount();

    do
    {
        if (!PeekMessageW(&aMsg, nullptr, 0, 0, PM_REMOVE))
            break;

        bWasMsg = true;
        TranslateMessage(&aMsg);
        LRESULT nRet = ImplSalDispatchMessage(&aMsg);

        bWasTimeoutMsg |= (SAL_MSG_TIMER_CALLBACK == aMsg.message) && static_cast<bool>(nRet);

        if (!bHandleAllCurrentEvents)
            break;

        if ((aMsg.time > nCurTicks) && (nLastTicks <= nCurTicks || aMsg.time < nLastTicks))
            break;
    }
    while( true );

    // 0ms timeouts are handled out-of-bounds to prevent busy-locking the
    // event loop with timeout messages.
    // We ensure we never handle more than one timeout per call.
    // This way we'll always process a normal system message.
    if ( !bWasTimeoutMsg && pTimer && pTimer->IsDirectTimeout() )
    {
        pTimer->ImplHandleElapsedTimer();
        bWasMsg = true;
    }

    nLastTicks = nCurTicks;

    if ( bWait && !bWasMsg )
    {
        switch (GetMessageW(&aMsg, nullptr, 0, 0))
        {
            case -1:
                SAL_WARN("vcl.schedule", "GetMessageW failed: " << WindowsErrorString(GetLastError()));
                // should we std::abort() / SalAbort here?
                break;
            case 0:
                SAL_INFO("vcl.schedule", "GetMessageW received WM_QUIT while waiting");
                break;
            default:
                bWasMsg = true;
                TranslateMessage(&aMsg);
                ImplSalDispatchMessage(&aMsg);
                break;
        }
    }

    // If we enabled ForceRealTimer mode skipping our direct timeout processing,
    // mainly because some Windows API call spawns its own nested message loop,
    // switch back to our own processing (like after window resize or move)
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
    const bool bIsOtherThreadMessage = InSendMessage();
    LRESULT nRet = 0;
    WinSalInstance *pInst = GetSalData()->mpInstance;
    WinSalTimer *const pTimer = static_cast<WinSalTimer*>( ImplGetSVData()->maSchedCtx.mpSalTimer );

    SAL_INFO("vcl.gdi.wndproc", "SalComWndProc(nMsg=" << nMsg << ", wParam=" << wParam
                                << ", lParam=" << lParam << "); inSendMsg: " << bIsOtherThreadMessage);

    if (ImplGetSVData()->mbDeInit)
    {
        SAL_WARN("vcl.gdi.wndproc", "ignoring timer event because we are shutting down");
        return 0;
    }

    switch ( nMsg )
    {
        case SAL_MSG_THREADYIELD:
            assert( !static_cast<bool>(wParam) );
            nRet = static_cast<LRESULT>(ImplSalYield(
                false, static_cast<bool>( lParam ) ));
            break;

        case SAL_MSG_STARTTIMER:
        {
            auto const nParam = static_cast<sal_uInt64>( lParam );
            sal_uInt64 nTime = tools::Time::GetSystemTicks();
            if ( nTime < nParam )
                nTime = nParam - nTime;
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

bool WinSalInstance::AnyInput( VclInputFlags nType )
{
    if ( nType & VclInputFlags::TIMER )
    {
        const WinSalTimer* pTimer = static_cast<WinSalTimer*>( ImplGetSVData()->maSchedCtx.mpSalTimer );
        if ( pTimer && pTimer->HasTimerElapsed() )
            return true;
    }

    // Note: Do not use PeekMessage(), despite the name it may dispatch events,
    // even with PM_NOREMOVE specified, which may lead to unwanted recursion.

    if ( (nType & VCL_INPUT_ANY) == VCL_INPUT_ANY )
    {
        // revert bugfix for #108919# which never reported timeouts when called from the timer handler
        // which made the application completely unresponsive during background formatting
        if ( GetQueueStatus( QS_ALLEVENTS ))
            return true;
    }
    else
    {
        UINT flags = 0;

        // This code previously considered modifier keys as OTHER,
        // but that makes this hard to do without PeekMessage,
        // is inconsistent with the X11 backend, and I see no good reason.
        if ( nType & VclInputFlags::KEYBOARD )
            flags |= QS_KEY;

        if ( nType & VclInputFlags::MOUSE )
            flags |= QS_MOUSE;

        if ( nType & VclInputFlags::PAINT )
            flags |= QS_PAINT;

        if ( nType & VclInputFlags::TIMER )
            flags |= QS_TIMER;

        if( nType & VclInputFlags::OTHER )
            flags |= QS_ALLEVENTS & ~QS_KEY & ~QS_MOUSE & ~QS_PAINT & ~QS_TIMER;

        if( GetQueueStatus( flags ))
            return true;
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
                OUString sApplicationID("TheDocumentFoundation.LibreOffice." + sApplicationName);

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
#if HAVE_FEATURE_SKIA
    if (SkiaHelper::isVCLSkiaEnabled())
        return std::make_shared<SkiaSalBitmap>();
    else
#endif
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

typedef LONG NTSTATUS;
typedef NTSTATUS(WINAPI* RtlGetVersion_t)(PRTL_OSVERSIONINFOW);
constexpr NTSTATUS STATUS_SUCCESS = 0x00000000;

OUString WinSalInstance::getOSVersion()
{
    OUStringBuffer aVer(50); // capacity for string like "Windows 6.1 Service Pack 1 build 7601"
    aVer.append("Windows ");
    // GetVersion(Ex) and VersionHelpers (based on VerifyVersionInfo) API are
    // subject to manifest-based behavior since Windows 8.1, so give wrong results.
    // Another approach would be to use NetWkstaGetInfo, but that has some small
    // reported delays (some milliseconds), and might get slower in domains with
    // poor network connections.
    // So go with a solution described at https://msdn.microsoft.com/en-us/library/ms724429
    bool bHaveVerFromKernel32 = false;
    if (HMODULE h_kernel32 = GetModuleHandleW(L"kernel32.dll"))
    {
        wchar_t szPath[MAX_PATH];
        DWORD dwCount = GetModuleFileNameW(h_kernel32, szPath, SAL_N_ELEMENTS(szPath));
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
                        VS_FIXEDFILEINFO* vi1 = static_cast<VS_FIXEDFILEINFO*>(pBlock);
                        aVer.append(OUString::number(HIWORD(vi1->dwProductVersionMS)) + "."
                                    + OUString::number(LOWORD(vi1->dwProductVersionMS)));
                        bHaveVerFromKernel32 = true;
                    }
                }
            }
        }
    }
    // Now use RtlGetVersion (which is not subject to deprecation for GetVersion(Ex) API)
    // to get build number and SP info
    bool bHaveVerFromRtlGetVersion = false;
    if (HMODULE h_ntdll = GetModuleHandleW(L"ntdll.dll"))
    {
        if (auto RtlGetVersion
            = reinterpret_cast<RtlGetVersion_t>(GetProcAddress(h_ntdll, "RtlGetVersion")))
        {
            RTL_OSVERSIONINFOW vi2{}; // initialize with zeroes - a better alternative to memset
            vi2.dwOSVersionInfoSize = sizeof(vi2);
            if (STATUS_SUCCESS == RtlGetVersion(&vi2))
            {
                if (!bHaveVerFromKernel32) // we failed above; let's hope this would be useful
                    aVer.append(OUString::number(vi2.dwMajorVersion) + "."
                                + OUString::number(vi2.dwMinorVersion));
                aVer.append(" ");
                if (vi2.szCSDVersion[0])
                    aVer.append(OUString::Concat(o3tl::toU(vi2.szCSDVersion)) + " ");
                aVer.append("Build " + OUString::number(vi2.dwBuildNumber));
                bHaveVerFromRtlGetVersion = true;
            }
        }
    }
    if (!bHaveVerFromKernel32 && !bHaveVerFromRtlGetVersion)
        aVer.append("unknown");
    return aVer.makeStringAndClear();
}

void WinSalInstance::BeforeAbort(const OUString&, bool)
{
    ImplFreeSalGDI();
}

css::uno::Reference<css::uno::XInterface> WinSalInstance::ImplCreateDragSource(const SystemEnvData* pSysEnv)
{
    return vcl::OleDnDHelper(new DragSource(comphelper::getProcessComponentContext()),
                             reinterpret_cast<sal_IntPtr>(pSysEnv->hWnd), vcl::DragOrDrop::Drag);
}

css::uno::Reference<css::uno::XInterface> WinSalInstance::ImplCreateDropTarget(const SystemEnvData* pSysEnv)
{
    return vcl::OleDnDHelper(new DropTarget(comphelper::getProcessComponentContext()),
                             reinterpret_cast<sal_IntPtr>(pSysEnv->hWnd), vcl::DragOrDrop::Drop);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
