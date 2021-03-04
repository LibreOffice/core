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

#include "Chart2PanelFactory.hxx"

#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/weldutils.hxx>

#include "ChartElementsPanel.hxx"
#include "ChartTypePanel.hxx"
#include "ChartSeriesPanel.hxx"
#include <ChartController.hxx>
#include "ChartAxisPanel.hxx"
#include "ChartErrorBarPanel.hxx"
#include "ChartAreaPanel.hxx"
#include "ChartLinePanel.hxx"

using namespace css::uno;

namespace chart::sidebar {

ChartPanelFactory::ChartPanelFactory()
    : PanelFactoryInterfaceBase(m_aMutex)
{
}

ChartPanelFactory::~ChartPanelFactory()
{
}

Reference<css::ui::XUIElement> SAL_CALL ChartPanelFactory::createUIElement (
    const OUString& rsResourceURL,
    const ::css::uno::Sequence<css::beans::PropertyValue>& rArguments)
{
    Reference<css::ui::XUIElement> xElement;

    try
    {
        const ::comphelper::NamedValueCollection aArguments (rArguments);
        Reference<css::frame::XFrame> xFrame (aArguments.getOrDefault("Frame", Reference<css::frame::XFrame>()));
        Reference<css::awt::XWindow> xParentWindow (aArguments.getOrDefault("ParentWindow", Reference<css::awt::XWindow>()));
        Reference<css::frame::XController> xController (aArguments.getOrDefault("Controller", Reference<css::frame::XController>()));

        weld::Widget* pParent(nullptr);
        if (weld::TransportAsXWindow* pTunnel = dynamic_cast<weld::TransportAsXWindow*>(xParentWindow.get()))
            pParent = pTunnel->getWidget();

        if (!pParent)
            throw RuntimeException(
                "PanelFactory::createUIElement called without ParentWindow",
                nullptr);
        if ( ! xFrame.is())
            throw RuntimeException(
                "PanelFactory::createUIElement called without Frame",
                nullptr);
        if (!xController.is())
            throw RuntimeException(
                "ChartPanelFactory::createUIElement called without Controller",
                nullptr);

        ChartController* pController = dynamic_cast<ChartController*>(xController.get());
        if (!pController)
            throw RuntimeException(
                "ChartPanelFactory::createUIElement called without valid ChartController",
                nullptr);

        std::unique_ptr<PanelLayout> xPanel;
        if (rsResourceURL.endsWith("/ElementsPanel"))
            xPanel = ChartElementsPanel::Create( pParent, pController );
        else if (rsResourceURL.endsWith("/TypePanel"))
            xPanel = std::make_unique<ChartTypePanel>(pParent, pController);
        else if (rsResourceURL.endsWith("/SeriesPanel"))
            xPanel = ChartSeriesPanel::Create(pParent, pController);
        else if (rsResourceURL.endsWith("/AxisPanel"))
            xPanel = ChartAxisPanel::Create(pParent, pController);
        else if (rsResourceURL.endsWith("/ErrorBarPanel"))
            xPanel = ChartErrorBarPanel::Create(pParent, pController);
        else if (rsResourceURL.endsWith("/AreaPanel"))
            xPanel = ChartAreaPanel::Create(pParent, xFrame, pController);
        else if (rsResourceURL.endsWith("/LinePanel"))
            xPanel = ChartLinePanel::Create(pParent, xFrame, pController);

        if (xPanel)
            xElement = sfx2::sidebar::SidebarPanelBase::Create(
                rsResourceURL,
                xFrame,
                std::move(xPanel),
                css::ui::LayoutSize(-1,-1,-1));
    }
    catch (const css::uno::RuntimeException &)
    {
        throw;
    }
    catch (const css::uno::Exception&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
            "ChartPanelFactory::createUIElement exception",
            nullptr, anyEx );
    }

    return xElement;
}

OUString ChartPanelFactory::getImplementationName()
{
    return "org.libreoffice.comp.chart2.sidebar.ChartPanelFactory";
}

sal_Bool ChartPanelFactory::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> ChartPanelFactory::getSupportedServiceNames()
{
    return { "com.sun.star.ui.UIElementFactory" };
}

} // end of namespace chart::sidebar

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_chart2_sidebar_ChartPanelFactory(css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::sidebar::ChartPanelFactory());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
