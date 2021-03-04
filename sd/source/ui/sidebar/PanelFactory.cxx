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
#include <framework/Pane.hxx>
#include <ViewShellBase.hxx>
#include <DrawController.hxx>
#include "LayoutMenu.hxx"
#include "CurrentMasterPagesSelector.hxx"
#include "RecentMasterPagesSelector.hxx"
#include "AllMasterPagesSelector.hxx"
#include <CustomAnimationPane.hxx>
#include "NavigatorWrapper.hxx"
#include <SlideTransitionPane.hxx>
#include <TableDesignPane.hxx>
#include "SlideBackground.hxx"

#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/servicehelper.hxx>
#include <vcl/window.hxx>
#include <toolkit/helper/vclunohelper.hxx>

using namespace css;
using namespace css::uno;
using namespace ::sd::framework;

namespace sd::sidebar {

//----- PanelFactory --------------------------------------------------------

PanelFactory::PanelFactory()
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
    const OUString& rsUIElementResourceURL,
    const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
{
    // Process arguments.
    const ::comphelper::NamedValueCollection aArguments (rArguments);
    Reference<frame::XFrame> xFrame (aArguments.getOrDefault("Frame", Reference<frame::XFrame>()));
    Reference<awt::XWindow> xParentWindow (aArguments.getOrDefault("ParentWindow", Reference<awt::XWindow>()));
    Reference<ui::XSidebar> xSidebar (aArguments.getOrDefault("Sidebar", Reference<ui::XSidebar>()));

    // Throw exceptions when the arguments are not as expected.
    VclPtr<vcl::Window> pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
    if ( ! xParentWindow.is() || !pParentWindow)
        throw RuntimeException(
            "PanelFactory::createUIElement called without ParentWindow");
    if ( ! xFrame.is())
        throw RuntimeException(
            "PanelFactory::createUIElement called without XFrame");

    // Tunnel through the controller to obtain a ViewShellBase.
    ViewShellBase* pBase = nullptr;
    auto pController = comphelper::getUnoTunnelImplementation<sd::DrawController>(xFrame->getController());
    if (pController != nullptr)
        pBase = pController->GetViewShellBase();
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
    if (rsUIElementResourceURL.endsWith("/CustomAnimations"))
        pControl = VclPtr<CustomAnimationPane>::Create(pParentWindow, *pBase, xFrame);
    else if (rsUIElementResourceURL.endsWith("/Layouts"))
        pControl = VclPtr<LayoutMenu>::Create(pParentWindow, *pBase, xSidebar);
    else if (rsUIElementResourceURL.endsWith("/AllMasterPages"))
        pControl = AllMasterPagesSelector::Create(pParentWindow, *pBase, xSidebar);
    else if (rsUIElementResourceURL.endsWith("/RecentMasterPages"))
        pControl = RecentMasterPagesSelector::Create(pParentWindow, *pBase, xSidebar);
    else if (rsUIElementResourceURL.endsWith("/UsedMasterPages"))
        pControl = CurrentMasterPagesSelector::Create(pParentWindow, *pBase, xSidebar);
    else if (rsUIElementResourceURL.endsWith("/SlideTransitions"))
        pControl = VclPtr<SlideTransitionPane>::Create(pParentWindow, *pBase, xFrame);
    else if (rsUIElementResourceURL.endsWith("/TableDesign"))
        pControl = VclPtr<TableDesignPane>::Create(pParentWindow, *pBase);
    else if (rsUIElementResourceURL.endsWith("/NavigatorPanel"))
        pControl = VclPtr<NavigatorWrapper>::Create(pParentWindow, *pBase, pBindings);
    else if (rsUIElementResourceURL.endsWith("/SlideBackgroundPanel"))
        pControl = VclPtr<SlideBackground>::Create(pParentWindow, *pBase, xFrame, pBindings);

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

} // end of namespace sd::sidebar


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_openoffice_comp_Draw_framework_PanelFactory_get_implementation(css::uno::XComponentContext* /*context*/,
                                                                   css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::sidebar::PanelFactory);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
