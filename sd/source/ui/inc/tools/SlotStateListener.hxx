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

#ifndef SD_TOOLS_SLOT_STATE_LISTENER_HXX
#define SD_TOOLS_SLOT_STATE_LISTENER_HXX

#include "MutexOwner.hxx"
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTIOIN_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/compbase1.hxx>
#include <tools/link.hxx>

namespace sd { namespace tools {


typedef cppu::WeakComponentImplHelper1<
    ::com::sun::star::frame::XStatusListener
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
        Link& rCallback,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XDispatchProvider>& rxDispatchProvider,
        const ::rtl::OUString& rSlotName);

    /** The constructor de-registers all remaining listeners.  Usually a prior
        dispose() call should have done that already.
    */
    virtual ~SlotStateListener (void);

    /** Set the callback to the given value.  Whenever one of the observed
        slots changes its state this callback is informed about it.
        Changing the callback does not release the listeners.
        @throws DisposedException
    */
    void SetCallback (const Link& rCallback);

    /** Set the frame whose slots shall be observed.  When an object of this
        class is already observing slots of another frame then these
        listeners are released first.
        @throws DisposedException
    */
    void ConnectToDispatchProvider (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XDispatchProvider>& rxDispatchProvider);

    /** Observe the slot specified by the given name.  Note that
        ConnectToFrame() has to have been called earlier.
        @param rSlotName
            The name of the slot to observe.  An example is
            ".uno:VerticalTextState".
        @throws DisposedException
    */
    void ObserveSlot (const ::rtl::OUString& rSlotName);

    //=====  frame::XStatusListener  ==========================================

    /** Called by slot state change broadcasters.  In turn the callback is
        informed about the state chage.
        @throws DisposedException
    */
    virtual void SAL_CALL
        statusChanged (
            const ::com::sun::star::frame::FeatureStateEvent& rState)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  lang::XEventListener  ============================================

    virtual void SAL_CALL
        disposing(const com::sun::star::lang::EventObject& rEvent)
        throw(com::sun::star::uno::RuntimeException);

protected:
    /** This method is called by the WeakComponentImplHelper base class in
        reaction to a XComponent::dispose() call.  It releases all currently
        active listeners.
    */
    virtual void SAL_CALL disposing (void);

private:
    Link maCallback;

    /** Remember the URLs that describe slots whose state changes we are
        listening to.
    */
    typedef ::std::vector<com::sun::star::util::URL> RegisteredURLList;
    RegisteredURLList maRegisteredURLList;

    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::frame::XDispatchProvider> mxDispatchProviderWeak;

    /** Deregister all currently active state change listeners.
    */
    void ReleaseListeners (void);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void)
        throw (::com::sun::star::lang::DisposedException);

    /** Transform the given string into a URL object.
    */
    ::com::sun::star::util::URL MakeURL (const ::rtl::OUString& rSlotName) const;

    /** Return an XDispatch object for the given URL.
    */
    ::com::sun::star::uno::Reference<com::sun::star::frame::XDispatch>
        GetDispatch (
            const ::com::sun::star::util::URL& rURL) const;
};

} } // end of namespace ::sd::tools

#endif
