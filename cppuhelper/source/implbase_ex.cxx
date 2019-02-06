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

#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <cppuhelper/compbase_ex.hxx>
#include <cppuhelper/implbase_ex.hxx>

#include <com/sun/star/uno/RuntimeException.hpp>

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{
    class theImplHelperInitMutex : public rtl::Static<Mutex, theImplHelperInitMutex>{};
}

namespace cppu
{

/** Shared mutex for implementation helper initialization.
    Not for public use.
*/
static ::osl::Mutex & getImplHelperInitMutex()
{
    return theImplHelperInitMutex::get();
}


static void checkInterface( Type const & rType )
{
    if (TypeClass_INTERFACE != rType.getTypeClass())
    {
        OUString msg( "querying for interface \"" + rType.getTypeName() + "\": no interface type!" );
        SAL_WARN( "cppuhelper", msg );
        throw RuntimeException( msg );
    }
}

static bool isXInterface( rtl_uString * pStr )
{
    return OUString::unacquired(&pStr) == "com.sun.star.uno.XInterface";
}

static void * makeInterface( sal_IntPtr nOffset, void * that )
{
    return (static_cast<char *>(that) + nOffset);
}

static bool td_equals(
    typelib_TypeDescriptionReference const * pTDR1,
    typelib_TypeDescriptionReference const * pTDR2 )
{
    return ((pTDR1 == pTDR2) ||
            OUString::unacquired(&pTDR1->pTypeName) == OUString::unacquired(&pTDR2->pTypeName));
}

static type_entry * getTypeEntries( class_data * cd )
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
                Type const & rType = (*pEntry->m_type.getCppuType)( nullptr );
                OSL_ENSURE( rType.getTypeClass() == TypeClass_INTERFACE, "### wrong helper init: expected interface!" );
                OSL_ENSURE( ! isXInterface( rType.getTypeLibType()->pTypeName ), "### want to implement XInterface: template argument is XInterface?!?!?!" );
                if (rType.getTypeClass() != TypeClass_INTERFACE)
                {
                    OUString msg( "type \"" + rType.getTypeName() + "\" is no interface type!" );
                    SAL_WARN( "cppuhelper", msg );
                    throw RuntimeException( msg );
                }
                // ref is statically held by getCppuType()
                pEntry->m_type.typeRef = rType.getTypeLibType();
            }
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            cd->m_storedTypeRefs = true;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    return pEntries;
}

static void fillTypes( Type * types, class_data * cd )
{
    type_entry * pEntries = getTypeEntries( cd );
    for ( sal_Int32 n = cd->m_nTypes; n--; )
    {
        types[ n ] = pEntries[ n ].m_type.typeRef;
    }
}

namespace {

bool recursivelyFindType(
    typelib_TypeDescriptionReference const * demandedType,
    typelib_InterfaceTypeDescription const * type, sal_IntPtr * offset)
{
    // This code assumes that the vtables of a multiple-inheritance class (the
    // offset amount by which to adjust the this pointer) follow one another in
    // the object layout, and that they contain slots for the inherited classes
    // in a specific order.  In theory, that need not hold for any given
    // platform; in practice, it seems to work well on all supported platforms:
 next:
    for (sal_Int32 i = 0; i < type->nBaseTypes; ++i) {
        if (i > 0) {
            *offset += sizeof (void *);
        }
        typelib_InterfaceTypeDescription const * base = type->ppBaseTypes[i];
        // ignore XInterface:
        if (base->nBaseTypes > 0) {
            if (td_equals(
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

static void * queryDeepNoXInterface(
    typelib_TypeDescriptionReference const * pDemandedTDR, class_data * cd, void * that )
{
    type_entry * pEntries = getTypeEntries( cd );
    sal_Int32 nTypes = cd->m_nTypes;
    sal_Int32 n;

    // try top interfaces without getting td
    for ( n = 0; n < nTypes; ++n )
    {
        if (td_equals( pEntries[ n ].m_type.typeRef, pDemandedTDR ))
        {
            return makeInterface( pEntries[ n ].m_offset, that );
        }
    }
    // query deep getting td
    for ( n = 0; n < nTypes; ++n )
    {
        typelib_TypeDescription * pTD = nullptr;
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
            OUString msg( "cannot get type description for type \"" + OUString(pEntries[ n ].m_type.typeRef->pTypeName) + "\"!" );
            SAL_WARN( "cppuhelper", msg );
            throw RuntimeException( msg );
        }
    }
    return nullptr;
}

// ImplHelper

Any SAL_CALL ImplHelper_query(
    Type const & rType, class_data * cd, void * that )
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
        p = queryDeepNoXInterface( pTDR, cd, that );
        if (! p)
        {
            return Any();
        }
    }
    return Any( &p, pTDR );
}

Any SAL_CALL ImplHelper_queryNoXInterface(
    Type const & rType, class_data * cd, void * that )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    void * p = queryDeepNoXInterface( pTDR, cd, that );
    if (p)
    {
        return Any( &p, pTDR );
    }
    return Any();
}

css::uno::Sequence<sal_Int8> ImplHelper_getImplementationId(
    SAL_UNUSED_PARAMETER class_data *)
{
    return css::uno::Sequence<sal_Int8>();
}

Sequence< Type > SAL_CALL ImplHelper_getTypes(
    class_data * cd )
{
    Sequence< Type > types( cd->m_nTypes );
    Type * pTypes = types.getArray();
    fillTypes( pTypes, cd );
    return types;
}

Sequence< Type >  SAL_CALL ImplInhHelper_getTypes(
    class_data * cd, Sequence< Type > const & rAddTypes )
{
    sal_Int32 nImplTypes = cd->m_nTypes;
    sal_Int32 nAddTypes = rAddTypes.getLength();
    Sequence< Type > types( nImplTypes + nAddTypes );
    Type * pTypes = types.getArray();
    fillTypes( pTypes, cd );
    // append base types
    Type const * pAddTypes = rAddTypes.getConstArray();
    while (nAddTypes--)
    {
        pTypes[ nImplTypes + nAddTypes ] = pAddTypes[ nAddTypes ];
    }
    return types;
}

// WeakImplHelper

Any SAL_CALL WeakImplHelper_query(
    Type const & rType, class_data * cd, void * that, OWeakObject * pBase )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    // shortcut XInterface to OWeakObject
    if (! isXInterface( pTDR->pTypeName ))
    {
        void * p = queryDeepNoXInterface( pTDR, cd, that );
        if (p)
        {
            return Any( &p, pTDR );
        }
    }
    return pBase->OWeakObject::queryInterface( rType );
}

Sequence< Type > SAL_CALL WeakImplHelper_getTypes(
    class_data * cd )
{
    sal_Int32 nTypes = cd->m_nTypes;
    Sequence< Type > types( nTypes +1 );
    Type * pTypes = types.getArray();
    fillTypes( pTypes, cd );
    pTypes[ nTypes ] = cppu::UnoType<XWeak>::get();
    return types;
}

// WeakAggImplHelper

Any SAL_CALL WeakAggImplHelper_queryAgg(
    Type const & rType, class_data * cd, void * that, OWeakAggObject * pBase )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    // shortcut XInterface to OWeakAggObject
    if (! isXInterface( pTDR->pTypeName ))
    {
        void * p = queryDeepNoXInterface( pTDR, cd, that );
        if (p)
        {
            return Any( &p, pTDR );
        }
    }
    return pBase->OWeakAggObject::queryAggregation( rType );
}

Sequence< Type > SAL_CALL WeakAggImplHelper_getTypes(
    class_data * cd )
{
    sal_Int32 nTypes = cd->m_nTypes;
    Sequence< Type > types( nTypes +2 );
    Type * pTypes = types.getArray();
    fillTypes( pTypes, cd );
    pTypes[ nTypes++ ] = cppu::UnoType<XWeak>::get();
    pTypes[ nTypes ] = cppu::UnoType<XAggregation>::get();
    return types;
}

// WeakComponentImplHelper

Any SAL_CALL WeakComponentImplHelper_query(
    Type const & rType, class_data * cd, void * that, WeakComponentImplHelperBase * pBase )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    // shortcut XInterface to WeakComponentImplHelperBase
    if (! isXInterface( pTDR->pTypeName ))
    {
        void * p = queryDeepNoXInterface( pTDR, cd, that );
        if (p)
        {
            return Any( &p, pTDR );
        }
    }
    return pBase->WeakComponentImplHelperBase::queryInterface( rType );
}

Sequence< Type > SAL_CALL WeakComponentImplHelper_getTypes(
    class_data * cd )
{
    sal_Int32 nTypes = cd->m_nTypes;
    Sequence< Type > types( nTypes +2 );
    Type * pTypes = types.getArray();
    fillTypes( pTypes, cd );
    pTypes[ nTypes++ ] = cppu::UnoType<XWeak>::get();
    pTypes[ nTypes ] = cppu::UnoType<lang::XComponent>::get();
    return types;
}

// WeakAggComponentImplHelper

Any SAL_CALL WeakAggComponentImplHelper_queryAgg(
    Type const & rType, class_data * cd, void * that, WeakAggComponentImplHelperBase * pBase )
{
    checkInterface( rType );
    typelib_TypeDescriptionReference * pTDR = rType.getTypeLibType();

    // shortcut XInterface to WeakAggComponentImplHelperBase
    if (! isXInterface( pTDR->pTypeName ))
    {
        void * p = queryDeepNoXInterface( pTDR, cd, that );
        if (p)
        {
            return Any( &p, pTDR );
        }
    }
    return pBase->WeakAggComponentImplHelperBase::queryAggregation( rType );
}

Sequence< Type > SAL_CALL WeakAggComponentImplHelper_getTypes(
    class_data * cd )
{
    sal_Int32 nTypes = cd->m_nTypes;
    Sequence< Type > types( nTypes +3 );
    Type * pTypes = types.getArray();
    fillTypes( pTypes, cd );
    pTypes[ nTypes++ ] = cppu::UnoType<XWeak>::get();
    pTypes[ nTypes++ ] = cppu::UnoType<XAggregation>::get();
    pTypes[ nTypes ] = cppu::UnoType<lang::XComponent>::get();
    return types;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
