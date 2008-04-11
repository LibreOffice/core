/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: writeguard.hxx,v $
 * $Revision: 1.7 $
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

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#define __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/inoncopyable.h>
#include <threadhelp/irwlock.h>

//#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
//#include <threadhelp/threadhelpbase.hxx>
//#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          implement a guard to set write locks
    @descr          This guard should be used to set a lock for reading AND writing object internal member.
                    We never need a own mutex to safe our internal member access - because
                    a guard is used as function-local member only. There exist no multithreaded access to it realy ...

    @attention      a) To prevent us against wrong using, the default ctor, copy ctor and the =operator are maked private!
                    b) Use interface "IRWLock" of set LockHelper only - because we must support a finer granularity of locking.
                       Interface "IMutex" should be used by easier guard implementations ... like "ResetableGuard"!

    @implements     -
    @base           INonCopyable

    @devstatus      ready to use
*//*-*************************************************************************************************************/
class WriteGuard : private INonCopyable
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /*-****************************************************************************************************//**
            @short      ctor
            @descr      These ctors initialize the guard with a reference to used lock member of object to protect.
                        Null isn't allowed as value!

            @seealso    -

            @param      "pLock" ,reference to used lock member of object to protect
            @param      "rLock" ,reference to used lock member of object to protect
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        inline WriteGuard( IRWLock* pLock )
            :   m_pLock ( pLock     )
            ,   m_eMode ( E_NOLOCK  )
        {
            lock();
        }

        //*********************************************************************************************************
        inline WriteGuard( IRWLock& rLock )
            :   m_pLock ( &rLock    )
            ,   m_eMode ( E_NOLOCK  )
        {
            lock();
        }

        /*-****************************************************************************************************//**
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

        /*-****************************************************************************************************//**
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

        /*-****************************************************************************************************//**
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
                                            // => There isn't realy a write lock ...
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

        /*-****************************************************************************************************//**
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

        /*-****************************************************************************************************//**
            @short      return internal states
            @descr      For user they dont know what they are doing ...

            @seealso    -

            @param      -
            @return     Current set lock mode.

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/
        inline ELockMode getMode() const
        {
            return m_eMode;
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
        WriteGuard();

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------
    private:

        IRWLock*    m_pLock ;   /// reference to lock-member of protected object
        ELockMode   m_eMode ;   /// protection against multiple lock calls without unlock and difference between supported lock modi

};      //  class WriteGuard

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
