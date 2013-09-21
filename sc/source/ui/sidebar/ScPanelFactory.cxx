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

#include <AlignmentPropertyPanel.hxx>
#include <CellAppearancePropertyPanel.hxx>
#include <NumberFormatPropertyPanel.hxx>
#include <navipi.hxx>
#include <dwfunctr.hxx>
#include "sc.hrc"

#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <rtl/ref.hxx>
#include <comphelper/namedvaluecollection.hxx>

#include <boost/bind.hpp>


using namespace css;
using namespace cssu;
using ::rtl::OUString;


namespace sc { namespace sidebar {

#define A2S(s) ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))
#define IMPLEMENTATION_NAME "org.apache.openoffice.comp.sc.sidebar.ScPanelFactory"
#define SERVICE_NAME "com.sun.star.ui.UIElementFactory"


::rtl::OUString SAL_CALL ScPanelFactory::getImplementationName (void)
{
    return A2S(IMPLEMENTATION_NAME);
}


cssu::Reference<cssu::XInterface> SAL_CALL ScPanelFactory::createInstance(
    const uno::Reference<lang::XMultiServiceFactory>& )
{
    ::rtl::Reference<ScPanelFactory> pPanelFactory (new ScPanelFactory());
    cssu::Reference<cssu::XInterface> xService (static_cast<XWeak*>(pPanelFactory.get()), cssu::UNO_QUERY);
    return xService;
}


cssu::Sequence<OUString> SAL_CALL ScPanelFactory::getSupportedServiceNames (void)
{
    cssu::Sequence<OUString> aServiceNames (1);
    aServiceNames[0] = A2S(SERVICE_NAME);
    return aServiceNames;

}


ScPanelFactory::ScPanelFactory (void)
    : PanelFactoryInterfaceBase(m_aMutex)
{
}


ScPanelFactory::~ScPanelFactory (void)
{
}


Reference<ui::XUIElement> SAL_CALL ScPanelFactory::createUIElement (
    const ::rtl::OUString& rsResourceURL,
    const ::cssu::Sequence<css::beans::PropertyValue>& rArguments)
    throw(
        container::NoSuchElementException,
        lang::IllegalArgumentException,
        RuntimeException)
{
    Reference<ui::XUIElement> xElement;

    const ::comphelper::NamedValueCollection aArguments (rArguments);
    Reference<frame::XFrame> xFrame (aArguments.getOrDefault("Frame", Reference<frame::XFrame>()));
    Reference<awt::XWindow> xParentWindow (aArguments.getOrDefault("ParentWindow", Reference<awt::XWindow>()));
    const sal_uInt64 nBindingsValue (aArguments.getOrDefault("SfxBindings", sal_uInt64(0)));
    SfxBindings* pBindings = reinterpret_cast<SfxBindings*>(nBindingsValue);

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

#define DoesResourceEndWith(s) rsResourceURL.endsWithAsciiL(s,strlen(s))
    if (DoesResourceEndWith("/AlignmentPropertyPanel"))
    {
        AlignmentPropertyPanel* pPanel = AlignmentPropertyPanel::Create( pParentWindow, xFrame, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            pPanel,
            ui::LayoutSize(-1,-1,-1));
    }
    else if (DoesResourceEndWith("/CellAppearancePropertyPanel"))
    {
        CellAppearancePropertyPanel* pPanel = CellAppearancePropertyPanel::Create( pParentWindow, xFrame, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            pPanel,
            ui::LayoutSize(-1,-1,-1));
    }
    else if (DoesResourceEndWith("/NumberFormatPropertyPanel"))
    {
        NumberFormatPropertyPanel* pPanel = NumberFormatPropertyPanel::Create( pParentWindow, xFrame, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            pPanel,
            ui::LayoutSize(-1,-1,-1));
    }
    else if (DoesResourceEndWith("/NavigatorPanel"))
    {
        Window* pPanel = new ScNavigatorDlg(pBindings, NULL, pParentWindow, false);
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            pPanel,
            ui::LayoutSize(0,-1,-1));
    }
    else if (DoesResourceEndWith("/FunctionsPanel"))
    {
        Window* pPanel = new ScFunctionDockWin(pBindings, NULL, pParentWindow, ScResId(FID_FUNCTION_BOX));
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            pPanel,
            ui::LayoutSize(0,-1,-1));
    }
#undef DoesResourceEndWith

    return xElement;
}

} } // end of namespace sc::sidebar

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
