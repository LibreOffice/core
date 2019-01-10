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

#include <osl/module.h>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <osl/module.hxx>

#include <rtl/tencinfo.h>
#include <rtl/instance.hxx>
#include <sal/log.hxx>

#include <tools/debug.hxx>
#include <tools/time.hxx>
#include <tools/stream.hxx>

#include <i18nlangtag/mslangid.hxx>

#include <unotools/configmgr.hxx>
#include <unotools/syslocaleoptions.hxx>

#include <vcl/dialog.hxx>
#include <vcl/floatwin.hxx>
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
#include <vcl/unohelp.hxx>
#include <vcl/lazydelete.hxx>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLWrapper.hxx>
#endif
#include <saltimer.hxx>

#include <salinst.hxx>
#include <salframe.hxx>
#include <salsys.hxx>
#include <svdata.hxx>
#include <salimestatus.hxx>
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
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/solarmutex.hxx>
#include <osl/process.h>

#include <cassert>
#include <utility>
#include <thread>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {
void InitSettings(ImplSVData* pSVData);
}

// keycodes handled internally by VCL
static vcl::KeyCode const ReservedKeys[]
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
    VclEventId const    mnEvent;
    VclPtr<vcl::Window> mpWin;
    ImplSVEvent *   mnEventId;
    KeyEvent        maKeyEvent;
    MouseEvent      maMouseEvent;

    ImplPostEventData( VclEventId nEvent, vcl::Window* pWin, const KeyEvent& rKeyEvent ) :
        mnEvent( nEvent ), mpWin( pWin ), mnEventId( nullptr ), maKeyEvent( rKeyEvent ) {}
    ImplPostEventData( VclEventId nEvent, vcl::Window* pWin, const MouseEvent& rMouseEvent ) :
        mnEvent( nEvent ), mpWin( pWin ), mnEventId( nullptr ), maMouseEvent( rMouseEvent ) {}
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
    WorkWindow* pAppWin = ImplGetSVData()->maWinData.mpAppWin;

    // call the close handler of the application window
    if ( pAppWin )
        return pAppWin->Close();
    else
        return true;
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

#ifdef DBG_UTIL
        case ExceptionCategory::ResourceNotLoaded:
            Abort("Resource not loaded");
            break;
        default:
            Abort("Unknown Error");
            break;
#else
        default:
            Abort(OUString());
            break;
#endif
    }
}

void Application::Abort( const OUString& rErrorText )
{
    //HACK: Dump core iff --norestore command line argument is given (assuming
    // this process is run by developers who are interested in cores, vs. end
    // users who are not):
    bool dumpCore = false;
    sal_uInt16 n = GetCommandLineParamCount();
    for (sal_uInt16 i = 0; i != n; ++i) {
        if (GetCommandLineParam(i) == "--norestore") {
            dumpCore = true;
            break;
        }
    }
#if OSL_DEBUG_LEVEL > 0
    dumpCore = true;
#endif

    SalAbort( rErrorText, dumpCore );
}

sal_uLong Application::GetReservedKeyCodeCount()
{
    return SAL_N_ELEMENTS(ReservedKeys);
}

const vcl::KeyCode* Application::GetReservedKeyCode( sal_uLong i )
{
    if( i >= GetReservedKeyCodeCount() )
        return nullptr;
    else
        return &ReservedKeys[i];
}

IMPL_STATIC_LINK_NOARG( ImplSVAppData, ImplEndAllPopupsMsg, void*, void )
{
    ImplSVData* pSVData = ImplGetSVData();
    while (pSVData->maWinData.mpFirstFloat)
        pSVData->maWinData.mpFirstFloat->EndPopupMode(FloatWinPopupEndFlags::Cancel);
}

IMPL_STATIC_LINK_NOARG( ImplSVAppData, ImplEndAllDialogsMsg, void*, void )
{
    vcl::Window* pAppWindow = Application::GetFirstTopLevelWindow();
    while (pAppWindow)
    {
        Dialog::EndAllDialogs(pAppWindow);
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

    while ( !pSVData->maAppData.mbAppQuit )
        Application::Yield();

    pSVData->maAppData.mbInAppExecute = false;
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
    return ImplYield(false, i_bAllEvents);
}

void Scheduler::ProcessEventsToIdle()
{
    int nSanity = 1;
    while( Application::Reschedule( true ) )
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
    if (!pSVData->mpDefInst->IsMainThread())
        return;
    for (int nTaskPriority = 0; nTaskPriority < PRIO_COUNT; ++nTaskPriority)
    {
        const ImplSchedulerData* pSchedulerData = pSVData->maSchedCtx.mpFirstSchedulerData[nTaskPriority];
        while (pSchedulerData)
        {
            if (pSchedulerData->mpTask && !pSchedulerData->mbInScheduler)
            {
                Idle *pIdle = dynamic_cast<Idle*>(pSchedulerData->mpTask);
                if (pIdle && pIdle->IsActive())
                {
                    SAL_WARN("vcl.schedule", "Unprocessed Idle: "
                             << pIdle << " " << pIdle->GetDebugName());
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
    ImplYield(true, false);
}

IMPL_STATIC_LINK_NOARG( ImplSVAppData, ImplQuitMsg, void*, void )
{
    ImplGetSVData()->maAppData.mbAppQuit = true;
}

void Application::Quit()
{
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
    return pSVData->maWinData.mpCaptureWin || pSVData->maWinData.mpTrackWin ||
         pSVData->maWinData.mpFirstFloat || nImplSysDialog;
}

void Application::OverrideSystemSettings( AllSettings& /*rSettings*/ )
{
}

void Application::MergeSystemSettings( AllSettings& rSettings )
{
    vcl::Window* pWindow = ImplGetSVData()->maWinData.mpFirstFrame;
    if( ! pWindow )
        pWindow = ImplGetDefaultWindow();
    if( pWindow )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if ( !pSVData->maAppData.mbSettingsInit )
        {
            // side effect: ImplUpdateGlobalSettings does an ImplGetFrame()->UpdateSettings
            pWindow->ImplUpdateGlobalSettings( *pSVData->maAppData.mpSettings );
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
    if ( !pSVData->maAppData.mpSettings )
    {
        InitSettings(pSVData);
        *pSVData->maAppData.mpSettings = rSettings;
    }
    else
    {
        AllSettings aOldSettings = *pSVData->maAppData.mpSettings;
        if (aOldSettings.GetUILanguageTag().getLanguageType() != rSettings.GetUILanguageTag().getLanguageType() &&
                pSVData->mbResLocaleSet)
        {
            pSVData->mbResLocaleSet = false;
        }
        *pSVData->maAppData.mpSettings = rSettings;
        AllSettingsFlags nChangeFlags = aOldSettings.GetChangeFlags( *pSVData->maAppData.mpSettings );
        if ( bool(nChangeFlags) )
        {
            DataChangedEvent aDCEvt( DataChangedEventType::SETTINGS, &aOldSettings, nChangeFlags );

            // notify data change handler
            ImplCallEventListenersApplicationDataChanged( &aDCEvt);

            // Update all windows
            vcl::Window* pFirstFrame = pSVData->maWinData.mpFirstFrame;
            // Reset data that needs to be re-calculated
            long nOldDPIX = 0;
            long nOldDPIY = 0;
            if ( pFirstFrame )
            {
                nOldDPIX = pFirstFrame->GetDPIX();
                nOldDPIY = pFirstFrame->GetDPIY();
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
            pFirstFrame = pSVData->maWinData.mpFirstFrame;
            if ( pFirstFrame )
            {
                if ( (pFirstFrame->GetDPIX() != nOldDPIX) ||
                     (pFirstFrame->GetDPIY() != nOldDPIY) )
                {
                    VirtualDevice* pVirDev = pSVData->maGDIData.mpFirstVirDev;
                    while ( pVirDev )
                    {
                        if ( pVirDev->mbScreenComp &&
                             (pVirDev->GetDPIX() == nOldDPIX) &&
                             (pVirDev->GetDPIY() == nOldDPIY) )
                        {
                            pVirDev->SetDPIX( pFirstFrame->GetDPIX() );
                            pVirDev->SetDPIY( pFirstFrame->GetDPIY() );
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
    if ( !pSVData->maAppData.mpSettings )
    {
        InitSettings(pSVData);
    }

    return *(pSVData->maAppData.mpSettings);
}

namespace {

void InitSettings(ImplSVData* pSVData)
{
    assert(!pSVData->maAppData.mpSettings && "initialization should not happen twice!");

    pSVData->maAppData.mpSettings.reset(new AllSettings());
    if (!utl::ConfigManager::IsFuzzing())
    {
        pSVData->maAppData.mpCfgListener = new LocaleConfigurationListener;
        pSVData->maAppData.mpSettings->GetSysLocale().GetOptions().AddListener( pSVData->maAppData.mpCfgListener );
    }
}

}

void Application::NotifyAllWindows( DataChangedEvent& rDCEvt )
{
    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window*     pFrame = pSVData->maWinData.mpFirstFrame;
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
    for ( Link<VclWindowEvent&,bool>& rLink : aCopy )
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

ImplSVEvent * Application::PostMouseEvent( VclEventId nEvent, vcl::Window *pWin, MouseEvent const * pMouseEvent )
{
    const SolarMutexGuard aGuard;
    ImplSVEvent * nEventId = nullptr;

    if( pWin && pMouseEvent )
    {
        Point aTransformedPos( pMouseEvent->GetPosPixel() );

        aTransformedPos.AdjustX(pWin->GetOutOffXPixel() );
        aTransformedPos.AdjustY(pWin->GetOutOffYPixel() );

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

        default:
            nEvent = SalEvent::NONE;
            pEventData = nullptr;
        break;
    };

    if( pData->mpWin && pData->mpWin.get()->mpWindowImpl->mpFrameWindow.get() && pEventData )
        ImplWindowFrameProc( pData->mpWin.get()->mpWindowImpl->mpFrameWindow.get(), nEvent, pEventData );

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
        // Double check that this is indeed a vcl::Window instance.
        assert(dynamic_cast<vcl::Window *>(
                        static_cast<OutputDevice *>(rLink.GetInstance())) ==
               static_cast<vcl::Window *>(rLink.GetInstance()));
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

void Application::LockFontUpdates(bool bLock)
{
    OutputDevice::LockFontUpdates(bLock);
}

WorkWindow* Application::GetAppWindow()
{
    return ImplGetSVData()->maWinData.mpAppWin;
}

vcl::Window* Application::GetFocusWindow()
{
    return ImplGetSVData()->maWinData.mpFocusWin;
}

OutputDevice* Application::GetDefaultDevice()
{
    return ImplGetDefaultWindow();
}

vcl::Window* Application::GetFirstTopLevelWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->maWinData.mpFirstFrame;
}

vcl::Window* Application::GetNextTopLevelWindow( vcl::Window const * pWindow )
{
    return pWindow->mpWindowImpl->mpFrameData->mpNextFrame;
}

long    Application::GetTopWindowCount()
{
    long nRet = 0;
    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window *pWin = pSVData ? pSVData->maWinData.mpFirstFrame.get() : nullptr;
    while( pWin )
    {
        if( pWin->ImplGetWindow()->IsTopWindow() )
            nRet++;
        pWin = pWin->mpWindowImpl->mpFrameData->mpNextFrame;
    }
    return nRet;
}

vcl::Window* Application::GetTopWindow( long nIndex )
{
    long nIdx = 0;
    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window *pWin = pSVData ? pSVData->maWinData.mpFirstFrame.get() : nullptr;
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
    vcl::Window *pWin = ImplGetSVData()->maWinData.mpFocusWin;
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

OUString Application::GetHWOSConfInfo()
{
    ImplSVData* pSVData = ImplGetSVData();
    OUStringBuffer aDetails;

    aDetails.append( VclResId(SV_APP_CPUTHREADS) );
    aDetails.append( static_cast<sal_Int32>(std::thread::hardware_concurrency()) );
    aDetails.append( "; " );

    OUString aVersion;
    if ( pSVData && pSVData->mpDefInst )
        aVersion = pSVData->mpDefInst->getOSVersion();
    else
        aVersion = "-";

    aDetails.append( VclResId(SV_APP_OSVERSION) );
    aDetails.append( aVersion );
    aDetails.append( "; " );

    aDetails.append( VclResId(SV_APP_UIRENDER) );
#if HAVE_FEATURE_OPENGL
    if ( OpenGLWrapper::isVCLOpenGLEnabled() )
        aDetails.append( VclResId(SV_APP_GL) );
    else
#endif
        aDetails.append( VclResId(SV_APP_DEFAULT) );
    aDetails.append( "; " );

#if (defined LINUX || defined _WIN32 || defined MACOSX)
    aDetails.append( SV_APP_VCLBACKEND );
    aDetails.append( GetToolkitName() );
    aDetails.append( "; " );
#endif

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
    else if ( pSVData->maWinData.mpAppWin )
        return pSVData->maWinData.mpAppWin->GetText();
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
    return pSys ? pSys->GetDisplayScreenPosSizePixel( nScreen ) : tools::Rectangle();
}

namespace {
unsigned long calcDistSquare( const Point& i_rPoint, const tools::Rectangle& i_rRect )
{
    const Point aRectCenter( (i_rRect.Left() + i_rRect.Right())/2,
                       (i_rRect.Top() + i_rRect.Bottom())/ 2 );
    const long nDX = aRectCenter.X() - i_rPoint.X();
    const long nDY = aRectCenter.Y() - i_rPoint.Y();
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
        if( aCurScreenRect.IsInside( i_rRect ) )
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
    unsigned long nDist = ULONG_MAX;
    for( unsigned int i = 0; i < nScreens; i++ )
    {
        const tools::Rectangle aCurScreenRect( GetScreenPosSizePixel( i ) );
        const unsigned long nCurDist( calcDistSquare( aCenter, aCurScreenRect ) );
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

vcl::Window* Application::GetDefDialogParent()
{
    ImplSVData* pSVData = ImplGetSVData();
    // find some useful dialog parent

    // always use the topmost parent of the candidate
    // window to avoid using dialogs or floaters
    // as DefDialogParent

    // current focus frame
    vcl::Window *pWin = pSVData->maWinData.mpFocusWin;
    if (pWin && !pWin->IsMenuFloatingWindow())
    {
        while (pWin->mpWindowImpl && pWin->mpWindowImpl->mpParent)
            pWin = pWin->mpWindowImpl->mpParent;

        // check for corrupted window hierarchy, #122232#, may be we now crash somewhere else
        if (!pWin->mpWindowImpl)
        {
            OSL_FAIL( "Window hierarchy corrupted!" );
            pSVData->maWinData.mpFocusWin = nullptr;   // avoid further access
            return nullptr;
        }

        if ((pWin->mpWindowImpl->mnStyle & WB_INTROWIN) == 0)
        {
            return pWin->mpWindowImpl->mpFrameWindow->ImplGetWindow();
        }
    }

    // last active application frame
    pWin = pSVData->maWinData.mpActiveApplicationFrame;
    if (pWin)
    {
        return pWin->mpWindowImpl->mpFrameWindow->ImplGetWindow();
    }

    // first visible top window (may be totally wrong....)
    pWin = pSVData->maWinData.mpFirstFrame;
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

    return pSVData->mxDisplayConnection.get();
}

void Application::SetFilterHdl( const Link<ConvertData&,bool>& rLink )
{
    ImplGetSVData()->maGDIData.mpGrfConverter->SetFilterHdl( rLink );
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

bool Application::CanToggleImeStatusWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpImeStatus )
        pSVData->mpImeStatus  = pSVData->mpDefInst->CreateI18NImeStatus();
    return pSVData->mpImeStatus->canToggle();
}

void Application::ShowImeStatusWindow(bool bShow)
{
    ImplGetSVData()->maAppData.meShowImeStatusWindow = bShow
        ? ImplSVAppData::ImeStatusWindowMode_SHOW
        : ImplSVAppData::ImeStatusWindowMode_HIDE;

    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpImeStatus )
        pSVData->mpImeStatus  = pSVData->mpDefInst->CreateI18NImeStatus();
    pSVData->mpImeStatus->toggle();
}

bool Application::GetShowImeStatusWindowDefault()
{
    rtl_TextEncodingInfo aInfo;
    aInfo.StructSize = sizeof aInfo;
    return rtl_getTextEncodingInfo(osl_getThreadTextEncoding(), &aInfo)
        && aInfo.MaximumCharSize > 1;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
