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

#ifndef OOX_DRAWINGML_CHART_TYPEGROUPMODEL_HXX
#define OOX_DRAWINGML_CHART_TYPEGROUPMODEL_HXX

#include "oox/drawingml/chart/seriesmodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct UpDownBarsModel
{
    typedef ModelRef< Shape > ShapeRef;

    ShapeRef            mxDownBars;         /// Formatting of down bars.
    ShapeRef            mxUpBars;           /// Formatting of up bars.
    sal_Int32           mnGapWidth;         /// Space between up/down bars.

    explicit            UpDownBarsModel();
                        ~UpDownBarsModel();
};

// ============================================================================

struct TypeGroupModel
{
    typedef ModelVector< SeriesModel >  SeriesVector;
    typedef ::std::vector< sal_Int32 >  AxisIdVector;
    typedef ModelRef< DataLabelsModel > DataLabelsRef;
    typedef ModelRef< UpDownBarsModel > UpDownBarsRef;
    typedef ModelRef< Shape >           ShapeRef;

    SeriesVector        maSeries;           /// Series attached to this chart type group.
    AxisIdVector        maAxisIds;          /// List of axis identifiers used by this chart type.
    DataLabelsRef       mxLabels;           /// Data point label settings for all series.
    UpDownBarsRef       mxUpDownBars;       /// Up/down bars in stock charts.
    ShapeRef            mxSerLines;         /// Connector lines in stacked bar charts.
    ShapeRef            mxDropLines;        /// Drop lines connecting data points with X axis.
    ShapeRef            mxHiLowLines;       /// High/low lines connecting lowest and highest data points.
    double              mfSplitPos;         /// Threshold value in pie-to charts.
    sal_Int32           mnBarDir;           /// Bar direction in bar charts (vertical/horizontal).
    sal_Int32           mnBubbleScale;      /// Relative scaling of bubble size (percent).
    sal_Int32           mnFirstAngle;       /// Rotation angle of first slice in pie charts.
    sal_Int32           mnGapDepth;         /// Space between series in deep 3D charts.
    sal_Int32           mnGapWidth;         /// Space between bars in bar charts, or space in pie-to charts.
    sal_Int32           mnGrouping;         /// Series grouping mode.
    sal_Int32           mnHoleSize;         /// Hole size in doughnut charts.
    sal_Int32           mnOfPieType;        /// Pie-to-pie or pie-to-bar chart.
    sal_Int32           mnOverlap;          /// Bar overlap per category (2D bar charts only).
    sal_Int32           mnRadarStyle;       /// Type of radar chart (lines, markers, filled).
    sal_Int32           mnScatterStyle;     /// Type of scatter chart (lines, markers, smooth).
    sal_Int32           mnSecondPieSize;    /// relative size of second pie/bar in pie-to charts (percent).
    sal_Int32           mnShape;            /// 3D bar shape type.
    sal_Int32           mnSizeRepresents;   /// Bubble size represents area or width.
    sal_Int32           mnSplitType;        /// Split type in pie-to charts.
    sal_Int32           mnTypeId;           /// Chart type identifier.
    bool                mbBubble3d;         /// True = show bubbles with 3D shade.
    bool                mbShowMarker;       /// True = show point markers in line charts.
    bool                mbShowNegBubbles;   /// True = show absolute value of negative bubbles.
    bool                mbSmooth;           /// True = smooth lines in line charts.
    bool                mbVaryColors;       /// True = different automatic colors for each point.
    bool                mbWireframe;        /// True = wireframe surface chart, false = filled surface chart.

    explicit            TypeGroupModel( sal_Int32 nTypeId );
                        ~TypeGroupModel();
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

