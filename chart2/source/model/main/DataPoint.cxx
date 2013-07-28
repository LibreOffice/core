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

#include "DataPoint.hxx"
#include "DataPointProperties.hxx"
#include "CharacterProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "ContainerHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{

struct StaticDataPointInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        ::chart::DataPointProperties::AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }
};

struct StaticDataPointInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticDataPointInfoHelper_Initializer >
{
};

struct StaticDataPointInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticDataPointInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticDataPointInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticDataPointInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

DataPoint::DataPoint( const uno::Reference< beans::XPropertySet > & rParentProperties ) :
        ::property::OPropertySet( m_aMutex ),
        m_xParentProperties( rParentProperties ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder()),
        m_bNoParentPropAllowed( false )
{
    SetNewValuesExplicitlyEvenIfTheyEqualDefault();
}

DataPoint::DataPoint( const DataPoint & rOther ) :
        MutexContainer(),
        impl::DataPoint_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder()),
        m_bNoParentPropAllowed( true )
{
    SetNewValuesExplicitlyEvenIfTheyEqualDefault();

    // m_xParentProperties has to be set from outside, like in the method
    // DataSeries::createClone

    // add as listener to XPropertySet properties
    Reference< beans::XPropertySet > xPropertySet;
    uno::Any aValue;

    getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_X );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        ModifyListenerHelper::addListener( xPropertySet, m_xModifyEventForwarder );

    getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_Y );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        ModifyListenerHelper::addListener( xPropertySet, m_xModifyEventForwarder );

    m_bNoParentPropAllowed = false;
}

DataPoint::~DataPoint()
{
    try
    {
        // remove listener from XPropertySet properties
        Reference< beans::XPropertySet > xPropertySet;
        uno::Any aValue;

        getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_X );
        if( ( aValue >>= xPropertySet )
            && xPropertySet.is())
            ModifyListenerHelper::removeListener( xPropertySet, m_xModifyEventForwarder );

        getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_Y );
        if( ( aValue >>= xPropertySet )
            && xPropertySet.is())
            ModifyListenerHelper::removeListener( xPropertySet, m_xModifyEventForwarder );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL DataPoint::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new DataPoint( *this ));
}

// ____ XChild ____
Reference< uno::XInterface > SAL_CALL DataPoint::getParent()
    throw (uno::RuntimeException)
{
    return Reference< uno::XInterface >( m_xParentProperties.get(), uno::UNO_QUERY );
}

void SAL_CALL DataPoint::setParent(
    const Reference< uno::XInterface >& Parent )
    throw (lang::NoSupportException,
           uno::RuntimeException)
{
    m_xParentProperties = Reference< beans::XPropertySet >( Parent, uno::UNO_QUERY );
}

// ____ OPropertySet ____
uno::Any DataPoint::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    // the value set at the data series is the default
    uno::Reference< beans::XFastPropertySet > xFast( m_xParentProperties.get(), uno::UNO_QUERY );
    if( !xFast.is())
    {
        OSL_ENSURE( m_bNoParentPropAllowed, "data point needs a parent property set to provide values correctly" );
        return uno::Any();
    }

    return xFast->getFastPropertyValue( nHandle );
}

void SAL_CALL DataPoint::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const uno::Any& rValue )
    throw (uno::Exception)
{
    if(    nHandle == DataPointProperties::PROP_DATAPOINT_ERROR_BAR_Y
        || nHandle == DataPointProperties::PROP_DATAPOINT_ERROR_BAR_X )
    {
        uno::Any aOldValue;
        Reference< util::XModifyBroadcaster > xBroadcaster;
        this->getFastPropertyValue( aOldValue, nHandle );
        if( aOldValue.hasValue() &&
            (aOldValue >>= xBroadcaster) &&
            xBroadcaster.is())
        {
            ModifyListenerHelper::removeListener( xBroadcaster, m_xModifyEventForwarder );
        }

        OSL_ASSERT( rValue.getValueType().getTypeClass() == uno::TypeClass_INTERFACE );
        if( rValue.hasValue() &&
            (rValue >>= xBroadcaster) &&
            xBroadcaster.is())
        {
            ModifyListenerHelper::addListener( xBroadcaster, m_xModifyEventForwarder );
        }
    }

    ::property::OPropertySet::setFastPropertyValue_NoBroadcast( nHandle, rValue );
}

::cppu::IPropertyArrayHelper & SAL_CALL DataPoint::getInfoHelper()
{
    return *StaticDataPointInfoHelper::get();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL DataPoint::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticDataPointInfo::get();
}

// ____ XModifyBroadcaster ____
void SAL_CALL DataPoint::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL DataPoint::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XModifyListener ____
void SAL_CALL DataPoint::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL DataPoint::disposing( const lang::EventObject& )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ OPropertySet ____
void DataPoint::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void DataPoint::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

Sequence< OUString > DataPoint::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 3 );
    aServices[ 0 ] = "com.sun.star.chart2.DataPoint";
    aServices[ 1 ] = "com.sun.star.chart2.DataPointProperties";
    aServices[ 2 ] = "com.sun.star.beans.PropertySet";
    return aServices;
}

// needed by MSC compiler
using impl::DataPoint_Base;

IMPLEMENT_FORWARD_XINTERFACE2( DataPoint, DataPoint_Base, ::property::OPropertySet )

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( DataPoint, OUString("com.sun.star.comp.chart.DataPoint") );

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
