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

#include "sheetdatacontext.hxx"

#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/XText.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "addressconverter.hxx"
#include "biffinputstream.hxx"
#include "formulaparser.hxx"
#include "richstringcontext.hxx"
#include "unitconverter.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;

using ::oox::core::ContextHandlerRef;

// ============================================================================

namespace {

// record constants -----------------------------------------------------------

const sal_uInt32 BIFF12_CELL_SHOWPHONETIC   = 0x01000000;

const sal_uInt8 BIFF12_DATATABLE_ROW        = 0x01;
const sal_uInt8 BIFF12_DATATABLE_2D         = 0x02;
const sal_uInt8 BIFF12_DATATABLE_REF1DEL    = 0x04;
const sal_uInt8 BIFF12_DATATABLE_REF2DEL    = 0x08;

const sal_uInt16 BIFF12_ROW_THICKTOP        = 0x0001;
const sal_uInt16 BIFF12_ROW_THICKBOTTOM     = 0x0002;
const sal_uInt16 BIFF12_ROW_COLLAPSED       = 0x0800;
const sal_uInt16 BIFF12_ROW_HIDDEN          = 0x1000;
const sal_uInt16 BIFF12_ROW_CUSTOMHEIGHT    = 0x2000;
const sal_uInt16 BIFF12_ROW_CUSTOMFORMAT    = 0x4000;
const sal_uInt8 BIFF12_ROW_SHOWPHONETIC     = 0x01;

const sal_uInt16 BIFF_DATATABLE_ROW         = 0x0004;
const sal_uInt16 BIFF_DATATABLE_2D          = 0x0008;
const sal_uInt16 BIFF_DATATABLE_REF1DEL     = 0x0010;
const sal_uInt16 BIFF_DATATABLE_REF2DEL     = 0x0020;

const sal_uInt8 BIFF_FORMULA_RES_STRING     = 0;        /// Result is a string.
const sal_uInt8 BIFF_FORMULA_RES_BOOL       = 1;        /// Result is Boolean value.
const sal_uInt8 BIFF_FORMULA_RES_ERROR      = 2;        /// Result is error code.
const sal_uInt8 BIFF_FORMULA_RES_EMPTY      = 3;        /// Result is empty cell (BIFF8 only).
const sal_uInt16 BIFF_FORMULA_SHARED        = 0x0008;   /// Shared formula cell.

const sal_uInt8 BIFF2_ROW_CUSTOMFORMAT      = 0x01;
const sal_uInt16 BIFF_ROW_DEFAULTHEIGHT     = 0x8000;
const sal_uInt16 BIFF_ROW_HEIGHTMASK        = 0x7FFF;
const sal_uInt32 BIFF_ROW_COLLAPSED         = 0x00000010;
const sal_uInt32 BIFF_ROW_HIDDEN            = 0x00000020;
const sal_uInt32 BIFF_ROW_CUSTOMHEIGHT      = 0x00000040;
const sal_uInt32 BIFF_ROW_CUSTOMFORMAT      = 0x00000080;
const sal_uInt32 BIFF_ROW_THICKTOP          = 0x10000000;
const sal_uInt32 BIFF_ROW_THICKBOTTOM       = 0x20000000;
const sal_uInt32 BIFF_ROW_SHOWPHONETIC      = 0x40000000;

const sal_Int32 BIFF2_CELL_USEIXFE          = 63;

} // namespace

// ============================================================================

SheetDataContextBase::SheetDataContextBase( const WorksheetHelper& rHelper ) :
    mrAddressConv( rHelper.getAddressConverter() ),
    mrFormulaParser( rHelper.getFormulaParser() ),
    mrSheetData( rHelper.getSheetData() ),
    mnSheet( rHelper.getSheetIndex() )
{
}

SheetDataContextBase::~SheetDataContextBase()
{
}

// ============================================================================

SheetDataContext::SheetDataContext( WorksheetFragmentBase& rFragment ) :
    WorksheetContextBase( rFragment ),
    SheetDataContextBase( rFragment ),
    mbHasFormula( false ),
    mbValidRange( false ),
    mnRow( -1 ),
    mnCol( -1 )
{
}

ContextHandlerRef SheetDataContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( sheetData ):
            if( nElement == XLS_TOKEN( row ) ) { importRow( rAttribs ); return this; }
        break;

        case XLS_TOKEN( row ):
            // do not process cell elements with invalid (out-of-range) address
            if( nElement == XLS_TOKEN( c ) && importCell( rAttribs ) )
                return this;
        break;

        case XLS_TOKEN( c ):
            switch( nElement )
            {
                case XLS_TOKEN( is ):
                    mxInlineStr.reset( new RichString( *this ) );
                    return new RichStringContext( *this, mxInlineStr );
                case XLS_TOKEN( v ):
                    return this;    // characters contain cell value
                case XLS_TOKEN( f ):
                    importFormula( rAttribs );
                    return this;    // characters contain formula string
            }
        break;
    }
    return 0;
}

void SheetDataContext::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( v ):
            maCellValue = rChars;
        break;
        case XLS_TOKEN( f ):
            if( maFmlaData.mnFormulaType != XML_TOKEN_INVALID )
            {
                maFormulaStr = rChars;
            }
        break;
    }
}

void SheetDataContext::onEndElement()
{
    if( getCurrentElement() == XLS_TOKEN( c ) )
    {
        // try to create a formula cell
        if( mbHasFormula ) switch( maFmlaData.mnFormulaType )
        {
            // will buffer formulas but need to
            // a) need to set format first
            // :/
            case XML_normal:
                setCellFormula( maCellData.maCellAddr, maFormulaStr );
                mrSheetData.setCellFormat( maCellData );

                // If a number cell has some preloaded value, stick it into the buffer
                // but do this only for real cell formulas (not array, shared etc.)
                if( !( maCellValue.isEmpty() ) && ( maCellData.mnCellType == XML_n ) )
                    setCellFormulaValue( maCellData.maCellAddr, maCellValue.toDouble() );
                break;

            case XML_shared:
                if( maFmlaData.mnSharedId >= 0 )
                {
                    if( mbValidRange && maFmlaData.isValidSharedRef( maCellData.maCellAddr ) )
                        createSharedFormulaMapEntry(maCellData.maCellAddr, maFmlaData.maFormulaRef, maFmlaData.mnSharedId, maFormulaStr);

                    setCellFormula(maCellData.maCellAddr, maFmlaData.mnSharedId, maCellValue, maCellData.mnCellType);
                    mrSheetData.setCellFormat( maCellData );
                }
                else
                    // no success, set plain cell value and formatting below
                    mbHasFormula = false;
            break;
            case XML_array:
                if( mbValidRange && maFmlaData.isValidArrayRef( maCellData.maCellAddr ) )
                    setCellArrayFormula( maFmlaData.maFormulaRef, maCellData.maCellAddr, maFormulaStr );
                // set cell formatting, but do not set result as cell value
                mrSheetData.setBlankCell( maCellData );
            break;
            case XML_dataTable:
                if( mbValidRange )
                    mrSheetData.createTableOperation( maFmlaData.maFormulaRef, maTableData );
                // set cell formatting, but do not set result as cell value
                mrSheetData.setBlankCell( maCellData );
            break;
            default:
                OSL_ENSURE( maFmlaData.mnFormulaType == XML_TOKEN_INVALID, "SheetDataContext::onEndElement - unknown formula type" );
                mbHasFormula = false;
        }

        if( !mbHasFormula )
        {
            // no formula created: try to set the cell value
            if( !maCellValue.isEmpty() ) switch( maCellData.mnCellType )
            {
                case XML_n:
                    mrSheetData.setValueCell( maCellData, maCellValue.toDouble() );
                break;
                case XML_b:
                    mrSheetData.setBooleanCell( maCellData, maCellValue.toDouble() != 0.0 );
                break;
                case XML_e:
                    mrSheetData.setErrorCell( maCellData, maCellValue );
                break;
                case XML_str:
                    mrSheetData.setStringCell( maCellData, maCellValue );
                break;
                case XML_s:
                    mrSheetData.setStringCell( maCellData, maCellValue.toInt32() );
                break;
            }
            else if( (maCellData.mnCellType == XML_inlineStr) && mxInlineStr.get() )
            {
                mxInlineStr->finalizeImport();
                mrSheetData.setStringCell( maCellData, mxInlineStr );
            }
            else
            {
                // empty cell, update cell type
                maCellData.mnCellType = XML_TOKEN_INVALID;
                mrSheetData.setBlankCell( maCellData );
            }
        }
    }
}

ContextHandlerRef SheetDataContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case BIFF12_ID_SHEETDATA:
            if( nRecId == BIFF12_ID_ROW ) { importRow( rStrm ); return this; }
        break;

        case BIFF12_ID_ROW:
            switch( nRecId )
            {
                case BIFF12_ID_ARRAY:           importArray( rStrm );                           break;
                case BIFF12_ID_CELL_BOOL:       importCellBool( rStrm, CELLTYPE_VALUE );        break;
                case BIFF12_ID_CELL_BLANK:      importCellBlank( rStrm, CELLTYPE_VALUE );       break;
                case BIFF12_ID_CELL_DOUBLE:     importCellDouble( rStrm, CELLTYPE_VALUE );      break;
                case BIFF12_ID_CELL_ERROR:      importCellError( rStrm, CELLTYPE_VALUE );       break;
                case BIFF12_ID_CELL_RK:         importCellRk( rStrm, CELLTYPE_VALUE );          break;
                case BIFF12_ID_CELL_RSTRING:    importCellRString( rStrm, CELLTYPE_VALUE );     break;
                case BIFF12_ID_CELL_SI:         importCellSi( rStrm, CELLTYPE_VALUE );          break;
                case BIFF12_ID_CELL_STRING:     importCellString( rStrm, CELLTYPE_VALUE );      break;
                case BIFF12_ID_DATATABLE:       importDataTable( rStrm );                       break;
                case BIFF12_ID_FORMULA_BOOL:    importCellBool( rStrm, CELLTYPE_FORMULA );      break;
                case BIFF12_ID_FORMULA_DOUBLE:  importCellDouble( rStrm, CELLTYPE_FORMULA );    break;
                case BIFF12_ID_FORMULA_ERROR:   importCellError( rStrm, CELLTYPE_FORMULA );     break;
                case BIFF12_ID_FORMULA_STRING:  importCellString( rStrm, CELLTYPE_FORMULA );    break;
                case BIFF12_ID_MULTCELL_BOOL:   importCellBool( rStrm, CELLTYPE_MULTI );        break;
                case BIFF12_ID_MULTCELL_BLANK:  importCellBlank( rStrm, CELLTYPE_MULTI );       break;
                case BIFF12_ID_MULTCELL_DOUBLE: importCellDouble( rStrm, CELLTYPE_MULTI );      break;
                case BIFF12_ID_MULTCELL_ERROR:  importCellError( rStrm, CELLTYPE_MULTI );       break;
                case BIFF12_ID_MULTCELL_RK:     importCellRk( rStrm, CELLTYPE_MULTI );          break;
                case BIFF12_ID_MULTCELL_RSTRING:importCellRString( rStrm, CELLTYPE_MULTI );     break;
                case BIFF12_ID_MULTCELL_SI:     importCellSi( rStrm, CELLTYPE_MULTI );          break;
                case BIFF12_ID_MULTCELL_STRING: importCellString( rStrm, CELLTYPE_MULTI );      break;
                case BIFF12_ID_SHAREDFMLA:      importSharedFmla( rStrm );                      break;
            }
        break;
    }
    return 0;
}

// private --------------------------------------------------------------------

void SheetDataContext::importRow( const AttributeList& rAttribs )
{
    RowModel aModel;
    sal_Int32 nRow = rAttribs.getInteger( XML_r, -1 );
    if(nRow != -1)
        aModel.mnRow          = nRow;
    else
        aModel.mnRow = ++mnRow;
    mnCol = -1;

    aModel.mfHeight       = rAttribs.getDouble( XML_ht, -1.0 );
    aModel.mnXfId         = rAttribs.getInteger( XML_s, -1 );
    aModel.mnLevel        = rAttribs.getInteger( XML_outlineLevel, 0 );
    aModel.mbCustomHeight = rAttribs.getBool( XML_customHeight, false );
    aModel.mbCustomFormat = rAttribs.getBool( XML_customFormat, false );
    aModel.mbShowPhonetic = rAttribs.getBool( XML_ph, false );
    aModel.mbHidden       = rAttribs.getBool( XML_hidden, false );
    aModel.mbCollapsed    = rAttribs.getBool( XML_collapsed, false );
    aModel.mbThickTop     = rAttribs.getBool( XML_thickTop, false );
    aModel.mbThickBottom  = rAttribs.getBool( XML_thickBot, false );

    // decode the column spans (space-separated list of colon-separated integer pairs)
    OUString aColSpansText = rAttribs.getString( XML_spans, OUString() );
    sal_Int32 nMaxCol = mrAddressConv.getMaxApiAddress().Column;
    sal_Int32 nIndex = 0;
    while( nIndex >= 0 )
    {
        OUString aColSpanToken = aColSpansText.getToken( 0, ' ', nIndex );
        sal_Int32 nSepPos = aColSpanToken.indexOf( ':' );
        if( (0 < nSepPos) && (nSepPos + 1 < aColSpanToken.getLength()) )
        {
            // OOXML uses 1-based integer column indexes, row model expects 0-based colspans
            sal_Int32 nLastCol = ::std::min( aColSpanToken.copy( nSepPos + 1 ).toInt32() - 1, nMaxCol );
            aModel.insertColSpan( ValueRange( aColSpanToken.copy( 0, nSepPos ).toInt32() - 1, nLastCol ) );
        }
    }

    // set row properties in the current sheet
    setRowModel( aModel );
}

bool SheetDataContext::importCell( const AttributeList& rAttribs )
{
    OUString aCellAddrStr =  rAttribs.getString( XML_r, OUString() );
    bool bValid = true;
    if(aCellAddrStr.isEmpty())
    {
        ++mnCol;
        maCellData.maCellAddr = CellAddress( mnSheet, mnCol, mnRow );
    }
    else
    {
        bValid = mrAddressConv.convertToCellAddress( maCellData.maCellAddr, aCellAddrStr, mnSheet, true );

        mnCol = maCellData.maCellAddr.Column;
    }

    if( bValid )
    {
        maCellData.mnCellType     = rAttribs.getToken( XML_t, XML_n );
        maCellData.mnXfId         = rAttribs.getInteger( XML_s, -1 );
        maCellData.mbShowPhonetic = rAttribs.getBool( XML_ph, false );

        // reset cell value, formula settings, and inline string
        maCellValue = OUString();
        mxInlineStr.reset();
        mbHasFormula = false;

        // update used area of the sheet
        extendUsedArea( maCellData.maCellAddr );
    }
    return bValid;
}

void SheetDataContext::importFormula( const AttributeList& rAttribs )
{
    mbHasFormula = true;
    mbValidRange = mrAddressConv.convertToCellRange( maFmlaData.maFormulaRef, rAttribs.getString( XML_ref, OUString() ), mnSheet, true, true );

    maFmlaData.mnFormulaType = rAttribs.getToken( XML_t, XML_normal );
    maFmlaData.mnSharedId    = rAttribs.getInteger( XML_si, -1 );

    if( maFmlaData.mnFormulaType == XML_dataTable )
    {
        maTableData.maRef1        = rAttribs.getString( XML_r1, OUString() );
        maTableData.maRef2        = rAttribs.getString( XML_r2, OUString() );
        maTableData.mb2dTable     = rAttribs.getBool( XML_dt2D, false );
        maTableData.mbRowTable    = rAttribs.getBool( XML_dtr, false );
        maTableData.mbRef1Deleted = rAttribs.getBool( XML_del1, false );
        maTableData.mbRef2Deleted = rAttribs.getBool( XML_del2, false );
    }

    maFormulaStr = OUString();
}

void SheetDataContext::importRow( SequenceInputStream& rStrm )
{
    RowModel aModel;
    sal_Int32 nSpanCount;
    sal_uInt16 nHeight, nFlags1;
    sal_uInt8 nFlags2;
    rStrm >> maCurrPos.mnRow >> aModel.mnXfId >> nHeight >> nFlags1 >> nFlags2 >> nSpanCount;
    maCurrPos.mnCol = 0;

    // row index is 0-based in BIFF12, but RowModel expects 1-based
    aModel.mnRow          = maCurrPos.mnRow + 1;
    // row height is in twips in BIFF12, convert to points
    aModel.mfHeight       = nHeight / 20.0;
    aModel.mnLevel        = extractValue< sal_Int32 >( nFlags1, 8, 3 );
    aModel.mbCustomHeight = getFlag( nFlags1, BIFF12_ROW_CUSTOMHEIGHT );
    aModel.mbCustomFormat = getFlag( nFlags1, BIFF12_ROW_CUSTOMFORMAT );
    aModel.mbShowPhonetic = getFlag( nFlags2, BIFF12_ROW_SHOWPHONETIC );
    aModel.mbHidden       = getFlag( nFlags1, BIFF12_ROW_HIDDEN );
    aModel.mbCollapsed    = getFlag( nFlags1, BIFF12_ROW_COLLAPSED );
    aModel.mbThickTop     = getFlag( nFlags1, BIFF12_ROW_THICKTOP );
    aModel.mbThickBottom  = getFlag( nFlags1, BIFF12_ROW_THICKBOTTOM );

    // read the column spans
    sal_Int32 nMaxCol = mrAddressConv.getMaxApiAddress().Column;
    for( sal_Int32 nSpanIdx = 0; (nSpanIdx < nSpanCount) && !rStrm.isEof(); ++nSpanIdx )
    {
        sal_Int32 nFirstCol, nLastCol;
        rStrm >> nFirstCol >> nLastCol;
        aModel.insertColSpan( ValueRange( nFirstCol, ::std::min( nLastCol, nMaxCol ) ) );
    }

    // set row properties in the current sheet
    setRowModel( aModel );
}

bool SheetDataContext::readCellHeader( SequenceInputStream& rStrm, CellType eCellType )
{
    switch( eCellType )
    {
        case CELLTYPE_VALUE:
        case CELLTYPE_FORMULA:  rStrm >> maCurrPos.mnCol;   break;
        case CELLTYPE_MULTI:    ++maCurrPos.mnCol;          break;
    }

    sal_uInt32 nXfId;
    rStrm >> nXfId;

    bool bValidAddr = mrAddressConv.convertToCellAddress( maCellData.maCellAddr, maCurrPos, mnSheet, true );
    maCellData.mnXfId = extractValue< sal_Int32 >( nXfId, 0, 24 );
    maCellData.mbShowPhonetic = getFlag( nXfId, BIFF12_CELL_SHOWPHONETIC );

    // update used area of the sheet
    if( bValidAddr )
        extendUsedArea( maCellData.maCellAddr );
    return bValidAddr;
}

ApiTokenSequence SheetDataContext::readCellFormula( SequenceInputStream& rStrm )
{
    rStrm.skip( 2 );
    return mrFormulaParser.importFormula( maCellData.maCellAddr, FORMULATYPE_CELL, rStrm );
}

bool SheetDataContext::readFormulaRef( SequenceInputStream& rStrm )
{
    BinRange aRange;
    rStrm >> aRange;
    return mrAddressConv.convertToCellRange( maFmlaData.maFormulaRef, aRange, mnSheet, true, true );
}

void SheetDataContext::importCellBool( SequenceInputStream& rStrm, CellType eCellType )
{
    if( readCellHeader( rStrm, eCellType ) )
    {
        maCellData.mnCellType = XML_b;
        bool bValue = rStrm.readuInt8() != 0;
        if( eCellType == CELLTYPE_FORMULA )
            mrSheetData.setFormulaCell( maCellData, readCellFormula( rStrm ) );
        else
            mrSheetData.setBooleanCell( maCellData, bValue );
    }
}

void SheetDataContext::importCellBlank( SequenceInputStream& rStrm, CellType eCellType )
{
    OSL_ENSURE( eCellType != CELLTYPE_FORMULA, "SheetDataContext::importCellBlank - no formula cells supported" );
    if( readCellHeader( rStrm, eCellType ) )
        mrSheetData.setBlankCell( maCellData );
}

void SheetDataContext::importCellDouble( SequenceInputStream& rStrm, CellType eCellType )
{
    if( readCellHeader( rStrm, eCellType ) )
    {
        maCellData.mnCellType = XML_n;
        double fValue = rStrm.readDouble();
        if( eCellType == CELLTYPE_FORMULA )
            mrSheetData.setFormulaCell( maCellData, readCellFormula( rStrm ) );
        else
            mrSheetData.setValueCell( maCellData, fValue );
    }
}

void SheetDataContext::importCellError( SequenceInputStream& rStrm, CellType eCellType )
{
    if( readCellHeader( rStrm, eCellType ) )
    {
        maCellData.mnCellType = XML_e;
        sal_uInt8 nErrorCode = rStrm.readuInt8();
        if( eCellType == CELLTYPE_FORMULA )
            mrSheetData.setFormulaCell( maCellData, readCellFormula( rStrm ) );
        else
            mrSheetData.setErrorCell( maCellData, nErrorCode );
    }
}

void SheetDataContext::importCellRk( SequenceInputStream& rStrm, CellType eCellType )
{
    OSL_ENSURE( eCellType != CELLTYPE_FORMULA, "SheetDataContext::importCellRk - no formula cells supported" );
    if( readCellHeader( rStrm, eCellType ) )
    {
        maCellData.mnCellType = XML_n;
        mrSheetData.setValueCell( maCellData, BiffHelper::calcDoubleFromRk( rStrm.readInt32() ) );
    }
}

void SheetDataContext::importCellRString( SequenceInputStream& rStrm, CellType eCellType )
{
    OSL_ENSURE( eCellType != CELLTYPE_FORMULA, "SheetDataContext::importCellRString - no formula cells supported" );
    if( readCellHeader( rStrm, eCellType ) )
    {
        maCellData.mnCellType = XML_inlineStr;
        RichStringRef xString( new RichString( *this ) );
        xString->importString( rStrm, true );
        xString->finalizeImport();
        mrSheetData.setStringCell( maCellData, xString );
    }
}

void SheetDataContext::importCellSi( SequenceInputStream& rStrm, CellType eCellType )
{
    OSL_ENSURE( eCellType != CELLTYPE_FORMULA, "SheetDataContext::importCellSi - no formula cells supported" );
    if( readCellHeader( rStrm, eCellType ) )
    {
        maCellData.mnCellType = XML_s;
        mrSheetData.setStringCell( maCellData, rStrm.readInt32() );
    }
}

void SheetDataContext::importCellString( SequenceInputStream& rStrm, CellType eCellType )
{
    if( readCellHeader( rStrm, eCellType ) )
    {
        maCellData.mnCellType = XML_inlineStr;
        // always import the string, stream will point to formula afterwards, if existing
        RichStringRef xString( new RichString( *this ) );
        xString->importString( rStrm, false );
        xString->finalizeImport();
        if( eCellType == CELLTYPE_FORMULA )
            mrSheetData.setFormulaCell( maCellData, readCellFormula( rStrm ) );
        else
            mrSheetData.setStringCell( maCellData, xString );
    }
}

void SheetDataContext::importArray( SequenceInputStream& rStrm )
{
    if( readFormulaRef( rStrm ) && maFmlaData.isValidArrayRef( maCellData.maCellAddr ) )
    {
        rStrm.skip( 1 );
        ApiTokenSequence aTokens = mrFormulaParser.importFormula( maCellData.maCellAddr, FORMULATYPE_ARRAY, rStrm );
        mrSheetData.createArrayFormula( maFmlaData.maFormulaRef, aTokens );
    }
}

void SheetDataContext::importDataTable( SequenceInputStream& rStrm )
{
    if( readFormulaRef( rStrm ) )
    {
        BinAddress aRef1, aRef2;
        sal_uInt8 nFlags;
        rStrm >> aRef1 >> aRef2 >> nFlags;
        maTableData.maRef1        = FormulaProcessorBase::generateAddress2dString( aRef1, false );
        maTableData.maRef2        = FormulaProcessorBase::generateAddress2dString( aRef2, false );
        maTableData.mbRowTable    = getFlag( nFlags, BIFF12_DATATABLE_ROW );
        maTableData.mb2dTable     = getFlag( nFlags, BIFF12_DATATABLE_2D );
        maTableData.mbRef1Deleted = getFlag( nFlags, BIFF12_DATATABLE_REF1DEL );
        maTableData.mbRef2Deleted = getFlag( nFlags, BIFF12_DATATABLE_REF2DEL );
        mrSheetData.createTableOperation( maFmlaData.maFormulaRef, maTableData );
    }
}

void SheetDataContext::importSharedFmla( SequenceInputStream& /*rStrm*/ )
{
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
