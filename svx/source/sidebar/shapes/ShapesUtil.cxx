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
#include <map>
#include <rtl/ustring.hxx>

namespace svx::sidebar{
SvxShapeCommandsMap::SvxShapeCommandsMap()
{
    mpLineShapes = decltype(mpLineShapes){
        {0, ".uno:Line"},
        {1, ".uno:LineArrowEnd"},
        {2, ".uno:LineCircleArrow"},
        {3, ".uno:LineSquareArrow"},
        {4, ".uno:LineArrows"},
        {5, ".uno:LineArrowStart"},
        {6, ".uno:LineArrowCircle"},
        {7, ".uno:LineArrowSquare"},
        {8, ".uno:MeasureLine"},
        {9, ".uno:Line_Diagonal"}
    };

    mpCurveShapes = decltype(mpCurveShapes){
        {0, ".uno:Freeline_Unfilled"},
        {1, ".uno:Bezier_Unfilled"},
        {2, ".uno:Polygon_Unfilled"},
        {3, ".uno:Polygon_Diagonal_Unfilled"},
        {4, ".uno:Freeline"},
        {5, ".uno:BezierFill"},
        {6, ".uno:Polygon"},
        {7, ".uno:Polygon_Diagonal"}
    };

    mpConnectorShapes = decltype(mpConnectorShapes){
        {0, ".uno:ConnectorArrowEnd"},
        {1, ".uno:ConnectorLineArrowEnd"},
        {2, ".uno:ConnectorCurveArrowEnd"},
        {3, ".uno:ConnectorLinesArrowEnd"},
        {4, ".uno:Connector"},
        {5, ".uno:ConnectorLine"},
        {6, ".uno:ConnectorCurve"},
        {7, ".uno:ConnectorLines"},
        {8, ".uno:ConnectorArrows"},
        {9, ".uno:ConnectorLineArrows"},
        {10, ".uno:ConnectorCurveArrows"},
        {11, ".uno:ConnectorLinesArrows"}
    };

    mpBasicShapes = decltype(mpBasicShapes){
        {0, ".uno:BasicShapes.rectangle"},
        {1, ".uno:BasicShapes.round-rectangle"},
        {2, ".uno:BasicShapes.quadrat"},
        {3, ".uno:BasicShapes.round-quadrat"},
        {4, ".uno:BasicShapes.parallelogram"},
        {5, ".uno:BasicShapes.trapezoid"},
        {6, ".uno:BasicShapes.ellipse"},
        {7, ".uno:BasicShapes.circle"},
        {8, ".uno:BasicShapes.circle-pie"},
        {9, ".uno:CircleCut"},
        {10, ".uno:Arc"},
        {11, ".uno:BasicShapes.block-arc"},
        {12, ".uno:BasicShapes.isosceles-triangle"},
        {13, ".uno:BasicShapes.right-triangle"},
        {14, ".uno:BasicShapes.diamond"},
        {15, ".uno:BasicShapes.pentagon"},
        {16, ".uno:BasicShapes.hexagon"},
        {17, ".uno:BasicShapes.octagon"},
        {18, ".uno:BasicShapes.cross"},
        {19, ".uno:BasicShapes.can"},
        {20, ".uno:BasicShapes.cube"},
        {21, ".uno:BasicShapes.paper"},
        {22, ".uno:BasicShapes.frame"},
        {23, ".uno:BasicShapes.ring"},
        {24, ".uno:SymbolShapes.sinusoid"}
    };

    mpSymbolShapes = decltype(mpSymbolShapes){
        {0, ".uno:SymbolShapes.smiley"},
        {1, ".uno:SymbolShapes.sun"},
        {2, ".uno:SymbolShapes.moon"},
        {3, ".uno:SymbolShapes.lightning"},
        {4, ".uno:SymbolShapes.heart"},
        {5, ".uno:SymbolShapes.flower"},
        {6, ".uno:SymbolShapes.cloud"},
        {7, ".uno:SymbolShapes.forbidden"},
        {8, ".uno:SymbolShapes.puzzle"},
        {9, ".uno:SymbolShapes.bracket-pair"},
        {10, ".uno:SymbolShapes.left-bracket"},
        {11, ".uno:SymbolShapes.right-bracket"},
        {12, ".uno:SymbolShapes.brace-pair"},
        {13, ".uno:SymbolShapes.left-brace"},
        {14, ".uno:SymbolShapes.right-brace"},
        {15, ".uno:SymbolShapes.quad-bevel"},
        {16, ".uno:SymbolShapes.octagon-bevel"},
        {17, ".uno:SymbolShapes.diamond-bevel"}
    };

    mpBlockArrowShapes = decltype(mpBlockArrowShapes){
        {0, ".uno:ArrowShapes.left-arrow"},
        {1, ".uno:ArrowShapes.right-arrow"},
        {2, ".uno:ArrowShapes.up-arrow"},
        {3, ".uno:ArrowShapes.down-arrow"},
        {4, ".uno:ArrowShapes.left-right-arrow"},
        {5, ".uno:ArrowShapes.up-down-arrow"},
        {6, ".uno:ArrowShapes.up-right-arrow"},
        {7, ".uno:ArrowShapes.up-right-down-arrow"},
        {8, ".uno:ArrowShapes.quad-arrow"},
        {9, ".uno:ArrowShapes.corner-right-arrow"},
        {10, ".uno:ArrowShapes.split-arrow"},
        {11, ".uno:ArrowShapes.striped-right-arrow"},
        {12, ".uno:ArrowShapes.notched-right-arrow"},
        {13, ".uno:ArrowShapes.pentagon-right"},
        {14, ".uno:ArrowShapes.chevron"},
        {15, ".uno:ArrowShapes.right-arrow-callout"},
        {16, ".uno:ArrowShapes.left-arrow-callout"},
        {17, ".uno:ArrowShapes.up-arrow-callout"},
        {18, ".uno:ArrowShapes.left-right-arrow-callout"},
        {19, ".uno:ArrowShapes.up-down-arrow-callout"},
        {20, ".uno:ArrowShapes.up-right-arrow-callout"},
        {21, ".uno:ArrowShapes.quad-arrow-callout"},
        {22, ".uno:ArrowShapes.circular-arrow"},
        {23, ".uno:ArrowShapes.down-arrow-callout"},
        {24, ".uno:ArrowShapes.split-round-arrow"},
        {25, ".uno:ArrowShapes.s-sharped-arrow"}
    };

    mpFlowchartShapes = decltype(mpFlowchartShapes){
        {0, ".uno:FlowChartShapes.flowchart-process"},
        {1, ".uno:FlowChartShapes.flowchart-alternate-process"},
        {2, ".uno:FlowChartShapes.flowchart-decision"},
        {3, ".uno:FlowChartShapes.flowchart-data"},
        {4, ".uno:FlowChartShapes.flowchart-predefined-process"},
        {5, ".uno:FlowChartShapes.flowchart-internal-storage"},
        {6, ".uno:FlowChartShapes.flowchart-document"},
        {7, ".uno:FlowChartShapes.flowchart-multidocument"},
        {8, ".uno:FlowChartShapes.flowchart-terminator"},
        {9, ".uno:FlowChartShapes.flowchart-preparation"},
        {10, ".uno:FlowChartShapes.flowchart-manual-input"},
        {11, ".uno:FlowChartShapes.flowchart-manual-operation"},
        {12, ".uno:FlowChartShapes.flowchart-connector"},
        {13, ".uno:FlowChartShapes.flowchart-off-page-connector"},
        {14, ".uno:FlowChartShapes.flowchart-card"},
        {15, ".uno:FlowChartShapes.flowchart-punched-tape"},
        {16, ".uno:FlowChartShapes.flowchart-summing-junction"},
        {17, ".uno:FlowChartShapes.flowchart-or"},
        {18, ".uno:FlowChartShapes.flowchart-collate"},
        {19, ".uno:FlowChartShapes.flowchart-sort"},
        {20, ".uno:FlowChartShapes.flowchart-extract"},
        {21, ".uno:FlowChartShapes.flowchart-merge"},
        {22, ".uno:FlowChartShapes.flowchart-stored-data"},
        {23, ".uno:FlowChartShapes.flowchart-delay"},
        {24, ".uno:FlowChartShapes.flowchart-sequential-access"},
        {25, ".uno:FlowChartShapes.flowchart-magnetic-disk"},
        {26, ".uno:FlowChartShapes.flowchart-direct-access-storage"},
        {27, ".uno:FlowChartShapes.flowchart-display"}
    };

    mpCalloutShapes = decltype(mpCalloutShapes){
        {0, ".uno:CalloutShapes.rectangular-callout"},
        {1, ".uno:CalloutShapes.round-rectangular-callout"},
        {2, ".uno:CalloutShapes.round-callout"},
        {3, ".uno:CalloutShapes.cloud-callout"},
        {4, ".uno:CalloutShapes.line-callout-1"},
        {5, ".uno:CalloutShapes.line-callout-2"},
        {6, ".uno:CalloutShapes.line-callout-3"}
    };

    mpStarShapes = decltype(mpStarShapes){
        {0, ".uno:StarShapes.star4"},
        {1, ".uno:StarShapes.star5"},
        {2, ".uno:StarShapes.star6"},
        {3, ".uno:StarShapes.star8"},
        {4, ".uno:StarShapes.star12"},
        {5, ".uno:StarShapes.star24"},
        {6, ".uno:StarShapes.bang"},
        {7, ".uno:StarShapes.vertical-scroll"},
        {8, ".uno:StarShapes.horizontal-scroll"},
        {9, ".uno:StarShapes.signet"},
        {10, ".uno:StarShapes.doorplate"},
        {11, ".uno:StarShapes.concave-star6"}
    };

    mp3DShapes = decltype(mp3DShapes){
        {0, ".uno:Cube"},
        {1, ".uno:Sphere"},
        {2, ".uno:Cylinder"},
        {3, ".uno:Cone"},
        {4, ".uno:Cyramid"},
        {5, ".uno:Torus"},
        {6, ".uno:Shell3D"},
        {7, ".uno:HalfSphere"}
    };
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */