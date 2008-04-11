/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svmain.cxx,v $
 * $Revision: 1.72 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifdef WNT
#include <tools/prewin.h>
#include <process.h>    // for _beginthreadex
#include <ole2.h>   // for _beginthreadex
#include <tools/postwin.h>
#endif

// [ed 5/14/02 Add in explicit check for quartz graphics.  OS X will define
// unx for both quartz and X11 graphics, but we include svunx.h only if we're
// building X11 graphics layers.

#if defined UNX && ! defined QUARTZ
#include <svunx.h>
#endif

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#include <vcl/salinst.hxx>
#include <vcl/salogl.hxx>
#include <vcl/salwtype.hxx>
#ifndef _VOS_SIGNAL_HXX
#include <vos/signal.hxx>
#endif
#ifndef _VOS_SOCKET_HXX
#include <vos/socket.hxx>
#endif
#include <tools/tools.h>
#include <tools/debug.hxx>
#ifndef _UNIQID_HXX
#include <tools/unqid.hxx>
#endif
#include <vcl/svdata.hxx>
#include <vcl/dbggui.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/image.hxx>
#ifndef _SV_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#include <vcl/accmgr.hxx>
#include <vcl/idlemgr.hxx>
#include <vcl/outdev.h>
#include <vcl/outfont.hxx>
#include <vcl/print.h>
#include <vcl/settings.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/salsys.hxx>
#include <vcl/saltimer.hxx>
#include <vcl/salimestatus.hxx>
#include <vcl/impimagetree.hxx>
#include <vcl/xconnection.hxx>

#include <vos/process.hxx>
#include <osl/file.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <rtl/logfile.hxx>

#if OSL_DEBUG_LEVEL > 0
#include <typeinfo>
#include <rtl/strbuf.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

#include <vcl/fontcfg.hxx>
#include <vcl/configsettings.hxx>
#include <cppuhelper/implbase1.hxx>
#include <uno/current_context.hxx>


// =======================================================================

class ImplVCLExceptionHandler : public ::vos::OSignalHandler
{
public:
    virtual ::vos::OSignalHandler::TSignalAction SAL_CALL signal( ::vos::OSignalHandler::TSignalInfo* pInfo );
};

// -----------------------------------------------------------------------

::vos::OSignalHandler::TSignalAction SAL_CALL ImplVCLExceptionHandler::signal( ::vos::OSignalHandler::TSignalInfo* pInfo )
{
    static BOOL bIn = FALSE;

    // Wenn wir nocheinmal abstuerzen, verabschieden wir uns gleich
    if ( !bIn )
    {
        USHORT nVCLException = 0;

        // UAE
        if ( (pInfo->Signal == osl_Signal_AccessViolation)     ||
             (pInfo->Signal == osl_Signal_IntegerDivideByZero) ||
             (pInfo->Signal == osl_Signal_FloatDivideByZero)   ||
             (pInfo->Signal == osl_Signal_DebugBreak) )
            nVCLException = EXC_SYSTEM;

        // RC
        if ((pInfo->Signal == osl_Signal_User) &&
            (pInfo->UserSignal == OSL_SIGNAL_USER_RESOURCEFAILURE) )
            nVCLException = EXC_RSCNOTLOADED;

        // DISPLAY-Unix
        if ((pInfo->Signal == osl_Signal_User) &&
            (pInfo->UserSignal == OSL_SIGNAL_USER_X11SUBSYSTEMERROR) )
            nVCLException = EXC_DISPLAY;

        // Remote-Client
        if ((pInfo->Signal == osl_Signal_User) &&
            (pInfo->UserSignal == OSL_SIGNAL_USER_RVPCONNECTIONERROR) )
            nVCLException = EXC_REMOTE;

        if ( nVCLException )
        {
            bIn = TRUE;

            ::vos::OGuard aLock(&Application::GetSolarMutex());

            // Timer nicht mehr anhalten, da ansonsten die UAE-Box
            // auch nicht mehr gepaintet wird
            ImplSVData* pSVData = ImplGetSVData();
            if ( pSVData->mpApp )
            {
                USHORT nOldMode = Application::GetSystemWindowMode();
                Application::SetSystemWindowMode( nOldMode & ~SYSTEMWINDOW_MODE_NOAUTOMODE );
                pSVData->mpApp->Exception( nVCLException );
                Application::SetSystemWindowMode( nOldMode );
            }
            bIn = FALSE;

            return vos::OSignalHandler::TAction_CallNextHandler;
        }
    }

    return vos::OSignalHandler::TAction_CallNextHandler;
}

// =======================================================================
BOOL ImplSVMain()
{
    // The 'real' SVMain()
    RTL_LOGFILE_CONTEXT( aLog, "vcl (ss112471) ::SVMain" );

    ImplSVData* pSVData = ImplGetSVData();

    DBG_ASSERT( pSVData->mpApp, "no instance of class Application" );

    Reference<XMultiServiceFactory> xMS;


    BOOL bInit = InitVCL( xMS );

    if( bInit )
    {
        // Application-Main rufen
        pSVData->maAppData.mbInAppMain = TRUE;
        pSVData->mpApp->Main();
        pSVData->maAppData.mbInAppMain = FALSE;
    }

    if( pSVData->mxDisplayConnection.is() )
    {
        vcl::DisplayConnection* pConnection =
            dynamic_cast<vcl::DisplayConnection*>(pSVData->mxDisplayConnection.get());

        if( pConnection )
            pConnection->dispatchDowningEvent();
        pSVData->mxDisplayConnection.clear();
    }

    // This is a hack to work around the problem of the asynchronous nature
    // of bridging accessibility through Java: on shutdown there might still
    // be some events in the AWT EventQueue, which need the SolarMutex which
    // - on the other hand - is destroyed in DeInitVCL(). So empty the queue
    // here ..
    Reference< XComponent > xComponent(pSVData->mxAccessBridge, UNO_QUERY);
    if( xComponent.is() )
    {
      ULONG nCount = Application::ReleaseSolarMutex();
      xComponent->dispose();
      Application::AcquireSolarMutex(nCount);
      pSVData->mxAccessBridge.clear();
    }

    DeInitVCL();
    return bInit;
}

BOOL SVMain()
{
    // #i47888# allow for alternative initialization as required for e.g. MacOSX
    extern BOOL ImplSVMainHook( BOOL* );

    BOOL bInit;
    if( ImplSVMainHook( &bInit ) )
        return bInit;
    else
        return ImplSVMain();
}
// This variable is set, when no Application object is instantiated
// before SVInit is called
static Application *        pOwnSvApp = NULL;
// Exception handler. pExceptionHandler != NULL => VCL already inited
ImplVCLExceptionHandler *   pExceptionHandler = NULL;

class Application_Impl : public Application
{
public:
    void                Main(){};
};

class DesktopEnvironmentContext: public cppu::WeakImplHelper1< com::sun::star::uno::XCurrentContext >
{
public:
    DesktopEnvironmentContext( const com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > & ctx)
        : m_xNextContext( ctx ) {}

    // XCurrentContext
    virtual com::sun::star::uno::Any SAL_CALL getValueByName( const rtl::OUString& Name )
            throw (com::sun::star::uno::RuntimeException);

private:
    com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > m_xNextContext;
};

Any SAL_CALL DesktopEnvironmentContext::getValueByName( const rtl::OUString& Name) throw (RuntimeException)
{
    Any retVal;

    if ( 0 == Name.compareToAscii( "system.desktop-environment" ) )
    {
        retVal = makeAny( Application::GetDesktopEnvironment() );
    }
    else if( m_xNextContext.is() )
    {
        // Call next context in chain if found
        retVal = m_xNextContext->getValueByName( Name );
    }
    return retVal;
}

BOOL InitVCL( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr )
{
    RTL_LOGFILE_CONTEXT( aLog, "vcl (ss112471) ::InitVCL" );

    if( pExceptionHandler != NULL )
        return FALSE;

    if( ! ImplGetSVData() )
        ImplInitSVData();

    if( !ImplGetSVData()->mpApp )
    {
        pOwnSvApp = new Application_Impl();
    }
    InitSalMain();

    /*AllSettings aAS;
    Application::SetSettings( aAS );// ???
    */
    ImplSVData* pSVData = ImplGetSVData();

    // SV bei den Tools anmelden
    InitTools();

    DBG_ASSERT( !pSVData->maAppData.mxMSF.is(), "VCL service factory already set" )
    pSVData->maAppData.mxMSF = rSMgr;

    // Main-Thread-Id merken
    pSVData->mnMainThreadId = ::vos::OThread::getCurrentIdentifier();

    vos::OStartupInfo   aStartInfo;
    rtl::OUString       aExeFileName;


    // Sal initialisieren
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ ::CreateSalInstance" );
    pSVData->mpDefInst = CreateSalInstance();
    if ( !pSVData->mpDefInst )
        return FALSE;
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "} ::CreateSalInstance" );

    // Desktop Environment context (to be able to get value of "system.desktop-environment" as soon as possible)
    com::sun::star::uno::setCurrentContext(
        new DesktopEnvironmentContext( com::sun::star::uno::getCurrentContext() ) );

    // Initialize application instance (should be done after initialization of VCL SAL part)
    if( pSVData->mpApp )
        // call init to initialize application class
        // soffice/sfx implementation creates the global service manager
        pSVData->mpApp->Init();

    // Den AppFileName gleich holen und absolut machen, bevor das
    // WorkingDirectory sich aendert...
    aStartInfo.getExecutableFile( aExeFileName );

    // convert path to native file format
    rtl::OUString aNativeFileName;
    osl::FileBase::getSystemPathFromFileURL( aExeFileName, aNativeFileName );
    pSVData->maAppData.mpAppFileName = new String( aNativeFileName );

    // Initialize global data
    pSVData->maGDIData.mpScreenFontList     = new ImplDevFontList;
    pSVData->maGDIData.mpScreenFontCache    = new ImplFontCache( FALSE );
    pSVData->maGDIData.mpGrfConverter       = new GraphicConverter;

    // Exception-Handler setzen
    pExceptionHandler = new ImplVCLExceptionHandler();

    // Debug-Daten initialisieren
    DBGGUI_INIT();

    return TRUE;
}

void DeInitVCL()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mbDeInit = TRUE;

    // give ime status a chance to destroy its own windows
    delete pSVData->mpImeStatus;
    pSVData->mpImeStatus = NULL;

    #if OSL_DEBUG_LEVEL > 0
    rtl::OStringBuffer aBuf( 256 );
    aBuf.append( "DeInitVCL: some top Windows are still alive\n" );
    long nTopWindowCount = Application::GetTopWindowCount();
    long nBadTopWindows = nTopWindowCount;
    for( long i = 0; i < nTopWindowCount; i++ )
    {
        Window* pWin = Application::GetTopWindow( i );
        // default window will be destroyed further down
        // but may still be useful during deinit up to that point
        if( pWin == pSVData->mpDefaultWin )
            nBadTopWindows--;
        else
        {
            aBuf.append( "text = \"" );
            aBuf.append( rtl::OUStringToOString( pWin->GetText(), osl_getThreadTextEncoding() ) );
            aBuf.append( "\" type = \"" );
            aBuf.append( typeid(*pWin).name() );
            aBuf.append( "\"\n" );
        }
    }
    DBG_ASSERT( nBadTopWindows==0, aBuf.getStr() );
    #endif

    ImplImageTree::cleanup();

    delete pExceptionHandler;
    pExceptionHandler = NULL;

    // Debug Daten zuruecksetzen
    DBGGUI_DEINIT();

    // free global data
    delete pSVData->maGDIData.mpGrfConverter;

    if( pSVData->mpSettingsConfigItem )
        delete pSVData->mpSettingsConfigItem, pSVData->mpSettingsConfigItem = NULL;
    if( pSVData->maGDIData.mpDefaultFontConfiguration )
        delete pSVData->maGDIData.mpDefaultFontConfiguration, pSVData->maGDIData.mpDefaultFontConfiguration = NULL;
    if( pSVData->maGDIData.mpFontSubstConfiguration )
        delete pSVData->maGDIData.mpFontSubstConfiguration, pSVData->maGDIData.mpFontSubstConfiguration = NULL;

    if ( pSVData->maAppData.mpIdleMgr )
        delete pSVData->maAppData.mpIdleMgr;
    Timer::ImplDeInitTimer();

    if ( pSVData->maWinData.mpMsgBoxImgList )
    {
        delete pSVData->maWinData.mpMsgBoxImgList;
        pSVData->maWinData.mpMsgBoxImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpCheckImgList )
    {
        delete pSVData->maCtrlData.mpCheckImgList;
        pSVData->maCtrlData.mpCheckImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpRadioImgList )
    {
        delete pSVData->maCtrlData.mpRadioImgList;
        pSVData->maCtrlData.mpRadioImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpPinImgList )
    {
        delete pSVData->maCtrlData.mpPinImgList;
        pSVData->maCtrlData.mpPinImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpSplitHPinImgList )
    {
        delete pSVData->maCtrlData.mpSplitHPinImgList;
        pSVData->maCtrlData.mpSplitHPinImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpSplitVPinImgList )
    {
        delete pSVData->maCtrlData.mpSplitVPinImgList;
        pSVData->maCtrlData.mpSplitVPinImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpSplitHArwImgList )
    {
        delete pSVData->maCtrlData.mpSplitHArwImgList;
        pSVData->maCtrlData.mpSplitHArwImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpSplitVArwImgList )
    {
        delete pSVData->maCtrlData.mpSplitVArwImgList;
        pSVData->maCtrlData.mpSplitVArwImgList = NULL;
    }
    if ( pSVData->mpDefaultWin )
    {
        delete pSVData->mpDefaultWin;
        pSVData->mpDefaultWin = NULL;
    }

    pSVData->maAppData.mxMSF.clear();

    if( pSVData->mpApp )
        // call deinit to deinitialize application class
        // soffice/sfx implementation disposes the global service manager
        // Warning: After this call you can't call uno services
        pSVData->mpApp->DeInit();

    if ( pSVData->maAppData.mpSettings )
    {
        delete pSVData->maAppData.mpSettings;
        pSVData->maAppData.mpSettings = NULL;
    }
    if ( pSVData->maAppData.mpAccelMgr )
    {
        delete pSVData->maAppData.mpAccelMgr;
        pSVData->maAppData.mpAccelMgr = NULL;
    }
    if ( pSVData->maAppData.mpUniqueIdCont )
    {
        delete pSVData->maAppData.mpUniqueIdCont;
        pSVData->maAppData.mpUniqueIdCont = NULL;
    }
    if ( pSVData->maAppData.mpAppFileName )
    {
        delete pSVData->maAppData.mpAppFileName;
        pSVData->maAppData.mpAppFileName = NULL;
    }
    if ( pSVData->maAppData.mpAppName )
    {
        delete pSVData->maAppData.mpAppName;
        pSVData->maAppData.mpAppName = NULL;
    }
    if ( pSVData->maAppData.mpDisplayName )
    {
        delete pSVData->maAppData.mpDisplayName;
        pSVData->maAppData.mpDisplayName = NULL;
    }
    if ( pSVData->maAppData.mpEventListeners )
    {
        delete pSVData->maAppData.mpEventListeners;
        pSVData->maAppData.mpEventListeners = NULL;
    }
    if ( pSVData->maAppData.mpKeyListeners )
    {
        delete pSVData->maAppData.mpKeyListeners;
        pSVData->maAppData.mpKeyListeners = NULL;
    }

    if ( pSVData->maAppData.mpFirstHotKey )
        ImplFreeHotKeyData();
    if ( pSVData->maAppData.mpFirstEventHook )
        ImplFreeEventHookData();

    // #114285# Moved here from ImplDeInitSVData...
    if ( pSVData->mpUnoWrapper )
    {
        pSVData->mpUnoWrapper->Destroy();
        pSVData->mpUnoWrapper = NULL;
    }

    ImplDeletePrnQueueList();
    delete pSVData->maGDIData.mpScreenFontList;
    pSVData->maGDIData.mpScreenFontList = NULL;
    delete pSVData->maGDIData.mpScreenFontCache;
    pSVData->maGDIData.mpScreenFontCache = NULL;
    ImplFreeOutDevFontData();

    if ( pSVData->mpResMgr )
    {
        delete pSVData->mpResMgr;
        pSVData->mpResMgr = NULL;
    }

    ResMgr::DestroyAllResMgr();

    // destroy all Sal interfaces before destorying the instance
    // and thereby unloading the plugin
    delete pSVData->mpSalSystem;
    pSVData->mpSalSystem = NULL;
    delete pSVData->mpSalTimer;
    pSVData->mpSalTimer = NULL;

    // Sal deinitialisieren
    DestroySalInstance( pSVData->mpDefInst );

    DeInitTools();

    DeInitSalMain();

    if( pOwnSvApp )
    {
        delete pOwnSvApp;
        pOwnSvApp = NULL;
    }
}

// only one call is allowed
struct WorkerThreadData
{
    oslWorkerFunction   pWorker;
    void *              pThreadData;
    WorkerThreadData( oslWorkerFunction pWorker_, void * pThreadData_ )
        : pWorker( pWorker_ )
        , pThreadData( pThreadData_ )
    {
    }
};

#ifdef WNT
static HANDLE hThreadID = 0;
static unsigned __stdcall _threadmain( void *pArgs )
{
    OleInitialize( NULL );
    ((WorkerThreadData*)pArgs)->pWorker( ((WorkerThreadData*)pArgs)->pThreadData );
    delete (WorkerThreadData*)pArgs;
    OleUninitialize();
    hThreadID = 0;
    return 0;
}
#else
static oslThread hThreadID = 0;
extern "C"
{
static void SAL_CALL MainWorkerFunction( void* pArgs )
{
    ((WorkerThreadData*)pArgs)->pWorker( ((WorkerThreadData*)pArgs)->pThreadData );
    delete (WorkerThreadData*)pArgs;
    hThreadID = 0;
}
} // extern "C"
#endif

void CreateMainLoopThread( oslWorkerFunction pWorker, void * pThreadData )
{
#ifdef WNT
    // sal thread alway call CoInitializeEx, so a sysdepen implementation is necessary

    unsigned uThreadID;
    hThreadID = (HANDLE)_beginthreadex(
        NULL,       // no security handle
        0,          // stacksize 0 means default
        _threadmain,    // thread worker function
        new WorkerThreadData( pWorker, pThreadData ),       // arguments for worker function
        0,          // 0 means: create immediatly otherwise use CREATE_SUSPENDED
        &uThreadID );   // thread id to fill
#else
    hThreadID = osl_createThread( MainWorkerFunction, new WorkerThreadData( pWorker, pThreadData ) );
#endif
}

void JoinMainLoopThread()
{
    if( hThreadID )
    {
#ifdef WNT
        WaitForSingleObject(hThreadID, INFINITE);
#else
        osl_joinWithThread(hThreadID);
        osl_destroyThread( hThreadID );
#endif
    }
}
