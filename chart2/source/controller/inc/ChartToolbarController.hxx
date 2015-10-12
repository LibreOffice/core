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
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>

#include <boost/noncopyable.hpp>

namespace chart {

namespace {

typedef cppu::WeakComponentImplHelper<
    css::frame::XToolbarController, css::frame::XStatusListener,
    css::util::XUpdatable, css::lang::XInitialization,
    css::lang::XServiceInfo> ChartToolbarControllerBase;

}

class ChartToolbarController : private boost::noncopyable,
                               private cppu::BaseMutex,
                               public ChartToolbarControllerBase
{
public:
    ChartToolbarController(const css::uno::Sequence<css::uno::Any>& rProperties);
    virtual ~ChartToolbarController();

    // XToolbarContoller
    virtual void SAL_CALL execute(sal_Int16 nKeyModifier)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL click()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL doubleClick()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::awt::XWindow> SAL_CALL createPopupWindow()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::awt::XWindow> SAL_CALL
        createItemWindow(const css::uno::Reference<css::awt::XWindow>& rParent)
        throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& rSource)
        throw (css::uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rAny)
        throw (css::uno::Exception, std::exception) override;

    // XUpdatable
    virtual void SAL_CALL update()
        throw (css::uno::RuntimeException, std::exception) override;

    using cppu::WeakComponentImplHelperBase::disposing;

private:

    css::uno::Reference<css::frame::XFramesSupplier> mxFramesSupplier;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
