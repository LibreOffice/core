/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typegroupmodel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:37:14 $
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

#include "oox/drawingml/chart/typegroupmodel.hxx"
#include "oox/drawingml/chart/seriesmodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

TypeGroupData::TypeGroupData( sal_Int32 nTypeId ) :
    mfSplitPos( 0.0 ),
    mnBarDir( XML_col ),
    mnBubbleScale( 100 ),
    mnFirstAngle( 0 ),
    mnGapDepth( 150 ),
    mnGapWidth( 150 ),
    mnGrouping( XML_standard ),
    mnHoleSize( 10 ),
    mnOfPieType( XML_pie ),
    mnOverlap( 0 ),
    mnRadarStyle( XML_standard ),
    mnScatterStyle( XML_marker ),
    mnSecondPieSize( 75 ),
    mnShape( XML_box ),
    mnSizeRepresents( XML_area ),
    mnSplitType( XML_auto ),
    mnTypeId( nTypeId ),
    mbBubble3d( false ),
    mbShowMarker( false ),
    mbShowNegBubbles( false ),
    mbSmooth( false ),
    mbVaryColors( false ),
    mbWireframe( false )
{
}

// ----------------------------------------------------------------------------

TypeGroupModel::TypeGroupModel( sal_Int32 nTypeId ) :
    ModelData< TypeGroupData >( nTypeId )
{
}

TypeGroupModel::~TypeGroupModel()
{
}

SeriesModel& TypeGroupModel::createSeries()
{
    SeriesVector::value_type xSeries( new SeriesModel );
    maSeries.push_back( xSeries );
    return *xSeries;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

