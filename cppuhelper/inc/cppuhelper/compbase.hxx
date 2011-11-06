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
