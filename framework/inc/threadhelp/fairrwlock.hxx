/*************************************************************************
 *
 *  $RCSfile: fairrwlock.hxx,v $
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

#ifndef __FRAMEWORK_THREADHELP_FAIRRWLOCK_HXX_
#define __FRAMEWORK_THREADHELP_FAIRRWLOCK_HXX_

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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
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
    @short          implement a read/write lock with fairness between read/write accessors
    @descr          These implementation never should used as base class! Use it as a member every time.
                    Use ReadGuard and/or WriteGuard in your methods (which work with these lock)
                    to make your code threadsafe.
                    Fair means: All reading or writing threads are synchronized AND serialzed by using one
                    mutex. For reader this mutex is used to access internal variables of this lock only;
                    for writer this mutex is used to have an exclusiv access on your class member!
                    => It's a multi-reader/single-writer lock, which no preferred accessor.

    @implements     -
    @base           IRWLock

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class FairRWLock    :   private INonCopyAble
                    ,   public  IRWLock
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /*-****************************************************************************************************//**
            @short      standard ctor
            @descr      Initialize instance with right start values for correct working.
                        no reader could exist               =>  m_nReadCount   = 0
                        user initialize himself             =>  m_eWorkingMode = E_INIT
                        don't block first comming writer    =>  m_aWriteCondition.set()

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        FairRWLock();

        /*-****************************************************************************************************//**
            @short      set new working mode for this lock
            @descr      These lock implementation knows three states of working: E_INIT, E_WORK, E_CLOSE
                        You can step during this ones only from the left to the right side and start at left side again!
                        (This is neccessary e.g. for refcounted objects!)
                        Follow results occure:
                            E_INIT  :   All requests on this lock are refused.
                                        It's your decision to react in a right way.
                                        Otherwise this call will block till write access to internal member is available.

                            E_WORK  :   The object work now. The full functionality is available.
                                        This set-call will block till write access to internal member is available.

                            E_CLOSE :   All further requests on this lock will be refused.
                                        It's your decision to react in a right way.
                                        This set-call will block till write access to internal member is available AND
                                        all current reader or writer are gone!

            @seealso    -

            @param      "eMode", is the new mode - but we don't accept setting mode in wrong order!
            @return     -

            @onerror    We do nothing.
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setWorkingMode( EWorkingMode eMode );

        /*-****************************************************************************************************//**
            @short      get current working mode
            @descr      If you stand in your close() or init() method ... but don't know
                        if you called more then ones(!) ... you can use this function to get
                        right information.
                        e.g:    You have a method init() which is used to change working mode from
                                E_INIT to E_WORK and should be used to initialize some member too ...
                                What should you do:
                                    void init( sal_Int32 nValue )
                                    {
                                        // Best place to initialize internal member is before you call
                                        // setWorkingMode() ... but if somewhere call this function
                                        // more then ones ...
                                        // => check current mode before!

                                        if( m_aLock.getWorkingMode() == E_INIT )
                                        {
                                            // OK - This is the first call of init().
                                            // Set new value and change mode then.

                                            m_nMember = nValue;
                                            m_aLock.setWorkingMode( E_WORK );

                                            // After that it's not a good idea to work with internal member
                                            // without using the lock!
                                        }
                                    }

            @attention  I think it's an easy method - we don't need any mutex here!

            @seealso    method setWorkingMode()

            @param      -
            @return     Current set mode.

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        virtual EWorkingMode SAL_CALL getWorkingMode();

        /*-****************************************************************************************************//**
            @short      set lock for reading
            @descr      A guard should call this method to acquire read access on your member.
                        Writing isn't allowed then - but nobody could check it for you!
                        You must look for a possible refused call by check given parameter - eReason!
                        What you should do then is your problem ... but there is no access to
                        safed variables allowed!

            @seealso    method setWorkingMode()
            @seealso    method releaseReadAccess()

            @param      "eRejectReason", is the reason for rejected calls.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL acquireReadAccess( ERefusalReason& eReason );

        /*-****************************************************************************************************//**
            @short      reset lock for reading
            @descr      A guard should call this method to release read access on your member.

            @seealso    method acquireReadAccess()

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL releaseReadAccess();

        /*-****************************************************************************************************//**
            @short      set lock for writing
            @descr      A guard should call this method to acquire write access on your member.
                        Reading is allowed too - of course.
                        You must look for a possible refused call by check given parameter - eReason!
                        What you should do then is your problem ... but there is no access to
                        safed variables allowed!
                        After successfully calling of this method you are the only writer.

            @seealso    method setWorkingMode()
            @seealso    method releaseWriteAccess()

            @param      "eRejectReason", is the reason for rejected calls.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL acquireWriteAccess( ERefusalReason& eReason );

        /*-****************************************************************************************************//**
            @short      reset lock for writing
            @descr      A guard should call this method to release write access on your member.

            @seealso    method acquireWriteAccess()

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL releaseWriteAccess();

        /*-****************************************************************************************************//**
            @short      downgrade a write access to a read access
            @descr      A guard should call this method to change a write to a read access.
                        New readers can work too - new writer are blocked!

            @attention  Don't call this method if you are not a writer!
                        Results are not defined then ...
                        An upgrade can't be implemented realy ... because acquiring new access
                        will be the same - there no differences!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL downgradeWriteAccess();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

        /*-****************************************************************************************************//**
            @short      used to check if calls must be refused
            @descr      If our internal state isn't E_WORK we must refuse all requests!

            @seealso    acquire methods

            @param      "eReason", is used to give you the reason for refused calls
            @return     true , if call must refused
                        false, otherwise

            @onerror    No error can occure.
        *//*-*****************************************************************************************************/

        sal_Bool impl_isCallRefused( ERefusalReason& eReason );

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------
    private:

        ::osl::Mutex        m_aAccessLock       ;   /// regulate access on internal member of this instance
        EWorkingMode        m_eWorkingMode      ;   /// current working mode of object which use this lock (used to reject calls at wrong time)
        ::osl::Mutex        m_aSerializer       ;   /// serialze incoming read/write access threads
        ::osl::Condition    m_aWriteCondition   ;   /// a writer must wait till current working reader are gone
        sal_Int32           m_nReadCount        ;   /// every reader is registered - the last one open the door for waiting writer

};      //  class FairRWLock

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_FAIRRWLOCK_HXX_
