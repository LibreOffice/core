/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_gui_dialog2.hxx,v $
 *
 * $Revision: 1.4.74.3 $
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

#include "osl/conditn.hxx"
#include "osl/mutex.hxx"

#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/deployment/XPackageManager.hpp"
#include "com/sun/star/util/XModifyListener.hpp"

namespace dp_gui {

//==============================================================================
class ExtensionBox_Impl;
class TheExtensionManager;

//==============================================================================
class ExtMgrDialog : public ModelessDialog
{
    ExtensionBox_Impl   *m_pExtensionBox;
    PushButton           m_aAddBtn;
    PushButton           m_aUpdateBtn;
    OKButton             m_aCloseBtn;
    HelpButton           m_aHelpBtn;
    FixedLine            m_aDivider;
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
    bool                 m_bUpdateWarning;
    bool                 m_bEnableWarning;
    bool                 m_bDisableWarning;
    bool                 m_bDeleteWarning;
    bool                 m_bIsBusy;
    long                 m_nProgress;
    Timer                m_aTimeoutTimer;
    TheExtensionManager *m_pManager;

    ::com::sun::star::uno::Reference< ::com::sun::star::task::XAbortChannel > m_xAbortChannel;

    bool continueOnSharedExtension( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > &,
                                    const USHORT nResID,
                                    bool &bHadWarning ) const;
    bool removeExtensionWarn( const ::rtl::OUString &rExtensionTitle ) const;

    DECL_DLLPRIVATE_LINK( HandleAddBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleUpdateBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleCancelBtn, void * );
    DECL_DLLPRIVATE_LINK( HandleHyperlink, svt::FixedHyperlink * );
    DECL_DLLPRIVATE_LINK( TimeOutHdl, Timer* );
    DECL_DLLPRIVATE_LINK( startProgress, void * );

public:
                    ExtMgrDialog( Window * pParent, TheExtensionManager *pManager );
    virtual        ~ExtMgrDialog();

    virtual void    Resize();
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual BOOL    Close();

    void            setGetExtensionsURL( const ::rtl::OUString &rURL );
    void            selectEntry( long nPos );
    long            addPackageToList( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &,
                                      const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > & );
    bool enablePackage( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > &xPackageManager,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage,
                        bool bEnable );
    bool removePackage( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > &xPackageManager,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );
    bool updatePackage( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > &xPackageManager,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );

    bool            isBusy() { return m_bIsBusy; }

    void            showProgress( bool bStart );
    void            updateProgress( const ::rtl::OUString &rText,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XAbortChannel > &xAbortChannel);
    void            updateProgress( const long nProgress );
    void            updatePackageInfo( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage );

    void            prepareChecking( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > &xPackageManager );
    void            checkEntries();

    ::com::sun::star::uno::Sequence< ::rtl::OUString > raiseAddPicker( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > &xPackageManager );

    void            openWebBrowser( ::rtl::OUString const &sURL ) const;

    bool            installForAllUsers( bool &bInstallForAll ) const;
    bool            installExtensionWarn( const ::rtl::OUString &rExtensionURL ) const;

    static ResId    getResId( USHORT id );
    static String   getResourceString( USHORT id );
    static bool     IsSharedPkgMgr( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > &);
};

} // namespace dp_gui

#endif
