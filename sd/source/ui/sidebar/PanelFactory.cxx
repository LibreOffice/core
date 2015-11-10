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
#include "facreg.hxx"
#include "framework/Pane.hxx"
#include "ViewShellBase.hxx"
#include "DrawController.hxx"
#include "LayoutMenu.hxx"
#include "CurrentMasterPagesSelector.hxx"
#include "RecentMasterPagesSelector.hxx"
#include "AllMasterPagesSelector.hxx"
#include "CustomAnimationPanel.hxx"
#include "NavigatorWrapper.hxx"
#include "SlideTransitionPanel.hxx"
#include "TableDesignPanel.hxx"

#include <sfx2/viewfrm.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <vcl/window.hxx>
#include <toolkit/helper/vclunohelper.hxx>

using namespace css;
using namespace css::uno;
using namespace ::sd::framework;
using ::rtl::OUString;

namespace sd { namespace sidebar {

Reference<lang::XEventListener> mxControllerDisposeListener;

//----- PanelFactory --------------------------------------------------------

PanelFactory::PanelFactory(
        const css::uno::Reference<css::uno::XComponentContext>& /*rxContext*/)
    : PanelFactoryInterfaceBase(m_aMutex)
{
}

PanelFactory::~PanelFactory()
{
}

void SAL_CALL PanelFactory::disposing()
{
}

// XUIElementFactory

Reference<ui::XUIElement> SAL_CALL PanelFactory::createUIElement (
    const ::rtl::OUString& rsUIElementResourceURL,
    const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
    throw(
        css::container::NoSuchElementException,
        css::lang::IllegalArgumentException,
        css::uno::RuntimeException, std::exception)
{
    // Process arguments.
    const ::comphelper::NamedValueCollection aArguments (rArguments);
    Reference<frame::XFrame> xFrame (aArguments.getOrDefault("Frame", Reference<frame::XFrame>()));
    Reference<awt::XWindow> xParentWindow (aArguments.getOrDefault("ParentWindow", Reference<awt::XWindow>()));
    Reference<ui::XSidebar> xSidebar (aArguments.getOrDefault("Sidebar", Reference<ui::XSidebar>()));

    // Throw exceptions when the arguments are not as expected.
    vcl::Window* pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
    if ( ! xParentWindow.is() || pParentWindow==nullptr)
        throw RuntimeException(
            "PanelFactory::createUIElement called without ParentWindow");
    if ( ! xFrame.is())
        throw RuntimeException(
            "PanelFactory::createUIElement called without XFrame");

    // Tunnel through the controller to obtain a ViewShellBase.
    ViewShellBase* pBase = nullptr;
    Reference<lang::XUnoTunnel> xTunnel (xFrame->getController(), UNO_QUERY);
    if (xTunnel.is())
    {
        ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
            xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
        if (pController != nullptr)
            pBase = pController->GetViewShellBase();
    }
    if (pBase == nullptr)
        throw RuntimeException("can not get ViewShellBase for frame");

    // Get bindings from given arguments.
    const sal_uInt64 nBindingsValue (aArguments.getOrDefault("SfxBindings", sal_uInt64(0)));
    SfxBindings* pBindings = reinterpret_cast<SfxBindings*>(nBindingsValue);

    // Create a framework view.
    VclPtr<vcl::Window> pControl;
    css::ui::LayoutSize aLayoutSize (-1,-1,-1);

    /** Note that these names have to be identical to (the tail of)
        the entries in officecfg/registry/data/org/openoffice/Office/Impress.xcu
        for the TaskPanelFactory.
    */
#define EndsWith(s,t) s.endsWithAsciiL(t,strlen(t))
    if (EndsWith(rsUIElementResourceURL, "/CustomAnimations"))
        pControl = VclPtr<CustomAnimationPanel>::Create(pParentWindow, *pBase, xFrame);
    else if (EndsWith(rsUIElementResourceURL, "/Layouts"))
        pControl = VclPtr<LayoutMenu>::Create(pParentWindow, *pBase, xSidebar);
    else if (EndsWith(rsUIElementResourceURL, "/AllMasterPages"))
        pControl = AllMasterPagesSelector::Create(pParentWindow, *pBase, xSidebar);
    else if (EndsWith(rsUIElementResourceURL, "/RecentMasterPages"))
        pControl = RecentMasterPagesSelector::Create(pParentWindow, *pBase, xSidebar);
    else if (EndsWith(rsUIElementResourceURL, "/UsedMasterPages"))
        pControl = CurrentMasterPagesSelector::Create(pParentWindow, *pBase, xSidebar);
    else if (EndsWith(rsUIElementResourceURL, "/SlideTransitions"))
        pControl = VclPtr<SlideTransitionPanel>::Create(pParentWindow, *pBase, xFrame);
    else if (EndsWith(rsUIElementResourceURL, "/TableDesign"))
        pControl = VclPtr<TableDesignPanel>::Create(pParentWindow, *pBase);
    else if (EndsWith(rsUIElementResourceURL, "/NavigatorPanel"))
        pControl = VclPtr<NavigatorWrapper>::Create(pParentWindow, *pBase, pBindings);
#undef EndsWith

    if (!pControl)
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


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
org_openoffice_comp_Draw_framework_PanelFactory_get_implementation(css::uno::XComponentContext* context,
                                                                   css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::sidebar::PanelFactory(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
