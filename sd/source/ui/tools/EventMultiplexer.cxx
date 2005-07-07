/*************************************************************************
 *
 *  $RCSfile: EventMultiplexer.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-07 13:38:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "EventMultiplexer.hxx"

#include "MutexOwner.hxx"
#include "ViewShellBase.hxx"
#include "PaneManager.hxx"
#include "drawdoc.hxx"
#include "DrawController.hxx"
#include "SlideSorterViewShell.hxx"

#ifndef _COM_SUN_STAR_DOCUMENT_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTIOIN_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#include <cppuhelper/weak.hxx>
#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif
#include <sfx2/viewfrm.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;


namespace sd { namespace tools {

const EventMultiplexer::EventTypeSet EventMultiplexer::ET_DISPOSING             (0x0001);
const EventMultiplexer::EventTypeSet EventMultiplexer::ET_EDIT_VIEW_SELECTION   (0x0002);
const EventMultiplexer::EventTypeSet EventMultiplexer::ET_SLIDE_SORTER_SELECTION(0x0004);
const EventMultiplexer::EventTypeSet EventMultiplexer::ET_CURRENT_PAGE          (0x0008);
const EventMultiplexer::EventTypeSet EventMultiplexer::ET_MAIN_VIEW             (0x0010);
const EventMultiplexer::EventTypeSet EventMultiplexer::ET_VIEW                  (0x0020);
const EventMultiplexer::EventTypeSet EventMultiplexer::ET_EDIT_MODE             (0x0040);
const EventMultiplexer::EventTypeSet EventMultiplexer::ET_PAGE_ORDER            (0x0080);
const EventMultiplexer::EventTypeSet EventMultiplexer::ET_TEXT_EDIT             (0x0100);

const EventMultiplexer::EventTypeSet EventMultiplexer::ETS_EMPTY_SET (0x00000000);
const EventMultiplexer::EventTypeSet EventMultiplexer::ETS_FULL_SET (0xffffffff);


typedef cppu::WeakComponentImplHelper3<
    ::com::sun::star::beans::XPropertyChangeListener,
    ::com::sun::star::frame::XFrameActionListener,
    ::com::sun::star::view::XSelectionChangeListener
    > EventMultiplexerImplementationInterfaceBase;

class EventMultiplexer::Implementation
    : protected MutexOwner,
      public EventMultiplexerImplementationInterfaceBase,
      public SfxListener
{
public:
    Implementation (ViewShellBase& rBase);
    ~Implementation (void);

    void AddEventListener (
        Link& rCallback,
        EventTypeSet aEventTypes);

    void RemoveEventListener (
        Link& rCallback,
        EventTypeSet aEventTypes);

    void CallListeners (
        EventType eType,
        EventMultiplexerEvent& rEvent);

    ViewShellBase& GetViewShellBase() const { return mrBase; }

    //===== lang::XEventListener ==============================================
    virtual void SAL_CALL
        disposing (const ::com::sun::star::lang::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);


    //===== beans::XPropertySetListener =======================================
    virtual void SAL_CALL
        propertyChange (
            const com::sun::star::beans::PropertyChangeEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException);

    //===== view::XSelectionChangeListener ====================================
    virtual void SAL_CALL
        selectionChanged (
            const com::sun::star::lang::EventObject& rEvent)
        throw (::com::sun::star::uno::RuntimeException);

    //===== frame::XFrameActionListener  ======================================
    /** For certain actions the listener connects to a new controller of the
        frame it is listening to.  This usually happens when the view shell
        in the center pane is replaced by another view shell.
    */
    virtual void SAL_CALL
        frameAction (const ::com::sun::star::frame::FrameActionEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing (void);

protected:
    virtual void Notify (
        SfxBroadcaster& rBroadcaster,
        const SfxHint& rHint);

private:
    ViewShellBase& mrBase;
    typedef ::std::pair<Link,EventTypeSet> ListenerDescriptor;
    typedef ::std::vector<ListenerDescriptor> ListenerList;
    ListenerList maListeners;

    /// Remember whether we are listening to the UNO controller.
    bool mbListeningToController;
    /// Remember whether we are listening to the frame.
    bool mbListeningToFrame;
    /// Remember when the pane manger becomes unavailable.
    bool mbPaneManagerAvailable;

    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::frame::XController> mxControllerWeak;
    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::frame::XFrame> mxFrameWeak;
    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::view::XSelectionSupplier> mxSlideSorterSelectionWeak;

    void ReleaseListeners (void);

    void ConnectToController (void);
    void DisconnectFromController (void);

    void CallListeners (
        EventType eType,
        EventMultiplexerEvent::EventId eId);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void)
        throw (::com::sun::star::lang::DisposedException);

    DECL_LINK(PaneManagerEventListener, PaneManagerEvent*);
    DECL_LINK(SlideSorterSelectionChangeListener, void*);
};




//===== EventMultiplexer ======================================================

EventMultiplexer::EventMultiplexer (ViewShellBase& rBase)
    : mpImpl (new EventMultiplexer::Implementation(rBase))
{
    mpImpl->acquire();
}




EventMultiplexer::~EventMultiplexer (void)
{
    mpImpl->dispose();
    // Now we call release twice.  One decreases the use count of the
    // implementation object (if all goes well to zero and thus deletes it.)
    // The other releases the auto_ptr and prevents the implementation
    // object from being deleted a second time.
    mpImpl->release();
    mpImpl.release();
}




void EventMultiplexer::AddEventListener (
    Link& rCallback,
    EventTypeSet aEventTypes)
{
    mpImpl->AddEventListener (rCallback, aEventTypes);
}




void EventMultiplexer::RemoveEventListener (
    Link& rCallback,
    EventTypeSet aEventTypes)
{
    mpImpl->RemoveEventListener (rCallback, aEventTypes);
}




void EventMultiplexer::MultiplexEvent( EventMultiplexerEvent::EventId eEventId, void* pUserData )
{
    EventType eEventType = ETS_FULL_SET;

    switch( eEventId )
    {
    case EventMultiplexerEvent::EID_DISPOSING:              eEventType = ET_DISPOSING; break;
    case EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION:    eEventType = ET_EDIT_VIEW_SELECTION; break;
    case EventMultiplexerEvent::EID_SLIDE_SORTER_SELECTION: eEventType = ET_SLIDE_SORTER_SELECTION; break;
    case EventMultiplexerEvent::EID_CURRENT_PAGE:           eEventType = ET_CURRENT_PAGE; break;
    case EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED:      eEventType = ET_MAIN_VIEW; break;
    case EventMultiplexerEvent::EID_MAIN_VIEW_ADDED:        eEventType = ET_MAIN_VIEW; break;
    case EventMultiplexerEvent::EID_VIEW_REMOVED:           eEventType = ET_VIEW; break;
    case EventMultiplexerEvent::EID_VIEW_ADDED:             eEventType = ET_VIEW; break;
    case EventMultiplexerEvent::EID_EDIT_MODE:              eEventType = ET_EDIT_MODE; break;
    case EventMultiplexerEvent::EID_PAGE_ORDER:             eEventType = ET_PAGE_ORDER; break;
    case EventMultiplexerEvent::EID_BEGIN_TEXT_EDIT:        eEventType = ET_TEXT_EDIT; break;
    case EventMultiplexerEvent::EID_END_TEXT_EDIT:          eEventType = ET_TEXT_EDIT; break;
    }

    EventMultiplexerEvent aEvent (mpImpl->GetViewShellBase(), eEventId, pUserData);
    mpImpl->CallListeners( eEventType, aEvent );
}




//===== EventMultiplexer::Implementation ======================================

EventMultiplexer::Implementation::Implementation (ViewShellBase& rBase)
    : MutexOwner(),
      EventMultiplexerImplementationInterfaceBase(maMutex),
      SfxListener(),
      mrBase (rBase),
      mbListeningToController (false),
      mbListeningToFrame (false),
      mbPaneManagerAvailable(true),
      mxControllerWeak(NULL),
      mxFrameWeak(NULL),
      mxSlideSorterSelectionWeak(NULL)
{
    // Connect to the frame to listen for controllers being exchanged.
    // Listen to changes of certain properties.
    Reference<frame::XFrame> xFrame (
        mrBase.GetFrame()->GetTopFrame()->GetFrameInterface(),
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
    StartListening (*mrBase.GetDocument());

    // Listen for view switches.
    if (mbPaneManagerAvailable)
        mrBase.GetPaneManager().AddEventListener (
            LINK(this,EventMultiplexer::Implementation, PaneManagerEventListener));
}




EventMultiplexer::Implementation::~Implementation (void)
{
    ReleaseListeners();
}




void EventMultiplexer::Implementation::ReleaseListeners (void)
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

    EndListening (*mrBase.GetDocument());

    // Stop listening for view switches.
    if (mbPaneManagerAvailable)
        mrBase.GetPaneManager().RemoveEventListener (
            LINK(this,EventMultiplexer::Implementation, PaneManagerEventListener));
}




void EventMultiplexer::Implementation::AddEventListener (
    Link& rCallback,
    EventTypeSet aEventTypes)
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
    Link& rCallback,
    EventTypeSet aEventTypes)
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
        if (iListener->second == ETS_EMPTY_SET)
            maListeners.erase (iListener);
    }
}




void EventMultiplexer::Implementation::ConnectToController (void)
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
    OSL_TRACE("EventMultiplexer %x connecting to controller %x", this,mxControllerWeak.get());

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
        try
        {
            if (xSet.is())
            {
                xSet->addPropertyChangeListener (
                    String::CreateFromAscii("CurrentPage"),
                    this);
                xSet->addPropertyChangeListener (
                    String::CreateFromAscii("IsMasterPageMode"),
                    this);
            }
        }
        catch (beans::UnknownPropertyException aEvent)
        {
            OSL_TRACE ("caught exception in SlideSorterController::SetupListeners: %s",
                ::rtl::OUStringToOString(aEvent.Message,
                    RTL_TEXTENCODING_UTF8).getStr());
        }

        // Listen for selection change events.
        Reference<view::XSelectionSupplier> xSelection (xController, UNO_QUERY);
        if (xSelection.is())
        {
            xSelection->addSelectionChangeListener(this);
        }
    }
    catch (const lang::DisposedException aException)
    {
        mbListeningToController = false;
    }
}




void EventMultiplexer::Implementation::DisconnectFromController (void)
{
    if (mbListeningToController)
    {
        mbListeningToController = false;

        OSL_TRACE("EventMultiplexer %x disconnecting from controller %x", this,mxControllerWeak.get());
        Reference<frame::XController> xController = mxControllerWeak;

        Reference<beans::XPropertySet> xSet (xController, UNO_QUERY);
        try
        {
            // Remove the property listener.
            if (xSet.is())
            {
                xSet->removePropertyChangeListener (
                    String::CreateFromAscii("CurrentPage"),
                    this);
                xSet->removePropertyChangeListener (
                    String::CreateFromAscii("IsMasterPageMode"),
                    this);
            }

            // Remove the dispose listener.
            Reference<XComponent> xComponent (xController, UNO_QUERY);
            if (xComponent.is())
                xComponent->removeEventListener (
                    Reference<lang::XEventListener>(
                        static_cast<XWeak*>(this), UNO_QUERY));
        }
        catch (beans::UnknownPropertyException aEvent)
        {
            OSL_TRACE ("caught exception in destructor of SlideSorterController: %s",
                ::rtl::OUStringToOString(aEvent.Message,
                    RTL_TEXTENCODING_UTF8).getStr());
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
            xComponent->removeEventListener (
                Reference<lang::XEventListener>(
                    static_cast<XWeak*>(this), UNO_QUERY));
    }
}




//=====  lang::XEventListener  ================================================

void SAL_CALL EventMultiplexer::Implementation::disposing (
    const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    if (mbListeningToController)
    {
        Reference<frame::XController> xController (mxControllerWeak);
        if (rEventObject.Source == xController)
        {
            mbListeningToController = false;
        }
    }
}




//=====  beans::XPropertySetListener  =========================================

void SAL_CALL EventMultiplexer::Implementation::propertyChange (
    const beans::PropertyChangeEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    static const ::rtl::OUString sCurrentPagePropertyName (
        RTL_CONSTASCII_USTRINGPARAM("CurrentPage"));
    static const ::rtl::OUString sEditModePropertyName (
        RTL_CONSTASCII_USTRINGPARAM("IsMasterPageMode"));

    OSL_TRACE ("property changed: %s",
        ::rtl::OUStringToOString(rEvent.PropertyName,
            RTL_TEXTENCODING_UTF8).getStr());

    if (rEvent.PropertyName.equals (sCurrentPagePropertyName))
    {
        CallListeners (
            ET_CURRENT_PAGE,
            EventMultiplexerEvent::EID_CURRENT_PAGE);
    }
    else if (rEvent.PropertyName.equals (sEditModePropertyName))
    {
        CallListeners (
            ET_EDIT_MODE,
            EventMultiplexerEvent::EID_EDIT_MODE);
    }
}




//===== frame::XFrameActionListener  ==========================================

void SAL_CALL EventMultiplexer::Implementation::frameAction (
    const frame::FrameActionEvent& rEvent)
    throw (::com::sun::star::uno::RuntimeException)
{
    Reference<frame::XFrame> xFrame (mxFrameWeak);
    if (rEvent.Frame == xFrame)
        switch (rEvent.Action)
        {
            case frame::FrameAction_COMPONENT_DETACHING:
                DisconnectFromController();
                break;

            case frame::FrameAction_COMPONENT_REATTACHED:
            case frame::FrameAction_COMPONENT_ATTACHED:
                ConnectToController();
                break;

            default:
                break;
        }
}




//===== view::XSelectionChangeListener ========================================

void SAL_CALL EventMultiplexer::Implementation::selectionChanged (
    const lang::EventObject& rEvent)
    throw (::com::sun::star::uno::RuntimeException)
{
    CallListeners (
        ET_EDIT_VIEW_SELECTION,
        EventMultiplexerEvent::EID_EDIT_VIEW_SELECTION);
}




void SAL_CALL EventMultiplexer::Implementation::disposing (void)
{
    CallListeners (
        ET_DISPOSING,
        EventMultiplexerEvent::EID_DISPOSING);
    ReleaseListeners();
}




void EventMultiplexer::Implementation::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "SlideSorterController object has already been disposed")),
            static_cast<uno::XWeak*>(this));
    }
}




void EventMultiplexer::Implementation::Notify (
    SfxBroadcaster& rBroadcaster,
    const SfxHint& rHint)
{
    if (rHint.ISA(SdrHint))
    {
        SdrHint& rSdrHint (*PTR_CAST(SdrHint,&rHint));
        switch (rSdrHint.GetKind())
        {
            case HINT_MODELCLEARED:
            case HINT_PAGEORDERCHG:
                CallListeners (
                    ET_PAGE_ORDER,
                    EventMultiplexerEvent::EID_PAGE_ORDER);
                break;

            case HINT_SWITCHTOPAGE:
                CallListeners (
                    ET_CURRENT_PAGE,
                    EventMultiplexerEvent::EID_CURRENT_PAGE);
                break;
        }
    }
}




void EventMultiplexer::Implementation::CallListeners (
    EventType eType,
    EventMultiplexerEvent::EventId eId)
{
    EventMultiplexerEvent aEvent (mrBase, eId, NULL);
    CallListeners( eType, aEvent );
}




void EventMultiplexer::Implementation::CallListeners (
    EventType eType,
    EventMultiplexerEvent& rEvent)
{
    ListenerList::const_iterator iListener (maListeners.begin());
    ListenerList::const_iterator iListenerEnd (maListeners.end());
    for (; iListener!=iListenerEnd; ++iListener)
    {
        if ((iListener->second && eType) != 0)
            iListener->first.Call(&rEvent);
    }
}




IMPL_LINK(EventMultiplexer::Implementation, PaneManagerEventListener,
    PaneManagerEvent*, pEvent)
{
    OSL_ASSERT(pEvent!=NULL);

    switch (pEvent->meEventId)
    {
        case PaneManagerEvent::EID_VIEW_SHELL_ADDED:
            CallListeners (
                ET_VIEW,
                EventMultiplexerEvent::EID_VIEW_ADDED);

            if (pEvent->mePane == PaneManager::PT_CENTER)
                CallListeners (ET_MAIN_VIEW,
                    EventMultiplexerEvent::EID_MAIN_VIEW_ADDED);

            // Add selection change listener at slide sorter.
            if (pEvent->mpShell != NULL
                && pEvent->mpShell->GetShellType()
                    == ViewShell::ST_SLIDE_SORTER)
            {
                static_cast<slidesorter::SlideSorterViewShell*>(
                    pEvent->mpShell)->AddSelectionChangeListener (
                        LINK(this,
                            EventMultiplexer::Implementation,
                            SlideSorterSelectionChangeListener));
            }
            break;

        case PaneManagerEvent::EID_VIEW_SHELL_REMOVED:
            if (pEvent->mePane == PaneManager::PT_CENTER)
                CallListeners (ET_MAIN_VIEW,
                    EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED);
            CallListeners (
                ET_VIEW,
                EventMultiplexerEvent::EID_VIEW_REMOVED);

            // Remove selection change listener from slide sorter.
            if (pEvent->mpShell != NULL
                && pEvent->mpShell->GetShellType()
                    == ViewShell::ST_SLIDE_SORTER)
            {
                static_cast<slidesorter::SlideSorterViewShell*>(
                    pEvent->mpShell)->RemoveSelectionChangeListener (
                        LINK(this,
                            EventMultiplexer::Implementation,
                            SlideSorterSelectionChangeListener));
            }
            break;

        case PaneManagerEvent::EID_PANE_MANAGER_DYING:
            // Stop listening for view switches.
            mrBase.GetPaneManager().RemoveEventListener (
                LINK(this,EventMultiplexer::Implementation, PaneManagerEventListener));
            mbPaneManagerAvailable = false;
            break;
    }

    return 0;
}




IMPL_LINK(EventMultiplexer::Implementation, SlideSorterSelectionChangeListener,
    void*, pEvent)
{
    CallListeners (ET_SLIDE_SORTER_SELECTION,
        EventMultiplexerEvent::EID_SLIDE_SORTER_SELECTION);
    return 0;
}




//===== EventMultiplexerEvent =================================================

EventMultiplexerEvent::EventMultiplexerEvent (
    const ViewShellBase& rBase,
    EventId eEventId,
    void* pUserData)
    : mrBase(rBase),
      meEventId(eEventId),
      mpUserData(pUserData)

{
}

} } // end of namespace ::sd::tools
