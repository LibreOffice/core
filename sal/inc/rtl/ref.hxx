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

#ifndef _RTL_REF_HXX_
#define _RTL_REF_HXX_

#include <sal/types.h>
#include <osl/diagnose.h>
#include <osl/interlck.h>

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

#if !defined _MSC_VER // public -> protected changes mangled names there
protected:
#endif
    ~IReference() {}
        // avoid warnings about virtual members and non-virtual dtor
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

    /** Set...
         Similar to assignment.
     */
    inline Reference<reference_type> &
    SAL_CALL set (reference_type * pBody)
    {
        if (pBody)
            pBody->acquire();
        reference_type * const pOld = m_pBody;
        m_pBody = pBody;
        if (pOld)
            pOld->release();
        return *this;
    }

    /** Assignment.
         Unbinds this instance from its body (if bound) and
         bind it to the body represented by the handle.
     */
    inline Reference<reference_type> &
    SAL_CALL operator= (const Reference<reference_type> & handle)
    {
        return set( handle.m_pBody );
    }

    /** Assignment...
     */
    inline Reference<reference_type> &
    SAL_CALL operator= (reference_type * pBody)
    {
        return set( pBody );
    }

    /** Unbind the body from this handle.
         Note that for a handle representing a large body,
         "handle.clear().set(new body());" _might_
         perform a little bit better than "handle.set(new body());",
         since in the second case two large objects exist in memory
         (the old body and the new body).
     */
    inline Reference<reference_type> & SAL_CALL clear()
    {
        if (m_pBody)
        {
            reference_type * const pOld = m_pBody;
            m_pBody = 0;
            pOld->release();
        }
        return *this;
    }


    /** Get the body. Can be used instead of operator->().
         I.e. handle->someBodyOp() and handle.get()->someBodyOp()
         are the same.
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
    inline sal_Bool SAL_CALL is() const
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

/// @cond INTERNAL
/** Enables boost::mem_fn and boost::bind to recognize Reference.
*/
template <typename T>
inline T * get_pointer( Reference<T> const& r )
{
    return r.get();
}
/// @endcond

} // namespace rtl

#endif /* !_RTL_REF_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
