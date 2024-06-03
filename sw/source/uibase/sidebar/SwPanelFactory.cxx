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

#include "A11yCheckIssuesPanel.hxx"
#include "ThemePanel.hxx"
#include "StylePresetsPanel.hxx"
#include "PageStylesPanel.hxx"
#include "PageFormatPanel.hxx"
#include "PageHeaderPanel.hxx"
#include "PageFooterPanel.hxx"
#include "QuickFindPanel.hxx"
#include "WrapPropertyPanel.hxx"
#include "WriterInspectorTextPanel.hxx"
#include "TableEditPanel.hxx"
#include <navipi.hxx>
#include <redlndlg.hxx>

#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <vcl/weldutils.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>


using namespace css;
using namespace css::uno;

namespace {

typedef comphelper::WeakComponentImplHelper <
        css::ui::XUIElementFactory, css::lang::XServiceInfo
        > PanelFactoryInterfaceBase;

class SwPanelFactory final : public PanelFactoryInterfaceBase
{
private:
    SwPanelFactory(SwPanelFactory const&) = delete;
    SwPanelFactory& operator=(SwPanelFactory const&) = delete;

public:
    SwPanelFactory();

    // XUIElementFactory
    css::uno::Reference<css::ui::XUIElement> SAL_CALL createUIElement(
        const OUString& rsResourceURL,
        const css::uno::Sequence<css::beans::PropertyValue>& rArguments) override;

    OUString SAL_CALL getImplementationName() override
    { return u"org.apache.openoffice.comp.sw.sidebar.SwPanelFactory"_ustr; }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    { return {u"com.sun.star.ui.UIElementFactory"_ustr}; }
};

SwPanelFactory::SwPanelFactory()
{
}

Reference<ui::XUIElement> SAL_CALL SwPanelFactory::createUIElement (
    const OUString& rsResourceURL,
    const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
{
    Reference<ui::XUIElement> xElement;

    const ::comphelper::NamedValueCollection aArguments (rArguments);
    Reference<frame::XFrame> xFrame (aArguments.getOrDefault(u"Frame"_ustr, Reference<frame::XFrame>()));
    Reference<awt::XWindow> xParentWindow (aArguments.getOrDefault(u"ParentWindow"_ustr, Reference<awt::XWindow>()));
    const sal_uInt64 nBindingsValue (aArguments.getOrDefault(u"SfxBindings"_ustr, sal_uInt64(0)));
    SfxBindings* pBindings = reinterpret_cast<SfxBindings*>(nBindingsValue);

    weld::Widget* pParent(nullptr);
    if (weld::TransportAsXWindow* pTunnel = dynamic_cast<weld::TransportAsXWindow*>(xParentWindow.get()))
        pParent = pTunnel->getWidget();

    if (!pParent)
        throw RuntimeException(
            u"PanelFactory::createUIElement called without ParentWindow"_ustr,
            nullptr);
    if ( ! xFrame.is())
        throw RuntimeException(
            u"PanelFactory::createUIElement called without Frame"_ustr,
            nullptr);
    if (pBindings == nullptr)
        throw RuntimeException(
            u"PanelFactory::createUIElement called without SfxBindings"_ustr,
            nullptr);

    if(rsResourceURL.endsWith("/PageStylesPanel"))
    {
        std::unique_ptr<PanelLayout> xPanel = sw::sidebar::PageStylesPanel::Create( pParent, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            std::move(xPanel),
            ui::LayoutSize(-1,-1,-1));
    }
    else if(rsResourceURL.endsWith("/PageFormatPanel"))
    {
        std::unique_ptr<PanelLayout> xPanel = sw::sidebar::PageFormatPanel::Create( pParent, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            std::move(xPanel),
            ui::LayoutSize(-1,-1,-1));
    }
    else if(rsResourceURL.endsWith("/PageHeaderPanel"))
    {
        std::unique_ptr<PanelLayout> xPanel = sw::sidebar::PageHeaderPanel::Create( pParent, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            std::move(xPanel),
            ui::LayoutSize(-1,-1,-1));
    }
    else if(rsResourceURL.endsWith("/PageFooterPanel"))
    {
        std::unique_ptr<PanelLayout> xPanel = sw::sidebar::PageFooterPanel::Create( pParent, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            std::move(xPanel),
            ui::LayoutSize(-1,-1,-1));
    }
    else if (rsResourceURL.endsWith("/WrapPropertyPanel"))
    {
        std::unique_ptr<PanelLayout> xPanel = sw::sidebar::WrapPropertyPanel::Create( pParent, xFrame, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            std::move(xPanel),
            ui::LayoutSize(-1,-1,-1));
    }
    else if (rsResourceURL.endsWith("/NavigatorPanel"))
    {
        std::unique_ptr<PanelLayout> xPanel = SwNavigationPI::Create( pParent, xFrame, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            std::move(xPanel),
            ui::LayoutSize(0,-1,-1));
    }
    else if (rsResourceURL.endsWith("/ManageChangesPanel"))
    {
        auto xPanel = std::make_unique<SwRedlineAcceptPanel>(pParent);
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            std::move(xPanel),
            ui::LayoutSize(0,-1,-1));
    }
    else if (rsResourceURL.endsWith("/WriterInspectorTextPanel"))
    {
        std::unique_ptr<PanelLayout> xPanel = sw::sidebar::WriterInspectorTextPanel::Create(pParent);
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            std::move(xPanel),
            ui::LayoutSize(0,-1,-1));
    }
    else if (rsResourceURL.endsWith("/StylePresetsPanel"))
    {
        std::unique_ptr<PanelLayout> xPanel = sw::sidebar::StylePresetsPanel::Create(pParent);
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
                        rsResourceURL, xFrame, std::move(xPanel), ui::LayoutSize(-1,-1,-1));
    }
    else if (rsResourceURL.endsWith("/ThemePanel"))
    {
        std::unique_ptr<PanelLayout> xPanel = sw::sidebar::ThemePanel::Create(pParent);
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
                        rsResourceURL, xFrame, std::move(xPanel), ui::LayoutSize(-1,-1,-1));
    }
    else if (rsResourceURL.endsWith("/TableEditPanel"))
    {
        std::unique_ptr<PanelLayout> xPanel = sw::sidebar::TableEditPanel::Create(pParent, xFrame, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
                        rsResourceURL, xFrame, std::move(xPanel), ui::LayoutSize(-1,-1,-1));
    }
    else if (rsResourceURL.endsWith("/A11yCheckIssuesPanel"))
    {
        std::unique_ptr<PanelLayout> xPanel = sw::sidebar::A11yCheckIssuesPanel::Create(pParent, pBindings);
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
                        rsResourceURL, xFrame, std::move(xPanel), ui::LayoutSize(-1,-1,-1));
    }
    else if (rsResourceURL.endsWith("/QuickFindPanel"))
    {
        std::unique_ptr<PanelLayout> xPanel = sw::sidebar::QuickFindPanel::Create(pParent);
        xElement = sfx2::sidebar::SidebarPanelBase::Create(rsResourceURL, xFrame, std::move(xPanel),
                                                           ui::LayoutSize(-1, -1, -1));
    }

    return xElement;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
org_apache_openoffice_comp_sw_sidebar_SwPanelFactory_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SwPanelFactory());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
