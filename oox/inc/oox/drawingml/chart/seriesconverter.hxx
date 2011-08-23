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

#ifndef OOX_DRAWINGML_CHART_SERIESCONVERTER_HXX
#define OOX_DRAWINGML_CHART_SERIESCONVERTER_HXX

#include "oox/drawingml/chart/converterbase.hxx"
#include "oox/drawingml/chart/seriesmodel.hxx"

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

// ============================================================================

class DataLabelConverter : public ConverterBase< DataLabelModel >
{
public:
    explicit            DataLabelConverter( const ConverterRoot& rParent, DataLabelModel& rModel );
    virtual             ~DataLabelConverter();

    /** Converts OOXML data label settings for the passed data point. */
    void                convertFromModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& rxDataSeries,
                            const TypeGroupConverter& rTypeGroup );

    /** Conversion helper for data series and data points. */
    static void         convertLabelFormatting(
                            PropertySet& rPropSet,
                            ObjectFormatter& rFormatter,
                            const DataLabelModelBase& rDataLabel,
                            const TypeGroupConverter& rTypeGroup,
                            bool bDataSeriesLabel );
};

// ============================================================================

class DataLabelsConverter : public ConverterBase< DataLabelsModel >
{
public:
    explicit            DataLabelsConverter( const ConverterRoot& rParent, DataLabelsModel& rModel );
    virtual             ~DataLabelsConverter();

    /** Converts OOXML data label settings for the passed data series. */
    void                convertFromModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& rxDataSeries,
                            const TypeGroupConverter& rTypeGroup );
};

// ============================================================================

class ErrorBarConverter : public ConverterBase< ErrorBarModel >
{
public:
    explicit            ErrorBarConverter( const ConverterRoot& rParent, ErrorBarModel& rModel );
    virtual             ~ErrorBarConverter();

    /** Converts an OOXML errorbar and inserts it into the passed data series. */
    void                convertFromModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& rxDataSeries );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >
                        createLabeledDataSequence( ErrorBarModel::SourceType eSourceType );
};

// ============================================================================

class TrendlineLabelConverter : public ConverterBase< TrendlineLabelModel >
{
public:
    explicit            TrendlineLabelConverter( const ConverterRoot& rParent, TrendlineLabelModel& rModel );
    virtual             ~TrendlineLabelConverter();

    /** Converts the OOXML trendline label. */
    void                convertFromModel( PropertySet& rPropSet );
};

// ============================================================================

class TrendlineConverter : public ConverterBase< TrendlineModel >
{
public:
    explicit            TrendlineConverter( const ConverterRoot& rParent, TrendlineModel& rModel );
    virtual             ~TrendlineConverter();

    /** Converts an OOXML trendline and inserts it into the passed data series. */
    void                convertFromModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& rxDataSeries );
};

// ============================================================================

class DataPointConverter : public ConverterBase< DataPointModel >
{
public:
    explicit            DataPointConverter( const ConverterRoot& rParent, DataPointModel& rModel );
    virtual             ~DataPointConverter();

    /** Converts settings for a data point in the passed series. */
    void                convertFromModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& rxDataSeries,
                            const TypeGroupConverter& rTypeGroup,
                            const SeriesModel& rSeries );
};

// ============================================================================

class SeriesConverter : public ConverterBase< SeriesModel >
{
public:
    explicit            SeriesConverter( const ConverterRoot& rParent, SeriesModel& rModel );
    virtual             ~SeriesConverter();

    /** Creates a labeled data sequence object from category data link. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >
                        createCategorySequence( const ::rtl::OUString& rRole );
    /** Creates a labeled data sequence object from value data link. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >
                        createValueSequence( const ::rtl::OUString& rRole );
    /** Creates a data series object with initialized source links. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >
                        createDataSeries( const TypeGroupConverter& rTypeGroup, bool bVaryColorsByPoint );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >
                        createLabeledDataSequence(
                            SeriesModel::SourceType eSourceType,
                            const ::rtl::OUString& rRole,
                            bool bUseTextLabel );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
