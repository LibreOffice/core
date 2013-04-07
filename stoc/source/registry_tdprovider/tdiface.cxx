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

#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include "registry/reader.hxx"
#include "registry/version.h"

#include <com/sun/star/reflection/XInterfaceMemberTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceAttributeTypeDescription2.hpp>
#include <com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp>
#include <com/sun/star/reflection/XMethodParameter.hpp>
#include <com/sun/star/reflection/XParameter.hpp>
#include "com/sun/star/uno/RuntimeException.hpp"
#include "base.hxx"
#include "functiondescription.hxx"
#include "methoddescription.hxx"

#include <memory>
#include <set>

namespace stoc_rdbtdp
{

//==================================================================================================
class InterfaceMethodImpl : public WeakImplHelper1< XInterfaceMethodTypeDescription >
{
    stoc::registry_tdprovider::MethodDescription _desc;

    Reference< XHierarchicalNameAccess >  _xTDMgr;

    OUString                              _aTypeName;

    OUString                              _aReturnType;
    Reference< XTypeDescription >         _xReturnTD;

    sal_Bool                              _bIsOneWay;
    sal_Int32                             _nPosition;

public:
    InterfaceMethodImpl( const Reference< XHierarchicalNameAccess > & xTDMgr,
                         const OUString & rTypeName,
                         const OUString & rMemberName,
                         const OUString & rReturnType,
                         const Sequence< sal_Int8 > & rBytes,
                         sal_uInt16 nMethodIndex,
                         sal_Bool bIsOneWay,
                         sal_Int32 nPosition )
        : _desc(xTDMgr, rMemberName, rBytes, nMethodIndex)
        , _xTDMgr( xTDMgr )
        , _aTypeName( rTypeName )
        , _aReturnType( rReturnType )
        , _bIsOneWay( bIsOneWay )
        , _nPosition( nPosition )
        {
            g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        }
    virtual ~InterfaceMethodImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XInterfaceMemberTypeDescription
    virtual OUString SAL_CALL getMemberName() throw(::com::sun::star::uno::RuntimeException)
    { return _desc.getName(); }
    virtual sal_Int32 SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException);

    // XInterfaceMethodTypeDescription
    virtual Reference< XTypeDescription > SAL_CALL getReturnType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isOneway() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XMethodParameter > > SAL_CALL getParameters() throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XTypeDescription > > SAL_CALL getExceptions() throw(::com::sun::star::uno::RuntimeException);
};
//__________________________________________________________________________________________________
InterfaceMethodImpl::~InterfaceMethodImpl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass InterfaceMethodImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_INTERFACE_METHOD;
}
//__________________________________________________________________________________________________
OUString InterfaceMethodImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aTypeName;
}

// XInterfaceMemberTypeDescription
//__________________________________________________________________________________________________
sal_Int32 InterfaceMethodImpl::getPosition()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _nPosition;
}

// XInterfaceMethodTypeDescription
//__________________________________________________________________________________________________
Reference<XTypeDescription > InterfaceMethodImpl::getReturnType()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (!_xReturnTD.is() && !_aReturnType.isEmpty())
    {
        try
        {
            Reference< XTypeDescription > xReturnTD;
            if (_xTDMgr->getByHierarchicalName( _aReturnType ) >>= xReturnTD)
            {
                MutexGuard aGuard( getMutex() );
                if (! _xReturnTD.is())
                    _xReturnTD = xReturnTD;
                return _xReturnTD;
            }
        }
        catch (NoSuchElementException &)
        {
        }
        // never try again, if no td was found
        _aReturnType = OUString();
    }
    return _xReturnTD;
}
//__________________________________________________________________________________________________
sal_Bool InterfaceMethodImpl::isOneway()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _bIsOneWay;
}
//__________________________________________________________________________________________________
Sequence<Reference<XMethodParameter > > InterfaceMethodImpl::getParameters()
    throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< Reference< XParameter > > s1(_desc.getParameters());
    Sequence< Reference< XMethodParameter > > s2(s1.getLength());
    for (sal_Int32 i = 0; i < s1.getLength(); ++i) {
        s2[i] = s1[i].get();
    }
    return s2;
}
//__________________________________________________________________________________________________
Sequence<Reference<XTypeDescription > > InterfaceMethodImpl::getExceptions()
    throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< Reference< XCompoundTypeDescription > > s1(
        _desc.getExceptions());
    Sequence< Reference< XTypeDescription > > s2(s1.getLength());
    for (sal_Int32 i = 0; i < s1.getLength(); ++i) {
        s2[i] = s1[i].get();
    }
    return s2;
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//==================================================================================================
class InterfaceAttributeImpl : public WeakImplHelper1< XInterfaceAttributeTypeDescription2 >
{
    Reference< XHierarchicalNameAccess >  _xTDMgr;

    OUString                              _aTypeName;
    OUString                              _aMemberName;

    OUString                              _aMemberTypeName;
    Reference< XTypeDescription >         _xMemberTD;

    sal_Bool                              _bReadOnly;
    sal_Bool                              _bBound;
    sal_Int32                             _nPosition;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< stoc::registry_tdprovider::FunctionDescription > _getter;
    std::auto_ptr< stoc::registry_tdprovider::FunctionDescription > _setter;
    SAL_WNODEPRECATED_DECLARATIONS_POP

public:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    InterfaceAttributeImpl(
        const Reference< XHierarchicalNameAccess > & xTDMgr,
        const OUString & rTypeName,
        const OUString & rMemberName,
        const OUString & rMemberTypeName,
        sal_Bool bReadOnly,
        sal_Bool bBound,
        std::auto_ptr< stoc::registry_tdprovider::FunctionDescription > &
            getter,
        std::auto_ptr< stoc::registry_tdprovider::FunctionDescription > &
            setter,
        sal_Int32 nPosition )
        : _xTDMgr( xTDMgr )
        , _aTypeName( rTypeName )
        , _aMemberName( rMemberName )
        , _aMemberTypeName( rMemberTypeName )
        , _bReadOnly( bReadOnly )
        , _bBound( bBound )
        , _nPosition( nPosition )
        , _getter( getter )
        , _setter( setter )
        {
            g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        }
    SAL_WNODEPRECATED_DECLARATIONS_POP
    virtual ~InterfaceAttributeImpl();

    // XTypeDescription
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);

    // XInterfaceMemberTypeDescription
    virtual OUString SAL_CALL getMemberName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException);

    // XInterfaceAttributeTypeDescription2
    virtual sal_Bool SAL_CALL isReadOnly() throw(::com::sun::star::uno::RuntimeException);
    virtual Reference< XTypeDescription > SAL_CALL getType() throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isBound() throw (RuntimeException)
    { return _bBound; }

    virtual Sequence< Reference< XCompoundTypeDescription > > SAL_CALL
    getGetExceptions() throw (RuntimeException)
    {
        if (_getter.get() != 0) {
            return _getter->getExceptions();
        } else {
            return Sequence< Reference< XCompoundTypeDescription > >();
        }
    }

    virtual Sequence< Reference< XCompoundTypeDescription > > SAL_CALL
    getSetExceptions() throw (RuntimeException)
    {
        if (_setter.get() != 0) {
            return _setter->getExceptions();
        } else {
            return Sequence< Reference< XCompoundTypeDescription > >();
        }
    }
};

InterfaceAttributeImpl::~InterfaceAttributeImpl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}
// XTypeDescription
//__________________________________________________________________________________________________
TypeClass InterfaceAttributeImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_INTERFACE_ATTRIBUTE;
}
//__________________________________________________________________________________________________
OUString InterfaceAttributeImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aTypeName;
}

// XInterfaceMemberTypeDescription
//__________________________________________________________________________________________________
OUString InterfaceAttributeImpl::getMemberName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aMemberName;
}
//__________________________________________________________________________________________________
sal_Int32 InterfaceAttributeImpl::getPosition()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _nPosition;
}

// XInterfaceAttributeTypeDescription2
//__________________________________________________________________________________________________
sal_Bool InterfaceAttributeImpl::isReadOnly()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _bReadOnly;
}
//__________________________________________________________________________________________________
Reference<XTypeDescription > InterfaceAttributeImpl::getType()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (!_xMemberTD.is() && !_aMemberTypeName.isEmpty())
    {
        try
        {
            Reference< XTypeDescription > xMemberTD;
            if (_xTDMgr->getByHierarchicalName( _aMemberTypeName ) >>= xMemberTD)
            {
                MutexGuard aGuard( getMutex() );
                if (! _xMemberTD.is())
                    _xMemberTD = xMemberTD;
                return _xMemberTD;
            }
        }
        catch (NoSuchElementException &)
        {
        }
        // never try again, if no td was found
        _aMemberTypeName = OUString();
    }
    return _xMemberTD;
}


//##################################################################################################
//##################################################################################################
//##################################################################################################

void InterfaceTypeDescriptionImpl::checkInterfaceType(
    Reference< XTypeDescription > const & type)
{
    if (resolveTypedefs(type)->getTypeClass() != TypeClass_INTERFACE) {
        throw RuntimeException(
            OUString(
                    "Interface base is not an interface type"),
            static_cast< OWeakObject * >(this));
    }
}

namespace {

class BaseOffset {
public:
    BaseOffset(Reference< XInterfaceTypeDescription2 > const & desc);

    sal_Int32 get() const { return offset; }

private:
    void calculateBases(Reference< XInterfaceTypeDescription2 > const & desc);

    void calculate(Reference< XInterfaceTypeDescription2 > const & desc);

    std::set< OUString > set;
    sal_Int32 offset;
};

BaseOffset::BaseOffset(Reference< XInterfaceTypeDescription2 > const & desc) {
    offset = 0;
    calculateBases(desc);
}

void BaseOffset::calculateBases(
    Reference< XInterfaceTypeDescription2 > const & desc)
{
    Sequence< Reference < XTypeDescription > > bases(desc->getBaseTypes());
    for (sal_Int32 i = 0; i < bases.getLength(); ++i) {
        calculate(
            Reference< XInterfaceTypeDescription2 >(
                resolveTypedefs(bases[i]), UNO_QUERY_THROW));
    }
}

void BaseOffset::calculate(Reference< XInterfaceTypeDescription2 > const & desc)
{
    if (set.insert(desc->getName()).second) {
        calculateBases(desc);
        offset += desc->getMembers().getLength();
    }
}

}

//__________________________________________________________________________________________________
InterfaceTypeDescriptionImpl::InterfaceTypeDescriptionImpl(
    const Reference< XHierarchicalNameAccess > & xTDMgr,
    const OUString & rName, const Sequence< OUString > & rBaseTypes,
    const Sequence< OUString > & rOptionalBaseTypes,
    const Sequence< sal_Int8 > & rBytes, bool published )
    : _xTDMgr( xTDMgr )
    , _aBytes( rBytes )
    , _aName( rName )
    , _aBaseTypes( rBaseTypes )
    , _aOptionalBaseTypes( rOptionalBaseTypes )
    , _membersInit( false )
    , _published( published )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
InterfaceTypeDescriptionImpl::~InterfaceTypeDescriptionImpl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XTypeDescription
//__________________________________________________________________________________________________
TypeClass InterfaceTypeDescriptionImpl::getTypeClass()
    throw(::com::sun::star::uno::RuntimeException)
{
    return TypeClass_INTERFACE;
}
//__________________________________________________________________________________________________
OUString InterfaceTypeDescriptionImpl::getName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return _aName;
}

// XInterfaceTypeDescription2
//__________________________________________________________________________________________________
Reference< XTypeDescription > InterfaceTypeDescriptionImpl::getBaseType()
    throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< Reference< XTypeDescription > > aBaseTypes(getBaseTypes());
    return aBaseTypes.getLength() >= 1 ? aBaseTypes[0] : 0;
}
//__________________________________________________________________________________________________
Uik SAL_CALL InterfaceTypeDescriptionImpl::getUik()
    throw(::com::sun::star::uno::RuntimeException)
{
    return Uik();
}
//__________________________________________________________________________________________________
Sequence< Reference< XInterfaceMemberTypeDescription > > InterfaceTypeDescriptionImpl::getMembers()
    throw(::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard guard(getMutex());
    if (!_membersInit) {
        _nBaseOffset = BaseOffset(this).get();
        typereg::Reader reader(
            _aBytes.getConstArray(), _aBytes.getLength(), false,
            TYPEREG_VERSION_1);
        sal_Int32 count = 0;
        sal_uInt16 methodCount = reader.getMethodCount();
        for (sal_uInt16 i = 0; i < methodCount; ++i) {
            RTMethodMode flags = reader.getMethodFlags(i);
            if (flags != RT_MODE_ATTRIBUTE_GET
                && flags != RT_MODE_ATTRIBUTE_SET)
            {
                ++count;
            }
        }
        sal_uInt16 fieldCount = reader.getFieldCount();
        count += fieldCount;
        _members.realloc(count);
        sal_Int32 index = 0;
        for (sal_uInt16 i = 0; i < fieldCount; ++i) {
            OUString name(reader.getFieldName(i));
            OUStringBuffer typeName(getName());
            typeName.appendAscii(RTL_CONSTASCII_STRINGPARAM("::"));
            typeName.append(name);
            RTFieldAccess flags = reader.getFieldFlags(i);
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            std::auto_ptr< stoc::registry_tdprovider::FunctionDescription >
                getter;
            std::auto_ptr< stoc::registry_tdprovider::FunctionDescription >
                setter;
            SAL_WNODEPRECATED_DECLARATIONS_POP
            for (sal_uInt16 j = 0; j < methodCount; ++j) {
                if (reader.getMethodName(j) == name) {
                    switch (reader.getMethodFlags(j)) {
                    case RT_MODE_ATTRIBUTE_GET:
                        OSL_ASSERT(getter.get() == 0);
                        getter.reset(
                            new stoc::registry_tdprovider::FunctionDescription(
                                _xTDMgr, _aBytes, j));
                        break;

                    case RT_MODE_ATTRIBUTE_SET:
                        OSL_ASSERT(setter.get() == 0);
                        setter.reset(
                            new stoc::registry_tdprovider::FunctionDescription(
                                _xTDMgr, _aBytes, j));
                        break;

                    default:
                        OSL_ASSERT(false);
                        break;
                    }
                }
            }
            _members[index] = new InterfaceAttributeImpl(
                _xTDMgr, typeName.makeStringAndClear(), name,
                reader.getFieldTypeName(i).replace('/', '.'),
                (flags & RT_ACCESS_READONLY) != 0,
                (flags & RT_ACCESS_BOUND) != 0, getter, setter,
                _nBaseOffset + index);
            ++index;
        }
        for (sal_uInt16 i = 0; i < methodCount; ++i) {
            RTMethodMode flags = reader.getMethodFlags(i);
            if (flags != RT_MODE_ATTRIBUTE_GET
                && flags != RT_MODE_ATTRIBUTE_SET)
            {
                OUString name(reader.getMethodName(i));
                OUStringBuffer typeName(getName());
                typeName.appendAscii(RTL_CONSTASCII_STRINGPARAM("::"));
                typeName.append(name);
                _members[index] = new InterfaceMethodImpl(
                    _xTDMgr, typeName.makeStringAndClear(), name,
                    reader.getMethodReturnTypeName(i).replace('/', '.'),
                    _aBytes, i, flags == RT_MODE_ONEWAY, _nBaseOffset + index);
                ++index;
            }
        }
        _membersInit = true;
    }
    return _members;
}

Sequence< Reference< XTypeDescription > >
InterfaceTypeDescriptionImpl::getBaseTypes() throw (RuntimeException) {
    MutexGuard guard(getMutex());
    if (_xBaseTDs.getLength() == 0 && _aBaseTypes.getLength() != 0) {
        Sequence< Reference< XTypeDescription > > tds(_aBaseTypes.getLength());
        for (sal_Int32 i = 0; i < _aBaseTypes.getLength(); ++i) {
            try {
                _xTDMgr->getByHierarchicalName(_aBaseTypes[i]) >>= tds[i];
            } catch (const NoSuchElementException & e) {
                throw RuntimeException(
                    (OUString(
                            "com.sun.star.container.NoSuchElementException: ")
                     + e.Message),
                    static_cast< OWeakObject * >(this));
            }
            OSL_ASSERT(tds[i].is());
            checkInterfaceType(tds[i]);
        }
        _xBaseTDs = tds;
    }
    return _xBaseTDs;
}

Sequence< Reference< XTypeDescription > >
InterfaceTypeDescriptionImpl::getOptionalBaseTypes() throw (RuntimeException) {
    MutexGuard guard(getMutex());
    if (_xOptionalBaseTDs.getLength() == 0
        && _aOptionalBaseTypes.getLength() != 0)
    {
        Sequence< Reference< XTypeDescription > > tds(
            _aOptionalBaseTypes.getLength());
        for (sal_Int32 i = 0; i < _aOptionalBaseTypes.getLength(); ++i) {
            try {
                _xTDMgr->getByHierarchicalName(_aOptionalBaseTypes[i])
                    >>= tds[i];
            } catch (const NoSuchElementException & e) {
                throw RuntimeException(
                    (OUString(
                            "com.sun.star.container.NoSuchElementException: ")
                     + e.Message),
                    static_cast< OWeakObject * >(this));
            }
            OSL_ASSERT(tds[i].is());
            checkInterfaceType(tds[i]);
        }
        _xOptionalBaseTDs = tds;
    }
    return _xOptionalBaseTDs;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
