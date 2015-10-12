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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_SHEETDATACONTEXT_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_SHEETDATACONTEXT_HXX

#include "excelhandlers.hxx"
#include "richstring.hxx"
#include "sheetdatabuffer.hxx"
#include <vcl/svapp.hxx>

#define MULTI_THREAD_SHEET_PARSING 1

namespace oox {
namespace xls {

/** Used as base for sheet data context classes. Provides fast access to often
    used converter objects and sheet index, to improve performance.
 */
struct SheetDataContextBase
{
    AddressConverter&   mrAddressConv;      /// The address converter.
    std::unique_ptr<FormulaParser> mxFormulaParser;    /// The formula parser, different one for each SheetDataContext
    SheetDataBuffer&    mrSheetData;        /// The sheet data buffer for cell content and formatting.
    CellModel           maCellData;         /// Position, contents, formatting of current imported cell.
    CellFormulaModel    maFmlaData;         /// Settings for a cell formula.
    sal_Int16           mnSheet;            /// Index of the current sheet.

    explicit            SheetDataContextBase( const WorksheetHelper& rHelper );
    virtual             ~SheetDataContextBase();
};

/** This class implements importing the sheetData element.

    The sheetData element contains all row settings and all cells in a single
    sheet of a spreadsheet document.
 */
class SheetDataContext : public WorksheetContextBase, private SheetDataContextBase
{
    // If we are doing threaded parsing, this SheetDataContext
    // forms the inner loop for bulk data parsing, and for the
    // duration of this we can drop the solar mutex.
#if MULTI_THREAD_SHEET_PARSING
    SolarMutexReleaser aReleaser;
#endif

public:
    explicit            SheetDataContext( WorksheetFragmentBase& rFragment );
    virtual            ~SheetDataContext();

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
    virtual void        onEndElement() override;

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;

private:
    /** Different types of cell records. */
    enum CellType { CELLTYPE_VALUE, CELLTYPE_MULTI, CELLTYPE_FORMULA };

    /** Imports row settings from a row element. */
    void                importRow( const AttributeList& rAttribs );
    /** Imports cell settings from a c element. */
    bool                importCell( const AttributeList& rAttribs );
    /** Imports cell settings from an f element. */
    void                importFormula( const AttributeList& rAttribs );

    /** Imports row settings from a ROW record. */
    void                importRow( SequenceInputStream& rStrm );

    /** Reads a cell address and the following XF identifier. */
    bool                readCellHeader( SequenceInputStream& rStrm, CellType eCellType );
    /** Reads a cell formula for the current cell. */
    ApiTokenSequence    readCellFormula( SequenceInputStream& rStrm );
    /** Reads the formula range used by shared formulas, arrays, and data tables. */
    bool                readFormulaRef( SequenceInputStream& rStrm );

    /** Imports an empty cell from a CELL_BLANK or MULTCELL_BLANK record. */
    void                importCellBlank( SequenceInputStream& rStrm, CellType eCellType );
    /** Imports a boolean cell from a CELL_BOOL, MULTCELL_BOOL, or FORMULA_BOOL record. */
    void                importCellBool( SequenceInputStream& rStrm, CellType eCellType );
    /** Imports a numeric cell from a CELL_DOUBLE, MULTCELL_DOUBLE, or FORMULA_DOUBLE record. */
    void                importCellDouble( SequenceInputStream& rStrm, CellType eCellType );
    /** Imports an error code cell from a CELL_ERROR, MULTCELL_ERROR, or FORMULA_ERROR record. */
    void                importCellError( SequenceInputStream& rStrm, CellType eCellType );
    /** Imports an encoded numeric cell from a CELL_RK or MULTCELL_RK record. */
    void                importCellRk( SequenceInputStream& rStrm, CellType eCellType );
    /** Imports a rich-string cell from a CELL_RSTRING or MULTCELL_RSTRING record. */
    void                importCellRString( SequenceInputStream& rStrm, CellType eCellType );
    /** Imports a string cell from a CELL_SI or MULTCELL_SI record. */
    void                importCellSi( SequenceInputStream& rStrm, CellType eCellType );
    /** Imports a string cell from a CELL_STRING, MULTCELL_STRING, or FORMULA_STRING record. */
    void                importCellString( SequenceInputStream& rStrm, CellType eCellType );

    /** Imports an array formula from an ARRAY record. */
    void                importArray( SequenceInputStream& rStrm );
    /** Imports table operation from a DATATABLE record. */
    void                importDataTable( SequenceInputStream& rStrm );
    /** Imports a shared formula from a SHAREDFORMULA record. */
    void                importSharedFmla( SequenceInputStream& rStrm );

private:
    OUString     maCellValue;        /// Cell value string (OOXML only).
    RichStringRef       mxInlineStr;        /// Inline rich string (OOXML only).
    OUString     maFormulaStr;
    DataTableModel      maTableData;        /// Settings for table operations.
    BinAddress          maCurrPos;          /// Current cell position (BIFF12 only).
    bool                mbHasFormula;       /// True = current cell has formula data (OOXML only).
    bool                mbValidRange;       /// True = maFmlaData.maFormulaRef is valid (OOXML only).

    sal_Int32 mnRow; /// row index (0-based)
    sal_Int32 mnCol; /// column index (0-based)
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
