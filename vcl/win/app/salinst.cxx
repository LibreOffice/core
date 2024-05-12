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
#include <comphelper/processfactory.hxx>
#include <comphelper/solarmutex.hxx>
#include <comphelper/windowserrorstring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <o3tl/char16_t2wchar_t.hxx>
#include <o3tl/temporary.hxx>

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
    auto initKey = [this](wchar_t ch, sal_uInt16 key)
    {
        if (UINT vkey = LOWORD(VkKeyScanW(ch)); vkey < 0xffff)
            maVKMap[vkey] = key;
    };

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
    initKey( L'}', KEY_RIGHTCURLYBRACKET );
    initKey( L'#', KEY_NUMBERSIGN);
    initKey( L':', KEY_COLON );
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
    mpInstance = nullptr;  // pointer of first instance
    mpFirstFrame = nullptr;     // pointer of first frame
    mpFirstObject = nullptr;    // pointer of first object window
    mpFirstVD = nullptr;        // first VirDev
    mpFirstPrinter = nullptr;   // first printing printer
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

namespace {

enum PreferredAppMode
{
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
};

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

void WinSalInstance::AfterAppInit()
{
// (1) Ideally this would be done at the place that creates the thread, but since this thread is normally
// just the default/main thread, that is not possible.
// (2) Don't do this on unix, where it causes tools like pstree on Linux to
// confusingly report soffice.bin as VCL Main instead.
    osl_setThreadName("VCL Main");
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

namespace
{
struct NoYieldLockGuard
{
    NoYieldLockGuard()
        : counter(InSendMessage() ? GetSalData()->mpInstance->m_nNoYieldLock : dummy())
    {
        ++counter;
    }
    ~NoYieldLockGuard() { --counter; }
    static decltype(WinSalInstance::m_nNoYieldLock)& dummy()
    {
        DBG_TESTSOLARMUTEX(); // called when !InSendMessage()
        static decltype(WinSalInstance::m_nNoYieldLock) n = 0;
        return n;
    }
    decltype(WinSalInstance::m_nNoYieldLock)& counter;
};
}

LRESULT CALLBACK SalComWndProc( HWND, UINT nMsg, WPARAM wParam, LPARAM lParam, bool& rDef )
{
    LRESULT nRet = 0;
    WinSalTimer *const pTimer = static_cast<WinSalTimer*>( ImplGetSVData()->maSchedCtx.mpSalTimer );

    SAL_INFO("vcl.gdi.wndproc", "SalComWndProc(nMsg=" << nMsg << ", wParam=" << wParam
                                << ", lParam=" << lParam << "); inSendMsg: " << InSendMessage());

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

        case (SAL_MSG_CREATEFRAME):
            {
                NoYieldLockGuard g;
                nRet = reinterpret_cast<LRESULT>(
                    ImplSalCreateFrame(GetSalData()->mpInstance, reinterpret_cast<HWND>(lParam),
                                       static_cast<SalFrameStyleFlags>(wParam)));
            }
            break;
        case (SAL_MSG_RECREATEHWND):
            {
                NoYieldLockGuard g;
                nRet = reinterpret_cast<LRESULT>(ImplSalReCreateHWND(
                    reinterpret_cast<HWND>(wParam), reinterpret_cast<HWND>(lParam), false));
            }
            break;
        case (SAL_MSG_RECREATECHILDHWND):
            {
                NoYieldLockGuard g;
                nRet = reinterpret_cast<LRESULT>(ImplSalReCreateHWND(
                    reinterpret_cast<HWND>(wParam), reinterpret_cast<HWND>(lParam), true));
            }
            break;
        case (SAL_MSG_DESTROYFRAME):
            {
                NoYieldLockGuard g;
                delete reinterpret_cast<SalFrame*>(lParam);
            }
            break;

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

        case (SAL_MSG_CREATEOBJECT):
            {
                NoYieldLockGuard g;
                nRet = reinterpret_cast<LRESULT>(ImplSalCreateObject(
                    GetSalData()->mpInstance, reinterpret_cast<WinSalFrame*>(lParam)));
            }
            break;
        case (SAL_MSG_DESTROYOBJECT):
            {
                NoYieldLockGuard g;
                delete reinterpret_cast<SalObject*>(lParam);
            }
            break;
        case (SAL_MSG_GETCACHEDDC):
            {
                NoYieldLockGuard g;
                nRet = reinterpret_cast<LRESULT>(
                    GetDCEx(reinterpret_cast<HWND>(wParam), nullptr, 0x00000002L));
            }
            break;
        case (SAL_MSG_RELEASEDC):
            {
                NoYieldLockGuard g;
                ReleaseDC(reinterpret_cast<HWND>(wParam), reinterpret_cast<HDC>(lParam));
            }
            break;

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

static OUString getWinArch()
{
    USHORT nNativeMachine = IMAGE_FILE_MACHINE_UNKNOWN;

    using LPFN_ISWOW64PROCESS2 = BOOL(WINAPI*)(HANDLE, USHORT*, USHORT*);
    auto fnIsWow64Process2 = reinterpret_cast<LPFN_ISWOW64PROCESS2>(
        GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "IsWow64Process2"));
    if (fnIsWow64Process2)
        fnIsWow64Process2(GetCurrentProcess(), &o3tl::temporary(USHORT()), &nNativeMachine);

    if (nNativeMachine == IMAGE_FILE_MACHINE_UNKNOWN)
    {
#if _WIN64

        nNativeMachine = IMAGE_FILE_MACHINE_AMD64;

#else

        BOOL isWow64 = FALSE;

        IsWow64Process(GetCurrentProcess(), &isWow64);

        if (isWow64)
            nNativeMachine = IMAGE_FILE_MACHINE_AMD64; // 32-bit process on 64-bit Windows
        else
            nNativeMachine = IMAGE_FILE_MACHINE_I386;

#endif
    }

    switch (nNativeMachine)
    {
        case IMAGE_FILE_MACHINE_I386:
            return u" X86_32"_ustr;
        case IMAGE_FILE_MACHINE_R3000:
            return u" R3000"_ustr;
        case IMAGE_FILE_MACHINE_R4000:
            return u" R4000"_ustr;
        case IMAGE_FILE_MACHINE_R10000:
            return u" R10000"_ustr;
        case IMAGE_FILE_MACHINE_WCEMIPSV2:
            return u" WCEMIPSV2"_ustr;
        case IMAGE_FILE_MACHINE_ALPHA:
            return u" ALPHA"_ustr;
        case IMAGE_FILE_MACHINE_SH3:
            return u" SH3"_ustr;
        case IMAGE_FILE_MACHINE_SH3DSP:
            return u" SH3DSP"_ustr;
        case IMAGE_FILE_MACHINE_SH3E:
            return u" SH3E"_ustr;
        case IMAGE_FILE_MACHINE_SH4:
            return u" SH4"_ustr;
        case IMAGE_FILE_MACHINE_SH5:
            return u" SH5"_ustr;
        case IMAGE_FILE_MACHINE_ARM:
            return u" ARM"_ustr;
        case IMAGE_FILE_MACHINE_THUMB:
            return u" THUMB"_ustr;
        case IMAGE_FILE_MACHINE_ARMNT:
            return u" ARMNT"_ustr;
        case IMAGE_FILE_MACHINE_AM33:
            return u" AM33"_ustr;
        case IMAGE_FILE_MACHINE_POWERPC:
            return u" POWERPC"_ustr;
        case IMAGE_FILE_MACHINE_POWERPCFP:
            return u" POWERPCFP"_ustr;
        case IMAGE_FILE_MACHINE_IA64:
            return u" IA64"_ustr;
        case IMAGE_FILE_MACHINE_MIPS16:
            return u" MIPS16"_ustr;
        case IMAGE_FILE_MACHINE_ALPHA64:
            return u" ALPHA64"_ustr;
        case IMAGE_FILE_MACHINE_MIPSFPU:
            return u" MIPSFPU"_ustr;
        case IMAGE_FILE_MACHINE_MIPSFPU16:
            return u" MIPSFPU16"_ustr;
        case IMAGE_FILE_MACHINE_TRICORE:
            return u" TRICORE"_ustr;
        case IMAGE_FILE_MACHINE_CEF:
            return u" CEF"_ustr;
        case IMAGE_FILE_MACHINE_EBC:
            return u" EBC"_ustr;
        case IMAGE_FILE_MACHINE_AMD64:
            return u" X86_64"_ustr;
        case IMAGE_FILE_MACHINE_M32R:
            return u" M32R"_ustr;
        case IMAGE_FILE_MACHINE_ARM64:
            return u" ARM64"_ustr;
        case IMAGE_FILE_MACHINE_CEE:
            return u" CEE"_ustr;
        default:
            assert(!"Yet unhandled case");
            return OUString();
    }
}

static OUString getOSVersionString(const OUString& aNtVersionString, DWORD nBuildNumber)
{
    OUStringBuffer result = u"Windows";
    if (aNtVersionString == "6.1")
        result.append(" 7 Service Pack 1");
    else if (aNtVersionString == "6.2")
        result.append(" 8");
    else if (aNtVersionString == "6.3")
        result.append(" 8.1");
    else if (aNtVersionString == "10.0")
    {
        if (nBuildNumber >= 22000)
            result.append(" 11");
        else
            result.append(" 10");
    }
    else // We don't know what Windows it is
        result.append(" unknown");

    result.append(getWinArch());

    if (!aNtVersionString.isEmpty() || nBuildNumber)
    {
        result.append(" (");
        if (!aNtVersionString.isEmpty())
        {
            result.append(aNtVersionString);
            if (nBuildNumber)
                result.append(" ");
        }
        if (nBuildNumber)
            result.append("build " + OUString::number(nBuildNumber));
        result.append(")");
    }

    return result.makeStringAndClear();
}

OUString WinSalInstance::getOSVersion()
{
    static const OUString result = []
    {
        // GetVersion(Ex) and VersionHelpers (based on VerifyVersionInfo) API are
        // subject to manifest-based behavior since Windows 8.1, so give wrong results.
        // Another approach would be to use NetWkstaGetInfo, but that has some small
        // reported delays (some milliseconds), and might get slower in domains with
        // poor network connections.
        // So go with a solution described at https://msdn.microsoft.com/en-us/library/ms724429
        bool bHaveVerFromKernel32 = false;
        OUString aNtVersion;
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
                        if (VerQueryValueW(ver.get(), L"\\", &pBlock, &dwBlockSz) != FALSE
                            && dwBlockSz >= sizeof(VS_FIXEDFILEINFO))
                        {
                            VS_FIXEDFILEINFO* vi1 = static_cast<VS_FIXEDFILEINFO*>(pBlock);
                            aNtVersion = (OUString::number(HIWORD(vi1->dwProductVersionMS)) + "."
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
        DWORD nBuildNumber = 0;
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
                        aNtVersion = (OUString::number(vi2.dwMajorVersion) + "."
                                      + OUString::number(vi2.dwMinorVersion));
                    nBuildNumber = vi2.dwBuildNumber;
                    bHaveVerFromRtlGetVersion = true;
                }
            }
        }
        return getOSVersionString(aNtVersion, nBuildNumber);
    }();
    return result;
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
