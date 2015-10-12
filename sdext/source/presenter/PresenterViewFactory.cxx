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
#include "PresenterHelper.hxx"
#include "PresenterHelpView.hxx"
#include "PresenterNotesView.hxx"
#include "PresenterSlideShowView.hxx"
#include "PresenterSlidePreview.hxx"
#include "PresenterSlideSorter.hxx"
#include "PresenterToolBar.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XSlideSorterBase.hpp>
#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sdext { namespace presenter {

const OUString PresenterViewFactory::msCurrentSlidePreviewViewURL(
    "private:resource/view/Presenter/CurrentSlidePreview");
const OUString PresenterViewFactory::msNextSlidePreviewViewURL(
    "private:resource/view/Presenter/NextSlidePreview");
const OUString PresenterViewFactory::msNotesViewURL(
    "private:resource/view/Presenter/Notes");
const OUString PresenterViewFactory::msToolBarViewURL(
    "private:resource/view/Presenter/ToolBar");
const OUString PresenterViewFactory::msSlideSorterURL(
    "private:resource/view/Presenter/SlideSorter");
const OUString PresenterViewFactory::msHelpViewURL(
    "private:resource/view/Presenter/Help");

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
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::drawing::framework::XPane>& rxAnchorPane,
        const ::rtl::Reference<PresenterController>& rpPresenterController)
        : PresenterSlidePreview(rxContext, rxViewId, rxAnchorPane, rpPresenterController)
    {
    }
    virtual ~NextSlidePreview() {}
    virtual void SAL_CALL setCurrentPage (
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide)
        throw (css::uno::RuntimeException, std::exception) override
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
                    if (rxSlide == Reference<drawing::XDrawPage>(
                        xSlideShowController->getSlideByIndex(nIndex), UNO_QUERY))
                    {
                        nNextSlideIndex = nIndex + 1;
                    }
                }
            }
            if (nNextSlideIndex >= 0)
            {
                if (nNextSlideIndex < nCount)
                {
                    xSlide = Reference<drawing::XDrawPage>(
                        xSlideShowController->getSlideByIndex(nNextSlideIndex),
                         UNO_QUERY);
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
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterViewFactoryInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mxConfigurationController(),
      mxControllerWeak(rxController),
      mpPresenterController(rpPresenterController),
      mpResourceCache()
{
}

Reference<drawing::framework::XResourceFactory> PresenterViewFactory::Create (
    const Reference<uno::XComponentContext>& rxContext,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
{
    rtl::Reference<PresenterViewFactory> pFactory (
        new PresenterViewFactory(rxContext,rxController,rpPresenterController));
    pFactory->Register(rxController);
    return Reference<drawing::framework::XResourceFactory>(
        static_cast<XWeak*>(pFactory.get()), UNO_QUERY);
}

void PresenterViewFactory::Register (const Reference<frame::XController>& rxController)
{
    try
    {
        // Get the configuration controller.
        Reference<XControllerManager> xCM (rxController, UNO_QUERY_THROW);
        mxConfigurationController = xCM->getConfigurationController();
        if ( ! mxConfigurationController.is())
        {
            throw RuntimeException();
        }
        else
        {
            mxConfigurationController->addResourceFactory(msCurrentSlidePreviewViewURL, this);
            mxConfigurationController->addResourceFactory(msNextSlidePreviewViewURL, this);
            mxConfigurationController->addResourceFactory(msNotesViewURL, this);
            mxConfigurationController->addResourceFactory(msToolBarViewURL, this);
            mxConfigurationController->addResourceFactory(msSlideSorterURL, this);
            mxConfigurationController->addResourceFactory(msHelpViewURL, this);
        }
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
        if (mxConfigurationController.is())
            mxConfigurationController->removeResourceFactoryForReference(this);
        mxConfigurationController = NULL;

        throw;
    }
}

PresenterViewFactory::~PresenterViewFactory()
{
}

void SAL_CALL PresenterViewFactory::disposing()
    throw (RuntimeException)
{
    if (mxConfigurationController.is())
        mxConfigurationController->removeResourceFactoryForReference(this);
    mxConfigurationController = NULL;

    if (mpResourceCache.get() != NULL)
    {
        // Dispose all views in the cache.
        ResourceContainer::const_iterator iView (mpResourceCache->begin());
        ResourceContainer::const_iterator iEnd (mpResourceCache->end());
        for ( ; iView!=iEnd; ++iView)
        {
            try
            {
                Reference<lang::XComponent> xComponent (iView->second.first, UNO_QUERY);
                if (xComponent.is())
                    xComponent->dispose();
            }
            catch (lang::DisposedException&)
            {
            }
        }
        mpResourceCache.reset();
    }
}

//----- XViewFactory ----------------------------------------------------------

Reference<XResource> SAL_CALL PresenterViewFactory::createResource (
    const Reference<XResourceId>& rxViewId)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    Reference<XResource> xView;

    if (rxViewId.is())
    {
        Reference<XPane> xAnchorPane (
            mxConfigurationController->getResource(rxViewId->getAnchor()),
            UNO_QUERY_THROW);
        xView = GetViewFromCache(rxViewId, xAnchorPane);
        if (xView == NULL)
            xView = CreateView(rxViewId, xAnchorPane);

        // Activate the view.
        PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
            mpPresenterController->GetPaneContainer()->FindPaneId(rxViewId->getAnchor()));
        if (pDescriptor.get() != NULL)
            pDescriptor->SetActivationState(true);
    }

    return xView;
}

void SAL_CALL PresenterViewFactory::releaseResource (const Reference<XResource>& rxView)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    if ( ! rxView.is())
        return;

    // Deactivate the view.
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPresenterController->GetPaneContainer()->FindPaneId(
            rxView->getResourceId()->getAnchor()));
    if (pDescriptor.get() != NULL)
        pDescriptor->SetActivationState(false);

    // Dispose only views that we can not put into the cache.
    CachablePresenterView* pView = dynamic_cast<CachablePresenterView*>(rxView.get());
    if (pView == NULL || mpResourceCache.get()==NULL)
    {
        try
        {
            if (pView != NULL)
                pView->ReleaseView();
            Reference<lang::XComponent> xComponent (rxView, UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
        }
        catch (lang::DisposedException&)
        {
            // Do not let disposed exceptions get out.  It might be interpreted
            // as coming from the factory, which would then be removed from the
            // drawing framework.
        }
    }
    else
    {
        // Put cachable views in the cache.
        Reference<XResourceId> xViewId (rxView->getResourceId());
        if (xViewId.is())
        {
            Reference<XPane> xAnchorPane (
                mxConfigurationController->getResource(xViewId->getAnchor()),
                UNO_QUERY_THROW);
            (*mpResourceCache)[xViewId->getResourceURL()]
                = ViewResourceDescriptor(Reference<XView>(rxView, UNO_QUERY), xAnchorPane);
            pView->DeactivatePresenterView();
        }
    }
}



Reference<XResource> PresenterViewFactory::GetViewFromCache(
    const Reference<XResourceId>& rxViewId,
    const Reference<XPane>& rxAnchorPane) const
{
    if (mpResourceCache.get() == NULL)
        return NULL;

    try
    {
        const OUString sResourceURL (rxViewId->getResourceURL());

        // Can we use a view from the cache?
        ResourceContainer::const_iterator iView (mpResourceCache->find(sResourceURL));
        if (iView != mpResourceCache->end())
        {
            // The view is in the container but it can only be used if
            // the anchor pane is the same now as it was at creation of
            // the view.
            if (iView->second.second == rxAnchorPane)
            {
                CachablePresenterView* pView
                    = dynamic_cast<CachablePresenterView*>(iView->second.first.get());
                if (pView != NULL)
                    pView->ActivatePresenterView();
                return Reference<XResource>(iView->second.first, UNO_QUERY);
            }

            // Right view, wrong pane.  Create a new view.
        }
    }
    catch (RuntimeException&)
    {
    }
    return NULL;
}

Reference<XResource> PresenterViewFactory::CreateView(
    const Reference<XResourceId>& rxViewId,
    const Reference<XPane>& rxAnchorPane)
{
    Reference<XView> xView;

    try
    {
        const OUString sResourceURL (rxViewId->getResourceURL());

        if (sResourceURL.equals(msCurrentSlidePreviewViewURL))
        {
            xView = CreateSlideShowView(rxViewId);
        }
        else if (sResourceURL.equals(msNotesViewURL))
        {
            xView = CreateNotesView(rxViewId, rxAnchorPane);
        }
        else if (sResourceURL.equals(msNextSlidePreviewViewURL))
        {
            xView = CreateSlidePreviewView(rxViewId, rxAnchorPane);
        }
        else if (sResourceURL.equals(msToolBarViewURL))
        {
            xView = CreateToolBarView(rxViewId);
        }
        else if (sResourceURL.equals(msSlideSorterURL))
        {
            xView = CreateSlideSorterView(rxViewId);
        }
        else if (sResourceURL.equals(msHelpViewURL))
        {
            xView = CreateHelpView(rxViewId);
        }

        // Activate it.
        CachablePresenterView* pView = dynamic_cast<CachablePresenterView*>(xView.get());
        if (pView != NULL)
            pView->ActivatePresenterView();
    }
    catch (RuntimeException&)
    {
        xView = NULL;
    }

    return xView;
}

Reference<XView> PresenterViewFactory::CreateSlideShowView(
    const Reference<XResourceId>& rxViewId) const
{
    Reference<XView> xView;

    if ( ! mxConfigurationController.is())
        return xView;
    if ( ! mxComponentContext.is())
        return xView;

    try
    {
        rtl::Reference<PresenterSlideShowView> pShowView (
            new PresenterSlideShowView(
                mxComponentContext,
                rxViewId,
                Reference<frame::XController>(mxControllerWeak),
                mpPresenterController));
        pShowView->LateInit();
        xView = Reference<XView>(pShowView.get());
    }
    catch (RuntimeException&)
    {
        xView = NULL;
    }

    return xView;
}

Reference<XView> PresenterViewFactory::CreateSlidePreviewView(
    const Reference<XResourceId>& rxViewId,
    const Reference<XPane>& rxAnchorPane) const
{
    Reference<XView> xView;

    if ( ! mxConfigurationController.is())
        return xView;
    if ( ! mxComponentContext.is())
        return xView;

    try
    {
        xView = Reference<XView>(
            static_cast<XWeak*>(new NextSlidePreview(
                mxComponentContext,
                rxViewId,
                rxAnchorPane,
                mpPresenterController)),
            UNO_QUERY_THROW);
    }
    catch (RuntimeException&)
    {
        xView = NULL;
    }

    return xView;
}

Reference<XView> PresenterViewFactory::CreateToolBarView(
    const Reference<XResourceId>& rxViewId) const
{
    return new PresenterToolBarView(
        mxComponentContext,
        rxViewId,
        Reference<frame::XController>(mxControllerWeak),
        mpPresenterController);
}

Reference<XView> PresenterViewFactory::CreateNotesView(
    const Reference<XResourceId>& rxViewId,
    const Reference<XPane>& rxAnchorPane) const
{
    (void)rxAnchorPane;
    Reference<XView> xView;

    if ( ! mxConfigurationController.is())
        return xView;
    if ( ! mxComponentContext.is())
        return xView;

    try
    {
        xView = Reference<XView>(static_cast<XWeak*>(
            new PresenterNotesView(
                mxComponentContext,
                rxViewId,
                Reference<frame::XController>(mxControllerWeak),
                mpPresenterController)),
            UNO_QUERY_THROW);
    }
    catch (RuntimeException&)
    {
        xView = NULL;
    }

    return xView;
}

Reference<XView> PresenterViewFactory::CreateSlideSorterView(
    const Reference<XResourceId>& rxViewId) const
{
    Reference<XView> xView;

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
                Reference<frame::XController>(mxControllerWeak),
                mpPresenterController));
        PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
            mpPresenterController->GetPaneContainer()->FindPaneId(rxViewId->getAnchor()));
        if (pDescriptor.get() != NULL)
            pDescriptor->maActivator = ::boost::bind(
                &PresenterSlideSorter::SetActiveState, _1);
        xView = pView.get();
    }
    catch (RuntimeException&)
    {
        xView = NULL;
    }

    return xView;
}

Reference<XView> PresenterViewFactory::CreateHelpView(
    const Reference<XResourceId>& rxViewId) const
{
    return Reference<XView>(new PresenterHelpView(
        mxComponentContext,
        rxViewId,
        Reference<frame::XController>(mxControllerWeak),
        mpPresenterController));
}

void PresenterViewFactory::ThrowIfDisposed() const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(
                "PresenterViewFactory object has already been disposed"),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
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

void CachablePresenterView::DeactivatePresenterView()
{
    mbIsPresenterViewActive = false;
}

void CachablePresenterView::ReleaseView()
{
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
