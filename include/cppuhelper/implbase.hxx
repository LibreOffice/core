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

#ifndef INCLUDED_CPPUHELPER_IMPLBASE_HXX
#define INCLUDED_CPPUHELPER_IMPLBASE_HXX

#include <sal/config.h>

#include <cstddef>
#include <exception>
#include <utility>

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <cppuhelper/implbase_ex.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/instance.hxx>
#include <sal/types.h>

namespace cppu {

/// @cond INTERNAL

namespace detail {

template<std::size_t N> struct class_dataN {
    sal_Int16 m_nTypes;
    sal_Bool m_storedTypeRefs;
    sal_Bool m_storedId;
    sal_Int8 m_id[16];
    type_entry m_typeEntries[N + 1];
};

template<typename Impl, typename... Ifc> struct ImplClassData {
    class_data * operator ()() {
        static class_dataN<sizeof... (Ifc)> s_cd = {
            sizeof... (Ifc) + 1, false, false, {},
            {
                { { Ifc::static_type },
                  (reinterpret_cast<sal_IntPtr>(
                      static_cast<Ifc *>(reinterpret_cast<Impl *>(16)))
                   - 16)
                }...,
                CPPUHELPER_DETAIL_TYPEENTRY(css::lang::XTypeProvider)
            }
        };
        return reinterpret_cast<class_data *>(&s_cd);
    }
};

}

/// @endcond

/** Implementation helper implementing interface com::sun::star::lang::XTypeProvider
    and method XInterface::queryInterface(), but no reference counting.

    @derive
    Inherit from this class giving your interface(s) to be implemented as template argument(s).
    Your sub class defines method implementations for these interface(s) including acquire()/
    release() and delegates incoming queryInterface() calls to this base class.
*/
template< typename... Ifc >
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE ImplHelper
    : public css::lang::XTypeProvider, public Ifc...
{
    struct cd :
        public rtl::StaticAggregate<
            class_data, detail::ImplClassData < ImplHelper, Ifc... > >
    {};

public:
    css::uno::Any SAL_CALL queryInterface( css::uno::Type const & rType )
        SAL_THROW_IfNotObjectiveC ( css::uno::RuntimeException, std::exception ) override
    { return ImplHelper_query( rType, cd::get(), this ); }

    css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        SAL_THROW_IfNotObjectiveC ( css::uno::RuntimeException, std::exception ) override
    { return ImplHelper_getTypes( cd::get() ); }

    css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        SAL_THROW_IfNotObjectiveC ( css::uno::RuntimeException, std::exception ) override
    { return ImplHelper_getImplementationId( cd::get() ); }

protected:
    ImplHelper() {}

    virtual ~ImplHelper() {}
};

/** Implementation helper implementing interfaces
    css::uno::XInterface, css::lang::XTypeProvider, and
    css::uno::XWeak (through cppu::OWeakObject).

    @derive
    Inherit from this class giving your interface(s) to be implemented as
    template argument(s).
*/
template<typename... Ifc>
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE WeakImplHelper:
    public OWeakObject, public css::lang::XTypeProvider, public Ifc...
{
    struct cd:
        rtl::StaticAggregate<
            class_data, detail::ImplClassData<WeakImplHelper, Ifc...>>
    {};

protected:
    WeakImplHelper() {}

    virtual ~WeakImplHelper() override {}

public:
    css::uno::Any SAL_CALL queryInterface(css::uno::Type const & aType)
        SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override
    { return WeakImplHelper_query(aType, cd::get(), this, this); }

    void SAL_CALL acquire() SAL_THROW_IfNotObjectiveC () override
    { OWeakObject::acquire(); }

    void SAL_CALL release() SAL_THROW_IfNotObjectiveC () override
    { OWeakObject::release(); }

    css::uno::Sequence<css::uno::Type> SAL_CALL getTypes()
        SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override
    { return WeakImplHelper_getTypes(cd::get()); }

    css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId()
        SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override
    { return css::uno::Sequence<sal_Int8>(); }
};

/** Implementation helper implementing interfaces com::sun::star::lang::XTypeProvider
    and com::sun::star::uno::XInterface which supports weak mechanism to be held weakly
    (supporting com::sun::star::uno::XWeak thru ::cppu::OWeakAggObject).
    In addition, it supports also aggregation meaning object of this class can be aggregated
    (com::sun::star::uno::XAggregation thru ::cppu::OWeakAggObject).
    If a delegator is set (this object is aggregated), then incoming queryInterface()
    calls are delegated to the delegator object. If the delegator does not support the
    demanded interface, it calls queryAggregation() on its aggregated objects.

    @derive
    Inherit from this class giving your interface(s) to be implemented as template argument(s).
    Your sub class defines method implementations for these interface(s).
*/
template< typename... Ifc >
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE WeakAggImplHelper
        : public OWeakAggObject
        , public css::lang::XTypeProvider
        , public Ifc...
{
    struct cd :
        public rtl::StaticAggregate<
            class_data, detail::ImplClassData < WeakAggImplHelper, Ifc... > >
    {};

protected:
    WeakAggImplHelper() {}

    virtual ~WeakAggImplHelper() {}

public:
    css::uno::Any SAL_CALL queryInterface( css::uno::Type const & rType )
        SAL_THROW_IfNotObjectiveC ( css::uno::RuntimeException, std::exception ) override
    { return OWeakAggObject::queryInterface( rType ); }

    css::uno::Any SAL_CALL queryAggregation( css::uno::Type const & rType )
        SAL_THROW_IfNotObjectiveC ( css::uno::RuntimeException, std::exception ) override
    { return WeakAggImplHelper_queryAgg( rType, cd::get(), this, static_cast<OWeakAggObject *>(this) ); }

    void SAL_CALL acquire() SAL_THROW_IfNotObjectiveC ( ) override
    { OWeakAggObject::acquire(); }

    void SAL_CALL release() SAL_THROW_IfNotObjectiveC ( ) override
    { OWeakAggObject::release(); }

    css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        SAL_THROW_IfNotObjectiveC ( css::uno::RuntimeException, std::exception ) override
    { return WeakAggImplHelper_getTypes( cd::get() ); }

    css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        SAL_THROW_IfNotObjectiveC ( css::uno::RuntimeException, std::exception ) override
    { return ImplHelper_getImplementationId( cd::get() ); }
};

/** Implementation helper implementing interfaces
    css::uno::XInterface and css::lang::XTypeProvider
    inherting from a BaseClass.

    All acquire() and release() calls are delegated to the BaseClass.  Upon
    queryInterface(), if a demanded interface is not supported by this class
    directly, the request is delegated to the BaseClass.

    @attention
    The BaseClass has to be complete in the sense that
    css::uno::XInterface and css::lang::XTypeProvider are
    implemented properly.

    @derive
    Inherit from this class giving your additional interface(s) to be
    implemented as template argument(s).
*/
template<typename BaseClass, typename... Ifc>
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE ImplInheritanceHelper:
    public BaseClass, public Ifc...
{
    struct cd:
        rtl::StaticAggregate<
            class_data, detail::ImplClassData<ImplInheritanceHelper, Ifc...>>
    {};

protected:
    template<typename... Arg> ImplInheritanceHelper(Arg &&... arg):
        BaseClass(std::forward<Arg>(arg)...)
    {}

    virtual ~ImplInheritanceHelper() {}

public:
    css::uno::Any SAL_CALL queryInterface(css::uno::Type const & aType)
        SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override
    {
        css::uno::Any ret(ImplHelper_queryNoXInterface(aType, cd::get(), this));
        return ret.hasValue() ? ret : BaseClass::queryInterface(aType);
    }

    void SAL_CALL acquire() SAL_THROW_IfNotObjectiveC () override
    { BaseClass::acquire(); }

    void SAL_CALL release() SAL_THROW_IfNotObjectiveC () override
    { BaseClass::release(); }

    css::uno::Sequence<css::uno::Type> SAL_CALL getTypes()
        SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override
    { return ImplInhHelper_getTypes(cd::get(), BaseClass::getTypes()); }

    css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId()
        SAL_THROW_IfNotObjectiveC (css::uno::RuntimeException, std::exception) override
    { return css::uno::Sequence<sal_Int8>(); }
};

/** Implementation helper implementing interfaces com::sun::star::lang::XTypeProvider
    and com::sun::star::uno::XInterface inherting from a BaseClass.
    All acquire(),  release() and queryInterface() calls are delegated to the BaseClass.
    Upon queryAggregation(), if a demanded interface is not supported by this class directly,
    the request is delegated to the BaseClass.

    @attention
    The BaseClass has to be complete in a sense, that com::sun::star::uno::XInterface,
    com::sun::star::uno::XAggregation and com::sun::star::lang::XTypeProvider
    are implemented properly.  The BaseClass must have at least one ctor
    that can be called with six or fewer arguments, of which none is of
    non-const reference type.

    @derive
    Inherit from this class giving your additional interface(s) to be implemented as
    template argument(s). Your sub class defines method implementations for these interface(s).
*/
template< typename BaseClass, typename... Ifc >
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE AggImplInheritanceHelper
    : public BaseClass, public Ifc...
{
    struct cd :
        public rtl::StaticAggregate<
            class_data, detail::ImplClassData < AggImplInheritanceHelper, Ifc... > >
    {};

protected:
    template< typename... Arg > AggImplInheritanceHelper(Arg &&... arg):
        BaseClass(std::forward<Arg>(arg)...)
    {}

    virtual ~AggImplInheritanceHelper() {}

public:
    css::uno::Any SAL_CALL queryInterface( css::uno::Type const & rType )
        SAL_THROW_IfNotObjectiveC ( css::uno::RuntimeException, std::exception ) override
    { return BaseClass::queryInterface( rType ); }

    css::uno::Any SAL_CALL queryAggregation( css::uno::Type const & rType )
        SAL_THROW_IfNotObjectiveC ( css::uno::RuntimeException, std::exception ) override
    {
        css::uno::Any aRet( ImplHelper_queryNoXInterface( rType, cd::get(), this ) );
        return (aRet.hasValue()) ? aRet : BaseClass::queryAggregation( rType );
    }

    void SAL_CALL acquire() SAL_THROW_IfNotObjectiveC ( ) override
    { BaseClass::acquire(); }

    void SAL_CALL release() SAL_THROW_IfNotObjectiveC ( ) override
    { BaseClass::release(); }

    css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        SAL_THROW_IfNotObjectiveC ( css::uno::RuntimeException, std::exception ) override
    { return ImplInhHelper_getTypes( cd::get(), BaseClass::getTypes() ); }

    css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        SAL_THROW_IfNotObjectiveC ( css::uno::RuntimeException, std::exception ) override
    { return ImplHelper_getImplementationId( cd::get() ); }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
