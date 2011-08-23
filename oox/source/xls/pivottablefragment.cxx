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

using ::rtl::OUString;
using ::oox::core::ContextHandlerRef;
using ::oox::core::RecordInfo;

namespace oox {
namespace xls {

// ============================================================================

OoxPivotTableFieldContext::OoxPivotTableFieldContext( OoxWorksheetFragmentBase& rFragment, PivotTableField& rTableField ) :
    OoxWorksheetContextBase( rFragment ),
    mrTableField( rTableField )
{
}

ContextHandlerRef OoxPivotTableFieldContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
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

void OoxPivotTableFieldContext::onStartElement( const AttributeList& rAttribs )
{
    if( isRootElement() )
        mrTableField.importPivotField( rAttribs );
}

ContextHandlerRef OoxPivotTableFieldContext::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_PTFIELD:
            switch( nRecId )
            {
                case OOBIN_ID_PTFITEMS:         return this;
                case OOBIN_ID_AUTOSORTSCOPE:    return this;
            }
        break;
        case OOBIN_ID_PTFITEMS:
            if( nRecId == OOBIN_ID_PTFITEM ) mrTableField.importPTFItem( rStrm );
        break;
        case OOBIN_ID_AUTOSORTSCOPE:
            if( nRecId == OOBIN_ID_PIVOTAREA ) return this;
        break;
        case OOBIN_ID_PIVOTAREA:
            if( nRecId == OOBIN_ID_PTREFERENCES ) return this;
        break;
        case OOBIN_ID_PTREFERENCES:
            if( nRecId == OOBIN_ID_PTREFERENCE ) { mrTableField.importPTReference( rStrm ); return this; }
        break;
        case OOBIN_ID_PTREFERENCE:
            if( nRecId == OOBIN_ID_PTREFERENCEITEM ) mrTableField.importPTReferenceItem( rStrm );
        break;
    }
    return 0;
}

void OoxPivotTableFieldContext::onStartRecord( RecordInputStream& rStrm )
{
    if( isRootElement() )
        mrTableField.importPTField( rStrm );
}

// ============================================================================

OoxPivotTableFilterContext::OoxPivotTableFilterContext( OoxWorksheetFragmentBase& rFragment, PivotTableFilter& rTableFilter ) :
    OoxWorksheetContextBase( rFragment ),
    mrTableFilter( rTableFilter )
{
}

ContextHandlerRef OoxPivotTableFilterContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
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

void OoxPivotTableFilterContext::onStartElement( const AttributeList& rAttribs )
{
    if( isRootElement() )
        mrTableFilter.importFilter( rAttribs );
}

ContextHandlerRef OoxPivotTableFilterContext::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_PTFILTER:
            if( nRecId == OOBIN_ID_AUTOFILTER ) return this;
        break;
        case OOBIN_ID_AUTOFILTER:
            if( nRecId == OOBIN_ID_FILTERCOLUMN ) return this;
        break;
        case OOBIN_ID_FILTERCOLUMN:
            if( nRecId == OOBIN_ID_TOP10FILTER ) mrTableFilter.importTop10Filter( rStrm );
        break;
    }
    return 0;
}

void OoxPivotTableFilterContext::onStartRecord( RecordInputStream& rStrm )
{
    if( isRootElement() )
        mrTableFilter.importPTFilter( rStrm );
}

// ============================================================================

OoxPivotTableFragment::OoxPivotTableFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath ),
    mrPivotTable( rHelper.getPivotTables().createPivotTable() )
{
}

ContextHandlerRef OoxPivotTableFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
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
            if( nElement == XLS_TOKEN( pivotField ) ) return new OoxPivotTableFieldContext( *this, mrPivotTable.createTableField() );
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
            if( nElement == XLS_TOKEN( filter ) ) return new OoxPivotTableFilterContext( *this, mrPivotTable.createTableFilter() );
        break;
    }
    return 0;
}

ContextHandlerRef OoxPivotTableFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == OOBIN_ID_PTDEFINITION ) { mrPivotTable.importPTDefinition( rStrm ); return this; }
        break;

        case OOBIN_ID_PTDEFINITION:
            switch( nRecId )
            {
                case OOBIN_ID_PTLOCATION:   mrPivotTable.importPTLocation( rStrm, getSheetIndex() );    break;
                case OOBIN_ID_PTFIELDS:     return this;
                case OOBIN_ID_PTROWFIELDS:  mrPivotTable.importPTRowFields( rStrm );                    break;
                case OOBIN_ID_PTCOLFIELDS:  mrPivotTable.importPTColFields( rStrm );                    break;
                case OOBIN_ID_PTPAGEFIELDS: return this;
                case OOBIN_ID_PTDATAFIELDS: return this;
                case OOBIN_ID_PTFILTERS:    return this;
            }
        break;

        case OOBIN_ID_PTFIELDS:
            if( nRecId == OOBIN_ID_PTFIELD ) return new OoxPivotTableFieldContext( *this, mrPivotTable.createTableField() );
        break;
        case OOBIN_ID_PTPAGEFIELDS:
            if( nRecId == OOBIN_ID_PTPAGEFIELD ) mrPivotTable.importPTPageField( rStrm );
        break;
        case OOBIN_ID_PTDATAFIELDS:
            if( nRecId == OOBIN_ID_PTDATAFIELD ) mrPivotTable.importPTDataField( rStrm );
        break;
        case OOBIN_ID_PTFILTERS:
            if( nRecId == OOBIN_ID_PTFILTER ) return new OoxPivotTableFilterContext( *this, mrPivotTable.createTableFilter() );
        break;
    }
    return 0;
}

const RecordInfo* OoxPivotTableFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { OOBIN_ID_AUTOFILTER,      OOBIN_ID_AUTOFILTER + 1         },
        { OOBIN_ID_AUTOSORTSCOPE,   OOBIN_ID_AUTOSORTSCOPE + 1      },
        { OOBIN_ID_FILTERCOLUMN,    OOBIN_ID_FILTERCOLUMN + 1       },
        { OOBIN_ID_PIVOTAREA,       OOBIN_ID_PIVOTAREA + 1          },
        { OOBIN_ID_PTCOLFIELDS,     OOBIN_ID_PTCOLFIELDS + 1        },
        { OOBIN_ID_PTDATAFIELD,     OOBIN_ID_PTDATAFIELD + 1        },
        { OOBIN_ID_PTDATAFIELDS,    OOBIN_ID_PTDATAFIELDS + 1       },
        { OOBIN_ID_PTDEFINITION,    OOBIN_ID_PTDEFINITION + 35      },
        { OOBIN_ID_PTFIELD,         OOBIN_ID_PTFIELD + 1            },
        { OOBIN_ID_PTFIELDS,        OOBIN_ID_PTFIELDS + 1           },
        { OOBIN_ID_PTFILTER,        OOBIN_ID_PTFILTER + 1           },
        { OOBIN_ID_PTFILTERS,       OOBIN_ID_PTFILTERS + 1          },
        { OOBIN_ID_PTFITEM,         OOBIN_ID_PTFITEM - 1            },
        { OOBIN_ID_PTFITEMS,        OOBIN_ID_PTFITEMS + 1           },
        { OOBIN_ID_PTLOCATION,      OOBIN_ID_PTLOCATION - 1         },
        { OOBIN_ID_PTPAGEFIELD,     OOBIN_ID_PTPAGEFIELD + 1        },
        { OOBIN_ID_PTPAGEFIELDS,    OOBIN_ID_PTPAGEFIELDS + 1       },
        { OOBIN_ID_PTREFERENCE,     OOBIN_ID_PTREFERENCE + 1        },
        { OOBIN_ID_PTREFERENCEITEM, OOBIN_ID_PTREFERENCEITEM + 1    },
        { OOBIN_ID_PTREFERENCES,    OOBIN_ID_PTREFERENCES + 1       },
        { OOBIN_ID_PTROWFIELDS,     OOBIN_ID_PTROWFIELDS + 1        },
        { -1,                       -1                              }
    };
    return spRecInfos;
}

// ============================================================================
// ============================================================================

BiffPivotTableContext::BiffPivotTableContext( const BiffWorksheetFragmentBase& rFragment, PivotTable& rPivotTable ) :
    BiffWorksheetContextBase( rFragment ),
    mrPivotTable( rPivotTable )
{
}

void BiffPivotTableContext::importRecord()
{
    switch( mrStrm.getRecId() )
    {
        case BIFF_ID_PTDEFINITION:      mrPivotTable.importPTDefinition( mrStrm, getSheetIndex() ); break;
        case BIFF_ID_PTDEFINITION2:     mrPivotTable.importPTDefinition2( mrStrm );                 break;
        case BIFF_ID_PTFIELD:           mrPivotTable.createTableField().importPTField( mrStrm );    break;
        case BIFF_ID_PTROWCOLFIELDS:    mrPivotTable.importPTRowColFields( mrStrm );                break;
        case BIFF_ID_PTPAGEFIELDS:      mrPivotTable.importPTPageFields( mrStrm );                  break;
        case BIFF_ID_PTDATAFIELD:       mrPivotTable.importPTDataField( mrStrm );                   break;
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
