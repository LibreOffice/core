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
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;

namespace sdext { namespace presenter {

const OUString PresenterPaneFactory::msCurrentSlidePreviewPaneURL(
    "private:resource/pane/Presenter/Pane1");
const OUString PresenterPaneFactory::msNextSlidePreviewPaneURL(
    "private:resource/pane/Presenter/Pane2");
const OUString PresenterPaneFactory::msNotesPaneURL(
    "private:resource/pane/Presenter/Pane3");
const OUString PresenterPaneFactory::msToolBarPaneURL(
    "private:resource/pane/Presenter/Pane4");
const OUString PresenterPaneFactory::msSlideSorterPaneURL(
    "private:resource/pane/Presenter/Pane5");
const OUString PresenterPaneFactory::msHelpPaneURL(
    "private:resource/pane/Presenter/Pane6");

const OUString PresenterPaneFactory::msOverlayPaneURL(
    "private:resource/pane/Presenter/Overlay");

//===== PresenterPaneFactory ==================================================

Reference<drawing::framework::XResourceFactory> PresenterPaneFactory::Create (
    const Reference<uno::XComponentContext>& rxContext,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
{
    rtl::Reference<PresenterPaneFactory> pFactory (
        new PresenterPaneFactory(rxContext,rpPresenterController));
    pFactory->Register(rxController);
    return Reference<drawing::framework::XResourceFactory>(
        static_cast<XWeak*>(pFactory.get()), UNO_QUERY);
}

PresenterPaneFactory::PresenterPaneFactory (
    const Reference<uno::XComponentContext>& rxContext,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterPaneFactoryInterfaceBase(m_aMutex),
      mxComponentContextWeak(rxContext),
      mxConfigurationControllerWeak(),
      mpPresenterController(rpPresenterController),
      mpResourceCache()
{
}

void PresenterPaneFactory::Register (const Reference<frame::XController>& rxController)
{
    Reference<XConfigurationController> xCC;
    try
    {
        // Get the configuration controller.
        Reference<XControllerManager> xCM (rxController, UNO_QUERY_THROW);
        xCC.set(xCM->getConfigurationController());
        mxConfigurationControllerWeak = xCC;
        if ( ! xCC.is())
        {
            throw RuntimeException();
        }
        xCC->addResourceFactory(
            "private:resource/pane/Presenter/*",
             this);
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
        if (xCC.is())
            xCC->removeResourceFactoryForReference(this);
        mxConfigurationControllerWeak = WeakReference<XConfigurationController>();

        throw;
    }
}

PresenterPaneFactory::~PresenterPaneFactory()
{
}

void SAL_CALL PresenterPaneFactory::disposing()
{
    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is())
        xCC->removeResourceFactoryForReference(this);
    mxConfigurationControllerWeak = WeakReference<XConfigurationController>();

    // Dispose the panes in the cache.
    if (mpResourceCache != nullptr)
    {
        for (const auto& rxPane : *mpResourceCache)
        {
            Reference<lang::XComponent> xPaneComponent (rxPane.second, UNO_QUERY);
            if (xPaneComponent.is())
                xPaneComponent->dispose();
        }
        mpResourceCache.reset();
    }
}

//----- XPaneFactory ----------------------------------------------------------

Reference<XResource> SAL_CALL PresenterPaneFactory::createResource (
    const Reference<XResourceId>& rxPaneId)
{
    ThrowIfDisposed();

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
            if (pDescriptor.get() != nullptr)
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
    Reference<XResource> xResource = CreatePane(rxPaneId);
    return xResource;
}

void SAL_CALL PresenterPaneFactory::releaseResource (const Reference<XResource>& rxResource)
{
    ThrowIfDisposed();

    if ( ! rxResource.is())
        throw lang::IllegalArgumentException();

    // Mark the pane as inactive.
    rtl::Reference<PresenterPaneContainer> pPaneContainer(
        mpPresenterController->GetPaneContainer());
    const OUString sPaneURL (rxResource->getResourceId()->getResourceURL());
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        pPaneContainer->FindPaneURL(sPaneURL));
    if (pDescriptor.get() == nullptr)
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
        Reference<lang::XComponent> xPaneComponent (rxResource, UNO_QUERY);
        if (xPaneComponent.is())
            xPaneComponent->dispose();
    }
}


Reference<XResource> PresenterPaneFactory::CreatePane (
    const Reference<XResourceId>& rxPaneId)
{
    if ( ! rxPaneId.is())
        return nullptr;

    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if ( ! xCC.is())
        return nullptr;

    Reference<XComponentContext> xContext (mxComponentContextWeak);
    if ( ! xContext.is())
        return nullptr;

    Reference<XPane> xParentPane (xCC->getResource(rxPaneId->getAnchor()), UNO_QUERY);
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

Reference<XResource> PresenterPaneFactory::CreatePane (
    const Reference<XResourceId>& rxPaneId,
    const Reference<drawing::framework::XPane>& rxParentPane,
    const bool bIsSpritePane)
{
    Reference<XComponentContext> xContext (mxComponentContextWeak);
    Reference<lang::XMultiComponentFactory> xFactory (
        xContext->getServiceManager(), UNO_QUERY_THROW);

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

    // Supply arguments.
    Sequence<Any> aArguments (6);
    aArguments[0] <<= rxPaneId;
    aArguments[1] <<= rxParentPane->getWindow();
    aArguments[2] <<= rxParentPane->getCanvas();
    aArguments[3] <<= OUString();
    aArguments[4] <<= Reference<drawing::framework::XPaneBorderPainter>(
        static_cast<XWeak*>(mpPresenterController->GetPaneBorderPainter().get()),
        UNO_QUERY);
    aArguments[5] <<= !bIsSpritePane;
    xPane->initialize(aArguments);

    // Store pane and canvases and windows in container.
    ::rtl::Reference<PresenterPaneContainer> pContainer (
        mpPresenterController->GetPaneContainer());
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor(
        pContainer->StoreBorderWindow(rxPaneId, xPane->GetBorderWindow()));
    pContainer->StorePane(xPane);
    if (pDescriptor.get() != nullptr)
    {
        pDescriptor->mbIsSprite = bIsSpritePane;

        // Get the window of the frame and make that visible.
        Reference<awt::XWindow> xWindow (pDescriptor->mxBorderWindow, UNO_QUERY_THROW);
        xWindow->setVisible(true);
    }

    return Reference<XResource>(static_cast<XWeak*>(xPane.get()), UNO_QUERY_THROW);
}

void PresenterPaneFactory::ThrowIfDisposed() const
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            "PresenterPaneFactory object has already been disposed",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

} } // end of namespace sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
