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

#include "CandleStickChartType.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "StockBar.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

enum
{
    PROP_CANDLESTICKCHARTTYPE_JAPANESE,
    PROP_CANDLESTICKCHARTTYPE_WHITE_DAY,
    PROP_CANDLESTICKCHARTTYPE_BLACK_DAY,

    PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST,
    PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Japanese" ),
                  PROP_CANDLESTICKCHARTTYPE_JAPANESE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "WhiteDay" ),
                  PROP_CANDLESTICKCHARTTYPE_WHITE_DAY,
                  ::getCppuType( reinterpret_cast< Reference< beans::XPropertySet > *>(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "BlackDay" ),
                  PROP_CANDLESTICKCHARTTYPE_BLACK_DAY,
                  ::getCppuType( reinterpret_cast< Reference< beans::XPropertySet > *>(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "ShowFirst" ),
                  PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "ShowHighLow" ),
                  PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticCandleStickChartTypeDefaults_Initializer
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
        // must match default in CTOR!
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CANDLESTICKCHARTTYPE_JAPANESE, false );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST, false );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW, true );
    }
};

struct StaticCandleStickChartTypeDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticCandleStickChartTypeDefaults_Initializer >
{
};

struct StaticCandleStickChartTypeInfoHelper_Initializer
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
        lcl_AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

};

struct StaticCandleStickChartTypeInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticCandleStickChartTypeInfoHelper_Initializer >
{
};

struct StaticCandleStickChartTypeInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticCandleStickChartTypeInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticCandleStickChartTypeInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticCandleStickChartTypeInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

CandleStickChartType::CandleStickChartType(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        ChartType( xContext )
{
    Reference< beans::XPropertySet > xWhiteDayProps( new ::chart::StockBar( true ));
    Reference< beans::XPropertySet > xBlackDayProps( new ::chart::StockBar( false ));

    ModifyListenerHelper::addListener( xWhiteDayProps, m_xModifyEventForwarder );
    ModifyListenerHelper::addListener( xBlackDayProps, m_xModifyEventForwarder );

    setFastPropertyValue_NoBroadcast(
        PROP_CANDLESTICKCHARTTYPE_WHITE_DAY, uno::makeAny( xWhiteDayProps ));
    setFastPropertyValue_NoBroadcast(
        PROP_CANDLESTICKCHARTTYPE_BLACK_DAY, uno::makeAny( xBlackDayProps ));
}

CandleStickChartType::CandleStickChartType( const CandleStickChartType & rOther ) :
        ChartType( rOther )
{
    Reference< beans::XPropertySet > xPropertySet;
    uno::Any aValue;

    getFastPropertyValue( aValue, PROP_CANDLESTICKCHARTTYPE_WHITE_DAY );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        ModifyListenerHelper::addListener( xPropertySet, m_xModifyEventForwarder );

    getFastPropertyValue( aValue, PROP_CANDLESTICKCHARTTYPE_BLACK_DAY );
    if( ( aValue >>= xPropertySet )
        && xPropertySet.is())
        ModifyListenerHelper::addListener( xPropertySet, m_xModifyEventForwarder );
}

CandleStickChartType::~CandleStickChartType()
{
    try
    {
        Reference< beans::XPropertySet > xPropertySet;
        uno::Any aValue;

        getFastPropertyValue( aValue, PROP_CANDLESTICKCHARTTYPE_WHITE_DAY );
        if( ( aValue >>= xPropertySet )
            && xPropertySet.is())
            ModifyListenerHelper::removeListener( xPropertySet, m_xModifyEventForwarder );

        getFastPropertyValue( aValue, PROP_CANDLESTICKCHARTTYPE_BLACK_DAY );
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
uno::Reference< util::XCloneable > SAL_CALL CandleStickChartType::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new CandleStickChartType( *this ));
}

// ____ XChartType ____
::rtl::OUString SAL_CALL CandleStickChartType::getChartType()
    throw (uno::RuntimeException)
{
    return CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK;
}

uno::Sequence< ::rtl::OUString > SAL_CALL CandleStickChartType::getSupportedMandatoryRoles()
    throw (uno::RuntimeException)
{
    bool bShowFirst = true;
    bool bShowHiLow = false;
    getFastPropertyValue( PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST ) >>= bShowFirst;
    getFastPropertyValue( PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW ) >>= bShowHiLow;

    ::std::vector< OUString > aMandRoles;

    aMandRoles.push_back( C2U( "label" ));
    if( bShowFirst )
        aMandRoles.push_back( C2U( "values-first" ));

    if( bShowHiLow )
    {
        aMandRoles.push_back( C2U( "values-min" ));
        aMandRoles.push_back( C2U( "values-max" ));
    }

    aMandRoles.push_back( C2U( "values-last" ));

    return ContainerHelper::ContainerToSequence( aMandRoles );
}

Sequence< OUString > SAL_CALL CandleStickChartType::getSupportedOptionalRoles()
    throw (uno::RuntimeException)
{
    bool bShowFirst = true;
    bool bShowHiLow = false;
    getFastPropertyValue( PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST ) >>= bShowFirst;
    getFastPropertyValue( PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW ) >>= bShowHiLow;

    ::std::vector< OUString > aOptRoles;

    if( ! bShowFirst )
        aOptRoles.push_back( C2U( "values-first" ));

    if( ! bShowHiLow )
    {
        aOptRoles.push_back( C2U( "values-min" ));
        aOptRoles.push_back( C2U( "values-max" ));
    }

    return ContainerHelper::ContainerToSequence( aOptRoles );
}

OUString SAL_CALL CandleStickChartType::getRoleOfSequenceForSeriesLabel()
    throw (uno::RuntimeException)
{
    return C2U( "values-last" );
}

// ____ OPropertySet ____
uno::Any CandleStickChartType::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticCandleStickChartTypeDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL CandleStickChartType::getInfoHelper()
{
    return *StaticCandleStickChartTypeInfoHelper::get();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL CandleStickChartType::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticCandleStickChartTypeInfo::get();
}

void SAL_CALL CandleStickChartType::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const uno::Any& rValue )
    throw (uno::Exception)
{
    if(    nHandle == PROP_CANDLESTICKCHARTTYPE_WHITE_DAY
        || nHandle == PROP_CANDLESTICKCHARTTYPE_BLACK_DAY )
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

uno::Sequence< ::rtl::OUString > CandleStickChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 3 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ChartType" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( CandleStickChartType,
                             C2U( "com.sun.star.comp.chart.CandleStickChartType" ));

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
