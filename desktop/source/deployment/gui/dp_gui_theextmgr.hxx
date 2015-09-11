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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_THEEXTMGR_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_THEEXTMGR_HXX

#include <comphelper/sequence.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/deployment/XExtensionManager.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include "dp_gui.h"
#include "dp_gui_dialog2.hxx"
#include "dp_gui_updatedata.hxx"


namespace dp_gui {


class ExtensionCmdQueue;


class TheExtensionManager :
    public ::cppu::WeakImplHelper< css::frame::XTerminateListener,
                                    css::util::XModifyListener >
{
private:
    css::uno::Reference< css::uno::XComponentContext >        m_xContext;
    css::uno::Reference< css::frame::XDesktop2 >              m_xDesktop;
    css::uno::Reference< css::deployment::XExtensionManager > m_xExtensionManager;
    css::uno::Reference< css::container::XNameAccess >        m_xNameAccessNodes;
    css::uno::Reference< css::awt::XWindow >                  m_xParent;
    VclPtr<ExtMgrDialog>         m_pExtMgrDialog;
    VclPtr<UpdateRequiredDialog> m_pUpdReqDialog;
    ExtensionCmdQueue           *m_pExecuteCmdQueue;

    OUString                     m_sGetExtensionsURL;

public:
    static ::rtl::Reference<TheExtensionManager> s_ExtMgr;

         TheExtensionManager( const css::uno::Reference< css::awt::XWindow > &xParent,
                              const css::uno::Reference< css::uno::XComponentContext > &xContext );
        virtual ~TheExtensionManager();

    void createDialog( const bool bCreateUpdDlg );
    sal_Int16 execute();

    Dialog* getDialog()
    {
        if (m_pExtMgrDialog)
            return m_pExtMgrDialog.get();
        return m_pUpdReqDialog.get();
    }
    DialogHelper* getDialogHelper()
    {
        if (m_pExtMgrDialog)
            return m_pExtMgrDialog.get();
        return m_pUpdReqDialog.get();
    }
    ExtensionCmdQueue* getCmdQueue() const { return m_pExecuteCmdQueue; }

    void SetText( const OUString &rTitle );
    void Show();
    void ToTop( ToTopFlags nFlags );
    bool Close();
    bool isVisible();


    bool checkUpdates( bool showUpdateOnly, bool parentVisible );
    bool installPackage( const OUString &rPackageURL, bool bWarnUser = false );
    void createPackageList();

    static bool queryTermination();
    void terminateDialog();

    // Tools
    bool supportsOptions( const css::uno::Reference< css::deployment::XPackage > &xPackage ) const;
    static PackageState getPackageState( const css::uno::Reference< css::deployment::XPackage > &xPackage );
    css::uno::Reference< css::uno::XComponentContext > getContext() const { return m_xContext; }
    css::uno::Reference< css::deployment::XExtensionManager > getExtensionManager() const { return m_xExtensionManager; }
    bool isReadOnly( const css::uno::Reference< css::deployment::XPackage > &xPackage ) const;


    static ::rtl::Reference<TheExtensionManager> get(
        css::uno::Reference< css::uno::XComponentContext> const & xContext,
        css::uno::Reference< css::awt::XWindow> const & xParent = 0,
        OUString const & view = OUString() );

    // XEventListener
    virtual void SAL_CALL disposing( css::lang::EventObject const & evt )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTerminateListener
    virtual void SAL_CALL queryTermination( css::lang::EventObject const & evt )
        throw (css::frame::TerminationVetoException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL notifyTermination( css::lang::EventObject const & evt )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XModifyListener
    virtual void SAL_CALL modified( css::lang::EventObject const & evt )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

} // namespace dp_gui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
