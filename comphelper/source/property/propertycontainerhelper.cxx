/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <comphelper/propertycontainerhelper.hxx>
#include <comphelper/property.hxx>
#include <osl/diagnose.h>
#include <uno/data.h>
#include <com/sun/star/uno/genfunc.h>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <rtl/ustrbuf.hxx>

#include <algorithm>

//.........................................................................
namespace comphelper
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

//--------------------------------------------------------------------------
namespace
{
    // comparing two property descriptions
    struct PropertyDescriptionCompareByHandle : public ::std::binary_function< PropertyDescription, PropertyDescription, bool >
    {
        bool operator() (const PropertyDescription& x, const PropertyDescription& y) const
        {
            return x.aProperty.Handle < y.aProperty.Handle;
        }
    };
    // comparing two property descriptions
    struct PropertyDescriptionHandleCompare : public ::std::binary_function< PropertyDescription, sal_Int32, bool >
    {
        bool operator() (const PropertyDescription& x, const sal_Int32& y) const
        {
            return x.aProperty.Handle < y;
        }
        bool operator() (const sal_Int32& x, const PropertyDescription& y) const
        {
            return x < y.aProperty.Handle;
        }
    };
    // comparing two property descriptions (by name)
    struct PropertyDescriptionNameMatch : public ::std::unary_function< PropertyDescription, bool >
    {
        ::rtl::OUString m_rCompare;
        PropertyDescriptionNameMatch( const ::rtl::OUString& _rCompare ) : m_rCompare( _rCompare ) { }

        bool operator() (const PropertyDescription& x ) const
        {
            return x.aProperty.Name.equals(m_rCompare);
        }
    };
}

//==========================================================================
//= OPropertyContainerHelper
//==========================================================================
//--------------------------------------------------------------------------
OPropertyContainerHelper::OPropertyContainerHelper()
    :m_bUnused(sal_False)
{
}

// -------------------------------------------------------------------------
OPropertyContainerHelper::~OPropertyContainerHelper()
{
}

//--------------------------------------------------------------------------
void OPropertyContainerHelper::registerProperty(const ::rtl::OUString& _rName, sal_Int32 _nHandle,
        sal_Int32 _nAttributes, void* _pPointerToMember, const Type& _rMemberType)
{
    OSL_ENSURE((_nAttributes & PropertyAttribute::MAYBEVOID) == 0,
        "OPropertyContainerHelper::registerProperty: don't use this for properties which may be void ! There is a method called \"registerMayBeVoidProperty\" for this !");
    OSL_ENSURE(!_rMemberType.equals(::getCppuType(static_cast< Any* >(NULL))),
        "OPropertyContainerHelper::registerProperty: don't give my the type of an uno::Any ! Really can't handle this !");
    OSL_ENSURE(_pPointerToMember,
        "OPropertyContainerHelper::registerProperty: you gave me nonsense : the pointer must be non-NULL");

    PropertyDescription aNewProp;
    aNewProp.aProperty = Property( _rName, _nHandle, _rMemberType, (sal_Int16)_nAttributes );
    aNewProp.eLocated = PropertyDescription::ltDerivedClassRealType;
    aNewProp.aLocation.pDerivedClassMember = _pPointerToMember;

    implPushBackProperty(aNewProp);
}

//--------------------------------------------------------------------------
void OPropertyContainerHelper::revokeProperty( sal_Int32 _nHandle )
{
    PropertiesIterator aPos = searchHandle( _nHandle );
    if ( aPos == m_aProperties.end() )
        throw UnknownPropertyException();
    m_aProperties.erase( aPos );
}

//--------------------------------------------------------------------------
void OPropertyContainerHelper::registerMayBeVoidProperty(const ::rtl::OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes,
        Any* _pPointerToMember, const Type& _rExpectedType)
{
    OSL_ENSURE((_nAttributes & PropertyAttribute::MAYBEVOID) != 0,
        "OPropertyContainerHelper::registerMayBeVoidProperty: why calling this when the attributes say nothing about may-be-void ?");
    OSL_ENSURE(!_rExpectedType.equals(::getCppuType(static_cast< Any* >(NULL))),
        "OPropertyContainerHelper::registerMayBeVoidProperty: don't give my the type of an uno::Any ! Really can't handle this !");
    OSL_ENSURE(_pPointerToMember,
        "OPropertyContainerHelper::registerMayBeVoidProperty: you gave me nonsense : the pointer must be non-NULL");

    _nAttributes |= PropertyAttribute::MAYBEVOID;

    PropertyDescription aNewProp;
    aNewProp.aProperty = Property( _rName, _nHandle, _rExpectedType, (sal_Int16)_nAttributes );
    aNewProp.eLocated = PropertyDescription::ltDerivedClassAnyType;
    aNewProp.aLocation.pDerivedClassMember = _pPointerToMember;

    implPushBackProperty(aNewProp);
}


//--------------------------------------------------------------------------
void OPropertyContainerHelper::registerPropertyNoMember(const ::rtl::OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes,
        const Type& _rType, const void* _pInitialValue)
{
    OSL_ENSURE(!_rType.equals(::getCppuType(static_cast< Any* >(NULL))),
        "OPropertyContainerHelper::registerPropertyNoMember : don't give my the type of an uno::Any ! Really can't handle this !");
    OSL_ENSURE(_pInitialValue || ((_nAttributes & PropertyAttribute::MAYBEVOID) != 0),
        "OPropertyContainerHelper::registerPropertyNoMember : you should not ommit the initial value if the property can't be void ! This will definitivly crash later !");

    PropertyDescription aNewProp;
    aNewProp.aProperty = Property( _rName, _nHandle, _rType, (sal_Int16)_nAttributes );
    aNewProp.eLocated = PropertyDescription::ltHoldMyself;
    aNewProp.aLocation.nOwnClassVectorIndex = m_aHoldProperties.size();
    if (_pInitialValue)
        m_aHoldProperties.push_back(Any(_pInitialValue, _rType));
    else
        m_aHoldProperties.push_back(Any());

    implPushBackProperty(aNewProp);
}

//--------------------------------------------------------------------------
sal_Bool OPropertyContainerHelper::isRegisteredProperty( sal_Int32 _nHandle ) const
{
    return const_cast< OPropertyContainerHelper* >( this )->searchHandle( _nHandle ) != m_aProperties.end();
}

//--------------------------------------------------------------------------
sal_Bool OPropertyContainerHelper::isRegisteredProperty( const ::rtl::OUString& _rName ) const
{
    // TODO: the current structure is from a time where properties were
    // static, not dynamic. Since we allow that properties are also dynamic,
    // i.e. registered and revoked even though the XPropertySet has already been
    // accessed, a vector is not really the best data structure anymore ...

    ConstPropertiesIterator pos = ::std::find_if(
        m_aProperties.begin(),
        m_aProperties.end(),
        PropertyDescriptionNameMatch( _rName )
    );
    return pos != m_aProperties.end();
}

//--------------------------------------------------------------------------
namespace
{
    struct ComparePropertyWithHandle
    {
        bool operator()( const PropertyDescription& _rLHS, sal_Int32 _nRHS ) const
        {
            return _rLHS.aProperty.Handle < _nRHS;
        }
        bool operator()( sal_Int32 _nLHS, const PropertyDescription& _rRHS ) const
        {
            return _nLHS < _rRHS.aProperty.Handle;
        }
    };
}

//--------------------------------------------------------------------------
void OPropertyContainerHelper::implPushBackProperty(const PropertyDescription& _rProp)
{
#ifdef DBG_UTIL
    for (   PropertiesIterator checkConflicts = m_aProperties.begin();
            checkConflicts != m_aProperties.end();
            ++checkConflicts
        )
    {
        OSL_ENSURE(checkConflicts->aProperty.Name != _rProp.aProperty.Name, "OPropertyContainerHelper::implPushBackProperty: name already exists!");
        OSL_ENSURE(checkConflicts->aProperty.Handle != _rProp.aProperty.Handle, "OPropertyContainerHelper::implPushBackProperty: handle already exists!");
    }
#endif

    PropertiesIterator pos = ::std::lower_bound(
        m_aProperties.begin(), m_aProperties.end(),
        _rProp.aProperty.Handle, ComparePropertyWithHandle() );

    m_aProperties.insert( pos, _rProp );
}

//--------------------------------------------------------------------------
namespace
{
    void lcl_throwIllegalPropertyValueTypeException( const PropertyDescription& _rProperty, const Any& _rValue )
    {
        ::rtl::OUStringBuffer aErrorMessage;
        aErrorMessage.appendAscii( "The given value cannot be converted to the required property type." );
        aErrorMessage.appendAscii( "\n(property name \"" );
        aErrorMessage.append( _rProperty.aProperty.Name );
        aErrorMessage.appendAscii( "\", found value type \"" );
        aErrorMessage.append( _rValue.getValueType().getTypeName() );
        aErrorMessage.appendAscii( "\", required property type \"" );
        aErrorMessage.append( _rProperty.aProperty.Type.getTypeName() );
        aErrorMessage.appendAscii( "\")" );
        throw IllegalArgumentException( aErrorMessage.makeStringAndClear(), NULL, 4 );
    }
}

//--------------------------------------------------------------------------
sal_Bool OPropertyContainerHelper::convertFastPropertyValue(
    Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue ) SAL_THROW( (IllegalArgumentException) )
{
    sal_Bool bModified = sal_False;

    // get the property somebody is asking for
    PropertiesIterator aPos = searchHandle(_nHandle);
    if (aPos == m_aProperties.end())
    {
        OSL_ENSURE( false, "OPropertyContainerHelper::convertFastPropertyValue: unknown handle!" );
        // should not happen if the derived class has built a correct property set info helper to be used by
        // our base class OPropertySetHelper
        return bModified;
    }

    switch (aPos->eLocated)
    {
        // similar handling for the two cases where the value is stored in an any
        case PropertyDescription::ltHoldMyself:
        case PropertyDescription::ltDerivedClassAnyType:
        {
            sal_Bool bMayBeVoid = ((aPos->aProperty.Attributes & PropertyAttribute::MAYBEVOID) != 0);


            // non modifiable version of the value-to-be-set
            Any aNewRequestedValue( _rValue );

            // normalization
            // #i29490#
            if ( !aNewRequestedValue.getValueType().equals( aPos->aProperty.Type ) )
            {   // the actually given value is not of the same type as the one required
                Any aProperlyTyped( NULL, aPos->aProperty.Type.getTypeLibType() );

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

            Any* pPropContainer = NULL;
                // the pointer to the any which holds the property value, no matter if located in the derived clas
                // or in out vector

            if (PropertyDescription::ltHoldMyself == aPos->eLocated)
            {
                OSL_ENSURE(aPos->aLocation.nOwnClassVectorIndex < (sal_Int32)m_aHoldProperties.size(),
                    "OPropertyContainerHelper::convertFastPropertyValue: invalid position !");
                PropertyContainerIterator aIter = m_aHoldProperties.begin() + aPos->aLocation.nOwnClassVectorIndex;
                pPropContainer = &(*aIter);
            }
            else
                pPropContainer = reinterpret_cast<Any*>(aPos->aLocation.pDerivedClassMember);

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
        case PropertyDescription::ltDerivedClassRealType:
            // let the UNO runtime library do any possible conversion
            // this may include a change of the type - for instance, if a LONG is required,
            // but a short is given, then this is valid, as it can be converted without any potential
            // data loss

            Any aProperlyTyped;
            const Any* pNewValue = &_rValue;

            if (!_rValue.getValueType().equals(aPos->aProperty.Type))
            {
                sal_Bool bConverted = sal_False;

                // a temporary any of the correct (required) type
                aProperlyTyped = Any( NULL, aPos->aProperty.Type.getTypeLibType() );
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
                    bConverted = sal_True;
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

//--------------------------------------------------------------------------
void OPropertyContainerHelper::setFastPropertyValue(sal_Int32 _nHandle, const Any& _rValue) SAL_THROW( (Exception) )
{
    // get the property somebody is asking for
    PropertiesIterator aPos = searchHandle(_nHandle);
    if (aPos == m_aProperties.end())
    {
        OSL_ENSURE( false, "OPropertyContainerHelper::setFastPropertyValue: unknown handle!" );
        // should not happen if the derived class has built a correct property set info helper to be used by
        // our base class OPropertySetHelper
        return;
    }

    switch (aPos->eLocated)
    {
        case PropertyDescription::ltHoldMyself:
            m_aHoldProperties[aPos->aLocation.nOwnClassVectorIndex] = _rValue;
            break;

        case PropertyDescription::ltDerivedClassAnyType:
            *reinterpret_cast< Any* >(aPos->aLocation.pDerivedClassMember) = _rValue;
            break;

        case PropertyDescription::ltDerivedClassRealType:
#if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
#endif
            // copy the data from the to-be-set value
            uno_type_assignData(
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

//--------------------------------------------------------------------------
void OPropertyContainerHelper::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    // get the property somebody is asking for
    PropertiesIterator aPos = const_cast<OPropertyContainerHelper*>(this)->searchHandle(_nHandle);
    if (aPos == m_aProperties.end())
    {
        OSL_ENSURE( false, "OPropertyContainerHelper::getFastPropertyValue: unknown handle!" );
        // should not happen if the derived class has built a correct property set info helper to be used by
        // our base class OPropertySetHelper
        return;
    }

    switch (aPos->eLocated)
    {
        case PropertyDescription::ltHoldMyself:
            OSL_ENSURE(aPos->aLocation.nOwnClassVectorIndex < (sal_Int32)m_aHoldProperties.size(),
                "OPropertyContainerHelper::convertFastPropertyValue: invalid position !");
            _rValue = m_aHoldProperties[aPos->aLocation.nOwnClassVectorIndex];
            break;
        case PropertyDescription::ltDerivedClassAnyType:
            _rValue = *reinterpret_cast<Any*>(aPos->aLocation.pDerivedClassMember);
            break;
        case PropertyDescription::ltDerivedClassRealType:
            _rValue.setValue(aPos->aLocation.pDerivedClassMember, aPos->aProperty.Type);
            break;
    }
}

//--------------------------------------------------------------------------
OPropertyContainerHelper::PropertiesIterator OPropertyContainerHelper::searchHandle(sal_Int32 _nHandle)
{
    // search a lower bound
    PropertiesIterator aLowerBound = ::std::lower_bound(
        m_aProperties.begin(),
        m_aProperties.end(),
        _nHandle,
        PropertyDescriptionHandleCompare());

    // check for identity
    if ((aLowerBound != m_aProperties.end()) && aLowerBound->aProperty.Handle != _nHandle)
        aLowerBound = m_aProperties.end();

    return aLowerBound;
}

//--------------------------------------------------------------------------
const Property& OPropertyContainerHelper::getProperty( const ::rtl::OUString& _rName ) const
{
    ConstPropertiesIterator pos = ::std::find_if(
        m_aProperties.begin(),
        m_aProperties.end(),
        PropertyDescriptionNameMatch( _rName )
    );
    if ( pos == m_aProperties.end() )
        throw UnknownPropertyException( _rName, NULL );

    return pos->aProperty;
}

//--------------------------------------------------------------------------
void OPropertyContainerHelper::modifyAttributes(sal_Int32 _nHandle, sal_Int32 _nAddAttrib, sal_Int32 _nRemoveAttrib)
{
    // get the property somebody is asking for
    PropertiesIterator aPos = searchHandle(_nHandle);
    if (aPos == m_aProperties.end())
    {
        OSL_ENSURE( false, "OPropertyContainerHelper::modifyAttributes: unknown handle!" );
        // should not happen if the derived class has built a correct property set info helper to be used by
        // our base class OPropertySetHelper
        return;
    }
    aPos->aProperty.Handle |= _nAddAttrib;
    aPos->aProperty.Handle &= ~_nRemoveAttrib;
}

//--------------------------------------------------------------------------
void OPropertyContainerHelper::describeProperties(Sequence< Property >& _rProps) const
{
    Sequence< Property > aOwnProps(m_aProperties.size());
    Property* pOwnProps = aOwnProps.getArray();

    for (   ConstPropertiesIterator aLoop = m_aProperties.begin();
            aLoop != m_aProperties.end();
            ++aLoop, ++pOwnProps
        )
    {
        pOwnProps->Name = aLoop->aProperty.Name;
        pOwnProps->Handle = aLoop->aProperty.Handle;
        pOwnProps->Attributes = (sal_Int16)aLoop->aProperty.Attributes;
        pOwnProps->Type = aLoop->aProperty.Type;
    }

    // as our property vector is sorted by handles, not by name, we have to sort aOwnProps
    ::std::sort(aOwnProps.getArray(), aOwnProps.getArray() + aOwnProps.getLength(), PropertyCompareByName());

    // unfortunally the STL merge function does not allow the output range to overlap one of the input ranges,
    // so we need an extra sequence
    Sequence< Property > aOutput;
    aOutput.realloc(_rProps.getLength() + aOwnProps.getLength());
    // do the merge
    ::std::merge(   _rProps.getConstArray(), _rProps.getConstArray() + _rProps.getLength(),         // input 1
                    aOwnProps.getConstArray(), aOwnProps.getConstArray() + aOwnProps.getLength(),   // input 2
                    aOutput.getArray(),                                                             // output
                    PropertyCompareByName()                                                         // compare operator
                );

    // copy the output
    _rProps = aOutput;
}

//.........................................................................
}   // namespace comphelper
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
