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

#include "ChartType.hxx"
#include <CartesianCoordinateSystem.hxx>
#include <AxisHelper.hxx>
#include <CloneHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <ModifyListenerHelper.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

namespace chart
{

ChartType::ChartType() :
        ::property::OPropertySet( m_aMutex ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder()),
        m_bNotifyChanges( true )
{}

ChartType::ChartType( const ChartType & rOther ) :
        impl::ChartType_Base(rOther),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder()),
    m_bNotifyChanges( true )
{
    {
        SolarMutexGuard g; // access to rOther.m_aDataSeries
        CloneHelper::CloneRefVector<css::chart2::XDataSeries>(
                rOther.m_aDataSeries, m_aDataSeries);
    }
    ModifyListenerHelper::addListenerToAllElements( m_aDataSeries, m_xModifyEventForwarder );
}

ChartType::~ChartType()
{
    ModifyListenerHelper::removeListenerFromAllElements( m_aDataSeries, m_xModifyEventForwarder );
    m_aDataSeries.clear();
}

// ____ XChartType ____
Reference< chart2::XCoordinateSystem > SAL_CALL
    ChartType::createCoordinateSystem( ::sal_Int32 DimensionCount )
{
    Reference< chart2::XCoordinateSystem > xResult(
        new CartesianCoordinateSystem( DimensionCount ));

    for( sal_Int32 i=0; i<DimensionCount; ++i )
    {
        Reference< chart2::XAxis > xAxis( xResult->getAxisByDimension( i, MAIN_AXIS_INDEX ) );
        if( !xAxis.is() )
        {
            OSL_FAIL("a created coordinate system should have an axis for each dimension");
            continue;
        }

        chart2::ScaleData aScaleData = xAxis->getScaleData();
        aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
        aScaleData.Scaling = AxisHelper::createLinearScaling();

        switch( i )
        {
            case 0: aScaleData.AxisType = chart2::AxisType::CATEGORY; break;
            case 2: aScaleData.AxisType = chart2::AxisType::SERIES; break;
            default: aScaleData.AxisType = chart2::AxisType::REALNUMBER; break;
        }

        xAxis->setScaleData( aScaleData );
    }

    return xResult;
}

Sequence< OUString > SAL_CALL ChartType::getSupportedMandatoryRoles()
{
    Sequence< OUString > aDefaultSeq(2);
    aDefaultSeq[0] = "label";
    aDefaultSeq[1] = "values-y";
    return aDefaultSeq;
}

Sequence< OUString > SAL_CALL ChartType::getSupportedOptionalRoles()
{
    return Sequence< OUString >();
}

Sequence< OUString > SAL_CALL ChartType::getSupportedPropertyRoles()
{
    return Sequence< OUString >();
}

OUString SAL_CALL ChartType::getRoleOfSequenceForSeriesLabel()
{
    return "values-y";
}

void ChartType::impl_addDataSeriesWithoutNotification(
        const Reference< chart2::XDataSeries >& xDataSeries )
{
    if( std::find( m_aDataSeries.begin(), m_aDataSeries.end(), xDataSeries )
        != m_aDataSeries.end())
        throw lang::IllegalArgumentException("dataseries not found", static_cast<cppu::OWeakObject*>(this), 1);

    m_aDataSeries.push_back( xDataSeries );
    ModifyListenerHelper::addListener( xDataSeries, m_xModifyEventForwarder );
}

// ____ XDataSeriesContainer ____
void SAL_CALL ChartType::addDataSeries( const Reference< chart2::XDataSeries >& xDataSeries )
{
    SolarMutexGuard g;

    impl_addDataSeriesWithoutNotification( xDataSeries );
    fireModifyEvent();
}

void SAL_CALL ChartType::removeDataSeries( const Reference< chart2::XDataSeries >& xDataSeries )
{
    if( !xDataSeries.is())
        throw container::NoSuchElementException();

    SolarMutexGuard g;

    tDataSeriesContainerType::iterator aIt(
            std::find( m_aDataSeries.begin(), m_aDataSeries.end(), xDataSeries ) );

    if( aIt == m_aDataSeries.end())
        throw container::NoSuchElementException(
            "The given series is no element of this charttype",
            static_cast< uno::XWeak * >( this ));

    ModifyListenerHelper::removeListener( xDataSeries, m_xModifyEventForwarder );
    m_aDataSeries.erase( aIt );
    fireModifyEvent();
}

Sequence< Reference< chart2::XDataSeries > > SAL_CALL ChartType::getDataSeries()
{
    SolarMutexGuard g;

    return comphelper::containerToSequence( m_aDataSeries );
}

void SAL_CALL ChartType::setDataSeries( const Sequence< Reference< chart2::XDataSeries > >& aDataSeries )
{
    SolarMutexGuard g;

    m_bNotifyChanges = false;
    try
    {
        const Sequence< Reference< chart2::XDataSeries > > aOldSeries( getDataSeries() );
        for( auto const & i : aOldSeries )
            ModifyListenerHelper::removeListener( i, m_xModifyEventForwarder );
        m_aDataSeries.clear();

        for( auto const & i : aDataSeries )
            impl_addDataSeriesWithoutNotification( i );
    }
    catch( ... )
    {
        m_bNotifyChanges = true;
        throw;
    }
    m_bNotifyChanges = true;
    fireModifyEvent();
}

// ____ OPropertySet ____
uno::Any ChartType::GetDefaultValue( sal_Int32 /* nHandle */ ) const
{
    return uno::Any();
}

namespace
{

struct StaticChartTypeInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( Sequence< beans::Property >{} );
        return &aPropHelper;
    }
};

struct StaticChartTypeInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticChartTypeInfoHelper_Initializer >
{
};

struct StaticChartTypeInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticChartTypeInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticChartTypeInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticChartTypeInfo_Initializer >
{
};

}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL ChartType::getInfoHelper()
{
    return *StaticChartTypeInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL ChartType::getPropertySetInfo()
{
    return *StaticChartTypeInfo::get();
}

// ____ XModifyBroadcaster ____
void SAL_CALL ChartType::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void SAL_CALL ChartType::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ XModifyListener ____
void SAL_CALL ChartType::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL ChartType::disposing( const lang::EventObject& /* Source */ )
{
    // nothing
}

// ____ OPropertySet ____
void ChartType::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void ChartType::fireModifyEvent()
{
    bool bNotifyChanges;

    {
        SolarMutexGuard g;
        bNotifyChanges = m_bNotifyChanges;
    }

    if (bNotifyChanges)
        m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

using impl::ChartType_Base;

IMPLEMENT_FORWARD_XINTERFACE2( ChartType, ChartType_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( ChartType, ChartType_Base, ::property::OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
