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

#include "BubbleChartTypeTemplate.hxx"
#include "BubbleChartType.hxx"
#include "BubbleDataInterpreter.hxx"
#include <servicenames_charttypes.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <PropertyHelper.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <tools/diagnose_ex.h>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace
{

struct StaticBubbleChartTypeTemplateDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        return &aStaticDefaults;
    }
};

struct StaticBubbleChartTypeTemplateDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticBubbleChartTypeTemplateDefaults_Initializer >
{
};

struct StaticBubbleChartTypeTemplateInfoHelper_Initializer
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

        std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }

};

struct StaticBubbleChartTypeTemplateInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticBubbleChartTypeTemplateInfoHelper_Initializer >
{
};

struct StaticBubbleChartTypeTemplateInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticBubbleChartTypeTemplateInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticBubbleChartTypeTemplateInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticBubbleChartTypeTemplateInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

BubbleChartTypeTemplate::BubbleChartTypeTemplate(
    Reference<
        uno::XComponentContext > const & xContext,
    const OUString & rServiceName ) :
        ChartTypeTemplate( xContext, rServiceName ),
        ::property::OPropertySet( m_aMutex )
{
}

BubbleChartTypeTemplate::~BubbleChartTypeTemplate()
{}

// ____ OPropertySet ____
void BubbleChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticBubbleChartTypeTemplateDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL BubbleChartTypeTemplate::getInfoHelper()
{
    return *StaticBubbleChartTypeTemplateInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL BubbleChartTypeTemplate::getPropertySetInfo()
{
    return *StaticBubbleChartTypeTemplateInfo::get();
}

sal_Int32 BubbleChartTypeTemplate::getDimension() const
{
    return 2;
}

StackMode BubbleChartTypeTemplate::getStackMode( sal_Int32 /* nChartTypeIndex */ ) const
{
    return StackMode::NONE;
}

void BubbleChartTypeTemplate::applyStyle(
    const rtl::Reference< DataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
{
    ChartTypeTemplate::applyStyle( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );
    DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", uno::Any( drawing::LineStyle_NONE ) );
}

// ____ ChartTypeTemplate ____
bool BubbleChartTypeTemplate::supportsCategories()
{
    return false;
}

rtl::Reference< ChartType > BubbleChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    return new BubbleChartType();
}

rtl::Reference< ChartType > BubbleChartTypeTemplate::getChartTypeForNewSeries(
        const std::vector< rtl::Reference< ChartType > >& aFormerlyUsedChartTypes )
{
    rtl::Reference< ChartType > xResult;

    try
    {
        xResult = new BubbleChartType();

        ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aFormerlyUsedChartTypes, xResult );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return xResult;
}

rtl::Reference< DataInterpreter > BubbleChartTypeTemplate::getDataInterpreter()
{
    if( ! m_xDataInterpreter.is())
        m_xDataInterpreter.set( new BubbleDataInterpreter );

    return m_xDataInterpreter;
}

IMPLEMENT_FORWARD_XINTERFACE2( BubbleChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( BubbleChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
