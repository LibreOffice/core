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

#ifndef INCLUDED_VCL_PTR_HXX
#define INCLUDED_VCL_PTR_HXX

#include <rtl/ref.hxx>
#include <cstddef>

/// @cond INTERNAL
namespace vcl { namespace detail {

// A mechanism to enable up-casts, used by the VclReference conversion constructor,
// heavily borrowed from boost::is_base_and_derived
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

#if defined _MSC_VER
    static char f(T2 *, long);
    static S f(T1 * const &, int);
#else
    template< typename U > static char f(T2 *, U);
    static S f(T1 *, int);
#endif

    struct H {
        H(); // avoid C2514 "class has no constructors" from MSVC 2008
#if defined _MSC_VER
        operator T1 * const & () const;
#else
        operator T1 * () const;
#endif
        operator T2 * ();
    };

public:
    typedef typename C< sizeof (f(H(), 0)) == 1, void *, void >::t t;
};

}; }; // namespace detail, namespace vcl

namespace vcl { class Window; }

/**
 * A thin wrapper around rtl::Reference to implement the acquire and dispose semantics we want for references to vcl::Window subclasses.
 *
 * For more details on the design please see vcl/README.lifecycle
 *
 * @param reference_type must be a subclass of vcl::Window
 */
template <class reference_type>
class VclPtr
{
    ::rtl::Reference<reference_type> m_rInnerRef;

public:
    /** Constructor...
     */
    inline VclPtr()
        : m_rInnerRef()
    {}


    /** Constructor...
     */
    inline VclPtr (reference_type * pBody)
        : m_rInnerRef(pBody)
    {}


    /** Copy constructor...
     */
    inline VclPtr (const VclPtr<reference_type> & handle)
        : m_rInnerRef (handle.m_rInnerRef)
    {}

    /** Up-casting conversion constructor: Copies interface reference.

        Does not work for up-casts to ambiguous bases.  For the special case of
        up-casting to Reference< XInterface >, see the corresponding conversion
        operator.

        @param rRef another reference
    */
    template< class derived_type >
    inline VclPtr(
        const VclPtr< derived_type > & rRef,
        typename ::vcl::detail::UpCast< reference_type, derived_type >::t = 0 )
        : m_rInnerRef( static_cast<reference_type*>(rRef) )
    {
    }

    /** Probably most common used: handle->someBodyOp().
     */
    inline reference_type * SAL_CALL operator->() const
    {
        return m_rInnerRef.get();
    }

    /** Get the body. Can be used instead of operator->().
         I.e. handle->someBodyOp() and handle.get()->someBodyOp()
         are the same.
      */
    inline reference_type * SAL_CALL get() const
    {
        return m_rInnerRef.get();
    }

    inline void SAL_CALL set(reference_type *pBody)
    {
        m_rInnerRef.set(pBody);
    }

    inline void SAL_CALL reset(reference_type *pBody)
    {
        m_rInnerRef.set(pBody);
    }

    inline VclPtr<reference_type>& SAL_CALL operator= (reference_type * pBody)
    {
        m_rInnerRef.set(pBody);
        return *this;
    }

    inline SAL_CALL operator reference_type * () const
    {
        return m_rInnerRef.get();
    }

    inline SAL_CALL operator bool () const
    {
        return m_rInnerRef.get() != NULL;
    }

    inline void SAL_CALL clear()
    {
        m_rInnerRef.clear();
    }

    inline void SAL_CALL reset()
    {
        m_rInnerRef.clear();
    }

    inline void disposeAndClear()
    {
        // hold it alive for the lifetime of this method
        ::rtl::Reference<reference_type> aTmp(m_rInnerRef);
        m_rInnerRef.clear(); // we should use some 'swap' method ideally ;-)
        if (aTmp.get()) {
            aTmp->disposeOnce();
        }
    }

    /** Returns True if handle points to the same body.
     */
    template<class T>
    inline bool SAL_CALL operator== (const VclPtr<T> & handle) const
    {
        return (get() == handle.get());
    }

    /** Needed to place VclPtr's into STL collection.
     */
    inline bool SAL_CALL operator!= (const VclPtr<reference_type> & handle) const
    {
        return (m_rInnerRef != handle.m_rInnerRef);
    }

    /** Makes comparing against NULL easier, resolves compile-time ambiguity */
    inline bool SAL_CALL operator!= (::std::nullptr_t ) const
    {
        return (get() != nullptr);
    }

    /** Needed to place VclPtr's into STL collection.
     */
    inline bool SAL_CALL operator< (const VclPtr<reference_type> & handle) const
    {
        return (m_rInnerRef < handle.m_rInnerRef);
    }

    /** Needed to place VclPtr's into STL collection.
     */
    inline bool SAL_CALL operator> (const VclPtr<reference_type> & handle) const
    {
        return (m_rInnerRef > handle.m_rInnerRef);
    }
}; // class VclPtr


template <class reference_type>
class ScopedVclPtr : public VclPtr<reference_type>
{
public:
    /** Constructor...
     */
    inline ScopedVclPtr()
        : VclPtr<reference_type>()
    {}

    /** Constructor
     */
    inline ScopedVclPtr (reference_type * pBody)
        : VclPtr<reference_type>(pBody)
    {}

    /** Copy constructor...
     */
    inline ScopedVclPtr (const VclPtr<reference_type> & handle)
        : VclPtr<reference_type>(handle)
    {}

    /**
       Assignment that releases the last reference.
     */
    inline ScopedVclPtr<reference_type>& SAL_CALL operator= (reference_type * pBody)
    {
        VclPtr<reference_type>::disposeAndClear();
        VclPtr<reference_type>::set(pBody);
        return *this;
    }

    /** Up-casting conversion constructor: Copies interface reference.

        Does not work for up-casts to ambiguous bases.  For the special case of
        up-casting to Reference< XInterface >, see the corresponding conversion
        operator.

        @param rRef another reference
    */
    template< class derived_type >
    inline ScopedVclPtr(
        const VclPtr< derived_type > & rRef,
        typename ::vcl::detail::UpCast< reference_type, derived_type >::t = 0 )
        : VclPtr<reference_type>( rRef )
    {
    }

    ~ScopedVclPtr()
    {
        VclPtr<reference_type>::disposeAndClear();
    }
private:
    // Most likely we don't want this default copy-construtor.
    ScopedVclPtr (const ScopedVclPtr<reference_type> &) SAL_DELETED_FUNCTION;
    // And certainly we don't want a default assignment operator.
    ScopedVclPtr<reference_type>& SAL_CALL operator= (const ScopedVclPtr<reference_type> &) SAL_DELETED_FUNCTION;
};

#endif // INCLUDED_VCL_PTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
