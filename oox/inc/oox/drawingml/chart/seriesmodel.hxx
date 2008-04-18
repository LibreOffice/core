/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: seriesmodel.hxx,v $
 * $Revision: 1.4 $
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

#ifndef OOX_DRAWINGML_CHART_SERIESMODEL_HXX
#define OOX_DRAWINGML_CHART_SERIESMODEL_HXX

#include "oox/drawingml/chart/datasourcemodel.hxx"
#include "oox/drawingml/chart/titlemodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct DataLabelModelBase
{
    typedef ModelRef< Shape >       ShapeRef;
    typedef ModelRef< TextBody >    TextBodyRef;

    ShapeRef            mxShapeProp;        /// Data label frame formatting.
    TextBodyRef         mxTextProp;         /// Data label text formatting.
    OptString           moaFormatCode;      /// Number format for numeric labels.
    OptString           moaSeparator;       /// Separator between label components.
    OptInt32            monLabelPos;        /// Data label position.
    OptBool             mobShowBubbleSize;  /// True = show size of bubbles in bubble charts.
    OptBool             mobShowCatName;     /// True = show category name of data points.
    OptBool             mobShowLegendKey;   /// True = show legend key of data series.
    OptBool             mobShowPercent;     /// True = show percentual value in pie/doughnut charts.
    OptBool             mobShowSerName;     /// True = show series name.
    OptBool             mobShowVal;         /// True = show data point value.
    OptBool             mobSourceLinked;    /// True = number format linked to source data.
    bool                mbDeleted;          /// True = data label(s) deleted.

    explicit            DataLabelModelBase();
                        ~DataLabelModelBase();
};

// ============================================================================

struct DataLabelModel : public DataLabelModelBase
{
    typedef ModelRef< LayoutModel > LayoutRef;
    typedef ModelRef< TextModel >   TextRef;

    LayoutRef           mxLayout;           /// Layout/position of the data point label frame.
    TextRef             mxText;             /// Manual or linked text for this data point label.
    sal_Int32           mnIndex;            /// Data point index for this data label.

    explicit            DataLabelModel();
                        ~DataLabelModel();
};

// ============================================================================

struct DataLabelsModel : public DataLabelModelBase
{
    typedef ModelVector< DataLabelModel >   DataLabelVector;
    typedef ModelRef< Shape >               ShapeRef;

    DataLabelVector     maPointLabels;      /// Settings for individual data point labels.
    ShapeRef            mxLeaderLines;      /// Formatting of connector lines between data points and labels.
    OptBool             mobShowLeaderLines; /// True = show connector lines between data points and labels.

    explicit            DataLabelsModel();
                        ~DataLabelsModel();
};

// ============================================================================

struct ErrorBarModel
{
    enum SourceType
    {
        PLUS,               /// Plus error bar values.
        MINUS               /// Minus error bar values.
    };

    typedef ModelMap< SourceType, DataSourceModel > DataSourceMap;
    typedef ModelRef< Shape >                       ShapeRef;

    DataSourceMap       maSources;          /// Source ranges for manual error bar values.
    ShapeRef            mxShapeProp;        /// Error line formatting.
    double              mfValue;            /// Fixed value for several error bar types.
    sal_Int32           mnDirection;        /// Direction of the error bars (x/y).
    sal_Int32           mnTypeId;           /// Type of the error bars (plus/minus/both).
    sal_Int32           mnValueType;        /// Type of the values.
    bool                mbNoEndCap;         /// True = no end cap at error bar lines.

    explicit            ErrorBarModel();
                        ~ErrorBarModel();
};

// ============================================================================

struct TrendlineModel
{
    typedef ModelRef< Shape > ShapeRef;

    ShapeRef            mxShapeProp;        /// Trendline formatting.
    ::rtl::OUString     maName;             /// User-defined name of the trendline.
    OptDouble           mfBackward;         /// Size of trendline before first data point.
    OptDouble           mfForward;          /// Size of trendline behind last data point.
    OptDouble           mfIntercept;        /// Crossing point with Y axis.
    sal_Int32           mnOrder;            /// Polynomial order in range [2, 6].
    sal_Int32           mnPeriod;           /// Moving average period in range [2, 255].
    sal_Int32           mnTypeId;           /// Type of the trendline.
    bool                mbDispEquation;     /// True = show equation of the trendline.
    bool                mbDispRSquared;     /// True = show R-squared of the trendline.

    explicit            TrendlineModel();
                        ~TrendlineModel();
};

// ============================================================================

struct DataPointModel
{
    typedef ModelRef< Shape > ShapeRef;

    ShapeRef            mxShapeProp;        /// Data point formatting.
    ShapeRef            mxMarkerProp;       /// Data point marker formatting.
    OptInt32            monExplosion;       /// Pie slice moved from pie center.
    OptInt32            monMarkerSize;      /// Size of the series line marker (2...72).
    OptInt32            monMarkerSymbol;    /// Series line marker symbol.
    OptBool             mobBubble3d;        /// True = show bubbles with 3D shade.
    OptBool             mobInvertNeg;       /// True = invert negative data points.
    sal_Int32           mnIndex;            /// Unique data point index.

    explicit            DataPointModel();
                        ~DataPointModel();
};

// ============================================================================

struct SeriesModel
{
    enum SourceType
    {
        CATEGORIES,         /// Data point categories.
        VALUES,             /// Data point values.
        POINTS              /// Data point size (e.g. bubble size in bubble charts).
    };

    typedef ModelMap< SourceType, DataSourceModel > DataSourceMap;
    typedef ModelVector< ErrorBarModel >            ErrorBarVector;
    typedef ModelVector< TrendlineModel >           TrendlineVector;
    typedef ModelVector< DataPointModel >           DataPointVector;
    typedef ModelRef< Shape >                       ShapeRef;
    typedef ModelRef< TextModel >                   TextRef;
    typedef ModelRef< DataLabelsModel >             DataLabelsRef;

    DataSourceMap       maSources;          /// Series source ranges.
    ErrorBarVector      maErrorBars;        /// All error bars of this series.
    TrendlineVector     maTrendlines;       /// All trendlines of this series.
    DataPointVector     maPoints;           /// Explicit formatted data points.
    ShapeRef            mxShapeProp;        /// Series formatting.
    ShapeRef            mxMarkerProp;       /// Data point marker formatting.
    TextRef             mxText;             /// Series title source.
    DataLabelsRef       mxLabels;           /// Data point label settings for all points.
    OptInt32            monShape;           /// 3D bar shape type.
    OptBool             mobBubble3d;        /// True = show bubbles with 3D shade.
    OptBool             mobSmooth;          /// True = smooth series line.
    sal_Int32           mnExplosion;        /// Pie slice moved from pie center.
    sal_Int32           mnIndex;            /// Unique series index.
    sal_Int32           mnMarkerSize;       /// Size of the series line marker (2...72).
    sal_Int32           mnMarkerSymbol;     /// Series line marker symbol.
    sal_Int32           mnOrder;            /// Series order used for automatic formatting.
    bool                mbInvertNeg;        /// True = invert negative data points.

    explicit            SeriesModel();
                        ~SeriesModel();
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

