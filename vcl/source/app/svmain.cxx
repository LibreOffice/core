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
#include <sal/log.hxx>

#include <cassert>

#include <osl/file.hxx>
#include <osl/signal.h>

#include <desktop/exithelper.h>

#include <tools/debug.hxx>
#include <unotools/resmgr.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/asyncnotification.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclmain.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/image.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/settings.hxx>
#include <vcl/toolkit/unowrap.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/configsettings.hxx>
#include <vcl/lazydelete.hxx>
#include <vcl/embeddedfontshelper.hxx>
#include <vcl/debugevent.hxx>
#include <vcl/dialog.hxx>
#include <vcl/menu.hxx>
#include <vcl/virdev.hxx>
#include <vcl/print.hxx>
#include <scrwnd.hxx>

#ifdef _WIN32
#include <svsys.h>
#include <process.h>
#include <ole2.h>
#else
#include <stdlib.h>
#endif

#ifdef ANDROID
#include <cppuhelper/bootstrap.hxx>
#include <jni.h>
#endif

#include <salinst.hxx>
#include <salwtype.hxx>
#include <svdata.hxx>
#include <vcl/svmain.hxx>
#include <dbggui.hxx>
#include <accmgr.hxx>
#include <outdev.h>
#include <fontinstance.hxx>
#include <PhysicalFontCollection.hxx>
#include <print.h>
#include <salgdi.hxx>
#include <salsys.hxx>
#include <saltimer.hxx>
#include <salimestatus.hxx>
#include <displayconnectiondispatch.hxx>

#include <config_features.h>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLContext.hxx>
#endif

#include <osl/process.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/Desktop.hpp>

#include <comphelper/lok.hxx>
#include <cppuhelper/implbase.hxx>
#include <uno/current_context.hxx>

#include <opencl/OpenCLZone.hxx>
#include <opengl/zone.hxx>
#include <opengl/watchdog.hxx>

#include <basegfx/utils/systemdependentdata.hxx>

#if OSL_DEBUG_LEVEL > 0
#include <typeinfo>
#include <rtl/strbuf.hxx>
#endif

using namespace ::com::sun::star;

static bool g_bIsLeanException;

static bool isInitVCL();

static oslSignalAction VCLExceptionSignal_impl( void* /*pData*/, oslSignalInfo* pInfo)
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
        {
            OpenCLZone::hardDisable();
#ifdef _WIN32
            if (OpenCLZone::isInInitialTest())
                TerminateProcess(GetCurrentProcess(), EXITHELPER_NORMAL_RESTART);
#endif
        }
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

        vcl::SolarMutexTryAndBuyGuard aLock;
        if( aLock.isAcquired())
        {
            // do not stop timer because otherwise the UAE-Box will not be painted as well
            ImplSVData* pSVData = ImplGetSVData();
            if ( pSVData->mpApp )
            {
                SystemWindowFlags nOldMode = Application::GetSystemWindowMode();
                Application::SetSystemWindowMode( nOldMode & ~SystemWindowFlags::NOAUTOMODE );
                pSVData->mpApp->Exception( nVCLException );
                Application::SetSystemWindowMode( nOldMode );
            }
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

    const bool bWasInitVCL = isInitVCL();
    const bool bInit = bWasInitVCL || InitVCL();
    int nRet = 0;
    if (!bWasInitVCL && bInit && pSVData->mpDefInst->SVMainHook(&nRet))
        return nRet;

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
    virtual css::uno::Any SAL_CALL getValueByName( const OUString& Name ) override;

private:
    css::uno::Reference< css::uno::XCurrentContext > m_xNextContext;
};

uno::Any SAL_CALL DesktopEnvironmentContext::getValueByName( const OUString& Name)
{
    uno::Any retVal;

    if ( Name == "system.desktop-environment" )
    {
        retVal <<= Application::GetDesktopEnvironment();
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

#ifdef DBG_UTIL
namespace vclmain
{
    bool isAlive()
    {
        return ImplGetSVData()->mpDefInst;
    }
}
#endif


bool InitVCL()
{
    if( pExceptionHandler != nullptr )
        return false;

    EmbeddedFontsHelper::clearTemporaryFontFiles();

    if( !ImplGetSVData()->mpApp )
    {
        pOwnSvApp = new Application();
    }

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
    if (pSVData->mpApp)
    {
        // call init to initialize application class
        // soffice/sfx implementation creates the global service manager
        pSVData->mpApp->Init();
    }

    try
    {
        //Now that uno has been bootstrapped we can ask the config what the UI language is so that we can
        //force that in as $LANGUAGE. That way we can get gtk to render widgets RTL
        //if we have a RTL UI in an otherwise LTR locale and get gettext using externals (e.g. python)
        //to match their translations to our preferred UI language
        OUString aLocaleString(SvtSysLocaleOptions().GetRealUILanguageTag().getGlibcLocaleString(".UTF-8"));
        if (!aLocaleString.isEmpty())
        {
            MsLangId::getSystemUILanguage(); //call this now to pin what the system UI really was
            OUString envVar("LANGUAGE");
            osl_setEnvironment(envVar.pData, aLocaleString.pData);
        }
    }
    catch (const uno::Exception &e)
    {
        SAL_INFO("vcl.app", "Unable to get ui language: '" << e);
    }

    pSVData->mpDefInst->AfterAppInit();

    // Fetch AppFileName and make it absolute before the workdir changes...
    OUString aExeFileName;
    osl_getExecutableFile( &aExeFileName.pData );

    // convert path to native file format
    OUString aNativeFileName;
    osl::FileBase::getSystemPathFromFileURL( aExeFileName, aNativeFileName );
    pSVData->maAppData.mxAppFileName = aNativeFileName;

    // Initialize global data
    pSVData->maGDIData.mxScreenFontList.reset(new PhysicalFontCollection);
    pSVData->maGDIData.mxScreenFontCache.reset(new ImplFontCache);
    pSVData->maGDIData.mpGrfConverter = new GraphicConverter;

    g_bIsLeanException = getenv("LO_LEAN_EXCEPTION") != nullptr;
    // Set exception handler
    pExceptionHandler = osl_addSignalHandler(VCLExceptionSignal_impl, nullptr);

#ifndef NDEBUG
    DbgGUIInitSolarMutexCheck();
#endif

#if OSL_DEBUG_LEVEL > 0
    DebugEventInjector::getCreate();
#endif

#ifndef _WIN32
    // Clear startup notification details for child processes
    // See https://bugs.freedesktop.org/show_bug.cgi?id=11375 for discussion
    unsetenv("DESKTOP_STARTUP_ID");
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
    virtual void SAL_CALL disposing(lang::EventObject const& rSource) override;
};

void
VCLUnoWrapperDeleter::disposing(lang::EventObject const& /* rSource */)
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
    //rhbz#1444437, when using LibreOffice like a library you can't realistically
    //tear everything down and recreate them on the next call, there's too many
    //(c++) singletons that point to stuff that gets deleted during shutdown
    //which won't be recreated on restart.
    if (comphelper::LibreOfficeKit::isActive())
        return;

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
    pSVData->mpImeStatus.reset();

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

    ImageTree::get().shutdown();

    osl_removeSignalHandler( pExceptionHandler);
    pExceptionHandler = nullptr;

    // free global data
    if (pSVData->maGDIData.mpGrfConverter)
    {
        delete pSVData->maGDIData.mpGrfConverter;
        pSVData->maGDIData.mpGrfConverter = nullptr;
    }

    pSVData->mpSettingsConfigItem.reset();

    // empty and deactivate the SystemDependentDataManager
    ImplGetSystemDependentDataManager().flushAll();

    Scheduler::ImplDeInitScheduler();

    pSVData->maWinData.maMsgBoxImgList.clear();
    pSVData->maCtrlData.maCheckImgList.clear();
    pSVData->maCtrlData.maRadioImgList.clear();
    pSVData->maCtrlData.mpDisclosurePlus.reset();
    pSVData->maCtrlData.mpDisclosureMinus.reset();
    pSVData->mpDefaultWin.disposeAndClear();

#ifndef NDEBUG
    DbgGUIDeInitSolarMutexCheck();
#endif

    if ( pSVData->mpUnoWrapper )
    {
        try
        {
            uno::Reference<frame::XDesktop2> const xDesktop = frame::Desktop::create(
                    comphelper::getProcessComponentContext() );
            xDesktop->addEventListener(new VCLUnoWrapperDeleter);
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

        pSVData->maAppData.mpSettings.reset();
    }
    if (pSVData->maAppData.mpAccelMgr)
    {
        delete pSVData->maAppData.mpAccelMgr;
        pSVData->maAppData.mpAccelMgr = nullptr;
    }
    pSVData->maAppData.maKeyListeners.clear();
    pSVData->mpBlendFrameCache.reset();

    ImplDeletePrnQueueList();

    // destroy all Sal interfaces before destroying the instance
    // and thereby unloading the plugin
    pSVData->mpSalSystem.reset();
    assert( !pSVData->maSchedCtx.mpSalTimer );
    delete pSVData->maSchedCtx.mpSalTimer;
    pSVData->maSchedCtx.mpSalTimer = nullptr;

    pSVData->mpDefaultWin = nullptr;
    pSVData->mpIntroWindow = nullptr;
    pSVData->maAppData.mpActivePopupMenu = nullptr;
    pSVData->maAppData.mpWheelWindow = nullptr;
    pSVData->maGDIData.mpFirstWinGraphics = nullptr;
    pSVData->maGDIData.mpLastWinGraphics = nullptr;
    pSVData->maGDIData.mpFirstVirGraphics = nullptr;
    pSVData->maGDIData.mpLastVirGraphics = nullptr;
    pSVData->maGDIData.mpFirstPrnGraphics = nullptr;
    pSVData->maGDIData.mpLastPrnGraphics = nullptr;
    pSVData->maGDIData.mpFirstVirDev = nullptr;
    pSVData->maGDIData.mpFirstPrinter = nullptr;
    pSVData->maWinData.mpFirstFrame = nullptr;
    pSVData->maWinData.mpAppWin = nullptr;
    pSVData->maWinData.mpActiveApplicationFrame = nullptr;
    pSVData->maWinData.mpCaptureWin = nullptr;
    pSVData->maWinData.mpLastDeacWin = nullptr;
    pSVData->maWinData.mpFirstFloat = nullptr;
    pSVData->maWinData.mpExecuteDialogs.clear();
    pSVData->maWinData.mpExtTextInputWin = nullptr;
    pSVData->maWinData.mpTrackWin = nullptr;
    pSVData->maWinData.mpAutoScrollWin = nullptr;
    pSVData->maWinData.mpLastWheelWindow = nullptr;

    pSVData->maGDIData.mxScreenFontList.reset();
    pSVData->maGDIData.mxScreenFontCache.reset();
    pSVData->maGDIData.maScaleCache.remove_if([](const o3tl::lru_map<SalBitmap*, BitmapEx>::key_value_pair_t&)
                                                { return true; });

    pSVData->maGDIData.maThemeDrawCommandsCache.clear();
    pSVData->maGDIData.maThemeImageCache.clear();

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
    oslWorkerFunction const   pWorker;
    void * const              pThreadData;
    WorkerThreadData( oslWorkerFunction pWorker_, void * pThreadData_ )
        : pWorker( pWorker_ )
        , pThreadData( pThreadData_ )
    {
    }
};

#ifdef _WIN32
static HANDLE hThreadID = nullptr;
static unsigned __stdcall threadmain( void *pArgs )
{
    OleInitialize( nullptr );
    static_cast<WorkerThreadData*>(pArgs)->pWorker( static_cast<WorkerThreadData*>(pArgs)->pThreadData );
    delete static_cast<WorkerThreadData*>(pArgs);
    OleUninitialize();
    hThreadID = nullptr;
    return 0;
}
#else
static oslThread hThreadID = nullptr;
extern "C"
{
static void MainWorkerFunction( void* pArgs )
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
    hThreadID = reinterpret_cast<HANDLE>(_beginthreadex(
        nullptr,       // no security handle
        0,          // stacksize 0 means default
        threadmain,    // thread worker function
        new WorkerThreadData( pWorker, pThreadData ),       // arguments for worker function
        0,          // 0 means: create immediately otherwise use CREATE_SUSPENDED
        &uThreadID ));   // thread id to fill
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
