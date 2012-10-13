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

#ifndef SDEXT_PRESENTER_PRESENTER_FRAMEWORK_OBSERVER_HXX
#define SDEXT_PRESENTER_PRESENTER_FRAMEWORK_OBSERVER_HXX

#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

typedef ::cppu::WeakComponentImplHelper1 <
    ::com::sun::star::drawing::framework::XConfigurationChangeListener
    > PresenterFrameworkObserverInterfaceBase;

/** Watch the drawing framework for changes and run callbacks when a certain
    change takes place.
*/
class PresenterFrameworkObserver
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PresenterFrameworkObserverInterfaceBase
{
public:
    typedef ::boost::function<bool(void)> Predicate;
    typedef ::boost::function<void(bool)> Action;

    static void RunOnUpdateEnd (
        const css::uno::Reference<css::drawing::framework::XConfigurationController>&rxController,
        const Action& rAction);

    virtual void SAL_CALL disposing (void);
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException);

private:
    ::rtl::OUString msEventType;
    ::css::uno::Reference<css::drawing::framework::XConfigurationController> mxConfigurationController;
    Predicate maPredicate;
    Action maAction;

    /** Create a new PresenterFrameworkObserver object.
        @param rsEventName
            An event name other than ConfigurationUpdateEnd.  When the
            observer shall only listen for ConfigurationUpdateEnd then pass
            an empty name.
        @param rPredicate
            This functor tests whether the action is to be executed or not.
        @param rAction
            The functor to execute when the predicate returns true,
            e.g. when some resource has been created.
    */
    PresenterFrameworkObserver (
        const css::uno::Reference<css::drawing::framework::XConfigurationController>&rxController,
        const ::rtl::OUString& rsEventName,
        const Predicate& rPredicate,
        const Action& rAction);
    virtual ~PresenterFrameworkObserver (void);

    void Shutdown (void);

    /** Predicate that always returns true.
    */
    static bool True (void);
};

} }  // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
