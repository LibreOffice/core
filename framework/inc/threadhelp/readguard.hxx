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

#ifndef INCLUDED_FRAMEWORK_INC_THREADHELP_READGUARD_HXX
#define INCLUDED_FRAMEWORK_INC_THREADHELP_READGUARD_HXX

#include <boost/noncopyable.hpp>
#include <threadhelp/lockhelper.hxx>

#include <sal/types.h>


namespace framework{

/*-************************************************************************************************************
    @short          implement a guard to set read locks
    @descr          This guard should be used to set a lock for reading object internal member.
                    Nobody can control it but don't use member after successfully locking for writing!
                    We never need a own mutex to safe our internal member access - because
                    a guard is used as function-local member only. There exist no multithreaded access to it really ...

    @attention      To prevent us against wrong using, the default ctor, copy ctor and the =operator are maked private!

    @implements     -

    @devstatus      ready to use
*//*-*************************************************************************************************************/
class ReadGuard : private boost::noncopyable
{

    //  public methods

    public:

        /*-****************************************************************************************************
            @short      ctor
            @descr      These ctors initialize the guard with a reference to used lock member of object to protect.
                        Null isn't allowed as value!

            @seealso    -

            @param      "pLock" ,reference to used lock member of object to protect
            @param      "rLock" ,reference to used lock member of object to protect
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        inline ReadGuard( LockHelper* pLock )
            :   m_pLock     ( pLock     )
            ,   m_bLocked   ( sal_False )
        {
            lock();
        }


        inline ReadGuard( LockHelper& rLock )
            :   m_pLock     ( &rLock    )
            ,   m_bLocked   ( sal_False )
        {
            lock();
        }

        /*-****************************************************************************************************
            @short      dtor
            @descr      We unlock the used lock member automaticly if user forget it.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        inline ~ReadGuard()
        {
            unlock();
        }

        /*-****************************************************************************************************
            @short      set read lock
            @descr      Call this method to set the read lock. The call will block till all current threads are synchronized!

            @seealso    method unlock()

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        inline void lock()
        {
            if( m_bLocked == sal_False )
            {
                m_pLock->acquireReadAccess();
                m_bLocked = sal_True;
            }
        }

        /*-****************************************************************************************************
            @short      unset read lock
            @descr      Call this method to unlock the rw-lock temp.!
                        Normaly we do it at dtor automaticly for you ...

            @seealso    method lock()

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        inline void unlock()
        {
            if( m_bLocked == sal_True )
            {
                m_pLock->releaseReadAccess();
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
        ReadGuard();


    //  private member

    private:

        LockHelper*    m_pLock     ;   /// reference to lock-member of protected object
        sal_Bool    m_bLocked   ;   /// protection against multiple lock calls without unlock!

};      //  class ReadGuard

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_THREADHELP_READGUARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
