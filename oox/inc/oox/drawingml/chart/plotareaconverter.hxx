/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: plotareaconverter.hxx,v $
 *
 * $Revision: 1.4 $
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

#ifndef OOX_DRAWINGML_CHART_PLOTAREACONVERTER_HXX
#define OOX_DRAWINGML_CHART_PLOTAREACONVERTER_HXX

#include "oox/drawingml/chart/converterbase.hxx"

namespace com { namespace sun { namespace star {
    namespace chart2 { class XDiagram; }
} } }

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct View3DModel;
class TypeGroupConverter;

class View3DConverter : public ConverterBase< View3DModel >
{
public:
    explicit            View3DConverter( const ConverterRoot& rParent, View3DModel& rModel );
    virtual             ~View3DConverter();

    /** Converts the OOXML plot area model to a chart2 diagram. */
    void                convertFromModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& rxDiagram,
                            TypeGroupConverter& rTypeGroup );
};

// ============================================================================

struct WallFloorModel;

class WallFloorConverter : public ConverterBase< WallFloorModel >
{
public:
    explicit            WallFloorConverter( const ConverterRoot& rParent, WallFloorModel& rModel );
    virtual             ~WallFloorConverter();

    /** Converts the OOXML wall/floor model to a chart2 diagram. */
    void                convertFromModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& rxDiagram,
                            ObjectType eObjType );
};

// ============================================================================

struct PlotAreaModel;

class PlotAreaConverter : public ConverterBase< PlotAreaModel >
{
public:
    explicit            PlotAreaConverter( const ConverterRoot& rParent, PlotAreaModel& rModel );
    virtual             ~PlotAreaConverter();

    /** Converts the OOXML plot area model to a chart2 diagram. */
    void                convertFromModel( View3DModel& rView3DModel );
    /** Converts the manual plot area position and size, if set. */
    void                convertPositionFromModel();

    /** Returns the automatic chart title if the chart contains only one series. */
    inline const ::rtl::OUString& getAutomaticTitle() const { return maAutoTitle; }
    /** Returns true, if the chart is three-dimensional. */
    inline bool         is3dChart() const { return mb3dChart; }
    /** Returns true, if chart type supports wall and floor format in 3D mode. */
    inline bool         isWall3dChart() const { return mbWall3dChart; }

private:
    ::rtl::OUString     maAutoTitle;
    bool                mb3dChart;
    bool                mbWall3dChart;
    bool                mbPieChart;
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

