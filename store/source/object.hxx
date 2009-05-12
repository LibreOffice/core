/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: object.hxx,v $
 *
 *  $Revision: 1.1.2.1 $
 *
 *  last change: $Author: mhu $ $Date: 2008/09/18 16:10:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _STORE_OBJECT_HXX_
#define _STORE_OBJECT_HXX_ "$Revision: 1.1.2.1 $"

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
