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

#ifndef SD_FRAMEWORK_READ_ONLY_MODE_OBSERVER_HXX
#define SD_FRAMEWORK_READ_ONLY_MODE_OBSERVER_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/compbase2.hxx>
#include <tools/link.hxx>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace {

typedef ::cppu::WeakComponentImplHelper2 <
      ::com::sun::star::drawing::framework::XConfigurationChangeListener,
      ::com::sun::star::frame::XStatusListener
    > ReadOnlyModeObserverInterfaceBase;

} // end of anonymous namespace.




namespace sd { namespace framework {

/** Wait for changes of the read-only mode.  On switching between read-only
    mode and read-write the registered listeners are called.

    This class handles the case that the given controller is not yet
    connected to a frame and that the dispatcher is not yet set up.  It
    waits for this to happen and then registers at the .uno:EditDoc command
    and waits for state changes.
*/
class ReadOnlyModeObserver
    : private sd::MutexOwner,
      public ReadOnlyModeObserverInterfaceBase
{
public:
    /** Create a new read-only mode observer for the given controller.
    */
    ReadOnlyModeObserver (
        const ::com::sun::star::uno::Reference<com::sun::star::frame::XController>& rxController);
    virtual ~ReadOnlyModeObserver (void);

    virtual void SAL_CALL disposing (void);


    /** Add a status listener that is called when the state of the
        .uno:EditDoc command changes.  Note that the listener has to take
        into account both the IsEnabled and the State fields of the
        FeatureStateEvent.  Only when IsEnabled is true then the State field
        is valid.
    */
    void AddStatusListener (
        const ::com::sun::star::uno::Reference<
            com::sun::star::frame::XStatusListener>& rxListener);

    // XEventListener

    virtual void SAL_CALL disposing (
        const com::sun::star::lang::EventObject& rEvent)
        throw (com::sun::star::uno::RuntimeException);


    // frame::XStatusListener

    /** Called by slot state change broadcasters.
        @throws DisposedException
    */
    virtual void SAL_CALL
        statusChanged (
            const ::com::sun::star::frame::FeatureStateEvent& rState)
        throw (::com::sun::star::uno::RuntimeException);

    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const ::com::sun::star::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::util::URL maSlotNameURL;
    /** The XController is stored to enable repeated calls to
        ConnectToDispatch() (get access to the XDispatchProvider.
    */
    ::com::sun::star::uno::Reference<com::sun::star::frame::XController>
        mxController;
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XConfigurationController>
        mxConfigurationController;
    ::com::sun::star::uno::Reference<com::sun::star::frame::XDispatch>
        mxDispatch;
    class ModifyBroadcaster;
    ::boost::scoped_ptr<ModifyBroadcaster> mpBroadcaster;

    /** Listen for the .uno:EditMode command.  Returns <TRUE/> when the connection
        has been established.
    */
    bool ConnectToDispatch (void);
};

} } // end of namespace sd::framework

#endif
