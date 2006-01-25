/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cow_wrapper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2006-01-25 16:14:25 $
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

#ifndef INCLUDED_O3TL_COW_WRAPPER_HXX
#define INCLUDED_O3TL_COW_WRAPPER_HXX

#include <osl/interlck.h>

#include <algorithm>

#include <boost/utility.hpp>
#include <boost/checked_delete.hpp>

namespace o3tl
{
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
        instance. Accessing a thread-safe pointee through multiple
        cow_wrapper instances might be thread-safe, if the individual
        pointee methods are thread-safe, <em>including</em> pointee's
        copy constructor. Any wrapped object that needs external
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
}
void cow_wrapper_client::modify( int nVal )
{
    maImpl->setValue( nVal );
}
void cow_wrapper_client::queryUnmodified() const
{
    return maImpl->getValue();
}
        </pre>
     */
    template<typename T> class cow_wrapper
    {
        /** shared value object - gets copied before cow_wrapper hands
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

            T                   m_value;
            oslInterlockedCount m_ref_count;
        };

        void release()
        {
            if( osl_decrementInterlockedCount(&m_pimpl->m_ref_count) == 0 )
                boost::checked_delete(m_pimpl), m_pimpl=0;
        }

    public:
        typedef T  element_type;
        typedef T  value_type;
        typedef T* pointer;

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
            osl_incrementInterlockedCount( &m_pimpl->m_ref_count );
        }

        ~cow_wrapper() // nothrow, if ~T does not throw
        {
            release();
        }

        /// now sharing rSrc cow_wrapper instance with us
        cow_wrapper& operator=( const cow_wrapper& rSrc ) // nothrow
        {
            // this already guards against self-assignment
            osl_incrementInterlockedCount( &rSrc.m_pimpl->m_ref_count );

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
        oslInterlockedCount use_count() const // nothrow
        {
            return m_pimpl->m_ref_count;
        }

        void swap(cow_wrapper& r) // never throws
        {
            std::swap(m_pimpl, r.m_pimpl);
        }

        pointer           operator->()       { return &make_unique(); }
        value_type&       operator*()        { return make_unique(); }
        const pointer     operator->() const { return &m_pimpl->m_value; }
        const value_type& operator*()  const { return m_pimpl->m_value; }

        pointer           get()       { return &make_unique(); }
        const pointer     get() const { return &m_pimpl->m_value; }

        /// true, if both cow_wrapper internally share the same object
        bool              same_object( const cow_wrapper& rOther ) const
        {
            return rOther.m_pimpl == m_pimpl;
        }

    private:
        impl_t* m_pimpl;
    };


    template<class A, class B> inline bool operator==( const cow_wrapper<A>& a,
                                                       const cow_wrapper<B>& b )
    {
        return *a == *b;
    }

    template<class A, class B> inline bool operator!=( const cow_wrapper<A>& a,
                                                       const cow_wrapper<B>& b )
    {
        return *a != *b;
    }

    template<class A, class B> inline bool operator<( const cow_wrapper<A>& a,
                                                      const cow_wrapper<B>& b )
    {
        return *a < *b;
    }

    template<class T> inline void swap( cow_wrapper<T>& a,
                                        cow_wrapper<T>& b )
    {
        a.swap(b);
    }

    // to enable boost::mem_fn on cow_wrapper
    template<class T> inline T * get_pointer( const cow_wrapper<T>& r )
    {
        return r.get();
    }

}

#endif /* INCLUDED_O3TL_COW_WRAPPER_HXX */
