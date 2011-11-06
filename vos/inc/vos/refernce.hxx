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



#ifndef _VOS_REFERNCE_HXX_
#define _VOS_REFERNCE_HXX_

#   include <vos/types.hxx>
#   include <osl/interlck.h>
#   include <vos/object.hxx>

namespace vos
{

/** Interface for refernce-counting
*/
class IReference
{
public:

    IReference() { }
    virtual ~IReference() { }

    typedef oslInterlockedCount RefCount;

    virtual RefCount SAL_CALL acquire()=0;
    virtual RefCount SAL_CALL release()=0;

    virtual RefCount SAL_CALL referenced() const=0;
};

class ORefCount
{
public:
    typedef IReference::RefCount RefCount;

    ORefCount() { m_RefCount = 0; }
    ORefCount(RefCount n) { m_RefCount = n; }
    virtual ~ORefCount();

    RefCount SAL_CALL acquire() { return (osl_incrementInterlockedCount(&m_RefCount)); };
    RefCount SAL_CALL release() { return (osl_decrementInterlockedCount(&m_RefCount)); };

    RefCount SAL_CALL operator++()   { return acquire(); }
    // don't implement the postfix operator, it won't function this way!

    RefCount SAL_CALL operator--()   { return release(); }
    // don't implement the postfix operator, it won't function this way!

    RefCount SAL_CALL referenced() const
        { return (m_RefCount); }

protected:
    RefCount m_RefCount;

private:
    // disable copy/assignment
    ORefCount(const ORefCount&);
    ORefCount& SAL_CALL operator= (const ORefCount&);
};

class OReference : public vos::IReference
{
public:
    OReference();
    virtual ~OReference();

    virtual RefCount SAL_CALL acquire();
    virtual RefCount SAL_CALL release();

    virtual RefCount SAL_CALL referenced() const
        { return (m_RefCount.referenced()); }

protected:
    ORefCount m_RefCount;

private:
    // disable copy/assignment
    OReference(const OReference&);
    OReference& SAL_CALL operator= (const OReference&);
};

}

#endif  // _VOS_REFERNCE_HXX_

