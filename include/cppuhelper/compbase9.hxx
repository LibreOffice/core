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
#ifndef INCLUDED_CPPUHELPER_COMPBASE9_HXX
#define INCLUDED_CPPUHELPER_COMPBASE9_HXX

#include "cppuhelper/implbase9.hxx"
#include "cppuhelper/compbase_ex.hxx"

namespace cppu
{

    /** Implementation helper supporting css::lang::XTypeProvider and
        css::lang::XComponent.

        Upon disposing objects of this class, sub-classes receive a disposing()
        call.  Objects of this class can be held weakly, i.e. by a
        css::uno::WeakReference.

        @attention
        The life-cycle of the passed mutex reference has to be longer than objects of this class.

        @derive
        Inherit from this class giving your interface(s) to be implemented as template argument(s).
        Your sub class defines method implementations for these interface(s).
    */
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6, class Ifc7, class Ifc8, class Ifc9 >
    class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE WeakComponentImplHelper9
        : public WeakComponentImplHelperBase
        , public css::lang::XTypeProvider
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5, public Ifc6, public Ifc7, public Ifc8, public Ifc9
    {
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData9< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6, Ifc7, Ifc8, Ifc9, WeakComponentImplHelper9<Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6, Ifc7, Ifc8, Ifc9> > > {};
    public:
        WeakComponentImplHelper9( ::osl::Mutex & rMutex ) throw ()
            : WeakComponentImplHelperBase( rMutex )
            {}
        virtual css::uno::Any SAL_CALL queryInterface( css::uno::Type const & rType ) SAL_OVERRIDE
            { return WeakComponentImplHelper_query( rType, cd::get(), this, static_cast<WeakComponentImplHelperBase *>(this) ); }
        virtual void SAL_CALL acquire() throw () SAL_OVERRIDE
            { WeakComponentImplHelperBase::acquire(); }
        virtual void SAL_CALL release() throw () SAL_OVERRIDE
            { WeakComponentImplHelperBase::release(); }
        virtual void SAL_CALL dispose() SAL_OVERRIDE
            { WeakComponentImplHelperBase::dispose(); }
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & xListener) SAL_OVERRIDE
            { WeakComponentImplHelperBase::addEventListener(xListener); }
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & xListener) SAL_OVERRIDE
            { WeakComponentImplHelperBase::removeEventListener(xListener); }
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() SAL_OVERRIDE
            { return WeakComponentImplHelper_getTypes( cd::get() ); }
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() SAL_OVERRIDE
            { return ImplHelper_getImplementationId( cd::get() ); }
    };

    /** Same as WeakComponentImplHelper9, except doesn't implement
        addEventListener, removeEventListener and dispose.

        This requires derived classes to implement those three methods.
        This makes it possible to implement classes which are required to
        implement methods from multiple bases which have different
        addEventListener/removeEventListener signatures without triggering
        the g++ overloaded-virtual warning
    */
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6, class Ifc7, class Ifc8, class Ifc9 >
    class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE PartialWeakComponentImplHelper9
        : public WeakComponentImplHelperBase
        , public css::lang::XTypeProvider
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5, public Ifc6, public Ifc7, public Ifc8, public Ifc9
    {
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData9< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6, Ifc7, Ifc8, Ifc9, PartialWeakComponentImplHelper9<Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6, Ifc7, Ifc8, Ifc9> > > {};
    public:
        PartialWeakComponentImplHelper9( ::osl::Mutex & rMutex ) throw ()
            : WeakComponentImplHelperBase( rMutex )
            {}
        virtual css::uno::Any SAL_CALL queryInterface( css::uno::Type const & rType ) SAL_OVERRIDE
            { return WeakComponentImplHelper_query( rType, cd::get(), this, static_cast<WeakComponentImplHelperBase *>(this) ); }
        virtual void SAL_CALL acquire() throw () SAL_OVERRIDE
            { WeakComponentImplHelperBase::acquire(); }
        virtual void SAL_CALL release() throw () SAL_OVERRIDE
            { WeakComponentImplHelperBase::release(); }
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() SAL_OVERRIDE
            { return WeakComponentImplHelper_getTypes( cd::get() ); }
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() SAL_OVERRIDE
            { return ImplHelper_getImplementationId( cd::get() ); }
    };

    /** Implementation helper supporting css::lang::XTypeProvider and
        css::lang::XComponent.
        Upon disposing objects of this class, sub-classes receive a disposing()
        call.  Objects of this class can be held weakly, i.e. by a
        css::uno::WeakReference.  Object of this class can be
        aggregated, i.e. incoming queryInterface() calls are delegated.

        @attention
        The life-cycle of the passed mutex reference has to be longer than objects of this class.

        @derive
        Inherit from this class giving your interface(s) to be implemented as template argument(s).
        Your sub class defines method implementations for these interface(s).

        @deprecated
    */
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6, class Ifc7, class Ifc8, class Ifc9 >
    class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE WeakAggComponentImplHelper9
        : public WeakAggComponentImplHelperBase
        , public css::lang::XTypeProvider
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5, public Ifc6, public Ifc7, public Ifc8, public Ifc9
    {
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData9< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6, Ifc7, Ifc8, Ifc9, WeakAggComponentImplHelper9<Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6, Ifc7, Ifc8, Ifc9> > > {};
    public:
        WeakAggComponentImplHelper9( ::osl::Mutex & rMutex ) throw ()
            : WeakAggComponentImplHelperBase( rMutex )
            {}
        virtual css::uno::Any SAL_CALL queryInterface( css::uno::Type const & rType ) SAL_OVERRIDE
            { return WeakAggComponentImplHelperBase::queryInterface( rType ); }
        virtual css::uno::Any SAL_CALL queryAggregation( css::uno::Type const & rType ) SAL_OVERRIDE
            { return WeakAggComponentImplHelper_queryAgg( rType, cd::get(), this, static_cast<WeakAggComponentImplHelperBase *>(this) ); }
        virtual void SAL_CALL acquire() throw () SAL_OVERRIDE
            { WeakAggComponentImplHelperBase::acquire(); }
        virtual void SAL_CALL release() throw () SAL_OVERRIDE
            { WeakAggComponentImplHelperBase::release(); }
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() SAL_OVERRIDE
            { return WeakAggComponentImplHelper_getTypes( cd::get() ); }
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() SAL_OVERRIDE
            { return ImplHelper_getImplementationId( cd::get() ); }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
