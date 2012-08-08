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

#include <threadhelp/lockhelper.hxx>
#include <general.h>
#include <macros/debug.hxx>

#include <macros/generic.hxx>
#include "vcl/solarmutex.hxx"

#include <osl/process.h>

namespace framework{

/*-************************************************************************************************************//**
    @short      use ctor to initialize instance
    @descr      We must initialize our member "m_eLockType". This value specify handling of locking.
                User use this helper as parameter for a guard creation.
                These guard use "m_eLockType" to set lock in the right way by using right mutex or rw-lock.

    @seealso    enum ELockType
    @seealso    class ReadGuard
    @seealso    class WriteGuard

    @param      "rSolarMutex", for some components we must be "vcl-free"! So we can't work with our solar mutex
                                directly. User must set his reference at this instance - so we can work with it!
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
LockHelper::LockHelper( ::osl::SolarMutex* pSolarMutex )
    :   m_pFairRWLock       ( NULL )
    ,   m_pOwnMutex         ( NULL )
    ,   m_pSolarMutex       ( NULL )
    ,   m_pShareableOslMutex( NULL )
    ,   m_bDummySolarMutex  ( sal_False )
{
    m_eLockType = implts_getLockType();
    switch( m_eLockType )
    {
        case E_NOTHING      :   break; // There is nothing to do ...
        case E_OWNMUTEX     :   {
                                    m_pOwnMutex = new ::osl::Mutex;
                                }
                                break;
        case E_SOLARMUTEX   :   {
            if( pSolarMutex == NULL )
            {
                m_pSolarMutex      = new ::vcl::SolarMutexObject;
                m_bDummySolarMutex = sal_True;
            }
            else
            {
                m_pSolarMutex = pSolarMutex;
            }
        }
            break;
        case E_FAIRRWLOCK   :   {
                                    m_pFairRWLock = new FairRWLock;
                                }
                                break;
        #ifdef ENABLE_ASSERTIONS
        default             :   LOG_ASSERT2( m_eLockType!=E_NOTHING, "LockHelper::ctor()", "Invalid lock type found .. so code will not be threadsafe!" )
        #endif
    }
}

/*-************************************************************************************************************//**
    @short      default dtor to release safed pointer
    @descr      We have created dynamical mutex- or lock-member ... or we hold a pointer to external objects.
                We must release it!

    @seealso    ctor()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
LockHelper::~LockHelper()
{
    if( m_pShareableOslMutex != NULL )
    {
        // Sometimes we hold two pointer to same object!
        // (e.g. if m_eLockType==E_OWNMUTEX!)
        // So we should forget it ... but don't delete it twice!
        if( m_pShareableOslMutex != m_pOwnMutex )
        {
            delete m_pShareableOslMutex;
        }
        m_pShareableOslMutex = NULL;
    }
    if( m_pOwnMutex != NULL )
    {
        delete m_pOwnMutex;
        m_pOwnMutex = NULL;
    }
    if( m_pSolarMutex != NULL )
    {
        if (m_bDummySolarMutex)
        {
            delete static_cast<vcl::SolarMutexObject*>(m_pSolarMutex);
            m_bDummySolarMutex = sal_False;
        }
        m_pSolarMutex = NULL;
    }
    if( m_pFairRWLock != NULL )
    {
        delete m_pFairRWLock;
        m_pFairRWLock = NULL;
    }
}

/*-************************************************************************************************************//**
    @interface  IMutex
    @short      set an exclusiv lock
    @descr      We must match this lock call with current set lock type and used lock member.
                If a mutex should be used - it will be easy ... but if a rw-lock should be used
                we must simulate it as a write access!

    @attention  If a shareable osl mutex exist, he must be used as twice!
                It's neccessary for some cppu-helper classes ...

    @seealso    method acquireWriteAccess()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::acquire()
{
    switch( m_eLockType )
    {
        case E_NOTHING      :   break; // There is nothing to do ...
        case E_OWNMUTEX     :   {
                                    m_pOwnMutex->acquire();
                                }
                                break;
        case E_SOLARMUTEX   :   {
                                    m_pSolarMutex->acquire();
                                }
                                break;
        case E_FAIRRWLOCK   :   {
                                    m_pFairRWLock->acquireWriteAccess();
                                }
                                break;
    }
}

/*-************************************************************************************************************//**
    @interface  IMutex
    @short      release exclusiv lock
    @descr      We must match this unlock call with current set lock type and used lock member.
                If a mutex should be used - it will be easy ... but if a rw-lock should be used
                we must simulate it as a write access!

    @attention  If a shareable osl mutex exist, he must be used as twice!
                It's neccessary for some cppu-helper classes ...

    @seealso    method releaseWriteAccess()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::release()
{
    switch( m_eLockType )
    {
        case E_NOTHING      :   break; // There is nothing to do ...
        case E_OWNMUTEX     :   {
                                    m_pOwnMutex->release();
                                }
                                break;
        case E_SOLARMUTEX   :   {
                                    m_pSolarMutex->release();
                                }
                                break;
        case E_FAIRRWLOCK   :   {
                                    m_pFairRWLock->releaseWriteAccess();
                                }
                                break;
    }
}

/*-************************************************************************************************************//**
    @interface  IRWLock
    @short      set lock for reading
    @descr      A guard should call this method to acquire read access on your member.
                Writing isn't allowed then - but nobody could check it for you!
                We use m_eLockType to differ between all possible "lock-member"!!!

    @attention  If a shareable osl mutex exist, he must be used as twice!
                It's neccessary for some cppu-helper classes ...

    @seealso    method releaseReadAccess()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::acquireReadAccess()
{
    switch( m_eLockType )
    {
        case E_NOTHING      :   break; // There is nothing to do ...
        case E_OWNMUTEX     :   {
                                    m_pOwnMutex->acquire();
                                }
                                break;
        case E_SOLARMUTEX   :   {
                                    m_pSolarMutex->acquire();
                                }
                                break;
        case E_FAIRRWLOCK   :   {
                                    m_pFairRWLock->acquireReadAccess();
                                }
                                break;
    }
}

/*-************************************************************************************************************//**
    @interface  IRWLock
    @short      reset lock for reading
    @descr      A guard should call this method to release read access on your member.
                We use m_eLockType to differ between all possible "lock-member"!!!

    @attention  If a shareable osl mutex exist, he must be used as twice!
                It's neccessary for some cppu-helper classes ...

    @seealso    method acquireReadAccess()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::releaseReadAccess()
{
    switch( m_eLockType )
    {
        case E_NOTHING      :   break; // There is nothing to do ...
        case E_OWNMUTEX     :   {
                                    m_pOwnMutex->release();
                                }
                                break;
        case E_SOLARMUTEX   :   {
                                    m_pSolarMutex->release();
                                }
                                break;
        case E_FAIRRWLOCK   :   {
                                    m_pFairRWLock->releaseReadAccess();
                                }
                                break;
    }
}

/*-************************************************************************************************************//**
    @interface  IRWLock
    @short      set lock for writing
    @descr      A guard should call this method to acquire write access on your member.
                Reading is allowed too - of course.
                After successfully calling of this method you are the only writer.
                We use m_eLockType to differ between all possible "lock-member"!!!

    @attention  If a shareable osl mutex exist, he must be used as twice!
                It's neccessary for some cppu-helper classes ...

    @seealso    method releaseWriteAccess()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::acquireWriteAccess()
{
    switch( m_eLockType )
    {
        case E_NOTHING      :   break; // There is nothing to do ...
        case E_OWNMUTEX     :   {
                                    m_pOwnMutex->acquire();
                                }
                                break;
        case E_SOLARMUTEX   :   {
                                    m_pSolarMutex->acquire();
                                }
                                break;
        case E_FAIRRWLOCK   :   {
                                    m_pFairRWLock->acquireWriteAccess();
                                }
                                break;
    }
}

/*-************************************************************************************************************//**
    @interface  IRWLock
    @short      reset lock for writing
    @descr      A guard should call this method to release write access on your member.
                We use m_eLockType to differ between all possible "lock-member"!!!

    @attention  If a shareable osl mutex exist, he must be used as twice!
                It's neccessary for some cppu-helper classes ...

    @seealso    method acquireWriteAccess()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::releaseWriteAccess()
{
    switch( m_eLockType )
    {
        case E_NOTHING      :   break; // There is nothing to do ...
        case E_OWNMUTEX     :   {
                                    m_pOwnMutex->release();
                                }
                                break;
        case E_SOLARMUTEX   :   {
                                    m_pSolarMutex->release();
                                }
                                break;
        case E_FAIRRWLOCK   :   {
                                    m_pFairRWLock->releaseWriteAccess();
                                }
                                break;
    }
}

/*-************************************************************************************************************//**
    @interface  IRWLock
    @short      downgrade a write access to a read access
    @descr      A guard should call this method to change a write to a read access.
                New readers can work too - new writer are blocked!
                We use m_eLockType to differ between all possible "lock-member"!!!

    @attention  Ignore shareable mutex(!) - because this call never should release a lock completly!
                We change a write access to a read access only.

    @attention  a) Don't call this method if you are not a writer!
                    Results are not defined then ...
                    An upgrade can't be implemented realy ... because acquiring new access
                    will be the same - there no differences!
                b) Without function if m_eLockTyp is different from E_FAIRRWLOCK(!) ...
                    because, a mutex don't support it realy.

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::downgradeWriteAccess()
{
    switch( m_eLockType )
    {
        case E_NOTHING      :   break; // There is nothing to do ...
        case E_OWNMUTEX     :   break; // Not supported for mutex!
        case E_SOLARMUTEX   :   break; // Not supported for mutex!
        case E_FAIRRWLOCK   :   m_pFairRWLock->downgradeWriteAccess();
                                break;
    }
}

/*-************************************************************************************************************//**
    @short      return a reference to a static lock helper
    @descr      Sometimes we need the global mutex or rw-lock! (e.g. in our own static methods)
                But it's not a good idea to use these global one very often ...
                Thats why we use this little helper method.
                We create our own "class global static" lock.
                It will be created at first call only!
                All other requests use these created one then directly.

    @seealso    -

    @param      -
    @return     A reference to a static mutex/lock member.

    @onerror    No error should occure.
*//*-*************************************************************************************************************/
LockHelper& LockHelper::getGlobalLock( ::osl::SolarMutex* pSolarMutex )
{
    // Initialize static "member" only for one time!
    // Algorithm:
    // a) Start with an invalid lock (NULL pointer)
    // b) If these method first called (lock not already exist!) ...
    // c) ... we must create a new one. Protect follow code with the global mutex -
    //    (It must be - we create a static variable!)
    // d) Check pointer again - because ... another instance of our class could be faster then these one!
    // e) Create the new lock and set it for return on static variable.
    // f) Return new created or already existing lock object.
    static LockHelper* pLock = NULL;
    if( pLock == NULL )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( pLock == NULL )
        {
            static LockHelper aLock( pSolarMutex );
            pLock = &aLock;
        }
    }
    return *pLock;
}

/*-************************************************************************************************************//**
    @short      return a reference to shared mutex member
    @descr      Sometimes we need a osl-mutex for sharing with our uno helper ...
                What can we do?
                a) If we have an initialized "own mutex" ... we can use it!
                b) Otherwhise we must use a different mutex member :-(
                I HOPE IT WORKS!

    @seealso    -

    @param      -
    @return     A reference to a shared mutex.

    @onerror    No error should occure.
*//*-*************************************************************************************************************/
::osl::Mutex& LockHelper::getShareableOslMutex()
{
    if( m_pShareableOslMutex == NULL )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( m_pShareableOslMutex == NULL )
        {
            switch( m_eLockType )
            {
                case E_OWNMUTEX     :   {
                                            m_pShareableOslMutex = m_pOwnMutex;
                                        }
                                        break;
                default             :   {
                                            m_pShareableOslMutex = new ::osl::Mutex;
                                        }
                                        break;
            }
        }
    }
    return *m_pShareableOslMutex;
}

/*-************************************************************************************************************//**
    @short      search for right lock type, which should be used by an instance of this struct
    @descr      We must initialize our member "m_eLockType". This value specify handling of locking.
                How we can do that? We search for an environment variable. We do it only for one time ....
                because the environment is fix. So we safe this value and use it for all further requests.
                If no variable could be found - we use a fallback!

    @attention  We have numbered all our enum values for ELockType. So we can use it as value of searched
                environment variable too!

    @seealso    enum ELockType
    @seealso    environment LOCKTYPE

    @param      -
    @return     A reference to a created and right initialized lock type!

    @onerror    We use a fallback!
*//*-*************************************************************************************************************/
ELockType& LockHelper::implts_getLockType()
{
    // Initialize static "member" only for one time!
    // Algorithm:
    // a) Start with an invalid variable (NULL pointer)
    // b) If these method first called (value not already exist!) ...
    // c) ... we must create a new one. Protect follow code with the global mutex -
    //    (It must be - we create a static variable!)
    // d) Check pointer again - because ... another instance of our class could be faster then these one!
    // e) Create the new static variable, get value from the environment and set it
    // f) Return new created or already existing static variable.
    static ELockType* pType = NULL;
    if( pType == NULL )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( pType == NULL )
        {
            static ELockType eType = FALLBACK_LOCKTYPE;

            ::rtl::OUString     aEnvVar( ENVVAR_LOCKTYPE );
            ::rtl::OUString     sValue      ;
            if( osl_getEnvironment( aEnvVar.pData, &sValue.pData ) == osl_Process_E_None )
            {
                eType = (ELockType)(sValue.toInt32());
            }

            LOG_LOCKTYPE( FALLBACK_LOCKTYPE, eType )

            pType = &eType;
        }
    }
    return *pType;
}

} //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
