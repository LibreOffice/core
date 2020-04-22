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

#ifndef INCLUDED_RTL_REF_HXX
#define INCLUDED_RTL_REF_HXX

#include "sal/config.h"

#include <cassert>
#include <cstddef>
#include <functional>

#include "sal/types.h"

namespace rtl
{
template <class reference_type>
class Reference;

/// @cond INTERNAL
namespace detail {

// A mechanism to enable up-casts, used by the Reference conversion constructor,
// but at the same time disable up-casts to XInterface, so that the conversion
// operator for that special case is used in an expression like
// Reference< XInterface >(x); heavily borrowed from boost::is_base_and_derived
// (which manages to avoid compilation problems with ambiguous bases and cites
// comp.lang.c++.moderated mail <http://groups.google.com/groups?
// selm=df893da6.0301280859.522081f7%40posting.google.com> "SuperSubclass
// (is_base_and_derived) complete implementation!" by Rani Sharoni and cites
// Aleksey Gurtovoy for the workaround for MSVC), to avoid including Boost
// headers in URE headers (could ultimately be based on C++11 std::is_base_of):

template< typename T1, typename T2 > struct UpCast {
private:
    template< bool, typename U1, typename > struct C
    { typedef U1 t; };

    template< typename U1, typename U2 > struct C< false, U1, U2 >
    { typedef U2 t; };

    struct S { char c[2]; };

#if defined _MSC_VER && _MSC_VER < 1800
    static char f(T2 *, long);
    static S f(T1 * const &, int);
#else
    template< typename U > static char f(T2 *, U);
    static S f(T1 *, int);
#endif

    struct H {
        H(); // avoid C2514 "class has no constructors" from MSVC
#if defined _MSC_VER && _MSC_VER < 1800
        operator T1 * const & () const;
#else
        operator T1 * () const;
#endif
        operator T2 * ();
    };

public:
    typedef typename C< sizeof (f(H(), 0)) == 1, void *, void >::t t;
};

}
/// @endcond

/** Template reference class for reference type.
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
    Reference()
        : m_pBody (NULL)
    {}


    /** Constructor...
     */
    Reference (reference_type * pBody, __sal_NoAcquire)
        : m_pBody (pBody)
    {
    }

    /** Constructor...
     */
    Reference (reference_type * pBody)
        : m_pBody (pBody)
    {
        if (m_pBody)
            m_pBody->acquire();
    }

    /** Copy constructor...
     */
    Reference (const Reference<reference_type> & handle)
        : m_pBody (handle.m_pBody)
    {
        if (m_pBody)
            m_pBody->acquire();
    }

#ifdef LIBO_INTERNAL_ONLY
    /** Move constructor...
     */
    Reference (Reference<reference_type> && handle) noexcept
        : m_pBody (handle.m_pBody)
    {
        handle.m_pBody = nullptr;
    }
#endif

    /** Up-casting conversion constructor: Copies interface reference.

        Does not work for up-casts to ambiguous bases.  For the special case of
        up-casting to Reference< XInterface >, see the corresponding conversion
        operator.

        @param rRef another reference
    */
    template< class derived_type >
    inline Reference(
        const Reference< derived_type > & rRef,
        typename detail::UpCast< reference_type, derived_type >::t = 0 )
        : m_pBody (rRef.get())
    {
        if (m_pBody)
            m_pBody->acquire();
    }

    /** Destructor...
     */
    ~Reference() COVERITY_NOEXCEPT_FALSE
    {
        if (m_pBody)
            m_pBody->release();
    }

    /** Set...
         Similar to assignment.
     */
    Reference<reference_type> &
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
    Reference<reference_type> &
    SAL_CALL operator= (const Reference<reference_type> & handle)
    {
        return set( handle.m_pBody );
    }

#ifdef LIBO_INTERNAL_ONLY
    /** Assignment.
     *   Unbinds this instance from its body (if bound),
     *   bind it to the body represented by the handle, and
     *   set the body represented by the handle to nullptr.
     */
    Reference<reference_type> &
    operator= (Reference<reference_type> && handle)
    {
        // self-movement guts ourself
        if (m_pBody)
            m_pBody->release();
        m_pBody = handle.m_pBody;
        handle.m_pBody = nullptr;
        return *this;
    }
#endif

    /** Assignment...
     */
    Reference<reference_type> &
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
    Reference<reference_type> & SAL_CALL clear()
    {
        if (m_pBody)
        {
            reference_type * const pOld = m_pBody;
            m_pBody = NULL;
            pOld->release();
        }
        return *this;
    }


    /** Get the body. Can be used instead of operator->().
         I.e. handle->someBodyOp() and handle.get()->someBodyOp()
         are the same.
     */
    reference_type * SAL_CALL get() const
    {
        return m_pBody;
    }


    /** Probably most common used: handle->someBodyOp().
     */
    reference_type * SAL_CALL operator->() const
    {
        assert(m_pBody != NULL);
        return m_pBody;
    }


    /** Allows (*handle).someBodyOp().
    */
    reference_type & SAL_CALL operator*() const
    {
        assert(m_pBody != NULL);
        return *m_pBody;
    }


    /** Returns True if the handle does point to a valid body.
     */
    bool SAL_CALL is() const
    {
        return (m_pBody != NULL);
    }

#if defined LIBO_INTERNAL_ONLY
    /** Returns True if the handle does point to a valid body.
     */
    explicit operator bool() const
    {
        return is();
    }
#endif

    /** Returns True if this points to pBody.
     */
    bool SAL_CALL operator== (const reference_type * pBody) const
    {
        return (m_pBody == pBody);
    }


    /** Returns True if handle points to the same body.
     */
    bool
    SAL_CALL operator== (const Reference<reference_type> & handle) const
    {
        return (m_pBody == handle.m_pBody);
    }


    /** Needed to place References into STL collection.
     */
    bool
    SAL_CALL operator!= (const Reference<reference_type> & handle) const
    {
        return (m_pBody != handle.m_pBody);
    }


    /** Needed to place References into STL collection.
     */
    bool
    SAL_CALL operator< (const Reference<reference_type> & handle) const
    {
        return (m_pBody < handle.m_pBody);
    }


    /** Needed to place References into STL collection.
     */
    bool
    SAL_CALL operator> (const Reference<reference_type> & handle) const
    {
        return (m_pBody > handle.m_pBody);
    }
};

} // namespace rtl

#if defined LIBO_INTERNAL_ONLY
namespace std
{

/// @cond INTERNAL
/**
  Make rtl::Reference hashable by default for use in STL containers.

  @since LibreOffice 6.3
*/
template<typename T>
struct hash<::rtl::Reference<T>>
{
    std::size_t operator()(::rtl::Reference<T> const & s) const
    { return std::size_t(s.get()); }
};
/// @endcond

}

#endif

#endif /* ! INCLUDED_RTL_REF_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
