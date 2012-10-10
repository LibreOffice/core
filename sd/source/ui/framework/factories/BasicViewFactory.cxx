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
#include "taskpane/ToolPanelViewShell.hxx"
#include "PresentationViewShell.hxx"
#include "SlideSorterViewShell.hxx"
#include "FrameView.hxx"

#include <sfx2/viewfrm.hxx>
#include <vcl/wrkwin.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;
using ::sd::framework::FrameworkHelper;


namespace sd { namespace framework {


Reference<XInterface> SAL_CALL BasicViewFactory_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<XWeak*>(new BasicViewFactory(rxContext)));
}




::rtl::OUString BasicViewFactory_getImplementationName (void) throw(RuntimeException)
{
    return ::rtl::OUString("com.sun.star.comp.Draw.framework.BasicViewFactory");
}




Sequence<rtl::OUString> SAL_CALL BasicViewFactory_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const ::rtl::OUString sServiceName("com.sun.star.drawing.framework.BasicViewFactory");
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




//===== ViewDescriptor ========================================================

class BasicViewFactory::ViewDescriptor
{
public:
    Reference<XResource> mxView;
    ::boost::shared_ptr<sd::ViewShell> mpViewShell;
    ViewShellWrapper* mpWrapper;
    Reference<XResourceId> mxViewId;
    static bool CompareView (const ::boost::shared_ptr<ViewDescriptor>& rpDescriptor,
        const Reference<XResource>& rxView)
    { return rpDescriptor->mxView.get() == rxView.get(); }
};





//===== BasicViewFactory::ViewShellContainer ==================================

class BasicViewFactory::ViewShellContainer
    : public ::std::vector<boost::shared_ptr<ViewDescriptor> >
{
public:
    ViewShellContainer (void) {};
};


class BasicViewFactory::ViewCache
    : public ::std::vector<boost::shared_ptr<ViewDescriptor> >
{
public:
    ViewCache (void) {};
};




//===== ViewFactory ===========================================================

BasicViewFactory::BasicViewFactory (
    const Reference<XComponentContext>& rxContext)
    : BasicViewFactoryInterfaceBase(MutexOwner::maMutex),
      mxConfigurationController(),
      mpViewShellContainer(new ViewShellContainer()),
      mpBase(NULL),
      mpFrameView(NULL),
      mpWindow(new WorkWindow(NULL,WB_STDWORK)),
      mpViewCache(new ViewCache()),
      mxLocalPane(new Pane(Reference<XResourceId>(), mpWindow.get()))
{
    (void)rxContext;
}




BasicViewFactory::~BasicViewFactory (void)
{
}




void SAL_CALL BasicViewFactory::disposing (void)
{
    // Disconnect from the frame view.
    if (mpFrameView != NULL)
    {
        mpFrameView->Disconnect();
        mpFrameView = NULL;
    }

    // Relase the view cache.
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
    throw(RuntimeException, IllegalArgumentException, WrappedTargetException)
{
    Reference<XResource> xView;
    const bool bIsCenterPane (
        rxViewId->isBoundToURL(FrameworkHelper::msCenterPaneURL, AnchorBindingMode_DIRECT));

    // Get the pane for the anchor URL.
    Reference<XPane> xPane;
    if (mxConfigurationController.is())
        xPane = Reference<XPane>(mxConfigurationController->getResource(rxViewId->getAnchor()),
            UNO_QUERY);

    // For main views use the frame view of the last main view.
    ::sd::FrameView* pFrameView = NULL;
    if (xPane.is() && bIsCenterPane)
    {
        pFrameView = mpFrameView;
    }

    // Get Window pointer for XWindow of the pane.
    ::Window* pWindow = NULL;
    if (xPane.is())
        pWindow = VCLUnoHelper::GetWindow(xPane->getWindow());

    // Get the view frame.
    SfxViewFrame* pFrame = NULL;
    if (mpBase != NULL)
        pFrame = mpBase->GetViewFrame();

    if (pFrame != NULL && mpBase!=NULL && pWindow!=NULL)
    {
        // Try to get the view from the cache.
        ::boost::shared_ptr<ViewDescriptor> pDescriptor (GetViewFromCache(rxViewId, xPane));

        // When the requested view is not in the cache then create a new view.
        if (pDescriptor.get() == NULL)
        {
            pDescriptor = CreateView(rxViewId, *pFrame, *pWindow, xPane, pFrameView, bIsCenterPane);
        }

        if (pDescriptor.get() != NULL)
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
    throw(RuntimeException)
{
    if ( ! rxView.is())
        throw lang::IllegalArgumentException();

    if (rxView.is() && mpBase!=NULL)
    {
        ViewShellContainer::iterator iViewShell (
            ::std::find_if(
                mpViewShellContainer->begin(),
                mpViewShellContainer->end(),
                ::boost::bind(&ViewDescriptor::CompareView, _1, rxView)));
        if (iViewShell != mpViewShellContainer->end())
        {
            ::boost::shared_ptr<ViewShell> pViewShell ((*iViewShell)->mpViewShell);

            if ((*iViewShell)->mxViewId->isBoundToURL(
                FrameworkHelper::msCenterPaneURL, AnchorBindingMode_DIRECT))
            {
                // Obtain a pointer to and connect to the frame view of the
                // view.  The next view, that is created, will be
                // initialized with this frame view.
                if (mpFrameView == NULL)
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
                if (pSfxViewShell != NULL)
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
    throw (Exception, RuntimeException)
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
            if (pController != NULL)
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
            mxConfigurationController->addResourceFactory(FrameworkHelper::msTaskPaneURL, this);
            mxConfigurationController->addResourceFactory(FrameworkHelper::msSlideSorterURL, this);
        }
        catch (RuntimeException&)
        {
            mpBase = NULL;
            if (mxConfigurationController.is())
                mxConfigurationController->removeResourceFactoryForReference(this);
            throw;
        }
    }
}




::boost::shared_ptr<BasicViewFactory::ViewDescriptor> BasicViewFactory::CreateView (
    const Reference<XResourceId>& rxViewId,
    SfxViewFrame& rFrame,
    ::Window& rWindow,
    const Reference<XPane>& rxPane,
    FrameView* pFrameView,
    const bool bIsCenterPane)
{
    ::boost::shared_ptr<ViewDescriptor> pDescriptor (new ViewDescriptor());

    pDescriptor->mpViewShell = CreateViewShell(
        rxViewId,
        rFrame,
        rWindow,
        pFrameView,
        bIsCenterPane);
    pDescriptor->mxViewId = rxViewId;

    if (pDescriptor->mpViewShell.get() != NULL)
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
            if (pDescriptor->mpViewShell != NULL)
            {
                pDescriptor->mpViewShell->Resize();
            }
        }

        pDescriptor->mxView.set( pDescriptor->mpWrapper->queryInterface( XResource::static_type() ), UNO_QUERY_THROW );
    }

    return pDescriptor;
}




::boost::shared_ptr<ViewShell> BasicViewFactory::CreateViewShell (
    const Reference<XResourceId>& rxViewId,
    SfxViewFrame& rFrame,
    ::Window& rWindow,
    FrameView* pFrameView,
    const bool bIsCenterPane)
{
    ::boost::shared_ptr<ViewShell> pViewShell;
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
    }
    else if (rsViewURL.equals(FrameworkHelper::msDrawViewURL))
    {
        pViewShell.reset(
            new GraphicViewShell (
                &rFrame,
                *mpBase,
                &rWindow,
                pFrameView));
    }
    else if (rsViewURL.equals(FrameworkHelper::msOutlineViewURL))
    {
        pViewShell.reset(
            new OutlineViewShell (
                &rFrame,
                *mpBase,
                &rWindow,
                pFrameView));
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
    }
    else if (rsViewURL.equals(FrameworkHelper::msPresentationViewURL))
    {
        pViewShell.reset(
            new PresentationViewShell(
                &rFrame,
                *mpBase,
                &rWindow,
                pFrameView));
    }
    else if (rsViewURL.equals(FrameworkHelper::msTaskPaneURL))
    {
        pViewShell.reset(
            new ::sd::toolpanel::ToolPanelViewShell(
                &rFrame,
                *mpBase,
                &rWindow,
                pFrameView));
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
    const ::boost::shared_ptr<ViewDescriptor>& rpDescriptor,
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




bool BasicViewFactory::IsCacheable (const ::boost::shared_ptr<ViewDescriptor>& rpDescriptor)
{
    bool bIsCacheable (false);

    Reference<XRelocatableResource> xResource (rpDescriptor->mxView, UNO_QUERY);
    if (xResource.is())
    {
        static ::std::vector<Reference<XResourceId> > maCacheableResources;
        if (maCacheableResources.empty() )
        {
            ::boost::shared_ptr<FrameworkHelper> pHelper (FrameworkHelper::Instance(*mpBase));

            // The slide sorter and the task panel are cacheable and relocatable.
            maCacheableResources.push_back(pHelper->CreateResourceId(
                FrameworkHelper::msSlideSorterURL, FrameworkHelper::msLeftDrawPaneURL));
            maCacheableResources.push_back(pHelper->CreateResourceId(
                FrameworkHelper::msSlideSorterURL, FrameworkHelper::msLeftImpressPaneURL));
            maCacheableResources.push_back(pHelper->CreateResourceId(
                FrameworkHelper::msTaskPaneURL, FrameworkHelper::msRightPaneURL));
        }

        ::std::vector<Reference<XResourceId> >::const_iterator iId;
        for (iId=maCacheableResources.begin(); iId!=maCacheableResources.end(); ++iId)
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




::boost::shared_ptr<BasicViewFactory::ViewDescriptor> BasicViewFactory::GetViewFromCache (
    const Reference<XResourceId>& rxViewId,
    const Reference<XPane>& rxPane)
{
    ::boost::shared_ptr<ViewDescriptor> pDescriptor;

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
    if (pDescriptor.get() != NULL)
    {
        bool bRelocationSuccessfull (false);
        Reference<XRelocatableResource> xResource (pDescriptor->mxView, UNO_QUERY);
        Reference<XResource> xNewAnchor (rxPane, UNO_QUERY);
        if (xResource.is() && xNewAnchor.is())
        {
            if (xResource->relocateToAnchor(xNewAnchor))
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
    const ::boost::shared_ptr<ViewDescriptor>& rpDescriptor)
{
    mpBase->GetDocShell()->Connect(rpDescriptor->mpViewShell.get());

    // During the creation of the new sub-shell, resize requests were not
    // forwarded to it because it was not yet registered.  Therefore, we
    // have to request a resize now.
    rpDescriptor->mpViewShell->UIFeatureChanged();
    if (mpBase->GetDocShell()->IsInPlaceActive())
        mpBase->GetViewFrame()->Resize(sal_True);

    mpBase->GetDrawController().SetSubController(
        rpDescriptor->mpViewShell->CreateSubController());
}

} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
