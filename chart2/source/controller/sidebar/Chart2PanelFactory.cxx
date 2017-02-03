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
#include <sfx2/sfxbasecontroller.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/supportsservice.hxx>

#include "ChartElementsPanel.hxx"
#include "ChartSeriesPanel.hxx"
#include "ChartController.hxx"
#include "ChartAxisPanel.hxx"
#include "ChartErrorBarPanel.hxx"
#include "ChartAreaPanel.hxx"
#include "ChartLinePanel.hxx"

using namespace css::uno;
using ::rtl::OUString;

namespace chart { namespace sidebar {

ChartPanelFactory::ChartPanelFactory()
    : PanelFactoryInterfaceBase(m_aMutex)
{
}

ChartPanelFactory::~ChartPanelFactory()
{
}

Reference<css::ui::XUIElement> SAL_CALL ChartPanelFactory::createUIElement (
    const ::rtl::OUString& rsResourceURL,
    const ::css::uno::Sequence<css::beans::PropertyValue>& rArguments)
{
    Reference<css::ui::XUIElement> xElement;

    try
    {
        const ::comphelper::NamedValueCollection aArguments (rArguments);
        Reference<css::frame::XFrame> xFrame (aArguments.getOrDefault("Frame", Reference<css::frame::XFrame>()));
        Reference<css::awt::XWindow> xParentWindow (aArguments.getOrDefault("ParentWindow", Reference<css::awt::XWindow>()));
        Reference<css::frame::XController> xController (aArguments.getOrDefault("Controller", Reference<css::frame::XController>()));

        VclPtr<vcl::Window> pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
        if ( ! xParentWindow.is() || pParentWindow==nullptr)
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

        sal_Int32 nMinimumSize = -1;
        VclPtr<vcl::Window> pPanel;
        if (rsResourceURL.endsWith("/ElementsPanel"))
            pPanel = ChartElementsPanel::Create( pParentWindow, xFrame, pController );
        else if (rsResourceURL.endsWith("/SeriesPanel"))
            pPanel = ChartSeriesPanel::Create(pParentWindow, xFrame, pController);
        else if (rsResourceURL.endsWith("/AxisPanel"))
            pPanel = ChartAxisPanel::Create(pParentWindow, xFrame, pController);
        else if (rsResourceURL.endsWith("/ErrorBarPanel"))
            pPanel = ChartErrorBarPanel::Create(pParentWindow, xFrame, pController);
        else if (rsResourceURL.endsWith("/AreaPanel"))
            pPanel = ChartAreaPanel::Create(pParentWindow, xFrame, pController);
        else if (rsResourceURL.endsWith("/LinePanel"))
            pPanel = ChartLinePanel::Create(pParentWindow, xFrame, pController);

        if (pPanel)
            xElement = sfx2::sidebar::SidebarPanelBase::Create(
                rsResourceURL,
                xFrame,
                pPanel,
                css::ui::LayoutSize(nMinimumSize,-1,-1));
    }
    catch (const css::uno::RuntimeException &)
    {
        throw;
    }
    catch (const css::uno::Exception& e)
    {
        throw css::lang::WrappedTargetRuntimeException(
            "ChartPanelFactory::createUIElement exception",
            nullptr, css::uno::Any(e));
    }

    return xElement;
}

OUString ChartPanelFactory::getImplementationName()
{
    return OUString("org.libreoffice.comp.chart2.sidebar.ChartPanelFactory");
}

sal_Bool ChartPanelFactory::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> ChartPanelFactory::getSupportedServiceNames()
{
    return { "com.sun.star.ui.UIElementFactory" };
}

} } // end of namespace chart::sidebar

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
org_libreoffice_comp_chart2_sidebar_ChartPanelFactory(css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::sidebar::ChartPanelFactory());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
