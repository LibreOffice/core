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
#ifndef _CPPUHELPER_IMPLBASE_EX_HXX_
#define _CPPUHELPER_IMPLBASE_EX_HXX_

#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakagg.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>

// Despite the fact that the following include is not used in this header, it has to remain,
// because it is expected by files including cppuhelper/implbaseN.hxx.
// So maybe we can omit it some time in the future...
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

/* If you need to define implementation helper classes that deal with more than
   12 interfaces, then use macros as follows, e.g. for 3 interfaces:

#include <cppuhelper/implbase_ex_pre.hxx>
#define __IFC_EX_TYPE_INIT3( class_cast ) \
    __IFC_EX_TYPE_INIT( class_cast, 1 ), __IFC_EX_TYPE_INIT( class_cast, 2 ), \
    __IFC_EX_TYPE_INIT( class_cast, 3 )
#include <cppuhelper/implbase_ex_post.hxx>
__DEF_IMPLHELPER_EX( 3 )
*/


namespace cppu
{

/** function pointer signature for getCppuType
    @internal
*/
typedef ::com::sun::star::uno::Type const & (SAL_CALL * fptr_getCppuType)( void * ) SAL_THROW( () );

/** single type + object offset
    @internal
*/
struct type_entry
{
    /** the type_entry is initialized with function pointer to ::getCppuType() function first,
        but holds an unacquired typelib_TypeDescriptionReference * after initialization,
        thus reusing the memory.  Flag class_data::m_storedTypeRefs
    */
    union
    {
        fptr_getCppuType getCppuType;
        typelib_TypeDescriptionReference * typeRef;
    } m_type;
    /** offset for interface pointer
    */
    sal_IntPtr m_offset;
};

/** identical dummy struct for casting class_dataN to class_data
    @internal
*/
struct class_data
{
    /** number of supported types in m_typeEntries
    */
    sal_Int16 m_nTypes;

    /** determines whether m_typeEntries is initialized and carries unacquired type refs
    */
    sal_Bool m_storedTypeRefs;

    /** determines whether an implementation id was created in m_id
    */
    sal_Bool m_createdId;

    /** implementation id
     */
    sal_Int8 m_id[ 16 ];

    /** type, object offset
    */
    type_entry m_typeEntries[ 1 ];
};

/** ImplHelper
    @internal
*/
::com::sun::star::uno::Any SAL_CALL ImplHelper_query(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** ImplHelper
    @internal
*/
::com::sun::star::uno::Any SAL_CALL ImplHelper_queryNoXInterface(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** ImplHelper
    @internal
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL ImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** ImplHelper
    @internal
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL ImplInhHelper_getTypes(
    class_data * cd,
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > const & rAddTypes )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** ImplHelper
    @internal
*/
::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL ImplHelper_getImplementationId(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

/** WeakImplHelper
    @internal
*/
::com::sun::star::uno::Any SAL_CALL WeakImplHelper_query(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::OWeakObject * pBase )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** WeakImplHelper
    @internal
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL WeakImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

/** WeakAggImplHelper
    @internal
*/
::com::sun::star::uno::Any SAL_CALL WeakAggImplHelper_queryAgg(
    ::com::sun::star::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::OWeakAggObject * pBase )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );
/** WeakAggImplHelper
    @internal
*/
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL WeakAggImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
