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



#ifndef _STORE_OBJECT_HXX_
#define _STORE_OBJECT_HXX_

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif

#ifndef _OSL_INTERLCK_H_
#include "osl/interlck.h"
#endif

namespace store
{

/*========================================================================
 *
 * IStoreHandle interface.
 *
 *======================================================================*/
class IStoreHandle : public rtl::IReference
{
public:
    /** Replaces dynamic_cast type checking.
     */
    virtual sal_Bool SAL_CALL isKindOf (sal_uInt32 nTypeId) = 0;
};


/** Template helper function as dynamic_cast replacement.
 */
template<class store_handle_type>
store_handle_type * SAL_CALL query (
    IStoreHandle * pHandle, store_handle_type *);

/*========================================================================
 *
 * OStoreObject interface.
 *
 *======================================================================*/
class OStoreObject : public store::IStoreHandle
{
    /** Template function specialization as dynamic_cast replacement.
     */
    friend OStoreObject*
    SAL_CALL query<> (IStoreHandle *pHandle, OStoreObject*);

public:
    /** Construction.
     */
    OStoreObject (void);

    /** Allocation.
     */
    static void* operator new (size_t n);
    static void  operator delete (void *p, size_t);

    /** IStoreHandle.
     */
    virtual sal_Bool SAL_CALL isKindOf (sal_uInt32 nTypeId);

    /** IReference.
     */
    virtual oslInterlockedCount SAL_CALL acquire (void);
    virtual oslInterlockedCount SAL_CALL release (void);

protected:
    /** Destruction.
     */
    virtual ~OStoreObject (void);

private:
    /** The IStoreHandle TypeId.
     */
    static const sal_uInt32 m_nTypeId;

    /** Representation.
     */
    oslInterlockedCount m_nRefCount;

    /** Not implemented.
     */
    OStoreObject (const OStoreObject&);
    OStoreObject& operator= (const OStoreObject&);
};

/** Template function specialization as dynamic_cast replacement.
 */
template<> inline OStoreObject*
SAL_CALL query (IStoreHandle *pHandle, OStoreObject*)
{
    if (pHandle && pHandle->isKindOf (OStoreObject::m_nTypeId))
    {
        // Handle is kind of OStoreObject.
        return static_cast<OStoreObject*>(pHandle);
    }
    return 0;
}

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

} // namespace store

#endif /* !_STORE_OBJECT_HXX_ */
