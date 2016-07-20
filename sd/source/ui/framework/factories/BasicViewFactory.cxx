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

#include "BasicViewFactory.hxx"

#include "framework/ViewShellWrapper.hxx"
#include "framework/FrameworkHelper.hxx"
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include "framework/Pane.hxx"
#include "DrawController.hxx"
#include "DrawSubController.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"
#include "DrawDocShell.hxx"
#include "DrawViewShell.hxx"
#include "GraphicViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "PresentationViewShell.hxx"
#include "SlideSorterViewShell.hxx"
#include "FrameView.hxx"
#include "facreg.hxx"
#include "Window.hxx"

#include <sfx2/viewfrm.hxx>
#include <vcl/wrkwin.hxx>
#include <toolkit/helper/vclunohelper.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;

namespace sd { namespace framework {

//===== ViewDescriptor ========================================================

class BasicViewFactory::ViewDescriptor
{
public:
    Reference<XResource> mxView;
    std::shared_ptr<sd::ViewShell> mpViewShell;
    ViewShellWrapper* mpWrapper;
    Reference<XResourceId> mxViewId;
    static bool CompareView (const std::shared_ptr<ViewDescriptor>& rpDescriptor,
        const Reference<XResource>& rxView)
    { return rpDescriptor->mxView.get() == rxView.get(); }
};

//===== BasicViewFactory::ViewShellContainer ==================================

class BasicViewFactory::ViewShellContainer
    : public ::std::vector<std::shared_ptr<ViewDescriptor> >
{
public:
    ViewShellContainer() {};
};

class BasicViewFactory::ViewCache
    : public ::std::vector<std::shared_ptr<ViewDescriptor> >
{
public:
    ViewCache() {};
};

//===== ViewFactory ===========================================================

BasicViewFactory::BasicViewFactory (
    const Reference<XComponentContext>& rxContext)
    : BasicViewFactoryInterfaceBase(MutexOwner::maMutex),
      mxConfigurationController(),
      mpViewShellContainer(new ViewShellContainer()),
      mpBase(nullptr),
      mpFrameView(nullptr),
      mpWindow(VclPtr<WorkWindow>::Create(nullptr,WB_STDWORK)),
      mpViewCache(new ViewCache()),
      mxLocalPane(new Pane(Reference<XResourceId>(), mpWindow.get()))
{
    (void)rxContext;
}

BasicViewFactory::~BasicViewFactory()
{
}

void SAL_CALL BasicViewFactory::disposing()
{
    // Disconnect from the frame view.
    if (mpFrameView != nullptr)
    {
        mpFrameView->Disconnect();
        mpFrameView = nullptr;
    }

    // Release the view cache.
    ViewShellContainer::const_iterator iView;
    for (iView=mpViewCache->begin(); iView!=mpViewCache->end(); ++iView)
    {
        ReleaseView(*iView, true);
    }

    // Release the view shell container.  At this point no one other than us
    // should hold references to the view shells (at the moment this is a
    // trivial requirement, because no one other then us holds a shared
    // pointer).
    //    ViewShellContainer::const_iterator iView;
    for (iView=mpViewShellContainer->begin(); iView!=mpViewShellContainer->end(); ++iView)
    {
        OSL_ASSERT((*iView)->mpViewShell.unique());
    }
    mpViewShellContainer.reset();
}

Reference<XResource> SAL_CALL BasicViewFactory::createResource (
    const Reference<XResourceId>& rxViewId)
    throw(RuntimeException, IllegalArgumentException, WrappedTargetException, std::exception)
{
    Reference<XResource> xView;
    const bool bIsCenterPane (
        rxViewId->isBoundToURL(FrameworkHelper::msCenterPaneURL, AnchorBindingMode_DIRECT));

    // Get the pane for the anchor URL.
    Reference<XPane> xPane;
    if (mxConfigurationController.is())
        xPane.set(mxConfigurationController->getResource(rxViewId->getAnchor()), UNO_QUERY);

    // For main views use the frame view of the last main view.
    ::sd::FrameView* pFrameView = nullptr;
    if (xPane.is() && bIsCenterPane)
    {
        pFrameView = mpFrameView;
    }

    // Get Window pointer for XWindow of the pane.
    vcl::Window* pWindow = nullptr;
    if (xPane.is())
        pWindow = VCLUnoHelper::GetWindow(xPane->getWindow());

    // Get the view frame.
    SfxViewFrame* pFrame = nullptr;
    if (mpBase != nullptr)
        pFrame = mpBase->GetViewFrame();

    if (pFrame != nullptr && mpBase!=nullptr && pWindow!=nullptr)
    {
        // Try to get the view from the cache.
        std::shared_ptr<ViewDescriptor> pDescriptor (GetViewFromCache(rxViewId, xPane));

        // When the requested view is not in the cache then create a new view.
        if (pDescriptor.get() == nullptr)
        {
            pDescriptor = CreateView(rxViewId, *pFrame, *pWindow, xPane, pFrameView, bIsCenterPane);
        }

        if (pDescriptor.get() != nullptr)
            xView = pDescriptor->mxView;

        mpViewShellContainer->push_back(pDescriptor);

        if (bIsCenterPane)
            ActivateCenterView(pDescriptor);
        else
            pWindow->Resize();
    }

    return xView;
}

void SAL_CALL BasicViewFactory::releaseResource (const Reference<XResource>& rxView)
    throw(RuntimeException, std::exception)
{
    if ( ! rxView.is())
        throw lang::IllegalArgumentException();

    if (rxView.is() && mpBase!=nullptr)
    {
        ViewShellContainer::iterator iViewShell (
            ::std::find_if(
                mpViewShellContainer->begin(),
                mpViewShellContainer->end(),
                [&] (std::shared_ptr<ViewDescriptor> const& pVD) {
                    return ViewDescriptor::CompareView(pVD, rxView);
                } ));
        if (iViewShell != mpViewShellContainer->end())
        {
            std::shared_ptr<ViewShell> pViewShell ((*iViewShell)->mpViewShell);

            if ((*iViewShell)->mxViewId->isBoundToURL(
                FrameworkHelper::msCenterPaneURL, AnchorBindingMode_DIRECT))
            {
                // Obtain a pointer to and connect to the frame view of the
                // view.  The next view, that is created, will be
                // initialized with this frame view.
                if (mpFrameView == nullptr)
                {
                    mpFrameView = pViewShell->GetFrameView();
                    if (mpFrameView)
                        mpFrameView->Connect();
                }

                // With the view in the center pane the sub controller is
                // released, too.
                mpBase->GetDrawController().SetSubController(
                    Reference<drawing::XDrawSubController>());

                SfxViewShell* pSfxViewShell = pViewShell->GetViewShell();
                if (pSfxViewShell != nullptr)
                    pSfxViewShell->DisconnectAllClients();
            }

            ReleaseView(*iViewShell);

            mpViewShellContainer->erase(iViewShell);
        }
        else
        {
            throw lang::IllegalArgumentException();
        }
    }
}

void SAL_CALL BasicViewFactory::initialize (const Sequence<Any>& aArguments)
    throw (Exception, RuntimeException, std::exception)
{
    if (aArguments.getLength() > 0)
    {
        Reference<XConfigurationController> xCC;
        try
        {
            // Get the XController from the first argument.
            Reference<frame::XController> xController (aArguments[0], UNO_QUERY_THROW);

            // Tunnel through the controller to obtain a ViewShellBase.
            Reference<lang::XUnoTunnel> xTunnel (xController, UNO_QUERY_THROW);
            ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
                xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
            if (pController != nullptr)
                mpBase = pController->GetViewShellBase();

            // Register the factory for its supported views.
            Reference<XControllerManager> xCM (xController,UNO_QUERY_THROW);
            mxConfigurationController = xCM->getConfigurationController();
            if ( ! mxConfigurationController.is())
                throw RuntimeException();
            mxConfigurationController->addResourceFactory(FrameworkHelper::msImpressViewURL, this);
            mxConfigurationController->addResourceFactory(FrameworkHelper::msDrawViewURL, this);
            mxConfigurationController->addResourceFactory(FrameworkHelper::msOutlineViewURL, this);
            mxConfigurationController->addResourceFactory(FrameworkHelper::msNotesViewURL, this);
            mxConfigurationController->addResourceFactory(FrameworkHelper::msHandoutViewURL, this);
            mxConfigurationController->addResourceFactory(FrameworkHelper::msPresentationViewURL, this);
            mxConfigurationController->addResourceFactory(FrameworkHelper::msSlideSorterURL, this);
        }
        catch (RuntimeException&)
        {
            mpBase = nullptr;
            if (mxConfigurationController.is())
                mxConfigurationController->removeResourceFactoryForReference(this);
            throw;
        }
    }
}

std::shared_ptr<BasicViewFactory::ViewDescriptor> BasicViewFactory::CreateView (
    const Reference<XResourceId>& rxViewId,
    SfxViewFrame& rFrame,
    vcl::Window& rWindow,
    const Reference<XPane>& rxPane,
    FrameView* pFrameView,
    const bool bIsCenterPane)
{
    std::shared_ptr<ViewDescriptor> pDescriptor (new ViewDescriptor());

    pDescriptor->mpViewShell = CreateViewShell(
        rxViewId,
        rFrame,
        rWindow,
        pFrameView,
        bIsCenterPane);
    pDescriptor->mxViewId = rxViewId;

    if (pDescriptor->mpViewShell.get() != nullptr)
    {
        pDescriptor->mpViewShell->Init(bIsCenterPane);
        mpBase->GetViewShellManager()->ActivateViewShell(pDescriptor->mpViewShell.get());

        Reference<awt::XWindow> xWindow(rxPane->getWindow());
        pDescriptor->mpWrapper = new ViewShellWrapper(
            pDescriptor->mpViewShell,
            rxViewId,
            xWindow);

        // register ViewShellWrapper on pane window
        if (xWindow.is())
        {
            xWindow->addWindowListener(pDescriptor->mpWrapper);
            if (pDescriptor->mpViewShell != nullptr)
            {
                pDescriptor->mpViewShell->Resize();
            }
        }

        pDescriptor->mxView.set( pDescriptor->mpWrapper->queryInterface( cppu::UnoType<XResource>::get() ), UNO_QUERY_THROW );
    }

    return pDescriptor;
}

std::shared_ptr<ViewShell> BasicViewFactory::CreateViewShell (
    const Reference<XResourceId>& rxViewId,
    SfxViewFrame& rFrame,
    vcl::Window& rWindow,
    FrameView* pFrameView,
    const bool bIsCenterPane)
{
    std::shared_ptr<ViewShell> pViewShell;
    const OUString& rsViewURL (rxViewId->getResourceURL());
    if (rsViewURL.equals(FrameworkHelper::msImpressViewURL))
    {
        pViewShell.reset(
            new DrawViewShell(
                &rFrame,
                *mpBase,
                &rWindow,
                PK_STANDARD,
                pFrameView));
        pViewShell->GetContentWindow()->set_id("impress_win");
    }
    else if (rsViewURL.equals(FrameworkHelper::msDrawViewURL))
    {
        pViewShell.reset(
            new GraphicViewShell (
                &rFrame,
                *mpBase,
                &rWindow,
                pFrameView));
        pViewShell->GetContentWindow()->set_id("draw_win");
    }
    else if (rsViewURL.equals(FrameworkHelper::msOutlineViewURL))
    {
        pViewShell.reset(
            new OutlineViewShell (
                &rFrame,
                *mpBase,
                &rWindow,
                pFrameView));
        pViewShell->GetContentWindow()->set_id("outline_win");
    }
    else if (rsViewURL.equals(FrameworkHelper::msNotesViewURL))
    {
        pViewShell.reset(
            new DrawViewShell(
                &rFrame,
                *mpBase,
                &rWindow,
                PK_NOTES,
                pFrameView));
        pViewShell->GetContentWindow()->set_id("notes_win");
    }
    else if (rsViewURL.equals(FrameworkHelper::msHandoutViewURL))
    {
        pViewShell.reset(
            new DrawViewShell(
                &rFrame,
                *mpBase,
                &rWindow,
                PK_HANDOUT,
                pFrameView));
        pViewShell->GetContentWindow()->set_id("handout_win");
    }
    else if (rsViewURL.equals(FrameworkHelper::msPresentationViewURL))
    {
        pViewShell.reset(
            new PresentationViewShell(
                &rFrame,
                *mpBase,
                &rWindow,
                pFrameView));
        pViewShell->GetContentWindow()->set_id("presentation_win");
    }
    else if (rsViewURL.equals(FrameworkHelper::msSlideSorterURL))
    {
        pViewShell = ::sd::slidesorter::SlideSorterViewShell::Create (
            &rFrame,
            *mpBase,
            &rWindow,
            pFrameView,
            bIsCenterPane);
    }

    return pViewShell;
}

void BasicViewFactory::ReleaseView (
    const std::shared_ptr<ViewDescriptor>& rpDescriptor,
    bool bDoNotCache)
{
    bool bIsCacheable (!bDoNotCache && IsCacheable(rpDescriptor));

    if (bIsCacheable)
    {
        Reference<XRelocatableResource> xResource (rpDescriptor->mxView, UNO_QUERY);
        if (xResource.is())
        {
            Reference<XResource> xNewAnchor (mxLocalPane, UNO_QUERY);
            if (xNewAnchor.is())
                if (xResource->relocateToAnchor(xNewAnchor))
                    mpViewCache->push_back(rpDescriptor);
                else
                    bIsCacheable = false;
            else
                bIsCacheable = false;
        }
        else
        {
            bIsCacheable = false;
        }
    }

    if ( ! bIsCacheable)
    {
        // Shut down the current view shell.
        rpDescriptor->mpViewShell->Shutdown ();
        mpBase->GetDocShell()->Disconnect(rpDescriptor->mpViewShell.get());
        mpBase->GetViewShellManager()->DeactivateViewShell(rpDescriptor->mpViewShell.get());

        Reference<XComponent> xComponent (rpDescriptor->mxView, UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
    }
}

bool BasicViewFactory::IsCacheable (const std::shared_ptr<ViewDescriptor>& rpDescriptor)
{
    bool bIsCacheable (false);

    Reference<XRelocatableResource> xResource (rpDescriptor->mxView, UNO_QUERY);
    if (xResource.is())
    {
        static ::std::vector<Reference<XResourceId> > s_aCacheableResources;
        if (s_aCacheableResources.empty() )
        {
            std::shared_ptr<FrameworkHelper> pHelper (FrameworkHelper::Instance(*mpBase));

            // The slide sorter and the task panel are cacheable and relocatable.
            s_aCacheableResources.push_back(FrameworkHelper::CreateResourceId(
                FrameworkHelper::msSlideSorterURL, FrameworkHelper::msLeftDrawPaneURL));
            s_aCacheableResources.push_back(FrameworkHelper::CreateResourceId(
                FrameworkHelper::msSlideSorterURL, FrameworkHelper::msLeftImpressPaneURL));
        }

        ::std::vector<Reference<XResourceId> >::const_iterator iId;
        for (iId=s_aCacheableResources.begin(); iId!=s_aCacheableResources.end(); ++iId)
        {
            if ((*iId)->compareTo(rpDescriptor->mxViewId) == 0)
            {
                bIsCacheable = true;
                break;
            }
        }
    }

    return bIsCacheable;
}

std::shared_ptr<BasicViewFactory::ViewDescriptor> BasicViewFactory::GetViewFromCache (
    const Reference<XResourceId>& rxViewId,
    const Reference<XPane>& rxPane)
{
    std::shared_ptr<ViewDescriptor> pDescriptor;

    // Search for the requested view in the cache.
    ViewCache::iterator iEntry;
    for (iEntry=mpViewCache->begin(); iEntry!=mpViewCache->end(); ++iEntry)
    {
        if ((*iEntry)->mxViewId->compareTo(rxViewId) == 0)
        {
            pDescriptor = *iEntry;
            mpViewCache->erase(iEntry);
            break;
        }
    }

    // When the view has been found then relocate it to the given pane and
    // remove it from the cache.
    if (pDescriptor.get() != nullptr)
    {
        bool bRelocationSuccessfull (false);
        Reference<XRelocatableResource> xResource (pDescriptor->mxView, UNO_QUERY);
        if (xResource.is() && rxPane.is())
        {
            if (xResource->relocateToAnchor(rxPane))
                bRelocationSuccessfull = true;
        }

        if ( ! bRelocationSuccessfull)
        {
            ReleaseView(pDescriptor, true);
            pDescriptor.reset();
        }
    }

    return pDescriptor;
}

void BasicViewFactory::ActivateCenterView (
    const std::shared_ptr<ViewDescriptor>& rpDescriptor)
{
    mpBase->GetDocShell()->Connect(rpDescriptor->mpViewShell.get());

    // During the creation of the new sub-shell, resize requests were not
    // forwarded to it because it was not yet registered.  Therefore, we
    // have to request a resize now.
    rpDescriptor->mpViewShell->UIFeatureChanged();
    if (mpBase->GetDocShell()->IsInPlaceActive())
        mpBase->GetViewFrame()->Resize(true);

    mpBase->GetDrawController().SetSubController(
        rpDescriptor->mpViewShell->CreateSubController());
}

} } // end of namespace sd::framework


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_Draw_framework_BasicViewFactory_get_implementation(css::uno::XComponentContext* context,
                                                                     css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::framework::BasicViewFactory(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
