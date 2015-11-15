/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ChartToolbarController.hxx"

#include <rtl/ref.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace chart {

ChartToolbarController::ChartToolbarController(const css::uno::Sequence<css::uno::Any>& rProperties):
    ChartToolbarControllerBase(m_aMutex)
{
    css::uno::Reference<css::frame::XFrame> xFrame;
    sal_Int32 nLength = rProperties.getLength();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        css::beans::PropertyValue aPropValue;
        rProperties[i] >>= aPropValue;
        if (aPropValue.Name == "Frame")
            aPropValue.Value >>= xFrame;
    }

    css::uno::Reference<css::frame::XFramesSupplier> xFramesSupplier(xFrame, css::uno::UNO_QUERY);
    mxFramesSupplier = xFramesSupplier;
}

ChartToolbarController::~ChartToolbarController()
{
}

void ChartToolbarController::execute(sal_Int16 /*nKeyModifier*/)
    throw (css::uno::RuntimeException, std::exception)
{
}

void ChartToolbarController::click()
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Reference<css::frame::XFrame> xActiveFrame = mxFramesSupplier->getActiveFrame();
    if (!xActiveFrame.is())
        return;

    css::uno::Reference<css::frame::XController> xActiveController = xActiveFrame->getController();
    if (!xActiveController.is())
        return;

    css::uno::Reference<css::frame::XDispatch> xDispatch(xActiveController, css::uno::UNO_QUERY);
    if (!xDispatch.is())
        return;

    css::util::URL aURL;
    aURL.Path = "FormatSelection";
    xDispatch->dispatch(aURL, css::uno::Sequence<css::beans::PropertyValue>());
}

void ChartToolbarController::doubleClick()
    throw (css::uno::RuntimeException, std::exception)
{
    SAL_INFO("chart2", "double clicked");
}


css::uno::Reference<css::awt::XWindow> ChartToolbarController::createPopupWindow()
        throw (css::uno::RuntimeException, std::exception)
{
    return css::uno::Reference<css::awt::XWindow>();
}

css::uno::Reference<css::awt::XWindow> ChartToolbarController::createItemWindow(
        const css::uno::Reference<css::awt::XWindow>& /*rParent*/)
        throw (css::uno::RuntimeException, std::exception)
{
    return css::uno::Reference<css::awt::XWindow>();
}

void ChartToolbarController::statusChanged(const css::frame::FeatureStateEvent& /*rEvent*/)
        throw (css::uno::RuntimeException, std::exception)
{

}

void ChartToolbarController::disposing(const css::lang::EventObject& /*rSource*/)
        throw (css::uno::RuntimeException, std::exception)
{
}

void ChartToolbarController::initialize(const css::uno::Sequence<css::uno::Any>& /*rAny*/)
    throw (css::uno::Exception, std::exception)
{
}

void ChartToolbarController::update()
    throw (css::uno::RuntimeException, std::exception)
{
}


OUString ChartToolbarController::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("org.libreoffice.chart2.ChartToolbarController");
}

sal_Bool ChartToolbarController::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> ChartToolbarController::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence<OUString> aServiceNames { "com.sun.star.frame.ToolbarController" };
    return aServiceNames;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
org_libreoffice_chart2_Chart2ToolboxController(css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const & rProperties)
{
    return cppu::acquire(new ::chart::ChartToolbarController(rProperties));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
