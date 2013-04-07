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

#ifndef INCLUDED_DP_GUI_THEEXTMGR_HXX
#define INCLUDED_DP_GUI_THEEXTMGR_HXX

#include "comphelper/sequence.hxx"

#include "cppuhelper/implbase2.hxx"

#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/deployment/XExtensionManager.hpp"
#include "com/sun/star/deployment/ExtensionManager.hpp"
#include "com/sun/star/frame/XDesktop2.hpp"
#include "com/sun/star/frame/XTerminateListener.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/util/XModifyListener.hpp"

#include "dp_gui.h"
#include "dp_gui_dialog2.hxx"
#include "dp_gui_updatedata.hxx"

//==============================================================================
namespace dp_gui {

//------------------------------------------------------------------------------
class ExtensionCmdQueue;

//------------------------------------------------------------------------------
class TheExtensionManager :
    public ::cppu::WeakImplHelper2< ::com::sun::star::frame::XTerminateListener,
                                    ::com::sun::star::util::XModifyListener >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop2 >          m_xDesktop;
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XExtensionManager > m_xExtensionManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xNameAccessNodes;

    Window                  *m_pParent;
    ExtMgrDialog            *m_pExtMgrDialog;
    UpdateRequiredDialog    *m_pUpdReqDialog;
    ExtensionCmdQueue       *m_pExecuteCmdQueue;

    OUString          m_sGetExtensionsURL;

public:
    static ::rtl::Reference<TheExtensionManager> s_ExtMgr;

         TheExtensionManager( Window * pParent,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &xContext );
        ~TheExtensionManager();

    void createDialog( const bool bCreateUpdDlg );
    sal_Int16 execute();

    Dialog* getDialog() { return m_pExtMgrDialog ? (Dialog*) m_pExtMgrDialog : (Dialog*) m_pUpdReqDialog; }
    DialogHelper* getDialogHelper() { return m_pExtMgrDialog ? (DialogHelper*) m_pExtMgrDialog : (DialogHelper*) m_pUpdReqDialog; }
    ExtensionCmdQueue* getCmdQueue() const { return m_pExecuteCmdQueue; }

    void SetText( const OUString &rTitle );
    void Show();
    void ToTop( sal_uInt16 nFlags );
    bool Close();
    bool isVisible();

    //-----------------
    bool checkUpdates( bool showUpdateOnly, bool parentVisible );
    bool installPackage( const OUString &rPackageURL, bool bWarnUser = false );
    void createPackageList();

    bool queryTermination();
    void terminateDialog();

    // Tools
    bool supportsOptions( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage ) const;
    PackageState getPackageState( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage ) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > getContext() const { return m_xContext; }
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XExtensionManager > getExtensionManager() const { return m_xExtensionManager; }
    bool isReadOnly( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage ) const;

    //-----------------
    static ::rtl::Reference<TheExtensionManager> get(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> const & xContext,
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow> const & xParent = 0,
        OUString const & view = OUString() );

    // XEventListener
    virtual void SAL_CALL disposing( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::uno::RuntimeException);

    // XTerminateListener
    virtual void SAL_CALL queryTermination( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::uno::RuntimeException);

    // XModifyListener
    virtual void SAL_CALL modified( ::com::sun::star::lang::EventObject const & evt )
        throw (::com::sun::star::uno::RuntimeException);
};

} // namespace dp_gui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
