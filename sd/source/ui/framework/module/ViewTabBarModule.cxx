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

#include "ViewTabBarModule.hxx"

#include <framework/ConfigurationController.hxx>
#include <framework/ConfigurationChangeEvent.hxx>
#include <framework/FrameworkHelper.hxx>
#include <ViewTabBar.hxx>
#include <DrawController.hxx>
#include <com/sun/star/frame/XController.hpp>

#include <strings.hrc>
#include <sdresid.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;

namespace sd::framework {

//===== ViewTabBarModule ==================================================

ViewTabBarModule::ViewTabBarModule (
    const rtl::Reference<::sd::DrawController>& rxController,
    const rtl::Reference<ResourceId>& rxViewTabBarId)
    : mxViewTabBarId(rxViewTabBarId)
{
    if (!rxController.is())
        return;

    mxConfigurationController = rxController->getConfigurationController();
    if (!mxConfigurationController.is())
        return;

    mxConfigurationController->addConfigurationChangeListener(
        this,
        ConfigurationChangeEventType::ResourceActivationRequest);
    mxConfigurationController->addConfigurationChangeListener(
        this,
        ConfigurationChangeEventType::ResourceDeactivationRequest);

    UpdateViewTabBar(nullptr);
    mxConfigurationController->addConfigurationChangeListener(
        this,
        ConfigurationChangeEventType::ResourceActivation);
}

ViewTabBarModule::~ViewTabBarModule()
{
}

void ViewTabBarModule::disposing(std::unique_lock<std::mutex>&)
{
    if (mxConfigurationController.is())
    {
        mxConfigurationController->removeConfigurationChangeListener(this);
        mxConfigurationController = nullptr;
    }
}

void ViewTabBarModule::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
{
    if (!mxConfigurationController.is())
        return;

    switch (rEvent.Type)
    {
        case ConfigurationChangeEventType::ResourceActivationRequest:
            if (mxViewTabBarId->isBoundTo(rEvent.ResourceId, AnchorBindingMode_DIRECT))
            {
                mxConfigurationController->requestResourceActivation(
                    mxViewTabBarId,
                    ResourceActivationMode::ADD);
            }
            break;

        case ConfigurationChangeEventType::ResourceDeactivationRequest:
            if (mxViewTabBarId->isBoundTo(rEvent.ResourceId, AnchorBindingMode_DIRECT))
            {
                mxConfigurationController->requestResourceDeactivation(mxViewTabBarId);
            }
            break;

        case ConfigurationChangeEventType::ResourceActivation:
            if (rEvent.ResourceId->compareTo(mxViewTabBarId) == 0)
            {
                UpdateViewTabBar(dynamic_cast<sd::ViewTabBar*>(rEvent.ResourceObject.get()));
            }
            break;

        default: break;
    }
}

void SAL_CALL ViewTabBarModule::disposing (
    const lang::EventObject& rEvent)
{
    if (mxConfigurationController.is()
        && rEvent.Source == cppu::getXWeak(mxConfigurationController.get()))
    {
        // Without the configuration controller this class can do nothing.
        mxConfigurationController = nullptr;
        dispose();
    }
}

void ViewTabBarModule::UpdateViewTabBar (const rtl::Reference<sd::ViewTabBar>& rxTabBar)
{
    if (!mxConfigurationController.is())
        return;

    rtl::Reference<ViewTabBar> xBar (rxTabBar);
    if ( ! xBar.is())
        xBar = dynamic_cast<ViewTabBar*>(mxConfigurationController->getResource(mxViewTabBarId).get());

    if (!xBar.is())
        return;

    TabBarButton aEmptyButton;

    rtl::Reference<ResourceId> xAnchor (mxViewTabBarId->getAnchor());

    TabBarButton aImpressViewButton;
    aImpressViewButton.ResourceId = new ::sd::framework::ResourceId(
        FrameworkHelper::msImpressViewURL,
        xAnchor);
    aImpressViewButton.ButtonLabel = SdResId(STR_NORMAL_MODE);
    if ( ! xBar->hasTabBarButton(aImpressViewButton))
        xBar->addTabBarButtonAfter(aImpressViewButton, aEmptyButton);

    TabBarButton aOutlineViewButton;
    aOutlineViewButton.ResourceId = new ::sd::framework::ResourceId(
        FrameworkHelper::msOutlineViewURL,
        xAnchor);
    aOutlineViewButton.ButtonLabel = SdResId(STR_OUTLINE_MODE);
    if ( ! xBar->hasTabBarButton(aOutlineViewButton))
        xBar->addTabBarButtonAfter(aOutlineViewButton, aImpressViewButton);

    TabBarButton aNotesViewButton;
    aNotesViewButton.ResourceId = new ::sd::framework::ResourceId(
        FrameworkHelper::msNotesViewURL,
        xAnchor);
    aNotesViewButton.ButtonLabel = SdResId(STR_NOTES_MODE);
    if ( ! xBar->hasTabBarButton(aNotesViewButton))
        xBar->addTabBarButtonAfter(aNotesViewButton, aOutlineViewButton);
}

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
