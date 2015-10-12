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

#include <rtl/strbuf.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <com/sun/star/reflection/XIdlField.hpp>
#include <com/sun/star/reflection/XIdlField2.hpp>
#include <com/sun/star/uno/TypeClass.hpp>

#include "base.hxx"

using namespace css::lang;
using namespace css::reflection;
using namespace css::uno;

namespace stoc_corefl
{


class IdlCompFieldImpl
    : public IdlMemberImpl
    , public XIdlField
    , public XIdlField2
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
    virtual Any SAL_CALL queryInterface( const Type & rType ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() throw (css::uno::RuntimeException, std::exception) override;
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (css::uno::RuntimeException, std::exception) override;

    // XIdlMember
    virtual Reference< XIdlClass > SAL_CALL getDeclaringClass() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    // XIdlField
    virtual Reference< XIdlClass > SAL_CALL getType() throw(css::uno::RuntimeException, std::exception) override;
    virtual FieldAccessMode SAL_CALL getAccessMode() throw(css::uno::RuntimeException, std::exception) override;
    virtual Any SAL_CALL get( const Any & rObj ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL set( const Any & rObj, const Any & rValue ) throw(css::lang::IllegalArgumentException, css::lang::IllegalAccessException, css::uno::RuntimeException, std::exception) override;
    // XIdlField2: getType, getAccessMode and get are equal to XIdlField
    virtual void SAL_CALL set( Any & rObj, const Any & rValue ) throw(css::lang::IllegalArgumentException, css::lang::IllegalAccessException, css::uno::RuntimeException, std::exception) override;
};

// XInterface

Any IdlCompFieldImpl::queryInterface( const Type & rType )
    throw(css::uno::RuntimeException, std::exception)
{
    Any aRet( ::cppu::queryInterface( rType,
                                      static_cast< XIdlField * >( this ),
                                      static_cast< XIdlField2 * >( this ) ) );
    return (aRet.hasValue() ? aRet : IdlMemberImpl::queryInterface( rType ));
}

void IdlCompFieldImpl::acquire() throw()
{
    IdlMemberImpl::acquire();
}

void IdlCompFieldImpl::release() throw()
{
    IdlMemberImpl::release();
}

// XTypeProvider

Sequence< Type > IdlCompFieldImpl::getTypes()
    throw (css::uno::RuntimeException, std::exception)
{
    static ::cppu::OTypeCollection * s_pTypes = 0;
    if (! s_pTypes)
    {
        ::osl::MutexGuard aGuard( getMutexAccess() );
        if (! s_pTypes)
        {
            static ::cppu::OTypeCollection s_aTypes(
                cppu::UnoType<XIdlField2>::get(),
                cppu::UnoType<XIdlField>::get(),
                IdlMemberImpl::getTypes() );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}

Sequence< sal_Int8 > IdlCompFieldImpl::getImplementationId()
    throw (css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XIdlMember

Reference< XIdlClass > IdlCompFieldImpl::getDeclaringClass()
    throw(css::uno::RuntimeException, std::exception)
{
    if (! _xDeclClass.is())
    {
        ::osl::MutexGuard aGuard( getMutexAccess() );
        if (! _xDeclClass.is())
        {
            typelib_CompoundTypeDescription * pTD =
                reinterpret_cast<typelib_CompoundTypeDescription *>(getDeclTypeDescr());
            while (pTD)
            {
                typelib_TypeDescriptionReference ** ppTypeRefs = pTD->ppTypeRefs;
                for ( sal_Int32 nPos = pTD->nMembers; nPos--; )
                {
                    if (td_equals( getTypeDescr(), ppTypeRefs[nPos] ))
                    {
                        _xDeclClass = getReflection()->forType( &pTD->aBase );
                        return _xDeclClass;
                    }
                }
                pTD = pTD->pBaseTypeDescription;
            }
        }
    }
    return _xDeclClass;
}

OUString IdlCompFieldImpl::getName()
    throw(css::uno::RuntimeException, std::exception)
{
    return IdlMemberImpl::getName();
}

// XIdlField

Reference< XIdlClass > IdlCompFieldImpl::getType()
    throw(css::uno::RuntimeException, std::exception)
{
    return getReflection()->forType( getTypeDescr() );
}

FieldAccessMode IdlCompFieldImpl::getAccessMode()
    throw(css::uno::RuntimeException, std::exception)
{
    return FieldAccessMode_READWRITE;
}

Any IdlCompFieldImpl::get( const Any & rObj )
    throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    if (rObj.getValueTypeClass() == css::uno::TypeClass_STRUCT ||
        rObj.getValueTypeClass() == css::uno::TypeClass_EXCEPTION)
    {
        typelib_TypeDescription * pObjTD = 0;
        TYPELIB_DANGER_GET( &pObjTD, rObj.getValueTypeRef() );

        typelib_TypeDescription * pTD = pObjTD;
        typelib_TypeDescription * pDeclTD = getDeclTypeDescr();
        while (pTD && !typelib_typedescription_equals( pTD, pDeclTD ))
            pTD = &reinterpret_cast<typelib_CompoundTypeDescription *>(pTD)->pBaseTypeDescription->aBase;

        OSL_ENSURE( pTD, "### illegal object type!" );
        if (pTD)
        {
            TYPELIB_DANGER_RELEASE( pObjTD );
            Any aRet;
            uno_any_destruct(
                &aRet, reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
            uno_any_construct(
                &aRet, const_cast<char *>(static_cast<char const *>(rObj.getValue()) + _nOffset), getTypeDescr(),
                reinterpret_cast< uno_AcquireFunc >(cpp_acquire) );
            return aRet;
        }
        TYPELIB_DANGER_RELEASE( pObjTD );
    }
    throw IllegalArgumentException(
        "illegal object given!",
        static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 0 );
}

void IdlCompFieldImpl::set( const Any & rObj, const Any & rValue )
    throw(css::lang::IllegalArgumentException, css::lang::IllegalAccessException, css::uno::RuntimeException, std::exception)
{
    if (rObj.getValueTypeClass() == css::uno::TypeClass_STRUCT ||
        rObj.getValueTypeClass() == css::uno::TypeClass_EXCEPTION)
    {
        typelib_TypeDescription * pObjTD = 0;
        TYPELIB_DANGER_GET( &pObjTD, rObj.getValueTypeRef() );

        typelib_TypeDescription * pTD = pObjTD;
        typelib_TypeDescription * pDeclTD = getDeclTypeDescr();
        while (pTD && !typelib_typedescription_equals( pTD, pDeclTD ))
            pTD = &reinterpret_cast<typelib_CompoundTypeDescription *>(pTD)->pBaseTypeDescription->aBase;

        OSL_ENSURE( pTD, "### illegal object type!" );
        if (pTD)
        {
            TYPELIB_DANGER_RELEASE( pObjTD );
            if (coerce_assign( const_cast<char *>(static_cast<char const *>(rObj.getValue()) + _nOffset), getTypeDescr(), rValue, getReflection() ))
            {
                return;
            }
            else
            {
                throw IllegalArgumentException(
                    "illegal value given!",
                    static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 1 );
            }
        }
        TYPELIB_DANGER_RELEASE( pObjTD );
    }
    throw IllegalArgumentException(
        "illegal object given!",
        static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 0 );
}


void IdlCompFieldImpl::set( Any & rObj, const Any & rValue )
    throw(css::lang::IllegalArgumentException, css::lang::IllegalAccessException, css::uno::RuntimeException, std::exception)
{
    if (rObj.getValueTypeClass() == css::uno::TypeClass_STRUCT ||
        rObj.getValueTypeClass() == css::uno::TypeClass_EXCEPTION)
    {
        typelib_TypeDescription * pObjTD = 0;
        TYPELIB_DANGER_GET( &pObjTD, rObj.getValueTypeRef() );

        typelib_TypeDescription * pTD = pObjTD;
        typelib_TypeDescription * pDeclTD = getDeclTypeDescr();
        while (pTD && !typelib_typedescription_equals( pTD, pDeclTD ))
            pTD = &reinterpret_cast<typelib_CompoundTypeDescription *>(pTD)->pBaseTypeDescription->aBase;

        OSL_ENSURE( pTD, "### illegal object type!" );
        if (pTD)
        {
            TYPELIB_DANGER_RELEASE( pObjTD );
            if (coerce_assign( const_cast<char *>(static_cast<char const *>(rObj.getValue()) + _nOffset), getTypeDescr(), rValue, getReflection() ))
            {
                return;
            }
            else
            {
                throw IllegalArgumentException(
                    "illegal value given!",
                    static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 1 );
            }
        }
        TYPELIB_DANGER_RELEASE( pObjTD );
    }
    throw IllegalArgumentException(
        "illegal object given!",
        static_cast<XWeak *>(static_cast<OWeakObject *>(this)), 0 );
}







CompoundIdlClassImpl::~CompoundIdlClassImpl()
{
    delete _pFields;
}


sal_Bool CompoundIdlClassImpl::isAssignableFrom( const Reference< XIdlClass > & xType )
    throw(css::uno::RuntimeException, std::exception)
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

Sequence< Reference< XIdlClass > > CompoundIdlClassImpl::getSuperclasses()
    throw(css::uno::RuntimeException, std::exception)
{
    if (! _xSuperClass.is())
    {
        ::osl::MutexGuard aGuard( getMutexAccess() );
        if (! _xSuperClass.is())
        {
            typelib_CompoundTypeDescription * pCompTypeDescr = getTypeDescr()->pBaseTypeDescription;
            if (pCompTypeDescr)
                _xSuperClass = getReflection()->forType( &pCompTypeDescr->aBase );
        }
    }
    if (_xSuperClass.is())
        return Sequence< Reference< XIdlClass > >( &_xSuperClass, 1 );
    else
        return Sequence< Reference< XIdlClass > >();
}

Reference< XIdlField > CompoundIdlClassImpl::getField( const OUString & rName )
    throw(css::uno::RuntimeException, std::exception)
{
    if (! _pFields)
        getFields(); // init fields

    const OUString2Field::const_iterator iFind( _aName2Field.find( rName ) );
    if (iFind != _aName2Field.end())
        return Reference< XIdlField >( (*iFind).second );
    else
        return Reference< XIdlField >();
}

Sequence< Reference< XIdlField > > CompoundIdlClassImpl::getFields()
    throw(css::uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( getMutexAccess() );
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
