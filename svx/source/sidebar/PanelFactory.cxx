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

#include <config_features.h>

#include "text/TextPropertyPanel.hxx"
#include "styles/StylesPropertyPanel.hxx"
#include "paragraph/ParaPropertyPanel.hxx"
#include "lists/ListsPropertyPanel.hxx"
#include "area/AreaPropertyPanel.hxx"
#include "fontwork/FontworkPropertyPanel.hxx"
#include "shadow/ShadowPropertyPanel.hxx"
#include "effect/EffectPropertyPanel.hxx"
#include "graphic/GraphicPropertyPanel.hxx"
#include "line/LinePropertyPanel.hxx"
#include "possize/PosSizePropertyPanel.hxx"
#include <DefaultShapesPanel.hxx>
#if HAVE_FEATURE_AVMEDIA
#include "media/MediaPlaybackPanel.hxx"
#endif
#include <GalleryControl.hxx>
#include "EmptyPanel.hxx"
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/templdlg.hxx>
#include <vcl/weldutils.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/XSidebar.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace css;
using namespace css::uno;
using namespace svx::sidebar;


namespace {

/* Why this is not used ? Doesn't it need to inherit from XServiceInfo ?
#define IMPLEMENTATION_NAME "org.apache.openoffice.comp.svx.sidebar.PanelFactory"
#define SERVICE_NAME "com.sun.star.ui.UIElementFactory"
*/

typedef ::cppu::WeakComponentImplHelper< css::ui::XUIElementFactory, css::lang::XServiceInfo >
    PanelFactoryInterfaceBase;

class PanelFactory
    : private ::cppu::BaseMutex,
      public PanelFactoryInterfaceBase
{
public:
    PanelFactory();
    PanelFactory(const PanelFactory&) = delete;
    PanelFactory& operator=(const PanelFactory&) = delete;

    // XUIElementFactory
    css::uno::Reference<css::ui::XUIElement> SAL_CALL createUIElement (
        const OUString& rsResourceURL,
        const ::css::uno::Sequence<css::beans::PropertyValue>& rArguments) override;

    OUString SAL_CALL getImplementationName() override
    { return "org.apache.openoffice.comp.svx.sidebar.PanelFactory"; }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    { return {"com.sun.star.ui.UIElementFactory"}; }
};

PanelFactory::PanelFactory()
    : PanelFactoryInterfaceBase(m_aMutex)
{
}

Reference<ui::XUIElement> SAL_CALL PanelFactory::createUIElement (
    const OUString& rsResourceURL,
    const ::css::uno::Sequence<css::beans::PropertyValue>& rArguments)
{
    const ::comphelper::NamedValueCollection aArguments (rArguments);
    Reference<frame::XFrame> xFrame (aArguments.getOrDefault("Frame", Reference<frame::XFrame>()));
    Reference<awt::XWindow> xParentWindow (aArguments.getOrDefault("ParentWindow", Reference<awt::XWindow>()));
    Reference<ui::XSidebar> xSidebar (aArguments.getOrDefault("Sidebar", Reference<ui::XSidebar>()));
    const sal_uInt64 nBindingsValue (aArguments.getOrDefault("SfxBindings", sal_uInt64(0)));
    SfxBindings* pBindings = reinterpret_cast<SfxBindings*>(nBindingsValue);

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
    if (pBindings == nullptr)
        throw RuntimeException(
            "PanelFactory::createUIElement called without SfxBindings",
            nullptr);

    std::unique_ptr<PanelLayout> xControl;
    ui::LayoutSize aLayoutSize (-1,-1,-1);

    if (rsResourceURL.endsWith("/TextPropertyPanel"))
    {
        xControl = TextPropertyPanel::Create(pParent, xFrame);
    }
    else if (rsResourceURL.endsWith("/StylesPropertyPanel"))
    {
        xControl = StylesPropertyPanel::Create(pParent, xFrame);
    }
    else if (rsResourceURL.endsWith("/ParaPropertyPanel"))
    {
        xControl = ParaPropertyPanel::Create(pParent, xFrame, pBindings, xSidebar);
    }
    else if (rsResourceURL.endsWith("/ListsPropertyPanel"))
    {
        xControl = ListsPropertyPanel::Create(pParent, xFrame);
    }
    else if (rsResourceURL.endsWith("/AreaPropertyPanel"))
    {
        xControl = AreaPropertyPanel::Create(pParent, xFrame, pBindings);
    }
    else if (rsResourceURL.endsWith("/FontworkPropertyPanel"))
    {
        xControl = FontworkPropertyPanel::Create(pParent, xFrame);
    }
    else if (rsResourceURL.endsWith("/ShadowPropertyPanel"))
    {
        xControl = ShadowPropertyPanel::Create(pParent, pBindings);
    }
    else if (rsResourceURL.endsWith("/EffectPropertyPanel"))
    {
        xControl = EffectPropertyPanel::Create(pParent, pBindings);
    }
    else if (rsResourceURL.endsWith("/GraphicPropertyPanel"))
    {
        xControl = GraphicPropertyPanel::Create(pParent, pBindings);
    }
    else if (rsResourceURL.endsWith("/LinePropertyPanel"))
    {
        xControl = LinePropertyPanel::Create(pParent, xFrame, pBindings);
    }
    else if (rsResourceURL.endsWith("/PosSizePropertyPanel"))
    {
        xControl = PosSizePropertyPanel::Create(pParent, xFrame, pBindings, xSidebar);
    }
    else if (rsResourceURL.endsWith("/DefaultShapesPanel"))
    {
        xControl = DefaultShapesPanel::Create(pParent, xFrame);
    }
#if HAVE_FEATURE_AVMEDIA
    else if (rsResourceURL.endsWith("/MediaPlaybackPanel"))
    {
        xControl = MediaPlaybackPanel::Create(pParent, pBindings);
    }
#endif
    else if (rsResourceURL.endsWith("/GalleryPanel"))
    {
        xControl = std::make_unique<GalleryControl>(pParent);
        aLayoutSize = ui::LayoutSize(300,-1,400);
    }
    else if (rsResourceURL.endsWith("/StyleListPanel"))
    {
        xControl = std::make_unique<SfxTemplatePanelControl>(pBindings, pParent);
        aLayoutSize = ui::LayoutSize(0,-1,-1);
    }
    else if (rsResourceURL.endsWith("/EmptyPanel"))
    {
        xControl = std::make_unique<EmptyPanel>(pParent);
        aLayoutSize = ui::LayoutSize(20,-1, 50);
    }

    if (xControl)
    {
        return sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            std::move(xControl),
            aLayoutSize);
    }
    else
        return Reference<ui::XUIElement>();
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
org_apache_openoffice_comp_svx_sidebar_PanelFactory_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new PanelFactory);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
