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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
