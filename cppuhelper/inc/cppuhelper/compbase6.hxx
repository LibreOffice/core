/*************************************************************************
 *
 *  $RCSfile: compbase6.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-16 17:49:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CPPUHELPER_COMPBASE6_HXX_
#define _CPPUHELPER_COMPBASE6_HXX_

#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif

#if defined( MACOSX ) && ( __GNUC__ < 3 )
/* use old impl helpers for macosx */

#ifndef _CPPUHELPER_COMPBASE_HXX_
#include <cppuhelper/compbase.hxx>
#endif

__DEF_COMPIMPLHELPER( 6 )

#else /* ! MACOSX */

#ifndef _CPPUHELPER_COMPBASE_EX_HXX_
#include <cppuhelper/compbase_ex.hxx>
#endif
/*
__DEF_COMPIMPLHELPER_EX( 6 )
*/

namespace cppu
{
    /** Implementation helper supporting ::com::sun::star::lang::XTypeProvider and
        ::com::sun::star::lang::XComponent.  Upon disposing objects of this class, sub-classes
        receive a disposing() call.  Objects of this class can be held weakly, i.e. by a
        ::com::sun::star::uno::WeakReference.

        @attention
        The life-cycle of the passed mutex reference has to be longer than objects of this class.

        @derive
        Inherit from this class giving your interface(s) to be implemented as template argument(s).
        Your sub class defines method implementations for these interface(s).
    */
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6 >
    class SAL_NO_VTABLE WeakComponentImplHelper6
        : public WeakComponentImplHelperBase
        , public ::com::sun::star::lang::XTypeProvider
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5, public Ifc6
    {
        /** @internal */
        static class_data6 s_cd;
    public:
        inline WeakComponentImplHelper6( ::osl::Mutex & rMutex ) throw ()
            : WeakComponentImplHelperBase( rMutex )
            {}
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return WeakComponentImplHelper_query( rType, (class_data *)&s_cd, this, (WeakComponentImplHelperBase *)this ); }
        virtual void SAL_CALL acquire() throw ()
            { WeakComponentImplHelperBase::acquire(); }
        virtual void SAL_CALL release() throw ()
            { WeakComponentImplHelperBase::release(); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return WeakComponentImplHelper_getTypes( (class_data *)&s_cd ); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( (class_data *)&s_cd ); }
    };
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6 >
    class_data6 WeakComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 >::s_cd =
    {
        6 +1, sal_False, sal_False,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        {
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc1 > const * ))&getCppuType, ((sal_Int32)(Ifc1 *) (WeakComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc2 > const * ))&getCppuType, ((sal_Int32)(Ifc2 *) (WeakComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc3 > const * ))&getCppuType, ((sal_Int32)(Ifc3 *) (WeakComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc4 > const * ))&getCppuType, ((sal_Int32)(Ifc4 *) (WeakComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc5 > const * ))&getCppuType, ((sal_Int32)(Ifc5 *) (WeakComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc6 > const * ))&getCppuType, ((sal_Int32)(Ifc6 *) (WeakComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider > const * ))&getCppuType, ((sal_Int32)(::com::sun::star::lang::XTypeProvider *) (WeakComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 }
        }
    };
    /** Implementation helper supporting ::com::sun::star::lang::XTypeProvider and
        ::com::sun::star::lang::XComponent.  Upon disposing objects of this class, sub-classes
        receive a disposing() call.  Objects of this class can be held weakly, i.e. by a
        ::com::sun::star::uno::WeakReference.  Object of this class can be aggregated, i.e.
        incoming queryInterface() calls are delegated.

        @attention
        The life-cycle of the passed mutex reference has to be longer than objects of this class.

        @derive
        Inherit from this class giving your interface(s) to be implemented as template argument(s).
        Your sub class defines method implementations for these interface(s).
    */
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6 >
    class SAL_NO_VTABLE WeakAggComponentImplHelper6
        : public WeakAggComponentImplHelperBase
        , public ::com::sun::star::lang::XTypeProvider
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5, public Ifc6
    {
        /** @internal */
        static class_data6 s_cd;
    public:
        inline WeakAggComponentImplHelper6( ::osl::Mutex & rMutex ) throw ()
            : WeakAggComponentImplHelperBase( rMutex )
            {}
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return WeakAggComponentImplHelperBase::queryInterface( rType ); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return WeakAggComponentImplHelper_queryAgg( rType, (class_data *)&s_cd, this, (WeakAggComponentImplHelperBase *)this ); }
        virtual void SAL_CALL acquire() throw ()
            { WeakAggComponentImplHelperBase::acquire(); }
        virtual void SAL_CALL release() throw ()
            { WeakAggComponentImplHelperBase::release(); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return WeakAggComponentImplHelper_getTypes( (class_data *)&s_cd ); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( (class_data *)&s_cd ); }
    };
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5, class Ifc6 >
    class_data6 WeakAggComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 >::s_cd =
    {
        6 +1, sal_False, sal_False,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        {
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc1 > const * ))&getCppuType, ((sal_Int32)(Ifc1 *) (WeakAggComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc2 > const * ))&getCppuType, ((sal_Int32)(Ifc2 *) (WeakAggComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc3 > const * ))&getCppuType, ((sal_Int32)(Ifc3 *) (WeakAggComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc4 > const * ))&getCppuType, ((sal_Int32)(Ifc4 *) (WeakAggComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc5 > const * ))&getCppuType, ((sal_Int32)(Ifc5 *) (WeakAggComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc6 > const * ))&getCppuType, ((sal_Int32)(Ifc6 *) (WeakAggComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider > const * ))&getCppuType, ((sal_Int32)(::com::sun::star::lang::XTypeProvider *) (WeakAggComponentImplHelper6< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5, Ifc6 > *) 16) - 16 }
        }
    };
}

#endif /* MACOSX */

#endif
