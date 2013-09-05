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
#include "sidebar/PanelFactory.hxx"

#include "text/TextPropertyPanel.hxx"
#include "paragraph/ParaPropertyPanel.hxx"
#include "area/AreaPropertyPanel.hxx"
#include "graphic/GraphicPropertyPanel.hxx"
#include "line/LinePropertyPanel.hxx"
#include "possize/PosSizePropertyPanel.hxx"
#include "insert/InsertPropertyPanel.hxx"
#include "GalleryControl.hxx"
#include "debug/ColorPanel.hxx"
#include "debug/ContextPanel.hxx"
#include "debug/NotYetImplementedPanel.hxx"
#include "EmptyPanel.hxx"
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <sfx2/templdlg.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <rtl/ref.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <com/sun/star/ui/XSidebar.hpp>

#include <boost/bind.hpp>


using namespace css;
using namespace cssu;
using ::rtl::OUString;


namespace svx { namespace sidebar {

#define IMPLEMENTATION_NAME "org.apache.openoffice.comp.svx.sidebar.PanelFactory"
#define SERVICE_NAME "com.sun.star.ui.UIElementFactory"


::rtl::OUString SAL_CALL PanelFactory::getImplementationName (void)
{
    return A2S(IMPLEMENTATION_NAME);
}




cssu::Reference<cssu::XInterface> SAL_CALL PanelFactory::createInstance (
    const uno::Reference<lang::XMultiServiceFactory>& rxFactory)
{
    (void)rxFactory;

    ::rtl::Reference<PanelFactory> pPanelFactory (new PanelFactory());
    cssu::Reference<cssu::XInterface> xService (static_cast<XWeak*>(pPanelFactory.get()), cssu::UNO_QUERY);
    return xService;
}




cssu::Sequence<OUString> SAL_CALL PanelFactory::getSupportedServiceNames (void)
{
    cssu::Sequence<OUString> aServiceNames (1);
    aServiceNames[0] = A2S(SERVICE_NAME);
    return aServiceNames;

}




PanelFactory::PanelFactory (void)
    : PanelFactoryInterfaceBase(m_aMutex)
{
}




PanelFactory::~PanelFactory (void)
{
}




Reference<ui::XUIElement> SAL_CALL PanelFactory::createUIElement (
    const ::rtl::OUString& rsResourceURL,
    const ::cssu::Sequence<css::beans::PropertyValue>& rArguments)
    throw(
        container::NoSuchElementException,
        lang::IllegalArgumentException,
        RuntimeException)
{
    const ::comphelper::NamedValueCollection aArguments (rArguments);
    Reference<frame::XFrame> xFrame (aArguments.getOrDefault("Frame", Reference<frame::XFrame>()));
    Reference<awt::XWindow> xParentWindow (aArguments.getOrDefault("ParentWindow", Reference<awt::XWindow>()));
    Reference<ui::XSidebar> xSidebar (aArguments.getOrDefault("Sidebar", Reference<ui::XSidebar>()));
    const sal_uInt64 nBindingsValue (aArguments.getOrDefault("SfxBindings", sal_uInt64(0)));
    SfxBindings* pBindings = reinterpret_cast<SfxBindings*>(nBindingsValue);
    ::sfx2::sidebar::EnumContext aContext (
        aArguments.getOrDefault("ApplicationName", OUString()),
        aArguments.getOrDefault("ContextName", OUString()));

    ::Window* pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
    if ( ! xParentWindow.is() || pParentWindow==NULL)
        throw RuntimeException(
            A2S("PanelFactory::createUIElement called without ParentWindow"),
            NULL);
    if ( ! xFrame.is())
        throw RuntimeException(
            A2S("PanelFactory::createUIElement called without Frame"),
            NULL);
    if (pBindings == NULL)
        throw RuntimeException(
            A2S("PanelFactory::createUIElement called without SfxBindings"),
            NULL);

    Window* pControl = NULL;
    ui::LayoutSize aLayoutSize (-1,-1,-1);

#define DoesResourceEndWith(s) rsResourceURL.endsWithAsciiL(s,strlen(s))
    if (DoesResourceEndWith("/TextPropertyPanel"))
    {
        pControl = TextPropertyPanel::Create(pParentWindow, xFrame, pBindings, aContext);
    }
    else if (DoesResourceEndWith("/ParaPropertyPanel"))
    {
        pControl = ParaPropertyPanel::Create(pParentWindow, xFrame, pBindings, xSidebar);
    }
    else if (DoesResourceEndWith("/AreaPropertyPanel"))
    {
        pControl = AreaPropertyPanel::Create(pParentWindow, xFrame, pBindings);
    }
    else if (DoesResourceEndWith("/GraphicPropertyPanel"))
    {
        pControl = GraphicPropertyPanel::Create(pParentWindow, xFrame, pBindings);
    }
    else if (DoesResourceEndWith("/LinePropertyPanel"))
    {
        pControl = LinePropertyPanel::Create(pParentWindow, xFrame, pBindings);
    }
    else if (DoesResourceEndWith("/PosSizePropertyPanel"))
    {
        pControl = PosSizePropertyPanel::Create(pParentWindow, xFrame, pBindings, xSidebar);
    }
    else if (DoesResourceEndWith("/InsertPropertyPanel"))
    {
        pControl = new InsertPropertyPanel(pParentWindow, xFrame);
    }
    else if (DoesResourceEndWith("/GalleryPanel"))
    {
        pControl = new GalleryControl(pBindings, pParentWindow);
        aLayoutSize = ui::LayoutSize(300,-1,400);
    }
    else if (DoesResourceEndWith("/StyleListPanel"))
    {
        pControl = new SfxTemplatePanelControl(pBindings, pParentWindow);
        aLayoutSize = ui::LayoutSize(0,-1,-1);
    }
    else if (DoesResourceEndWith("/Debug_ColorPanel"))
    {
        pControl = new ColorPanel(pParentWindow);
        aLayoutSize = ui::LayoutSize(300,-1,400);
    }
    else if (DoesResourceEndWith("/Debug_ContextPanel"))
    {
        pControl = new ContextPanel(pParentWindow);
        aLayoutSize = ui::LayoutSize(45,45,45);
    }
    else if (DoesResourceEndWith("/Debug_NotYetImplementedPanel"))
    {
        pControl = new NotYetImplementedPanel(pParentWindow);
        aLayoutSize = ui::LayoutSize(20,25,25);
    }
    else if (DoesResourceEndWith("/EmptyPanel"))
    {
        pControl = new EmptyPanel(pParentWindow);
        aLayoutSize = ui::LayoutSize(20,-1, 50);
    }
#undef DoesResourceEndWith

    if (pControl != NULL)
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

} } // end of namespace svx::sidebar

// eof
