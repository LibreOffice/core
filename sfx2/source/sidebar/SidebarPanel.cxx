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
#include "precompiled_sfx2.hxx"

#include "SidebarPanel.hxx"

#include "Panel.hxx"
#include "sfx2/sidebar/Theme.hxx"

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svl/smplhint.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/XWindowPeer.hpp>


using namespace css;
using namespace cssu;

namespace sfx2 { namespace sidebar {

Reference<css::ui::XSidebarPanel> SidebarPanel::Create (Panel* pPanel)
{
    return Reference<css::ui::XSidebarPanel>(new SidebarPanel(pPanel));
}




SidebarPanel::SidebarPanel(Panel* pPanel)
    : SidebarPanelInterfaceBase(m_aMutex),
      mpPanel(pPanel),
      mxCanvas()
{
    if (mpPanel != NULL)
        mpPanel->AddEventListener(LINK(this, SidebarPanel, HandleWindowEvent));
    else
    {
        mpPanel = NULL;
        dispose();
    }
}




SidebarPanel::~SidebarPanel (void)
{
}




void SAL_CALL SidebarPanel::disposing (const css::lang::EventObject& rEventObject)
    throw(cssu::RuntimeException)
{
    (void)rEventObject;
}




void SAL_CALL SidebarPanel::disposing (void)
{
    if (mpPanel != NULL)
    {
        mpPanel->RemoveEventListener(LINK(this, SidebarPanel, HandleWindowEvent));
        mpPanel = NULL;
    }
}




cssu::Reference<css::rendering::XCanvas> SAL_CALL SidebarPanel::getCanvas (void)
    throw (cssu::RuntimeException)
{
    if ( ! mxCanvas.is())
    {
        Sequence<Any> aArg (5);

        // common: first any is VCL pointer to window (for VCL canvas)
        aArg[0] = makeAny(reinterpret_cast<sal_Int64>(mpPanel));
        aArg[1] = Any();
        aArg[2] = makeAny(::com::sun::star::awt::Rectangle());
        aArg[3] = makeAny(sal_False);
        aArg[4] = makeAny(mpPanel->GetComponentInterface());

        const ::comphelper::ComponentContext aComponentContext (::comphelper::getProcessServiceFactory());
        mxCanvas = Reference<rendering::XCanvas>(
            aComponentContext.createComponentWithArguments(
                "com.sun.star.rendering.Canvas.VCL",
                aArg),
            UNO_QUERY);
    }

    return mxCanvas;

}




awt::Point SAL_CALL SidebarPanel::getPositionOnScreen (void)
    throw (cssu::RuntimeException)
{
    awt::Point aAwtPoint;

    if (mpPanel != NULL)
    {
        ::vos::OGuard aGuard (Application::GetSolarMutex());

        //        mpPanel->GetPosPixel()
        const Point aLocationOnScreen (mpPanel->OutputToAbsoluteScreenPixel(Point(0,0)));

        aAwtPoint.X = aLocationOnScreen.X();
        aAwtPoint.Y = aLocationOnScreen.Y();
    }

    return aAwtPoint;
}




Reference<beans::XPropertySet> SAL_CALL SidebarPanel::getThemeProperties (void)
    throw (RuntimeException)
{
    return Theme::GetPropertySet();
}




IMPL_LINK(SidebarPanel, HandleWindowEvent, VclWindowEvent*, pEvent)
{
    if (pEvent != NULL)
    {
        switch (pEvent->GetId())
        {
            case SFX_HINT_DYING:
                dispose();
                break;

            default:
                break;
        }
    }

    return sal_True;
}



} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
