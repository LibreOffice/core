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
#ifndef INCLUDED_CPPUHELPER_COMPBASE2_HXX
#define INCLUDED_CPPUHELPER_COMPBASE2_HXX

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/compbase_ex.hxx>

namespace cppu
{

    /** Implementation helper supporting com::sun::star::lang::XTypeProvider and
        com::sun::star::lang::XComponent.

        Upon disposing objects of this class, sub-classes receive a disposing()
        call.  Objects of this class can be held weakly, i.e. by a
        com::sun::star::uno::WeakReference.

        @attention
        The life-cycle of the passed mutex reference has to be longer than objects of this class.

        @derive
        Inherit from this class giving your interface(s) to be implemented as template argument(s).
        Your sub class defines method implementations for these interface(s).
    */
    template< class Ifc1, class Ifc2 >
    class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE WeakComponentImplHelper2
        : public WeakComponentImplHelperBase
        , public ::com::sun::star::lang::XTypeProvider
        , public Ifc1, public Ifc2
    {
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData2 < Ifc1, Ifc2, WeakComponentImplHelper2<Ifc1, Ifc2> > > {};
    public:
        inline WeakComponentImplHelper2( ::osl::Mutex & rMutex ) throw ()
            : WeakComponentImplHelperBase( rMutex )
            {}
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
            { return WeakComponentImplHelper_query( rType, cd::get(), this, (WeakComponentImplHelperBase *)this ); }
        virtual void SAL_CALL acquire() throw () SAL_OVERRIDE
            { WeakComponentImplHelperBase::acquire(); }
        virtual void SAL_CALL release() throw () SAL_OVERRIDE
            { WeakComponentImplHelperBase::release(); }
        virtual void SAL_CALL dispose()throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
            { WeakComponentImplHelperBase::dispose(); }
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & xListener)throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
            { WeakComponentImplHelperBase::addEventListener(xListener); }
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & xListener)throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
            { WeakComponentImplHelperBase::removeEventListener(xListener); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
            { return WeakComponentImplHelper_getTypes( cd::get() ); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
            { return ImplHelper_getImplementationId( cd::get() ); }
    };

    /** Same as WeakComponentImplHelper2, except doesn't implement
        addEventListener, removeEventListener and dispose.

        This requires derived classes to implement those three methods.
        This makes it possible to implement classes which are required to
        implement methods from multiple bases which have different
        addEventListener/removeEventListener signatures without triggering
        the g++ overloaded-virtual warning
    */
    template< class Ifc1, class Ifc2 >
    class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE PartialWeakComponentImplHelper2
        : public WeakComponentImplHelperBase
        , public ::com::sun::star::lang::XTypeProvider
        , public Ifc1, public Ifc2
    {
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData2 < Ifc1, Ifc2, PartialWeakComponentImplHelper2<Ifc1, Ifc2> > > {};
    public:
        inline PartialWeakComponentImplHelper2( ::osl::Mutex & rMutex ) throw ()
            : WeakComponentImplHelperBase( rMutex )
            {}
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
            { return WeakComponentImplHelper_query( rType, cd::get(), this, (WeakComponentImplHelperBase *)this ); }
        virtual void SAL_CALL acquire() throw () SAL_OVERRIDE
            { WeakComponentImplHelperBase::acquire(); }
        virtual void SAL_CALL release() throw () SAL_OVERRIDE
            { WeakComponentImplHelperBase::release(); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
            { return WeakComponentImplHelper_getTypes( cd::get() ); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
            { return ImplHelper_getImplementationId( cd::get() ); }
    };


    /** Implementation helper supporting com::sun::star::lang::XTypeProvider and
        com::sun::star::lang::XComponent.

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
    template< class Ifc1, class Ifc2 >
    class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE WeakAggComponentImplHelper2
        : public WeakAggComponentImplHelperBase
        , public ::com::sun::star::lang::XTypeProvider
        , public Ifc1, public Ifc2
    {
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData2 < Ifc1, Ifc2, WeakAggComponentImplHelper2<Ifc1, Ifc2> > > {};
    public:
        inline WeakAggComponentImplHelper2( ::osl::Mutex & rMutex ) throw ()
            : WeakAggComponentImplHelperBase( rMutex )
            {}
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
            { return WeakAggComponentImplHelperBase::queryInterface( rType ); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
            { return WeakAggComponentImplHelper_queryAgg( rType, cd::get(), this, (WeakAggComponentImplHelperBase *)this ); }
        virtual void SAL_CALL acquire() throw () SAL_OVERRIDE
            { WeakAggComponentImplHelperBase::acquire(); }
        virtual void SAL_CALL release() throw () SAL_OVERRIDE
            { WeakAggComponentImplHelperBase::release(); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
            { return WeakAggComponentImplHelper_getTypes( cd::get() ); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
            { return ImplHelper_getImplementationId( cd::get() ); }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
