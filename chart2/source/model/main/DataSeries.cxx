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

#include <DataSeries.hxx>
#include "DataSeriesProperties.hxx"
#include "DataPointProperties.hxx"
#include <CharacterProperties.hxx>
#include <UserDefinedProperties.hxx>
#include "DataPoint.hxx"
#include <DataSeriesHelper.hxx>
#include <CloneHelper.hxx>
#include <ModifyListenerHelper.hxx>
#include <EventListenerHelper.hxx>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/ref.hxx>

#include <algorithm>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::osl::MutexGuard;

namespace
{

struct StaticDataSeriesDefaults : public rtl::StaticWithInit< ::chart::tPropertyValueMap, StaticDataSeriesDefaults >
{
    ::chart::tPropertyValueMap operator()()
    {
        ::chart::tPropertyValueMap aStaticDefaults;
        ::chart::DataSeriesProperties::AddDefaultsToMap( aStaticDefaults );
        ::chart::CharacterProperties::AddDefaultsToMap( aStaticDefaults );
        float fDefaultCharHeight = 10.0;
        aStaticDefaults.setPropertyValue( ::chart::CharacterProperties::PROP_CHAR_CHAR_HEIGHT, fDefaultCharHeight );
        aStaticDefaults.setPropertyValue( ::chart::CharacterProperties::PROP_CHAR_ASIAN_CHAR_HEIGHT, fDefaultCharHeight );
        aStaticDefaults.setPropertyValue( ::chart::CharacterProperties::PROP_CHAR_COMPLEX_CHAR_HEIGHT, fDefaultCharHeight );
        return aStaticDefaults;
    }
};

struct StaticDataSeriesInfoHelper : public rtl::StaticWithInit< ::cppu::OPropertyArrayHelper, StaticDataSeriesInfoHelper, StaticDataSeriesInfoHelper, uno::Sequence< Property > >
{
    uno::Sequence< Property > operator()()
    {
        std::vector< css::beans::Property > aProperties;
        ::chart::DataSeriesProperties::AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }
};

struct StaticDataSeriesInfo : public rtl::StaticWithInit< uno::Reference< beans::XPropertySetInfo >, StaticDataSeriesInfo >
{
    uno::Reference< beans::XPropertySetInfo > operator()()
    {
        return ::cppu::OPropertySetHelper::createPropertySetInfo(StaticDataSeriesInfoHelper::get() );
    }
};

void lcl_SetParent(
    const uno::Reference< uno::XInterface > & xChildInterface,
    const uno::Reference< uno::XInterface > & xParentInterface )
{
    uno::Reference< container::XChild > xChild( xChildInterface, uno::UNO_QUERY );
    if( xChild.is())
        xChild->setParent( xParentInterface );
}

typedef std::map< sal_Int32, css::uno::Reference< css::beans::XPropertySet > >
    lcl_tDataPointMap;

void lcl_CloneAttributedDataPoints(
    const lcl_tDataPointMap & rSource, lcl_tDataPointMap & rDestination,
    const uno::Reference< uno::XInterface > & xSeries )
{
    for (auto const& elem : rSource)
    {
        Reference< beans::XPropertySet > xPoint( elem.second );
        if( xPoint.is())
        {
            Reference< util::XCloneable > xCloneable( xPoint, uno::UNO_QUERY );
            if( xCloneable.is())
            {
                xPoint.set( xCloneable->createClone(), uno::UNO_QUERY );
                if( xPoint.is())
                {
                    lcl_SetParent( xPoint, xSeries );
                    rDestination.emplace( elem.first, xPoint );
                }
            }
        }
    }
}

} // anonymous namespace

namespace chart
{

DataSeries::DataSeries() :
        ::property::OPropertySet( m_aMutex ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
}

DataSeries::DataSeries( const DataSeries & rOther ) :
        impl::DataSeries_Base(rOther),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    if( ! rOther.m_aDataSequences.empty())
    {
        CloneHelper::CloneRefVector<css::chart2::data::XLabeledDataSequence>(
            rOther.m_aDataSequences, m_aDataSequences );
        ModifyListenerHelper::addListenerToAllElements( m_aDataSequences, m_xModifyEventForwarder );
    }

    CloneHelper::CloneRefVector< chart2::XRegressionCurve >( rOther.m_aRegressionCurves, m_aRegressionCurves );
    ModifyListenerHelper::addListenerToAllElements( m_aRegressionCurves, m_xModifyEventForwarder );

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
}

// late initialization to call after copy-constructing
void DataSeries::Init( const DataSeries & rOther )
{
    if( ! rOther.m_aDataSequences.empty())
        EventListenerHelper::addListenerToAllElements( m_aDataSequences, this );

    Reference< uno::XInterface > xThisInterface( static_cast< ::cppu::OWeakObject * >( this ));
    if( ! rOther.m_aAttributedDataPoints.empty())
    {
        lcl_CloneAttributedDataPoints(
            rOther.m_aAttributedDataPoints, m_aAttributedDataPoints, xThisInterface );
        ModifyListenerHelper::addListenerToAllMapElements( m_aAttributedDataPoints, m_xModifyEventForwarder );
    }

    // add as parent to error bars
    Reference< beans::XPropertySet > xPropertySet;
    uno::Any aValue;

    getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_X );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        lcl_SetParent( xPropertySet, xThisInterface );

    getFastPropertyValue( aValue, DataPointProperties::PROP_DATAPOINT_ERROR_BAR_Y );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        lcl_SetParent( xPropertySet, xThisInterface );
}

DataSeries::~DataSeries()
{
    try
    {
        ModifyListenerHelper::removeListenerFromAllMapElements( m_aAttributedDataPoints, m_xModifyEventForwarder );
        ModifyListenerHelper::removeListenerFromAllElements( m_aRegressionCurves, m_xModifyEventForwarder );
        ModifyListenerHelper::removeListenerFromAllElements( m_aDataSequences, m_xModifyEventForwarder );

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
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL DataSeries::createClone()
{
    rtl::Reference<DataSeries> pNewSeries( new DataSeries( *this ));
    // do initialization that uses uno references to the clone
    pNewSeries->Init( *this );

    return pNewSeries;
}

// ____ OPropertySet ____
void DataSeries::GetDefaultValue( sal_Int32 nHandle, uno::Any& rDest ) const
{
    const tPropertyValueMap& rStaticDefaults = StaticDataSeriesDefaults::get();
    rStaticDefaults.get(nHandle, rDest);
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL DataSeries::getInfoHelper()
{
    return StaticDataSeriesInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL DataSeries::getPropertySetInfo()
{
    return StaticDataSeriesInfo::get();
}

void SAL_CALL DataSeries::getFastPropertyValue
    ( uno::Any& rValue,
      sal_Int32 nHandle ) const
{
    // special handling for get.  set is not possible for this property
    if( nHandle == DataSeriesProperties::PROP_DATASERIES_ATTRIBUTED_DATA_POINTS )
    {
        // TODO: only add those property sets that are really modified

        rValue <<= comphelper::mapKeysToSequence(m_aAttributedDataPoints);
    }
    else
        OPropertySet::getFastPropertyValue( rValue, nHandle );
}

void SAL_CALL DataSeries::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const uno::Any& rValue )
{
    if(    nHandle == DataPointProperties::PROP_DATAPOINT_ERROR_BAR_Y
        || nHandle == DataPointProperties::PROP_DATAPOINT_ERROR_BAR_X )
    {
        uno::Any aOldValue;
        Reference< util::XModifyBroadcaster > xBroadcaster;
        getFastPropertyValue( aOldValue, nHandle );
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

Reference< beans::XPropertySet >
    SAL_CALL DataSeries::getDataPointByIndex( sal_Int32 nIndex )
{
    Reference< beans::XPropertySet > xResult;

    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences;
    {
        MutexGuard aGuard( m_aMutex );
        aSequences = comphelper::containerToSequence( m_aDataSequences );
    }

    std::vector< Reference< chart2::data::XLabeledDataSequence > > aValuesSeries(
        DataSeriesHelper::getAllDataSequencesByRole( aSequences , "values" ) );

    if (aValuesSeries.empty())
        throw lang::IndexOutOfBoundsException();

    Reference< chart2::data::XDataSequence > xSeq( aValuesSeries.front()->getValues() );
    if( 0 <= nIndex && nIndex < xSeq->getData().getLength() )
    {
        {
            MutexGuard aGuard( m_aMutex );
            tDataPointAttributeContainer::iterator aIt( m_aAttributedDataPoints.find( nIndex ) );
            if( aIt != m_aAttributedDataPoints.end() )
                xResult = (*aIt).second;
        }
        if( !xResult.is() )
        {
            Reference< beans::XPropertySet > xParentProperties;
            Reference< util::XModifyListener > xModifyEventForwarder;
            {
                MutexGuard aGuard( m_aMutex );
                xParentProperties = this;
                xModifyEventForwarder = m_xModifyEventForwarder;
            }

            // create a new XPropertySet for this data point
            xResult.set( new DataPoint( xParentProperties ) );
            {
                MutexGuard aGuard( m_aMutex );
                m_aAttributedDataPoints[ nIndex ] = xResult;
            }
            ModifyListenerHelper::addListener( xResult, xModifyEventForwarder );
        }
    }

    return xResult;
}

void SAL_CALL DataSeries::resetDataPoint( sal_Int32 nIndex )
{
    Reference< beans::XPropertySet > xDataPointProp;
    Reference< util::XModifyListener > xModifyEventForwarder;
    {
        MutexGuard aGuard( m_aMutex );
        xModifyEventForwarder = m_xModifyEventForwarder;
        tDataPointAttributeContainer::iterator aIt( m_aAttributedDataPoints.find( nIndex ));
        if( aIt != m_aAttributedDataPoints.end())
        {
            xDataPointProp = (*aIt).second;
            m_aAttributedDataPoints.erase(aIt);
        }

    }
    if( xDataPointProp.is() )
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( xDataPointProp, uno::UNO_QUERY );
        if( xBroadcaster.is() && xModifyEventForwarder.is())
            xBroadcaster->removeModifyListener( xModifyEventForwarder );
        fireModifyEvent();
    }
}

void SAL_CALL DataSeries::resetAllDataPoints()
{
    tDataPointAttributeContainer  aOldAttributedDataPoints;
    Reference< util::XModifyListener > xModifyEventForwarder;
    {
        MutexGuard aGuard( m_aMutex );
        xModifyEventForwarder = m_xModifyEventForwarder;
        std::swap( aOldAttributedDataPoints, m_aAttributedDataPoints );
    }
    ModifyListenerHelper::removeListenerFromAllMapElements( aOldAttributedDataPoints, xModifyEventForwarder );
    aOldAttributedDataPoints.clear();
    fireModifyEvent();
}

// ____ XDataSink ____
void SAL_CALL DataSeries::setData( const uno::Sequence< Reference< chart2::data::XLabeledDataSequence > >& aData )
{
    tDataSequenceContainer aOldDataSequences;
    tDataSequenceContainer aNewDataSequences;
    Reference< util::XModifyListener > xModifyEventForwarder;
    Reference< lang::XEventListener > xListener;
    {
        MutexGuard aGuard( m_aMutex );
        xModifyEventForwarder = m_xModifyEventForwarder;
        xListener = this;
        std::swap( aOldDataSequences, m_aDataSequences );
        aNewDataSequences = comphelper::sequenceToContainer<tDataSequenceContainer>( aData );
        m_aDataSequences = aNewDataSequences;
    }
    ModifyListenerHelper::removeListenerFromAllElements( aOldDataSequences, xModifyEventForwarder );
    EventListenerHelper::removeListenerFromAllElements( aOldDataSequences, xListener );
    EventListenerHelper::addListenerToAllElements( aNewDataSequences, xListener );
    ModifyListenerHelper::addListenerToAllElements( aNewDataSequences, xModifyEventForwarder );
    fireModifyEvent();
}

// ____ XDataSource ____
Sequence< Reference< chart2::data::XLabeledDataSequence > > SAL_CALL DataSeries::getDataSequences()
{
    MutexGuard aGuard( m_aMutex );
    return comphelper::containerToSequence( m_aDataSequences );
}

// ____ XRegressionCurveContainer ____
void SAL_CALL DataSeries::addRegressionCurve(
    const uno::Reference< chart2::XRegressionCurve >& xRegressionCurve )
{
    Reference< util::XModifyListener > xModifyEventForwarder;
    {
        MutexGuard aGuard( m_aMutex );
        xModifyEventForwarder = m_xModifyEventForwarder;
        if( std::find( m_aRegressionCurves.begin(), m_aRegressionCurves.end(), xRegressionCurve )
            != m_aRegressionCurves.end())
            throw lang::IllegalArgumentException("curve not found", static_cast<cppu::OWeakObject*>(this), 1);
        m_aRegressionCurves.push_back( xRegressionCurve );
    }
    ModifyListenerHelper::addListener( xRegressionCurve, xModifyEventForwarder );
    fireModifyEvent();
}

void SAL_CALL DataSeries::removeRegressionCurve(
    const uno::Reference< chart2::XRegressionCurve >& xRegressionCurve )
{
    if( !xRegressionCurve.is() )
        throw container::NoSuchElementException();

    Reference< util::XModifyListener > xModifyEventForwarder;
    {
        MutexGuard aGuard( m_aMutex );
        xModifyEventForwarder = m_xModifyEventForwarder;
        tRegressionCurveContainerType::iterator aIt(
            std::find( m_aRegressionCurves.begin(), m_aRegressionCurves.end(), xRegressionCurve ) );
        if( aIt == m_aRegressionCurves.end())
            throw container::NoSuchElementException(
                "The given regression curve is no element of this series",
                static_cast< uno::XWeak * >( this ));
        m_aRegressionCurves.erase( aIt );
    }

    ModifyListenerHelper::removeListener( xRegressionCurve, xModifyEventForwarder );
    fireModifyEvent();
}

uno::Sequence< uno::Reference< chart2::XRegressionCurve > > SAL_CALL DataSeries::getRegressionCurves()
{
    MutexGuard aGuard( m_aMutex );
    return comphelper::containerToSequence( m_aRegressionCurves );
}

void SAL_CALL DataSeries::setRegressionCurves(
    const Sequence< Reference< chart2::XRegressionCurve > >& aRegressionCurves )
{
    tRegressionCurveContainerType aOldCurves;
    auto aNewCurves( comphelper::sequenceToContainer<tRegressionCurveContainerType>( aRegressionCurves ) );
    Reference< util::XModifyListener > xModifyEventForwarder;
    {
        MutexGuard aGuard( m_aMutex );
        xModifyEventForwarder = m_xModifyEventForwarder;
        std::swap( aOldCurves, m_aRegressionCurves );
        m_aRegressionCurves = aNewCurves;
    }
    ModifyListenerHelper::removeListenerFromAllElements( aOldCurves, xModifyEventForwarder );
    ModifyListenerHelper::addListenerToAllElements( aNewCurves, xModifyEventForwarder );
    fireModifyEvent();
}

// ____ XModifyBroadcaster ____
void SAL_CALL DataSeries::addModifyListener( const Reference< util::XModifyListener >& aListener )
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void SAL_CALL DataSeries::removeModifyListener( const Reference< util::XModifyListener >& aListener )
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ XModifyListener ____
void SAL_CALL DataSeries::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL DataSeries::disposing( const lang::EventObject& rEventObject )
{
    // forget disposed data sequences
    tDataSequenceContainer::iterator aIt(
        std::find( m_aDataSequences.begin(), m_aDataSequences.end(), rEventObject.Source ));
    if( aIt != m_aDataSequences.end())
        m_aDataSequences.erase( aIt );
}

// ____ OPropertySet ____
void DataSeries::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void DataSeries::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

using impl::DataSeries_Base;
using ::property::OPropertySet;

IMPLEMENT_FORWARD_XINTERFACE2( DataSeries, DataSeries_Base, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( DataSeries, DataSeries_Base, OPropertySet )

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL DataSeries::getImplementationName()
{
    return "com.sun.star.comp.chart.DataSeries";
}

sal_Bool SAL_CALL DataSeries::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL DataSeries::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart2.DataSeries",
        "com.sun.star.chart2.DataPointProperties",
        "com.sun.star.beans.PropertySet" };
}

}  // namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_DataSeries_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::DataSeries );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
