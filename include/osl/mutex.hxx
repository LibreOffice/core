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

#ifndef INCLUDED_OSL_MUTEX_HXX
#define INCLUDED_OSL_MUTEX_HXX

#include "osl/mutex.h"

#include <cassert>

namespace osl
{
    /** A mutual exclusion synchronization object
    */
    class SAL_WARN_UNUSED Mutex {

    public:
        /** Create a mutex.
            @return 0 if the mutex could not be created, otherwise a handle to the mutex.
            @see ::osl_createMutex()
        */
        Mutex()
        {
            mutex = osl_createMutex();
        }

        /** Release the OS-structures and free mutex data-structure.
            @see ::osl_destroyMutex()
        */
        ~Mutex()
        {
            osl_destroyMutex(mutex);
        }

        /** Acquire the mutex, block if already acquired by another thread.
            @return false if system-call fails.
            @see ::osl_acquireMutex()
        */
        bool acquire()
        {
            return osl_acquireMutex(mutex);
        }

        /** Try to acquire the mutex without blocking.
            @return false if it could not be acquired.
            @see ::osl_tryToAcquireMutex()
        */
        bool tryToAcquire()
        {
            return osl_tryToAcquireMutex(mutex);
        }

        /** Release the mutex.
            @return false if system-call fails.
            @see ::osl_releaseMutex()
        */
        bool release()
        {
            return osl_releaseMutex(mutex);
        }

        /** Returns a global static mutex object.
            The global and static mutex object can be used to initialize other
            static objects in a thread safe manner.
            @return the global mutex object
            @see ::osl_getGlobalMutex()
        */
        static Mutex * getGlobalMutex()
        {
            return reinterpret_cast<Mutex *>(osl_getGlobalMutex());
        }

    private:
        oslMutex mutex;

        /** The underlying oslMutex has no reference count.

        Since the underlying oslMutex is not a reference counted object, copy
        constructed Mutex may work on an already destructed oslMutex object.

        */
        Mutex(const Mutex&) SAL_DELETED_FUNCTION;

        /** This assignment operator is deleted for the same reason as
            the copy constructor.
        */
        Mutex& operator= (const Mutex&) SAL_DELETED_FUNCTION;
    };

    /** Object lifetime scoped mutex object or interface lock.
     *
     * Acquires the template object on construction and releases it on
     * destruction.
     *
     * @see MutexGuard
     */
    template<class T>
    class Guard
    {
        Guard(const Guard&) SAL_DELETED_FUNCTION;
        Guard& operator=(const Guard&) SAL_DELETED_FUNCTION;

    protected:
        T * pT;

    public:
        /** Acquires the object specified as parameter.
        */
        Guard(T * pT_) : pT(pT_)
        {
            assert(pT != NULL);
            pT->acquire();
        }

        /** Acquires the object specified as parameter.
        */
        Guard(T & t) : pT(&t)
        {
            pT->acquire();
        }

        /** Releases the mutex or interface. */
        ~Guard()
        {
            pT->release();
        }
    };

    /** Object lifetime scoped mutex object or interface lock with unlock.
     *
     * Use this if you can't use scoped code blocks and Guard.
     *
     * @see ClearableMutexGuard, Guard
     */
    template<class T>
    class ClearableGuard
    {
        ClearableGuard( const ClearableGuard& ) SAL_DELETED_FUNCTION;
        ClearableGuard& operator=(const ClearableGuard&) SAL_DELETED_FUNCTION;

    protected:
        T * pT;

    public:
        /** Acquires the object specified as parameter.
        */
        ClearableGuard(T * pT_) : pT(pT_)
        {
            assert(pT != NULL);
            pT->acquire();
        }

        /** Acquires the object specified as parameter.
        */
        ClearableGuard(T & t) : pT(&t)
        {
            pT->acquire();
        }

        /** Releases the mutex or interface if not already released by clear().
        */
        ~ClearableGuard()
        {
            if (pT)
                pT->release();
        }

        /** Releases the mutex or interface.
        */
        void clear()
        {
#ifdef LIBO_INTERNAL_ONLY
            assert(pT);
#else
            if (pT)
#endif
            {
                pT->release();
                pT = NULL;
            }
        }
    };

    /** Template for temporary releasable mutex objects and interfaces locks.
     *
     * Use this if you want to acquire a lock but need to temporary release
     * it and can't use multiple scoped Guard objects.
     *
     * @see ResettableMutexGuard
     */
    template< class T >
    class ResettableGuard : public ClearableGuard< T >
    {
        ResettableGuard(const ResettableGuard&) SAL_DELETED_FUNCTION;
        ResettableGuard& operator=(const ResettableGuard&) SAL_DELETED_FUNCTION;

    protected:
        T* pResetT;

    public:
        /** Acquires the object specified as parameter.
        */
        ResettableGuard( T* pT_ ) :
                ClearableGuard<T>( pT_ ),
                pResetT( pT_ )
        {}

        /** Acquires the object specified as parameter.
        */
        ResettableGuard( T& rT ) :
                ClearableGuard<T>( rT ),
                pResetT( &rT )
        {}

        /** Re-acquires the mutex or interface.
        */
        void reset()
        {
#ifdef LIBO_INTERNAL_ONLY
            assert(!this->pT);
#endif
            if (pResetT)
            {
                this->pT = pResetT;
                this->pT->acquire();
            }
        }
    };

    typedef Guard<Mutex> MutexGuard;
    typedef ClearableGuard<Mutex> ClearableMutexGuard;
    typedef ResettableGuard< Mutex > ResettableMutexGuard;
}

#endif // INCLUDED_OSL_MUTEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
