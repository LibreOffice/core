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
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <cppuhelper/compbase_ex.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/instance.hxx>
#include <sal/types.h>

namespace osl { class Mutex; }

#if defined LIBO_INTERNAL_ONLY

namespace cppu {

/** Implementation helper implementing interfaces
    com::sun::star::uno::XInterface, com::sun::star::lang::XTypeProvider, and
    com::sun::star::lang::XComponent.

    Upon disposing objects of this class, sub-classes receive a disposing()
    call.

    @attention
    The mutex reference passed to the constructor has to outlive the constructed
    instance.
*/
template<typename... Ifc>
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE WeakComponentImplHelper:
    public WeakComponentImplHelperBase, public css::lang::XTypeProvider,
    public Ifc...
{
    struct cd:
        rtl::StaticAggregate<
            class_data, detail::ImplClassData<WeakComponentImplHelper, Ifc...>>
    {};

public:
    WeakComponentImplHelper(osl::Mutex & mutex) throw ():
        WeakComponentImplHelperBase(mutex) {}

    css::uno::Any SAL_CALL queryInterface(css::uno::Type const & aType)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return WeakComponentImplHelper_query(aType, cd::get(), this, this); }

    void SAL_CALL acquire() throw () SAL_OVERRIDE
    { WeakComponentImplHelperBase::acquire(); }

    void SAL_CALL release() throw () SAL_OVERRIDE
    { WeakComponentImplHelperBase::release(); }

    css::uno::Sequence<css::uno::Type> SAL_CALL getTypes()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return WeakComponentImplHelper_getTypes(cd::get()); }

    css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return css::uno::Sequence<sal_Int8>(); }
};

}

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
