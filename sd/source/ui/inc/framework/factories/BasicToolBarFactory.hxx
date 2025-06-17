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

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <comphelper/compbase.hxx>
#include <rtl/ref.hxx>

namespace sd { class DrawController; }

namespace sd::framework
{
class ConfigurationController;

typedef comphelper::WeakComponentImplHelper <
    css::drawing::framework::XResourceFactory,
    css::lang::XEventListener
    > BasicToolBarFactoryInterfaceBase;

/** This factory provides some of the frequently used tool bars:
        private:resource/toolbar/ViewTabBar
*/
class BasicToolBarFactory final
    : public BasicToolBarFactoryInterfaceBase
{
public:
    BasicToolBarFactory (const rtl::Reference<::sd::DrawController>& rxController);
    virtual ~BasicToolBarFactory() override;

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    // ToolBarFactory

    virtual css::uno::Reference<css::drawing::framework::XResource> SAL_CALL
        createResource (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxToolBarId) override;

    virtual void SAL_CALL
        releaseResource (
            const css::uno::Reference<css::drawing::framework::XResource>&
                rxToolBar) override;

    // lang::XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEventObject) override;

private:
    rtl::Reference<ConfigurationController> mxConfigurationController;
    rtl::Reference<::sd::DrawController> mxController;

    void Shutdown();

    /// @throws css::lang::DisposedException
    void ThrowIfDisposed() const;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
