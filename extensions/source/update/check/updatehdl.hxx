/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updatehdl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-19 16:49:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_UPDATE_HDL_HXX
#define INCLUDED_UPDATE_HDL_HXX

#include "boost/utility.hpp"

#ifndef  _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef  _COM_SUN_STAR_UNO_ANY_H_
#include "com/sun/star/uno/Any.h"
#endif
#ifndef  _COM_SUN_STAR_UNO_REFERENCE_HPP_
#include "com/sun/star/uno/Reference.h"
#endif
#ifndef  _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include "com/sun/star/uno/XComponentContext.hpp"
#endif

#ifndef  _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include "com/sun/star/awt/Rectangle.hpp"
#endif
#ifndef  _COM_SUN_STAR_AWT_XACTIONLISTENER_HPP_
#include "com/sun/star/awt/XActionListener.hpp"
#endif
#ifndef  _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include "com/sun/star/awt/XControlModel.hpp"
#endif
#ifndef  _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include "com/sun/star/awt/XDialog.hpp"
#endif
#ifndef  _COM_SUN_STAR_AWT_XTOPWINDOWLISTENER_HPP_
#include "com/sun/star/awt/XTopWindowListener.hpp"
#endif

#ifndef  _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include "com/sun/star/beans/NamedValue.hpp"
#endif

#ifndef  _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include "com/sun/star/frame/XTerminateListener.hpp"
#endif

#ifndef  _COM_SUN_STAR_RESOURCE_XRESOURCEBUNDLE_HPP_
#include <com/sun/star/resource/XResourceBundle.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef  _CPPUHELPER_IMPLBASE4_HXX_
#include "cppuhelper/implbase4.hxx"
#endif

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

class UpdateHandler : ::boost::noncopyable,
                      public cppu::WeakImplHelper4< com::sun::star::awt::XActionListener,
                                                    com::sun::star::awt::XTopWindowListener,
                                                    com::sun::star::task::XInteractionHandler,
                                                    com::sun::star::frame::XTerminateListener >
{
private:
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxContext;
    com::sun::star::uno::Reference< com::sun::star::awt::XDialog > mxUpdDlg;
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > mxInteractionHdl;
    rtl::Reference< IActionListener > mxActionListener;

    UpdateState             meCurState;
    UpdateState             meLastState;
    sal_Int32               mnPercent;
    short                   mnLastCtrlState;
    bool                    mbDownloadBtnHasDots;
    bool                    mbVisible;
    bool                    mbStringsLoaded;
    bool                    mbMinimized;
    bool                    mbListenerAdded;

    osl::Mutex              maMutex;

    rtl::OUString           msProductName;
    rtl::OUString           msProductVersion;
    rtl::OUString           msNextVersion;
    rtl::OUString           msDownloadPath;
    rtl::OUString           msDownloadFile;
    rtl::OUString           msDescriptionMsg;
    rtl::OUString           msChecking;         // RID_UPDATE_STR_CHECKING
    rtl::OUString           msCheckingError;    // RID_UPDATE_STR_CHECKING_ERR
    rtl::OUString           msNoUpdFound;       // RID_UPDATE_STR_NO_UPD_FOUND
    rtl::OUString           msUpdFound;         // RID_UPDATE_STR_UPD_FOUND
    rtl::OUString           msDlgTitle;         // RID_UPDATE_STR_DLG_TITLE
    rtl::OUString           msDownloadPause;    // RID_UPDATE_STR_DOWNLOAD_PAUSE
    rtl::OUString           msDownloadError;    // RID_UPDATE_STR_DOWNLOAD_ERR
    rtl::OUString           msDownloadWarning;  // RID_UPDATE_STR_DOWNLOAD_WARN
    rtl::OUString           msDownloadDescr;    // RID_UPDATE_STR_DOWNLOAD_WARN
    rtl::OUString           msDownloadNotAvail; // RID_UPDATE_STR_DOWNLOAD_UNAVAIL
    rtl::OUString           msDownloading;      // RID_UPDATE_STR_DOWNLOADING
    rtl::OUString           msReady2Install;    // RID_UPDATE_STR_READY_INSTALL
    rtl::OUString           msCancelTitle;      // RID_UPDATE_STR_CANCEL_TITLE
    rtl::OUString           msCancelMessage;    // RID_UPDATE_STR_CANCEL_DOWNLOAD
    rtl::OUString           msInstallMessage;   // RID_UPDATE_STR_BEGIN_INSTALL
    rtl::OUString           msInstallNow;       // RID_UPDATE_STR_INSTALL_NOW
    rtl::OUString           msInstallLater;     // RID_UPDATE_STR_INSTALL_LATER
    rtl::OUString           msInstallError;     // RID_UPDATE_STR_INSTALL_ERROR
    rtl::OUString           msOverwriteWarning; // RID_UPDATE_STR_OVERWRITE_WARNING
    rtl::OUString           msPercent;          // RID_UPDATE_STR_PERCENT
    rtl::OUString           msStatusFL;         // RID_UPDATE_FT_STATUS
    rtl::OUString           msDescription;      // RID_UPDATE_FT_DESCRIPTION
    rtl::OUString           msClose;            // RID_UPDATE_BTN_CLOSE
    rtl::OUString           msDownload;         // RID_UPDATE_BTN_DOWNLOAD
    rtl::OUString           msInstall;          // RID_UPDATE_BTN_INSTALL
    rtl::OUString           msPauseBtn;         // RID_UPDATE_BTN_PAUSE
    rtl::OUString           msResumeBtn;        // RID_UPDATE_BTN_RESUME
    rtl::OUString           msCancelBtn;        // RID_UPDATE_BTN_CANCEL
    rtl::OUString           msButtonIDs[ BUTTON_COUNT ];
    rtl::OUString           msBubbleTexts[ UPDATESTATES_COUNT ];
    rtl::OUString           msBubbleTitles[ UPDATESTATES_COUNT ];

    void                    createDialog();
    void                    updateState( UpdateState eNewState );
    void                    startThrobber( bool bStart = true );
    void                    setControlProperty( const rtl::OUString &rCtrlName,
                                                const rtl::OUString &rPropName,
                                                const com::sun::star::uno::Any &rPropValue );
    void                    showControl( const rtl::OUString &rCtrlName, bool bShow = true );
    void                    showControls( short nControls );
    void                    focusControl( DialogControls eID );
    void                    enableControls( short nCtrlState );
    void                    setDownloadBtnLabel( bool bAppendDots );
    void                    loadStrings();
    rtl::OUString           loadString( const com::sun::star::uno::Reference< com::sun::star::resource::XResourceBundle > xBundle,
                                        sal_Int32 nResourceId ) const;
    rtl::OUString           substVariables( const rtl::OUString &rSource ) const;
    static void             setProperty( com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > &rProps,
                                         const int nIndex, const rtl::OUString &rPropName, const com::sun::star::uno::Any &rPropValue )
                                         { rProps[ nIndex ].Name = rPropName; rProps[ nIndex ].Value = rPropValue; }
    static void             insertControlModel( com::sun::star::uno::Reference< com::sun::star::awt::XControlModel > & rxDialogModel,
                                                rtl::OUString const & rServiceName,
                                                rtl::OUString const & rControlName,
                                                com::sun::star::awt::Rectangle const & rPosSize,
                                                com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > const & rProps );

    void                    getProductName();
    void                    searchAndReplaceAll( rtl::OUString &rText, const rtl::OUString &rWhat, const rtl::OUString &rWith ) const;

public:
                            UpdateHandler( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > & rxContext,
                                           const rtl::Reference< IActionListener > & rxActionListener );
    virtual                ~UpdateHandler();

    bool                    isVisible() const;
    bool                    isMinimized() const { return mbMinimized; }
    void                    setVisible( bool bVisible = true );
    void                    setProgress( sal_Int32 nPercent );
    void                    setNextVersion( const rtl::OUString &rNextVersion ) { msNextVersion = rNextVersion; }
    void                    setDownloadPath( const rtl::OUString &rPath ) { msDownloadPath = rPath; }
    void                    setDownloadFile( const rtl::OUString &rPath );
    void                    setErrorMessage( const rtl::OUString &rErrorMsg );
    void                    setDescription( const rtl::OUString &rDescription ){ msDescriptionMsg = rDescription; }

    void                    setState( UpdateState eState );
    rtl::OUString           getBubbleText( UpdateState eState );
    rtl::OUString           getBubbleTitle( UpdateState eState );
    rtl::OUString           getDefaultInstErrMsg();
    bool                    showWarning( const rtl::OUString &rWarning ) const;
    bool                    showOverwriteWarning() const;

    // Allows runtime exceptions to be thrown by const methods
    inline SAL_CALL operator com::sun::star::uno::Reference< com::sun::star::uno::XInterface > () const
        { return const_cast< cppu::OWeakObject * > (static_cast< cppu::OWeakObject const * > (this)); };

    // XActionListener
    virtual void SAL_CALL   disposing( const com::sun::star::lang::EventObject &rObj ) throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL   actionPerformed( com::sun::star::awt::ActionEvent const & rEvent) throw( com::sun::star::uno::RuntimeException );

    // XTopWindowListener
    virtual void SAL_CALL   windowOpened( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   windowClosing( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   windowClosed( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   windowMinimized( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   windowNormalized( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   windowActivated( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   windowDeactivated( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);

    // XInteractionHandler
    virtual void SAL_CALL   handle( const com::sun::star::uno::Reference< com::sun::star::task::XInteractionRequest >& Request )
                                throw( com::sun::star::uno::RuntimeException );

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
};

#endif /* INCLUDED_UPDATE_HDL_HXX */
