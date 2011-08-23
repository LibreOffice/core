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

#ifndef OOX_DRAWINGML_CHART_CHARTSPACEMODEL_HXX
#define OOX_DRAWINGML_CHART_CHARTSPACEMODEL_HXX

#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/chart/plotareamodel.hxx"
#include "oox/drawingml/chart/titlemodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

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
    ::rtl::OUString     maDrawingPath;      /// Path to drawing fragment with embedded shapes.
    sal_Int32           mnDispBlanksAs;     /// Mode how to display blank values.
    sal_Int32           mnStyle;            /// Index to default formatting.
    bool                mbAutoTitleDel;     /// True = automatic title deleted manually.
    bool                mbPlotVisOnly;      /// True = plot visible cells in a sheet only.
    bool                mbShowLabelsOverMax;/// True = show labels over chart maximum.

    explicit            ChartSpaceModel();
                        ~ChartSpaceModel();
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

