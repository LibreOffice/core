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

#include <sal/config.h>

#include <cassert>

#include <osl/file.hxx>
#include <osl/signal.h>

#include <tools/debug.hxx>
#include <tools/resmgr.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/asyncnotification.hxx>

#include <unotools/syslocaleoptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/image.hxx>
#include <vcl/implimagetree.hxx>
#include <vcl/settings.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/configsettings.hxx>
#include <vcl/lazydelete.hxx>
#include <vcl/embeddedfontshelper.hxx>
#include <vcl/debugevent.hxx>

#ifdef _WIN32
#include <svsys.h>
#include <process.h>
#include <ole2.h>
#endif

#ifdef ANDROID
#include <cppuhelper/bootstrap.hxx>
#include <jni.h>
#endif

#include "salinst.hxx"
#include "salwtype.hxx"
#include "svdata.hxx"
#include <vcl/svmain.hxx>
#include "dbggui.hxx"
#include "accmgr.hxx"
#include "idlemgr.hxx"
#include "outdev.h"
#include "fontinstance.hxx"
#include "PhysicalFontCollection.hxx"
#include "print.h"
#include "salgdi.hxx"
#include "salsys.hxx"
#include "saltimer.hxx"
#include "salimestatus.hxx"
#include "displayconnectiondispatch.hxx"

#include <config_features.h>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLContext.hxx>
#endif

#include <osl/process.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/Desktop.hpp>

#include <cppuhelper/implbase.hxx>
#include <uno/current_context.hxx>

#include <opencl/OpenCLZone.hxx>
#include <opengl/zone.hxx>
#include <opengl/watchdog.hxx>

#if OSL_DEBUG_LEVEL > 0
#include <typeinfo>
#include <rtl/strbuf.hxx>
#endif

using namespace ::com::sun::star;

static bool g_bIsLeanException;

static bool isInitVCL();

oslSignalAction SAL_CALL VCLExceptionSignal_impl( void* /*pData*/, oslSignalInfo* pInfo)
{
    static volatile bool bIn = false;

    // if we crash again, bail out immediately
    if ( bIn  || g_bIsLeanException)
        return osl_Signal_ActCallNextHdl;

    ExceptionCategory nVCLException = ExceptionCategory::NONE;

    // UAE
    if ( (pInfo->Signal == osl_Signal_AccessViolation)     ||
         (pInfo->Signal == osl_Signal_IntegerDivideByZero) ||
         (pInfo->Signal == osl_Signal_FloatDivideByZero)   ||
         (pInfo->Signal == osl_Signal_DebugBreak) )
    {
        nVCLException = ExceptionCategory::System;
#if HAVE_FEATURE_OPENGL
        if (OpenGLZone::isInZone())
            OpenGLZone::hardDisable();
#endif
#if HAVE_FEATURE_OPENCL
        if (OpenCLZone::isInZone())
            OpenCLZone::hardDisable();
#endif
    }

    // RC
    if ((pInfo->Signal == osl_Signal_User) &&
        (pInfo->UserSignal == OSL_SIGNAL_USER_RESOURCEFAILURE) )
        nVCLException = ExceptionCategory::ResourceNotLoaded;

    // DISPLAY-Unix
    if ((pInfo->Signal == osl_Signal_User) &&
        (pInfo->UserSignal == OSL_SIGNAL_USER_X11SUBSYSTEMERROR) )
        nVCLException = ExceptionCategory::UserInterface;

    if ( nVCLException != ExceptionCategory::NONE )
    {
        bIn = true;

        SolarMutexGuard aLock;

        // do not stop timer because otherwise the UAE-Box will not be painted as well
        ImplSVData* pSVData = ImplGetSVData();
        if ( pSVData->mpApp )
        {
            SystemWindowFlags nOldMode = Application::GetSystemWindowMode();
            Application::SetSystemWindowMode( nOldMode & ~SystemWindowFlags::NOAUTOMODE );
            pSVData->mpApp->Exception( nVCLException );
            Application::SetSystemWindowMode( nOldMode );
        }
        bIn = false;
    }

    return osl_Signal_ActCallNextHdl;

}

int ImplSVMain()
{
    // The 'real' SVMain()
    ImplSVData* pSVData = ImplGetSVData();

    SAL_WARN_IF( !pSVData->mpApp, "vcl", "no instance of class Application" );

    int nReturn = EXIT_FAILURE;

    bool bInit = isInitVCL() || InitVCL();

    if( bInit )
    {
        // call application main
        pSVData->maAppData.mbInAppMain = true;
        nReturn = pSVData->mpApp->Main();
        pSVData->maAppData.mbInAppMain = false;
    }

    if( pSVData->mxDisplayConnection.is() )
    {
        pSVData->mxDisplayConnection->terminate();
        pSVData->mxDisplayConnection.clear();
    }

    // This is a hack to work around the problem of the asynchronous nature
    // of bridging accessibility through Java: on shutdown there might still
    // be some events in the AWT EventQueue, which need the SolarMutex which
    // - on the other hand - is destroyed in DeInitVCL(). So empty the queue
    // here ..
    if( pSVData->mxAccessBridge.is() )
    {
        {
            SolarMutexReleaser aReleaser;
            pSVData->mxAccessBridge->dispose();
        }
      pSVData->mxAccessBridge.clear();
    }

#if HAVE_FEATURE_OPENGL
    OpenGLWatchdogThread::stop();
#endif
    DeInitVCL();

    return nReturn;
}

int SVMain()
{
    int nRet;
    if( !Application::IsConsoleOnly() && ImplSVMainHook( &nRet ) )
        return nRet;
    else
        return ImplSVMain();
}

// This variable is set when no Application object has been instantiated
// before InitVCL is called
static Application *        pOwnSvApp = nullptr;

// Exception handler. pExceptionHandler != NULL => VCL already inited
static oslSignalHandler pExceptionHandler = nullptr;

class DesktopEnvironmentContext: public cppu::WeakImplHelper< css::uno::XCurrentContext >
{
public:
    explicit DesktopEnvironmentContext( const css::uno::Reference< css::uno::XCurrentContext > & ctx)
        : m_xNextContext( ctx ) {}

    // XCurrentContext
    virtual css::uno::Any SAL_CALL getValueByName( const OUString& Name )
            throw (css::uno::RuntimeException, std::exception) override;

private:
    css::uno::Reference< css::uno::XCurrentContext > m_xNextContext;
};

uno::Any SAL_CALL DesktopEnvironmentContext::getValueByName( const OUString& Name) throw (uno::RuntimeException, std::exception)
{
    uno::Any retVal;

    if ( Name == "system.desktop-environment" )
    {
        retVal = uno::makeAny( Application::GetDesktopEnvironment() );
    }
    else if( m_xNextContext.is() )
    {
        // Call next context in chain if found
        retVal = m_xNextContext->getValueByName( Name );
    }
    return retVal;
}

static bool isInitVCL()
{
    ImplSVData* pSVData = ImplGetSVData();
    return  pExceptionHandler != nullptr &&
            pSVData->mpApp != nullptr &&
            pSVData->mpDefInst != nullptr;
}

bool InitVCL()
{
    if( pExceptionHandler != nullptr )
        return false;

    EmbeddedFontsHelper::clearTemporaryFontFiles();

    if( !ImplGetSVData()->mpApp )
    {
        pOwnSvApp = new Application();
    }
    InitSalMain();

    ImplSVData* pSVData = ImplGetSVData();

    // remember Main-Thread-Id
    pSVData->mnMainThreadId = ::osl::Thread::getCurrentIdentifier();

    // Initialize Sal
    pSVData->mpDefInst = CreateSalInstance();
    if ( !pSVData->mpDefInst )
        return false;

    // Desktop Environment context (to be able to get value of "system.desktop-environment" as soon as possible)
    css::uno::setCurrentContext(
        new DesktopEnvironmentContext( css::uno::getCurrentContext() ) );

    // Initialize application instance (should be done after initialization of VCL SAL part)
    if( pSVData->mpApp )
        // call init to initialize application class
        // soffice/sfx implementation creates the global service manager
        pSVData->mpApp->Init();

    pSVData->mpDefInst->AfterAppInit();

    // Fetch AppFileName and make it absolute before the workdir changes...
    OUString aExeFileName;
    osl_getExecutableFile( &aExeFileName.pData );

    // convert path to native file format
    OUString aNativeFileName;
    osl::FileBase::getSystemPathFromFileURL( aExeFileName, aNativeFileName );
    pSVData->maAppData.mpAppFileName = new OUString( aNativeFileName );

    // Initialize global data
    pSVData->maGDIData.mpScreenFontList     = new PhysicalFontCollection;
    pSVData->maGDIData.mpScreenFontCache    = new ImplFontCache;
    pSVData->maGDIData.mpGrfConverter       = new GraphicConverter;

    g_bIsLeanException = getenv("LO_LEAN_EXCEPTION") != nullptr;
    // Set exception handler
    pExceptionHandler = osl_addSignalHandler(VCLExceptionSignal_impl, nullptr);

#ifdef DBG_UTIL
    DbgGUIInitSolarMutexCheck();
#endif

#if OSL_DEBUG_LEVEL > 0
    DebugEventInjector::getCreate();
#endif

    return true;
}

namespace
{

/** Serves for destroying the VCL UNO wrapper as late as possible. This avoids
  crash at exit in some special cases when a11y is enabled (e.g., when
  a bundled extension is registered/deregistered during startup, forcing exit
  while the app is still in splash screen.)
 */
class VCLUnoWrapperDeleter : public cppu::WeakImplHelper<css::lang::XEventListener>
{
    virtual void SAL_CALL disposing(lang::EventObject const& rSource) throw(uno::RuntimeException, std::exception) override;
};

void
VCLUnoWrapperDeleter::disposing(lang::EventObject const& /* rSource */)
    throw(uno::RuntimeException, std::exception)
{
    ImplSVData* const pSVData = ImplGetSVData();
    if (pSVData && pSVData->mpUnoWrapper)
    {
        pSVData->mpUnoWrapper->Destroy();
        pSVData->mpUnoWrapper = nullptr;
    }
}

}

void DeInitVCL()
{
    {
        SolarMutexReleaser r; // unblock threads blocked on that so we can join
        ::comphelper::JoinAsyncEventNotifiers();
    }
    ImplSVData* pSVData = ImplGetSVData();
    // lp#1560328: clear cache before disposing rest of VCL
    if(pSVData->mpBlendFrameCache)
        pSVData->mpBlendFrameCache->m_aLastResult.Clear();
    pSVData->mbDeInit = true;

    vcl::DeleteOnDeinitBase::ImplDeleteOnDeInit();

    // give ime status a chance to destroy its own windows
    delete pSVData->mpImeStatus;
    pSVData->mpImeStatus = nullptr;

#if OSL_DEBUG_LEVEL > 0
    OStringBuffer aBuf( 256 );
    aBuf.append( "DeInitVCL: some top Windows are still alive\n" );
    long nTopWindowCount = Application::GetTopWindowCount();
    long nBadTopWindows = nTopWindowCount;
    for( long i = 0; i < nTopWindowCount; i++ )
    {
        vcl::Window* pWin = Application::GetTopWindow( i );
        // default window will be destroyed further down
        // but may still be useful during deinit up to that point
        if( pWin == pSVData->mpDefaultWin )
            nBadTopWindows--;
        else
        {
            aBuf.append( "text = \"" );
            aBuf.append( OUStringToOString( pWin->GetText(), osl_getThreadTextEncoding() ) );
            aBuf.append( "\" type = \"" );
            aBuf.append( typeid(*pWin).name() );
            aBuf.append( "\", ptr = 0x" );
            aBuf.append( sal_Int64( pWin ), 16 );
            aBuf.append( "\n" );
        }
    }
    SAL_WARN_IF( nBadTopWindows!=0, "vcl", aBuf.getStr() );
#endif

    ImplImageTree::get().shutDown();

    osl_removeSignalHandler( pExceptionHandler);
    pExceptionHandler = nullptr;

    // free global data
    delete pSVData->maGDIData.mpGrfConverter;

    if( pSVData->mpSettingsConfigItem )
    {
        delete pSVData->mpSettingsConfigItem;
        pSVData->mpSettingsConfigItem = nullptr;
    }

    if ( pSVData->maAppData.mpIdleMgr )
    {
        delete pSVData->maAppData.mpIdleMgr;
        pSVData->maAppData.mpIdleMgr = nullptr;
    }
    Scheduler::ImplDeInitScheduler();

    if ( pSVData->maWinData.mpMsgBoxImgList )
    {
        delete pSVData->maWinData.mpMsgBoxImgList;
        pSVData->maWinData.mpMsgBoxImgList = nullptr;
    }
    if ( pSVData->maCtrlData.mpCheckImgList )
    {
        delete pSVData->maCtrlData.mpCheckImgList;
        pSVData->maCtrlData.mpCheckImgList = nullptr;
    }
    if ( pSVData->maCtrlData.mpRadioImgList )
    {
        delete pSVData->maCtrlData.mpRadioImgList;
        pSVData->maCtrlData.mpRadioImgList = nullptr;
    }
    if ( pSVData->maCtrlData.mpPinImgList )
    {
        delete pSVData->maCtrlData.mpPinImgList;
        pSVData->maCtrlData.mpPinImgList = nullptr;
    }
    if ( pSVData->maCtrlData.mpSplitHPinImgList )
    {
        delete pSVData->maCtrlData.mpSplitHPinImgList;
        pSVData->maCtrlData.mpSplitHPinImgList = nullptr;
    }
    if ( pSVData->maCtrlData.mpSplitVPinImgList )
    {
        delete pSVData->maCtrlData.mpSplitVPinImgList;
        pSVData->maCtrlData.mpSplitVPinImgList = nullptr;
    }
    if ( pSVData->maCtrlData.mpSplitHArwImgList )
    {
        delete pSVData->maCtrlData.mpSplitHArwImgList;
        pSVData->maCtrlData.mpSplitHArwImgList = nullptr;
    }
    if ( pSVData->maCtrlData.mpSplitVArwImgList )
    {
        delete pSVData->maCtrlData.mpSplitVArwImgList;
        pSVData->maCtrlData.mpSplitVArwImgList = nullptr;
    }
    if ( pSVData->maCtrlData.mpDisclosurePlus )
    {
        delete pSVData->maCtrlData.mpDisclosurePlus;
        pSVData->maCtrlData.mpDisclosurePlus = nullptr;
    }
    if ( pSVData->maCtrlData.mpDisclosureMinus )
    {
        delete pSVData->maCtrlData.mpDisclosureMinus;
        pSVData->maCtrlData.mpDisclosureMinus = nullptr;
    }
    pSVData->mpDefaultWin.disposeAndClear();

#ifdef DBG_UTIL
    DbgGUIDeInitSolarMutexCheck();
#endif

    if ( pSVData->mpUnoWrapper )
    {
        try
        {
            uno::Reference<frame::XDesktop2> const xDesktop = frame::Desktop::create(
                    comphelper::getProcessComponentContext() );
            xDesktop->addEventListener(new VCLUnoWrapperDeleter());
        }
        catch (uno::Exception const&)
        {
            // ignore
        }
    }

    if( pSVData->mpApp || pSVData->maDeInitHook.IsSet() )
    {
        SolarMutexReleaser aReleaser;
        // call deinit to deinitialize application class
        // soffice/sfx implementation disposes the global service manager
        // Warning: After this call you can't call uno services
        if( pSVData->mpApp )
        {
            pSVData->mpApp->DeInit();
        }
        if( pSVData->maDeInitHook.IsSet() )
        {
            pSVData->maDeInitHook.Call(nullptr);
        }
    }

    if ( pSVData->maAppData.mpSettings )
    {
        if ( pSVData->maAppData.mpCfgListener )
        {
            pSVData->maAppData.mpSettings->GetSysLocale().GetOptions().RemoveListener( pSVData->maAppData.mpCfgListener );
            delete pSVData->maAppData.mpCfgListener;
        }

        delete pSVData->maAppData.mpSettings;
        pSVData->maAppData.mpSettings = nullptr;
    }
    if ( pSVData->maAppData.mpAccelMgr )
    {
        delete pSVData->maAppData.mpAccelMgr;
        pSVData->maAppData.mpAccelMgr = nullptr;
    }
    if ( pSVData->maAppData.mpAppFileName )
    {
        delete pSVData->maAppData.mpAppFileName;
        pSVData->maAppData.mpAppFileName = nullptr;
    }
    if ( pSVData->maAppData.mpAppName )
    {
        delete pSVData->maAppData.mpAppName;
        pSVData->maAppData.mpAppName = nullptr;
    }
    if ( pSVData->maAppData.mpDisplayName )
    {
        delete pSVData->maAppData.mpDisplayName;
        pSVData->maAppData.mpDisplayName = nullptr;
    }
    if ( pSVData->maAppData.mpToolkitName )
    {
        delete pSVData->maAppData.mpToolkitName;
        pSVData->maAppData.mpToolkitName = nullptr;
    }
    if ( pSVData->maAppData.mpEventListeners )
    {
        delete pSVData->maAppData.mpEventListeners;
        pSVData->maAppData.mpEventListeners = nullptr;
    }
    if ( pSVData->maAppData.mpKeyListeners )
    {
        delete pSVData->maAppData.mpKeyListeners;
        pSVData->maAppData.mpKeyListeners = nullptr;
    }

    if ( pSVData->maAppData.mpFirstHotKey )
        ImplFreeHotKeyData();
    if ( pSVData->maAppData.mpFirstEventHook )
        ImplFreeEventHookData();

    if (pSVData->mpBlendFrameCache)
    {
        delete pSVData->mpBlendFrameCache;
        pSVData->mpBlendFrameCache = nullptr;
    }

    if (pSVData->mpCommandInfoProvider)
    {
        pSVData->mpCommandInfoProvider->dispose();
        pSVData->mpCommandInfoProvider = nullptr;
    }

    ImplDeletePrnQueueList();
    delete pSVData->maGDIData.mpScreenFontList;
    pSVData->maGDIData.mpScreenFontList = nullptr;
    delete pSVData->maGDIData.mpScreenFontCache;
    pSVData->maGDIData.mpScreenFontCache = nullptr;

    if ( pSVData->mpResMgr )
    {
        delete pSVData->mpResMgr;
        pSVData->mpResMgr = nullptr;
    }

    ResMgr::DestroyAllResMgr();

    // destroy all Sal interfaces before destroying the instance
    // and thereby unloading the plugin
    delete pSVData->mpSalSystem;
    pSVData->mpSalSystem = nullptr;
    delete pSVData->mpSalTimer;
    pSVData->mpSalTimer = nullptr;

    // Deinit Sal
    if (pSVData->mpDefInst)
    {
        DestroySalInstance( pSVData->mpDefInst );
        pSVData->mpDefInst = nullptr;
    }

    if( pOwnSvApp )
    {
        delete pOwnSvApp;
        pOwnSvApp = nullptr;
    }

    EmbeddedFontsHelper::clearTemporaryFontFiles();
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

#ifdef _WIN32
static HANDLE hThreadID = 0;
static unsigned __stdcall _threadmain( void *pArgs )
{
    OleInitialize( nullptr );
    static_cast<WorkerThreadData*>(pArgs)->pWorker( static_cast<WorkerThreadData*>(pArgs)->pThreadData );
    delete static_cast<WorkerThreadData*>(pArgs);
    OleUninitialize();
    hThreadID = 0;
    return 0;
}
#else
static oslThread hThreadID = nullptr;
extern "C"
{
static void SAL_CALL MainWorkerFunction( void* pArgs )
{
    static_cast<WorkerThreadData*>(pArgs)->pWorker( static_cast<WorkerThreadData*>(pArgs)->pThreadData );
    delete static_cast<WorkerThreadData*>(pArgs);
    hThreadID = nullptr;
}
} // extern "C"
#endif

void CreateMainLoopThread( oslWorkerFunction pWorker, void * pThreadData )
{
#ifdef _WIN32
    // sal thread always call CoInitializeEx, so a system dependent implementation is necessary

    unsigned uThreadID;
    hThreadID = (HANDLE)_beginthreadex(
        NULL,       // no security handle
        0,          // stacksize 0 means default
        _threadmain,    // thread worker function
        new WorkerThreadData( pWorker, pThreadData ),       // arguments for worker function
        0,          // 0 means: create immediately otherwise use CREATE_SUSPENDED
        &uThreadID );   // thread id to fill
#else
    hThreadID = osl_createThread( MainWorkerFunction, new WorkerThreadData( pWorker, pThreadData ) );
#endif
}

void JoinMainLoopThread()
{
    if( hThreadID )
    {
#ifdef _WIN32
        WaitForSingleObject(hThreadID, INFINITE);
#else
        osl_joinWithThread(hThreadID);
        osl_destroyThread( hThreadID );
#endif
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
