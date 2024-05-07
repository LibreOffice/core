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


#include <memory>
#include "dp_gui_theextmgr.hxx"
#include <osl/diagnose.h>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/configmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/unwrapargs.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/weld.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>

#include <optional>
#include "license_dialog.hxx"
#include "dp_gui_dialog2.hxx"
#include "dp_gui_extensioncmdqueue.hxx"
#include <dp_misc.h>

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace dp_gui {

namespace {

class MyApp : public Application
{
public:
    MyApp();

    MyApp(const MyApp&) = delete;
    const MyApp& operator=(const MyApp&) = delete;

    // Application
    virtual int Main() override;
    virtual void DeInit() override;
};

}

MyApp::MyApp()
{
}


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
    comphelper::setProcessServiceFactory(nullptr);
}

static OUString ReplaceProductNameHookProc( const OUString& rStr )
{
    if (rStr.indexOf( "%PRODUCT" ) == -1)
        return rStr;

    static const OUString sProductName = utl::ConfigManager::getProductName();
    static const OUString sVersion = utl::ConfigManager::getProductVersion();
    static const OUString sAboutBoxVersion = utl::ConfigManager::getAboutBoxProductVersion();
    static const OUString sAboutBoxVersionSuffix = utl::ConfigManager::getAboutBoxProductVersionSuffix();
    static const OUString sExtension = utl::ConfigManager::getProductExtension();
    static const OUString sOOOVendor = utl::ConfigManager::getVendor();

    OUString sRet = rStr.replaceAll( "%PRODUCTNAME", sProductName );
    sRet = sRet.replaceAll( "%PRODUCTVERSION", sVersion );
    sRet = sRet.replaceAll( "%ABOUTBOXPRODUCTVERSIONSUFFIX", sAboutBoxVersionSuffix );
    sRet = sRet.replaceAll( "%ABOUTBOXPRODUCTVERSION", sAboutBoxVersion );
    sRet = sRet.replaceAll( "%OOOVENDOR", sOOOVendor );
    sRet = sRet.replaceAll( "%PRODUCTEXTENSION", sExtension );
    return sRet;
}

namespace {

class ServiceImpl
    : public ::cppu::WeakImplHelper<ui::dialogs::XAsynchronousExecutableDialog,
                                     task::XJobExecutor, css::lang::XServiceInfo>
{
    Reference<XComponentContext> const m_xComponentContext;
    std::optional< Reference<awt::XWindow> > /* const */ m_parent;
    std::optional<OUString> m_extensionURL;
    OUString m_initialTitle;
    bool m_bShowUpdateOnly;

public:
    ServiceImpl( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XAsynchronousExecutableDialog
    virtual void SAL_CALL setDialogTitle( OUString const & aTitle ) override;
    virtual void SAL_CALL startExecuteModal(
        Reference< ui::dialogs::XDialogClosedListener > const & xListener ) override;

    // XJobExecutor
    virtual void SAL_CALL trigger( OUString const & event ) override;
};

}

ServiceImpl::ServiceImpl( Sequence<Any> const& args,
                          Reference<XComponentContext> const& xComponentContext)
    : m_xComponentContext(xComponentContext),
      m_bShowUpdateOnly( false )
{
    /* if true then this service is running in a unopkg process and not in an office process */
    std::optional<OUString> view;
    try {
        std::optional<sal_Bool> unopkg;
        comphelper::unwrapArgs( args, m_parent, view, unopkg );
        return;
    } catch ( const css::lang::IllegalArgumentException & ) {
    }
    try {
        comphelper::unwrapArgs( args, m_extensionURL);
    } catch ( const css::lang::IllegalArgumentException & ) {
    }

    ResHookProc pProc = Translate::GetReadStringHook();
    if ( !pProc )
        Translate::SetReadStringHook(ReplaceProductNameHookProc);
}

// XServiceInfo
OUString ServiceImpl::getImplementationName()
{
    return u"com.sun.star.comp.deployment.ui.PackageManagerDialog"_ustr;
}

sal_Bool ServiceImpl::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > ServiceImpl::getSupportedServiceNames()
{
    return { u"com.sun.star.deployment.ui.PackageManagerDialog"_ustr };
}

// XAsynchronousExecutableDialog

void ServiceImpl::setDialogTitle( OUString const & title )
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


void ServiceImpl::startExecuteModal(
    Reference< ui::dialogs::XDialogClosedListener > const & xListener )
{
    bool bCloseDialog = true;  // only used if m_bShowUpdateOnly is true
    std::unique_ptr<Application> app;
    //ToDo: synchronize access to s_dialog !!!
    if (! dp_gui::TheExtensionManager::s_ExtMgr.is())
    {
        const bool bAppUp = (GetpApp() != nullptr);
        bool bOfficePipePresent;
        try {
            bOfficePipePresent = dp_misc::office_is_running();
        }
        catch (const Exception & exc) {
            if (bAppUp) {
                const SolarMutexGuard guard;
                vcl::Window* pWin = Application::GetActiveTopWindow();
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                          VclMessageType::Warning, VclButtonsType::Ok, exc.Message));
                xBox->run();
            }
            throw;
        }

        if (! bOfficePipePresent) {
            OSL_ASSERT( ! bAppUp );
            app.reset( new MyApp );
            if (! InitVCL() )
                throw RuntimeException( u"Cannot initialize VCL!"_ustr,
                                        static_cast<OWeakObject *>(this) );
            Application::SetDisplayName(
                utl::ConfigManager::getProductName() +
                " " +
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
            m_initialTitle.clear();
        }
        if ( m_bShowUpdateOnly )
        {
            myExtMgr->checkUpdates();
            if ( bCloseDialog )
                myExtMgr->Close();
            else
                myExtMgr->ToTop();
        }
        else
        {
            myExtMgr->Show();
            myExtMgr->ToTop();
        }
    }

    if (app != nullptr)
    {
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

void ServiceImpl::trigger( OUString const &rEvent )
{
    if ( rEvent == "SHOW_UPDATE_DIALOG" )
        m_bShowUpdateOnly = true;
    else
        m_bShowUpdateOnly = false;

    startExecuteModal( Reference< ui::dialogs::XDialogClosedListener >() );
}

} // namespace dp_gui

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
desktop_LicenseDialog_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& args)
{
    return cppu::acquire(new dp_gui::LicenseDialog(args, context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
desktop_ServiceImpl_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& args)
{
    return cppu::acquire(new dp_gui::ServiceImpl(args, context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
desktop_UpdateRequiredDialogService_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& args)
{
    return cppu::acquire(new dp_gui::UpdateRequiredDialogService(args, context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
