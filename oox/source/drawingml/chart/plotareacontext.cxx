/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: plotareacontext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:35:23 $
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

