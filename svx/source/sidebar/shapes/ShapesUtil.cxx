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

#include <ShapesUtil.hxx>
#include <rtl/ustring.hxx>

namespace svx::sidebar{
SvxShapeCommandsMap::SvxShapeCommandsMap()
{
    m_aLineShapes = {
        u".uno:Line"_ustr,
        u".uno:LineArrowEnd"_ustr,
        u".uno:LineCircleArrow"_ustr,
        u".uno:LineSquareArrow"_ustr,
        u".uno:LineArrows"_ustr,
        u".uno:LineArrowStart"_ustr,
        u".uno:LineArrowCircle"_ustr,
        u".uno:LineArrowSquare"_ustr,
        u".uno:MeasureLine"_ustr,
        u".uno:Line_Diagonal"_ustr
    };

    m_aCurveShapes = {
        u".uno:Freeline_Unfilled"_ustr,
        u".uno:Bezier_Unfilled"_ustr,
        u".uno:Polygon_Unfilled"_ustr,
        u".uno:Polygon_Diagonal_Unfilled"_ustr,
        u".uno:Freeline"_ustr,
        u".uno:BezierFill"_ustr,
        u".uno:Polygon"_ustr,
        u".uno:Polygon_Diagonal"_ustr
    };

    m_aConnectorShapes = {
        u".uno:ConnectorArrowEnd"_ustr,
        u".uno:ConnectorLineArrowEnd"_ustr,
        u".uno:ConnectorCurveArrowEnd"_ustr,
        u".uno:ConnectorLinesArrowEnd"_ustr,
        u".uno:Connector"_ustr,
        u".uno:ConnectorLine"_ustr,
        u".uno:ConnectorCurve"_ustr,
        u".uno:ConnectorLines"_ustr,
        u".uno:ConnectorArrows"_ustr,
        u".uno:ConnectorLineArrows"_ustr,
        u".uno:ConnectorCurveArrows"_ustr,
        u".uno:ConnectorLinesArrows"_ustr
    };

    m_aBasicShapes = {
        u".uno:BasicShapes.rectangle"_ustr,
        u".uno:BasicShapes.round-rectangle"_ustr,
        u".uno:BasicShapes.quadrat"_ustr,
        u".uno:BasicShapes.round-quadrat"_ustr,
        u".uno:BasicShapes.parallelogram"_ustr,
        u".uno:BasicShapes.trapezoid"_ustr,
        u".uno:BasicShapes.ellipse"_ustr,
        u".uno:BasicShapes.circle"_ustr,
        u".uno:BasicShapes.circle-pie"_ustr,
        u".uno:CircleCut"_ustr,
        u".uno:Arc"_ustr,
        u".uno:BasicShapes.block-arc"_ustr,
        u".uno:BasicShapes.isosceles-triangle"_ustr,
        u".uno:BasicShapes.right-triangle"_ustr,
        u".uno:BasicShapes.diamond"_ustr,
        u".uno:BasicShapes.pentagon"_ustr,
        u".uno:BasicShapes.hexagon"_ustr,
        u".uno:BasicShapes.octagon"_ustr,
        u".uno:BasicShapes.cross"_ustr,
        u".uno:BasicShapes.can"_ustr,
        u".uno:BasicShapes.cube"_ustr,
        u".uno:BasicShapes.paper"_ustr,
        u".uno:BasicShapes.frame"_ustr,
        u".uno:BasicShapes.ring"_ustr,
        u".uno:BasicShapes.sinusoid"_ustr
    };

    m_aSymbolShapes = {
        u".uno:SymbolShapes.smiley"_ustr,
        u".uno:SymbolShapes.sun"_ustr,
        u".uno:SymbolShapes.moon"_ustr,
        u".uno:SymbolShapes.lightning"_ustr,
        u".uno:SymbolShapes.heart"_ustr,
        u".uno:SymbolShapes.flower"_ustr,
        u".uno:SymbolShapes.cloud"_ustr,
        u".uno:SymbolShapes.forbidden"_ustr,
        u".uno:SymbolShapes.puzzle"_ustr,
        u".uno:SymbolShapes.bracket-pair"_ustr,
        u".uno:SymbolShapes.left-bracket"_ustr,
        u".uno:SymbolShapes.right-bracket"_ustr,
        u".uno:SymbolShapes.brace-pair"_ustr,
        u".uno:SymbolShapes.left-brace"_ustr,
        u".uno:SymbolShapes.right-brace"_ustr,
        u".uno:SymbolShapes.quad-bevel"_ustr,
        u".uno:SymbolShapes.octagon-bevel"_ustr,
        u".uno:SymbolShapes.diamond-bevel"_ustr
    };

    m_aBlockArrowShapes = {
        u".uno:ArrowShapes.left-arrow"_ustr,
        u".uno:ArrowShapes.right-arrow"_ustr,
        u".uno:ArrowShapes.up-arrow"_ustr,
        u".uno:ArrowShapes.down-arrow"_ustr,
        u".uno:ArrowShapes.left-right-arrow"_ustr,
        u".uno:ArrowShapes.up-down-arrow"_ustr,
        u".uno:ArrowShapes.up-right-arrow"_ustr,
        u".uno:ArrowShapes.up-right-down-arrow"_ustr,
        u".uno:ArrowShapes.quad-arrow"_ustr,
        u".uno:ArrowShapes.corner-right-arrow"_ustr,
        u".uno:ArrowShapes.split-arrow"_ustr,
        u".uno:ArrowShapes.striped-right-arrow"_ustr,
        u".uno:ArrowShapes.notched-right-arrow"_ustr,
        u".uno:ArrowShapes.pentagon-right"_ustr,
        u".uno:ArrowShapes.chevron"_ustr,
        u".uno:ArrowShapes.right-arrow-callout"_ustr,
        u".uno:ArrowShapes.left-arrow-callout"_ustr,
        u".uno:ArrowShapes.up-arrow-callout"_ustr,
        u".uno:ArrowShapes.left-right-arrow-callout"_ustr,
        u".uno:ArrowShapes.up-down-arrow-callout"_ustr,
        u".uno:ArrowShapes.up-right-arrow-callout"_ustr,
        u".uno:ArrowShapes.quad-arrow-callout"_ustr,
        u".uno:ArrowShapes.circular-arrow"_ustr,
        u".uno:ArrowShapes.down-arrow-callout"_ustr,
        u".uno:ArrowShapes.split-round-arrow"_ustr,
        u".uno:ArrowShapes.s-sharped-arrow"_ustr
    };

    m_aFlowchartShapes = {
        u".uno:FlowChartShapes.flowchart-process"_ustr,
        u".uno:FlowChartShapes.flowchart-alternate-process"_ustr,
        u".uno:FlowChartShapes.flowchart-decision"_ustr,
        u".uno:FlowChartShapes.flowchart-data"_ustr,
        u".uno:FlowChartShapes.flowchart-predefined-process"_ustr,
        u".uno:FlowChartShapes.flowchart-internal-storage"_ustr,
        u".uno:FlowChartShapes.flowchart-document"_ustr,
        u".uno:FlowChartShapes.flowchart-multidocument"_ustr,
        u".uno:FlowChartShapes.flowchart-terminator"_ustr,
        u".uno:FlowChartShapes.flowchart-preparation"_ustr,
        u".uno:FlowChartShapes.flowchart-manual-input"_ustr,
        u".uno:FlowChartShapes.flowchart-manual-operation"_ustr,
        u".uno:FlowChartShapes.flowchart-connector"_ustr,
        u".uno:FlowChartShapes.flowchart-off-page-connector"_ustr,
        u".uno:FlowChartShapes.flowchart-card"_ustr,
        u".uno:FlowChartShapes.flowchart-punched-tape"_ustr,
        u".uno:FlowChartShapes.flowchart-summing-junction"_ustr,
        u".uno:FlowChartShapes.flowchart-or"_ustr,
        u".uno:FlowChartShapes.flowchart-collate"_ustr,
        u".uno:FlowChartShapes.flowchart-sort"_ustr,
        u".uno:FlowChartShapes.flowchart-extract"_ustr,
        u".uno:FlowChartShapes.flowchart-merge"_ustr,
        u".uno:FlowChartShapes.flowchart-stored-data"_ustr,
        u".uno:FlowChartShapes.flowchart-delay"_ustr,
        u".uno:FlowChartShapes.flowchart-sequential-access"_ustr,
        u".uno:FlowChartShapes.flowchart-magnetic-disk"_ustr,
        u".uno:FlowChartShapes.flowchart-direct-access-storage"_ustr,
        u".uno:FlowChartShapes.flowchart-display"_ustr
    };

    m_aCalloutShapes = {
        u".uno:CalloutShapes.rectangular-callout"_ustr,
        u".uno:CalloutShapes.round-rectangular-callout"_ustr,
        u".uno:CalloutShapes.round-callout"_ustr,
        u".uno:CalloutShapes.cloud-callout"_ustr,
        u".uno:CalloutShapes.line-callout-1"_ustr,
        u".uno:CalloutShapes.line-callout-2"_ustr,
        u".uno:CalloutShapes.line-callout-3"_ustr
    };

    m_aStarShapes = {
        u".uno:StarShapes.star4"_ustr,
        u".uno:StarShapes.star5"_ustr,
        u".uno:StarShapes.star6"_ustr,
        u".uno:StarShapes.star8"_ustr,
        u".uno:StarShapes.star12"_ustr,
        u".uno:StarShapes.star24"_ustr,
        u".uno:StarShapes.bang"_ustr,
        u".uno:StarShapes.vertical-scroll"_ustr,
        u".uno:StarShapes.horizontal-scroll"_ustr,
        u".uno:StarShapes.signet"_ustr,
        u".uno:StarShapes.doorplate"_ustr,
        u".uno:StarShapes.concave-star6"_ustr
    };

    m_a3DShapes = {
        u".uno:Cube"_ustr,
        u".uno:Sphere"_ustr,
        u".uno:Cylinder"_ustr,
        u".uno:Cone"_ustr,
        u".uno:Cyramid"_ustr,
        u".uno:Torus"_ustr,
        u".uno:Shell3D"_ustr,
        u".uno:HalfSphere"_ustr
    };
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
