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

//_______________________________________________
// namespace

namespace framework{

//_______________________________________________
// definitions

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
