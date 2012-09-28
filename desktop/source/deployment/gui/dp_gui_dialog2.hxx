/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_DP_GUI_DIALOG2_HXX
#define INCLUDED_DP_GUI_DIALOG2_HXX

#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "vcl/timer.hxx"

#include "svtools/fixedhyper.hxx"
#include "svtools/prgsbar.hxx"
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

    void            openWebBrowser( const ::rtl::OUString & sURL, const ::rtl::OUString & sTitle ) const;
    Dialog*         getWindow() const { return m_pVCLWindow; };
    void            PostUserEvent( const Link& rLink, void* pCaller );
    void            clearEventID() { m_nEventID = 0; }

    virtual void    showProgress( bool bStart ) = 0;
    virtual void    updateProgress( const ::rtl::OUString &rText,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XAbortChannel > &xAbortChannel) = 0;
    virtual void    updateProgress( const long nProgress ) = 0;

    virtual void    updatePackageInfo( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage ) = 0;
    virtual long    addPackageToList( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage,
                                      bool bLicenseMissing = false ) = 0;

    virtual void    prepareChecking() = 0;
    virtual void    checkEntries() = 0;

    static ResId    getResId( sal_uInt16 nId );
    static String   getResourceString( sal_uInt16 id );
    static bool     IsSharedPkgMgr( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &);
    static bool     continueOnSharedExtension( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &,
                                               Window *pParent,
                                               const sal_uInt16 nResID,
                                               bool &bHadWarning );

    void            setBusy( const bool bBusy ) { m_bIsBusy = bBusy; }
    bool            isBusy() const { return m_bIsBusy; }
    bool            installExtensionWarn( const ::rtl::OUString &rExtensionURL ) const;
    bool            installForAllUsers( bool &bInstallForAll ) const;
};

//==============================================================================
class ExtMgrDialog : public ModelessDialog,
                     public DialogHelper
{
    ExtBoxWithBtns_Impl *m_pExtensionBox;
    PushButton           m_aAddBtn;
    PushButton           m_aUpdateBtn;
    OKButton             m_aCloseBtn;
    HelpButton           m_aHelpBtn;
    FixedLine            m_aDivider;
    FixedLine            m_aDivider2;
    FixedText            m_aTypeOfExtTxt;
    CheckBox             m_aBundledCbx;
    CheckBox             m_aSharedCbx;
    CheckBox             m_aUserCbx;
    svt::FixedHyperlink  m_aGetExtensions;
    FixedText            m_aProgressText;
    ProgressBar          m_aProgressBar;
    CancelButton         m_aCancelBtn;
    const String         m_sAddPackages;
    String               m_sProgressText;
    String               m_sLastFolderURL;
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

    bool removeExtensionWarn( const ::rtl::OUString &rExtensionTitle ) const;

    DECL_DLLPRIVATE_LINK( HandleAddBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleUpdateBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleCancelBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleExtTypeCbx, void * );
    DECL_DLLPRIVATE_LINK( HandleHyperlink, svt::FixedHyperlink * );
    DECL_DLLPRIVATE_LINK(TimeOutHdl, void *);
    DECL_DLLPRIVATE_LINK( startProgress, void * );

public:
                    ExtMgrDialog( Window * pParent, TheExtensionManager *pManager );
    virtual        ~ExtMgrDialog();

    virtual void    Resize();
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual sal_Bool    Close();

    virtual void    showProgress( bool bStart );
    virtual void    updateProgress( const ::rtl::OUString &rText,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XAbortChannel > &xAbortChannel);
    virtual void    updateProgress( const long nProgress );

    virtual void    updatePackageInfo( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );

    void            setGetExtensionsURL( const ::rtl::OUString &rURL );
    virtual long    addPackageToList( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &,
                                      bool bLicenseMissing = false );
    bool enablePackage(const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage,
                        bool bEnable );
    bool removePackage(const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );
    bool updatePackage(const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );
    bool acceptLicense(const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );

    virtual void    prepareChecking();
    virtual void    checkEntries();

    ::com::sun::star::uno::Sequence< ::rtl::OUString > raiseAddPicker();
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
    const String         m_sAddPackages;
    const String         m_sCloseText;
    String               m_sProgressText;
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
    DECL_DLLPRIVATE_LINK( HandleHyperlink, svt::FixedHyperlink * );

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
    virtual void    updateProgress( const ::rtl::OUString &rText,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XAbortChannel > &xAbortChannel);
    virtual void    updateProgress( const long nProgress );

    virtual void    updatePackageInfo( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );

    void            selectEntry( long nPos );
    virtual long    addPackageToList( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &,
                                      bool bLicenseMissing = false );
    bool enablePackage( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage, bool bEnable );
    bool updatePackage( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );

    virtual void    prepareChecking();
    virtual void    checkEntries();

    ::com::sun::star::uno::Sequence< ::rtl::OUString > raiseAddPicker();

    bool            installForAllUsers( bool &bInstallForAll ) const;
    bool            installExtensionWarn( const ::rtl::OUString &rExtensionURL ) const;
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
    ::rtl::OUString m_sInitialTitle;

public:
    UpdateRequiredDialogService( ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > const & args,
                                 ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> const & xComponentContext );

    // XExecutableDialog
    virtual void SAL_CALL         setTitle( rtl::OUString const & title ) throw ( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int16 SAL_CALL    execute() throw ( ::com::sun::star::uno::RuntimeException );
};

} // namespace dp_gui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
