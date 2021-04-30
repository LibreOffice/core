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

#include "base.hxx"

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <com/sun/star/reflection/XIdlField2.hpp>

using namespace css::lang;
using namespace css::reflection;
using namespace css::uno;

namespace stoc_corefl
{

namespace {

class IdlEnumFieldImpl
    : public IdlMemberImpl
    , public XIdlField
    , public XIdlField2
{
    sal_Int32               _nValue;

public:
    IdlEnumFieldImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                      typelib_TypeDescription * pTypeDescr, sal_Int32 nValue )
        : IdlMemberImpl( pReflection, rName, pTypeDescr, pTypeDescr )
        , _nValue( nValue )
        {}

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() override;
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

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

// XInterface

Any IdlEnumFieldImpl::queryInterface( const Type & rType )
{
    Any aRet( ::cppu::queryInterface( rType,
                                      static_cast< XIdlField * >( this ),
                                      static_cast< XIdlField2 * >( this ) ) );
    return (aRet.hasValue() ? aRet : IdlMemberImpl::queryInterface( rType ));
}

void IdlEnumFieldImpl::acquire() noexcept
{
    IdlMemberImpl::acquire();
}

void IdlEnumFieldImpl::release() noexcept
{
    IdlMemberImpl::release();
}

// XTypeProvider

Sequence< Type > IdlEnumFieldImpl::getTypes()
{
    static cppu::OTypeCollection s_aTypes(
        cppu::UnoType<XIdlField2>::get(),
        cppu::UnoType<XIdlField>::get(),
        IdlMemberImpl::getTypes() );

    return s_aTypes.getTypes();
}

Sequence< sal_Int8 > IdlEnumFieldImpl::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XIdlMember

Reference< XIdlClass > IdlEnumFieldImpl::getDeclaringClass()
{
    return IdlMemberImpl::getDeclaringClass();
}

OUString IdlEnumFieldImpl::getName()
{
    return IdlMemberImpl::getName();
}

// XIdlField

Reference< XIdlClass > IdlEnumFieldImpl::getType()
{
    return getDeclaringClass();
}

FieldAccessMode IdlEnumFieldImpl::getAccessMode()
{
    return FieldAccessMode_READONLY;
}

Any IdlEnumFieldImpl::get( const Any & )
{
    return Any( &_nValue, getTypeDescr() );
}

void IdlEnumFieldImpl::set( const Any &, const Any & )
{
    throw IllegalAccessException(
        "cannot set enum field, it is constant",
        static_cast<XWeak *>(static_cast<OWeakObject *>(this)) );
}

void IdlEnumFieldImpl::set( Any &, const Any & )
{
    throw IllegalAccessException(
        "cannot set enum field, it is constant",
        static_cast<XWeak *>(static_cast<OWeakObject *>(this)) );
}


EnumIdlClassImpl::~EnumIdlClassImpl()
{
}

// IdlClassImpl modifications

Reference< XIdlField > EnumIdlClassImpl::getField( const OUString & rName )
{
    if (! _pFields)
        getFields(); // init members

    const OUString2Field::const_iterator iFind( _aName2Field.find( rName ) );
    if (iFind != _aName2Field.end())
        return (*iFind).second;
    else
        return Reference< XIdlField >();
}

Sequence< Reference< XIdlField > > EnumIdlClassImpl::getFields()
{
    if (! _pFields)
    {
        ::osl::MutexGuard aGuard( getMutexAccess() );
        if (! _pFields)
        {
            sal_Int32 nFields = getTypeDescr()->nEnumValues;
            Sequence< Reference< XIdlField > > * pFields =
                new Sequence< Reference< XIdlField > >( nFields );
            Reference< XIdlField > * pSeq = pFields->getArray();

            while (nFields--)
            {
                OUString aName( getTypeDescr()->ppEnumNames[nFields] );
                _aName2Field[aName] = pSeq[nFields] = new IdlEnumFieldImpl(
                    getReflection(), aName, IdlClassImpl::getTypeDescr(), getTypeDescr()->pEnumValues[nFields] );
            }

            _pFields.reset( pFields );
        }
    }
    return *_pFields;
}

void EnumIdlClassImpl::createObject( Any & rObj )
{
    sal_Int32 eVal =
        reinterpret_cast<typelib_EnumTypeDescription *>(IdlClassImpl::getTypeDescr())->nDefaultEnumValue;
    rObj.setValue( &eVal, IdlClassImpl::getTypeDescr() );
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
