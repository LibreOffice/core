/*************************************************************************
 *
 *  $RCSfile: svapp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:35 $
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

#define _SV_APP_CXX

#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#else
#include "rvp.hxx"
#include <rmwindow.hxx>
#include <rmevents.hxx>
#include <vos/thread.hxx>
#include <unobrok.hxx>
#ifndef _SV_MSGBOX_HXX
#include <msgbox.hxx>
#endif
#endif

#ifndef _VOS_PROCESS_HXX
#include <vos/process.hxx>
#endif
#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif

#if defined( WIN ) || defined( WNT ) || defined( OS2 )
#ifndef _DLL_HXX
#include <tools/dll.hxx>
#endif
#endif
#ifndef _TOOLS_H
#include <tools/tools.h>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif
#ifndef _ACCMGR_HXX
#include <accmgr.hxx>
#endif
#ifndef _SV_KEYCOD_HXX
#include <keycod.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_WINDATA_HXX
#include <windata.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_IDLEMGR_HXX
#include <idlemgr.hxx>
#endif
#ifndef _SV_DRAG_HXX
#include <drag.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_CVTGRF_HXX
#include <cvtgrf.hxx>
#endif
#ifndef _SV_ACCESS_HXX
#include <access.hxx>
#endif
#ifndef _VCL_UNOWRAP_HXX
#include <unowrap.hxx>
#endif

#include <com/sun/star/awt/XToolkit.hpp>

// #include <usr/refl.hxx>
class Reflection;

#pragma hdrstop

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
    ImplInitSVData();
    ImplGetSVData()->mpApp = this;
#ifndef REMOTE_APPSERVER
    InitSalData();
#endif
}

// -----------------------------------------------------------------------

Application::~Application()
{
    ImplDeInitSVData();
#ifndef REMOTE_APPSERVER
    DeInitSalData();
#endif
    ImplGetSVData()->mpApp = NULL;
    ImplDestroySVData();
    GlobalDeInitTools();
}

// -----------------------------------------------------------------------

void Application::InitAppRes( const ResId& rResId )
{
}

// -----------------------------------------------------------------------

BOOL Application::QueryExit()
{
    WorkWindow* pAppWin = ImplGetSVData()->maWinData.mpAppWin;

    // Aufruf des Close-Handlers des Applikationsfensters
    if ( pAppWin )
        return pAppWin->Close();
    else
        return TRUE;
}

// -----------------------------------------------------------------------

void Application::UserEvent( ULONG, void* )
{
}

// -----------------------------------------------------------------------

void Application::ShowStatusText( const XubString& )
{
}

// -----------------------------------------------------------------------

void Application::ShowHelpStatusText( const XubString& )
{
}

// -----------------------------------------------------------------------

void Application::ActivateExtHelp()
{
}

// -----------------------------------------------------------------------

void Application::DeactivateExtHelp()
{
}

// -----------------------------------------------------------------------

void Application::HideStatusText()
{
}

// -----------------------------------------------------------------------

void Application::HideHelpStatusText()
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

USHORT Application::GetCommandLineParamCount()
{
    NAMESPACE_VOS( OStartupInfo ) aStartInfo;
    return (USHORT)aStartInfo.getCommandArgCount();
}

// -----------------------------------------------------------------------

XubString Application::GetCommandLineParam( USHORT nParam )
{
    NAMESPACE_VOS( OStartupInfo )   aStartInfo;
    NAMESPACE_RTL( OUString )       aParam;
    aStartInfo.getCommandArg( nParam, aParam );
    return XubString( aParam );
}

// -----------------------------------------------------------------------

const XubString& Application::GetAppFileName()
{
    ImplSVData* pSVData = ImplGetSVData();
    DBG_ASSERT( pSVData->maAppData.mpAppFileName, "AppFileName vor SVMain ?!" );
    if ( pSVData->maAppData.mpAppFileName )
        return *pSVData->maAppData.mpAppFileName;
    return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

USHORT Application::Exception( USHORT nError )
{
    switch ( nError & EXC_MAJORTYPE )
    {
        // Bei System machen wir nichts und lassen dem System den
        // vortritt
        case EXC_SYSTEM:
            return 0;

        case EXC_DISPLAY:
        case EXC_REMOTE:
            return 0;

#ifdef DBG_UTIL
        case EXC_RSCNOTLOADED:
            Abort( XubString( RTL_CONSTASCII_USTRINGPARAM( "Resource not loaded" ) ) );
            break;
        case EXC_SYSOBJNOTCREATED:
            Abort( XubString( RTL_CONSTASCII_USTRINGPARAM( "System Object not created" ) ) );
            break;
        default:
            Abort( XubString( RTL_CONSTASCII_USTRINGPARAM( "Unknown Error" ) ) );
            break;
#else
        default:
            Abort( ImplGetSVEmptyStr() );
            break;
#endif
    }

    return 0;
}

// -----------------------------------------------------------------------

void Application::Abort( const XubString& rErrorText )
{
#ifndef REMOTE_APPSERVER
    SalAbort( rErrorText );
#else
    ErrorBox aErrorBox( NULL, WB_OK, rErrorText );
    aErrorBox.Execute();
    exit(-1);
#endif
}

// -----------------------------------------------------------------------

#ifdef REMOTE_APPSERVER

ImplRemoteYieldMutex::ImplRemoteYieldMutex()
{
    mnCount = 0;
    mnMainThreadId  = NAMESPACE_VOS(OThread)::getCurrentIdentifier();
    mnThreadId      = 0;
}

void SAL_CALL ImplRemoteYieldMutex::acquire()
{
    OMutex::acquire();
    mnThreadId = NAMESPACE_VOS(OThread)::getCurrentIdentifier();
    mnCount++;
}

void SAL_CALL ImplRemoteYieldMutex::release()
{
    if ( mnThreadId == NAMESPACE_VOS(OThread)::getCurrentIdentifier() )
    {
        if ( mnCount == 1 )
            mnThreadId = 0;
        mnCount--;
    }
    OMutex::release();
}

sal_Bool SAL_CALL ImplRemoteYieldMutex::tryToAcquire()
{
    if ( OMutex::tryToAcquire() )
    {
        mnThreadId = NAMESPACE_VOS(OThread)::getCurrentIdentifier();
        mnCount++;
        return True;
    }
    else
        return False;
}

// -----------------------------------------------------------------------

#ifdef DBG_UTIL

void ImplDbgTestSolarMutex()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maAppData.mpSolarMutex->GetMainThreadId() !=
         NAMESPACE_VOS(OThread)::getCurrentIdentifier() )
    {
        if ( pSVData->maAppData.mpSolarMutex->GetThreadId() !=
             NAMESPACE_VOS(OThread)::getCurrentIdentifier() )
        {
            DBG_ERROR( "SolarMutex not locked, and not thread save code in VCL is called from outside of the main thread" );
        }
    }
    else
    {
        if ( pSVData->maAppData.mpSolarMutex->GetThreadId() !=
             NAMESPACE_VOS(OThread)::getCurrentIdentifier() )
        {
            DBG_ERROR( "SolarMutex not locked in the main thread" );
        }
    }
}

#endif


NAMESPACE_VOS(OThreadData)* getThreadLocalEnvironment();

static void ImplRemoteDispatch( BOOL bWait )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Yield-Semaphore freigeben
    ULONG nAcquireCount;
    ULONG i;
    if ( pSVData->maAppData.mpSolarMutex->GetThreadId() ==
         ::vos::OThread::getCurrentIdentifier() )
    {
        nAcquireCount = pSVData->maAppData.mpSolarMutex->GetAcquireCount();
        for ( i = 0; i < nAcquireCount; i++ )
            pSVData->maAppData.mpSolarMutex->release();
    }
    else
        nAcquireCount = 0;

    RmEvent* pEvent = pSVData->mpRmEventQueue->GetNextEvent( bWait );

    // Yield-Semaphore wieder holen
    while ( nAcquireCount )
    {
        pSVData->maAppData.mpSolarMutex->acquire();
        nAcquireCount--;
    }

    if ( pEvent )
        ImplDispatchEvent( (ExtRmEvent*)pEvent );
    else
        ::vos::OThread::yield();
}

#endif

// -----------------------------------------------------------------------

void Application::Execute()
{
    DBG_STARTAPPEXECUTE();

    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mbInAppExecute = TRUE;

    while ( !pSVData->maAppData.mbAppQuit )
        Application::Yield();

    pSVData->maAppData.mbInAppExecute = FALSE;

    DBG_ENDAPPEXECUTE();
}

// -----------------------------------------------------------------------

void Application::Reschedule()
{
    ImplSVData* pSVData = ImplGetSVData();

    // Restliche Timer abarbeitet
    while ( pSVData->mbNotAllTimerCalled )
        ImplTimerCallbackProc();

    pSVData->maAppData.mnDispatchLevel++;
#ifndef REMOTE_APPSERVER
    pSVData->mpDefInst->Yield( FALSE );
#else
    ImplRemoteDispatch( FALSE );
#endif
    pSVData->maAppData.mnDispatchLevel--;
}

// -----------------------------------------------------------------------

void Application::Yield()
{
    ImplSVData* pSVData = ImplGetSVData();

    // Restliche Timer abarbeitet
    while ( pSVData->mbNotAllTimerCalled )
        ImplTimerCallbackProc();

    // Wenn Application schon beendet wurde, warten wir nicht mehr auf
    // Messages, sondern verarbeiten nur noch welche, wenn noch welche
    // vorliegen
    pSVData->maAppData.mnDispatchLevel++;
#ifndef REMOTE_APPSERVER
    pSVData->mpDefInst->Yield( !pSVData->maAppData.mbAppQuit );
#else
    ImplRemoteDispatch( TRUE );
#endif
    pSVData->maAppData.mnDispatchLevel--;
}

// -----------------------------------------------------------------------

IMPL_STATIC_LINK( ImplSVAppData, ImplQuitMsg, void*, EMPTYARG )
{
    ImplGetSVData()->maAppData.mbAppQuit = TRUE;
    return 0;
}

// -----------------------------------------------------------------------

void Application::Quit()
{
    Application::PostUserEvent( STATIC_LINK( NULL, ImplSVAppData, ImplQuitMsg ) );
}

// -----------------------------------------------------------------------

#ifdef _VOS_NO_NAMESPACE
IMutex& Application::GetSolarMutex()
#else
vos::IMutex& Application::GetSolarMutex()
#endif
{
#ifndef REMOTE_APPSERVER
    ImplSVData* pSVData = ImplGetSVData();
    return *(pSVData->mpDefInst->GetYieldMutex());
#else
    return *(ImplGetSVData()->maAppData.mpSolarMutex);
#endif
}

// -----------------------------------------------------------------------

#ifdef _VOS_NO_NAMESPACE
OThread::TThreadIdentifier Application::GetMainThreadIdentifier()
#else
vos::OThread::TThreadIdentifier Application::GetMainThreadIdentifier()
#endif
{
    return ImplGetSVData()->mnMainThreadId;
}

// -----------------------------------------------------------------------

ULONG Application::ReleaseSolarMutex()
{
#ifndef REMOTE_APPSERVER
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->mpDefInst->ReleaseYieldMutex();
#else
    ImplSVData* pSVData = ImplGetSVData();

    // Wenn wir gelockt haben, dann freigeben
    if ( pSVData->maAppData.mpSolarMutex->GetThreadId() ==
         NAMESPACE_VOS(OThread)::getCurrentIdentifier() )
    {
        ULONG nCount = pSVData->maAppData.mpSolarMutex->GetAcquireCount();
        ULONG n = nCount;
        while ( n )
        {
            pSVData->maAppData.mpSolarMutex->release();
            n--;
        }

        return nCount;
    }
    else
        return 0;
#endif
}

// -----------------------------------------------------------------------

void Application::AcquireSolarMutex( ULONG nCount )
{
#ifndef REMOTE_APPSERVER
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDefInst->AcquireYieldMutex( nCount );
#else
    ImplSVData* pSVData = ImplGetSVData();
    while ( nCount )
    {
        pSVData->maAppData.mpSolarMutex->acquire();
        nCount--;
    }
#endif
}

// -----------------------------------------------------------------------

BOOL Application::IsInMain()
{
    return ImplGetSVData()->maAppData.mbInAppMain;
}

// -----------------------------------------------------------------------

BOOL Application::IsInExecute()
{
    return ImplGetSVData()->maAppData.mbInAppExecute;
}

// -----------------------------------------------------------------------

BOOL Application::IsShutDown()
{
    return ImplGetSVData()->maAppData.mbAppQuit;
}

// -----------------------------------------------------------------------

BOOL Application::IsInModalMode()
{
    return (ImplGetSVData()->maAppData.mnModalMode != 0);
}

// -----------------------------------------------------------------------

USHORT Application::GetDispatchLevel()
{
    return ImplGetSVData()->maAppData.mnDispatchLevel;
}

// -----------------------------------------------------------------------

BOOL Application::AnyInput( USHORT nType )
{
#ifndef REMOTE_APPSERVER
    return SalInstance::AnyInput( nType );
#else
    ImplSVData* pSVData = ImplGetSVData();

    if( ( nType & ( INPUT_ANY ) ) == ( INPUT_ANY ) )
    {
        return( pSVData->mpRmEventQueue->HasMouseEvent() ||
                pSVData->mpRmEventQueue->HasKeyEvent() ||
                pSVData->mpRmEventQueue->HasPaintEvent() ||
                pSVData->mpRmEventQueue->HasTimerEvent() ||
                pSVData->mpRmEventQueue->HasOtherEvent() );
    }
    else if( ( nType & ( INPUT_MOUSEANDKEYBOARD ) ) == ( INPUT_MOUSEANDKEYBOARD ) )
    {
        return( pSVData->mpRmEventQueue->HasMouseEvent() ||
                pSVData->mpRmEventQueue->HasKeyEvent() );
    }
    else
    {
        if( nType & INPUT_MOUSE )
            return pSVData->mpRmEventQueue->HasMouseEvent();

        if( nType & INPUT_KEYBOARD )
            return pSVData->mpRmEventQueue->HasKeyEvent();

        if( nType & INPUT_PAINT )
            return pSVData->mpRmEventQueue->HasPaintEvent();

        if( nType & INPUT_TIMER )
            return pSVData->mpRmEventQueue->HasTimerEvent();

        if( nType & INPUT_OTHER )
            return pSVData->mpRmEventQueue->HasOtherEvent();
    }

    return FALSE;
#endif
}

// -----------------------------------------------------------------------

ULONG Application::GetLastInputInterval()
{
    return (Time::GetSystemTicks()-ImplGetSVData()->maAppData.mnLastInputTime);
}

// -----------------------------------------------------------------------

extern int nImplSysDialog;

BOOL Application::IsUICaptured()
{
    ImplSVData* pSVData = ImplGetSVData();
    // Wenn Mouse gecaptured, oder im TrackingModus oder im Auswahlmodus
    // eines FloatingWindows (wie Menus, Aufklapp-ToolBoxen) soll kein
    // weiteres Fenster aufgezogen werden
    if ( pSVData->maWinData.mpCaptureWin || pSVData->maWinData.mpTrackWin ||
         pSVData->maWinData.mpFirstFloat || DragManager::GetDragManager() ||
         nImplSysDialog )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

BOOL Application::IsUserActive( USHORT nTest )
{
    if ( nTest & (USERACTIVE_MOUSEDRAG | USERACTIVE_INPUT) )
    {
        if ( IsUICaptured() )
            return TRUE;
    }

    if ( nTest & USERACTIVE_INPUT )
    {
        if ( GetLastInputInterval() < 500 )
            return TRUE;

        if ( AnyInput( INPUT_KEYBOARD ) )
            return TRUE;
    }

    if ( nTest & USERACTIVE_MODALDIALOG )
    {
        if ( ImplGetSVData()->maAppData.mnModalDialog )
            return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void Application::SystemSettingsChanging( AllSettings& rSettings,
                                          Window* pFrame )
{
}

// -----------------------------------------------------------------------

void Application::MergeSystemSettings( AllSettings& rSettings )
{
#ifndef REMOTE_APPSERVER
    ImplGetDefaultWindow()->ImplGetFrame()->UpdateSettings( rSettings );
#endif
}

// -----------------------------------------------------------------------

void Application::SetSettings( const AllSettings& rSettings )
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maAppData.mpSettings )
    {
        pSVData->maAppData.mpSettings = new AllSettings();
        *pSVData->maAppData.mpSettings = rSettings;
    }
    else
    {
        AllSettings aOldSettings = *pSVData->maAppData.mpSettings;
        *pSVData->maAppData.mpSettings = rSettings;
        ULONG nChangeFlags = aOldSettings.GetChangeFlags( *pSVData->maAppData.mpSettings );
        if ( nChangeFlags )
        {
            DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &aOldSettings, nChangeFlags );
            GetpApp()->DataChanged( aDCEvt );

            // Update all windows
            Window* pFirstFrame = pSVData->maWinData.mpFirstFrame;
            // Daten, die neu berechnet werden muessen, zuruecksetzen
            long nOldDPIX;
            long nOldDPIY;
            if ( pFirstFrame )
            {
                nOldDPIX = pFirstFrame->mnDPIX;
                nOldDPIY = pFirstFrame->mnDPIY;
                pSVData->maGDIData.mnAppFontX = 0;
            }
            Window* pFrame = pFirstFrame;
            while ( pFrame )
            {
                // AppFont-Cache-Daten zuruecksetzen
                pFrame->mpFrameData->meMapUnit = MAP_PIXEL;

                // UpdateSettings am ClientWindow aufrufen, damit
                // die Daten nicht doppelt geupdatet werden
                Window* pClientWin = pFrame;
                while ( pClientWin->ImplGetClientWindow() )
                    pClientWin = pClientWin->ImplGetClientWindow();
                pClientWin->UpdateSettings( rSettings, TRUE );

                Window* pTempWin = pFrame->mpFrameData->mpFirstOverlap;
                while ( pTempWin )
                {
                    // UpdateSettings am ClientWindow aufrufen, damit
                    // die Daten nicht doppelt geupdatet werden
                    pClientWin = pTempWin;
                    while ( pClientWin->ImplGetClientWindow() )
                        pClientWin = pClientWin->ImplGetClientWindow();
                    pClientWin->UpdateSettings( rSettings, TRUE );
                    pTempWin = pTempWin->mpNextOverlap;
                }

                pFrame = pFrame->mpFrameData->mpNextFrame;
            }

            // Wenn sich die DPI-Aufloesung fuer Screen-Ausgaben
            // geaendert hat, setzen wir auch bei allen
            // Screen-Kompatiblen VirDev's die neue Aufloesung
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
        pSVData->maAppData.mpSettings = new AllSettings();
    return *(pSVData->maAppData.mpSettings);
}

// -----------------------------------------------------------------------

void Application::NotifyAllWindows( DataChangedEvent& rDCEvt )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pFrame = pSVData->maWinData.mpFirstFrame;
    while ( pFrame )
    {
        pFrame->NotifyAllChilds( rDCEvt );

        Window* pSysWin = pFrame->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            pSysWin->NotifyAllChilds( rDCEvt );
            pSysWin = pSysWin->mpNextOverlap;
        }

        pFrame = pFrame->mpFrameData->mpNextFrame;
    }
}

// -----------------------------------------------------------------------

ULONG Application::PostUserEvent( ULONG nEvent, void* pEventData )
{
    ULONG nEventId;
    PostUserEvent( nEventId, nEvent, pEventData );
    return nEventId;
}

// -----------------------------------------------------------------------

ULONG Application::PostUserEvent( const Link& rLink, void* pCaller )
{
    ULONG nEventId;
    PostUserEvent( nEventId, rLink, pCaller );
    return nEventId;
}

// -----------------------------------------------------------------------

BOOL Application::PostUserEvent( ULONG& rEventId, ULONG nEvent, void* pEventData )
{
    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mnEvent   = nEvent;
    pSVEvent->mpData    = pEventData;
    pSVEvent->mpLink    = NULL;
    pSVEvent->mpWindow  = NULL;
    pSVEvent->mbCall    = TRUE;
    rEventId = (ULONG)pSVEvent;
#ifndef REMOTE_APPSERVER
    if ( ImplGetDefaultWindow()->ImplGetFrame()->PostEvent( pSVEvent ) )
        return TRUE;
    else
    {
        rEventId = 0;
        delete pSVEvent;
        return FALSE;
    }
#else
    ExtRmEvent* pEvt = new ExtRmEvent( RMEVENT_USEREVENT, NULL, pSVEvent );
    ImplPostEvent( pEvt );
    return TRUE;
#endif
}

// -----------------------------------------------------------------------

BOOL Application::PostUserEvent( ULONG& rEventId, const Link& rLink, void* pCaller )
{
    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mnEvent   = 0;
    pSVEvent->mpData    = pCaller;
    pSVEvent->mpLink    = new Link( rLink );
    pSVEvent->mpWindow  = NULL;
    pSVEvent->mbCall    = TRUE;
    rEventId = (ULONG)pSVEvent;
#ifndef REMOTE_APPSERVER
    if ( ImplGetDefaultWindow()->ImplGetFrame()->PostEvent( pSVEvent ) )
        return TRUE;
    else
    {
        rEventId = 0;
        delete pSVEvent;
        return FALSE;
    }
#else
    ExtRmEvent* pEvt = new ExtRmEvent( RMEVENT_USEREVENT, NULL, pSVEvent );
    ImplPostEvent( pEvt );
    return TRUE;
#endif
}

// -----------------------------------------------------------------------

void Application::RemoveUserEvent( ULONG nUserEvent )
{
    ImplSVEvent* pSVEvent = (ImplSVEvent*)nUserEvent;

    DBG_ASSERT( !pSVEvent->mpWindow,
                "Application::RemoveUserEvent(): Event is send to a window" );
    DBG_ASSERT( pSVEvent->mbCall,
                "Application::RemoveUserEvent(): Event is already removed" );

    if ( pSVEvent->mpWindow )
    {
        pSVEvent->mpWindow->ImplRemoveDel( &(pSVEvent->maDelData) );
        pSVEvent->mpWindow = NULL;
    }

    pSVEvent->mbCall = FALSE;
}

// -----------------------------------------------------------------------

BOOL Application::InsertIdleHdl( const Link& rLink, USHORT nPrio )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Falls er noch nicht existiert, dann anlegen
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
    return pWindow->mpFrameData->mpNextFrame;
}

// -----------------------------------------------------------------------

void Application::SetAppName( const XubString& rUniqueName )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Falls er noch nicht existiert, dann anlegen
    if ( !pSVData->maAppData.mpAppName )
        pSVData->maAppData.mpAppName = new XubString( rUniqueName );
    else
        *(pSVData->maAppData.mpAppName) = rUniqueName;
}

// -----------------------------------------------------------------------

XubString Application::GetAppName()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maAppData.mpAppName )
        return *(pSVData->maAppData.mpAppName);
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void Application::SetDisplayName( const UniString& rName )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Falls er noch nicht existiert, dann anlegen
    if ( !pSVData->maAppData.mpDisplayName )
        pSVData->maAppData.mpDisplayName = new UniString( rName );
    else
        *(pSVData->maAppData.mpDisplayName) = rName;
}

// -----------------------------------------------------------------------

UniString Application::GetDisplayName()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maAppData.mpDisplayName )
        return *(pSVData->maAppData.mpDisplayName);
    else if ( pSVData->maWinData.mpAppWin )
        return pSVData->maWinData.mpAppWin->GetText();
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

BOOL Application::InsertAccel( Accelerator* pAccel )
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

void Application::FlushAccel()
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maAppData.mpAccelMgr )
        pSVData->maAppData.mpAccelMgr->FlushAccel();
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

void Application::EnableAutoHelpId( BOOL bEnabled )
{
    ImplGetSVData()->maHelpData.mbAutoHelpId = bEnabled;
}

// -----------------------------------------------------------------------

BOOL Application::IsAutoHelpIdEnabled()
{
    return ImplGetSVData()->maHelpData.mbAutoHelpId;
}

// -----------------------------------------------------------------------

void Application::EnableAutoMnemonic( BOOL bEnabled )
{
    ImplGetSVData()->maAppData.mbAutoMnemonics = bEnabled;
}

// -----------------------------------------------------------------------

BOOL Application::IsAutoMnemonicEnabled()
{
    return ImplGetSVData()->maAppData.mbAutoMnemonics;
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

short Application::GetDialogScaleX()
{
    return ImplGetSVData()->maAppData.mnDialogScaleX;
}

// -----------------------------------------------------------------------

void Application::SetDefDialogParent( Window* pWindow )
{
    ImplGetSVData()->maWinData.mpDefDialogParent = pWindow;
}

// -----------------------------------------------------------------------

Window* Application::GetDefDialogParent()
{
    return ImplGetSVData()->maWinData.mpDefDialogParent;
}

// -----------------------------------------------------------------------

void Application::EnableDialogCancel( BOOL bDialogCancel )
{
    ImplGetSVData()->maAppData.mbDialogCancel = bDialogCancel;
}

// -----------------------------------------------------------------------

BOOL Application::IsDialogCancelEnabled()
{
    return ImplGetSVData()->maAppData.mbDialogCancel;
}

// -----------------------------------------------------------------------

void Application::SetSystemWindowMode( USHORT nMode )
{
    ImplGetSVData()->maAppData.mnSysWinMode = nMode;
}

// -----------------------------------------------------------------------

USHORT Application::GetSystemWindowMode()
{
    return ImplGetSVData()->maAppData.mnSysWinMode;
}

// -----------------------------------------------------------------------

void Application::SetResourcePath( const XubString& rPath )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Falls er noch nicht existiert, dann anlegen
    if ( !pSVData->maAppData.mpResPath )
        pSVData->maAppData.mpResPath = new XubString( rPath );
    else
        *(pSVData->maAppData.mpResPath) = rPath;
}

// -----------------------------------------------------------------------

const XubString& Application::GetResourcePath()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maAppData.mpResPath )
        return *(pSVData->maAppData.mpResPath);
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void Application::EnterMultiThread( BOOL bEnter )
{
    ImplSVData* pSVData = ImplGetSVData();

#if defined( WIN ) || defined( WNT ) || defined( OS2 )
    ::EnterMultiThread( bEnter );
#endif

    if ( bEnter )
    {
        pSVData->mnThreadCount++;

#ifndef REMOTE_APPSERVER
        // Unser DefaultWindow muss vor einem Thread-Starten erzeugt werden,
        // damit dieses im Hauptthread laeuft
        ImplGetDefaultWindow();
#endif
    }
    else
        pSVData->mnThreadCount--;
}

// -----------------------------------------------------------------------

BOOL Application::IsMultiThread()
{
    return (ImplGetSVData()->mnThreadCount != 0);
}

// -----------------------------------------------------------------------

UniqueItemId Application::CreateUniqueId()
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( !pSVData->maAppData.mpUniqueIdCont )
        pSVData->maAppData.mpUniqueIdCont = new UniqueIdContainer( UNIQUEID_SV_BEGIN );
    return pSVData->maAppData.mpUniqueIdCont->CreateId();
}

// -----------------------------------------------------------------------

SystemInfoType Application::GetClientSystem()
{
#ifndef REMOTE_APPSERVER
    return GetServerSystem();
#else
    static SystemInfoType nImplClientSystemInfo = 0;
    if ( !nImplClientSystemInfo )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if ( pSVData->mxStatus.is() )
            nImplClientSystemInfo = (SystemInfoType)pSVData->mxStatus->GetSystemType();
    }

    return nImplClientSystemInfo;
#endif
}

// -----------------------------------------------------------------------

SystemInfoType Application::GetServerSystem()
{
#if defined( WIN )
    return SYSTEMINFO_SYSTEM_WINDOWS | SYSTEMINFO_SYSTEMBASE_DOS;
#elif defined( WNT )
    return SYSTEMINFO_SYSTEM_WINDOWS | SYSTEMINFO_SYSTEMBASE_NT;
#elif defined( OS2 )
    return SYSTEMINFO_SYSTEM_OS2;
#elif defined( MAC )
    return SYSTEMINFO_SYSTEM_MAC;
#elif defined( UNX )
#if defined( LINUX )
    return SYSTEMINFO_SYSTEM_UNIX | SYSTEMINFO_SYSTEMBASE_LINUX;
#elif defined( SOLARIS )
    return SYSTEMINFO_SYSTEM_UNIX | SYSTEMINFO_SYSTEMBASE_SOLARIS;
#elif defined( SCO )
    return SYSTEMINFO_SYSTEM_UNIX | SYSTEMINFO_SYSTEMBASE_SCO;
#elif defined( NETBSD )
    return SYSTEMINFO_SYSTEM_UNIX | SYSTEMINFO_SYSTEMBASE_NETBSD;
#elif defined( AIX )
    return SYSTEMINFO_SYSTEM_UNIX | SYSTEMINFO_SYSTEMBASE_AIX;
#elif defined( IRIX )
    return SYSTEMINFO_SYSTEM_UNIX | SYSTEMINFO_SYSTEMBASE_IRIX;
#elif defined( HPUX )
    return SYSTEMINFO_SYSTEM_UNIX | SYSTEMINFO_SYSTEMBASE_HPUX;
#elif defined( FREEBSD )
    return SYSTEMINFO_SYSTEM_UNIX | SYSTEMINFO_SYSTEMBASE_FREEBSD;
#elif defined( MACOSX )
    return SYSTEMINFO_SYSTEM_UNIX | SYSTEMINFO_SYSTEMBASE_MACOSX;
#else
#error Unknown Unix-System, new SystemBase must be defined!
#endif
#else
#error Unknown System, new System must be defined!
#endif
}

// -----------------------------------------------------------------------

BOOL Application::IsRemoteServer()
{
#ifndef REMOTE_APPSERVER
    return FALSE;
#else
    return TRUE;
#endif
}

// -----------------------------------------------------------------------

void* Application::GetRemoteEnvironment()
{
#ifndef REMOTE_APPSERVER
    return NULL;
#else
    return ImplGetSVData()->mhRemoteEnv;
#endif
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > Application::GetVCLToolkit()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > xT;
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
    if ( pWrapper )
        xT = pWrapper->GetVCLToolkit();
    return xT;
}

// -----------------------------------------------------------------------

void Application::RegisterUnoServices()
{
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
    if ( pWrapper )
        pWrapper->RegisterUnoServices();
}

// -----------------------------------------------------------------------

void Application::SetUnoWrapper( UnoWrapperBase* pWrapper )
{
    ImplSVData* pSVData = ImplGetSVData();
    DBG_ASSERT( !pSVData->mpUnoWrapper, "SetUnoWrapper: Wrapper allready exists" );
    pSVData->mpUnoWrapper = pWrapper;
}

// -----------------------------------------------------------------------

UnoWrapperBase* Application::GetUnoWrapper()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->mpUnoWrapper;
}

// -----------------------------------------------------------------------

void Application::SetFilterHdl( const Link& rLink )
{
    ImplGetSVData()->maGDIData.mpGrfConverter->SetFilterHdl( rLink );
}

// -----------------------------------------------------------------------

const Link& Application::GetFilterHdl()
{
    return ImplGetSVData()->maGDIData.mpGrfConverter->GetFilterHdl();
}

// -----------------------------------------------------------------------

void Application::AccessNotify( const AccessNotification& rNotification )
{
    GetFirstAccessHdl().Call( (void*) &rNotification );
}

// -----------------------------------------------------------------------

BOOL Application::GenerateAccessEvent( ULONG nAccessEvent,
                                       long nData1,
                                       long nData2,
                                       long nData3 )
{
    BOOL bRet = FALSE;

    switch( nAccessEvent )
    {
        case( ACCESS_EVENT_DLGCONTROLS ):
        {
            if( IsInModalMode() )
            {
                Window* pDlgWin = GetFocusWindow();
                BOOL    bFound = FALSE;

                // find modal dialog
                while( pDlgWin && !bFound )
                {
                    switch( pDlgWin->GetType() )
                    {
                        case( WINDOW_MESSBOX ):
                        case( WINDOW_INFOBOX ):
                        case( WINDOW_WARNINGBOX ):
                        case( WINDOW_ERRORBOX ):
                        case( WINDOW_QUERYBOX ):
                        case( WINDOW_MODALDIALOG ):
                        case( WINDOW_PATHDIALOG ):
                        case( WINDOW_FILEDIALOG ):
                        case( WINDOW_PRINTERSETUPDIALOG ):
                        case( WINDOW_PRINTDIALOG ):
                        case( WINDOW_COLORDIALOG ):
                        case( WINDOW_FONTDIALOG ):
                        case( WINDOW_TABDIALOG ):
                        case( WINDOW_BUTTONDIALOG ):
                            bFound = TRUE;
                        break;

                        default:
                            pDlgWin = pDlgWin->GetWindow( WINDOW_PARENT );
                        break;
                    }
                }

                if( pDlgWin )
                {
                    AccessNotify( AccessNotification( ACCESS_EVENT_DLGCONTROLS, pDlgWin ) );
                    bRet = TRUE;
                }
            }
        }
        break;

        case( ACCESS_EVENT_KEY ):
            AccessNotify( AccessNotification( ACCESS_EVENT_KEY, nData1, nData2, nData3 ) );
        break;

        default:
        break;
    }

    return bRet;
}

// -----------------------------------------------------------------------

void Application::AddAccessHdl( const Link& rLink )
{
    if( !ImplGetSVData()->maAppData.mpAccessList )
        ImplGetSVData()->maAppData.mpAccessList = new List;

    List* pList = ImplGetSVData()->maAppData.mpAccessList;
    BOOL  bInserted = FALSE;

    for( void* pLink = pList->First(); pLink; pLink = pList->Next() )
    {
        if( *(Link*) pLink == rLink )
        {
            bInserted = TRUE;
            break;
        }
    }

    if( !bInserted )
    {
        ImplGetSVData()->maAppData.mnAccessCount++;
        pList->Insert( new Link( rLink ), LIST_APPEND );
    }
}

// -----------------------------------------------------------------------

void Application::RemoveAccessHdl( const Link& rLink )
{
    List* pList = ImplGetSVData()->maAppData.mpAccessList;

    if( pList )
    {
        for( void* pLink = pList->First(); pLink; pLink = pList->Next() )
        {
            if( *(Link*) pLink == rLink )
            {
                ImplGetSVData()->maAppData.mnAccessCount--;
                delete (Link*) pList->Remove( pLink );
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------

USHORT Application::GetAccessHdlCount()
{
    return ImplGetSVData()->maAppData.mnAccessCount;
}

// -----------------------------------------------------------------------

Link Application::GetFirstAccessHdl()
{
    List* pList = ImplGetSVData()->maAppData.mpAccessList;

    if( pList && pList->Count() )
        return *(Link*) pList->First();
    else
        return Link();
}

// -----------------------------------------------------------------------

void Application::CallNextAccessHdl( AccessNotification* pData )
{
    List* pList = ImplGetSVData()->maAppData.mpAccessList;

    if( pList )
    {
        Link* pNext = (Link*) pList->Next();

        if( pNext )
            pNext->Call( pData );
    }
}

// -----------------------------------------------------------------------

BOOL ImplCallHotKey( const KeyCode& rKeyCode )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplHotKey*     pHotKeyData = pSVData->maAppData.mpFirstHotKey;
    while ( pHotKeyData )
    {
        if ( pHotKeyData->maKeyCode.IsDefinedKeyCodeEqual( rKeyCode ) )
        {
            pHotKeyData->maLink.Call( pHotKeyData->mpUserData );
            return TRUE;
        }

        pHotKeyData = pHotKeyData->mpNext;
    }

    return FALSE;
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

ULONG Application::AddHotKey( const KeyCode& rKeyCode, const Link& rLink, void* pData )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplHotKey*     pHotKeyData = new ImplHotKey;
    pHotKeyData->mpUserData = pData;
    pHotKeyData->maKeyCode  = rKeyCode;
    pHotKeyData->maLink     = rLink;
    pHotKeyData->mpNext     = pSVData->maAppData.mpFirstHotKey;
    pSVData->maAppData.mpFirstHotKey = pHotKeyData;
    return (ULONG)pHotKeyData;
}

// -----------------------------------------------------------------------

void Application::RemoveHotKey( ULONG nId )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplHotKey*     pFindHotKeyData = (ImplHotKey*)nId;
    ImplHotKey*     pPrevHotKeyData = NULL;
    ImplHotKey*     pHotKeyData = pSVData->maAppData.mpFirstHotKey;
    while ( pHotKeyData )
    {
        if ( pHotKeyData == pFindHotKeyData )
        {
            if ( pPrevHotKeyData )
                pPrevHotKeyData->mpNext = pFindHotKeyData->mpNext;
            else
                pSVData->maAppData.mpFirstHotKey = pFindHotKeyData->mpNext;
            delete pFindHotKeyData;
            break;
        }

        pPrevHotKeyData = pHotKeyData;
        pHotKeyData = pHotKeyData->mpNext;
    }

    DBG_ASSERT( pHotKeyData, "Application::RemoveHotKey() - HotKey is not added" );
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

ULONG Application::AddEventHook( VCLEventHookProc pProc, void* pData )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplEventHook*  pEventHookData = new ImplEventHook;
    pEventHookData->mpUserData = pData;
    pEventHookData->mpProc     = pProc;
    pEventHookData->mpNext     = pSVData->maAppData.mpFirstEventHook;
    pSVData->maAppData.mpFirstEventHook = pEventHookData;
    return (ULONG)pEventHookData;
}

// -----------------------------------------------------------------------

void Application::RemoveEventHook( ULONG nId )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplEventHook*  pFindEventHookData = (ImplEventHook*)nId;
    ImplEventHook*  pPrevEventHookData = NULL;
    ImplEventHook*  pEventHookData = pSVData->maAppData.mpFirstEventHook;
    while ( pEventHookData )
    {
        if ( pEventHookData == pFindEventHookData )
        {
            if ( pPrevEventHookData )
                pPrevEventHookData->mpNext = pFindEventHookData->mpNext;
            else
                pSVData->maAppData.mpFirstEventHook = pFindEventHookData->mpNext;
            delete pFindEventHookData;
            break;
        }

        pPrevEventHookData = pEventHookData;
        pEventHookData = pEventHookData->mpNext;
    }

    DBG_ASSERT( pEventHookData, "Application::RemoveEventHook() - EventHook is not added" );
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

long Application::CallPreNotify( NotifyEvent& rEvt )
{
    return ImplCallPreNotify( rEvt );
}

// -----------------------------------------------------------------------

long Application::CallEvent( NotifyEvent& rEvt )
{
    return ImplCallEvent( rEvt );
}

// -----------------------------------------------------------------------

void Application::SetAppInternational( const International& rIntn )
{
    AllSettings aSettings = GetSettings();
    aSettings.SetInternational( rIntn );
    SetSettings( aSettings );
}

// -----------------------------------------------------------------------

const International& Application::GetAppInternational()
{
    return GetSettings().GetInternational();
}

// =======================================================================

void InitVCL()
{
}

// -----------------------------------------------------------------------

void DeInitVCL()
{
}
