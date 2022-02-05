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

#include <MultipleChartConverters.hxx>

#include "SchWhichPairs.hxx"
#include <AxisItemConverter.hxx>
#include <StatisticsItemConverter.hxx>
#include <GraphicPropertyItemConverter.hxx>
#include <DataPointItemConverter.hxx>
#include <ChartModelHelper.hxx>
#include <ChartModel.hxx>
#include <Diagram.hxx>
#include <DataSeries.hxx>
#include <TitleHelper.hxx>
#include <TitleItemConverter.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <chartview/ExplicitValueProvider.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/chart2/XAxis.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart::wrapper {

AllAxisItemConverter::AllAxisItemConverter(
    const rtl::Reference<::chart::ChartModel> & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const awt::Size* pRefSize )
        : MultipleItemConverter( rItemPool )
{
    rtl::Reference< Diagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    const std::vector< rtl::Reference< Axis > > aElementList = AxisHelper::getAllAxesOfDiagram( xDiagram );
    for( rtl::Reference< Axis > const & axis : aElementList )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties(axis);
        m_aConverters.emplace_back( new ::chart::wrapper::AxisItemConverter(
            xObjectProperties, rItemPool, rDrawModel,
            xChartModel, nullptr, nullptr,
            pRefSize));
    }
}

AllAxisItemConverter::~AllAxisItemConverter()
{
}

const WhichRangesContainer& AllAxisItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nAllAxisWhichPairs;
}

AllGridItemConverter::AllGridItemConverter(
    const rtl::Reference<::chart::ChartModel> & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory )
        : MultipleItemConverter( rItemPool )
{
    rtl::Reference< Diagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    const Sequence< Reference< beans::XPropertySet > > aElementList( AxisHelper::getAllGrids( xDiagram ) );
    for( Reference< beans::XPropertySet > const & xObjectProperties : aElementList )
    {
        m_aConverters.emplace_back( new ::chart::wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rItemPool, rDrawModel, xNamedPropertyContainerFactory,
                                        ::chart::wrapper::GraphicObjectType::LineProperties ) );
    }
}

AllGridItemConverter::~AllGridItemConverter()
{
}

const WhichRangesContainer& AllGridItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nGridWhichPairs;
}

AllDataLabelItemConverter::AllDataLabelItemConverter(
    const rtl::Reference<::chart::ChartModel> & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory )
        : MultipleItemConverter( rItemPool )
{
    std::vector< rtl::Reference< DataSeries > > aSeriesList =
        ::chart::ChartModelHelper::getDataSeries( xChartModel );

    for (auto const& series : aSeriesList)
    {
        uno::Reference< uno::XComponentContext> xContext;//do not need Context for label properties

        sal_Int32 nNumberFormat=ExplicitValueProvider::getExplicitNumberFormatKeyForDataLabel( series );
        sal_Int32 nPercentNumberFormat=ExplicitValueProvider::getExplicitPercentageNumberFormatKeyForDataLabel(
                series,xChartModel);

        m_aConverters.emplace_back(
            new ::chart::wrapper::DataPointItemConverter(
                xChartModel, xContext, series, series, rItemPool, rDrawModel,
                xNamedPropertyContainerFactory, GraphicObjectType::FilledDataPoint,
                nullptr, true, false, 0, true, nNumberFormat, nPercentNumberFormat));
    }
}

AllDataLabelItemConverter::~AllDataLabelItemConverter()
{
}

const WhichRangesContainer& AllDataLabelItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nDataLabelWhichPairs;
}

AllTitleItemConverter::AllTitleItemConverter(
    const rtl::Reference<::chart::ChartModel> & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory )
        : MultipleItemConverter( rItemPool )
{
    for(sal_Int32 nTitle = TitleHelper::TITLE_BEGIN; nTitle < TitleHelper::NORMAL_TITLE_END; nTitle++ )
    {
        uno::Reference< chart2::XTitle > xTitle( TitleHelper::getTitle( TitleHelper::eTitleType(nTitle), xChartModel ) );
        if(!xTitle.is())
            continue;
        uno::Reference< beans::XPropertySet > xObjectProperties( xTitle, uno::UNO_QUERY);
        m_aConverters.emplace_back(
            new ::chart::wrapper::TitleItemConverter(
                xObjectProperties, rItemPool, rDrawModel, xNamedPropertyContainerFactory, nullptr));
    }
}

AllTitleItemConverter::~AllTitleItemConverter()
{
}

const WhichRangesContainer& AllTitleItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nTitleWhichPairs;
}

AllSeriesStatisticsConverter::AllSeriesStatisticsConverter(
    const rtl::Reference<::chart::ChartModel> & xChartModel,
    SfxItemPool& rItemPool )
        : MultipleItemConverter( rItemPool )
{
    std::vector< rtl::Reference< DataSeries > > aSeriesList =
        ::chart::ChartModelHelper::getDataSeries( xChartModel );

    for (auto const& series : aSeriesList)
    {
        m_aConverters.emplace_back( new ::chart::wrapper::StatisticsItemConverter(
                                     xChartModel, series, rItemPool ));
    }
}

AllSeriesStatisticsConverter::~AllSeriesStatisticsConverter()
{}

const WhichRangesContainer& AllSeriesStatisticsConverter::GetWhichPairs() const
{
    // must span all used items!
    return nStatWhichPairs;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
