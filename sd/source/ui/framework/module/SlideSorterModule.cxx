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


#include "SlideSorterModule.hxx"

#include "framework/FrameworkHelper.hxx"
#include <com/sun/star/drawing/framework/XTabBar.hpp>
#include <com/sun/star/drawing/framework/TabBarButton.hpp>

#include "strings.hrc"
#include "sdresid.hxx"
#include "svtools/slidesorterbaropt.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;


namespace sd { namespace framework {


//===== SlideSorterModule ==================================================

SlideSorterModule::SlideSorterModule (
    const Reference<frame::XController>& rxController,
    const OUString& rsLeftPaneURL)
    : ResourceManager(rxController,
        FrameworkHelper::CreateResourceId(FrameworkHelper::msSlideSorterURL, rsLeftPaneURL)),
      mxViewTabBarId(FrameworkHelper::CreateResourceId(
          FrameworkHelper::msViewTabBarURL,
          FrameworkHelper::msCenterPaneURL)),
      mxControllerManager(rxController,UNO_QUERY)
{
    if (mxConfigurationController.is())
    {
        UpdateViewTabBar(NULL);

        if (SvtSlideSorterBarOptions().GetVisibleImpressView()==sal_True)
            AddActiveMainView(FrameworkHelper::msImpressViewURL);
        if (SvtSlideSorterBarOptions().GetVisibleOutlineView()==sal_True)
            AddActiveMainView(FrameworkHelper::msOutlineViewURL);
        if (SvtSlideSorterBarOptions().GetVisibleNotesView()==sal_True)
            AddActiveMainView(FrameworkHelper::msNotesViewURL);
        if (SvtSlideSorterBarOptions().GetVisibleHandoutView()==sal_True)
            AddActiveMainView(FrameworkHelper::msHandoutViewURL);
        if (SvtSlideSorterBarOptions().GetVisibleSlideSorterView()==sal_True)
            AddActiveMainView(FrameworkHelper::msSlideSorterURL);
        if (SvtSlideSorterBarOptions().GetVisibleDrawView()==sal_True)
            AddActiveMainView(FrameworkHelper::msDrawViewURL);

        mxConfigurationController->addConfigurationChangeListener(
            this,
            FrameworkHelper::msResourceActivationEvent,
            Any());
    }
}




SlideSorterModule::~SlideSorterModule (void)
{
}

void SlideSorterModule::SaveResourceState (void)
{
    SvtSlideSorterBarOptions().SetVisibleImpressView(IsResourceActive(FrameworkHelper::msImpressViewURL));
    SvtSlideSorterBarOptions().SetVisibleOutlineView(IsResourceActive(FrameworkHelper::msOutlineViewURL));
    SvtSlideSorterBarOptions().SetVisibleNotesView(IsResourceActive(FrameworkHelper::msNotesViewURL));
    SvtSlideSorterBarOptions().SetVisibleHandoutView(IsResourceActive(FrameworkHelper::msHandoutViewURL));
    SvtSlideSorterBarOptions().SetVisibleSlideSorterView(IsResourceActive(FrameworkHelper::msSlideSorterURL));
    SvtSlideSorterBarOptions().SetVisibleDrawView(IsResourceActive(FrameworkHelper::msDrawViewURL));
}


void SAL_CALL SlideSorterModule::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Type.equals(FrameworkHelper::msResourceActivationEvent))
    {
        if (rEvent.ResourceId->compareTo(mxViewTabBarId) == 0)
        {
            // Update the view tab bar because the view tab bar has just
            // become active.
            UpdateViewTabBar(Reference<XTabBar>(rEvent.ResourceObject,UNO_QUERY));
        }
        else if (rEvent.ResourceId->getResourceTypePrefix().equals(
            FrameworkHelper::msViewURLPrefix)
            && rEvent.ResourceId->isBoundTo(
                FrameworkHelper::CreateResourceId(FrameworkHelper::msCenterPaneURL),
                AnchorBindingMode_DIRECT))
        {
            // Update the view tab bar because the view in the center pane
            // has changed.
            UpdateViewTabBar(NULL);
        }
    }
    else
    {
        ResourceManager::notifyConfigurationChange(rEvent);
    }
}

void SlideSorterModule::UpdateViewTabBar (const Reference<XTabBar>& rxTabBar)
{
    if ( ! mxControllerManager.is())
        return;

    Reference<XTabBar> xBar (rxTabBar);
    if ( ! xBar.is())
    {
        Reference<XConfigurationController> xCC (
            mxControllerManager->getConfigurationController());
        if (xCC.is())
            xBar = Reference<XTabBar>(xCC->getResource(mxViewTabBarId), UNO_QUERY);
    }

    if (xBar.is())
    {
        TabBarButton aButtonA;
        aButtonA.ResourceId = FrameworkHelper::CreateResourceId(
            FrameworkHelper::msSlideSorterURL,
            FrameworkHelper::msCenterPaneURL);
        aButtonA.ButtonLabel = SD_RESSTR(STR_SLIDE_MODE);

        TabBarButton aButtonB;
        aButtonB.ResourceId = FrameworkHelper::CreateResourceId(
            FrameworkHelper::msHandoutViewURL,
            FrameworkHelper::msCenterPaneURL);

        if ( ! xBar->hasTabBarButton(aButtonA))
            xBar->addTabBarButtonAfter(aButtonA, aButtonB);
    }
}



} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
