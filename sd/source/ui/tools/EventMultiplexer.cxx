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

#include <EventMultiplexer.hxx>

#include <MutexOwner.hxx>
#include <ViewShellBase.hxx>
#include <drawdoc.hxx>
#include <DrawController.hxx>
#include <SlideSorterViewShell.hxx>
#include <framework/FrameworkHelper.hxx>
#include <sal/log.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/compbase.hxx>
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

typedef cppu::WeakComponentImplHelper<
      css::beans::XPropertyChangeListener,
      css::frame::XFrameActionListener,
      css::view::XSelectionChangeListener,
      css::drawing::framework::XConfigurationChangeListener
    > EventMultiplexerImplementationInterfaceBase;

class EventMultiplexer::Implementation
    : protected MutexOwner,
      public EventMultiplexerImplementationInterfaceBase,
      public SfxListener
{
public:
    explicit Implementation (ViewShellBase& rBase);
    virtual ~Implementation() override;

    void AddEventListener (
        const Link<EventMultiplexerEvent&,void>& rCallback);

    void RemoveEventListener (
        const Link<EventMultiplexerEvent&,void>& rCallback);

    void CallListeners (EventMultiplexerEvent& rEvent);

    //===== lang::XEventListener ==============================================
    virtual void SAL_CALL
        disposing (const css::lang::EventObject& rEventObject) override;

    //===== beans::XPropertySetListener =======================================
    virtual void SAL_CALL
        propertyChange (
            const css::beans::PropertyChangeEvent& rEvent) override;

    //===== view::XSelectionChangeListener ====================================
    virtual void SAL_CALL
        selectionChanged (
            const css::lang::EventObject& rEvent) override;

    //===== frame::XFrameActionListener  ======================================
    /** For certain actions the listener connects to a new controller of the
        frame it is listening to.  This usually happens when the view shell
        in the center pane is replaced by another view shell.
    */
    virtual void SAL_CALL
        frameAction (const css::frame::FrameActionEvent& rEvent) override;

    //===== drawing::framework::XConfigurationChangeListener ==================
    virtual void SAL_CALL
        notifyConfigurationChange (
            const css::drawing::framework::ConfigurationChangeEvent& rEvent) override;

    virtual void SAL_CALL disposing() override;

protected:
    virtual void Notify (
        SfxBroadcaster& rBroadcaster,
        const SfxHint& rHint) override;

private:
    ViewShellBase& mrBase;
    typedef ::std::vector<Link<EventMultiplexerEvent&,void>> ListenerList;
    ListenerList maListeners;

    /// Remember whether we are listening to the UNO controller.
    bool mbListeningToController;
    /// Remember whether we are listening to the frame.
    bool mbListeningToFrame;

    css::uno::WeakReference<css::frame::XController> mxControllerWeak;
    css::uno::WeakReference<css::frame::XFrame> mxFrameWeak;
    SdDrawDocument* mpDocument;
    css::uno::WeakReference<css::drawing::framework::XConfigurationController>
         mxConfigurationControllerWeak;

    void ReleaseListeners();

    void ConnectToController();
    void DisconnectFromController();

    void CallListeners (
        EventMultiplexerEventId eId,
        void const * pUserData = nullptr);

    DECL_LINK(SlideSorterSelectionChangeListener, LinkParamNone*, void);
};

const char aCurrentPagePropertyName[] = "CurrentPage";
const char aEditModePropertyName[] = "IsMasterPageMode";

//===== EventMultiplexer ======================================================

EventMultiplexer::EventMultiplexer (ViewShellBase& rBase)
    : mpImpl (new EventMultiplexer::Implementation(rBase))
{
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
    const Link<EventMultiplexerEvent&,void>& rCallback)
{
    mpImpl->AddEventListener(rCallback);
}

void EventMultiplexer::RemoveEventListener (
    const Link<EventMultiplexerEvent&,void>& rCallback)
{
    mpImpl->RemoveEventListener(rCallback);
}

void EventMultiplexer::MultiplexEvent(
    EventMultiplexerEventId eEventId,
    void const * pUserData )
{
    EventMultiplexerEvent aEvent(eEventId, pUserData);
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
      mxControllerWeak(nullptr),
      mxFrameWeak(nullptr),
      mpDocument(nullptr),
      mxConfigurationControllerWeak()
{
    // Connect to the frame to listen for controllers being exchanged.
    // Listen to changes of certain properties.
    Reference<frame::XFrame> xFrame (
        mrBase.GetFrame()->GetFrame().GetFrameInterface(),
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
    if (mpDocument != nullptr)
        StartListening (*mpDocument);

    // Listen for configuration changes.
    Reference<XControllerManager> xControllerManager (
        Reference<XWeak>(&mrBase.GetDrawController()), UNO_QUERY);
    if (!xControllerManager.is())
        return;

    Reference<XConfigurationController> xConfigurationController (
        xControllerManager->getConfigurationController());
    mxConfigurationControllerWeak = xConfigurationController;
    if (!xConfigurationController.is())
        return;

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

    if (mpDocument != nullptr)
    {
        EndListening (*mpDocument);
        mpDocument = nullptr;
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
    const Link<EventMultiplexerEvent&,void>& rCallback)
{
    for (auto const & i : maListeners)
        if (i == rCallback)
            return;
    maListeners.push_back(rCallback);
}

void EventMultiplexer::Implementation::RemoveEventListener (
    const Link<EventMultiplexerEvent&,void>& rCallback)
{
    auto iListener = std::find(maListeners.begin(), maListeners.end(), rCallback);
    if (iListener != maListeners.end())
        maListeners.erase(iListener);
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
                    xSet->addPropertyChangeListener(aCurrentPagePropertyName, this);
                }
                catch (const beans::UnknownPropertyException&)
                {
                    SAL_WARN("sd", "EventMultiplexer::ConnectToController: CurrentPage unknown");
                }

                try
                {
                    xSet->addPropertyChangeListener(aEditModePropertyName, this);
                }
                catch (const beans::UnknownPropertyException&)
                {
                    SAL_WARN("sd", "EventMultiplexer::ConnectToController: IsMasterPageMode unknown");
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
    if (!mbListeningToController)
        return;

    mbListeningToController = false;

    Reference<frame::XController> xController = mxControllerWeak;

    Reference<beans::XPropertySet> xSet (xController, UNO_QUERY);
    // Remove the property listener.
    if (xSet.is())
    {
        try
        {
            xSet->removePropertyChangeListener(aCurrentPagePropertyName, this);
        }
        catch (const beans::UnknownPropertyException&)
        {
            SAL_WARN("sd", "DisconnectFromController: CurrentPage unknown");
        }

        try
        {
            xSet->removePropertyChangeListener(aEditModePropertyName, this);
        }
        catch (const beans::UnknownPropertyException&)
        {
            SAL_WARN("sd", "DisconnectFromController: IsMasterPageMode unknown");
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

//=====  lang::XEventListener  ================================================

void SAL_CALL EventMultiplexer::Implementation::disposing (
    const lang::EventObject& rEventObject)
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
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            "SlideSorterController object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }

    if ( rEvent.PropertyName == aCurrentPagePropertyName )
    {
        CallListeners(EventMultiplexerEventId::CurrentPageChanged);
    }
    else if ( rEvent.PropertyName == aEditModePropertyName )
    {
        bool bIsMasterPageMode (false);
        rEvent.NewValue >>= bIsMasterPageMode;
        if (bIsMasterPageMode)
            CallListeners(EventMultiplexerEventId::EditModeMaster);
        else
            CallListeners(EventMultiplexerEventId::EditModeNormal);
    }
}

//===== frame::XFrameActionListener  ==========================================

void SAL_CALL EventMultiplexer::Implementation::frameAction (
    const frame::FrameActionEvent& rEvent)
{
    Reference<frame::XFrame> xFrame (mxFrameWeak);
    if (rEvent.Frame != xFrame)
        return;

    switch (rEvent.Action)
    {
        case frame::FrameAction_COMPONENT_DETACHING:
            DisconnectFromController();
            CallListeners (EventMultiplexerEventId::ControllerDetached);
            break;

        case frame::FrameAction_COMPONENT_REATTACHED:
            CallListeners (EventMultiplexerEventId::ControllerDetached);
            DisconnectFromController();
            ConnectToController();
            CallListeners (EventMultiplexerEventId::ControllerAttached);
            break;

        case frame::FrameAction_COMPONENT_ATTACHED:
            ConnectToController();
            CallListeners (EventMultiplexerEventId::ControllerAttached);
            break;

        default:
            break;
    }
}

//===== view::XSelectionChangeListener ========================================

void SAL_CALL EventMultiplexer::Implementation::selectionChanged (
    const lang::EventObject& )
{
    CallListeners (EventMultiplexerEventId::EditViewSelection);
}

//===== drawing::framework::XConfigurationChangeListener ==================

void SAL_CALL EventMultiplexer::Implementation::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
{
    sal_Int32 nEventType = 0;
    rEvent.UserData >>= nEventType;
    switch (nEventType)
    {
        case ResourceActivationEvent:
            if (rEvent.ResourceId->getResourceURL().match(FrameworkHelper::msViewURLPrefix))
            {
                CallListeners (EventMultiplexerEventId::ViewAdded);

                if (rEvent.ResourceId->isBoundToURL(
                    FrameworkHelper::msCenterPaneURL, AnchorBindingMode_DIRECT))
                {
                    CallListeners (EventMultiplexerEventId::MainViewAdded);
                }

                // Add selection change listener at slide sorter.
                if (rEvent.ResourceId->getResourceURL() == FrameworkHelper::msSlideSorterURL)
                {
                    slidesorter::SlideSorterViewShell* pViewShell
                        = dynamic_cast<slidesorter::SlideSorterViewShell*>(
                            FrameworkHelper::GetViewShell(
                                Reference<XView>(rEvent.ResourceObject,UNO_QUERY)).get());
                    if (pViewShell != nullptr)
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
                if (rEvent.ResourceId->isBoundToURL(
                    FrameworkHelper::msCenterPaneURL, AnchorBindingMode_DIRECT))
                {
                    CallListeners (EventMultiplexerEventId::MainViewRemoved);
                }

                // Remove selection change listener from slide sorter.  Add
                // selection change listener at slide sorter.
                if (rEvent.ResourceId->getResourceURL() == FrameworkHelper::msSlideSorterURL)
                {
                    slidesorter::SlideSorterViewShell* pViewShell
                        = dynamic_cast<slidesorter::SlideSorterViewShell*>(
                            FrameworkHelper::GetViewShell(
                                Reference<XView>(rEvent.ResourceObject, UNO_QUERY)).get());
                    if (pViewShell != nullptr)
                        pViewShell->RemoveSelectionChangeListener (
                            LINK(this,
                                EventMultiplexer::Implementation,
                                SlideSorterSelectionChangeListener));
                }
            }
            break;

        case ConfigurationUpdateEvent:
            CallListeners (EventMultiplexerEventId::ConfigurationUpdated);
            break;
    }

}

void SAL_CALL EventMultiplexer::Implementation::disposing()
{
    CallListeners (EventMultiplexerEventId::Disposing);
    ReleaseListeners();
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
            case SdrHintKind::ModelCleared:
            case SdrHintKind::PageOrderChange:
                CallListeners (EventMultiplexerEventId::PageOrder);
                break;

            case SdrHintKind::SwitchToPage:
                CallListeners (EventMultiplexerEventId::CurrentPageChanged);
                break;

            case SdrHintKind::ObjectChange:
                CallListeners(EventMultiplexerEventId::ShapeChanged,
                    static_cast<const void*>(pSdrHint->GetPage()));
                break;

            case SdrHintKind::ObjectInserted:
                CallListeners(EventMultiplexerEventId::ShapeInserted,
                    static_cast<const void*>(pSdrHint->GetPage()));
                break;

            case SdrHintKind::ObjectRemoved:
                CallListeners(EventMultiplexerEventId::ShapeRemoved,
                    static_cast<const void*>(pSdrHint->GetPage()));
                break;
            default:
                break;
        }
    }
    else
    {
        if (rHint.GetId() == SfxHintId::Dying)
            mpDocument = nullptr;
    }
}

void EventMultiplexer::Implementation::CallListeners (
    EventMultiplexerEventId eId,
    void const * pUserData)
{
    EventMultiplexerEvent aEvent(eId, pUserData);
    CallListeners(aEvent);
}

void EventMultiplexer::Implementation::CallListeners (EventMultiplexerEvent& rEvent)
{
    ListenerList aCopyListeners( maListeners );
    for (auto& rListener : aCopyListeners)
    {
        rListener.Call(rEvent);
    }
}

IMPL_LINK_NOARG(EventMultiplexer::Implementation, SlideSorterSelectionChangeListener, LinkParamNone*, void)
{
    CallListeners(EventMultiplexerEventId::SlideSortedSelection);
}

//===== EventMultiplexerEvent =================================================

EventMultiplexerEvent::EventMultiplexerEvent (
    EventMultiplexerEventId eEventId,
    const void* pUserData)
    : meEventId(eEventId),
      mpUserData(pUserData)
{
}

} } // end of namespace ::sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
