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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_DIALOG2_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_DIALOG2_HXX

#include <config_extension_update.h>

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/fixedhyper.hxx>
#include <vcl/prgsbar.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>

#include <svtools/svmedit.hxx>

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

namespace dp_gui {


class ExtBoxWithBtns_Impl;
class ExtensionBox_Impl;
class TheExtensionManager;


class DialogHelper
{
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    VclPtr<Dialog>  m_pVCLWindow;
    ImplSVEvent *   m_nEventID;
    bool            m_bIsBusy;

public:
                    DialogHelper( const css::uno::Reference< css::uno::XComponentContext > &,
                                  Dialog *pWindow );
    virtual        ~DialogHelper();

    void            openWebBrowser( const OUString & sURL, const OUString & sTitle ) const;
    Dialog*         getWindow() const { return m_pVCLWindow; };
    void            PostUserEvent( const Link<void*,void>& rLink, void* pCaller, bool bReferenceLink = false );
    void            clearEventID() { m_nEventID = nullptr; }

    virtual void    showProgress( bool bStart ) = 0;
    virtual void    updateProgress( const OUString &rText,
                                    const css::uno::Reference< css::task::XAbortChannel > &xAbortChannel) = 0;
    virtual void    updateProgress( const long nProgress ) = 0;

    virtual void    updatePackageInfo( const css::uno::Reference< css::deployment::XPackage > &xPackage ) = 0;
    virtual long    addPackageToList( const css::uno::Reference< css::deployment::XPackage > &xPackage,
                                      bool bLicenseMissing = false ) = 0;

    virtual void    prepareChecking() = 0;
    virtual void    checkEntries() = 0;

    static ResId    getResId( sal_uInt16 nId );
    static OUString getResourceString( sal_uInt16 id );
    static bool     IsSharedPkgMgr( const css::uno::Reference< css::deployment::XPackage > &);
    static bool     continueOnSharedExtension( const css::uno::Reference< css::deployment::XPackage > &,
                                               vcl::Window *pParent,
                                               const sal_uInt16 nResID,
                                               bool &bHadWarning );

    void            setBusy( const bool bBusy ) { m_bIsBusy = bBusy; }
    bool            isBusy() const { return m_bIsBusy; }
    bool            installExtensionWarn( const OUString &rExtensionURL ) const;
    bool            installForAllUsers( bool &bInstallForAll ) const;
};


class ExtMgrDialog : public ModelessDialog,
                     public DialogHelper
{
    VclPtr<ExtBoxWithBtns_Impl> m_pExtensionBox;
    VclPtr<PushButton>          m_pAddBtn;
    VclPtr<PushButton>          m_pUpdateBtn;
    VclPtr<CloseButton>         m_pCloseBtn;
    VclPtr<CheckBox>            m_pBundledCbx;
    VclPtr<CheckBox>            m_pSharedCbx;
    VclPtr<CheckBox>            m_pUserCbx;
    VclPtr<FixedHyperlink>      m_pGetExtensions;
    VclPtr<FixedText>           m_pProgressText;
    VclPtr<ProgressBar>         m_pProgressBar;
    VclPtr<CancelButton>        m_pCancelBtn;
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
    long                 m_nProgress;
    Idle                 m_aIdle;
    TheExtensionManager *m_pManager;

    css::uno::Reference< css::task::XAbortChannel > m_xAbortChannel;

    bool removeExtensionWarn( const OUString &rExtensionTitle ) const;

    DECL_DLLPRIVATE_LINK_TYPED( HandleAddBtn, Button*, void );
    DECL_DLLPRIVATE_LINK_TYPED( HandleUpdateBtn, Button*, void );
    DECL_DLLPRIVATE_LINK_TYPED( HandleCancelBtn, Button*, void );
    DECL_DLLPRIVATE_LINK_TYPED( HandleCloseBtn, Button*, void );
    DECL_DLLPRIVATE_LINK_TYPED( HandleExtTypeCbx, Button*, void );
    DECL_DLLPRIVATE_LINK_TYPED( HandleHyperlink, FixedHyperlink&, void );
    DECL_DLLPRIVATE_LINK_TYPED(TimeOutHdl, Idle *, void);
    DECL_DLLPRIVATE_LINK_TYPED( startProgress, void *, void );

public:
                    ExtMgrDialog( vcl::Window * pParent, TheExtensionManager *pManager, Dialog::InitFlag eFlag = Dialog::InitFlag::Default );
    virtual        ~ExtMgrDialog();
    virtual void    dispose() override;

    virtual bool    Notify( NotifyEvent& rNEvt ) override;
    virtual bool    Close() override;

    virtual void    showProgress( bool bStart ) override;
    virtual void    updateProgress( const OUString &rText,
                                    const css::uno::Reference< css::task::XAbortChannel > &xAbortChannel) override;
    virtual void    updateProgress( const long nProgress ) override;

    virtual void    updatePackageInfo( const css::uno::Reference< css::deployment::XPackage > &xPackage ) override;

    void            setGetExtensionsURL( const OUString &rURL );
    virtual long    addPackageToList( const css::uno::Reference< css::deployment::XPackage > &,
                                      bool bLicenseMissing = false ) override;
    bool enablePackage(const css::uno::Reference< css::deployment::XPackage > &xPackage,
                        bool bEnable );
    bool removePackage(const css::uno::Reference< css::deployment::XPackage > &xPackage );
    bool updatePackage(const css::uno::Reference< css::deployment::XPackage > &xPackage );
    bool acceptLicense(const css::uno::Reference< css::deployment::XPackage > &xPackage );

    TheExtensionManager*    getExtensionManager() const { return m_pManager; }

    virtual void    prepareChecking() override;
    virtual void    checkEntries() override;

    css::uno::Sequence< OUString > raiseAddPicker();
};


class UpdateRequiredDialog : public ModalDialog,
                             public DialogHelper
{
    VclPtr<ExtensionBox_Impl>   m_pExtensionBox;
    VclPtr<FixedText>           m_pUpdateNeeded;
    VclPtr<PushButton>          m_pUpdateBtn;
    VclPtr<PushButton>          m_pCloseBtn;
    VclPtr<CancelButton>        m_pCancelBtn;
    VclPtr<FixedText>           m_pProgressText;
    VclPtr<ProgressBar>         m_pProgressBar;
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

    DECL_DLLPRIVATE_LINK_TYPED( HandleUpdateBtn, Button*, void );
    DECL_DLLPRIVATE_LINK_TYPED( HandleCloseBtn, Button*, void );
    DECL_DLLPRIVATE_LINK_TYPED( HandleCancelBtn, Button*, void );
    DECL_DLLPRIVATE_LINK_TYPED(TimeOutHdl, Idle *, void);
    DECL_DLLPRIVATE_LINK_TYPED( startProgress, void *, void );
    DECL_DLLPRIVATE_LINK_TYPED( HandleHyperlink, FixedHyperlink&, void );

    static bool     isEnabled( const css::uno::Reference< css::deployment::XPackage > &xPackage );
    static bool     checkDependencies( const css::uno::Reference< css::deployment::XPackage > &xPackage );
    bool            hasActiveEntries();
    void            disableAllEntries();

public:
                    UpdateRequiredDialog( vcl::Window * pParent, TheExtensionManager *pManager );
    virtual        ~UpdateRequiredDialog();
    virtual void    dispose() override;

    virtual short   Execute() override;
    virtual bool    Close() override;

    virtual void    showProgress( bool bStart ) override;
    virtual void    updateProgress( const OUString &rText,
                                    const css::uno::Reference< css::task::XAbortChannel > &xAbortChannel) override;
    virtual void    updateProgress( const long nProgress ) override;

    virtual void    updatePackageInfo( const css::uno::Reference< css::deployment::XPackage > &xPackage ) override;

    virtual long    addPackageToList( const css::uno::Reference< css::deployment::XPackage > &,
                                      bool bLicenseMissing = false ) override;
    bool            enablePackage( const css::uno::Reference< css::deployment::XPackage > &xPackage, bool bEnable );

    virtual void    prepareChecking() override;
    virtual void    checkEntries() override;
};


class ShowLicenseDialog : public ModalDialog
{
    VclPtr<VclMultiLineEdit> m_pLicenseText;
public:
    ShowLicenseDialog(vcl::Window * pParent, const css::uno::Reference< css::deployment::XPackage > &xPackage);
    virtual ~ShowLicenseDialog();
    virtual void dispose() override;
};


class UpdateRequiredDialogService : public ::cppu::WeakImplHelper< css::ui::dialogs::XExecutableDialog >
{
    css::uno::Reference< css::uno::XComponentContext > const m_xComponentContext;
    css::uno::Reference< css::awt::XWindow > m_xParent;
public:
    UpdateRequiredDialogService( css::uno::Sequence< css::uno::Any > const & args,
                                 css::uno::Reference< css::uno::XComponentContext> const & xComponentContext );

    // XExecutableDialog
    virtual void SAL_CALL         setTitle( OUString const & title ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int16 SAL_CALL    execute() throw ( css::uno::RuntimeException, std::exception ) override;
};

} // namespace dp_gui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
