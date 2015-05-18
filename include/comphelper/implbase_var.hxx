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

/** This header generates the following template classes with a variable number
    of interfaces:

    comphelper::PartialWeakComponentImplHelper<N> <typename Ifc1, ...,
                                            typename Ifc<N> >

    as already present in headers cppuhelper/implbase<1-12>.hxx and
    cppuhelper/compbase<1-12>.hxx.
    <N> denotes the number of interface types passed as template arguments.
    Don't use this header for interface numbers up to 12;
    always use the existing cppuhelper/(impl|comp)base<1-12>.hxx headers
    for this purpose, which eases debugging.

    Including this header requires a little discipline, because it has no
    include guards.  Please use the following external include guard rule
    where <N> is the number of interface types:

    #ifndef INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_<N>
    #define INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_<N>
    #define COMPHELPER_IMPLBASE_INTERFACE_NUMBER <N>
    #include <comphelper/implbase_var.hxx>
    #endif

    Additionally you can

    #define COMPHELPER_IMPLBASE_MAX_CTOR_ARGS <N>

    to control the maximum number of templated ctor arguments for the
    ImplInheritanceHelper<N> classes.
    The default is a maximum of 6 arguments.
*/

#ifndef COMPHELPER_IMPLBASE_INTERFACE_NUMBER
#error "you have to define COMPHELPER_IMPLBASE_INTERFACE_NUMBER prior to including comphelper/implbase_var.hxx!"
#endif // ! defined(COMPHELPER_IMPLBASE_INTERFACE_NUMBER)

#if !defined(COMPHELPER_IMPLBASE_TEST_PHASE) && COMPHELPER_IMPLBASE_INTERFACE_NUMBER <= 12
#error "include proper header file: cppuhelper/implbase<N>.hxx or cppuhelper/compbase<N>.hxx!"
#endif

#ifndef COMPHELPER_IMPLBASE_MAX_CTOR_ARGS
#define COMPHELPER_IMPLBASE_MAX_CTOR_ARGS 6 // default
#endif

#include <cppuhelper/implbase_ex.hxx>
#include <rtl/instance.hxx>
#include <cppuhelper/compbase_ex.hxx>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic/add.hpp>

namespace comphelper {

// Suppress warnings about hidden functions in case any of the IfcN has
// functions named dispose, addEventListener, or removeEventListener:

namespace detail {

struct BOOST_PP_CAT(class_data, COMPHELPER_IMPLBASE_INTERFACE_NUMBER)
{
    sal_Int16 m_nTypes;
    sal_Bool m_storedTypeRefs;
    sal_Bool m_storedId;
    sal_Int8 m_id[16];
    ::cppu::type_entry m_typeEntries[COMPHELPER_IMPLBASE_INTERFACE_NUMBER + 1];
};

/// @internal
template < BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER,
                                typename Ifc), typename Impl >
struct BOOST_PP_CAT(ImplClassData, COMPHELPER_IMPLBASE_INTERFACE_NUMBER)
{
    ::cppu::class_data * operator()() {
        static BOOST_PP_CAT(class_data, COMPHELPER_IMPLBASE_INTERFACE_NUMBER)
        s_cd = {
            COMPHELPER_IMPLBASE_INTERFACE_NUMBER + 1, sal_False, sal_False,
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            {
#define COMPHELPER_IMPLBASE_classdataList(z_, n_, unused_) \
{ { BOOST_PP_CAT(Ifc, n_)::static_type }, \
  reinterpret_cast<sal_IntPtr>( static_cast< BOOST_PP_CAT(Ifc, n_) * >( \
                               reinterpret_cast<Impl *>(16) ) ) - 16 },
                BOOST_PP_REPEAT(COMPHELPER_IMPLBASE_INTERFACE_NUMBER,
                                COMPHELPER_IMPLBASE_classdataList, ~)
#undef COMPHELPER_IMPLBASE_classdataList
                { { ::com::sun::star::lang::XTypeProvider::static_type },
                  reinterpret_cast<sal_IntPtr>(
                      static_cast< ::com::sun::star::lang::XTypeProvider * >(
                          reinterpret_cast<Impl *>(16) ) ) - 16 }
            }
        };
        return reinterpret_cast< ::cppu::class_data * >(&s_cd);
    }
};

}

template < BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER,
                                typename Ifc) >
class SAL_NO_VTABLE BOOST_PP_CAT(PartialWeakComponentImplHelper,
                                 COMPHELPER_IMPLBASE_INTERFACE_NUMBER)
    : public ::cppu::WeakComponentImplHelperBase,
      public ::com::sun::star::lang::XTypeProvider,
      BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER, public Ifc)
{
    /// @internal
    struct cd : public ::rtl::StaticAggregate<
        ::cppu::class_data,
        BOOST_PP_CAT(detail::ImplClassData,
                     COMPHELPER_IMPLBASE_INTERFACE_NUMBER)
        <
            BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER, Ifc),
            BOOST_PP_CAT(PartialWeakComponentImplHelper,
                         COMPHELPER_IMPLBASE_INTERFACE_NUMBER)<
                BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER, Ifc)>
        > > {};

public:
    BOOST_PP_CAT(PartialWeakComponentImplHelper, COMPHELPER_IMPLBASE_INTERFACE_NUMBER)(
        ::osl::Mutex & rMutex ) : WeakComponentImplHelperBase(rMutex) {}

    virtual ::com::sun::star::uno::Any
        SAL_CALL queryInterface( ::com::sun::star::uno::Type const& rType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        return ::cppu::WeakComponentImplHelper_query(
            rType, cd::get(), this,
            static_cast< ::cppu::WeakComponentImplHelperBase * >(this) );
    }
    virtual void SAL_CALL acquire() throw () SAL_OVERRIDE
        { WeakComponentImplHelperBase::acquire(); }
    virtual void SAL_CALL release() throw () SAL_OVERRIDE
        { WeakComponentImplHelperBase::release(); }
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
        SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return ::cppu::WeakComponentImplHelper_getTypes( cd::get() ); }
    virtual ::com::sun::star::uno::Sequence<sal_Int8>
        SAL_CALL getImplementationId()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return ::cppu::ImplHelper_getImplementationId( cd::get() ); }
};


}

// undef for multiple use/inclusion of this header:
#undef COMPHELPER_IMPLBASE_MAX_CTOR_ARGS
#undef COMPHELPER_IMPLBASE_INTERFACE_NUMBER

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
