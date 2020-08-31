/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#ifndef INCLUDED_SVX_SOURCE_INC_STYLES_PREVIEW_TOOLBOX_CONTROL_HXX
#define INCLUDED_SVX_SOURCE_INC_STYLES_PREVIEW_TOOLBOX_CONTROL_HXX

#include <svx/svxdllapi.h>
#include <sfx2/tbxctrl.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "StylesPreviewWindow.hxx"
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>

class SVX_DLLPUBLIC StylesPreviewToolBoxControl final
    : public cppu::ImplInheritanceHelper<svt::ToolboxController, css::lang::XServiceInfo>
{
    VclPtr<StylesPreviewWindow_Impl> m_xVclBox;
    std::unique_ptr<StylesPreviewWindow_Base> m_xWeldBox;
    StylesPreviewWindow_Base* m_pBox;

    css::uno::Reference<css::frame::XDispatchProvider> m_xDispatchProvider;

    std::vector<std::pair<OUString, OUString>> m_aDefaultStyles;

public:
    StylesPreviewToolBoxControl();
    virtual ~StylesPreviewToolBoxControl() override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    // XToolbarController
    virtual css::uno::Reference<css::awt::XWindow>
        SAL_CALL createItemWindow(const css::uno::Reference<css::awt::XWindow>& rParent) override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& aArguments) override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XUpdatable
    virtual void SAL_CALL update() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

private:
    void InitializeStyles(const css::uno::Reference<css::frame::XModel>& xModel);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */