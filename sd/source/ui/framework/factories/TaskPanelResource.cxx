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
#include "precompiled_sd.hxx"

#include "framework/TaskPanelResource.hxx"

#include <vcl/window.hxx>
#include <toolkit/helper/vclunohelper.hxx>


using namespace css;
using namespace cssu;
using namespace cssdf;


namespace sd { namespace framework {

namespace {
    ::Window* GetWindowForResource (
        ViewShellBase& rViewShellBase,
        const cssu::Reference<cssdf::XResourceId>& rxResourceId)
    {
        ::Window* pWindow = NULL;
        if (rxResourceId.is() && rxResourceId->getAnchor().is())
        {
            ::boost::shared_ptr<FrameworkHelper> pFrameworkHelper (FrameworkHelper::Instance(rViewShellBase));
            Reference<awt::XWindow> xWindow (
                pFrameworkHelper->GetPaneWindow(rxResourceId->getAnchor()->getAnchor()));
            pWindow = VCLUnoHelper::GetWindow(xWindow);
        }
        return pWindow;
    }
}




TaskPanelResource::TaskPanelResource (
    sidebar::SidebarViewShell& rSidebarViewShell,
    sidebar::PanelId ePanelId,
    const Reference<XResourceId>& rxResourceId)
    : TaskPanelResourceInterfaceBase(m_aMutex),
      mxResourceId(rxResourceId),
      mpControl(rSidebarViewShell.CreatePanel(
              GetWindowForResource(rSidebarViewShell.GetViewShellBase(), rxResourceId),
              ePanelId))
{
    if (mpControl.get() != NULL)
    {
        mpControl->Show();
        mpControl->GetParent()->Show();
        mpControl->AddEventListener(LINK(this,TaskPanelResource,WindowEventHandler));
    }
}




TaskPanelResource::~TaskPanelResource (void)
{
    mpControl.reset();
}




void SAL_CALL TaskPanelResource::disposing ()
{
    mpControl.reset();
}




Reference<XResourceId> SAL_CALL TaskPanelResource::getResourceId ()
    throw (css::uno::RuntimeException)
{
    return mxResourceId;
}




sal_Bool SAL_CALL TaskPanelResource::isAnchorOnly (void)
    throw (RuntimeException)
{
    return false;
}




::Window* TaskPanelResource::GetControl (void) const
{
    return mpControl.get();
}




IMPL_LINK(TaskPanelResource,WindowEventHandler,VclWindowEvent*,pEvent)
{
    if (pEvent!=NULL && pEvent->GetId()==SFX_HINT_DYING)
    {
        // Somebody else deleted the window.  Release our reference so
        // that we do not delete it again.
        mpControl.release();
        return sal_True;
    }
    else
        return sal_False;
}

} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
