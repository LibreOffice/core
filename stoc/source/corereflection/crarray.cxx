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
//__________________________________________________________________________________________________
Any ArrayIdlClassImpl::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet( ::cppu::queryInterface( rType, static_cast< XIdlArray * >( this ) ) );
    return (aRet.hasValue() ? aRet : IdlClassImpl::queryInterface( rType ));
}
//__________________________________________________________________________________________________
void ArrayIdlClassImpl::acquire() throw()
{
    IdlClassImpl::acquire();
}
//__________________________________________________________________________________________________
void ArrayIdlClassImpl::release() throw()
{
    IdlClassImpl::release();
}

// XTypeProvider
//__________________________________________________________________________________________________
Sequence< Type > ArrayIdlClassImpl::getTypes()
    throw (::com::sun::star::uno::RuntimeException)
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
//__________________________________________________________________________________________________
Sequence< sal_Int8 > ArrayIdlClassImpl::getImplementationId()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OImplementationId * s_pId = 0;
    if (! s_pId)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! s_pId)
        {
            static OImplementationId s_aId;
            s_pId = &s_aId;
        }
    }
    return s_pId->getImplementationId();
}

// XIdlArray
//__________________________________________________________________________________________________
void ArrayIdlClassImpl::realloc( Any & rArray, sal_Int32 nLen )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE && eTC != TypeClass_ARRAY)
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
//__________________________________________________________________________________________________
sal_Int32 ArrayIdlClassImpl::getLen( const Any & rArray )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE && eTC != TypeClass_ARRAY)
    {
        throw IllegalArgumentException(
            OUString("no sequence given!"),
            (XWeak *)(OWeakObject *)this, 0 );
    }

    return (*(uno_Sequence **)rArray.getValue())->nElements;
}
//__________________________________________________________________________________________________
Any ArrayIdlClassImpl::get( const Any & rArray, sal_Int32 nIndex )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::ArrayIndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE && eTC != TypeClass_ARRAY)
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

//__________________________________________________________________________________________________
void ArrayIdlClassImpl::set( Any & rArray, sal_Int32 nIndex, const Any & rNewValue )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::ArrayIndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    TypeClass eTC = rArray.getValueTypeClass();
    if (eTC != TypeClass_SEQUENCE && eTC != TypeClass_ARRAY)
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
//__________________________________________________________________________________________________
sal_Bool ArrayIdlClassImpl::isAssignableFrom( const Reference< XIdlClass > & xType )
    throw(::com::sun::star::uno::RuntimeException)
{
    return (xType.is() &&
            (equals( xType ) ||
             (xType->getTypeClass() == getTypeClass() && // must be sequence|array
              getComponentType()->isAssignableFrom( xType->getComponentType() ))));
}
//__________________________________________________________________________________________________
Reference< XIdlClass > ArrayIdlClassImpl::getComponentType()
    throw(::com::sun::star::uno::RuntimeException)
{
    return getReflection()->forType( getTypeDescr()->pType );
}
//__________________________________________________________________________________________________
Reference< XIdlArray > ArrayIdlClassImpl::getArray()
    throw(::com::sun::star::uno::RuntimeException)
{
    return this;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
