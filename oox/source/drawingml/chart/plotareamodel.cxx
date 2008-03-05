/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: plotareamodel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:35:34 $
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

#include "oox/drawingml/chart/plotareamodel.hxx"
#include "oox/drawingml/chart/axismodel.hxx"
#include "oox/drawingml/chart/layoutmodel.hxx"
#include "oox/drawingml/chart/typegroupmodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

PlotAreaModel::PlotAreaModel()
{
}

PlotAreaModel::~PlotAreaModel()
{
}

TypeGroupModel& PlotAreaModel::createTypeGroup( sal_Int32 nTypeId )
{
    TypeGroupVector::value_type xTypeGroup( new TypeGroupModel( nTypeId ) );
    maTypeGroups.push_back( xTypeGroup );
    return *xTypeGroup;
}

AxisModel& PlotAreaModel::createAxis( sal_Int32 nTypeId )
{
    AxisVector::value_type xAxis( new AxisModel( nTypeId ) );
    maAxes.push_back( xAxis );
    return *xAxis;
}

LayoutModel& PlotAreaModel::createLayout()
{
    mxLayout.reset( new LayoutModel );
    return *mxLayout;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

