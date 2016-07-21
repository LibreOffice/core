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

#include "text/TextPropertyPanel.hxx"
#include "styles/StylesPropertyPanel.hxx"
#include "paragraph/ParaPropertyPanel.hxx"
#include "area/AreaPropertyPanel.hxx"
#include "shadow/ShadowPropertyPanel.hxx"
#include "graphic/GraphicPropertyPanel.hxx"
#include "line/LinePropertyPanel.hxx"
#include "possize/PosSizePropertyPanel.hxx"
#include "shapes/DefaultShapesPanel.hxx"
#include "media/MediaPlaybackPanel.hxx"
#include "GalleryControl.hxx"
#include "EmptyPanel.hxx"
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <sfx2/templdlg.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <rtl/ref.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/XSidebar.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>

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
    virtual ~PanelFactory();
    PanelFactory(const PanelFactory&) = delete;
    PanelFactory& operator=(const PanelFactory&) = delete;

    // XUIElementFactory
    css::uno::Reference<css::ui::XUIElement> SAL_CALL createUIElement (
        const ::rtl::OUString& rsResourceURL,
        const ::css::uno::Sequence<css::beans::PropertyValue>& rArguments)
        throw(
            css::container::NoSuchElementException,
            css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("org.apache.openoffice.comp.svx.sidebar.PanelFactory"); }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    { return cppu::supportsService(this, ServiceName); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    { return css::uno::Sequence<OUString>{"com.sun.star.ui.UIElementFactory"}; }
};

PanelFactory::PanelFactory()
    : PanelFactoryInterfaceBase(m_aMutex)
{
}


PanelFactory::~PanelFactory()
{
}


Reference<ui::XUIElement> SAL_CALL PanelFactory::createUIElement (
    const ::rtl::OUString& rsResourceURL,
    const ::css::uno::Sequence<css::beans::PropertyValue>& rArguments)
    throw(
        container::NoSuchElementException,
        lang::IllegalArgumentException,
        RuntimeException, std::exception)
{
    const ::comphelper::NamedValueCollection aArguments (rArguments);
    Reference<frame::XFrame> xFrame (aArguments.getOrDefault("Frame", Reference<frame::XFrame>()));
    Reference<awt::XWindow> xParentWindow (aArguments.getOrDefault("ParentWindow", Reference<awt::XWindow>()));
    Reference<ui::XSidebar> xSidebar (aArguments.getOrDefault("Sidebar", Reference<ui::XSidebar>()));
    const sal_uInt64 nBindingsValue (aArguments.getOrDefault("SfxBindings", sal_uInt64(0)));
    SfxBindings* pBindings = reinterpret_cast<SfxBindings*>(nBindingsValue);
    vcl::EnumContext aContext (
        aArguments.getOrDefault("ApplicationName", OUString()),
        aArguments.getOrDefault("ContextName", OUString()));

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

    VclPtr<vcl::Window> pControl;
    ui::LayoutSize aLayoutSize (-1,-1,-1);

    if (rsResourceURL.endsWith("/TextPropertyPanel"))
    {
        pControl = TextPropertyPanel::Create(pParentWindow, xFrame, pBindings, aContext);
    }
    else if (rsResourceURL.endsWith("/StylesPropertyPanel"))
    {
        pControl = StylesPropertyPanel::Create(pParentWindow, xFrame);
    }
    else if (rsResourceURL.endsWith("/ParaPropertyPanel"))
    {
        pControl = ParaPropertyPanel::Create(pParentWindow, xFrame, pBindings, xSidebar);
    }
    else if (rsResourceURL.endsWith("/AreaPropertyPanel"))
    {
        pControl = AreaPropertyPanel::Create(pParentWindow, xFrame, pBindings);
    }
    else if (rsResourceURL.endsWith("/ShadowPropertyPanel"))
    {
        pControl = ShadowPropertyPanel::Create(pParentWindow, xFrame, pBindings);
    }
    else if (rsResourceURL.endsWith("/GraphicPropertyPanel"))
    {
        pControl = GraphicPropertyPanel::Create(pParentWindow, xFrame, pBindings);
    }
    else if (rsResourceURL.endsWith("/LinePropertyPanel"))
    {
        pControl = LinePropertyPanel::Create(pParentWindow, xFrame, pBindings);
    }
    else if (rsResourceURL.endsWith("/PosSizePropertyPanel"))
    {
        pControl = PosSizePropertyPanel::Create(pParentWindow, xFrame, pBindings, xSidebar);
    }
    else if (rsResourceURL.endsWith("/DefaultShapesPanel"))
    {
        pControl = DefaultShapesPanel::Create(pParentWindow, xFrame);
    }
    else if (rsResourceURL.endsWith("/MediaPlaybackPanel"))
    {
        pControl = MediaPlaybackPanel::Create(pParentWindow, xFrame, pBindings);
    }
    else if (rsResourceURL.endsWith("/GalleryPanel"))
    {
        pControl.reset(VclPtr<GalleryControl>::Create(pBindings, pParentWindow));
        aLayoutSize = ui::LayoutSize(300,-1,400);
    }
    else if (rsResourceURL.endsWith("/StyleListPanel"))
    {
        pControl.reset(VclPtr<SfxTemplatePanelControl>::Create(pBindings, pParentWindow));
        aLayoutSize = ui::LayoutSize(0,-1,-1);
    }
    else if (rsResourceURL.endsWith("/EmptyPanel"))
    {
        pControl.reset(VclPtr<EmptyPanel>::Create(pParentWindow));
        aLayoutSize = ui::LayoutSize(20,-1, 50);
    }

    if (pControl)
    {
        return sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            pControl,
            aLayoutSize);
    }
    else
        return Reference<ui::XUIElement>();
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
org_apache_openoffice_comp_svx_sidebar_PanelFactory_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new PanelFactory);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
