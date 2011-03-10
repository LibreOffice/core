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
#include "ChartType.hxx"
#include "PropertyHelper.hxx"
#include "CommonFunctors.hxx"
#include "macros.hxx"
#include "CartesianCoordinateSystem.hxx"
#include "AxisHelper.hxx"
#include "CloneHelper.hxx"
#include "AxisIndexDefines.hxx"
#include "ContainerHelper.hxx"
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace chart
{

ChartType::ChartType(
    const Reference< uno::XComponentContext > & xContext ) :
        ::property::OPropertySet( m_aMutex ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder()),
        m_xContext( xContext ),
        m_bNotifyChanges( true )
{}

ChartType::ChartType( const ChartType & rOther ) :
        MutexContainer(),
        impl::ChartType_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder()),
    m_xContext( rOther.m_xContext ),
    m_bNotifyChanges( true )
{
    CloneHelper::CloneRefVector< Reference< chart2::XDataSeries > >( rOther.m_aDataSeries, m_aDataSeries );
    ModifyListenerHelper::addListenerToAllElements( m_aDataSeries, m_xModifyEventForwarder );
}

ChartType::~ChartType()
{
    ModifyListenerHelper::removeListenerFromAllElements( m_aDataSeries, m_xModifyEventForwarder );
    m_aDataSeries.clear();
}

Reference< uno::XComponentContext > ChartType::GetComponentContext() const
{
    return m_xContext;
}

// ____ XChartType ____
Reference< chart2::XCoordinateSystem > SAL_CALL
    ChartType::createCoordinateSystem( ::sal_Int32 DimensionCount )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    Reference< chart2::XCoordinateSystem > xResult(
        new CartesianCoordinateSystem(
            GetComponentContext(), DimensionCount, /* bSwapXAndYAxis */ sal_False ));

    for( sal_Int32 i=0; i<DimensionCount; ++i )
    {
        Reference< chart2::XAxis > xAxis( xResult->getAxisByDimension( i, MAIN_AXIS_INDEX ) );
        if( !xAxis.is() )
        {
            OSL_ENSURE(false,"a created coordinate system should have an axis for each dimension");
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
    throw (uno::RuntimeException)
{
    static Sequence< OUString > aDefaultSeq;

    if( aDefaultSeq.getLength() == 0 )
    {
        aDefaultSeq.realloc( 2 );
        aDefaultSeq[0] = C2U( "label" );
        aDefaultSeq[1] = C2U( "values-y" );
    }

    return aDefaultSeq;
}

Sequence< OUString > SAL_CALL ChartType::getSupportedOptionalRoles()
    throw (uno::RuntimeException)
{
    static Sequence< OUString > aDefaultOptRolesSeq;

    return aDefaultOptRolesSeq;
}

OUString SAL_CALL ChartType::getRoleOfSequenceForSeriesLabel()
    throw (uno::RuntimeException)
{
    return C2U( "values-y" );
}

void ChartType::impl_addDataSeriesWithoutNotification(
        const Reference< chart2::XDataSeries >& xDataSeries )
{
    if( ::std::find( m_aDataSeries.begin(), m_aDataSeries.end(), xDataSeries )
        != m_aDataSeries.end())
        throw lang::IllegalArgumentException();

    m_aDataSeries.push_back( xDataSeries );
    ModifyListenerHelper::addListener( xDataSeries, m_xModifyEventForwarder );
}

// ____ XDataSeriesContainer ____
void SAL_CALL ChartType::addDataSeries( const Reference< chart2::XDataSeries >& xDataSeries )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    impl_addDataSeriesWithoutNotification( xDataSeries );
    fireModifyEvent();
}

void SAL_CALL ChartType::removeDataSeries( const Reference< chart2::XDataSeries >& xDataSeries )
    throw (container::NoSuchElementException,
           uno::RuntimeException)
{
    if( !xDataSeries.is())
        throw container::NoSuchElementException();

    tDataSeriesContainerType::iterator aIt(
            ::std::find( m_aDataSeries.begin(), m_aDataSeries.end(), xDataSeries ) );

    if( aIt == m_aDataSeries.end())
        throw container::NoSuchElementException(
            C2U( "The given series is no element of this charttype" ),
            static_cast< uno::XWeak * >( this ));

    ModifyListenerHelper::removeListener( xDataSeries, m_xModifyEventForwarder );
    m_aDataSeries.erase( aIt );
    fireModifyEvent();
}

Sequence< Reference< chart2::XDataSeries > > SAL_CALL ChartType::getDataSeries()
    throw (uno::RuntimeException)
{
    return ContainerHelper::ContainerToSequence( m_aDataSeries );
}

void SAL_CALL ChartType::setDataSeries( const Sequence< Reference< chart2::XDataSeries > >& aDataSeries )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    m_bNotifyChanges = false;
    try
    {
        Sequence< Reference< chart2::XDataSeries > > aOldSeries( this->getDataSeries() );
        for( sal_Int32 nN=0; nN<aOldSeries.getLength(); ++nN )
            ModifyListenerHelper::removeListener( aOldSeries[nN], m_xModifyEventForwarder );
        m_aDataSeries.clear();

        for( sal_Int32 i=0; i<aDataSeries.getLength(); ++i )
            impl_addDataSeriesWithoutNotification( aDataSeries[i] );
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
    throw(beans::UnknownPropertyException)
{
    return uno::Any();
}

namespace
{

struct StaticChartTypeInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        // using assignment for broken gcc 3.3
        static ::cppu::OPropertyArrayHelper aPropHelper = ::cppu::OPropertyArrayHelper(
            Sequence< beans::Property >() );
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
    throw (uno::RuntimeException)
{
    return *StaticChartTypeInfo::get();
}

// ____ XModifyBroadcaster ____
void SAL_CALL ChartType::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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

void SAL_CALL ChartType::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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
void SAL_CALL ChartType::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL ChartType::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
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
    if( m_bNotifyChanges )
        m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

// ================================================================================

using impl::ChartType_Base;

IMPLEMENT_FORWARD_XINTERFACE2( ChartType, ChartType_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( ChartType, ChartType_Base, ::property::OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
