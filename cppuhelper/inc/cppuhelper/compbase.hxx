/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: compbase.hxx,v $
 * $Revision: 1.14 $
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
#ifndef _CPPUHELPER_COMPBASE_HXX_
#define _CPPUHELPER_COMPBASE_HXX_

#include <cppuhelper/compbase_ex.hxx>
#include <cppuhelper/implbase.hxx>

/* This header should not be used anymore.
   @deprecated
*/

//==================================================================================================
/** @internal */
#define __DEF_COMPIMPLHELPER_A( N ) \
namespace cppu \
{ \
template< __CLASS_IFC##N > \
class SAL_NO_VTABLE WeakComponentImplHelper##N \
    : public ::cppu::WeakComponentImplHelperBase \
    , public ImplHelperBase##N< __IFC##N > \
{ \
    static ClassData##N s_aCD; \
public: \
    WeakComponentImplHelper##N( ::osl::Mutex & rMutex ) SAL_THROW( () ) \
        : WeakComponentImplHelperBase( rMutex ) \
        {} \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aRet( getClassData( s_aCD ).query( rType, (ImplHelperBase##N< __IFC##N > *)this ) ); \
        if (aRet.hasValue()) \
            return aRet; \
        return WeakComponentImplHelperBase::queryInterface( rType ); \
    } \
    virtual void SAL_CALL acquire() throw () \
        { WeakComponentImplHelperBase::acquire(); } \
    virtual void SAL_CALL release() throw () \
        { WeakComponentImplHelperBase::release(); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getTypes(); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getImplementationId(); } \
}; \
template< __CLASS_IFC##N > \
class SAL_NO_VTABLE WeakAggComponentImplHelper##N \
    : public ::cppu::WeakAggComponentImplHelperBase \
    , public ImplHelperBase##N< __IFC##N > \
{ \
    static ClassData##N s_aCD; \
public: \
    WeakAggComponentImplHelper##N( ::osl::Mutex & rMutex ) SAL_THROW( () ) \
        : WeakAggComponentImplHelperBase( rMutex ) \
        {} \
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
        { return WeakAggComponentImplHelperBase::queryInterface( rType ); } \
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( ::com::sun::star::uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        ::com::sun::star::uno::Any aRet( getClassData( s_aCD ).query( rType, (ImplHelperBase##N< __IFC##N > *)this ) ); \
        if (aRet.hasValue()) \
            return aRet; \
        return WeakAggComponentImplHelperBase::queryAggregation( rType ); \
    } \
    virtual void SAL_CALL acquire() throw () \
        { WeakAggComponentImplHelperBase::acquire(); } \
    virtual void SAL_CALL release() throw () \
        { WeakAggComponentImplHelperBase::release(); } \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getTypes(); } \
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException) \
        { return getClassData( s_aCD ).getImplementationId(); } \
};

//==================================================================================================
/** @internal */
#define __DEF_COMPIMPLHELPER_B( N ) \
template< __CLASS_IFC##N > \
ClassData##N WeakComponentImplHelper##N< __IFC##N >::s_aCD = ClassData##N( 4 ); \
template< __CLASS_IFC##N > \
ClassData##N WeakAggComponentImplHelper##N< __IFC##N >::s_aCD = ClassData##N( 3 );
//==================================================================================================
/** @internal */
#define __DEF_COMPIMPLHELPER_C( N ) \
}
//==================================================================================================
/** @internal */
#define __DEF_COMPIMPLHELPER( N ) \
__DEF_COMPIMPLHELPER_A( N ) \
__DEF_COMPIMPLHELPER_B( N ) \
__DEF_COMPIMPLHELPER_C( N )

#endif
