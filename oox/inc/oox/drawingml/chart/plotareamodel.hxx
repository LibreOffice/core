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



#ifndef OOX_DRAWINGML_CHART_PLOTAREAMODEL_HXX
#define OOX_DRAWINGML_CHART_PLOTAREAMODEL_HXX

#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/chart/axismodel.hxx"
#include "oox/drawingml/chart/seriesmodel.hxx"
#include "oox/drawingml/chart/typegroupmodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct View3DModel
{
    OptValue< sal_Int32 > monHeightPercent; /// Height of the 3D view, relative to chart width.
    OptValue< sal_Int32 > monRotationX;     /// Horizontal rotation in degrees.
    OptValue< sal_Int32 > monRotationY;     /// Vertical rotation in degrees.
    sal_Int32           mnDepthPercent;     /// Depth of the 3D view, relative to chart width.
    sal_Int32           mnPerspective;      /// Eye distance to the 3D objects.
    bool                mbRightAngled;      /// True = right-angled axes in 3D view.

    explicit            View3DModel();
                        ~View3DModel();
};

// ============================================================================

struct WallFloorModel
{
    typedef ModelRef< Shape >               ShapeRef;
    typedef ModelRef< PictureOptionsModel > PictureOptionsRef;

    ShapeRef            mxShapeProp;        /// Wall/floor frame formatting.
    PictureOptionsRef   mxPicOptions;       /// Fill bitmap settings.

    explicit            WallFloorModel();
                        ~WallFloorModel();
};

// ============================================================================

struct PlotAreaModel
{
    typedef ModelVector< TypeGroupModel >   TypeGroupVector;
    typedef ModelVector< AxisModel >        AxisVector;
    typedef ModelRef< Shape >               ShapeRef;
    typedef ModelRef< LayoutModel >         LayoutRef;

    TypeGroupVector     maTypeGroups;       /// All chart type groups contained in the chart.
    AxisVector          maAxes;             /// All axes contained in the chart.
    ShapeRef            mxShapeProp;        /// Plot area frame formatting.
    LayoutRef           mxLayout;           /// Layout/position of the plot area.

    explicit            PlotAreaModel();
                        ~PlotAreaModel();
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif
