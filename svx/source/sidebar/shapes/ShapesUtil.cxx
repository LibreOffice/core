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
        ".uno:Line",
        ".uno:LineArrowEnd",
        ".uno:LineCircleArrow",
        ".uno:LineSquareArrow",
        ".uno:LineArrows",
        ".uno:LineArrowStart",
        ".uno:LineArrowCircle",
        ".uno:LineArrowSquare",
        ".uno:MeasureLine",
        ".uno:Line_Diagonal"
    };

    m_aCurveShapes = {
        ".uno:Freeline_Unfilled",
        ".uno:Bezier_Unfilled",
        ".uno:Polygon_Unfilled",
        ".uno:Polygon_Diagonal_Unfilled",
        ".uno:Freeline",
        ".uno:BezierFill",
        ".uno:Polygon",
        ".uno:Polygon_Diagonal"
    };

    m_aConnectorShapes = {
        ".uno:ConnectorArrowEnd",
        ".uno:ConnectorLineArrowEnd",
        ".uno:ConnectorCurveArrowEnd",
        ".uno:ConnectorLinesArrowEnd",
        ".uno:Connector",
        ".uno:ConnectorLine",
        ".uno:ConnectorCurve",
        ".uno:ConnectorLines",
        ".uno:ConnectorArrows",
        ".uno:ConnectorLineArrows",
        ".uno:ConnectorCurveArrows",
        ".uno:ConnectorLinesArrows"
    };

    m_aBasicShapes = {
        ".uno:BasicShapes.rectangle",
        ".uno:BasicShapes.round-rectangle",
        ".uno:BasicShapes.quadrat",
        ".uno:BasicShapes.round-quadrat",
        ".uno:BasicShapes.parallelogram",
        ".uno:BasicShapes.trapezoid",
        ".uno:BasicShapes.ellipse",
        ".uno:BasicShapes.circle",
        ".uno:BasicShapes.circle-pie",
        ".uno:CircleCut",
        ".uno:Arc",
        ".uno:BasicShapes.block-arc",
        ".uno:BasicShapes.isosceles-triangle",
        ".uno:BasicShapes.right-triangle",
        ".uno:BasicShapes.diamond",
        ".uno:BasicShapes.pentagon",
        ".uno:BasicShapes.hexagon",
        ".uno:BasicShapes.octagon",
        ".uno:BasicShapes.cross",
        ".uno:BasicShapes.can",
        ".uno:BasicShapes.cube",
        ".uno:BasicShapes.paper",
        ".uno:BasicShapes.frame",
        ".uno:BasicShapes.ring",
        ".uno:BasicShapes.sinusoid"
    };

    m_aSymbolShapes = {
        ".uno:SymbolShapes.smiley",
        ".uno:SymbolShapes.sun",
        ".uno:SymbolShapes.moon",
        ".uno:SymbolShapes.lightning",
        ".uno:SymbolShapes.heart",
        ".uno:SymbolShapes.flower",
        ".uno:SymbolShapes.cloud",
        ".uno:SymbolShapes.forbidden",
        ".uno:SymbolShapes.puzzle",
        ".uno:SymbolShapes.bracket-pair",
        ".uno:SymbolShapes.left-bracket",
        ".uno:SymbolShapes.right-bracket",
        ".uno:SymbolShapes.brace-pair",
        ".uno:SymbolShapes.left-brace",
        ".uno:SymbolShapes.right-brace",
        ".uno:SymbolShapes.quad-bevel",
        ".uno:SymbolShapes.octagon-bevel",
        ".uno:SymbolShapes.diamond-bevel"
    };

    m_aBlockArrowShapes = {
        ".uno:ArrowShapes.left-arrow",
        ".uno:ArrowShapes.right-arrow",
        ".uno:ArrowShapes.up-arrow",
        ".uno:ArrowShapes.down-arrow",
        ".uno:ArrowShapes.left-right-arrow",
        ".uno:ArrowShapes.up-down-arrow",
        ".uno:ArrowShapes.up-right-arrow",
        ".uno:ArrowShapes.up-right-down-arrow",
        ".uno:ArrowShapes.quad-arrow",
        ".uno:ArrowShapes.corner-right-arrow",
        ".uno:ArrowShapes.split-arrow",
        ".uno:ArrowShapes.striped-right-arrow",
        ".uno:ArrowShapes.notched-right-arrow",
        ".uno:ArrowShapes.pentagon-right",
        ".uno:ArrowShapes.chevron",
        ".uno:ArrowShapes.right-arrow-callout",
        ".uno:ArrowShapes.left-arrow-callout",
        ".uno:ArrowShapes.up-arrow-callout",
        ".uno:ArrowShapes.left-right-arrow-callout",
        ".uno:ArrowShapes.up-down-arrow-callout",
        ".uno:ArrowShapes.up-right-arrow-callout",
        ".uno:ArrowShapes.quad-arrow-callout",
        ".uno:ArrowShapes.circular-arrow",
        ".uno:ArrowShapes.down-arrow-callout",
        ".uno:ArrowShapes.split-round-arrow",
        ".uno:ArrowShapes.s-sharped-arrow"
    };

    m_aFlowchartShapes = {
        ".uno:FlowChartShapes.flowchart-process",
        ".uno:FlowChartShapes.flowchart-alternate-process",
        ".uno:FlowChartShapes.flowchart-decision",
        ".uno:FlowChartShapes.flowchart-data",
        ".uno:FlowChartShapes.flowchart-predefined-process",
        ".uno:FlowChartShapes.flowchart-internal-storage",
        ".uno:FlowChartShapes.flowchart-document",
        ".uno:FlowChartShapes.flowchart-multidocument",
        ".uno:FlowChartShapes.flowchart-terminator",
        ".uno:FlowChartShapes.flowchart-preparation",
        ".uno:FlowChartShapes.flowchart-manual-input",
        ".uno:FlowChartShapes.flowchart-manual-operation",
        ".uno:FlowChartShapes.flowchart-connector",
        ".uno:FlowChartShapes.flowchart-off-page-connector",
        ".uno:FlowChartShapes.flowchart-card",
        ".uno:FlowChartShapes.flowchart-punched-tape",
        ".uno:FlowChartShapes.flowchart-summing-junction",
        ".uno:FlowChartShapes.flowchart-or",
        ".uno:FlowChartShapes.flowchart-collate",
        ".uno:FlowChartShapes.flowchart-sort",
        ".uno:FlowChartShapes.flowchart-extract",
        ".uno:FlowChartShapes.flowchart-merge",
        ".uno:FlowChartShapes.flowchart-stored-data",
        ".uno:FlowChartShapes.flowchart-delay",
        ".uno:FlowChartShapes.flowchart-sequential-access",
        ".uno:FlowChartShapes.flowchart-magnetic-disk",
        ".uno:FlowChartShapes.flowchart-direct-access-storage",
        ".uno:FlowChartShapes.flowchart-display"
    };

    m_aCalloutShapes = {
        ".uno:CalloutShapes.rectangular-callout",
        ".uno:CalloutShapes.round-rectangular-callout",
        ".uno:CalloutShapes.round-callout",
        ".uno:CalloutShapes.cloud-callout",
        ".uno:CalloutShapes.line-callout-1",
        ".uno:CalloutShapes.line-callout-2",
        ".uno:CalloutShapes.line-callout-3"
    };

    m_aStarShapes = {
        ".uno:StarShapes.star4",
        ".uno:StarShapes.star5",
        ".uno:StarShapes.star6",
        ".uno:StarShapes.star8",
        ".uno:StarShapes.star12",
        ".uno:StarShapes.star24",
        ".uno:StarShapes.bang",
        ".uno:StarShapes.vertical-scroll",
        ".uno:StarShapes.horizontal-scroll",
        ".uno:StarShapes.signet",
        ".uno:StarShapes.doorplate",
        ".uno:StarShapes.concave-star6"
    };

    m_a3DShapes = {
        ".uno:Cube",
        ".uno:Sphere",
        ".uno:Cylinder",
        ".uno:Cone",
        ".uno:Cyramid",
        ".uno:Torus",
        ".uno:Shell3D",
        ".uno:HalfSphere"
    };
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
