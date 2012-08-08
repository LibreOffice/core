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

#ifndef __FRAMEWORK_HELPER_WAKEUPTHREAD_HXX_
#define __FRAMEWORK_HELPER_WAKEUPTHREAD_HXX_

//_______________________________________________
// include files of own module

#include <threadhelp/threadhelpbase.hxx>
#include <macros/debug.hxx>
#include <macros/generic.hxx>

#include <general.h>

//_______________________________________________
// include UNO interfaces

#include <com/sun/star/util/XUpdatable.hpp>

//_______________________________________________
// include all others
#include <cppuhelper/weakref.hxx>
#include <osl/thread.hxx>


namespace framework{


//===============================================
/** @short  implements a "sleeping" thread, which try to sleep
            without a using cpu consumption :-) */
class WakeUpThread : public ThreadHelpBase
                   , public ::osl::Thread
{
    //-------------------------------------------
    // member
    private:

        /** @short  this listener will be notified if this thread
                    waked up. */
        css::uno::WeakReference< css::util::XUpdatable > m_xListener;

    //-------------------------------------------
    // interface
    public:

        /** @short  Register a new listener on this thread.

            @descr  The listener is holded as a weak reference.
                    If the thread detects, that no listener exists ...
                    he will terminate itself.
         */
        WakeUpThread(const css::uno::Reference< css::util::XUpdatable >& xListener);

        /** @descr  The thread waits on a condition using a fix timeout value.
                    If the thread wakes up he notify the internal set listener.
                    The listener can use this "timeout" info for it's own purpose.
                    The thread itself will wait on the condition again.
         */
        virtual void SAL_CALL run();

        virtual void SAL_CALL onTerminated();
};

} // namespace framework

#endif // __FRAMEWORK_HELPER_WAKEUPTHREAD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
