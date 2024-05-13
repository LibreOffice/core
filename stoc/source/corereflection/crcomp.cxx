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

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <com/sun/star/reflection/XIdlField.hpp>
#include <com/sun/star/reflection/XIdlField2.hpp>
#include <com/sun/star/uno/TypeClass.hpp>

#include "base.hxx"

using namespace css::lang;
using namespace css::reflection;
using namespace css::uno;

namespace stoc_corefl
{

namespace {

typedef cppu::ImplInheritanceHelper<IdlMemberImpl, XIdlField, XIdlField2> IdlCompFieldImpl_Base;
class IdlCompFieldImpl : public IdlCompFieldImpl_Base
{
    sal_Int32                   _nOffset;

public:
    IdlCompFieldImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                      typelib_TypeDescription * pTypeDescr, typelib_TypeDescription * pDeclTypeDescr,
                      sal_Int32 nOffset )
        : IdlCompFieldImpl_Base( pReflection, rName, pTypeDescr, pDeclTypeDescr )
        , _nOffset( nOffset )
        {}

    // XIdlMember
    virtual Reference< XIdlClass > SAL_CALL getDeclaringClass() override;
    virtual OUString SAL_CALL getName() override;
    // XIdlField
    virtual Reference< XIdlClass > SAL_CALL getType() override;
    virtual FieldAccessMode SAL_CALL getAccessMode() override;
    virtual Any SAL_CALL get( const Any & rObj ) override;
    virtual void SAL_CALL set( const Any & rObj, const Any & rValue ) override;
    // XIdlField2: getType, getAccessMode and get are equal to XIdlField
    virtual void SAL_CALL set( Any & rObj, const Any & rValue ) override;
};

}

// XIdlMember

Reference< XIdlClass > IdlCompFieldImpl::getDeclaringClass()
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
{
    return IdlMemberImpl::getName();
}

// XIdlField

Reference< XIdlClass > IdlCompFieldImpl::getType()
{
    return getReflection()->forType( getTypeDescr() );
}

FieldAccessMode IdlCompFieldImpl::getAccessMode()
{
    return FieldAccessMode_READWRITE;
}

Any IdlCompFieldImpl::get( const Any & rObj )
{
    if (rObj.getValueTypeClass() == css::uno::TypeClass_STRUCT ||
        rObj.getValueTypeClass() == css::uno::TypeClass_EXCEPTION)
    {
        typelib_TypeDescription * pObjTD = nullptr;
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
        "expected struct or exception, got " + rObj.getValueType().getTypeName(),
        getXWeak(), 0 );
}

void IdlCompFieldImpl::set( const Any & rObj, const Any & rValue )
{
    if (rObj.getValueTypeClass() == css::uno::TypeClass_STRUCT ||
        rObj.getValueTypeClass() == css::uno::TypeClass_EXCEPTION)
    {
        typelib_TypeDescription * pObjTD = nullptr;
        TYPELIB_DANGER_GET( &pObjTD, rObj.getValueTypeRef() );

        typelib_TypeDescription * pTD = pObjTD;
        typelib_TypeDescription * pDeclTD = getDeclTypeDescr();
        while (pTD && !typelib_typedescription_equals( pTD, pDeclTD ))
            pTD = &reinterpret_cast<typelib_CompoundTypeDescription *>(pTD)->pBaseTypeDescription->aBase;

        OSL_ENSURE( pTD, "### illegal object type!" );
        if (pTD)
        {
            TYPELIB_DANGER_RELEASE( pObjTD );
            if (!coerce_assign( const_cast<char *>(static_cast<char const *>(rObj.getValue()) + _nOffset), getTypeDescr(), rValue, getReflection() ))
            {
                throw IllegalArgumentException(
                    u"cannot assign value to destination"_ustr,
                    getXWeak(), 1 );
            }
            return;
        }
        TYPELIB_DANGER_RELEASE( pObjTD );
    }
    throw IllegalArgumentException(
        "expected struct or exception, got " + rObj.getValueType().getTypeName(),
        getXWeak(), 0 );
}


void IdlCompFieldImpl::set( Any & rObj, const Any & rValue )
{
    if (rObj.getValueTypeClass() == css::uno::TypeClass_STRUCT ||
        rObj.getValueTypeClass() == css::uno::TypeClass_EXCEPTION)
    {
        typelib_TypeDescription * pObjTD = nullptr;
        TYPELIB_DANGER_GET( &pObjTD, rObj.getValueTypeRef() );

        typelib_TypeDescription * pTD = pObjTD;
        typelib_TypeDescription * pDeclTD = getDeclTypeDescr();
        while (pTD && !typelib_typedescription_equals( pTD, pDeclTD ))
            pTD = &reinterpret_cast<typelib_CompoundTypeDescription *>(pTD)->pBaseTypeDescription->aBase;

        OSL_ENSURE( pTD, "### illegal object type!" );
        if (pTD)
        {
            TYPELIB_DANGER_RELEASE( pObjTD );
            if (!coerce_assign( const_cast<char *>(static_cast<char const *>(rObj.getValue()) + _nOffset), getTypeDescr(), rValue, getReflection() ))
            {
                throw IllegalArgumentException(
                    u"cannot assign to destination"_ustr,
                    getXWeak(), 1 );
            }
            return;
        }
        TYPELIB_DANGER_RELEASE( pObjTD );
    }
    throw IllegalArgumentException(
        "expected struct or exception, got " + rObj.getValueType().getTypeName(),
        getXWeak(), 0 );
}


CompoundIdlClassImpl::~CompoundIdlClassImpl()
{
}


sal_Bool CompoundIdlClassImpl::isAssignableFrom( const Reference< XIdlClass > & xType )
{
    if (xType.is())
    {
        TypeClass eTC = xType->getTypeClass();
        if (eTC == TypeClass_STRUCT || eTC == TypeClass_EXCEPTION)
        {
            if (equals( xType ))
                return true;
            else
            {
                const Sequence< Reference< XIdlClass > > & rSeq = xType->getSuperclasses();
                if (rSeq.hasElements())
                {
                    OSL_ENSURE( rSeq.getLength() == 1, "### unexpected len of super classes!" );
                    return isAssignableFrom( rSeq[0] );
                }
            }
        }
    }
    return false;
}

Sequence< Reference< XIdlClass > > CompoundIdlClassImpl::getSuperclasses()
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
{
    if (! m_xFields)
        getFields(); // init fields

    const OUString2Field::const_iterator iFind( _aName2Field.find( rName ) );
    if (iFind != _aName2Field.end())
        return Reference< XIdlField >( (*iFind).second );
    else
        return Reference< XIdlField >();
}

Sequence< Reference< XIdlField > > CompoundIdlClassImpl::getFields()
{
    ::osl::MutexGuard aGuard( getMutexAccess() );
    if (! m_xFields)
    {
        sal_Int32 nAll = 0;
        typelib_CompoundTypeDescription * pCompTypeDescr = getTypeDescr();
        for ( ; pCompTypeDescr; pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
            nAll += pCompTypeDescr->nMembers;

        Sequence< Reference< XIdlField > > aFields( nAll );
        Reference< XIdlField > * pSeq = aFields.getArray();

        for ( pCompTypeDescr = getTypeDescr(); pCompTypeDescr;
              pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
        {
            typelib_TypeDescriptionReference ** ppTypeRefs = pCompTypeDescr->ppTypeRefs;
            rtl_uString ** ppNames                         = pCompTypeDescr->ppMemberNames;
            sal_Int32 * pMemberOffsets                     = pCompTypeDescr->pMemberOffsets;

            for ( sal_Int32 nPos = pCompTypeDescr->nMembers; nPos--; )
            {
                typelib_TypeDescription * pTD = nullptr;
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

        m_xFields = std::move( aFields );
    }
    return *m_xFields;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
