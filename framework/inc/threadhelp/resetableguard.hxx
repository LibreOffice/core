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

#ifndef INCLUDED_FRAMEWORK_INC_THREADHELP_RESETABLEGUARD_HXX
#define INCLUDED_FRAMEWORK_INC_THREADHELP_RESETABLEGUARD_HXX

#include <boost/noncopyable.hpp>
#include <framework/imutex.hxx>

#include <sal/types.h>


namespace framework{

/*-************************************************************************************************************
    @short          implement a guard for implementing save thread access
    @descr          These guard has an additional feature to well known one ::osl::Guard.
                    You can lock() and unlock() it very often!
                    A set bool flag inside protect this implementation against multiple lock() calls
                    without any unlock()! So the increasing of guarded mutex couldn't be greater then 1 ...

    @attention      a) To prevent us against wrong using, the default ctor, copy ctor and the =operator are maked private!
                    b) Use interface "IMutex" of set LockHelper only - because we must support an exclusiv locking.

    @implements     -

    @devstatus      ready to use
*//*-*************************************************************************************************************/
class ResetableGuard : private boost::noncopyable
{

    //  public methods

    public:

        /*-****************************************************************************************************
            @short      ctors
            @descr      Use these ctor methods to initialize the guard right.
                        Given lock reference must be valid - otherwise crashes could occur!

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


        inline ResetableGuard( IMutex& rLock )
            :   m_pLock    ( &rLock    )
            ,   m_bLocked  ( sal_False )
        {
            lock();
        }

        /*-****************************************************************************************************
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

        /*-****************************************************************************************************
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


        inline void unlock()
        {
            if( m_bLocked == sal_True )
            {
                m_pLock->release();
                m_bLocked = sal_False;
            }
        }


    //  private methods

    private:

        /*-****************************************************************************************************
            @short      disable using of these functions!
            @descr      It's not allowed to use this methods. Different problem can occur otherwise.
                        Thats why we disable it by make it private.

            @seealso    other ctor

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        ResetableGuard();


    //  private member

    private:

        IMutex*         m_pLock     ;   /// pointer to safed lock member of user
        sal_Bool        m_bLocked   ;   /// protection against multiple lock() calls without unlock()

};      //  class ResetableGuard

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_THREADHELP_RESETABLEGUARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
