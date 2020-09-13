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

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/deployment/XExtensionManager.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include "dp_gui.h"
#include "dp_gui_dialog2.hxx"


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
    std::shared_ptr<ExtMgrDialog> m_xExtMgrDialog;
    std::unique_ptr<UpdateRequiredDialog> m_xUpdReqDialog;
    std::unique_ptr<ExtensionCmdQueue> m_xExecuteCmdQueue;

    OUString                     m_sGetExtensionsURL;
    bool                         m_bModified;
    bool                         m_bExtMgrDialogExecuting;

public:
    static ::rtl::Reference<TheExtensionManager> s_ExtMgr;

         TheExtensionManager( const css::uno::Reference< css::awt::XWindow > &xParent,
                              const css::uno::Reference< css::uno::XComponentContext > &xContext );
        virtual ~TheExtensionManager() override;

    void createDialog( const bool bCreateUpdDlg );
    sal_Int16 execute();

    bool isModified() const { return m_bModified; }
    void clearModified() { m_bModified = false; }

    weld::Window* getDialog()
    {
        if (m_xExtMgrDialog)
            return m_xExtMgrDialog->getDialog();
        if (m_xUpdReqDialog)
            return m_xUpdReqDialog->getDialog();
        return nullptr;
    }
    DialogHelper* getDialogHelper()
    {
        if (m_xExtMgrDialog)
            return m_xExtMgrDialog.get();
        return m_xUpdReqDialog.get();
    }
    ExtensionCmdQueue* getCmdQueue() const { return m_xExecuteCmdQueue.get(); }

    void SetText( const OUString &rTitle );
    void Show();
    void ToTop();
    void Close();
    bool isVisible();


    void checkUpdates();
    bool installPackage( const OUString &rPackageURL, bool bWarnUser = false );
    void createPackageList();

    void terminateDialog();

    // Tools
    bool supportsOptions( const css::uno::Reference< css::deployment::XPackage > &xPackage ) const;
    static PackageState getPackageState( const css::uno::Reference< css::deployment::XPackage > &xPackage );
    const css::uno::Reference< css::uno::XComponentContext >& getContext() const { return m_xContext; }
    const css::uno::Reference< css::deployment::XExtensionManager >& getExtensionManager() const { return m_xExtensionManager; }
    bool isReadOnly( const css::uno::Reference< css::deployment::XPackage > &xPackage ) const;


    static ::rtl::Reference<TheExtensionManager> get(
        css::uno::Reference< css::uno::XComponentContext> const & xContext,
        css::uno::Reference< css::awt::XWindow> const & xParent = nullptr,
        OUString const & view = OUString() );

    // XEventListener
    virtual void SAL_CALL disposing( css::lang::EventObject const & evt ) override;

    // XTerminateListener
    virtual void SAL_CALL queryTermination( css::lang::EventObject const & evt ) override;
    virtual void SAL_CALL notifyTermination( css::lang::EventObject const & evt ) override;

    // XModifyListener
    virtual void SAL_CALL modified( css::lang::EventObject const & evt ) override;
};

} // namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
