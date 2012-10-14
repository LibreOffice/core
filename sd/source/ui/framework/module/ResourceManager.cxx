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


#include "ResourceManager.hxx"

#include "framework/FrameworkHelper.hxx"
#include "framework/ConfigurationController.hxx"
#include <com/sun/star/drawing/framework/XControllerManager.hpp>

#include <comphelper/stl_types.hxx>
#include <set>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;
using ::sd::framework::FrameworkHelper;

namespace {
    static const sal_Int32 ResourceActivationRequestEvent = 0;
    static const sal_Int32 ResourceDeactivationRequestEvent = 1;
}




namespace sd { namespace framework {

class ResourceManager::MainViewContainer
    : public ::std::set<OUString, ::comphelper::UStringLess>
{
public:
    MainViewContainer (void) {}
};




//===== ResourceManager =======================================================

ResourceManager::ResourceManager (
    const Reference<frame::XController>& rxController,
    const Reference<XResourceId>& rxResourceId)
    : ResourceManagerInterfaceBase(MutexOwner::maMutex),
      mxConfigurationController(),
      mpActiveMainViewContainer(new MainViewContainer()),
      mxResourceId(rxResourceId),
      mxMainViewAnchorId(FrameworkHelper::Instance(rxController)->CreateResourceId(
          FrameworkHelper::msCenterPaneURL)),
      msCurrentMainViewURL()
{
    Reference<XControllerManager> xControllerManager (rxController, UNO_QUERY);
    if (xControllerManager.is())
    {
        mxConfigurationController = xControllerManager->getConfigurationController();

        if (mxConfigurationController.is())
        {
            uno::Reference<lang::XComponent> const xComppnent(
                    mxConfigurationController, UNO_QUERY_THROW);
            xComppnent->addEventListener(this);
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceActivationRequestEvent,
                makeAny(ResourceActivationRequestEvent));
            mxConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceDeactivationRequestEvent,
                makeAny(ResourceDeactivationRequestEvent));
        }
    }
}




ResourceManager::~ResourceManager (void)
{
}




void ResourceManager::AddActiveMainView (
    const OUString& rsMainViewURL)
{
    mpActiveMainViewContainer->insert(rsMainViewURL);
}

sal_Bool ResourceManager::IsResourceActive (
    const OUString& rsMainViewURL)
{
    return (mpActiveMainViewContainer->find(rsMainViewURL) != mpActiveMainViewContainer->end());
}

void ResourceManager::SaveResourceState (void)
{
}

void SAL_CALL ResourceManager::disposing (void)
{
    if (mxConfigurationController.is())
    {
        mxConfigurationController->removeConfigurationChangeListener(this);
        mxConfigurationController = NULL;
    }
}

void SAL_CALL ResourceManager::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    OSL_ASSERT(rEvent.ResourceId.is());

    sal_Int32 nEventType = 0;
    rEvent.UserData >>= nEventType;
    switch (nEventType)
    {
        case ResourceActivationRequestEvent:
            if (rEvent.ResourceId->isBoundToURL(
                FrameworkHelper::msCenterPaneURL,
                AnchorBindingMode_DIRECT))
            {
                // A resource directly bound to the center pane has been
                // requested.
                if (rEvent.ResourceId->getResourceTypePrefix().equals(
                    FrameworkHelper::msViewURLPrefix))
                {
                    // The requested resource is a view.  Show or hide the
                    // resource managed by this ResourceManager accordingly.
                    HandleMainViewSwitch(
                        rEvent.ResourceId->getResourceURL(),
                        rEvent.Configuration,
                        true);
                }
            }
            else if (rEvent.ResourceId->compareTo(mxResourceId) == 0)
            {
                // The resource managed by this ResourceManager has been
                // explicitly been requested (maybe by us).  Remember this
                // setting.
                HandleResourceRequest(true, rEvent.Configuration);
            }
            break;

        case ResourceDeactivationRequestEvent:
            if (rEvent.ResourceId->compareTo(mxMainViewAnchorId) == 0)
            {
                HandleMainViewSwitch(
                    OUString(),
                    rEvent.Configuration,
                    false);
            }
            else if (rEvent.ResourceId->compareTo(mxResourceId) == 0)
            {
                // The resource managed by this ResourceManager has been
                // explicitly been requested to be hidden (maybe by us).
                // Remember this setting.
                HandleResourceRequest(false, rEvent.Configuration);
            }
            break;
    }
}




void ResourceManager::UpdateForMainViewShell (void)
{
    if (mxConfigurationController.is())
    {
        ConfigurationController::Lock aLock (mxConfigurationController);

        if (mpActiveMainViewContainer->find(msCurrentMainViewURL)
               != mpActiveMainViewContainer->end())
        {
            // Activate resource.
            mxConfigurationController->requestResourceActivation(
                mxResourceId->getAnchor(),
                ResourceActivationMode_ADD);
            mxConfigurationController->requestResourceActivation(
                mxResourceId,
                ResourceActivationMode_REPLACE);
        }
        else
        {
            mxConfigurationController->requestResourceDeactivation(mxResourceId);
        }
    }
}




void ResourceManager::HandleMainViewSwitch (
    const OUString& rsViewURL,
    const Reference<XConfiguration>& rxConfiguration,
    const bool bIsActivated)
{
    (void)rxConfiguration;
    if (bIsActivated)
        msCurrentMainViewURL = rsViewURL;
    else
        msCurrentMainViewURL = OUString();
    UpdateForMainViewShell();
}




void ResourceManager::HandleResourceRequest(
    bool bActivation,
    const Reference<XConfiguration>& rxConfiguration)
{
    Sequence<Reference<XResourceId> > aCenterViews = rxConfiguration->getResources(
        FrameworkHelper::CreateResourceId(FrameworkHelper::msCenterPaneURL),
        FrameworkHelper::msViewURLPrefix,
        AnchorBindingMode_DIRECT);
    if (aCenterViews.getLength() == 1)
    {
        if (bActivation)
        {
            mpActiveMainViewContainer->insert(aCenterViews[0]->getResourceURL());
        }
        else
        {
            MainViewContainer::iterator iElement (
                mpActiveMainViewContainer->find(aCenterViews[0]->getResourceURL()));
            if (iElement != mpActiveMainViewContainer->end())
                mpActiveMainViewContainer->erase(iElement);
        }
    }
}




void SAL_CALL ResourceManager::disposing (
    const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (mxConfigurationController.is()
        && rEvent.Source == mxConfigurationController)
    {
        SaveResourceState();
        // Without the configuration controller this class can do nothing.
        mxConfigurationController = NULL;
        dispose();
    }
}

} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
