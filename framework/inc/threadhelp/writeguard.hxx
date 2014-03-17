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

#ifndef INCLUDED_FRAMEWORK_INC_THREADHELP_WRITEGUARD_HXX
#define INCLUDED_FRAMEWORK_INC_THREADHELP_WRITEGUARD_HXX

#include <boost/noncopyable.hpp>
#include <threadhelp/lockhelper.hxx>


namespace framework{

/*-************************************************************************************************************
    @short          implement a guard to set write locks
    @descr          This guard should be used to set a lock for reading AND writing object internal member.
                    We never need a own mutex to safe our internal member access - because
                    a guard is used as function-local member only. There exist no multithreaded access to it really ...

    @attention      To prevent us against wrong using, the default ctor, copy ctor and the =operator are maked private!

    @implements     -

    @devstatus      ready to use
*//*-*************************************************************************************************************/
class WriteGuard : private boost::noncopyable
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
        inline WriteGuard( LockHelper* pLock )
            :   m_pLock ( pLock     )
            ,   m_eMode ( E_NOLOCK  )
        {
            lock();
        }


        inline WriteGuard( LockHelper& rLock )
            :   m_pLock ( &rLock    )
            ,   m_eMode ( E_NOLOCK  )
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
        inline ~WriteGuard()
        {
            unlock();
        }

        /*-****************************************************************************************************
            @short      set write lock
            @descr      Call this method to set the write lock. The call will block till all current threads are synchronized!

            @seealso    method unlock()

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        inline void lock()
        {
            switch( m_eMode )
            {
                case E_NOLOCK       :   {
                                            // Acquire write access and set return state.
                                            // Mode is set later if it was successful!
                                            m_pLock->acquireWriteAccess();
                                            m_eMode = E_WRITELOCK;
                                        }
                                        break;
                case E_READLOCK     :   {
                                            // User has downgrade to read access before!
                                            // We must release it before we can set a new write access!
                                            m_pLock->releaseReadAccess();
                                            m_pLock->acquireWriteAccess();
                                            m_eMode = E_WRITELOCK;
                                        }
                                        break;
                default:                break; // nothing to do
            }
        }

        /*-****************************************************************************************************
            @short      unset write lock
            @descr      Call this method to unlock the rw-lock temp.!
                        Normaly we do it at dtor automaticly for you ...

            @seealso    method lock()

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        inline void unlock()
        {
            switch( m_eMode )
            {
                case E_READLOCK     :   {
                                            // User has downgraded to a read lock before!
                                            // => There isn't really a write lock ...
                                            m_pLock->releaseReadAccess();
                                            m_eMode = E_NOLOCK;
                                        }
                                        break;
                case E_WRITELOCK    :   {
                                            m_pLock->releaseWriteAccess();
                                            m_eMode = E_NOLOCK;
                                        }
                                        break;
                default:                break; // nothing to do
            }
        }

        /*-****************************************************************************************************
            @short      downgrade write access to read access without new blocking!
            @descr      If this write lock is set you can change it to a "read lock".
                        An "upgrade" is the same like new calling "lock()"!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        inline void downgrade()
        {
            if( m_eMode == E_WRITELOCK )
            {
                m_pLock->downgradeWriteAccess();
                m_eMode = E_READLOCK;
            }
        }

        /*-****************************************************************************************************
            @short      return internal states
            @descr      For user they dont know what they are doing ...

            @seealso    -

            @param      -
            @return     Current set lock mode.

            @onerror    No error should occur.
        *//*-*****************************************************************************************************/
        inline ELockMode getMode() const
        {
            return m_eMode;
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
        WriteGuard();


    //  private member

    private:

        LockHelper*    m_pLock ;   /// reference to lock-member of protected object
        ELockMode   m_eMode ;   /// protection against multiple lock calls without unlock and difference between supported lock modi

};      //  class WriteGuard

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_THREADHELP_WRITEGUARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
