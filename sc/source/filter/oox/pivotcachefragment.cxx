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

#include <pivotcachefragment.hxx>

#include <osl/diagnose.h>
#include <oox/token/namespaces.hxx>
#include <biffhelper.hxx>
#include <formulabuffer.hxx>
#include <pivotcachebuffer.hxx>
#include <worksheetbuffer.hxx>

namespace oox::xls {

using namespace ::com::sun::star::uno;
using namespace ::oox::core;

PivotCacheFieldContext::PivotCacheFieldContext( WorkbookFragmentBase& rFragment, PivotCacheField& rCacheField ) :
    WorkbookContextBase( rFragment ),
    mrCacheField( rCacheField )
{
}

ContextHandlerRef PivotCacheFieldContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( cacheField ):
            if( nElement == XLS_TOKEN( sharedItems ) )  { mrCacheField.importSharedItems( rAttribs );   return this; }
            if( nElement == XLS_TOKEN( fieldGroup ) )   { mrCacheField.importFieldGroup( rAttribs );    return this; }
        break;

        case XLS_TOKEN( fieldGroup ):
            switch( nElement )
            {
                case XLS_TOKEN( rangePr ):      mrCacheField.importRangePr( rAttribs );     break;
                case XLS_TOKEN( discretePr ):   return this;
                case XLS_TOKEN( groupItems ):   return this;
            }
        break;

        case XLS_TOKEN( sharedItems ):  mrCacheField.importSharedItem( nElement, rAttribs );        break;
        case XLS_TOKEN( discretePr ):   mrCacheField.importDiscretePrItem( nElement, rAttribs );    break;
        case XLS_TOKEN( groupItems ):   mrCacheField.importGroupItem( nElement, rAttribs );         break;
    }
    return nullptr;
}

void PivotCacheFieldContext::onStartElement( const AttributeList& rAttribs )
{
    if( isRootElement() )
        mrCacheField.importCacheField( rAttribs );
}

ContextHandlerRef PivotCacheFieldContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_PCDFIELD:
            switch( nRecId )
            {
                case BIFF12_ID_PCDFSHAREDITEMS: mrCacheField.importPCDFSharedItems( rStrm );  return this;
                case BIFF12_ID_PCDFIELDGROUP:   mrCacheField.importPCDFieldGroup( rStrm );    return this;
            }
        break;

        case BIFF12_ID_PCDFIELDGROUP:
            switch( nRecId )
            {
                case BIFF12_ID_PCDFRANGEPR:     mrCacheField.importPCDFRangePr( rStrm );    break;
                case BIFF12_ID_PCDFDISCRETEPR:  return this;
                case BIFF12_ID_PCDFGROUPITEMS:  return this;
            }
        break;

        case BIFF12_ID_PCDFSHAREDITEMS: mrCacheField.importPCDFSharedItem( nRecId, rStrm );     break;
        case BIFF12_ID_PCDFDISCRETEPR:  mrCacheField.importPCDFDiscretePrItem( nRecId, rStrm ); break;
        case BIFF12_ID_PCDFGROUPITEMS:  mrCacheField.importPCDFGroupItem( nRecId, rStrm );      break;
    }
    return nullptr;
}

void PivotCacheFieldContext::onStartRecord( SequenceInputStream& rStrm )
{
    if( isRootElement() )
        mrCacheField.importPCDField( rStrm );
}

PivotCacheDefinitionFragment::PivotCacheDefinitionFragment(
        const WorkbookHelper& rHelper, const OUString& rFragmentPath, PivotCache& rPivotCache ) :
    WorkbookFragmentBase( rHelper, rFragmentPath ),
    mrPivotCache( rPivotCache )
{
}

ContextHandlerRef PivotCacheDefinitionFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( pivotCacheDefinition ) ) { mrPivotCache.importPivotCacheDefinition( rAttribs ); return this; }
        break;

        case XLS_TOKEN( pivotCacheDefinition ):
            switch( nElement )
            {
                case XLS_TOKEN( cacheSource ):  mrPivotCache.importCacheSource( rAttribs ); return this;
                case XLS_TOKEN( cacheFields ):  return this;
            }
        break;

        case XLS_TOKEN( cacheSource ):
            if( nElement == XLS_TOKEN( worksheetSource ) ) mrPivotCache.importWorksheetSource( rAttribs, getRelations() );
        break;

        case XLS_TOKEN( cacheFields ):
            if( nElement == XLS_TOKEN( cacheField ) ) return new PivotCacheFieldContext( *this, mrPivotCache.createCacheField() );
        break;
    }
    return nullptr;
}

ContextHandlerRef PivotCacheDefinitionFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_PCDEFINITION ) { mrPivotCache.importPCDefinition( rStrm ); return this; }
        break;

        case BIFF12_ID_PCDEFINITION:
            switch( nRecId )
            {
                case BIFF12_ID_PCDSOURCE: mrPivotCache.importPCDSource( rStrm ); return this;
                case BIFF12_ID_PCDFIELDS: return this;
            }
        break;

        case BIFF12_ID_PCDSOURCE:
            if( nRecId == BIFF12_ID_PCDSHEETSOURCE ) mrPivotCache.importPCDSheetSource( rStrm, getRelations() );
        break;

        case BIFF12_ID_PCDFIELDS:
            if( nRecId == BIFF12_ID_PCDFIELD ) return new PivotCacheFieldContext( *this, mrPivotCache.createCacheField() );
        break;
    }
    return nullptr;
}

const RecordInfo* PivotCacheDefinitionFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_PCDEFINITION,       BIFF12_ID_PCDEFINITION + 1      },
        { BIFF12_ID_PCDFDISCRETEPR,     BIFF12_ID_PCDFDISCRETEPR + 1    },
        { BIFF12_ID_PCDFGROUPITEMS,     BIFF12_ID_PCDFGROUPITEMS + 1    },
        { BIFF12_ID_PCDFIELD,           BIFF12_ID_PCDFIELD + 1          },
        { BIFF12_ID_PCDFIELDGROUP,      BIFF12_ID_PCDFIELDGROUP + 1     },
        { BIFF12_ID_PCDFIELDS,          BIFF12_ID_PCDFIELDS + 1         },
        { BIFF12_ID_PCDFRANGEPR,        BIFF12_ID_PCDFRANGEPR + 1       },
        { BIFF12_ID_PCDFSHAREDITEMS,    BIFF12_ID_PCDFSHAREDITEMS + 1   },
        { BIFF12_ID_PCITEM_ARRAY,       BIFF12_ID_PCITEM_ARRAY + 1      },
        { BIFF12_ID_PCDSHEETSOURCE,     BIFF12_ID_PCDSHEETSOURCE + 1    },
        { BIFF12_ID_PCDSOURCE,          BIFF12_ID_PCDSOURCE + 1         },
        { -1,                           -1                              }
    };
    return spRecInfos;
}

void PivotCacheDefinitionFragment::finalizeImport()
{
    // finalize the cache (check source range etc.)
    mrPivotCache.finalizeImport();

    // load the cache records, if the cache is based on a deleted or an external worksheet
    if( mrPivotCache.isValidDataSource() && mrPivotCache.isBasedOnDummySheet() )
    {
        OUString aRecFragmentPath = getRelations().getFragmentPathFromRelId( mrPivotCache.getRecordsRelId() );
        if( !aRecFragmentPath.isEmpty() )
        {
            SCTAB nSheet = mrPivotCache.getSourceRange().aStart.Tab();
            WorksheetGlobalsRef xSheetGlob = WorksheetHelper::constructGlobals( *this, ISegmentProgressBarRef(), WorksheetType::Work, nSheet );
            if( xSheetGlob )
                importOoxFragment( new PivotCacheRecordsFragment( *xSheetGlob, aRecFragmentPath, mrPivotCache ) );
        }
    }
}

PivotCacheRecordsFragment::PivotCacheRecordsFragment( const WorksheetHelper& rHelper,
        const OUString& rFragmentPath, const PivotCache& rPivotCache ) :
    WorksheetFragmentBase( rHelper, rFragmentPath ),
    mrPivotCache( rPivotCache ),
    mnColIdx( 0 ),
    mnRowIdx( 0 ),
    mbInRecord( false )
{
    sal_Int32 nSheetCount = rPivotCache.getWorksheets().getAllSheetCount();

    // prepare sheet: insert column header names into top row
    rPivotCache.writeSourceHeaderCells( *this );
    // resize formula buffers since we've added a new dummy sheet
    rHelper.getFormulaBuffer().SetSheetCount( nSheetCount );
}

ContextHandlerRef PivotCacheRecordsFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( pivotCacheRecords ) ) return this;
        break;

        case XLS_TOKEN( pivotCacheRecords ):
            if( nElement == XLS_TOKEN( r ) ) { startCacheRecord(); return this; }
        break;

        case XLS_TOKEN( r ):
        {
            PivotCacheItem aItem;
            switch( nElement )
            {
                case XLS_TOKEN( m ):                                                        break;
                case XLS_TOKEN( s ):    aItem.readString( rAttribs );                       break;
                case XLS_TOKEN( n ):    aItem.readNumeric( rAttribs );                      break;
                case XLS_TOKEN( d ):    aItem.readDate( rAttribs );                         break;
                case XLS_TOKEN( b ):    aItem.readBool( rAttribs );                         break;
                case XLS_TOKEN( e ):    aItem.readError( rAttribs );                        break;
                case XLS_TOKEN( x ):    aItem.readIndex( rAttribs );                        break;
                default:    OSL_FAIL( "OoxPivotCacheRecordsFragment::onCreateContext - unexpected element" );
            }
            mrPivotCache.writeSourceDataCell( *this, mnColIdx, mnRowIdx, aItem );
            ++mnColIdx;
        }
        break;
    }
    return nullptr;
}

ContextHandlerRef PivotCacheRecordsFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_PCRECORDS ) return this;
        break;

        case BIFF12_ID_PCRECORDS:
            switch( nRecId )
            {
                case BIFF12_ID_PCRECORD:    importPCRecord( rStrm );                break;
                case BIFF12_ID_PCRECORDDT:  startCacheRecord();                     break;
                default:                    importPCRecordItem( nRecId, rStrm );    break;
            }
        break;
    }
    return nullptr;
}

const RecordInfo* PivotCacheRecordsFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_PCRECORDS,  BIFF12_ID_PCRECORDS + 1 },
        { -1,                   -1                      }
    };
    return spRecInfos;
}

// private --------------------------------------------------------------------

void PivotCacheRecordsFragment::startCacheRecord()
{
    mnColIdx = 0;
    ++mnRowIdx;
    mbInRecord = true;
}

void PivotCacheRecordsFragment::importPCRecord( SequenceInputStream& rStrm )
{
    startCacheRecord();
    mrPivotCache.importPCRecord( rStrm, *this, mnRowIdx );
    mbInRecord = false;
}

void PivotCacheRecordsFragment::importPCRecordItem( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    if( !mbInRecord )
        return;

    PivotCacheItem aItem;
    switch( nRecId )
    {
        case BIFF12_ID_PCITEM_MISSING:                              break;
        case BIFF12_ID_PCITEM_STRING:   aItem.readString( rStrm );  break;
        case BIFF12_ID_PCITEM_DOUBLE:   aItem.readDouble( rStrm );  break;
        case BIFF12_ID_PCITEM_DATE:     aItem.readDate( rStrm );    break;
        case BIFF12_ID_PCITEM_BOOL:     aItem.readBool( rStrm );    break;
        case BIFF12_ID_PCITEM_ERROR:    aItem.readError( rStrm );   break;
        case BIFF12_ID_PCITEM_INDEX:    aItem.readIndex( rStrm );   break;
        default:    OSL_FAIL( "OoxPivotCacheRecordsFragment::importPCRecordItem - unexpected record" );
    }
    mrPivotCache.writeSourceDataCell( *this, mnColIdx, mnRowIdx, aItem );
    ++mnColIdx;
}


} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
