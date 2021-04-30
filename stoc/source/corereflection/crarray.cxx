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

#include <typelib/typedescription.h>
#include <uno/data.h>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

#include "base.hxx"

using namespace css::lang;
using namespace css::reflection;
using namespace css::uno;

namespace stoc_corefl
{

// XInterface

Any ArrayIdlClassImpl::queryInterface( const Type & rType )
{
    Any aRet( ::cppu::queryInterface( rType, static_cast< XIdlArray * >( this ) ) );
    return (aRet.hasValue() ? aRet : IdlClassImpl::queryInterface( rType ));
}

void ArrayIdlClassImpl::acquire() noexcept
{
    IdlClassImpl::acquire();
}

void ArrayIdlClassImpl::release() noexcept
{
    IdlClassImpl::release();
}

// XTypeProvider

Sequence< Type > ArrayIdlClassImpl::getTypes()
{
    static cppu::OTypeCollection s_aTypes(
        cppu::UnoType<XIdlArray>::get(),
        IdlClassImpl::getTypes() );

    return s_aTypes.getTypes();
}

Sequence< sal_Int8 > ArrayIdlClassImpl::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XIdlArray

void ArrayIdlClassImpl::realloc( Any & rArray, sal_Int32 nLen )
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE)
    {
        throw IllegalArgumentException(
            "expected sequence, but found " + rArray.getValueType().getTypeName(),
            static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 0 );
    }
    if (nLen < 0)
    {
        throw IllegalArgumentException(
            "negative length given!",
            static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 1 );
    }

    uno_Sequence ** ppSeq = const_cast<uno_Sequence **>(static_cast<uno_Sequence * const *>(rArray.getValue()));
    uno_sequence_realloc( ppSeq, &getTypeDescr()->aBase,
                          nLen,
                          reinterpret_cast< uno_AcquireFunc >(cpp_acquire),
                          reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    rArray.pData = ppSeq;
}

sal_Int32 ArrayIdlClassImpl::getLen( const Any & rArray )
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE)
    {
        throw IllegalArgumentException(
            "expected sequence, but found " + rArray.getValueType().getTypeName(),
            static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 0 );
    }

    return (*static_cast<uno_Sequence * const *>(rArray.getValue()))->nElements;
}

Any ArrayIdlClassImpl::get( const Any & rArray, sal_Int32 nIndex )
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE)
    {
        throw IllegalArgumentException(
            "expected sequence, but found " + rArray.getValueType().getTypeName(),
            static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 0 );
    }

    uno_Sequence * pSeq = *static_cast<uno_Sequence * const *>(rArray.getValue());
    if (pSeq->nElements <= nIndex)
    {
        throw ArrayIndexOutOfBoundsException(
            "illegal index given, index " + OUString::number(nIndex) + " is < " + OUString::number(pSeq->nElements),
            static_cast<XWeak *>(static_cast<OWeakObject *>(this)) );
    }

    Any aRet;
    typelib_TypeDescription * pElemTypeDescr = nullptr;
    TYPELIB_DANGER_GET( &pElemTypeDescr, getTypeDescr()->pType );
    uno_any_destruct( &aRet, reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    uno_any_construct( &aRet, &pSeq->elements[nIndex * pElemTypeDescr->nSize],
                       pElemTypeDescr,
                       reinterpret_cast< uno_AcquireFunc >(cpp_acquire) );
    TYPELIB_DANGER_RELEASE( pElemTypeDescr );
    return aRet;
}


void ArrayIdlClassImpl::set( Any & rArray, sal_Int32 nIndex, const Any & rNewValue )
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE)
    {
        throw IllegalArgumentException(
            "expected sequence, but found " + rArray.getValueType().getTypeName(),
            static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 0 );
    }

    uno_Sequence * pSeq = *static_cast<uno_Sequence * const *>(rArray.getValue());
    if (pSeq->nElements <= nIndex)
    {
        throw ArrayIndexOutOfBoundsException(
            "illegal index given, index " + OUString::number(nIndex) + " is < " + OUString::number(pSeq->nElements),
            static_cast<XWeak *>(static_cast<OWeakObject *>(this)) );
    }

    uno_Sequence ** ppSeq = const_cast<uno_Sequence **>(static_cast<uno_Sequence * const *>(rArray.getValue()));
    uno_sequence_reference2One(
        ppSeq, &getTypeDescr()->aBase,
        reinterpret_cast< uno_AcquireFunc >(cpp_acquire),
        reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    rArray.pData = ppSeq;
    pSeq = *ppSeq;

    typelib_TypeDescription * pElemTypeDescr = nullptr;
    TYPELIB_DANGER_GET( &pElemTypeDescr, getTypeDescr()->pType );

    if (! coerce_assign( &pSeq->elements[nIndex * pElemTypeDescr->nSize],
                         pElemTypeDescr, rNewValue, getReflection() ))
    {
        TYPELIB_DANGER_RELEASE( pElemTypeDescr );
        throw IllegalArgumentException(
            "sequence element is not assignable by given value!",
            static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 2 );
    }
    TYPELIB_DANGER_RELEASE( pElemTypeDescr );
}

// ArrayIdlClassImpl

sal_Bool ArrayIdlClassImpl::isAssignableFrom( const Reference< XIdlClass > & xType )
{
    return (xType.is() &&
            (equals( xType ) ||
             (xType->getTypeClass() == getTypeClass() && // must be sequence|array
              getComponentType()->isAssignableFrom( xType->getComponentType() ))));
}

Reference< XIdlClass > ArrayIdlClassImpl::getComponentType()
{
    return getReflection()->forType( getTypeDescr()->pType );
}

Reference< XIdlArray > ArrayIdlClassImpl::getArray()
{
    return this;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
