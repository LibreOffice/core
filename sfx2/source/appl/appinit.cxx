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

#include <sfx2/app.hxx>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <basic/sbdef.hxx>
#include <svtools/soerr.hxx>
#include <svtools/strings.hrc>
#include <tools/simplerm.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/saveopt.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/ehdl.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/security.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/historyoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <vcl/edit.hxx>
#include <vcl/scheduler.hxx>

#include <sfx2/unoctitm.hxx>
#include "sfx2/strings.hrc"
#include "sfx2/strings.hrc"
#include "appdata.hxx"
#include "arrdecl.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/sfxhelp.hxx>
#include <sfx2/sfxresid.hxx>
#include "sfxtypes.hxx"
#include <sfx2/viewsh.hxx>
#include "nochaos.hxx"
#include <sfx2/fcontnr.hxx>
#include "helper.hxx"
#include "sfxpicklist.hxx"
#include <ctrlfactoryimpl.hxx>
#include <shellimpl.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;

class SfxTerminateListener_Impl : public ::cppu::WeakImplHelper< XTerminateListener, XServiceInfo >
{
public:

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const EventObject& aEvent ) override;
    virtual void SAL_CALL notifyTermination( const EventObject& aEvent ) override;
    virtual void SAL_CALL disposing( const EventObject& Source ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

void SAL_CALL SfxTerminateListener_Impl::disposing( const EventObject& )
{
}

void SAL_CALL SfxTerminateListener_Impl::queryTermination( const EventObject& )
{
}

void SAL_CALL SfxTerminateListener_Impl::notifyTermination( const EventObject& aEvent )
{
    Reference< XDesktop > xDesktop( aEvent.Source, UNO_QUERY );
    if( xDesktop.is() )
        xDesktop->removeTerminateListener( this );

    SolarMutexGuard aGuard;
    utl::ConfigManager::storeConfigItems();

    // Timers may access the SfxApplication and are only deleted in
    // Application::Quit(), which is asynchronous (PostUserEvent) - disable!
    Scheduler::ImplDeInitScheduler();

    SfxApplication* pApp = SfxGetpApp();
    pApp->Broadcast( SfxHint( SfxHintId::Deinitializing ) );
    pApp->Get_Impl()->mxAppDispatch->ReleaseAll();
    pApp->Get_Impl()->mxAppDispatch.clear();

    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    css::uno::Reference< css::document::XDocumentEventListener > xGlobalBroadcaster(css::frame::theGlobalEventBroadcaster::get(xContext), css::uno::UNO_QUERY_THROW);

    css::document::DocumentEvent aEvent2;
    aEvent2.EventName = "OnCloseApp";
    xGlobalBroadcaster->documentEventOccured(aEvent2);

    delete pApp;
    Application::Quit();
}

OUString SAL_CALL SfxTerminateListener_Impl::getImplementationName()
{
    return OUString("com.sun.star.comp.sfx2.SfxTerminateListener");
}

sal_Bool SAL_CALL SfxTerminateListener_Impl::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

Sequence< OUString > SAL_CALL SfxTerminateListener_Impl::getSupportedServiceNames()
{
    // Note: That service  does not really exists .-)
    // But this implementation is not thought to be registered really within our service.rdb.
    // At least we need the implementation name only to identify these service at the global desktop instance.
    // The desktop must know, which listener will terminate the SfxApplication in real !
    // It must call this special listener as last one ... otherwise we shutdown the SfxApplication BEFORE other listener
    // can react ...
    Sequence< OUString > lNames { "com.sun.star.frame.TerminateListener" };
    return lNames;
}


typedef bool ( *PFunc_getSpecialCharsForEdit)( vcl::Window* i_pParent, const vcl::Font& i_rFont, OUString& o_rOutString );


// Lazy binding of the GetSpecialCharsForEdit function as it resides in
// a library above us.


#ifndef DISABLE_DYNLOADING

extern "C" { static void SAL_CALL thisModule() {} }

#else

extern "C" bool GetSpecialCharsForEdit( vcl::Window* i_pParent, const vcl::Font& i_rFont, OUString& o_rOutString );

#endif

OUString GetSpecialCharsForEdit(vcl::Window* pParent, const vcl::Font& rFont)
{
    static bool bDetermineFunction = false;
    static PFunc_getSpecialCharsForEdit pfunc_getSpecialCharsForEdit = nullptr;

    SolarMutexGuard aGuard;
    if ( !bDetermineFunction )
    {
        bDetermineFunction = true;

#ifndef DISABLE_DYNLOADING
        osl::Module aMod;
        aMod.loadRelative(&thisModule, SVLIBRARY("cui"));

        // get symbol
        pfunc_getSpecialCharsForEdit = reinterpret_cast<PFunc_getSpecialCharsForEdit>(aMod.getFunctionSymbol("GetSpecialCharsForEdit"));
        DBG_ASSERT( pfunc_getSpecialCharsForEdit, "GetSpecialCharsForEdit() not found!" );
        aMod.release();
#else
        pfunc_getSpecialCharsForEdit = GetSpecialCharsForEdit;
#endif
    }

    OUString aRet;
    if ( pfunc_getSpecialCharsForEdit )
        (*pfunc_getSpecialCharsForEdit)( pParent, rFont, aRet );
    return aRet;
}


void SfxApplication::Initialize_Impl()
{
#ifdef TLX_VALIDATE
    StgIo::SetErrorLink( LINK( this, SfxStorageErrHdl, Error ) );
#endif

    Reference < XDesktop2 > xDesktop = Desktop::create ( ::comphelper::getProcessComponentContext() );
    xDesktop->addTerminateListener( new SfxTerminateListener_Impl );

    pImpl->mxAppDispatch = new SfxStatusDispatcher;

    // SV-Look
    Help::EnableContextHelp();
    Help::EnableExtHelp();

    pImpl->m_pToolsErrorHdl = new SfxErrorHandler(
        getRID_ERRHDL(), ErrCode(ERRCODE_AREA_IO), ErrCode(ERRCODE_AREA_SVX));

#if HAVE_FEATURE_SCRIPTING
    pImpl->aBasicResLocale = Translate::Create("sb", Application::GetSettings().GetUILanguageTag());
#endif
    pImpl->aSvtResLocale = Translate::Create("svt", Application::GetSettings().GetUILanguageTag());

    pImpl->m_pSoErrorHdl = new SfxErrorHandler(
        getRID_SO_ERROR_HANDLER(), ErrCode(ERRCODE_AREA_SO), ErrCode(ERRCODE_AREA_SO_END), &(pImpl->aSvtResLocale));
#if HAVE_FEATURE_SCRIPTING
    pImpl->m_pSbxErrorHdl = new SfxErrorHandler(
        getRID_BASIC_START(), ErrCode(ERRCODE_AREA_SBX), ErrCode(ERRCODE_AREA_SBX_END), &(pImpl->aBasicResLocale));
#endif

    if (!utl::ConfigManager::IsAvoidConfig())
    {
        //ensure instantiation of listener that manages the internal recently-used
        //list
        SfxPickList::ensure();
    }

    DBG_ASSERT( !pImpl->pAppDispat, "AppDispatcher already exists" );
    pImpl->pAppDispat = new SfxDispatcher;
    pImpl->pSlotPool = new SfxSlotPool;
    pImpl->pTbxCtrlFac = new SfxTbxCtrlFactArr_Impl;
    pImpl->pStbCtrlFac = new SfxStbCtrlFactArr_Impl;
    pImpl->pViewFrames = new SfxViewFrameArr_Impl;
    pImpl->pViewShells = new SfxViewShellArr_Impl;
    pImpl->pObjShells = new SfxObjectShellArr_Impl;

    Registrations_Impl();

    // Subklasse initialisieren
    pImpl->bDowning = false;

    // get CHAOS item pool...
    pImpl->pPool = NoChaos::GetItemPool();
    SetPool( pImpl->pPool );

    if ( pImpl->bDowning )
        return;

    // App-Dispatcher aufbauen
    pImpl->pAppDispat->Push(*this);
    pImpl->pAppDispat->Flush();
    pImpl->pAppDispat->DoActivate_Impl( true );

    {
        SolarMutexGuard aGuard;
        // Set special characters callback on vcl edit control
        Edit::SetGetSpecialCharsFunction(&GetSpecialCharsForEdit);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
