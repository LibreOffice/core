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

namespace com { namespace sun { namespace star {
    namespace table { class XCell; }
} } }

namespace oox {
namespace xls {

// ============================================================================

/** This class implements importing the sheetData element.

    The sheetData element contains all row settings and all cells in a single
    sheet of a spreadsheet document.
 */
class OoxSheetDataContext : public OoxWorksheetContextBase
{
public:
    explicit            OoxSheetDataContext( OoxWorksheetFragmentBase& rFragment );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );

private:
    /** Different types of cell records. */
    enum CellType { CELLTYPE_VALUE, CELLTYPE_MULTI, CELLTYPE_FORMULA };

    /** Imports row settings from a row element. */
    void                importRow( const AttributeList& rAttribs );
    /** Imports cell settings from a c element. */
    void                importCell( const AttributeList& rAttribs );
    /** Imports cell settings from an f element. */
    void                importFormula( const AttributeList& rAttribs );

    /** Imports a cell address and the following XF identifier. */
    void                importCellHeader( RecordInputStream& rStrm, CellType eCellType );
    /** Imports an empty cell from a CELL_BLANK or MULTCELL_BLANK record. */
    void                importCellBlank( RecordInputStream& rStrm, CellType eCellType );
    /** Imports a boolean cell from a CELL_BOOL, MULTCELL_BOOL, or FORMULA_BOOL record. */
    void                importCellBool( RecordInputStream& rStrm, CellType eCellType );
    /** Imports a numeric cell from a CELL_DOUBLE, MULTCELL_DOUBLE, or FORMULA_DOUBLE record. */
    void                importCellDouble( RecordInputStream& rStrm, CellType eCellType );
    /** Imports an error code cell from a CELL_ERROR, MULTCELL_ERROR, or FORMULA_ERROR record. */
    void                importCellError( RecordInputStream& rStrm, CellType eCellType );
    /** Imports an encoded numeric cell from a CELL_RK or MULTCELL_RK record. */
    void                importCellRk( RecordInputStream& rStrm, CellType eCellType );
    /** Imports a rich-string cell from a CELL_RSTRING or MULTCELL_RSTRING record. */
    void                importCellRString( RecordInputStream& rStrm, CellType eCellType );
    /** Imports a string cell from a CELL_SI or MULTCELL_SI record. */
    void                importCellSi( RecordInputStream& rStrm, CellType eCellType );
    /** Imports a string cell from a CELL_STRING, MULTCELL_STRING, or FORMULA_STRING record. */
    void                importCellString( RecordInputStream& rStrm, CellType eCellType );

    /** Imports a cell formula for the current cell. */
    void                importCellFormula( RecordInputStream& rStrm );

    /** Imports row settings from a ROW record. */
    void                importRow( RecordInputStream& rStrm );
    /** Imports an array formula from an ARRAY record. */
    void                importArray( RecordInputStream& rStrm );
    /** Imports a shared formula from a SHAREDFORMULA record. */
    void                importSharedFmla( RecordInputStream& rStrm );
    /** Imports table operation from a DATATABLE record. */
    void                importDataTable( RecordInputStream& rStrm );

private:
    CellModel           maCurrCell;         /// Position and formatting of current imported cell.
    DataTableModel      maTableData;        /// Additional data for table operation ranges.
    BinAddress          maCurrPos;          /// Current position for binary import.
    RichStringRef       mxInlineStr;        /// Inline rich string from 'is' element.
};

// ============================================================================

/** This class implements importing row settings and all cells of a sheet.
 */
class BiffSheetDataContext : public BiffWorksheetContextBase
{
public:
    explicit            BiffSheetDataContext( const BiffWorksheetFragmentBase& rParent );

    /** Tries to import a sheet data record. */
    virtual void        importRecord();

private:
    /** Sets current cell according to the passed address. */
    void                setCurrCell( const BinAddress& rAddr );

    /** Imports an XF identifier and sets the mnXfId member. */
    void                importXfId( bool bBiff2 );
    /** Imports a BIFF cell address and the following XF identifier. */
    void                importCellHeader( bool bBiff2 );

    /** Imports a BLANK record describing a blank but formatted cell. */
    void                importBlank();
    /** Imports a BOOLERR record describing a boolean or error code cell. */
    void                importBoolErr();
    /** Imports a FORMULA record describing a formula cell. */
    void                importFormula();
    /** Imports an INTEGER record describing a BIFF2 integer cell. */
    void                importInteger();
    /** Imports a LABEL record describing an unformatted string cell. */
    void                importLabel();
    /** Imports a LABELSST record describing a string cell using the shared string list. */
    void                importLabelSst();
    /** Imports a MULTBLANK record describing a range of blank but formatted cells. */
    void                importMultBlank();
    /** Imports a MULTRK record describing a range of numeric cells. */
    void                importMultRk();
    /** Imports a NUMBER record describing a floating-point cell. */
    void                importNumber();
    /** Imports an RK record describing a numeric cell. */
    void                importRk();

    /** Imports row settings from a ROW record. */
    void                importRow();
    /** Imports an ARRAY record describing an array formula of a cell range. */
    void                importArray();
    /** Imports a SHAREDFMLA record describing a shared formula in a cell range. */
    void                importSharedFmla();
    /** Imports table operation from a DATATABLE or DATATABLE2 record. */
    void                importDataTable();

private:
    CellModel           maCurrCell;             /// Position and formatting of current imported cell.
    sal_uInt32          mnFormulaIgnoreSize;    /// Number of bytes to be ignored in FORMULA record.
    sal_uInt32          mnArrayIgnoreSize;      /// Number of bytes to be ignored in ARRAY record.
    sal_uInt16          mnBiff2XfId;            /// Current XF identifier from IXFE record.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

