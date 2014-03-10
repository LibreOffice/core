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

#include "base.hxx"


namespace stoc_corefl
{

// XInterface

Any ArrayIdlClassImpl::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException, std::exception)
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
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    static OTypeCollection * s_pTypes = 0;
    if (! s_pTypes)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! s_pTypes)
        {
            static OTypeCollection s_aTypes(
                ::getCppuType( (const Reference< XIdlArray > *)0 ),
                IdlClassImpl::getTypes() );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}

Sequence< sal_Int8 > ArrayIdlClassImpl::getImplementationId()
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XIdlArray

void ArrayIdlClassImpl::realloc( Any & rArray, sal_Int32 nLen )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE)
    {
        throw IllegalArgumentException(
            OUString("no sequence given!"),
            (XWeak *)(OWeakObject *)this, 0 );
    }
    if (nLen < 0)
    {
        throw IllegalArgumentException(
            OUString("illegal length given!"),
            (XWeak *)(OWeakObject *)this, 1 );
    }

    uno_Sequence ** ppSeq = (uno_Sequence **)rArray.getValue();
    uno_sequence_realloc( ppSeq, (typelib_TypeDescription *)getTypeDescr(),
                          nLen,
                          reinterpret_cast< uno_AcquireFunc >(cpp_acquire),
                          reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    rArray.pData = ppSeq;
}

sal_Int32 ArrayIdlClassImpl::getLen( const Any & rArray )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE)
    {
        throw IllegalArgumentException(
            OUString("no sequence given!"),
            (XWeak *)(OWeakObject *)this, 0 );
    }

    return (*(uno_Sequence **)rArray.getValue())->nElements;
}

Any ArrayIdlClassImpl::get( const Any & rArray, sal_Int32 nIndex )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::ArrayIndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE)
    {
        throw IllegalArgumentException(
            OUString("no sequence given!"),
            (XWeak *)(OWeakObject *)this, 0 );
    }

    uno_Sequence * pSeq = *(uno_Sequence **)rArray.getValue();
    if (pSeq->nElements <= nIndex)
    {
        throw ArrayIndexOutOfBoundsException(
            OUString("illegal index given!"),
            (XWeak *)(OWeakObject *)this );
    }

    Any aRet;
    typelib_TypeDescription * pElemTypeDescr = 0;
    TYPELIB_DANGER_GET( &pElemTypeDescr, getTypeDescr()->pType );
    uno_any_destruct( &aRet, reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    uno_any_construct( &aRet, &pSeq->elements[nIndex * pElemTypeDescr->nSize],
                       pElemTypeDescr,
                       reinterpret_cast< uno_AcquireFunc >(cpp_acquire) );
    TYPELIB_DANGER_RELEASE( pElemTypeDescr );
    return aRet;
}


void ArrayIdlClassImpl::set( Any & rArray, sal_Int32 nIndex, const Any & rNewValue )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::ArrayIndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE)
    {
        throw IllegalArgumentException(
            OUString("no sequence given!"),
            (XWeak *)(OWeakObject *)this, 0 );
    }

    uno_Sequence * pSeq = *(uno_Sequence **)rArray.getValue();
    if (pSeq->nElements <= nIndex)
    {
        throw ArrayIndexOutOfBoundsException(
            OUString("illegal index given!"),
            (XWeak *)(OWeakObject *)this );
    }

    uno_Sequence ** ppSeq = (uno_Sequence **)rArray.getValue();
    uno_sequence_reference2One(
        ppSeq, (typelib_TypeDescription *)getTypeDescr(),
        reinterpret_cast< uno_AcquireFunc >(cpp_acquire),
        reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    rArray.pData = ppSeq;
    pSeq = *ppSeq;

    typelib_TypeDescription * pElemTypeDescr = 0;
    TYPELIB_DANGER_GET( &pElemTypeDescr, getTypeDescr()->pType );

    if (! coerce_assign( &pSeq->elements[nIndex * pElemTypeDescr->nSize],
                         pElemTypeDescr, rNewValue, getReflection() ))
    {
        TYPELIB_DANGER_RELEASE( pElemTypeDescr );
        throw IllegalArgumentException(
            OUString("sequence element is not assignable by given value!"),
            (XWeak *)(OWeakObject *)this, 2 );
    }
    TYPELIB_DANGER_RELEASE( pElemTypeDescr );
}

// ArrayIdlClassImpl

sal_Bool ArrayIdlClassImpl::isAssignableFrom( const Reference< XIdlClass > & xType )
    throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return (xType.is() &&
            (equals( xType ) ||
             (xType->getTypeClass() == getTypeClass() && // must be sequence|array
              getComponentType()->isAssignableFrom( xType->getComponentType() ))));
}

Reference< XIdlClass > ArrayIdlClassImpl::getComponentType()
    throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return getReflection()->forType( getTypeDescr()->pType );
}

Reference< XIdlArray > ArrayIdlClassImpl::getArray()
    throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return this;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
