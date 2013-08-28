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

#if defined UNX
#include <limits.h>
#else // UNX
#include <stdlib.h>
#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif
#endif // UNX

#include <sfx2/app.hxx>
#include <sfx2/frame.hxx>
#include <basic/basrdll.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <svtools/asynclink.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/urlbmk.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>

#include <svl/svdde.hxx>
#include <unotools/tempfile.hxx>
#include <osl/file.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <basic/basmgr.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/appuno.hxx>
#include "sfx2/sfxhelp.hxx"
#include <sfx2/request.hxx>
#include "sfxtypes.hxx"
#include "sfx2/sfxresid.hxx"
#include "arrdecl.hxx"
#include <sfx2/progress.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/new.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/genlink.hxx>
#include <sfx2/viewfrm.hxx>
#include "appdata.hxx"
#include "openflag.hxx"
#include "app.hrc"
#include "virtmenu.hxx"
#include <sfx2/module.hxx>
#include <sfx2/event.hxx>
#include "imestatuswindow.hxx"
#include "workwin.hxx"
#include <sfx2/tbxctrl.hxx>
#include <sfx2/sfxdlg.hxx>
#include "sfx2/stbitem.hxx"
#include "eventsupplier.hxx"
#include <sfx2/dockwin.hxx>

#ifdef DBG_UTIL
#include <sfx2/mnuitem.hxx>
#endif

#include <unotools/saveopt.hxx>
#include <svtools/helpopt.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/historyoptions.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/localisationoptions.hxx>
#include <unotools/fontoptions.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unotools/syslocale.hxx>
#include <framework/addonsoptions.hxx>
#include <unotools/extendedsecurityoptions.hxx>
#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>

using namespace ::com::sun::star;

// Static member
SfxApplication* SfxApplication::pApp = NULL;
#ifndef DISABLE_SCRIPTING
static BasicDLL*       pBasic   = NULL;
#endif

#if HAVE_FEATURE_DESKTOP
static SfxHelp*        pSfxHelp = NULL;
#endif

namespace
{
    class theApplicationMutex
        : public rtl::Static<osl::Mutex, theApplicationMutex> {};
}

#include <framework/imageproducer.hxx>
#include <framework/sfxhelperfunctions.hxx>
#include "sfx2/imagemgr.hxx"
#include "fwkhelper.hxx"

SfxApplication* SfxApplication::GetOrCreate()
{
    // SFX on demand
    ::osl::MutexGuard aGuard(theApplicationMutex::get());
    if (!pApp)
    {
        SAL_INFO( "sfx.appl", "sfx2 (mb93783) ::SfxApplication::SetApp" );

        pApp = new SfxApplication;

        // at the moment a bug may occur when Initialize_Impl returns FALSE,
        // but this is only temporary because all code that may cause such
        // a fault will be moved outside the SFX
        pApp->Initialize_Impl();

        ::framework::SetImageProducer( GetImage );
        ::framework::SetRefreshToolbars( RefreshToolbars );
        ::framework::SetToolBoxControllerCreator( SfxToolBoxControllerFactory );
        ::framework::SetStatusBarControllerCreator( SfxStatusBarControllerFactory );
        ::framework::SetDockingWindowCreator( SfxDockingWindowFactory );
        ::framework::SetIsDockingWindowVisible( IsDockingWindowVisible );
        ::framework::SetActivateToolPanel( &SfxViewFrame::ActivateToolPanel );
#if HAVE_FEATURE_DESKTOP
        Application::SetHelp( pSfxHelp );
        if ( SvtHelpOptions().IsHelpTips() )
            Help::EnableQuickHelp();
        else
            Help::DisableQuickHelp();
        if ( SvtHelpOptions().IsHelpTips() && SvtHelpOptions().IsExtendedHelp() )
            Help::EnableBalloonHelp();
        else
            Help::DisableBalloonHelp();
#endif
    }
    return pApp;
}

SfxApplication::SfxApplication()
    : pAppData_Impl( 0 )
{
    SAL_INFO( "sfx.appl", "sfx2 (mb93783) ::SfxApplication::SfxApplication" );

    SetName( OUString("StarOffice") );
    SvtViewOptions::AcquireOptions();

    pAppData_Impl = new SfxAppData_Impl( this );
    pAppData_Impl->m_xImeStatusWindow->init();

    SAL_INFO( "sfx.appl", "{ initialize DDE" );

    sal_Bool bOk = InitializeDde();

#ifdef DBG_UTIL
    if( !bOk )
    {
        OStringBuffer aStr(
            RTL_CONSTASCII_STRINGPARAM("No DDE-Service possible. Error: "));
        if( GetDdeService() )
            aStr.append(static_cast<sal_Int32>(GetDdeService()->GetError()));
        else
            aStr.append('?');
        DBG_ASSERT( sal_False, aStr.getStr() );
    }
#else
    (void)bOk;
#endif

#if HAVE_FEATURE_DESKTOP
    pSfxHelp = new SfxHelp;
#endif

#ifndef DISABLE_SCRIPTING
    pBasic   = new BasicDLL;
    StarBASIC::SetGlobalErrorHdl( LINK( this, SfxApplication, GlobalBasicErrorHdl_Impl ) );
#endif
    SAL_INFO( "sfx.appl", "} initialize DDE" );
}

SfxApplication::~SfxApplication()
{
    OSL_ENSURE( GetObjectShells_Impl().size() == 0, "Memory leak: some object shells were not removed!" );

    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    SfxModule::DestroyModules_Impl();

#if HAVE_FEATURE_DESKTOP
    delete pSfxHelp;
    Application::SetHelp( NULL );
#endif

    // delete global options
    SvtViewOptions::ReleaseOptions();

#ifndef DISABLE_SCRIPTING
    delete pBasic;
#endif
    if ( !pAppData_Impl->bDowning )
        Deinitialize();

    delete pAppData_Impl;
    pApp = 0;
}

//====================================================================

const OUString& SfxApplication::GetLastDir_Impl() const

/*  [Description]

    Internal method by which the last set directory with the method
    <SfxApplication::SetLastDir_Impl()> in SFX is returned.

    This is usually the most recently addressed by the
    SfxFileDialog directory.

    [Cross-reference]
    <SfxApplication::SetLastDir_Impl()>
*/

{
    return pAppData_Impl->aLastDir;
}

const OUString& SfxApplication::GetLastSaveDirectory() const

/*  [Description]

    As <SfxApplication::GetLastDir_Impl()>, only external

    [Cross-reference]
    <SfxApplication::GetLastDir_Impl()>
*/

{
    return GetLastDir_Impl();
}

//--------------------------------------------------------------------

void SfxApplication::SetLastDir_Impl
(
    const OUString&   rNewDir     /* Complete directory path as a string */
)

/*  [Description]

    Internal Method, by which a directory path is set that was last addressed
    (eg by the SfxFileDialog).

    [Cross-reference]
    <SfxApplication::GetLastDir_Impl()>
*/

{
    pAppData_Impl->aLastDir = rNewDir;
}

//--------------------------------------------------------------------

void SfxApplication::ResetLastDir()
{
    String aEmpty;
    pAppData_Impl->aLastDir = aEmpty;
}

//--------------------------------------------------------------------

SfxDispatcher* SfxApplication::GetDispatcher_Impl()
{
    return pAppData_Impl->pViewFrame? pAppData_Impl->pViewFrame->GetDispatcher(): pAppData_Impl->pAppDispat;
}

//--------------------------------------------------------------------
void SfxApplication::SetViewFrame_Impl( SfxViewFrame *pFrame )
{
    if ( pFrame != pAppData_Impl->pViewFrame )
    {
        // get the containerframes ( if one of the frames is an InPlaceFrame )
        SfxViewFrame *pOldContainerFrame = pAppData_Impl->pViewFrame;
        while ( pOldContainerFrame && pOldContainerFrame->GetParentViewFrame_Impl() )
            pOldContainerFrame = pOldContainerFrame->GetParentViewFrame_Impl();
        SfxViewFrame *pNewContainerFrame = pFrame;
        while ( pNewContainerFrame && pNewContainerFrame->GetParentViewFrame_Impl() )
            pNewContainerFrame = pNewContainerFrame->GetParentViewFrame_Impl();

        // DocWinActivate : both frames belong to the same TopWindow
        // TopWinActivate : both frames belong to different TopWindows

        sal_Bool bTaskActivate = pOldContainerFrame != pNewContainerFrame;

        if ( pOldContainerFrame )
        {
            if ( bTaskActivate )
                NotifyEvent( SfxViewEventHint( SFX_EVENT_DEACTIVATEDOC, GlobalEventConfig::GetEventName(STR_EVENT_DEACTIVATEDOC), pOldContainerFrame->GetObjectShell(), pOldContainerFrame->GetFrame().GetController() ) );
            pOldContainerFrame->DoDeactivate( bTaskActivate, pFrame );

            if( pOldContainerFrame->GetProgress() )
                pOldContainerFrame->GetProgress()->Suspend();
        }

        pAppData_Impl->pViewFrame = pFrame;

        if( pNewContainerFrame )
        {
            pNewContainerFrame->DoActivate( bTaskActivate );
            if ( bTaskActivate && pNewContainerFrame->GetObjectShell() )
            {
                pNewContainerFrame->GetObjectShell()->PostActivateEvent_Impl( pNewContainerFrame );
                NotifyEvent(SfxViewEventHint(SFX_EVENT_ACTIVATEDOC, GlobalEventConfig::GetEventName(STR_EVENT_ACTIVATEDOC), pNewContainerFrame->GetObjectShell(), pNewContainerFrame->GetFrame().GetController() ) );
            }

            SfxProgress *pProgress = pNewContainerFrame->GetProgress();
            if ( pProgress )
            {
                if( pProgress->IsSuspended() )
                    pProgress->Resume();
                else
                    pProgress->SetState( pProgress->GetState() );
            }

            if ( pAppData_Impl->pViewFrame->GetViewShell() )
            {
                SfxDispatcher* pDisp = pAppData_Impl->pViewFrame->GetDispatcher();
                pDisp->Flush();
                pDisp->Update_Impl(sal_True);
            }
        }
    }

    // even if the frame actually didn't change, ensure its document is forwarded
    // to SfxObjectShell::SetCurrentComponent.
    // Otherwise, the CurrentComponent might not be correct, in case it has meanwhile
    // been reset to some other document, by some non-SFX component.
    // #i49133# / 2007-12-19 / frank.schoenheit@sun.com
    if ( pFrame && pFrame->GetViewShell() )
        pFrame->GetViewShell()->SetCurrentDocument();
}

//---------------------------------------------------------------------

ResMgr* SfxApplication::CreateResManager( const char *pPrefix )
{
    return ResMgr::CreateResMgr(pPrefix);
}

//--------------------------------------------------------------------

ResMgr* SfxApplication::GetSfxResManager()
{
    return SfxResId::GetResMgr();
}

//------------------------------------------------------------------------

void SfxApplication::SetProgress_Impl
(
    SfxProgress *pProgress

)
{
    DBG_ASSERT( ( !pAppData_Impl->pProgress && pProgress ) ||
                ( pAppData_Impl->pProgress && !pProgress ),
                "Progress acitivation/deacitivation mismatch" );

    if ( pAppData_Impl->pProgress && pProgress )
    {
        pAppData_Impl->pProgress->Suspend();
        pAppData_Impl->pProgress->UnLock();
        delete pAppData_Impl->pProgress;
    }

    pAppData_Impl->pProgress = pProgress;
}

//------------------------------------------------------------------------

sal_uInt16 SfxApplication::GetFreeIndex()
{
    return pAppData_Impl->aIndexBitSet.GetFreeIndex()+1;
}

//------------------------------------------------------------------------

void SfxApplication::ReleaseIndex(sal_uInt16 i)
{
    pAppData_Impl->aIndexBitSet.ReleaseIndex(i-1);
}

//--------------------------------------------------------------------

Window* SfxApplication::GetTopWindow() const
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( SfxViewFrame::Current() );
    return pWork ? pWork->GetWindow() : NULL;
}

SfxTbxCtrlFactArr_Impl&     SfxApplication::GetTbxCtrlFactories_Impl() const
{
    return *pAppData_Impl->pTbxCtrlFac;
}

SfxStbCtrlFactArr_Impl&     SfxApplication::GetStbCtrlFactories_Impl() const
{
    return *pAppData_Impl->pStbCtrlFac;
}

SfxMenuCtrlFactArr_Impl&    SfxApplication::GetMenuCtrlFactories_Impl() const
{
    return *pAppData_Impl->pMenuCtrlFac;
}

SfxViewFrameArr_Impl&       SfxApplication::GetViewFrames_Impl() const
{
    return *pAppData_Impl->pViewFrames;
}

SfxViewShellArr_Impl&       SfxApplication::GetViewShells_Impl() const
{
    return *pAppData_Impl->pViewShells;
}

SfxObjectShellArr_Impl&     SfxApplication::GetObjectShells_Impl() const
{
    return *pAppData_Impl->pObjShells;
}

void SfxApplication::Invalidate( sal_uInt16 nId )
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst(); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame ) )
        Invalidate_Impl( pFrame->GetBindings(), nId );
}

#ifndef DISABLE_SCRIPTING

#ifndef DISABLE_DYNLOADING

typedef long (SAL_CALL *basicide_handle_basic_error)(void*);
typedef void* (SAL_CALL *basicide_macro_organizer)(sal_Int16);

extern "C" { static void SAL_CALL thisModule() {} }

#else

extern "C" long basicide_handle_basic_error(void*);
extern "C" void *basicide_macro_organizer(sal_Int16);

#endif

#endif

IMPL_LINK( SfxApplication, GlobalBasicErrorHdl_Impl, StarBASIC*, pStarBasic )
{
#ifdef DISABLE_SCRIPTING
    (void) pStarBasic;
    return 0;
#else

#ifndef DISABLE_DYNLOADING
    // get basctl dllname
    static OUString aLibName( SVLIBRARY( "basctl"  ) );

    // load module
    oslModule handleMod = osl_loadModuleRelative(
        &thisModule, aLibName.pData, 0 );

    // get symbol
    OUString aSymbol( "basicide_handle_basic_error"  );
    basicide_handle_basic_error pSymbol = (basicide_handle_basic_error) osl_getFunctionSymbol( handleMod, aSymbol.pData );

    // call basicide_handle_basic_error in basctl
    long nRet = pSymbol ? pSymbol( pStarBasic ) : 0;

#else

    long nRet = basicide_handle_basic_error( pStarBasic );

#endif

    return nRet;

#endif
}

sal_Bool SfxApplication::IsXScriptURL( const OUString& rScriptURL )
{
    sal_Bool result = sal_False;

#ifdef DISABLE_SCRIPTING
    (void) rScriptURL;
#else
    ::com::sun::star::uno::Reference
        < ::com::sun::star::uno::XComponentContext > xContext =
            ::comphelper::getProcessComponentContext();

    ::com::sun::star::uno::Reference
        < ::com::sun::star::uri::XUriReferenceFactory >
            xFactory = ::com::sun::star::uri::UriReferenceFactory::create( xContext );

    try
    {
        ::com::sun::star::uno::Reference
            < ::com::sun::star::uri::XVndSunStarScriptUrl >
                xUrl( xFactory->parse( rScriptURL ),
                    ::com::sun::star::uno::UNO_QUERY );

        if ( xUrl.is() )
        {
            result = sal_True;
        }
    }
    catch (const ::com::sun::star::uno::RuntimeException&)
    {
        // ignore, will just return FALSE
    }
#endif
    return result;
}

OUString
SfxApplication::ChooseScript()
{
    OUString aScriptURL;

#ifndef DISABLE_SCRIPTING
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    if ( pFact )
    {
        SAL_INFO( "sfx.appl", "create selector dialog");

        const SfxViewFrame* pViewFrame = SfxViewFrame::Current();
        const SfxFrame* pFrame = pViewFrame ? &pViewFrame->GetFrame() : NULL;
        uno::Reference< frame::XFrame > xFrame( pFrame ? pFrame->GetFrameInterface() : uno::Reference< frame::XFrame >() );

          AbstractScriptSelectorDialog* pDlg =
            pFact->CreateScriptSelectorDialog( NULL, sal_False, xFrame );

        SAL_INFO( "sfx.appl", "done, now exec it");

          sal_uInt16 nRet = pDlg->Execute();

        SAL_INFO( "sfx.appl", "has returned");

        if ( nRet == RET_OK )
        {
            aScriptURL = pDlg->GetScriptURL();
        }

          delete pDlg;
    }
#endif
    return aScriptURL;
}

void SfxApplication::MacroOrganizer( sal_Int16 nTabId )
{
#ifdef DISABLE_SCRIPTING
    (void) nTabId;
#else

#ifndef DISABLE_DYNLOADING
    // get basctl dllname
    static OUString aLibName( SVLIBRARY( "basctl"  ) );

    // load module
    oslModule handleMod = osl_loadModuleRelative(
        &thisModule, aLibName.pData, 0 );

    // get symbol
    OUString aSymbol( "basicide_macro_organizer"  );
    basicide_macro_organizer pSymbol = (basicide_macro_organizer) osl_getFunctionSymbol( handleMod, aSymbol.pData );

    // call basicide_macro_organizer in basctl
    pSymbol( nTabId );

#else

    basicide_macro_organizer( nTabId );

#endif

#endif
}

ErrCode SfxApplication::CallBasic( const OUString& rCode, BasicManager* pMgr, SbxArray* pArgs, SbxValue* pRet )
{
#ifdef DISABLE_SCRIPTING
    (void) rCode;
    (void) pMgr;
    (void) pArgs;
    (void) pRet;
    return ERRCODE_BASIC_CANNOT_LOAD;
#else
    return pMgr->ExecuteMacro( rCode, pArgs, pRet);
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
