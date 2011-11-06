/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
