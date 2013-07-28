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
#include "macros.hxx"
#include "servicenames_charttypes.hxx"
#include "PropertyHelper.hxx"
#include "ContainerHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>

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
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "OverlapSequence",
                  PROP_BARCHARTTYPE_OVERLAP_SEQUENCE,
                  ::getCppuType( reinterpret_cast< const Sequence< sal_Int32 > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "GapwidthSequence",
                  PROP_BARCHARTTYPE_GAPWIDTH_SEQUENCE,
                  ::getCppuType( reinterpret_cast< const Sequence< sal_Int32 > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
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
    void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        Sequence< sal_Int32 > aSeq(2);

        aSeq[0] = aSeq[1] = 0;
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_BARCHARTTYPE_OVERLAP_SEQUENCE, aSeq );

        aSeq[0] = aSeq[1] = 100;
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_BARCHARTTYPE_GAPWIDTH_SEQUENCE, aSeq );
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
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
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

ColumnChartType::ColumnChartType(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        ChartType( xContext )
{}

ColumnChartType::ColumnChartType( const ColumnChartType & rOther ) :
        ChartType( rOther )
{
}

ColumnChartType::~ColumnChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL ColumnChartType::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new ColumnChartType( *this ));
}

// ____ XChartType ____
OUString SAL_CALL ColumnChartType::getChartType()
    throw (uno::RuntimeException)
{
    return CHART2_SERVICE_NAME_CHARTTYPE_COLUMN;
}

// ____ OPropertySet ____
uno::Any ColumnChartType::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticColumnChartTypeDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL ColumnChartType::getInfoHelper()
{
    return *StaticColumnChartTypeInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL ColumnChartType::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticColumnChartTypeInfo::get();
}

uno::Sequence< OUString > ColumnChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_COLUMN;
    aServices[ 1 ] = "com.sun.star.chart2.ChartType";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ColumnChartType,
                             OUString("com.sun.star.comp.chart.ColumnChartType") );

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
