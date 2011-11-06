/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __FRAMEWORK_HELPER_WAKEUPTHREAD_HXX_
#define __FRAMEWORK_HELPER_WAKEUPTHREAD_HXX_

//_______________________________________________
// include files of own module

#include <threadhelp/threadhelpbase.hxx>
#include <macros/debug.hxx>
#include <macros/generic.hxx>

#ifndef __FRAMEWORK_GENERAL_HXX_
#include <general.h>
#endif

//_______________________________________________
// include UNO interfaces

#ifndef _COM_SUN_STAR_URTIL_XUPDATABLE_HPP_
#include <com/sun/star/util/XUpdatable.hpp>
#endif

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
