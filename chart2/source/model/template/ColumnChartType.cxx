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

#include "ColumnChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <PropertyHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace
{

enum
{
    PROP_BARCHARTTYPE_OVERLAP_SEQUENCE,
    PROP_BARCHARTTYPE_GAPWIDTH_SEQUENCE
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "OverlapSequence",
                  PROP_BARCHARTTYPE_OVERLAP_SEQUENCE,
                  cppu::UnoType<Sequence< sal_Int32 >>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "GapwidthSequence",
                  PROP_BARCHARTTYPE_GAPWIDTH_SEQUENCE,
                  cppu::UnoType<Sequence< sal_Int32 >>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

struct StaticColumnChartTypeDefaults_Initializer
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
        Sequence< sal_Int32 > aSeq{ 0, 0 };
        rOutMap.setPropertyValueDefault( PROP_BARCHARTTYPE_OVERLAP_SEQUENCE, aSeq );

        aSeq = { 100, 100 };
        rOutMap.setPropertyValueDefault( PROP_BARCHARTTYPE_GAPWIDTH_SEQUENCE, aSeq );
    }
};

struct StaticColumnChartTypeDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticColumnChartTypeDefaults_Initializer >
{
};

struct StaticColumnChartTypeInfoHelper_Initializer
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

struct StaticColumnChartTypeInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticColumnChartTypeInfoHelper_Initializer >
{
};

struct StaticColumnChartTypeInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticColumnChartTypeInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticColumnChartTypeInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticColumnChartTypeInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

ColumnChartType::ColumnChartType()
{}

ColumnChartType::ColumnChartType( const ColumnChartType & rOther ) :
        ChartType( rOther )
{
}

ColumnChartType::~ColumnChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL ColumnChartType::createClone()
{
    return uno::Reference< util::XCloneable >( new ColumnChartType( *this ));
}

// ____ XChartType ____
OUString SAL_CALL ColumnChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_COLUMN;
}

uno::Sequence< OUString > ColumnChartType::getSupportedPropertyRoles()
{
    return { "FillColor", "BorderColor" };
}

// ____ OPropertySet ____
void ColumnChartType::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticColumnChartTypeDefaults::get();
    rStaticDefaults.get(nHandle, rAny);
}

::cppu::IPropertyArrayHelper & SAL_CALL ColumnChartType::getInfoHelper()
{
    return *StaticColumnChartTypeInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL ColumnChartType::getPropertySetInfo()
{
    return *StaticColumnChartTypeInfo::get();
}

OUString SAL_CALL ColumnChartType::getImplementationName()
{
    return "com.sun.star.comp.chart.ColumnChartType";
}

sal_Bool SAL_CALL ColumnChartType::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ColumnChartType::getSupportedServiceNames()
{
    return {
        CHART2_SERVICE_NAME_CHARTTYPE_COLUMN,
        "com.sun.star.chart2.ChartType" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_ColumnChartType_get_implementation(css::uno::XComponentContext * /*context*/,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::ColumnChartType());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
