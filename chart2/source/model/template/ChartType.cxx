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

#include <ChartType.hxx>
#include <CartesianCoordinateSystem.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <CloneHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <ModifyListenerHelper.hxx>
#include <DataSeries.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

namespace chart
{

ChartType::ChartType() :
        m_xModifyEventForwarder( new ModifyEventForwarder() ),
        m_bNotifyChanges( true )
{}

ChartType::ChartType( const ChartType & rOther ) :
        impl::ChartType_Base(rOther),
        ::property::OPropertySet( rOther ),
    m_xModifyEventForwarder( new ModifyEventForwarder() ),
    m_bNotifyChanges( true )
{
    {
        SolarMutexGuard g; // access to rOther.m_aDataSeries
        CloneHelper::CloneRefVector(
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
    return createCoordinateSystem2(DimensionCount);
}

rtl::Reference< BaseCoordinateSystem >
    ChartType::createCoordinateSystem2( ::sal_Int32 DimensionCount )
{
    rtl::Reference< CartesianCoordinateSystem > xResult =
        new CartesianCoordinateSystem( DimensionCount );

    for( sal_Int32 i=0; i<DimensionCount; ++i )
    {
        rtl::Reference< Axis > xAxis = xResult->getAxisByDimension2( i, MAIN_AXIS_INDEX );
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
    return { u"label"_ustr, u"values-y"_ustr };
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
    return u"values-y"_ustr;
}

void ChartType::impl_addDataSeriesWithoutNotification(
        const rtl::Reference< DataSeries >& xDataSeries )
{
    if( std::find( m_aDataSeries.begin(), m_aDataSeries.end(), xDataSeries )
        != m_aDataSeries.end())
        throw lang::IllegalArgumentException(u"dataseries not found"_ustr, static_cast<cppu::OWeakObject*>(this), 1);

    m_aDataSeries.push_back( xDataSeries );
    ModifyListenerHelper::addListener( xDataSeries, m_xModifyEventForwarder );
}

// ____ XDataSeriesContainer ____
void SAL_CALL ChartType::addDataSeries( const Reference< chart2::XDataSeries >& xDataSeries )
{
    rtl::Reference<DataSeries> xTmp = dynamic_cast<DataSeries*>(xDataSeries.get());
    assert(xTmp);
    addDataSeries(xTmp);
}

void ChartType::addDataSeries( const rtl::Reference< DataSeries >& xDataSeries )
{
    SolarMutexGuard g;

    impl_addDataSeriesWithoutNotification( xDataSeries );
    fireModifyEvent();
}

void SAL_CALL ChartType::removeDataSeries( const Reference< chart2::XDataSeries >& xDataSeries )
{
    rtl::Reference<DataSeries> xTmp = dynamic_cast<DataSeries*>(xDataSeries.get());
    assert(xTmp);
    removeDataSeries(xTmp);
}

void ChartType::removeDataSeries( const rtl::Reference< DataSeries >& xDataSeries )
{
    if( !xDataSeries.is())
        throw container::NoSuchElementException();

    SolarMutexGuard g;

    auto aIt = std::find( m_aDataSeries.begin(), m_aDataSeries.end(), xDataSeries );

    if( aIt == m_aDataSeries.end())
        throw container::NoSuchElementException(
            u"The given series is no element of this charttype"_ustr,
            static_cast< uno::XWeak * >( this ));

    ModifyListenerHelper::removeListener( xDataSeries, m_xModifyEventForwarder );
    m_aDataSeries.erase( aIt );
    fireModifyEvent();
}

Sequence< Reference< chart2::XDataSeries > > SAL_CALL ChartType::getDataSeries()
{
    SolarMutexGuard g;

    return comphelper::containerToSequence< Reference< chart2::XDataSeries > >( m_aDataSeries );
}

void SAL_CALL ChartType::setDataSeries( const Sequence< Reference< chart2::XDataSeries > >& aDataSeries )
{
    std::vector< rtl::Reference<DataSeries> > aTmp;
    for (auto const & i : aDataSeries)
    {
        auto p = dynamic_cast<DataSeries*>(i.get());
        assert(p);
        aTmp.push_back(p);
    }
    setDataSeries(aTmp);
}

void ChartType::setDataSeries( const std::vector< rtl::Reference< DataSeries > >& aDataSeries )
{
    SolarMutexGuard g;

    m_bNotifyChanges = false;
    try
    {
        for( auto const & i : m_aDataSeries )
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
void ChartType::GetDefaultValue( sal_Int32 /* nHandle */, uno::Any& rAny ) const
{
    rAny.clear();
}

namespace
{

::cppu::OPropertyArrayHelper& StaticChartTypeInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper( Sequence< beans::Property >{} );
    return aPropHelper;
};

}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL ChartType::getInfoHelper()
{
    return StaticChartTypeInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL ChartType::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo( StaticChartTypeInfoHelper() ) );
    return xPropertySetInfo;
}

// ____ XModifyBroadcaster ____
void SAL_CALL ChartType::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    try
    {
        m_xModifyEventForwarder->addModifyListener( aListener );
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
        m_xModifyEventForwarder->removeModifyListener( aListener );
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
