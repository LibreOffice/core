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
#include <utility>
#include <type_traits>

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

    template< typename U > static char f(T2 *, U);
    static S f(T1 *, int);

    struct H {
        H(); // avoid C2514 "class has no constructors" from MSVC 2008
        operator T1 * () const;
        operator T2 * ();
    };

public:
    static bool const value = sizeof (f(H(), 0)) == 1;
    typedef typename C< value, void *, void >::t t;
};

}; }; // namespace detail, namespace vcl

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

    /** Constructor... that doesn't take a ref.
     */
    inline VclPtr (reference_type * pBody, __sal_NoAcquire)
        : m_rInnerRef(pBody, SAL_NO_ACQUIRE)
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

    /**
     * A construction helper for VclPtr. Since VclPtr types are created
     * with a reference-count of one - to help fit into the existing
     * code-flow; this helps us to construct them easily.
     *
     * For more details on the design please see vcl/README.lifecycle
     *
     * @param reference_type must be a subclass of vcl::Window
     */
    template<typename... Arg> static VclPtr< reference_type > Create(Arg &&... arg)
    {
        return VclPtr< reference_type >( new reference_type(std::forward<Arg>(arg)...), SAL_NO_ACQUIRE );
    }

    /** Probably most common used: handle->someBodyOp().
     */
    inline reference_type * operator->() const
    {
        return m_rInnerRef.get();
    }

    /** Get the body. Can be used instead of operator->().
         I.e. handle->someBodyOp() and handle.get()->someBodyOp()
         are the same.
      */
    inline reference_type * get() const
    {
        return m_rInnerRef.get();
    }

    inline void set(reference_type *pBody)
    {
        m_rInnerRef.set(pBody);
    }

    inline void reset(reference_type *pBody)
    {
        m_rInnerRef.set(pBody);
    }

    /** Up-casting assignment operator.

        Does not work for up-casts to ambiguous bases.

        @param rRef another reference
    */
    template<typename derived_type>
    typename std::enable_if<
        vcl::detail::UpCast<reference_type, derived_type>::value,
        VclPtr &>::type
    operator =(VclPtr<derived_type> const & rRef)
    {
        m_rInnerRef.set(rRef.get());
        return *this;
    }

    VclPtr & operator =(reference_type * pBody)
    {
        m_rInnerRef.set(pBody);
        return *this;
    }

    inline operator reference_type * () const
    {
        return m_rInnerRef.get();
    }

    inline explicit operator bool () const
    {
        return m_rInnerRef.get() != NULL;
    }

    inline void clear()
    {
        m_rInnerRef.clear();
    }

    inline void reset()
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

    /** Needed to place VclPtr's into STL collection.
     */
    inline bool operator< (const VclPtr<reference_type> & handle) const
    {
        return (m_rInnerRef < handle.m_rInnerRef);
    }

    /** Needed to place VclPtr's into STL collection.
     */
    inline bool operator> (const VclPtr<reference_type> & handle) const
    {
        return (m_rInnerRef > handle.m_rInnerRef);
    }
}; // class VclPtr

template<typename T1, typename T2>
inline bool operator ==(VclPtr<T1> const & p1, VclPtr<T2> const & p2) {
    return p1.get() == p2.get();
}

template<typename T> inline bool operator ==(VclPtr<T> const & p1, T const * p2)
{
    return p1.get() == p2;
}

template<typename T> inline bool operator ==(VclPtr<T> const & p1, T * p2) {
    return p1.get() == p2;
}

template<typename T> inline bool operator ==(T const * p1, VclPtr<T> const & p2)
{
    return p1 == p2.get();
}

template<typename T> inline bool operator ==(T * p1, VclPtr<T> const & p2) {
    return p1 == p2.get();
}

template<typename T1, typename T2>
inline bool operator !=(VclPtr<T1> const & p1, VclPtr<T2> const & p2) {
    return !(p1 == p2);
}

template<typename T> inline bool operator !=(VclPtr<T> const & p1, T const * p2)
{
    return !(p1 == p2);
}

template<typename T> inline bool operator !=(VclPtr<T> const & p1, T * p2) {
    return !(p1 == p2);
}

template<typename T> inline bool operator !=(T const * p1, VclPtr<T> const & p2)
{
    return !(p1 == p2);
}

template<typename T> inline bool operator !=(T * p1, VclPtr<T> const & p2) {
    return !(p1 == p2);
}

/**
 * A construction helper for a temporary VclPtr. Since VclPtr types
 * are created with a reference-count of one - to help fit into
 * the existing code-flow; this helps us to construct them easily.
 * see also VclPtr::Create and ScopedVclPtr
 *
 * For more details on the design please see vcl/README.lifecycle
 *
 * @param reference_type must be a subclass of vcl::Window
 */
template <class reference_type>
class VclPtrInstance : public VclPtr<reference_type>
{
public:
    template<typename... Arg> VclPtrInstance(Arg &&... arg)
        : VclPtr<reference_type>( new reference_type(std::forward<Arg>(arg)...), SAL_NO_ACQUIRE )
    {
    }
};

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
    inline void disposeAndReset(reference_type *pBody)
    {
        VclPtr<reference_type>::disposeAndClear();
        VclPtr<reference_type>::set(pBody);
    }

    /**
       Assignment that releases the last reference.
     */
    inline ScopedVclPtr<reference_type>& operator= (reference_type * pBody)
    {
        disposeAndReset(pBody);
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
        assert(VclPtr<reference_type>::get() == nullptr); // make sure there are no lingering references
    }

private:
    // Most likely we don't want this default copy-construtor.
    ScopedVclPtr (const ScopedVclPtr<reference_type> &) SAL_DELETED_FUNCTION;
    // And certainly we don't want a default assignment operator.
    ScopedVclPtr<reference_type>& operator= (const ScopedVclPtr<reference_type> &) SAL_DELETED_FUNCTION;
    // And disallow reset as that doesn't call disposeAndClear on the original reference
    void reset() SAL_DELETED_FUNCTION;
    void reset(reference_type *pBody) SAL_DELETED_FUNCTION;

protected:
    inline ScopedVclPtr (reference_type * pBody, __sal_NoAcquire)
        : VclPtr<reference_type>(pBody, SAL_NO_ACQUIRE)
    {}
};

/**
 * A construction helper for ScopedVclPtr. Since VclPtr types are created
 * with a reference-count of one - to help fit into the existing
 * code-flow; this helps us to construct them easily.
 *
 * For more details on the design please see vcl/README.lifecycle
 *
 * @param reference_type must be a subclass of vcl::Window
 */
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4521) // " multiple copy constructors specified"
#endif
template <class reference_type>
class ScopedVclPtrInstance : public ScopedVclPtr<reference_type>
{
public:
    template<typename... Arg> ScopedVclPtrInstance(Arg &&... arg)
        : ScopedVclPtr<reference_type>( new reference_type(std::forward<Arg>(arg)...), SAL_NO_ACQUIRE )
    {
    }

private:
    // Prevent the above perfect forwarding ctor from hijacking (accidental)
    // attempts at ScopedVclPtrInstance copy construction (where the hijacking
    // would typically lead to somewhat obscure error messages); both non-const
    // and const variants are needed here, as the ScopedVclPtr base class has a
    // const--variant copy ctor, so the implicitly declared copy ctor for
    // ScopedVclPtrInstance would also be the const variant, so non-const copy
    // construction attempts would be hijacked by the perfect forwarding ctor;
    // but if we only declared a non-const variant here, the const variant would
    // no longer be implicitly declared (as there would already be an explicitly
    // declared copy ctor), so const copy construction attempts would then be
    // hijacked by the perfect forwarding ctor:
    ScopedVclPtrInstance(ScopedVclPtrInstance &) = delete;
    ScopedVclPtrInstance(ScopedVclPtrInstance const &) = delete;
};
#if defined _MSC_VER
#pragma warning(pop)
#endif

#endif // INCLUDED_VCL_PTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
