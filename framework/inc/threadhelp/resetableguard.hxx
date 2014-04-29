/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#define __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/inoncopyable.h>
#include <framework/imutex.hxx>

//#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
//#include <threadhelp/threadhelpbase.hxx>
//#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <sal/types.h>

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
            @descr      It's not allowed to use this methods. Different problems can occur otherwise.
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
