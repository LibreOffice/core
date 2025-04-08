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

#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <comphelper/compbase.hxx>
#include <rtl/ref.hxx>

namespace sd { class DrawController; }

namespace sd::framework {

typedef comphelper::WeakComponentImplHelper <
    css::drawing::framework::XResourceFactory,
    css::drawing::framework::XConfigurationChangeListener
    > PresentationFactoryInterfaceBase;

/** This factory creates a marker view whose existence in a configuration
    indicates that a slideshow is running (in another but associated
    application window).
*/
class PresentationFactory final
    : public PresentationFactoryInterfaceBase
{
public:
    PresentationFactory (
        const rtl::Reference<::sd::DrawController>& rxController);
    virtual ~PresentationFactory() override;

    static void install(const rtl::Reference<::sd::DrawController>& rxController);

    // XResourceFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL createResource (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxViewId) override;

    virtual void SAL_CALL releaseResource (
        const css::uno::Reference<css::drawing::framework::XResource>& xView) override;

    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent) override;

    // lang::XEventListener

    using WeakComponentImplHelperBase::disposing;
    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEventObject) override;

private:
    rtl::Reference<::sd::DrawController> mxController;

    /// @throws css::lang::DisposedException
    void ThrowIfDisposed() const;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
