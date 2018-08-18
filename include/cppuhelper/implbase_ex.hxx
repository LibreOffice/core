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
#ifndef INCLUDED_CPPUHELPER_IMPLBASE_EX_HXX
#define INCLUDED_CPPUHELPER_IMPLBASE_EX_HXX

#include "com/sun/star/uno/Any.h"
#include "com/sun/star/uno/Sequence.h"
#include "com/sun/star/uno/Type.h"
#include "com/sun/star/uno/genfunc.h"
#include "cppuhelper/cppuhelperdllapi.h"
#include "sal/types.h"

namespace cppu { class OWeakAggObject; }
namespace cppu { class OWeakObject; }


/* If you need to define implementation helper classes that deal with more than
   12 interfaces, then use macros as follows, e.g. for 3 interfaces:

#include <cppuhelper/implbase_ex_pre.hxx>
#define __IFC_EX_TYPE_INIT3( class_cast ) \
    __IFC_EX_TYPE_INIT( class_cast, 1 ), __IFC_EX_TYPE_INIT( class_cast, 2 ), \
    __IFC_EX_TYPE_INIT( class_cast, 3 )
#include <cppuhelper/implbase_ex_post.hxx>
__DEF_IMPLHELPER_EX( 3 )
*/

/// @cond INTERNAL

namespace cppu
{

/** function pointer signature for getCppuType
*/
typedef css::uno::Type const & (SAL_CALL * fptr_getCppuType)( void * );

/** single type + object offset
*/
struct SAL_WARN_UNUSED type_entry
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

/** in the future, this can be a constexpr template method */
#define CPPUHELPER_DETAIL_TYPEENTRY(Ifc) \
    { { Ifc::static_type }, \
      reinterpret_cast<sal_IntPtr>( static_cast<Ifc *>( reinterpret_cast<Impl *>(16) )) - 16 }

/** identical dummy struct for casting class_dataN to class_data
*/
struct SAL_WARN_UNUSED class_data
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
*/
CPPUHELPER_DLLPUBLIC css::uno::Any SAL_CALL ImplHelper_query(
    css::uno::Type const & rType,
    class_data * cd,
    void * that );
/** ImplHelper
*/
CPPUHELPER_DLLPUBLIC css::uno::Any SAL_CALL ImplHelper_queryNoXInterface(
    css::uno::Type const & rType,
    class_data * cd,
    void * that );
/** ImplHelper
*/
CPPUHELPER_DLLPUBLIC css::uno::Sequence< css::uno::Type >
SAL_CALL ImplHelper_getTypes(
    class_data * cd );
/** ImplHelper
*/
CPPUHELPER_DLLPUBLIC css::uno::Sequence< css::uno::Type >
SAL_CALL ImplInhHelper_getTypes(
    class_data * cd,
    css::uno::Sequence< css::uno::Type > const & rAddTypes );
/** ImplHelper
*/
CPPUHELPER_DLLPUBLIC css::uno::Sequence< sal_Int8 >
SAL_CALL ImplHelper_getImplementationId(
    class_data * cd );

/** WeakImplHelper
*/
CPPUHELPER_DLLPUBLIC css::uno::Any SAL_CALL WeakImplHelper_query(
    css::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::OWeakObject * pBase );
/** WeakImplHelper
*/
CPPUHELPER_DLLPUBLIC css::uno::Sequence< css::uno::Type >
SAL_CALL WeakImplHelper_getTypes(
    class_data * cd );

/** WeakAggImplHelper
*/
CPPUHELPER_DLLPUBLIC css::uno::Any
SAL_CALL WeakAggImplHelper_queryAgg(
    css::uno::Type const & rType,
    class_data * cd,
    void * that,
    ::cppu::OWeakAggObject * pBase );
/** WeakAggImplHelper
*/
CPPUHELPER_DLLPUBLIC css::uno::Sequence< css::uno::Type >
SAL_CALL WeakAggImplHelper_getTypes(
    class_data * cd );

}

/// @endcond

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
