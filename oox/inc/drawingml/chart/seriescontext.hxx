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

namespace oox {
namespace drawingml {
namespace chart {



struct DataLabelModel;

/** Handler for a chart data point label context (c:dLbl element).
 */
class DataLabelContext : public ContextBase< DataLabelModel >
{
public:
    explicit            DataLabelContext( ::oox::core::ContextHandler2Helper& rParent, DataLabelModel& rModel );
    virtual             ~DataLabelContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
};



struct DataLabelsModel;

/** Handler for a chart data point label context (c:dLbls element).
 */
class DataLabelsContext : public ContextBase< DataLabelsModel >
{
public:
    explicit            DataLabelsContext( ::oox::core::ContextHandler2Helper& rParent, DataLabelsModel& rModel );
    virtual             ~DataLabelsContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
};



struct PictureOptionsModel;

/** Handler for fill bitmap settings (c:pictureOptions element).
 */
class PictureOptionsContext : public ContextBase< PictureOptionsModel >
{
public:
    explicit            PictureOptionsContext( ::oox::core::ContextHandler2Helper& rParent, PictureOptionsModel& rModel );
    virtual             ~PictureOptionsContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



struct ErrorBarModel;

/** Handler for a series error bar context (c:errBars element).
 */
class ErrorBarContext : public ContextBase< ErrorBarModel >
{
public:
    explicit            ErrorBarContext( ::oox::core::ContextHandler2Helper& rParent, ErrorBarModel& rModel );
    virtual             ~ErrorBarContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



struct TrendlineLabelModel;

/** Handler for a series trendline label context (c:trendlineLbl element).
 */
class TrendlineLabelContext : public ContextBase< TrendlineLabelModel >
{
public:
    explicit            TrendlineLabelContext( ::oox::core::ContextHandler2Helper& rParent, TrendlineLabelModel& rModel );
    virtual             ~TrendlineLabelContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



struct TrendlineModel;

/** Handler for a series trendline context (c:trendline element).
 */
class TrendlineContext : public ContextBase< TrendlineModel >
{
public:
    explicit            TrendlineContext( ::oox::core::ContextHandler2Helper& rParent, TrendlineModel& rModel );
    virtual             ~TrendlineContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
};



struct DataPointModel;

/** Handler for a chart data point context (c:dPt element).
 */
class DataPointContext : public ContextBase< DataPointModel >
{
public:
    explicit            DataPointContext( ::oox::core::ContextHandler2Helper& rParent, DataPointModel& rModel );
    virtual             ~DataPointContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



struct SeriesModel;

/** Handler base class for chart data series contexts (c:ser element).
 */
class SeriesContextBase : public ContextBase< SeriesModel >
{
public:
    explicit            SeriesContextBase( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~SeriesContextBase();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



/** Handler for a data series context for area chart types (c:ser element).
 */
class AreaSeriesContext : public SeriesContextBase
{
public:
    explicit            AreaSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~AreaSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



/** Handler for a data series context for bar chart types (c:ser element).
 */
class BarSeriesContext : public SeriesContextBase
{
public:
    explicit            BarSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~BarSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



/** Handler for a data series context for bubble chart types (c:ser element).
 */
class BubbleSeriesContext : public SeriesContextBase
{
public:
    explicit            BubbleSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~BubbleSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



/** Handler for a data series context for line and stock chart types (c:ser
    element).
 */
class LineSeriesContext : public SeriesContextBase
{
public:
    explicit            LineSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~LineSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



/** Handler for a data series context for pie and doughnut chart types (c:ser
    element).
 */
class PieSeriesContext : public SeriesContextBase
{
public:
    explicit            PieSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~PieSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



/** Handler for a data series context for radar chart types (c:ser element).
 */
class RadarSeriesContext : public SeriesContextBase
{
public:
    explicit            RadarSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~RadarSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



/** Handler for a data series context for scatter chart types (c:ser element).
 */
class ScatterSeriesContext : public SeriesContextBase
{
public:
    explicit            ScatterSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~ScatterSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



/** Handler for a data series context for scatter chart types (c:ser element).
 */
class SurfaceSeriesContext : public SeriesContextBase
{
public:
    explicit            SurfaceSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~SurfaceSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
};



} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
