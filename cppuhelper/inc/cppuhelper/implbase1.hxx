/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#define _CPPUHELPER_IMPLBASE1_HXX_

#include <cppuhelper/implbase_ex.hxx>
#include <rtl/instance.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace cppu
{
    /// @cond INTERNAL

    struct class_data1
    {
        sal_Int16 m_nTypes;
        sal_Bool m_storedTypeRefs;
        sal_Bool m_storedId;
        sal_Int8 m_id[ 16 ];
        type_entry m_typeEntries[ 1 + 1 ];
    };

    template< typename Ifc1, typename Impl > struct ImplClassData1
    {
        class_data* operator ()()
        {
            static class_data1 s_cd =
            {
                1 +1, sal_False, sal_False,
                { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {
                    { { Ifc1::static_type }, ((sal_IntPtr)(Ifc1 *) (Impl *) 16) - 16 },
                    { { com::sun::star::lang::XTypeProvider::static_type }, ((sal_IntPtr)(com::sun::star::lang::XTypeProvider *) (Impl *) 16) - 16 }
                }
            };
            return reinterpret_cast< class_data * >(&s_cd);
        }
    };

    /// @endcond

    /** Implementation helper implementing interface com::sun::star::lang::XTypeProvider
        and method XInterface::queryInterface(), but no reference counting.

        @derive
        Inherit from this class giving your interface(s) to be implemented as template argument(s).
        Your sub class defines method implementations for these interface(s) including acquire()/
        release() and delegates incoming queryInterface() calls to this base class.
    */
    template< class Ifc1 >
    class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE ImplHelper1
        : public com::sun::star::lang::XTypeProvider
        , public Ifc1
    {
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData1 < Ifc1, ImplHelper1<Ifc1> > > {};
    public:
        virtual com::sun::star::uno::Any SAL_CALL queryInterface( com::sun::star::uno::Type const & rType ) throw (com::sun::star::uno::RuntimeException)
            { return ImplHelper_query( rType, cd::get(), this ); }
        virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL getTypes() throw (com::sun::star::uno::RuntimeException)
            { return ImplHelper_getTypes( cd::get() ); }
        virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( cd::get() ); }

#if !defined _MSC_VER // public -> protected changes mangled names there
    protected:
#endif
        ~ImplHelper1() throw () {}
    };
    /** Implementation helper implementing interfaces com::sun::star::lang::XTypeProvider and
        com::sun::star::uno::XInterface which supports weak mechanism to be held weakly
        (supporting com::sun::star::uno::XWeak thru ::cppu::OWeakObject).

        @derive
        Inherit from this class giving your interface(s) to be implemented as template argument(s).
        Your sub class defines method implementations for these interface(s).

        These classes are used when you implement your UNO component.
        WeakImplHelper1 till WeakImplHelper12 can be used when you want
        to implement 1 till 12 interfaces in your component.
    */
    template< class Ifc1 >
    class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE SAL_DLLPUBLIC_EXPORT WeakImplHelper1
        : public OWeakObject
        , public com::sun::star::lang::XTypeProvider
        , public Ifc1
    {
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData1< Ifc1, WeakImplHelper1< Ifc1 > > > {};
    public:
        virtual com::sun::star::uno::Any SAL_CALL queryInterface( com::sun::star::uno::Type const & rType ) throw (com::sun::star::uno::RuntimeException)
            { return WeakImplHelper_query( rType, cd::get(), this, (OWeakObject *)this ); }
        virtual void SAL_CALL acquire() throw ()
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw ()
            { OWeakObject::release(); }
        virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL getTypes() throw (com::sun::star::uno::RuntimeException)
            { return WeakImplHelper_getTypes( cd::get() ); }
        virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( cd::get() ); }
    };
    /** Implementation helper implementing interfaces com::sun::star::lang::XTypeProvider and
        com::sun::star::uno::XInterface which supports weak mechanism to be held weakly
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
    template< class Ifc1 >
    class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE WeakAggImplHelper1
        : public OWeakAggObject
        , public com::sun::star::lang::XTypeProvider
        , public Ifc1
    {
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData1< Ifc1, WeakAggImplHelper1< Ifc1 > > > {};
    public:
        virtual com::sun::star::uno::Any SAL_CALL queryInterface( com::sun::star::uno::Type const & rType ) throw (com::sun::star::uno::RuntimeException)
            { return OWeakAggObject::queryInterface( rType ); }
        virtual com::sun::star::uno::Any SAL_CALL queryAggregation( com::sun::star::uno::Type const & rType ) throw (com::sun::star::uno::RuntimeException)
            { return WeakAggImplHelper_queryAgg( rType, cd::get(), this, (OWeakAggObject *)this ); }
        virtual void SAL_CALL acquire() throw ()
            { OWeakAggObject::acquire(); }
        virtual void SAL_CALL release() throw ()
            { OWeakAggObject::release(); }
        virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL getTypes() throw (com::sun::star::uno::RuntimeException)
            { return WeakAggImplHelper_getTypes( cd::get() ); }
        virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( cd::get() ); }
    };
    /** Implementation helper implementing interfaces com::sun::star::lang::XTypeProvider and
        com::sun::star::uno::XInterface inherting from a BaseClass.
        All acquire() and release() calls are delegated to the BaseClass. Upon queryInterface(),
        if a demanded interface is not supported by this class directly, the request is
        delegated to the BaseClass.

        @attention
        The BaseClass has to be complete in a sense, that com::sun::star::uno::XInterface
        and com::sun::star::lang::XTypeProvider are implemented properly.  The
        BaseClass must have at least one ctor that can be called with six or
        fewer arguments, of which none is of non-const reference type.

        @derive
        Inherit from this class giving your additional interface(s) to be implemented as
        template argument(s). Your sub class defines method implementations for these interface(s).
    */
    template< class BaseClass, class Ifc1 >
    class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE ImplInheritanceHelper1
        : public BaseClass
        , public Ifc1
    {
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData1< Ifc1, ImplInheritanceHelper1< BaseClass, Ifc1 > > > {};
    protected:
#if (defined __SUNPRO_CC && __SUNPRO_CC <= 0x550)
        // Hack, to get comphelper::service_decl to work for non-trivial impl classes
        ImplInheritanceHelper1( com::sun::star::uno::Sequence<com::sun::star::uno::Any> const& args,
                                com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext> const& xContext ) : BaseClass(args,xContext) {}
#endif
        template< typename T1 >
        explicit ImplInheritanceHelper1(T1 const & arg1): BaseClass(arg1) {}
        template< typename T1, typename T2 >
        ImplInheritanceHelper1(T1 const & arg1, T2 const & arg2):
            BaseClass(arg1, arg2) {}
        template< typename T1, typename T2, typename T3 >
        ImplInheritanceHelper1(
            T1 const & arg1, T2 const & arg2, T3 const & arg3):
            BaseClass(arg1, arg2, arg3) {}
        template< typename T1, typename T2, typename T3, typename T4 >
        ImplInheritanceHelper1(
            T1 const & arg1, T2 const & arg2, T3 const & arg3, T4 const & arg4):
            BaseClass(arg1, arg2, arg3, arg4) {}
        template<
            typename T1, typename T2, typename T3, typename T4, typename T5 >
        ImplInheritanceHelper1(
            T1 const & arg1, T2 const & arg2, T3 const & arg3, T4 const & arg4,
            T5 const & arg5):
            BaseClass(arg1, arg2, arg3, arg4, arg5) {}
        template<
            typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6 >
        ImplInheritanceHelper1(
            T1 const & arg1, T2 const & arg2, T3 const & arg3, T4 const & arg4,
            T5 const & arg5, T6 const & arg6):
            BaseClass(arg1, arg2, arg3, arg4, arg5, arg6) {}
    public:
        ImplInheritanceHelper1() {}
        virtual com::sun::star::uno::Any SAL_CALL queryInterface( com::sun::star::uno::Type const & rType ) throw (com::sun::star::uno::RuntimeException)
            {
                com::sun::star::uno::Any aRet( ImplHelper_queryNoXInterface( rType, cd::get(), this ) );
                if (aRet.hasValue())
                    return aRet;
                return BaseClass::queryInterface( rType );
            }
        virtual void SAL_CALL acquire() throw ()
            { BaseClass::acquire(); }
        virtual void SAL_CALL release() throw ()
            { BaseClass::release(); }
        virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL getTypes() throw (com::sun::star::uno::RuntimeException)
            { return ImplInhHelper_getTypes( cd::get(), BaseClass::getTypes() ); }
        virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( cd::get() ); }
    };
    /** Implementation helper implementing interfaces com::sun::star::lang::XTypeProvider and
        com::sun::star::uno::XInterface inherting from a BaseClass.
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
    template< class BaseClass, class Ifc1 >
    class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE AggImplInheritanceHelper1
        : public BaseClass
        , public Ifc1
    {
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData1< Ifc1, AggImplInheritanceHelper1< BaseClass, Ifc1 > > > {};
    protected:
        template< typename T1 >
        explicit AggImplInheritanceHelper1(T1 const & arg1): BaseClass(arg1) {}
        template< typename T1, typename T2 >
        AggImplInheritanceHelper1(T1 const & arg1, T2 const & arg2):
            BaseClass(arg1, arg2) {}
        template< typename T1, typename T2, typename T3 >
        AggImplInheritanceHelper1(
            T1 const & arg1, T2 const & arg2, T3 const & arg3):
            BaseClass(arg1, arg2, arg3) {}
        template< typename T1, typename T2, typename T3, typename T4 >
        AggImplInheritanceHelper1(
            T1 const & arg1, T2 const & arg2, T3 const & arg3, T4 const & arg4):
            BaseClass(arg1, arg2, arg3, arg4) {}
        template<
            typename T1, typename T2, typename T3, typename T4, typename T5 >
        AggImplInheritanceHelper1(
            T1 const & arg1, T2 const & arg2, T3 const & arg3, T4 const & arg4,
            T5 const & arg5):
            BaseClass(arg1, arg2, arg3, arg4, arg5) {}
        template<
            typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6 >
        AggImplInheritanceHelper1(
            T1 const & arg1, T2 const & arg2, T3 const & arg3, T4 const & arg4,
            T5 const & arg5, T6 const & arg6):
            BaseClass(arg1, arg2, arg3, arg4, arg5, arg6) {}
    public:
        AggImplInheritanceHelper1() {}
        virtual com::sun::star::uno::Any SAL_CALL queryInterface( com::sun::star::uno::Type const & rType ) throw (com::sun::star::uno::RuntimeException)
            { return BaseClass::queryInterface( rType ); }
        virtual com::sun::star::uno::Any SAL_CALL queryAggregation( com::sun::star::uno::Type const & rType ) throw (com::sun::star::uno::RuntimeException)
            {
                com::sun::star::uno::Any aRet( ImplHelper_queryNoXInterface( rType, cd::get(), this ) );
                if (aRet.hasValue())
                    return aRet;
                return BaseClass::queryAggregation( rType );
            }
        virtual void SAL_CALL acquire() throw ()
            { BaseClass::acquire(); }
        virtual void SAL_CALL release() throw ()
            { BaseClass::release(); }
        virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL getTypes() throw (com::sun::star::uno::RuntimeException)
            { return ImplInhHelper_getTypes( cd::get(), BaseClass::getTypes() ); }
        virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( cd::get() ); }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
