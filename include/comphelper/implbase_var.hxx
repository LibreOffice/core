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

    comphelper::ImplHelper<N> <typename Ifc1, ..., typename Ifc<N> >
    comphelper::WeakImplHelper<N> <typename Ifc1, ..., typename Ifc<N> >
    comphelper::WeakComponentImplHelper<N> <typename Ifc1, ...,
                                            typename Ifc<N> >
    comphelper::ImplInheritanceHelper<N> <typename BaseClass,
                                          typename Ifc1, ..., typename Ifc<N> >

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

} // namespace detail

/** Implementation helper implementing interface
    ::com::sun::star::lang::XTypeProvider and method
    XInterface::queryInterface(), but no reference counting.

    @derive
    Inherit from this class giving your interface(s) to be implemented as
    template argument(s).  Your sub class defines method implementations for
    these interface(s) including acquire()/release() and delegates incoming
    queryInterface() calls to this base class.
*/
template< BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER,
                               typename Ifc) >
class SAL_NO_VTABLE BOOST_PP_CAT(ImplHelper,
                                 COMPHELPER_IMPLBASE_INTERFACE_NUMBER)
    : public ::com::sun::star::lang::XTypeProvider,
      BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER, public Ifc)
{
    /// @internal
    struct cd : public ::rtl::StaticAggregate<
        ::cppu::class_data,
        BOOST_PP_CAT(detail::ImplClassData,
                     COMPHELPER_IMPLBASE_INTERFACE_NUMBER)
        <
            BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER, Ifc),
            BOOST_PP_CAT(ImplHelper, COMPHELPER_IMPLBASE_INTERFACE_NUMBER)<
                BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER, Ifc)>
        > > {};

protected:
    BOOST_PP_CAT(ImplHelper, COMPHELPER_IMPLBASE_INTERFACE_NUMBER)() {}
    virtual ~BOOST_PP_CAT(ImplHelper, COMPHELPER_IMPLBASE_INTERFACE_NUMBER)() {}

public:
    virtual ::com::sun::star::uno::Any
        SAL_CALL queryInterface( ::com::sun::star::uno::Type const& rType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return ::cppu::ImplHelper_query( rType, cd::get(), this ); }
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
        SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return ::cppu::ImplHelper_getTypes( cd::get() ); }
    virtual ::com::sun::star::uno::Sequence<sal_Int8>
        SAL_CALL getImplementationId()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return ::cppu::ImplHelper_getImplementationId( cd::get() ); }
};

/** Implementation helper implementing interfaces
    ::com::sun::star::lang::XTypeProvider and
    ::com::sun::star::uno::XInterface
    which supports weak mechanism to be held weakly
    (supporting ::com::sun::star::uno::XWeak through ::cppu::OWeakObject).

    @derive
    Inherit from this class giving your interface(s) to be implemented as
    template argument(s).  Your sub class defines method implementations for
    these interface(s).
*/
template< BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER,
                               typename Ifc) >
class SAL_NO_VTABLE BOOST_PP_CAT(WeakImplHelper,
                                 COMPHELPER_IMPLBASE_INTERFACE_NUMBER)
    : public ::cppu::OWeakObject,
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
            BOOST_PP_CAT(WeakImplHelper, COMPHELPER_IMPLBASE_INTERFACE_NUMBER)<
                BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER, Ifc)>
        > > {};

public:
    virtual ::com::sun::star::uno::Any
        SAL_CALL queryInterface( ::com::sun::star::uno::Type const& rType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        return ::cppu::WeakImplHelper_query(
            rType, cd::get(), this, static_cast<OWeakObject *>(this) );
    }
    virtual void SAL_CALL acquire() throw () SAL_OVERRIDE
        { OWeakObject::acquire(); }
    virtual void SAL_CALL release() throw () SAL_OVERRIDE
        { OWeakObject::release(); }
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
        SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return ::cppu::WeakImplHelper_getTypes( cd::get() ); }
    virtual ::com::sun::star::uno::Sequence<sal_Int8>
        SAL_CALL getImplementationId()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { return ::cppu::ImplHelper_getImplementationId( cd::get() ); }
};

/** Implementation helper implementing interfaces
    ::com::sun::star::lang::XTypeProvider and
    ::com::sun::star::uno::XInterface inherting from a BaseClass.

    All acquire() and release() calls are delegated to the BaseClass.
    Upon queryInterface(), if a demanded interface is not supported by this
    class directly, the request is delegated to the BaseClass.

    @attention
    The BaseClass has to be complete in a sense, that
    ::com::sun::star::uno::XInterface and
    ::com::sun::star::lang::XTypeProvider are implemented properly.
    The BaseClass must have at least one ctor that can be called with
    COMPHELPER_IMPLBASE_MAX_CTOR_ARGS or fewer arguments.

    @derive
    Inherit from this class giving your additional interface(s) to be
    implemented as template argument(s).  Your sub class defines method
    implementations for these interface(s).
*/
template <typename BaseClass,
          BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER,
                               typename Ifc) >
class SAL_NO_VTABLE BOOST_PP_CAT(ImplInheritanceHelper,
                                 COMPHELPER_IMPLBASE_INTERFACE_NUMBER)
    : public BaseClass,
      BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER, public Ifc)
{
    /// @internal
    struct cd : public ::rtl::StaticAggregate<
        ::cppu::class_data,
        BOOST_PP_CAT(detail::ImplClassData,
                     COMPHELPER_IMPLBASE_INTERFACE_NUMBER)
        <
            BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER, Ifc),
            BOOST_PP_CAT(ImplInheritanceHelper,
                         COMPHELPER_IMPLBASE_INTERFACE_NUMBER)<
                BaseClass,
                BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER, Ifc)>
        > > {};

protected:
#define COMPHELPER_IMPLBASE_templctor_args(z_, n_, unused_) \
    BOOST_PP_CAT(T, n_) const& BOOST_PP_CAT(arg, n_)
#define COMPHELPER_IMPLBASE_templctor(z_, n_, classname_) \
    template< BOOST_PP_ENUM_PARAMS( BOOST_PP_ADD(n_, 1), typename T) > \
    explicit BOOST_PP_CAT(classname_, COMPHELPER_IMPLBASE_INTERFACE_NUMBER)( \
        BOOST_PP_ENUM(BOOST_PP_ADD(n_, 1), \
                      COMPHELPER_IMPLBASE_templctor_args, ~) ) \
        : BaseClass( BOOST_PP_ENUM_PARAMS(BOOST_PP_ADD(n_, 1), arg) ) {}

    BOOST_PP_CAT(ImplInheritanceHelper, COMPHELPER_IMPLBASE_INTERFACE_NUMBER)()
        : BaseClass() {}
    BOOST_PP_REPEAT(COMPHELPER_IMPLBASE_MAX_CTOR_ARGS,
                    COMPHELPER_IMPLBASE_templctor, ImplInheritanceHelper)

public:
    virtual ::com::sun::star::uno::Any
        SAL_CALL queryInterface( ::com::sun::star::uno::Type const& rType )
        throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE
    {
        ::com::sun::star::uno::Any const aRet(
            ::cppu::ImplHelper_queryNoXInterface( rType, cd::get(), this ) );
        if (aRet.hasValue())
            return aRet;
        return BaseClass::queryInterface( rType );
    }
    virtual void SAL_CALL acquire() throw () SAL_OVERRIDE
        { BaseClass::acquire(); }
    virtual void SAL_CALL release() throw () SAL_OVERRIDE
        { BaseClass::release(); }
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >
        SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE
    {
        return ::cppu::ImplInhHelper_getTypes(
            cd::get(), BaseClass::getTypes() );
    }
    virtual ::com::sun::star::uno::Sequence<sal_Int8>
        SAL_CALL getImplementationId()
        throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE
        { return ::cppu::ImplHelper_getImplementationId( cd::get() ); }
};

// not needed anymore:
#undef COMPHELPER_IMPLBASE_templctor_args
#undef COMPHELPER_IMPLBASE_templctor

/** Implementation helper supporting
    ::com::sun::star::lang::XTypeProvider and
    ::com::sun::star::lang::XComponent.

    Upon disposing objects of this class, sub-classes receive a disposing()
    call.  Objects of this class can be held weakly, i.e. by a
    ::com::sun::star::uno::WeakReference.

    @attention
    The life-cycle of the passed mutex reference has to be longer than objects
    of this class.

    @derive
    Inherit from this class giving your interface(s) to be implemented as
    template argument(s).  Your sub class defines method implementations for
    these interface(s).
*/
template < BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER,
                                typename Ifc) >
class SAL_NO_VTABLE BOOST_PP_CAT(WeakComponentImplHelper,
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
            BOOST_PP_CAT(WeakComponentImplHelper,
                         COMPHELPER_IMPLBASE_INTERFACE_NUMBER)<
                BOOST_PP_ENUM_PARAMS(COMPHELPER_IMPLBASE_INTERFACE_NUMBER, Ifc)>
        > > {};

public:
    BOOST_PP_CAT(WeakComponentImplHelper, COMPHELPER_IMPLBASE_INTERFACE_NUMBER)(
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

    // implement XComponent directly avoiding ambiguities:
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { WeakComponentImplHelperBase::dispose(); }
    virtual void SAL_CALL addEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>
        const & xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { WeakComponentImplHelperBase::addEventListener( xListener ); }
    virtual void SAL_CALL removeEventListener(
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>
        const & xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        { WeakComponentImplHelperBase::removeEventListener( xListener ); }
};

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


} // namespace comphelper

// undef for multiple use/inclusion of this header:
#undef COMPHELPER_IMPLBASE_MAX_CTOR_ARGS
#undef COMPHELPER_IMPLBASE_INTERFACE_NUMBER

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
