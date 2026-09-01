/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <svtools/toolboxcontroller.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "StylesPreviewWindow.hxx"
#include <com/sun/star/frame/XModel.hpp>

using StylesPreviewToolBoxControl_Base
    = cppu::ImplInheritanceHelper<svt::ToolboxController, css::lang::XServiceInfo>;

class StylesPreviewToolBoxControl final : public StylesPreviewToolBoxControl_Base
{
    VclPtr<StylesPreviewWindow_Impl> m_xVclBox;
    std::unique_ptr<StylesPreviewWindow_Base> m_xWeldBox;

    StylePreviewList m_aDefaultStyles;

public:
    StylesPreviewToolBoxControl();
    virtual ~StylesPreviewToolBoxControl() override;

    // XStatusListener
    virtual void statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    // XToolbarController
    virtual css::uno::Reference<css::awt::XWindow>
        createItemWindow(const css::uno::Reference<css::awt::XWindow>& rParent) override;

    // XInitialization
    virtual void initialize(const cpo::uno::Sequence<cpo::uno::Any>& rArguments) override;

    // WeakComponentImplHelperBase
    using StylesPreviewToolBoxControl_Base::disposing;
    virtual void disposing(std::unique_lock<std::mutex>& rGuard) override;

    // XUpdatable
    virtual void update() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService(const OUString& rServiceName) override;
    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

private:
    void InitializeStyles(const css::uno::Reference<css::frame::XModel>& xModel);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
