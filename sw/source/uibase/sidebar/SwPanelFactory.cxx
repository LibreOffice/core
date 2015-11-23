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

#include <com/sun/star/ui/XUIElementFactory.hpp>

#include <ThemePanel.hxx>
#include <StylePresetsPanel.hxx>
#include <PagePropertyPanel.hxx>
#include <WrapPropertyPanel.hxx>
#include <navipi.hxx>
#include <redlndlg.hxx>

#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <boost/noncopyable.hpp>

using namespace css;
using namespace css::uno;

namespace {

typedef ::cppu::WeakComponentImplHelper <
        css::ui::XUIElementFactory, css::lang::XServiceInfo
        > PanelFactoryInterfaceBase;

class SwPanelFactory
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PanelFactoryInterfaceBase
{
public:
    SwPanelFactory();
    virtual ~SwPanelFactory();

    // XUIElementFactory
    css::uno::Reference<css::ui::XUIElement> SAL_CALL createUIElement(
        const OUString& rsResourceURL,
        const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
        throw(
            css::container::NoSuchElementException,
            css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception ) override;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("org.apache.openoffice.comp.sw.sidebar.SwPanelFactory"); }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    { return cppu::supportsService(this, ServiceName); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    { return css::uno::Sequence<OUString>{"com.sun.star.ui.UIElementFactory"}; }
};

SwPanelFactory::SwPanelFactory()
    : PanelFactoryInterfaceBase(m_aMutex)
{
}

SwPanelFactory::~SwPanelFactory()
{
}

Reference<ui::XUIElement> SAL_CALL SwPanelFactory::createUIElement (
    const OUString& rsResourceURL,
    const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
    throw(
        container::NoSuchElementException,
        lang::IllegalArgumentException,
        RuntimeException, std::exception)
{
    Reference<ui::XUIElement> xElement;

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

    if (rsResourceURL.endsWith("/PagePropertyPanel"))
    {
        VclPtr<vcl::Window> pPanel = sw::sidebar::PagePropertyPanel::Create( pParentWindow, xFrame, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            pPanel,
            ui::LayoutSize(-1,-1,-1));
    }
    else if (rsResourceURL.endsWith("/WrapPropertyPanel"))
    {
        VclPtr<vcl::Window> pPanel = sw::sidebar::WrapPropertyPanel::Create( pParentWindow, xFrame, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            pPanel,
            ui::LayoutSize(-1,-1,-1));
    }
    else if (rsResourceURL.endsWith("/NavigatorPanel"))
    {
        VclPtrInstance<SwNavigationPI> pPanel(pBindings, nullptr, pParentWindow);
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            pPanel,
            ui::LayoutSize(0,-1,-1));
    }
    else if (rsResourceURL.endsWith("/ManageChangesPanel"))
    {
        VclPtrInstance<SwRedlineAcceptPanel> pPanel(pParentWindow, xFrame);
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            pPanel,
            ui::LayoutSize(-1,-1,-1));
    }
    else if (rsResourceURL.endsWith("/StylePresetsPanel"))
    {
        VclPtr<vcl::Window> pPanel = sw::sidebar::StylePresetsPanel::Create(pParentWindow, xFrame);
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
                        rsResourceURL, xFrame, pPanel, ui::LayoutSize(-1,-1,-1));
    }
    else if (rsResourceURL.endsWith("/ThemePanel"))
    {
        VclPtr<vcl::Window> pPanel = sw::sidebar::ThemePanel::Create(pParentWindow, xFrame);
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
                        rsResourceURL, xFrame, pPanel, ui::LayoutSize(-1,-1,-1));
    }

    return xElement;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
org_apache_openoffice_comp_sw_sidebar_SwPanelFactory_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwPanelFactory());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
