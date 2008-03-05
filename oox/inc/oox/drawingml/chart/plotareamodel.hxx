/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: plotareamodel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:50:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

