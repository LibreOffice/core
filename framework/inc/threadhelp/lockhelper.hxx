/*************************************************************************
 *
 *  $RCSfile: lockhelper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-06-11 10:11:46 $
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

#ifndef __FRAMEWORK_THREADHELP_LOCKHELPER_HXX_
#define __FRAMEWORK_THREADHELP_LOCKHELPER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_INONCOPYABLE_H_
#include <threadhelp/inoncopyable.h>
#endif

#ifndef __FRAMEWORK_THREADHELP_IMUTEX_H_
#include <threadhelp/imutex.h>
#endif

#ifndef __FRAMEWORK_THREADHELP_IRWLOCK_H_
#include <threadhelp/irwlock.h>
#endif

#ifndef __FRAMEWORK_THREADHELP_FAIRRWLOCK_HXX_
#include <threadhelp/fairrwlock.hxx>
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

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          If you use a lock or mutex as a member of your class and whish to use it earlier then other ones
                    you should have a look on this implementation. You must use it as the first base class
                    of your implementation - because base classes are initialized by his order and before your
                    member! Thats why ist a good place to declare your thread help member so.
*//*-*************************************************************************************************************/
enum ELockType
{
    E_NOTHING       = 0 ,
    E_OWNMUTEX      = 1 ,
    E_SOLARMUTEX    = 2 ,
    E_FAIRRWLOCK    = 3
};

#define ENVVAR_LOCKTYPE     DECLARE_ASCII("LOCKTYPE_FRAMEWORK")
#define FALLBACK_LOCKTYPE   E_OWNMUTEX

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          helper to set right lock in right situation
    @descr          This helper support different types of locking:
                        a)  no locks - transparent for user!
                            This could be usefull for simluation or single threaded environments!
                        b)  own mutex
                            An object use his own osl-mutex to be threadsafe. Usefull for easy and exclusiv locking.
                        c)  solar mutex
                            An object use our solar mutex and will be a part of a greater safed "threadsafe code block".
                            Could be usefull for simulation and testing of higher modules!
                        d)  fair rw-lock
                            An object use an implementation of a fair rw-lock. This increase granularity of t hreadsafe mechanism
                            and should be used for high performance threadsafe code!

    @attention      We support two interfaces - "IMutex" and "IRWLock". Don't mix using of it!
                    A guard implementation should use one interface only!

    @implements     IMutex
    @implements     IRWLock

    @base           INonCopyable
                    IMutex
                    IRWLock

    @devstatus      draft
*//*-*************************************************************************************************************/
class LockHelper : public  IMutex
                 , public  IRWLock
                 , private INonCopyable
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:

        //-------------------------------------------------------------------------------------------------------------
        //  ctor/dtor
        //-------------------------------------------------------------------------------------------------------------
                 LockHelper( ::vos::IMutex* pSolarMutex = NULL );
        virtual ~LockHelper(                                   );

        //-------------------------------------------------------------------------------------------------------------
        //  interface ::framework::IMutex
        //-------------------------------------------------------------------------------------------------------------
        virtual void acquire();
        virtual void release();

        //-------------------------------------------------------------------------------------------------------------
        //  interface ::framework::IRWLock
        //-------------------------------------------------------------------------------------------------------------
        virtual void acquireReadAccess   ();
        virtual void releaseReadAccess   ();
        virtual void acquireWriteAccess  ();
        virtual void releaseWriteAccess  ();
        virtual void downgradeWriteAccess();

        //-------------------------------------------------------------------------------------------------------------
        //  something else
        //-------------------------------------------------------------------------------------------------------------
        static LockHelper&  getGlobalLock       ( ::vos::IMutex* pSolarMutex = NULL );
        ::osl::Mutex&       getShareableOslMutex(                                   );

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:

        static ELockType& implts_getLockType();

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //  a) Make some member mutable for using in const functions!
    //  b) "m_eLockType" define, which of follow members is used!
    //     You can use "m_pFairRWLock" as a fair rw-lock (multiple reader / one writer / looks for incoming order of threads too) ...
    //     or you can use a normal osl mutex ("m_pOwnMutex") ...
    //     ... or the solarmuex as "m_pSolarMutex" (must be set from outside! because some components must be vcl-free!)
    //     ... but sometimes you need a shareable osl mutex!
    //     In this case you has some problems: i  ) If your lock type is set to E_OWNMUTEX => it's easy; you can use your member "m_pOwnMutex" - it's a osl mutex.
    //                                              Creation and using of "m_pShareableOslMutex" isn't neccessary!
    //                                         ii ) Otherwise you have no osl mutex ... so you must create "m_pShareableOslMutex" and use it twice!
    //                                              In this case you must lock two member everytime - "m_pShareableMutex" AND "m_pFairRWLock" or "m_pSolarMutex" or ...
    //                                              It isn't realy fine - but the only possible way.
    //                                         iii) There exist another special case - E_NOTHING is set! Then we should create this shareable mutex ...
    //                                              nad you can use it ... but this implmentation ignore it.
    //-------------------------------------------------------------------------------------------------------------
    private:

        ELockType               m_eLockType             ;

        mutable FairRWLock*     m_pFairRWLock           ;
        mutable ::osl::Mutex*   m_pOwnMutex             ;
        mutable ::vos::IMutex*  m_pSolarMutex           ;
        mutable ::osl::Mutex*   m_pShareableOslMutex    ;
};

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_LOCKHELPER_HXX_
