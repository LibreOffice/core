/*************************************************************************
 *
 *  $RCSfile: crcomp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 15:32:15 $
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

#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif

#include <com/sun/star/reflection/XIdlField.hpp>

#include "base.hxx"


namespace stoc_corefl
{

//==================================================================================================
class IdlCompFieldImpl
    : public IdlMemberImpl
    , public XIdlField
{
    sal_Int32                   _nOffset;

public:
    IdlCompFieldImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                      typelib_TypeDescription * pTypeDescr, typelib_TypeDescription * pDeclTypeDescr,
                      sal_Int32 nOffset )
        : IdlMemberImpl( pReflection, rName, pTypeDescr, pDeclTypeDescr )
        , _nOffset( nOffset )
        {}

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type & rType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // XIdlMember
    virtual Reference< XIdlClass > SAL_CALL getDeclaringClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    // XIdlField
    virtual Reference< XIdlClass > SAL_CALL getType() throw(::com::sun::star::uno::RuntimeException);
    virtual FieldAccessMode SAL_CALL getAccessMode() throw(::com::sun::star::uno::RuntimeException);
    virtual Any SAL_CALL get( const Any & rObj ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL set( const Any & rObj, const Any & rValue ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
};

// XInterface
//__________________________________________________________________________________________________
Any IdlCompFieldImpl::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet( ::cppu::queryInterface( rType, static_cast< XIdlField * >( this ) ) );
    return (aRet.hasValue() ? aRet : IdlMemberImpl::queryInterface( rType ));
}
//__________________________________________________________________________________________________
void IdlCompFieldImpl::acquire() throw()
{
    IdlMemberImpl::acquire();
}
//__________________________________________________________________________________________________
void IdlCompFieldImpl::release() throw()
{
    IdlMemberImpl::release();
}

// XTypeProvider
//__________________________________________________________________________________________________
Sequence< Type > IdlCompFieldImpl::getTypes()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OTypeCollection * s_pTypes = 0;
    if (! s_pTypes)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! s_pTypes)
        {
            static OTypeCollection s_aTypes(
                ::getCppuType( (const Reference< XIdlField > *)0 ),
                IdlMemberImpl::getTypes() );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}
//__________________________________________________________________________________________________
Sequence< sal_Int8 > IdlCompFieldImpl::getImplementationId()
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

// XIdlMember
//__________________________________________________________________________________________________
Reference< XIdlClass > IdlCompFieldImpl::getDeclaringClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _xDeclClass.is())
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _xDeclClass.is())
        {
            typelib_CompoundTypeDescription * pTD =
                (typelib_CompoundTypeDescription *)getDeclTypeDescr();
            while (pTD)
            {
                typelib_TypeDescriptionReference ** ppTypeRefs = pTD->ppTypeRefs;
                for ( sal_Int32 nPos = pTD->nMembers; nPos--; )
                {
                    if (td_equals( (typelib_TypeDescription *)getTypeDescr(), ppTypeRefs[nPos] ))
                    {
                        _xDeclClass = getReflection()->forType( (typelib_TypeDescription *)pTD );
                        return _xDeclClass;
                    }
                }
                pTD = pTD->pBaseTypeDescription;
            }
        }
    }
    return _xDeclClass;
}
//__________________________________________________________________________________________________
OUString IdlCompFieldImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return IdlMemberImpl::getName();
}

// XIdlField
//__________________________________________________________________________________________________
Reference< XIdlClass > IdlCompFieldImpl::getType()
    throw(::com::sun::star::uno::RuntimeException)
{
    return getReflection()->forType( getTypeDescr() );
}
//__________________________________________________________________________________________________
FieldAccessMode IdlCompFieldImpl::getAccessMode()
    throw(::com::sun::star::uno::RuntimeException)
{
    return FieldAccessMode_READWRITE;
}
//__________________________________________________________________________________________________
Any IdlCompFieldImpl::get( const Any & rObj )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    if (rObj.getValueTypeClass() == typelib_TypeClass_STRUCT ||
        rObj.getValueTypeClass() == typelib_TypeClass_EXCEPTION)
    {
        typelib_TypeDescription * pObjTD = 0;
        TYPELIB_DANGER_GET( &pObjTD, rObj.getValueTypeRef() );

        typelib_TypeDescription * pTD = pObjTD;
        typelib_TypeDescription * pDeclTD = getDeclTypeDescr();
        while (pTD && !typelib_typedescription_equals( pTD, pDeclTD ))
            pTD = (typelib_TypeDescription *)((typelib_CompoundTypeDescription *)pTD)->pBaseTypeDescription;

        OSL_ENSURE( pTD, "### illegal object type!" );
        if (pTD)
        {
            TYPELIB_DANGER_RELEASE( pObjTD );
            Any aRet;
            uno_any_destruct( &aRet, cpp_release );
            uno_any_construct( &aRet, (char *)rObj.getValue() + _nOffset, getTypeDescr(), cpp_acquire );
            return aRet;
        }
        TYPELIB_DANGER_RELEASE( pObjTD );
    }
    throw IllegalArgumentException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("illegal object given!") ),
        (XWeak *)(OWeakObject *)this, 0 );
    return Any(); // dummy
}
//__________________________________________________________________________________________________
void IdlCompFieldImpl::set( const Any & rObj, const Any & rValue )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException)
{
    if (rObj.getValueTypeClass() == typelib_TypeClass_STRUCT ||
        rObj.getValueTypeClass() == typelib_TypeClass_EXCEPTION)
    {
        typelib_TypeDescription * pObjTD = 0;
        TYPELIB_DANGER_GET( &pObjTD, rObj.getValueTypeRef() );

        typelib_TypeDescription * pTD = pObjTD;
        typelib_TypeDescription * pDeclTD = getDeclTypeDescr();
        while (pTD && !typelib_typedescription_equals( pTD, pDeclTD ))
            pTD = (typelib_TypeDescription *)((typelib_CompoundTypeDescription *)pTD)->pBaseTypeDescription;

        OSL_ENSURE( pTD, "### illegal object type!" );
        if (pTD)
        {
            TYPELIB_DANGER_RELEASE( pObjTD );
            if (coerce_assign( (char *)rObj.getValue() + _nOffset, getTypeDescr(), rValue, getReflection() ))
            {
                return;
            }
            else
            {
                throw IllegalArgumentException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("illegal value given!") ),
                    (XWeak *)(OWeakObject *)this, 1 );
            }
        }
        TYPELIB_DANGER_RELEASE( pObjTD );
    }
    throw IllegalArgumentException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("illegal object given!") ),
        (XWeak *)(OWeakObject *)this, 0 );
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//__________________________________________________________________________________________________
CompoundIdlClassImpl::~CompoundIdlClassImpl()
{
    delete _pFields;
}

//__________________________________________________________________________________________________
sal_Bool CompoundIdlClassImpl::isAssignableFrom( const Reference< XIdlClass > & xType )
    throw(::com::sun::star::uno::RuntimeException)
{
    if (xType.is())
    {
        TypeClass eTC = xType->getTypeClass();
        if (eTC == TypeClass_STRUCT || eTC == TypeClass_EXCEPTION)
        {
            if (equals( xType ))
                return sal_True;
            else
            {
                const Sequence< Reference< XIdlClass > > & rSeq = xType->getSuperclasses();
                if (rSeq.getLength())
                {
                    OSL_ENSURE( rSeq.getLength() == 1, "### unexpected len of super classes!" );
                    return isAssignableFrom( rSeq[0] );
                }
            }
        }
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Sequence< Reference< XIdlClass > > CompoundIdlClassImpl::getSuperclasses()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _xSuperClass.is())
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _xSuperClass.is())
        {
            typelib_CompoundTypeDescription * pCompTypeDescr = getTypeDescr()->pBaseTypeDescription;
            if (pCompTypeDescr)
                _xSuperClass = getReflection()->forType( (typelib_TypeDescription *)pCompTypeDescr );
        }
    }
    if (_xSuperClass.is())
        return Sequence< Reference< XIdlClass > >( &_xSuperClass, 1 );
    else
        return Sequence< Reference< XIdlClass > >();
}
//__________________________________________________________________________________________________
Reference< XIdlField > CompoundIdlClassImpl::getField( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pFields)
        getFields(); // init fields

    const OUString2Field::const_iterator iFind( _aName2Field.find( rName ) );
    if (iFind != _aName2Field.end())
        return Reference< XIdlField >( (*iFind).second );
    else
        return Reference< XIdlField >();
}
//__________________________________________________________________________________________________
Sequence< Reference< XIdlField > > CompoundIdlClassImpl::getFields()
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( getMutexAccess() );
    if (! _pFields)
    {
        sal_Int32 nAll = 0;
        typelib_CompoundTypeDescription * pCompTypeDescr = getTypeDescr();
        for ( ; pCompTypeDescr; pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
            nAll += pCompTypeDescr->nMembers;

        Sequence< Reference< XIdlField > > * pFields =
            new Sequence< Reference< XIdlField > >( nAll );
        Reference< XIdlField > * pSeq = pFields->getArray();

        for ( pCompTypeDescr = getTypeDescr(); pCompTypeDescr;
              pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
        {
            typelib_TypeDescriptionReference ** ppTypeRefs = pCompTypeDescr->ppTypeRefs;
            rtl_uString ** ppNames                         = pCompTypeDescr->ppMemberNames;
            sal_Int32 * pMemberOffsets                     = pCompTypeDescr->pMemberOffsets;

            for ( sal_Int32 nPos = pCompTypeDescr->nMembers; nPos--; )
            {
                typelib_TypeDescription * pTD = 0;
                TYPELIB_DANGER_GET( &pTD, ppTypeRefs[nPos] );
                OSL_ENSURE( pTD, "### cannot get field in struct!" );
                if (pTD)
                {
                    OUString aName( ppNames[nPos] );
                    _aName2Field[aName] = pSeq[--nAll] = new IdlCompFieldImpl(
                        getReflection(), aName, pTD, IdlClassImpl::getTypeDescr(), pMemberOffsets[nPos] );
                    TYPELIB_DANGER_RELEASE( pTD );
                }
            }
        }

        _pFields = pFields;
    }
    return *_pFields;
}

}


