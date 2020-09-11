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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_TYPEGROUPMODEL_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_TYPEGROUPMODEL_HXX

#include <drawingml/chart/seriesmodel.hxx>

namespace oox::drawingml::chart {

struct UpDownBarsModel
{
    typedef ModelRef< Shape > ShapeRef;

    ShapeRef            mxDownBars;         /// Formatting of down bars.
    ShapeRef            mxUpBars;           /// Formatting of up bars.
    sal_Int32           mnGapWidth;         /// Space between up/down bars.

    explicit            UpDownBarsModel();
                        ~UpDownBarsModel();
};

struct TypeGroupModel
{
    typedef ModelVector< SeriesModel >  SeriesVector;
    typedef ModelRef< DataLabelsModel > DataLabelsRef;
    typedef ModelRef< UpDownBarsModel > UpDownBarsRef;
    typedef ModelRef< Shape >           ShapeRef;

    SeriesVector        maSeries;           /// Series attached to this chart type group.
    std::vector<sal_Int32>
                        maAxisIds;          /// List of axis identifiers used by this chart type.
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

    explicit            TypeGroupModel( sal_Int32 nTypeId, bool bMSO2007Doc );
                        ~TypeGroupModel();
};

} // namespace oox::drawingml::chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
