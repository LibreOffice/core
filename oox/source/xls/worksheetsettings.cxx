/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: worksheetsettings.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:09:58 $
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

#include "oox/xls/worksheetsettings.hxx"
#include <com/sun/star/util/XProtectable.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/pagesettings.hxx"
#include "oox/xls/workbooksettings.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::util::XProtectable;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const sal_uInt8 OOBIN_SHEETPR_FILTERMODE        = 0x01;
const sal_uInt8 OOBIN_SHEETPR_EVAL_CF           = 0x02;

const sal_uInt16 BIFF_SHEETPR_DIALOGSHEET       = 0x0010;
const sal_uInt16 BIFF_SHEETPR_APPLYSTYLES       = 0x0020;
const sal_uInt16 BIFF_SHEETPR_SYMBOLSBELOW      = 0x0040;
const sal_uInt16 BIFF_SHEETPR_SYMBOLSRIGHT      = 0x0080;
const sal_uInt16 BIFF_SHEETPR_FITTOPAGES        = 0x0100;
const sal_uInt16 BIFF_SHEETPR_SKIPEXT           = 0x0200;       /// BIFF3-BIFF4

const sal_uInt16 BIFF_SHEETPROT_OBJECTS         = 0x0001;
const sal_uInt16 BIFF_SHEETPROT_SCENARIOS       = 0x0002;
const sal_uInt16 BIFF_SHEETPROT_FORMAT_CELLS    = 0x0004;
const sal_uInt16 BIFF_SHEETPROT_FORMAT_COLUMNS  = 0x0008;
const sal_uInt16 BIFF_SHEETPROT_FORMAT_ROWS     = 0x0010;
const sal_uInt16 BIFF_SHEETPROT_INSERT_COLUMNS  = 0x0020;
const sal_uInt16 BIFF_SHEETPROT_INSERT_ROWS     = 0x0040;
const sal_uInt16 BIFF_SHEETPROT_INSERT_HLINKS   = 0x0080;
const sal_uInt16 BIFF_SHEETPROT_DELETE_COLUMNS  = 0x0100;
const sal_uInt16 BIFF_SHEETPROT_DELETE_ROWS     = 0x0200;
const sal_uInt16 BIFF_SHEETPROT_SELECT_LOCKED   = 0x0400;
const sal_uInt16 BIFF_SHEETPROT_SORT            = 0x0800;
const sal_uInt16 BIFF_SHEETPROT_AUTOFILTER      = 0x1000;
const sal_uInt16 BIFF_SHEETPROT_PIVOTTABLES     = 0x2000;
const sal_uInt16 BIFF_SHEETPROT_SELECT_UNLOCKED = 0x4000;

} // namespace

// ============================================================================

OoxSheetPrData::OoxSheetPrData() :
    mbFilterMode( false ),
    mbApplyStyles( false ),
    mbSummaryBelow( true ),
    mbSummaryRight( true )
{
}

// ============================================================================

OoxSheetProtectionData::OoxSheetProtectionData() :
    mnPasswordHash( 0 ),
    mbSheet( false ),
    mbObjects( false ),
    mbScenarios( false ),
    mbFormatCells( true ),
    mbFormatColumns( true ),
    mbFormatRows( true ),
    mbInsertColumns( true ),
    mbInsertRows( true ),
    mbInsertHyperlinks( true ),
    mbDeleteColumns( true ),
    mbDeleteRows( true ),
    mbSelectLocked( false ),
    mbSort( true ),
    mbAutoFilter( true ),
    mbPivotTables( true ),
    mbSelectUnlocked( false )
{
}

// ============================================================================

namespace {

sal_uInt16 lclGetCheckedHash( sal_Int32 nHash )
{
    OSL_ENSURE( (0 <= nHash) && (nHash <= SAL_MAX_UINT16), "lclGetCheckedHash - invalid password hash" );
    return getLimitedValue< sal_uInt16, sal_Int32 >( nHash, 0, SAL_MAX_UINT16 );
}

} // namespace

WorksheetSettings::WorksheetSettings( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper ),
    maPhoneticSett( rHelper )
{
}

void WorksheetSettings::importSheetPr( const AttributeList& rAttribs )
{
    maOoxSheetData.maCodeName = rAttribs.getString( XML_codeName );
    maOoxSheetData.mbFilterMode = rAttribs.getBool( XML_filterMode, false );
}

void WorksheetSettings::importChartSheetPr( const AttributeList& rAttribs )
{
    maOoxSheetData.maCodeName = rAttribs.getString( XML_codeName );
}

void WorksheetSettings::importTabColor( const AttributeList& rAttribs )
{
    maOoxSheetData.maTabColor.importColor( rAttribs );
}

void WorksheetSettings::importOutlinePr( const AttributeList& rAttribs )
{
    maOoxSheetData.mbApplyStyles  = rAttribs.getBool( XML_applyStyles, false );
    maOoxSheetData.mbSummaryBelow = rAttribs.getBool( XML_summaryBelow, true );
    maOoxSheetData.mbSummaryRight = rAttribs.getBool( XML_summaryRight, true );
}

void WorksheetSettings::importSheetProtection( const AttributeList& rAttribs )
{
    maOoxProtData.mnPasswordHash     = lclGetCheckedHash( rAttribs.getHex( XML_password, 0 ) );
    maOoxProtData.mbSheet            = rAttribs.getBool( XML_sheet, false );
    maOoxProtData.mbObjects          = rAttribs.getBool( XML_objects, false );
    maOoxProtData.mbScenarios        = rAttribs.getBool( XML_scenarios, false );
    maOoxProtData.mbFormatCells      = rAttribs.getBool( XML_formatCells, true );
    maOoxProtData.mbFormatColumns    = rAttribs.getBool( XML_formatColumns, true );
    maOoxProtData.mbFormatRows       = rAttribs.getBool( XML_formatRows, true );
    maOoxProtData.mbInsertColumns    = rAttribs.getBool( XML_insertColumns, true );
    maOoxProtData.mbInsertRows       = rAttribs.getBool( XML_insertRows, true );
    maOoxProtData.mbInsertHyperlinks = rAttribs.getBool( XML_insertHyperlinks, true );
    maOoxProtData.mbDeleteColumns    = rAttribs.getBool( XML_deleteColumns, true );
    maOoxProtData.mbDeleteRows       = rAttribs.getBool( XML_deleteRows, true );
    maOoxProtData.mbSelectLocked     = rAttribs.getBool( XML_selectLockedCells, false );
    maOoxProtData.mbSort             = rAttribs.getBool( XML_sort, true );
    maOoxProtData.mbAutoFilter       = rAttribs.getBool( XML_autoFilter, true );
    maOoxProtData.mbPivotTables      = rAttribs.getBool( XML_pivotTables, true );
    maOoxProtData.mbSelectUnlocked   = rAttribs.getBool( XML_selectUnlockedCells, false );
}

void WorksheetSettings::importChartProtection( const AttributeList& rAttribs )
{
    maOoxProtData.mnPasswordHash = lclGetCheckedHash( rAttribs.getHex( XML_password, 0 ) );
    maOoxProtData.mbSheet        = rAttribs.getBool( XML_content, false );
    maOoxProtData.mbObjects      = rAttribs.getBool( XML_objects, false );
}

void WorksheetSettings::importPhoneticPr( const AttributeList& rAttribs )
{
    maPhoneticSett.importPhoneticPr( rAttribs );
}

void WorksheetSettings::importSheetPr( RecordInputStream& rStrm )
{
    sal_uInt16 nFlags1;
    sal_uInt8 nFlags2;
    rStrm >> nFlags1 >> nFlags2 >> maOoxSheetData.maTabColor;
    rStrm.skip( 8 );    // sync anchor cell
    rStrm >> maOoxSheetData.maCodeName;
    // sheet settings
    maOoxSheetData.mbFilterMode = getFlag( nFlags2, OOBIN_SHEETPR_FILTERMODE );
    // outline settings, equal flags in BIFF and OOBIN
    maOoxSheetData.mbApplyStyles  = getFlag( nFlags1, BIFF_SHEETPR_APPLYSTYLES );
    maOoxSheetData.mbSummaryRight = getFlag( nFlags1, BIFF_SHEETPR_SYMBOLSRIGHT );
    maOoxSheetData.mbSummaryBelow = getFlag( nFlags1, BIFF_SHEETPR_SYMBOLSBELOW );
    /*  Fit printout to width/height - for whatever reason, this flag is still
        stored separated from the page settings */
    getPageSettings().setFitToPagesMode( getFlag( nFlags1, BIFF_SHEETPR_FITTOPAGES ) );
}

void WorksheetSettings::importChartSheetPr( RecordInputStream& rStrm )
{
    rStrm.skip( 2 );    // flags, contains only the 'published' flag
    rStrm >> maOoxSheetData.maTabColor >> maOoxSheetData.maCodeName;
}

void WorksheetSettings::importSheetProtection( RecordInputStream& rStrm )
{
    rStrm >> maOoxProtData.mnPasswordHash;
    // no flags field for all these boolean flags?!?
    maOoxProtData.mbSheet            = rStrm.readInt32() != 0;
    maOoxProtData.mbObjects          = rStrm.readInt32() != 0;
    maOoxProtData.mbScenarios        = rStrm.readInt32() != 0;
    maOoxProtData.mbFormatCells      = rStrm.readInt32() != 0;
    maOoxProtData.mbFormatColumns    = rStrm.readInt32() != 0;
    maOoxProtData.mbFormatRows       = rStrm.readInt32() != 0;
    maOoxProtData.mbInsertColumns    = rStrm.readInt32() != 0;
    maOoxProtData.mbInsertRows       = rStrm.readInt32() != 0;
    maOoxProtData.mbInsertHyperlinks = rStrm.readInt32() != 0;
    maOoxProtData.mbDeleteColumns    = rStrm.readInt32() != 0;
    maOoxProtData.mbDeleteRows       = rStrm.readInt32() != 0;
    maOoxProtData.mbSelectLocked     = rStrm.readInt32() != 0;
    maOoxProtData.mbSort             = rStrm.readInt32() != 0;
    maOoxProtData.mbAutoFilter       = rStrm.readInt32() != 0;
    maOoxProtData.mbPivotTables      = rStrm.readInt32() != 0;
    maOoxProtData.mbSelectUnlocked   = rStrm.readInt32() != 0;
}

void WorksheetSettings::importChartProtection( RecordInputStream& rStrm )
{
    rStrm >> maOoxProtData.mnPasswordHash;
    // no flags field for all these boolean flags?!?
    maOoxProtData.mbSheet            = rStrm.readInt32() != 0;
    maOoxProtData.mbObjects          = rStrm.readInt32() != 0;
}

void WorksheetSettings::importPhoneticPr( RecordInputStream& rStrm )
{
    maPhoneticSett.importPhoneticPr( rStrm );
}

void WorksheetSettings::importSheetPr( BiffInputStream& rStrm )
{
    sal_uInt16 nFlags;
    rStrm >> nFlags;
    // worksheet vs. dialogsheet
    if( getFlag( nFlags, BIFF_SHEETPR_DIALOGSHEET ) )
    {
        OSL_ENSURE( getSheetType() == SHEETTYPE_WORKSHEET, "WorksheetSettings::importSheetPr - unexpected sheet type" );
        setSheetType( SHEETTYPE_DIALOGSHEET );
    }
    // outline settings
    maOoxSheetData.mbApplyStyles  = getFlag( nFlags, BIFF_SHEETPR_APPLYSTYLES );
    maOoxSheetData.mbSummaryRight = getFlag( nFlags, BIFF_SHEETPR_SYMBOLSRIGHT );
    maOoxSheetData.mbSummaryBelow = getFlag( nFlags, BIFF_SHEETPR_SYMBOLSBELOW );
    // fit printout to width/height
    getPageSettings().setFitToPagesMode( getFlag( nFlags, BIFF_SHEETPR_FITTOPAGES ) );
    // save external linked values, in BIFF5-BIFF8 moved to BOOKBOOK record
    if( getBiff() <= BIFF4 )
        getWorkbookSettings().setSaveExtLinkValues( !getFlag( nFlags, BIFF_SHEETPR_SKIPEXT ) );
}

void WorksheetSettings::importProtect( BiffInputStream& rStrm )
{
    maOoxProtData.mbSheet = rStrm.readuInt16() != 0;
}

void WorksheetSettings::importObjectProtect( BiffInputStream& rStrm )
{
    maOoxProtData.mbObjects = rStrm.readuInt16() != 0;
}

void WorksheetSettings::importScenProtect( BiffInputStream& rStrm )
{
    maOoxProtData.mbScenarios = rStrm.readuInt16() != 0;
}

void WorksheetSettings::importPassword( BiffInputStream& rStrm )
{
    rStrm >> maOoxProtData.mnPasswordHash;
}

void WorksheetSettings::importSheetProtection( BiffInputStream& rStrm )
{
    sal_uInt16 nFlags = rStrm.skip( 19 ).readuInt16();
    // set flag means protection is disabled
    maOoxProtData.mbObjects          = !getFlag( nFlags, BIFF_SHEETPROT_OBJECTS );
    maOoxProtData.mbScenarios        = !getFlag( nFlags, BIFF_SHEETPROT_SCENARIOS );
    maOoxProtData.mbFormatCells      = !getFlag( nFlags, BIFF_SHEETPROT_FORMAT_CELLS );
    maOoxProtData.mbFormatColumns    = !getFlag( nFlags, BIFF_SHEETPROT_FORMAT_COLUMNS );
    maOoxProtData.mbFormatRows       = !getFlag( nFlags, BIFF_SHEETPROT_FORMAT_ROWS );
    maOoxProtData.mbInsertColumns    = !getFlag( nFlags, BIFF_SHEETPROT_INSERT_COLUMNS );
    maOoxProtData.mbInsertRows       = !getFlag( nFlags, BIFF_SHEETPROT_INSERT_ROWS );
    maOoxProtData.mbInsertHyperlinks = !getFlag( nFlags, BIFF_SHEETPROT_INSERT_HLINKS );
    maOoxProtData.mbDeleteColumns    = !getFlag( nFlags, BIFF_SHEETPROT_DELETE_COLUMNS );
    maOoxProtData.mbDeleteRows       = !getFlag( nFlags, BIFF_SHEETPROT_DELETE_ROWS );
    maOoxProtData.mbSelectLocked     = !getFlag( nFlags, BIFF_SHEETPROT_SELECT_LOCKED );
    maOoxProtData.mbSort             = !getFlag( nFlags, BIFF_SHEETPROT_SORT );
    maOoxProtData.mbAutoFilter       = !getFlag( nFlags, BIFF_SHEETPROT_AUTOFILTER );
    maOoxProtData.mbPivotTables      = !getFlag( nFlags, BIFF_SHEETPROT_PIVOTTABLES );
    maOoxProtData.mbSelectUnlocked   = !getFlag( nFlags, BIFF_SHEETPROT_SELECT_UNLOCKED );
}

void WorksheetSettings::importPhoneticPr( BiffInputStream& rStrm )
{
    maPhoneticSett.importPhoneticPr( rStrm );
}

void WorksheetSettings::finalizeImport()
{
    if( maOoxProtData.mbSheet )
    {
        Reference< XProtectable > xProtectable( getXSpreadsheet(), UNO_QUERY );
        if( xProtectable.is() )
            xProtectable->protect( OUString() );
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

