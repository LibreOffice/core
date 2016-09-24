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

#include "base.hxx"

using namespace css::lang;
using namespace css::reflection;
using namespace css::uno;

namespace stoc_corefl
{

// XInterface

Any ArrayIdlClassImpl::queryInterface( const Type & rType )
    throw(css::uno::RuntimeException, std::exception)
{
    Any aRet( ::cppu::queryInterface( rType, static_cast< XIdlArray * >( this ) ) );
    return (aRet.hasValue() ? aRet : IdlClassImpl::queryInterface( rType ));
}

void ArrayIdlClassImpl::acquire() throw()
{
    IdlClassImpl::acquire();
}

void ArrayIdlClassImpl::release() throw()
{
    IdlClassImpl::release();
}

// XTypeProvider

Sequence< Type > ArrayIdlClassImpl::getTypes()
    throw (css::uno::RuntimeException, std::exception)
{
    static ::cppu::OTypeCollection * s_pTypes = nullptr;
    if (! s_pTypes)
    {
        ::osl::MutexGuard aGuard( getMutexAccess() );
        if (! s_pTypes)
        {
            static ::cppu::OTypeCollection s_aTypes(
                cppu::UnoType<XIdlArray>::get(),
                IdlClassImpl::getTypes() );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}

Sequence< sal_Int8 > ArrayIdlClassImpl::getImplementationId()
    throw (css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XIdlArray

void ArrayIdlClassImpl::realloc( Any & rArray, sal_Int32 nLen )
    throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE)
    {
        throw IllegalArgumentException(
            "no sequence given!",
            static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 0 );
    }
    if (nLen < 0)
    {
        throw IllegalArgumentException(
            "illegal length given!",
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
    throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE)
    {
        throw IllegalArgumentException(
            "no sequence given!",
            static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 0 );
    }

    return (*static_cast<uno_Sequence * const *>(rArray.getValue()))->nElements;
}

Any ArrayIdlClassImpl::get( const Any & rArray, sal_Int32 nIndex )
    throw(css::lang::IllegalArgumentException, css::lang::ArrayIndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE)
    {
        throw IllegalArgumentException(
            "no sequence given!",
            static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 0 );
    }

    uno_Sequence * pSeq = *static_cast<uno_Sequence * const *>(rArray.getValue());
    if (pSeq->nElements <= nIndex)
    {
        throw ArrayIndexOutOfBoundsException(
            "illegal index given!",
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
    throw(css::lang::IllegalArgumentException, css::lang::ArrayIndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE)
    {
        throw IllegalArgumentException(
            "no sequence given!",
            static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 0 );
    }

    uno_Sequence * pSeq = *static_cast<uno_Sequence * const *>(rArray.getValue());
    if (pSeq->nElements <= nIndex)
    {
        throw ArrayIndexOutOfBoundsException(
            "illegal index given!",
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
    throw(css::uno::RuntimeException, std::exception)
{
    return (xType.is() &&
            (equals( xType ) ||
             (xType->getTypeClass() == getTypeClass() && // must be sequence|array
              getComponentType()->isAssignableFrom( xType->getComponentType() ))));
}

Reference< XIdlClass > ArrayIdlClassImpl::getComponentType()
    throw(css::uno::RuntimeException, std::exception)
{
    return getReflection()->forType( getTypeDescr()->pType );
}

Reference< XIdlArray > ArrayIdlClassImpl::getArray()
    throw(css::uno::RuntimeException, std::exception)
{
    return this;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
