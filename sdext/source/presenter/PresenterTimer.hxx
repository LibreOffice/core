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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERTIMER_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERTIMER_HXX

#include <com/sun/star/awt/XCallback.hpp>
#include <com/sun/star/awt/XRequestCallback.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <osl/mutex.hxx>
#include <osl/time.h>
#include <rtl/ref.hxx>
#include <sal/types.h>

#include <functional>
#include <memory>
#include <vector>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace sdext { namespace presenter {

/** The timer allows tasks to be scheduled for execution at a specified time
    in the future.
*/
class PresenterTimer
{
public:
    /** A task is called with the current time.
    */
    typedef ::std::function<void (const TimeValue&)> Task;

    static const sal_Int32 NotAValidTaskId = 0;

    /** Schedule a task to be executed repeatedly.  The task is executed the
        first time after nFirst nano-seconds (1000000000 corresponds to one
        second).  After that task is executed in intervalls that are
        nIntervall ns long until CancelTask is called.
    */
    static sal_Int32 ScheduleRepeatedTask (
        const Task& rTask,
        const sal_Int64 nFirst,
        const sal_Int64 nIntervall);

    static void CancelTask (const sal_Int32 nTaskId);
};

typedef cppu::WeakComponentImplHelper<
    css::awt::XCallback
    > PresenterClockTimerInterfaceBase;

/** A timer that calls its listeners, typically clocks, every second to
    update their current time value.
*/
class PresenterClockTimer
    : protected ::cppu::BaseMutex,
      public PresenterClockTimerInterfaceBase
{
public:
    class Listener {
    public:
        virtual void TimeHasChanged (const oslDateTime& rCurrentTime) = 0;

    protected:
        ~Listener() {}
    };
    typedef std::shared_ptr<Listener> SharedListener;

    static ::rtl::Reference<PresenterClockTimer> Instance (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);

    void AddListener (const SharedListener& rListener);
    void RemoveListener (const SharedListener& rListener);

    static oslDateTime GetCurrentTime();

    // XCallback

    virtual void SAL_CALL notify (const css::uno::Any& rUserData)
        throw (css::uno::RuntimeException, std::exception) override;

private:
    static ::rtl::Reference<PresenterClockTimer> mpInstance;

    ::osl::Mutex maMutex;
    typedef ::std::vector<SharedListener> ListenerContainer;
    ListenerContainer maListeners;
    oslDateTime maDateTime;
    sal_Int32 mnTimerTaskId;
    bool mbIsCallbackPending;
    css::uno::Reference<css::awt::XRequestCallback> mxRequestCallback;

    PresenterClockTimer (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterClockTimer();

    void CheckCurrentTime (const TimeValue& rCurrentTime);
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
