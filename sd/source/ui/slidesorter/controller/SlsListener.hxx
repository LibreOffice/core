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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_CONTROLLER_SLSLISTENER_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_CONTROLLER_SLSLISTENER_HXX

#include <MutexOwner.hxx>
#include <controller/SlideSorterController.hxx>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <cppuhelper/compbase.hxx>

#include <svl/lstner.hxx>
#include <tools/link.hxx>
#include <memory>

class SdrPage;

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

typedef cppu::WeakComponentImplHelper<
    css::document::XEventListener,
    css::beans::XPropertyChangeListener,
    css::accessibility::XAccessibleEventListener,
    css::frame::XFrameActionListener
    > ListenerInterfaceBase;

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
    explicit Listener (SlideSorter& rSlideSorter);
    virtual ~Listener() override;

    /** Connect to the current controller of the view shell as listener.
        This method is called once during initialization and every time a
        FrameActionEvent signals the current controller being exchanged.
        When the connection is successful then the flag
        mbListeningToController is set to <TRUE/>.
    */
    void ConnectToController();

    /** Disconnect from the current controller of the view shell as
        listener.  This method is called once during initialization and
        every time a FrameActionEvent signals the current controller being
        exchanged.  When this method terminates then mbListeningToController
        is <FALSE/>.
    */
    void DisconnectFromController();

    virtual void Notify (
        SfxBroadcaster& rBroadcaster,
        const SfxHint& rHint) override;

    //=====  lang::XEventListener  ============================================
    virtual void SAL_CALL
        disposing (const css::lang::EventObject& rEventObject) override;

    //=====  document::XEventListener  ========================================
    virtual void SAL_CALL
        notifyEvent (
            const css::document::EventObject& rEventObject) override;

    //=====  beans::XPropertySetListener  =====================================
    virtual void SAL_CALL
        propertyChange (
            const css::beans::PropertyChangeEvent& rEvent) override;

    //===== accessibility::XAccessibleEventListener  ==========================
    virtual void SAL_CALL
        notifyEvent (
            const css::accessibility::AccessibleEventObject&
            rEvent) override;

    //===== frame::XFrameActionListener  ======================================
    /** For certain actions the listener connects to a new controller of the
        frame it is listening to.  This usually happens when the view shell
        in the center pane is replaced by another view shell.
    */
    virtual void SAL_CALL
        frameAction (const css::frame::FrameActionEvent& rEvent) override;

    virtual void SAL_CALL disposing() override;

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

    css::uno::WeakReference< css::frame::XController> mxControllerWeak;
    css::uno::WeakReference< css::frame::XFrame> mxFrameWeak;

    /** This object is used to lock the model between some
        events.  It is references counted in order to cope with events that
        are expected but never sent.
    */
    std::shared_ptr<SlideSorterController::ModelChangeLock> mpModelChangeLock;

    void ReleaseListeners();

    /** Called when the edit mode has changed.  Update model accordingly.
    */
    void UpdateEditMode();

    /** Handle a change in the order of slides or when the set of slides has
        changed, i.e. a slide has been created.
    */
    void HandleModelChange (const SdrPage* pPage);

    /** Handle a modification to a shape on the given page.  When this is a
        regular page then update its preview.  When it is a master page then
        additionally update the previews of all pages linked to it.
    */
    void HandleShapeModification (const SdrPage* pPage);

    DECL_LINK(EventMultiplexerCallback, tools::EventMultiplexerEvent&, void);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
