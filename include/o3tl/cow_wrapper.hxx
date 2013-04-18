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

#ifndef INCLUDED_O3TL_COW_WRAPPER_HXX
#define INCLUDED_O3TL_COW_WRAPPER_HXX

#include <osl/interlck.h>

#include <algorithm>

#include <boost/utility.hpp>
#include <boost/checked_delete.hpp>

namespace o3tl
{
    /** Thread-unsafe refcounting

        This is the default locking policy for cow_wrapper. No
        locking/guarding against concurrent access is performed
        whatsoever.
     */
    struct UnsafeRefCountingPolicy
    {
        typedef sal_uInt32 ref_count_t;
        static void incrementCount( ref_count_t& rCount ) { ++rCount; }
        static bool decrementCount( ref_count_t& rCount ) { return --rCount != 0; }
    };

    /** Thread-safe refcounting

        Use this to have the cow_wrapper refcounting mechanisms employ
        the thread-safe oslInterlockedCount .
     */
    struct ThreadSafeRefCountingPolicy
    {
        typedef oslInterlockedCount ref_count_t;
        static void incrementCount( ref_count_t& rCount ) { osl_atomic_increment(&rCount); }
        static bool decrementCount( ref_count_t& rCount )
        {
            if( rCount == 1 ) // caller is already the only/last reference
                return false;
            else
                return osl_atomic_decrement(&rCount) != 0;
        }
    };

    /** Copy-on-write wrapper.

        This template provides copy-on-write semantics for the wrapped
        type: when copying, the operation is performed shallow,
        i.e. different cow_wrapper objects share the same underlying
        instance. Only when accessing the underlying object via
        non-const methods, a unique copy is provided.

        The type parameter <code>T</code> must satisfy the following
        requirements: it must be default-constructible, copyable (it
        need not be assignable), and be of non-reference type. Note
        that, despite the fact that this template provides access to
        the wrapped type via pointer-like methods
        (<code>operator->()</code> and <code>operator*()</code>), it does
        <em>not</em> work like e.g. the boost pointer wrappers
        (shared_ptr, scoped_ptr, etc.). Internally, the cow_wrapper
        holds a by-value instance of the wrapped object. This is to
        avoid one additional heap allocation, and providing access via
        <code>operator->()</code>/<code>operator*()</code> is because
        <code>operator.()</code> cannot be overridden.

        Regarding thread safety: this wrapper is <em>not</em>
        thread-safe per se, because cow_wrapper has no way of
        syncronizing the potentially many different cow_wrapper
        instances, that reference a single shared value_type
        instance. That said, when passing
        <code>ThreadSafeRefCountingPolicy</code> as the
        <code>MTPolicy</code> parameter, accessing a thread-safe
        pointee through multiple cow_wrapper instances might be
        thread-safe, if the individual pointee methods are
        thread-safe, <em>including</em> pointee's copy
        constructor. Any wrapped object that needs external
        synchronisation (e.g. via an external mutex, which arbitrates
        access to object methods, and can be held across multiple
        object method calls) cannot easily be dealt with in a
        thread-safe way, because, as noted, objects are shared behind
        the client's back.

        @attention if one wants to use the pimpl idiom together with
        cow_wrapper (i.e. put an opaque type into the cow_wrapper),
        then <em>all<em> methods in the surrounding class needs to be
        non-inline (<em>including</em> destructor, copy constructor
        and assignment operator).

        @example
        <pre>
class cow_wrapper_client_impl;

class cow_wrapper_client
{
public:
    cow_wrapper_client();
    cow_wrapper_client( const cow_wrapper_client& );
    ~cow_wrapper_client();

    cow_wrapper_client& operator=( const cow_wrapper_client& );

    void modify( int nVal );
    int queryUnmodified() const;

private:
    otl::cow_wrapper< cow_wrapper_client_impl > maImpl;
};
        </pre>
        and the implementation file would look like this:
        <pre>
class cow_wrapper_client_impl
{
public:
    void setValue( int nVal ) { mnValue = nVal; }
    int getValue() const { return mnValue; }

private:
    int mnValue;
}

cow_wrapper_client::cow_wrapper_client() :
    maImpl()
{
}
cow_wrapper_client::cow_wrapper_client( const cow_wrapper_client& rSrc ) :
    maImpl( rSrc.maImpl )
{
}
cow_wrapper_client::~cow_wrapper_client()
{
}
cow_wrapper_client& cow_wrapper_client::operator=( const cow_wrapper_client& rSrc )
{
    maImpl = rSrc.maImpl;
    return *this;
}
void cow_wrapper_client::modify( int nVal )
{
    maImpl->setValue( nVal );
}
int cow_wrapper_client::queryUnmodified() const
{
    return maImpl->getValue();
}
        </pre>
     */
    template<typename T, class MTPolicy=UnsafeRefCountingPolicy> class cow_wrapper
    {
        /** shared value object - gets cloned before cow_wrapper hands
            out a non-const reference to it
         */
        struct impl_t : private boost::noncopyable
        {
            impl_t() :
                m_value(),
                m_ref_count(1)
            {
            }

            explicit impl_t( const T& v ) :
                m_value(v),
                m_ref_count(1)
            {
            }

            T                              m_value;
            typename MTPolicy::ref_count_t m_ref_count;
        };

        void release()
        {
            if( !MTPolicy::decrementCount(m_pimpl->m_ref_count) )
                boost::checked_delete(m_pimpl), m_pimpl=0;
        }

    public:
        typedef T        value_type;
        typedef T*       pointer;
        typedef const T* const_pointer;
        typedef MTPolicy mt_policy;

        /** Default-construct wrapped type instance
         */
        cow_wrapper() :
            m_pimpl( new impl_t() )
        {
        }

        /** Copy-construct wrapped type instance from given object
         */
        explicit cow_wrapper( const value_type& r ) :
            m_pimpl( new impl_t(r) )
        {
        }

        /** Shallow-copy given cow_wrapper
         */
        explicit cow_wrapper( const cow_wrapper& rSrc ) : // nothrow
            m_pimpl( rSrc.m_pimpl )
        {
            MTPolicy::incrementCount( m_pimpl->m_ref_count );
        }

        ~cow_wrapper() // nothrow, if ~T does not throw
        {
            release();
        }

        /// now sharing rSrc cow_wrapper instance with us
        cow_wrapper& operator=( const cow_wrapper& rSrc ) // nothrow
        {
            // this already guards against self-assignment
            MTPolicy::incrementCount( rSrc.m_pimpl->m_ref_count );

            release();
            m_pimpl = rSrc.m_pimpl;

            return *this;
        }

        /// unshare with any other cow_wrapper instance
        value_type& make_unique()
        {
            if( m_pimpl->m_ref_count > 1 )
            {
                impl_t* pimpl = new impl_t(m_pimpl->m_value);
                release();
                m_pimpl = pimpl;
            }

            return m_pimpl->m_value;
        }

        /// true, if not shared with any other cow_wrapper instance
        bool is_unique() const // nothrow
        {
            return m_pimpl->m_ref_count == 1;
        }

        /// return number of shared instances (1 for unique object)
        typename MTPolicy::ref_count_t use_count() const // nothrow
        {
            return m_pimpl->m_ref_count;
        }

        void swap(cow_wrapper& r) // never throws
        {
            std::swap(m_pimpl, r.m_pimpl);
        }

        pointer           operator->()       { return &make_unique(); }
        value_type&       operator*()        { return make_unique(); }
        const_pointer     operator->() const { return &m_pimpl->m_value; }
        const value_type& operator*()  const { return m_pimpl->m_value; }

        pointer           get()       { return &make_unique(); }
        const_pointer     get() const { return &m_pimpl->m_value; }

        /// true, if both cow_wrapper internally share the same object
        bool              same_object( const cow_wrapper& rOther ) const
        {
            return rOther.m_pimpl == m_pimpl;
        }

    private:
        impl_t* m_pimpl;
    };


    template<class T, class P> inline bool operator==( const cow_wrapper<T,P>& a,
                                                       const cow_wrapper<T,P>& b )
    {
        return a.same_object(b) ? true : *a == *b;
    }

    template<class T, class P> inline bool operator!=( const cow_wrapper<T,P>& a,
                                                       const cow_wrapper<T,P>& b )
    {
        return a.same_object(b) ? false : *a != *b;
    }

    template<class A, class B, class P> inline bool operator<( const cow_wrapper<A,P>& a,
                                                               const cow_wrapper<B,P>& b )
    {
        return *a < *b;
    }

    template<class T, class P> inline void swap( cow_wrapper<T,P>& a,
                                                 cow_wrapper<T,P>& b )
    {
        a.swap(b);
    }

    // to enable boost::mem_fn on cow_wrapper
    template<class T, class P> inline T * get_pointer( const cow_wrapper<T,P>& r )
    {
        return r.get();
    }

}

#endif /* INCLUDED_O3TL_COW_WRAPPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
