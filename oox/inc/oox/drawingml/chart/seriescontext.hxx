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

#ifndef OOX_DRAWINGML_CHART_SERIESCONTEXT_HXX
#define OOX_DRAWINGML_CHART_SERIESCONTEXT_HXX

#include "oox/drawingml/chart/chartcontextbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct DataLabelModel;

/** Handler for a chart data point label context (c:dLbl element).
 */
class DataLabelContext : public ContextBase< DataLabelModel >
{
public:
    explicit            DataLabelContext( ::oox::core::ContextHandler2Helper& rParent, DataLabelModel& rModel );
    virtual             ~DataLabelContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );
};

// ============================================================================

struct DataLabelsModel;

/** Handler for a chart data point label context (c:dLbl element).
 */
class DataLabelsContext : public ContextBase< DataLabelsModel >
{
public:
    explicit            DataLabelsContext( ::oox::core::ContextHandler2Helper& rParent, DataLabelsModel& rModel );
    virtual             ~DataLabelsContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );
};

// ============================================================================

struct PictureOptionsModel;

/** Handler for fill bitmap settings (c:pictureOptions element).
 */
class PictureOptionsContext : public ContextBase< PictureOptionsModel >
{
public:
    explicit            PictureOptionsContext( ::oox::core::ContextHandler2Helper& rParent, PictureOptionsModel& rModel );
    virtual             ~PictureOptionsContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

struct ErrorBarModel;

/** Handler for a series error bar context (c:errBars element).
 */
class ErrorBarContext : public ContextBase< ErrorBarModel >
{
public:
    explicit            ErrorBarContext( ::oox::core::ContextHandler2Helper& rParent, ErrorBarModel& rModel );
    virtual             ~ErrorBarContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

struct TrendlineLabelModel;

/** Handler for a series trendline label context (c:trendlineLbl element).
 */
class TrendlineLabelContext : public ContextBase< TrendlineLabelModel >
{
public:
    explicit            TrendlineLabelContext( ::oox::core::ContextHandler2Helper& rParent, TrendlineLabelModel& rModel );
    virtual             ~TrendlineLabelContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

struct TrendlineModel;

/** Handler for a series trendline context (c:trendline element).
 */
class TrendlineContext : public ContextBase< TrendlineModel >
{
public:
    explicit            TrendlineContext( ::oox::core::ContextHandler2Helper& rParent, TrendlineModel& rModel );
    virtual             ~TrendlineContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );
};

// ============================================================================

struct DataPointModel;

/** Handler for a chart data point context (c:dPt element).
 */
class DataPointContext : public ContextBase< DataPointModel >
{
public:
    explicit            DataPointContext( ::oox::core::ContextHandler2Helper& rParent, DataPointModel& rModel );
    virtual             ~DataPointContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

struct SeriesModel;

/** Handler base class for chart data series contexts (c:ser element).
 */
class SeriesContextBase : public ContextBase< SeriesModel >
{
public:
    explicit            SeriesContextBase( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~SeriesContextBase();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a data series context for area chart types (c:ser element).
 */
class AreaSeriesContext : public SeriesContextBase
{
public:
    explicit            AreaSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~AreaSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a data series context for bar chart types (c:ser element).
 */
class BarSeriesContext : public SeriesContextBase
{
public:
    explicit            BarSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~BarSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a data series context for bubble chart types (c:ser element).
 */
class BubbleSeriesContext : public SeriesContextBase
{
public:
    explicit            BubbleSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~BubbleSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a data series context for line and stock chart types (c:ser
    element).
 */
class LineSeriesContext : public SeriesContextBase
{
public:
    explicit            LineSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~LineSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a data series context for pie and doughnut chart types (c:ser
    element).
 */
class PieSeriesContext : public SeriesContextBase
{
public:
    explicit            PieSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~PieSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a data series context for radar chart types (c:ser element).
 */
class RadarSeriesContext : public SeriesContextBase
{
public:
    explicit            RadarSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~RadarSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a data series context for scatter chart types (c:ser element).
 */
class ScatterSeriesContext : public SeriesContextBase
{
public:
    explicit            ScatterSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~ScatterSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

/** Handler for a data series context for scatter chart types (c:ser element).
 */
class SurfaceSeriesContext : public SeriesContextBase
{
public:
    explicit            SurfaceSeriesContext( ::oox::core::ContextHandler2Helper& rParent, SeriesModel& rModel );
    virtual             ~SurfaceSeriesContext();

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

