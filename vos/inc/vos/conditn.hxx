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

