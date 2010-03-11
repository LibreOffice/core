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

#ifndef OOX_DRAWINGML_CHART_PLOTAREAMODEL_HXX
#define OOX_DRAWINGML_CHART_PLOTAREAMODEL_HXX

#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/chart/axismodel.hxx"
#include "oox/drawingml/chart/seriesmodel.hxx"
#include "oox/drawingml/chart/titlemodel.hxx"
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

