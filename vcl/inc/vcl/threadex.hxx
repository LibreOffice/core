/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: threadex.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <osl/conditn.h>
#include <osl/thread.h>
#include <tools/link.hxx>
#include <vcl/dllapi.h>

#if ! defined(_CPPUHELPER_EXC_HLP_HXX_)
#include "cppuhelper/exc_hlp.hxx"
#endif
#include "boost/optional.hpp"
#include <memory>

namespace vcl
{
    class VCL_DLLPUBLIC ThreadExecutor
    {
        oslThread               m_aThread;
        oslCondition            m_aFinish;
        long                    m_nReturn;

    #ifdef THREADEX_IMPLEMENTATION
    public:
        SAL_DLLPRIVATE static void SAL_CALL worker( void* );
    #endif
    public:
        ThreadExecutor();
        virtual ~ThreadExecutor();

        virtual long doIt() = 0;
        long execute();
    };

    class VCL_DLLPUBLIC SolarThreadExecutor
    {
        oslCondition            m_aStart;
        oslCondition            m_aFinish;
        long                    m_nReturn;
        bool                    m_bTimeout;

        DECL_DLLPRIVATE_LINK( worker, void* );

    public:
        SolarThreadExecutor();
        virtual ~SolarThreadExecutor();

        virtual long doIt() = 0;
        long execute() { return impl_execute( NULL ); }
        // caution: timeout for getting the solar mutex, not for ending
        // the operation of doIt(). If doIt actually gets called within
        // the specified timeout, execute will only return after
        // doIt() completed
        long execute( const TimeValue& _rTimeout ) { return impl_execute( &_rTimeout ); }

    public:
        bool    didTimeout() const { return m_bTimeout; }

    private:
        long impl_execute( const TimeValue* _pTimeout );
    };

namespace solarthread {

/// @internal
namespace detail {

template <typename FuncT, typename ResultT>
class GenericSolarThreadExecutor : public SolarThreadExecutor
{
public:
    static ResultT exec( FuncT const& func )
    {
        typedef GenericSolarThreadExecutor<FuncT, ResultT> ExecutorT;
        ::std::auto_ptr<ExecutorT> const pExecutor( new ExecutorT(func) );
        pExecutor->execute();
#if ! defined(EXCEPTIONS_OFF)
        if (pExecutor->m_exc.hasValue())
            ::cppu::throwException( pExecutor->m_exc );
#endif
        return *pExecutor->m_result;
    }

private:
    explicit GenericSolarThreadExecutor( FuncT const& func )
        : m_exc(), m_func(func), m_result() {}

    virtual long doIt()
    {
#if defined(EXCEPTIONS_OFF)
        m_result.reset( m_func() );
#else
        try {
            m_result.reset( m_func() );
        }
        catch (::com::sun::star::uno::Exception &) {
            // only UNO exceptions can be dispatched:
            m_exc = ::cppu::getCaughtException();
        }
#endif
        return 0;
    }

    ::com::sun::star::uno::Any m_exc;
    FuncT const m_func;
    // using boost::optional here omits the need that ResultT is default
    // constructable:
    ::boost::optional<ResultT> m_result;
};

template <typename FuncT>
class GenericSolarThreadExecutor<FuncT, void> : public SolarThreadExecutor
{
public:
    static void exec( FuncT const& func )
    {
        typedef GenericSolarThreadExecutor<FuncT, void> ExecutorT;
        ::std::auto_ptr<ExecutorT> const pExecutor( new ExecutorT(func) );
        pExecutor->execute();
#if ! defined(EXCEPTIONS_OFF)
        if (pExecutor->m_exc.hasValue())
            ::cppu::throwException( pExecutor->m_exc );
#endif
    }

private:
    explicit GenericSolarThreadExecutor( FuncT const& func )
        : m_exc(), m_func(func) {}

    virtual long doIt()
    {
#if defined(EXCEPTIONS_OFF)
        m_func();
#else
        try {
            m_func();
        }
        catch (::com::sun::star::uno::Exception &) {
            // only UNO exceptions can be dispatched:
            m_exc = ::cppu::getCaughtException();
        }
#endif
        return 0;
    }

    ::com::sun::star::uno::Any m_exc;
    FuncT const m_func;
};

template <typename T>
class copy_back_wrapper
{
public:
    operator T *() const { return &m_holder->m_value; }
    operator T &() const { return m_holder->m_value; }

    explicit copy_back_wrapper( T * p ) : m_holder( new data_holder(p) ) {}

    // no thread-safe counting needed here, because calling thread blocks
    // until solar thread has executed the functor.
    copy_back_wrapper( copy_back_wrapper<T> const& r )
        : m_holder(r.m_holder) { ++m_holder->m_refCount; }
    ~copy_back_wrapper() {
        --m_holder->m_refCount;
        if (m_holder->m_refCount == 0) {
            delete m_holder;
        }
    }
private:
    struct data_holder {
        T m_value;
        T * const m_ptr;
        sal_Int32 m_refCount;
        data_holder( T * p ) : m_value(*p), m_ptr(p), m_refCount(1) {}
        ~data_holder() { *m_ptr = m_value; }
    };
    data_holder * const m_holder;
};

} // namespace detail

/** Makes a copy back reference wrapper to be used for inout parameters.
    Only use for syncExecute(), the returned wrapper relies on its
    implemenation, i.e. the function object is stored in free store.
    Type T needs to be copy constructable assignable.

    @see syncExecute()
    @param r reference to a stack variable
    @return reference wrapper
*/
template <typename T>
inline detail::copy_back_wrapper<T> inout_by_ref( T & r )
{
    return detail::copy_back_wrapper<T>(&r);
}

/** Makes a copy back ptr wrapper to be used for inout parameters.
    Only use for syncExecute(), the returned wrapper relies on its
    implemenation, i.e. the function object is stored in free store.
    Type T needs to be copy constructable assignable.

    @see syncExecute()
    @param p pointer to a stack variable
    @return ptr wrapper
*/
template <typename T>
inline detail::copy_back_wrapper<T> inout_by_ptr( T * p )
{
    return detail::copy_back_wrapper<T>(p);
}

/** This function will execute the passed functor synchronously in the
    solar thread, thus the calling thread will (eventually) be blocked until
    the functor has been called.
    Any UNO exception that came up calling the functor in the solar thread
    will be caught and rethrown in the calling thread.  Any non-UNO
    exception needs to be handled by the called functor.
    The result type of this function needs to be default constructable.
    Please keep in mind not to pass addresses to stack variables
    (e.g. for out parameters) to foreign threads, use inout_by_ref()
    for this purpose.  For in parameters, this may not affect you, because
    the functor object is copy constructed into free store.  This way
    you must not use boost::cref()/boost::ref() or similar for objects on
    your thread's stack.
    Use inout_by_ref() or inout_by_ptr() for this purpose, e.g.

    <pre>
        using namespace vcl::solarthread;

        long n = 3;
        // calling foo( long & r ):
        syncExecute( boost::bind( &foo, inout_by_ref(n) ) );
        // calling foo( long * p ):
        syncExecute( boost::bind( &foo, inout_by_ptr(&n) ) );

        char const* pc = "default";
        // calling foo( char const** ppc ):
        syncExecute( boost::bind( &foo, inout_by_ptr(&pc) ) );
        // calling foo( char const*& rpc ):
        syncExecute( boost::bind( &foo, inout_by_ref(pc) ) );
    </pre>

    @tpl ResultT result type, defaults to FuncT::result_type to seamlessly
                 support mem_fn and bind
    @tpl FuncT functor type, let your compiler deduce this type
    @param func functor object to be executed in solar thread
    @return return value of functor
*/
template <typename ResultT, typename FuncT>
inline ResultT syncExecute( FuncT const& func )
{
    return detail::GenericSolarThreadExecutor<FuncT, ResultT>::exec(func);
}

template <typename FuncT>
inline typename FuncT::result_type syncExecute( FuncT const& func )
{
    return detail::GenericSolarThreadExecutor<
        FuncT, typename FuncT::result_type>::exec(func);
}

} // namespace solarthread
} // namespace vcl

