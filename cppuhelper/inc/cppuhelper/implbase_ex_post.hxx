/*************************************************************************
 *
 *  $RCSfile: implbase_ex_post.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2001-09-04 09:03:08 $
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
#ifndef _CPPUHELPER_IMPLBASE_EX_POST_HXX_
#define _CPPUHELPER_IMPLBASE_EX_POST_HXX_


#define __DEF_CLASS_DATA_INIT_EX( N, class_cast ) \
{ \
N +1, sal_False, sal_False, \
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, \
{ \
__IFC_EX_TYPE_INIT##N( class_cast ), \
__IFC_EX_TYPE_INIT_NAME( class_cast, ::com::sun::star::lang::XTypeProvider ) \
} \
}

#define __DEF_IMPLHELPER_EX( N ) \
namespace cppu \
{ \
struct class_data##N \
{ \
    sal_Int16 m_nTypes; \
    sal_Bool m_storedTypeRefs; \
    sal_Bool m_storedId; \
    sal_Int8 m_id[ 16 ]; \
    type_entry m_typeEntries[ N + 1 ]; \
}; \
template< __CLASS_IFC##N > \
class SAL_NO_VTABLE ImplHelper##N \
    : public ::com::sun::star::lang::XTypeProvider \
    , __PUBLIC_IFC##N \
{ \
    static class_data##N s_cd; \
public: \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
        { return ImplHelper_query( rType, (class_data *)&s_cd, this ); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return ImplHelper_getTypes( (class_data *)&s_cd ); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return ImplHelper_getImplementationId( (class_data *)&s_cd ); } \
}; \
template< __CLASS_IFC##N > \
class_data##N ImplHelper##N< __IFC##N >::s_cd = \
__DEF_CLASS_DATA_INIT_EX( N, (ImplHelper##N< __IFC##N > *) ); \
template< __CLASS_IFC##N > \
class SAL_NO_VTABLE WeakImplHelper##N \
    : public OWeakObject \
    , public ::com::sun::star::lang::XTypeProvider \
    , __PUBLIC_IFC##N \
{ \
    static class_data##N s_cd; \
public: \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
        { return WeakImplHelper_query( rType, (class_data *)&s_cd, this, (OWeakObject *)this ); } \
    virtual void SAL_CALL acquire() throw () \
        { OWeakObject::acquire(); } \
    virtual void SAL_CALL release() throw () \
        { OWeakObject::release(); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return WeakImplHelper_getTypes( (class_data *)&s_cd ); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return ImplHelper_getImplementationId( (class_data *)&s_cd ); } \
}; \
template< __CLASS_IFC##N > \
class_data##N WeakImplHelper##N< __IFC##N >::s_cd = \
__DEF_CLASS_DATA_INIT_EX( N, (WeakImplHelper##N< __IFC##N > *) ); \
template< __CLASS_IFC##N > \
class SAL_NO_VTABLE WeakAggImplHelper##N \
    : public OWeakAggObject \
    , public ::com::sun::star::lang::XTypeProvider \
    , __PUBLIC_IFC##N \
{ \
    static class_data##N s_cd; \
public: \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
        { return OWeakAggObject::queryInterface( rType ); } \
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
        { return WeakAggImplHelper_queryAgg( rType, (class_data *)&s_cd, this, (OWeakAggObject *)this ); } \
    virtual void SAL_CALL acquire() throw () \
        { OWeakAggObject::acquire(); } \
    virtual void SAL_CALL release() throw () \
        { OWeakAggObject::release(); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return WeakAggImplHelper_getTypes( (class_data *)&s_cd ); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return ImplHelper_getImplementationId( (class_data *)&s_cd ); } \
}; \
template< __CLASS_IFC##N > \
class_data##N WeakAggImplHelper##N< __IFC##N >::s_cd = \
__DEF_CLASS_DATA_INIT_EX( N, (WeakAggImplHelper##N< __IFC##N > *) ); \
template< class BaseClass, __CLASS_IFC##N > \
class SAL_NO_VTABLE ImplInheritanceHelper##N \
    : public BaseClass \
    , __PUBLIC_IFC##N \
{ \
    static class_data##N s_cd; \
public: \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aRet( ImplHelper_queryNoXInterface( rType, (class_data *)&s_cd, this ) ); \
        if (aRet.hasValue()) \
            return aRet; \
        return BaseClass::queryInterface( rType ); \
    } \
    virtual void SAL_CALL acquire() throw () \
        { BaseClass::acquire(); } \
    virtual void SAL_CALL release() throw () \
        { BaseClass::release(); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return ImplInhHelper_getTypes( (class_data *)&s_cd, BaseClass::getTypes() ); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return ImplHelper_getImplementationId( (class_data *)&s_cd ); } \
}; \
template< class BaseClass, __CLASS_IFC##N > \
class_data##N ImplInheritanceHelper##N< BaseClass, __IFC##N >::s_cd = \
__DEF_CLASS_DATA_INIT_EX( N, (ImplInheritanceHelper##N< BaseClass, __IFC##N > *) ); \
template< class BaseClass, __CLASS_IFC##N > \
class SAL_NO_VTABLE AggImplInheritanceHelper##N \
    : public BaseClass \
    , __PUBLIC_IFC##N \
{ \
    static class_data##N s_cd; \
public: \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
        { return BaseClass::queryInterface( rType ); } \
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aRet( ImplHelper_queryNoXInterface( rType, (class_data *)&s_cd, this ) ); \
        if (aRet.hasValue()) \
            return aRet; \
        return BaseClass::queryAggregation( rType ); \
    } \
    virtual void SAL_CALL acquire() throw () \
        { BaseClass::acquire(); } \
    virtual void SAL_CALL release() throw () \
        { BaseClass::release(); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return ImplInhHelper_getTypes( (class_data *)&s_cd, BaseClass::getTypes() ); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return ImplHelper_getImplementationId( (class_data *)&s_cd ); } \
}; \
template< class BaseClass, __CLASS_IFC##N > \
class_data##N AggImplInheritanceHelper##N< BaseClass, __IFC##N >::s_cd = \
__DEF_CLASS_DATA_INIT_EX( N, (AggImplInheritanceHelper##N< BaseClass, __IFC##N > *) ); \
}

#define __DEF_COMPIMPLHELPER_EX( N ) \
namespace cppu \
{ \
template< __CLASS_IFC##N > \
class SAL_NO_VTABLE WeakComponentImplHelper##N \
    : public WeakComponentImplHelperBase \
    , public ::com::sun::star::lang::XTypeProvider \
    , __PUBLIC_IFC##N \
{ \
    static class_data##N s_cd; \
public: \
    inline WeakComponentImplHelper##N( ::osl::Mutex & rMutex ) SAL_THROW( () ) \
        : WeakComponentImplHelperBase( rMutex ) \
        {} \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
        { return WeakComponentImplHelper_query( rType, (class_data *)&s_cd, this, (WeakComponentImplHelperBase *)this ); } \
    virtual void SAL_CALL acquire() throw () \
        { WeakComponentImplHelperBase::acquire(); } \
    virtual void SAL_CALL release() throw () \
        { WeakComponentImplHelperBase::release(); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return WeakComponentImplHelper_getTypes( (class_data *)&s_cd ); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return ImplHelper_getImplementationId( (class_data *)&s_cd ); } \
}; \
template< __CLASS_IFC##N > \
class_data##N WeakComponentImplHelper##N< __IFC##N >::s_cd = \
__DEF_CLASS_DATA_INIT_EX( N, (WeakComponentImplHelper##N< __IFC##N > *) ); \
template< __CLASS_IFC##N > \
class SAL_NO_VTABLE WeakAggComponentImplHelper##N \
    : public WeakAggComponentImplHelperBase \
    , public ::com::sun::star::lang::XTypeProvider \
    , __PUBLIC_IFC##N \
{ \
    static class_data##N s_cd; \
public: \
    inline WeakAggComponentImplHelper##N( ::osl::Mutex & rMutex ) SAL_THROW( () ) \
        : WeakAggComponentImplHelperBase( rMutex ) \
        {} \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
        { return WeakAggComponentImplHelperBase::queryInterface( rType ); } \
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
        { return WeakAggComponentImplHelper_queryAgg( rType, (class_data *)&s_cd, this, (WeakAggComponentImplHelperBase *)this ); } \
    virtual void SAL_CALL acquire() throw () \
        { WeakAggComponentImplHelperBase::acquire(); } \
    virtual void SAL_CALL release() throw () \
        { WeakAggComponentImplHelperBase::release(); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return WeakAggComponentImplHelper_getTypes( (class_data *)&s_cd ); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return ImplHelper_getImplementationId( (class_data *)&s_cd ); } \
}; \
template< __CLASS_IFC##N > \
class_data##N WeakAggComponentImplHelper##N< __IFC##N >::s_cd = \
__DEF_CLASS_DATA_INIT_EX( N, (WeakAggComponentImplHelper##N< __IFC##N > *) ); \
}

#endif
