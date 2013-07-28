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

#include "OPropertySet.hxx"
#include "ImplOPropertySet.hxx"
#include "ContainerHelper.hxx"
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/servicehelper.hxx>

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::style::XStyleSupplier;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

// needed for MS compiler
using ::cppu::OBroadcastHelper;
using ::cppu::OPropertySetHelper;
using ::cppu::OWeakObject;

namespace property
{

OPropertySet::OPropertySet( ::osl::Mutex & par_rMutex ) :
        OBroadcastHelper( par_rMutex ),
        // the following causes a warning; there seems to be no way to avoid it
        OPropertySetHelper( static_cast< OBroadcastHelper & >( *this )),
        m_rMutex( par_rMutex ),
        m_pImplProperties( new impl::ImplOPropertySet() ),
        m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault(false)
{
}

OPropertySet::OPropertySet( const OPropertySet & rOther, ::osl::Mutex & par_rMutex ) :
        OBroadcastHelper( par_rMutex ),
        // the following causes a warning; there seems to be no way to avoid it
        OPropertySetHelper( static_cast< OBroadcastHelper & >( *this )),
        m_rMutex( par_rMutex ),
        m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault(false)
{
    MutexGuard aGuard( m_rMutex );
    if( rOther.m_pImplProperties.get())
        m_pImplProperties.reset( new impl::ImplOPropertySet( * rOther.m_pImplProperties.get()));
}

void OPropertySet::SetNewValuesExplicitlyEvenIfTheyEqualDefault()
{
    m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault = true;
}

OPropertySet::~OPropertySet()
{}

void OPropertySet::disposePropertySet()
{
    m_pImplProperties.reset( 0 );
}

Any SAL_CALL OPropertySet::queryInterface( const uno::Type& aType )
    throw (uno::RuntimeException)
{
    return ::cppu::queryInterface(
        aType,
        static_cast< lang::XTypeProvider * >( this ),
        static_cast< beans::XPropertySet * >( this ),
        static_cast< beans::XMultiPropertySet * >( this ),
        static_cast< beans::XFastPropertySet * >( this ),
        static_cast< beans::XPropertyState * >( this ),
        static_cast< beans::XMultiPropertyStates * >( this ),
        static_cast< XStyleSupplier * >( this ) );
}

#define LCL_PROP_CPPUTYPE(t) (::getCppuType( reinterpret_cast< const Reference<t> *>(0)))

// // ____ XTypeProvider ____
Sequence< uno::Type > SAL_CALL
    OPropertySet::getTypes()
    throw (uno::RuntimeException)
{
    static Sequence< uno::Type > aTypeList;

    MutexGuard aGuard( m_rMutex );

    if( aTypeList.getLength() == 0 )
    {
        ::std::vector< uno::Type > aTypes;

        aTypes.push_back( LCL_PROP_CPPUTYPE( lang::XTypeProvider ));
        aTypes.push_back( LCL_PROP_CPPUTYPE( beans::XPropertySet ));
        aTypes.push_back( LCL_PROP_CPPUTYPE( beans::XMultiPropertySet ));
        aTypes.push_back( LCL_PROP_CPPUTYPE( beans::XFastPropertySet ));
        aTypes.push_back( LCL_PROP_CPPUTYPE( beans::XPropertyState ));
        aTypes.push_back( LCL_PROP_CPPUTYPE( beans::XMultiPropertyStates ));
        aTypes.push_back( LCL_PROP_CPPUTYPE( XStyleSupplier ));

        aTypeList = ::chart::ContainerHelper::ContainerToSequence( aTypes );
    }

    return aTypeList;
}

namespace
{
    class theOPropertySetImplementationId : public rtl::Static< UnoTunnelIdInit, theOPropertySetImplementationId > {};
}

Sequence< sal_Int8 > SAL_CALL
    OPropertySet::getImplementationId()
    throw (uno::RuntimeException)
{
    return theOPropertySetImplementationId::get().getSeq();
}

// ____ XPropertyState ____
beans::PropertyState SAL_CALL
    OPropertySet::getPropertyState( const OUString& PropertyName )
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    return m_pImplProperties->GetPropertyStateByHandle(
        rPH.getHandleByName( PropertyName ));
}

Sequence< beans::PropertyState > SAL_CALL
    OPropertySet::getPropertyStates( const Sequence< OUString >& aPropertyName )
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    sal_Int32 * pHandles = new sal_Int32[ aPropertyName.getLength() ];
    rPH.fillHandles( pHandles, aPropertyName );

    ::std::vector< sal_Int32 > aHandles( pHandles, pHandles + aPropertyName.getLength());
    delete[] pHandles;

    return m_pImplProperties->GetPropertyStatesByHandle( aHandles );
}

void SAL_CALL
    OPropertySet::setPropertyToDefault( const OUString& PropertyName )
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    m_pImplProperties->SetPropertyToDefault( rPH.getHandleByName( PropertyName ));
    firePropertyChangeEvent();
}

Any SAL_CALL
    OPropertySet::getPropertyDefault( const OUString& aPropertyName )
    throw (beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    return GetDefaultValue( rPH.getHandleByName( aPropertyName ) );
}

// ____ XMultiPropertyStates ____

// Note: getPropertyStates() is already implemented in XPropertyState with the
// same signature

void SAL_CALL
    OPropertySet::setAllPropertiesToDefault()
    throw (uno::RuntimeException)
{
    m_pImplProperties->SetAllPropertiesToDefault();
    firePropertyChangeEvent();
}

void SAL_CALL
    OPropertySet::setPropertiesToDefault( const Sequence< OUString >& aPropertyNames )
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    sal_Int32 * pHandles = new sal_Int32[ aPropertyNames.getLength() ];
    rPH.fillHandles( pHandles, aPropertyNames );

    ::std::vector< sal_Int32 > aHandles( pHandles, pHandles + aPropertyNames.getLength());
    delete[] pHandles;

    m_pImplProperties->SetPropertiesToDefault( aHandles );
}

Sequence< Any > SAL_CALL
    OPropertySet::getPropertyDefaults( const Sequence< OUString >& aPropertyNames )
    throw (beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    ::cppu::IPropertyArrayHelper & rPH = getInfoHelper();
    const sal_Int32 nElements = aPropertyNames.getLength();

    Sequence< Any > aResult( nElements );
    Any * pResultArray = aResult.getArray();
    sal_Int32 nI = 0;

    for( ; nI < nElements; ++nI )
    {
        pResultArray[ nI ] = GetDefaultValue(
            rPH.getHandleByName( aPropertyNames[ nI ] ));
    }

    return aResult;
}

sal_Bool SAL_CALL OPropertySet::convertFastPropertyValue
    ( Any & rConvertedValue,
      Any & rOldValue,
      sal_Int32 nHandle,
      const Any& rValue )
    throw (lang::IllegalArgumentException)
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
                rConvertedValue = uno::makeAny( static_cast<sal_Int16>(n32Value) );
                return sal_True;
            }

            sal_Int64 n64Value = 0;
            if( rValue>>=n64Value )
            {
                rConvertedValue = uno::makeAny( static_cast<sal_Int16>(n64Value) );
                return sal_True;
            }
        }
    }
    rConvertedValue = rValue;
    if( !m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault && rOldValue == rConvertedValue )
        return sal_False;//no change necessary
    return sal_True;
}

void SAL_CALL OPropertySet::setFastPropertyValue_NoBroadcast
    ( sal_Int32 nHandle,
      const Any& rValue )
    throw (uno::Exception)
{
#if OSL_DEBUG_LEVEL > 0
    if( rValue.hasValue())
    {
        cppu::IPropertyArrayHelper & rPH = getInfoHelper();
        OUString aName;
        rPH.fillPropertyMembersByHandle( &aName, 0, nHandle );
        OSL_ENSURE( rValue.isExtractableTo( rPH.getPropertyByName( aName ).Type ),
                    "Property type is wrong" );
    }
#endif

    Any aDefault;
    try
    {
        aDefault = GetDefaultValue( nHandle );
    }
    catch( const beans::UnknownPropertyException& )
    {
        aDefault.clear();
    }
    m_pImplProperties->SetPropertyValueByHandle( nHandle, rValue );
    if( !m_bSetNewValuesExplicitlyEvenIfTheyEqualDefault && aDefault.hasValue() && aDefault == rValue ) //#i98893# don't export defaults to file
        m_pImplProperties->SetPropertyToDefault( nHandle );
    else
        m_pImplProperties->SetPropertyValueByHandle( nHandle, rValue );
}

void SAL_CALL OPropertySet::getFastPropertyValue
    ( Any& rValue,
      sal_Int32 nHandle ) const
{
    if( ! m_pImplProperties->GetPropertyValueByHandle( rValue, nHandle ))
    {
        // property was not set -> try style
        uno::Reference< beans::XFastPropertySet > xStylePropSet( m_pImplProperties->GetStyle(), uno::UNO_QUERY );
        if( xStylePropSet.is() )
        {
#ifdef DBG_UTIL
            {
                // check if the handle of the style points to the same property
                // name as the handle in this property set
                uno::Reference< beans::XPropertySet > xPropSet( xStylePropSet, uno::UNO_QUERY );
                if( xPropSet.is())
                {
                    uno::Reference< beans::XPropertySetInfo > xInfo( xPropSet->getPropertySetInfo(),
                                                                     uno::UNO_QUERY );
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
                rValue = GetDefaultValue( nHandle );
            }
            catch( const beans::UnknownPropertyException& )
            {
                rValue.clear();
            }
        }
    }
}

void OPropertySet::firePropertyChangeEvent()
{
    // nothing in base class
}

// ____ XStyleSupplier ____
Reference< style::XStyle > SAL_CALL OPropertySet::getStyle()
    throw (uno::RuntimeException)
{
    return m_pImplProperties->GetStyle();
}

void SAL_CALL OPropertySet::setStyle( const Reference< style::XStyle >& xStyle )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( ! m_pImplProperties->SetStyle( xStyle ))
        throw lang::IllegalArgumentException(
            "Empty Style",
            static_cast< beans::XPropertySet * >( this ),
            0 );
}

// ____ XMultiPropertySet ____
void SAL_CALL OPropertySet::setPropertyValues(
    const Sequence< OUString >& PropertyNames, const Sequence< Any >& Values )
    throw(beans::PropertyVetoException,
          lang::IllegalArgumentException,
          lang::WrappedTargetException,
          uno::RuntimeException)
{
    ::cppu::OPropertySetHelper::setPropertyValues( PropertyNames, Values );

    firePropertyChangeEvent();
}

// ____ XFastPropertySet ____
void SAL_CALL OPropertySet::setFastPropertyValue( sal_Int32 nHandle, const Any& rValue )
    throw(beans::UnknownPropertyException,
          beans::PropertyVetoException,
          lang::IllegalArgumentException,
          lang::WrappedTargetException, uno::RuntimeException)
{
    ::cppu::OPropertySetHelper::setFastPropertyValue( nHandle, rValue );

    firePropertyChangeEvent();
}

} //  namespace property

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
