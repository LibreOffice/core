/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wakeupthread.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:20:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef __FRAMEWORK_HELPER_WAKEUPTHREAD_HXX_
#define __FRAMEWORK_HELPER_WAKEUPTHREAD_HXX_

//_______________________________________________
// include files of own module

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

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

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif

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
