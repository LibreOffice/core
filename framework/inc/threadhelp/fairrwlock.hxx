/*************************************************************************
 *
 *  $RCSfile: fairrwlock.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: as $ $Date: 2001-05-02 13:00:41 $
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

    @implements     IRWlock
    @base           INonCopyAble
                    IRWLock


    @devstatus      ready to use
*//*-*************************************************************************************************************/

class FairRWLock    :   private INonCopyAble
                    ,   public  IRWLock
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        //---------------------------------------------------------------------------------------------------------
        //  ctor/dtor
        //---------------------------------------------------------------------------------------------------------
        FairRWLock();

        //---------------------------------------------------------------------------------------------------------
        //  IRWLock
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL acquireReadAccess   ();
        virtual void SAL_CALL releaseReadAccess   ();
        virtual void SAL_CALL acquireWriteAccess  ();
        virtual void SAL_CALL releaseWriteAccess  ();
        virtual void SAL_CALL downgradeWriteAccess();

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------
    private:

        ::osl::Mutex        m_aAccessLock       ;   /// regulate access on internal member of this instance
        ::osl::Mutex        m_aSerializer       ;   /// serialze incoming read/write access threads
        ::osl::Condition    m_aWriteCondition   ;   /// a writer must wait till current working reader are gone
        sal_Int32           m_nReadCount        ;   /// every reader is registered - the last one open the door for waiting writer

};      //  class FairRWLock

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_FAIRRWLOCK_HXX_
