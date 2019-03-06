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

#include <memory>
#include <worksheetsettings.hxx>

#include <oox/core/binarycodec.hxx>
#include <oox/core/filterbase.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <pagesettings.hxx>
#include <tabprotection.hxx>
#include <document.hxx>
#include <addressconverter.hxx>
#include <biffhelper.hxx>

namespace oox {
namespace xls {

using namespace ::com::sun::star::uno;

namespace {

const sal_uInt8 BIFF12_SHEETPR_FILTERMODE       = 0x01;

const sal_uInt16 BIFF_SHEETPR_APPLYSTYLES       = 0x0020;
const sal_uInt16 BIFF_SHEETPR_SYMBOLSBELOW      = 0x0040;
const sal_uInt16 BIFF_SHEETPR_SYMBOLSRIGHT      = 0x0080;
const sal_uInt16 BIFF_SHEETPR_FITTOPAGES        = 0x0100;

} // namespace

SheetSettingsModel::SheetSettingsModel() :
    mbFilterMode( false ),
    mbApplyStyles( false ),
    mbSummaryBelow( true ),
    mbSummaryRight( true )
{
}

SheetProtectionModel::SheetProtectionModel() :
    mnSpinCount( 0 ),
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
    maSheetProt.maAlgorithmName    = rAttribs.getString( XML_algorithmName, OUString());
    maSheetProt.maHashValue        = rAttribs.getString( XML_hashValue, OUString());
    maSheetProt.maSaltValue        = rAttribs.getString( XML_saltValue, OUString());
    maSheetProt.mnSpinCount        = rAttribs.getUnsigned( XML_spinCount, 0);
    maSheetProt.mnPasswordHash     = oox::core::CodecHelper::getPasswordHash( rAttribs, XML_password );
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

void WorksheetSettings::importProtectedRange( const AttributeList& rAttribs )
{
    ScEnhancedProtection aProt;
    aProt.maTitle = rAttribs.getString( XML_name, OUString());
    /* XXX ECMA-376/OOXML XMLSchema and ISO/IEC 29500 say 'securityDescriptor'
     * would be an element, but Excel2013 stores it as attribute. */
    aProt.maSecurityDescriptorXML = rAttribs.getString( XML_securityDescriptor, OUString());
    /* XXX ECMA-376/OOXML or ISO/IEC 29500 do not even mention a 'password'
     * attribute here (or anywhere else), but this is what Excel2013 writes,
     * similar to BIFF, if the original file was a BIFF file. OOXML XMLschema
     * and ISO/IEC 29500 instead define 'algorithmName', 'hashValue',
     * 'saltValue' and 'spinCount' that are written if the protection was newly
     * created. */
    aProt.mnPasswordVerifier = rAttribs.getIntegerHex( XML_password, 0);
    aProt.maPasswordHash.maAlgorithmName = rAttribs.getString( XML_algorithmName, OUString());
    aProt.maPasswordHash.maHashValue = rAttribs.getString( XML_hashValue, OUString());
    aProt.maPasswordHash.maSaltValue = rAttribs.getString( XML_saltValue, OUString());
    aProt.maPasswordHash.mnSpinCount = rAttribs.getUnsigned( XML_spinCount, 0);
    OUString aRefs( rAttribs.getString( XML_sqref, OUString()));
    if (!aRefs.isEmpty())
    {
        std::unique_ptr<ScRangeList> xRangeList(new ScRangeList());
        getAddressConverter().convertToCellRangeList( *xRangeList, aRefs, getSheetIndex(), true );
        if (!xRangeList->empty())
        {
            aProt.maRangeList = xRangeList.release();
        }
    }
    maSheetProt.maEnhancedProtections.push_back( aProt);
}

void WorksheetSettings::importChartProtection( const AttributeList& rAttribs )
{
    maSheetProt.mnPasswordHash = oox::core::CodecHelper::getPasswordHash( rAttribs, XML_password );
    maSheetProt.mbSheet        = rAttribs.getBool( XML_content, false );
    maSheetProt.mbObjects      = rAttribs.getBool( XML_objects, false );
}

void WorksheetSettings::importPhoneticPr( const AttributeList& rAttribs )
{
    maPhoneticSett.importPhoneticPr( rAttribs );
}

void WorksheetSettings::importSheetPr( SequenceInputStream& rStrm )
{
    sal_uInt16 nFlags1;
    sal_uInt8 nFlags2;
    nFlags1 = rStrm.readuInt16();
    nFlags2 = rStrm.readuChar();
    rStrm >> maSheetSettings.maTabColor;
    rStrm.skip( 8 );    // sync anchor cell
    rStrm >> maSheetSettings.maCodeName;
    // sheet settings
    maSheetSettings.mbFilterMode = getFlag( nFlags2, BIFF12_SHEETPR_FILTERMODE );
    // outline settings, equal flags in all BIFFs
    maSheetSettings.mbApplyStyles  = getFlag( nFlags1, BIFF_SHEETPR_APPLYSTYLES );
    maSheetSettings.mbSummaryRight = getFlag( nFlags1, BIFF_SHEETPR_SYMBOLSRIGHT );
    maSheetSettings.mbSummaryBelow = getFlag( nFlags1, BIFF_SHEETPR_SYMBOLSBELOW );
    /*  Fit printout to width/height - for whatever reason, this flag is still
        stored separated from the page settings */
    getPageSettings().setFitToPagesMode( getFlag( nFlags1, BIFF_SHEETPR_FITTOPAGES ) );
}

void WorksheetSettings::importChartSheetPr( SequenceInputStream& rStrm )
{
    rStrm.skip( 2 );    // flags, contains only the 'published' flag
    rStrm >> maSheetSettings.maTabColor >> maSheetSettings.maCodeName;
}

void WorksheetSettings::importSheetProtection( SequenceInputStream& rStrm )
{
    maSheetProt.mnPasswordHash = rStrm.readuInt16();
    // no flags field for all these boolean flags?!?
    maSheetProt.mbSheet            = rStrm.readInt32() != 0;
    maSheetProt.mbObjects          = rStrm.readInt32() == 0;
    maSheetProt.mbScenarios        = rStrm.readInt32() == 0;
    maSheetProt.mbFormatCells      = rStrm.readInt32() == 0;
    maSheetProt.mbFormatColumns    = rStrm.readInt32() == 0;
    maSheetProt.mbFormatRows       = rStrm.readInt32() == 0;
    maSheetProt.mbInsertColumns    = rStrm.readInt32() == 0;
    maSheetProt.mbInsertRows       = rStrm.readInt32() == 0;
    maSheetProt.mbInsertHyperlinks = rStrm.readInt32() == 0;
    maSheetProt.mbDeleteColumns    = rStrm.readInt32() == 0;
    maSheetProt.mbDeleteRows       = rStrm.readInt32() == 0;
    maSheetProt.mbSelectLocked     = rStrm.readInt32() == 0;
    maSheetProt.mbSort             = rStrm.readInt32() == 0;
    maSheetProt.mbAutoFilter       = rStrm.readInt32() == 0;
    maSheetProt.mbPivotTables      = rStrm.readInt32() == 0;
    maSheetProt.mbSelectUnlocked   = rStrm.readInt32() == 0;
}

void WorksheetSettings::importChartProtection( SequenceInputStream& rStrm )
{
    maSheetProt.mnPasswordHash = rStrm.readuInt16();
    // no flags field for all these boolean flags?!?
    maSheetProt.mbSheet            = rStrm.readInt32() != 0;
    maSheetProt.mbObjects          = rStrm.readInt32() != 0;
}

void WorksheetSettings::importPhoneticPr( SequenceInputStream& rStrm )
{
    maPhoneticSett.importPhoneticPr( rStrm );
}

void WorksheetSettings::finalizeImport()
{
    // sheet protection
    if( maSheetProt.mbSheet )
    {
        ScTableProtection aProtect;
        aProtect.setProtected(true);
        aProtect.setPasswordHash( maSheetProt.maAlgorithmName, maSheetProt.maHashValue,
                maSheetProt.maSaltValue, maSheetProt.mnSpinCount);
        // Set the simple hash after the proper hash because setting the proper
        // hash resets the simple hash, yet if the simple hash is present we
        // may as well use it and more important want to keep it for saving the
        // document again.
        if (maSheetProt.mnPasswordHash)
        {
            Sequence<sal_Int8> aPass{
                sal_Int8(maSheetProt.mnPasswordHash >> 8),
                sal_Int8(maSheetProt.mnPasswordHash & 0xFF)};
            aProtect.setPasswordHash(aPass, PASSHASH_XL);
        }
        aProtect.setOption( ScTableProtection::OBJECTS, !maSheetProt.mbObjects);
        aProtect.setOption( ScTableProtection::SCENARIOS, !maSheetProt.mbScenarios );
        aProtect.setOption( ScTableProtection::FORMAT_CELLS, !maSheetProt.mbFormatCells );
        aProtect.setOption( ScTableProtection::FORMAT_COLUMNS, !maSheetProt.mbFormatColumns );
        aProtect.setOption( ScTableProtection::FORMAT_ROWS, !maSheetProt.mbFormatRows );
        aProtect.setOption( ScTableProtection::INSERT_COLUMNS, !maSheetProt.mbInsertColumns );
        aProtect.setOption( ScTableProtection::INSERT_ROWS,  !maSheetProt.mbInsertRows );
        aProtect.setOption( ScTableProtection::INSERT_HYPERLINKS, !maSheetProt.mbInsertHyperlinks );
        aProtect.setOption( ScTableProtection::DELETE_COLUMNS, !maSheetProt.mbDeleteColumns );
        aProtect.setOption( ScTableProtection::DELETE_ROWS,!maSheetProt.mbDeleteRows );
        aProtect.setOption( ScTableProtection::SELECT_LOCKED_CELLS, !maSheetProt.mbSelectLocked );
        aProtect.setOption( ScTableProtection::SORT, !maSheetProt.mbSort );
        aProtect.setOption( ScTableProtection::AUTOFILTER, !maSheetProt.mbAutoFilter );
        aProtect.setOption( ScTableProtection::PIVOT_TABLES, !maSheetProt.mbPivotTables );
        aProtect.setOption( ScTableProtection::SELECT_UNLOCKED_CELLS, !maSheetProt.mbSelectUnlocked );

        aProtect.setEnhancedProtection( maSheetProt.maEnhancedProtections);

        getScDocument().SetTabProtection( getSheetIndex(), &aProtect );
    }

    // VBA code name
    PropertySet aPropSet( getSheet() );
    aPropSet.setProperty( PROP_CodeName, maSheetSettings.maCodeName );

    // sheet tab color
    if( !maSheetSettings.maTabColor.isAuto() )
    {
        ::Color nColor = maSheetSettings.maTabColor.getColor( getBaseFilter().getGraphicHelper() );
        aPropSet.setProperty( PROP_TabColor, nColor );
    }
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
