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

#ifndef INCLUDED_EXTENSIONS_SOURCE_UPDATE_CHECK_UPDATEHDL_HXX
#define INCLUDED_EXTENSIONS_SOURCE_UPDATE_CHECK_UPDATEHDL_HXX

#include <osl/mutex.hxx>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <cppuhelper/implbase.hxx>

#include "actionlistener.hxx"

enum DialogControls
{
    CANCEL_BUTTON = 0,
    PAUSE_BUTTON,
    RESUME_BUTTON,
    INSTALL_BUTTON,
    DOWNLOAD_BUTTON,
    CLOSE_BUTTON,
    HELP_BUTTON,
    BUTTON_COUNT,
    THROBBER_CTRL,
    PROGRESS_CTRL
};

enum UpdateState {
    UPDATESTATE_CHECKING = 0,
    UPDATESTATE_ERROR_CHECKING,
    UPDATESTATE_NO_UPDATE_AVAIL,
    UPDATESTATE_UPDATE_AVAIL,
    UPDATESTATE_UPDATE_NO_DOWNLOAD,
    UPDATESTATE_AUTO_START,
    UPDATESTATE_DOWNLOADING,
    UPDATESTATE_DOWNLOAD_PAUSED,
    UPDATESTATE_ERROR_DOWNLOADING,
    UPDATESTATE_DOWNLOAD_AVAIL,
    UPDATESTATE_EXT_UPD_AVAIL,
    UPDATESTATES_COUNT
};

class UpdateHandler : public cppu::WeakImplHelper< css::awt::XActionListener,
                                                    css::awt::XTopWindowListener,
                                                    css::task::XInteractionHandler,
                                                    css::frame::XTerminateListener >
{
private:
    css::uno::Reference< css::uno::XComponentContext >    mxContext;
    css::uno::Reference< css::awt::XDialog >              mxUpdDlg;
    css::uno::Reference< css::task::XInteractionHandler > mxInteractionHdl;
    rtl::Reference< IActionListener >                     mxActionListener;

    UpdateState             meCurState;
    UpdateState             meLastState;
    sal_Int32               mnPercent;
    short                   mnLastCtrlState;
    bool                    mbDownloadBtnHasDots;
    bool                    mbVisible;
    bool                    mbStringsLoaded;
    bool                    mbMinimized;
    bool                    mbListenerAdded;
    mutable bool            mbShowsMessageBox;

    osl::Mutex              maMutex;

    OUString           msNextVersion;
    OUString           msDownloadPath;
    OUString           msDownloadFile;
    OUString           msDescriptionMsg;
    OUString           msChecking;         // RID_UPDATE_STR_CHECKING
    OUString           msCheckingError;    // RID_UPDATE_STR_CHECKING_ERR
    OUString           msNoUpdFound;       // RID_UPDATE_STR_NO_UPD_FOUND
    OUString           msUpdFound;         // RID_UPDATE_STR_UPD_FOUND
    OUString           msDlgTitle;         // RID_UPDATE_STR_DLG_TITLE
    OUString           msDownloadPause;    // RID_UPDATE_STR_DOWNLOAD_PAUSE
    OUString           msDownloadError;    // RID_UPDATE_STR_DOWNLOAD_ERR
    OUString           msDownloadWarning;  // RID_UPDATE_STR_DOWNLOAD_WARN
    OUString           msDownloadDescr;    // RID_UPDATE_STR_DOWNLOAD_WARN
    OUString           msDownloadNotAvail; // RID_UPDATE_STR_DOWNLOAD_UNAVAIL
    OUString           msDownloading;      // RID_UPDATE_STR_DOWNLOADING
    OUString           msReady2Install;    // RID_UPDATE_STR_READY_INSTALL
    OUString           msCancelMessage;    // RID_UPDATE_STR_CANCEL_DOWNLOAD
    OUString           msInstallMessage;   // RID_UPDATE_STR_BEGIN_INSTALL
    OUString           msInstallError;     // RID_UPDATE_STR_INSTALL_ERROR
    OUString           msOverwriteWarning; // RID_UPDATE_STR_OVERWRITE_WARNING
    OUString           msPercent;          // RID_UPDATE_STR_PERCENT
    OUString           msReloadWarning;    // RID_UPDATE_STR_OVERWRITE_WARNING
    OUString           msReloadReload;     // RID_UPDATE_STR_OVERWRITE_WARNING
    OUString           msReloadContinue;   // RID_UPDATE_STR_OVERWRITE_WARNING
    OUString           msStatusFL;         // RID_UPDATE_FT_STATUS
    OUString           msDescription;      // RID_UPDATE_FT_DESCRIPTION
    OUString           msClose;            // RID_UPDATE_BTN_CLOSE
    OUString           msDownload;         // RID_UPDATE_BTN_DOWNLOAD
    OUString           msInstall;          // RID_UPDATE_BTN_INSTALL
    OUString           msPauseBtn;         // RID_UPDATE_BTN_PAUSE
    OUString           msResumeBtn;        // RID_UPDATE_BTN_RESUME
    OUString           msCancelBtn;        // RID_UPDATE_BTN_CANCEL
    OUString           msButtonIDs[ BUTTON_COUNT ];
    OUString           msBubbleTexts[ UPDATESTATES_COUNT ];
    OUString           msBubbleTitles[ UPDATESTATES_COUNT ];

    void                    createDialog();
    void                    updateState( UpdateState eNewState );
    void                    startThrobber( bool bStart = true );
    void                    setControlProperty( const OUString &rCtrlName,
                                                const OUString &rPropName,
                                                const css::uno::Any &rPropValue );
    void                    showControl( const OUString &rCtrlName, bool bShow = true );
    void                    showControls( short nControls );
    void                    focusControl( DialogControls eID );
    void                    enableControls( short nCtrlState );
    void                    setDownloadBtnLabel( bool bAppendDots );
    void                    loadStrings();
    static OUString         loadString(const std::locale& rLocale,
                                       const char* pResourceId);
    OUString                substVariables( const OUString &rSource ) const;
    static void             insertControlModel( css::uno::Reference< css::awt::XControlModel > const & rxDialogModel,
                                                OUString const & rServiceName,
                                                OUString const & rControlName,
                                                css::awt::Rectangle const & rPosSize,
                                                css::uno::Sequence< css::beans::NamedValue > const & rProps );

    void                    setFullVersion( OUString& rString );

public:
                            UpdateHandler( const css::uno::Reference< css::uno::XComponentContext > & rxContext,
                                           const rtl::Reference< IActionListener > & rxActionListener );
    virtual                ~UpdateHandler() override;
                            UpdateHandler(const UpdateHandler&) = delete;
    UpdateHandler&          operator=(const UpdateHandler&) = delete;

    bool                    isVisible() const;
    bool                    isMinimized() const { return mbMinimized; }
    void                    setVisible( bool bVisible = true );
    void                    setProgress( sal_Int32 nPercent );
    void                    setNextVersion( const OUString &rNextVersion ) { msNextVersion = rNextVersion; }
    void                    setDownloadPath( const OUString &rPath ) { msDownloadPath = rPath; }
    void                    setDownloadFile( const OUString &rPath );
    void                    setErrorMessage( const OUString &rErrorMsg );
    void                    setDescription( const OUString &rDescription ){ msDescriptionMsg = rDescription; }

    void                    setState( UpdateState eState );
    OUString           getBubbleText( UpdateState eState );
    OUString           getBubbleTitle( UpdateState eState );
    OUString           getDefaultInstErrMsg();
    bool                    showWarning( const OUString &rWarning ) const;
    bool                    showWarning( const OUString &rWarning, const OUString& rBtnText_1, const OUString& rBtnText_2 ) const;
    bool                    showOverwriteWarning( const OUString &rFileName ) const;
    bool                    showOverwriteWarning() const;

    // Allows runtime exceptions to be thrown by const methods
    operator css::uno::Reference< css::uno::XInterface > () const
        { return const_cast< cppu::OWeakObject * > (static_cast< cppu::OWeakObject const * > (this)); };

    // XActionListener
    virtual void SAL_CALL   disposing( const css::lang::EventObject &rObj ) override;
    virtual void SAL_CALL   actionPerformed( css::awt::ActionEvent const & rEvent) override;

    // XTopWindowListener
    virtual void SAL_CALL   windowOpened( const css::lang::EventObject& e ) override;
    virtual void SAL_CALL   windowClosing( const css::lang::EventObject& e ) override;
    virtual void SAL_CALL   windowClosed( const css::lang::EventObject& e ) override;
    virtual void SAL_CALL   windowMinimized( const css::lang::EventObject& e ) override;
    virtual void SAL_CALL   windowNormalized( const css::lang::EventObject& e ) override;
    virtual void SAL_CALL   windowActivated( const css::lang::EventObject& e ) override;
    virtual void SAL_CALL   windowDeactivated( const css::lang::EventObject& e ) override;

    // XInteractionHandler
    virtual void SAL_CALL   handle( const css::uno::Reference< css::task::XInteractionRequest >& Request ) override;

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const css::lang::EventObject& e ) override;
    virtual void SAL_CALL notifyTermination( const css::lang::EventObject& e ) override;
};

#endif // INCLUDED_EXTENSIONS_SOURCE_UPDATE_CHECK_UPDATEHDL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
