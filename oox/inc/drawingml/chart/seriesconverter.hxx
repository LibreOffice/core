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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_SERIESCONVERTER_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_SERIESCONVERTER_HXX

#include <drawingml/chart/converterbase.hxx>
#include <drawingml/chart/seriesmodel.hxx>

namespace com { namespace sun { namespace star {
    namespace chart2 { class XDataSeries; }
    namespace chart2 { namespace data { class XLabeledDataSequence; } }
} } }

namespace oox {
namespace drawingml {
namespace chart {

class TypeGroupConverter;

// #i66858# enable this when Chart2 supports smoothed lines per data series
#define OOX_CHART_SMOOTHED_PER_SERIES 0



class DataLabelConverter : public ConverterBase< DataLabelModel >
{
public:
    explicit            DataLabelConverter( const ConverterRoot& rParent, DataLabelModel& rModel );
    virtual             ~DataLabelConverter();

    /** Converts OOXML data label settings for the passed data point. */
    void                convertFromModel(
                            const css::uno::Reference< css::chart2::XDataSeries >& rxDataSeries,
                            const TypeGroupConverter& rTypeGroup );
};



class DataLabelsConverter : public ConverterBase< DataLabelsModel >
{
public:
    explicit            DataLabelsConverter( const ConverterRoot& rParent, DataLabelsModel& rModel );
    virtual             ~DataLabelsConverter();

    /** Converts OOXML data label settings for the passed data series. */
    void                convertFromModel(
                            const css::uno::Reference< css::chart2::XDataSeries >& rxDataSeries,
                            const TypeGroupConverter& rTypeGroup );
};



class ErrorBarConverter : public ConverterBase< ErrorBarModel >
{
public:
    explicit            ErrorBarConverter( const ConverterRoot& rParent, ErrorBarModel& rModel );
    virtual             ~ErrorBarConverter();

    /** Converts an OOXML errorbar and inserts it into the passed data series. */
    void                convertFromModel(
                            const css::uno::Reference< css::chart2::XDataSeries >& rxDataSeries );

private:
    css::uno::Reference< css::chart2::data::XLabeledDataSequence >
                        createLabeledDataSequence( ErrorBarModel::SourceType eSourceType );
};



class TrendlineLabelConverter : public ConverterBase< TrendlineLabelModel >
{
public:
    explicit            TrendlineLabelConverter( const ConverterRoot& rParent, TrendlineLabelModel& rModel );
    virtual             ~TrendlineLabelConverter();

    /** Converts the OOXML trendline label. */
    void                convertFromModel( PropertySet& rPropSet );
};



class TrendlineConverter : public ConverterBase< TrendlineModel >
{
public:
    explicit            TrendlineConverter( const ConverterRoot& rParent, TrendlineModel& rModel );
    virtual             ~TrendlineConverter();

    /** Converts an OOXML trendline and inserts it into the passed data series. */
    void                convertFromModel(
                            const css::uno::Reference< css::chart2::XDataSeries >& rxDataSeries );
};



class DataPointConverter : public ConverterBase< DataPointModel >
{
public:
    explicit            DataPointConverter( const ConverterRoot& rParent, DataPointModel& rModel );
    virtual             ~DataPointConverter();

    /** Converts settings for a data point in the passed series. */
    void                convertFromModel(
                            const css::uno::Reference< css::chart2::XDataSeries >& rxDataSeries,
                            const TypeGroupConverter& rTypeGroup,
                            const SeriesModel& rSeries );
};



class SeriesConverter : public ConverterBase< SeriesModel >
{
public:
    explicit            SeriesConverter( const ConverterRoot& rParent, SeriesModel& rModel );
    virtual             ~SeriesConverter();

    /** Creates a labeled data sequence object from category data link. */
    css::uno::Reference< css::chart2::data::XLabeledDataSequence >
                        createCategorySequence( const OUString& rRole );
    /** Creates a labeled data sequence object from value data link. */
    css::uno::Reference< css::chart2::data::XLabeledDataSequence >
                        createValueSequence( const OUString& rRole );
    /** Creates a data series object with initialized source links. */
    css::uno::Reference< css::chart2::XDataSeries >
                        createDataSeries( const TypeGroupConverter& rTypeGroup, bool bVaryColorsByPoint );

private:
    css::uno::Reference< css::chart2::data::XLabeledDataSequence >
                        createLabeledDataSequence(
                            SeriesModel::SourceType eSourceType,
                            const OUString& rRole,
                            bool bUseTextLabel );
};



} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
