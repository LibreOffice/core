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

#include <threadhelp/lockhelper.hxx>
#include <general.h>

#include <macros/generic.hxx>
#include "vcl/solarmutex.hxx"

#include <osl/process.h>

namespace framework{

/*-************************************************************************************************************//**
    @short      use ctor to initialize instance

    @seealso    class ReadGuard
    @seealso    class WriteGuard

    @param      "rSolarMutex", for some components we must be "vcl-free"! So we can't work with our solar mutex
                                directly. User must set his reference at this instance - so we can work with it!
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
LockHelper::LockHelper( comphelper::SolarMutex* pSolarMutex )
    :   m_pSolarMutex       ( NULL )
    ,   m_pShareableOslMutex( NULL )
    ,   m_bDummySolarMutex  ( sal_False )
{
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
        delete m_pShareableOslMutex;
        m_pShareableOslMutex = NULL;
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
    m_pSolarMutex->acquire();
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
    m_pSolarMutex->release();
}

/*-************************************************************************************************************//**
    @interface  IRWLock
    @short      set lock for reading
    @descr      A guard should call this method to acquire read access on your member.
                Writing isn't allowed then - but nobody could check it for you!

    @attention  If a shareable osl mutex exist, he must be used as twice!
                It's neccessary for some cppu-helper classes ...

    @seealso    method releaseReadAccess()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::acquireReadAccess()
{
    m_pSolarMutex->acquire();
}

/*-************************************************************************************************************//**
    @interface  IRWLock
    @short      reset lock for reading
    @descr      A guard should call this method to release read access on your member.

    @attention  If a shareable osl mutex exist, he must be used as twice!
                It's neccessary for some cppu-helper classes ...

    @seealso    method acquireReadAccess()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::releaseReadAccess()
{
    m_pSolarMutex->release();
}

/*-************************************************************************************************************//**
    @interface  IRWLock
    @short      set lock for writing
    @descr      A guard should call this method to acquire write access on your member.
                Reading is allowed too - of course.
                After successfully calling of this method you are the only writer.

    @attention  If a shareable osl mutex exist, he must be used as twice!
                It's neccessary for some cppu-helper classes ...

    @seealso    method releaseWriteAccess()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::acquireWriteAccess()
{
    m_pSolarMutex->acquire();
}

/*-************************************************************************************************************//**
    @interface  IRWLock
    @short      reset lock for writing
    @descr      A guard should call this method to release write access on your member.

    @attention  If a shareable osl mutex exist, he must be used as twice!
                It's neccessary for some cppu-helper classes ...

    @seealso    method acquireWriteAccess()

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::releaseWriteAccess()
{
    m_pSolarMutex->release();
}

/*-************************************************************************************************************//**
    @interface  IRWLock
    @short      downgrade a write access to a read access
    @descr      A guard should call this method to change a write to a read access.
                New readers can work too - new writer are blocked!

    @attention  Ignore shareable mutex(!) - because this call never should release a lock completely!
                We change a write access to a read access only.

    @attention  a) Don't call this method if you are not a writer!
                    Results are not defined then ...
                    An upgrade can't be implemented realy ... because acquiring new access
                    will be the same - there no differences!
                b) Without function ...
                    because, a mutex don't support it realy.

    @seealso    -

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::downgradeWriteAccess()
{
    // Not supported for mutex!
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
LockHelper& LockHelper::getGlobalLock()
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
            static LockHelper aLock;
            pLock = &aLock;
        }
    }
    return *pLock;
}

/*-************************************************************************************************************//**
    @short      return a reference to shared mutex member
    @descr      Sometimes we need a osl-mutex for sharing with our uno helper ...
                What can we do?
                We must use a different mutex member :-(
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
            m_pShareableOslMutex = new ::osl::Mutex;
        }
    }
    return *m_pShareableOslMutex;
}

} //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
