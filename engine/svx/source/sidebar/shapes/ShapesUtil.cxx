/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

namespace svx::sidebar::commands {

const std::map<sal_uInt16, OUString> gaLineShapes {
    {0, u".uno:Line"_ustr},
    {1, u".uno:LineArrowEnd"_ustr},
    {2, u".uno:LineCircleArrow"_ustr},
    {3, u".uno:LineSquareArrow"_ustr},
    {4, u".uno:LineArrows"_ustr},
    {5, u".uno:LineArrowStart"_ustr},
    {6, u".uno:LineArrowCircle"_ustr},
    {7, u".uno:LineArrowSquare"_ustr},
    {8, u".uno:MeasureLine"_ustr},
    {9, u".uno:Line_Diagonal"_ustr}
};

const std::map<sal_uInt16, OUString> gaCurveShapes {
        {0, u".uno:Freeline_Unfilled"_ustr},
        {1, u".uno:Bezier_Unfilled"_ustr},
        {2, u".uno:Polygon_Unfilled"_ustr},
        {3, u".uno:Polygon_Diagonal_Unfilled"_ustr},
        {4, u".uno:Freeline"_ustr},
        {5, u".uno:BezierFill"_ustr},
        {6, u".uno:Polygon"_ustr},
        {7, u".uno:Polygon_Diagonal"_ustr}
    };

const std::map<sal_uInt16, OUString> gaConnectorShapes {
        {0, u".uno:ConnectorArrowEnd"_ustr},
        {1, u".uno:ConnectorLineArrowEnd"_ustr},
        {2, u".uno:ConnectorCurveArrowEnd"_ustr},
        {3, u".uno:ConnectorLinesArrowEnd"_ustr},
        {4, u".uno:Connector"_ustr},
        {5, u".uno:ConnectorLine"_ustr},
        {6, u".uno:ConnectorCurve"_ustr},
        {7, u".uno:ConnectorLines"_ustr},
        {8, u".uno:ConnectorArrows"_ustr},
        {9, u".uno:ConnectorLineArrows"_ustr},
        {10, u".uno:ConnectorCurveArrows"_ustr},
        {11, u".uno:ConnectorLinesArrows"_ustr}
    };

const std::map<sal_uInt16, OUString> gaBasicShapes {
        {0, u".uno:BasicShapes.rectangle"_ustr},
        {1, u".uno:BasicShapes.round-rectangle"_ustr},
        {2, u".uno:BasicShapes.quadrat"_ustr},
        {3, u".uno:BasicShapes.round-quadrat"_ustr},
        {4, u".uno:BasicShapes.parallelogram"_ustr},
        {5, u".uno:BasicShapes.trapezoid"_ustr},
        {6, u".uno:BasicShapes.ellipse"_ustr},
        {7, u".uno:BasicShapes.circle"_ustr},
        {8, u".uno:BasicShapes.circle-pie"_ustr},
        {9, u".uno:CircleCut"_ustr},
        {10, u".uno:Arc"_ustr},
        {11, u".uno:BasicShapes.block-arc"_ustr},
        {12, u".uno:BasicShapes.isosceles-triangle"_ustr},
        {13, u".uno:BasicShapes.right-triangle"_ustr},
        {14, u".uno:BasicShapes.diamond"_ustr},
        {15, u".uno:BasicShapes.pentagon"_ustr},
        {16, u".uno:BasicShapes.hexagon"_ustr},
        {17, u".uno:BasicShapes.octagon"_ustr},
        {18, u".uno:BasicShapes.cross"_ustr},
        {19, u".uno:BasicShapes.can"_ustr},
        {20, u".uno:BasicShapes.cube"_ustr},
        {21, u".uno:BasicShapes.paper"_ustr},
        {22, u".uno:BasicShapes.frame"_ustr},
        {23, u".uno:BasicShapes.ring"_ustr},
        {24, u".uno:SymbolShapes.sinusoid"_ustr}
    };

const std::map<sal_uInt16, OUString> gaSymbolShapes {
        {0, u".uno:SymbolShapes.smiley"_ustr},
        {1, u".uno:SymbolShapes.sun"_ustr},
        {2, u".uno:SymbolShapes.moon"_ustr},
        {3, u".uno:SymbolShapes.lightning"_ustr},
        {4, u".uno:SymbolShapes.heart"_ustr},
        {5, u".uno:SymbolShapes.flower"_ustr},
        {6, u".uno:SymbolShapes.cloud"_ustr},
        {7, u".uno:SymbolShapes.forbidden"_ustr},
        {8, u".uno:SymbolShapes.puzzle"_ustr},
        {9, u".uno:SymbolShapes.bracket-pair"_ustr},
        {10, u".uno:SymbolShapes.left-bracket"_ustr},
        {11, u".uno:SymbolShapes.right-bracket"_ustr},
        {12, u".uno:SymbolShapes.brace-pair"_ustr},
        {13, u".uno:SymbolShapes.left-brace"_ustr},
        {14, u".uno:SymbolShapes.right-brace"_ustr},
        {15, u".uno:SymbolShapes.quad-bevel"_ustr},
        {16, u".uno:SymbolShapes.octagon-bevel"_ustr},
        {17, u".uno:SymbolShapes.diamond-bevel"_ustr}
    };

const std::map<sal_uInt16, OUString> gaBlockArrowShapes {
        {0, u".uno:ArrowShapes.left-arrow"_ustr},
        {1, u".uno:ArrowShapes.right-arrow"_ustr},
        {2, u".uno:ArrowShapes.up-arrow"_ustr},
        {3, u".uno:ArrowShapes.down-arrow"_ustr},
        {4, u".uno:ArrowShapes.left-right-arrow"_ustr},
        {5, u".uno:ArrowShapes.up-down-arrow"_ustr},
        {6, u".uno:ArrowShapes.up-right-arrow"_ustr},
        {7, u".uno:ArrowShapes.up-right-down-arrow"_ustr},
        {8, u".uno:ArrowShapes.quad-arrow"_ustr},
        {9, u".uno:ArrowShapes.corner-right-arrow"_ustr},
        {10, u".uno:ArrowShapes.split-arrow"_ustr},
        {11, u".uno:ArrowShapes.striped-right-arrow"_ustr},
        {12, u".uno:ArrowShapes.notched-right-arrow"_ustr},
        {13, u".uno:ArrowShapes.pentagon-right"_ustr},
        {14, u".uno:ArrowShapes.chevron"_ustr},
        {15, u".uno:ArrowShapes.right-arrow-callout"_ustr},
        {16, u".uno:ArrowShapes.left-arrow-callout"_ustr},
        {17, u".uno:ArrowShapes.up-arrow-callout"_ustr},
        {18, u".uno:ArrowShapes.left-right-arrow-callout"_ustr},
        {19, u".uno:ArrowShapes.up-down-arrow-callout"_ustr},
        {20, u".uno:ArrowShapes.up-right-arrow-callout"_ustr},
        {21, u".uno:ArrowShapes.quad-arrow-callout"_ustr},
        {22, u".uno:ArrowShapes.circular-arrow"_ustr},
        {23, u".uno:ArrowShapes.down-arrow-callout"_ustr},
        {24, u".uno:ArrowShapes.split-round-arrow"_ustr},
        {25, u".uno:ArrowShapes.s-sharped-arrow"_ustr}
    };

const std::map<sal_uInt16, OUString> gaFlowchartShapes {
        {0, u".uno:FlowChartShapes.flowchart-process"_ustr},
        {1, u".uno:FlowChartShapes.flowchart-alternate-process"_ustr},
        {2, u".uno:FlowChartShapes.flowchart-decision"_ustr},
        {3, u".uno:FlowChartShapes.flowchart-data"_ustr},
        {4, u".uno:FlowChartShapes.flowchart-predefined-process"_ustr},
        {5, u".uno:FlowChartShapes.flowchart-internal-storage"_ustr},
        {6, u".uno:FlowChartShapes.flowchart-document"_ustr},
        {7, u".uno:FlowChartShapes.flowchart-multidocument"_ustr},
        {8, u".uno:FlowChartShapes.flowchart-terminator"_ustr},
        {9, u".uno:FlowChartShapes.flowchart-preparation"_ustr},
        {10, u".uno:FlowChartShapes.flowchart-manual-input"_ustr},
        {11, u".uno:FlowChartShapes.flowchart-manual-operation"_ustr},
        {12, u".uno:FlowChartShapes.flowchart-connector"_ustr},
        {13, u".uno:FlowChartShapes.flowchart-off-page-connector"_ustr},
        {14, u".uno:FlowChartShapes.flowchart-card"_ustr},
        {15, u".uno:FlowChartShapes.flowchart-punched-tape"_ustr},
        {16, u".uno:FlowChartShapes.flowchart-summing-junction"_ustr},
        {17, u".uno:FlowChartShapes.flowchart-or"_ustr},
        {18, u".uno:FlowChartShapes.flowchart-collate"_ustr},
        {19, u".uno:FlowChartShapes.flowchart-sort"_ustr},
        {20, u".uno:FlowChartShapes.flowchart-extract"_ustr},
        {21, u".uno:FlowChartShapes.flowchart-merge"_ustr},
        {22, u".uno:FlowChartShapes.flowchart-stored-data"_ustr},
        {23, u".uno:FlowChartShapes.flowchart-delay"_ustr},
        {24, u".uno:FlowChartShapes.flowchart-sequential-access"_ustr},
        {25, u".uno:FlowChartShapes.flowchart-magnetic-disk"_ustr},
        {26, u".uno:FlowChartShapes.flowchart-direct-access-storage"_ustr},
        {27, u".uno:FlowChartShapes.flowchart-display"_ustr}
    };

const std::map<sal_uInt16, OUString> gaCalloutShapes {
        {0, u".uno:CalloutShapes.rectangular-callout"_ustr},
        {1, u".uno:CalloutShapes.round-rectangular-callout"_ustr},
        {2, u".uno:CalloutShapes.round-callout"_ustr},
        {3, u".uno:CalloutShapes.cloud-callout"_ustr},
        {4, u".uno:CalloutShapes.line-callout-1"_ustr},
        {5, u".uno:CalloutShapes.line-callout-2"_ustr},
        {6, u".uno:CalloutShapes.line-callout-3"_ustr}
    };

const std::map<sal_uInt16, OUString> gaStarShapes {
        {0, u".uno:StarShapes.star4"_ustr},
        {1, u".uno:StarShapes.star5"_ustr},
        {2, u".uno:StarShapes.star6"_ustr},
        {3, u".uno:StarShapes.star8"_ustr},
        {4, u".uno:StarShapes.star12"_ustr},
        {5, u".uno:StarShapes.star24"_ustr},
        {6, u".uno:StarShapes.bang"_ustr},
        {7, u".uno:StarShapes.vertical-scroll"_ustr},
        {8, u".uno:StarShapes.horizontal-scroll"_ustr},
        {9, u".uno:StarShapes.signet"_ustr},
        {10, u".uno:StarShapes.doorplate"_ustr},
        {11, u".uno:StarShapes.concave-star6"_ustr}
    };

const std::map<sal_uInt16, OUString> ga3DShapes {
        {0, u".uno:Cube"_ustr},
        {1, u".uno:Sphere"_ustr},
        {2, u".uno:Cylinder"_ustr},
        {3, u".uno:Cone"_ustr},
        {4, u".uno:Cyramid"_ustr},
        {5, u".uno:Torus"_ustr},
        {6, u".uno:Shell3D"_ustr},
        {7, u".uno:HalfSphere"_ustr}
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
