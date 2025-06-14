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
#include <svtools/toolboxcontroller.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <toolkit/awt/vclxmenu.hxx>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <vcl/vclenum.hxx>

namespace framework
{
typedef cppu::ImplInheritanceHelper<svt::ToolboxController, css::lang::XServiceInfo> ToolBarBase;

class PopupMenuToolbarController : public ToolBarBase
{
public:
    // XComponent
    virtual void SAL_CALL dispose() override;
    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& aArguments) override;
    // XToolbarController
    virtual css::uno::Reference<css::awt::XWindow> SAL_CALL createPopupWindow() override;
    // XStatusListener
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

protected:
    PopupMenuToolbarController(const css::uno::Reference<css::uno::XComponentContext>& rxContext,
                               OUString aPopupCommand = OUString());
    virtual void functionExecuted(const OUString& rCommand);
    virtual ToolBoxItemBits getDropDownStyle() const;
    void createPopupMenuController();

    bool m_bHasController;
    bool m_bResourceURL;
    OUString m_aPopupCommand;
    rtl::Reference<VCLXPopupMenu> m_xPopupMenu;

private:
    css::uno::Reference<css::frame::XUIControllerFactory> m_xPopupMenuFactory;
    css::uno::Reference<css::frame::XPopupMenuController> m_xPopupMenuController;
};

class GenericPopupToolbarController : public PopupMenuToolbarController
{
public:
    GenericPopupToolbarController(const css::uno::Reference<css::uno::XComponentContext>& rxContext,
                                  const css::uno::Sequence<css::uno::Any>& rxArgs);

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rxArgs) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const& rServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

private:
    bool m_bSplitButton, m_bReplaceWithLast;
    void functionExecuted(const OUString& rCommand) override;
    ToolBoxItemBits getDropDownStyle() const override;
};

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
