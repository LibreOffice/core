/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sheetdatacontext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:49 $
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

#ifndef OOX_XLS_SHEETDATACONTEXT_HXX
#define OOX_XLS_SHEETDATACONTEXT_HXX

#include "oox/xls/ooxcontexthandler.hxx"
#include "oox/xls/richstring.hxx"
#include "oox/xls/worksheethelper.hxx"

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
    explicit            OoxSheetDataContext( const OoxWorksheetFragmentBase& rFragment );

protected:
    // oox.xls.OoxContextHelper interface -------------------------------------

    virtual bool        onCanCreateContext( sal_Int32 nElement ) const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler >
                        onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual bool        onCanCreateRecordContext( sal_Int32 nRecId );
    virtual void        onStartRecord( RecordInputStream& rStrm );

private:
    /** Imports row settings from a row element. */
    void                importRow( const AttributeList& rAttribs );
    /** Imports cell settings from a c element. */
    void                importCell( const AttributeList& rAttribs );
    /** Imports cell settings from an f element. */
    void                importFormula( const AttributeList& rAttribs );

    /** Imports a cell address and the following XF identifier. */
    void                importCellHeader( RecordInputStream& rStrm );
    /** Imports a boolean cell from a CELL_BOOL or FORMULA_BOOL record. */
    void                importCellBool( RecordInputStream& rStrm, bool bFormula );
    /** Imports an empty cell from a CELL_BLANK record. */
    void                importCellBlank( RecordInputStream& rStrm );
    /** Imports a numeric cell from a CELL_DOUBLE or FORMULA_DOUBLE record. */
    void                importCellDouble( RecordInputStream& rStrm, bool bFormula );
    /** Imports an error code cell from a CELL_ERROR or FORMULA_ERROR record. */
    void                importCellError( RecordInputStream& rStrm, bool bFormula );
    /** Imports an encoded numeric cell from a CELL_RK record. */
    void                importCellRk( RecordInputStream& rStrm );
    /** Imports a rich-string cell from a CELL_RSTRING record. */
    void                importCellRString( RecordInputStream& rStrm );
    /** Imports a string cell from a CELL_SI record. */
    void                importCellSi( RecordInputStream& rStrm );
    /** Imports a string cell from a CELL_STRING or FORMULA_STRING record. */
    void                importCellString( RecordInputStream& rStrm, bool bFormula );

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
    OoxCellData         maCurrCell;         /// Position and formatting of current imported cell.
    OoxDataTableData    maTableData;        /// Additional data for table operation ranges.
    BinAddress          maCurrPos;          /// Current position for binary import.
    RichStringRef       mxInlineStr;        /// Inline rich string from 'is' element.
};

// ============================================================================

/** This class implements importing the sheetData element in external sheets.

    The sheetData element embedded in the externalBook element contains cached
    cells from externally linked sheets.
 */
class OoxExternalSheetDataContext : public OoxWorksheetContextBase
{
public:
    explicit            OoxExternalSheetDataContext(
                            const OoxWorkbookFragmentBase& rFragment,
                            WorksheetType eSheetType,
                            sal_Int32 nSheet );

protected:
    // oox.xls.ContextHelper interface ----------------------------------------

    virtual bool        onCanCreateContext( sal_Int32 nElement ) const;
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual bool        onCanCreateRecordContext( sal_Int32 nRecId );
    virtual void        onStartRecord( RecordInputStream& rStrm );

private:
    /** Imports cell settings from a c element. */
    void                importCell( const AttributeList& rAttribs );

    void                importCellHeader( RecordInputStream& rStrm );

    /** Imports the EXTCELL_BOOL from the passed stream. */
    void                importExtCellBool( RecordInputStream& rStrm );
    /** Imports the EXTCELL_DOUBLE from the passed stream. */
    void                importExtCellDouble( RecordInputStream& rStrm );
    /** Imports the EXTCELL_ERROR from the passed stream. */
    void                importExtCellError( RecordInputStream& rStrm );
    /** Imports the EXTCELL_STRING from the passed stream. */
    void                importExtCellString( RecordInputStream& rStrm );

private:
    OoxCellData         maCurrCell;         /// Position of current imported cell.
    BinAddress          maCurrPos;          /// Current position for binary import.
};

// ============================================================================

/** This class implements importing row settings and all cells of a sheet.
 */
class BiffSheetDataContext : public WorksheetHelper
{
public:
    explicit            BiffSheetDataContext( const WorksheetHelper& rHelper );

    /** Tries to import a sheet data record. */
    void                importRecord( BiffInputStream& rStrm );

private:
    /** Sets current cell according to the passed address. */
    void                setCurrCell( const BinAddress& rAddr );

    /** Imports an XF identifier and sets the mnXfId member. */
    void                importXfId( BiffInputStream& rStrm, bool bBiff2 );
    /** Imports a BIFF cell address and the following XF identifier. */
    void                importCellHeader( BiffInputStream& rStrm, bool bBiff2 );

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

    /** Imports row settings from a ROW record. */
    void                importRow( BiffInputStream& rStrm );
    /** Imports an ARRAY record describing an array formula of a cell range. */
    void                importArray( BiffInputStream& rStrm );
    /** Imports a SHAREDFMLA record describing a shared formula in a cell range. */
    void                importSharedFmla( BiffInputStream& rStrm );
    /** Imports table operation from a DATATABLE or DATATABLE2 record. */
    void                importDataTable( BiffInputStream& rStrm );

private:
    OoxCellData         maCurrCell;             /// Position and formatting of current imported cell.
    sal_uInt32          mnFormulaIgnoreSize;    /// Number of bytes to be ignored in FORMULA record.
    sal_uInt32          mnArrayIgnoreSize;      /// Number of bytes to be ignored in ARRAY record.
    sal_uInt16          mnBiff2XfId;            /// Current XF identifier from IXFE record.
};

// ============================================================================

/** This class implements importing cached cell data of external links.
 */
class BiffExternalSheetDataContext : public WorksheetHelperRoot
{
public:
    explicit            BiffExternalSheetDataContext(
                            const WorkbookHelper& rHelper,
                            WorksheetType eSheetType,
                            sal_Int32 nSheet );

    /** Import the CRN record containing cached cell values. */
    void                importCrn( BiffInputStream& rStrm );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

