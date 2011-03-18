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

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#define __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_

#include <threadhelp/inoncopyable.h>
#include <framework/imutex.hxx>

#include <sal/types.h>


namespace framework{

/*-************************************************************************************************************//**
    @short          implement a guard for implementing save thread access
    @descr          These guard has an additional feature to well known one ::osl::Guard.
                    You can lock() and unlock() it very often!
                    A set bool flag inside protect this implementation against multiple lock() calls
                    without any unlock()! So the increasing of guarded mutex couldn't be greater then 1 ...

    @attention      a) To prevent us against wrong using, the default ctor, copy ctor and the =operator are maked private!
                    b) Use interface "IMutex" of set LockHelper only - because we must support an exclusiv locking.
                       Interface "IRWLock" should be used by special guard implementations ... like "ReadGuard" or "WriteGuard"!

    @implements     -
    @base           INonCopyable

    @devstatus      ready to use
*//*-*************************************************************************************************************/
class ResetableGuard : private INonCopyable
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /*-****************************************************************************************************//**
            @short      ctors
            @descr      Use these ctor methods to initialize the guard right.
                        Given lock reference must be valid - otherwise crashes could occure!

            @seealso    -

            @param      "pLock", pointer to lock helper of user
            @param      "rLock", reference to lock helper of user
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        inline ResetableGuard( IMutex* pLock )
            :   m_pLock    ( pLock     )
            ,   m_bLocked  ( sal_False )
        {
            lock();
        }

        //*********************************************************************************************************
        inline ResetableGuard( IMutex& rLock )
            :   m_pLock    ( &rLock    )
            ,   m_bLocked  ( sal_False )
        {
            lock();
        }

        /*-****************************************************************************************************//**
            @short      dtor
            @descr      We must release set mutex if programmer forget it ...

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        inline ~ResetableGuard()
        {
            unlock();
        }

        /*-****************************************************************************************************//**
            @short      enable/disable the lock
            @descr      Use this methods to lock or unlock the mutex.
                        You can do it so often you wish to do that ...

            @attention  We use another member to prevent us against multiple acquire calls of the same guard
                        without suitable release calls!
                        You don't must protect access at these bool member by using an own mutex ....
                        because nobody use the same guard instance from different threads!
                        It will be a function-local object every time.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        inline void lock()
        {
            if( m_bLocked == sal_False )
            {
                m_pLock->acquire();
                m_bLocked = sal_True;
            }
        }

        //*********************************************************************************************************
        inline void unlock()
        {
            if( m_bLocked == sal_True )
            {
                m_pLock->release();
                m_bLocked = sal_False;
            }
        }

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

        /*-****************************************************************************************************//**
            @short      disable using of these functions!
            @descr      It's not allowed to use this methods. Different problem can occure otherwise.
                        Thats why we disable it by make it private.

            @seealso    other ctor

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        ResetableGuard();

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------
    private:

        IMutex*         m_pLock     ;   /// pointer to safed lock member of user
        sal_Bool        m_bLocked   ;   /// protection against multiple lock() calls without unlock()

};      //  class ResetableGuard

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
