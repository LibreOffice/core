/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <vcl/weldutils.hxx>

#include "SmElementsPanel.hxx"
#include "SmPropertiesPanel.hxx"

namespace
{
typedef comphelper::WeakComponentImplHelper<css::ui::XUIElementFactory, css::lang::XServiceInfo>
    PanelFactoryInterfaceBase;

class SmPanelFactory final : public PanelFactoryInterfaceBase
{
public:
    SmPanelFactory() = default;

    SmPanelFactory(const SmPanelFactory&) = delete;
    const SmPanelFactory& operator=(const SmPanelFactory&) = delete;

    // XUIElementFactory
    css::uno::Reference<css::ui::XUIElement> SAL_CALL
    createUIElement(const OUString& ResourceURL,
                    const css::uno::Sequence<css::beans::PropertyValue>& Arguments) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

css::uno::Reference<css::ui::XUIElement> SAL_CALL SmPanelFactory::createUIElement(
    const OUString& ResourceURL, const css::uno::Sequence<css::beans::PropertyValue>& Arguments)
{
    try
    {
        const comphelper::NamedValueCollection aArguments(Arguments);
        auto xFrame(
            aArguments.getOrDefault(u"Frame"_ustr, css::uno::Reference<css::frame::XFrame>()));
        auto xParentWindow(aArguments.getOrDefault(u"ParentWindow"_ustr,
                                                   css::uno::Reference<css::awt::XWindow>()));
        const sal_uInt64 nBindingsValue(
            aArguments.getOrDefault(u"SfxBindings"_ustr, sal_uInt64(0)));
        SfxBindings* pBindings = reinterpret_cast<SfxBindings*>(nBindingsValue);

        weld::Widget* pParent(nullptr);
        if (auto pTunnel = dynamic_cast<weld::TransportAsXWindow*>(xParentWindow.get()))
            pParent = pTunnel->getWidget();

        if (!pParent)
            throw css::uno::RuntimeException(
                u"SmPanelFactory::createUIElement: no ParentWindow"_ustr);
        if (!xFrame)
            throw css::uno::RuntimeException(u"SmPanelFactory::createUIElement: no Frame"_ustr);
        if (!pBindings)
            throw css::uno::RuntimeException(
                u"SmPanelFactory::createUIElement: no SfxBindings"_ustr);

        std::unique_ptr<PanelLayout> pPanel;
        css::ui::LayoutSize aLayoutSize{ -1, -1, -1 };
        if (ResourceURL.endsWith("/MathPropertiesPanel"))
        {
            pPanel = sm::sidebar::SmPropertiesPanel::Create(*pParent, xFrame);
        }
        else if (ResourceURL.endsWith("/MathElementsPanel"))
        {
            pPanel = sm::sidebar::SmElementsPanel::Create(*pParent, *pBindings);
            aLayoutSize = { 300, -1, -1 };
        }

        if (pPanel)
            return sfx2::sidebar::SidebarPanelBase::Create(ResourceURL, xFrame, std::move(pPanel),
                                                           aLayoutSize);
    }
    catch (const css::uno::RuntimeException&)
    {
        throw;
    }
    catch (const css::uno::Exception&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
            u"SmPanelFactory::createUIElement exception"_ustr, nullptr, anyEx);
    }

    return {};
}

OUString SmPanelFactory::getImplementationName()
{
    return u"org.libreoffice.comp.Math.sidebar.SmPanelFactory"_ustr;
}

sal_Bool SmPanelFactory::supportsService(OUString const& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> SmPanelFactory::getSupportedServiceNames()
{
    return { u"com.sun.star.ui.UIElementFactory"_ustr };
}

} // end of unnamed namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_Math_sidebar_SmPanelFactory(css::uno::XComponentContext*,
                                                 css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SmPanelFactory);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
