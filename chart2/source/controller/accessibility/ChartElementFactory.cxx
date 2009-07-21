/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChartElementFactory.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "ChartElementFactory.hxx"
#include "ObjectIdentifier.hxx"
#include "AccessibleChartElement.hxx"

namespace chart
{

AccessibleBase* ChartElementFactory::CreateChartElement( const AccessibleElementInfo& rAccInfo )
{
    ObjectIdentifier aOID( rAccInfo.m_aOID );
    ObjectType eType( aOID.getObjectType() );

    switch( eType )
    {
        case OBJECTTYPE_DATA_POINT:
        case OBJECTTYPE_LEGEND_ENTRY:
            return new AccessibleChartElement( rAccInfo, false, false );
        case OBJECTTYPE_PAGE:
        case OBJECTTYPE_TITLE:
        case OBJECTTYPE_LEGEND:
        case OBJECTTYPE_DIAGRAM:
        case OBJECTTYPE_DIAGRAM_WALL:
        case OBJECTTYPE_DIAGRAM_FLOOR:
        case OBJECTTYPE_AXIS:
        case OBJECTTYPE_AXIS_UNITLABEL:
        case OBJECTTYPE_GRID:
        case OBJECTTYPE_SUBGRID:
        case OBJECTTYPE_DATA_SERIES:
        case OBJECTTYPE_DATA_LABELS:
        case OBJECTTYPE_DATA_LABEL:
        case OBJECTTYPE_DATA_ERRORS:
        case OBJECTTYPE_DATA_ERRORS_X:
        case OBJECTTYPE_DATA_ERRORS_Y:
        case OBJECTTYPE_DATA_ERRORS_Z:
        case OBJECTTYPE_DATA_CURVE: // e.g. a statistical method printed as line
        case OBJECTTYPE_DATA_AVERAGE_LINE:
        case OBJECTTYPE_DATA_STOCK_RANGE:
        case OBJECTTYPE_DATA_STOCK_LOSS:
        case OBJECTTYPE_DATA_STOCK_GAIN:
        case OBJECTTYPE_DATA_CURVE_EQUATION:
            return new AccessibleChartElement( rAccInfo, true, false );
        case OBJECTTYPE_UNKNOWN:
            break;
        default:
            break;
    }

    return 0;

    /*
    sal_uInt16 nObjId = rId.GetObjectId();
    switch( nObjId )
    {
        case CHOBJID_LEGEND:
            return new AccLegend( pParent );
        case AccLegendEntry::ObjectId:
            return new AccLegendEntry( pParent, rId.GetIndex1() );

        case CHOBJID_TITLE_MAIN:
            return new AccTitle( pParent, Title::MAIN );
        case CHOBJID_TITLE_SUB:
            return new AccTitle( pParent, Title::SUB );
        case CHOBJID_DIAGRAM_TITLE_X_AXIS:
            return new AccTitle( pParent, Title::X_AXIS );
        case CHOBJID_DIAGRAM_TITLE_Y_AXIS:
            return new AccTitle( pParent, Title::Y_AXIS );
        case CHOBJID_DIAGRAM_TITLE_Z_AXIS:
            return new AccTitle( pParent, Title::Z_AXIS );

        case CHOBJID_DIAGRAM:
            return new AccDiagram( pParent );

        // series
        case CHOBJID_DIAGRAM_ROWGROUP:
            return new AccDataSeries( pParent, rId.GetIndex1() );

        // data points
        case CHOBJID_DIAGRAM_DATA:
            return new AccDataPoint( pParent, rId.GetIndex1(), rId.GetIndex2() );

        case Axis::X_AXIS:
        case Axis::Y_AXIS:
        case Axis::Z_AXIS:
        case Axis::SEC_X_AXIS:
        case Axis::SEC_Y_AXIS:
            return new AccAxis( pParent, static_cast< Axis::AxisType >( nObjId ) );

        case Grid::X_MAJOR:
        case Grid::Y_MAJOR:
        case Grid::Z_MAJOR:
        case Grid::X_MINOR:
        case Grid::Y_MINOR:
        case Grid::Z_MINOR:
            return new AccGrid( pParent, static_cast< AccGrid::GridType >( nObjId ) );

        case AccStatisticsObject::MEAN_VAL_LINE:
        case AccStatisticsObject::ERROR_BARS:
        case AccStatisticsObject::REGRESSION:
            return new AccStatisticsObject( pParent,
                                         static_cast< AccStatisticsObject::StatisticsObjectType >( nObjId ),
                                         rId.GetIndex1() );

        case CHOBJID_DIAGRAM_WALL:
            return new AccWall( pParent );

        case CHOBJID_DIAGRAM_FLOOR:
            return new AccFloor( pParent );

        case CHOBJID_DIAGRAM_AREA:
            return new AccArea( pParent );
    }
    */
}

} // namespace chart
