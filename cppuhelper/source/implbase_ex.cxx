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

#include <sal/alloca.h>

#include <string.h>
#include <osl/diagnose.h>
#include <rtl/byteseq.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/uuid.h>
#include <cppuhelper/compbase_ex.hxx>

#include "com/sun/star/uno/RuntimeException.hpp"

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using rtl::OString;
using rtl::OUString;
using rtl::OUStringBuffer;

namespace cppu
{

/** Shared mutex for implementation helper initialization.
    Not for public use.
*/
::osl::Mutex & SAL_CALL getImplHelperInitMutex(void) SAL_THROW(());

//--------------------------------------------------------------------------------------------------
static inline void checkInterface( Type const & rType )
    SAL_THROW( (RuntimeException) )
{
    if (TypeClass_INTERFACE != rType.getTypeClass())
    {
        OUStringBuffer buf( 64 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("querying for interface \"") );
        buf.append( rType.getTypeName() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\": no interface type!") );
        OUString msg( buf.makeStringAndClear() );
#if OSL_DEBUG_LEVEL > 0
        OString str( OUStringToOString( msg, RTL_TEXTENCODING_ASCII_US ) );
        OSL_FAIL( str.getStr() );
#endif
        throw RuntimeException( msg, Reference< XInterface >() );
    }
}
//--------------------------------------------------------------------------------------------------
static inline bool isXInterface( rtl_uString * pStr ) SAL_THROW(())
{
    return (*((OUString const *)&pStr) == "com.sun.star.uno.XInterface");
}
//--------------------------------------------------------------------------------------------------
static inline void * makeInterface( sal_IntPtr nOffset, void * that ) SAL_THROW(())
{
    return (((char *)that) + nOffset);
}
//--------------------------------------------------------------------------------------------------
static inline bool __td_equals(
    typelib_TypeDescriptionReference const * pTDR1,
    typelib_TypeDescriptionReference const * pTDR2 )
    SAL_THROW(())
{
    return ((pTDR1 == pTDR2) ||
            ((OUString const *)&pTDR1->pTypeName)->equals( *(OUString const *)&pTDR2->pTypeName ) != sal_False);
}
//--------------------------------------------------------------------------------------------------
static inline type_entry * __getTypeEntries( class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    type_entry * pEntries = cd->m_typeEntries;
    if (! cd->m_storedTypeRefs) // not inited?
    {
        MutexGuard guard( getImplHelperInitMutex() );
        if (! cd->m_storedTypeRefs) // not inited?
        {
            // get all types
            for ( sal_Int32 n = cd->m_nTypes; n--; )
            {
                type_entry * pEntry = &pEntries[ n ];
                Type const & rType = (*pEntry->m_type.getCppuType)( 0 );
                OSL_ENSURE( rType.getTypeClass() == TypeClass_INTERFACE, "### wrong helper init: expected interface!" );
                OSL_ENSURE( ! isXInterface( rType.getTypeLibType()->pTypeName ), "### want to implement XInterface: template argument is XInterface?!?!?!" );
                if (rType.getTypeClass() != TypeClass_INTERFACE)
                {
                    OUStringBuffer buf( 48 );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("type \"") );
                    buf.append( rType.getTypeName() );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" is no interface type!") );
                    OUString msg( buf.makeStringAndClear() );
#if OSL_DEBUG_LEVEL > 0
                    OString str( OUStringToOString( msg, RTL_TEXTENCODING_ASCII_US ) );
                    OSL_FAIL( str.getStr() );
#endif
                    throw RuntimeException( msg, Reference< XInterface >() );
                }
                // ref is statically held by getCppuType()
                pEntry->m_type.typeRef = rType.getTypeLibType();
            }
            cd->m_storedTypeRefs = sal_True;
        }
    }
    return pEntries;
}
//--------------------------------------------------------------------------------------------------
static inline void __fillTypes( Type * types, class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    type_entry * pEntries = __getTypeEntries( cd );
    for ( sal_Int32 n = cd->m_nTypes; n--; )
    {
        types[ n ] = pEntries[ n ].m_type.typeRef;
    }
}
//--------------------------------------------------------------------------------------------------
namespace {

bool recursivelyFindType(
    typelib_TypeDescriptionReference const * demandedType,
    typelib_InterfaceTypeDescription const * type, sal_IntPtr * offset)
{
    // This code assumes that the vtables of a multiple-inheritance class (the
    // offset amount by which to adjust the this pointer) follow one another in
    // the object layout, and that they contain slots for the inherited classes
    // in a specifc order.  In theory, that need not hold for any given
    // platform; in practice, it seems to work well on all supported platforms:
 next:
    for (sal_Int32 i = 0; i < type->nBaseTypes; ++i) {
        if (i > 0) {
            *offset += sizeof (void *);
        }
        typelib_InterfaceTypeDescription const * base = type->ppBaseTypes[i];
        // ignore XInterface:
        if (base->nBaseTypes > 0) {
            if (__td_equals(
                    reinterpret_cast<
                        typelib_TypeDescriptionReference const * >(base),
                    demandedType))
            {
                return true;
            }
            // Profiling showed that it is important to speed up the common case
            // of only one base:
            if (type->nBaseTypes == 1) {
                type = base;
                goto next;
            }
            if (recursivelyFindType(demandedType, base, offset)) {
                return true;
            }
        }
    }
    return false;
}

}

static inline void * __queryDeepNoXInterface(
    typelib_TypeDescriptionReference * pDemandedTDR, class_data * cd, void * that )
    SAL_THROW( (RuntimeException) )
{
    type_entry * pEntries = __getTypeEntries( cd );
    sal_Int32 nTypes = cd->m_nTypes;
    sal_Int32 n;

    // try top interfaces without getting td
    for ( n = 0; n < nTypes; ++n )
    {
        if (__td_equals( pEntries[ n ].m_type.typeRef, pDemandedTDR ))
        {
            return makeInterface( pEntries[ n ].m_offset, that );
        }
    }
    // query deep getting td
    for ( n = 0; n < nTypes; ++n )
    {
        typelib_TypeDescription * pTD = 0;
        TYPELIB_DANGER_GET( &pTD, pEntries[ n ].m_type.typeRef );
        if (pTD)
        {
            // exclude top (already tested) and bottom (XInterface) interface
            OSL_ENSURE(
                reinterpret_cast< typelib_InterfaceTypeDescription * >(pTD)->
                    nBaseTypes > 0,
                "### want to implement XInterface:"
                    " template argument is XInterface?!?!?!" );
            sal_IntPtr offset = pEntries[n].m_offset;
            bool found = recursivelyFindType(
                pDemandedTDR,
                reinterpret_cast< typelib_InterfaceTypeDescription * >(pTD),
                &offset);
            TYPELIB_DANGER_RELEASE( pTD );
            if (found) {
                return makeInterface( offset, that );
            }
        }
        else
        {
            OUStringBuffer buf( 64 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("cannot get type description for type \"") );
            buf.append( pEntries[ n ].m_type.typeRef->pTypeName );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
            OUString msg( buf.makeStringAndClear() );
#if OSL_DEBUG_LEVEL > 0
            OString str( OUStringToOString( msg, RTL_TEXTENCODING_ASCII_US ) );
            OSL_FAIL( str.getStr() );
#endif
            throw RuntimeException( msg, Reference< XInterface >() );
        }
    }
    return 0;
}

// ImplHelper
//==================================================================================================
Any SAL_CALL ImplHelper_query(
    Type const & rType, class_data * cd, void * that )
    SAL_THROW( (RuntimeException) )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    void * p;
    // shortcut for XInterface
    if (isXInterface( pTDR->pTypeName ))
    {
        // take first one
        p = makeInterface( cd->m_typeEntries[ 0 ].m_offset, that );
    }
    else
    {
        p = __queryDeepNoXInterface( pTDR, cd, that );
        if (! p)
        {
            return Any();
        }
    }
    return Any( &p, pTDR );
}
//==================================================================================================
Any SAL_CALL ImplHelper_queryNoXInterface(
    Type const & rType, class_data * cd, void * that )
    SAL_THROW( (RuntimeException) )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    void * p = __queryDeepNoXInterface( pTDR, cd, that );
    if (p)
    {
        return Any( &p, pTDR );
    }
    else
    {
        return Any();
    }
}
//==================================================================================================
Sequence< sal_Int8 > SAL_CALL ImplHelper_getImplementationId( class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    if (! cd->m_createdId)
    {
        sal_uInt8 * id = (sal_uInt8 *)alloca( 16 );
        ::rtl_createUuid( (sal_uInt8 *)id, 0, sal_True );

        MutexGuard guard( getImplHelperInitMutex() );
        if (! cd->m_createdId)
        {
            memcpy( cd->m_id, id, 16 );
            cd->m_createdId = sal_True;
        }
    }

    sal_Sequence * seq = 0;
    ::rtl_byte_sequence_constructFromArray( &seq, cd->m_id, 16 );
    return Sequence< sal_Int8 >( seq, SAL_NO_ACQUIRE );
}
//==================================================================================================
Sequence< Type > SAL_CALL ImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    Sequence< Type > types( cd->m_nTypes );
    Type * pTypes = types.getArray();
    __fillTypes( pTypes, cd );
    return types;
}
//==================================================================================================
Sequence< Type >  SAL_CALL ImplInhHelper_getTypes(
    class_data * cd, Sequence< Type > const & rAddTypes )
    SAL_THROW( (RuntimeException) )
{
    sal_Int32 nImplTypes = cd->m_nTypes;
    sal_Int32 nAddTypes = rAddTypes.getLength();
    Sequence< Type > types( nImplTypes + nAddTypes );
    Type * pTypes = types.getArray();
    __fillTypes( pTypes, cd );
    // append base types
    Type const * pAddTypes = rAddTypes.getConstArray();
    while (nAddTypes--)
    {
        pTypes[ nImplTypes + nAddTypes ] = pAddTypes[ nAddTypes ];
    }
    return types;
}

// WeakImplHelper
//==================================================================================================
Any SAL_CALL WeakImplHelper_query(
    Type const & rType, class_data * cd, void * that, OWeakObject * pBase )
    SAL_THROW( (RuntimeException) )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    // shortcut XInterface to OWeakObject
    if (! isXInterface( pTDR->pTypeName ))
    {
        void * p = __queryDeepNoXInterface( pTDR, cd, that );
        if (p)
        {
            return Any( &p, pTDR );
        }
    }
    return pBase->OWeakObject::queryInterface( rType );
}
//==================================================================================================
Sequence< Type > SAL_CALL WeakImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    sal_Int32 nTypes = cd->m_nTypes;
    Sequence< Type > types( nTypes +1 );
    Type * pTypes = types.getArray();
    __fillTypes( pTypes, cd );
    pTypes[ nTypes ] = ::getCppuType( (Reference< XWeak > const *)0 );
    return types;
}

// WeakAggImplHelper
//==================================================================================================
Any SAL_CALL WeakAggImplHelper_queryAgg(
    Type const & rType, class_data * cd, void * that, OWeakAggObject * pBase )
    SAL_THROW( (RuntimeException) )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    // shortcut XInterface to OWeakAggObject
    if (! isXInterface( pTDR->pTypeName ))
    {
        void * p = __queryDeepNoXInterface( pTDR, cd, that );
        if (p)
        {
            return Any( &p, pTDR );
        }
    }
    return pBase->OWeakAggObject::queryAggregation( rType );
}
//==================================================================================================
Sequence< Type > SAL_CALL WeakAggImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    sal_Int32 nTypes = cd->m_nTypes;
    Sequence< Type > types( nTypes +2 );
    Type * pTypes = types.getArray();
    __fillTypes( pTypes, cd );
    pTypes[ nTypes++ ] = ::getCppuType( (Reference< XWeak > const *)0 );
    pTypes[ nTypes ] = ::getCppuType( (const Reference< XAggregation > *)0 );
    return types;
}

// WeakComponentImplHelper
//==================================================================================================
Any SAL_CALL WeakComponentImplHelper_query(
    Type const & rType, class_data * cd, void * that, WeakComponentImplHelperBase * pBase )
    SAL_THROW( (RuntimeException) )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    // shortcut XInterface to WeakComponentImplHelperBase
    if (! isXInterface( pTDR->pTypeName ))
    {
        void * p = __queryDeepNoXInterface( pTDR, cd, that );
        if (p)
        {
            return Any( &p, pTDR );
        }
    }
    return pBase->WeakComponentImplHelperBase::queryInterface( rType );
}
//==================================================================================================
Sequence< Type > SAL_CALL WeakComponentImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    sal_Int32 nTypes = cd->m_nTypes;
    Sequence< Type > types( nTypes +2 );
    Type * pTypes = types.getArray();
    __fillTypes( pTypes, cd );
    pTypes[ nTypes++ ] = ::getCppuType( (Reference< XWeak > const *)0 );
    pTypes[ nTypes ] = ::getCppuType( (Reference< lang::XComponent > const *)0 );
    return types;
}

// WeakAggComponentImplHelper
//==================================================================================================
Any SAL_CALL WeakAggComponentImplHelper_queryAgg(
    Type const & rType, class_data * cd, void * that, WeakAggComponentImplHelperBase * pBase )
    SAL_THROW( (RuntimeException) )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    // shortcut XInterface to WeakAggComponentImplHelperBase
    if (! isXInterface( pTDR->pTypeName ))
    {
        void * p = __queryDeepNoXInterface( pTDR, cd, that );
        if (p)
        {
            return Any( &p, pTDR );
        }
    }
    return pBase->WeakAggComponentImplHelperBase::queryAggregation( rType );
}
//==================================================================================================
Sequence< Type > SAL_CALL WeakAggComponentImplHelper_getTypes(
    class_data * cd )
    SAL_THROW( (RuntimeException) )
{
    sal_Int32 nTypes = cd->m_nTypes;
    Sequence< Type > types( nTypes +3 );
    Type * pTypes = types.getArray();
    __fillTypes( pTypes, cd );
    pTypes[ nTypes++ ] = ::getCppuType( (Reference< XWeak > const *)0 );
    pTypes[ nTypes++ ] = ::getCppuType( (const Reference< XAggregation > *)0 );
    pTypes[ nTypes ] = ::getCppuType( (const Reference< lang::XComponent > *)0 );
    return types;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
