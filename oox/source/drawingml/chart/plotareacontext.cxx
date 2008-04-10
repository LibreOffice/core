/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: plotareacontext.cxx,v $
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

#include "oox/drawingml/chart/plotareacontext.hxx"
#include "oox/drawingml/chart/axiscontext.hxx"
#include "oox/drawingml/chart/layoutcontext.hxx"
#include "oox/drawingml/chart/plotareamodel.hxx"
#include "oox/drawingml/chart/typegroupcontext.hxx"

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextWrapper;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

PlotAreaContext::PlotAreaContext( ContextHandler2Helper& rParent, PlotAreaModel& rModel ) :
    ChartContextBase< PlotAreaModel >( rParent, rModel )
{
}

PlotAreaContext::~PlotAreaContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper PlotAreaContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( plotArea ):
            switch( nElement )
            {
                case C_TOKEN( area3DChart ):
                case C_TOKEN( areaChart ):
                    return new AreaTypeGroupContext( *this, getModel().createTypeGroup( nElement ) );
                case C_TOKEN( bar3DChart ):
                case C_TOKEN( barChart ):
                    return new BarTypeGroupContext( *this, getModel().createTypeGroup( nElement ) );
                case C_TOKEN( bubbleChart ):
                    return new BubbleTypeGroupContext( *this, getModel().createTypeGroup( nElement ) );
                case C_TOKEN( line3DChart ):
                case C_TOKEN( lineChart ):
                case C_TOKEN( stockChart ):
                    return new LineTypeGroupContext( *this, getModel().createTypeGroup( nElement ) );
                case C_TOKEN( doughnutChart ):
                case C_TOKEN( ofPieChart ):
                case C_TOKEN( pie3DChart ):
                case C_TOKEN( pieChart ):
                    return new PieTypeGroupContext( *this, getModel().createTypeGroup( nElement ) );
                case C_TOKEN( radarChart ):
                    return new RadarTypeGroupContext( *this, getModel().createTypeGroup( nElement ) );
                case C_TOKEN( scatterChart ):
                    return new ScatterTypeGroupContext( *this, getModel().createTypeGroup( nElement ) );
                case C_TOKEN( surface3DChart ):
                case C_TOKEN( surfaceChart ):
                    return new SurfaceTypeGroupContext( *this, getModel().createTypeGroup( nElement ) );

                case C_TOKEN( catAx ):
                    return new CatAxisContext( *this, getModel().createAxis( nElement ) );
                case C_TOKEN( dateAx ):
                    return new DateAxisContext( *this, getModel().createAxis( nElement ) );
                case C_TOKEN( serAx ):
                    return new SerAxisContext( *this, getModel().createAxis( nElement ) );
                case C_TOKEN( valAx ):
                    return new ValAxisContext( *this, getModel().createAxis( nElement ) );

                case C_TOKEN( layout ):
                    return new LayoutContext( *this, getModel().createLayout() );
            }
        break;
    }
    return false;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

