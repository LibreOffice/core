/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: axismodel.cxx,v $
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

#include "oox/drawingml/chart/axismodel.hxx"
#include "oox/drawingml/chart/layoutmodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

AxisData::AxisData( sal_Int32 nTypeId ) :
    mfCrossesAt( 0.0 ),
    mfCustomUnit( 0.0 ),
    mfMajorUnit( 0.0 ),
    mfMinorUnit( 0.0 ),
    mfLogBase( 0.0 ),
    mfMax( 0.0 ),
    mfMin( 0.0 ),
    mnAxisId( -1 ),
    mnAxisPos( XML_TOKEN_INVALID ),
    mnBaseTimeUnit( XML_days ),
    mnBuiltInUnit( XML_TOKEN_INVALID ),
    mnCrossAxisId( -1 ),
    mnCrossBetween( XML_between ),
    mnCrossMode( XML_autoZero ),
    mnLabelAlign( XML_ctr ),
    mnLabelOffset( 100 ),
    mnMajorTickMark( XML_cross ),
    mnMajorTimeUnit( XML_days ),
    mnMinorTickMark( XML_cross ),
    mnMinorTimeUnit( XML_days ),
    mnOrientation( XML_minMax ),
    mnTickLabelPos( XML_nextTo ),
    mnTickLabelSkip( 0 ),
    mnTickMarkSkip( 0 ),
    mnTypeId( nTypeId ),
    mbAuto( false ),
    mbDeleted( false ),
    mbNoMultiLevel( false ),
    mbSourceLinked( false )
{
}

// ----------------------------------------------------------------------------

AxisModel::AxisModel( sal_Int32 nTypeId ) :
    ModelData< AxisData >( nTypeId )
{
}

AxisModel::~AxisModel()
{
}

LayoutModel& AxisModel::createUnitLabelsLayout()
{
    mxLayout.reset( new LayoutModel );
    return *mxLayout;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

