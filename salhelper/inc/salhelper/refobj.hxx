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



#ifndef _SALHELPER_REFOBJ_HXX_
#define _SALHELPER_REFOBJ_HXX_

#include <sal/types.h>
#include <rtl/alloc.h>
#include <rtl/ref.hxx>
#include <osl/diagnose.h>
#include <osl/interlck.h>

namespace salhelper
{

//----------------------------------------------------------------------------

class ReferenceObject : public rtl::IReference
{
    /** Representation.
     */
    oslInterlockedCount m_nReferenceCount;

    /** Not implemented.
     */
    ReferenceObject (const ReferenceObject&);
    ReferenceObject& operator= (const ReferenceObject&);

public:
    /** Allocation.
     */
    static void* operator new (size_t n) SAL_THROW(())
    {
        return ::rtl_allocateMemory (n);
    }
    static void operator delete (void* p) SAL_THROW(())
    {
        ::rtl_freeMemory (p);
    }
    static void* operator new (size_t, void* p) SAL_THROW(())
    {
        return (p);
    }
    static void operator delete (void*, void*) SAL_THROW(())
    {}

public:
    /** Construction.
     */
    inline ReferenceObject() SAL_THROW(()) : m_nReferenceCount (0)
    {}


    /** IReference.
     */
    virtual oslInterlockedCount SAL_CALL acquire() SAL_THROW(())
    {
        return ::osl_incrementInterlockedCount (&m_nReferenceCount);
    }

    virtual oslInterlockedCount SAL_CALL release() SAL_THROW(())
    {
        oslInterlockedCount result;
        result = ::osl_decrementInterlockedCount (&m_nReferenceCount);
        if (result == 0)
        {
            // Last reference released.
            delete this;
        }
        return (result);
    }

protected:
    /** Destruction.
     */
    virtual ~ReferenceObject() SAL_THROW(())
    {
        OSL_ASSERT(m_nReferenceCount == 0);
    }
};

//----------------------------------------------------------------------------

} // namespace salhelper

#endif /* !_SALHELPER_REFOBJ_HXX_ */
