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

#pragma once

#include <MutexOwner.hxx>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <cppuhelper/compbase.hxx>
#include <tools/link.hxx>

namespace com::sun::star::frame { class XDispatch; }
namespace com::sun::star::frame { class XDispatchProvider; }
namespace com::sun::star::frame { class XStatusListener; }
namespace com::sun::star::frame { struct FeatureStateEvent; }

namespace sd::tools {

typedef cppu::WeakComponentImplHelper<
    css::frame::XStatusListener
    > SlotStateListenerInterfaceBase;

/** Listen for state changes of slots.  This class has been created in order
    to be informed when the support for vertical writing changes but it can
    be used to relay state changes of other slots as well.
*/
class SlotStateListener
    : protected MutexOwner,
      public SlotStateListenerInterfaceBase
{
public:
    /** This convenience version of the constructor takes all parameters
        that are necessary to observe a single slot.  See descriptions of
        the SetCallback(), ConnectToFrame(), and ObserveSlot() methods for
        explanations about the parameters.
    */
    SlotStateListener (
        Link<const OUString&,void> const & rCallback,
        const css::uno::Reference<css::frame::XDispatchProvider>& rxDispatchProvider,
        const OUString& rSlotName);

    /** The constructor de-registers all remaining listeners.  Usually a prior
        dispose() call should have done that already.
    */
    virtual ~SlotStateListener() override;

    /** Set the callback to the given value.  Whenever one of the observed
        slots changes its state this callback is informed about it.
        Changing the callback does not release the listeners.
        @throws DisposedException
    */
    void SetCallback (const Link<const OUString&,void>& rCallback);

    /** Set the frame whose slots shall be observed.  When an object of this
        class is already observing slots of another frame then these
        listeners are released first.
        @throws DisposedException
    */
    void ConnectToDispatchProvider (
        const css::uno::Reference<css::frame::XDispatchProvider>& rxDispatchProvider);

    /** Observe the slot specified by the given name.  Note that
        ConnectToFrame() has to have been called earlier.
        @param rSlotName
            The name of the slot to observe.  An example is
            ".uno:VerticalTextState".
        @throws DisposedException
    */
    void ObserveSlot (const OUString& rSlotName);

    //=====  frame::XStatusListener  ==========================================

    /** Called by slot state change broadcasters.  In turn the callback is
        informed about the state change.
        @throws DisposedException
    */
    virtual void SAL_CALL
        statusChanged (
            const css::frame::FeatureStateEvent& rState) override;

    //=====  lang::XEventListener  ============================================

    virtual void SAL_CALL
        disposing(const css::lang::EventObject& rEvent) override;

protected:
    /** This method is called by the WeakComponentImplHelper base class in
        reaction to a XComponent::dispose() call.  It releases all currently
        active listeners.
    */
    virtual void SAL_CALL disposing() override;

private:
    Link<const OUString&,void> maCallback;

    /** Remember the URLs that describe slots whose state changes we are
        listening to.
    */
    std::vector<css::util::URL> maRegisteredURLList;

    css::uno::WeakReference<css::frame::XDispatchProvider> mxDispatchProviderWeak;

    /** Deregister all currently active state change listeners.
    */
    void ReleaseListeners();

    /** @throws css::lang::DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed();

    /** Transform the given string into a URL object.
    */
    static css::util::URL MakeURL (const OUString& rSlotName);

    /** Return an XDispatch object for the given URL.
    */
    css::uno::Reference<css::frame::XDispatch>
        GetDispatch (
            const css::util::URL& rURL) const;
};

} // end of namespace ::sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
