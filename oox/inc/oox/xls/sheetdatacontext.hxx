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

#ifndef OOX_XLS_SHEETDATACONTEXT_HXX
#define OOX_XLS_SHEETDATACONTEXT_HXX

#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/richstring.hxx"
#include "oox/xls/sheetdatabuffer.hxx"

namespace oox {
namespace xls {

// ============================================================================

/** Used as base for sheet data context classes. Provides fast access to often
    used converter objects and sheet index, to improve performance.
 */
struct SheetDataContextBase
{
    AddressConverter&   mrAddressConv;      /// The address converter.
    FormulaParser&      mrFormulaParser;    /// The formula parser.
    SheetDataBuffer&    mrSheetData;        /// The sheet data buffer for cell content and formatting.
    CellModel           maCellData;         /// Position, contents, formatting of current imported cell.
    CellFormulaModel    maFmlaData;         /// Settings for a cell formula.
    sal_Int16           mnSheet;            /// Index of the current sheet.

    explicit            SheetDataContextBase( const WorksheetHelper& rHelper );
    virtual             ~SheetDataContextBase();
};

// ============================================================================

/** This class implements importing the sheetData element.

    The sheetData element contains all row settings and all cells in a single
    sheet of a spreadsheet document.
 */
class SheetDataContext : public WorksheetContextBase, private SheetDataContextBase
{
public:
    explicit            SheetDataContext( WorksheetFragmentBase& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );
    virtual void        onEndElement();

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm );

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
    ::rtl::OUString     maCellValue;        /// Cell value string (OOXML only).
    RichStringRef       mxInlineStr;        /// Inline rich string (OOXML only).
    ApiTokenSequence    maTokens;           /// Formula token array (OOXML only).
    DataTableModel      maTableData;        /// Settings for table operations.
    BinAddress          maCurrPos;          /// Current cell position (BIFF12 only).
    bool                mbHasFormula;       /// True = current cell has formula data (OOXML only).
    bool                mbValidRange;       /// True = maFmlaData.maFormulaRef is valid (OOXML only).
};

// ============================================================================

/** This class implements importing row settings and all cells of a sheet.
 */
class BiffSheetDataContext : public BiffWorksheetContextBase, private SheetDataContextBase
{
public:
    explicit            BiffSheetDataContext( const WorksheetHelper& rHelper );

    /** Tries to import a sheet data record. */
    virtual void        importRecord( BiffInputStream& rStrm );

private:
    /** Imports row settings from a ROW record. */
    void                importRow( BiffInputStream& rStrm );

    /** Reads an XF identifier and initializes a new cell. */
    bool                readCellXfId( BiffInputStream& rStrm, const BinAddress& rAddr, bool bBiff2 );
    /** Reads a BIFF cell address and the following XF identifier. */
    bool                readCellHeader( BiffInputStream& rStrm, bool bBiff2 );
    /** Reads the formula range used by shared formulas, arrays, and data tables. */
    bool                readFormulaRef( BiffInputStream& rStrm );

    /** Imports a BLANK record describing a blank but formatted cell. */
    void                importBlank( BiffInputStream& rStrm );
    /** Imports a BOOLERR record describing a boolean or error code cell. */
    void                importBoolErr( BiffInputStream& rStrm );
    /** Imports a FORMULA record describing a formula cell. */
    void                importFormula( BiffInputStream& rStrm );
    /** Imports an INTEGER record describing a BIFF2 integer cell. */
    void                importInteger( BiffInputStream& rStrm );
    /** Imports a LABEL record describing an unformatted string cell. */
    void                importLabel( BiffInputStream& rStrm );
    /** Imports a LABELSST record describing a string cell using the shared string list. */
    void                importLabelSst( BiffInputStream& rStrm );
    /** Imports a MULTBLANK record describing a range of blank but formatted cells. */
    void                importMultBlank( BiffInputStream& rStrm );
    /** Imports a MULTRK record describing a range of numeric cells. */
    void                importMultRk( BiffInputStream& rStrm );
    /** Imports a NUMBER record describing a floating-point cell. */
    void                importNumber( BiffInputStream& rStrm );
    /** Imports an RK record describing a numeric cell. */
    void                importRk( BiffInputStream& rStrm );

    /** Imports an ARRAY record describing an array formula of a cell range. */
    void                importArray( BiffInputStream& rStrm );
    /** Imports table operation from a DATATABLE or DATATABLE2 record. */
    void                importDataTable( BiffInputStream& rStrm );
    /** Imports a SHAREDFMLA record describing a shared formula in a cell range. */
    void                importSharedFmla( BiffInputStream& rStrm );

private:
    sal_uInt32          mnFormulaSkipSize;  /// Number of bytes to be ignored in FORMULA record.
    sal_uInt32          mnArraySkipSize;    /// Number of bytes to be ignored in ARRAY record.
    sal_uInt16          mnBiff2XfId;        /// Current XF identifier from IXFE record.
    OptValue< bool >    mobBiff2HasXfs;     /// Select XF formatting or direct formatting in BIFF2.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
