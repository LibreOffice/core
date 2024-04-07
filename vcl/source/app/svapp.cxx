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

#include <config_features.h>

#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <osl/module.hxx>
#include <rtl/ustrbuf.hxx>

#include <sal/log.hxx>

#include <tools/debug.hxx>
#include <tools/time.hxx>
#include <tools/stream.hxx>
#include <tools/json_writer.hxx>

#include <unotools/configmgr.hxx>
#include <unotools/resmgr.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/syslocaleoptions.hxx>

#include <vcl/toolkit/dialog.hxx>
#include <vcl/dialoghelper.hxx>
#include <vcl/lok.hxx>
#include <vcl/toolkit/floatwin.hxx>
#include <vcl/settings.hxx>
#include <vcl/keycod.hxx>
#include <vcl/event.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/toolkit/unowrap.hxx>
#include <vcl/timer.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/skia/SkiaHelper.hxx>

#include <salinst.hxx>
#include <salframe.hxx>
#include <salsys.hxx>
#include <svdata.hxx>
#include <displayconnectiondispatch.hxx>
#include <window.h>
#include <accmgr.hxx>
#include <strings.hrc>
#include <strings.hxx>
#if OSL_DEBUG_LEVEL > 0
#include <schedulerimpl.hxx>
#endif

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/awt/XToolkit.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/solarmutex.hxx>
#include <osl/process.h>

#include <cassert>
#include <limits>
#include <string_view>
#include <utility>
#include <thread>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {
void InitSettings(ImplSVData* pSVData);
}

// keycodes handled internally by VCL
vcl::KeyCode const ReservedKeys[]
{
                vcl::KeyCode(KEY_F1,0)                  ,
                vcl::KeyCode(KEY_F1,KEY_SHIFT)          ,
                vcl::KeyCode(KEY_F1,KEY_MOD1)           ,
                vcl::KeyCode(KEY_F2,KEY_SHIFT)          ,
                vcl::KeyCode(KEY_F4,KEY_MOD1)           ,
                vcl::KeyCode(KEY_F4,KEY_MOD2)           ,
                vcl::KeyCode(KEY_F4,KEY_MOD1|KEY_MOD2)  ,
                vcl::KeyCode(KEY_F6,0)                  ,
                vcl::KeyCode(KEY_F6,KEY_MOD1)           ,
                vcl::KeyCode(KEY_F6,KEY_SHIFT)          ,
                vcl::KeyCode(KEY_F6,KEY_MOD1|KEY_SHIFT) ,
                vcl::KeyCode(KEY_F10,0)
#ifdef UNX
                ,
                vcl::KeyCode(KEY_1,KEY_SHIFT|KEY_MOD1),
                vcl::KeyCode(KEY_2,KEY_SHIFT|KEY_MOD1),
                vcl::KeyCode(KEY_3,KEY_SHIFT|KEY_MOD1),
                vcl::KeyCode(KEY_4,KEY_SHIFT|KEY_MOD1),
                vcl::KeyCode(KEY_5,KEY_SHIFT|KEY_MOD1),
                vcl::KeyCode(KEY_6,KEY_SHIFT|KEY_MOD1),
                vcl::KeyCode(KEY_7,KEY_SHIFT|KEY_MOD1),
                vcl::KeyCode(KEY_8,KEY_SHIFT|KEY_MOD1),
                vcl::KeyCode(KEY_9,KEY_SHIFT|KEY_MOD1),
                vcl::KeyCode(KEY_0,KEY_SHIFT|KEY_MOD1),
                vcl::KeyCode(KEY_ADD,KEY_SHIFT|KEY_MOD1)
#endif
};

extern "C" {
    typedef UnoWrapperBase* (*FN_TkCreateUnoWrapper)();
}

struct ImplPostEventData
{
    VclPtr<vcl::Window> mpWin;
    ImplSVEvent *   mnEventId;
    MouseEvent      maMouseEvent;
    VclEventId      mnEvent;
    KeyEvent        maKeyEvent;
    GestureEventPan maGestureEvent;

    ImplPostEventData(VclEventId nEvent, vcl::Window* pWin, const KeyEvent& rKeyEvent)
        : mpWin(pWin)
        , mnEventId(nullptr)
        , mnEvent(nEvent)
        , maKeyEvent(rKeyEvent)
    {}
    ImplPostEventData(VclEventId nEvent, vcl::Window* pWin, const MouseEvent& rMouseEvent)
        : mpWin(pWin)
        , mnEventId(nullptr)
        , maMouseEvent(rMouseEvent)
        , mnEvent(nEvent)
    {}
    ImplPostEventData(VclEventId nEvent, vcl::Window* pWin, const GestureEventPan& rGestureEvent)
        : mpWin(pWin)
        , mnEventId(nullptr)
        , mnEvent(nEvent)
        , maGestureEvent(rGestureEvent)
    {}
};

Application* GetpApp()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData )
        return nullptr;
    return pSVData->mpApp;
}

Application::Application()
{
    // useful for themes at least, perhaps extensions too
    OUString aVar("LIBO_VERSION"), aValue(LIBO_VERSION_DOTTED);
    osl_setEnvironment(aVar.pData, aValue.pData);

    ImplGetSVData()->mpApp = this;
    m_pCallbackData = nullptr;
    m_pCallback = nullptr;
}

Application::~Application()
{
    ImplDeInitSVData();
    ImplGetSVData()->mpApp = nullptr;
}

int Application::Main()
{
    SAL_WARN("vcl", "Application is a base class and should be overridden.");
    return EXIT_SUCCESS;
}

bool Application::QueryExit()
{
    WorkWindow* pAppWin = ImplGetSVData()->maFrameData.mpAppWin;

    // call the close handler of the application window
    if ( pAppWin )
        return pAppWin->Close();
    else
        return true;
}

void Application::Shutdown()
{
}

void Application::Init()
{
}

void Application::InitFinished()
{
}

void Application::DeInit()
{
}

sal_uInt16 Application::GetCommandLineParamCount()
{
    return static_cast<sal_uInt16>(osl_getCommandArgCount());
}

OUString Application::GetCommandLineParam( sal_uInt16 nParam )
{
    OUString aParam;
    osl_getCommandArg( nParam, &aParam.pData );
    return aParam;
}

OUString Application::GetAppFileName()
{
    ImplSVData* pSVData = ImplGetSVData();
    SAL_WARN_IF( !pSVData->maAppData.mxAppFileName, "vcl", "AppFileName should be set to something after SVMain!" );
    if ( pSVData->maAppData.mxAppFileName )
        return *pSVData->maAppData.mxAppFileName;

    /*
     *  provide a fallback for people without initialized vcl here (like setup
     *  in responsefile mode)
     */
    OUString aAppFileName;
    OUString aExeFileName;
    osl_getExecutableFile(&aExeFileName.pData);

    // convert path to native file format
    osl::FileBase::getSystemPathFromFileURL(aExeFileName, aAppFileName);

    return aAppFileName;
}

void Application::Exception( ExceptionCategory nCategory )
{
    switch ( nCategory )
    {
        // System has precedence (so do nothing)
        case ExceptionCategory::System:
        case ExceptionCategory::UserInterface:
            break;
        default:
            Abort("Unknown Error");
            break;
    }
}

void Application::Abort( const OUString& rErrorText )
{
    //HACK: Dump core iff --norestore command line argument is given (assuming
    // this process is run by developers who are interested in cores, vs. end
    // users who are not):
#if OSL_DEBUG_LEVEL > 0
    bool dumpCore = true;
#else
    bool dumpCore = false;
    sal_uInt16 n = GetCommandLineParamCount();
    for (sal_uInt16 i = 0; i != n; ++i) {
        if (GetCommandLineParam(i) == "--norestore") {
            dumpCore = true;
            break;
        }
    }
#endif

    SalAbort( rErrorText, dumpCore );
}

size_t Application::GetReservedKeyCodeCount()
{
    return SAL_N_ELEMENTS(ReservedKeys);
}

const vcl::KeyCode* Application::GetReservedKeyCode( size_t i )
{
    if( i >= GetReservedKeyCodeCount() )
        return nullptr;
    else
        return &ReservedKeys[i];
}

IMPL_STATIC_LINK_NOARG( ImplSVAppData, ImplEndAllPopupsMsg, void*, void )
{
    ImplSVData* pSVData = ImplGetSVData();
    while (pSVData->mpWinData->mpFirstFloat)
        pSVData->mpWinData->mpFirstFloat->EndPopupMode(FloatWinPopupEndFlags::Cancel);
}

IMPL_STATIC_LINK_NOARG( ImplSVAppData, ImplEndAllDialogsMsg, void*, void )
{
    vcl::Window* pAppWindow = Application::GetFirstTopLevelWindow();
    while (pAppWindow)
    {
        vcl::EndAllDialogs(pAppWindow);
        pAppWindow = Application::GetNextTopLevelWindow(pAppWindow);
    }
}

void Application::EndAllDialogs()
{
    Application::PostUserEvent( LINK( nullptr, ImplSVAppData, ImplEndAllDialogsMsg ) );
}

void Application::EndAllPopups()
{
    Application::PostUserEvent( LINK( nullptr, ImplSVAppData, ImplEndAllPopupsMsg ) );
}

void Application::notifyWindow(vcl::LOKWindowId /*nLOKWindowId*/,
                               const OUString& /*rAction*/,
                               const std::vector<vcl::LOKPayloadItem>& /*rPayload = std::vector<LOKPayloadItem>()*/) const
{
}

void Application::libreOfficeKitViewCallback(int nType, const char* pPayload) const
{
    if (!comphelper::LibreOfficeKit::isActive())
        return;

    if (m_pCallback)
    {
        m_pCallback(nType, pPayload, m_pCallbackData);
    }
}


namespace
{
    VclPtr<vcl::Window> GetEventWindow()
    {
        VclPtr<vcl::Window> xWin(Application::GetFirstTopLevelWindow());
        while (xWin)
        {
            if (xWin->IsVisible())
                break;
            xWin.reset(Application::GetNextTopLevelWindow(xWin));
        }
        return xWin;
    }

    bool InjectKeyEvent(SvStream& rStream)
    {
        VclPtr<vcl::Window> xWin(GetEventWindow());
        if (!xWin)
            return false;

        // skip the first available cycle and insert on the next one when we
        // are trying the initial event, flagged by a triggered but undeleted
        // mpEventTestingIdle
        ImplSVData* pSVData = ImplGetSVData();
        if (pSVData->maAppData.mpEventTestingIdle)
        {
            delete pSVData->maAppData.mpEventTestingIdle;
            pSVData->maAppData.mpEventTestingIdle = nullptr;
            return false;
        }

        sal_uInt16 nCode, nCharCode;
        rStream.ReadUInt16(nCode);
        rStream.ReadUInt16(nCharCode);
        if (!rStream.good())
            return false;

        KeyEvent aVCLKeyEvt(nCharCode, nCode);
        Application::PostKeyEvent(VclEventId::WindowKeyInput, xWin.get(), &aVCLKeyEvt);
        Application::PostKeyEvent(VclEventId::WindowKeyUp, xWin.get(), &aVCLKeyEvt);
        return true;
    }

    void CloseDialogsAndQuit()
    {
        Application::EndAllPopups();
        Application::EndAllDialogs();
        Application::PostUserEvent( LINK( nullptr, ImplSVAppData, ImplPrepareExitMsg ) );
    }
}

IMPL_LINK_NOARG(ImplSVAppData, VclEventTestingHdl, Timer *, void)
{
    if (Application::AnyInput())
    {
        mpEventTestingIdle->Start();
    }
    else
    {
        Application::PostUserEvent( LINK( nullptr, ImplSVAppData, ImplVclEventTestingHdl ) );
    }
}

IMPL_STATIC_LINK_NOARG( ImplSVAppData, ImplVclEventTestingHdl, void*, void )
{
    ImplSVData* pSVData = ImplGetSVData();
    SAL_INFO("vcl.eventtesting", "EventTestLimit is " << pSVData->maAppData.mnEventTestLimit);
    if (pSVData->maAppData.mnEventTestLimit == 0)
    {
        delete pSVData->maAppData.mpEventTestInput;
        SAL_INFO("vcl.eventtesting", "Event Limit reached, exiting" << pSVData->maAppData.mnEventTestLimit);
        CloseDialogsAndQuit();
    }
    else
    {
        if (InjectKeyEvent(*pSVData->maAppData.mpEventTestInput))
            --pSVData->maAppData.mnEventTestLimit;
        if (!pSVData->maAppData.mpEventTestInput->good())
        {
            SAL_INFO("vcl.eventtesting", "Event Input exhausted, exit next cycle");
            pSVData->maAppData.mnEventTestLimit = 0;
        }
        Application::PostUserEvent( LINK( nullptr, ImplSVAppData, ImplVclEventTestingHdl ) );
    }
}

IMPL_STATIC_LINK_NOARG( ImplSVAppData, ImplPrepareExitMsg, void*, void )
{
    //now close top level frames
    (void)GetpApp()->QueryExit();
}

void Application::Execute()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mbInAppExecute = true;
    pSVData->maAppData.mbAppQuit = false;

    if (Application::IsEventTestingModeEnabled())
    {
        pSVData->maAppData.mnEventTestLimit = 50;
        pSVData->maAppData.mpEventTestingIdle = new Idle("eventtesting");
        pSVData->maAppData.mpEventTestingIdle->SetInvokeHandler(LINK(&(pSVData->maAppData), ImplSVAppData, VclEventTestingHdl));
        pSVData->maAppData.mpEventTestingIdle->SetPriority(TaskPriority::HIGH_IDLE);
        pSVData->maAppData.mpEventTestInput = new SvFileStream("eventtesting", StreamMode::READ);
        pSVData->maAppData.mpEventTestingIdle->Start();
    }

    int nExitCode = 0;
    if (!pSVData->mpDefInst->DoExecute(nExitCode))
    {
        if (Application::IsOnSystemEventLoop())
        {
            SAL_WARN("vcl.schedule", "Can't omit DoExecute when running on system event loop!");
            std::abort();
        }
        while (!pSVData->maAppData.mbAppQuit)
        {
            Application::Yield();
            SolarMutexReleaser releaser; // Give a chance for the waiting threads to lock the mutex
            pSVData->m_inExecuteCondtion.set();
        }
    }

    pSVData->maAppData.mbInAppExecute = false;

    GetpApp()->Shutdown();
}

static bool ImplYield(bool i_bWait, bool i_bAllEvents)
{
    ImplSVData* pSVData = ImplGetSVData();

    SAL_INFO("vcl.schedule", "Enter ImplYield: " << (i_bWait ? "wait" : "no wait") <<
             ": " << (i_bAllEvents ? "all events" : "one event"));

    // there's a data race here on WNT only because ImplYield may be
    // called without SolarMutex; but the only remaining use of mnDispatchLevel
    // is in OSX specific code
    pSVData->maAppData.mnDispatchLevel++;

    // do not wait for events if application was already quit; in that
    // case only dispatch events already available
    bool bProcessedEvent = pSVData->mpDefInst->DoYield(
            i_bWait && !pSVData->maAppData.mbAppQuit, i_bAllEvents );

    pSVData->maAppData.mnDispatchLevel--;

    DBG_TESTSOLARMUTEX(); // must be locked on return from Yield

    SAL_INFO("vcl.schedule", "Leave ImplYield with return " << bProcessedEvent );
    return bProcessedEvent;
}

bool Application::Reschedule( bool i_bAllEvents )
{
    static const bool bAbort = Application::IsOnSystemEventLoop();
    if (bAbort)
    {
        SAL_WARN("vcl.schedule", "Application::Reschedule(" << i_bAllEvents << ")");
        std::abort();
    }
    return ImplYield(false, i_bAllEvents);
}

bool Application::IsOnSystemEventLoop()
{
    return ImplGetSVData()->maAppData.m_bUseSystemLoop;
}

void Scheduler::ProcessEventsToIdle()
{
    int nSanity = 1;
    while (ImplYield(false, true))
    {
        if (0 == ++nSanity % 1000)
        {
            SAL_WARN("vcl.schedule", "ProcessEventsToIdle: " << nSanity);
        }
    }
#if OSL_DEBUG_LEVEL > 0
    // If we yield from a non-main thread we just can guarantee that all idle
    // events were processed at some point, but our check can't prevent further
    // processing in the main thread, which may add new events, so skip it.
    const ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->mpDefInst->IsMainThread() )
        return;
    for (int nTaskPriority = 0; nTaskPriority < PRIO_COUNT; ++nTaskPriority)
    {
        const ImplSchedulerData* pSchedulerData = pSVData->maSchedCtx.mpFirstSchedulerData[nTaskPriority];
        while (pSchedulerData)
        {
            assert(!pSchedulerData->mbInScheduler);
            if (pSchedulerData->mpTask)
            {
                Idle *pIdle = dynamic_cast<Idle*>(pSchedulerData->mpTask);
                if (pIdle && pIdle->IsActive())
                {
                    SAL_WARN("vcl.schedule",
                             "Unprocessed Idle: "
                                 << pIdle << " "
                                 << (pIdle->GetDebugName() ? pIdle->GetDebugName() : "(nullptr)"));
                }
            }
            pSchedulerData = pSchedulerData->mpNext;
        }
    }
#endif
}

extern "C" {
/// used by unit tests that test only via the LOK API
SAL_DLLPUBLIC_EXPORT void unit_lok_process_events_to_idle()
{
    const SolarMutexGuard aGuard;
    Scheduler::ProcessEventsToIdle();
}
}

void Application::Yield()
{
    static const bool bAbort = Application::IsOnSystemEventLoop();
    if (bAbort)
    {
        SAL_WARN("vcl.schedule", "Application::Yield()");
        std::abort();
    }
    ImplYield(true, false);
}

IMPL_STATIC_LINK_NOARG( ImplSVAppData, ImplQuitMsg, void*, void )
{
    assert(ImplGetSVData()->maAppData.mbAppQuit);
    ImplGetSVData()->mpDefInst->DoQuit();
}

void Application::Quit()
{
    ImplGetSVData()->maAppData.mbAppQuit = true;
    Application::PostUserEvent( LINK( nullptr, ImplSVAppData, ImplQuitMsg ) );
}

comphelper::SolarMutex& Application::GetSolarMutex()
{
    ImplSVData* pSVData = ImplGetSVData();
    return *(pSVData->mpDefInst->GetYieldMutex());
}

bool Application::IsMainThread()
{
    return ImplGetSVData()->mnMainThreadId == osl::Thread::getCurrentIdentifier();
}

sal_uInt32 Application::ReleaseSolarMutex()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->mpDefInst->ReleaseYieldMutexAll();
}

void Application::AcquireSolarMutex( sal_uInt32 nCount )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDefInst->AcquireYieldMutex( nCount );
}

bool Application::IsInMain()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData && pSVData->maAppData.mbInAppMain;
}

bool Application::IsInExecute()
{
    return ImplGetSVData()->maAppData.mbInAppExecute;
}

bool Application::IsQuit()
{
    return ImplGetSVData()->maAppData.mbAppQuit;
}

bool Application::IsInModalMode()
{
    return (ImplGetSVData()->maAppData.mnModalMode != 0);
}

sal_uInt16 Application::GetDispatchLevel()
{
    return ImplGetSVData()->maAppData.mnDispatchLevel;
}

bool Application::AnyInput( VclInputFlags nType )
{
    return ImplGetSVData()->mpDefInst->AnyInput( nType );
}

sal_uInt64 Application::GetLastInputInterval()
{
    return (tools::Time::GetSystemTicks()-ImplGetSVData()->maAppData.mnLastInputTime);
}

bool Application::IsUICaptured()
{
    ImplSVData* pSVData = ImplGetSVData();

    // If mouse was captured, or if in tracking- or in select-mode of a floatingwindow (e.g. menus
    // or pulldown toolboxes) another window should be created
    // D&D active !!!
    return pSVData->mpWinData->mpCaptureWin || pSVData->mpWinData->mpTrackWin
           || pSVData->mpWinData->mpFirstFloat || nImplSysDialog;
}

void Application::OverrideSystemSettings( AllSettings& /*rSettings*/ )
{
}

void Application::MergeSystemSettings( AllSettings& rSettings )
{
    vcl::Window* pWindow = ImplGetSVData()->maFrameData.mpFirstFrame;
    if( ! pWindow )
        pWindow = ImplGetDefaultWindow();
    if( pWindow )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if ( !pSVData->maAppData.mbSettingsInit )
        {
            // side effect: ImplUpdateGlobalSettings does an ImplGetFrame()->UpdateSettings
            pWindow->ImplUpdateGlobalSettings( *pSVData->maAppData.mxSettings );
            pSVData->maAppData.mbSettingsInit = true;
        }
        // side effect: ImplUpdateGlobalSettings does an ImplGetFrame()->UpdateSettings
        pWindow->ImplUpdateGlobalSettings( rSettings, false );
    }
}

void Application::SetSettings( const AllSettings& rSettings )
{
    const SolarMutexGuard aGuard;

    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maAppData.mxSettings )
    {
        InitSettings(pSVData);
        *pSVData->maAppData.mxSettings = rSettings;
    }
    else
    {
        AllSettings aOldSettings = *pSVData->maAppData.mxSettings;
        if (aOldSettings.GetUILanguageTag().getLanguageType() != rSettings.GetUILanguageTag().getLanguageType() &&
                pSVData->mbResLocaleSet)
        {
            pSVData->mbResLocaleSet = false;
        }
        *pSVData->maAppData.mxSettings = rSettings;
        AllSettingsFlags nChangeFlags = aOldSettings.GetChangeFlags( *pSVData->maAppData.mxSettings );
        if ( bool(nChangeFlags) )
        {
            DataChangedEvent aDCEvt( DataChangedEventType::SETTINGS, &aOldSettings, nChangeFlags );

            // notify data change handler
            ImplCallEventListenersApplicationDataChanged( &aDCEvt);

            // Update all windows
            vcl::Window* pFirstFrame = pSVData->maFrameData.mpFirstFrame;
            // Reset data that needs to be re-calculated
            tools::Long nOldDPIX = 0;
            tools::Long nOldDPIY = 0;
            if ( pFirstFrame )
            {
                nOldDPIX = pFirstFrame->GetOutDev()->GetDPIX();
                nOldDPIY = pFirstFrame->GetOutDev()->GetDPIY();
                vcl::Window::ImplInitAppFontData(pFirstFrame);
            }
            vcl::Window* pFrame = pFirstFrame;
            while ( pFrame )
            {
                // call UpdateSettings from ClientWindow in order to prevent updating data twice
                vcl::Window* pClientWin = pFrame;
                while ( pClientWin->ImplGetClientWindow() )
                    pClientWin = pClientWin->ImplGetClientWindow();
                pClientWin->UpdateSettings( rSettings, true );

                vcl::Window* pTempWin = pFrame->mpWindowImpl->mpFrameData->mpFirstOverlap;
                while ( pTempWin )
                {
                    // call UpdateSettings from ClientWindow in order to prevent updating data twice
                    pClientWin = pTempWin;
                    while ( pClientWin->ImplGetClientWindow() )
                        pClientWin = pClientWin->ImplGetClientWindow();
                    pClientWin->UpdateSettings( rSettings, true );
                    pTempWin = pTempWin->mpWindowImpl->mpNextOverlap;
                }

                pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
            }

            // if DPI resolution for screen output was changed set the new resolution for all
            // screen compatible VirDev's
            pFirstFrame = pSVData->maFrameData.mpFirstFrame;
            if ( pFirstFrame )
            {
                if ( (pFirstFrame->GetOutDev()->GetDPIX() != nOldDPIX) ||
                     (pFirstFrame->GetOutDev()->GetDPIY() != nOldDPIY) )
                {
                    VirtualDevice* pVirDev = pSVData->maGDIData.mpFirstVirDev;
                    while ( pVirDev )
                    {
                        if ( pVirDev->mbScreenComp &&
                             (pVirDev->GetDPIX() == nOldDPIX) &&
                             (pVirDev->GetDPIY() == nOldDPIY) )
                        {
                            pVirDev->SetDPIX( pFirstFrame->GetOutDev()->GetDPIX() );
                            pVirDev->SetDPIY( pFirstFrame->GetOutDev()->GetDPIY() );
                            if ( pVirDev->IsMapModeEnabled() )
                            {
                                MapMode aMapMode = pVirDev->GetMapMode();
                                pVirDev->SetMapMode();
                                pVirDev->SetMapMode( aMapMode );
                            }
                        }

                        pVirDev = pVirDev->mpNext;
                    }
                }
            }
        }
    }
}

const AllSettings& Application::GetSettings()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maAppData.mxSettings )
    {
        InitSettings(pSVData);
    }

    return *(pSVData->maAppData.mxSettings);
}

namespace {

void InitSettings(ImplSVData* pSVData)
{
    assert(!pSVData->maAppData.mxSettings && "initialization should not happen twice!");

    pSVData->maAppData.mxSettings.emplace();
    if (!utl::ConfigManager::IsFuzzing())
    {
        pSVData->maAppData.mpCfgListener = new LocaleConfigurationListener;
        pSVData->maAppData.mxSettings->GetSysLocale().GetOptions().AddListener( pSVData->maAppData.mpCfgListener );
    }
}

}

void Application::NotifyAllWindows( DataChangedEvent& rDCEvt )
{
    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window* pFrame = pSVData->maFrameData.mpFirstFrame;
    while ( pFrame )
    {
        pFrame->NotifyAllChildren( rDCEvt );

        vcl::Window* pSysWin = pFrame->mpWindowImpl->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            pSysWin->NotifyAllChildren( rDCEvt );
            pSysWin = pSysWin->mpWindowImpl->mpNextOverlap;
        }

        pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
    }
}

void Application::ImplCallEventListenersApplicationDataChanged( void* pData )
{
    ImplSVData* pSVData = ImplGetSVData();
    VclWindowEvent aEvent( nullptr, VclEventId::ApplicationDataChanged, pData );

    pSVData->maAppData.maEventListeners.Call( aEvent );
}

void Application::ImplCallEventListeners( VclSimpleEvent& rEvent )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.maEventListeners.Call( rEvent );
}

void Application::AddEventListener( const Link<VclSimpleEvent&,void>& rEventListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.maEventListeners.addListener( rEventListener );
}

void Application::RemoveEventListener( const Link<VclSimpleEvent&,void>& rEventListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.maEventListeners.removeListener( rEventListener );
}

void Application::AddKeyListener( const Link<VclWindowEvent&,bool>& rKeyListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.maKeyListeners.push_back( rKeyListener );
}

void Application::RemoveKeyListener( const Link<VclWindowEvent&,bool>& rKeyListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    auto & rVec = pSVData->maAppData.maKeyListeners;
    rVec.erase( std::remove(rVec.begin(), rVec.end(), rKeyListener ), rVec.end() );
}

bool Application::HandleKey( VclEventId nEvent, vcl::Window *pWin, KeyEvent* pKeyEvent )
{
    // let listeners process the key event
    VclWindowEvent aEvent( pWin, nEvent, static_cast<void *>(pKeyEvent) );

    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maAppData.maKeyListeners.empty() )
        return false;

    bool bProcessed = false;
    // Copy the list, because this can be destroyed when calling a Link...
    std::vector<Link<VclWindowEvent&,bool>> aCopy( pSVData->maAppData.maKeyListeners );
    for ( const Link<VclWindowEvent&,bool>& rLink : aCopy )
    {
        if( rLink.Call( aEvent ) )
        {
            bProcessed = true;
            break;
        }
    }
    return bProcessed;
}

ImplSVEvent * Application::PostKeyEvent( VclEventId nEvent, vcl::Window *pWin, KeyEvent const * pKeyEvent )
{
    const SolarMutexGuard aGuard;
    ImplSVEvent * nEventId = nullptr;

    if( pWin && pKeyEvent )
    {
        std::unique_ptr<ImplPostEventData> pPostEventData(new ImplPostEventData( nEvent, pWin, *pKeyEvent ));

        nEventId = PostUserEvent(
                       LINK( nullptr, Application, PostEventHandler ),
                       pPostEventData.get() );

        if( nEventId )
        {
            pPostEventData->mnEventId = nEventId;
            ImplGetSVData()->maAppData.maPostedEventList.emplace_back( pWin, pPostEventData.release() );
        }
    }

    return nEventId;
}

ImplSVEvent* Application::PostGestureEvent(VclEventId nEvent, vcl::Window* pWin,
                                           GestureEventPan const * pGestureEvent)
{
    const SolarMutexGuard aGuard;
    ImplSVEvent * nEventId = nullptr;

    if (pWin && pGestureEvent)
    {
        Point aTransformedPosition(pGestureEvent->mnX, pGestureEvent->mnY);

        aTransformedPosition.AdjustX(pWin->GetOutOffXPixel());
        aTransformedPosition.AdjustY(pWin->GetOutOffYPixel());

        const GestureEventPan aGestureEvent(
            sal_Int32(aTransformedPosition.X()),
            sal_Int32(aTransformedPosition.Y()),
            pGestureEvent->meEventType,
            pGestureEvent->mnOffset,
            pGestureEvent->meOrientation
        );

        std::unique_ptr<ImplPostEventData> pPostEventData(new ImplPostEventData(nEvent, pWin, aGestureEvent));

        nEventId = PostUserEvent(
                       LINK( nullptr, Application, PostEventHandler ),
                       pPostEventData.get());

        if (nEventId)
        {
            pPostEventData->mnEventId = nEventId;
            ImplGetSVData()->maAppData.maPostedEventList.emplace_back(pWin, pPostEventData.release());
        }
    }

    return nEventId;
}

bool Application::LOKHandleMouseEvent(VclEventId nEvent, vcl::Window* pWindow, const MouseEvent* pEvent)
{
    bool bSuccess = false;
    SalMouseEvent aMouseEvent;

    if (!pWindow)
        return false;

    if (!pEvent)
        return false;

    aMouseEvent.mnTime = tools::Time::GetSystemTicks();
    aMouseEvent.mnX = pEvent->GetPosPixel().X();
    aMouseEvent.mnY = pEvent->GetPosPixel().Y();
    aMouseEvent.mnCode = pEvent->GetButtons() | pEvent->GetModifier();

    switch (nEvent)
    {
        case VclEventId::WindowMouseMove:
            aMouseEvent.mnButton = 0;
            bSuccess = ImplLOKHandleMouseEvent(pWindow, NotifyEventType::MOUSEMOVE, false,
                                               aMouseEvent.mnX, aMouseEvent.mnY,
                                               aMouseEvent.mnTime, aMouseEvent.mnCode,
                                               ImplGetMouseMoveMode(&aMouseEvent),
                                               pEvent->GetClicks());
        break;

        case VclEventId::WindowMouseButtonDown:
            aMouseEvent.mnButton = pEvent->GetButtons();
            bSuccess = ImplLOKHandleMouseEvent(pWindow, NotifyEventType::MOUSEBUTTONDOWN, false,
                                               aMouseEvent.mnX, aMouseEvent.mnY,
                                               aMouseEvent.mnTime,
#ifdef MACOSX
                                               aMouseEvent.mnButton |
                                               (aMouseEvent.mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_MOD3)),
#else
                                               aMouseEvent.mnButton |
                                               (aMouseEvent.mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2)),
#endif
                                               ImplGetMouseButtonMode(&aMouseEvent),
                                               pEvent->GetClicks());
            break;

        case VclEventId::WindowMouseButtonUp:
            aMouseEvent.mnButton = pEvent->GetButtons();
            bSuccess = ImplLOKHandleMouseEvent(pWindow, NotifyEventType::MOUSEBUTTONUP, false,
                                               aMouseEvent.mnX, aMouseEvent.mnY,
                                               aMouseEvent.mnTime,
#ifdef MACOSX
                                               aMouseEvent.mnButton |
                                               (aMouseEvent.mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_MOD3)),
#else
                                               aMouseEvent.mnButton |
                                               (aMouseEvent.mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2)),
#endif
                                               ImplGetMouseButtonMode(&aMouseEvent),
                                               pEvent->GetClicks());
            break;

        default:
            SAL_WARN( "vcl.layout", "Application::HandleMouseEvent unknown event (" << static_cast<int>(nEvent) << ")" );
            break;
    }

    return bSuccess;
}


ImplSVEvent* Application::PostMouseEvent( VclEventId nEvent, vcl::Window *pWin, MouseEvent const * pMouseEvent )
{
    const SolarMutexGuard aGuard;
    ImplSVEvent * nEventId = nullptr;

    if( pWin && pMouseEvent )
    {
        Point aTransformedPos( pMouseEvent->GetPosPixel() );

        // LOK uses (0, 0) as the origin of all windows; don't offset.
        if (!comphelper::LibreOfficeKit::isActive())
        {
            aTransformedPos.AdjustX(pWin->GetOutOffXPixel());
            aTransformedPos.AdjustY(pWin->GetOutOffYPixel());
        }

        const MouseEvent aTransformedEvent( aTransformedPos, pMouseEvent->GetClicks(), pMouseEvent->GetMode(),
                                            pMouseEvent->GetButtons(), pMouseEvent->GetModifier() );

        std::unique_ptr<ImplPostEventData> pPostEventData(new ImplPostEventData( nEvent, pWin, aTransformedEvent ));

        nEventId = PostUserEvent(
                       LINK( nullptr, Application, PostEventHandler ),
                       pPostEventData.get() );

        if( nEventId )
        {
            pPostEventData->mnEventId = nEventId;
            ImplGetSVData()->maAppData.maPostedEventList.emplace_back( pWin, pPostEventData.release() );
        }
    }

    return nEventId;
}


IMPL_STATIC_LINK( Application, PostEventHandler, void*, pCallData, void )
{
    const SolarMutexGuard aGuard;
    ImplPostEventData*  pData = static_cast< ImplPostEventData * >( pCallData );
    const void*         pEventData;
    SalEvent            nEvent;
    ImplSVEvent * const nEventId = pData->mnEventId;

    switch( pData->mnEvent )
    {
        case VclEventId::WindowMouseMove:
            nEvent = SalEvent::ExternalMouseMove;
            pEventData = &pData->maMouseEvent;
        break;

        case VclEventId::WindowMouseButtonDown:
            nEvent = SalEvent::ExternalMouseButtonDown;
            pEventData = &pData->maMouseEvent;
        break;

        case VclEventId::WindowMouseButtonUp:
            nEvent = SalEvent::ExternalMouseButtonUp;
            pEventData = &pData->maMouseEvent;
        break;

        case VclEventId::WindowKeyInput:
            nEvent = SalEvent::ExternalKeyInput;
            pEventData = &pData->maKeyEvent;
        break;

        case VclEventId::WindowKeyUp:
            nEvent = SalEvent::ExternalKeyUp;
            pEventData = &pData->maKeyEvent;
        break;

        case VclEventId::WindowGestureEvent:
            nEvent = SalEvent::ExternalGesture;
            pEventData = &pData->maGestureEvent;
        break;

        default:
            nEvent = SalEvent::NONE;
            pEventData = nullptr;
        break;
    }

    if( pData->mpWin && pData->mpWin->mpWindowImpl->mpFrameWindow && pEventData )
        ImplWindowFrameProc( pData->mpWin->mpWindowImpl->mpFrameWindow.get(), nEvent, pEventData );

    // remove this event from list of posted events, watch for destruction of internal data
    auto svdata = ImplGetSVData();
    ::std::vector< ImplPostEventPair >::iterator aIter( svdata->maAppData.maPostedEventList.begin() );

    while( aIter != svdata->maAppData.maPostedEventList.end() )
    {
        if( nEventId == (*aIter).second->mnEventId )
        {
            delete (*aIter).second;
            aIter = svdata->maAppData.maPostedEventList.erase( aIter );
        }
        else
            ++aIter;
    }
}

void Application::RemoveMouseAndKeyEvents( vcl::Window* pWin )
{
    const SolarMutexGuard aGuard;

    // remove all events for specific window, watch for destruction of internal data
    auto svdata = ImplGetSVData();
    ::std::vector< ImplPostEventPair >::iterator aIter( svdata->maAppData.maPostedEventList.begin() );

    while( aIter != svdata->maAppData.maPostedEventList.end() )
    {
        if( pWin == (*aIter).first )
        {
            if( (*aIter).second->mnEventId )
                RemoveUserEvent( (*aIter).second->mnEventId );

            delete (*aIter).second;
            aIter = svdata->maAppData.maPostedEventList.erase( aIter );
        }
        else
            ++aIter;
    }
}

ImplSVEvent * Application::PostUserEvent( const Link<void*,void>& rLink, void* pCaller,
                                          bool bReferenceLink )
{
    vcl::Window* pDefWindow = ImplGetDefaultWindow();
    if ( pDefWindow == nullptr )
        return nullptr;

    std::unique_ptr<ImplSVEvent> pSVEvent(new ImplSVEvent);
    pSVEvent->mpData    = pCaller;
    pSVEvent->maLink    = rLink;
    pSVEvent->mpWindow  = nullptr;
    pSVEvent->mbCall    = true;
    if (bReferenceLink)
    {
        SolarMutexGuard aGuard;
        pSVEvent->mpInstanceRef = static_cast<vcl::Window *>(rLink.GetInstance());
    }

    auto pTmpEvent = pSVEvent.get();
    if (!pDefWindow->ImplGetFrame()->PostEvent( std::move(pSVEvent) ))
        return nullptr;
    return pTmpEvent;
}

void Application::RemoveUserEvent( ImplSVEvent * nUserEvent )
{
    if(nUserEvent)
    {
        SAL_WARN_IF( nUserEvent->mpWindow, "vcl",
                    "Application::RemoveUserEvent(): Event is send to a window" );
        SAL_WARN_IF( !nUserEvent->mbCall, "vcl",
                    "Application::RemoveUserEvent(): Event is already removed" );

        nUserEvent->mpWindow.clear();
        nUserEvent->mpInstanceRef.clear();
        nUserEvent->mbCall = false;
    }
}

vcl::Window* Application::GetFocusWindow()
{
    return ImplGetSVData()->mpWinData->mpFocusWin;
}

OutputDevice* Application::GetDefaultDevice()
{
    vcl::Window* pWindow = ImplGetDefaultWindow();
    if (pWindow != nullptr)
    {
        return pWindow->GetOutDev();
    }
    else
    {
        return nullptr;
    }
}

basegfx::SystemDependentDataManager& Application::GetSystemDependentDataManager()
{
    return ImplGetSystemDependentDataManager();
}

vcl::Window* Application::GetFirstTopLevelWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->maFrameData.mpFirstFrame;
}

vcl::Window* Application::GetNextTopLevelWindow( vcl::Window const * pWindow )
{
    return pWindow->mpWindowImpl->mpFrameData->mpNextFrame;
}

tools::Long    Application::GetTopWindowCount()
{
    tools::Long nRet = 0;
    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window *pWin = pSVData ? pSVData->maFrameData.mpFirstFrame.get() : nullptr;
    while( pWin )
    {
        if( pWin->ImplGetWindow()->IsTopWindow() )
            nRet++;
        pWin = pWin->mpWindowImpl->mpFrameData->mpNextFrame;
    }
    return nRet;
}

vcl::Window* Application::GetTopWindow( tools::Long nIndex )
{
    tools::Long nIdx = 0;
    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window *pWin = pSVData ? pSVData->maFrameData.mpFirstFrame.get() : nullptr;
    while( pWin )
    {
        if( pWin->ImplGetWindow()->IsTopWindow() )
        {
            if( nIdx == nIndex )
                return pWin->ImplGetWindow();
            else
                nIdx++;
        }
        pWin = pWin->mpWindowImpl->mpFrameData->mpNextFrame;
    }
    return nullptr;
}

vcl::Window* Application::GetActiveTopWindow()
{
    vcl::Window *pWin = ImplGetSVData()->mpWinData->mpFocusWin;
    while( pWin )
    {
        if( pWin->IsTopWindow() )
            return pWin;
        pWin = pWin->mpWindowImpl->mpParent;
    }
    return nullptr;
}

void Application::SetAppName( const OUString& rUniqueName )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mxAppName = rUniqueName;
}

OUString Application::GetAppName()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maAppData.mxAppName )
        return *(pSVData->maAppData.mxAppName);
    else
        return OUString();
}

enum {hwAll=0, hwEnv=1, hwUI=2};

static OUString Localize(TranslateId aId, const bool bLocalize)
{
    if (bLocalize)
        return VclResId(aId);
    else
        return Translate::get(aId, Translate::Create("vcl", LanguageTag("en-US")));
}

OUString Application::GetOSVersion()
{
    ImplSVData* pSVData = ImplGetSVData();
    OUString aVersion;
    if (pSVData && pSVData->mpDefInst)
        aVersion = pSVData->mpDefInst->getOSVersion();
    else
        aVersion = "-";
    return aVersion;
}

OUString Application::GetHWOSConfInfo(const int bSelection, const bool bLocalize)
{
    OUStringBuffer aDetails;

    const auto appendDetails = [&aDetails](std::u16string_view sep, auto&& val) {
        if (!aDetails.isEmpty() && !sep.empty())
            aDetails.append(sep);
        aDetails.append(std::move(val));
    };

    if (bSelection != hwUI) {
        appendDetails(u"; ", Localize(SV_APP_CPUTHREADS, bLocalize)
                                + OUString::number(std::thread::hardware_concurrency()));

        OUString aVersion = GetOSVersion();

        appendDetails(u"; ", Localize(SV_APP_OSVERSION, bLocalize) + aVersion);
    }

    if (bSelection != hwEnv) {
        appendDetails(u"; ", Localize(SV_APP_UIRENDER, bLocalize));
#if HAVE_FEATURE_SKIA
        if ( SkiaHelper::isVCLSkiaEnabled() )
        {
            switch(SkiaHelper::renderMethodToUse())
            {
                case SkiaHelper::RenderVulkan:
                    appendDetails(u"", Localize(SV_APP_SKIA_VULKAN, bLocalize));
                    break;
                case SkiaHelper::RenderMetal:
                    appendDetails(u"", Localize(SV_APP_SKIA_METAL, bLocalize));
                    break;
                case SkiaHelper::RenderRaster:
                    appendDetails(u"", Localize(SV_APP_SKIA_RASTER, bLocalize));
                    break;
            }
        }
        else
#endif
            appendDetails(u"", Localize(SV_APP_DEFAULT, bLocalize));

#if (defined LINUX || defined _WIN32 || defined MACOSX || defined __FreeBSD__ || defined EMSCRIPTEN)
        appendDetails(u"; ", SV_APP_VCLBACKEND + GetToolkitName());
#endif
    }

    return aDetails.makeStringAndClear();
}

void Application::SetDisplayName( const OUString& rName )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mxDisplayName = rName;
}

OUString Application::GetDisplayName()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maAppData.mxDisplayName )
        return *(pSVData->maAppData.mxDisplayName);
    else if (pSVData->maFrameData.mpAppWin)
        return pSVData->maFrameData.mpAppWin->GetText();
    else
        return OUString();
}

unsigned int Application::GetScreenCount()
{
    SalSystem* pSys = ImplGetSalSystem();
    return pSys ? pSys->GetDisplayScreenCount() : 0;
}

bool Application::IsUnifiedDisplay()
{
    SalSystem* pSys = ImplGetSalSystem();
    return pSys == nullptr || pSys->IsUnifiedDisplay();
}

unsigned int Application::GetDisplayBuiltInScreen()
{
    SalSystem* pSys = ImplGetSalSystem();
    return pSys ? pSys->GetDisplayBuiltInScreen() : 0;
}

unsigned int Application::GetDisplayExternalScreen()
{
    // This is really unpleasant, in theory we could have multiple
    // external displays etc.
    int nExternal(0);
    switch (GetDisplayBuiltInScreen())
    {
    case 0:
        nExternal = 1;
        break;
    case 1:
        nExternal = 0;
        break;
    default:
        // When the built-in display is neither 0 nor 1
        // then place the full-screen presentation on the
        // first available screen.
        nExternal = 0;
        break;
    }
    return nExternal;
}

tools::Rectangle Application::GetScreenPosSizePixel( unsigned int nScreen )
{
    SalSystem* pSys = ImplGetSalSystem();
    if (!pSys)
    {
        SAL_WARN("vcl", "Requesting screen size/pos for screen #" << nScreen << " failed");
        assert(false);
        return tools::Rectangle();
    }
    tools::Rectangle aRect = pSys->GetDisplayScreenPosSizePixel(nScreen);
    if (aRect.GetHeight() == 0)
        SAL_WARN("vcl", "Requesting screen size/pos for screen #" << nScreen << " returned 0 height.");
    return aRect;
}

namespace {
tools::Long calcDistSquare( const Point& i_rPoint, const tools::Rectangle& i_rRect )
{
    const Point aRectCenter( (i_rRect.Left() + i_rRect.Right())/2,
                       (i_rRect.Top() + i_rRect.Bottom())/ 2 );
    const tools::Long nDX = aRectCenter.X() - i_rPoint.X();
    const tools::Long nDY = aRectCenter.Y() - i_rPoint.Y();
    return nDX*nDX + nDY*nDY;
}
}

unsigned int Application::GetBestScreen( const tools::Rectangle& i_rRect )
{
    if( !IsUnifiedDisplay() )
        return GetDisplayBuiltInScreen();

    const unsigned int nScreens = GetScreenCount();
    unsigned int nBestMatchScreen = 0;
    unsigned long nOverlap = 0;
    for( unsigned int i = 0; i < nScreens; i++ )
    {
        const tools::Rectangle aCurScreenRect( GetScreenPosSizePixel( i ) );
        // if a screen contains the rectangle completely it is obviously the best screen
        if( aCurScreenRect.Contains( i_rRect ) )
            return i;
        // next the screen which contains most of the area of the rect is the best
        tools::Rectangle aIntersection( aCurScreenRect.GetIntersection( i_rRect ) );
        if( ! aIntersection.IsEmpty() )
        {
            const unsigned long nCurOverlap( aIntersection.GetWidth() * aIntersection.GetHeight() );
            if( nCurOverlap > nOverlap )
            {
                nOverlap = nCurOverlap;
                nBestMatchScreen = i;
            }
        }
    }
    if( nOverlap > 0 )
        return nBestMatchScreen;

    // finally the screen which center is nearest to the rect is the best
    const Point aCenter( (i_rRect.Left() + i_rRect.Right())/2,
                         (i_rRect.Top() + i_rRect.Bottom())/2 );
    tools::Long nDist = std::numeric_limits<tools::Long>::max();
    for( unsigned int i = 0; i < nScreens; i++ )
    {
        const tools::Rectangle aCurScreenRect( GetScreenPosSizePixel( i ) );
        const tools::Long nCurDist( calcDistSquare( aCenter, aCurScreenRect ) );
        if( nCurDist < nDist )
        {
            nBestMatchScreen = i;
            nDist = nCurDist;
        }
    }
    return nBestMatchScreen;
}

bool Application::InsertAccel( Accelerator* pAccel )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( !pSVData->maAppData.mpAccelMgr )
        pSVData->maAppData.mpAccelMgr = new ImplAccelManager();
    return pSVData->maAppData.mpAccelMgr->InsertAccel( pAccel );
}

void Application::RemoveAccel( Accelerator const * pAccel )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maAppData.mpAccelMgr )
        pSVData->maAppData.mpAccelMgr->RemoveAccel( pAccel );
}

void Application::SetHelp( Help* pHelp )
{
    ImplGetSVData()->maAppData.mpHelp = pHelp;
}

void Application::UpdateMainThread()
{
    ImplSVData* pSVData = ImplGetSVData();
    if (pSVData && pSVData->mpDefInst)
        pSVData->mpDefInst->updateMainThread();
}

Help* Application::GetHelp()
{
    return ImplGetSVData()->maAppData.mpHelp;
}

OUString Application::GetToolkitName()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maAppData.mxToolkitName )
        return *(pSVData->maAppData.mxToolkitName);
    else
        return OUString();
}

vcl::Window* Dialog::GetDefDialogParent()
{
    ImplSVData* pSVData = ImplGetSVData();
    // find some useful dialog parent

    // always use the topmost parent of the candidate
    // window to avoid using dialogs or floaters
    // as DefDialogParent

    // current focus frame
    vcl::Window *pWin = pSVData->mpWinData->mpFocusWin;
    if (pWin && !pWin->IsMenuFloatingWindow())
    {
        while (pWin->mpWindowImpl && pWin->mpWindowImpl->mpParent)
            pWin = pWin->mpWindowImpl->mpParent;

        // check for corrupted window hierarchy, #122232#, may be we now crash somewhere else
        if (!pWin->mpWindowImpl)
        {
            OSL_FAIL( "Window hierarchy corrupted!" );
            pSVData->mpWinData->mpFocusWin = nullptr;   // avoid further access
            return nullptr;
        }

        if ((pWin->mpWindowImpl->mnStyle & WB_INTROWIN) == 0)
        {
            return pWin->mpWindowImpl->mpFrameWindow->ImplGetWindow();
        }
    }

    // last active application frame
    pWin = pSVData->maFrameData.mpActiveApplicationFrame;
    if (pWin)
    {
        return pWin->mpWindowImpl->mpFrameWindow->ImplGetWindow();
    }

    // first visible top window (may be totally wrong...)
    pWin = pSVData->maFrameData.mpFirstFrame;
    while (pWin)
    {
        if( pWin->ImplGetWindow()->IsTopWindow() &&
            pWin->mpWindowImpl->mbReallyVisible &&
            (pWin->mpWindowImpl->mnStyle & WB_INTROWIN) == 0
        )
        {
            while( pWin->mpWindowImpl->mpParent )
                pWin = pWin->mpWindowImpl->mpParent;
            return pWin->mpWindowImpl->mpFrameWindow->ImplGetWindow();
        }
        pWin = pWin->mpWindowImpl->mpFrameData->mpNextFrame;
    }

    // use the desktop
    return nullptr;
}

weld::Window* Application::GetDefDialogParent()
{
    vcl::Window* pWindow = Dialog::GetDefDialogParent();
    return pWindow ? pWindow->GetFrameWeld() : nullptr;
}

DialogCancelMode Application::GetDialogCancelMode()
{
    return ImplGetSVData()->maAppData.meDialogCancel;
}

void Application::SetDialogCancelMode( DialogCancelMode mode )
{
    ImplGetSVData()->maAppData.meDialogCancel = mode;
}

bool Application::IsDialogCancelEnabled()
{
    return ImplGetSVData()->maAppData.meDialogCancel != DialogCancelMode::Off;
}

void Application::SetSystemWindowMode( SystemWindowFlags nMode )
{
    ImplGetSVData()->maAppData.mnSysWinMode = nMode;
}

SystemWindowFlags Application::GetSystemWindowMode()
{
    return ImplGetSVData()->maAppData.mnSysWinMode;
}

css::uno::Reference< css::awt::XToolkit > Application::GetVCLToolkit()
{
    css::uno::Reference< css::awt::XToolkit > xT;
    UnoWrapperBase* pWrapper = UnoWrapperBase::GetUnoWrapper();
    if ( pWrapper )
        xT = pWrapper->GetVCLToolkit();
    return xT;
}

#ifdef DISABLE_DYNLOADING

extern "C" { UnoWrapperBase* CreateUnoWrapper(); }

#else

extern "C" { static void thisModule() {} }

#endif

UnoWrapperBase* UnoWrapperBase::GetUnoWrapper( bool bCreateIfNotExist )
{
    ImplSVData* pSVData = ImplGetSVData();
    static bool bAlreadyTriedToCreate = false;
    if ( !pSVData->mpUnoWrapper && bCreateIfNotExist && !bAlreadyTriedToCreate )
    {
#ifndef DISABLE_DYNLOADING
        osl::Module aTkLib;
        aTkLib.loadRelative(&thisModule, TK_DLL_NAME);
        if (aTkLib.is())
        {
            FN_TkCreateUnoWrapper fnCreateWrapper = reinterpret_cast<FN_TkCreateUnoWrapper>(aTkLib.getFunctionSymbol("CreateUnoWrapper"));
            if ( fnCreateWrapper )
            {
                pSVData->mpUnoWrapper = fnCreateWrapper();
            }
            aTkLib.release();
        }
        SAL_WARN_IF( !pSVData->mpUnoWrapper, "vcl", "UnoWrapper could not be created!" );
#else
        pSVData->mpUnoWrapper = CreateUnoWrapper();
#endif
        bAlreadyTriedToCreate = true;
    }
    return pSVData->mpUnoWrapper;
}

void UnoWrapperBase::SetUnoWrapper( UnoWrapperBase* pWrapper )
{
    ImplSVData* pSVData = ImplGetSVData();
    SAL_WARN_IF( pSVData->mpUnoWrapper, "vcl", "SetUnoWrapper: Wrapper already exists" );
    pSVData->mpUnoWrapper = pWrapper;
}

css::uno::Reference< css::awt::XDisplayConnection > Application::GetDisplayConnection()
{
    ImplSVData* pSVData = ImplGetSVData();

    if( !pSVData->mxDisplayConnection.is() )
    {
        pSVData->mxDisplayConnection.set( new vcl::DisplayConnectionDispatch );
        pSVData->mxDisplayConnection->start();
    }

    return pSVData->mxDisplayConnection;
}

void Application::SetFilterHdl( const Link<ConvertData&,bool>& rLink )
{
    ImplGetSVData()->maGDIData.mxGrfConverter->SetFilterHdl( rLink );
}

const LocaleDataWrapper& Application::GetAppLocaleDataWrapper()
{
    return GetSettings().GetLocaleDataWrapper();
}

void Application::EnableHeadlessMode( bool dialogsAreFatal )
{
    DialogCancelMode eNewMode = dialogsAreFatal ? DialogCancelMode::Fatal : DialogCancelMode::Silent;
    DialogCancelMode eOldMode = GetDialogCancelMode();
    assert(eOldMode == DialogCancelMode::Off || GetDialogCancelMode() == eNewMode);
    if (eOldMode != eNewMode)
        SetDialogCancelMode( eNewMode );
}

bool Application::IsHeadlessModeEnabled()
{
    return IsDialogCancelEnabled() || comphelper::LibreOfficeKit::isActive();
}

void Application::EnableBitmapRendering()
{
    ImplGetSVData()->maAppData.mbRenderToBitmaps = true;
}

bool Application::IsBitmapRendering()
{
    return ImplGetSVData()->maAppData.mbRenderToBitmaps;
}

void Application::EnableConsoleOnly()
{
    EnableHeadlessMode(true);
    EnableBitmapRendering();
}

static bool bEventTestingMode = false;

bool Application::IsEventTestingModeEnabled()
{
    return bEventTestingMode;
}

void Application::EnableEventTestingMode()
{
    bEventTestingMode = true;
}

static bool bSafeMode = false;

bool Application::IsSafeModeEnabled()
{
    return bSafeMode;
}

void Application::EnableSafeMode()
{
    bSafeMode = true;
}

void Application::ShowNativeErrorBox(const OUString& sTitle  ,
                                     const OUString& sMessage)
{
    int btn = ImplGetSalSystem()->ShowNativeMessageBox(
            sTitle,
            sMessage);
    if (btn != SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK) {
        SAL_WARN( "vcl", "ShowNativeMessageBox returned " << btn);
    }
}

const OUString& Application::GetDesktopEnvironment()
{
    if (IsHeadlessModeEnabled())
    {
        static const OUString aNone("none");
        return aNone;
    }
    else
        return SalGetDesktopEnvironment();
}

void Application::AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService)
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDefInst->AddToRecentDocumentList(rFileUrl, rMimeType, rDocumentService);
}

bool InitAccessBridge()
{
// Disable MSAA bridge on UNIX
#if defined UNX
    return true;
#else
    bool bRet = ImplInitAccessBridge();

    if( !bRet )
    {
        // disable accessibility if the user chooses to continue
        AllSettings aSettings = Application::GetSettings();
        MiscSettings aMisc = aSettings.GetMiscSettings();
        aMisc.SetEnableATToolSupport( false );
        aSettings.SetMiscSettings( aMisc );
        Application::SetSettings( aSettings );
    }
    return bRet;
#endif // !UNX
}

// MT: AppEvent was in oldsv.cxx, but is still needed...
void Application::AppEvent( const ApplicationEvent& /*rAppEvent*/ )
{
}

bool Application::hasNativeFileSelection()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->mpDefInst->hasNativeFileSelection();
}

Reference< ui::dialogs::XFilePicker2 >
Application::createFilePicker( const Reference< uno::XComponentContext >& xSM )
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->mpDefInst->createFilePicker( xSM );
}

Reference< ui::dialogs::XFolderPicker2 >
Application::createFolderPicker( const Reference< uno::XComponentContext >& xSM )
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->mpDefInst->createFolderPicker( xSM );
}

void Application::setDeInitHook(Link<LinkParamNone*,void> const & hook) {
    ImplSVData * pSVData = ImplGetSVData();
    assert(!pSVData->maDeInitHook.IsSet());
    pSVData->maDeInitHook = hook;
    // Fake this for VCLXToolkit ctor instantiated from
    // postprocess/CppunitTest_services.mk:
    pSVData->maAppData.mbInAppMain = true;
}

namespace vcl::lok {

void registerPollCallbacks(
    LibreOfficeKitPollCallback pPollCallback,
    LibreOfficeKitWakeCallback pWakeCallback,
    void *pData) {

    ImplSVData * pSVData = ImplGetSVData();
    if (pSVData)
    {
        pSVData->mpPollCallback = pPollCallback;
        pSVData->mpWakeCallback = pWakeCallback;
        pSVData->mpPollClosure = pData;
    }
}

void unregisterPollCallbacks()
{
    ImplSVData * pSVData = ImplGetSVData();
    if (!pSVData)
        return;

    // Not hyper-elegant - but in the case of Android & unipoll we need to detach
    // this thread from the JVM's clutches to avoid a crash closing document
    if (pSVData->mpPollClosure && pSVData->mpDefInst)
        pSVData->mpDefInst->releaseMainThread();

    // Just set mpPollClosure to null as that is what calling this means, that the callback data
    // points to an object that no longer exists. In particular, don't set
    // pSVData->mpPollCallback to nullptr as that is used to detect whether Unipoll is in use in
    // isUnipoll().
    pSVData->mpPollClosure = nullptr;
}

bool isUnipoll()
{
    ImplSVData * pSVData = ImplGetSVData();
    return pSVData && pSVData->mpPollCallback != nullptr;
}

void numberOfViewsChanged(int count)
{
    if (count == 0)
        return;
    ImplSVData * pSVData = ImplGetSVData();
    auto& rCache = pSVData->maGDIData.maScaleCache;
    // Normally the cache size is set to 10, scale according to the number of users.
    rCache.setMaxSize(count * 10);
}

void dumpState(rtl::OStringBuffer &rState)
{
    ImplSVData* pSVData = ImplGetSVData();
    if (!pSVData)
        return;

    rState.append("\nWindows:\t");
    rState.append(static_cast<sal_Int32>(Application::GetTopWindowCount()));

    vcl::Window *pWin = Application::GetFirstTopLevelWindow();
    while (pWin)
    {
        tools::JsonWriter props;
        pWin->DumpAsPropertyTree(props);

        rState.append("\n\tWindow: ");
        rState.append(props.extractAsOString());

        pWin = Application::GetNextTopLevelWindow( pWin );
    }
}

} // namespace lok, namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
