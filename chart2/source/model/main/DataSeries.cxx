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
#include "precompiled_chart2.hxx"
#include "DataSeries.hxx"
#include "DataSeriesProperties.hxx"
#include "DataPointProperties.hxx"
#include "CharacterProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "DataPoint.hxx"
#include "macros.hxx"
#include "DataSeriesHelper.hxx"
#include "ContainerHelper.hxx"
#include "CloneHelper.hxx"
#include "ModifyListenerHelper.hxx"
#include "EventListenerHelper.hxx"

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;
using ::osl::MutexGuard;

// ----------------------------------------

namespace
{

struct StaticDataSeriesDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        lcl_AddDefaultsToMap( aStaticDefaults );
        return &aStaticDefaults;
    }
private:
    void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::DataSeriesProperties::AddDefaultsToMap( rOutMap );
        ::chart::CharacterProperties::AddDefaultsToMap( rOutMap );

        float fDefaultCharHeight = 10.0;
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_CHAR_HEIGHT, fDefaultCharHeight );
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_ASIAN_CHAR_HEIGHT, fDefaultCharHeight );
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_COMPLEX_CHAR_HEIGHT, fDefaultCharHeight );
    }
};

struct StaticDataSeriesDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticDataSeriesDefaults_Initializer >
{
};

struct StaticDataSeriesInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    uno::Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        ::chart::DataSeriesProperties::AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

};

struct StaticDataSeriesInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticDataSeriesInfoHelper_Initializer >
{
};

struct StaticDataSeriesInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticDataSeriesInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticDataSeriesInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticDataSeriesInfo_Initializer >
{
};

void lcl_SetParent(
    const uno::Reference< uno::XInterface > & xChildInterface,
    const uno::Reference< uno::XInterface > & xParentInterface )
{
    uno::Reference< container::XChild > xChild( xChildInterface, uno::UNO_QUERY );
    if( xChild.is())
        xChild->setParent( xParentInterface );
}

typedef ::std::map< sal_Int32, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > >
    lcl_tDataPointMap;

void lcl_CloneAttributedDataPoints(
    const lcl_tDataPointMap & rSource, lcl_tDataPointMap & rDestination,
    const uno::Reference< uno::XInterface > & xSeries )
{
    for( lcl_tDataPointMap::const_iterator aIt( rSource.begin());
         aIt != rSource.end(); ++aIt )
    {
        Reference< beans::XPropertySet > xPoint( (*aIt).second );
        if( xPoint.is())
        {
            Reference< util::XCloneable > xCloneable( xPoint, uno::UNO_QUERY );
            if( xCloneable.is())
            {
                xPoint.set( xCloneable->createClone(), uno::UNO_QUERY );
                if( xPoint.is())
                {
                    lcl_SetParent( xPoint, xSeries );
                    rDestination.insert( lcl_tDataPointMap::value_type( (*aIt).first, xPoint ));
                }
            }
        }
    }
}

} // anonymous namespace

// ----------------------------------------

namespace chart
{

DataSeries::DataSeries( const uno::Reference< uno::XComponentContext > & xContext ) :
        ::property::OPropertySet( m_aMutex ),
        m_xContext( xContext ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
}

DataSeries::DataSeries( const DataSeries & rOther ) :
        MutexContainer(),
        impl::DataSeries_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xContext( rOther.m_xContext ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    if( ! rOther.m_aDataSequences.empty())
    {
        CloneHelper::CloneRefVector< tDataSequenceContainer::value_type >(
            rOther.m_aDataSequences, m_aDataSequences );
        ModifyListenerHelper::addListenerToAllElements( m_aDataSequences, m_xModifyEventForwarder );
    }

    CloneHelper::CloneRefVector< Reference< chart2::XRegressionCurve > >( rOther.m_aRegressionCurves, m_aRegressionCurves );
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
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL DataSeries::createClone()
    throw (uno::RuntimeException)
{
    DataSeries * pNewSeries( new DataSeries( *this ));
    // hold a reference to the clone
    uno::Reference< util::XCloneable > xResult( pNewSeries );
    // do initialization that uses uno references to the clone
    pNewSeries->Init( *this );

    return xResult;
}

Sequence< OUString > DataSeries::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 3 );
    aServices[ 0 ] = C2U( "com.sun.star.chart2.DataSeries" );
    aServices[ 1 ] = C2U( "com.sun.star.chart2.DataPointProperties" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// ____ OPropertySet ____
uno::Any DataSeries::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticDataSeriesDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL DataSeries::getInfoHelper()
{
    return *StaticDataSeriesInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL DataSeries::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticDataSeriesInfo::get();
}

void SAL_CALL DataSeries::getFastPropertyValue
    ( uno::Any& rValue,
      sal_Int32 nHandle ) const
{
    // special handling for get.  set is not possible for this property
    if( nHandle == DataSeriesProperties::PROP_DATASERIES_ATTRIBUTED_DATA_POINTS )
    {
        // ToDo: only add those property sets that are really modified
        uno::Sequence< sal_Int32 > aSeq( m_aAttributedDataPoints.size());
        sal_Int32 * pIndexArray = aSeq.getArray();
        sal_Int32 i = 0;

        for( tDataPointAttributeContainer::const_iterator aIt( m_aAttributedDataPoints.begin());
             aIt != m_aAttributedDataPoints.end(); ++aIt )
        {
            pIndexArray[ i ] = (*aIt).first;
            ++i;
        }

        rValue <<= aSeq;
    }
    else
        OPropertySet::getFastPropertyValue( rValue, nHandle );
}

void SAL_CALL DataSeries::setFastPropertyValue_NoBroadcast(
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

Reference< beans::XPropertySet >
    SAL_CALL DataSeries::getDataPointByIndex( sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    Reference< beans::XPropertySet > xResult;

    Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences;
    {
        MutexGuard aGuard( GetMutex() );
        aSequences = ContainerHelper::ContainerToSequence( m_aDataSequences );
    }

    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aValuesSeries(
        DataSeriesHelper::getAllDataSequencesByRole( aSequences , C2U("values"), true ) );
    if( !aValuesSeries.empty() )
    {
        Reference< chart2::data::XDataSequence > xSeq( aValuesSeries.front()->getValues() );
        if( 0 <= nIndex && nIndex < xSeq->getData().getLength() )
        {
            {
                MutexGuard aGuard( GetMutex() );
                tDataPointAttributeContainer::iterator aIt( m_aAttributedDataPoints.find( nIndex ) );
                if( aIt != m_aAttributedDataPoints.end() )
                    xResult = (*aIt).second;
            }
            if( !xResult.is() )
            {
                Reference< beans::XPropertySet > xParentProperties;
                Reference< util::XModifyListener > xModifyEventForwarder;
                {
                    MutexGuard aGuard( GetMutex() );
                    xParentProperties = this;
                    xModifyEventForwarder = m_xModifyEventForwarder;
                }

                // create a new XPropertySet for this data point
                xResult.set( new DataPoint( xParentProperties ) );
                {
                    MutexGuard aGuard( GetMutex() );
                    m_aAttributedDataPoints[ nIndex ] = xResult;
                }
                ModifyListenerHelper::addListener( xResult, xModifyEventForwarder );
            }
        }
    }
    else
    {
        throw lang::IndexOutOfBoundsException();
    }

    return xResult;
}

void SAL_CALL DataSeries::resetDataPoint( sal_Int32 nIndex )
        throw (uno::RuntimeException)
{
    Reference< beans::XPropertySet > xDataPointProp;
    Reference< util::XModifyListener > xModifyEventForwarder;
    {
        MutexGuard aGuard( GetMutex() );
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
        throw (uno::RuntimeException)
{
    tDataPointAttributeContainer  aOldAttributedDataPoints;
    Reference< util::XModifyListener > xModifyEventForwarder;
    {
        MutexGuard aGuard( GetMutex() );
        xModifyEventForwarder = m_xModifyEventForwarder;
        std::swap( aOldAttributedDataPoints, m_aAttributedDataPoints );
    }
    ModifyListenerHelper::removeListenerFromAllMapElements( aOldAttributedDataPoints, xModifyEventForwarder );
    aOldAttributedDataPoints.clear();
    fireModifyEvent();
}

// ____ XDataSink ____
void SAL_CALL DataSeries::setData( const uno::Sequence< Reference< chart2::data::XLabeledDataSequence > >& aData )
    throw (uno::RuntimeException)
{
    tDataSequenceContainer aOldDataSequences;
    tDataSequenceContainer aNewDataSequences;
    Reference< util::XModifyListener > xModifyEventForwarder;
    Reference< lang::XEventListener > xListener;
    {
        MutexGuard aGuard( GetMutex() );
        xModifyEventForwarder = m_xModifyEventForwarder;
        xListener = this;
        std::swap( aOldDataSequences, m_aDataSequences );
        aNewDataSequences = ContainerHelper::SequenceToVector( aData );
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
    throw (uno::RuntimeException)
{
    MutexGuard aGuard( GetMutex() );
    return ContainerHelper::ContainerToSequence( m_aDataSequences );
}


// ____ XRegressionCurveContainer ____
void SAL_CALL DataSeries::addRegressionCurve(
    const uno::Reference< chart2::XRegressionCurve >& xRegressionCurve )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    Reference< util::XModifyListener > xModifyEventForwarder;
    {
        MutexGuard aGuard( GetMutex() );
        xModifyEventForwarder = m_xModifyEventForwarder;
        if( ::std::find( m_aRegressionCurves.begin(), m_aRegressionCurves.end(), xRegressionCurve )
            != m_aRegressionCurves.end())
            throw lang::IllegalArgumentException();
        m_aRegressionCurves.push_back( xRegressionCurve );
    }
    ModifyListenerHelper::addListener( xRegressionCurve, xModifyEventForwarder );
    fireModifyEvent();
}

void SAL_CALL DataSeries::removeRegressionCurve(
    const uno::Reference< chart2::XRegressionCurve >& xRegressionCurve )
    throw (container::NoSuchElementException,
           uno::RuntimeException)
{
    if( !xRegressionCurve.is() )
        throw container::NoSuchElementException();

    Reference< util::XModifyListener > xModifyEventForwarder;
    {
        MutexGuard aGuard( GetMutex() );
        xModifyEventForwarder = m_xModifyEventForwarder;
        tRegressionCurveContainerType::iterator aIt(
            ::std::find( m_aRegressionCurves.begin(), m_aRegressionCurves.end(), xRegressionCurve ) );
        if( aIt == m_aRegressionCurves.end())
            throw container::NoSuchElementException(
                C2U( "The given regression curve is no element of this series" ),
                static_cast< uno::XWeak * >( this ));
        m_aRegressionCurves.erase( aIt );
    }

    ModifyListenerHelper::removeListener( xRegressionCurve, xModifyEventForwarder );
    fireModifyEvent();
}

uno::Sequence< uno::Reference< chart2::XRegressionCurve > > SAL_CALL DataSeries::getRegressionCurves()
    throw (uno::RuntimeException)
{
    MutexGuard aGuard( GetMutex() );
    return ContainerHelper::ContainerToSequence( m_aRegressionCurves );
}

void SAL_CALL DataSeries::setRegressionCurves(
    const Sequence< Reference< chart2::XRegressionCurve > >& aRegressionCurves )
    throw (uno::RuntimeException)
{
    tRegressionCurveContainerType aOldCurves;
    tRegressionCurveContainerType aNewCurves( ContainerHelper::SequenceToVector( aRegressionCurves ) );
    Reference< util::XModifyListener > xModifyEventForwarder;
    {
        MutexGuard aGuard( GetMutex() );
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
    throw (uno::RuntimeException)
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

void SAL_CALL DataSeries::removeModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
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

// ____ XModifyListener ____
void SAL_CALL DataSeries::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL DataSeries::disposing( const lang::EventObject& rEventObject )
    throw (uno::RuntimeException)
{
    // forget disposed data sequences
    tDataSequenceContainer::iterator aIt(
        ::std::find( m_aDataSequences.begin(), m_aDataSequences.end(), rEventObject.Source ));
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


// ================================================================================

using impl::DataSeries_Base;
using ::property::OPropertySet;

IMPLEMENT_FORWARD_XINTERFACE2( DataSeries, DataSeries_Base, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( DataSeries, DataSeries_Base, OPropertySet )

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( DataSeries,
                             C2U( "com.sun.star.comp.chart.DataSeries" ));

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
