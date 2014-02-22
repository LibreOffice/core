/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "base.hxx"

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

    
    virtual Any SAL_CALL queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    
    virtual Sequence< Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    
    virtual Reference< XIdlClass > SAL_CALL getDeclaringClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    
    virtual Reference< XIdlClass > SAL_CALL getType() throw(::com::sun::star::uno::RuntimeException);
    virtual FieldAccessMode SAL_CALL getAccessMode() throw(::com::sun::star::uno::RuntimeException);
    virtual Any SAL_CALL get( const Any & rObj ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL set( const Any & rObj, const Any & rValue ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
    
    virtual void SAL_CALL set( Any & rObj, const Any & rValue ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException);
};

IdlEnumFieldImpl::~IdlEnumFieldImpl()
{
}



Any IdlEnumFieldImpl::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
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



Sequence< Type > IdlEnumFieldImpl::getTypes()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OTypeCollection * s_pTypes = 0;
    if (! s_pTypes)
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! s_pTypes)
        {
            static OTypeCollection s_aTypes(
                ::getCppuType( (const Reference< XIdlField2 > *)0 ),
                ::getCppuType( (const Reference< XIdlField > *)0 ),
                IdlMemberImpl::getTypes() );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}

Sequence< sal_Int8 > IdlEnumFieldImpl::getImplementationId()
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



Reference< XIdlClass > IdlEnumFieldImpl::getDeclaringClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return IdlMemberImpl::getDeclaringClass();
}

OUString IdlEnumFieldImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return IdlMemberImpl::getName();
}



Reference< XIdlClass > IdlEnumFieldImpl::getType()
    throw(::com::sun::star::uno::RuntimeException)
{
    return getDeclaringClass();
}

FieldAccessMode IdlEnumFieldImpl::getAccessMode()
    throw(::com::sun::star::uno::RuntimeException)
{
    return FieldAccessMode_READONLY;
}

Any IdlEnumFieldImpl::get( const Any & )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    return Any( &_nValue, getTypeDescr() );
}

void IdlEnumFieldImpl::set( const Any &, const Any & )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException)
{
    throw IllegalAccessException(
        OUString("enum field is constant!"),
        (XWeak *)(OWeakObject *)this );
}

void IdlEnumFieldImpl::set( Any &, const Any & )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException)
{
    throw IllegalAccessException(
        OUString("enum field is constant!"),
        (XWeak *)(OWeakObject *)this );
}







EnumIdlClassImpl::~EnumIdlClassImpl()
{
    delete _pFields;
}



Reference< XIdlField > EnumIdlClassImpl::getField( const OUString & rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pFields)
        getFields(); 

    const OUString2Field::const_iterator iFind( _aName2Field.find( rName ) );
    if (iFind != _aName2Field.end())
        return (*iFind).second;
    else
        return Reference< XIdlField >();
}

Sequence< Reference< XIdlField > > EnumIdlClassImpl::getFields()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _pFields)
    {
        MutexGuard aGuard( getMutexAccess() );
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
    throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 eVal =
        ((typelib_EnumTypeDescription *)IdlClassImpl::getTypeDescr())->nDefaultEnumValue;
    rObj.setValue( &eVal, IdlClassImpl::getTypeDescr() );
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
