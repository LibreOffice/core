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

#include "AlignmentPropertyPanel.hxx"
#include "CellAppearancePropertyPanel.hxx"
#include "NumberFormatPropertyPanel.hxx"
#include <navipi.hxx>
#include <dwfunctr.hxx>
#include "sc.hrc"

#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <comphelper/namedvaluecollection.hxx>

#include <boost/bind.hpp>

using namespace css;
using namespace css::uno;
using ::rtl::OUString;

namespace sc { namespace sidebar {

ScPanelFactory::ScPanelFactory (void)
    : PanelFactoryInterfaceBase(m_aMutex)
{
}

ScPanelFactory::~ScPanelFactory (void)
{
}

Reference<ui::XUIElement> SAL_CALL ScPanelFactory::createUIElement (
    const ::rtl::OUString& rsResourceURL,
    const ::css::uno::Sequence<css::beans::PropertyValue>& rArguments)
    throw(
        container::NoSuchElementException,
        lang::IllegalArgumentException,
        RuntimeException, std::exception)
{
    Reference<ui::XUIElement> xElement;

    try
    {
        const ::comphelper::NamedValueCollection aArguments (rArguments);
        Reference<frame::XFrame> xFrame (aArguments.getOrDefault("Frame", Reference<frame::XFrame>()));
        Reference<awt::XWindow> xParentWindow (aArguments.getOrDefault("ParentWindow", Reference<awt::XWindow>()));
        const sal_uInt64 nBindingsValue (aArguments.getOrDefault("SfxBindings", sal_uInt64(0)));
        SfxBindings* pBindings = reinterpret_cast<SfxBindings*>(nBindingsValue);

        vcl::Window* pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
        if ( ! xParentWindow.is() || pParentWindow==NULL)
            throw RuntimeException(
                "PanelFactory::createUIElement called without ParentWindow",
                NULL);
        if ( ! xFrame.is())
            throw RuntimeException(
                "PanelFactory::createUIElement called without Frame",
                NULL);
        if (pBindings == NULL)
            throw RuntimeException(
                "PanelFactory::createUIElement called without SfxBindings",
                NULL);

        if (rsResourceURL.endsWith("/AlignmentPropertyPanel"))
        {
            AlignmentPropertyPanel* pPanel = AlignmentPropertyPanel::Create( pParentWindow, xFrame, pBindings );
            xElement = sfx2::sidebar::SidebarPanelBase::Create(
                rsResourceURL,
                xFrame,
                pPanel,
                ui::LayoutSize(-1,-1,-1));
        }
        else if (rsResourceURL.endsWith("/CellAppearancePropertyPanel"))
        {
            CellAppearancePropertyPanel* pPanel = CellAppearancePropertyPanel::Create( pParentWindow, xFrame, pBindings );
            xElement = sfx2::sidebar::SidebarPanelBase::Create(
                rsResourceURL,
                xFrame,
                pPanel,
                ui::LayoutSize(-1,-1,-1));
        }
        else if (rsResourceURL.endsWith("/NumberFormatPropertyPanel"))
        {
            NumberFormatPropertyPanel* pPanel = NumberFormatPropertyPanel::Create( pParentWindow, xFrame, pBindings );
            xElement = sfx2::sidebar::SidebarPanelBase::Create(
                rsResourceURL,
                xFrame,
                pPanel,
                ui::LayoutSize(-1,-1,-1));
        }
        else if (rsResourceURL.endsWith("/NavigatorPanel"))
        {
            vcl::Window* pPanel = new ScNavigatorDlg(pBindings, NULL, pParentWindow, false);
            xElement = sfx2::sidebar::SidebarPanelBase::Create(
                rsResourceURL,
                xFrame,
                pPanel,
                ui::LayoutSize(0,-1,-1));
        }
        else if (rsResourceURL.endsWith("/FunctionsPanel"))
        {
            vcl::Window* pPanel = new ScFunctionDockWin(pBindings, NULL, pParentWindow, ScResId(FID_FUNCTION_BOX));
            xElement = sfx2::sidebar::SidebarPanelBase::Create(
                rsResourceURL,
                xFrame,
                pPanel,
                ui::LayoutSize(0,-1,-1));
        }
    }
    catch (const uno::RuntimeException &)
    {
        throw;
    }
    catch (const uno::Exception& e)
    {
        throw lang::WrappedTargetRuntimeException(
            OUString("ScPanelFactory::createUIElement exception"),
            0, uno::makeAny(e));
    }

    return xElement;
}

} } // end of namespace sc::sidebar


extern "C" SAL_DLLPUBLIC_EXPORT ::com::sun::star::uno::XInterface* SAL_CALL
org_apache_openoffice_comp_sc_sidebar_ScPanelFactory_get_implementation(::com::sun::star::uno::XComponentContext*,
                                                                        ::com::sun::star::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sc::sidebar::ScPanelFactory());
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
