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
        FrameworkHelper::CreateResourceId(FrameworkHelper::msSidebarViewURL, rsSidebarPaneURL))
{
    if (mxConfigurationController.is())
    {
        if (SvtToolPanelOptions().GetVisibleImpressView())
            AddActiveMainView(FrameworkHelper::msImpressViewURL);
        if (SvtToolPanelOptions().GetVisibleOutlineView())
            AddActiveMainView(FrameworkHelper::msOutlineViewURL);
        if (SvtToolPanelOptions().GetVisibleNotesView())
            AddActiveMainView(FrameworkHelper::msNotesViewURL);
        if (SvtToolPanelOptions().GetVisibleHandoutView())
            AddActiveMainView(FrameworkHelper::msHandoutViewURL);
        if (SvtToolPanelOptions().GetVisibleSlideSorterView())
            AddActiveMainView(FrameworkHelper::msSlideSorterURL);

        mxConfigurationController->addConfigurationChangeListener(
            this,
            FrameworkHelper::msResourceActivationEvent,
            Any());
    }
}

ToolPanelModule::~ToolPanelModule()
{
}

void ToolPanelModule::SaveResourceState()
{
    SvtToolPanelOptions().SetVisibleImpressView(IsResourceActive(FrameworkHelper::msImpressViewURL));
    SvtToolPanelOptions().SetVisibleOutlineView(IsResourceActive(FrameworkHelper::msOutlineViewURL));
    SvtToolPanelOptions().SetVisibleNotesView(IsResourceActive(FrameworkHelper::msNotesViewURL));
    SvtToolPanelOptions().SetVisibleHandoutView(IsResourceActive(FrameworkHelper::msHandoutViewURL));
    SvtToolPanelOptions().SetVisibleSlideSorterView(IsResourceActive(FrameworkHelper::msSlideSorterURL));
}

void SAL_CALL ToolPanelModule::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
{
    if (rEvent.Type != FrameworkHelper::msResourceActivationEvent)
        ResourceManager::notifyConfigurationChange(rEvent);
}

} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
