/*************************************************************************
 *
 *  $RCSfile: ref.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mhu $ $Date: 2001-03-12 12:31:39 $
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

#ifndef _RTL_REF_HXX_
#define _RTL_REF_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_INTERLCK_H_
#include <osl/interlck.h>
#endif

namespace rtl
{

/** Interface for a reference type.
*/
class IReference
{
public:
    /** @see osl_incrementInterlockedCount.
     */
    virtual oslInterlockedCount SAL_CALL acquire() = 0;

    /** @see osl_decrementInterlockedCount.
     */
    virtual oslInterlockedCount SAL_CALL release() = 0;
};


/** Template reference class for reference type derived from IReference.
*/
template <class reference_type>
class Reference
{
    /** The <b>reference_type</b> body pointer.
     */
    reference_type * m_pBody;


public:
    /** Constructor...
     */
    inline Reference()
        : m_pBody (0)
    {}


    /** Constructor...
     */
    inline Reference (reference_type * pBody)
        : m_pBody (pBody)
    {
        if (m_pBody)
            m_pBody->acquire();
    }


    /** Copy constructor...
     */
    inline Reference (const Reference<reference_type> & handle)
        : m_pBody (handle.m_pBody)
    {
        if (m_pBody)
            m_pBody->acquire();
    }


    /** Destructor...
     */
    inline ~Reference()
    {
        if (m_pBody)
            m_pBody->release();
    }


    /** Assignment.
     *  Unbinds this instance from its body (if bound) and
     *  bind it to the body represented by the handle.
     */
    inline Reference<reference_type> &
    SAL_CALL operator= (const Reference<reference_type> & handle)
    {
        if (m_pBody)
            m_pBody->release();
        m_pBody = handle.m_pBody;
        if (m_pBody)
            m_pBody->acquire();
        return *this;
    }


    /** Assignment...
     */
    inline Reference<reference_type> &
    SAL_CALL operator= (reference_type * pBody)
    {
        if (m_pBody)
            m_pBody->release();
        m_pBody = pBody;
        if (m_pBody)
            m_pBody->acquire();
        return *this;
    }


    /** Set...
     *  Similar to assignment.
     */
    inline Reference<reference_type> &
    SAL_CALL set (reference_type * pBody)
    {
        if (m_pBody)
            m_pBody->release();
        m_pBody = pBody;
        if (m_pBody)
            m_pBody->acquire();
        return *this;
    }

    /** Unbind the body from this handle.
     *  Note that for a handle representing a large body,
     *  "handle.clear().set(new body());" _might_
     *  perform a little bit better than "handle.set(new body());",
     *  since in the second case two large objects exist in memory
     *  (the old body and the new body).
     */
    inline Reference<reference_type> & SAL_CALL clear()
    {
        if (m_pBody)
        {
            m_pBody->release();
            m_pBody = 0;
        }
        return *this;
    }


    /** Get the body. Can be used instead of operator->().
     *  I.e. handle->someBodyOp() and handle.get()->someBodyOp()
     *  are the same.
     */
    inline reference_type * SAL_CALL get() const
    {
        return m_pBody;
    }


    /** Probably most common used: handle->someBodyOp().
     */
    inline reference_type * SAL_CALL operator->() const
    {
        OSL_PRECOND(m_pBody, "Reference::operator->() : null body");
        return m_pBody;
    }


    /** Allows (*handle).someBodyOp().
    */
    inline reference_type & SAL_CALL operator*() const
    {
        OSL_PRECOND(m_pBody, "Reference::operator*() : null body");
        return *m_pBody;
    }


    /** Returns True if the handle does point to a valid body.
     */
    inline sal_Bool SAL_CALL isValid() const
    {
        return (m_pBody != 0);
    }


    /** Returns True if this points to pBody.
     */
    inline sal_Bool SAL_CALL operator== (const reference_type * pBody) const
    {
        return (m_pBody == pBody);
    }


    /** Returns True if handle points to the same body.
     */
    inline sal_Bool
    SAL_CALL operator== (const Reference<reference_type> & handle) const
    {
        return (m_pBody == handle.m_pBody);
    }


    /** Needed to place References into STL collection.
     */
    inline sal_Bool
    SAL_CALL operator!= (const Reference<reference_type> & handle) const
    {
        return (m_pBody != handle.m_pBody);
    }


    /** Needed to place References into STL collection.
     */
    inline sal_Bool
    SAL_CALL operator< (const Reference<reference_type> & handle) const
    {
        return (m_pBody < handle.m_pBody);
    }


    /** Needed to place References into STL collection.
     */
    inline sal_Bool
    SAL_CALL operator> (const Reference<reference_type> & handle) const
    {
        return (m_pBody > handle.m_pBody);
    }
};


} // namespace rtl

#endif /* !_RTL_REF_HXX_ */
