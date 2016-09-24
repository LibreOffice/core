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

#ifndef INCLUDED_OSL_CONDITN_HXX
#define INCLUDED_OSL_CONDITN_HXX

#include <sal/config.h>

#include <cstddef>

#include <osl/time.h>

#include <osl/conditn.h>


namespace osl
{
    /**
     * Warning: the Condition abstraction is inadequate for any
     * situation where there may be multiple threads setting,
     * waiting, and resetting the same condition. It can only be
     * used to synchronise interactions between two threads
     * cf. lost wakeups in:
     *    http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
     */
    class Condition
    {
    public:

        enum Result
        {
            result_ok      = osl_cond_result_ok,
            result_error   = osl_cond_result_error,
            result_timeout = osl_cond_result_timeout
        };

        /* Create a condition.
         */
        Condition()
        {
            condition = osl_createCondition();
        }

        /* Release the OS-structures and free condition data-structure.
         */
        ~Condition()
        {
            osl_destroyCondition(condition);
        }

        /* Release all waiting threads, check returns true.
         */
        void set()
        {
            osl_setCondition(condition);
        }

        /* Reset condition to false: wait() will block, check() returns false.
         */
        void reset() {
            osl_resetCondition(condition);
        }

        /** Blocks the calling thread until condition is set.
         */
        Result wait(const TimeValue *pTimeout = NULL)
        {
            return (Result) osl_waitCondition(condition, pTimeout);
        }

#if defined LIBO_INTERNAL_ONLY
        Result wait(TimeValue const & timeout) { return wait(&timeout); }
#endif

        /** Checks if the condition is set without blocking.
         */
        bool check()
        {
            return osl_checkCondition(condition);
        }


    private:
        oslCondition condition;

        /** The underlying oslCondition has no reference count.

        Since the underlying oslCondition is not a reference counted object, copy
        constructed Condition may work on an already destructed oslCondition object.

        */
        Condition(const Condition&) SAL_DELETED_FUNCTION;

        /** This assignment operator is deleted for the same reason as
            the copy constructor.
        */
        Condition& operator= (const Condition&) SAL_DELETED_FUNCTION;
    };

}

#endif // INCLUDED_OSL_CONDITN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
