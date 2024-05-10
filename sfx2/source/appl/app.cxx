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

#include <sal/log.hxx>
#include <tools/debug.hxx>

#include <sfx2/app.hxx>
#include <sfx2/frame.hxx>
#include <basic/sberrors.hxx>

#include <svl/svdde.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <basic/basmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/sfxhelp.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <appdata.hxx>
#include <sfx2/module.hxx>
#include <sfx2/event.hxx>
#include <workwin.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/stbitem.hxx>
#include <sfx2/dockwin.hxx>

#include <officecfg/Office/Common.hxx>
#include <rtl/strbuf.hxx>
#include <memory>
#include <mutex>
#include <framework/sfxhelperfunctions.hxx>
#include <fwkhelper.hxx>

#include "getbasctlfunction.hxx"

using namespace ::com::sun::star;

static SfxApplication* g_pSfxApplication = nullptr;

#if HAVE_FEATURE_XMLHELP
static SfxHelp*        pSfxHelp = nullptr;
#endif

SfxApplication* SfxApplication::Get()
{
    return g_pSfxApplication;
}

void SfxApplication::SetModule(SfxToolsModule nSharedLib, std::unique_ptr<SfxModule> pModule)
{
    assert(g_pSfxApplication != nullptr);

    g_pSfxApplication->pImpl->aModules[nSharedLib] = std::move(pModule);
}

SfxModule* SfxApplication::GetModule(SfxToolsModule nSharedLib)
{
    if (!g_pSfxApplication) // It is possible GetModule is called before SfxApplication is initialised via GetOrCreate()
        return nullptr;
    return g_pSfxApplication->pImpl->aModules[nSharedLib].get();
}

SfxApplication* SfxApplication::GetOrCreate()
{
    static std::mutex theApplicationMutex;

    // SFX on demand
    std::unique_lock aGuard(theApplicationMutex);
    if (!g_pSfxApplication)
    {
        SAL_INFO( "sfx.appl", "SfxApplication::SetApp" );

        g_pSfxApplication = new SfxApplication;

        // at the moment a bug may occur when Initialize_Impl returns FALSE,
        // but this is only temporary because all code that may cause such
        // a fault will be moved outside the SFX
        g_pSfxApplication->Initialize_Impl();

        ::framework::SetRefreshToolbars( RefreshToolbars );
        ::framework::SetToolBoxControllerCreator( SfxToolBoxControllerFactory );
        ::framework::SetStatusBarControllerCreator( SfxStatusBarControllerFactory );
        ::framework::SetDockingWindowCreator( SfxDockingWindowFactory );
        ::framework::SetIsDockingWindowVisible( IsDockingWindowVisible );
#if HAVE_FEATURE_XMLHELP
        Application::SetHelp( pSfxHelp );
#endif
#if HAVE_FEATURE_XMLHELP || defined(EMSCRIPTEN)
        bool bHelpTip = officecfg::Office::Common::Help::Tip::get();
        bool bExtendedHelpTip = officecfg::Office::Common::Help::ExtendedTip::get();
        if (bHelpTip)
            Help::EnableQuickHelp();
        else
            Help::DisableQuickHelp();
        if (bHelpTip && bExtendedHelpTip)
            Help::EnableBalloonHelp();
        else
            Help::DisableBalloonHelp();
#endif
    }
    return g_pSfxApplication;
}

SfxApplication::SfxApplication()
    : pImpl( new SfxAppData_Impl )
{
    SetName( u"StarOffice"_ustr );

    SAL_INFO( "sfx.appl", "{ initialize DDE" );

    bool bOk = InitializeDde();

#ifdef DBG_UTIL
    if( !bOk )
    {
        OStringBuffer aStr("No DDE-Service possible. Error: ");
        if( GetDdeService() )
            aStr.append(static_cast<sal_Int32>(GetDdeService()->GetError()));
        else
            aStr.append('?');
        SAL_WARN( "sfx.appl", aStr.getStr() );
    }
#else
    (void)bOk;
#endif

#if HAVE_FEATURE_XMLHELP
    pSfxHelp = new SfxHelp;
#endif

#if HAVE_FEATURE_SCRIPTING
    StarBASIC::SetGlobalErrorHdl( LINK( this, SfxApplication, GlobalBasicErrorHdl_Impl ) );
#endif

    SAL_INFO( "sfx.appl", "} initialize DDE" );
}

SfxApplication::~SfxApplication()
{
    SAL_WARN_IF(!GetObjectShells_Impl().empty(), "sfx.appl", "Memory leak: some object shells were not removed!");

    Broadcast( SfxHint(SfxHintId::Dying) );

    for (auto &module : pImpl->aModules)    // Clear modules
        module.reset();

#if HAVE_FEATURE_XMLHELP
    delete pSfxHelp;
    Application::SetHelp();
#endif

    if ( !pImpl->bDowning )
        Deinitialize();

    g_pSfxApplication = nullptr;
}


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
    return pImpl->aLastDir;
}

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
    pImpl->aLastDir = rNewDir;
}


void SfxApplication::ResetLastDir()
{
    pImpl->aLastDir.clear();
}


SfxDispatcher* SfxApplication::GetDispatcher_Impl()
{
    return pImpl->pViewFrame ? pImpl->pViewFrame->GetDispatcher() : &*pImpl->pAppDispat;
}


void SfxApplication::SetViewFrame_Impl( SfxViewFrame *pFrame )
{
    if ( pFrame != pImpl->pViewFrame )
    {
        SfxViewFrame *pOldFrame = pImpl->pViewFrame;

        // DocWinActivate : both frames belong to the same TopWindow
        // TopWinActivate : both frames belong to different TopWindows

        bool bTaskActivate = pOldFrame != pFrame;

        if ( pOldFrame )
        {
            if ( bTaskActivate )
                NotifyEvent( SfxViewEventHint( SfxEventHintId::DeactivateDoc, GlobalEventConfig::GetEventName(GlobalEventId::DEACTIVATEDOC), pOldFrame->GetObjectShell(), pOldFrame->GetFrame().GetController() ) );

            pOldFrame->DoDeactivate( bTaskActivate, pFrame );

            if( pOldFrame->GetProgress() )
                pOldFrame->GetProgress()->Suspend();
        }

        pImpl->pViewFrame = pFrame;

        if( pFrame )
        {
            pFrame->DoActivate( bTaskActivate );
            if ( bTaskActivate && pFrame->GetObjectShell() )
            {
                pFrame->GetObjectShell()->PostActivateEvent_Impl( pFrame );
                NotifyEvent(SfxViewEventHint(SfxEventHintId::ActivateDoc, GlobalEventConfig::GetEventName(GlobalEventId::ACTIVATEDOC), pFrame->GetObjectShell(), pFrame->GetFrame().GetController() ) );
            }

            SfxProgress *pProgress = pFrame->GetProgress();
            if ( pProgress )
            {
                if( pProgress->IsSuspended() )
                    pProgress->Resume();
                else
                    pProgress->SetState( pProgress->GetState() );
            }

            if ( pImpl->pViewFrame->GetViewShell() )
            {
                SfxDispatcher* pDisp = pImpl->pViewFrame->GetDispatcher();
                pDisp->Flush();
                pDisp->Update_Impl(true);
            }
        }
    }

    // even if the frame actually didn't change, ensure its document is forwarded
    // to SfxObjectShell::SetCurrentComponent.
    // Otherwise, the CurrentComponent might not be correct, in case it has meanwhile
    // been reset to some other document, by some non-SFX component. #i49133#
    if ( pFrame && pFrame->GetViewShell() )
        pFrame->GetViewShell()->SetCurrentDocument();
}

void SfxApplication::SetProgress_Impl
(
    SfxProgress *pProgress
)
{
    DBG_ASSERT( ( !pImpl->pProgress && pProgress ) ||
                ( pImpl->pProgress && !pProgress ),
                "Progress activation/deactivation mismatch" );

    if ( pImpl->pProgress && pProgress )
    {
        pImpl->pProgress->Suspend();
        delete pImpl->pProgress;
    }

    pImpl->pProgress = pProgress;
}


sal_uInt16 SfxApplication::GetFreeIndex()
{
    return pImpl->aIndexBitSet.GetFreeIndex()+1;
}


void SfxApplication::ReleaseIndex(sal_uInt16 i)
{
    pImpl->aIndexBitSet.ReleaseIndex(i-1);
}


weld::Window* SfxApplication::GetTopWindow() const
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( SfxViewFrame::Current() );
    if (!pWork)
        return nullptr;
    vcl::Window* pWindow = pWork->GetWindow();
    if (!pWindow)
        return nullptr;
    return pWindow->GetFrameWeld();
}

SfxTbxCtrlFactory* SfxApplication::GetTbxCtrlFactory(const std::type_info& rSlotType, sal_uInt16 nSlotID) const
{
    // search for a factory with the given slot id
    for (auto& rFactory : pImpl->maTbxCtrlFactories)
        if( rFactory.nTypeId == rSlotType && rFactory.nSlotId == nSlotID )
            return &rFactory;

    // if no factory exists for the given slot id, see if we
    // have a generic factory with the correct slot type and slot id == 0
    for (auto& rFactory : pImpl->maTbxCtrlFactories)
        if( rFactory.nTypeId == rSlotType && rFactory.nSlotId == 0 )
            return &rFactory;

    return nullptr;
}

SfxStbCtrlFactory* SfxApplication::GetStbCtrlFactory(const std::type_info& rSlotType, sal_uInt16 nSlotID) const
{
    for (auto& rFactory : pImpl->maStbCtrlFactories)
        if ( rFactory.nTypeId == rSlotType &&
             ( rFactory.nSlotId == 0 || rFactory.nSlotId == nSlotID ) )
            return &rFactory;
    return nullptr;
}

std::vector<SfxViewFrame*>& SfxApplication::GetViewFrames_Impl() const
{
    return pImpl->maViewFrames;
}

std::vector<SfxViewShell*>& SfxApplication::GetViewShells_Impl() const
{
    return pImpl->maViewShells;
}

std::unordered_map<OUString, css::uno::Reference<css::ui::XAcceleratorConfiguration>>& SfxApplication::GetAcceleratorConfs_Impl() const
{
    return pImpl->maAcceleratorConfs;
}

std::vector<SfxObjectShell*>& SfxApplication::GetObjectShells_Impl() const
{
    return pImpl->maObjShells;
}

void SfxApplication::Invalidate( sal_uInt16 nId )
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst(); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame ) )
        Invalidate_Impl( pFrame->GetBindings(), nId );
}

#if HAVE_FEATURE_SCRIPTING

#ifndef DISABLE_DYNLOADING

typedef long (*basicide_handle_basic_error)(void const *);
typedef void (*basicide_macro_organizer)(void *, void *, sal_Int16);

#else

extern "C" long basicide_handle_basic_error(void const*);
extern "C" void basicide_macro_organizer(void*, void*, sal_Int16);

#endif

#endif

IMPL_STATIC_LINK( SfxApplication, GlobalBasicErrorHdl_Impl, StarBASIC*, pStarBasic, bool )
{
#if !HAVE_FEATURE_SCRIPTING
    (void) pStarBasic;
    return false;
#else

    if (comphelper::LibreOfficeKit::isActive())
    {
        OUString aError;
        ErrCodeMsg nErr = StarBASIC::GetErrorCode();
        if (ErrorStringFactory::CreateString(nErr, aError))
        {
            const SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            std::shared_ptr<weld::MessageDialog> xBox;
            xBox.reset(Application::CreateMessageDialog(
                           pViewFrame ? pViewFrame->GetFrameWeld() : nullptr,
                           VclMessageType::Error,
                           VclButtonsType::Ok,
                           aError,
                           GetpApp()));

            xBox->runAsync(xBox, [](sal_Int32 /*nResult*/) {});
        }
        return true;
    }

#ifndef DISABLE_DYNLOADING
    basicide_handle_basic_error pSymbol = reinterpret_cast<basicide_handle_basic_error>(sfx2::getBasctlFunction("basicide_handle_basic_error"));

    // call basicide_handle_basic_error in basctl
    bool bRet = pSymbol( pStarBasic );

#else

    bool bRet = basicide_handle_basic_error( pStarBasic );

#endif

    return bRet;

#endif
}

bool SfxApplication::IsXScriptURL( const OUString& rScriptURL )
{
    bool result = false;

#if !HAVE_FEATURE_SCRIPTING
    (void) rScriptURL;
#else
    css::uno::Reference< css::uno::XComponentContext > xContext =
            ::comphelper::getProcessComponentContext();

    css::uno::Reference< css::uri::XUriReferenceFactory >
            xFactory = css::uri::UriReferenceFactory::create( xContext );

    try
    {
        css::uno::Reference< css::uri::XVndSunStarScriptUrl >
                xUrl( xFactory->parse( rScriptURL ),  css::uno::UNO_QUERY );

        if ( xUrl.is() )
        {
            result = true;
        }
    }
    catch (const css::uno::RuntimeException&)
    {
        // ignore, will just return FALSE
    }
#endif
    return result;
}

OUString
SfxApplication::ChooseScript(weld::Window *pParent)
{
    OUString aScriptURL;

#if HAVE_FEATURE_SCRIPTING
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    SAL_INFO( "sfx.appl", "create selector dialog");

    const SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    const SfxFrame* pFrame = pViewFrame ? &pViewFrame->GetFrame() : nullptr;
    uno::Reference< frame::XFrame > xFrame( pFrame ? pFrame->GetFrameInterface() : uno::Reference< frame::XFrame >() );

    ScopedVclPtr<AbstractScriptSelectorDialog> pDlg(pFact->CreateScriptSelectorDialog(pParent, xFrame));

    SAL_INFO( "sfx.appl", "done, now exec it");

    sal_uInt16 nRet = pDlg->Execute();

    SAL_INFO( "sfx.appl", "has returned");

    if ( nRet == RET_OK )
    {
        aScriptURL = pDlg->GetScriptURL();
    }
#else
    (void) pParent;
#endif
    return aScriptURL;
}

void SfxApplication::MacroOrganizer(weld::Window* pParent, const uno::Reference<frame::XFrame>& xDocFrame, sal_Int16 nTabId)
{
#if !HAVE_FEATURE_SCRIPTING
    (void) pParent;
    (void) nTabId;
    (void) xDocFrame;
#else

#ifndef DISABLE_DYNLOADING
    basicide_macro_organizer pSymbol = reinterpret_cast<basicide_macro_organizer>(sfx2::getBasctlFunction("basicide_macro_organizer"));

    // call basicide_macro_organizer in basctl
    pSymbol(pParent, xDocFrame.get(), nTabId);

#else

    basicide_macro_organizer(pParent, xDocFrame.get(), nTabId);

#endif

#endif
}

ErrCode SfxApplication::CallBasic( const OUString& rCode, BasicManager* pMgr, SbxArray* pArgs, SbxValue* pRet )
{
#if !HAVE_FEATURE_SCRIPTING
    (void) rCode;
    (void) pMgr;
    (void) pArgs;
    (void) pRet;
    return ERRCODE_BASIC_CANNOT_LOAD;
#else
    (void) ERRCODE_BASIC_CANNOT_LOAD; // So that the !HAVE_FEATURE_SCRIPTING case isn't broken again by IWYU
    return pMgr->ExecuteMacro( rCode, pArgs, pRet);
#endif
}

sfx2::sidebar::Theme & SfxApplication::GetSidebarTheme()
{
    if (!pImpl->m_pSidebarTheme.is())
    {
        pImpl->m_pSidebarTheme.set(new sfx2::sidebar::Theme);
        pImpl->m_pSidebarTheme->InitializeTheme();
    }
    return *pImpl->m_pSidebarTheme;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
