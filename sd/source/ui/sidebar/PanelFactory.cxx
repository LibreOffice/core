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

#include "PanelFactory.hxx"
#include "framework/Pane.hxx"
#include "ViewShellBase.hxx"
#include "DrawController.hxx"
#include "LayoutMenu.hxx"
#include "CurrentMasterPagesSelector.hxx"
#include "RecentMasterPagesSelector.hxx"
#include "AllMasterPagesSelector.hxx"
#include "CustomAnimationPanel.hxx"
#include "SlideTransitionPanel.hxx"
#include "NavigatorWrapper.hxx"

#include <sfx2/viewfrm.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <vcl/window.hxx>
#include <toolkit/helper/vclunohelper.hxx>

using namespace css;
using namespace cssu;
using namespace ::sd::framework;
using ::rtl::OUString;

namespace sd {
    extern ::Window * createTableDesignPanel (::Window* pParent, ViewShellBase& rBase);
}

namespace sd { namespace sidebar {

namespace {
    /** Note that these names have to be identical to (the tail of)
        the entries in officecfg/registry/data/org/openoffice/Office/Impress.xcu
        for the TaskPanelFactory.
    */
    const static char* gsResourceNameCustomAnimations = "/CustomAnimations";
    const static char* gsResourceNameLayouts = "/Layouts";
    const static char* gsResourceNameAllMasterPages = "/AllMasterPages";
    const static char* gsResourceNameRecentMasterPages = "/RecentMasterPages";
    const static char* gsResourceNameUsedMasterPages = "/UsedMasterPages";
    const static char* gsResourceNameSlideTransitions = "/SlideTransitions";
    const static char* gsResourceNameTableDesign = "/TableDesign";
    const static char* gsResourceNameNavigator = "/NavigatorPanel";
}

Reference<lang::XEventListener> mxControllerDisposeListener;



// ----- Service functions ----------------------------------------------------

Reference<XInterface> SAL_CALL PanelFactory_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<XWeak*>(new PanelFactory(rxContext)));
}




::rtl::OUString PanelFactory_getImplementationName (void) throw(RuntimeException)
{
    return OUString("org.openoffice.comp.Draw.framework.PanelFactory");
}




Sequence<rtl::OUString> SAL_CALL PanelFactory_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const ::rtl::OUString sServiceName("com.sun.star.drawing.framework.PanelFactory");
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




//----- PanelFactory --------------------------------------------------------

PanelFactory::PanelFactory(
        const css::uno::Reference<css::uno::XComponentContext>& /*rxContext*/)
    : PanelFactoryInterfaceBase(m_aMutex)
{
}




PanelFactory::~PanelFactory (void)
{
}




void SAL_CALL PanelFactory::disposing (void)
{
}




// XUIElementFactory

Reference<ui::XUIElement> SAL_CALL PanelFactory::createUIElement (
    const ::rtl::OUString& rsUIElementResourceURL,
    const ::cssu::Sequence<css::beans::PropertyValue>& rArguments)
    throw(
        css::container::NoSuchElementException,
        css::lang::IllegalArgumentException,
        cssu::RuntimeException)
{
    // Process arguments.
    const ::comphelper::NamedValueCollection aArguments (rArguments);
    Reference<frame::XFrame> xFrame (aArguments.getOrDefault("Frame", Reference<frame::XFrame>()));
    Reference<awt::XWindow> xParentWindow (aArguments.getOrDefault("ParentWindow", Reference<awt::XWindow>()));
    Reference<ui::XSidebar> xSidebar (aArguments.getOrDefault("Sidebar", Reference<ui::XSidebar>()));

    // Throw exceptions when the arguments are not as expected.
    ::Window* pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
    if ( ! xParentWindow.is() || pParentWindow==NULL)
        throw RuntimeException(
            OUString("PanelFactory::createUIElement called without ParentWindow"),
            NULL);
    if ( ! xFrame.is())
        throw RuntimeException(
            OUString("PanelFactory::createUIElement called without XFrame"),
            NULL);

    // Tunnel through the controller to obtain a ViewShellBase.
    ViewShellBase* pBase = NULL;
    Reference<lang::XUnoTunnel> xTunnel (xFrame->getController(), UNO_QUERY);
    if (xTunnel.is())
    {
        ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
            xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
        if (pController != NULL)
            pBase = pController->GetViewShellBase();
    }
    if (pBase == NULL)
        throw RuntimeException("can not get ViewShellBase for frame", NULL);

    // Get bindings from given arguments.
    const sal_uInt64 nBindingsValue (aArguments.getOrDefault("SfxBindings", sal_uInt64(0)));
    SfxBindings* pBindings = reinterpret_cast<SfxBindings*>(nBindingsValue);

    // Create a framework view.
    ::Window* pControl = NULL;
    css::ui::LayoutSize aLayoutSize (-1,-1,-1);

#define EndsWith(s,t) s.endsWithAsciiL(t,strlen(t))
    if (EndsWith(rsUIElementResourceURL, gsResourceNameCustomAnimations))
        pControl = new CustomAnimationPanel(pParentWindow, *pBase, xFrame);
    else if (EndsWith(rsUIElementResourceURL, gsResourceNameLayouts))
        pControl = new LayoutMenu(pParentWindow, *pBase, xSidebar);
    else if (EndsWith(rsUIElementResourceURL, gsResourceNameAllMasterPages))
        pControl = AllMasterPagesSelector::Create(pParentWindow, *pBase, xSidebar);
    else if (EndsWith(rsUIElementResourceURL, gsResourceNameRecentMasterPages))
        pControl = RecentMasterPagesSelector::Create(pParentWindow, *pBase, xSidebar);
    else if (EndsWith(rsUIElementResourceURL, gsResourceNameUsedMasterPages))
        pControl = CurrentMasterPagesSelector::Create(pParentWindow, *pBase, xSidebar);
    else if (EndsWith(rsUIElementResourceURL, gsResourceNameSlideTransitions))
        pControl = new SlideTransitionPanel(pParentWindow, *pBase, xFrame);
    else if (EndsWith(rsUIElementResourceURL, gsResourceNameTableDesign))
        pControl = createTableDesignPanel(pParentWindow, *pBase);
    else if (EndsWith(rsUIElementResourceURL, gsResourceNameNavigator))
        pControl = new NavigatorWrapper(pParentWindow, *pBase, pBindings);
#undef EndsWith

    if (pControl == NULL)
        throw lang::IllegalArgumentException();

    // Create a wrapper around the control that implements the
    // necessary UNO interfaces.
    return sfx2::sidebar::SidebarPanelBase::Create(
        rsUIElementResourceURL,
        xFrame,
        pControl,
        aLayoutSize);
}




} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
