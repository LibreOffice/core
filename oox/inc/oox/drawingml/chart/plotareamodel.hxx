/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: plotareamodel.hxx,v $
 * $Revision: 1.3 $
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

#include "oox/helper/containerhelper.hxx"
#include "oox/drawingml/chart/modelbase.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

class TypeGroupModel;
class AxisModel;
class LayoutModel;

class PlotAreaModel
{
public:
    explicit            PlotAreaModel();
    virtual             ~PlotAreaModel();

    /** Creates and returns a new chart type model object. */
    TypeGroupModel&     createTypeGroup( sal_Int32 nTypeId );

    /** Creates and returns a new axis model object. */
    AxisModel&          createAxis( sal_Int32 nTypeId );

    /** Returns true, if this chart model contains a layout object. */
    inline bool         hasLayout() const { return mxLayout.get() != 0; }
    /** Returns a pointer to an existing layout object, or null if not present. */
    inline LayoutModel* getLayout() const { return mxLayout.get(); }
    /** Creates and returns a new layout model object. */
    LayoutModel&        createLayout();

private:
    typedef RefVector< TypeGroupModel >         TypeGroupVector;
    typedef RefVector< AxisModel >              AxisVector;
    typedef ::boost::shared_ptr< LayoutModel >  LayoutRef;

    TypeGroupVector     maTypeGroups;       /// List of all chart types.
    AxisVector          maAxes;             /// All axes used in the chart.
    LayoutRef           mxLayout;           /// Plot area position.
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

