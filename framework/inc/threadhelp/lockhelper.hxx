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

#ifndef INCLUDED_FRAMEWORK_INC_THREADHELP_LOCKHELPER_HXX
#define INCLUDED_FRAMEWORK_INC_THREADHELP_LOCKHELPER_HXX

#include <boost/noncopyable.hpp>

#include <comphelper/solarmutex.hxx>
#include <fwidllapi.h>

namespace osl { class Mutex; }

namespace framework{

/*-************************************************************************************************************
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

    @devstatus      draft
*//*-*************************************************************************************************************/
class FWI_DLLPUBLIC LockHelper : private boost::noncopyable
{

    //  public methods

    public:


        //  ctor/dtor

                 LockHelper( comphelper::SolarMutex* pSolarMutex = NULL );
                ~LockHelper(                                   );

        void acquire();
        void release();

        //  something else

        static LockHelper&  getGlobalLock();
            //TODO: this presumable should return the SolarMutex, though it
            // actually returns some independent mutex

        ::osl::Mutex&       getShareableOslMutex(                                   );


    //  private member
    //  Make some member mutable for using in const functions!

    private:

        mutable comphelper::SolarMutex* m_pSolarMutex   ;
        mutable ::osl::Mutex*   m_pShareableOslMutex    ;
        mutable sal_Bool        m_bDummySolarMutex      ;
};

}       //  namespace framework

#endif // INCLUDED_FRAMEWORK_INC_THREADHELP_LOCKHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
