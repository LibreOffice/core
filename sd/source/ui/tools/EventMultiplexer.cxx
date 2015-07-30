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

#include "EventMultiplexer.hxx"

#include "MutexOwner.hxx"
#include "ViewShellBase.hxx"
#include "drawdoc.hxx"
#include "DrawController.hxx"
#include "SlideSorterViewShell.hxx"
#include "framework/FrameworkHelper.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/compbase4.hxx>
#include <sfx2/viewfrm.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::sd::framework::FrameworkHelper;

class SdDrawDocument;

namespace {
static const sal_Int32 ResourceActivationEvent = 0;
static const sal_Int32 ResourceDeactivationEvent = 1;
static const sal_Int32 ConfigurationUpdateEvent = 2;
}

namespace sd { namespace tools {

typedef cppu::WeakComponentImplHelper4<
      ::com::sun::star::beans::XPropertyChangeListener,
      ::com::sun::star::frame::XFrameActionListener,
      ::com::sun::star::view::XSelectionChangeListener,
      ::com::sun::star::drawing::framework::XConfigurationChangeListener
    > EventMultiplexerImplementationInterfaceBase;

class EventMultiplexer::Implementation
    : protected MutexOwner,
      public EventMultiplexerImplementationInterfaceBase,
      public SfxListener
{
public:
    Implementation (ViewShellBase& rBase);
    virtual ~Implementation();

    void AddEventListener (
        Link<>& rCallback,
        EventMultiplexerEvent::EventId aEventTypes);

    void RemoveEventListener (
        Link<>& rCallback,
        EventMultiplexerEvent::EventId aEventTypes);

    void CallListeners (EventMultiplexerEvent& rEvent);

    ViewShellBase& GetViewShellBase() const { return mrBase; }

    //===== lang::XEventListener ==============================================
    virtual void SAL_CALL
        disposing (const ::com::sun::star::lang::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //===== beans::XPropertySetListener =======================================
    virtual void SAL_CALL
        propertyChange (
            const com::sun::star::beans::PropertyChangeEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //===== view::XSelectionChangeListener ====================================
    virtual void SAL_CALL
        selectionChanged (
            const com::sun::star::lang::EventObject& rEvent)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //===== frame::XFrameActionListener  ======================================
    /** For certain actions the listener connects to a new controller of the
        frame it is listening to.  This usually happens when the view shell
        in the center pane is replaced by another view shell.
    */
    virtual void SAL_CALL
        frameAction (const ::com::sun::star::frame::FrameActionEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //===== drawing::framework::XConfigurationChangeListener ==================
    virtual void SAL_CALL
        notifyConfigurationChange (
            const ::com::sun::star::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL disposing() SAL_OVERRIDE;

protected:
    virtual void Notify (
        SfxBroadcaster& rBroadcaster,
        const SfxHint& rHint) SAL_OVERRIDE;

private:
    ViewShellBase& mrBase;
    typedef ::std::pair<Link<>,EventMultiplexerEvent::EventId> ListenerDescriptor;
    typedef ::std::vector<ListenerDescriptor> ListenerList;
    ListenerList maListeners;

    /// Remember whether we are listening to the UNO controller.
    bool mbListeningToController;
    /// Remember whether we are listening to the frame.
    bool mbListeningToFrame;

    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::frame::XController> mxControllerWeak;
    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::frame::XFrame> mxFrameWeak;
    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::view::XSelectionSupplier> mxSlideSorterSelectionWeak;
    SdDrawDocument* mpDocument;
    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::drawing::framework::XConfigurationController>
        mxConfigurationControllerWeak;

    void ReleaseListeners();

    void ConnectToController();
    void DisconnectFromController();

    void CallListeners (
        EventMultiplexerEvent::EventId eId,
        void* pUserData = NULL);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed()
        throw (::com::sun::star::lang::DisposedException);

    DECL_LINK(SlideSorterSelectionChangeListener, void*);
};

const char aCurrentPagePropertyName[] = "CurrentPage";
const char aEditModePropertyName[] = "IsMasterPageMode";

//===== EventMultiplexer ======================================================

EventMultiplexer::EventMultiplexer (ViewShellBase& rBase)
    : mpImpl (new EventMultiplexer::Implementation(rBase))
{
    mpImpl->acquire();
}

EventMultiplexer::~EventMultiplexer()
{
    try
    {
        mpImpl->dispose();
    }
    catch (const RuntimeException&)
    {
    }
    catch (const Exception&)
    {
    }
}

void EventMultiplexer::AddEventListener (
    Link<>& rCallback,
    EventMultiplexerEvent::EventId aEventTypes)
{
    mpImpl->AddEventListener (rCallback, aEventTypes);
}

void EventMultiplexer::RemoveEventListener (
    Link<>& rCallback,
    EventMultiplexerEvent::EventId aEventTypes)
{
    mpImpl->RemoveEventListener (rCallback, aEventTypes);
}

void EventMultiplexer::MultiplexEvent(
    EventMultiplexerEvent::EventId eEventId,
    void* pUserData )
{
    EventMultiplexerEvent aEvent (mpImpl->GetViewShellBase(), eEventId, pUserData);
    mpImpl->CallListeners(aEvent);
}

//===== EventMultiplexer::Implementation ======================================

EventMultiplexer::Implementation::Implementation (ViewShellBase& rBase)
    : MutexOwner(),
      EventMultiplexerImplementationInterfaceBase(maMutex),
      SfxListener(),
      mrBase (rBase),
      mbListeningToController (false),
      mbListeningToFrame (false),
      mxControllerWeak(NULL),
      mxFrameWeak(NULL),
      mxSlideSorterSelectionWeak(NULL),
      mpDocument(NULL),
      mxConfigurationControllerWeak()
{
    // Connect to the frame to listen for controllers being exchanged.
    // Listen to changes of certain properties.
    Reference<frame::XFrame> xFrame (
        mrBase.GetFrame()->GetTopFrame().GetFrameInterface(),
        uno::UNO_QUERY);
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

    // Listen for document changes.
    mpDocument = mrBase.GetDocument();
    if (mpDocument != NULL)
        StartListening (*mpDocument);

    // Listen for configuration changes.
    Reference<XControllerManager> xControllerManager (
        Reference<XWeak>(&mrBase.GetDrawController()), UNO_QUERY);
    if (xControllerManager.is())
    {
        Reference<XConfigurationController> xConfigurationController (
            xControllerManager->getConfigurationController());
        mxConfigurationControllerWeak = xConfigurationController;
        if (xConfigurationController.is())
        {
            Reference<XComponent> xComponent (xConfigurationController, UNO_QUERY);
            if (xComponent.is())
                xComponent->addEventListener(static_cast<beans::XPropertyChangeListener*>(this));

            xConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceActivationEvent,
                makeAny(ResourceActivationEvent));
            xConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msResourceDeactivationEvent,
                makeAny(ResourceDeactivationEvent));
            xConfigurationController->addConfigurationChangeListener(
                this,
                FrameworkHelper::msConfigurationUpdateEndEvent,
                makeAny(ConfigurationUpdateEvent));
        }
    }
}

EventMultiplexer::Implementation::~Implementation()
{
    DBG_ASSERT( !mbListeningToFrame,
        "sd::EventMultiplexer::Implementation::~Implementation(), disposing was not called!" );
}

void EventMultiplexer::Implementation::ReleaseListeners()
{
    if (mbListeningToFrame)
    {
        mbListeningToFrame = false;

        // Stop listening for changes of certain properties.
        Reference<frame::XFrame> xFrame (mxFrameWeak);
        if (xFrame.is())
        {
            xFrame->removeFrameActionListener (
                Reference<frame::XFrameActionListener>(
                    static_cast<XWeak*>(this), UNO_QUERY));
        }
    }

    DisconnectFromController ();

    if (mpDocument != NULL)
    {
        EndListening (*mpDocument);
        mpDocument = NULL;
    }

    // Stop listening for configuration changes.
    Reference<XConfigurationController> xConfigurationController (mxConfigurationControllerWeak);
    if (xConfigurationController.is())
    {
        Reference<XComponent> xComponent (xConfigurationController, UNO_QUERY);
        if (xComponent.is())
            xComponent->removeEventListener(static_cast<beans::XPropertyChangeListener*>(this));

        xConfigurationController->removeConfigurationChangeListener(this);
    }
}

void EventMultiplexer::Implementation::AddEventListener (
    Link<>& rCallback,
    EventMultiplexerEvent::EventId aEventTypes)
{
    ListenerList::iterator iListener (maListeners.begin());
    ListenerList::const_iterator iEnd (maListeners.end());
    for (;iListener!=iEnd; ++iListener)
        if (iListener->first == rCallback)
            break;
    if (iListener != maListeners.end())
    {
        // Listener exists.  Update its event type set.
        iListener->second |= aEventTypes;
    }
    else
    {
        maListeners.push_back (ListenerDescriptor(rCallback,aEventTypes));
    }
}

void EventMultiplexer::Implementation::RemoveEventListener (
    Link<>& rCallback,
    EventMultiplexerEvent::EventId aEventTypes)
{
    ListenerList::iterator iListener (maListeners.begin());
    ListenerList::const_iterator iEnd (maListeners.end());
    for (;iListener!=iEnd; ++iListener)
        if (iListener->first == rCallback)
            break;
    if (iListener != maListeners.end())
    {
        // Update the event type set.
        iListener->second &= ~aEventTypes;
        // When no events remain in the set then remove the listener.
        if (iListener->second == EID_EMPTY_SET)
            maListeners.erase (iListener);
    }
}

void EventMultiplexer::Implementation::ConnectToController()
{
    // Just in case that we missed some event we now disconnect from the old
    // controller.
    DisconnectFromController ();

    // Register at the controller of the main view shell.

    // We have to store a (weak) reference to the controller so that we can
    // unregister without having to ask the mrBase member (which at that
    // time may be destroyed.)
    Reference<frame::XController> xController = mrBase.GetController();
    mxControllerWeak = mrBase.GetController();

    try
    {
        // Listen for disposing events.
        Reference<lang::XComponent> xComponent (xController, UNO_QUERY);
        if (xComponent.is())
        {
            xComponent->addEventListener (
                Reference<lang::XEventListener>(
                    static_cast<XWeak*>(this), UNO_QUERY));
            mbListeningToController = true;
        }

        // Listen to changes of certain properties.
        Reference<beans::XPropertySet> xSet (xController, UNO_QUERY);
        if (xSet.is())
        {
                try
                {
                    xSet->addPropertyChangeListener(OUString(aCurrentPagePropertyName), this);
                }
                catch (const beans::UnknownPropertyException&)
                {
                    OSL_TRACE("EventMultiplexer::ConnectToController: CurrentPage unknown");
                }

                try
                {
                    xSet->addPropertyChangeListener(OUString(aEditModePropertyName), this);
                }
                catch (const beans::UnknownPropertyException&)
                {
                    OSL_TRACE("EventMultiplexer::ConnectToController: IsMasterPageMode unknown");
                }
        }

        // Listen for selection change events.
        Reference<view::XSelectionSupplier> xSelection (xController, UNO_QUERY);
        if (xSelection.is())
        {
            xSelection->addSelectionChangeListener(this);
        }
    }
    catch (const lang::DisposedException&)
    {
        mbListeningToController = false;
    }
}

void EventMultiplexer::Implementation::DisconnectFromController()
{
    if (mbListeningToController)
    {
        mbListeningToController = false;

        Reference<frame::XController> xController = mxControllerWeak;

        Reference<beans::XPropertySet> xSet (xController, UNO_QUERY);
        // Remove the property listener.
        if (xSet.is())
        {
            try
            {
                xSet->removePropertyChangeListener(OUString(aCurrentPagePropertyName), this);
            }
            catch (const beans::UnknownPropertyException&)
            {
                OSL_TRACE ("DisconnectFromController: CurrentPage unknown");
            }

            try
            {
                xSet->removePropertyChangeListener(OUString(aEditModePropertyName), this);
            }
            catch (const beans::UnknownPropertyException&)
            {
                OSL_TRACE ("DisconnectFromController: IsMasterPageMode unknown");
            }
        }

        // Remove selection change listener.
        Reference<view::XSelectionSupplier> xSelection (xController, UNO_QUERY);
        if (xSelection.is())
        {
            xSelection->removeSelectionChangeListener(this);
        }

        // Remove listener for disposing events.
        Reference<lang::XComponent> xComponent (xController, UNO_QUERY);
        if (xComponent.is())
        {
            xComponent->removeEventListener (
                Reference<lang::XEventListener>(static_cast<XWeak*>(this), UNO_QUERY));
        }
    }
}

//=====  lang::XEventListener  ================================================

void SAL_CALL EventMultiplexer::Implementation::disposing (
    const lang::EventObject& rEventObject)
    throw (RuntimeException, std::exception)
{
    if (mbListeningToController)
    {
        Reference<frame::XController> xController (mxControllerWeak);
        if (rEventObject.Source == xController)
        {
            mbListeningToController = false;
        }
    }

    Reference<XConfigurationController> xConfigurationController (
        mxConfigurationControllerWeak);
    if (xConfigurationController.is()
        && rEventObject.Source == xConfigurationController)
    {
        mxConfigurationControllerWeak.clear();
    }
}

//=====  beans::XPropertySetListener  =========================================

void SAL_CALL EventMultiplexer::Implementation::propertyChange (
    const beans::PropertyChangeEvent& rEvent)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    if ( rEvent.PropertyName == aCurrentPagePropertyName )
    {
        CallListeners(EventMultiplexerEvent::EID_CURRENT_PAGE);
    }
    else if ( rEvent.PropertyName == aEditModePropertyName )
    {
        bool bIsMasterPageMode (false);
        rEvent.NewValue >>= bIsMasterPageMode;
        if (bIsMasterPageMode)
            CallListeners(EventMultiplexerEvent::EID_EDIT_MODE_MASTER);
        else
            CallListeners(EventMultiplexerEvent::EID_EDIT_MODE_NORMAL);
    }
}

//===== frame::XFrameActionListener  ==========================================

void SAL_CALL EventMultiplexer::Implementation::frameAction (
    const frame::FrameActionEvent& rEvent)
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    Reference<frame::XFrame> xFrame (mxFrameWeak);
    if (rEvent.Frame == xFrame)
        switch (rEvent.Action)
        {
            case frame::FrameAction_COMPONENT_DETACHING:
                DisconnectFromController();
                CallListeners (EventMultiplexerEvent::EID_CONTROLLER_DETACHED);
                break;

            case frame::FrameAction_COMPONENT_REATTACHED:
                CallListeners (EventMultiplexerEvent::EID_CONTROLLER_DETACHED);
                DisconnectFromController();
                ConnectToController();
                CallListeners (EventMultiplexerEvent::EID_CONTROLLER_ATTACHED);
                break;

            case frame::FrameAction_COMPONENT_ATTACHED:
                ConnectToController();
                CallListeners (EventMultiplexerEvent::EID_CONTROLLER_ATTACHED);
                break;

            default:
                break;
        }
}

//===== view::XSelectionChangeListener ========================================

void SAL_CALL EventMultiplexer::Implementation::selectionChanged (
    const lang::EventObject& )
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    CallListeners (EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION);
}

//===== drawing::framework::XConfigurationChangeListener ==================

void SAL_CALL EventMultiplexer::Implementation::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException, std::exception)
{
    sal_Int32 nEventType = 0;
    rEvent.UserData >>= nEventType;
    switch (nEventType)
    {
        case ResourceActivationEvent:
            if (rEvent.ResourceId->getResourceURL().match(FrameworkHelper::msViewURLPrefix))
            {
                CallListeners (EventMultiplexerEvent::EID_VIEW_ADDED);

                if (rEvent.ResourceId->isBoundToURL(
                    FrameworkHelper::msCenterPaneURL, AnchorBindingMode_DIRECT))
                {
                    CallListeners (EventMultiplexerEvent::EID_MAIN_VIEW_ADDED);
                }

                // Add selection change listener at slide sorter.
                if (rEvent.ResourceId->getResourceURL().equals(FrameworkHelper::msSlideSorterURL))
                {
                    slidesorter::SlideSorterViewShell* pViewShell
                        = dynamic_cast<slidesorter::SlideSorterViewShell*>(
                            FrameworkHelper::GetViewShell(
                                Reference<XView>(rEvent.ResourceObject,UNO_QUERY)).get());
                    if (pViewShell != NULL)
                        pViewShell->AddSelectionChangeListener (
                            LINK(this,
                                EventMultiplexer::Implementation,
                                SlideSorterSelectionChangeListener));
                }
            }
            break;

        case ResourceDeactivationEvent:
            if (rEvent.ResourceId->getResourceURL().match(FrameworkHelper::msViewURLPrefix))
            {
                CallListeners (EventMultiplexerEvent::EID_VIEW_REMOVED);

                if (rEvent.ResourceId->isBoundToURL(
                    FrameworkHelper::msCenterPaneURL, AnchorBindingMode_DIRECT))
                {
                    CallListeners (EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED);
                }

                // Remove selection change listener from slide sorter.  Add
                // selection change listener at slide sorter.
                if (rEvent.ResourceId->getResourceURL().equals(FrameworkHelper::msSlideSorterURL))
                {
                    slidesorter::SlideSorterViewShell* pViewShell
                        = dynamic_cast<slidesorter::SlideSorterViewShell*>(
                            FrameworkHelper::GetViewShell(
                                Reference<XView>(rEvent.ResourceObject, UNO_QUERY)).get());
                    if (pViewShell != NULL)
                        pViewShell->RemoveSelectionChangeListener (
                            LINK(this,
                                EventMultiplexer::Implementation,
                                SlideSorterSelectionChangeListener));
                }
            }
            break;

        case ConfigurationUpdateEvent:
            CallListeners (EventMultiplexerEvent::EID_CONFIGURATION_UPDATED);
            break;
    }

}

void SAL_CALL EventMultiplexer::Implementation::disposing()
{
    CallListeners (EventMultiplexerEvent::EID_DISPOSING);
    ReleaseListeners();
}

void EventMultiplexer::Implementation::ThrowIfDisposed()
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            "SlideSorterController object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }
}

void EventMultiplexer::Implementation::Notify (
    SfxBroadcaster&,
    const SfxHint& rHint)
{
    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint);
    if (pSdrHint)
    {
        switch (pSdrHint->GetKind())
        {
            case HINT_MODELCLEARED:
            case HINT_PAGEORDERCHG:
                CallListeners (EventMultiplexerEvent::EID_PAGE_ORDER);
                break;

            case HINT_SWITCHTOPAGE:
                CallListeners (EventMultiplexerEvent::EID_CURRENT_PAGE);
                break;

            case HINT_OBJCHG:
                CallListeners(EventMultiplexerEvent::EID_SHAPE_CHANGED,
                    const_cast<void*>(static_cast<const void*>(pSdrHint->GetPage())));
                break;

            case HINT_OBJINSERTED:
                CallListeners(EventMultiplexerEvent::EID_SHAPE_INSERTED,
                    const_cast<void*>(static_cast<const void*>(pSdrHint->GetPage())));
                break;

            case HINT_OBJREMOVED:
                CallListeners(EventMultiplexerEvent::EID_SHAPE_REMOVED,
                    const_cast<void*>(static_cast<const void*>(pSdrHint->GetPage())));
                break;
            default:
                break;
        }
    }
    else if (dynamic_cast<const SfxSimpleHint*>(&rHint))
    {
        const SfxSimpleHint& rSimpleHint = static_cast<const SfxSimpleHint&>(rHint);
        if (rSimpleHint.GetId() == SFX_HINT_DYING)
            mpDocument = NULL;
    }
}

void EventMultiplexer::Implementation::CallListeners (
    EventMultiplexerEvent::EventId eId,
    void* pUserData)
{
    EventMultiplexerEvent aEvent (mrBase, eId, pUserData);
    CallListeners(aEvent);
}

void EventMultiplexer::Implementation::CallListeners (EventMultiplexerEvent& rEvent)
{
    ListenerList aCopyListeners( maListeners );
    ListenerList::iterator iListener (aCopyListeners.begin());
    ListenerList::const_iterator iListenerEnd (aCopyListeners.end());
    for (; iListener!=iListenerEnd; ++iListener)
    {
        if ((iListener->second && rEvent.meEventId))
            iListener->first.Call(&rEvent);
    }
}

IMPL_LINK_NOARG(EventMultiplexer::Implementation, SlideSorterSelectionChangeListener)
{
    CallListeners (EventMultiplexerEvent::EID_SLIDE_SORTER_SELECTION);
    return 0;
}

//===== EventMultiplexerEvent =================================================

EventMultiplexerEvent::EventMultiplexerEvent (
    const ViewShellBase& rBase,
    EventId eEventId,
    const void* pUserData)
    : mrBase(rBase),
      meEventId(eEventId),
      mpUserData(pUserData)

{
}

} } // end of namespace ::sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
