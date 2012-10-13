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

#include "querytablebuffer.hxx"

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/sheet/XAreaLink.hpp>
#include <com/sun/star/sheet/XAreaLinks.hpp>
#include "oox/core/filterbase.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/token/properties.hxx"
#include "addressconverter.hxx"
#include "biffinputstream.hxx"
#include "connectionsbuffer.hxx"
#include "defnamesbuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

// ============================================================================

namespace {

const sal_uInt32 BIFF12_QUERYTABLE_HEADERS          = 0x00000001;
const sal_uInt32 BIFF12_QUERYTABLE_ROWNUMBERS       = 0x00000002;
const sal_uInt32 BIFF12_QUERYTABLE_DISABLEREFRESH   = 0x00000004;
const sal_uInt32 BIFF12_QUERYTABLE_BACKGROUND       = 0x00000008;
const sal_uInt32 BIFF12_QUERYTABLE_FIRSTBACKGROUND  = 0x00000010;
const sal_uInt32 BIFF12_QUERYTABLE_REFRESHONLOAD    = 0x00000020;
const sal_uInt32 BIFF12_QUERYTABLE_FILLFORMULAS     = 0x00000100;
const sal_uInt32 BIFF12_QUERYTABLE_SAVEDATA         = 0x00000200;
const sal_uInt32 BIFF12_QUERYTABLE_DISABLEEDIT      = 0x00000400;
const sal_uInt32 BIFF12_QUERYTABLE_PRESERVEFORMAT   = 0x00000800;
const sal_uInt32 BIFF12_QUERYTABLE_ADJUSTCOLWIDTH   = 0x00001000;
const sal_uInt32 BIFF12_QUERYTABLE_INTERMEDIATE     = 0x00002000;
const sal_uInt32 BIFF12_QUERYTABLE_APPLYNUMFMT      = 0x00004000;
const sal_uInt32 BIFF12_QUERYTABLE_APPLYFONT        = 0x00008000;
const sal_uInt32 BIFF12_QUERYTABLE_APPLYALIGNMENT   = 0x00010000;
const sal_uInt32 BIFF12_QUERYTABLE_APPLYBORDER      = 0x00020000;
const sal_uInt32 BIFF12_QUERYTABLE_APPLYFILL        = 0x00040000;
const sal_uInt32 BIFF12_QUERYTABLE_APPLYPROTECTION  = 0x00080000;

const sal_uInt16 BIFF_QUERYTABLE_HEADERS            = 0x0001;
const sal_uInt16 BIFF_QUERYTABLE_ROWNUMBERS         = 0x0002;
const sal_uInt16 BIFF_QUERYTABLE_DISABLEREFRESH     = 0x0004;
const sal_uInt16 BIFF_QUERYTABLE_BACKGROUND         = 0x0008;
const sal_uInt16 BIFF_QUERYTABLE_FIRSTBACKGROUND    = 0x0010;
const sal_uInt16 BIFF_QUERYTABLE_REFRESHONLOAD      = 0x0020;
const sal_uInt16 BIFF_QUERYTABLE_DELETEUNUSED       = 0x0040;
const sal_uInt16 BIFF_QUERYTABLE_FILLFORMULAS       = 0x0080;
const sal_uInt16 BIFF_QUERYTABLE_ADJUSTCOLWIDTH     = 0x0100;
const sal_uInt16 BIFF_QUERYTABLE_SAVEDATA           = 0x0200;
const sal_uInt16 BIFF_QUERYTABLE_DISABLEEDIT        = 0x0400;
const sal_uInt16 BIFF_QUERYTABLE_OVERWRITEEXISTING  = 0x2000;

const sal_uInt16 BIFF_QUERYTABLE_APPLYNUMFMT        = 0x0001;
const sal_uInt16 BIFF_QUERYTABLE_APPLYFONT          = 0x0002;
const sal_uInt16 BIFF_QUERYTABLE_APPLYALIGNMENT     = 0x0004;
const sal_uInt16 BIFF_QUERYTABLE_APPLYBORDER        = 0x0008;
const sal_uInt16 BIFF_QUERYTABLE_APPLYFILL          = 0x0010;
const sal_uInt16 BIFF_QUERYTABLE_APPLYPROTECTION    = 0x0020;

const sal_uInt32 BIFF_QTREFRESH_PRESERVEFORMAT      = 0x00000001;
const sal_uInt32 BIFF_QTREFRESH_ADJUSTCOLWIDTH      = 0x00000002;

// ----------------------------------------------------------------------------

void lclAppendWebQueryTableName( OUStringBuffer& rTables, const OUString& rTableName )
{
    if( !rTableName.isEmpty() )
    {
        if( rTables.getLength() > 0 )
            rTables.append( sal_Unicode( ';' ) );
        rTables.appendAscii( RTL_CONSTASCII_STRINGPARAM( "HTML__" ) ).append( rTableName );
    }
}

void lclAppendWebQueryTableIndex( OUStringBuffer& rTables, sal_Int32 nTableIndex )
{
    if( nTableIndex > 0 )
    {
        if( rTables.getLength() > 0 )
            rTables.append( sal_Unicode( ';' ) );
        rTables.appendAscii( RTL_CONSTASCII_STRINGPARAM( "HTML_" ) ).append( nTableIndex );
    }
}

OUString lclBuildWebQueryTables( const WebPrModel::TablesVector& rTables )
{
    if( rTables.empty() )
        return OUString( "HTML_tables" );

    OUStringBuffer aTables;
    for( WebPrModel::TablesVector::const_iterator aIt = rTables.begin(), aEnd = rTables.end(); aIt != aEnd; ++aIt )
    {
        if( aIt->has< OUString >() )
            lclAppendWebQueryTableName( aTables, aIt->get< OUString >() );
        else if( aIt->has< sal_Int32 >() )
            lclAppendWebQueryTableIndex( aTables, aIt->get< sal_Int32 >() );
    }
    return aTables.makeStringAndClear();
}

Reference< XAreaLink > lclFindAreaLink(
        const Reference< XAreaLinks >& rxAreaLinks, const CellAddress& rDestPos,
        const OUString& rFileUrl, const OUString& rTables, const OUString& rFilterName, const OUString& rFilterOptions )
{
    try
    {
        Reference< XEnumerationAccess > xAreaLinksEA( rxAreaLinks, UNO_QUERY_THROW );
        Reference< XEnumeration > xAreaLinksEnum( xAreaLinksEA->createEnumeration(), UNO_SET_THROW );
        while( xAreaLinksEnum->hasMoreElements() )
        {
            Reference< XAreaLink > xAreaLink( xAreaLinksEnum->nextElement(), UNO_QUERY_THROW );
            PropertySet aPropSet( xAreaLink );
            CellRangeAddress aDestArea = xAreaLink->getDestArea();
            OUString aString;
            if( (rDestPos.Sheet == aDestArea.Sheet) && (rDestPos.Column == aDestArea.StartColumn) && (rDestPos.Row == aDestArea.StartRow) &&
                    (rTables == xAreaLink->getSourceArea()) &&
                    aPropSet.getProperty( aString, PROP_Url ) && (rFileUrl == aString) &&
                    aPropSet.getProperty( aString, PROP_Filter ) && (rFilterName == aString) &&
                    aPropSet.getProperty( aString, PROP_FilterOptions ) && (rFilterOptions == aString) )
                return xAreaLink;
        }
    }
    catch( Exception& )
    {
    }
    return Reference< XAreaLink >();
}

} // namespace

// ============================================================================

QueryTableModel::QueryTableModel() :
    mnConnId( -1 ),
    mnGrowShrinkType( XML_insertDelete ),
    mbHeaders( true ),
    mbRowNumbers( false ),
    mbDisableRefresh( false ),
    mbBackground( true ),
    mbFirstBackground( false ),
    mbRefreshOnLoad( false ),
    mbFillFormulas( false ),
    mbRemoveDataOnSave( false ),
    mbDisableEdit( false ),
    mbPreserveFormat( true ),
    mbAdjustColWidth( true ),
    mbIntermediate( false )
{
}

// ----------------------------------------------------------------------------

QueryTable::QueryTable( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
{
}

void QueryTable::importQueryTable( const AttributeList& rAttribs )
{
    maModel.maDefName          = rAttribs.getXString( XML_name, OUString() );
    maModel.mnConnId           = rAttribs.getInteger( XML_connectionId, -1 );
    maModel.mnGrowShrinkType   = rAttribs.getToken( XML_growShrinkType, XML_insertDelete );
    maModel.mnAutoFormatId     = rAttribs.getInteger( XML_autoFormatId, 0 );
    maModel.mbHeaders          = rAttribs.getBool( XML_headers, true );
    maModel.mbRowNumbers       = rAttribs.getBool( XML_rowNumbers, false );
    maModel.mbDisableRefresh   = rAttribs.getBool( XML_disableRefresh, false );
    maModel.mbBackground       = rAttribs.getBool( XML_backgroundRefresh, true );
    maModel.mbFirstBackground  = rAttribs.getBool( XML_firstBackgroundRefresh, false );
    maModel.mbRefreshOnLoad    = rAttribs.getBool( XML_refreshOnLoad, false );
    maModel.mbFillFormulas     = rAttribs.getBool( XML_fillFormulas, false );
    maModel.mbRemoveDataOnSave = rAttribs.getBool( XML_removeDataOnSave, false );
    maModel.mbDisableEdit      = rAttribs.getBool( XML_disableEdit, false );
    maModel.mbPreserveFormat   = rAttribs.getBool( XML_preserveFormatting, true );
    maModel.mbAdjustColWidth   = rAttribs.getBool( XML_adjustColumnWidth, true );
    maModel.mbIntermediate     = rAttribs.getBool( XML_intermediate, false );
    maModel.mbApplyNumFmt      = rAttribs.getBool( XML_applyNumberFormats, false );
    maModel.mbApplyFont        = rAttribs.getBool( XML_applyFontFormats, false );
    maModel.mbApplyAlignment   = rAttribs.getBool( XML_applyAlignmentFormats, false );
    maModel.mbApplyBorder      = rAttribs.getBool( XML_applyBorderFormats, false );
    maModel.mbApplyFill        = rAttribs.getBool( XML_applyPatternFormats, false );
    // OOXML and BIFF12 documentation differ: OOXML mentions width/height, BIFF12 mentions protection
    maModel.mbApplyProtection  = rAttribs.getBool( XML_applyWidthHeightFormats, false );
}

void QueryTable::importQueryTable( SequenceInputStream& rStrm )
{
    sal_uInt32 nFlags;
    rStrm >> nFlags;
    maModel.mnAutoFormatId = rStrm.readuInt16();
    rStrm >> maModel.mnConnId >> maModel.maDefName;

    static const sal_Int32 spnGrowShrinkTypes[] = { XML_insertClear, XML_insertDelete, XML_overwriteClear };
    maModel.mnGrowShrinkType = STATIC_ARRAY_SELECT( spnGrowShrinkTypes, extractValue< sal_uInt8 >( nFlags, 6, 2 ), XML_insertDelete );

    maModel.mbHeaders           = getFlag( nFlags, BIFF12_QUERYTABLE_HEADERS );
    maModel.mbRowNumbers        = getFlag( nFlags, BIFF12_QUERYTABLE_ROWNUMBERS );
    maModel.mbDisableRefresh    = getFlag( nFlags, BIFF12_QUERYTABLE_DISABLEREFRESH );
    maModel.mbBackground        = getFlag( nFlags, BIFF12_QUERYTABLE_BACKGROUND );
    maModel.mbFirstBackground   = getFlag( nFlags, BIFF12_QUERYTABLE_FIRSTBACKGROUND );
    maModel.mbRefreshOnLoad     = getFlag( nFlags, BIFF12_QUERYTABLE_REFRESHONLOAD );
    maModel.mbFillFormulas      = getFlag( nFlags, BIFF12_QUERYTABLE_FILLFORMULAS );
    maModel.mbRemoveDataOnSave  = !getFlag( nFlags, BIFF12_QUERYTABLE_SAVEDATA ); // flag negated in BIFF12
    maModel.mbDisableEdit       = getFlag( nFlags, BIFF12_QUERYTABLE_DISABLEEDIT );
    maModel.mbPreserveFormat    = getFlag( nFlags, BIFF12_QUERYTABLE_PRESERVEFORMAT );
    maModel.mbAdjustColWidth    = getFlag( nFlags, BIFF12_QUERYTABLE_ADJUSTCOLWIDTH );
    maModel.mbIntermediate      = getFlag( nFlags, BIFF12_QUERYTABLE_INTERMEDIATE );
    maModel.mbApplyNumFmt       = getFlag( nFlags, BIFF12_QUERYTABLE_APPLYNUMFMT );
    maModel.mbApplyFont         = getFlag( nFlags, BIFF12_QUERYTABLE_APPLYFONT );
    maModel.mbApplyAlignment    = getFlag( nFlags, BIFF12_QUERYTABLE_APPLYALIGNMENT );
    maModel.mbApplyBorder       = getFlag( nFlags, BIFF12_QUERYTABLE_APPLYBORDER );
    maModel.mbApplyFill         = getFlag( nFlags, BIFF12_QUERYTABLE_APPLYFILL );
    maModel.mbApplyProtection   = getFlag( nFlags, BIFF12_QUERYTABLE_APPLYPROTECTION );
}

void QueryTable::finalizeImport()
{
    ConnectionRef xConnection = getConnections().getConnection( maModel.mnConnId );
    OSL_ENSURE( xConnection.get(), "QueryTable::finalizeImport - missing connection object" );
    if( xConnection.get() && (xConnection->getConnectionType() == BIFF12_CONNECTION_HTML) )
    {
        // check that valid web query properties exist
        const WebPrModel* pWebPr = xConnection->getModel().mxWebPr.get();
        if( pWebPr && !pWebPr->mbXml )
        {
            OUString aFileUrl = getBaseFilter().getAbsoluteUrl( pWebPr->maUrl );
            if( !aFileUrl.isEmpty() )
            {
                // resolve destination cell range (stored as defined name containing the range)
                OUString aDefName = maModel.maDefName.replace( ' ', '_' ).replace( '-', '_' );
                DefinedNameRef xDefName = getDefinedNames().getByModelName( aDefName, getSheetIndex() );
                OSL_ENSURE( xDefName.get(), "QueryTable::finalizeImport - missing defined name" );
                if( xDefName.get() )
                {
                    CellRangeAddress aDestRange;
                    bool bIsRange = xDefName->getAbsoluteRange( aDestRange ) && (aDestRange.Sheet == getSheetIndex());
                    OSL_ENSURE( bIsRange, "QueryTable::finalizeImport - defined name does not contain valid cell range" );
                    if( bIsRange && getAddressConverter().checkCellRange( aDestRange, false, true ) )
                    {
                        CellAddress aDestPos( aDestRange.Sheet, aDestRange.StartColumn, aDestRange.StartRow );
                        // find tables mode: entire document, all tables, or specific tables
                        OUString aTables = pWebPr->mbHtmlTables ? lclBuildWebQueryTables( pWebPr->maTables ) : "HTML_all";
                        if( !aTables.isEmpty() ) try
                        {
                            PropertySet aDocProps( getDocument() );
                            Reference< XAreaLinks > xAreaLinks( aDocProps.getAnyProperty( PROP_AreaLinks ), UNO_QUERY_THROW );
                            OUString aFilterName = "calc_HTML_WebQuery";
                            OUString aFilterOptions;
                            xAreaLinks->insertAtPosition( aDestPos, aFileUrl, aTables, aFilterName, aFilterOptions );
                            // set refresh interval (convert minutes to seconds)
                            sal_Int32 nRefreshPeriod = xConnection->getModel().mnInterval * 60;
                            if( nRefreshPeriod > 0 )
                            {
                                PropertySet aPropSet( lclFindAreaLink( xAreaLinks, aDestPos, aFileUrl, aTables, aFilterName, aFilterOptions ) );
                                aPropSet.setProperty( PROP_RefreshPeriod, nRefreshPeriod );
                            }
                        }
                        catch( Exception& )
                        {
                        }
                    }
                }
            }
        }
    }
}

// ============================================================================

QueryTableBuffer::QueryTableBuffer( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper )
{
}

QueryTable& QueryTableBuffer::createQueryTable()
{
    QueryTableVector::value_type xQueryTable( new QueryTable( *this ) );
    maQueryTables.push_back( xQueryTable );
    return *xQueryTable;
}

void QueryTableBuffer::finalizeImport()
{
    maQueryTables.forEachMem( &QueryTable::finalizeImport );
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
