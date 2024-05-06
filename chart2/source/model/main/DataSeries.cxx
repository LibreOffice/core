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
#include <DataSeriesProperties.hxx>
#include "DataPointProperties.hxx"
#include <CharacterProperties.hxx>
#include <UserDefinedProperties.hxx>
#include "DataPoint.hxx"
#include <DataSeriesHelper.hxx>
#include <CloneHelper.hxx>
#include <RegressionCurveModel.hxx>
#include <ModifyListenerHelper.hxx>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

#include <algorithm>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::osl::MutexGuard;

namespace chart
{
const ::chart::tPropertyValueMap & StaticDataSeriesDefaults()
{
    static const ::chart::tPropertyValueMap aStaticDefaults = []()
        {
            ::chart::tPropertyValueMap aMap;
            ::chart::DataSeriesProperties::AddDefaultsToMap( aMap );
            ::chart::CharacterProperties::AddDefaultsToMap( aMap );
            float fDefaultCharHeight = 10.0;
            ::chart::PropertyHelper::setPropertyValue( aMap, ::chart::CharacterProperties::PROP_CHAR_CHAR_HEIGHT, fDefaultCharHeight );
            ::chart::PropertyHelper::setPropertyValue( aMap, ::chart::CharacterProperties::PROP_CHAR_ASIAN_CHAR_HEIGHT, fDefaultCharHeight );
            ::chart::PropertyHelper::setPropertyValue( aMap, ::chart::CharacterProperties::PROP_CHAR_COMPLEX_CHAR_HEIGHT, fDefaultCharHeight );
            return aMap;
        }();
    return aStaticDefaults;
};
} // namespace chart

namespace
{

::cppu::OPropertyArrayHelper& StaticDataSeriesInfoHelper()
{
    static ::cppu::OPropertyArrayHelper oHelper = []()
        {
            std::vector< css::beans::Property > aProperties;
            ::chart::DataSeriesProperties::AddPropertiesToVector( aProperties );
            ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
            ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return oHelper;
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
        m_xModifyEventForwarder( new ModifyEventForwarder() )
{
}

DataSeries::DataSeries( const DataSeries & rOther ) :
        impl::DataSeries_Base(rOther),
        ::property::OPropertySet( rOther ),
    m_xModifyEventForwarder( new ModifyEventForwarder() )
{
    if( ! rOther.m_aDataSequences.empty())
    {
        CloneHelper::CloneRefVector(rOther.m_aDataSequences, m_aDataSequences );
        ModifyListenerHelper::addListenerToAllElements( m_aDataSequences, m_xModifyEventForwarder );
    }

    CloneHelper::CloneRefVector( rOther.m_aRegressionCurves, m_aRegressionCurves );
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
    const tPropertyValueMap& rStaticDefaults = StaticDataSeriesDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rDest.clear();
    else
        rDest = (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL DataSeries::getInfoHelper()
{
    return StaticDataSeriesInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL DataSeries::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropSetInfo =
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticDataSeriesInfoHelper() );
    return xPropSetInfo;
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

    std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences;
    {
        MutexGuard aGuard( m_aMutex );
        aSequences = m_aDataSequences;
    }

    std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aValuesSeries(
        DataSeriesHelper::getAllDataSequencesByRole( aSequences , u"values"_ustr ) );

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
    {
        MutexGuard aGuard( m_aMutex );
        xModifyEventForwarder = m_xModifyEventForwarder;
        std::swap( aOldDataSequences, m_aDataSequences );
        for (const auto & i : aData)
        {
            aNewDataSequences.push_back(i);
        }
        m_aDataSequences = aNewDataSequences;
    }
    ModifyListenerHelper::removeListenerFromAllElements( aOldDataSequences, xModifyEventForwarder );
    ModifyListenerHelper::addListenerToAllElements( aNewDataSequences, xModifyEventForwarder );
    fireModifyEvent();
}

void DataSeries::setData( const std::vector< uno::Reference< chart2::data::XLabeledDataSequence > >& aData )
{
    tDataSequenceContainer aOldDataSequences;
    tDataSequenceContainer aNewDataSequences;
    Reference< util::XModifyListener > xModifyEventForwarder;
    {
        MutexGuard aGuard( m_aMutex );
        xModifyEventForwarder = m_xModifyEventForwarder;
        std::swap( aOldDataSequences, m_aDataSequences );
        aNewDataSequences = aData;
        m_aDataSequences = aNewDataSequences;
    }
    ModifyListenerHelper::removeListenerFromAllElements( aOldDataSequences, xModifyEventForwarder );
    ModifyListenerHelper::addListenerToAllElements( aNewDataSequences, xModifyEventForwarder );
    fireModifyEvent();
}

// ____ XDataSource ____
Sequence< Reference< chart2::data::XLabeledDataSequence > > SAL_CALL DataSeries::getDataSequences()
{
    MutexGuard aGuard( m_aMutex );
    return comphelper::containerToSequence<Reference< chart2::data::XLabeledDataSequence >>( m_aDataSequences );
}

// ____ XRegressionCurveContainer ____
void SAL_CALL DataSeries::addRegressionCurve(
    const uno::Reference< chart2::XRegressionCurve >& xRegressionCurve )
{
    auto pRegressionCurve = dynamic_cast<RegressionCurveModel*>(xRegressionCurve.get());
    assert(pRegressionCurve);
    Reference< util::XModifyListener > xModifyEventForwarder;
    {
        MutexGuard aGuard( m_aMutex );
        xModifyEventForwarder = m_xModifyEventForwarder;
        if( std::find( m_aRegressionCurves.begin(), m_aRegressionCurves.end(), pRegressionCurve )
            != m_aRegressionCurves.end())
            throw lang::IllegalArgumentException(u"curve not found"_ustr, static_cast<cppu::OWeakObject*>(this), 1);
        m_aRegressionCurves.push_back( pRegressionCurve );
    }
    ModifyListenerHelper::addListener( rtl::Reference<RegressionCurveModel>(pRegressionCurve), xModifyEventForwarder );
    fireModifyEvent();
}

void SAL_CALL DataSeries::removeRegressionCurve(
    const uno::Reference< chart2::XRegressionCurve >& xRegressionCurve )
{
    if( !xRegressionCurve.is() )
        throw container::NoSuchElementException();
    auto pRegressionCurve = dynamic_cast<RegressionCurveModel*>(xRegressionCurve.get());
    assert(pRegressionCurve);

    Reference< util::XModifyListener > xModifyEventForwarder;
    {
        MutexGuard aGuard( m_aMutex );
        xModifyEventForwarder = m_xModifyEventForwarder;
        tRegressionCurveContainerType::iterator aIt(
            std::find( m_aRegressionCurves.begin(), m_aRegressionCurves.end(), pRegressionCurve ) );
        if( aIt == m_aRegressionCurves.end())
            throw container::NoSuchElementException(
                u"The given regression curve is no element of this series"_ustr,
                static_cast< uno::XWeak * >( this ));
        m_aRegressionCurves.erase( aIt );
    }

    ModifyListenerHelper::removeListener( rtl::Reference<RegressionCurveModel>(pRegressionCurve), xModifyEventForwarder );
    fireModifyEvent();
}

uno::Sequence< uno::Reference< chart2::XRegressionCurve > > SAL_CALL DataSeries::getRegressionCurves()
{
    MutexGuard aGuard( m_aMutex );
    return comphelper::containerToSequence<uno::Reference< chart2::XRegressionCurve >>( m_aRegressionCurves );
}

void SAL_CALL DataSeries::setRegressionCurves(
    const Sequence< Reference< chart2::XRegressionCurve > >& aRegressionCurves )
{
    tRegressionCurveContainerType aOldCurves;
    tRegressionCurveContainerType aNewCurves;
    for (const auto & i : aRegressionCurves)
    {
        auto pRegressionCurve = dynamic_cast<RegressionCurveModel*>(i.get());
        assert(pRegressionCurve);
        aNewCurves.push_back(pRegressionCurve);
    }
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
    m_xModifyEventForwarder->addModifyListener( aListener );
}

void SAL_CALL DataSeries::removeModifyListener( const Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->removeModifyListener( aListener );
}

// ____ XModifyListener ____
void SAL_CALL DataSeries::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL DataSeries::disposing( const lang::EventObject& )
{
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
    return u"com.sun.star.comp.chart.DataSeries"_ustr;
}

sal_Bool SAL_CALL DataSeries::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL DataSeries::getSupportedServiceNames()
{
    return {
        u"com.sun.star.chart2.DataSeries"_ustr,
        u"com.sun.star.chart2.DataPointProperties"_ustr,
        u"com.sun.star.beans.PropertySet"_ustr };
}

static Reference< chart2::data::XLabeledDataSequence > lcl_findLSequenceWithOnlyLabel(
    const Sequence< Reference< chart2::data::XLabeledDataSequence > > & rDataSequences )
{
    Reference< chart2::data::XLabeledDataSequence > xResult;

    for( auto const & labeledData : rDataSequences )
    {
        OSL_ENSURE( labeledData.is(), "empty LabeledDataSequence" );
        // no values are set but a label exists
        if( labeledData.is() &&
            ( ! labeledData->getValues().is() &&
              labeledData->getLabel().is()))
        {
            xResult.set( labeledData );
            break;
        }
    }

    return xResult;
}

static OUString lcl_getDataSequenceLabel( const Reference< chart2::data::XDataSequence > & xSequence )
{
    OUString aResult;

    Reference< chart2::data::XTextualDataSequence > xTextSeq( xSequence, uno::UNO_QUERY );
    if( xTextSeq.is())
    {
        Sequence< OUString > aSeq( xTextSeq->getTextualData());

        const sal_Int32 nMax = aSeq.getLength() - 1;
        OUStringBuffer aBuf;

        for( sal_Int32 i = 0; i <= nMax; ++i )
        {
            aBuf.append( aSeq[i] );
            if( i < nMax )
                aBuf.append( ' ');
        }
        aResult = aBuf.makeStringAndClear();
    }
    else if( xSequence.is())
    {
        Sequence< uno::Any > aSeq( xSequence->getData());

        const sal_Int32 nMax = aSeq.getLength() - 1;
        OUString aVal;
        OUStringBuffer aBuf;
        double fNum = 0;

        for( sal_Int32 i = 0; i <= nMax; ++i )
        {
            if( aSeq[i] >>= aVal )
            {
                aBuf.append( aVal );
                if( i < nMax )
                    aBuf.append(  ' ');
            }
            else if( aSeq[ i ] >>= fNum )
            {
                aBuf.append( fNum );
                if( i < nMax )
                    aBuf.append( ' ');
            }
        }
        aResult = aBuf.makeStringAndClear();
    }

    return aResult;
}

static OUString getLabelForLabeledDataSequence(
    const Reference< chart2::data::XLabeledDataSequence > & xLabeledSeq )
{
    OUString aResult;
    if( xLabeledSeq.is())
    {
        Reference< chart2::data::XDataSequence > xSeq( xLabeledSeq->getLabel());
        if( xSeq.is() )
            aResult = lcl_getDataSequenceLabel( xSeq );
        if( !xSeq.is() || aResult.isEmpty() )
        {
            // no label set or label content is empty -> use auto-generated one
            Reference< chart2::data::XDataSequence > xValueSeq( xLabeledSeq->getValues() );
            if( xValueSeq.is() )
            {
                Sequence< OUString > aLabels( xValueSeq->generateLabel(
                    chart2::data::LabelOrigin_SHORT_SIDE ) );
                // no labels returned is interpreted as: auto-generation not
                // supported by sequence
                if( aLabels.hasElements() )
                    aResult=aLabels[0];
                else
                {
                    //todo?: maybe use the index of the series as name
                    //but as the index may change it would be better to have such a name persistent
                    //what is not possible at the moment
                    //--> maybe use the identifier as part of the name ...
                    aResult = lcl_getDataSequenceLabel( xValueSeq );
                }
            }
        }
    }
    return aResult;
}

OUString DataSeries::getLabelForRole( const OUString & rLabelSequenceRole )
{
    OUString aResult;

    Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
        ::chart::DataSeriesHelper::getDataSequenceByRole( this, rLabelSequenceRole ));
    if( xLabeledSeq.is())
        aResult = getLabelForLabeledDataSequence( xLabeledSeq );
    else
    {
        // special case: labeled data series with only a label and no values may
        // serve as label
        xLabeledSeq.set( lcl_findLSequenceWithOnlyLabel( getDataSequences() ));
        if( xLabeledSeq.is())
        {
            Reference< chart2::data::XDataSequence > xSeq( xLabeledSeq->getLabel());
            if( xSeq.is())
                aResult = lcl_getDataSequenceLabel( xSeq );
        }
    }

    return aResult;
}

static bool lcl_SequenceHasUnhiddenData( const uno::Reference< chart2::data::XDataSequence >& xDataSequence )
{
    if (!xDataSequence.is())
        return false;
    uno::Reference< beans::XPropertySet > xProp( xDataSequence, uno::UNO_QUERY );
    if( xProp.is() )
    {
        uno::Sequence< sal_Int32 > aHiddenValues;
        try
        {
            xProp->getPropertyValue( u"HiddenValues"_ustr ) >>= aHiddenValues;
            if( !aHiddenValues.hasElements() )
                return true;
        }
        catch( const uno::Exception& )
        {
            return true;
        }
    }
    return xDataSequence->getData().hasElements();
}

bool DataSeries::hasUnhiddenData()
{
    MutexGuard aGuard( m_aMutex );

    for(uno::Reference< chart2::data::XLabeledDataSequence > const & rDataSequence : m_aDataSequences)
    {
        if( !rDataSequence.is() )
            continue;
        if( lcl_SequenceHasUnhiddenData( rDataSequence->getValues() ) )
            return true;
    }
    return false;
}


}  // namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_DataSeries_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::DataSeries );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
