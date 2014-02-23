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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_SERIESMODEL_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_SERIESMODEL_HXX

#include <oox/drawingml/chart/datasourcemodel.hxx>
#include <oox/drawingml/chart/titlemodel.hxx>

namespace oox {
namespace drawingml {
namespace chart {



struct DataLabelModelBase
{
    typedef ModelRef< Shape >       ShapeRef;
    typedef ModelRef< TextBody >    TextBodyRef;

    ShapeRef            mxShapeProp;        /// Data label frame formatting.
    TextBodyRef         mxTextProp;         /// Data label text formatting.
    NumberFormat        maNumberFormat;     /// Number format for numeric data labels.
    OptValue< OUString > moaSeparator;/// Separator between label components.
    OptValue< sal_Int32 > monLabelPos;      /// Data label position.
    OptValue< bool >    mobShowBubbleSize;  /// True = show size of bubbles in bubble charts.
    OptValue< bool >    mobShowCatName;     /// True = show category name of data points.
    OptValue< bool >    mobShowLegendKey;   /// True = show legend key of data series.
    OptValue< bool >    mobShowPercent;     /// True = show percentual value in pie/doughnut charts.
    OptValue< bool >    mobShowSerName;     /// True = show series name.
    OptValue< bool >    mobShowVal;         /// True = show data point value.
    bool                mbDeleted;          /// True = data label(s) deleted.

    explicit            DataLabelModelBase();
                        ~DataLabelModelBase();
};



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



struct DataLabelsModel : public DataLabelModelBase
{
    typedef ModelVector< DataLabelModel >   DataLabelVector;
    typedef ModelRef< Shape >               ShapeRef;

    DataLabelVector     maPointLabels;      /// Settings for individual data point labels.
    ShapeRef            mxLeaderLines;      /// Formatting of connector lines between data points and labels.
    bool                mbShowLeaderLines;  /// True = show connector lines between data points and labels.

    explicit            DataLabelsModel();
                        ~DataLabelsModel();
};



struct PictureOptionsModel
{
    double              mfStackUnit;        /// Bitmap stacking unit.
    sal_Int32           mnPictureFormat;    /// Bitmap mode (stretch/tile).
    bool                mbApplyToFront;     /// True = draw picture at front/back side of 3D data points.
    bool                mbApplyToSides;     /// True = draw picture at left/right side of 3D data points.
    bool                mbApplyToEnd;       /// True = draw picture at top/bottom side of 3D data points.

    explicit            PictureOptionsModel();
                        ~PictureOptionsModel();
};



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



struct TrendlineLabelModel
{
    typedef ModelRef< Shape >       ShapeRef;
    typedef ModelRef< TextBody >    TextBodyRef;
    typedef ModelRef< LayoutModel > LayoutRef;
    typedef ModelRef< TextModel >   TextRef;

    ShapeRef            mxShapeProp;        /// Label frame formatting.
    TextBodyRef         mxTextProp;         /// Label text formatting.
    LayoutRef           mxLayout;           /// Layout/position of the frame.
    TextRef             mxText;             /// Text source of the label.
    NumberFormat        maNumberFormat;     /// Number format for coefficients.

    explicit            TrendlineLabelModel();
                        ~TrendlineLabelModel();
};



struct TrendlineModel
{
    typedef ModelRef< Shape >               ShapeRef;
    typedef ModelRef< TrendlineLabelModel > TrendlineLabelRef;

    ShapeRef            mxShapeProp;        /// Trendline formatting.
    TrendlineLabelRef   mxLabel;            /// Trendline label text object.
    OUString            maName;             /// User-defined name of the trendline.
    OptValue< double >  mfBackward;         /// Size of trendline before first data point.
    OptValue< double >  mfForward;          /// Size of trendline behind last data point.
    OptValue< double >  mfIntercept;        /// Crossing point with Y axis.
    sal_Int32           mnOrder;            /// Polynomial order in range [2, 6].
    sal_Int32           mnPeriod;           /// Moving average period in range [2, 255].
    sal_Int32           mnTypeId;           /// Type of the trendline.
    bool                mbDispEquation;     /// True = show equation of the trendline.
    bool                mbDispRSquared;     /// True = show R-squared of the trendline.

    explicit            TrendlineModel();
                        ~TrendlineModel();
};



struct DataPointModel
{
    typedef ModelRef< Shape >               ShapeRef;
    typedef ModelRef< PictureOptionsModel > PictureOptionsRef;

    ShapeRef            mxShapeProp;        /// Data point formatting.
    PictureOptionsRef   mxPicOptions;       /// Fill bitmap settings.
    ShapeRef            mxMarkerProp;       /// Data point marker formatting.
    OptValue< sal_Int32 > monExplosion;     /// Pie slice moved from pie center.
    OptValue< sal_Int32 > monMarkerSize;    /// Size of the series line marker (2...72).
    OptValue< sal_Int32 > monMarkerSymbol;  /// Series line marker symbol.
    OptValue< bool >    mobBubble3d;        /// True = show bubbles with 3D shade.
    sal_Int32           mnIndex;            /// Unique data point index.
    bool                mbInvertNeg;        /// True = invert negative data points (not derived from series!).

    explicit            DataPointModel();
                        ~DataPointModel();
};



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
    typedef ModelRef< PictureOptionsModel >         PictureOptionsRef;
    typedef ModelRef< TextModel >                   TextRef;
    typedef ModelRef< DataLabelsModel >             DataLabelsRef;

    DataSourceMap       maSources;          /// Series source ranges.
    ErrorBarVector      maErrorBars;        /// All error bars of this series.
    TrendlineVector     maTrendlines;       /// All trendlines of this series.
    DataPointVector     maPoints;           /// Explicit formatted data points.
    ShapeRef            mxShapeProp;        /// Series formatting.
    PictureOptionsRef   mxPicOptions;       /// Fill bitmap settings.
    ShapeRef            mxMarkerProp;       /// Data point marker formatting.
    TextRef             mxText;             /// Series title source.
    DataLabelsRef       mxLabels;           /// Data point label settings for all points.
    OptValue< sal_Int32 > monShape;         /// 3D bar shape type.
    sal_Int32           mnExplosion;        /// Pie slice moved from pie center.
    sal_Int32           mnIndex;            /// Series index used for automatic formatting.
    sal_Int32           mnMarkerSize;       /// Size of the series line marker (2...72).
    sal_Int32           mnMarkerSymbol;     /// Series line marker symbol.
    sal_Int32           mnOrder;            /// Series order.
    bool                mbBubble3d;         /// True = show bubbles with 3D shade.
    bool                mbInvertNeg;        /// True = invert negative data points.
    bool                mbSmooth;           /// True = smooth series line.

    explicit            SeriesModel();
                        ~SeriesModel();
};



} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
