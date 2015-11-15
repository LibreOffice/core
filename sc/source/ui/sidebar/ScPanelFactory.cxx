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

#include "ScPanelFactory.hxx"

#include "AlignmentPropertyPanel.hxx"
#include "CellAppearancePropertyPanel.hxx"
#include "NumberFormatPropertyPanel.hxx"
#include <navipi.hxx>
#include <dwfunctr.hxx>
#include "sc.hrc"

#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <boost/bind.hpp>

using namespace css;
using namespace css::uno;
using ::rtl::OUString;

namespace sc { namespace sidebar {

ScPanelFactory::ScPanelFactory()
    : PanelFactoryInterfaceBase(m_aMutex)
{
}

ScPanelFactory::~ScPanelFactory()
{
}

Reference<ui::XUIElement> SAL_CALL ScPanelFactory::createUIElement (
    const ::rtl::OUString& rsResourceURL,
    const ::css::uno::Sequence<css::beans::PropertyValue>& rArguments)
    throw(
        container::NoSuchElementException,
        lang::IllegalArgumentException,
        RuntimeException, std::exception)
{
    Reference<ui::XUIElement> xElement;

    try
    {
        const ::comphelper::NamedValueCollection aArguments (rArguments);
        Reference<frame::XFrame> xFrame (aArguments.getOrDefault("Frame", Reference<frame::XFrame>()));
        Reference<awt::XWindow> xParentWindow (aArguments.getOrDefault("ParentWindow", Reference<awt::XWindow>()));
        const sal_uInt64 nBindingsValue (aArguments.getOrDefault("SfxBindings", sal_uInt64(0)));
        SfxBindings* pBindings = reinterpret_cast<SfxBindings*>(nBindingsValue);

        vcl::Window* pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
        if ( ! xParentWindow.is() || pParentWindow==nullptr)
            throw RuntimeException(
                "PanelFactory::createUIElement called without ParentWindow",
                nullptr);
        if ( ! xFrame.is())
            throw RuntimeException(
                "PanelFactory::createUIElement called without Frame",
                nullptr);
        if (pBindings == nullptr)
            throw RuntimeException(
                "PanelFactory::createUIElement called without SfxBindings",
                nullptr);

        sal_Int32 nMinimumSize = -1;
        VclPtr<vcl::Window> pPanel;
        if (rsResourceURL.endsWith("/AlignmentPropertyPanel"))
            pPanel = AlignmentPropertyPanel::Create( pParentWindow, xFrame, pBindings );
        else if (rsResourceURL.endsWith("/CellAppearancePropertyPanel"))
            pPanel = CellAppearancePropertyPanel::Create( pParentWindow, xFrame, pBindings );
        else if (rsResourceURL.endsWith("/NumberFormatPropertyPanel"))
            pPanel = NumberFormatPropertyPanel::Create( pParentWindow, xFrame, pBindings );
        else if (rsResourceURL.endsWith("/NavigatorPanel"))
        {
            pPanel = VclPtr<ScNavigatorDlg>::Create(pBindings, nullptr, pParentWindow, false);
            nMinimumSize = 0;
        }
        else if (rsResourceURL.endsWith("/FunctionsPanel"))
        {
            pPanel = VclPtr<ScFunctionDockWin>::Create(pBindings, nullptr, pParentWindow, ScResId(FID_FUNCTION_BOX));
            nMinimumSize = 0;
        }

        if (pPanel)
            xElement = sfx2::sidebar::SidebarPanelBase::Create(
                rsResourceURL,
                xFrame,
                pPanel,
                ui::LayoutSize(nMinimumSize,-1,-1));
    }
    catch (const uno::RuntimeException &)
    {
        throw;
    }
    catch (const uno::Exception& e)
    {
        throw lang::WrappedTargetRuntimeException(
            OUString("ScPanelFactory::createUIElement exception"),
            nullptr, uno::makeAny(e));
    }

    return xElement;
}

OUString ScPanelFactory::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("org.apache.openoffice.comp.sc.sidebar.ScPanelFactory");
}

sal_Bool ScPanelFactory::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> ScPanelFactory::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    css::uno::Sequence<OUString> aServiceNames { "com.sun.star.ui.UIElementFactory" };
    return aServiceNames;
}

} } // end of namespace sc::sidebar

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
ScPanelFactory_get_implementation(css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sc::sidebar::ScPanelFactory());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
