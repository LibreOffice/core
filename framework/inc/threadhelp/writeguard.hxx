/*************************************************************************
 *
 *  $RCSfile: writeguard.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2001-04-04 13:28:33 $
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

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#define __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_INONCOPYABLE_H_
#include <threadhelp/inoncopyable.h>
#endif

#ifndef __FRAMEWORK_THREADHELP_IRWLOCK_H_
#include <threadhelp/irwlock.h>
#endif

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

    @attention      1) To prevent us against wrong using, the default ctor, copy ctor and the =operator are maked private!
                    2) Every method support a return value "eReason". Use this value to react for refused lock-calls!
                       this means: You can't work with this object! We are not ready for working yet ...
                       React for that in a right way.
                       (see irwlock.h for further informations)

    @implements     -
    @base           INonCopyAble

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class WriteGuard : private INonCopyAble
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /*-****************************************************************************************************//**
            @short      ctor
            @descr      These ctors initialize the guard with a reference to used lock member of object to protect.
                        Null isn't allowed as value!

            @attention  If eRefusalReason is different from E_WORK

            @seealso    -

            @param      "pLock",    reference to used lock member of object to protect
            @param      "rLock",    reference to used lock member of object to protect
            @param      "eReason",  return value for working mode
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        WriteGuard( IRWLock*        pLock   ,
                    ERejectReason&  eReason );

        WriteGuard( IRWLock&        rLock   ,
                    ERejectReason&  eReason );

        /*-****************************************************************************************************//**
            @short      dtor
            @descr      We unlock the used lock member automaticly if user forget it.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        ~WriteGuard();

        /*-****************************************************************************************************//**
            @short      set write lock
            @descr      Call this method to set the write lock. The call will block till all current threads are synchronized!

            @attention  You have to check "eReason". If is different from E_NONE your access to our member was refused!

            @seealso    method unlock()

            @param      "eReason", return the reason for refused calls
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void lock( ERejectReason& eReason );

        /*-****************************************************************************************************//**
            @short      unset write lock
            @descr      Call this method to unlock the rw-lock temp.!
                        Normaly we do it at dtor automaticly for you ...

            @seealso    method lock()

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void unlock();

        /*-****************************************************************************************************//**
            @short      downgrade write access to read access without new blocking!
            @descr      If this write lock is set you can change it to a "read lock".
                        An "upgrade" is the same like new calling "lock()"!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void downgrade();

        /*-****************************************************************************************************//**
            @short      return internal lock state
            @descr      For user they dont know what they are doing there ...

            @seealso    -

            @param      -
            @return     Current set lock mode.

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        ELockMode getMode() const;

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

        IRWLock*    m_pLock ;   /// refrence to lock-member of protected object
        ELockMode   m_eMode ;   /// protection against multiple lock calls without unlock and difference between supported lock modi

};      //  class WriteGuard

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
