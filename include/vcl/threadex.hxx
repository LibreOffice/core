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

#ifndef INCLUDED_VCL_THREADEX_HXX
#define INCLUDED_VCL_THREADEX_HXX

#include <osl/conditn.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>

#include <boost/optional.hpp>
#include <memory>

namespace vcl
{
    class VCL_DLLPUBLIC SolarThreadExecutor
    {
        osl::Condition          m_aStart;
        osl::Condition          m_aFinish;
        bool                    m_bTimeout;

        DECL_DLLPRIVATE_LINK( worker, void*, void );

    public:
        SolarThreadExecutor();
        virtual ~SolarThreadExecutor();

        virtual void doIt() = 0;
        void execute();
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
        ::std::unique_ptr<ExecutorT> const pExecutor( new ExecutorT(func) );
        pExecutor->execute();
        if (pExecutor->m_exc)
            std::rethrow_exception(pExecutor->m_exc);
        return *pExecutor->m_result;
    }

private:
    explicit GenericSolarThreadExecutor( FuncT const& func )
        : m_func(func), m_result() {}

    virtual void doIt() override
    {
        try {
            m_result.reset( m_func() );
        }
        catch (...) {
            m_exc = std::current_exception();
        }
    }

    std::exception_ptr m_exc;
#ifdef _MSC_VER
    FuncT m_func; // "const" and std::bind() results in Error C3848 expression would lose const-volatile qualifiers
#else
    FuncT const m_func;
#endif
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
        ::std::unique_ptr<ExecutorT> const pExecutor( new ExecutorT(func) );
        pExecutor->execute();
        if (pExecutor->m_exc)
            std::rethrow_exception(pExecutor->m_exc);
    }

private:
    explicit GenericSolarThreadExecutor( FuncT const& func )
        : m_func(func) {}

    virtual void doIt() override
    {
        try {
            m_func();
        }
        catch (...) {
            m_exc = std::current_exception();
        }
    }

    std::exception_ptr m_exc;
    FuncT const m_func;
};

} // namespace detail


/** This function will execute the passed functor synchronously in the
    solar thread, thus the calling thread will (eventually) be blocked until
    the functor has been called.
    Any exception that came up calling the functor in the solar thread
    will be caught and rethrown in the calling thread.
    The result type of this function needs to be default constructable.
    Please keep in mind not to pass addresses to stack variables
    (e.g. for out parameters) to foreign threads, use inout_by_ref()
    for this purpose.  For in parameters, this may not affect you, because
    the functor object is copy constructed into free store.  This way
    you must not use \verbatim std::cref()/std::ref() \endverbatim or similar
    for objects on your thread's stack.
    Use inout_by_ref() or inout_by_ptr() for this purpose, e.g.

    \code{.cpp}
        using namespace vcl::solarthread;

        long n = 3;
        // calling foo( long & r ):
        syncExecute( std::bind( &foo, inout_by_ref(n) ) );
        // calling foo( long * p ):
        syncExecute( std::bind( &foo, inout_by_ptr(&n) ) );

        char const* pc = "default";
        // calling foo( char const** ppc ):
        syncExecute( std::bind( &foo, inout_by_ptr(&pc) ) );
        // calling foo( char const*& rpc ):
        syncExecute( std::bind( &foo, inout_by_ref(pc) ) );
    \endcode

    @tpl ResultT result type, defaults to FuncT::result_type to seamlessly
                 support mem_fn and bind
    @tpl FuncT functor type, let your compiler deduce this type
    @param func functor object to be executed in solar thread
    @return return value of functor
*/
template <typename FuncT>
inline auto syncExecute(FuncT const& func) -> decltype(func())
{
    return detail::GenericSolarThreadExecutor<
        FuncT, decltype(func())>::exec(func);
}

} // namespace solarthread
} // namespace vcl

#endif // INCLUDED_VCL_THREADEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
