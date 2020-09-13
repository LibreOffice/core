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

#pragma once

#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>

#include <osl/mutex.hxx>

#include <rtl/ustring.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace dp_gui {


class ExtBoxWithBtns_Impl;
class ExtensionBox_Impl;
class TheExtensionManager;


class DialogHelper
{
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    weld::Window*   m_pWindow;
    ImplSVEvent *   m_nEventID;
    TopLevelWindowLocker m_aBusy;

public:
                    DialogHelper(const css::uno::Reference< css::uno::XComponentContext > &,
                                 weld::Window* pWindow);
    virtual        ~DialogHelper();

    void            openWebBrowser(const OUString& rURL, const OUString& rTitle);
    weld::Window*   getFrameWeld() const { return m_pWindow; }
    void            PostUserEvent( const Link<void*,void>& rLink, void* pCaller );
    void            clearEventID() { m_nEventID = nullptr; }

    virtual void    showProgress( bool bStart ) = 0;
    virtual void    updateProgress( const OUString &rText,
                                    const css::uno::Reference< css::task::XAbortChannel > &xAbortChannel) = 0;
    virtual void    updateProgress( const long nProgress ) = 0;

    virtual void    updatePackageInfo( const css::uno::Reference< css::deployment::XPackage > &xPackage ) = 0;
    virtual void    addPackageToList( const css::uno::Reference< css::deployment::XPackage > &xPackage,
                                      bool bLicenseMissing = false ) = 0;

    virtual void    prepareChecking() = 0;
    virtual void    checkEntries() = 0;

    static bool     IsSharedPkgMgr( const css::uno::Reference< css::deployment::XPackage > &);
           bool     continueOnSharedExtension( const css::uno::Reference< css::deployment::XPackage > &,
                                               weld::Widget* pParent,
                                               const char* pResID,
                                               bool &bHadWarning );

    void            incBusy() { m_aBusy.incBusy(m_pWindow); }
    void            decBusy() { m_aBusy.decBusy(); }
    bool            isBusy() const { return m_aBusy.isBusy(); }
    bool            installExtensionWarn(const OUString &rExtensionURL);
    bool            installForAllUsers(bool &bInstallForAll);
};

class ExtMgrDialog : public weld::GenericDialogController
                   , public DialogHelper
{
    const OUString       m_sAddPackages;
    OUString             m_sProgressText;
    OUString             m_sLastFolderURL;
    ::osl::Mutex         m_aMutex;
    bool                 m_bHasProgress;
    bool                 m_bProgressChanged;
    bool                 m_bStartProgress;
    bool                 m_bStopProgress;
    bool                 m_bEnableWarning;
    bool                 m_bDisableWarning;
    bool                 m_bDeleteWarning;
    bool                 m_bClosed;
    long                 m_nProgress;
    Idle                 m_aIdle;
    TheExtensionManager *m_pManager;

    css::uno::Reference< css::task::XAbortChannel > m_xAbortChannel;

    std::unique_ptr<ExtBoxWithBtns_Impl> m_xExtensionBox;
    std::unique_ptr<weld::CustomWeld> m_xExtensionBoxWnd;
    std::unique_ptr<weld::Button> m_xOptionsBtn;
    std::unique_ptr<weld::Button> m_xAddBtn;
    std::unique_ptr<weld::Button> m_xRemoveBtn;
    std::unique_ptr<weld::Button> m_xEnableBtn;
    std::unique_ptr<weld::Button> m_xUpdateBtn;
    std::unique_ptr<weld::Button> m_xCloseBtn;
    std::unique_ptr<weld::CheckButton> m_xBundledCbx;
    std::unique_ptr<weld::CheckButton> m_xSharedCbx;
    std::unique_ptr<weld::CheckButton> m_xUserCbx;
    std::unique_ptr<weld::LinkButton> m_xGetExtensions;
    std::unique_ptr<weld::Label> m_xProgressText;
    std::unique_ptr<weld::ProgressBar> m_xProgressBar;
    std::unique_ptr<weld::Button> m_xCancelBtn;

    bool removeExtensionWarn(const OUString &rExtensionTitle);

    DECL_LINK( HandleOptionsBtn, weld::Button&, void );
    DECL_LINK( HandleAddBtn, weld::Button&, void );
    DECL_LINK( HandleRemoveBtn, weld::Button&, void );
    DECL_LINK( HandleEnableBtn, weld::Button&, void );
    DECL_LINK( HandleUpdateBtn, weld::Button&, void );
    DECL_LINK( HandleCancelBtn, weld::Button&, void );
    DECL_LINK( HandleCloseBtn, weld::Button&, void );
    DECL_LINK( HandleExtTypeCbx, weld::Button&, void );
    DECL_LINK( TimeOutHdl, Timer *, void );
    DECL_LINK( startProgress, void *, void );

public:
    ExtMgrDialog(weld::Window * pParent, TheExtensionManager *pManager);
    virtual ~ExtMgrDialog() override;

    virtual void    showProgress( bool bStart ) override;
    virtual void    updateProgress( const OUString &rText,
                                    const css::uno::Reference< css::task::XAbortChannel > &xAbortChannel) override;
    virtual void    updateProgress( const long nProgress ) override;

    virtual void    updatePackageInfo( const css::uno::Reference< css::deployment::XPackage > &xPackage ) override;

    void            setGetExtensionsURL( const OUString &rURL );
    virtual void    addPackageToList( const css::uno::Reference< css::deployment::XPackage > &,
                                      bool bLicenseMissing = false ) override;
    void enablePackage(const css::uno::Reference< css::deployment::XPackage > &xPackage,
                        bool bEnable );
    void removePackage(const css::uno::Reference< css::deployment::XPackage > &xPackage );
    void updatePackage(const css::uno::Reference< css::deployment::XPackage > &xPackage );
    bool acceptLicense(const css::uno::Reference< css::deployment::XPackage > &xPackage );

    void Close();

    TheExtensionManager*    getExtensionManager() const { return m_pManager; }

    virtual void    prepareChecking() override;
    virtual void    checkEntries() override;

    css::uno::Sequence< OUString > raiseAddPicker();

    void enableOptionsButton( bool bEnable );
    void enableRemoveButton( bool bEnable );
    void enableEnableButton( bool bEnable );
    /*
     * Transform the button to "Enable", or to "Disable"
     * based on the value of bEnable.
     */
    void enableButtontoEnable( bool bEnable );
};


class UpdateRequiredDialog : public weld::GenericDialogController
                           , public DialogHelper
{
    const OUString       m_sCloseText;
    OUString             m_sProgressText;
    ::osl::Mutex         m_aMutex;
    bool                 m_bHasProgress;
    bool                 m_bProgressChanged;
    bool                 m_bStartProgress;
    bool                 m_bStopProgress;
    bool                 m_bHasLockedEntries;
    long                 m_nProgress;
    Idle                 m_aIdle;
    TheExtensionManager *m_pManager;

    css::uno::Reference< css::task::XAbortChannel > m_xAbortChannel;

    std::unique_ptr<ExtensionBox_Impl> m_xExtensionBox;
    std::unique_ptr<weld::CustomWeld> m_xExtensionBoxWnd;
    std::unique_ptr<weld::Label> m_xUpdateNeeded;
    std::unique_ptr<weld::Button> m_xUpdateBtn;
    std::unique_ptr<weld::Button> m_xCloseBtn;
    std::unique_ptr<weld::Button> m_xCancelBtn;
    std::unique_ptr<weld::Label> m_xProgressText;
    std::unique_ptr<weld::ProgressBar> m_xProgressBar;

    DECL_LINK( HandleUpdateBtn, weld::Button&, void );
    DECL_LINK( HandleCloseBtn, weld::Button&, void );
    DECL_LINK( HandleCancelBtn, weld::Button&, void );
    DECL_LINK( TimeOutHdl, Timer *, void );
    DECL_LINK( startProgress, void *, void );

    static bool     isEnabled( const css::uno::Reference< css::deployment::XPackage > &xPackage );
    static bool     checkDependencies( const css::uno::Reference< css::deployment::XPackage > &xPackage );
    bool            hasActiveEntries();
    void            disableAllEntries();

public:
    UpdateRequiredDialog(weld::Window * pParent, TheExtensionManager *pManager);
    virtual        ~UpdateRequiredDialog() override;

    virtual short   run() override;

    virtual void    showProgress( bool bStart ) override;
    virtual void    updateProgress( const OUString &rText,
                                    const css::uno::Reference< css::task::XAbortChannel > &xAbortChannel) override;
    virtual void    updateProgress( const long nProgress ) override;

    virtual void    updatePackageInfo( const css::uno::Reference< css::deployment::XPackage > &xPackage ) override;

    virtual void    addPackageToList( const css::uno::Reference< css::deployment::XPackage > &,
                                      bool bLicenseMissing = false ) override;

    virtual void    prepareChecking() override;
    virtual void    checkEntries() override;
};


class ShowLicenseDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::TextView> m_xLicenseText;
public:
    ShowLicenseDialog(weld::Window * pParent, const css::uno::Reference< css::deployment::XPackage > &xPackage);
    virtual ~ShowLicenseDialog() override;
};

class UpdateRequiredDialogService : public ::cppu::WeakImplHelper< css::ui::dialogs::XExecutableDialog, css::lang::XServiceInfo >
{
    css::uno::Reference< css::uno::XComponentContext > const m_xComponentContext;
public:
    UpdateRequiredDialogService( css::uno::Sequence< css::uno::Any > const & args,
                                 css::uno::Reference< css::uno::XComponentContext> const & xComponentContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XExecutableDialog
    virtual void SAL_CALL         setTitle( OUString const & title ) override;
    virtual sal_Int16 SAL_CALL    execute() override;
};

} // namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
