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

#include "PresenterViewFactory.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterHelpView.hxx"
#include "PresenterNotesView.hxx"
#include "PresenterSlideShowView.hxx"
#include "PresenterSlidePreview.hxx"
#include "PresenterSlideSorter.hxx"
#include "PresenterToolBar.hxx"
#include <DrawController.hxx>
#include <framework/ConfigurationController.hxx>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sdext::presenter {

namespace {

/** By default the PresenterSlidePreview shows the preview of the current
    slide.  This adapter class makes it display the preview of the next
    slide.
*/
class NextSlidePreview : public PresenterSlidePreview
{
public:
    NextSlidePreview (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const rtl::Reference<sd::framework::ResourceId>& rxViewId,
        const rtl::Reference<sd::framework::AbstractPane>& rxAnchorPane,
        const ::rtl::Reference<PresenterController>& rpPresenterController)
        : PresenterSlidePreview(rxContext, rxViewId, rxAnchorPane, rpPresenterController)
    {
    }

    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide) override
    {
        Reference<presentation::XSlideShowController> xSlideShowController (
            mpPresenterController->GetSlideShowController());
        Reference<drawing::XDrawPage> xSlide;
        if (xSlideShowController.is())
        {
            const sal_Int32 nCount (xSlideShowController->getSlideCount());
            sal_Int32 nNextSlideIndex (-1);
            if (xSlideShowController->getCurrentSlide() == rxSlide)
            {
                nNextSlideIndex = xSlideShowController->getNextSlideIndex();
            }
            else
            {
                for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
                {
                    if (rxSlide == xSlideShowController->getSlideByIndex(nIndex))
                    {
                        nNextSlideIndex = nIndex + 1;
                    }
                }
            }
            if (nNextSlideIndex >= 0)
            {
                if (nNextSlideIndex < nCount)
                {
                    xSlide = xSlideShowController->getSlideByIndex(nNextSlideIndex);
                }
            }
        }
        PresenterSlidePreview::setCurrentPage(xSlide);
    }
};

} // end of anonymous namespace

//===== PresenterViewFactory ==============================================

PresenterViewFactory::PresenterViewFactory (
    const Reference<uno::XComponentContext>& rxContext,
    const rtl::Reference<::sd::DrawController>& rxController,
    ::rtl::Reference<PresenterController> pPresenterController)
    : mxComponentContext(rxContext),
      mxControllerWeak(rxController),
      mpPresenterController(std::move(pPresenterController))
{
}

rtl::Reference<sd::framework::ResourceFactory> PresenterViewFactory::Create (
    const Reference<uno::XComponentContext>& rxContext,
    const rtl::Reference<::sd::DrawController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
{
    rtl::Reference<PresenterViewFactory> pFactory (
        new PresenterViewFactory(rxContext,rxController,rpPresenterController));
    pFactory->Register(rxController);
    return pFactory;
}

void PresenterViewFactory::Register (const rtl::Reference<::sd::DrawController>& rxController)
{
    try
    {
        // Get the configuration controller.
        mxConfigurationController = rxController->getConfigurationController();
        if ( ! mxConfigurationController.is())
        {
            throw RuntimeException();
        }
        mxConfigurationController->addResourceFactory(msCurrentSlidePreviewViewURL, this);
        mxConfigurationController->addResourceFactory(msNextSlidePreviewViewURL, this);
        mxConfigurationController->addResourceFactory(msNotesViewURL, this);
        mxConfigurationController->addResourceFactory(msToolBarViewURL, this);
        mxConfigurationController->addResourceFactory(msSlideSorterURL, this);
        mxConfigurationController->addResourceFactory(msHelpViewURL, this);
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
        if (mxConfigurationController.is())
            mxConfigurationController->removeResourceFactoryForReference(this);
        mxConfigurationController = nullptr;

        throw;
    }
}

PresenterViewFactory::~PresenterViewFactory()
{
}

void PresenterViewFactory::disposing(std::unique_lock<std::mutex>&)
{
    if (mxConfigurationController.is())
        mxConfigurationController->removeResourceFactoryForReference(this);
    mxConfigurationController = nullptr;
}

//----- XViewFactory ----------------------------------------------------------

rtl::Reference<sd::framework::AbstractResource> PresenterViewFactory::createResource (
    const rtl::Reference<sd::framework::ResourceId>& rxViewId)
{
    {
        std::unique_lock l(m_aMutex);
        throwIfDisposed(l);
    }

    rtl::Reference<sd::framework::AbstractView> xView;

    if (rxViewId.is())
    {
        rtl::Reference<sd::framework::AbstractPane> xAnchorPane = dynamic_cast<sd::framework::AbstractPane*>(
            mxConfigurationController->getResource(rxViewId->getAnchor()).get());
        xView = CreateView(rxViewId, xAnchorPane);

        // Activate the view.
        PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
            mpPresenterController->GetPaneContainer()->FindPaneId(rxViewId->getAnchor()));
        if (pDescriptor)
            pDescriptor->SetActivationState(true);
    }

    return xView;
}

void PresenterViewFactory::releaseResource (const rtl::Reference<sd::framework::AbstractResource>& rxView)
{
    {
        std::unique_lock l(m_aMutex);
        throwIfDisposed(l);
    }

    if ( ! rxView.is())
        return;

    // Deactivate the view.
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPresenterController->GetPaneContainer()->FindPaneId(
            rxView->getResourceId()->getAnchor()));
    if (pDescriptor)
        pDescriptor->SetActivationState(false);

    CachablePresenterView* pView = dynamic_cast<CachablePresenterView*>(rxView.get());
    try
    {
        if (pView != nullptr)
            pView->ReleaseView();
        if (rxView.is())
            rxView->dispose();
    }
    catch (lang::DisposedException&)
    {
        // Do not let disposed exceptions get out.  It might be interpreted
        // as coming from the factory, which would then be removed from the
        // drawing framework.
    }
}

rtl::Reference<sd::framework::AbstractView> PresenterViewFactory::CreateView(
    const rtl::Reference<sd::framework::ResourceId>& rxViewId,
    const rtl::Reference<sd::framework::AbstractPane>& rxAnchorPane)
{
    rtl::Reference<sd::framework::AbstractView> xView;

    try
    {
        const OUString sResourceURL (rxViewId->getResourceURL());

        if (sResourceURL == msCurrentSlidePreviewViewURL)
        {
            xView = CreateSlideShowView(rxViewId);
        }
        else if (sResourceURL == msNotesViewURL)
        {
            xView = CreateNotesView(rxViewId);
        }
        else if (sResourceURL == msNextSlidePreviewViewURL)
        {
            xView = CreateSlidePreviewView(rxViewId, rxAnchorPane);
        }
        else if (sResourceURL == msToolBarViewURL)
        {
            xView = CreateToolBarView(rxViewId);
        }
        else if (sResourceURL == msSlideSorterURL)
        {
            xView = CreateSlideSorterView(rxViewId);
        }
        else if (sResourceURL == msHelpViewURL)
        {
            xView = CreateHelpView(rxViewId);
        }

        // Activate it.
        CachablePresenterView* pView = dynamic_cast<CachablePresenterView*>(xView.get());
        if (pView != nullptr)
            pView->ActivatePresenterView();
    }
    catch (RuntimeException&)
    {
        xView = nullptr;
    }

    return xView;
}

rtl::Reference<sd::framework::AbstractView> PresenterViewFactory::CreateSlideShowView(
    const rtl::Reference<sd::framework::ResourceId>& rxViewId) const
{
    if ( ! mxConfigurationController.is())
        return nullptr;
    if ( ! mxComponentContext.is())
        return nullptr;

    try
    {
        rtl::Reference<PresenterSlideShowView> xView;
        xView =
            new PresenterSlideShowView(
                mxComponentContext,
                rxViewId,
                mxControllerWeak.get(),
                mpPresenterController);
        xView->LateInit();
        return xView;
    }
    catch (RuntimeException&)
    {
        return nullptr;
    }
}

rtl::Reference<sd::framework::AbstractView> PresenterViewFactory::CreateSlidePreviewView(
    const rtl::Reference<sd::framework::ResourceId>& rxViewId,
    const rtl::Reference<sd::framework::AbstractPane>& rxAnchorPane) const
{
    rtl::Reference<sd::framework::AbstractView> xView;

    if ( ! mxConfigurationController.is())
        return xView;
    if ( ! mxComponentContext.is())
        return xView;

    try
    {
        xView =
            new NextSlidePreview(
                mxComponentContext,
                rxViewId,
                rxAnchorPane,
                mpPresenterController);
    }
    catch (RuntimeException&)
    {
        xView = nullptr;
    }

    return xView;
}

rtl::Reference<sd::framework::AbstractView> PresenterViewFactory::CreateToolBarView(
    const rtl::Reference<sd::framework::ResourceId>& rxViewId) const
{
    return new PresenterToolBarView(
        mxComponentContext,
        rxViewId,
        mxControllerWeak.get(),
        mpPresenterController);
}

rtl::Reference<sd::framework::AbstractView> PresenterViewFactory::CreateNotesView(
    const rtl::Reference<sd::framework::ResourceId>& rxViewId) const
{
    rtl::Reference<sd::framework::AbstractView> xView;

    if ( ! mxConfigurationController.is())
        return xView;
    if ( ! mxComponentContext.is())
        return xView;

    try
    {
        xView =
            new PresenterNotesView(
                mxComponentContext,
                rxViewId,
                mxControllerWeak.get(),
                mpPresenterController);
    }
    catch (RuntimeException&)
    {
        xView = nullptr;
    }

    return xView;
}

rtl::Reference<sd::framework::AbstractView> PresenterViewFactory::CreateSlideSorterView(
    const rtl::Reference<sd::framework::ResourceId>& rxViewId) const
{
    rtl::Reference<sd::framework::AbstractView> xView;

    if ( ! mxConfigurationController.is())
        return xView;
    if ( ! mxComponentContext.is())
        return xView;

    try
    {
        rtl::Reference<PresenterSlideSorter> pView (
            new PresenterSlideSorter(
                mxComponentContext,
                rxViewId,
                mxControllerWeak.get(),
                mpPresenterController));
        xView = pView.get();
    }
    catch (RuntimeException&)
    {
        xView = nullptr;
    }

    return xView;
}

rtl::Reference<sd::framework::AbstractView> PresenterViewFactory::CreateHelpView(
    const rtl::Reference<sd::framework::ResourceId>& rxViewId) const
{
    return new PresenterHelpView(
        mxComponentContext,
        rxViewId,
        mxControllerWeak.get(),
        mpPresenterController);
}

//===== CachablePresenterView =================================================

CachablePresenterView::CachablePresenterView()
    : mbIsPresenterViewActive(true)
{
}

void CachablePresenterView::ActivatePresenterView()
{
    mbIsPresenterViewActive = true;
}

void CachablePresenterView::ReleaseView()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
