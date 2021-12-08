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

#include "CandleStickChartType.hxx"
#include <PropertyHelper.hxx>
#include <StockBar.hxx>
#include <ModifyListenerHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

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
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "Japanese",
                  PROP_CANDLESTICKCHARTTYPE_JAPANESE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "WhiteDay",
                  PROP_CANDLESTICKCHARTTYPE_WHITE_DAY,
                  cppu::UnoType<beans::XPropertySet>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );
    rOutProperties.emplace_back( "BlackDay",
                  PROP_CANDLESTICKCHARTTYPE_BLACK_DAY,
                  cppu::UnoType<beans::XPropertySet>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "ShowFirst",
                  PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "ShowHighLow",
                  PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
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
    static void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        // must match default in CTOR!
        rOutMap.setPropertyValueDefault( PROP_CANDLESTICKCHARTTYPE_JAPANESE, false );
        rOutMap.setPropertyValueDefault( PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST, false );
        rOutMap.setPropertyValueDefault( PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW, true );
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
    static Sequence< Property > lcl_GetPropertySequence()
    {
        std::vector< css::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
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

CandleStickChartType::CandleStickChartType()
{
    Reference< beans::XPropertySet > xWhiteDayProps( new ::chart::StockBar( true ));
    Reference< beans::XPropertySet > xBlackDayProps( new ::chart::StockBar( false ));

    ModifyListenerHelper::addListener( xWhiteDayProps, m_xModifyEventForwarder );
    ModifyListenerHelper::addListener( xBlackDayProps, m_xModifyEventForwarder );

    setFastPropertyValue_NoBroadcast(
        PROP_CANDLESTICKCHARTTYPE_WHITE_DAY, uno::Any( xWhiteDayProps ));
    setFastPropertyValue_NoBroadcast(
        PROP_CANDLESTICKCHARTTYPE_BLACK_DAY, uno::Any( xBlackDayProps ));
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
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL CandleStickChartType::createClone()
{
    return uno::Reference< util::XCloneable >( new CandleStickChartType( *this ));
}

// ____ XChartType ____
OUString SAL_CALL CandleStickChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK;
}

uno::Sequence< OUString > SAL_CALL CandleStickChartType::getSupportedMandatoryRoles()
{
    bool bShowFirst = true;
    bool bShowHiLow = false;
    getFastPropertyValue( PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST ) >>= bShowFirst;
    getFastPropertyValue( PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW ) >>= bShowHiLow;

    std::vector< OUString > aMandRoles;

    aMandRoles.emplace_back("label");
    if( bShowFirst )
        aMandRoles.emplace_back("values-first");

    if( bShowHiLow )
    {
        aMandRoles.emplace_back("values-min");
        aMandRoles.emplace_back("values-max");
    }

    aMandRoles.emplace_back("values-last");

    return comphelper::containerToSequence( aMandRoles );
}

Sequence< OUString > SAL_CALL CandleStickChartType::getSupportedOptionalRoles()
{
    bool bShowFirst = true;
    bool bShowHiLow = false;
    getFastPropertyValue( PROP_CANDLESTICKCHARTTYPE_SHOW_FIRST ) >>= bShowFirst;
    getFastPropertyValue( PROP_CANDLESTICKCHARTTYPE_SHOW_HIGH_LOW ) >>= bShowHiLow;

    std::vector< OUString > aOptRoles;

    if( ! bShowFirst )
        aOptRoles.emplace_back("values-first");

    if( ! bShowHiLow )
    {
        aOptRoles.emplace_back("values-min");
        aOptRoles.emplace_back("values-max");
    }

    return comphelper::containerToSequence( aOptRoles );
}

OUString SAL_CALL CandleStickChartType::getRoleOfSequenceForSeriesLabel()
{
    return "values-last";
}

// ____ OPropertySet ____
void CandleStickChartType::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticCandleStickChartTypeDefaults::get();
    rStaticDefaults.get(nHandle, rAny);
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL CandleStickChartType::getInfoHelper()
{
    return *StaticCandleStickChartTypeInfoHelper::get();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL CandleStickChartType::getPropertySetInfo()
{
    return *StaticCandleStickChartTypeInfo::get();
}

void SAL_CALL CandleStickChartType::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, const uno::Any& rValue )
{
    if(    nHandle == PROP_CANDLESTICKCHARTTYPE_WHITE_DAY
        || nHandle == PROP_CANDLESTICKCHARTTYPE_BLACK_DAY )
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

OUString SAL_CALL CandleStickChartType::getImplementationName()
{
    return "com.sun.star.comp.chart.CandleStickChartType" ;
}

sal_Bool SAL_CALL CandleStickChartType::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL CandleStickChartType::getSupportedServiceNames()
{
    return {
        CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK,
        "com.sun.star.chart2.ChartType",
        "com.sun.star.beans.PropertySet" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_CandleStickChartType_get_implementation(css::uno::XComponentContext * /*context*/,
                                                         css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::CandleStickChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
