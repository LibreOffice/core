/*************************************************************************
 *
 *  $RCSfile: svmain.cxx,v $
 *
 *  $Revision: 1.42 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:32:23 $
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

#define _SV_SVMAIN_CXX

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
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALSOUND_HXX
#include <salsound.hxx>
#endif
#ifndef _SV_SALOGL_HXX
#include <salogl.hxx>
#endif
#ifndef _SV_SALWTYPE_HXX
#include <salwtype.hxx>
#endif
#ifndef _VOS_SIGNAL_HXX
#include <vos/signal.hxx>
#endif
#ifndef _VOS_SOCKET_HXX
#include <vos/socket.hxx>
#endif
#ifndef _TOOLS_H
#include <tools/tools.h>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _UNIQID_HXX
#include <tools/unqid.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_DBGGUI_HXX
#include <dbggui.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_CVTGRF_HXX
#include <cvtgrf.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <image.hxx>
#endif
#ifndef _SV_RESMGR_HXX
#include <resmgr.hxx>
#endif
#ifndef _SV_ACCMGR_HXX
#include <accmgr.hxx>
#endif
#ifndef _SV_IDLEMGR_HXX
#include <idlemgr.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_PRINT_H
#include <print.h>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif

#include <vos/process.hxx>
#include <osl/file.hxx>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#include <rtl/logfile.hxx>

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

#include <fontcfg.hxx>

#pragma hdrstop

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
        if ( (pInfo->Signal == TSignal_AccessViolation)     ||
             (pInfo->Signal == TSignal_IntegerDivideByZero) ||
             (pInfo->Signal == TSignal_FloatDivideByZero)   ||
             (pInfo->Signal == TSignal_DebugBreak) )
            nVCLException = EXC_SYSTEM;

        // RC
        if ((pInfo->Signal == TSignal_SignalUser) &&
            (pInfo->UserSignal == OSL_SIGNAL_USER_RESOURCEFAILURE) )
            nVCLException = EXC_RSCNOTLOADED;

        // DISPLAY-Unix
        if ((pInfo->Signal == TSignal_SignalUser) &&
            (pInfo->UserSignal == OSL_SIGNAL_USER_X11SUBSYSTEMERROR) )
            nVCLException = EXC_DISPLAY;

        // Remote-Client
        if ((pInfo->Signal == TSignal_SignalUser) &&
            (pInfo->UserSignal == OSL_SIGNAL_USER_RVPCONNECTIONERROR) )
            nVCLException = EXC_REMOTE;

        if ( nVCLException )
        {
            bIn = TRUE;
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

#ifndef REMOTE_APPSERVER
            return vos::OSignalHandler::TAction_CallNextHandler;
#else
            return vos::OSignalHandler::TAction_KillApplication;
#endif
        }
    }

    return vos::OSignalHandler::TAction_CallNextHandler;
}

// =======================================================================
BOOL SVMain()
{
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

    DeInitVCL();
    return bInit;
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

BOOL InitVCL( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr )
{
    RTL_LOGFILE_CONTEXT( aLog, "vcl (ss112471) ::InitVCL" );

    if( pExceptionHandler != NULL )
        return FALSE;

    if( !ImplGetSVData()->mpApp )
    {
        pOwnSvApp = new Application_Impl();
    }
#ifndef REMOTE_APPSERVER
    InitSalMain();
#endif

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

#ifdef REMOTE_APPSERVER
    // create condition now to avoid race
    pSVData->mpStartUpCond = new vos::OCondition;
#endif

    // Sal initialisieren
#ifndef REMOTE_APPSERVER
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ ::CreateSalInstance" );
    pSVData->mpDefInst = CreateSalInstance();
    if ( !pSVData->mpDefInst )
        return FALSE;
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "} ::CreateSalInstance" );
#endif

    // Initialize application instance (should be done after initialization of VCL SAL part)
    if( pSVData->mpApp )
        // call init to initialize application class
        // soffice/sfx implementation creates the global service manager
        pSVData->mpApp->Init();

#ifdef REMOTE_APPSERVER
    {
    Reference< XMultiServiceFactory > rSMgr = ::comphelper::getProcessServiceFactory();

    pSVData->mpRmEventQueue = new RmEventQueue;
    pSVData->mpWindowObjectMutex = new vos::OMutex;
    pSVData->maAppData.mpSolarMutex = new ImplRemoteYieldMutex;

    pSVData->maGDIData.mpScreenFontList   = new ImplDevFontList;
    pSVData->maGDIData.mpScreenFontCache  = new ImplFontCache( FALSE );

    pSVData->maAppData.mpSolarMutex->acquire(); // mutex should be aquired for startup

//    ImplInitRemotePrinterList();
    }

#endif

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
    delete pExceptionHandler;
    pExceptionHandler = NULL;

    ImplSVData* pSVData = ImplGetSVData();

    // Debug Daten zuruecksetzen
    DBGGUI_DEINIT();

    // Access list
    List* pList = pSVData->maAppData.mpAccessList;
    if( pList )
    {
        for( void* pLink = pList->First(); pLink; pLink = pList->Next() )
            delete (Link*) pLink;
        delete pList;
        pSVData->maAppData.mpAccessList = NULL;
    }

    // free global data
    delete pSVData->maGDIData.mpGrfConverter;

    if( pSVData->mpSettingsConfigItem )
        delete pSVData->mpSettingsConfigItem, pSVData->mpSettingsConfigItem = NULL;
    if( pSVData->maGDIData.mpDefFontConfig )
        delete pSVData->maGDIData.mpDefFontConfig, pSVData->maGDIData.mpDefFontConfig = NULL;
    if( pSVData->maGDIData.mpFontSubstConfig )
        delete pSVData->maGDIData.mpFontSubstConfig, pSVData->maGDIData.mpFontSubstConfig = NULL;

    if ( pSVData->maAppData.mpIdleMgr )
        delete pSVData->maAppData.mpIdleMgr;
    ImplDeInitTimer();

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
    if ( pSVData->mpResMgr )
    {
        delete pSVData->mpResMgr;
        pSVData->mpResMgr = NULL;
    }

#ifdef REMOTE_APPSERVER
    if( pSVData->mxClientFactory.is() )
    {
        try
        {
            pSVData->mxClientFactory = Reference < XMultiServiceFactory >();
        }
        catch(::com::sun::star::uno::Exception&)
        {
        }
    }

    if( pSVData->mxMultiFactory.is() )
    {
        try
        {
            pSVData->mxMultiFactory.clear();
        }
        catch(::com::sun::star::uno::Exception&)
        {
        }

    }
    CORmStarOffice::eraseRemoteCaches();
    if( pSVData->mxStatus.is() )
    {
        try
        {
            // #93174 DO NOT SYNC HERE -
            // the client's sync object might already be destructed by the ORmRemoteClientFactory destructor
            // CHECK_FOR_RVPSYNC_NORMAL()

            delete pSVData->mpRVPNormalSync;
            delete pSVData->mpRVPSoundSync;
            delete pSVData->mpAtoms;

            pSVData->mxStatus->Quit();
            pSVData->mxStatus = Reference < ::com::sun::star::portal::client::XRmStatus >();
        }
        catch(::com::sun::star::uno::Exception&)
        {
        }
    }

    if( pSVData->mpApp )
        // call deinit to deinitialize application class
        // soffice/sfx implementation disposes the global service manager
        // Warning: After this call you can't call uno services
        pSVData->mpApp->DeInit();

    pSVData->maAppData.mpSolarMutex->release();
    delete pSVData->maAppData.mpSolarMutex;
    pSVData->maAppData.mpSolarMutex = NULL;
    pSVData->mpOTimer->release();
    pSVData->mpOTimer = NULL;
    delete pSVData->mpRmEventQueue;
    pSVData->mpRmEventQueue = NULL;
    delete pSVData->mpWindowObjectMutex;
    pSVData->mpWindowObjectMutex = NULL;

    if ( pSVData->mpKeyNames )
    {
        for( String* pObj = pSVData->mpKeyNames->First(); pObj; pObj = pSVData->mpKeyNames->Next() )
            delete pObj;
        delete pSVData->mpKeyNames;
    }
#else
    if( pSVData->mpApp )
        // call deinit to deinitialize application class
        // soffice/sfx implementation disposes the global service manager
        // Warning: After this call you can't call uno services
        pSVData->mpApp->DeInit();
#endif

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
    if ( pSVData->maAppData.mpResPath )
    {
        delete pSVData->maAppData.mpResPath;
        pSVData->maAppData.mpResPath = NULL;
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

    ImplDeletePrnQueueList();
    delete pSVData->maGDIData.mpScreenFontList;
    pSVData->maGDIData.mpScreenFontList = NULL;
    delete pSVData->maGDIData.mpScreenFontCache;
    pSVData->maGDIData.mpScreenFontCache = NULL;
    ImplFreeOutDevFontData();

    ResMgr::DestroyAllResMgr();

    // Sal deinitialisieren
#ifndef REMOTE_APPSERVER
    DestroySalInstance( pSVData->mpDefInst );
#endif

    DeInitTools();

#ifndef REMOTE_APPSERVER
    DeInitSalMain();
#endif
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
static void SAL_CALL MainWorkerFunction( void* pArgs )
{
    ((WorkerThreadData*)pArgs)->pWorker( ((WorkerThreadData*)pArgs)->pThreadData );
    delete (WorkerThreadData*)pArgs;
    hThreadID = 0;
}
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
