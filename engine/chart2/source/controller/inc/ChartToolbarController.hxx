/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <comphelper/compbase.hxx>

#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com::sun::star::frame { class XFramesSupplier; }

namespace chart {

typedef comphelper::WeakComponentImplHelper<
    css::frame::XToolbarController, css::frame::XStatusListener,
    css::util::XUpdatable, css::lang::XInitialization,
    css::lang::XServiceInfo> ChartToolbarControllerBase;

class ChartToolbarController final : public ChartToolbarControllerBase
{
public:
    ChartToolbarController(const cpo::uno::Sequence<cpo::uno::Any>& rProperties);
    virtual ~ChartToolbarController() override;

    ChartToolbarController(const ChartToolbarController&) = delete;
    const ChartToolbarController& operator=(const ChartToolbarController&) = delete;

    // XToolbarController
    virtual void execute(sal_Int16 nKeyModifier) override;

    virtual void click() override;

    virtual void doubleClick() override;

    virtual css::uno::Reference<css::awt::XWindow> createPopupWindow() override;

    virtual css::uno::Reference<css::awt::XWindow>
        createItemWindow(const css::uno::Reference<css::awt::XWindow>& rParent) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;

    virtual bool supportsService(OUString const & ServiceName) override;

    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    // XStatusListener
    virtual void statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    // XEventListener
    virtual void disposing(const css::lang::EventObject& rSource) override;

    // XInitialization
    virtual void initialize(const cpo::uno::Sequence<cpo::uno::Any>& rAny) override;

    // XUpdatable
    virtual void update() override;

    using comphelper::WeakComponentImplHelperBase::disposing;

private:

    css::uno::Reference<css::frame::XFramesSupplier> mxFramesSupplier;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
