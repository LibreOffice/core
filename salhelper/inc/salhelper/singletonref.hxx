/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: singletonref.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 13:23:11 $
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

#ifndef _SALHELPER_SINGLETONREF_HXX_
#define _SALHELPER_SINGLETONREF_HXX_

//_______________________________________________
// includes

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef INCLUDED_RTL_INSTANCE_HXX
#include "rtl/instance.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include "osl/diagnose.h"
#endif

#ifndef INCLUDED_OSL_GETGLOBALMUTEX_HXX
#include "osl/getglobalmutex.hxx"
#endif

//_______________________________________________
// namespace

namespace salhelper{

//_______________________________________________
// definitions

/** @short  template for implementing singleton classes.

    @descr  Such classes can be instanciated everytimes they
            are needed. But the internal wrapped object will
            be created one times only. Of course its used
            resources are referenced one times only too.
            This template hold it alive till the last
            reference is gone. Further all operations
            on this reference are threadsafe. Only
            calls directly to the internal object (which modify
            its state) must be made threadsafe by the object itself
            or from outside.

    @attention  To prevent the code against race conditions, its not
                allowed to start operations inside the ctor
                of the internal wrapped object - especialy operations
                which needs a reference to the same singleton too.

                The only chance to supress such strange constellations
                is a lazy-init mechanism.

                <ul>
                    <li>a) The singleton class can provide a special init()
                           method, which must be called as first after creation.</li>
                    <li>b) The singleton class can call a special impl_init()
                           method implicit for every called interface method.</li>
                </ul>

                Note further that this singleton pattern can work only, if
                all user of such singleton are located inside the same library!
                Because static values cant be exported - e.g. from windows libraries.
 */
template< class SingletonClass >
class SingletonRef
{
    //-------------------------------------------
    // member

    private :

        /** @short  pointer to the internal wrapped singleton. */
        static SingletonClass* m_pInstance;

        /** @short  ref count, which regulate creation and removing of m_pInstance. */
        static sal_Int32 m_nRef;

    //-------------------------------------------
    // interface

    public :

        //---------------------------------------

        /** @short  standard ctor.

            @descr  The internal wrapped object is created only,
                    if its ref count was 0. Otherwhise this method
                    does nothing ... except increasing of the internal
                    ref count!
         */
        SingletonRef()
        {
            // GLOBAL SAFE ->
            ::osl::MutexGuard aLock(SingletonRef::ownStaticLock());

            // must be increased before(!) the check is done.
            // Otherwhise this check can fail inside the same thread ...
            ++m_nRef;
            if (m_nRef == 1)
                m_pInstance = new SingletonClass();

            OSL_ENSURE(m_nRef>0 && m_pInstance, "Race? Ref count of singleton >0, but instance is NULL!");
            // <- GLOBAL SAFE
        }

        //---------------------------------------

        /** @short  standard dtor.

            @descr  The internal wrapped object is removed only,
                    if its ref count wil be 0. Otherwhise this method
                    does nothing ... except decreasing of the internal
                    ref count!
         */
        ~SingletonRef()
        {
            // GLOBAL SAFE ->
            ::osl::MutexGuard aLock(SingletonRef::ownStaticLock());

            // must be decreased before(!) the check is done.
            // Otherwhise this check can fail inside the same thread ...
            --m_nRef;
            if (m_nRef == 0)
            {
                delete m_pInstance;
                m_pInstance = 0;
            }
            // <- GLOBAL SAFE
        }

        //---------------------------------------

        /** @short  Allows rSingle->someBodyOp().
         */
        SingletonClass* operator->() const
        {
            // GLOBAL SAFE ->
            ::osl::MutexGuard aLock(SingletonRef::ownStaticLock());
            return m_pInstance;
            // <- GLOBAL SAFE
        }

        //---------------------------------------

        /** @short  Allows (*rSingle).someBodyOp().
         */
        SingletonClass& operator*() const
        {
            // GLOBAL SAFE ->
            ::osl::MutexGuard aLock(SingletonRef::ownStaticLock());
            return *m_pInstance;
            // <- GLOBAL SAFE
        }

    //-------------------------------------------
    // helper

    private :

        //---------------------------------------

        /** @short  creates an own mutex for guarding static contents.

            @descr  The global mutex the osl library is used one times
                    only to create an own static mutex, which can be used
                    next time to guard own static member operations.
         */
        struct SingletonLockInit
        {
            ::osl::Mutex* operator()()
            {
                static ::osl::Mutex aInstance;
                return &aInstance;
            }
        };

        ::osl::Mutex& ownStaticLock() const
        {
            return *rtl_Instance< ::osl::Mutex,
                                  SingletonLockInit,
                                  ::osl::MutexGuard,
                                  ::osl::GetGlobalMutex >::create(SingletonLockInit(), ::osl::GetGlobalMutex());
        }
};

template< class SingletonClass >
SingletonClass* SingletonRef< SingletonClass >::m_pInstance = 0;

template< class SingletonClass >
sal_Int32 SingletonRef< SingletonClass >::m_nRef = 0;

} // namespace salhelper

#endif // _SALHELPER_SINGLETONREF_HXX_
