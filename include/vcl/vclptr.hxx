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

#include <sal/config.h>

#include <config_global.h>
#include <rtl/ref.hxx>
#include <vcl/vclreferencebase.hxx>

#include <utility>
#include <type_traits>

#ifdef DBG_UTIL
#ifndef WNT
#include <vcl/vclmain.hxx>
#endif
#endif

#if !(defined _MSC_VER && _MSC_VER <= 1900 && !defined __clang__)

namespace vcl { namespace detail {

template<typename>
constexpr bool isIncompleteOrDerivedFromVclReferenceBase(...) { return true; }

template<typename T> constexpr bool isIncompleteOrDerivedFromVclReferenceBase(
    int (*)[sizeof(T)])
{ return std::is_base_of<VclReferenceBase, T>::value; }

}; }; // namespace detail, namespace vcl

#endif

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
#if !(defined _MSC_VER && _MSC_VER <= 1900 && !defined __clang__)
    static_assert(
        vcl::detail::isIncompleteOrDerivedFromVclReferenceBase<reference_type>(
            nullptr),
        "template argument type must be derived from VclReferenceBase");
#endif

    ::rtl::Reference<reference_type> m_rInnerRef;

public:
    /** Constructor...
     */
    VclPtr()
        : m_rInnerRef()
    {}

    /** Constructor...
     */
    VclPtr (reference_type * pBody)
        : m_rInnerRef(pBody)
    {}

    /** Constructor... that doesn't take a ref.
     */
    VclPtr (reference_type * pBody, __sal_NoAcquire)
        : m_rInnerRef(pBody, SAL_NO_ACQUIRE)
    {}

    /** Up-casting conversion constructor: Copies interface reference.

        Does not work for up-casts to ambiguous bases.  For the special case of
        up-casting to Reference< XInterface >, see the corresponding conversion
        operator.

        @param rRef another reference
    */
    template< class derived_type >
    VclPtr(
        const VclPtr< derived_type > & rRef,
        typename std::enable_if<
            std::is_base_of<reference_type, derived_type>::value, int>::type
            = 0 )
        : m_rInnerRef( static_cast<reference_type*>(rRef) )
    {
    }

#if defined(DBG_UTIL) && !defined(WNT)
    virtual ~VclPtr()
    {
        assert(m_rInnerRef.get() == nullptr || vclmain::isAlive());
        // We can be one of the intermediate counts, but if we are the last
        // VclPtr keeping this object alive, then something forgot to call dispose().
        assert((!m_rInnerRef.get() || m_rInnerRef->isDisposed() || m_rInnerRef->getRefCount() > 1)
                && "someone forgot to call dispose()");
    }
    VclPtr(VclPtr const &) = default;
    VclPtr(VclPtr &&) = default;
    VclPtr & operator =(VclPtr const &) = default;
    VclPtr & operator =(VclPtr &&) = default;
#endif

    /**
     * A construction helper for VclPtr. Since VclPtr types are created
     * with a reference-count of one - to help fit into the existing
     * code-flow; this helps us to construct them easily.
     *
     * For more details on the design please see vcl/README.lifecycle
     *
     * @tparam reference_type must be a subclass of vcl::Window
     */
    template<typename... Arg> static SAL_WARN_UNUSED_RESULT VclPtr< reference_type > Create(Arg &&... arg)
    {
        return VclPtr< reference_type >( new reference_type(std::forward<Arg>(arg)...), SAL_NO_ACQUIRE );
    }

    /** Probably most common used: handle->someBodyOp().
     */
    reference_type * operator->() const
    {
        return m_rInnerRef.get();
    }

    /** Get the body. Can be used instead of operator->().
         I.e. handle->someBodyOp() and handle.get()->someBodyOp()
         are the same.
      */
    reference_type * get() const
    {
        return m_rInnerRef.get();
    }

    void set(reference_type *pBody)
    {
        m_rInnerRef.set(pBody);
    }

    void reset(reference_type *pBody)
    {
        m_rInnerRef.set(pBody);
    }

    /** Up-casting copy assignment operator.

        Does not work for up-casts to ambiguous bases.

        @param rRef another reference
    */
    template<typename derived_type>
    typename std::enable_if<
        std::is_base_of<reference_type, derived_type>::value,
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

    operator reference_type * () const
    {
        return m_rInnerRef.get();
    }

    explicit operator bool () const
    {
        return m_rInnerRef.get() != nullptr;
    }

    void clear()
    {
        m_rInnerRef.clear();
    }

    void reset()
    {
        m_rInnerRef.clear();
    }

    void disposeAndClear()
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
    bool operator< (const VclPtr<reference_type> & handle) const
    {
        return (m_rInnerRef < handle.m_rInnerRef);
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
class SAL_WARN_UNUSED VclPtrInstance : public VclPtr<reference_type>
{
public:
    template<typename... Arg> VclPtrInstance(Arg &&... arg)
        : VclPtr<reference_type>( new reference_type(std::forward<Arg>(arg)...), SAL_NO_ACQUIRE )
    {
    }

    /**
     * Override and disallow this, to prevent people accidentally calling it and actually
     * getting VclPtr::Create and getting a naked VclPtr<> instance
     */
    template<typename... Arg> static VclPtrInstance< reference_type > Create(Arg &&... ) = delete;
};

template <class reference_type>
class ScopedVclPtr : public VclPtr<reference_type>
{
public:
    /** Constructor...
     */
    ScopedVclPtr()
        : VclPtr<reference_type>()
    {}

    /** Constructor
     */
    ScopedVclPtr (reference_type * pBody)
        : VclPtr<reference_type>(pBody)
    {}

    /** Copy constructor...
     */
    ScopedVclPtr (const VclPtr<reference_type> & handle)
        : VclPtr<reference_type>(handle)
    {}

    /**
       Assignment that releases the last reference.
     */
    void disposeAndReset(reference_type *pBody)
    {
        if (pBody != this->get()) {
            VclPtr<reference_type>::disposeAndClear();
            VclPtr<reference_type>::set(pBody);
        }
    }

    /**
       Assignment that releases the last reference.
     */
    ScopedVclPtr<reference_type>& operator = (reference_type * pBody)
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
    ScopedVclPtr(
        const VclPtr< derived_type > & rRef,
        typename std::enable_if<
            std::is_base_of<reference_type, derived_type>::value, int>::type
            = 0 )
        : VclPtr<reference_type>( rRef )
    {
    }

    /** Up-casting assignment operator.

        Does not work for up-casts to ambiguous bases.

        @param rRef another VclPtr
    */
    template<typename derived_type>
    typename std::enable_if<
        std::is_base_of<reference_type, derived_type>::value,
        ScopedVclPtr &>::type
    operator =(VclPtr<derived_type> const & rRef)
    {
        disposeAndReset(rRef.get());
        return *this;
    }

    /**
     * Override and disallow this, to prevent people accidentally calling it and actually
     * getting VclPtr::Create and getting a naked VclPtr<> instance
     */
    template<typename... Arg> static ScopedVclPtr< reference_type > Create(Arg &&... ) = delete;

    ~ScopedVclPtr()
    {
        VclPtr<reference_type>::disposeAndClear();
        assert(VclPtr<reference_type>::get() == nullptr); // make sure there are no lingering references
    }

private:
    // Most likely we don't want this default copy-constructor.
    ScopedVclPtr (const ScopedVclPtr<reference_type> &) = delete;
    // And certainly we don't want a default assignment operator.
    ScopedVclPtr<reference_type>& operator = (const ScopedVclPtr<reference_type> &) = delete;
    // And disallow reset as that doesn't call disposeAndClear on the original reference
    void reset() = delete;
    void reset(reference_type *pBody) = delete;

protected:
    ScopedVclPtr (reference_type * pBody, __sal_NoAcquire)
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
class SAL_WARN_UNUSED ScopedVclPtrInstance : public ScopedVclPtr<reference_type>
{
public:
    template<typename... Arg> ScopedVclPtrInstance(Arg &&... arg)
        : ScopedVclPtr<reference_type>( new reference_type(std::forward<Arg>(arg)...), SAL_NO_ACQUIRE )
    {
    }

    /**
     * Override and disallow this, to prevent people accidentally calling it and actually
     * getting VclPtr::Create and getting a naked VclPtr<> instance
     */
    template<typename... Arg> static ScopedVclPtrInstance< reference_type > Create(Arg &&...) = delete;

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
