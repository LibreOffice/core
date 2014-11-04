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

    // Connect to the UNO document.
    Reference<document::XEventBroadcaster> xBroadcaster (
        mrSlideSorter.GetModel().GetDocument()->getUnoModel(), uno::UNO_QUERY);
    if (xBroadcaster.is())
    {
        xBroadcaster->addEventListener (this);
        mbListeningToUNODocument = true;
    }

    // Listen for disposing events from the document.
    Reference<XComponent> xComponent (xBroadcaster, UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener (
            Reference<lang::XEventListener>(
                static_cast<XWeak*>(this), UNO_QUERY));

    // Connect to the frame to listen for controllers being exchanged.
    bool bIsMainViewShell (false);
    ViewShell* pViewShell = mrSlideSorter.GetViewShell();
    if (pViewShell != NULL)
        bIsMainViewShell = pViewShell->IsMainViewShell();
    if ( ! bIsMainViewShell)
    {
        // Listen to changes of certain properties.
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

        // Connect to the current controller.
        ConnectToController ();
    }

    // Listen for hints of the MainViewShell as well.  If that is not yet
    // present then the EventMultiplexer will tell us when it is available.
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

        // Remove the dispose listener.
        Reference<XComponent> xComponent (xBroadcaster, UNO_QUERY);
        if (xComponent.is())
            xComponent->removeEventListener (
                Reference<lang::XEventListener>(
                    static_cast<XWeak*>(this), UNO_QUERY));

        mbListeningToUNODocument = false;
    }

    if (mbListeningToFrame)
    {
        // Listen to changes of certain properties.
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

    // Register at the controller of the main view shell (if we are that not
    // ourself).
    if (pShell==NULL || ! pShell->IsMainViewShell())
    {
        Reference<frame::XController> xController (mrSlideSorter.GetXController());

        // Listen to changes of certain properties.
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

        // Listen for disposing events.
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
            // Remove the property listener.
            if (xSet.is())
            {
                xSet->removePropertyChangeListener (
                    OUString("CurrentPage"),
                    this);
                xSet->removePropertyChangeListener (
                    OUString("IsMasterPageMode"),
                    this);
            }

            // Remove the dispose listener.
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
    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint);
    if (pSdrHint)
    {
        switch (pSdrHint->GetKind())
        {
            case HINT_MODELCLEARED:
                if (&rBroadcaster == mrSlideSorter.GetModel().GetDocument())
                {   // rhbz#965646 stop listening to dying document
                    EndListening(rBroadcaster);
                    return;
                }
                break;
            case HINT_PAGEORDERCHG:
                if (&rBroadcaster == mrSlideSorter.GetModel().GetDocument())
                    HandleModelChange(pSdrHint->GetPage());
                break;

            default:
                break;
        }
    }
    else if (dynamic_cast<const ViewShellHint*>(&rHint))
    {
        const ViewShellHint& rViewShellHint = static_cast<const ViewShellHint&>(rHint);
        switch (rViewShellHint.GetHintId())
        {
            case ViewShellHint::HINT_PAGE_RESIZE_START:
                // Initiate a model change but do nothing (well, not much)
                // until we are told that all slides have been resized.
                mpModelChangeLock.reset(new SlideSorterController::ModelChangeLock(mrController));
                mrController.HandleModelChange();
                break;

            case ViewShellHint::HINT_PAGE_RESIZE_END:
                // All slides have been resized.  The model has to be updated.
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
    else if (dynamic_cast<const SfxSimpleHint*>(&rHint))
    {
        const SfxSimpleHint& rSfxSimpleHint = static_cast<const SfxSimpleHint&>(rHint);
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
            //            mrController.GetPageSelector().GetCoreSelection();
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

//=====  lang::XEventListener  ================================================

void SAL_CALL Listener::disposing (
    const lang::EventObject& rEventObject)
    throw (RuntimeException, std::exception)
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

//=====  document::XEventListener  ============================================

void SAL_CALL Listener::notifyEvent (
    const document::EventObject& )
    throw (RuntimeException, std::exception)
{
}

//=====  beans::XPropertySetListener  =========================================

void SAL_CALL Listener::propertyChange (
    const PropertyChangeEvent& rEvent)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    static const char sCurrentPagePropertyName[] = "CurrentPage";
    static const char sEditModePropertyName[] = "IsMasterPageMode";

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
                // The selection is already set but we call SelectPage()
                // nevertheless in order to make the new current page the
                // last recently selected page of the PageSelector.  This is
                // used when making the selection visible.
                mrController.GetCurrentSlideManager()->NotifyCurrentSlideChange(nCurrentPage-1);
                mrController.GetPageSelector().SelectPage(nCurrentPage-1);
            }
            catch (beans::UnknownPropertyException&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            catch (lang::DisposedException&)
            {
                // Something is already disposed.  There is not much we can
                // do, except not to crash.
            }
        }
    }
    else if (rEvent.PropertyName.equals (sEditModePropertyName))
    {
        bool bIsMasterPageMode = false;
        rEvent.NewValue >>= bIsMasterPageMode;
        mrController.ChangeEditMode (
            bIsMasterPageMode ? EM_MASTERPAGE : EM_PAGE);
    }
}

//===== frame::XFrameActionListener  ==========================================

void SAL_CALL Listener::frameAction (const frame::FrameActionEvent& rEvent)
    throw (::com::sun::star::uno::RuntimeException, std::exception)
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

//===== accessibility::XAccessibleEventListener  ==============================

void SAL_CALL Listener::notifyEvent (
    const AccessibleEventObject& )
    throw (RuntimeException, std::exception)
{
}

void SAL_CALL Listener::disposing (void)
{
    ReleaseListeners();
}

void Listener::UpdateEditMode (void)
{
    // When there is a new controller then the edit mode may have changed at
    // the same time.
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
            // When the property is not supported then the master page mode
            // is not supported, too.
            bIsMasterPageMode = false;
        }
    }
    mrController.ChangeEditMode (
        bIsMasterPageMode ? EM_MASTERPAGE : EM_PAGE);
}

void Listener::HandleModelChange (const SdrPage* pPage)
{
    // Notify model and selection observer about the page.  The return value
    // of the model call acts as filter as to which events to pass to the
    // selection observer.
    if (mrSlideSorter.GetModel().NotifyPageEvent(pPage))
    {
        // The page of the hint belongs (or belonged) to the model.

        // Tell the cache manager that the preview bitmaps for a deleted
        // page can be removed from all caches.
        if (pPage!=NULL && ! pPage->IsInserted())
            cache::PageCacheManager::Instance()->ReleasePreviewBitmap(pPage);

        mrController.GetSelectionManager()->GetSelectionObserver()->NotifyPageEvent(pPage);
    }

    // Tell the controller about the model change only when the document is
    // in a sane state, not just in the middle of a larger change.
    SdDrawDocument* pDocument (mrSlideSorter.GetModel().GetDocument());
    if (pDocument != NULL
        && pDocument->GetMasterSdPageCount(PK_STANDARD) == pDocument->GetMasterSdPageCount(PK_NOTES))
    {
        // A model change can make updates of some text fields necessary
        // (like page numbers and page count.)  Invalidate all previews in
        // the cache to cope with this.  Doing this on demand would be a
        // nice optimization.
        cache::PageCacheManager::Instance()->InvalidateAllPreviewBitmaps(pDocument->getUnoModel());

        mrController.HandleModelChange();
    }
}

void Listener::HandleShapeModification (const SdrPage* pPage)
{
    if (pPage == NULL)
        return;

    // Invalidate the preview of the page (in all slide sorters that display
    // it.)
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

    // When the page is a master page then invalidate the previews of all
    // pages that are linked to this master page.
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

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
