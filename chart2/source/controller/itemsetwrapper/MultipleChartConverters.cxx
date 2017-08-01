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

#include "MultipleChartConverters.hxx"

#include "chartview/ChartSfxItemIds.hxx"
#include "SchWhichPairs.hxx"
#include "AxisItemConverter.hxx"
#include "StatisticsItemConverter.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "DataPointItemConverter.hxx"
#include "ChartModelHelper.hxx"
#include "TitleHelper.hxx"
#include "TitleItemConverter.hxx"
#include "AxisHelper.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "DiagramHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart { namespace wrapper {

AllAxisItemConverter::AllAxisItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const awt::Size* pRefSize )
        : MultipleItemConverter( rItemPool )
{
    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    Sequence< Reference< XAxis > > aElementList( AxisHelper::getAllAxesOfDiagram( xDiagram ) );
    for( sal_Int32 nA = 0; nA < aElementList.getLength(); nA++ )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties(aElementList[nA], uno::UNO_QUERY);
        m_aConverters.push_back( new ::chart::wrapper::AxisItemConverter(
            xObjectProperties, rItemPool, rDrawModel,
            uno::Reference< chart2::XChartDocument >( xChartModel, uno::UNO_QUERY ), nullptr, nullptr,
            pRefSize));
    }
}

AllAxisItemConverter::~AllAxisItemConverter()
{
}

const sal_uInt16 * AllAxisItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nAllAxisWhichPairs;
}

AllGridItemConverter::AllGridItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory )
        : MultipleItemConverter( rItemPool )
{
    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    Sequence< Reference< beans::XPropertySet > > aElementList( AxisHelper::getAllGrids( xDiagram ) );
    for( sal_Int32 nA = 0; nA < aElementList.getLength(); nA++ )
    {
        Reference< beans::XPropertySet > xObjectProperties(aElementList[nA]);
        m_aConverters.push_back( new ::chart::wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rItemPool, rDrawModel, xNamedPropertyContainerFactory,
                                        ::chart::wrapper::GraphicObjectType::LineProperties ) );
    }
}

AllGridItemConverter::~AllGridItemConverter()
{
}

const sal_uInt16 * AllGridItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nGridWhichPairs;
}

AllDataLabelItemConverter::AllDataLabelItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory )
        : MultipleItemConverter( rItemPool )
{
    std::vector< uno::Reference< chart2::XDataSeries > > aSeriesList(
        ::chart::ChartModelHelper::getDataSeries( xChartModel ));

    std::vector< uno::Reference< chart2::XDataSeries > >::const_iterator aIt;
    for( aIt = aSeriesList.begin(); aIt != aSeriesList.end(); ++aIt )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties( *aIt, uno::UNO_QUERY);
        uno::Reference< uno::XComponentContext> xContext(nullptr);//do not need Context for label properties

        sal_Int32 nNumberFormat=ExplicitValueProvider::getExplicitNumberFormatKeyForDataLabel( xObjectProperties, *aIt, -1/*nPointIndex*/, ChartModelHelper::findDiagram( xChartModel ) );
        sal_Int32 nPercentNumberFormat=ExplicitValueProvider::getExplicitPercentageNumberFormatKeyForDataLabel(
                xObjectProperties,uno::Reference< util::XNumberFormatsSupplier >(xChartModel, uno::UNO_QUERY));

        m_aConverters.push_back(
            new ::chart::wrapper::DataPointItemConverter(
                xChartModel, xContext, xObjectProperties, *aIt, rItemPool, rDrawModel,
                xNamedPropertyContainerFactory, GraphicObjectType::FilledDataPoint,
                nullptr, true, false, 0, true, nNumberFormat, nPercentNumberFormat));
    }
}

AllDataLabelItemConverter::~AllDataLabelItemConverter()
{
}

const sal_uInt16 * AllDataLabelItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nDataLabelWhichPairs;
}

AllTitleItemConverter::AllTitleItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
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
        m_aConverters.push_back(
            new ::chart::wrapper::TitleItemConverter(
                xObjectProperties, rItemPool, rDrawModel, xNamedPropertyContainerFactory, nullptr));
    }
}

AllTitleItemConverter::~AllTitleItemConverter()
{
}

const sal_uInt16 * AllTitleItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nTitleWhichPairs;
}

AllSeriesStatisticsConverter::AllSeriesStatisticsConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool )
        : MultipleItemConverter( rItemPool )
{
    std::vector< uno::Reference< chart2::XDataSeries > > aSeriesList(
        ::chart::ChartModelHelper::getDataSeries( xChartModel ));

    std::vector< uno::Reference< chart2::XDataSeries > >::const_iterator aIt;
    for( aIt = aSeriesList.begin(); aIt != aSeriesList.end(); ++aIt )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties( *aIt, uno::UNO_QUERY);
        m_aConverters.push_back( new ::chart::wrapper::StatisticsItemConverter(
                                     xChartModel, xObjectProperties, rItemPool ));
    }
}

AllSeriesStatisticsConverter::~AllSeriesStatisticsConverter()
{}

const sal_uInt16 * AllSeriesStatisticsConverter::GetWhichPairs() const
{
    // must span all used items!
    return nStatWhichPairs;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
