/*************************************************************************
 *
 *  $RCSfile: implbase5.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dbo $ $Date: 2001-09-04 13:24:09 $
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
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#define _CPPUHELPER_IMPLBASE5_HXX_

#ifdef MACOSX /* use old impl helpers for macosx */

#define __IFC5 Ifc1, Ifc2, Ifc3, Ifc4, Ifc5
#define __CLASS_IFC5 class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5
#define __PUBLIC_IFC5 public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5

#ifndef _CPPUHELPER_IMPLBASE_HXX_
#include <cppuhelper/implbase.hxx>
#endif

__DEF_IMPLHELPER_PRE( 5 )
    __IFC_WRITEOFFSET( 1 ) __IFC_WRITEOFFSET( 2 ) __IFC_WRITEOFFSET( 3 ) __IFC_WRITEOFFSET( 4 )
    __IFC_WRITEOFFSET( 5 )
__DEF_IMPLHELPER_POST( 5 )

#else /* ! MACOSX */

#ifndef _CPPUHELPER_IMPLBASE_EX_HXX_
#include <cppuhelper/implbase_ex.hxx>
#endif
/*
#include <cppuhelper/implbase_ex_pre.hxx>
#define __IFC_EX_TYPE_INIT5( class_cast ) \
    __IFC_EX_TYPE_INIT( class_cast, 1 ), __IFC_EX_TYPE_INIT( class_cast, 2 ), \
    __IFC_EX_TYPE_INIT( class_cast, 3 ), __IFC_EX_TYPE_INIT( class_cast, 4 ), \
    __IFC_EX_TYPE_INIT( class_cast, 5 )
#include <cppuhelper/implbase_ex_post.hxx>
__DEF_IMPLHELPER_EX( 5 )
*/

namespace cppu
{
    struct class_data5
    {
        sal_Int16 m_nTypes;
        sal_Bool m_storedTypeRefs;
        sal_Bool m_storedId;
        sal_Int8 m_id[ 16 ];
        type_entry m_typeEntries[ 5 + 1 ];
    };
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5 >
    class SAL_NO_VTABLE ImplHelper5
        : public ::com::sun::star::lang::XTypeProvider
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5
    {
        static class_data5 s_cd;
    public:
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return ImplHelper_query( rType, (class_data *)&s_cd, this ); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return ImplHelper_getTypes( (class_data *)&s_cd ); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( (class_data *)&s_cd ); }
    };
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5 >
    class_data5 ImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 >::s_cd =
    {
        5 +1, sal_False, sal_False,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        {
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc1 > const * ))&getCppuType, ((sal_Int32)(Ifc1 *) (ImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc2 > const * ))&getCppuType, ((sal_Int32)(Ifc2 *) (ImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc3 > const * ))&getCppuType, ((sal_Int32)(Ifc3 *) (ImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc4 > const * ))&getCppuType, ((sal_Int32)(Ifc4 *) (ImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc5 > const * ))&getCppuType, ((sal_Int32)(Ifc5 *) (ImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider > const * ))&getCppuType, ((sal_Int32)(::com::sun::star::lang::XTypeProvider *) (ImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 }
        }
    };
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5 >
    class SAL_NO_VTABLE WeakImplHelper5
        : public OWeakObject
        , public ::com::sun::star::lang::XTypeProvider
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5
    {
        static class_data5 s_cd;
    public:
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return WeakImplHelper_query( rType, (class_data *)&s_cd, this, (OWeakObject *)this ); }
        virtual void SAL_CALL acquire() throw ()
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw ()
            { OWeakObject::release(); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return WeakImplHelper_getTypes( (class_data *)&s_cd ); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( (class_data *)&s_cd ); }
    };
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5 >
    class_data5 WeakImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 >::s_cd =
    {
        5 +1, sal_False, sal_False,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        {
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc1 > const * ))&getCppuType, ((sal_Int32)(Ifc1 *) (WeakImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc2 > const * ))&getCppuType, ((sal_Int32)(Ifc2 *) (WeakImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc3 > const * ))&getCppuType, ((sal_Int32)(Ifc3 *) (WeakImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc4 > const * ))&getCppuType, ((sal_Int32)(Ifc4 *) (WeakImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc5 > const * ))&getCppuType, ((sal_Int32)(Ifc5 *) (WeakImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider > const * ))&getCppuType, ((sal_Int32)(::com::sun::star::lang::XTypeProvider *) (WeakImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 }
        }
    };
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5 >
    class SAL_NO_VTABLE WeakAggImplHelper5
        : public OWeakAggObject
        , public ::com::sun::star::lang::XTypeProvider
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5
    {
        static class_data5 s_cd;
    public:
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return OWeakAggObject::queryInterface( rType ); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return WeakAggImplHelper_queryAgg( rType, (class_data *)&s_cd, this, (OWeakAggObject *)this ); }
        virtual void SAL_CALL acquire() throw ()
            { OWeakAggObject::acquire(); }
        virtual void SAL_CALL release() throw ()
            { OWeakAggObject::release(); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return WeakAggImplHelper_getTypes( (class_data *)&s_cd ); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( (class_data *)&s_cd ); }
    };
    template< class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5 >
    class_data5 WeakAggImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 >::s_cd =
    {
        5 +1, sal_False, sal_False,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        {
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc1 > const * ))&getCppuType, ((sal_Int32)(Ifc1 *) (WeakAggImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc2 > const * ))&getCppuType, ((sal_Int32)(Ifc2 *) (WeakAggImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc3 > const * ))&getCppuType, ((sal_Int32)(Ifc3 *) (WeakAggImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc4 > const * ))&getCppuType, ((sal_Int32)(Ifc4 *) (WeakAggImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc5 > const * ))&getCppuType, ((sal_Int32)(Ifc5 *) (WeakAggImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider > const * ))&getCppuType, ((sal_Int32)(::com::sun::star::lang::XTypeProvider *) (WeakAggImplHelper5< Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 }
        }
    };
    template< class BaseClass, class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5 >
    class SAL_NO_VTABLE ImplInheritanceHelper5
        : public BaseClass
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5
    {
        static class_data5 s_cd;
    public:
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
            {
                ::com::sun::star::uno::Any aRet( ImplHelper_queryNoXInterface( rType, (class_data *)&s_cd, this ) );
                if (aRet.hasValue())
                    return aRet;
                return BaseClass::queryInterface( rType );
            }
        virtual void SAL_CALL acquire() throw ()
            { BaseClass::acquire(); }
        virtual void SAL_CALL release() throw ()
            { BaseClass::release(); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return ImplInhHelper_getTypes( (class_data *)&s_cd, BaseClass::getTypes() ); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( (class_data *)&s_cd ); }
    };
    template< class BaseClass, class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5 >
    class_data5 ImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 >::s_cd =
    {
        5 +1, sal_False, sal_False,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        {
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc1 > const * ))&getCppuType, ((sal_Int32)(Ifc1 *) (ImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc2 > const * ))&getCppuType, ((sal_Int32)(Ifc2 *) (ImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc3 > const * ))&getCppuType, ((sal_Int32)(Ifc3 *) (ImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc4 > const * ))&getCppuType, ((sal_Int32)(Ifc4 *) (ImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc5 > const * ))&getCppuType, ((sal_Int32)(Ifc5 *) (ImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider > const * ))&getCppuType, ((sal_Int32)(::com::sun::star::lang::XTypeProvider *) (ImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 }
        }
    };
    template< class BaseClass, class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5 >
    class SAL_NO_VTABLE AggImplInheritanceHelper5
        : public BaseClass
        , public Ifc1, public Ifc2, public Ifc3, public Ifc4, public Ifc5
    {
        static class_data5 s_cd;
    public:
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
            { return BaseClass::queryInterface( rType ); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
            {
                ::com::sun::star::uno::Any aRet( ImplHelper_queryNoXInterface( rType, (class_data *)&s_cd, this ) );
                if (aRet.hasValue())
                    return aRet;
                return BaseClass::queryAggregation( rType );
            }
        virtual void SAL_CALL acquire() throw ()
            { BaseClass::acquire(); }
        virtual void SAL_CALL release() throw ()
            { BaseClass::release(); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException)
            { return ImplInhHelper_getTypes( (class_data *)&s_cd, BaseClass::getTypes() ); }
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException)
            { return ImplHelper_getImplementationId( (class_data *)&s_cd ); }
    };
    template< class BaseClass, class Ifc1, class Ifc2, class Ifc3, class Ifc4, class Ifc5 >
    class_data5 AggImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 >::s_cd =
    {
        5 +1, sal_False, sal_False,
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        {
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc1 > const * ))&getCppuType, ((sal_Int32)(Ifc1 *) (AggImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc2 > const * ))&getCppuType, ((sal_Int32)(Ifc2 *) (AggImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc3 > const * ))&getCppuType, ((sal_Int32)(Ifc3 *) (AggImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc4 > const * ))&getCppuType, ((sal_Int32)(Ifc4 *) (AggImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< Ifc5 > const * ))&getCppuType, ((sal_Int32)(Ifc5 *) (AggImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 },
            { (::cppu::fptr_getCppuType)(::com::sun::star::uno::Type const & (SAL_CALL *)( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider > const * ))&getCppuType, ((sal_Int32)(::com::sun::star::lang::XTypeProvider *) (AggImplInheritanceHelper5< BaseClass, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 > *) 16) - 16 }
        }
    };
}

#endif /* MACOSX */

#endif
