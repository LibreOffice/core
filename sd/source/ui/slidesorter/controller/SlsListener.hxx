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

#ifndef SD_SLIDESORTER_SLIDE_SORTER_LISTENER_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_LISTENER_HXX

#include "MutexOwner.hxx"
#include "controller/SlideSorterController.hxx"
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <cppuhelper/compbase4.hxx>

#include <svl/lstner.hxx>
#include <tools/link.hxx>
#include <boost/shared_ptr.hpp>

namespace sd {
class ViewShellBase;
}

namespace sd { namespace tools {
class EventMultiplexerEvent;
} }

namespace sd { namespace slidesorter {
class SlideSorter;
} }


namespace sd { namespace slidesorter { namespace controller {

typedef cppu::WeakComponentImplHelper4<
    ::com::sun::star::document::XEventListener,
    ::com::sun::star::beans::XPropertyChangeListener,
    ::com::sun::star::accessibility::XAccessibleEventListener,
    ::com::sun::star::frame::XFrameActionListener
    > ListenerInterfaceBase;

class SlideSorterController;

/** Listen for events of various types and sources and react to them.  This
    class is a part of the controller.

    When the view shell in the center pane is replaced by another the
    associated controller is replaced as well.  Therefore we have to
    register at the frame and on certain FrameActionEvents to stop listening
    to the old controller and register as listener at the new one.
*/
class Listener
    : protected MutexOwner,
      public ListenerInterfaceBase,
      public SfxListener
{
public:
    Listener (SlideSorter& rSlideSorter);
    virtual ~Listener (void);

    /** Connect to the current controller of the view shell as listener.
        This method is called once during initialization and every time a
        FrameActionEvent signals the current controller being exchanged.
        When the connection is successfull then the flag
        mbListeningToController is set to <TRUE/>.
    */
    void ConnectToController (void);

    /** Disconnect from the current controller of the view shell as
        listener.  This method is called once during initialization and
        every time a FrameActionEvent signals the current controller being
        exchanged.  When this method terminates then mbListeningToController
        is <FALSE/>.
    */
    void DisconnectFromController (void);

    virtual void Notify (
        SfxBroadcaster& rBroadcaster,
        const SfxHint& rHint);

    //=====  lang::XEventListener  ============================================
    virtual void SAL_CALL
        disposing (const ::com::sun::star::lang::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  document::XEventListener  ========================================
    virtual void SAL_CALL
        notifyEvent (
            const ::com::sun::star::document::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  beans::XPropertySetListener  =====================================
    virtual void SAL_CALL
        propertyChange (
            const com::sun::star::beans::PropertyChangeEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException);

    //===== accessibility::XAccessibleEventListener  ==========================
    virtual void SAL_CALL
        notifyEvent (
            const ::com::sun::star::accessibility::AccessibleEventObject&
            rEvent)
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

private:
    SlideSorter& mrSlideSorter;
    SlideSorterController& mrController;
    ViewShellBase* mpBase;

    /// Remember whether we are listening to the document.
    bool mbListeningToDocument;
    /// Remember whether we are listening to the UNO document.
    bool mbListeningToUNODocument;
    /// Remember whether we are listening to the UNO controller.
    bool mbListeningToController;
    /// Remember whether we are listening to the frame.
    bool mbListeningToFrame;
    bool mbIsMainViewChangePending;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XController> mxControllerWeak;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XFrame> mxFrameWeak;

    /** This object is used to lock the model between some
        events.  It is refernce counted in order to cope with events that
        are expected but never sent.
    */
    ::boost::shared_ptr<SlideSorterController::ModelChangeLock> mpModelChangeLock;

    void ReleaseListeners (void);

    /** Called when the edit mode has changed.  Update model accordingly.
    */
    void UpdateEditMode (void);

    /** Handle a change in the order of slides or when the set of slides has
        changed, i.e. a slide has been created.
    */
    void HandleModelChange (const SdrPage* pPage);

    /** Handle a modification to a shape on the given page.  When this is a
        regular page then update its preview.  When it is a master page then
        additionally update the previews of all pages linked to it.
    */
    void HandleShapeModification (const SdrPage* pPage);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void)
        throw (::com::sun::star::lang::DisposedException);

    DECL_LINK(EventMultiplexerCallback, tools::EventMultiplexerEvent*);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
