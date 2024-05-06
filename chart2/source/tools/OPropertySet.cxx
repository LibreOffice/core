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

#include <OPropertySet.hxx>
#include <CloneHelper.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/style/XStyle.hpp>

#include <algorithm>
#include <memory>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

// needed for MS compiler
using ::cppu::OBroadcastHelper;
using ::cppu::OPropertySetHelper;

namespace property
{

OPropertySet::OPropertySet( ) :
        OBroadcastHelper( m_aMutex ),
        // the following causes a warning; there seems to be no way to avoid it
        OPropertySetHelper( static_cast< OBroadcastHelper & >( *this )),
        m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault(false)
{
}

OPropertySet::OPropertySet( const OPropertySet & rOther ) :
        OBroadcastHelper( m_aMutex ),
        // the following causes a warning; there seems to be no way to avoid it
        OPropertySetHelper( static_cast< OBroadcastHelper & >( *this )),
        css::lang::XTypeProvider(),
        css::beans::XPropertyState(),
        css::beans::XMultiPropertyStates(),
        css::style::XStyleSupplier(),
        m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault(false)
{
    MutexGuard aGuard( m_aMutex );

    m_aProperties = rOther.m_aProperties;

    // clone interface properties
    for(auto& rProp : m_aProperties)
    {
        if( rProp.second.hasValue() &&
            rProp.second.getValueType().getTypeClass() == uno::TypeClass_INTERFACE )
        {
            Reference< util::XCloneable > xCloneable;
            if( rProp.second >>= xCloneable )
                rProp.second <<= xCloneable->createClone();
        }
    }

    m_xStyle.set( ::chart::CloneHelper::CreateRefClone< style::XStyle >()( rOther.m_xStyle ));
}

void OPropertySet::SetNewValuesExplicitlyEvenIfTheyEqualDefault()
{
    m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault = true;
}

OPropertySet::~OPropertySet()
{}

Any SAL_CALL OPropertySet::queryInterface( const uno::Type& aType )
{
    return ::cppu::queryInterface(
        aType,
        static_cast< lang::XTypeProvider * >( this ),
        static_cast< beans::XPropertySet * >( this ),
        static_cast< beans::XMultiPropertySet * >( this ),
        static_cast< beans::XFastPropertySet * >( this ),
        static_cast< beans::XPropertyState * >( this ),
        static_cast< beans::XMultiPropertyStates * >( this ),
        static_cast< style::XStyleSupplier * >( this ) );
}

//  ____ XTypeProvider ____
Sequence< uno::Type > SAL_CALL
    OPropertySet::getTypes()
{
    static const Sequence< uno::Type > aTypeList{
        cppu::UnoType<lang::XTypeProvider>::get(),
        cppu::UnoType<beans::XPropertySet>::get(),
        cppu::UnoType<beans::XMultiPropertySet>::get(),
        cppu::UnoType<beans::XFastPropertySet>::get(),
        cppu::UnoType<beans::XPropertyState>::get(),
        cppu::UnoType<beans::XMultiPropertyStates>::get(),
        cppu::UnoType<style::XStyleSupplier>::get() };

    return aTypeList;
}

Sequence< sal_Int8 > SAL_CALL
    OPropertySet::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// ____ XPropertyState ____
beans::PropertyState SAL_CALL
    OPropertySet::getPropertyState( const OUString& PropertyName )
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    return GetPropertyStateByHandle(
        rPH.getHandleByName( PropertyName ));
}

Sequence< beans::PropertyState > SAL_CALL
    OPropertySet::getPropertyStates( const Sequence< OUString >& aPropertyName )
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    std::unique_ptr<sal_Int32[]> pHandles(new sal_Int32[ aPropertyName.getLength() ]);
    rPH.fillHandles( pHandles.get(), aPropertyName );

    std::vector< sal_Int32 > aHandles( pHandles.get(), pHandles.get() + aPropertyName.getLength());
    pHandles.reset();

    return GetPropertyStatesByHandle( aHandles );
}

void SAL_CALL
    OPropertySet::setPropertyToDefault( const OUString& PropertyName )
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    SetPropertyToDefault( rPH.getHandleByName( PropertyName ));
    firePropertyChangeEvent();
}

Any SAL_CALL
    OPropertySet::getPropertyDefault( const OUString& aPropertyName )
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    Any any;
    GetDefaultValue( rPH.getHandleByName( aPropertyName ), any );
    return any;
}

// ____ XMultiPropertyStates ____

// Note: getPropertyStates() is already implemented in XPropertyState with the
// same signature

void SAL_CALL
    OPropertySet::setAllPropertiesToDefault()
{
    SetAllPropertiesToDefault();
    firePropertyChangeEvent();
}

void SAL_CALL
    OPropertySet::setPropertiesToDefault( const Sequence< OUString >& aPropertyNames )
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    std::unique_ptr<sal_Int32[]> pHandles(new sal_Int32[ aPropertyNames.getLength() ]);
    rPH.fillHandles( pHandles.get(), aPropertyNames );

    std::vector< sal_Int32 > aHandles( pHandles.get(), pHandles.get() + aPropertyNames.getLength());
    pHandles.reset();

    SetPropertiesToDefault( aHandles );
}

Sequence< Any > SAL_CALL
    OPropertySet::getPropertyDefaults( const Sequence< OUString >& aPropertyNames )
{
    ::cppu::IPropertyArrayHelper & rPH = getInfoHelper();
    const sal_Int32 nElements = aPropertyNames.getLength();

    Sequence< Any > aResult( nElements );
    Any * pResultArray = aResult.getArray();
    sal_Int32 nI = 0;

    for( ; nI < nElements; ++nI )
    {
        GetDefaultValue(
            rPH.getHandleByName( aPropertyNames[ nI ] ),
            pResultArray[ nI ]);
    }

    return aResult;
}

sal_Bool SAL_CALL OPropertySet::convertFastPropertyValue
    ( Any & rConvertedValue,
      Any & rOldValue,
      sal_Int32 nHandle,
      const Any& rValue )
{
    getFastPropertyValue( rOldValue, nHandle );
    //accept longs also for short values
    {
        sal_Int16 nValue;
        if( (rOldValue>>=nValue) && !(rValue>>=nValue) )
        {
            sal_Int32 n32Value = 0;
            if( rValue>>=n32Value )
            {
                rConvertedValue <<= static_cast<sal_Int16>(n32Value);
                return true;
            }

            sal_Int64 n64Value = 0;
            if( rValue>>=n64Value )
            {
                rConvertedValue <<= static_cast<sal_Int16>(n64Value);
                return true;
            }
        }
    }
    rConvertedValue = rValue;
    if( !m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault && rOldValue == rConvertedValue )
        return false;//no change necessary
    return true;
}

void SAL_CALL OPropertySet::setFastPropertyValue_NoBroadcast
    ( sal_Int32 nHandle,
      const Any& rValue )
{
#if OSL_DEBUG_LEVEL > 0
    if( rValue.hasValue())
    {
        cppu::IPropertyArrayHelper & rPH = getInfoHelper();
        OUString aName;
        rPH.fillPropertyMembersByHandle( &aName, nullptr, nHandle );
        OSL_ENSURE( rValue.isExtractableTo( rPH.getPropertyByName( aName ).Type ),
                    "Property type is wrong" );
    }
#endif

    Any aDefault;
    try
    {
        GetDefaultValue( nHandle, aDefault );
    }
    catch( const beans::UnknownPropertyException& )
    {
        aDefault.clear();
    }
    if( !m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault && aDefault.hasValue() && aDefault == rValue ) //#i98893# don't export defaults to file
        SetPropertyToDefault( nHandle );
    else
        SetPropertyValueByHandle( nHandle, rValue );
}

void SAL_CALL OPropertySet::getFastPropertyValue
    ( Any& rValue,
      sal_Int32 nHandle ) const
{
    if(  GetPropertyValueByHandle( rValue, nHandle ))
        return;

    // property was not set -> try style
    uno::Reference< beans::XFastPropertySet > xStylePropSet( m_xStyle, uno::UNO_QUERY );
    if( xStylePropSet.is() )
    {
#ifdef DBG_UTIL
        {
            // check if the handle of the style points to the same property
            // name as the handle in this property set
            uno::Reference< beans::XPropertySet > xPropSet( xStylePropSet, uno::UNO_QUERY );
            if( xPropSet.is())
            {
                uno::Reference< beans::XPropertySetInfo > xInfo = xPropSet->getPropertySetInfo();
                if( xInfo.is() )
                {
                    // for some reason the virtual method getInfoHelper() is
                    // not const
                    ::cppu::IPropertyArrayHelper & rPH =
                          const_cast< OPropertySet * >( this )->getInfoHelper();

                    // find the Property with Handle nHandle in Style
                    Sequence< beans::Property > aProps( xInfo->getProperties() );
                    sal_Int32 nI = aProps.getLength() - 1;
                    while( ( nI >= 0 ) && nHandle != aProps[ nI ].Handle )
                        --nI;

                    if( nI >= 0 ) // => nHandle == aProps[nI].Handle
                    {
                        // check whether the handle in this property set is
                        // the same as the one in the style
                        beans::Property aProp( rPH.getPropertyByName( aProps[ nI ].Name ) );
                        OSL_ENSURE( nHandle == aProp.Handle,
                                    "HandleCheck: Handles for same property differ!" );

                        if( nHandle == aProp.Handle )
                        {
                            OSL_ENSURE( aProp.Type == aProps[nI].Type,
                                        "HandleCheck: Types differ!" );
                            OSL_ENSURE( aProp.Attributes == aProps[nI].Attributes,
                                        "HandleCheck: Attributes differ!" );
                        }
                    }
                    else
                    {
                        OSL_FAIL(  "HandleCheck: Handle not found in Style" );
                    }
                }
                else
                    OSL_FAIL( "HandleCheck: Invalid XPropertySetInfo returned" );
            }
            else
                OSL_FAIL( "HandleCheck: XPropertySet not supported" );
        }
#endif
        rValue = xStylePropSet->getFastPropertyValue( nHandle );
    }
    else
    {
        // there is no style (or the style does not support XFastPropertySet)
        // => take the default value
        try
        {
            GetDefaultValue( nHandle, rValue );
        }
        catch( const beans::UnknownPropertyException& )
        {
            rValue.clear();
        }
    }
}

void OPropertySet::firePropertyChangeEvent()
{
    // nothing in base class
}

// ____ XStyleSupplier ____
Reference< style::XStyle > SAL_CALL OPropertySet::getStyle()
{
    return m_xStyle;
}

void SAL_CALL OPropertySet::setStyle( const Reference< style::XStyle >& xStyle )
{
    if( ! SetStyle( xStyle ))
        throw lang::IllegalArgumentException(
            u"Empty Style"_ustr,
            static_cast< beans::XPropertySet * >( this ),
            0 );
}

// ____ XMultiPropertySet ____
void SAL_CALL OPropertySet::setPropertyValues(
    const Sequence< OUString >& PropertyNames, const Sequence< Any >& Values )
{
    ::cppu::OPropertySetHelper::setPropertyValues( PropertyNames, Values );

    firePropertyChangeEvent();
}

// ____ XFastPropertySet ____
void SAL_CALL OPropertySet::setFastPropertyValue( sal_Int32 nHandle, const Any& rValue )
{
    ::cppu::OPropertySetHelper::setFastPropertyValue( nHandle, rValue );

    firePropertyChangeEvent();
}

beans::PropertyState OPropertySet::GetPropertyStateByHandle( sal_Int32 nHandle ) const
{
    if( m_aProperties.end() == m_aProperties.find( nHandle ))
        return beans::PropertyState_DEFAULT_VALUE;
    return beans::PropertyState_DIRECT_VALUE;
}

Sequence< beans::PropertyState > OPropertySet::GetPropertyStatesByHandle(
    const std::vector< sal_Int32 > & aHandles ) const
{
    Sequence< beans::PropertyState > aResult( aHandles.size());

    std::transform( aHandles.begin(), aHandles.end(),
                      aResult.getArray(),
                      [this](sal_Int32 nHandle) { return GetPropertyStateByHandle(nHandle); });

    return aResult;
}

void OPropertySet::SetPropertyToDefault( sal_Int32 nHandle )
{
    auto aFoundIter( m_aProperties.find( nHandle ) );

    if( m_aProperties.end() != aFoundIter )
    {
        m_aProperties.erase( aFoundIter );
    }
}

void OPropertySet::SetPropertiesToDefault(
    const std::vector< sal_Int32 > & aHandles )
{
    for(auto nHandle : aHandles)
        m_aProperties.erase(nHandle);
}

void OPropertySet::SetAllPropertiesToDefault()
{
    m_aProperties.clear();
}

bool OPropertySet::GetPropertyValueByHandle(
    Any & rValue,
    sal_Int32 nHandle ) const
{
    bool bResult = false;

    auto aFoundIter( m_aProperties.find( nHandle ) );

    if( m_aProperties.end() != aFoundIter )
    {
        rValue = (*aFoundIter).second;
        bResult = true;
    }

    return bResult;
}

void OPropertySet::SetPropertyValueByHandle(
    sal_Int32 nHandle, const Any & rValue )
{
    m_aProperties[ nHandle ] = rValue;
}

bool OPropertySet::SetStyle( const Reference< style::XStyle > & xStyle )
{
    if( ! xStyle.is())
        return false;

    m_xStyle = xStyle;
    return true;
}


} //  namespace property

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
