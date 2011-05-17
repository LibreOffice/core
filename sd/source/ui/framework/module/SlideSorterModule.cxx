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

#include "SlideSorterModule.hxx"

#include "framework/FrameworkHelper.hxx"
#include <com/sun/star/drawing/framework/XTabBar.hpp>
#include <com/sun/star/drawing/framework/TabBarButton.hpp>

#include "strings.hrc"
#include "sdresid.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;
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

        AddActiveMainView(FrameworkHelper::msImpressViewURL);
        AddActiveMainView(FrameworkHelper::msOutlineViewURL);
        AddActiveMainView(FrameworkHelper::msNotesViewURL);

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
        aButtonA.ButtonLabel = String(SdResId(STR_SLIDE_MODE));

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
