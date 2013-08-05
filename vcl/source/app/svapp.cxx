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

#include "comphelper/processfactory.hxx"

#include "osl/module.h"
#include "osl/file.hxx"
#include "osl/thread.h"

#include "rtl/tencinfo.h"
#include "rtl/instance.hxx"
#include "rtl/process.h"

#include "tools/tools.h"
#include "tools/debug.hxx"
#include "tools/time.hxx"

#include "i18nlangtag/mslangid.hxx"

#include "unotools/syslocaleoptions.hxx"

#include "vcl/settings.hxx"
#include "vcl/keycod.hxx"
#include "vcl/event.hxx"
#include "vcl/vclevent.hxx"
#include "vcl/virdev.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/svapp.hxx"
#include "vcl/cvtgrf.hxx"
#include "vcl/unowrap.hxx"
#include "vcl/timer.hxx"
#include "vcl/unohelp.hxx"
#include "vcl/lazydelete.hxx"

#include "salinst.hxx"
#include "salframe.hxx"
#include "salsys.hxx"
#include "svdata.hxx"
#include "salimestatus.hxx"
#include "xconnection.hxx"
#include "window.h"
#include "accmgr.hxx"
#include "idlemgr.hxx"
#include "svids.hrc"

#include "com/sun/star/uno/Reference.h"
#include "com/sun/star/awt/XToolkit.hpp"
#include "com/sun/star/uno/XNamingService.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "comphelper/solarmutex.hxx"
#include "osl/process.h"

#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

// keycodes handled internally by VCL
class ImplReservedKey
{
public:
    ImplReservedKey( KeyCode aKeyCode, sal_uInt16 nResId ) :
      mKeyCode(aKeyCode), mnResId( nResId)
     {}

    KeyCode mKeyCode;
    sal_uInt16  mnResId;
};

typedef std::pair<ImplReservedKey*, size_t> ReservedKeys;
namespace
{
    struct ImplReservedKeysImpl
    {
        ReservedKeys* operator()()
        {
            static ImplReservedKey ImplReservedKeys[] =
            {
                ImplReservedKey(KeyCode(KEY_F1,0),                  SV_SHORTCUT_HELP),
                ImplReservedKey(KeyCode(KEY_F1,KEY_SHIFT),          SV_SHORTCUT_ACTIVEHELP),
                ImplReservedKey(KeyCode(KEY_F1,KEY_MOD1),           SV_SHORTCUT_CONTEXTHELP),
                ImplReservedKey(KeyCode(KEY_F2,KEY_SHIFT),          SV_SHORTCUT_CONTEXTHELP),
                ImplReservedKey(KeyCode(KEY_F4,KEY_MOD1),           SV_SHORTCUT_DOCKUNDOCK),
                ImplReservedKey(KeyCode(KEY_F4,KEY_MOD2),           SV_SHORTCUT_DOCKUNDOCK),
                ImplReservedKey(KeyCode(KEY_F4,KEY_MOD1|KEY_MOD2),  SV_SHORTCUT_DOCKUNDOCK),
                ImplReservedKey(KeyCode(KEY_F6,0),                  SV_SHORTCUT_NEXTSUBWINDOW),
                ImplReservedKey(KeyCode(KEY_F6,KEY_MOD1),           SV_SHORTCUT_TODOCUMENT),
                ImplReservedKey(KeyCode(KEY_F6,KEY_SHIFT),          SV_SHORTCUT_PREVSUBWINDOW),
                ImplReservedKey(KeyCode(KEY_F6,KEY_MOD1|KEY_SHIFT), SV_SHORTCUT_SPLITTER),
                ImplReservedKey(KeyCode(KEY_F10,0),                 SV_SHORTCUT_MENUBAR)
#ifdef UNX
                ,
                ImplReservedKey(KeyCode(KEY_1,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_2,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_3,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_4,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_5,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_6,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_7,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_8,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_9,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_0,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_ADD,KEY_SHIFT|KEY_MOD1), 0)
#endif
            };
            static ReservedKeys aKeys
            (
                &ImplReservedKeys[0],
                sizeof(ImplReservedKeys) / sizeof(ImplReservedKey)
            );
            return &aKeys;
        }
    };

    struct ImplReservedKeys
        : public rtl::StaticAggregate<ReservedKeys, ImplReservedKeysImpl> {};
}


extern "C" {
    typedef UnoWrapperBase* (SAL_CALL *FN_TkCreateUnoWrapper)();
}

// =======================================================================

// --------------
// - ImplHotKey -
// --------------

struct ImplHotKey
{
    ImplHotKey*             mpNext;
    void*                   mpUserData;
    KeyCode                 maKeyCode;
    Link                    maLink;
};

// =======================================================================

// -----------------
// - ImplEventHook -
// -----------------

struct ImplEventHook
{
    ImplEventHook*          mpNext;
    void*                   mpUserData;
    VCLEventHookProc        mpProc;
};

// =======================================================================

// ---------------------
// - ImplPostEventData -
// ---------------------

struct ImplPostEventData
{
    sal_uLong           mnEvent;
    const Window*   mpWin;
    sal_uLong           mnEventId;
    KeyEvent        maKeyEvent;
    MouseEvent      maMouseEvent;
    ZoomEvent       maZoomEvent;
    ScrollEvent     maScrollEvent;

       ImplPostEventData( sal_uLong nEvent, const Window* pWin, const KeyEvent& rKeyEvent ) :
        mnEvent( nEvent ), mpWin( pWin ), mnEventId( 0 ), maKeyEvent( rKeyEvent ) {}
       ImplPostEventData( sal_uLong nEvent, const Window* pWin, const MouseEvent& rMouseEvent ) :
        mnEvent( nEvent ), mpWin( pWin ), mnEventId( 0 ), maMouseEvent( rMouseEvent ) {}
       ImplPostEventData( sal_uLong nEvent, const Window* pWin, const ZoomEvent& rZoomEvent ) :
        mnEvent( nEvent ), mpWin( pWin ), mnEventId( 0 ), maZoomEvent( rZoomEvent ) {}
       ImplPostEventData( sal_uLong nEvent, const Window* pWin, const ScrollEvent& rScrollEvent ) :
        mnEvent( nEvent ), mpWin( pWin ), mnEventId( 0 ), maScrollEvent( rScrollEvent ) {}

    ~ImplPostEventData() {}
};

typedef ::std::pair< Window*, ImplPostEventData* > ImplPostEventPair;

static ::std::list< ImplPostEventPair > aPostedEventList;

// =======================================================================

Application* GetpApp()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData )
        return NULL;
    return pSVData->mpApp;
}

// -----------------------------------------------------------------------

Application::Application()
{
    // useful for themes at least, perhaps extensions too
    OUString aVar("LIBO_VERSION"), aValue(LIBO_VERSION_DOTTED);
    osl_setEnvironment(aVar.pData, aValue.pData);

    if( ! ImplGetSVData() )
        ImplInitSVData();
    ImplGetSVData()->mpApp = this;
    InitSalData();
}

// -----------------------------------------------------------------------

Application::~Application()
{
    ImplDeInitSVData();
    DeInitSalData();
    ImplGetSVData()->mpApp = NULL;
    ImplDestroySVData();
    GlobalDeInitTools();
}

// -----------------------------------------------------------------------

sal_Bool Application::QueryExit()
{
    WorkWindow* pAppWin = ImplGetSVData()->maWinData.mpAppWin;

    // call the close handler of the application window
    if ( pAppWin )
        return pAppWin->Close();
    else
        return sal_True;
}

// -----------------------------------------------------------------------

void Application::UserEvent( sal_uLong, void* )
{
}

// -----------------------------------------------------------------------

void Application::FocusChanged()
{
}

// -----------------------------------------------------------------------

void Application::DataChanged( const DataChangedEvent& )
{
}

// -----------------------------------------------------------------------
void Application::Init()
{
}

// -----------------------------------------------------------------------
void Application::InitFinished()
{
}

// -----------------------------------------------------------------------

void Application::DeInit()
{
}

// -----------------------------------------------------------------------

sal_uInt16 Application::GetCommandLineParamCount()
{
    return (sal_uInt16)osl_getCommandArgCount();
}

// -----------------------------------------------------------------------

OUString Application::GetCommandLineParam( sal_uInt16 nParam )
{
    OUString aParam;
    osl_getCommandArg( nParam, &aParam.pData );
    return aParam;
}

// -----------------------------------------------------------------------

OUString Application::GetAppFileName()
{
    ImplSVData* pSVData = ImplGetSVData();
    DBG_ASSERT( pSVData->maAppData.mpAppFileName, "AppFileName should be set to something after SVMain!" );
    if ( pSVData->maAppData.mpAppFileName )
        return *pSVData->maAppData.mpAppFileName;

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

// -----------------------------------------------------------------------

sal_uInt16 Application::Exception( sal_uInt16 nError )
{
    switch ( nError & EXC_MAJORTYPE )
    {
        // System has precedence (so do nothing)
        case EXC_SYSTEM:
            return 0;

        case EXC_DISPLAY:
        case EXC_REMOTE:
            return 0;

#ifdef DBG_UTIL
        case EXC_RSCNOTLOADED:
            Abort(OUString("Resource not loaded"));
            break;
        case EXC_SYSOBJNOTCREATED:
            Abort(OUString("System Object not created"));
            break;
        default:
            Abort(OUString("Unknown Error"));
            break;
#else
        default:
            Abort(OUString());
            break;
#endif
    }

    return 0;
}

// -----------------------------------------------------------------------

void Application::Abort( const OUString& rErrorText )
{
    //HACK: Dump core iff --norestore command line argument is given (assuming
    // this process is run by developers who are interested in cores, vs. end
    // users who are not):
    bool dumpCore = false;
    sal_uInt16 n = GetCommandLineParamCount();
    for (sal_uInt16 i = 0; i != n; ++i) {
        if (GetCommandLineParam(i).equals("--norestore")) {
            dumpCore = true;
            break;
        }
    }

    SalAbort( rErrorText, dumpCore );
}

// -----------------------------------------------------------------------

sal_uLong   Application::GetReservedKeyCodeCount()
{
    return ImplReservedKeys::get()->second;
}

const KeyCode*  Application::GetReservedKeyCode( sal_uLong i )
{
    if( i >= GetReservedKeyCodeCount() )
        return NULL;
    else
        return &ImplReservedKeys::get()->first[i].mKeyCode;
}

// -----------------------------------------------------------------------

void Application::Execute()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mbInAppExecute = sal_True;

    while ( !pSVData->maAppData.mbAppQuit )
        Application::Yield();

    pSVData->maAppData.mbInAppExecute = sal_False;
}

// -----------------------------------------------------------------------

inline void ImplYield( bool i_bWait, bool i_bAllEvents )
{
    ImplSVData* pSVData = ImplGetSVData();

    // run timers that have timed out
    if ( !pSVData->mbNoCallTimer )
        while ( pSVData->mbNotAllTimerCalled )
            Timer::ImplTimerCallbackProc();

    pSVData->maAppData.mnDispatchLevel++;
    // do not wait for events if application was already quit; in that
    // case only dispatch events already available
    // do not wait for events either if the app decided that it is too busy for timers
    // (feature added for the slideshow)
    pSVData->mpDefInst->Yield( i_bWait && !pSVData->maAppData.mbAppQuit && !pSVData->maAppData.mbNoYield, i_bAllEvents );
    pSVData->maAppData.mnDispatchLevel--;

    // flush lazy deleted objects
    if( pSVData->maAppData.mnDispatchLevel == 0 )
        vcl::LazyDelete::flush();

    // the system timer events will not necessarily come in in non waiting mode
    // e.g. on aqua; need to trigger timer checks manually
    if( pSVData->maAppData.mbNoYield && !pSVData->mbNoCallTimer )
    {
        do
        {
            Timer::ImplTimerCallbackProc();
        }
        while( pSVData->mbNotAllTimerCalled );
    }

    // call post yield listeners
    if( pSVData->maAppData.mpPostYieldListeners )
        pSVData->maAppData.mpPostYieldListeners->callListeners( NULL );
}

// -----------------------------------------------------------------------

void Application::Reschedule( bool i_bAllEvents )
{
    ImplYield( false, i_bAllEvents );
}

// -----------------------------------------------------------------------

void Application::Yield()
{
    ImplYield( true, false );
}

// -----------------------------------------------------------------------

IMPL_STATIC_LINK_NOINSTANCE( ImplSVAppData, ImplQuitMsg, void*, EMPTYARG )
{
    ImplGetSVData()->maAppData.mbAppQuit = sal_True;
    return 0;
}

// -----------------------------------------------------------------------

void Application::Quit()
{
    Application::PostUserEvent( STATIC_LINK( NULL, ImplSVAppData, ImplQuitMsg ) );
}

// -----------------------------------------------------------------------

comphelper::SolarMutex& Application::GetSolarMutex()
{
    ImplSVData* pSVData = ImplGetSVData();
    return *(pSVData->mpDefInst->GetYieldMutex());
}

// -----------------------------------------------------------------------

oslThreadIdentifier Application::GetMainThreadIdentifier()
{
    return ImplGetSVData()->mnMainThreadId;
}

// -----------------------------------------------------------------------

sal_uLong Application::ReleaseSolarMutex()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->mpDefInst->ReleaseYieldMutex();
}

// -----------------------------------------------------------------------

void Application::AcquireSolarMutex( sal_uLong nCount )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDefInst->AcquireYieldMutex( nCount );
}

// -----------------------------------------------------------------------

sal_Bool Application::IsInMain()
{
    return ImplGetSVData()->maAppData.mbInAppMain;
}

// -----------------------------------------------------------------------

sal_Bool Application::IsInExecute()
{
    return ImplGetSVData()->maAppData.mbInAppExecute;
}

// -----------------------------------------------------------------------

sal_Bool Application::IsInModalMode()
{
    return (ImplGetSVData()->maAppData.mnModalMode != 0);
}

// -----------------------------------------------------------------------

sal_uInt16 Application::GetDispatchLevel()
{
    return ImplGetSVData()->maAppData.mnDispatchLevel;
}

// -----------------------------------------------------------------------

bool Application::AnyInput( sal_uInt16 nType )
{
    return ImplGetSVData()->mpDefInst->AnyInput( nType );
}

// -----------------------------------------------------------------------

sal_uLong Application::GetLastInputInterval()
{
    return (Time::GetSystemTicks()-ImplGetSVData()->maAppData.mnLastInputTime);
}

// -----------------------------------------------------------------------

extern int nImplSysDialog;

sal_Bool Application::IsUICaptured()
{
    ImplSVData* pSVData = ImplGetSVData();

    // If mouse was captured, or if in tracking- or in select-mode of a floatingwindow (e.g. menus
    // or pulldown toolboxes) another window should be created
    // D&D active !!!
    if ( pSVData->maWinData.mpCaptureWin || pSVData->maWinData.mpTrackWin ||
         pSVData->maWinData.mpFirstFloat || nImplSysDialog )
        return sal_True;
    else
        return sal_False;
}

// -----------------------------------------------------------------------

void Application::SystemSettingsChanging( AllSettings& /*rSettings*/,
                                          Window* /*pFrame*/ )
{
}

// -----------------------------------------------------------------------

void Application::MergeSystemSettings( AllSettings& rSettings )
{
    Window* pWindow = ImplGetSVData()->maWinData.mpFirstFrame;
    if( ! pWindow )
        pWindow = ImplGetDefaultWindow();
    if( pWindow )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if ( !pSVData->maAppData.mbSettingsInit )
        {
            // side effect: ImplUpdateGlobalSettings does an ImplGetFrame()->UpdateSettings
            pWindow->ImplUpdateGlobalSettings( *pSVData->maAppData.mpSettings );
            pSVData->maAppData.mbSettingsInit = sal_True;
        }
        // side effect: ImplUpdateGlobalSettings does an ImplGetFrame()->UpdateSettings
        pWindow->ImplUpdateGlobalSettings( rSettings, sal_False );
    }
}

// -----------------------------------------------------------------------

bool Application::ValidateSystemFont()
{
    Window* pWindow = ImplGetSVData()->maWinData.mpFirstFrame;
    if( ! pWindow )
        pWindow = ImplGetDefaultWindow();

    if( pWindow )
    {
        AllSettings aSettings;
        pWindow->ImplGetFrame()->UpdateSettings( aSettings );
        return pWindow->ImplCheckUIFont( aSettings.GetStyleSettings().GetAppFont() );
    }
    return false;
}

// -----------------------------------------------------------------------

void Application::SetSettings( const AllSettings& rSettings )
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maAppData.mpSettings )
    {
        GetSettings();
        *pSVData->maAppData.mpSettings = rSettings;
        ResMgr::SetDefaultLocale( rSettings.GetUILanguageTag() );
    }
    else
    {
        AllSettings aOldSettings = *pSVData->maAppData.mpSettings;
        if( aOldSettings.GetUILanguageTag().getLanguageType() != rSettings.GetUILanguageTag().getLanguageType() &&
                pSVData->mpResMgr )
        {
            delete pSVData->mpResMgr;
            pSVData->mpResMgr = NULL;
        }
        ResMgr::SetDefaultLocale( rSettings.GetUILanguageTag() );
        *pSVData->maAppData.mpSettings = rSettings;
        sal_uLong nChangeFlags = aOldSettings.GetChangeFlags( *pSVData->maAppData.mpSettings );
        if ( nChangeFlags )
        {
            DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &aOldSettings, nChangeFlags );
            GetpApp()->DataChanged( aDCEvt );

            // notify data change handler
            ImplCallEventListeners( VCLEVENT_APPLICATION_DATACHANGED, NULL, &aDCEvt);

            // Update all windows
            Window* pFirstFrame = pSVData->maWinData.mpFirstFrame;
            // Reset data that needs to be re-calculated
            long nOldDPIX = 0;
            long nOldDPIY = 0;
            if ( pFirstFrame )
            {
                nOldDPIX = pFirstFrame->mnDPIX;
                nOldDPIY = pFirstFrame->mnDPIY;
                pSVData->maGDIData.mnAppFontX = 0;
            }
            Window* pFrame = pFirstFrame;
            while ( pFrame )
            {
                // restore AppFont cache data
                pFrame->mpWindowImpl->mpFrameData->meMapUnit = MAP_PIXEL;

                // call UpdateSettings from ClientWindow in order to prevent updating data twice
                Window* pClientWin = pFrame;
                while ( pClientWin->ImplGetClientWindow() )
                    pClientWin = pClientWin->ImplGetClientWindow();
                pClientWin->UpdateSettings( rSettings, sal_True );

                Window* pTempWin = pFrame->mpWindowImpl->mpFrameData->mpFirstOverlap;
                while ( pTempWin )
                {
                    // call UpdateSettings from ClientWindow in order to prevent updating data twice
                    pClientWin = pTempWin;
                    while ( pClientWin->ImplGetClientWindow() )
                        pClientWin = pClientWin->ImplGetClientWindow();
                    pClientWin->UpdateSettings( rSettings, sal_True );
                    pTempWin = pTempWin->mpWindowImpl->mpNextOverlap;
                }

                pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
            }

            // if DPI resolution for screen output was changed set the new resolution for all
            // screen compatible VirDev´s
            pFirstFrame = pSVData->maWinData.mpFirstFrame;
            if ( pFirstFrame )
            {
                if ( (pFirstFrame->mnDPIX != nOldDPIX) ||
                     (pFirstFrame->mnDPIY != nOldDPIY) )
                {
                    VirtualDevice* pVirDev = pSVData->maGDIData.mpFirstVirDev;
                    while ( pVirDev )
                    {
                        if ( pVirDev->mbScreenComp &&
                             (pVirDev->mnDPIX == nOldDPIX) &&
                             (pVirDev->mnDPIY == nOldDPIY) )
                        {
                            pVirDev->mnDPIX = pFirstFrame->mnDPIX;
                            pVirDev->mnDPIY = pFirstFrame->mnDPIY;
                            if ( pVirDev->IsMapMode() )
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

// -----------------------------------------------------------------------

const AllSettings& Application::GetSettings()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maAppData.mpSettings )
    {
        pSVData->maAppData.mpCfgListener = new LocaleConfigurationListener;
        pSVData->maAppData.mpSettings = new AllSettings();
        pSVData->maAppData.mpSettings->GetSysLocale().GetOptions().AddListener( pSVData->maAppData.mpCfgListener );
    }

    return *(pSVData->maAppData.mpSettings);
}

// -----------------------------------------------------------------------

void Application::NotifyAllWindows( DataChangedEvent& rDCEvt )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pFrame = pSVData->maWinData.mpFirstFrame;
    while ( pFrame )
    {
        pFrame->NotifyAllChildren( rDCEvt );

        Window* pSysWin = pFrame->mpWindowImpl->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            pSysWin->NotifyAllChildren( rDCEvt );
            pSysWin = pSysWin->mpWindowImpl->mpNextOverlap;
        }

        pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
    }
}

// -----------------------------------------------------------------------

void Application::ImplCallEventListeners( sal_uLong nEvent, Window *pWin, void* pData )
{
    ImplSVData* pSVData = ImplGetSVData();
    VclWindowEvent aEvent( pWin, nEvent, pData );

    if ( pSVData->maAppData.mpEventListeners )
        pSVData->maAppData.mpEventListeners->Call( &aEvent );
}

// -----------------------------------------------------------------------

void Application::ImplCallEventListeners( VclSimpleEvent* pEvent )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maAppData.mpEventListeners )
        pSVData->maAppData.mpEventListeners->Call( pEvent );
}

// -----------------------------------------------------------------------

void Application::AddEventListener( const Link& rEventListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( !pSVData->maAppData.mpEventListeners )
        pSVData->maAppData.mpEventListeners = new VclEventListeners;
    pSVData->maAppData.mpEventListeners->addListener( rEventListener );
}

// -----------------------------------------------------------------------

void Application::RemoveEventListener( const Link& rEventListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maAppData.mpEventListeners )
        pSVData->maAppData.mpEventListeners->removeListener( rEventListener );
}

// -----------------------------------------------------------------------
void Application::AddKeyListener( const Link& rKeyListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( !pSVData->maAppData.mpKeyListeners )
        pSVData->maAppData.mpKeyListeners = new VclEventListeners;
    pSVData->maAppData.mpKeyListeners->addListener( rKeyListener );
}

// -----------------------------------------------------------------------

void Application::RemoveKeyListener( const Link& rKeyListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maAppData.mpKeyListeners )
        pSVData->maAppData.mpKeyListeners->removeListener( rKeyListener );
}

// -----------------------------------------------------------------------

sal_Bool Application::HandleKey( sal_uLong nEvent, Window *pWin, KeyEvent* pKeyEvent )
{
    // let listeners process the key event
    VclWindowEvent aEvent( pWin, nEvent, (void *) pKeyEvent );

    ImplSVData* pSVData = ImplGetSVData();
    sal_Bool bProcessed = sal_False;

    if ( pSVData->maAppData.mpKeyListeners )
        bProcessed = pSVData->maAppData.mpKeyListeners->Process( &aEvent );

    return bProcessed;
}

// -----------------------------------------------------------------------------

sal_uLong Application::PostKeyEvent( sal_uLong nEvent, Window *pWin, KeyEvent* pKeyEvent )
{
    const SolarMutexGuard aGuard;
    sal_uLong               nEventId = 0;

    if( pWin && pKeyEvent )
    {
        ImplPostEventData* pPostEventData = new ImplPostEventData( nEvent, pWin, *pKeyEvent );

        PostUserEvent( nEventId,
                       STATIC_LINK( NULL, Application, PostEventHandler ),
                       pPostEventData );

        if( nEventId )
        {
            pPostEventData->mnEventId = nEventId;
            aPostedEventList.push_back( ImplPostEventPair( pWin, pPostEventData ) );
        }
        else
            delete pPostEventData;
    }

    return nEventId;
}

// -----------------------------------------------------------------------------

sal_uLong Application::PostMouseEvent( sal_uLong nEvent, Window *pWin, MouseEvent* pMouseEvent )
{
    const SolarMutexGuard aGuard;
    sal_uLong               nEventId = 0;

    if( pWin && pMouseEvent )
    {
        Point aTransformedPos( pMouseEvent->GetPosPixel() );

        aTransformedPos.X() += pWin->mnOutOffX;
        aTransformedPos.Y() += pWin->mnOutOffY;

        const MouseEvent aTransformedEvent( aTransformedPos, pMouseEvent->GetClicks(), pMouseEvent->GetMode(),
                                            pMouseEvent->GetButtons(), pMouseEvent->GetModifier() );

        ImplPostEventData* pPostEventData = new ImplPostEventData( nEvent, pWin, aTransformedEvent );

        PostUserEvent( nEventId,
                       STATIC_LINK( NULL, Application, PostEventHandler ),
                       pPostEventData );

        if( nEventId )
        {
            pPostEventData->mnEventId = nEventId;
            aPostedEventList.push_back( ImplPostEventPair( pWin, pPostEventData ) );
        }
        else
            delete pPostEventData;
    }

    return nEventId;
}

#if !HAVE_FEATURE_DESKTOP

sal_uLong Application::PostZoomEvent( sal_uLong nEvent, Window *pWin, ZoomEvent* pZoomEvent )
{
    const SolarMutexGuard aGuard;
    sal_uLong               nEventId = 0;

    if( pWin && pZoomEvent )
    {
        Point aTransformedPos( pZoomEvent->GetCenter() );

        aTransformedPos.X() += pWin->mnOutOffX;
        aTransformedPos.Y() += pWin->mnOutOffY;

        const ZoomEvent aTransformedEvent( aTransformedPos, pZoomEvent->GetScale() );

        ImplPostEventData* pPostEventData = new ImplPostEventData( nEvent, pWin, aTransformedEvent );

        PostUserEvent( nEventId,
                       STATIC_LINK( NULL, Application, PostEventHandler ),
                       pPostEventData );

        if( nEventId )
        {
            pPostEventData->mnEventId = nEventId;
            aPostedEventList.push_back( ImplPostEventPair( pWin, pPostEventData ) );
        }
        else
            delete pPostEventData;
    }

    return nEventId;
}

sal_uLong Application::PostScrollEvent( sal_uLong nEvent, Window *pWin, ScrollEvent* pScrollEvent )
{
    const SolarMutexGuard aGuard;
    sal_uLong               nEventId = 0;

    if( pWin && pScrollEvent )
    {
        ImplPostEventData* pPostEventData = new ImplPostEventData( nEvent, pWin, *pScrollEvent );

        PostUserEvent( nEventId,
                       STATIC_LINK( NULL, Application, PostEventHandler ),
                       pPostEventData );

        if( nEventId )
        {
            pPostEventData->mnEventId = nEventId;
            aPostedEventList.push_back( ImplPostEventPair( pWin, pPostEventData ) );
        }
        else
            delete pPostEventData;
    }

    return nEventId;
}

#endif // !HAVE_FEATURE_DESKTOP

IMPL_STATIC_LINK_NOINSTANCE( Application, PostEventHandler, void*, pCallData )
{
    const SolarMutexGuard aGuard;
    ImplPostEventData*  pData = static_cast< ImplPostEventData * >( pCallData );
    const void*         pEventData;
    sal_uLong               nEvent;
    const sal_uLong         nEventId = pData->mnEventId;

    switch( pData->mnEvent )
    {
        case VCLEVENT_WINDOW_MOUSEMOVE:
            nEvent = SALEVENT_EXTERNALMOUSEMOVE;
            pEventData = &pData->maMouseEvent;
        break;

        case VCLEVENT_WINDOW_MOUSEBUTTONDOWN:
            nEvent = SALEVENT_EXTERNALMOUSEBUTTONDOWN;
            pEventData = &pData->maMouseEvent;
        break;

        case VCLEVENT_WINDOW_MOUSEBUTTONUP:
            nEvent = SALEVENT_EXTERNALMOUSEBUTTONUP;
            pEventData = &pData->maMouseEvent;
        break;

        case VCLEVENT_WINDOW_KEYINPUT:
            nEvent = SALEVENT_EXTERNALKEYINPUT;
            pEventData = &pData->maKeyEvent;
        break;

        case VCLEVENT_WINDOW_KEYUP:
            nEvent = SALEVENT_EXTERNALKEYUP;
            pEventData = &pData->maKeyEvent;
        break;

        case VCLEVENT_WINDOW_ZOOM:
            nEvent = SALEVENT_EXTERNALZOOM;
            pEventData = &pData->maZoomEvent;
        break;

        case VCLEVENT_WINDOW_SCROLL:
            nEvent = SALEVENT_EXTERNALSCROLL;
            pEventData = &pData->maScrollEvent;
        break;

        default:
            nEvent = 0;
            pEventData = NULL;
        break;
    };

    if( pData->mpWin && pData->mpWin->mpWindowImpl->mpFrameWindow && pEventData )
        ImplWindowFrameProc( pData->mpWin->mpWindowImpl->mpFrameWindow, NULL, (sal_uInt16) nEvent, pEventData );

    // remove this event from list of posted events, watch for destruction of internal data
    ::std::list< ImplPostEventPair >::iterator aIter( aPostedEventList.begin() );

    while( aIter != aPostedEventList.end() )
    {
        if( nEventId == (*aIter).second->mnEventId )
        {
            delete (*aIter).second;
            aIter = aPostedEventList.erase( aIter );
        }
        else
            ++aIter;
    }

    return 0;
}

// -----------------------------------------------------------------------

void Application::RemoveMouseAndKeyEvents( Window* pWin )
{
    const SolarMutexGuard aGuard;

    // remove all events for specific window, watch for destruction of internal data
    ::std::list< ImplPostEventPair >::iterator aIter( aPostedEventList.begin() );

    while( aIter != aPostedEventList.end() )
    {
        if( pWin == (*aIter).first )
        {
            if( (*aIter).second->mnEventId )
                RemoveUserEvent( (*aIter).second->mnEventId );

            delete (*aIter).second;
            aIter = aPostedEventList.erase( aIter );
        }
        else
            ++aIter;
    }
}

// -----------------------------------------------------------------------

sal_uLong Application::PostUserEvent( const Link& rLink, void* pCaller )
{
    sal_uLong nEventId;
    PostUserEvent( nEventId, rLink, pCaller );
    return nEventId;
}

// -----------------------------------------------------------------------

sal_Bool Application::PostUserEvent( sal_uLong& rEventId, const Link& rLink, void* pCaller )
{
    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mnEvent   = 0;
    pSVEvent->mpData    = pCaller;
    pSVEvent->mpLink    = new Link( rLink );
    pSVEvent->mpWindow  = NULL;
    pSVEvent->mbCall    = sal_True;
    rEventId = (sal_uLong)pSVEvent;
    Window* pDefWindow = ImplGetDefaultWindow();
    if ( pDefWindow && pDefWindow->ImplGetFrame()->PostEvent( pSVEvent ) )
        return sal_True;
    else
    {
        rEventId = 0;
        delete pSVEvent;
        return sal_False;
    }
}

// -----------------------------------------------------------------------

void Application::RemoveUserEvent( sal_uLong nUserEvent )
{
    if(nUserEvent)
    {
        ImplSVEvent* pSVEvent = (ImplSVEvent*)nUserEvent;

        DBG_ASSERT( !pSVEvent->mpWindow,
                    "Application::RemoveUserEvent(): Event is send to a window" );
        DBG_ASSERT( pSVEvent->mbCall,
                    "Application::RemoveUserEvent(): Event is already removed" );

        if ( pSVEvent->mpWindow )
        {
            if( ! pSVEvent->maDelData.IsDead() )
                pSVEvent->mpWindow->ImplRemoveDel( &(pSVEvent->maDelData) );
            pSVEvent->mpWindow = NULL;
        }

        pSVEvent->mbCall = sal_False;
    }
}

// -----------------------------------------------------------------------

sal_Bool Application::InsertIdleHdl( const Link& rLink, sal_uInt16 nPrio )
{
    ImplSVData* pSVData = ImplGetSVData();

    // create if does not exist
    if ( !pSVData->maAppData.mpIdleMgr )
        pSVData->maAppData.mpIdleMgr = new ImplIdleMgr;

    return pSVData->maAppData.mpIdleMgr->InsertIdleHdl( rLink, nPrio );
}

// -----------------------------------------------------------------------

void Application::RemoveIdleHdl( const Link& rLink )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maAppData.mpIdleMgr )
        pSVData->maAppData.mpIdleMgr->RemoveIdleHdl( rLink );
}

// -----------------------------------------------------------------------

void Application::EnableNoYieldMode( bool i_bNoYield )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mbNoYield = i_bNoYield;
}

// -----------------------------------------------------------------------

void Application::AddPostYieldListener( const Link& i_rListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->maAppData.mpPostYieldListeners )
        pSVData->maAppData.mpPostYieldListeners = new VclEventListeners2();
    pSVData->maAppData.mpPostYieldListeners->addListener( i_rListener );
}

// -----------------------------------------------------------------------

void Application::RemovePostYieldListener( const Link& i_rListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maAppData.mpPostYieldListeners )
        pSVData->maAppData.mpPostYieldListeners->removeListener( i_rListener );
}

// -----------------------------------------------------------------------

WorkWindow* Application::GetAppWindow()
{
    return ImplGetSVData()->maWinData.mpAppWin;
}

// -----------------------------------------------------------------------

Window* Application::GetFocusWindow()
{
    return ImplGetSVData()->maWinData.mpFocusWin;
}

// -----------------------------------------------------------------------

OutputDevice* Application::GetDefaultDevice()
{
    return ImplGetDefaultWindow();
}

// -----------------------------------------------------------------------

Window* Application::GetFirstTopLevelWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->maWinData.mpFirstFrame;
}

// -----------------------------------------------------------------------

Window* Application::GetNextTopLevelWindow( Window* pWindow )
{
    return pWindow->mpWindowImpl->mpFrameData->mpNextFrame;
}

// -----------------------------------------------------------------------

long    Application::GetTopWindowCount()
{
    long nRet = 0;
    ImplSVData* pSVData = ImplGetSVData();
    Window *pWin = pSVData ? pSVData->maWinData.mpFirstFrame : NULL;
    while( pWin )
    {
        if( pWin->ImplGetWindow()->IsTopWindow() )
            nRet++;
        pWin = pWin->mpWindowImpl->mpFrameData->mpNextFrame;
    }
    return nRet;
}

// -----------------------------------------------------------------------

Window* Application::GetTopWindow( long nIndex )
{
    long nIdx = 0;
    ImplSVData* pSVData = ImplGetSVData();
    Window *pWin = pSVData ? pSVData->maWinData.mpFirstFrame : NULL;
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
    return NULL;
}

// -----------------------------------------------------------------------

Window* Application::GetActiveTopWindow()
{
    Window *pWin = ImplGetSVData()->maWinData.mpFocusWin;
    while( pWin )
    {
        if( pWin->IsTopWindow() )
            return pWin;
        pWin = pWin->mpWindowImpl->mpParent;
    }
    return NULL;
}

// -----------------------------------------------------------------------

void Application::SetAppName( const OUString& rUniqueName )
{
    ImplSVData* pSVData = ImplGetSVData();

    // create if does not exist
    if ( !pSVData->maAppData.mpAppName )
        pSVData->maAppData.mpAppName = new OUString( rUniqueName );
    else
        *(pSVData->maAppData.mpAppName) = rUniqueName;
}

// -----------------------------------------------------------------------

OUString Application::GetAppName()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maAppData.mpAppName )
        return *(pSVData->maAppData.mpAppName);
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void Application::SetDisplayName( const OUString& rName )
{
    ImplSVData* pSVData = ImplGetSVData();

    // create if does not exist
    if ( !pSVData->maAppData.mpDisplayName )
        pSVData->maAppData.mpDisplayName = new OUString( rName );
    else
        *(pSVData->maAppData.mpDisplayName) = rName;
}

// -----------------------------------------------------------------------

OUString Application::GetDisplayName()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maAppData.mpDisplayName )
        return *(pSVData->maAppData.mpDisplayName);
    else if ( pSVData->maWinData.mpAppWin )
        return pSVData->maWinData.mpAppWin->GetText();
    else
        return OUString("");
}

// -----------------------------------------------------------------------

unsigned int Application::GetScreenCount()
{
    SalSystem* pSys = ImplGetSalSystem();
    return pSys ? pSys->GetDisplayScreenCount() : 0;
}

bool Application::IsUnifiedDisplay()
{
    SalSystem* pSys = ImplGetSalSystem();
    return pSys ? pSys->IsUnifiedDisplay() : true;
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

Rectangle Application::GetScreenPosSizePixel( unsigned int nScreen )
{
    SalSystem* pSys = ImplGetSalSystem();
    return pSys ? pSys->GetDisplayScreenPosSizePixel( nScreen ) : Rectangle();
}

namespace {
unsigned long calcDistSquare( const Point& i_rPoint, const Rectangle& i_rRect )
{
    const Point aRectCenter( (i_rRect.Left() + i_rRect.Right())/2,
                       (i_rRect.Top() + i_rRect.Bottom())/ 2 );
    const long nDX = aRectCenter.X() - i_rPoint.X();
    const long nDY = aRectCenter.Y() - i_rPoint.Y();
    return nDX*nDX + nDY*nDY;
}
}

unsigned int Application::GetBestScreen( const Rectangle& i_rRect )
{
    if( !IsUnifiedDisplay() )
        return GetDisplayBuiltInScreen();

    const unsigned int nScreens = GetScreenCount();
    unsigned int nBestMatchScreen = 0;
    unsigned long nOverlap = 0;
    for( unsigned int i = 0; i < nScreens; i++ )
    {
        const Rectangle aCurScreenRect( GetScreenPosSizePixel( i ) );
        // if a screen contains the rectangle completely it is obviously the best screen
        if( aCurScreenRect.IsInside( i_rRect ) )
            return i;
        // next the screen which contains most of the area of the rect is the best
        Rectangle aIntersection( aCurScreenRect.GetIntersection( i_rRect ) );
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
        const Rectangle aCurScreenRect( GetScreenPosSizePixel( i ) );
        const unsigned long nCurDist( calcDistSquare( aCenter, aCurScreenRect ) );
        if( nCurDist < nDist )
        {
            nBestMatchScreen = i;
            nDist = nCurDist;
        }
    }
    return nBestMatchScreen;
}

// -----------------------------------------------------------------------

sal_Bool Application::InsertAccel( Accelerator* pAccel )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( !pSVData->maAppData.mpAccelMgr )
        pSVData->maAppData.mpAccelMgr = new ImplAccelManager();
    return pSVData->maAppData.mpAccelMgr->InsertAccel( pAccel );
}

// -----------------------------------------------------------------------

void Application::RemoveAccel( Accelerator* pAccel )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maAppData.mpAccelMgr )
        pSVData->maAppData.mpAccelMgr->RemoveAccel( pAccel );
}

// -----------------------------------------------------------------------

void Application::SetHelp( Help* pHelp )
{
    ImplGetSVData()->maAppData.mpHelp = pHelp;
}

// -----------------------------------------------------------------------

Help* Application::GetHelp()
{
    return ImplGetSVData()->maAppData.mpHelp;
}

// -----------------------------------------------------------------------

void Application::EnableAutoHelpId( sal_Bool bEnabled )
{
    ImplGetSVData()->maHelpData.mbAutoHelpId = bEnabled;
}

// -----------------------------------------------------------------------

sal_Bool Application::IsAutoHelpIdEnabled()
{
    return ImplGetSVData()->maHelpData.mbAutoHelpId;
}

// -----------------------------------------------------------------------

void Application::EnableAutoMnemonic( sal_Bool bEnabled )
{
    AllSettings aSettings = GetSettings();
    StyleSettings aStyle = aSettings.GetStyleSettings();
    aStyle.SetAutoMnemonic( bEnabled );
    aSettings.SetStyleSettings( aStyle );
    SetSettings( aSettings );
}

// -----------------------------------------------------------------------

sal_Bool Application::IsAutoMnemonicEnabled()
{
    return GetSettings().GetStyleSettings().GetAutoMnemonic();
}

// -----------------------------------------------------------------------

void Application::SetDialogScaleX( short nScale )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mnDialogScaleX = nScale;
    pSVData->maGDIData.mnAppFontX = pSVData->maGDIData.mnRealAppFontX;
    if ( nScale )
        pSVData->maGDIData.mnAppFontX += (pSVData->maGDIData.mnAppFontX*nScale)/100;
}

// -----------------------------------------------------------------------

void Application::SetDefDialogParent( Window* pWindow )
{
    ImplGetSVData()->maWinData.mpDefDialogParent = pWindow;
}

// -----------------------------------------------------------------------

Window* Application::GetDefDialogParent()
{
    ImplSVData* pSVData = ImplGetSVData();
    // #103442# find some useful dialog parent if there
    // was no default set
    // NOTE: currently even the default is not used
    if( sal_False && pSVData->maWinData.mpDefDialogParent != NULL )
        return pSVData->maWinData.mpDefDialogParent;
    else
    {
        // always use the topmost parent of the candidate
        // window to avoid using dialogs or floaters
        // as DefDialogParent

        // current focus frame
        Window *pWin = NULL;
        if( (pWin = pSVData->maWinData.mpFocusWin) != NULL )
        {
            while( pWin->mpWindowImpl && pWin->mpWindowImpl->mpParent )
                pWin = pWin->mpWindowImpl->mpParent;

            if( (pWin->mpWindowImpl->mnStyle & WB_INTROWIN) == 0 )
            {
                // check for corrupted window hierarchy, #122232#, may be we now crash somewhere else
                if( !pWin->mpWindowImpl )
                {
                    OSL_FAIL( "Window hierarchy corrupted!" );
                    pSVData->maWinData.mpFocusWin = NULL;   // avoid further access
                    return NULL;
                }

                return pWin->mpWindowImpl->mpFrameWindow->ImplGetWindow();
            }
        }
        // last active application frame
        if( NULL != (pWin = pSVData->maWinData.mpActiveApplicationFrame) )
        {
            return pWin->mpWindowImpl->mpFrameWindow->ImplGetWindow();
        }
        else
        {
            // first visible top window (may be totally wrong....)
            pWin = pSVData->maWinData.mpFirstFrame;
            while( pWin )
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
            return NULL;
        }
    }
}

// -----------------------------------------------------------------------

Application::DialogCancelMode Application::GetDialogCancelMode()
{
    return ImplGetSVData()->maAppData.meDialogCancel;
}

void Application::SetDialogCancelMode( DialogCancelMode mode )
{
    ImplGetSVData()->maAppData.meDialogCancel = mode;
}

// -----------------------------------------------------------------------

sal_Bool Application::IsDialogCancelEnabled()
{
    return ImplGetSVData()->maAppData.meDialogCancel != DIALOG_CANCEL_OFF;
}

// -----------------------------------------------------------------------

void Application::SetSystemWindowMode( sal_uInt16 nMode )
{
    ImplGetSVData()->maAppData.mnSysWinMode = nMode;
}

// -----------------------------------------------------------------------

sal_uInt16 Application::GetSystemWindowMode()
{
    return ImplGetSVData()->maAppData.mnSysWinMode;
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > Application::GetVCLToolkit()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > xT;
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper( sal_True );
    if ( pWrapper )
        xT = pWrapper->GetVCLToolkit();
    return xT;
}

// -----------------------------------------------------------------------

#ifdef DISABLE_DYNLOADING

extern "C" { UnoWrapperBase* CreateUnoWrapper(); }

#else

extern "C" { static void SAL_CALL thisModule() {} }

#endif

UnoWrapperBase* Application::GetUnoWrapper( sal_Bool bCreateIfNotExist )
{
    ImplSVData* pSVData = ImplGetSVData();
    static bool bAlreadyTriedToCreate = false;
    if ( !pSVData->mpUnoWrapper && bCreateIfNotExist && !bAlreadyTriedToCreate )
    {
#ifndef DISABLE_DYNLOADING
        OUString aLibName = ::vcl::unohelper::CreateLibraryName(
#ifdef LIBO_MERGELIBS
                                                                       "merged",
#else
                                                                       "tk",
#endif
                                                                       sal_True );
        oslModule hTkLib = osl_loadModuleRelative(
            &thisModule, aLibName.pData, SAL_LOADMODULE_DEFAULT );
        if ( hTkLib )
        {
            OUString aFunctionName( "CreateUnoWrapper" );
            FN_TkCreateUnoWrapper fnCreateWrapper = (FN_TkCreateUnoWrapper)osl_getFunctionSymbol( hTkLib, aFunctionName.pData );
            if ( fnCreateWrapper )
            {
                pSVData->mpUnoWrapper = fnCreateWrapper();
            }
        }
        DBG_ASSERT( pSVData->mpUnoWrapper, "UnoWrapper could not be created!" );
#else
        pSVData->mpUnoWrapper = CreateUnoWrapper();
#endif
        bAlreadyTriedToCreate = true;
    }
    return pSVData->mpUnoWrapper;
}

// -----------------------------------------------------------------------

void Application::SetUnoWrapper( UnoWrapperBase* pWrapper )
{
    ImplSVData* pSVData = ImplGetSVData();
    DBG_ASSERT( !pSVData->mpUnoWrapper, "SetUnoWrapper: Wrapper already exists" );
    pSVData->mpUnoWrapper = pWrapper;
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayConnection > Application::GetDisplayConnection()
{
    ImplSVData* pSVData = ImplGetSVData();

    if( !pSVData->mxDisplayConnection.is() )
    {
        pSVData->mxDisplayConnection.set( new ::vcl::DisplayConnection );
        pSVData->mxDisplayConnection->start();
    }

    return pSVData->mxDisplayConnection.get();
}

// -----------------------------------------------------------------------

void Application::SetFilterHdl( const Link& rLink )
{
    ImplGetSVData()->maGDIData.mpGrfConverter->SetFilterHdl( rLink );
}

// -----------------------------------------------------------------------

bool ImplCallHotKey( const KeyCode& rKeyCode )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplHotKey*     pHotKeyData = pSVData->maAppData.mpFirstHotKey;
    while ( pHotKeyData )
    {
        if ( pHotKeyData->maKeyCode.IsDefinedKeyCodeEqual( rKeyCode ) )
        {
            pHotKeyData->maLink.Call( pHotKeyData->mpUserData );
            return true;
        }

        pHotKeyData = pHotKeyData->mpNext;
    }

    return false;
}

// -----------------------------------------------------------------------

void ImplFreeHotKeyData()
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplHotKey*     pTempHotKeyData;
    ImplHotKey*     pHotKeyData = pSVData->maAppData.mpFirstHotKey;
    while ( pHotKeyData )
    {
        pTempHotKeyData = pHotKeyData->mpNext;
        delete pHotKeyData;
        pHotKeyData = pTempHotKeyData;
    }

    pSVData->maAppData.mpFirstHotKey = NULL;
}

// -----------------------------------------------------------------------

void ImplFreeEventHookData()
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplEventHook*  pTempEventHookData;
    ImplEventHook*  pEventHookData = pSVData->maAppData.mpFirstEventHook;
    while ( pEventHookData )
    {
        pTempEventHookData = pEventHookData->mpNext;
        delete pEventHookData;
        pEventHookData = pTempEventHookData;
    }

    pSVData->maAppData.mpFirstEventHook = NULL;
}

// -----------------------------------------------------------------------

long Application::CallEventHooks( NotifyEvent& rEvt )
{
    ImplSVData*     pSVData = ImplGetSVData();
    long            nRet = 0;
    ImplEventHook*  pTempEventHookData;
    ImplEventHook*  pEventHookData = pSVData->maAppData.mpFirstEventHook;
    while ( pEventHookData )
    {
        pTempEventHookData = pEventHookData->mpNext;
        nRet = pEventHookData->mpProc( rEvt, pEventHookData->mpUserData );
        if ( nRet )
            break;
        pEventHookData = pTempEventHookData;
    }

    return nRet;
}

// -----------------------------------------------------------------------

const LocaleDataWrapper& Application::GetAppLocaleDataWrapper()
{
    return GetSettings().GetLocaleDataWrapper();
}

// -----------------------------------------------------------------------

void Application::EnableHeadlessMode( bool dialogsAreFatal )
{
    SetDialogCancelMode(
        dialogsAreFatal ? DIALOG_CANCEL_FATAL : DIALOG_CANCEL_SILENT );
}

// -----------------------------------------------------------------------

sal_Bool Application::IsHeadlessModeEnabled()
{
    return IsDialogCancelEnabled();
}

bool Application::IsHeadlessModeRequested()
{
    sal_uInt32 n = rtl_getAppCommandArgCount();
    for (sal_uInt32 i = 0; i < n; ++i) {
        OUString arg;
        rtl_getAppCommandArg(i, &arg.pData);
        if ( arg == "--headless" || arg == "-headless" ) {
            return true;
        }
    }
    return false;
}

static bool bConsoleOnly = false;

bool Application::IsConsoleOnly()
{
    return bConsoleOnly;
}

void Application::EnableConsoleOnly()
{
    bConsoleOnly = true;
}


// -----------------------------------------------------------------------

void Application::ShowNativeErrorBox(const OUString& sTitle  ,
                                     const OUString& sMessage)
{
    int btn = ImplGetSalSystem()->ShowNativeMessageBox (
            sTitle,
            sMessage,
            SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK,
            SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK, false);
    if (btn != SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK) {
        OSL_TRACE("ShowNativeMessageBox returned %d", btn);
    }
}

// -----------------------------------------------------------------------

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
    return SalGetDesktopEnvironment();
}

void Application::AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService)
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDefInst->AddToRecentDocumentList(rFileUrl, rMimeType, rDocumentService);
}

bool InitAccessBridge( bool bShowCancel, bool &rCancelled )
{
    bool bRet = true;

// Disable Java bridge on UNIX
#if defined UNX
    (void) bShowCancel; // unused
    (void) rCancelled; // unused
#else
    bRet = ImplInitAccessBridge( bShowCancel, rCancelled );

    if( !bRet && bShowCancel && !rCancelled )
    {
        // disable accessibility if the user chooses to continue
        AllSettings aSettings = Application::GetSettings();
        MiscSettings aMisc = aSettings.GetMiscSettings();
        aMisc.SetEnableATToolSupport( sal_False );
        aSettings.SetMiscSettings( aMisc );
        Application::SetSettings( aSettings );
    }
#endif // !UNX

    return bRet;
}

// MT: AppEvent was in oldsv.cxx, but is still needed...
// ------------------------------------------------------------------------

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
