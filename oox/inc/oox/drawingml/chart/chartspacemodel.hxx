/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    bool                mbPivotChart;       /// True = pivot chart.

    explicit            ChartSpaceModel();
                        ~ChartSpaceModel();
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif
