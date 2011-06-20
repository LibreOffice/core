/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "ViewTabBarModule.hxx"

#include "framework/FrameworkHelper.hxx"
#include "framework/ConfigurationController.hxx"
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XTabBar.hpp>

#include "strings.hrc"
#include "sdresid.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;
using ::sd::framework::FrameworkHelper;

namespace {

static const sal_Int32 ResourceActivationRequestEvent = 0;
static const sal_Int32 ResourceDeactivationRequestEvent = 1;
static const sal_Int32 ResourceActivationEvent = 2;

}

namespace sd { namespace framework {

//===== ViewTabBarModule ==================================================

ViewTabBarModule::ViewTabBarModule (
    const Reference<frame::XController>& rxController,
    const Reference<XResourceId>& rxViewTabBarId)
    : ViewTabBarModuleInterfaceBase(MutexOwner::maMutex),
      mxConfigurationController(),
      mxViewTabBarId(rxViewTabBarId)
{
    Reference<XControllerManager> xControllerManager (rxController, UNO_QUERY);

    if (xControllerManager.is())
    {
        mxConfigurationController = xControllerManager->getConfigurationController();
        if (mxConfigurationController.is())
        {
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceActivationRequestEvent,
                makeAny(ResourceActivationRequestEvent));
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceDeactivationRequestEvent,
                makeAny(ResourceDeactivationRequestEvent));

            UpdateViewTabBar(NULL);
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceActivationEvent,
                makeAny(ResourceActivationEvent));
        }
    }
}




ViewTabBarModule::~ViewTabBarModule (void)
{
}




void SAL_CALL ViewTabBarModule::disposing (void)
{
    if (mxConfigurationController.is())
        mxConfigurationController->removeConfigurationChangeListener(this);

    mxConfigurationController = NULL;
}




void SAL_CALL ViewTabBarModule::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    if (mxConfigurationController.is())
    {
        sal_Int32 nEventType = 0;
        rEvent.UserData >>= nEventType;
        switch (nEventType)
        {
            case ResourceActivationRequestEvent:
                if (mxViewTabBarId->isBoundTo(rEvent.ResourceId, AnchorBindingMode_DIRECT))
                {
                    mxConfigurationController->requestResourceActivation(
                        mxViewTabBarId,
                        ResourceActivationMode_ADD);
                }
                break;

            case ResourceDeactivationRequestEvent:
                if (mxViewTabBarId->isBoundTo(rEvent.ResourceId, AnchorBindingMode_DIRECT))
                {
                    mxConfigurationController->requestResourceDeactivation(mxViewTabBarId);
                }
                break;

            case ResourceActivationEvent:
                if (rEvent.ResourceId->compareTo(mxViewTabBarId) == 0)
                {
                    UpdateViewTabBar(Reference<XTabBar>(rEvent.ResourceObject,UNO_QUERY));
                }
        }
    }
}




void SAL_CALL ViewTabBarModule::disposing (
    const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (mxConfigurationController.is()
        && rEvent.Source == mxConfigurationController)
    {
        // Without the configuration controller this class can do nothing.
        mxConfigurationController = NULL;
        disposing();
    }
}




void ViewTabBarModule::UpdateViewTabBar (const Reference<XTabBar>& rxTabBar)
{
    if (mxConfigurationController.is())
    {
        Reference<XTabBar> xBar (rxTabBar);
        if ( ! xBar.is())
            xBar = Reference<XTabBar>(
                mxConfigurationController->getResource(mxViewTabBarId), UNO_QUERY);

        if (xBar.is())
        {
            TabBarButton aEmptyButton;

            Reference<XResourceId> xAnchor (mxViewTabBarId->getAnchor());

            TabBarButton aImpressViewButton;
            aImpressViewButton.ResourceId = FrameworkHelper::CreateResourceId(
                FrameworkHelper::msImpressViewURL,
                xAnchor);
            aImpressViewButton.ButtonLabel = String(SdResId(STR_DRAW_MODE));
            if ( ! xBar->hasTabBarButton(aImpressViewButton))
                xBar->addTabBarButtonAfter(aImpressViewButton, aEmptyButton);

            TabBarButton aOutlineViewButton;
            aOutlineViewButton.ResourceId = FrameworkHelper::CreateResourceId(
                FrameworkHelper::msOutlineViewURL,
                xAnchor);
            aOutlineViewButton.ButtonLabel = String(SdResId(STR_OUTLINE_MODE));
            if ( ! xBar->hasTabBarButton(aOutlineViewButton))
                xBar->addTabBarButtonAfter(aOutlineViewButton, aImpressViewButton);

            TabBarButton aNotesViewButton;
            aNotesViewButton.ResourceId = FrameworkHelper::CreateResourceId(
                FrameworkHelper::msNotesViewURL,
                xAnchor);
            aNotesViewButton.ButtonLabel = String(SdResId(STR_NOTES_MODE));
            if ( ! xBar->hasTabBarButton(aNotesViewButton))
                xBar->addTabBarButtonAfter(aNotesViewButton, aOutlineViewButton);

            TabBarButton aHandoutViewButton;
            aHandoutViewButton.ResourceId = FrameworkHelper::CreateResourceId(
                FrameworkHelper::msHandoutViewURL,
                xAnchor);
            aHandoutViewButton.ButtonLabel = String(SdResId(STR_HANDOUT_MODE));
            if ( ! xBar->hasTabBarButton(aHandoutViewButton))
                xBar->addTabBarButtonAfter(aHandoutViewButton, aNotesViewButton);
        }
    }
}




} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
