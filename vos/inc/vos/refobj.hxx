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




#ifndef _VOS_REFOBJ_HXX_
#define _VOS_REFOBJ_HXX_

/**
    ORefObj<T>

    template type to implement handle/body behaviour
    with reference-counting.
*/

#   include <vos/refernce.hxx>
#ifndef _VOS_DIAGNOSE_HXX_
#   include <vos/diagnose.hxx>
#endif

namespace vos
{

template <class T>
class ORefObj : public IReference
{
public:
    ORefObj(const T& Obj);

    inline ~ORefObj();

    virtual RefCount SAL_CALL acquire()
        { return (m_RefCount.acquire()); }
    virtual RefCount SAL_CALL release()
        { return (m_RefCount.release()); }
    virtual RefCount SAL_CALL referenced() const
        { return (m_RefCount.referenced()); }

    T& SAL_CALL operator=(const T& Obj);

    SAL_CALL operator T&();
    SAL_CALL operator const T&() const;

    T& SAL_CALL operator() ();
    const T& SAL_CALL operator() () const;

    const   T& SAL_CALL getObj() const;
    T& SAL_CALL getObj();

protected:
    T         m_Obj;
    ORefCount m_RefCount;

private:
    ORefObj(const ORefObj<T>& handle);
    ORefObj<T>& SAL_CALL operator= (const ORefObj<T>& handle);
};

// include template implementation
#include <vos/refobj.inl>

}


#endif // _VOS_REF_HXX_

