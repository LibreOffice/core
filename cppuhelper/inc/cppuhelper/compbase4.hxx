/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: compbase4.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2005-10-27 17:15:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CPPUHELPER_COMPBASE4_HXX_
#define _CPPUHELPER_COMPBASE4_HXX_

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif

#ifndef _CPPUHELPER_COMPBASE_EX_HXX_
#include <cppuhelper/compbase_ex.hxx>
#endif
/*
__DEF_COMPIMPLHELPER_EX( 4 )
*/

namespace cppu
{
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
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4 >
    class SAL_NO_VTABLE WeakComponentImplHelper4
        : public WeakComponentImplHelperBase
        , public ::com::sun::star::lang::XTypeProvider
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4
    {
        /** @internal */
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData4 < Ifc1, Ifc2, Ifc3, Ifc4, WeakComponentImplHelper4<Ifc1, Ifc2, Ifc3, Ifc4> > > {};
    public:
        inline WeakComponentImplHelper4( ::osl::Mutex & rMutex ) throw ()
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
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4 >
    class SAL_NO_VTABLE WeakAggComponentImplHelper4
        : public WeakAggComponentImplHelperBase
        , public ::com::sun::star::lang::XTypeProvider
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4
    {
        /** @internal */
        struct cd : public rtl::StaticAggregate< class_data, ImplClassData4 < Ifc1, Ifc2, Ifc3, Ifc4, WeakAggComponentImplHelper4<Ifc1, Ifc2, Ifc3, Ifc4> > > {};
    public:
        inline WeakAggComponentImplHelper4( ::osl::Mutex & rMutex ) throw ()
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
