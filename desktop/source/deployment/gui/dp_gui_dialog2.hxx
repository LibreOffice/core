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

#ifndef INCLUDED_DP_GUI_DIALOG2_HXX
#define INCLUDED_DP_GUI_DIALOG2_HXX

#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "vcl/fixedhyper.hxx"
#include "vcl/prgsbar.hxx"
#include "vcl/timer.hxx"

#include "svtools/svmedit.hxx"

#include "osl/conditn.hxx"
#include "osl/mutex.hxx"

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "cppuhelper/implbase1.hxx"

#include "com/sun/star/awt/XWindow.hpp"
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/ui/dialogs/XExecutableDialog.hpp"
#include "com/sun/star/util/XModifyListener.hpp"

namespace dp_gui {

//==============================================================================
class ExtBoxWithBtns_Impl;
class ExtensionBox_Impl;
class TheExtensionManager;

//==============================================================================
class DialogHelper
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    Dialog*         m_pVCLWindow;
    sal_uLong           m_nEventID;
    bool            m_bIsBusy;

public:
                    DialogHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &,
                                  Dialog *pWindow );
    virtual        ~DialogHelper();

    void            openWebBrowser( const OUString & sURL, const OUString & sTitle ) const;
    Dialog*         getWindow() const { return m_pVCLWindow; };
    void            PostUserEvent( const Link& rLink, void* pCaller );
    void            clearEventID() { m_nEventID = 0; }

    virtual void    showProgress( bool bStart ) = 0;
    virtual void    updateProgress( const OUString &rText,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XAbortChannel > &xAbortChannel) = 0;
    virtual void    updateProgress( const long nProgress ) = 0;

    virtual void    updatePackageInfo( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage ) = 0;
    virtual long    addPackageToList( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage,
                                      bool bLicenseMissing = false ) = 0;

    virtual void    prepareChecking() = 0;
    virtual void    checkEntries() = 0;

    static ResId    getResId( sal_uInt16 nId );
    static OUString getResourceString( sal_uInt16 id );
    static bool     IsSharedPkgMgr( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &);
    static bool     continueOnSharedExtension( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &,
                                               Window *pParent,
                                               const sal_uInt16 nResID,
                                               bool &bHadWarning );

    void            setBusy( const bool bBusy ) { m_bIsBusy = bBusy; }
    bool            isBusy() const { return m_bIsBusy; }
    bool            installExtensionWarn( const OUString &rExtensionURL ) const;
    bool            installForAllUsers( bool &bInstallForAll ) const;
};

//==============================================================================
class ExtMgrDialog : public ModelessDialog,
                     public DialogHelper
{
    ExtBoxWithBtns_Impl *m_pExtensionBox;
    PushButton          *m_pAddBtn;
    PushButton          *m_pUpdateBtn;
    CloseButton         *m_pCloseBtn;
    CheckBox            *m_pBundledCbx;
    CheckBox            *m_pSharedCbx;
    CheckBox            *m_pUserCbx;
    FixedHyperlink      *m_pGetExtensions;
    FixedText           *m_pProgressText;
    ProgressBar         *m_pProgressBar;
    CancelButton        *m_pCancelBtn;
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
    Timer                m_aTimeoutTimer;
    TheExtensionManager *m_pManager;

    ::com::sun::star::uno::Reference< ::com::sun::star::task::XAbortChannel > m_xAbortChannel;

    bool removeExtensionWarn( const OUString &rExtensionTitle ) const;

    DECL_DLLPRIVATE_LINK( HandleAddBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleUpdateBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleCancelBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleCloseBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleExtTypeCbx, void * );
    DECL_DLLPRIVATE_LINK( HandleHyperlink, FixedHyperlink * );
    DECL_DLLPRIVATE_LINK(TimeOutHdl, void *);
    DECL_DLLPRIVATE_LINK( startProgress, void * );

public:
                    ExtMgrDialog( Window * pParent, TheExtensionManager *pManager );
    virtual        ~ExtMgrDialog();

    virtual long    Notify( NotifyEvent& rNEvt );
    virtual sal_Bool    Close();

    virtual void    showProgress( bool bStart );
    virtual void    updateProgress( const OUString &rText,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XAbortChannel > &xAbortChannel);
    virtual void    updateProgress( const long nProgress );

    virtual void    updatePackageInfo( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );

    void            setGetExtensionsURL( const OUString &rURL );
    virtual long    addPackageToList( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &,
                                      bool bLicenseMissing = false );
    bool enablePackage(const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage,
                        bool bEnable );
    bool removePackage(const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );
    bool updatePackage(const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );
    bool acceptLicense(const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );

    TheExtensionManager*    getExtensionManager() const { return m_pManager; }

    virtual void    prepareChecking();
    virtual void    checkEntries();

    ::com::sun::star::uno::Sequence< OUString > raiseAddPicker();
};

//==============================================================================
class UpdateRequiredDialog : public ModalDialog,
                             public DialogHelper
{
    ExtensionBox_Impl   *m_pExtensionBox;
    FixedText            m_aUpdateNeeded;
    PushButton           m_aUpdateBtn;
    PushButton           m_aCloseBtn;
    HelpButton           m_aHelpBtn;
    CancelButton         m_aCancelBtn;
    FixedLine            m_aDivider;
    FixedText            m_aProgressText;
    ProgressBar          m_aProgressBar;
    const OUString       m_sAddPackages;
    const OUString       m_sCloseText;
    OUString             m_sProgressText;
    ::osl::Mutex         m_aMutex;
    bool                 m_bHasProgress;
    bool                 m_bProgressChanged;
    bool                 m_bStartProgress;
    bool                 m_bStopProgress;
    bool                 m_bUpdateWarning;
    bool                 m_bDisableWarning;
    bool                 m_bHasLockedEntries;
    long                 m_nProgress;
    Timer                m_aTimeoutTimer;
    TheExtensionManager *m_pManager;

    ::com::sun::star::uno::Reference< ::com::sun::star::task::XAbortChannel > m_xAbortChannel;

    DECL_DLLPRIVATE_LINK( HandleUpdateBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleCloseBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleCancelBtn, void * );
    DECL_DLLPRIVATE_LINK(TimeOutHdl, void *);
    DECL_DLLPRIVATE_LINK( startProgress, void * );
    DECL_DLLPRIVATE_LINK( HandleHyperlink, FixedHyperlink * );

    bool            isEnabled( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage ) const;
    bool            checkDependencies( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage ) const;
    bool            hasActiveEntries();
    void            disableAllEntries();

public:
                    UpdateRequiredDialog( Window * pParent, TheExtensionManager *pManager );
    virtual        ~UpdateRequiredDialog();

    virtual short   Execute();
    virtual void    Resize();
    virtual sal_Bool    Close();

    virtual void    showProgress( bool bStart );
    virtual void    updateProgress( const OUString &rText,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XAbortChannel > &xAbortChannel);
    virtual void    updateProgress( const long nProgress );

    virtual void    updatePackageInfo( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );

    virtual long    addPackageToList( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &,
                                      bool bLicenseMissing = false );
    bool enablePackage( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage, bool bEnable );
    bool updatePackage( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );

    virtual void    prepareChecking();
    virtual void    checkEntries();

    ::com::sun::star::uno::Sequence< OUString > raiseAddPicker();

    bool            installForAllUsers( bool &bInstallForAll ) const;
    bool            installExtensionWarn( const OUString &rExtensionURL ) const;
};

//==============================================================================
class ShowLicenseDialog : public ModalDialog
{
    MultiLineEdit   m_aLicenseText;
    OKButton        m_aCloseBtn;

public:
                    ShowLicenseDialog( Window * pParent,
                                       const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );
    virtual        ~ShowLicenseDialog();

    virtual void    Resize();
};

//==============================================================================
class UpdateRequiredDialogService : public ::cppu::WeakImplHelper1< ::com::sun::star::ui::dialogs::XExecutableDialog >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const m_xComponentContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > m_xParent;
    OUString m_sInitialTitle;

public:
    UpdateRequiredDialogService( ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > const & args,
                                 ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> const & xComponentContext );

    // XExecutableDialog
    virtual void SAL_CALL         setTitle( OUString const & title ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int16 SAL_CALL    execute() throw ( ::com::sun::star::uno::RuntimeException );
};

} // namespace dp_gui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
