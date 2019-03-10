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

#include <comphelper/propertycontainerhelper.hxx>
#include <comphelper/property.hxx>
#include <osl/diagnose.h>
#include <uno/data.h>
#include <com/sun/star/uno/genfunc.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <rtl/ustrbuf.hxx>

#include <algorithm>


namespace comphelper
{


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;


namespace
{
    // comparing two property descriptions
    struct PropertyDescriptionHandleCompare
    {
        bool operator() (const PropertyDescription& x, const PropertyDescription& y) const
        {
            return x.aProperty.Handle < y.aProperty.Handle;
        }
    };
    // comparing two property descriptions (by name)
    struct PropertyDescriptionNameMatch
    {
        OUString const m_rCompare;
        explicit PropertyDescriptionNameMatch( const OUString& _rCompare ) : m_rCompare( _rCompare ) { }

        bool operator() (const PropertyDescription& x ) const
        {
            return x.aProperty.Name == m_rCompare;
        }
    };
}

OPropertyContainerHelper::OPropertyContainerHelper()
{
}


OPropertyContainerHelper::~OPropertyContainerHelper()
{
}


void OPropertyContainerHelper::registerProperty(const OUString& _rName, sal_Int32 _nHandle,
        sal_Int32 _nAttributes, void* _pPointerToMember, const Type& _rMemberType)
{
    OSL_ENSURE((_nAttributes & PropertyAttribute::MAYBEVOID) == 0,
        "OPropertyContainerHelper::registerProperty: don't use this for properties which may be void ! There is a method called \"registerMayBeVoidProperty\" for this !");
    OSL_ENSURE(!_rMemberType.equals(cppu::UnoType<Any>::get()),
        "OPropertyContainerHelper::registerProperty: don't give my the type of an uno::Any ! Really can't handle this !");
    OSL_ENSURE(_pPointerToMember,
        "OPropertyContainerHelper::registerProperty: you gave me nonsense : the pointer must be non-NULL");

    PropertyDescription aNewProp;
    aNewProp.aProperty = Property( _rName, _nHandle, _rMemberType, static_cast<sal_Int16>(_nAttributes) );
    aNewProp.eLocated = PropertyDescription::LocationType::DerivedClassRealType;
    aNewProp.aLocation.pDerivedClassMember = _pPointerToMember;

    implPushBackProperty(aNewProp);
}


void OPropertyContainerHelper::revokeProperty( sal_Int32 _nHandle )
{
    PropertiesIterator aPos = searchHandle( _nHandle );
    if ( aPos == m_aProperties.end() )
        throw UnknownPropertyException();
    m_aProperties.erase( aPos );
}


void OPropertyContainerHelper::registerMayBeVoidProperty(const OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes,
        Any* _pPointerToMember, const Type& _rExpectedType)
{
    OSL_ENSURE((_nAttributes & PropertyAttribute::MAYBEVOID) != 0,
        "OPropertyContainerHelper::registerMayBeVoidProperty: why calling this when the attributes say nothing about may-be-void ?");
    OSL_ENSURE(!_rExpectedType.equals(cppu::UnoType<Any>::get()),
        "OPropertyContainerHelper::registerMayBeVoidProperty: don't give my the type of an uno::Any ! Really can't handle this !");
    OSL_ENSURE(_pPointerToMember,
        "OPropertyContainerHelper::registerMayBeVoidProperty: you gave me nonsense : the pointer must be non-NULL");

    _nAttributes |= PropertyAttribute::MAYBEVOID;

    PropertyDescription aNewProp;
    aNewProp.aProperty = Property( _rName, _nHandle, _rExpectedType, static_cast<sal_Int16>(_nAttributes) );
    aNewProp.eLocated = PropertyDescription::LocationType::DerivedClassAnyType;
    aNewProp.aLocation.pDerivedClassMember = _pPointerToMember;

    implPushBackProperty(aNewProp);
}


void OPropertyContainerHelper::registerPropertyNoMember(const OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes,
        const Type& _rType, css::uno::Any const & _pInitialValue)
{
    OSL_ENSURE(!_rType.equals(cppu::UnoType<Any>::get()),
        "OPropertyContainerHelper::registerPropertyNoMember : don't give my the type of an uno::Any ! Really can't handle this !");
    OSL_ENSURE(
        (_pInitialValue.isExtractableTo(_rType)
         || (!_pInitialValue.hasValue()
             && (_nAttributes & PropertyAttribute::MAYBEVOID) != 0)),
        "bad initial value");

    PropertyDescription aNewProp;
    aNewProp.aProperty = Property( _rName, _nHandle, _rType, static_cast<sal_Int16>(_nAttributes) );
    aNewProp.eLocated = PropertyDescription::LocationType::HoldMyself;
    aNewProp.aLocation.nOwnClassVectorIndex = m_aHoldProperties.size();
    m_aHoldProperties.push_back(_pInitialValue);

    implPushBackProperty(aNewProp);
}


bool OPropertyContainerHelper::isRegisteredProperty( sal_Int32 _nHandle ) const
{
    return const_cast< OPropertyContainerHelper* >( this )->searchHandle( _nHandle ) != m_aProperties.end();
}


bool OPropertyContainerHelper::isRegisteredProperty( const OUString& _rName ) const
{
    // TODO: the current structure is from a time where properties were
    // static, not dynamic. Since we allow that properties are also dynamic,
    // i.e. registered and revoked even though the XPropertySet has already been
    // accessed, a vector is not really the best data structure anymore ...

    return std::any_of(
        m_aProperties.begin(),
        m_aProperties.end(),
        PropertyDescriptionNameMatch( _rName )
    );
}


namespace
{
    struct ComparePropertyHandles
    {
        bool operator()( const PropertyDescription& _rLHS, const PropertyDescription& _nRHS ) const
        {
            return _rLHS.aProperty.Handle < _nRHS.aProperty.Handle;
        }
    };
}


void OPropertyContainerHelper::implPushBackProperty(const PropertyDescription& _rProp)
{
#ifdef DBG_UTIL
    for (auto& checkConflicts : m_aProperties)
    {
        OSL_ENSURE(checkConflicts.aProperty.Name != _rProp.aProperty.Name, "OPropertyContainerHelper::implPushBackProperty: name already exists!");
        OSL_ENSURE(checkConflicts.aProperty.Handle != _rProp.aProperty.Handle, "OPropertyContainerHelper::implPushBackProperty: handle already exists!");
    }
#endif

    PropertiesIterator pos = std::lower_bound(
        m_aProperties.begin(), m_aProperties.end(),
        _rProp, ComparePropertyHandles() );

    m_aProperties.insert( pos, _rProp );
}


namespace
{
    void lcl_throwIllegalPropertyValueTypeException( const PropertyDescription& _rProperty, const Any& _rValue )
    {
        throw IllegalArgumentException(
            "The given value cannot be converted to the required property type."
            " (property name \"" +  _rProperty.aProperty.Name
            + "\", found value type \"" + _rValue.getValueType().getTypeName()
            + "\", required property type \"" + _rProperty.aProperty.Type.getTypeName()
            + "\")",
            nullptr, 4 );
    }
}


bool OPropertyContainerHelper::convertFastPropertyValue(
    Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue )
{
    bool bModified = false;

    // get the property somebody is asking for
    PropertiesIterator aPos = searchHandle(_nHandle);
    if (aPos == m_aProperties.end())
    {
        OSL_FAIL( "OPropertyContainerHelper::convertFastPropertyValue: unknown handle!" );
        // should not happen if the derived class has built a correct property set info helper to be used by
        // our base class OPropertySetHelper
        return bModified;
    }

    switch (aPos->eLocated)
    {
        // similar handling for the two cases where the value is stored in an any
        case PropertyDescription::LocationType::HoldMyself:
        case PropertyDescription::LocationType::DerivedClassAnyType:
        {
            bool bMayBeVoid = ((aPos->aProperty.Attributes & PropertyAttribute::MAYBEVOID) != 0);


            // non modifiable version of the value-to-be-set
            Any aNewRequestedValue( _rValue );

            // normalization
            // #i29490#
            if ( !aNewRequestedValue.getValueType().equals( aPos->aProperty.Type ) )
            {   // the actually given value is not of the same type as the one required
                Any aProperlyTyped( nullptr, aPos->aProperty.Type.getTypeLibType() );

                if (    uno_type_assignData(
                            const_cast< void* >( aProperlyTyped.getValue() ), aProperlyTyped.getValueType().getTypeLibType(),
                            const_cast< void* >( aNewRequestedValue.getValue() ), aNewRequestedValue.getValueType().getTypeLibType(),
                            reinterpret_cast< uno_QueryInterfaceFunc >( cpp_queryInterface ),
                            reinterpret_cast< uno_AcquireFunc >( cpp_acquire ),
                            reinterpret_cast< uno_ReleaseFunc >( cpp_release )
                        )
                    )
                {
                    // we were able to query the given XInterface-derivee for the interface
                    // which is required for this property
                    aNewRequestedValue = aProperlyTyped;
                }
            }

            // argument check
            if  (   !   (   (bMayBeVoid && !aNewRequestedValue.hasValue())                      // void is allowed if the attribute says so
                        ||  (aNewRequestedValue.getValueType().equals(aPos->aProperty.Type))    // else the types have to be equal
                        )
                )
            {
                lcl_throwIllegalPropertyValueTypeException( *aPos, _rValue );
            }

            Any* pPropContainer = nullptr;
                // the pointer to the any which holds the property value, no matter if located in the derived class
                // or in out vector

            if (PropertyDescription::LocationType::HoldMyself == aPos->eLocated)
            {
                OSL_ENSURE(aPos->aLocation.nOwnClassVectorIndex < static_cast<sal_Int32>(m_aHoldProperties.size()),
                    "OPropertyContainerHelper::convertFastPropertyValue: invalid position !");
                auto aIter = m_aHoldProperties.begin() + aPos->aLocation.nOwnClassVectorIndex;
                pPropContainer = &(*aIter);
            }
            else
                pPropContainer = static_cast<Any*>(aPos->aLocation.pDerivedClassMember);

            // check if the new value differs from the current one
            if (!pPropContainer->hasValue() || !aNewRequestedValue.hasValue())
                bModified = pPropContainer->hasValue() != aNewRequestedValue.hasValue();
            else
                bModified = !uno_type_equalData(
                                const_cast< void* >( pPropContainer->getValue() ), aPos->aProperty.Type.getTypeLibType(),
                                const_cast< void* >( aNewRequestedValue.getValue() ), aPos->aProperty.Type.getTypeLibType(),
                                reinterpret_cast< uno_QueryInterfaceFunc >( cpp_queryInterface ),
                                reinterpret_cast< uno_ReleaseFunc >( cpp_release )
                            );

            if (bModified)
            {
                _rOldValue = *pPropContainer;
                _rConvertedValue = aNewRequestedValue;
            }
        }
        break;
        case PropertyDescription::LocationType::DerivedClassRealType:
            // let the UNO runtime library do any possible conversion
            // this may include a change of the type - for instance, if a LONG is required,
            // but a short is given, then this is valid, as it can be converted without any potential
            // data loss

            Any aProperlyTyped;
            const Any* pNewValue = &_rValue;

            if (!_rValue.getValueType().equals(aPos->aProperty.Type))
            {
                bool bConverted = false;

                // a temporary any of the correct (required) type
                aProperlyTyped = Any( nullptr, aPos->aProperty.Type.getTypeLibType() );
                    // (need this as we do not want to overwrite the derived class member here)

                if (    uno_type_assignData(
                            const_cast<void*>(aProperlyTyped.getValue()), aProperlyTyped.getValueType().getTypeLibType(),
                            const_cast<void*>(_rValue.getValue()), _rValue.getValueType().getTypeLibType(),
                            reinterpret_cast< uno_QueryInterfaceFunc >( cpp_queryInterface ),
                            reinterpret_cast< uno_AcquireFunc >( cpp_acquire ),
                            reinterpret_cast< uno_ReleaseFunc >( cpp_release )
                        )
                    )
                {
                    // could query for the requested interface
                    bConverted = true;
                    pNewValue = &aProperlyTyped;
                }

                if ( !bConverted )
                    lcl_throwIllegalPropertyValueTypeException( *aPos, _rValue );
            }

            // from here on, we should have the proper type
            OSL_ENSURE( pNewValue->getValueType() == aPos->aProperty.Type,
                "OPropertyContainerHelper::convertFastPropertyValue: conversion failed!" );
            bModified = !uno_type_equalData(
                            aPos->aLocation.pDerivedClassMember, aPos->aProperty.Type.getTypeLibType(),
                            const_cast<void*>(pNewValue->getValue()), aPos->aProperty.Type.getTypeLibType(),
                            reinterpret_cast< uno_QueryInterfaceFunc >( cpp_queryInterface ),
                            reinterpret_cast< uno_ReleaseFunc >( cpp_release )
                        );

            if (bModified)
            {
                _rOldValue.setValue(aPos->aLocation.pDerivedClassMember, aPos->aProperty.Type);
                _rConvertedValue = *pNewValue;
            }
            break;
    }

    return bModified;
}


void OPropertyContainerHelper::setFastPropertyValue(sal_Int32 _nHandle, const Any& _rValue)
{
    // get the property somebody is asking for
    PropertiesIterator aPos = searchHandle(_nHandle);
    if (aPos == m_aProperties.end())
    {
        OSL_FAIL( "OPropertyContainerHelper::setFastPropertyValue: unknown handle!" );
        // should not happen if the derived class has built a correct property set info helper to be used by
        // our base class OPropertySetHelper
        return;
    }

    bool bSuccess = true;

    switch (aPos->eLocated)
    {
        case PropertyDescription::LocationType::HoldMyself:
            m_aHoldProperties[aPos->aLocation.nOwnClassVectorIndex] = _rValue;
            break;

        case PropertyDescription::LocationType::DerivedClassAnyType:
            *static_cast< Any* >(aPos->aLocation.pDerivedClassMember) = _rValue;
            break;

        case PropertyDescription::LocationType::DerivedClassRealType:
            // copy the data from the to-be-set value
            bSuccess = uno_type_assignData(
                aPos->aLocation.pDerivedClassMember,        aPos->aProperty.Type.getTypeLibType(),
                const_cast< void* >( _rValue.getValue() ),  _rValue.getValueType().getTypeLibType(),
                reinterpret_cast< uno_QueryInterfaceFunc >( cpp_queryInterface ),
                reinterpret_cast< uno_AcquireFunc >( cpp_acquire ),
                reinterpret_cast< uno_ReleaseFunc >( cpp_release ) );

            OSL_ENSURE( bSuccess,
                "OPropertyContainerHelper::setFastPropertyValue: ooops .... the value could not be assigned!");

            break;
    }
}

void OPropertyContainerHelper::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    // get the property somebody is asking for
    PropertiesIterator aPos = const_cast<OPropertyContainerHelper*>(this)->searchHandle(_nHandle);
    if (aPos == m_aProperties.end())
    {
        OSL_FAIL( "OPropertyContainerHelper::getFastPropertyValue: unknown handle!" );
        // should not happen if the derived class has built a correct property set info helper to be used by
        // our base class OPropertySetHelper
        return;
    }

    switch (aPos->eLocated)
    {
        case PropertyDescription::LocationType::HoldMyself:
            OSL_ENSURE(aPos->aLocation.nOwnClassVectorIndex < static_cast<sal_Int32>(m_aHoldProperties.size()),
                "OPropertyContainerHelper::convertFastPropertyValue: invalid position !");
            _rValue = m_aHoldProperties[aPos->aLocation.nOwnClassVectorIndex];
            break;
        case PropertyDescription::LocationType::DerivedClassAnyType:
            _rValue = *static_cast<Any*>(aPos->aLocation.pDerivedClassMember);
            break;
        case PropertyDescription::LocationType::DerivedClassRealType:
            _rValue.setValue(aPos->aLocation.pDerivedClassMember, aPos->aProperty.Type);
            break;
    }
}


OPropertyContainerHelper::PropertiesIterator OPropertyContainerHelper::searchHandle(sal_Int32 _nHandle)
{
    PropertyDescription aHandlePropDesc;
    aHandlePropDesc.aProperty.Handle = _nHandle;
    // search a lower bound
    PropertiesIterator aLowerBound = std::lower_bound(
        m_aProperties.begin(),
        m_aProperties.end(),
        aHandlePropDesc,
        PropertyDescriptionHandleCompare());

    // check for identity
    if ((aLowerBound != m_aProperties.end()) && aLowerBound->aProperty.Handle != _nHandle)
        aLowerBound = m_aProperties.end();

    return aLowerBound;
}


const Property& OPropertyContainerHelper::getProperty( const OUString& _rName ) const
{
    ConstPropertiesIterator pos = std::find_if(
        m_aProperties.begin(),
        m_aProperties.end(),
        PropertyDescriptionNameMatch( _rName )
    );
    if ( pos == m_aProperties.end() )
        throw UnknownPropertyException( _rName );

    return pos->aProperty;
}


void OPropertyContainerHelper::describeProperties(Sequence< Property >& _rProps) const
{
    Sequence< Property > aOwnProps(m_aProperties.size());
    Property* pOwnProps = aOwnProps.getArray();

    for (const auto& rProp : m_aProperties)
    {
        pOwnProps->Name = rProp.aProperty.Name;
        pOwnProps->Handle = rProp.aProperty.Handle;
        pOwnProps->Attributes = static_cast<sal_Int16>(rProp.aProperty.Attributes);
        pOwnProps->Type = rProp.aProperty.Type;
        ++pOwnProps;
    }

    // as our property vector is sorted by handles, not by name, we have to sort aOwnProps
    std::sort(aOwnProps.begin(), aOwnProps.end(), PropertyCompareByName());

    // unfortunately the STL merge function does not allow the output range to overlap one of the input ranges,
    // so we need an extra sequence
    Sequence< Property > aOutput;
    aOutput.realloc(_rProps.getLength() + aOwnProps.getLength());
    // do the merge
    std::merge(   _rProps.begin(), _rProps.end(),       // input 1
                  aOwnProps.begin(), aOwnProps.end(),   // input 2
                  aOutput.getArray(),                   // output
                  PropertyCompareByName()               // compare operator
              );

    // copy the output
    _rProps = aOutput;
}


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
