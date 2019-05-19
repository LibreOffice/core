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



#ifndef __FRAMEWORK_THREADHELP_LOCKHELPER_HXX_
#define __FRAMEWORK_THREADHELP_LOCKHELPER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/inoncopyable.h>
#include <framework/imutex.hxx>
#include <threadhelp/irwlock.h>
#include <threadhelp/fairrwlock.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <osl/mutex.hxx>
#include <vos/mutex.hxx>
#include <fwidllapi.h>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          If you use a lock or mutex as a member of your class and wish to use it earlier than other ones
                    you should have a look on this implementation. You must use it as the first base class
                    of your implementation - because base classes are initialized by his order and before your
                    member! Thats why it is a good place to declare your thread help member so.
*//*-*************************************************************************************************************/
enum ELockType
{
    E_NOTHING       = 0 ,
    E_OWNMUTEX      = 1 ,
    E_SOLARMUTEX    = 2 ,
    E_FAIRRWLOCK    = 3
};

#define ENVVAR_LOCKTYPE     DECLARE_ASCII("LOCKTYPE_FRAMEWORK")
#define FALLBACK_LOCKTYPE   E_SOLARMUTEX

//_________________________________________________________________________________________________________________
// declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          helper to set right lock in right situation
    @descr          This helper support different types of locking:
                        a)  no locks - transparent for user!
                            This could be useful for simulation or single threaded environments!
                        b)  own mutex
                            An object use his own osl-mutex to be threadsafe. Useful for easy and exclusive locking.
                        c)  solar mutex
                            An object use our solar mutex and will be a part of a greater saved "threadsafe code block".
                            Could be useful for simulation and testing of higher modules!
                        d)  fair rw-lock
                            An object use an implementation of a fair rw-lock. This increase granularity of threadsafe mechanism
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
class FWI_DLLPUBLIC LockHelper : public  IMutex
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
    //                                              Creation and use of "m_pShareableOslMutex" isn't necessary!
    //                                         ii ) Otherwise you have no osl mutex ... so you must create "m_pShareableOslMutex" and use it twice!
    //                                              In this case you must lock two members every time - "m_pShareableMutex" AND "m_pFairRWLock" or "m_pSolarMutex" or ...
    //                                              It isn't really fine - but the only possible way.
    //                                         iii) There exists another special case - E_NOTHING is set! Then we should create this shareable mutex ...
    //                                              and you can use it ... but this implementation ignores it.
    //-------------------------------------------------------------------------------------------------------------
    private:

        ELockType               m_eLockType             ;

        mutable FairRWLock*     m_pFairRWLock           ;
        mutable ::osl::Mutex*   m_pOwnMutex             ;
        mutable ::vos::IMutex*  m_pSolarMutex           ;
        mutable ::osl::Mutex*   m_pShareableOslMutex    ;
        mutable sal_Bool        m_bDummySolarMutex      ;
};

}       // namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_LOCKHELPER_HXX_
