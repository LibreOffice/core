/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "oox/xls/pivottablefragment.hxx"

#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/pivottablebuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::oox::core;

using ::rtl::OUString;

// ============================================================================

PivotTableFieldContext::PivotTableFieldContext( WorksheetFragmentBase& rFragment, PivotTableField& rTableField ) :
    WorksheetContextBase( rFragment ),
    mrTableField( rTableField )
{
}

ContextHandlerRef PivotTableFieldContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( pivotField ):
            switch( nElement )
            {
                case XLS_TOKEN( items ):            return this;
                case XLS_TOKEN( autoSortScope ):    return this;
            }
        break;
        case XLS_TOKEN( items ):
            if( nElement == XLS_TOKEN( item ) ) mrTableField.importItem( rAttribs );
        break;
        case XLS_TOKEN( autoSortScope ):
            if( nElement == XLS_TOKEN( pivotArea ) ) return this;
        break;
        case XLS_TOKEN( pivotArea ):
            if( nElement == XLS_TOKEN( references ) ) return this;
        break;
        case XLS_TOKEN( references ):
            if( nElement == XLS_TOKEN( reference ) ) { mrTableField.importReference( rAttribs ); return this; }
        break;
        case XLS_TOKEN( reference ):
            if( nElement == XLS_TOKEN( x ) ) mrTableField.importReferenceItem( rAttribs );
        break;
    }
    return 0;
}

void PivotTableFieldContext::onStartElement( const AttributeList& rAttribs )
{
    if( isRootElement() )
        mrTableField.importPivotField( rAttribs );
}

ContextHandlerRef PivotTableFieldContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_PTFIELD:
            switch( nRecId )
            {
                case BIFF12_ID_PTFITEMS:        return this;
                case BIFF12_ID_AUTOSORTSCOPE:   return this;
            }
        break;
        case BIFF12_ID_PTFITEMS:
            if( nRecId == BIFF12_ID_PTFITEM ) mrTableField.importPTFItem( rStrm );
        break;
        case BIFF12_ID_AUTOSORTSCOPE:
            if( nRecId == BIFF12_ID_PIVOTAREA ) return this;
        break;
        case BIFF12_ID_PIVOTAREA:
            if( nRecId == BIFF12_ID_PTREFERENCES ) return this;
        break;
        case BIFF12_ID_PTREFERENCES:
            if( nRecId == BIFF12_ID_PTREFERENCE ) { mrTableField.importPTReference( rStrm ); return this; }
        break;
        case BIFF12_ID_PTREFERENCE:
            if( nRecId == BIFF12_ID_PTREFERENCEITEM ) mrTableField.importPTReferenceItem( rStrm );
        break;
    }
    return 0;
}

void PivotTableFieldContext::onStartRecord( SequenceInputStream& rStrm )
{
    if( isRootElement() )
        mrTableField.importPTField( rStrm );
}

// ============================================================================

PivotTableFilterContext::PivotTableFilterContext( WorksheetFragmentBase& rFragment, PivotTableFilter& rTableFilter ) :
    WorksheetContextBase( rFragment ),
    mrTableFilter( rTableFilter )
{
}

ContextHandlerRef PivotTableFilterContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( filter ):
            if( nElement == XLS_TOKEN( autoFilter ) ) return this;
        break;
        case XLS_TOKEN( autoFilter ):
            if( nElement == XLS_TOKEN( filterColumn ) ) return this;
        break;
        case XLS_TOKEN( filterColumn ):
            if( nElement == XLS_TOKEN( top10 ) ) mrTableFilter.importTop10( rAttribs );
        break;
    }
    return 0;
}

void PivotTableFilterContext::onStartElement( const AttributeList& rAttribs )
{
    if( isRootElement() )
        mrTableFilter.importFilter( rAttribs );
}

ContextHandlerRef PivotTableFilterContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_PTFILTER:
            if( nRecId == BIFF12_ID_AUTOFILTER ) return this;
        break;
        case BIFF12_ID_AUTOFILTER:
            if( nRecId == BIFF12_ID_FILTERCOLUMN ) return this;
        break;
        case BIFF12_ID_FILTERCOLUMN:
            if( nRecId == BIFF12_ID_TOP10FILTER ) mrTableFilter.importTop10Filter( rStrm );
        break;
    }
    return 0;
}

void PivotTableFilterContext::onStartRecord( SequenceInputStream& rStrm )
{
    if( isRootElement() )
        mrTableFilter.importPTFilter( rStrm );
}

// ============================================================================

PivotTableFragment::PivotTableFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    WorksheetFragmentBase( rHelper, rFragmentPath ),
    mrPivotTable( getPivotTables().createPivotTable() )
{
}

ContextHandlerRef PivotTableFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( pivotTableDefinition ) ) { mrPivotTable.importPivotTableDefinition( rAttribs ); return this; }
        break;

        case XLS_TOKEN( pivotTableDefinition ):
            switch( nElement )
            {
                case XLS_TOKEN( location ):     mrPivotTable.importLocation( rAttribs, getSheetIndex() );   break;
                case XLS_TOKEN( pivotFields ):  return this;
                case XLS_TOKEN( rowFields ):    return this;
                case XLS_TOKEN( colFields ):    return this;
                case XLS_TOKEN( pageFields ):   return this;
                case XLS_TOKEN( dataFields ):   return this;
                case XLS_TOKEN( filters ):      return this;
            }
        break;

        case XLS_TOKEN( pivotFields ):
            if( nElement == XLS_TOKEN( pivotField ) ) return new PivotTableFieldContext( *this, mrPivotTable.createTableField() );
        break;
        case XLS_TOKEN( rowFields ):
            if( nElement == XLS_TOKEN( field ) ) mrPivotTable.importRowField( rAttribs );
        break;
        case XLS_TOKEN( colFields ):
            if( nElement == XLS_TOKEN( field ) ) mrPivotTable.importColField( rAttribs );
        break;
        case XLS_TOKEN( pageFields ):
            if( nElement == XLS_TOKEN( pageField ) ) mrPivotTable.importPageField( rAttribs );
        break;
        case XLS_TOKEN( dataFields ):
            if( nElement == XLS_TOKEN( dataField ) ) mrPivotTable.importDataField( rAttribs );
        break;
        case XLS_TOKEN( filters ):
            if( nElement == XLS_TOKEN( filter ) ) return new PivotTableFilterContext( *this, mrPivotTable.createTableFilter() );
        break;
    }
    return 0;
}

ContextHandlerRef PivotTableFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_PTDEFINITION ) { mrPivotTable.importPTDefinition( rStrm ); return this; }
        break;

        case BIFF12_ID_PTDEFINITION:
            switch( nRecId )
            {
                case BIFF12_ID_PTLOCATION:      mrPivotTable.importPTLocation( rStrm, getSheetIndex() );    break;
                case BIFF12_ID_PTFIELDS:        return this;
                case BIFF12_ID_PTROWFIELDS:     mrPivotTable.importPTRowFields( rStrm );                    break;
                case BIFF12_ID_PTCOLFIELDS:     mrPivotTable.importPTColFields( rStrm );                    break;
                case BIFF12_ID_PTPAGEFIELDS:    return this;
                case BIFF12_ID_PTDATAFIELDS:    return this;
                case BIFF12_ID_PTFILTERS:       return this;
            }
        break;

        case BIFF12_ID_PTFIELDS:
            if( nRecId == BIFF12_ID_PTFIELD ) return new PivotTableFieldContext( *this, mrPivotTable.createTableField() );
        break;
        case BIFF12_ID_PTPAGEFIELDS:
            if( nRecId == BIFF12_ID_PTPAGEFIELD ) mrPivotTable.importPTPageField( rStrm );
        break;
        case BIFF12_ID_PTDATAFIELDS:
            if( nRecId == BIFF12_ID_PTDATAFIELD ) mrPivotTable.importPTDataField( rStrm );
        break;
        case BIFF12_ID_PTFILTERS:
            if( nRecId == BIFF12_ID_PTFILTER ) return new PivotTableFilterContext( *this, mrPivotTable.createTableFilter() );
        break;
    }
    return 0;
}

const RecordInfo* PivotTableFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_AUTOFILTER,         BIFF12_ID_AUTOFILTER + 1        },
        { BIFF12_ID_AUTOSORTSCOPE,      BIFF12_ID_AUTOSORTSCOPE + 1     },
        { BIFF12_ID_FILTERCOLUMN,       BIFF12_ID_FILTERCOLUMN + 1      },
        { BIFF12_ID_PIVOTAREA,          BIFF12_ID_PIVOTAREA + 1         },
        { BIFF12_ID_PTCOLFIELDS,        BIFF12_ID_PTCOLFIELDS + 1       },
        { BIFF12_ID_PTDATAFIELD,        BIFF12_ID_PTDATAFIELD + 1       },
        { BIFF12_ID_PTDATAFIELDS,       BIFF12_ID_PTDATAFIELDS + 1      },
        { BIFF12_ID_PTDEFINITION,       BIFF12_ID_PTDEFINITION + 35     },
        { BIFF12_ID_PTFIELD,            BIFF12_ID_PTFIELD + 1           },
        { BIFF12_ID_PTFIELDS,           BIFF12_ID_PTFIELDS + 1          },
        { BIFF12_ID_PTFILTER,           BIFF12_ID_PTFILTER + 1          },
        { BIFF12_ID_PTFILTERS,          BIFF12_ID_PTFILTERS + 1         },
        { BIFF12_ID_PTFITEM,            BIFF12_ID_PTFITEM - 1           },
        { BIFF12_ID_PTFITEMS,           BIFF12_ID_PTFITEMS + 1          },
        { BIFF12_ID_PTLOCATION,         BIFF12_ID_PTLOCATION - 1        },
        { BIFF12_ID_PTPAGEFIELD,        BIFF12_ID_PTPAGEFIELD + 1       },
        { BIFF12_ID_PTPAGEFIELDS,       BIFF12_ID_PTPAGEFIELDS + 1      },
        { BIFF12_ID_PTREFERENCE,        BIFF12_ID_PTREFERENCE + 1       },
        { BIFF12_ID_PTREFERENCEITEM,    BIFF12_ID_PTREFERENCEITEM + 1   },
        { BIFF12_ID_PTREFERENCES,       BIFF12_ID_PTREFERENCES + 1      },
        { BIFF12_ID_PTROWFIELDS,        BIFF12_ID_PTROWFIELDS + 1       },
        { -1,                           -1                              }
    };
    return spRecInfos;
}

// ============================================================================
// ============================================================================

BiffPivotTableContext::BiffPivotTableContext( const WorksheetHelper& rHelper ) :
    BiffWorksheetContextBase( rHelper ),
    mrPivotTable( getPivotTables().createPivotTable() )
{
}

void BiffPivotTableContext::importRecord( BiffInputStream& rStrm )
{
    switch( rStrm.getRecId() )
    {
        case BIFF_ID_PTDEFINITION:      mrPivotTable.importPTDefinition( rStrm, getSheetIndex() );  break;
        case BIFF_ID_PTDEFINITION2:     mrPivotTable.importPTDefinition2( rStrm );                  break;
        case BIFF_ID_PTFIELD:           mrPivotTable.createTableField().importPTField( rStrm );     break;
        case BIFF_ID_PTROWCOLFIELDS:    mrPivotTable.importPTRowColFields( rStrm );                 break;
        case BIFF_ID_PTPAGEFIELDS:      mrPivotTable.importPTPageFields( rStrm );                   break;
        case BIFF_ID_PTDATAFIELD:       mrPivotTable.importPTDataField( rStrm );                    break;
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */