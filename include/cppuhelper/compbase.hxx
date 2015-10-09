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

#ifndef INCLUDED_CPPUHELPER_COMPBASE_HXX
#define INCLUDED_CPPUHELPER_COMPBASE_HXX

#include <sal/config.h>

#include <exception>

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <cppuhelper/compbase_ex.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/instance.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star { namespace lang {
    class XEventListener;
} } } }
namespace osl { class Mutex; }

#if defined LIBO_INTERNAL_ONLY

namespace cppu {

/** Implementation helper implementing interfaces
    css::uno::XInterface, css::lang::XTypeProvider, and
    css::lang::XComponent.

    Like WeakComponentImplHelper, but does not define
    XComponent::add/removeEventListener.  Use for classes deriving from multiple
    UNO interfaces with competing add/removeEventListener methods, to avoid
    warnings about hiding of overloaded virtual functions.

    Upon disposing objects of this class, sub-classes receive a disposing()
    call.

    @attention
    The mutex reference passed to the constructor has to outlive the constructed
    instance.
*/
template<typename... Ifc>
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE PartialWeakComponentImplHelper:
    public WeakComponentImplHelperBase, public css::lang::XTypeProvider,
    public Ifc...
{
    struct cd:
        rtl::StaticAggregate<
            class_data,
            detail::ImplClassData<PartialWeakComponentImplHelper, Ifc...>>
    {};

public:
    PartialWeakComponentImplHelper(osl::Mutex & mutex) throw ():
        WeakComponentImplHelperBase(mutex) {}

    css::uno::Any SAL_CALL queryInterface(css::uno::Type const & aType)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return WeakComponentImplHelper_query(aType, cd::get(), this, this); }

    void SAL_CALL acquire() throw () SAL_OVERRIDE
    { WeakComponentImplHelperBase::acquire(); }

    void SAL_CALL release() throw () SAL_OVERRIDE
    { WeakComponentImplHelperBase::release(); }

    void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception)
        SAL_OVERRIDE
    { WeakComponentImplHelperBase::dispose(); }

    css::uno::Sequence<css::uno::Type> SAL_CALL getTypes()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return WeakComponentImplHelper_getTypes(cd::get()); }

    css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return css::uno::Sequence<sal_Int8>(); }
};

/** Implementation helper implementing interfaces
    css::uno::XInterface, css::lang::XTypeProvider, and
    css::lang::XComponent.

    Upon disposing objects of this class, sub-classes receive a disposing()
    call.

    @attention
    The mutex reference passed to the constructor has to outlive the constructed
    instance.
*/
template<typename... Ifc>
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE WeakComponentImplHelper:
    public PartialWeakComponentImplHelper<Ifc...>
{
public:
    WeakComponentImplHelper(osl::Mutex & mutex) throw ():
        PartialWeakComponentImplHelper<Ifc...>(mutex) {}

    void SAL_CALL addEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { WeakComponentImplHelperBase::addEventListener(xListener); }

    void SAL_CALL removeEventListener(
        css::uno::Reference<css::lang::XEventListener> const & aListener)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { WeakComponentImplHelperBase::removeEventListener(aListener); }
};

/** Implementation helper supporting com::sun::star::lang::XTypeProvider
    and com::sun::star::lang::XComponent.

    Upon disposing objects of this class, sub-classes receive a disposing()
    call.  Objects of this class can be held weakly, i.e. by a
    com::sun::star::uno::WeakReference.  Object of this class can be
    aggregated, i.e. incoming queryInterface() calls are delegated.

    @attention
    The life-cycle of the passed mutex reference has to be longer than objects of this class.

    @derive
    Inherit from this class giving your interface(s) to be implemented as template argument(s).
    Your sub class defines method implementations for these interface(s).

    @deprecated
*/
template< typename... Ifc >
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE WeakAggComponentImplHelper
    : public WeakAggComponentImplHelperBase
    , public css::lang::XTypeProvider
    , public Ifc...
{
    struct cd :
        public rtl::StaticAggregate<
            class_data, detail::ImplClassData < WeakAggComponentImplHelper, Ifc... > >
    {};

public:
    inline WeakAggComponentImplHelper( ::osl::Mutex & rMutex )
        throw ()
    : WeakAggComponentImplHelperBase( rMutex )
    {}

    css::uno::Any SAL_CALL queryInterface( css::uno::Type const & rType )
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE
    { return WeakAggComponentImplHelperBase::queryInterface( rType ); }

    css::uno::Any SAL_CALL queryAggregation( css::uno::Type const & rType )
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE
    { return WeakAggComponentImplHelper_queryAgg( rType, cd::get(), this, static_cast<WeakAggComponentImplHelperBase *>(this) ); }

    void SAL_CALL acquire()
        throw ( ) SAL_OVERRIDE
    { WeakAggComponentImplHelperBase::acquire(); }

    void SAL_CALL release()
        throw ( ) SAL_OVERRIDE
    { WeakAggComponentImplHelperBase::release(); }

    css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE
    { return WeakAggComponentImplHelper_getTypes( cd::get() ); }

    css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE
    { return ImplHelper_getImplementationId( cd::get() ); }

};

}

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
