/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: irwlock.h,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:36:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef __FRAMEWORK_THREADHELP_IRWLOCK_H_
#define __FRAMEWORK_THREADHELP_IRWLOCK_H_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

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
            @descr      These functions must be supported by a derived class!
                            acquireReadAccess()     -try to register thread as reader
                            releaseReadAccess()     -unregister thread as reader
                            acquireWriteAccess()    -try to register thread as writer
                            releaseWriteAccess()    -unregister thread as writer
                            downgradeWriteAccess()  -make writer to reader
        *//*-*****************************************************************************************************/
        virtual void acquireReadAccess    () =0;
        virtual void releaseReadAccess    () =0;
        virtual void acquireWriteAccess   () =0;
        virtual void releaseWriteAccess   () =0;
        virtual void downgradeWriteAccess () =0;

};      //  class IRWLock

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_IRWLOCK_H_
