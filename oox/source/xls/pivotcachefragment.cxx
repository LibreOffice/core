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

#include "oox/xls/pivotcachefragment.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/pivotcachebuffer.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::oox::core::ContextHandlerRef;
using ::oox::core::RecordInfo;

namespace oox {
namespace xls {

// ============================================================================

OoxPivotCacheFieldContext::OoxPivotCacheFieldContext( OoxWorkbookFragmentBase& rFragment, PivotCacheField& rCacheField ) :
    OoxWorkbookContextBase( rFragment ),
    mrCacheField( rCacheField )
{
}

ContextHandlerRef OoxPivotCacheFieldContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
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
    return 0;
}

void OoxPivotCacheFieldContext::onStartElement( const AttributeList& rAttribs )
{
    if( isRootElement() )
        mrCacheField.importCacheField( rAttribs );
}

ContextHandlerRef OoxPivotCacheFieldContext::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_PCDFIELD:
            switch( nRecId )
            {
                case OOBIN_ID_PCDFSHAREDITEMS:  mrCacheField.importPCDFSharedItems( rStrm );  return this;
                case OOBIN_ID_PCDFIELDGROUP:    mrCacheField.importPCDFieldGroup( rStrm );    return this;
            }
        break;

        case OOBIN_ID_PCDFIELDGROUP:
            switch( nRecId )
            {
                case OOBIN_ID_PCDFRANGEPR:      mrCacheField.importPCDFRangePr( rStrm );    break;
                case OOBIN_ID_PCDFDISCRETEPR:   return this;
                case OOBIN_ID_PCDFGROUPITEMS:   return this;
            }
        break;

        case OOBIN_ID_PCDFSHAREDITEMS:  mrCacheField.importPCDFSharedItem( nRecId, rStrm );     break;
        case OOBIN_ID_PCDFDISCRETEPR:   mrCacheField.importPCDFDiscretePrItem( nRecId, rStrm ); break;
        case OOBIN_ID_PCDFGROUPITEMS:   mrCacheField.importPCDFGroupItem( nRecId, rStrm );      break;
    }
    return 0;
}

void OoxPivotCacheFieldContext::onStartRecord( RecordInputStream& rStrm )
{
    if( isRootElement() )
        mrCacheField.importPCDField( rStrm );
}

// ============================================================================

OoxPivotCacheDefinitionFragment::OoxPivotCacheDefinitionFragment(
        const WorkbookHelper& rHelper, const OUString& rFragmentPath, PivotCache& rPivotCache ) :
    OoxWorkbookFragmentBase( rHelper, rFragmentPath ),
    mrPivotCache( rPivotCache )
{
}

ContextHandlerRef OoxPivotCacheDefinitionFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
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
            if( nElement == XLS_TOKEN( cacheField ) ) return new OoxPivotCacheFieldContext( *this, mrPivotCache.createCacheField() );
        break;
    }
    return 0;
}

ContextHandlerRef OoxPivotCacheDefinitionFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == OOBIN_ID_PCDEFINITION ) { mrPivotCache.importPCDefinition( rStrm ); return this; }
        break;

        case OOBIN_ID_PCDEFINITION:
            switch( nRecId )
            {
                case OOBIN_ID_PCDSOURCE: mrPivotCache.importPCDSource( rStrm ); return this;
                case OOBIN_ID_PCDFIELDS: return this;
            }
        break;

        case OOBIN_ID_PCDSOURCE:
            if( nRecId == OOBIN_ID_PCDSHEETSOURCE ) mrPivotCache.importPCDSheetSource( rStrm, getRelations() );
        break;

        case OOBIN_ID_PCDFIELDS:
            if( nRecId == OOBIN_ID_PCDFIELD ) return new OoxPivotCacheFieldContext( *this, mrPivotCache.createCacheField() );
        break;
    }
    return 0;
}

const RecordInfo* OoxPivotCacheDefinitionFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { OOBIN_ID_PCDEFINITION,    OOBIN_ID_PCDEFINITION + 1       },
        { OOBIN_ID_PCDFDISCRETEPR,  OOBIN_ID_PCDFDISCRETEPR + 1     },
        { OOBIN_ID_PCDFGROUPITEMS,  OOBIN_ID_PCDFGROUPITEMS + 1     },
        { OOBIN_ID_PCDFIELD,        OOBIN_ID_PCDFIELD + 1           },
        { OOBIN_ID_PCDFIELDGROUP,   OOBIN_ID_PCDFIELDGROUP + 1      },
        { OOBIN_ID_PCDFIELDS,       OOBIN_ID_PCDFIELDS + 1          },
        { OOBIN_ID_PCDFRANGEPR,     OOBIN_ID_PCDFRANGEPR + 1        },
        { OOBIN_ID_PCDFSHAREDITEMS, OOBIN_ID_PCDFSHAREDITEMS + 1    },
        { OOBIN_ID_PCITEM_ARRAY,    OOBIN_ID_PCITEM_ARRAY + 1       },
        { OOBIN_ID_PCDSHEETSOURCE,  OOBIN_ID_PCDSHEETSOURCE + 1     },
        { OOBIN_ID_PCDSOURCE,       OOBIN_ID_PCDSOURCE + 1          },
        { -1,                       -1                              }
    };
    return spRecInfos;
}

void OoxPivotCacheDefinitionFragment::finalizeImport()
{
    // finalize the cache (check source range etc.)
    mrPivotCache.finalizeImport();

    // load the cache records, if the cache is based on a deleted or an external worksheet
    if( mrPivotCache.isValidDataSource() && mrPivotCache.isBasedOnDummySheet() )
    {
        OUString aRecFragmentPath = getRelations().getFragmentPathFromRelId( mrPivotCache.getRecordsRelId() );
        if( aRecFragmentPath.getLength() > 0 )
            importOoxFragment( new OoxPivotCacheRecordsFragment( *this, aRecFragmentPath, mrPivotCache ) );
    }
}

// ============================================================================

OoxPivotCacheRecordsFragment::OoxPivotCacheRecordsFragment( const WorkbookHelper& rHelper,
        const OUString& rFragmentPath, const PivotCache& rPivotCache ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath, ISegmentProgressBarRef(), SHEETTYPE_WORKSHEET, rPivotCache.getSourceRange().Sheet ),
    mrPivotCache( rPivotCache ),
    mnCol( 0 ),
    mnRow( 0 ),
    mbInRecord( false )
{
    // prepare sheet: insert column header names into top row
    rPivotCache.writeSourceHeaderCells( *this );
}

ContextHandlerRef OoxPivotCacheRecordsFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
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
                case XLS_TOKEN( e ):    aItem.readError( rAttribs, getUnitConverter() );    break;
                case XLS_TOKEN( x ):    aItem.readIndex( rAttribs );                        break;
                default:    OSL_ENSURE( false, "OoxPivotCacheRecordsFragment::onCreateContext - unexpected element" );
            }
            mrPivotCache.writeSourceDataCell( *this, mnCol, mnRow, aItem );
            ++mnCol;
        }
        break;
    }
    return 0;
}

ContextHandlerRef OoxPivotCacheRecordsFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == OOBIN_ID_PCRECORDS ) return this;
        break;

        case OOBIN_ID_PCRECORDS:
            switch( nRecId )
            {
                case OOBIN_ID_PCRECORD:     importPCRecord( rStrm );                break;
                case OOBIN_ID_PCRECORDDT:   startCacheRecord();                     break;
                default:                    importPCRecordItem( nRecId, rStrm );    break;
            }
        break;
    }
    return 0;
}

const RecordInfo* OoxPivotCacheRecordsFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { OOBIN_ID_PCRECORDS,   OOBIN_ID_PCRECORDS + 1  },
        { -1,                   -1                      }
    };
    return spRecInfos;
}

// private --------------------------------------------------------------------

void OoxPivotCacheRecordsFragment::startCacheRecord()
{
    mnCol = 0;
    ++mnRow;
    mbInRecord = true;
}

void OoxPivotCacheRecordsFragment::importPCRecord( RecordInputStream& rStrm )
{
    startCacheRecord();
    mrPivotCache.importPCRecord( rStrm, *this, mnRow );
    mbInRecord = false;
}

void OoxPivotCacheRecordsFragment::importPCRecordItem( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    if( mbInRecord )
    {
        PivotCacheItem aItem;
        switch( nRecId )
        {
            case OOBIN_ID_PCITEM_MISSING:                               break;
            case OOBIN_ID_PCITEM_STRING:    aItem.readString( rStrm );  break;
            case OOBIN_ID_PCITEM_DOUBLE:    aItem.readDouble( rStrm );  break;
            case OOBIN_ID_PCITEM_DATE:      aItem.readDate( rStrm );    break;
            case OOBIN_ID_PCITEM_BOOL:      aItem.readBool( rStrm );    break;
            case OOBIN_ID_PCITEM_ERROR:     aItem.readError( rStrm );   break;
            case OOBIN_ID_PCITEM_INDEX:     aItem.readIndex( rStrm );   break;
            default:    OSL_ENSURE( false, "OoxPivotCacheRecordsFragment::importPCRecordItem - unexpected record" );
        }
        mrPivotCache.writeSourceDataCell( *this, mnCol, mnRow, aItem );
        ++mnCol;
    }
}

// ============================================================================
// ============================================================================

namespace {

bool lclSeekToPCDField( BiffInputStream& rStrm )
{
    sal_Int64 nRecHandle = rStrm.getRecHandle();
    while( rStrm.startNextRecord() )
        if( rStrm.getRecId() == BIFF_ID_PCDFIELD )
            return true;
    rStrm.startRecordByHandle( nRecHandle );
    return false;
}

} // namespace

// ----------------------------------------------------------------------------

BiffPivotCacheFragment::BiffPivotCacheFragment(
        const WorkbookHelper& rHelper, const ::rtl::OUString& rStrmName, PivotCache& rPivotCache ) :
    BiffWorkbookFragmentBase( rHelper, rStrmName, true ),
    mrPivotCache( rPivotCache )
{
}

bool BiffPivotCacheFragment::importFragment()
{
    if( mrStrm.startNextRecord() && (mrStrm.getRecId() == BIFF_ID_PCDEFINITION) )
    {
        // read PCDEFINITION and optional PCDEFINITION2 records
        mrPivotCache.importPCDefinition( mrStrm );

        // read cache fields as long as another PCDFIELD record can be found
        while( lclSeekToPCDField( mrStrm ) )
            mrPivotCache.createCacheField( true ).importPCDField( mrStrm );

        // finalize the cache (check source range etc.)
        mrPivotCache.finalizeImport();

        // load the cache records, if the cache is based on a deleted or an external worksheet
        if( mrPivotCache.isValidDataSource() && mrPivotCache.isBasedOnDummySheet() )
        {
            /*  Last call of lclSeekToPCDField() failed and kept stream position
                unchanged. Stream should point to source data table now. */
            BiffPivotCacheRecordsContext aContext( *this, mrPivotCache );
            if( aContext.isValidSheet() )
                while( mrStrm.startNextRecord() && (mrStrm.getRecId() != BIFF_ID_EOF) )
                    aContext.importRecord();
        }
    }

    return mrStrm.getRecId() == BIFF_ID_EOF;
}

// ============================================================================

BiffPivotCacheRecordsContext::BiffPivotCacheRecordsContext(
        const BiffWorkbookFragmentBase& rFragment, const PivotCache& rPivotCache ) :
    BiffWorksheetContextBase( rFragment, ISegmentProgressBarRef(), SHEETTYPE_WORKSHEET, rPivotCache.getSourceRange().Sheet ),
    mrPivotCache( rPivotCache ),
    mnColIdx( 0 ),
    mnRow( 0 ),
    mbHasShared( false ),
    mbInRow( false )
{
    // prepare sheet: insert column header names into top row
    mrPivotCache.writeSourceHeaderCells( *this );

    // find all fields without shared items, remember column indexes in source data
    for( sal_Int32 nFieldIdx = 0, nFieldCount = mrPivotCache.getCacheFieldCount(), nCol = 0; nFieldIdx < nFieldCount; ++nFieldIdx )
    {
        const PivotCacheField* pCacheField = mrPivotCache.getCacheField( nFieldIdx );
        if( pCacheField && pCacheField->isDatabaseField() )
        {
            if( pCacheField->hasSharedItems() )
                mbHasShared = true;
            else
                maUnsharedCols.push_back( nCol );
            ++nCol;
        }
    }
}

void BiffPivotCacheRecordsContext::importRecord()
{
    if( mrStrm.getRecId() == BIFF_ID_PCITEM_INDEXLIST )
    {
        OSL_ENSURE( mbHasShared, "BiffPivotCacheRecordsContext::importRecord - unexpected PCITEM_INDEXLIST record" );
        // PCITEM_INDEXLIST record always in front of a new data row
        startNextRow();
        mrPivotCache.importPCItemIndexList( mrStrm, *this, mnRow );
        mbInRow = !maUnsharedCols.empty();  // mbInRow remains true, if unshared items are expected
        return;
    }

    PivotCacheItem aItem;
    switch( mrStrm.getRecId() )
    {
        case BIFF_ID_PCITEM_MISSING:                                        break;
        case BIFF_ID_PCITEM_STRING:     aItem.readString( mrStrm, *this );  break;
        case BIFF_ID_PCITEM_DOUBLE:     aItem.readDouble( mrStrm );         break;
        case BIFF_ID_PCITEM_INTEGER:    aItem.readInteger( mrStrm );        break;
        case BIFF_ID_PCITEM_DATE:       aItem.readDate( mrStrm );           break;
        case BIFF_ID_PCITEM_BOOL:       aItem.readBool( mrStrm );           break;
        case BIFF_ID_PCITEM_ERROR:      aItem.readError( mrStrm );          break;
        default:                        return; // unknown record, ignore
    }

    // find next column index, might start new row if no fields with shared items exist
    if( mbInRow && (mnColIdx == maUnsharedCols.size()) )
    {
        OSL_ENSURE( !mbHasShared, "BiffPivotCacheRecordsContext::importRecord - PCITEM_INDEXLIST record missing" );
        mbInRow = mbHasShared;  // do not leave current row if PCITEM_INDEXLIST is expected
    }
    // start next row on first call, or on row wrap without shared items
    if( !mbInRow )
        startNextRow();

    // write the item data to the sheet cell
    OSL_ENSURE( mnColIdx < maUnsharedCols.size(), "BiffPivotCacheRecordsContext::importRecord - invalid column index" );
    if( mnColIdx < maUnsharedCols.size() )
        mrPivotCache.writeSourceDataCell( *this, maUnsharedCols[ mnColIdx ], mnRow, aItem );
    ++mnColIdx;
}

void BiffPivotCacheRecordsContext::startNextRow()
{
    mnColIdx = 0;
    ++mnRow;
    mbInRow = true;
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
