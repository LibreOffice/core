/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifndef _VOS_CONDITN_HXX_
#define _VOS_CONDITN_HXX_

#ifndef _OSL_CONDITN_H_
#   include <osl/conditn.h>
#endif
#include <osl/time.h>
#   include <vos/object.hxx>

namespace vos
{

/** ICondition

    Interface for a thread-spanning condition. If a condition-object
    is created, its initial condition is False. You can check the
    condition nonblocking with "check()" or wait for it to become set
    with "wait()". The methods "set()" and "reset()" are used to change
    the conditions state.

    @author  Bernd Hofner
    @version 1.0
*/
class ICondition
{
public:

    ICondition() { }
    virtual ~ICondition() { }



    enum TResult
    {
        result_ok          = osl_cond_result_ok,
        result_error       = osl_cond_result_error,
        result_timeout     = osl_cond_result_timeout
    };

    /** set condition to True =>
        wait() will not block, check() returns True
    */
    virtual void SAL_CALL set()= 0;

    /** set condition to False =>
        wait() will block, check() returns False
    */
    virtual void SAL_CALL reset()= 0;

    /** Blocks if condition is not set<BR>
        If condition has been destroyed prematurely, wait() will
        return with False.
    */
    virtual TResult SAL_CALL wait(const TimeValue* pTimeout = 0)= 0;

    /** True: condition is set <BR>
        False: condition is not set <BR>
        does not block
    */
    virtual sal_Bool SAL_CALL check()= 0;
};


/** OCondition

    Implements the ICondition interface.

    @author  Bernd Hofner
    @version 1.0

*/
class OCondition : public OObject, public ICondition
{

    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OCondition, vos));

public:

    /// initial state of condition is not set
    OCondition();
    virtual ~OCondition();

    /// set condition to True => wait() will not block, check() returns True
    virtual void SAL_CALL set();

    /// set condition to False => wait() will block, check() returns False
    virtual void SAL_CALL reset();

    /** Blocks if condition is not set<BR>
        If condition has been destroyed prematurely, wait() will
        return with False.
    */
    TResult SAL_CALL wait(const TimeValue* pTimeout = 0);

    /** True: condition is set <BR>
        False: condition is not set <BR>
        does not block
    */
    virtual sal_Bool SAL_CALL check();

protected:

    oslCondition    m_Condition;

};

}

#endif  // _VOS_CONDITN_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
