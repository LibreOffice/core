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

/*-************************************************************************************************************
    @short      use ctor to initialize instance

    @seealso    class Guard

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

/*-************************************************************************************************************
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

/*-************************************************************************************************************
    @short      set an exclusiv lock
    @descr      We must match this lock call with current set lock type and used lock member.
                If a mutex should be used - it will be easy ... but if a rw-lock should be used
                we must simulate it as a write access!

    @attention  If a shareable osl mutex exist, he must be used as twice!
                It's necessary for some cppu-helper classes ...

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::acquire()
{
    m_pSolarMutex->acquire();
}

/*-************************************************************************************************************
    @short      release exclusiv lock
    @descr      We must match this unlock call with current set lock type and used lock member.
                If a mutex should be used - it will be easy ... but if a rw-lock should be used
                we must simulate it as a write access!

    @attention  If a shareable osl mutex exist, he must be used as twice!
                It's necessary for some cppu-helper classes ...

    @param      -
    @return     -

    @onerror    -
*//*-*************************************************************************************************************/
void LockHelper::release()
{
    m_pSolarMutex->release();
}

/*-************************************************************************************************************
    @short      return a reference to shared mutex member
    @descr      Sometimes we need a osl-mutex for sharing with our uno helper ...
                What can we do?
                We must use a different mutex member :-(
                I HOPE IT WORKS!

    @seealso    -

    @param      -
    @return     A reference to a shared mutex.

    @onerror    No error should occur.
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
