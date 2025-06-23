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

#include <framework/ResourceFactory.hxx>
#include <framework/ConfigurationChangeListener.hxx>
#include <rtl/ref.hxx>

namespace sd { class DrawController; }

namespace sd::framework {

/** This factory creates a marker view whose existence in a configuration
    indicates that a slideshow is running (in another but associated
    application window).
*/
class PresentationFactory final
    : public sd::framework::ResourceFactory
{
public:
    PresentationFactory (
        const rtl::Reference<::sd::DrawController>& rxController);
    virtual ~PresentationFactory() override;

    static void install(const rtl::Reference<::sd::DrawController>& rxController);

    // XResourceFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        createResource (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxViewId) override;

    virtual void releaseResource (
        const css::uno::Reference<css::drawing::framework::XResource>& xView) override;

private:
    class Listener : public sd::framework::ConfigurationChangeListener
    {
        using WeakComponentImplHelperBase::disposing;

        // ConfigurationChangeListener
        virtual void notifyConfigurationChange (
            const sd::framework::ConfigurationChangeEvent& rEvent) override;

        // lang::XEventListener
        virtual void SAL_CALL disposing (
            const css::lang::EventObject& rEventObject) override;
    };

    rtl::Reference<::sd::DrawController> mxController;
    rtl::Reference<Listener> mxListener;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
