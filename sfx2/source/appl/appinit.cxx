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


#include <sfx2/app.hxx>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/frame/GlobalEventBroadcaster.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <svtools/soerr.hxx>
#include <svtools/svtools.hrc>
#include <unotools/saveopt.hxx>
#include <unotools/localisationoptions.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/ehdl.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/configmgr.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/security.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/historyoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <cppuhelper/implbase2.hxx>

#include <rtl/logfile.hxx>
#include <vcl/edit.hxx>

#include <sfx2/unoctitm.hxx>
#include "app.hrc"
#include "sfxlocal.hrc"
#include "appdata.hxx"
#include "arrdecl.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/mnumgr.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/progress.hxx>
#include "sfx2/sfxhelp.hxx"
#include "sfx2/sfxresid.hxx"
#include "sfxtypes.hxx"
#include <sfx2/viewsh.hxx>
#include "nochaos.hxx"
#include <sfx2/fcontnr.hxx>
#include "helper.hxx"   // SfxContentHelper::Kill()
#include "sfxpicklist.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;
namespace css = ::com::sun::star;

class SfxTerminateListener_Impl : public ::cppu::WeakImplHelper2< XTerminateListener, XServiceInfo >
{
public:

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const EventObject& aEvent ) throw( TerminationVetoException, RuntimeException );
    virtual void SAL_CALL notifyTermination( const EventObject& aEvent ) throw( RuntimeException );
    virtual void SAL_CALL disposing( const EventObject& Source ) throw( RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& sServiceName ) throw (RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);
};

void SAL_CALL SfxTerminateListener_Impl::disposing( const EventObject& ) throw( RuntimeException )
{
}

void SAL_CALL SfxTerminateListener_Impl::queryTermination( const EventObject& ) throw(TerminationVetoException, RuntimeException )
{
    SolarMutexGuard aGuard;
    if ( !SFX_APP()->QueryExit_Impl() )
        throw TerminationVetoException();
}

void SAL_CALL SfxTerminateListener_Impl::notifyTermination( const EventObject& aEvent ) throw(RuntimeException )
{
    Reference< XDesktop > xDesktop( aEvent.Source, UNO_QUERY );
    if( xDesktop.is() == sal_True )
        xDesktop->removeTerminateListener( this );

    SolarMutexGuard aGuard;
    utl::ConfigManager::storeConfigItems();

    SfxApplication* pApp = SFX_APP();
    pApp->Broadcast( SfxSimpleHint( SFX_HINT_DEINITIALIZING ) );
    pApp->Get_Impl()->pAppDispatch->ReleaseAll();
    pApp->Get_Impl()->pAppDispatch->release();

    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    css::uno::Reference< css::document::XEventListener > xGlobalBroadcaster(css::frame::GlobalEventBroadcaster::create(xContext), css::uno::UNO_QUERY_THROW);

    css::document::EventObject aEvent2;
    aEvent2.EventName = "OnCloseApp";
    xGlobalBroadcaster->notifyEvent(aEvent2);

    delete pApp;
    Application::Quit();
}

::rtl::OUString SAL_CALL SfxTerminateListener_Impl::getImplementationName() throw (RuntimeException)
{
    return ::rtl::OUString("com.sun.star.comp.sfx2.SfxTerminateListener");
}

::sal_Bool SAL_CALL SfxTerminateListener_Impl::supportsService( const ::rtl::OUString& sServiceName ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > lNames  = getSupportedServiceNames();
    ::sal_Int32                 c       = lNames.getLength();
    ::sal_Int32                 i       = 0;

    for (i=0; i<c; ++i)
    {
        if (lNames[i].equals(sServiceName))
            return sal_True;
    }

    return sal_False;
}

Sequence< ::rtl::OUString > SAL_CALL SfxTerminateListener_Impl::getSupportedServiceNames() throw (RuntimeException)
{
    // Note: That service  does not realy exists .-)
    // But this implementation is not thought to be registered realy within our service.rdb.
    // At least we need the implementation name only to identify these service at the global desktop instance.
    // The desktop must know, which listener will terminate the SfxApplication in real !
    // It must call this special listener as last one ... otherwise we shutdown the SfxApplication BEFORE other listener
    // can react ...
    static const ::rtl::OUString SERVICENAME("com.sun.star.frame.TerminateListener");
    Sequence< ::rtl::OUString > lNames(1);
    lNames[0] = SERVICENAME;
    return lNames;
}

//====================================================================

typedef bool ( *PFunc_getSpecialCharsForEdit)( Window* i_pParent, const Font& i_rFont, String& o_rOutString );

//====================================================================
// Lazy binding of the GetSpecialCharsForEdit function as it resides in
// a library above us.
//====================================================================

#ifndef DISABLE_DYNLOADING

extern "C" { static void SAL_CALL thisModule() {} }

#else

extern "C" bool GetSpecialCharsForEdit( Window* i_pParent, const Font& i_rFont, String& o_rOutString );

#endif

String GetSpecialCharsForEdit(Window* pParent, const Font& rFont)
{
    static bool bDetermineFunction = false;
    static PFunc_getSpecialCharsForEdit pfunc_getSpecialCharsForEdit = 0;

    SolarMutexGuard aGuard;
    if ( !bDetermineFunction )
    {
        bDetermineFunction = true;

#ifndef DISABLE_DYNLOADING
        static ::rtl::OUString aLibName( SVLIBRARY( "cui"  ) );
        oslModule handleMod = osl_loadModuleRelative(
            &thisModule, aLibName.pData, 0 );

        // get symbol
        ::rtl::OUString aSymbol( "GetSpecialCharsForEdit"  );
        pfunc_getSpecialCharsForEdit = (PFunc_getSpecialCharsForEdit)osl_getFunctionSymbol( handleMod, aSymbol.pData );
        DBG_ASSERT( pfunc_getSpecialCharsForEdit, "GetSpecialCharsForEdit() not found!" );
#else
        pfunc_getSpecialCharsForEdit = GetSpecialCharsForEdit;
#endif
    }

    String aRet;
    if ( pfunc_getSpecialCharsForEdit )
        (*pfunc_getSpecialCharsForEdit)( pParent, rFont, aRet );
    return aRet;
}

//====================================================================

bool SfxApplication::Initialize_Impl()
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxApplication::Initialize_Impl" );

#ifdef TLX_VALIDATE
    StgIo::SetErrorLink( LINK( this, SfxStorageErrHdl, Error ) );
#endif

    Reference < XDesktop > xDesktop ( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
    if (!xDesktop.is())
         throw RuntimeException( rtl::OUString( "Couldn't create mandatory desktop service!" ), xDesktop );
    xDesktop->addTerminateListener( new SfxTerminateListener_Impl() );

    Application::EnableAutoHelpId();

    pAppData_Impl->pAppDispatch = new SfxStatusDispatcher;
    pAppData_Impl->pAppDispatch->acquire();

    // SV-Look
    Help::EnableContextHelp();
    Help::EnableExtHelp();

    SvtLocalisationOptions aLocalisation;
    Application::EnableAutoMnemonic ( aLocalisation.IsAutoMnemonic() );
    Application::SetDialogScaleX    ( (short)(aLocalisation.GetDialogScale()) );


#ifdef DBG_UTIL
    // The SimplerErrorHandler is for debugging. In the Product errors
    // not processed are given to SFX as Errorcode 1.
    pAppData_Impl->m_pSimpleErrorHdl = new SimpleErrorHandler;
#endif
    pAppData_Impl->m_pToolsErrorHdl = new SfxErrorHandler(
        RID_ERRHDL, ERRCODE_AREA_TOOLS, ERRCODE_AREA_LIB1);

    pAppData_Impl->pBasicResMgr = CreateResManager("sb");
    pAppData_Impl->pSvtResMgr = CreateResManager("svt");

    pAppData_Impl->m_pSoErrorHdl = new SfxErrorHandler(
        RID_SO_ERROR_HANDLER, ERRCODE_AREA_SO, ERRCODE_AREA_SO_END, pAppData_Impl->pSvtResMgr );
    pAppData_Impl->m_pSbxErrorHdl = new SfxErrorHandler(
        RID_BASIC_START, ERRCODE_AREA_SBX, ERRCODE_AREA_SBX_END, pAppData_Impl->pBasicResMgr );

    //ensure instantiation of listener that manages the internal recently-used
    //list
    SfxPickList::ensure();

    DBG_ASSERT( !pAppData_Impl->pAppDispat, "AppDispatcher already exists" );
    pAppData_Impl->pAppDispat = new SfxDispatcher((SfxDispatcher*)0);
    pAppData_Impl->pSlotPool = new SfxSlotPool;
    pAppData_Impl->pTbxCtrlFac = new SfxTbxCtrlFactArr_Impl;
    pAppData_Impl->pStbCtrlFac = new SfxStbCtrlFactArr_Impl;
    pAppData_Impl->pMenuCtrlFac = new SfxMenuCtrlFactArr_Impl;
    pAppData_Impl->pViewFrames = new SfxViewFrameArr_Impl;
    pAppData_Impl->pViewShells = new SfxViewShellArr_Impl;
    pAppData_Impl->pObjShells = new SfxObjectShellArr_Impl;
    pAppData_Impl->nInterfaces = SFX_INTERFACE_APP+8;
    pAppData_Impl->pInterfaces = new SfxInterface*[pAppData_Impl->nInterfaces];
    memset( pAppData_Impl->pInterfaces, 0, sizeof(SfxInterface*) * pAppData_Impl->nInterfaces );

    Registrations_Impl();

    // Subklasse initialisieren
    pAppData_Impl->bDowning = sal_False;
    Init();

    // get CHAOS item pool...
    pAppData_Impl->pPool = NoChaos::GetItemPool();
    SetPool( pAppData_Impl->pPool );

    if ( pAppData_Impl->bDowning )
        return sal_False;

    // App-Dispatcher aufbauen
    pAppData_Impl->pAppDispat->Push(*this);
    pAppData_Impl->pAppDispat->Flush();
    pAppData_Impl->pAppDispat->DoActivate_Impl( sal_True, NULL );

    {
        SolarMutexGuard aGuard;
        // Set special characters callback on vcl edit control
        Edit::SetGetSpecialCharsFunction(&GetSpecialCharsForEdit);
    }

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
