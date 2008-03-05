/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: axismodel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:33:07 $
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

