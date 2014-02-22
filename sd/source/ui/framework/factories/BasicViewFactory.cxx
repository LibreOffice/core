/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

#include <sfx2/viewfrm.hxx>
#include <vcl/wrkwin.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;


namespace sd { namespace framework {


Reference<XInterface> SAL_CALL BasicViewFactory_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<XWeak*>(new BasicViewFactory(rxContext)));
}




OUString BasicViewFactory_getImplementationName (void) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.Draw.framework.BasicViewFactory");
}




Sequence<OUString> SAL_CALL BasicViewFactory_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const OUString sServiceName("com.sun.star.drawing.framework.BasicViewFactory");
    return Sequence<OUString>(&sServiceName, 1);
}






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
    
    if (mpFrameView != NULL)
    {
        mpFrameView->Disconnect();
        mpFrameView = NULL;
    }

    
    ViewShellContainer::const_iterator iView;
    for (iView=mpViewCache->begin(); iView!=mpViewCache->end(); ++iView)
    {
        ReleaseView(*iView, true);
    }

    
    
    
    
    
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

    
    Reference<XPane> xPane;
    if (mxConfigurationController.is())
        xPane = Reference<XPane>(mxConfigurationController->getResource(rxViewId->getAnchor()),
            UNO_QUERY);

    
    ::sd::FrameView* pFrameView = NULL;
    if (xPane.is() && bIsCenterPane)
    {
        pFrameView = mpFrameView;
    }

    
    ::Window* pWindow = NULL;
    if (xPane.is())
        pWindow = VCLUnoHelper::GetWindow(xPane->getWindow());

    
    SfxViewFrame* pFrame = NULL;
    if (mpBase != NULL)
        pFrame = mpBase->GetViewFrame();

    if (pFrame != NULL && mpBase!=NULL && pWindow!=NULL)
    {
        
        ::boost::shared_ptr<ViewDescriptor> pDescriptor (GetViewFromCache(rxViewId, xPane));

        
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
                
                
                
                if (mpFrameView == NULL)
                {
                    mpFrameView = pViewShell->GetFrameView();
                    if (mpFrameView)
                        mpFrameView->Connect();
                }

                
                
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
            
            Reference<frame::XController> xController (aArguments[0], UNO_QUERY_THROW);

            
            Reference<lang::XUnoTunnel> xTunnel (xController, UNO_QUERY_THROW);
            ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
                xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
            if (pController != NULL)
                mpBase = pController->GetViewShellBase();

            
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

        
        if (xWindow.is())
        {
            xWindow->addWindowListener(pDescriptor->mpWrapper);
            if (pDescriptor->mpViewShell != 0)
            {
                pDescriptor->mpViewShell->Resize();
            }
        }

        pDescriptor->mxView.set( pDescriptor->mpWrapper->queryInterface( cppu::UnoType<XResource>::get() ), UNO_QUERY_THROW );
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

            
            maCacheableResources.push_back(pHelper->CreateResourceId(
                FrameworkHelper::msSlideSorterURL, FrameworkHelper::msLeftDrawPaneURL));
            maCacheableResources.push_back(pHelper->CreateResourceId(
                FrameworkHelper::msSlideSorterURL, FrameworkHelper::msLeftImpressPaneURL));
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

    
    
    if (pDescriptor.get() != NULL)
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
    const ::boost::shared_ptr<ViewDescriptor>& rpDescriptor)
{
    mpBase->GetDocShell()->Connect(rpDescriptor->mpViewShell.get());

    
    
    
    rpDescriptor->mpViewShell->UIFeatureChanged();
    if (mpBase->GetDocShell()->IsInPlaceActive())
        mpBase->GetViewFrame()->Resize(sal_True);

    mpBase->GetDrawController().SetSubController(
        rpDescriptor->mpViewShell->CreateSubController());
}

} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
