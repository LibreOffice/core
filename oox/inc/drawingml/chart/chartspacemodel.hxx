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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_CHARTSPACEMODEL_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_CHARTSPACEMODEL_HXX

#include <oox/drawingml/shape.hxx>
#include <drawingml/chart/plotareamodel.hxx>
#include <drawingml/chart/titlemodel.hxx>

namespace oox {
namespace drawingml {
namespace chart {



struct ChartSpaceModel
{
    typedef ModelRef< Shape >           ShapeRef;
    typedef ModelRef< TextBody >        TextBodyRef;
    typedef ModelRef< PlotAreaModel >   PlotAreaRef;
    typedef ModelRef< WallFloorModel >  WallFloorRef;
    typedef ModelRef< View3DModel >     View3DRef;
    typedef ModelRef< TitleModel >      TitleRef;
    typedef ModelRef< LegendModel >     LegendRef;

    ShapeRef            mxShapeProp;        /// Chart frame formatting.
    TextBodyRef         mxTextProp;         /// Global chart text formatting.
    PlotAreaRef         mxPlotArea;         /// Plot area of the chart.
    WallFloorRef        mxFloor;            /// Floor formatting in 3D charts.
    WallFloorRef        mxBackWall;         /// Back wall formatting in 3D charts.
    WallFloorRef        mxSideWall;         /// Side wall formatting in 3D charts.
    View3DRef           mxView3D;           /// 3D settings.
    TitleRef            mxTitle;            /// Chart main title.
    LegendRef           mxLegend;           /// Chart legend.
    OUString            maDrawingPath;      /// Path to drawing fragment with embedded shapes.
    OUString            maSheetPath;        /// Path to embedded charts.
    sal_Int32           mnDispBlanksAs;     /// Mode how to display blank values.
    sal_Int32           mnStyle;            /// Index to default formatting.
    bool                mbAutoTitleDel;     /// True = automatic title deleted manually.
    bool                mbPlotVisOnly;      /// True = plot visible cells in a sheet only.
    bool                mbShowLabelsOverMax;/// True = show labels over chart maximum.
    bool                mbPivotChart;       /// True = pivot chart.

    explicit            ChartSpaceModel(bool bMSO2007Doc);
                        ~ChartSpaceModel();
};



}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
