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
#ifndef _CPPUHELPER_IMPLBASE_EX_POST_HXX_
#define _CPPUHELPER_IMPLBASE_EX_POST_HXX_

/// @cond INTERNAL

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
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE ImplHelper##N \
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
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE WeakImplHelper##N \
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
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE WeakAggImplHelper##N \
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
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE ImplInheritanceHelper##N \
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
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE AggImplInheritanceHelper##N \
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

/// @endcond

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
