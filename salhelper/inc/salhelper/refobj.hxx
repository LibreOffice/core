/*************************************************************************
 *
 *  $RCSfile: refobj.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mhu $ $Date: 2001-05-14 11:50:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SALHELPER_REFOBJ_HXX_
#define _SALHELPER_REFOBJ_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_INTERLCK_H_
#include <osl/interlck.h>
#endif

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
