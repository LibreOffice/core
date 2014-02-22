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


#include "SlsListener.hxx"

#include "SlideSorter.hxx"
#include "SlideSorterViewShell.hxx"
#include "ViewShellHint.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "controller/SlsSelectionObserver.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "view/SlideSorterView.hxx"
#include "cache/SlsPageCache.hxx"
#include "cache/SlsPageCacheManager.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"

#include "glob.hrc"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"
#include "FrameView.hxx"
#include "EventMultiplexer.hxx"
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <sfx2/viewfrm.hxx>
#include <tools/diagnose_ex.h>


using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

namespace sd { namespace slidesorter { namespace controller {


Listener::Listener (
    SlideSorter& rSlideSorter)
    : ListenerInterfaceBase(maMutex),
      mrSlideSorter(rSlideSorter),
      mrController(mrSlideSorter.GetController()),
      mpBase(mrSlideSorter.GetViewShellBase()),
      mbListeningToDocument (false),
      mbListeningToUNODocument (false),
      mbListeningToController (false),
      mbListeningToFrame (false),
      mbIsMainViewChangePending(false),
      mxControllerWeak(),
      mxFrameWeak(),
      mpModelChangeLock()
{
    StartListening(*mrSlideSorter.GetModel().GetDocument());
    StartListening(*mrSlideSorter.GetModel().GetDocument()->GetDocSh());
    mbListeningToDocument = true;

    
    Reference<document::XEventBroadcaster> xBroadcaster (
        mrSlideSorter.GetModel().GetDocument()->getUnoModel(), uno::UNO_QUERY);
    if (xBroadcaster.is())
    {
        xBroadcaster->addEventListener (this);
        mbListeningToUNODocument = true;
    }

    
    Reference<XComponent> xComponent (xBroadcaster, UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener (
            Reference<lang::XEventListener>(
                static_cast<XWeak*>(this), UNO_QUERY));

    
    bool bIsMainViewShell (false);
    ViewShell* pViewShell = mrSlideSorter.GetViewShell();
    if (pViewShell != NULL)
        bIsMainViewShell = pViewShell->IsMainViewShell();
    if ( ! bIsMainViewShell)
    {
        
        Reference<frame::XFrame> xFrame;
        Reference<frame::XController> xController (mrSlideSorter.GetXController());
        if (xController.is())
            xFrame = xController->getFrame();
        mxFrameWeak = xFrame;
        if (xFrame.is())
        {
            xFrame->addFrameActionListener (
                Reference<frame::XFrameActionListener>(
                    static_cast<XWeak*>(this), UNO_QUERY));
            mbListeningToFrame = true;
        }

        
        ConnectToController ();
    }

    
    
    if (mpBase != NULL)
    {
        ViewShell* pMainViewShell = mpBase->GetMainViewShell().get();
        if (pMainViewShell != NULL
            && pMainViewShell!=pViewShell)
        {
            StartListening(*pMainViewShell);
        }

        Link aLink (LINK(this, Listener, EventMultiplexerCallback));
        mpBase->GetEventMultiplexer()->AddEventListener(
            aLink,
            tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED
            | tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED
            | tools::EventMultiplexerEvent::EID_CONTROLLER_ATTACHED
            | tools::EventMultiplexerEvent::EID_CONTROLLER_DETACHED
            | tools::EventMultiplexerEvent::EID_CONFIGURATION_UPDATED);
    }
}




Listener::~Listener (void)
{
    DBG_ASSERT( !mbListeningToDocument && !mbListeningToUNODocument && !mbListeningToFrame,
        "sd::Listener::~Listener(), disposing() was not called, ask DBO!" );
}




void Listener::ReleaseListeners (void)
{
    if (mbListeningToDocument)
    {
        EndListening(*mrSlideSorter.GetModel().GetDocument()->GetDocSh());
        EndListening(*mrSlideSorter.GetModel().GetDocument());
        mbListeningToDocument = false;
    }

    if (mbListeningToUNODocument)
    {
        Reference<document::XEventBroadcaster> xBroadcaster (
            mrSlideSorter.GetModel().GetDocument()->getUnoModel(), UNO_QUERY);
        if (xBroadcaster.is())
            xBroadcaster->removeEventListener (this);

        
        Reference<XComponent> xComponent (xBroadcaster, UNO_QUERY);
        if (xComponent.is())
            xComponent->removeEventListener (
                Reference<lang::XEventListener>(
                    static_cast<XWeak*>(this), UNO_QUERY));

        mbListeningToUNODocument = false;
    }

    if (mbListeningToFrame)
    {
        
        Reference<frame::XFrame> xFrame (mxFrameWeak);
        if (xFrame.is())
        {
            xFrame->removeFrameActionListener (
                Reference<frame::XFrameActionListener>(
                    static_cast<XWeak*>(this), UNO_QUERY));
            mbListeningToFrame = false;
        }
    }

    DisconnectFromController ();

    if (mpBase != NULL)
    {
        Link aLink (LINK(this, Listener, EventMultiplexerCallback));
        mpBase->GetEventMultiplexer()->RemoveEventListener(
            aLink,
            tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED
            | tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED
            | tools::EventMultiplexerEvent::EID_CONTROLLER_ATTACHED
            | tools::EventMultiplexerEvent::EID_CONTROLLER_DETACHED
            | tools::EventMultiplexerEvent::EID_CONFIGURATION_UPDATED);
    }
}




void Listener::ConnectToController (void)
{
    ViewShell* pShell = mrSlideSorter.GetViewShell();

    
    
    if (pShell==NULL || ! pShell->IsMainViewShell())
    {
        Reference<frame::XController> xController (mrSlideSorter.GetXController());

        
        Reference<beans::XPropertySet> xSet (xController, UNO_QUERY);
        if (xSet.is())
        {
            try
            {
                xSet->addPropertyChangeListener(OUString("CurrentPage"), this);
            }
            catch (beans::UnknownPropertyException&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            try
            {
                xSet->addPropertyChangeListener(OUString("IsMasterPageMode"), this);
            }
            catch (beans::UnknownPropertyException&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        
        Reference<XComponent> xComponent (xController, UNO_QUERY);
        if (xComponent.is())
        {
            xComponent->addEventListener (
                Reference<lang::XEventListener>(static_cast<XWeak*>(this), UNO_QUERY));

            mxControllerWeak = xController;
            mbListeningToController = true;
        }
    }
}




void Listener::DisconnectFromController (void)
{
    if (mbListeningToController)
    {
        Reference<frame::XController> xController = mxControllerWeak;
        Reference<beans::XPropertySet> xSet (xController, UNO_QUERY);
        try
        {
            
            if (xSet.is())
            {
                xSet->removePropertyChangeListener (
                    OUString("CurrentPage"),
                    this);
                xSet->removePropertyChangeListener (
                    OUString("IsMasterPageMode"),
                    this);
            }

            
            Reference<XComponent> xComponent (xController, UNO_QUERY);
            if (xComponent.is())
                xComponent->removeEventListener (
                    Reference<lang::XEventListener>(
                        static_cast<XWeak*>(this), UNO_QUERY));
        }
        catch (beans::UnknownPropertyException&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        mbListeningToController = false;
        mxControllerWeak = Reference<frame::XController>();
    }
}




void Listener::Notify (
    SfxBroadcaster& rBroadcaster,
    const SfxHint& rHint)
{
    if (rHint.ISA(SdrHint))
    {
        SdrHint& rSdrHint (*PTR_CAST(SdrHint,&rHint));
        switch (rSdrHint.GetKind())
        {
            case HINT_MODELCLEARED:
                if (&rBroadcaster == mrSlideSorter.GetModel().GetDocument())
                {   
                    EndListening(rBroadcaster);
                    return;
                }
                break;
            case HINT_PAGEORDERCHG:
                if (&rBroadcaster == mrSlideSorter.GetModel().GetDocument())
                    HandleModelChange(rSdrHint.GetPage());
                break;

            default:
                break;
        }
    }
    else if (rHint.ISA(ViewShellHint))
    {
        ViewShellHint& rViewShellHint (*PTR_CAST(ViewShellHint,&rHint));
        switch (rViewShellHint.GetHintId())
        {
            case ViewShellHint::HINT_PAGE_RESIZE_START:
                
                
                mpModelChangeLock.reset(new SlideSorterController::ModelChangeLock(mrController));
                mrController.HandleModelChange();
                break;

            case ViewShellHint::HINT_PAGE_RESIZE_END:
                
                mpModelChangeLock.reset();
                break;

            case ViewShellHint::HINT_CHANGE_EDIT_MODE_START:
                mrController.PrepareEditModeChange();
                break;

            case ViewShellHint::HINT_CHANGE_EDIT_MODE_END:
                mrController.FinishEditModeChange();
                break;

            case ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_START:
                mpModelChangeLock.reset(new SlideSorterController::ModelChangeLock(mrController));
                break;

            case ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_END:
                mpModelChangeLock.reset();
                break;
        }
    }
    else if (rHint.ISA(SfxSimpleHint))
    {
        SfxSimpleHint& rSfxSimpleHint (*PTR_CAST(SfxSimpleHint,&rHint));
        switch (rSfxSimpleHint.GetId())
        {
            case SFX_HINT_DOCCHANGED:
                mrController.CheckForMasterPageAssignment();
                mrController.CheckForSlideTransitionAssignment();
                break;
        }
    }
}




IMPL_LINK(Listener, EventMultiplexerCallback, ::sd::tools::EventMultiplexerEvent*, pEvent)
{
    switch (pEvent->meEventId)
    {
        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED:
        {
            if (mpBase != NULL)
            {
                ViewShell* pMainViewShell = mpBase->GetMainViewShell().get();
                if (pMainViewShell != NULL)
                    EndListening(*pMainViewShell);
            }
        }
        break;


        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED:
            mbIsMainViewChangePending = true;
            break;

        case tools::EventMultiplexerEvent::EID_CONFIGURATION_UPDATED:
            if (mbIsMainViewChangePending && mpBase != NULL)
            {
                mbIsMainViewChangePending = false;
                ViewShell* pMainViewShell = mpBase->GetMainViewShell().get();
                if (pMainViewShell != NULL
                    && pMainViewShell!=mrSlideSorter.GetViewShell())
                {
                    StartListening (*pMainViewShell);
                }
            }
            break;

        case tools::EventMultiplexerEvent::EID_CONTROLLER_ATTACHED:
        {
            ConnectToController();
            
            UpdateEditMode();
        }
        break;


        case tools::EventMultiplexerEvent::EID_CONTROLLER_DETACHED:
            DisconnectFromController();
            break;

        case tools::EventMultiplexerEvent::EID_SHAPE_CHANGED:
        case tools::EventMultiplexerEvent::EID_SHAPE_INSERTED:
        case tools::EventMultiplexerEvent::EID_SHAPE_REMOVED:
            HandleShapeModification(static_cast<const SdrPage*>(pEvent->mpUserData));
            break;

        case tools::EventMultiplexerEvent::EID_END_TEXT_EDIT:
            if (pEvent->mpUserData != NULL)
            {
                const SdrObject* pObject = static_cast<const SdrObject*>(pEvent->mpUserData);
                HandleShapeModification(pObject->GetPage());
            }
            break;

        default:
            break;
    }

    return 0;
}






void SAL_CALL Listener::disposing (
    const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    if ((mbListeningToDocument || mbListeningToUNODocument)
        && mrSlideSorter.GetModel().GetDocument()!=NULL
        && rEventObject.Source
           == mrSlideSorter.GetModel().GetDocument()->getUnoModel())
    {
        mbListeningToDocument = false;
        mbListeningToUNODocument = false;
    }
    else if (mbListeningToController)
    {
        Reference<frame::XController> xController (mxControllerWeak);
        if (rEventObject.Source == xController)
        {
            mbListeningToController = false;
        }
    }
}






void SAL_CALL Listener::notifyEvent (
    const document::EventObject& )
    throw (RuntimeException)
{
}






void SAL_CALL Listener::propertyChange (
    const PropertyChangeEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    static const OUString sCurrentPagePropertyName ("CurrentPage");
    static const OUString sEditModePropertyName ("IsMasterPageMode");

    if (rEvent.PropertyName.equals(sCurrentPagePropertyName))
    {
        Any aCurrentPage = rEvent.NewValue;
        Reference<beans::XPropertySet> xPageSet (aCurrentPage, UNO_QUERY);
        if (xPageSet.is())
        {
            try
            {
                Any aPageNumber = xPageSet->getPropertyValue ("Number");
                sal_Int32 nCurrentPage = 0;
                aPageNumber >>= nCurrentPage;
                
                
                
                
                mrController.GetCurrentSlideManager()->NotifyCurrentSlideChange(nCurrentPage-1);
                mrController.GetPageSelector().SelectPage(nCurrentPage-1);
            }
            catch (beans::UnknownPropertyException&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            catch (lang::DisposedException&)
            {
                
                
            }
        }
    }
    else if (rEvent.PropertyName.equals (sEditModePropertyName))
    {
        sal_Bool bIsMasterPageMode = sal_False;
        rEvent.NewValue >>= bIsMasterPageMode;
        mrController.ChangeEditMode (
            bIsMasterPageMode ? EM_MASTERPAGE : EM_PAGE);
    }
}






void SAL_CALL Listener::frameAction (const frame::FrameActionEvent& rEvent)
    throw (::com::sun::star::uno::RuntimeException)
{
    switch (rEvent.Action)
    {
        case frame::FrameAction_COMPONENT_DETACHING:
            DisconnectFromController();
            break;

        case frame::FrameAction_COMPONENT_REATTACHED:
        {
            ConnectToController();
            mrController.GetPageSelector().GetCoreSelection();
            UpdateEditMode();
        }
        break;

        default:
            break;
    }
}






void SAL_CALL Listener::notifyEvent (
    const AccessibleEventObject& )
    throw (RuntimeException)
{
}




void SAL_CALL Listener::disposing (void)
{
    ReleaseListeners();
}




void Listener::UpdateEditMode (void)
{
    
    
    Reference<frame::XController> xController (mxControllerWeak);
    Reference<beans::XPropertySet> xSet (xController, UNO_QUERY);
    bool bIsMasterPageMode = false;
    if (xSet != NULL)
    {
        try
        {
            Any aValue (xSet->getPropertyValue( "IsMasterPageMode" ));
            aValue >>= bIsMasterPageMode;
        }
        catch (beans::UnknownPropertyException&)
        {
            
            
            bIsMasterPageMode = false;
        }
    }
    mrController.ChangeEditMode (
        bIsMasterPageMode ? EM_MASTERPAGE : EM_PAGE);
}



void Listener::HandleModelChange (const SdrPage* pPage)
{
    
    
    
    if (mrSlideSorter.GetModel().NotifyPageEvent(pPage))
    {
        

        
        
        if (pPage!=NULL && ! pPage->IsInserted())
            cache::PageCacheManager::Instance()->ReleasePreviewBitmap(pPage);

        mrController.GetSelectionManager()->GetSelectionObserver()->NotifyPageEvent(pPage);
    }

    
    
    SdDrawDocument* pDocument (mrSlideSorter.GetModel().GetDocument());
    if (pDocument != NULL
        && pDocument->GetMasterSdPageCount(PK_STANDARD) == pDocument->GetMasterSdPageCount(PK_NOTES))
    {
        
        
        
        
        cache::PageCacheManager::Instance()->InvalidateAllPreviewBitmaps(pDocument->getUnoModel());

        mrController.HandleModelChange();
    }
}



void Listener::HandleShapeModification (const SdrPage* pPage)
{
    if (pPage == NULL)
        return;

    
    
    ::boost::shared_ptr<cache::PageCacheManager> pCacheManager (cache::PageCacheManager::Instance());
    if ( ! pCacheManager)
        return;
    SdDrawDocument* pDocument = mrSlideSorter.GetModel().GetDocument();
    if (pDocument == NULL)
    {
        OSL_ASSERT(pDocument!=NULL);
        return;
    }
    pCacheManager->InvalidatePreviewBitmap(pDocument->getUnoModel(), pPage);
    mrSlideSorter.GetView().GetPreviewCache()->RequestPreviewBitmap(pPage);

    
    
    if (pPage->IsMasterPage())
    {
        for (sal_uInt16 nIndex=0,nCount=pDocument->GetSdPageCount(PK_STANDARD);
             nIndex<nCount;
             ++nIndex)
        {
            const SdPage* pCandidate = pDocument->GetSdPage(nIndex, PK_STANDARD);
            if (pCandidate!=NULL && pCandidate->TRG_HasMasterPage())
            {
                if (&pCandidate->TRG_GetMasterPage() == pPage)
                    pCacheManager->InvalidatePreviewBitmap(pDocument->getUnoModel(), pCandidate);
            }
            else
            {
                OSL_ASSERT(pCandidate!=NULL && pCandidate->TRG_HasMasterPage());
            }
        }
    }
}




void Listener::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException ("SlideSorterController object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }
}




} } } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
