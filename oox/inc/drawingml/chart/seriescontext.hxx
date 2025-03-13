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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_SERIESCONTEXT_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_SERIESCONTEXT_HXX

#include <drawingml/chart/chartcontextbase.hxx>

namespace oox::drawingml::chart {


struct DataLabelModel;

/** Handler for a chart data point label context (c:dLbl element).
 */
class DataLabelContext final : public ContextBase< DataLabelModel >
{
public:
    explicit            DataLabelContext( ::oox::core::ContextHandler2Helper& rParent, DataLabelModel& rModel );
    virtual             ~DataLabelContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
};


struct DataLabelsModel;

/** Handler for a chart data point label context (c:dLbls element).
 */
class DataLabelsContext final : public ContextBase< DataLabelsModel >
{
public:
    explicit            DataLabelsContext( ::oox::core::ContextHandler2Helper& rParent, DataLabelsModel& rModel );
    virtual             ~DataLabelsContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
};


struct PictureOptionsModel;

/** Handler for fill bitmap settings (c:pictureOptions element).
 */
class PictureOptionsContext final : public ContextBase< PictureOptionsModel >
{
public:
    explicit            PictureOptionsContext( ::oox::core::ContextHandler2Helper& rParent, PictureOptionsModel& rModel );
    virtual             ~PictureOptionsContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


struct ErrorBarModel;

/** Handler for a series error bar context (c:errBars element).
 */
class ErrorBarContext final : public ContextBase< ErrorBarModel >
{
public:
    explicit            ErrorBarContext( ::oox::core::ContextHandler2Helper& rParent, ErrorBarModel& rModel );
    virtual             ~ErrorBarContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


struct TrendlineLabelModel;

/** Handler for a series trendline label context (c:trendlineLbl element).
 */
class TrendlineLabelContext final : public ContextBase< TrendlineLabelModel >
{
public:
    explicit            TrendlineLabelContext( ::oox::core::ContextHandler2Helper& rParent, TrendlineLabelModel& rModel );
    virtual             ~TrendlineLabelContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


struct TrendlineModel;

/** Handler for a series trendline context (c:trendline element).
 */
class TrendlineContext final : public ContextBase< TrendlineModel >
{
public:
    explicit            TrendlineContext( ::oox::core::ContextHandler2Helper& rParent, TrendlineModel& rModel );
    virtual             ~TrendlineContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
};


struct DataPointModel;

/** Handler for a chart data point context (c:dPt element).
 */
class DataPointContext final : public ContextBase< DataPointModel >
{
public:
    explicit            DataPointContext( ::oox::core::ContextHandler2Helper& rParent, DataPointModel& rModel );
    virtual             ~DataPointContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


struct SeriesModel;

/** Handler base class for chart data series contexts (c:ser element).
 */
class SeriesContextBase : public ContextBase< SeriesModel >
{
public:
    explicit            SeriesContextBase( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~SeriesContextBase() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for a data series context for area chart types (c:ser element).
 */
class AreaSeriesContext final : public SeriesContextBase
{
public:
    explicit            AreaSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~AreaSeriesContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for a data series context for bar chart types (c:ser element).
 */
class BarSeriesContext final : public SeriesContextBase
{
public:
    explicit            BarSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~BarSeriesContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for a data series context for bubble chart types (c:ser element).
 */
class BubbleSeriesContext final : public SeriesContextBase
{
public:
    explicit            BubbleSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~BubbleSeriesContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for a data series context for line and stock chart types (c:ser
    element).
 */
class LineSeriesContext final : public SeriesContextBase
{
public:
    explicit            LineSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~LineSeriesContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for a data series context for pie and doughnut chart types (c:ser
    element).
 */
class PieSeriesContext final : public SeriesContextBase
{
public:
    explicit            PieSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~PieSeriesContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for a data series context for radar chart types (c:ser element).
 */
class RadarSeriesContext final : public SeriesContextBase
{
public:
    explicit            RadarSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~RadarSeriesContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for a data series context for scatter chart types (c:ser element).
 */
class ScatterSeriesContext final : public SeriesContextBase
{
public:
    explicit            ScatterSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~ScatterSeriesContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};


/** Handler for a data series context for scatter chart types (c:ser element).
 */
class SurfaceSeriesContext final : public SeriesContextBase
{
public:
    explicit            SurfaceSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~SurfaceSeriesContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};

/** Handler for a data series context for chartex chart types (cx:series element
 * for boxWhisker, clusteredColumn/histogram, funnel, paretoLine, waterfall,
 * sunburst, treemap, and regionMap).
 */
class ChartexSeriesContext final : public SeriesContextBase
{
public:
    explicit            ChartexSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~ChartexSeriesContext() override;

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



} // namespace oox::drawingml::chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
