/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: plotareamodel.cxx,v $
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

