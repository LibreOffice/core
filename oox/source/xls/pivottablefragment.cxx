/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pivottablefragment.cxx,v $
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

#include "oox/xls/pivottablefragment.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/xls/addressconverter.hxx"

#include <vector>

using ::rtl::OUString;
using ::com::sun::star::table::CellRangeAddress;

namespace oox {
namespace xls {

OoxPivotTableFragment::OoxPivotTableFragment(
        const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath ),
    mbValidRange( false )
{
}

ContextWrapper OoxPivotTableFragment::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nElement == XLS_TOKEN( pivotTableDefinition ));
        case XLS_TOKEN( pivotTableDefinition ):
            return  (nElement == XLS_TOKEN( location )) ||
                    (nElement == XLS_TOKEN( pivotFields )) ||
                    (nElement == XLS_TOKEN( rowFields )) ||
                    (nElement == XLS_TOKEN( rowItems )) ||
                    (nElement == XLS_TOKEN( colFields )) ||
                    (nElement == XLS_TOKEN( colItems )) ||
                    (nElement == XLS_TOKEN( pageFields )) ||
                    (nElement == XLS_TOKEN( dataFields )) ||
                    (nElement == XLS_TOKEN( pivotTableStyleInfo ));
        case XLS_TOKEN( pivotFields ):
            return  (nElement == XLS_TOKEN( pivotField ));
        case XLS_TOKEN( pivotField ):
            return  (nElement == XLS_TOKEN( items ));
        case XLS_TOKEN( items ):
            return  (nElement == XLS_TOKEN( item ));
        case XLS_TOKEN( rowFields ):
            return  (nElement == XLS_TOKEN( field ));
        case XLS_TOKEN( colFields ):
            return  (nElement == XLS_TOKEN( field ));
        case XLS_TOKEN( pageFields ):
            return  (nElement == XLS_TOKEN( pageField ));
        case XLS_TOKEN( dataFields ):
            return  (nElement == XLS_TOKEN( dataField ));
        case XLS_TOKEN( colItems ):
            return  (nElement == XLS_TOKEN( i ));
        case XLS_TOKEN( rowItems ):
            return  (nElement == XLS_TOKEN( i ));
    }
    return false;
}

void OoxPivotTableFragment::onStartElement( const AttributeList& rAttribs )
{
    switch ( getCurrentElement() )
    {
        case XLS_TOKEN( pivotTableDefinition ):
            importPivotTableDefinition( rAttribs );
        break;
        case XLS_TOKEN( location ):
            importLocation( rAttribs );
        break;
        case XLS_TOKEN( pivotFields ):
            importPivotFields( rAttribs );
        break;
        case XLS_TOKEN( pivotField ):
            importPivotField( rAttribs );
        break;
    }
}

void OoxPivotTableFragment::finalizeImport()
{
    if( mbValidRange )
        getPivotTables().setPivotTable( maName, maData );
}

void OoxPivotTableFragment::importLocation( const AttributeList& rAttribs )
{
    CellRangeAddress aRange;
    OUString aRangeName = rAttribs.getString( XML_ref, OUString() );
    mbValidRange = getAddressConverter().convertToCellRange(
        aRange, aRangeName, getSheetIndex(), true );

    if ( mbValidRange )
        maData.maRange = aRange;
}

void OoxPivotTableFragment::importPivotTableDefinition( const AttributeList& rAttribs )
{
    if ( !rAttribs.hasAttribute( XML_cacheId ) )
        return;

    maName = rAttribs.getString( XML_name, OUString() );
    maData.mnCacheId = rAttribs.getInteger( XML_cacheId, 0 );

    // name="PivotTable3"
    // cacheId="0"
    // applyNumberFormats="0"
    // applyBorderFormats="0"
    // applyFontFormats="0"
    // applyPatternFormats="0"
    // applyAlignmentFormats="0"
    // applyWidthHeightFormats="1"
    // dataCaption="Values"
    // updatedVersion="3"
    // minRefreshableVersion="3"
    // showCalcMbrs="0"
    // useAutoFormatting="1"
    // itemPrintTitles="1"
    // createdVersion="3"
    // indent="0"
    // outline="1"
    // outlineData="1"
    // multipleFieldFilters="0"
}

void OoxPivotTableFragment::importPivotFields( const AttributeList& rAttribs )
{
    maData.maFields.reserve( rAttribs.getUnsignedInteger( XML_count, 1 ) );
}

void OoxPivotTableFragment::importPivotField( const AttributeList& rAttribs )
{
    maData.maFields.push_back( PivotTableField() );
    PivotTableField& rField = maData.maFields.back();
    rField.mbDataField = rAttribs.getBool( XML_dataField, false );

    // Possible values are: axisCol, axisRow, axisPage, axisValues
    switch ( rAttribs.getToken( XML_axis, XML_TOKEN_INVALID ) )
    {
        case XML_axisCol:
            rField.meAxis = PivotTableField::COLUMN;
        break;
        case XML_axisRow:
            rField.meAxis = PivotTableField::ROW;
        break;
        case XML_axisPage:
            rField.meAxis = PivotTableField::PAGE;
        break;
        case XML_axisValues:
            rField.meAxis = PivotTableField::VALUES;
        break;
        default:
        break;
    }
}

} // namespace xls
} // namespace oox

