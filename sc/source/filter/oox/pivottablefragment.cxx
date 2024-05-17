/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <pivottablefragment.hxx>
#include <pivottablebuffer.hxx>
#include <PivotTableFormatContext.hxx>
#include <biffhelper.hxx>
#include <oox/token/namespaces.hxx>

namespace oox::xls {

using namespace ::oox::core;

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
    return nullptr;
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
    return nullptr;
}

void PivotTableFieldContext::onStartRecord( SequenceInputStream& rStrm )
{
    if( isRootElement() )
        mrTableField.importPTField( rStrm );
}

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
    return nullptr;
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
    return nullptr;
}

void PivotTableFilterContext::onStartRecord( SequenceInputStream& rStrm )
{
    if( isRootElement() )
        mrTableFilter.importPTFilter( rStrm );
}

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
                case XLS_TOKEN(pivotFields):
                case XLS_TOKEN(rowFields):
                case XLS_TOKEN(colFields):
                case XLS_TOKEN(pageFields):
                case XLS_TOKEN(dataFields):
                case XLS_TOKEN(filters):
                case XLS_TOKEN(formats):
                    return this;
                case XLS_TOKEN(pivotTableStyleInfo):
                    mrPivotTable.putToInteropGrabBag(u"pivotTableStyleInfo"_ustr, rAttribs);
                    break;
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
        case XLS_TOKEN(formats):
            if (nElement == XLS_TOKEN(format))
                return new PivotTableFormatContext(*this, mrPivotTable.createFormat());
        break;
    }
    return nullptr;
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
    return nullptr;
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

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
