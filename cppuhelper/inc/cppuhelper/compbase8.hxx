/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _CPPUHELPER_COMPBASE8_HXX_
#define _CPPUHELPER_COMPBASE8_HXX_

#include <cppuhelper/implbase8.hxx>
#include <cppuhelper/compbase_ex.hxx>
/*
__DEF_COMPIMPLHELPER_EX( 8 )
*/

namespace cppu
{

// Suppress warnings about hidden functions in case any of the IfcN has
// functions named dispose, addEventListener, or removeEventListener:
#if defined __SUNPRO_CC
#pragma disable_warn
#endif

    /** Implementation helper supporting ::com::sun::star::lang::XTypeProvider and
        ::com::sun::star::lang::XComponent.

        Upon disposing objects of this class, sub-classes receive a disposing()
        call.  Objects of this class can be held weakly, i.e. by a
        ::com::sun::star::uno::WeakReference.

        @attention
        The life-cycle of the passed mutex reference has to be longer than objects of this class.

        @derive
        Inherit from this class giving your interface(s) to be implemented as template argument(s).
        Your sub class defines method implementations for these interface(s).
    */
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6, class Ifc7, class Ifc8 >
    class SAL_NO_VTABLE WeakComponentImplHelper8
        : public WeakComponentImplHelperBase
        , public ::com::sun::star::lang::XTypeProvider
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5, public Ifc6, public Ifc7, public Ifc8
    {
        /** @internal */
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData8< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6, Ifc7, Ifc8, WeakComponentImplHelper8<Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6, Ifc7, Ifc8> > > {};
    public:
        inline WeakComponentImplHelper8( ::osl::Mutex & rMutex ) throw ()
            : WeakComponentImplHelperBase( rMutex )
            {}
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return WeakComponentImplHelper_query( rType, cd::get(), this, (WeakComponentImplHelperBase *)this ); }
        virtual void SAL_CALL acquire() throw ()
            { WeakComponentImplHelperBase::acquire(); }
        virtual void SAL_CALL release() throw ()
            { WeakComponentImplHelperBase::release(); }
        virtual void SAL_CALL dispose()throw (::com::sun::star::uno::RuntimeException)
            { WeakComponentImplHelperBase::dispose(); }
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & xListener)throw (::com::sun::star::uno::RuntimeException)
            { WeakComponentImplHelperBase::addEventListener(xListener); }
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & xListener)throw (::com::sun::star::uno::RuntimeException)
            { WeakComponentImplHelperBase::removeEventListener(xListener); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return WeakComponentImplHelper_getTypes( cd::get() ); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( cd::get() ); }
    };

#if defined __SUNPRO_CC
#pragma enable_warn
#endif

    /** Implementation helper supporting ::com::sun::star::lang::XTypeProvider and
        ::com::sun::star::lang::XComponent.

        Upon disposing objects of this class, sub-classes receive a disposing()
        call.  Objects of this class can be held weakly, i.e. by a
        ::com::sun::star::uno::WeakReference.  Object of this class can be
        aggregated, i.e. incoming queryInterface() calls are delegated.

        @attention
        The life-cycle of the passed mutex reference has to be longer than objects of this class.

        @derive
        Inherit from this class giving your interface(s) to be implemented as template argument(s).
        Your sub class defines method implementations for these interface(s).

        @deprecated
    */
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6, class Ifc7, class Ifc8 >
    class SAL_NO_VTABLE WeakAggComponentImplHelper8
        : public WeakAggComponentImplHelperBase
        , public ::com::sun::star::lang::XTypeProvider
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5, public Ifc6, public Ifc7, public Ifc8
    {
        /** @internal */
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData8< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6, Ifc7, Ifc8, WeakAggComponentImplHelper8<Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6, Ifc7, Ifc8> > > {};
    public:
        inline WeakAggComponentImplHelper8( ::osl::Mutex & rMutex ) throw ()
            : WeakAggComponentImplHelperBase( rMutex )
            {}
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return WeakAggComponentImplHelperBase::queryInterface( rType ); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return WeakAggComponentImplHelper_queryAgg( rType, cd::get(), this, (WeakAggComponentImplHelperBase *)this ); }
        virtual void SAL_CALL acquire() throw ()
            { WeakAggComponentImplHelperBase::acquire(); }
        virtual void SAL_CALL release() throw ()
            { WeakAggComponentImplHelperBase::release(); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return WeakAggComponentImplHelper_getTypes( cd::get() ); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( cd::get() ); }
    };
}

#endif
