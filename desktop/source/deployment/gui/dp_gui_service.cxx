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


#include "dp_gui_shared.hxx"
#include "dp_gui.h"
#include "dp_gui_theextmgr.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "unotools/configmgr.hxx"
#include "comphelper/processfactory.hxx"
#include "comphelper/servicedecl.hxx"
#include "comphelper/unwrapargs.hxx"
#include <i18nlangtag/languagetag.hxx>
#include "vcl/svapp.hxx"
#include "vcl/msgbox.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/task/XJobExecutor.hpp"
#include "com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp"

#include <boost/optional.hpp>
#include <boost/utility.hpp>
#include "license_dialog.hxx"
#include "dp_gui_dialog2.hxx"
#include "dp_gui_extensioncmdqueue.hxx"

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace dp_gui {

//==============================================================================
class MyApp : public Application, private boost::noncopyable
{
public:
    MyApp();
    virtual ~MyApp();

    // Application
    virtual int Main();
    virtual void DeInit();
};

//______________________________________________________________________________
MyApp::~MyApp()
{
}

//______________________________________________________________________________
MyApp::MyApp()
{
}

//______________________________________________________________________________
int MyApp::Main()
{
    return EXIT_SUCCESS;
}

void MyApp::DeInit()
{
    css::uno::Reference< css::uno::XComponentContext > context(
        comphelper::getProcessComponentContext());
    dp_misc::disposeBridges(context);
    css::uno::Reference< css::lang::XComponent >(
        context, css::uno::UNO_QUERY_THROW)->dispose();
    comphelper::setProcessServiceFactory(0);
}

namespace
{
    struct ProductName
        : public rtl::Static< String, ProductName > {};
    struct Version
        : public rtl::Static< String, Version > {};
    struct AboutBoxVersion
        : public rtl::Static< String, AboutBoxVersion > {};
    struct AboutBoxVersionSuffix
        : public rtl::Static< String, AboutBoxVersionSuffix > {};
    struct OOOVendor
        : public rtl::Static< String, OOOVendor > {};
    struct Extension
        : public rtl::Static< String, Extension > {};
}

OUString ReplaceProductNameHookProc( const OUString& rStr )
{
    if (rStr.indexOf( "%PRODUCT" ) == -1)
        return rStr;

    OUString sProductName = ProductName::get();
    OUString sVersion = Version::get();
    OUString sAboutBoxVersion = AboutBoxVersion::get();
    OUString sAboutBoxVersionSuffix = AboutBoxVersionSuffix::get();
    OUString sExtension = Extension::get();
    OUString sOOOVendor = OOOVendor::get();

    if ( sProductName.isEmpty() )
    {
        sProductName = utl::ConfigManager::getProductName();
        sVersion = utl::ConfigManager::getProductVersion();
        sAboutBoxVersion = utl::ConfigManager::getAboutBoxProductVersion();
        sAboutBoxVersionSuffix = utl::ConfigManager::getAboutBoxProductVersionSuffix();
        sOOOVendor = utl::ConfigManager::getVendor();
        if ( sExtension.isEmpty() )
        {
            sExtension = utl::ConfigManager::getProductExtension();
        }
    }

    OUString sRet = rStr.replaceAll( "%PRODUCTNAME", sProductName );
    sRet = sRet.replaceAll( "%PRODUCTVERSION", sVersion );
    sRet = sRet.replaceAll( "%ABOUTBOXPRODUCTVERSIONSUFFIX", sAboutBoxVersionSuffix );
    sRet = sRet.replaceAll( "%ABOUTBOXPRODUCTVERSION", sAboutBoxVersion );
    sRet = sRet.replaceAll( "%OOOVENDOR", sOOOVendor );
    sRet = sRet.replaceAll( "%PRODUCTEXTENSION", sExtension );
    return sRet;
}

//==============================================================================
class ServiceImpl
    : public ::cppu::WeakImplHelper2<ui::dialogs::XAsynchronousExecutableDialog,
                                     task::XJobExecutor>
{
    Reference<XComponentContext> const m_xComponentContext;
    boost::optional< Reference<awt::XWindow> > /* const */ m_parent;
    boost::optional<OUString> /* const */ m_view;
    /* if true then this service is running in an unopkg process and not in an office process */
    boost::optional<sal_Bool> /* const */ m_unopkg;
    boost::optional<OUString> m_extensionURL;
    OUString m_initialTitle;
    bool m_bShowUpdateOnly;

public:
    ServiceImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XAsynchronousExecutableDialog
    virtual void SAL_CALL setDialogTitle( OUString const & aTitle )
        throw (RuntimeException);
    virtual void SAL_CALL startExecuteModal(
        Reference< ui::dialogs::XDialogClosedListener > const & xListener )
        throw (RuntimeException);

    // XJobExecutor
    virtual void SAL_CALL trigger( OUString const & event )
        throw (RuntimeException);
};

//______________________________________________________________________________
ServiceImpl::ServiceImpl( Sequence<Any> const& args,
                          Reference<XComponentContext> const& xComponentContext)
    : m_xComponentContext(xComponentContext),
      m_bShowUpdateOnly( false )
{
    try {
        comphelper::unwrapArgs( args, m_parent, m_view, m_unopkg );
        return;
    } catch ( const css::lang::IllegalArgumentException & ) {
    }
    try {
        comphelper::unwrapArgs( args, m_extensionURL);
    } catch ( const css::lang::IllegalArgumentException & ) {
    }

    ResHookProc pProc = ResMgr::GetReadStringHook();
    if ( !pProc )
        ResMgr::SetReadStringHook( ReplaceProductNameHookProc );
}

// XAsynchronousExecutableDialog
//______________________________________________________________________________
void ServiceImpl::setDialogTitle( OUString const & title )
    throw (RuntimeException)
{
    if ( dp_gui::TheExtensionManager::s_ExtMgr.is() )
    {
        const SolarMutexGuard guard;
        ::rtl::Reference< ::dp_gui::TheExtensionManager > dialog(
            ::dp_gui::TheExtensionManager::get( m_xComponentContext,
                                                m_parent ? *m_parent : Reference<awt::XWindow>(),
                                                m_extensionURL ? *m_extensionURL : OUString() ) );
        dialog->SetText( title );
    }
    else
        m_initialTitle = title;
}

//______________________________________________________________________________
void ServiceImpl::startExecuteModal(
    Reference< ui::dialogs::XDialogClosedListener > const & xListener )
    throw (RuntimeException)
{
    bool bCloseDialog = true;  // only used if m_bShowUpdateOnly is true
    ::std::auto_ptr<Application> app;
    //ToDo: synchronize access to s_dialog !!!
    if (! dp_gui::TheExtensionManager::s_ExtMgr.is())
    {
        const bool bAppUp = (GetpApp() != 0);
        bool bOfficePipePresent;
        try {
            bOfficePipePresent = dp_misc::office_is_running();
        }
        catch (const Exception & exc) {
            if (bAppUp) {
                const SolarMutexGuard guard;
                std::auto_ptr<ErrorBox> box(
                    new ErrorBox( Application::GetActiveTopWindow(),
                                  WB_OK, exc.Message ) );
                box->Execute();
            }
            throw;
        }

        if (! bOfficePipePresent) {
            OSL_ASSERT( ! bAppUp );
            app.reset( new MyApp );
            if (! InitVCL() )
                throw RuntimeException( "Cannot initialize VCL!",
                                        static_cast<OWeakObject *>(this) );
            AllSettings as = app->GetSettings();
            as.SetUILanguageTag( LanguageTag( utl::ConfigManager::getLocale() ).makeFallback() );
            app->SetSettings( as );
            app->SetDisplayName(
                utl::ConfigManager::getProductName() +
                OUString(" ") +
                utl::ConfigManager::getProductVersion());
            ExtensionCmdQueue::syncRepositories( m_xComponentContext );
        }
    }
    else
    {
        // When m_bShowUpdateOnly is set, we are inside the office and the user clicked
        // the update notification icon in the menu bar. We must not close the extensions
        // dialog after displaying the update dialog when it has been visible before
        if ( m_bShowUpdateOnly )
            bCloseDialog = ! dp_gui::TheExtensionManager::s_ExtMgr->isVisible();
    }

    {
        const SolarMutexGuard guard;
        ::rtl::Reference< ::dp_gui::TheExtensionManager > myExtMgr(
            ::dp_gui::TheExtensionManager::get(
                m_xComponentContext,
                m_parent ? *m_parent : Reference<awt::XWindow>(),
                m_extensionURL ? *m_extensionURL : OUString() ) );
        myExtMgr->createDialog( false );
        if (!m_initialTitle.isEmpty()) {
            myExtMgr->SetText( m_initialTitle );
            m_initialTitle = OUString();
        }
        if ( m_bShowUpdateOnly )
        {
            myExtMgr->checkUpdates( true, !bCloseDialog );
            if ( bCloseDialog )
                myExtMgr->Close();
            else
                myExtMgr->ToTop( TOTOP_RESTOREWHENMIN );
        }
        else
        {
            myExtMgr->Show();
            myExtMgr->ToTop( TOTOP_RESTOREWHENMIN );
        }
    }

    if (app.get() != 0) {
        Application::Execute();
        DeInitVCL();
    }

    if (xListener.is())
        xListener->dialogClosed(
            ui::dialogs::DialogClosedEvent(
                static_cast< ::cppu::OWeakObject * >(this),
                sal_Int16(0)) );
}

// XJobExecutor
//______________________________________________________________________________
void ServiceImpl::trigger( OUString const &rEvent ) throw (RuntimeException)
{
    if ( rEvent == "SHOW_UPDATE_DIALOG" )
        m_bShowUpdateOnly = true;
    else
        m_bShowUpdateOnly = false;

    startExecuteModal( Reference< ui::dialogs::XDialogClosedListener >() );
}

namespace sdecl = comphelper::service_decl;
sdecl::class_<ServiceImpl, sdecl::with_args<true> > serviceSI;
sdecl::ServiceDecl const serviceDecl(
    serviceSI,
    "com.sun.star.comp.deployment.ui.PackageManagerDialog",
    "com.sun.star.deployment.ui.PackageManagerDialog" );

sdecl::class_<LicenseDialog, sdecl::with_args<true> > licenseSI;
sdecl::ServiceDecl const licenseDecl(
    licenseSI,
    "com.sun.star.comp.deployment.ui.LicenseDialog",
    "com.sun.star.deployment.ui.LicenseDialog" );

sdecl::class_<UpdateRequiredDialogService, sdecl::with_args<true> > updateSI;
sdecl::ServiceDecl const updateDecl(
    updateSI,
    "com.sun.star.comp.deployment.ui.UpdateRequiredDialog",
    "com.sun.star.deployment.ui.UpdateRequiredDialog" );
} // namespace dp_gui

extern "C" {

SAL_DLLPUBLIC_EXPORT void * SAL_CALL deploymentgui_component_getFactory(
    sal_Char const * pImplName,
    lang::XMultiServiceFactory * pServiceManager,
    registry::XRegistryKey * pRegistryKey )
{
    return component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, dp_gui::serviceDecl, dp_gui::licenseDecl, dp_gui::updateDecl );
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
