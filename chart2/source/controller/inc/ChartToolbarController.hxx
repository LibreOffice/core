/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_CHARTTOOLBARCONTROLLER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_CHARTTOOLBARCONTROLLER_HXX

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com { namespace sun { namespace star { namespace awt { class XWindow; } } } }
namespace com { namespace sun { namespace star { namespace frame { class XFramesSupplier; } } } }

namespace chart {

typedef cppu::WeakComponentImplHelper<
    css::frame::XToolbarController, css::frame::XStatusListener,
    css::util::XUpdatable, css::lang::XInitialization,
    css::lang::XServiceInfo> ChartToolbarControllerBase;

class ChartToolbarController : private cppu::BaseMutex,
                               public ChartToolbarControllerBase
{
public:
    ChartToolbarController(const css::uno::Sequence<css::uno::Any>& rProperties);
    virtual ~ChartToolbarController() override;

    ChartToolbarController(const ChartToolbarController&) = delete;
    const ChartToolbarController& operator=(const ChartToolbarController&) = delete;

    // XToolbarController
    virtual void SAL_CALL execute(sal_Int16 nKeyModifier) override;

    virtual void SAL_CALL click() override;

    virtual void SAL_CALL doubleClick() override;

    virtual css::uno::Reference<css::awt::XWindow> SAL_CALL createPopupWindow() override;

    virtual css::uno::Reference<css::awt::XWindow> SAL_CALL
        createItemWindow(const css::uno::Reference<css::awt::XWindow>& rParent) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& rSource) override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rAny) override;

    // XUpdatable
    virtual void SAL_CALL update() override;

    using cppu::WeakComponentImplHelperBase::disposing;

private:

    css::uno::Reference<css::frame::XFramesSupplier> mxFramesSupplier;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
