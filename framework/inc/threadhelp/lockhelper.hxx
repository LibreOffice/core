/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __FRAMEWORK_THREADHELP_LOCKHELPER_HXX_
#define __FRAMEWORK_THREADHELP_LOCKHELPER_HXX_

#include <threadhelp/inoncopyable.h>
#include <framework/imutex.hxx>
#include <threadhelp/irwlock.h>
#include <threadhelp/fairrwlock.hxx>

#include <osl/mutex.hxx>
#include <fwidllapi.h>

namespace framework{

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
#define FALLBACK_LOCKTYPE   E_SOLARMUTEX

/*-************************************************************************************************************//**
    @short          helper to set right lock in right situation
    @descr          This helper support different types of locking:
                        a)  no locks - transparent for user!
                            This could be useful for simluation or single threaded environments!
                        b)  own mutex
                            An object use his own osl-mutex to be threadsafe. Useful for easy and exclusiv locking.
                        c)  solar mutex
                            An object use our solar mutex and will be a part of a greater safed "threadsafe code block".
                            Could be useful for simulation and testing of higher modules!
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
                 LockHelper( ::osl::SolarMutex* pSolarMutex = NULL );
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
        static LockHelper&  getGlobalLock       ( ::osl::SolarMutex* pSolarMutex = NULL );
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
    //                                              Creation and using of "m_pShareableOslMutex" isn't necessary!
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
        mutable ::osl::SolarMutex*  m_pSolarMutex       ;
        mutable ::osl::Mutex*   m_pShareableOslMutex    ;
        mutable sal_Bool        m_bDummySolarMutex      ;
};

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_THREADHELP_LOCKHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
