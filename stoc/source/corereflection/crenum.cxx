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

using namespace css::lang;
using namespace css::reflection;
using namespace css::uno;

namespace stoc_corefl
{


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
    virtual ~IdlEnumFieldImpl();

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

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

IdlEnumFieldImpl::~IdlEnumFieldImpl()
{
}

// XInterface

Any IdlEnumFieldImpl::queryInterface( const Type & rType )
    throw(css::uno::RuntimeException, std::exception)
{
    Any aRet( ::cppu::queryInterface( rType,
                                      static_cast< XIdlField * >( this ),
                                      static_cast< XIdlField2 * >( this ) ) );
    return (aRet.hasValue() ? aRet : IdlMemberImpl::queryInterface( rType ));
}

void IdlEnumFieldImpl::acquire() throw()
{
    IdlMemberImpl::acquire();
}

void IdlEnumFieldImpl::release() throw()
{
    IdlMemberImpl::release();
}

// XTypeProvider

Sequence< Type > IdlEnumFieldImpl::getTypes()
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

Sequence< sal_Int8 > IdlEnumFieldImpl::getImplementationId()
    throw (css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XIdlMember

Reference< XIdlClass > IdlEnumFieldImpl::getDeclaringClass()
    throw(css::uno::RuntimeException, std::exception)
{
    return IdlMemberImpl::getDeclaringClass();
}

OUString IdlEnumFieldImpl::getName()
    throw(css::uno::RuntimeException, std::exception)
{
    return IdlMemberImpl::getName();
}

// XIdlField

Reference< XIdlClass > IdlEnumFieldImpl::getType()
    throw(css::uno::RuntimeException, std::exception)
{
    return getDeclaringClass();
}

FieldAccessMode IdlEnumFieldImpl::getAccessMode()
    throw(css::uno::RuntimeException, std::exception)
{
    return FieldAccessMode_READONLY;
}

Any IdlEnumFieldImpl::get( const Any & )
    throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    return Any( &_nValue, getTypeDescr() );
}

void IdlEnumFieldImpl::set( const Any &, const Any & )
    throw(css::lang::IllegalArgumentException, css::lang::IllegalAccessException, css::uno::RuntimeException, std::exception)
{
    throw IllegalAccessException(
        "enum field is constant!",
        static_cast<XWeak *>(static_cast<OWeakObject *>(this)) );
}

void IdlEnumFieldImpl::set( Any &, const Any & )
    throw(css::lang::IllegalArgumentException, css::lang::IllegalAccessException, css::uno::RuntimeException, std::exception)
{
    throw IllegalAccessException(
        "enum field is constant!",
        static_cast<XWeak *>(static_cast<OWeakObject *>(this)) );
}







EnumIdlClassImpl::~EnumIdlClassImpl()
{
    delete _pFields;
}

// IdlClassImpl modifications

Reference< XIdlField > EnumIdlClassImpl::getField( const OUString & rName )
    throw(css::uno::RuntimeException, std::exception)
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
    throw(css::uno::RuntimeException, std::exception)
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

            _pFields = pFields;
        }
    }
    return *_pFields;
}

void EnumIdlClassImpl::createObject( Any & rObj )
    throw(css::uno::RuntimeException, std::exception)
{
    sal_Int32 eVal =
        reinterpret_cast<typelib_EnumTypeDescription *>(IdlClassImpl::getTypeDescr())->nDefaultEnumValue;
    rObj.setValue( &eVal, IdlClassImpl::getTypeDescr() );
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
