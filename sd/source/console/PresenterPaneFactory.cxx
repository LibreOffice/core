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

#include "PresenterPaneFactory.hxx"
#include "PresenterController.hxx"
#include "PresenterPane.hxx"
#include "PresenterPaneBorderPainter.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterSpritePane.hxx"
#include <DrawController.hxx>
#include <framework/ConfigurationController.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;

namespace sdext::presenter {

//===== PresenterPaneFactory ==================================================

rtl::Reference<sd::framework::ResourceFactory> PresenterPaneFactory::Create (
    const Reference<uno::XComponentContext>& rxContext,
    const rtl::Reference<::sd::DrawController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
{
    rtl::Reference<PresenterPaneFactory> pFactory (
        new PresenterPaneFactory(rxContext,rpPresenterController));
    pFactory->Register(rxController);
    return pFactory;
}

PresenterPaneFactory::PresenterPaneFactory (
    const Reference<uno::XComponentContext>& rxContext,
    ::rtl::Reference<PresenterController> xPresenterController)
    : mxComponentContextWeak(rxContext),
      mpPresenterController(std::move(xPresenterController))
{
}

void PresenterPaneFactory::Register (const rtl::Reference<::sd::DrawController>& rxController)
{
    rtl::Reference<::sd::framework::ConfigurationController> xCC;
    try
    {
        // Get the configuration controller.
        xCC = rxController->getConfigurationController();
        mxConfigurationControllerWeak = xCC.get();
        if ( ! xCC.is())
        {
            throw RuntimeException();
        }
        xCC->addResourceFactory(
            u"private:resource/pane/Presenter/*"_ustr,
             this);
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
        if (xCC.is())
            xCC->removeResourceFactoryForReference(this);
        mxConfigurationControllerWeak.clear();

        throw;
    }
}

PresenterPaneFactory::~PresenterPaneFactory()
{
}

void PresenterPaneFactory::disposing(std::unique_lock<std::mutex>&)
{
    rtl::Reference<::sd::framework::ConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is())
        xCC->removeResourceFactoryForReference(this);
    mxConfigurationControllerWeak.clear();

    // Dispose the panes in the cache.
    if (mpResourceCache != nullptr)
    {
        for (const auto& rxPane : *mpResourceCache)
        {
            if (rxPane.second.is())
                rxPane.second->dispose();
        }
        mpResourceCache.reset();
    }
}

//----- AbstractPaneFactory ----------------------------------------------------------

rtl::Reference<sd::framework::AbstractResource> PresenterPaneFactory::createResource (
    const rtl::Reference<sd::framework::ResourceId>& rxPaneId)
{
    {
        std::unique_lock l(m_aMutex);
        throwIfDisposed(l);
    }

    if ( ! rxPaneId.is())
        return nullptr;

    const OUString sPaneURL (rxPaneId->getResourceURL());
    if (sPaneURL.isEmpty())
        return nullptr;

    if (mpResourceCache != nullptr)
    {
        // Has the requested resource already been created?
        ResourceContainer::const_iterator iResource (mpResourceCache->find(sPaneURL));
        if (iResource != mpResourceCache->end())
        {
            // Yes.  Mark it as active.
            rtl::Reference<PresenterPaneContainer> pPaneContainer(
                mpPresenterController->GetPaneContainer());
            PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
                pPaneContainer->FindPaneURL(sPaneURL));
            if (pDescriptor)
            {
                pDescriptor->SetActivationState(true);
                if (pDescriptor->mxBorderWindow.is())
                    pDescriptor->mxBorderWindow->setVisible(true);
                pPaneContainer->StorePane(pDescriptor->mxPane);
            }

            return iResource->second;
        }
    }

    // No.  Create a new one.
    rtl::Reference<sd::framework::AbstractResource> xResource = CreatePane(rxPaneId);
    return xResource;
}

void PresenterPaneFactory::releaseResource (const rtl::Reference<sd::framework::AbstractResource>& rxResource)
{
    {
        std::unique_lock l(m_aMutex);
        throwIfDisposed(l);
    }

    if ( ! rxResource.is())
        throw lang::IllegalArgumentException();

    // Mark the pane as inactive.
    rtl::Reference<PresenterPaneContainer> pPaneContainer(
        mpPresenterController->GetPaneContainer());
    const OUString sPaneURL (rxResource->getResourceId()->getResourceURL());
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        pPaneContainer->FindPaneURL(sPaneURL));
    if (!pDescriptor)
        return;

    pDescriptor->SetActivationState(false);
    if (pDescriptor->mxBorderWindow.is())
        pDescriptor->mxBorderWindow->setVisible(false);

    if (mpResourceCache != nullptr)
    {
        // Store the pane in the cache.
        (*mpResourceCache)[sPaneURL] = rxResource;
    }
    else
    {
        // Dispose the pane.
        if (rxResource.is())
            rxResource->dispose();
    }
}


rtl::Reference<sd::framework::AbstractResource> PresenterPaneFactory::CreatePane (
    const rtl::Reference<sd::framework::ResourceId>& rxPaneId)
{
    if ( ! rxPaneId.is())
        return nullptr;

    rtl::Reference<::sd::framework::ConfigurationController> xCC (mxConfigurationControllerWeak);
    if ( ! xCC.is())
        return nullptr;

    Reference<XComponentContext> xContext (mxComponentContextWeak);
    if ( ! xContext.is())
        return nullptr;

    rtl::Reference<sd::framework::AbstractPane> xParentPane = dynamic_cast<sd::framework::AbstractPane*>(xCC->getResource(rxPaneId->getAnchor()).get());
    if ( ! xParentPane.is())
        return nullptr;

    try
    {
        return CreatePane(
            rxPaneId,
            xParentPane,
            rxPaneId->getFullResourceURL().Arguments == "Sprite=1");
    }
    catch (Exception&)
    {
        OSL_ASSERT(false);
    }

    return nullptr;
}

rtl::Reference<sd::framework::AbstractResource> PresenterPaneFactory::CreatePane (
    const rtl::Reference<sd::framework::ResourceId>& rxPaneId,
    const rtl::Reference<sd::framework::AbstractPane>& rxParentPane,
    const bool bIsSpritePane)
{
    Reference<XComponentContext> xContext (mxComponentContextWeak);
    Reference<lang::XMultiComponentFactory> xFactory (
        xContext->getServiceManager(), UNO_SET_THROW);

    // Create a border window and canvas and store it in the pane
    // container.

    // Create the pane.
    ::rtl::Reference<PresenterPaneBase> xPane;
    if (bIsSpritePane)
    {
        xPane.set( new PresenterSpritePane(xContext, mpPresenterController));
    }
    else
    {
        xPane.set( new PresenterPane(xContext, mpPresenterController));
    }

    xPane->initialize(rxPaneId, rxParentPane->getWindow(), rxParentPane->getCanvas(),
                      mpPresenterController->GetPaneBorderPainter(), !bIsSpritePane);

    // Store pane and canvases and windows in container.
    ::rtl::Reference<PresenterPaneContainer> pContainer (
        mpPresenterController->GetPaneContainer());
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor(
        pContainer->StoreBorderWindow(rxPaneId, xPane->GetBorderWindow()));
    pContainer->StorePane(xPane);
    if (pDescriptor)
    {
        pDescriptor->mbIsSprite = bIsSpritePane;

        // Get the window of the frame and make that visible.
        Reference<awt::XWindow> xWindow (pDescriptor->mxBorderWindow, UNO_SET_THROW);
        xWindow->setVisible(true);
    }

    return xPane;
}


} // end of namespace sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
