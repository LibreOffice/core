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

#include "oox/xls/worksheetsettings.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/pagesettings.hxx"
#include "oox/xls/workbooksettings.hxx"
#include "oox/core/filterbase.hxx"
#include "properties.hxx"

#include <com/sun/star/util/XProtectable.hpp>

using ::rtl::OUString;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::util::XProtectable;
using ::oox::core::CodecHelper;

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

const sal_uInt32 BIFF_SHEETPROT_OBJECTS         = 0x00000001;
const sal_uInt32 BIFF_SHEETPROT_SCENARIOS       = 0x00000002;
const sal_uInt32 BIFF_SHEETPROT_FORMAT_CELLS    = 0x00000004;
const sal_uInt32 BIFF_SHEETPROT_FORMAT_COLUMNS  = 0x00000008;
const sal_uInt32 BIFF_SHEETPROT_FORMAT_ROWS     = 0x00000010;
const sal_uInt32 BIFF_SHEETPROT_INSERT_COLUMNS  = 0x00000020;
const sal_uInt32 BIFF_SHEETPROT_INSERT_ROWS     = 0x00000040;
const sal_uInt32 BIFF_SHEETPROT_INSERT_HLINKS   = 0x00000080;
const sal_uInt32 BIFF_SHEETPROT_DELETE_COLUMNS  = 0x00000100;
const sal_uInt32 BIFF_SHEETPROT_DELETE_ROWS     = 0x00000200;
const sal_uInt32 BIFF_SHEETPROT_SELECT_LOCKED   = 0x00000400;
const sal_uInt32 BIFF_SHEETPROT_SORT            = 0x00000800;
const sal_uInt32 BIFF_SHEETPROT_AUTOFILTER      = 0x00001000;
const sal_uInt32 BIFF_SHEETPROT_PIVOTTABLES     = 0x00002000;
const sal_uInt32 BIFF_SHEETPROT_SELECT_UNLOCKED = 0x00004000;

} // namespace

// ============================================================================

SheetSettingsModel::SheetSettingsModel() :
    mbFilterMode( false ),
    mbApplyStyles( false ),
    mbSummaryBelow( true ),
    mbSummaryRight( true )
{
}

// ============================================================================

SheetProtectionModel::SheetProtectionModel() :
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

WorksheetSettings::WorksheetSettings( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper ),
    maPhoneticSett( rHelper )
{
}

void WorksheetSettings::importSheetPr( const AttributeList& rAttribs )
{
    maSheetSettings.maCodeName = rAttribs.getString( XML_codeName, OUString() );
    maSheetSettings.mbFilterMode = rAttribs.getBool( XML_filterMode, false );
}

void WorksheetSettings::importChartSheetPr( const AttributeList& rAttribs )
{
    maSheetSettings.maCodeName = rAttribs.getString( XML_codeName, OUString() );
}

void WorksheetSettings::importTabColor( const AttributeList& rAttribs )
{
    maSheetSettings.maTabColor.importColor( rAttribs );
}

void WorksheetSettings::importOutlinePr( const AttributeList& rAttribs )
{
    maSheetSettings.mbApplyStyles  = rAttribs.getBool( XML_applyStyles, false );
    maSheetSettings.mbSummaryBelow = rAttribs.getBool( XML_summaryBelow, true );
    maSheetSettings.mbSummaryRight = rAttribs.getBool( XML_summaryRight, true );
}

void WorksheetSettings::importSheetProtection( const AttributeList& rAttribs )
{
    maSheetProt.mnPasswordHash     = CodecHelper::getPasswordHash( rAttribs, XML_password );
    maSheetProt.mbSheet            = rAttribs.getBool( XML_sheet, false );
    maSheetProt.mbObjects          = rAttribs.getBool( XML_objects, false );
    maSheetProt.mbScenarios        = rAttribs.getBool( XML_scenarios, false );
    maSheetProt.mbFormatCells      = rAttribs.getBool( XML_formatCells, true );
    maSheetProt.mbFormatColumns    = rAttribs.getBool( XML_formatColumns, true );
    maSheetProt.mbFormatRows       = rAttribs.getBool( XML_formatRows, true );
    maSheetProt.mbInsertColumns    = rAttribs.getBool( XML_insertColumns, true );
    maSheetProt.mbInsertRows       = rAttribs.getBool( XML_insertRows, true );
    maSheetProt.mbInsertHyperlinks = rAttribs.getBool( XML_insertHyperlinks, true );
    maSheetProt.mbDeleteColumns    = rAttribs.getBool( XML_deleteColumns, true );
    maSheetProt.mbDeleteRows       = rAttribs.getBool( XML_deleteRows, true );
    maSheetProt.mbSelectLocked     = rAttribs.getBool( XML_selectLockedCells, false );
    maSheetProt.mbSort             = rAttribs.getBool( XML_sort, true );
    maSheetProt.mbAutoFilter       = rAttribs.getBool( XML_autoFilter, true );
    maSheetProt.mbPivotTables      = rAttribs.getBool( XML_pivotTables, true );
    maSheetProt.mbSelectUnlocked   = rAttribs.getBool( XML_selectUnlockedCells, false );
}

void WorksheetSettings::importChartProtection( const AttributeList& rAttribs )
{
    maSheetProt.mnPasswordHash = CodecHelper::getPasswordHash( rAttribs, XML_password );
    maSheetProt.mbSheet        = rAttribs.getBool( XML_content, false );
    maSheetProt.mbObjects      = rAttribs.getBool( XML_objects, false );
}

void WorksheetSettings::importPhoneticPr( const AttributeList& rAttribs )
{
    maPhoneticSett.importPhoneticPr( rAttribs );
}

void WorksheetSettings::importSheetPr( RecordInputStream& rStrm )
{
    sal_uInt16 nFlags1;
    sal_uInt8 nFlags2;
    rStrm >> nFlags1 >> nFlags2 >> maSheetSettings.maTabColor;
    rStrm.skip( 8 );    // sync anchor cell
    rStrm >> maSheetSettings.maCodeName;
    // sheet settings
    maSheetSettings.mbFilterMode = getFlag( nFlags2, OOBIN_SHEETPR_FILTERMODE );
    // outline settings, equal flags in BIFF and OOBIN
    maSheetSettings.mbApplyStyles  = getFlag( nFlags1, BIFF_SHEETPR_APPLYSTYLES );
    maSheetSettings.mbSummaryRight = getFlag( nFlags1, BIFF_SHEETPR_SYMBOLSRIGHT );
    maSheetSettings.mbSummaryBelow = getFlag( nFlags1, BIFF_SHEETPR_SYMBOLSBELOW );
    /*  Fit printout to width/height - for whatever reason, this flag is still
        stored separated from the page settings */
    getPageSettings().setFitToPagesMode( getFlag( nFlags1, BIFF_SHEETPR_FITTOPAGES ) );
}

void WorksheetSettings::importChartSheetPr( RecordInputStream& rStrm )
{
    rStrm.skip( 2 );    // flags, contains only the 'published' flag
    rStrm >> maSheetSettings.maTabColor >> maSheetSettings.maCodeName;
}

void WorksheetSettings::importSheetProtection( RecordInputStream& rStrm )
{
    rStrm >> maSheetProt.mnPasswordHash;
    // no flags field for all these boolean flags?!?
    maSheetProt.mbSheet            = rStrm.readInt32() != 0;
    maSheetProt.mbObjects          = rStrm.readInt32() != 0;
    maSheetProt.mbScenarios        = rStrm.readInt32() != 0;
    maSheetProt.mbFormatCells      = rStrm.readInt32() != 0;
    maSheetProt.mbFormatColumns    = rStrm.readInt32() != 0;
    maSheetProt.mbFormatRows       = rStrm.readInt32() != 0;
    maSheetProt.mbInsertColumns    = rStrm.readInt32() != 0;
    maSheetProt.mbInsertRows       = rStrm.readInt32() != 0;
    maSheetProt.mbInsertHyperlinks = rStrm.readInt32() != 0;
    maSheetProt.mbDeleteColumns    = rStrm.readInt32() != 0;
    maSheetProt.mbDeleteRows       = rStrm.readInt32() != 0;
    maSheetProt.mbSelectLocked     = rStrm.readInt32() != 0;
    maSheetProt.mbSort             = rStrm.readInt32() != 0;
    maSheetProt.mbAutoFilter       = rStrm.readInt32() != 0;
    maSheetProt.mbPivotTables      = rStrm.readInt32() != 0;
    maSheetProt.mbSelectUnlocked   = rStrm.readInt32() != 0;
}

void WorksheetSettings::importChartProtection( RecordInputStream& rStrm )
{
    rStrm >> maSheetProt.mnPasswordHash;
    // no flags field for all these boolean flags?!?
    maSheetProt.mbSheet            = rStrm.readInt32() != 0;
    maSheetProt.mbObjects          = rStrm.readInt32() != 0;
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
    maSheetSettings.mbApplyStyles  = getFlag( nFlags, BIFF_SHEETPR_APPLYSTYLES );
    maSheetSettings.mbSummaryRight = getFlag( nFlags, BIFF_SHEETPR_SYMBOLSRIGHT );
    maSheetSettings.mbSummaryBelow = getFlag( nFlags, BIFF_SHEETPR_SYMBOLSBELOW );
    // fit printout to width/height
    getPageSettings().setFitToPagesMode( getFlag( nFlags, BIFF_SHEETPR_FITTOPAGES ) );
    // save external linked values, in BIFF5-BIFF8 moved to BOOKBOOK record
    if( getBiff() <= BIFF4 )
        getWorkbookSettings().setSaveExtLinkValues( !getFlag( nFlags, BIFF_SHEETPR_SKIPEXT ) );
}

void WorksheetSettings::importProtect( BiffInputStream& rStrm )
{
    maSheetProt.mbSheet = rStrm.readuInt16() != 0;
}

void WorksheetSettings::importObjectProtect( BiffInputStream& rStrm )
{
    maSheetProt.mbObjects = rStrm.readuInt16() != 0;
}

void WorksheetSettings::importScenProtect( BiffInputStream& rStrm )
{
    maSheetProt.mbScenarios = rStrm.readuInt16() != 0;
}

void WorksheetSettings::importPassword( BiffInputStream& rStrm )
{
    rStrm >> maSheetProt.mnPasswordHash;
}

void WorksheetSettings::importSheetProtection( BiffInputStream& rStrm )
{
    sal_uInt32 nFlags = rStrm.readuInt32();
    // set flag means protection is disabled
    maSheetProt.mbObjects          = !getFlag( nFlags, BIFF_SHEETPROT_OBJECTS );
    maSheetProt.mbScenarios        = !getFlag( nFlags, BIFF_SHEETPROT_SCENARIOS );
    maSheetProt.mbFormatCells      = !getFlag( nFlags, BIFF_SHEETPROT_FORMAT_CELLS );
    maSheetProt.mbFormatColumns    = !getFlag( nFlags, BIFF_SHEETPROT_FORMAT_COLUMNS );
    maSheetProt.mbFormatRows       = !getFlag( nFlags, BIFF_SHEETPROT_FORMAT_ROWS );
    maSheetProt.mbInsertColumns    = !getFlag( nFlags, BIFF_SHEETPROT_INSERT_COLUMNS );
    maSheetProt.mbInsertRows       = !getFlag( nFlags, BIFF_SHEETPROT_INSERT_ROWS );
    maSheetProt.mbInsertHyperlinks = !getFlag( nFlags, BIFF_SHEETPROT_INSERT_HLINKS );
    maSheetProt.mbDeleteColumns    = !getFlag( nFlags, BIFF_SHEETPROT_DELETE_COLUMNS );
    maSheetProt.mbDeleteRows       = !getFlag( nFlags, BIFF_SHEETPROT_DELETE_ROWS );
    maSheetProt.mbSelectLocked     = !getFlag( nFlags, BIFF_SHEETPROT_SELECT_LOCKED );
    maSheetProt.mbSort             = !getFlag( nFlags, BIFF_SHEETPROT_SORT );
    maSheetProt.mbAutoFilter       = !getFlag( nFlags, BIFF_SHEETPROT_AUTOFILTER );
    maSheetProt.mbPivotTables      = !getFlag( nFlags, BIFF_SHEETPROT_PIVOTTABLES );
    maSheetProt.mbSelectUnlocked   = !getFlag( nFlags, BIFF_SHEETPROT_SELECT_UNLOCKED );
}

void WorksheetSettings::importCodeName( BiffInputStream& rStrm )
{
    maSheetSettings.maCodeName = rStrm.readUniString();
}

void WorksheetSettings::importPhoneticPr( BiffInputStream& rStrm )
{
    maPhoneticSett.importPhoneticPr( rStrm );
}

void WorksheetSettings::finalizeImport()
{
    // sheet protection
    if( maSheetProt.mbSheet ) try
    {
        Reference< XProtectable > xProtectable( getSheet(), UNO_QUERY_THROW );
        xProtectable->protect( OUString() );
    }
    catch( Exception& )
    {
    }

    // VBA code name
    PropertySet aPropSet( getSheet() );
    aPropSet.setProperty( PROP_CodeName, maSheetSettings.maCodeName );

    if (!maSheetSettings.maTabColor.isAuto())
    {
        sal_Int32 nColor = maSheetSettings.maTabColor.getColor(getBaseFilter().getGraphicHelper());
        aPropSet.setProperty(PROP_TabColor, nColor);
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
