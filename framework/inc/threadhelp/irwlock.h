/*************************************************************************
 *
 *  $RCSfile: irwlock.h,v $
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

#ifndef __FRAMEWORK_THREADHELP_IRWLOCK_H_
#define __FRAMEWORK_THREADHELP_IRWLOCK_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          A guard (specialy a write guard) support different internal working states.
                    His lock can set for reading or writing/reading! Or he was unlocked by user ...
*//*-*************************************************************************************************************/

enum ELockMode
{
    E_NOLOCK    ,
    E_READLOCK  ,
    E_WRITELOCK
};

/*-************************************************************************************************************//**
    @descr          Describe different states of a feature of following implementation.
                    During live time of an object different working states occure:
                        initialization - working - closing/disposing
                    If you whish to implement thread safe classes you should use these feature to protect
                    your code against calls at wrong time. e.g. you are not full initialized but somewhere
                    call an interface method (initialize phase means startup time from creating object till
                    calling specified first method e.g. XInitialization::initialze()!) then you should refuse
                    this call. The same for closing/disposing object!
*//*-*************************************************************************************************************/

enum EWorkingMode
{
    E_INIT  ,
    E_WORK  ,
    E_CLOSE
};

/*-************************************************************************************************************//**
    @descr          If a request was refused by a lock implementation (internal state different E_WORK)
                    use can check the reason.
*//*-*************************************************************************************************************/

enum ERefusalReason
{
    E_NOREASON      ,
    E_UNINITIALIZED ,
    E_CLOSED
};

/*-************************************************************************************************************//**
    @descr          We implement two guards for using an rw-lock. But if you wish to implement
                    different rw-locks to you will have problems by using with same guard implementation!
                    Thats why we define this "pure virtual base class" ...
                    All rw-locks must support this base interface for working and all guard must use this one too!
*//*-*************************************************************************************************************/

class IRWLock
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        /*-****************************************************************************************************//**
            @descr      The dtor isn't realy important ... but if you whish to use derived classes
                        with any pointer ... you must have it!
        *//*-*****************************************************************************************************/

        virtual ~IRWLock() {}

        /*-****************************************************************************************************//**
            @descr      These function must be supported by a derived class!
        *//*-*****************************************************************************************************/

        virtual void            SAL_CALL setWorkingMode         ( EWorkingMode      eMode   ) = 0;
        virtual EWorkingMode    SAL_CALL getWorkingMode         (                           ) = 0;
        virtual void            SAL_CALL acquireReadAccess      ( ERefusalReason&   eReason ) = 0;
        virtual void            SAL_CALL releaseReadAccess      (                           ) = 0;
        virtual void            SAL_CALL acquireWriteAccess     ( ERefusalReason&   eReason ) = 0;
        virtual void            SAL_CALL releaseWriteAccess     (                           ) = 0;
        virtual void            SAL_CALL downgradeWriteAccess   (                           ) = 0;

};      //  class IRWLock

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_IRWLOCK_H_
