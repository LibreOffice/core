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

#include "UncachedDataSequence.hxx"
#include "macros.hxx"
#include "PropertyHelper.hxx"
#include "CommonFunctors.hxx"
#include "ModifyListenerHelper.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <algorithm>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <rtl/math.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

// necessary for MS compiler
using ::comphelper::OPropertyContainer;
using ::chart::impl::UncachedDataSequence_Base;

namespace
{
static const char lcl_aServiceName[] = "com.sun.star.comp.chart.UncachedDataSequence";

enum
{
    PROP_NUMBERFORMAT_KEY,
    PROP_PROPOSED_ROLE,
    PROP_XML_RANGE
};
}  // anonymous namespace

namespace chart
{

UncachedDataSequence::UncachedDataSequence(
    const Reference< chart2::XInternalDataProvider > & xIntDataProv,
    const OUString & rRangeRepresentation )
        : OPropertyContainer( GetBroadcastHelper()),
          UncachedDataSequence_Base( GetMutex()),
          m_nNumberFormatKey(0),
          m_xDataProvider( xIntDataProv ),
          m_aSourceRepresentation( rRangeRepresentation ),
          m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    registerProperties();
}

UncachedDataSequence::UncachedDataSequence(
    const Reference< chart2::XInternalDataProvider > & xIntDataProv,
    const OUString & rRangeRepresentation,
    const OUString & rRole )
        : OPropertyContainer( GetBroadcastHelper()),
          UncachedDataSequence_Base( GetMutex()),
          m_nNumberFormatKey(0),
          m_xDataProvider( xIntDataProv ),
          m_aSourceRepresentation( rRangeRepresentation ),
          m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    registerProperties();
    setFastPropertyValue_NoBroadcast( PROP_PROPOSED_ROLE, uno::Any( rRole ));
}

UncachedDataSequence::UncachedDataSequence( const UncachedDataSequence & rSource )
        : ::comphelper::OMutexAndBroadcastHelper(),
          OPropertyContainer( GetBroadcastHelper()),
          ::comphelper::OPropertyArrayUsageHelper< UncachedDataSequence >(),
          UncachedDataSequence_Base( GetMutex()),
          m_nNumberFormatKey( rSource.m_nNumberFormatKey ),
          m_sRole( rSource.m_sRole ),
          m_xDataProvider( rSource.m_xDataProvider ),
          m_aSourceRepresentation( rSource.m_aSourceRepresentation ),
          m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    registerProperties();
}

UncachedDataSequence::~UncachedDataSequence()
{}

void UncachedDataSequence::registerProperties()
{
    registerProperty( "NumberFormatKey",
                      PROP_NUMBERFORMAT_KEY,
                      0,   // PropertyAttributes
                      & m_nNumberFormatKey,
                      cppu::UnoType<decltype(m_nNumberFormatKey)>::get() );

    registerProperty( "Role",
                      PROP_PROPOSED_ROLE,
                      0,   // PropertyAttributes
                      & m_sRole,
                      cppu::UnoType<decltype(m_sRole)>::get() );

    registerProperty( "CachedXMLRange",
                      PROP_XML_RANGE,
                      0,   // PropertyAttributes
                      & m_aXMLRange,
                      cppu::UnoType<decltype(m_aXMLRange)>::get() );
}

IMPLEMENT_FORWARD_XINTERFACE2( UncachedDataSequence, UncachedDataSequence_Base, OPropertyContainer )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( UncachedDataSequence, UncachedDataSequence_Base, OPropertyContainer )

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL UncachedDataSequence::getPropertySetInfo()
{
    return Reference< beans::XPropertySetInfo >( createPropertySetInfo( getInfoHelper() ) );
}

// ____ ::comphelper::OPropertySetHelper ____
::cppu::IPropertyArrayHelper& UncachedDataSequence::getInfoHelper()
{
    return *getArrayHelper();
}

// ____ ::comphelper::OPropertyArrayHelper ____
::cppu::IPropertyArrayHelper* UncachedDataSequence::createArrayHelper() const
{
    Sequence< beans::Property > aProps;
    // describes all properties which have been registered in the ctor
    describeProperties( aProps );

    return new ::cppu::OPropertyArrayHelper( aProps );
}

OUString SAL_CALL UncachedDataSequence::getImplementationName()
{
    return OUString(lcl_aServiceName);
}

sal_Bool SAL_CALL UncachedDataSequence::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL UncachedDataSequence::getSupportedServiceNames()
{
    return {
        lcl_aServiceName,
        "com.sun.star.chart2.data.DataSequence",
        "com.sun.star.chart2.data.NumericalDataSequence",
        "com.sun.star.chart2.data.TextualDataSequence"
    };
}

// ________ XNumericalDataSequence ________
Sequence< double > SAL_CALL UncachedDataSequence::getNumericalData()
{
    Sequence< double > aResult;
    MutexGuard aGuard( GetMutex() );
    if( m_xDataProvider.is())
    {
        Sequence< uno::Any > aValues( m_xDataProvider->getDataByRangeRepresentation( m_aSourceRepresentation ));
        aResult.realloc( aValues.getLength());
        ::std::transform( aValues.getConstArray(), aValues.getConstArray() + aValues.getLength(),
                          aResult.getArray(), CommonFunctors::AnyToDouble());
    }
    return aResult;
}

// ________ XTextualDataSequence ________
Sequence< OUString > SAL_CALL UncachedDataSequence::getTextualData()
{
    Sequence< OUString > aResult;
    MutexGuard aGuard( GetMutex() );
    if( m_xDataProvider.is())
    {
        Sequence< uno::Any > aValues( m_xDataProvider->getDataByRangeRepresentation( m_aSourceRepresentation ));
        aResult.realloc( aValues.getLength());
        ::std::transform( aValues.getConstArray(), aValues.getConstArray() + aValues.getLength(),
                          aResult.getArray(), CommonFunctors::AnyToString());
    }
    return aResult;
}

// ________ XDataSequence  ________
Sequence< Any > SAL_CALL UncachedDataSequence::getData()
{
    MutexGuard aGuard( GetMutex() );
    if( m_xDataProvider.is())
        return m_xDataProvider->getDataByRangeRepresentation( m_aSourceRepresentation );
    return Sequence< Any >();
}

OUString SAL_CALL UncachedDataSequence::getSourceRangeRepresentation()
{
    return getName();
}

Sequence< OUString > SAL_CALL UncachedDataSequence::generateLabel( chart2::data::LabelOrigin )
{
    // auto-generated label is an empty string
    return Sequence< OUString >(1);
}

::sal_Int32 SAL_CALL UncachedDataSequence::getNumberFormatKeyByIndex( ::sal_Int32 )
{
    return m_nNumberFormatKey;
}

// ____ XIndexReplace ____
void SAL_CALL UncachedDataSequence::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element )
{
    MutexGuard aGuard( GetMutex() );
    Sequence< Any > aData( getData());
    if( Index < aData.getLength() &&
        m_xDataProvider.is() )
    {
        aData[Index] = Element;
        m_xDataProvider->setDataByRangeRepresentation( m_aSourceRepresentation, aData );
        fireModifyEvent();
    }
}

// ____ XIndexAccess (base of XIndexReplace) ____
::sal_Int32 SAL_CALL UncachedDataSequence::getCount()
{
    OSL_FAIL( "Implement!" );
    return 0;
}

uno::Any SAL_CALL UncachedDataSequence::getByIndex( ::sal_Int32 )
{
    OSL_FAIL( "Implement!" );
    return uno::Any();
}

// ____ XElementAccess (base of XIndexAccess) ____
uno::Type SAL_CALL UncachedDataSequence::getElementType()
{
    return cppu::UnoType<uno::Any>::get();
}

sal_Bool SAL_CALL UncachedDataSequence::hasElements()
{
    if( ! m_xDataProvider.is())
        return false;
    return m_xDataProvider->hasDataByRangeRepresentation( m_aSourceRepresentation );
}

// ____ XNamed ____
OUString SAL_CALL UncachedDataSequence::getName()
{
    return m_aSourceRepresentation;
}

void SAL_CALL UncachedDataSequence::setName( const OUString& aName )
{
    m_aSourceRepresentation = aName;
    fireModifyEvent();
}

Reference< util::XCloneable > SAL_CALL UncachedDataSequence::createClone()
{
    UncachedDataSequence * pNewSeq = new UncachedDataSequence( *this );
    return Reference< util::XCloneable >( pNewSeq );
}

// ____ XModifiable ____
sal_Bool SAL_CALL UncachedDataSequence::isModified()
{
    return false;
}

void SAL_CALL UncachedDataSequence::setModified( sal_Bool bModified )
{
    if( bModified )
        fireModifyEvent();
}

// ____ XModifyBroadcaster (base of XModifiable) ____
void SAL_CALL UncachedDataSequence::addModifyListener( const Reference< util::XModifyListener >& aListener )
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL UncachedDataSequence::removeModifyListener( const Reference< util::XModifyListener >& aListener )
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void UncachedDataSequence::fireModifyEvent()
{
    // @todo: currently never called, as data changes are not yet reported by
    // the data provider
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
