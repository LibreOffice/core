/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

AxisDispUnitsModel::AxisDispUnitsModel() :
    mfCustomUnit( 0.0 ),
    mnBuiltInUnit( XML_TOKEN_INVALID )
{
}

AxisDispUnitsModel::~AxisDispUnitsModel()
{
}

// ============================================================================

AxisModel::AxisModel( sal_Int32 nTypeId ) :
    mnAxisId( -1 ),
    mnAxisPos( XML_TOKEN_INVALID ),
    mnBaseTimeUnit( XML_days ),
    mnCrossAxisId( -1 ),
    mnCrossBetween( XML_between ),
    mnCrossMode( XML_autoZero ),
    mnLabelAlign( XML_ctr ),
    mnLabelOffset( 100 ),
    mnMajorTickMark( XML_out ),
    mnMajorTimeUnit( XML_days ),
    mnMinorTickMark( XML_none ),
    mnMinorTimeUnit( XML_days ),
    mnOrientation( XML_minMax ),
    mnTickLabelPos( XML_nextTo ),
    mnTickLabelSkip( 0 ),
    mnTickMarkSkip( 0 ),
    mnTypeId( nTypeId ),
    mbAuto( false ),
    mbDeleted( false ),
    mbNoMultiLevel( false )
{
}

AxisModel::~AxisModel()
{
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

