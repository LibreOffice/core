/*************************************************************************
 *
 *  $RCSfile: resetableguard.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-03-29 13:17:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#define __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_INONCOPYABLE_H_
#include <threadhelp/inoncopyable.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

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

    @attention      To prevent us against wrong using, the default ctor, copy ctor and the =operator are maked private!

    @implements     -
    @base           INonCopyAble

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class ResetableGuard : private INonCopyAble
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /*-****************************************************************************************************//**
            @short      ctors
            @descr      Use these ctor methods to initialize the guard right.
                        Given mutex reference must be valid - otherwise crashes could occure!

            @seealso    -

            @param      "pMutex"    pointer to mutex for using as lock
            @param      "rMutex"    reference to mutex for using as lock
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        ResetableGuard( ::osl::Mutex* pMutex );
        ResetableGuard( ::osl::Mutex& rMutex );

        /*-****************************************************************************************************//**
            @short      dtor
            @descr      We must release set mutex if programmer forget it ...

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        ~ResetableGuard();

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

        void lock();
        void unlock();

        /*-****************************************************************************************************//**
            @short      try to lock the mutex
            @descr      Try to acquire the mutex without blocking.

            @seealso    -

            @param      -
            @return     true, if lock already set or could new acquired
                        false, otherwise

            @onerror    No error could occure.
        *//*-*****************************************************************************************************/

        sal_Bool tryToLock();

        /*-****************************************************************************************************//**
            @short      get information about current lock state
            @descr      Use it if you not shure what going on ... but I think this never should realy neccessary!

            @seealso    -

            @param      -
            @return     true, if lock is set
                        false, otherwise

            @onerror    No error could occure!
        *//*-*****************************************************************************************************/

        sal_Bool isLocked() const;

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

        ::osl::Mutex*   m_pMutex    ;   /// pointer to safed mutex
        sal_Bool        m_bLocked   ;   /// protection against multiple lock() calls without unlock()

};      //  class ResetableGuard

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
