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
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

const ::rtl::OUString PresenterPaneFactory::msCurrentSlidePreviewPaneURL(
    "private:resource/pane/Presenter/Pane1");
const ::rtl::OUString PresenterPaneFactory::msNextSlidePreviewPaneURL(
    "private:resource/pane/Presenter/Pane2");
const ::rtl::OUString PresenterPaneFactory::msNotesPaneURL(
    "private:resource/pane/Presenter/Pane3");
const ::rtl::OUString PresenterPaneFactory::msToolBarPaneURL(
    "private:resource/pane/Presenter/Pane4");
const ::rtl::OUString PresenterPaneFactory::msSlideSorterPaneURL(
    "private:resource/pane/Presenter/Pane5");
const ::rtl::OUString PresenterPaneFactory::msHelpPaneURL(
    "private:resource/pane/Presenter/Pane6");

const ::rtl::OUString PresenterPaneFactory::msOverlayPaneURL(
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
        xCC = Reference<XConfigurationController>(xCM->getConfigurationController());
        mxConfigurationControllerWeak = xCC;
        if ( ! xCC.is())
        {
            throw RuntimeException();
        }
        else
        {
            xCC->addResourceFactory(
                OUString("private:resource/pane/Presenter/*"),
                this);
        }
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

PresenterPaneFactory::~PresenterPaneFactory (void)
{
}

void SAL_CALL PresenterPaneFactory::disposing (void)
    throw (RuntimeException)
{
    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is())
        xCC->removeResourceFactoryForReference(this);
    mxConfigurationControllerWeak = WeakReference<XConfigurationController>();

    // Dispose the panes in the cache.
    if (mpResourceCache.get() != NULL)
    {
        ResourceContainer::const_iterator iPane (mpResourceCache->begin());
        ResourceContainer::const_iterator iEnd (mpResourceCache->end());
        for ( ; iPane!=iEnd; ++iPane)
        {
            Reference<lang::XComponent> xPaneComponent (iPane->second, UNO_QUERY);
            if (xPaneComponent.is())
                xPaneComponent->dispose();
        }
        mpResourceCache.reset();
    }
}

//----- XPaneFactory ----------------------------------------------------------

Reference<XResource> SAL_CALL PresenterPaneFactory::createResource (
    const Reference<XResourceId>& rxPaneId)
    throw (RuntimeException, IllegalArgumentException, WrappedTargetException)
{
    ThrowIfDisposed();

    if ( ! rxPaneId.is())
        return NULL;

    const OUString sPaneURL (rxPaneId->getResourceURL());
    if (sPaneURL.isEmpty())
        return NULL;

    if (mpResourceCache.get() != NULL)
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
            if (pDescriptor.get() != NULL)
            {
                pDescriptor->SetActivationState(true);
                if (pDescriptor->mxBorderWindow.is())
                    pDescriptor->mxBorderWindow->setVisible(sal_True);
                pPaneContainer->StorePane(pDescriptor->mxPane);
            }

            return iResource->second;
        }
    }

    // No.  Create a new one.
    Reference<XResource> xResource = CreatePane(rxPaneId, OUString());
    return xResource;
}

void SAL_CALL PresenterPaneFactory::releaseResource (const Reference<XResource>& rxResource)
    throw (RuntimeException)
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
    if (pDescriptor.get() != NULL)
    {
        pDescriptor->SetActivationState(false);
        if (pDescriptor->mxBorderWindow.is())
            pDescriptor->mxBorderWindow->setVisible(sal_False);

        if (mpResourceCache.get() != NULL)
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
}

//-----------------------------------------------------------------------------

Reference<XResource> PresenterPaneFactory::CreatePane (
    const Reference<XResourceId>& rxPaneId,
    const OUString& rsTitle)
{
    if ( ! rxPaneId.is())
        return NULL;

    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if ( ! xCC.is())
        return NULL;

    Reference<XComponentContext> xContext (mxComponentContextWeak);
    if ( ! xContext.is())
        return NULL;

    Reference<XPane> xParentPane (xCC->getResource(rxPaneId->getAnchor()), UNO_QUERY);
    if ( ! xParentPane.is())
        return NULL;

    try
    {
        return CreatePane(
            rxPaneId,
            rsTitle,
            xParentPane,
            rxPaneId->getFullResourceURL().Arguments.compareToAscii("Sprite=1") == 0);
    }
    catch (Exception&)
    {
        OSL_ASSERT(false);
    }

    return NULL;
}

Reference<XResource> PresenterPaneFactory::CreatePane (
    const Reference<XResourceId>& rxPaneId,
    const OUString& rsTitle,
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
        xPane = ::rtl::Reference<PresenterPaneBase>(
            new PresenterSpritePane(xContext, mpPresenterController));
    }
    else
    {
        xPane = ::rtl::Reference<PresenterPaneBase>(
            new PresenterPane(xContext, mpPresenterController));
    }

    // Supply arguments.
    Sequence<Any> aArguments (6);
    aArguments[0] <<= rxPaneId;
    aArguments[1] <<= rxParentPane->getWindow();
    aArguments[2] <<= rxParentPane->getCanvas();
    aArguments[3] <<= rsTitle;
    aArguments[4] <<= Reference<drawing::framework::XPaneBorderPainter>(
        static_cast<XWeak*>(mpPresenterController->GetPaneBorderPainter().get()),
        UNO_QUERY);
    aArguments[5] <<= bIsSpritePane ? false : true;
    xPane->initialize(aArguments);

    // Store pane and canvases and windows in container.
    ::rtl::Reference<PresenterPaneContainer> pContainer (
        mpPresenterController->GetPaneContainer());
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor(
        pContainer->StoreBorderWindow(rxPaneId, xPane->GetBorderWindow()));
    pContainer->StorePane(xPane);
    if (pDescriptor.get() != NULL)
    {
        if (bIsSpritePane)
        {
            pDescriptor->maSpriteProvider = ::boost::bind(
                &PresenterSpritePane::GetSprite,
                dynamic_cast<PresenterSpritePane*>(xPane.get()));
            pDescriptor->mbIsSprite = true;
            pDescriptor->mbNeedsClipping = false;
        }
        else
        {
            pDescriptor->mbIsSprite = false;
            pDescriptor->mbNeedsClipping = true;
        }

        // Get the window of the frame and make that visible.
        Reference<awt::XWindow> xWindow (pDescriptor->mxBorderWindow, UNO_QUERY_THROW);
        xWindow->setVisible(sal_True);
    }

    return Reference<XResource>(static_cast<XWeak*>(xPane.get()), UNO_QUERY_THROW);
}

void PresenterPaneFactory::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString( "PresenterPaneFactory object has already been disposed"),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

} } // end of namespace sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
