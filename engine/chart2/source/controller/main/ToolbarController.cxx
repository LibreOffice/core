/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ChartToolbarController.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>

namespace cpo::uno { class XComponentContext; }

namespace chart {

ChartToolbarController::ChartToolbarController(const cpo::uno::Sequence<cpo::uno::Any>& rProperties)
{
    for (const auto& rProperty : rProperties)
    {
        css::beans::PropertyValue aPropValue;
        rProperty >>= aPropValue;
        if (aPropValue.Name == "Frame")
        {
            mxFramesSupplier.set(aPropValue.Value, cpo::uno::UNO_QUERY);
            break;
        }
    }
}

ChartToolbarController::~ChartToolbarController()
{
}

void ChartToolbarController::execute(sal_Int16 /*nKeyModifier*/)
{
}

void ChartToolbarController::click()
{
    cpo::uno::Reference<css::frame::XFrame> xActiveFrame = mxFramesSupplier->getActiveFrame();
    if (!xActiveFrame.is())
        return;

    cpo::uno::Reference<css::frame::XController> xActiveController = xActiveFrame->getController();
    if (!xActiveController.is())
        return;

    cpo::uno::Reference<css::frame::XDispatch> xDispatch(xActiveController, cpo::uno::UNO_QUERY);
    if (!xDispatch.is())
        return;

    css::util::URL aURL;
    aURL.Path = u"FormatSelection"_ustr;
    xDispatch->dispatch(aURL, cpo::uno::Sequence<css::beans::PropertyValue>());
}

void ChartToolbarController::doubleClick()
{
    SAL_INFO("chart2", "double clicked");
}


cpo::uno::Reference<css::awt::XWindow> ChartToolbarController::createPopupWindow()
{
    return cpo::uno::Reference<css::awt::XWindow>();
}

cpo::uno::Reference<css::awt::XWindow> ChartToolbarController::createItemWindow(
        const cpo::uno::Reference<css::awt::XWindow>& /*rParent*/)
{
    return cpo::uno::Reference<css::awt::XWindow>();
}

void ChartToolbarController::statusChanged(const css::frame::FeatureStateEvent& /*rEvent*/)
{

}

void ChartToolbarController::disposing(const css::lang::EventObject& /*rSource*/)
{
}

void ChartToolbarController::initialize(const cpo::uno::Sequence<cpo::uno::Any>& /*rAny*/)
{
}

void ChartToolbarController::update()
{
}


OUString ChartToolbarController::getImplementationName()
{
    return u"org.libreoffice.chart2.Chart2ToolboxController"_ustr;
}

bool ChartToolbarController::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

cpo::uno::Sequence<OUString> ChartToolbarController::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ToolbarController"_ustr };
}

}

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
org_libreoffice_chart2_Chart2ToolboxController(cpo::uno::XComponentContext*, cpo::uno::Sequence<cpo::uno::Any> const & rProperties)
{
    return cppu::acquire(new ::chart::ChartToolbarController(rProperties));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
