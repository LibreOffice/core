/*************************************************************************
 *
 *  $RCSfile: crarray.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include <typelib/typedescription.h>
#endif
#ifndef _UNO_DATA_H_
#include <uno/data.h>
#endif

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
            OUString( RTL_CONSTASCII_USTRINGPARAM("no sequence given!") ),
            (XWeak *)(OWeakObject *)this, 0 );
    }
    if (nLen < 0)
    {
        throw IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal length given!") ),
            (XWeak *)(OWeakObject *)this, 1 );
    }

    uno_Sequence ** ppSeq = (uno_Sequence **)rArray.getValue();
    uno_sequence_realloc( ppSeq, (typelib_TypeDescription *)getTypeDescr(),
                          nLen, cpp_acquire, cpp_release );
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("no sequence given!") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("no sequence given!") ),
            (XWeak *)(OWeakObject *)this, 0 );
    }

    uno_Sequence * pSeq = *(uno_Sequence **)rArray.getValue();
    if (pSeq->nElements <= nIndex)
    {
        throw ArrayIndexOutOfBoundsException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal index given!") ),
            (XWeak *)(OWeakObject *)this );
    }

    Any aRet;
    typelib_TypeDescription * pElemTypeDescr = 0;
    TYPELIB_DANGER_GET( &pElemTypeDescr, getTypeDescr()->pType );
    uno_any_destruct( &aRet, cpp_release );
    uno_any_construct( &aRet, &pSeq->elements[nIndex * pElemTypeDescr->nSize],
                       pElemTypeDescr, cpp_acquire );
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("no sequence given!") ),
            (XWeak *)(OWeakObject *)this, 0 );
    }

    uno_Sequence * pSeq = *(uno_Sequence **)rArray.getValue();
    if (pSeq->nElements <= nIndex)
    {
        throw ArrayIndexOutOfBoundsException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal index given!") ),
            (XWeak *)(OWeakObject *)this );
    }

    uno_Sequence ** ppSeq = (uno_Sequence **)rArray.getValue();
    uno_sequence_reference2One( ppSeq, (typelib_TypeDescription *)getTypeDescr(), cpp_acquire, cpp_release );
    rArray.pData = ppSeq;
    pSeq = *ppSeq;

    typelib_TypeDescription * pElemTypeDescr = 0;
    TYPELIB_DANGER_GET( &pElemTypeDescr, getTypeDescr()->pType );

    if (! coerce_assign( &pSeq->elements[nIndex * pElemTypeDescr->nSize],
                         pElemTypeDescr, rNewValue, getReflection() ))
    {
        TYPELIB_DANGER_RELEASE( pElemTypeDescr );
        throw IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("sequence element is not assignable by given value!") ),
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


