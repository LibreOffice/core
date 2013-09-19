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


#include "ToolPanelModule.hxx"

#include "framework/FrameworkHelper.hxx"
#include <com/sun/star/drawing/framework/XTabBar.hpp>
#include <com/sun/star/drawing/framework/TabBarButton.hpp>

#include "strings.hrc"
#include "sdresid.hxx"
#include "svtools/toolpanelopt.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;


namespace sd { namespace framework {


//===== ToolPanelModule ==================================================

ToolPanelModule::ToolPanelModule (
    const Reference<frame::XController>& rxController,
    const OUString& rsSidebarPaneURL)
    : ResourceManager(rxController,
        FrameworkHelper::CreateResourceId(FrameworkHelper::msSidebarViewURL, rsSidebarPaneURL)),
      mxControllerManager(rxController,UNO_QUERY)
{
    if (mxConfigurationController.is())
    {
        if (SvtToolPanelOptions().GetVisibleImpressView()==sal_True)
            AddActiveMainView(FrameworkHelper::msImpressViewURL);
        if (SvtToolPanelOptions().GetVisibleOutlineView()==sal_True)
            AddActiveMainView(FrameworkHelper::msOutlineViewURL);
        if (SvtToolPanelOptions().GetVisibleNotesView()==sal_True)
            AddActiveMainView(FrameworkHelper::msNotesViewURL);
        if (SvtToolPanelOptions().GetVisibleHandoutView()==sal_True)
            AddActiveMainView(FrameworkHelper::msHandoutViewURL);
        if (SvtToolPanelOptions().GetVisibleSlideSorterView()==sal_True)
            AddActiveMainView(FrameworkHelper::msSlideSorterURL);

        mxConfigurationController->addConfigurationChangeListener(
            this,
            FrameworkHelper::msResourceActivationEvent,
            Any());
    }
}

ToolPanelModule::~ToolPanelModule (void)
{
}

void ToolPanelModule::SaveResourceState (void)
{
    SvtToolPanelOptions().SetVisibleImpressView(IsResourceActive(FrameworkHelper::msImpressViewURL));
    SvtToolPanelOptions().SetVisibleOutlineView(IsResourceActive(FrameworkHelper::msOutlineViewURL));
    SvtToolPanelOptions().SetVisibleNotesView(IsResourceActive(FrameworkHelper::msNotesViewURL));
    SvtToolPanelOptions().SetVisibleHandoutView(IsResourceActive(FrameworkHelper::msHandoutViewURL));
    SvtToolPanelOptions().SetVisibleSlideSorterView(IsResourceActive(FrameworkHelper::msSlideSorterURL));
}

void SAL_CALL ToolPanelModule::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    if (!rEvent.Type.equals(FrameworkHelper::msResourceActivationEvent))
        ResourceManager::notifyConfigurationChange(rEvent);
}

} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
