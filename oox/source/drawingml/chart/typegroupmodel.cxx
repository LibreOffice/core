/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: typegroupmodel.cxx,v $
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

