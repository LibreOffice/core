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

#ifndef _OSL_MUTEX_HXX_
#define _OSL_MUTEX_HXX_

#ifdef __cplusplus

#include <osl/mutex.h>


namespace osl
{
    /** A mutual exclusion synchronization object
    */
    class SAL_WARN_UNUSED Mutex {

    public:
        /** Create a thread-local mutex.
            @return 0 if the mutex could not be created, otherwise a handle to the mutex.
            @seealso ::osl_createMutex()
        */
        Mutex()
        {
            mutex = osl_createMutex();
        }

        /** Release the OS-structures and free mutex data-structure.
            @seealso ::osl_destroyMutex()
        */
        ~Mutex()
        {
            osl_destroyMutex(mutex);
        }

        /** Acquire the mutex, block if already acquired by another thread.
            @return sal_False if system-call fails.
            @seealso ::osl_acquireMutex()
        */
        sal_Bool acquire()
        {
            return osl_acquireMutex(mutex);
        }

        /** Try to acquire the mutex without blocking.
            @return sal_False if it could not be acquired.
            @seealso ::osl_tryToAcquireMutex()
        */
        sal_Bool tryToAcquire()
        {
            return osl_tryToAcquireMutex(mutex);
        }

        /** Release the mutex.
            @return sal_False if system-call fails.
            @seealso ::osl_releaseMutex()
        */
        sal_Bool release()
        {
            return osl_releaseMutex(mutex);
        }

        /** Returns a global static mutex object.
            The global and static mutex object can be used to initialize other
            static objects in a thread safe manner.
            @return the global mutex object
            @seealso ::osl_getGlobalMutex()
        */
        static Mutex * getGlobalMutex()
        {
            return (Mutex *)osl_getGlobalMutex();
        }

    private:
        oslMutex mutex;

        /** The underlying oslMutex has no reference count.

        Since the underlying oslMutex is not a reference counted object, copy
        constructed Mutex may work on an already destructed oslMutex object.

        */
        Mutex(const Mutex&);

        /** The underlying oslMutex has no reference count.

        When destructed, the Mutex object destroys the undelying oslMutex,
        which might cause severe problems in case it's a temporary object.

        */
        Mutex(oslMutex Mutex);

        /** This assignment operator is private for the same reason as
            the copy constructor.
        */
        Mutex& operator= (const Mutex&);

        /** This assignment operator is private for the same reason as
            the constructor taking a oslMutex argument.
        */
        Mutex& operator= (oslMutex);
    };

    /** A helper class for mutex objects and interfaces.
    */
    template<class T>
    class Guard
    {
    private:
        Guard( const Guard& );
        const Guard& operator = ( const Guard& );

    protected:
        T * pT;
    public:

        /** Acquires the object specified as parameter.
        */
        Guard(T * pT_) : pT(pT_)
        {
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

    /** A helper class for mutex objects and interfaces.
    */
    template<class T>
    class ClearableGuard
    {
    private:
        ClearableGuard( const ClearableGuard& );
        const ClearableGuard& operator = ( const ClearableGuard& );
    protected:
        T * pT;
    public:

        /** Acquires the object specified as parameter.
        */
        ClearableGuard(T * pT_) : pT(pT_)
        {
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
            if(pT)
            {
                pT->release();
                pT = NULL;
            }
        }
    };

    /** A helper class for mutex objects and interfaces.
    */
    template< class T >
    class ResettableGuard : public ClearableGuard< T >
    {
    private:
        ResettableGuard(ResettableGuard &); // not defined
        void operator =(ResettableGuard &); // not defined

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

        /** Re-aquires the mutex or interface.
        */
        void reset()
        {
            if( pResetT )
            {
                this->pT = pResetT;
                this->pT->acquire();
            }
        }
    };

    typedef Guard<Mutex> MutexGuard;
    typedef ClearableGuard<Mutex> ClearableMutexGuard;
    typedef ResettableGuard< Mutex > ResettableMutexGuard;

    /** SolarMutex interface, needed for SolarMutex.
        Deprecated, used just for Application::GetSolarMutex().
    */
    class SolarMutex
    {
        public:
            /** Blocks if mutex is already in use
            */
            virtual void SAL_CALL acquire() = 0;

            /** Tries to get the mutex without blocking.
            */
            virtual sal_Bool SAL_CALL tryToAcquire() = 0;

            /** Releases the mutex.
            */
            virtual void SAL_CALL release() = 0;

        protected:
            SolarMutex() {}
            virtual ~SolarMutex() {}
    };
    typedef osl::Guard< SolarMutex > SolarGuard;
    typedef osl::ClearableGuard< SolarMutex > ClearableSolarGuard;
    typedef osl::ResettableGuard< SolarMutex > ResettableSolarGuard;
}

#endif  /* __cplusplus */
#endif  /* _OSL_MUTEX_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
