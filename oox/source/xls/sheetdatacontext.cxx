/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sheetdatacontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:06:15 $
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

#include "oox/xls/sheetdatacontext.hxx"
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/XArrayFormulaTokens.hpp>
#include <com/sun/star/text/XText.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/formulaparser.hxx"
#include "oox/xls/pivottablebuffer.hxx"
#include "oox/xls/richstringcontext.hxx"
#include "oox/xls/sharedformulabuffer.hxx"
#include "oox/xls/unitconverter.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::table::CellContentType_EMPTY;
using ::com::sun::star::table::XCell;
using ::com::sun::star::table::XCellRange;
using ::com::sun::star::sheet::XFormulaTokens;
using ::com::sun::star::sheet::XArrayFormulaTokens;
using ::com::sun::star::text::XText;

namespace oox {
namespace xls {

// ============================================================================

namespace {

// record constants -----------------------------------------------------------

const sal_uInt32 OOBIN_CELL_SHOWPHONETIC    = 0x01000000;

const sal_uInt8 OOBIN_DATATABLE_ROW         = 0x01;
const sal_uInt8 OOBIN_DATATABLE_2D          = 0x02;
const sal_uInt8 OOBIN_DATATABLE_REF1DEL     = 0x04;
const sal_uInt8 OOBIN_DATATABLE_REF2DEL     = 0x08;

const sal_uInt16 OOBIN_ROW_THICKTOP         = 0x0001;
const sal_uInt16 OOBIN_ROW_THICKBOTTOM      = 0x0002;
const sal_uInt16 OOBIN_ROW_COLLAPSED        = 0x0800;
const sal_uInt16 OOBIN_ROW_HIDDEN           = 0x1000;
const sal_uInt16 OOBIN_ROW_CUSTOMHEIGHT     = 0x2000;
const sal_uInt16 OOBIN_ROW_CUSTOMFORMAT     = 0x4000;
const sal_uInt8 OOBIN_ROW_SHOWPHONETIC      = 0x01;

const sal_uInt8 BIFF_BOOLERR_BOOL           = 0;
const sal_uInt8 BIFF_BOOLERR_ERROR          = 1;

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

const sal_Int32 BIFF_XF_EXTENDED_IDS        = 63;
const sal_uInt8 BIFF2_XF_MASK               = 0x3F;

// ----------------------------------------------------------------------------

/** Formula context for cell formulas. */
class CellFormulaContext : public SimpleFormulaContext
{
public:
    explicit            CellFormulaContext(
                            const Reference< XFormulaTokens >& rxTokens,
                            const CellAddress& rCellPos );
};

CellFormulaContext::CellFormulaContext( const Reference< XFormulaTokens >& rxTokens, const CellAddress& rCellPos ) :
    SimpleFormulaContext( rxTokens, false, false )
{
    setBaseAddress( rCellPos );
}

// ----------------------------------------------------------------------------

/** Uses the XArrayFormulaTokens interface to set a token sequence. */
class ArrayFormulaContext : public FormulaContext
{
public:
    explicit            ArrayFormulaContext(
                            const Reference< XArrayFormulaTokens >& rxTokens,
                            const CellRangeAddress& rArrayRange );

    virtual void        setTokens( const ApiTokenSequence& rTokens );

private:
    Reference< XArrayFormulaTokens > mxTokens;
};

ArrayFormulaContext::ArrayFormulaContext(
        const Reference< XArrayFormulaTokens >& rxTokens, const CellRangeAddress& rArrayRange ) :
    FormulaContext( false, false ),
    mxTokens( rxTokens )
{
    OSL_ENSURE( mxTokens.is(), "ArrayFormulaContext::ArrayFormulaContext - missing XArrayFormulaTokens interface" );
    setBaseAddress( CellAddress( rArrayRange.Sheet, rArrayRange.StartColumn, rArrayRange.StartRow ) );
}

void ArrayFormulaContext::setTokens( const ApiTokenSequence& rTokens )
{
    mxTokens->setArrayTokens( rTokens );
}

// ----------------------------------------------------------------------------

} // namespace

// ============================================================================

OoxSheetDataContext::OoxSheetDataContext( OoxWorksheetFragmentBase& rFragment ) :
    OoxWorksheetContextBase( rFragment )
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper OoxSheetDataContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( sheetData ):
            return  (nElement == XLS_TOKEN( row ));
        case XLS_TOKEN( row ):
            return  (nElement == XLS_TOKEN( c ));
        case XLS_TOKEN( c ):
            if( maCurrCell.mxCell.is() )
            {
                if( nElement == XLS_TOKEN( is ) )
                {
                    mxInlineStr.reset( new RichString( *this ) );
                    return new OoxRichStringContext( *this, mxInlineStr );
                }
                return  (nElement == XLS_TOKEN( v )) ||
                        (nElement == XLS_TOKEN( f ));
            }
        break;
    }
    return false;
}

void OoxSheetDataContext::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( row ):  importRow( rAttribs );      break;
        case XLS_TOKEN( c ):    importCell( rAttribs );     break;
        case XLS_TOKEN( f ):    importFormula( rAttribs );  break;
    }
}

void OoxSheetDataContext::onEndElement( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( v ):
            maCurrCell.maValueStr = rChars;
            maCurrCell.mbHasValueStr = true;
        break;

        case XLS_TOKEN( f ):
            if( maCurrCell.mxCell.is() ) try
            {
                switch( maCurrCell.mnFormulaType )
                {
                    case XML_normal:
                        if( rChars.getLength() > 0 )
                        {
                            Reference< XFormulaTokens > xTokens( maCurrCell.mxCell, UNO_QUERY_THROW );
                            CellFormulaContext aContext( xTokens, maCurrCell.maAddress );
                            getFormulaParser().importFormula( aContext, rChars );
                        }
                    break;

                    case XML_array:
                        if( (maCurrCell.maFormulaRef.getLength() > 0) && (rChars.getLength() > 0) )
                        {
                            CellRangeAddress aArrayRange;
                            Reference< XArrayFormulaTokens > xTokens( getCellRange( maCurrCell.maFormulaRef, &aArrayRange ), UNO_QUERY_THROW );
                            ArrayFormulaContext aContext( xTokens, aArrayRange );
                            getFormulaParser().importFormula( aContext, rChars );
                        }
                    break;

                    case XML_shared:
                        if( maCurrCell.mnSharedId >= 0 )
                        {
                            if( rChars.getLength() > 0 )
                                getSharedFormulas().importSharedFmla( rChars, maCurrCell.maFormulaRef, maCurrCell.mnSharedId, maCurrCell.maAddress );
                            Reference< XFormulaTokens > xTokens( maCurrCell.mxCell, UNO_QUERY_THROW );
                            ExtCellFormulaContext aContext( *this, xTokens, maCurrCell.maAddress );
                            getSharedFormulas().setSharedFormulaCell( aContext, maCurrCell.mnSharedId );
                        }
                    break;

                    case XML_dataTable:
                        if( maCurrCell.maFormulaRef.getLength() > 0 )
                        {
                            CellRangeAddress aTableRange;
                            if( getAddressConverter().convertToCellRange( aTableRange, maCurrCell.maFormulaRef, getSheetIndex(), true ) )
                                setTableOperation( aTableRange, maTableData );
                        }
                    break;

                    default:
                        OSL_ENSURE( false, "OoxSheetDataContext::onEndElement - unknown formula type" );
                }
            }
            catch( Exception& )
            {
            }
        break;

        case XLS_TOKEN( c ):
            if( maCurrCell.mxCell.is() )
            {
                if( maCurrCell.mxCell->getType() == CellContentType_EMPTY )
                {
                    if( maCurrCell.mbHasValueStr )
                    {
                        // implemented in WorksheetHelper class
                        setOoxCell( maCurrCell );
                    }
                    else if( (maCurrCell.mnCellType == XML_inlineStr) && mxInlineStr.get() )
                    {
                        // convert font settings
                        mxInlineStr->finalizeImport();
                        // write string to cell
                        Reference< XText > xText( maCurrCell.mxCell, UNO_QUERY );
                        if( xText.is() )
                            mxInlineStr->convert( xText, maCurrCell.mnXfId );
                    }
                    else
                    {
                        // empty cell, update cell type
                        maCurrCell.mnCellType = XML_TOKEN_INVALID;
                    }
                }

                // store the cell formatting data
                setCellFormat( maCurrCell );
            }
        break;
    }
}

ContextWrapper OoxSheetDataContext::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_SHEETDATA:
            return  (nRecId == OOBIN_ID_ROW);
        case OOBIN_ID_ROW:
            return  (nRecId == OOBIN_ID_ARRAY) ||
                    (nRecId == OOBIN_ID_CELL_BOOL) ||
                    (nRecId == OOBIN_ID_CELL_BLANK) ||
                    (nRecId == OOBIN_ID_CELL_DOUBLE) ||
                    (nRecId == OOBIN_ID_CELL_ERROR) ||
                    (nRecId == OOBIN_ID_CELL_RK) ||
                    (nRecId == OOBIN_ID_CELL_RSTRING) ||
                    (nRecId == OOBIN_ID_CELL_SI) ||
                    (nRecId == OOBIN_ID_CELL_STRING) ||
                    (nRecId == OOBIN_ID_DATATABLE) ||
                    (nRecId == OOBIN_ID_FORMULA_BOOL) ||
                    (nRecId == OOBIN_ID_FORMULA_DOUBLE) ||
                    (nRecId == OOBIN_ID_FORMULA_ERROR) ||
                    (nRecId == OOBIN_ID_FORMULA_STRING) ||
                    (nRecId == OOBIN_ID_MULTCELL_BOOL) ||
                    (nRecId == OOBIN_ID_MULTCELL_BLANK) ||
                    (nRecId == OOBIN_ID_MULTCELL_DOUBLE) ||
                    (nRecId == OOBIN_ID_MULTCELL_ERROR) ||
                    (nRecId == OOBIN_ID_MULTCELL_RK) ||
                    (nRecId == OOBIN_ID_MULTCELL_RSTRING) ||
                    (nRecId == OOBIN_ID_MULTCELL_SI) ||
                    (nRecId == OOBIN_ID_MULTCELL_STRING) ||
                    (nRecId == OOBIN_ID_SHAREDFMLA);
    }
    return false;
}

void OoxSheetDataContext::onStartRecord( RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_ARRAY:            importArray( rStrm );                           break;
        case OOBIN_ID_CELL_BOOL:        importCellBool( rStrm, CELLTYPE_VALUE );        break;
        case OOBIN_ID_CELL_BLANK:       importCellBlank( rStrm, CELLTYPE_VALUE );       break;
        case OOBIN_ID_CELL_DOUBLE:      importCellDouble( rStrm, CELLTYPE_VALUE );      break;
        case OOBIN_ID_CELL_ERROR:       importCellError( rStrm, CELLTYPE_VALUE );       break;
        case OOBIN_ID_CELL_RK:          importCellRk( rStrm, CELLTYPE_VALUE );          break;
        case OOBIN_ID_CELL_RSTRING:     importCellRString( rStrm, CELLTYPE_VALUE );     break;
        case OOBIN_ID_CELL_SI:          importCellSi( rStrm, CELLTYPE_VALUE );          break;
        case OOBIN_ID_CELL_STRING:      importCellString( rStrm, CELLTYPE_VALUE );      break;
        case OOBIN_ID_DATATABLE:        importDataTable( rStrm );                       break;
        case OOBIN_ID_FORMULA_BOOL:     importCellBool( rStrm, CELLTYPE_FORMULA );      break;
        case OOBIN_ID_FORMULA_DOUBLE:   importCellDouble( rStrm, CELLTYPE_FORMULA );    break;
        case OOBIN_ID_FORMULA_ERROR:    importCellError( rStrm, CELLTYPE_FORMULA );     break;
        case OOBIN_ID_FORMULA_STRING:   importCellString( rStrm, CELLTYPE_FORMULA );    break;
        case OOBIN_ID_MULTCELL_BOOL:    importCellBool( rStrm, CELLTYPE_MULTI );        break;
        case OOBIN_ID_MULTCELL_BLANK:   importCellBlank( rStrm, CELLTYPE_MULTI );       break;
        case OOBIN_ID_MULTCELL_DOUBLE:  importCellDouble( rStrm, CELLTYPE_MULTI );      break;
        case OOBIN_ID_MULTCELL_ERROR:   importCellError( rStrm, CELLTYPE_MULTI );       break;
        case OOBIN_ID_MULTCELL_RK:      importCellRk( rStrm, CELLTYPE_MULTI );          break;
        case OOBIN_ID_MULTCELL_RSTRING: importCellRString( rStrm, CELLTYPE_MULTI );     break;
        case OOBIN_ID_MULTCELL_SI:      importCellSi( rStrm, CELLTYPE_MULTI );          break;
        case OOBIN_ID_MULTCELL_STRING:  importCellString( rStrm, CELLTYPE_MULTI );      break;
        case OOBIN_ID_ROW:              importRow( rStrm );                             break;
        case OOBIN_ID_SHAREDFMLA:       importSharedFmla( rStrm );                      break;
    }
}

// private --------------------------------------------------------------------

void OoxSheetDataContext::importRow( const AttributeList& rAttribs )
{
    OoxRowData aData;
    aData.mnFirstRow = aData.mnLastRow = rAttribs.getInteger( XML_r, -1 );
    aData.mfHeight = rAttribs.getDouble( XML_ht, -1.0 );
    aData.mnXfId = rAttribs.getInteger( XML_s, -1 );
    aData.mnLevel = rAttribs.getInteger( XML_outlineLevel, 0 );
    aData.mbCustomHeight = rAttribs.getBool( XML_customHeight, false );
    aData.mbCustomFormat = rAttribs.getBool( XML_customFormat, false );
    aData.mbShowPhonetic = rAttribs.getBool( XML_ph, false );
    aData.mbHidden = rAttribs.getBool( XML_hidden, false );
    aData.mbCollapsed = rAttribs.getBool( XML_collapsed, false );
    aData.mbThickTop = rAttribs.getBool( XML_thickTop, false );
    aData.mbThickBottom = rAttribs.getBool( XML_thickBot, false );
    // set row properties in the current sheet
    setRowData( aData );
}

void OoxSheetDataContext::importCell( const AttributeList& rAttribs )
{
    maCurrCell.reset();
    maCurrCell.mxCell         = getCell( rAttribs.getString( XML_r ), &maCurrCell.maAddress );
    maCurrCell.mnCellType     = rAttribs.getToken( XML_t, XML_n );
    maCurrCell.mnXfId         = rAttribs.getInteger( XML_s, -1 );
    maCurrCell.mbShowPhonetic = rAttribs.getBool( XML_ph, false );
    mxInlineStr.reset();

    if( maCurrCell.mxCell.is() && getPivotTables().isOverlapping( maCurrCell.maAddress ) )
        // This cell overlaps a pivot table.  Skip it.
        maCurrCell.mxCell.clear();
}

void OoxSheetDataContext::importFormula( const AttributeList& rAttribs )
{
    maCurrCell.maFormulaRef   = rAttribs.getString( XML_ref );
    maCurrCell.mnFormulaType  = rAttribs.getToken( XML_t, XML_normal );
    maCurrCell.mnSharedId     = rAttribs.getInteger( XML_si, -1 );
    maTableData.maRef1        = rAttribs.getString( XML_r1 );
    maTableData.maRef2        = rAttribs.getString( XML_r2 );
    maTableData.mb2dTable     = rAttribs.getBool( XML_dt2D, false );
    maTableData.mbRowTable    = rAttribs.getBool( XML_dtr, false );
    maTableData.mbRef1Deleted = rAttribs.getBool( XML_del1, false );
    maTableData.mbRef2Deleted = rAttribs.getBool( XML_del2, false );
}

void OoxSheetDataContext::importCellHeader( RecordInputStream& rStrm, CellType eCellType )
{
    maCurrCell.reset();

    switch( eCellType )
    {
        case CELLTYPE_VALUE:
        case CELLTYPE_FORMULA:  rStrm >> maCurrPos.mnCol;   break;
        case CELLTYPE_MULTI:    ++maCurrPos.mnCol;          break;
    }

    sal_uInt32 nXfId;
    rStrm >> nXfId;

    maCurrCell.mxCell         = getCell( maCurrPos, &maCurrCell.maAddress );
    maCurrCell.mnXfId         = extractValue< sal_Int32 >( nXfId, 0, 24 );
    maCurrCell.mbShowPhonetic = getFlag( nXfId, OOBIN_CELL_SHOWPHONETIC );
}

void OoxSheetDataContext::importCellBool( RecordInputStream& rStrm, CellType eCellType )
{
    importCellHeader( rStrm, eCellType );
    maCurrCell.mnCellType = XML_b;
    if( maCurrCell.mxCell.is() && (maCurrCell.mxCell->getType() == CellContentType_EMPTY) )
    {
        bool bValue = rStrm.readuInt8() != 0;
        if( eCellType == CELLTYPE_FORMULA )
        {
            importCellFormula( rStrm );
        }
        else
        {
            setBooleanCell( maCurrCell.mxCell, bValue );
            // #108770# set 'Standard' number format for all Boolean cells
            maCurrCell.mnNumFmtId = 0;
        }
    }
    setCellFormat( maCurrCell );
}

void OoxSheetDataContext::importCellBlank( RecordInputStream& rStrm, CellType eCellType )
{
    OSL_ENSURE( eCellType != CELLTYPE_FORMULA, "OoxSheetDataContext::importCellBlank - no formula cells supported" );
    importCellHeader( rStrm, eCellType );
    setCellFormat( maCurrCell );
}

void OoxSheetDataContext::importCellDouble( RecordInputStream& rStrm, CellType eCellType )
{
    importCellHeader( rStrm, eCellType );
    maCurrCell.mnCellType = XML_n;
    if( maCurrCell.mxCell.is() && (maCurrCell.mxCell->getType() == CellContentType_EMPTY) )
    {
        double fValue = rStrm.readDouble();
        if( eCellType == CELLTYPE_FORMULA )
            importCellFormula( rStrm );
        else
            maCurrCell.mxCell->setValue( fValue );
    }
    setCellFormat( maCurrCell );
}

void OoxSheetDataContext::importCellError( RecordInputStream& rStrm, CellType eCellType )
{
    importCellHeader( rStrm, eCellType );
    maCurrCell.mnCellType = XML_e;
    if( maCurrCell.mxCell.is() && (maCurrCell.mxCell->getType() == CellContentType_EMPTY) )
    {
        sal_uInt8 nErrorCode = rStrm.readuInt8();
        if( eCellType == CELLTYPE_FORMULA )
            importCellFormula( rStrm );
        else
            setErrorCell( maCurrCell.mxCell, nErrorCode );
    }
    setCellFormat( maCurrCell );
}

void OoxSheetDataContext::importCellRk( RecordInputStream& rStrm, CellType eCellType )
{
    OSL_ENSURE( eCellType != CELLTYPE_FORMULA, "OoxSheetDataContext::importCellRk - no formula cells supported" );
    importCellHeader( rStrm, eCellType );
    maCurrCell.mnCellType = XML_n;
    if( maCurrCell.mxCell.is() && (maCurrCell.mxCell->getType() == CellContentType_EMPTY) )
        maCurrCell.mxCell->setValue( BiffHelper::calcDoubleFromRk( rStrm.readInt32() ) );
    setCellFormat( maCurrCell );
}

void OoxSheetDataContext::importCellRString( RecordInputStream& rStrm, CellType eCellType )
{
    OSL_ENSURE( eCellType != CELLTYPE_FORMULA, "OoxSheetDataContext::importCellRString - no formula cells supported" );
    importCellHeader( rStrm, eCellType );
    maCurrCell.mnCellType = XML_inlineStr;
    Reference< XText > xText( maCurrCell.mxCell, UNO_QUERY );
    if( xText.is() && (maCurrCell.mxCell->getType() == CellContentType_EMPTY) )
    {
        RichString aString( *this );
        aString.importString( rStrm, true );
        aString.finalizeImport();
        aString.convert( xText, maCurrCell.mnXfId );
    }
    setCellFormat( maCurrCell );
}

void OoxSheetDataContext::importCellSi( RecordInputStream& rStrm, CellType eCellType )
{
    OSL_ENSURE( eCellType != CELLTYPE_FORMULA, "OoxSheetDataContext::importCellSi - no formula cells supported" );
    importCellHeader( rStrm, eCellType );
    maCurrCell.mnCellType = XML_s;
    if( maCurrCell.mxCell.is() && (maCurrCell.mxCell->getType() == CellContentType_EMPTY) )
        setSharedStringCell( maCurrCell.mxCell, rStrm.readInt32(), maCurrCell.mnXfId );
    setCellFormat( maCurrCell );
}

void OoxSheetDataContext::importCellString( RecordInputStream& rStrm, CellType eCellType )
{
    importCellHeader( rStrm, eCellType );
    maCurrCell.mnCellType = XML_inlineStr;
    Reference< XText > xText( maCurrCell.mxCell, UNO_QUERY );
    if( xText.is() && (maCurrCell.mxCell->getType() == CellContentType_EMPTY) )
    {
        RichString aString( *this );
        aString.importString( rStrm, false );
        aString.finalizeImport();
        if( eCellType == CELLTYPE_FORMULA )
            importCellFormula( rStrm );
        else
            aString.convert( xText, maCurrCell.mnXfId );
    }
    setCellFormat( maCurrCell );
}

void OoxSheetDataContext::importCellFormula( RecordInputStream& rStrm )
{
    rStrm.skip( 2 );
    Reference< XFormulaTokens > xTokens( maCurrCell.mxCell, UNO_QUERY );
    if( xTokens.is() )
    {
        ExtCellFormulaContext aContext( *this, xTokens, maCurrCell.maAddress );
        getFormulaParser().importFormula( aContext, rStrm );
    }
}

void OoxSheetDataContext::importRow( RecordInputStream& rStrm )
{
    OoxRowData aData;

    sal_uInt16 nHeight, nFlags1;
    sal_uInt8 nFlags2;
    rStrm >> maCurrPos.mnRow >> aData.mnXfId >> nHeight >> nFlags1 >> nFlags2;

    // row index is 0-based in OOBIN, but OoxRowData expects 1-based
    aData.mnFirstRow = aData.mnLastRow = maCurrPos.mnRow + 1;
    // row height is in twips in OOBIN, convert to points
    aData.mfHeight = nHeight / 20.0;
    aData.mnLevel = extractValue< sal_Int32 >( nFlags1, 8, 3 );
    aData.mbCustomHeight = getFlag( nFlags1, OOBIN_ROW_CUSTOMHEIGHT );
    aData.mbCustomFormat = getFlag( nFlags1, OOBIN_ROW_CUSTOMFORMAT );
    aData.mbShowPhonetic = getFlag( nFlags2, OOBIN_ROW_SHOWPHONETIC );
    aData.mbHidden = getFlag( nFlags1, OOBIN_ROW_HIDDEN );
    aData.mbCollapsed = getFlag( nFlags1, OOBIN_ROW_COLLAPSED );
    aData.mbThickTop = getFlag( nFlags1, OOBIN_ROW_THICKTOP );
    aData.mbThickBottom = getFlag( nFlags1, OOBIN_ROW_THICKBOTTOM );
    // set row properties in the current sheet
    setRowData( aData );
}

void OoxSheetDataContext::importArray( RecordInputStream& rStrm )
{
    BinRange aRange;
    rStrm >> aRange;
    CellRangeAddress aArrayRange;
    Reference< XCellRange > xRange = getCellRange( aRange, &aArrayRange );
    Reference< XArrayFormulaTokens > xTokens( xRange, UNO_QUERY );
    if( xRange.is() && xTokens.is() )
    {
        rStrm.skip( 1 );
        ArrayFormulaContext aContext( xTokens, aArrayRange );
        getFormulaParser().importFormula( aContext, rStrm );
    }
}

void OoxSheetDataContext::importSharedFmla( RecordInputStream& rStrm )
{
    getSharedFormulas().importSharedFmla( rStrm, maCurrCell.maAddress );
}

void OoxSheetDataContext::importDataTable( RecordInputStream& rStrm )
{
    BinRange aRange;
    rStrm >> aRange;
    CellRangeAddress aTableRange;
    if( getAddressConverter().convertToCellRange( aTableRange, aRange, getSheetIndex(), true ) )
    {
        OoxDataTableData aTableData;
        BinAddress aRef1, aRef2;
        sal_uInt8 nFlags;
        rStrm >> aRef1 >> aRef2 >> nFlags;
        aTableData.maRef1 = FormulaProcessorBase::generateAddress2dString( aRef1, false );
        aTableData.maRef2 = FormulaProcessorBase::generateAddress2dString( aRef2, false );
        aTableData.mbRowTable = getFlag( nFlags, OOBIN_DATATABLE_ROW );
        aTableData.mb2dTable = getFlag( nFlags, OOBIN_DATATABLE_2D );
        aTableData.mbRef1Deleted = getFlag( nFlags, OOBIN_DATATABLE_REF1DEL );
        aTableData.mbRef2Deleted = getFlag( nFlags, OOBIN_DATATABLE_REF2DEL );
        setTableOperation( aTableRange, aTableData );
    }
}

// ============================================================================

OoxExternalSheetDataContext::OoxExternalSheetDataContext(
        OoxWorkbookFragmentBase& rFragment, WorksheetType eSheetType, sal_Int32 nSheet ) :
    OoxWorksheetContextBase( rFragment, ISegmentProgressBarRef(), eSheetType, nSheet )
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper OoxExternalSheetDataContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( sheetData ):
            return  (nElement == XLS_TOKEN( row ));
        case XLS_TOKEN( row ):
            return  (nElement == XLS_TOKEN( cell ));
        case XLS_TOKEN( cell ):
            return  (nElement == XLS_TOKEN( v )) && maCurrCell.mxCell.is();
    }
    return false;
}

void OoxExternalSheetDataContext::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( cell ):
            importCell( rAttribs );
        break;
    }
}

void OoxExternalSheetDataContext::onEndElement( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( v ):
            maCurrCell.maValueStr = rChars;
            maCurrCell.mbHasValueStr = true;
        break;

        case XLS_TOKEN( cell ):
            if( maCurrCell.mxCell.is() )
                setOoxCell( maCurrCell, true );
        break;
    }
}

ContextWrapper OoxExternalSheetDataContext::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_EXTSHEETDATA:
            return  (nRecId == OOBIN_ID_EXTROW);
        case OOBIN_ID_EXTROW:
            return  (nRecId == OOBIN_ID_EXTCELL_BLANK) ||
                    (nRecId == OOBIN_ID_EXTCELL_BOOL) ||
                    (nRecId == OOBIN_ID_EXTCELL_DOUBLE) ||
                    (nRecId == OOBIN_ID_EXTCELL_ERROR) ||
                    (nRecId == OOBIN_ID_EXTCELL_STRING);
    }
    return false;
}

void OoxExternalSheetDataContext::onStartRecord( RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_EXTCELL_BLANK:    importExtCellBlank( rStrm );    break;
        case OOBIN_ID_EXTCELL_BOOL:     importExtCellBool( rStrm );     break;
        case OOBIN_ID_EXTCELL_DOUBLE:   importExtCellDouble( rStrm );   break;
        case OOBIN_ID_EXTCELL_ERROR:    importExtCellError( rStrm );    break;
        case OOBIN_ID_EXTCELL_STRING:   importExtCellString( rStrm );   break;
        case OOBIN_ID_EXTROW:           rStrm >> maCurrPos.mnRow;       break;
    }
}

// private --------------------------------------------------------------------

void OoxExternalSheetDataContext::importCell( const AttributeList& rAttribs )
{
    maCurrCell.reset();
    maCurrCell.mxCell = getCell( rAttribs.getString( XML_r ), &maCurrCell.maAddress );
    maCurrCell.mnCellType = rAttribs.getToken( XML_t, XML_n );
}

void OoxExternalSheetDataContext::importCellHeader( RecordInputStream& rStrm )
{
    maCurrCell.reset();
    rStrm >> maCurrPos.mnCol;
    maCurrCell.mxCell = getCell( maCurrPos, &maCurrCell.maAddress );
}

void OoxExternalSheetDataContext::importExtCellBlank( RecordInputStream& rStrm )
{
    importCellHeader( rStrm );
    if( maCurrCell.mxCell.is() )
        setEmptyStringCell( maCurrCell.mxCell );
}

void OoxExternalSheetDataContext::importExtCellBool( RecordInputStream& rStrm )
{
    importCellHeader( rStrm );
    if( maCurrCell.mxCell.is() )
        setBooleanCell( maCurrCell.mxCell, rStrm.readuInt8() != 0 );
}

void OoxExternalSheetDataContext::importExtCellDouble( RecordInputStream& rStrm )
{
    importCellHeader( rStrm );
    if( maCurrCell.mxCell.is() )
        maCurrCell.mxCell->setValue( rStrm.readDouble() );
}

void OoxExternalSheetDataContext::importExtCellError( RecordInputStream& rStrm )
{
    importCellHeader( rStrm );
    if( maCurrCell.mxCell.is() )
        setErrorCell( maCurrCell.mxCell, rStrm.readuInt8() );
}

void OoxExternalSheetDataContext::importExtCellString( RecordInputStream& rStrm )
{
    importCellHeader( rStrm );
    if( maCurrCell.mxCell.is() )
        setStringCell( maCurrCell.mxCell, rStrm.readString(), true );
}

// ============================================================================
// ============================================================================

BiffSheetDataContext::BiffSheetDataContext( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper ),
    mnBiff2XfId( 0 )
{
    mnArrayIgnoreSize = (getBiff() == BIFF2) ? 1 : ((getBiff() <= BIFF4) ? 2 : 6);
    switch( getBiff() )
    {
        case BIFF2:
            mnFormulaIgnoreSize = 9;    // double formula result, 1 byte flags
            mnArrayIgnoreSize = 1;      // recalc-always flag
        break;
        case BIFF3:
        case BIFF4:
            mnFormulaIgnoreSize = 10;   // double formula result, 2 byte flags
            mnArrayIgnoreSize = 2;      // 2 byte flags
        break;
        case BIFF5:
        case BIFF8:
            mnFormulaIgnoreSize = 14;   // double formula result, 2 byte flags, 4 bytes nothing
            mnArrayIgnoreSize = 6;      // 2 byte flags, 4 bytes nothing
        break;
        case BIFF_UNKNOWN: break;
    }
}

void BiffSheetDataContext::importRecord( BiffInputStream& rStrm )
{
    sal_uInt16 nRecId = rStrm.getRecId();
    switch( nRecId )
    {
        // records in all BIFF versions
        case BIFF2_ID_ARRAY:        // #i72713#
        case BIFF3_ID_ARRAY:        importArray( rStrm );       break;
        case BIFF2_ID_BLANK:
        case BIFF3_ID_BLANK:        importBlank( rStrm );       break;
        case BIFF2_ID_BOOLERR:
        case BIFF3_ID_BOOLERR:      importBoolErr( rStrm );     break;
        case BIFF2_ID_INTEGER:      importInteger( rStrm );     break;
        case BIFF_ID_IXFE:          rStrm >> mnBiff2XfId;       break;
        case BIFF2_ID_LABEL:
        case BIFF3_ID_LABEL:        importLabel( rStrm );       break;
        case BIFF2_ID_NUMBER:
        case BIFF3_ID_NUMBER:       importNumber( rStrm );      break;
        case BIFF_ID_RK:            importRk( rStrm );          break;

        // BIFF specific records
        default: switch( getBiff() )
        {
            case BIFF2: switch( nRecId )
            {
                case BIFF2_ID_DATATABLE:    importDataTable( rStrm );   break;
                case BIFF2_ID_DATATABLE2:   importDataTable( rStrm );   break;
                case BIFF2_ID_FORMULA:      importFormula( rStrm );     break;
                case BIFF2_ID_ROW:          importRow( rStrm );         break;
            }
            break;

            case BIFF3: switch( nRecId )
            {
                case BIFF3_ID_DATATABLE:    importDataTable( rStrm );   break;
                case BIFF3_ID_FORMULA:      importFormula( rStrm );     break;
                case BIFF3_ID_ROW:          importRow( rStrm );         break;
            }
            break;

            case BIFF4: switch( nRecId )
            {
                case BIFF3_ID_DATATABLE:    importDataTable( rStrm );   break;
                case BIFF4_ID_FORMULA:      importFormula( rStrm );     break;
                case BIFF3_ID_ROW:          importRow( rStrm );         break;
            }
            break;

            case BIFF5: switch( nRecId )
            {
                case BIFF3_ID_DATATABLE:    importDataTable( rStrm );   break;
                case BIFF3_ID_FORMULA:
                case BIFF4_ID_FORMULA:
                case BIFF5_ID_FORMULA:      importFormula( rStrm );     break;
                case BIFF_ID_MULTBLANK:     importMultBlank( rStrm );   break;
                case BIFF_ID_MULTRK:        importMultRk( rStrm );      break;
                case BIFF3_ID_ROW:          importRow( rStrm );         break;
                case BIFF_ID_RSTRING:       importLabel( rStrm );       break;
                case BIFF_ID_SHAREDFMLA:    importSharedFmla( rStrm );  break;
            }
            break;

            case BIFF8: switch( nRecId )
            {
                case BIFF3_ID_DATATABLE:    importDataTable( rStrm );   break;
                case BIFF3_ID_FORMULA:
                case BIFF4_ID_FORMULA:
                case BIFF5_ID_FORMULA:      importFormula( rStrm );     break;
                case BIFF_ID_LABELSST:      importLabelSst( rStrm );    break;
                case BIFF_ID_MULTBLANK:     importMultBlank( rStrm );   break;
                case BIFF_ID_MULTRK:        importMultRk( rStrm );      break;
                case BIFF3_ID_ROW:          importRow( rStrm );         break;
                case BIFF_ID_RSTRING:       importLabel( rStrm );       break;
                case BIFF_ID_SHAREDFMLA:    importSharedFmla( rStrm );  break;
            }
            break;

            case BIFF_UNKNOWN: break;
        }
    }
}

// private --------------------------------------------------------------------

void BiffSheetDataContext::setCurrCell( const BinAddress& rAddr )
{
    maCurrCell.reset();
    maCurrCell.mxCell = getCell( rAddr, &maCurrCell.maAddress );
}

void BiffSheetDataContext::importXfId( BiffInputStream& rStrm, bool bBiff2 )
{
    if( bBiff2 )
    {
        sal_uInt8 nBiff2XfId;
        rStrm >> nBiff2XfId;
        rStrm.skip( 2 );
        maCurrCell.mnXfId = nBiff2XfId & BIFF2_XF_MASK;
        if( maCurrCell.mnXfId == BIFF_XF_EXTENDED_IDS )
            maCurrCell.mnXfId = mnBiff2XfId;
    }
    else
    {
        maCurrCell.mnXfId = rStrm.readuInt16();
    }
}

void BiffSheetDataContext::importCellHeader( BiffInputStream& rStrm, bool bBiff2 )
{
    BinAddress aAddr;
    rStrm >> aAddr;
    setCurrCell( aAddr );
    importXfId( rStrm, bBiff2 );
}

void BiffSheetDataContext::importBlank( BiffInputStream& rStrm )
{
    importCellHeader( rStrm, rStrm.getRecId() == BIFF2_ID_BLANK );
    setCellFormat( maCurrCell );
}

void BiffSheetDataContext::importBoolErr( BiffInputStream& rStrm )
{
    importCellHeader( rStrm, rStrm.getRecId() == BIFF2_ID_BOOLERR );
    if( maCurrCell.mxCell.is() )
    {
        sal_uInt8 nValue, nType;
        rStrm >> nValue >> nType;
        switch( nType )
        {
            case BIFF_BOOLERR_BOOL:
                maCurrCell.mnCellType = XML_b;
                setBooleanCell( maCurrCell.mxCell, nValue != 0 );
                // #108770# set 'Standard' number format for all Boolean cells
                maCurrCell.mnNumFmtId = 0;
            break;
            case BIFF_BOOLERR_ERROR:
                maCurrCell.mnCellType = XML_e;
                setErrorCell( maCurrCell.mxCell, nValue );
            break;
            default:
                OSL_ENSURE( false, "BiffSheetDataContext::importBoolErr - unknown cell type" );
        }
    }
    setCellFormat( maCurrCell );
}

void BiffSheetDataContext::importFormula( BiffInputStream& rStrm )
{
    importCellHeader( rStrm, getBiff() == BIFF2 );
    maCurrCell.mnCellType = XML_n;
    Reference< XFormulaTokens > xTokens( maCurrCell.mxCell, UNO_QUERY );
    if( xTokens.is() )
    {
        rStrm.skip( mnFormulaIgnoreSize );
        ExtCellFormulaContext aContext( *this, xTokens, maCurrCell.maAddress );
        getFormulaParser().importFormula( aContext, rStrm );
    }
    setCellFormat( maCurrCell );
}

void BiffSheetDataContext::importInteger( BiffInputStream& rStrm )
{
    importCellHeader( rStrm, true );
    maCurrCell.mnCellType = XML_n;
    if( maCurrCell.mxCell.is() )
        maCurrCell.mxCell->setValue( rStrm.readuInt16() );
    setCellFormat( maCurrCell );
}

void BiffSheetDataContext::importLabel( BiffInputStream& rStrm )
{
    bool bBiff2Xf = rStrm.getRecId() == BIFF2_ID_LABEL;
    importCellHeader( rStrm, bBiff2Xf );
    maCurrCell.mnCellType = XML_inlineStr;
    Reference< XText > xText( maCurrCell.mxCell, UNO_QUERY );
    if( xText.is() )
    {
        /*  the deep secrets of BIFF type and record identifier...
            record id   BIFF    XF type     String type
            0x0004      2-7     3 byte      8-bit length, byte string
            0x0004      8       3 byte      16-bit length, unicode string
            0x0204      2-7     2 byte      16-bit length, byte string
            0x0204      8       2 byte      16-bit length, unicode string */

        RichString aString( *this );
        if( getBiff() == BIFF8 )
        {
            aString.importUniString( rStrm );
        }
        else
        {
            // #i63105# use text encoding from FONT record
            rtl_TextEncoding eTextEnc = getTextEncoding();
            if( const Font* pFont = getStyles().getFontFromCellXf( maCurrCell.mnXfId ).get() )
                eTextEnc = pFont->getFontEncoding();
            BiffStringFlags nFlags = bBiff2Xf ? BIFF_STR_8BITLENGTH : BIFF_STR_DEFAULT;
            setFlag( nFlags, BIFF_STR_EXTRAFONTS, rStrm.getRecId() == BIFF_ID_RSTRING );
            aString.importByteString( rStrm, eTextEnc, nFlags );
        }
        aString.finalizeImport();
        aString.convert( xText, maCurrCell.mnXfId );
    }
    setCellFormat( maCurrCell );
}

void BiffSheetDataContext::importLabelSst( BiffInputStream& rStrm )
{
    importCellHeader( rStrm, false );
    maCurrCell.mnCellType = XML_s;
    if( maCurrCell.mxCell.is() )
        setSharedStringCell( maCurrCell.mxCell, rStrm.readInt32(), maCurrCell.mnXfId );
    setCellFormat( maCurrCell );
}

void BiffSheetDataContext::importMultBlank( BiffInputStream& rStrm )
{
    BinAddress aAddr;
    for( rStrm >> aAddr; rStrm.getRecLeft() > 2; ++aAddr.mnCol )
    {
        setCurrCell( aAddr );
        importXfId( rStrm, false );
        setCellFormat( maCurrCell );
    }
}

void BiffSheetDataContext::importMultRk( BiffInputStream& rStrm )
{
    BinAddress aAddr;
    for( rStrm >> aAddr; rStrm.getRecLeft() > 2; ++aAddr.mnCol )
    {
        setCurrCell( aAddr );
        maCurrCell.mnCellType = XML_n;
        importXfId( rStrm, false );
        sal_Int32 nRkValue = rStrm.readInt32();
        if( maCurrCell.mxCell.is() )
            maCurrCell.mxCell->setValue( BiffHelper::calcDoubleFromRk( nRkValue ) );
        setCellFormat( maCurrCell );
    }
}

void BiffSheetDataContext::importNumber( BiffInputStream& rStrm )
{
    importCellHeader( rStrm, rStrm.getRecId() == BIFF2_ID_NUMBER );
    maCurrCell.mnCellType = XML_n;
    if( maCurrCell.mxCell.is() )
        maCurrCell.mxCell->setValue( rStrm.readDouble() );
    setCellFormat( maCurrCell );
}

void BiffSheetDataContext::importRk( BiffInputStream& rStrm )
{
    importCellHeader( rStrm, false );
    maCurrCell.mnCellType = XML_n;
    if( maCurrCell.mxCell.is() )
        maCurrCell.mxCell->setValue( BiffHelper::calcDoubleFromRk( rStrm.readInt32() ) );
    setCellFormat( maCurrCell );
}

void BiffSheetDataContext::importRow( BiffInputStream& rStrm )
{
    OoxRowData aData;

    sal_uInt16 nRow, nHeight;
    rStrm >> nRow;
    rStrm.skip( 4 );
    rStrm >> nHeight;
    if( getBiff() == BIFF2 )
    {
        aData.mbCustomFormat = rStrm.skip( 2 ).readuInt8() == BIFF2_ROW_CUSTOMFORMAT;
        if( aData.mbCustomFormat )
            aData.mnXfId = rStrm.skip( 5 ).readuInt16();
    }
    else
    {
        sal_uInt32 nFlags = rStrm.skip( 4 ).readuInt32();
        aData.mnXfId = extractValue< sal_Int32 >( nFlags, 16, 12 );
        aData.mnLevel = extractValue< sal_Int32 >( nFlags, 0, 3 );
        aData.mbCustomFormat = getFlag( nFlags, BIFF_ROW_CUSTOMFORMAT );
        aData.mbCustomHeight = getFlag( nFlags, BIFF_ROW_CUSTOMHEIGHT );
        aData.mbShowPhonetic = getFlag( nFlags, BIFF_ROW_SHOWPHONETIC );
        aData.mbHidden = getFlag( nFlags, BIFF_ROW_HIDDEN );
        aData.mbCollapsed = getFlag( nFlags, BIFF_ROW_COLLAPSED );
        aData.mbThickTop = getFlag( nFlags, BIFF_ROW_THICKTOP );
        aData.mbThickBottom = getFlag( nFlags, BIFF_ROW_THICKBOTTOM );
    }

    // row index is 0-based in BIFF, but OoxRowData expects 1-based
    aData.mnFirstRow = aData.mnLastRow = nRow + 1;
    // row height is in twips in BIFF, convert to points
    aData.mfHeight = (nHeight & BIFF_ROW_HEIGHTMASK) / 20.0;
    // set row properties in the current sheet
    setRowData( aData );
}

void BiffSheetDataContext::importArray( BiffInputStream& rStrm )
{
    BinRange aRange;
    aRange.read( rStrm, false );    // columns always 8-bit
    CellRangeAddress aArrayRange;
    Reference< XCellRange > xRange = getCellRange( aRange, &aArrayRange );
    Reference< XArrayFormulaTokens > xTokens( xRange, UNO_QUERY );
    if( xRange.is() && xTokens.is() )
    {
        rStrm.skip( mnArrayIgnoreSize );
        ArrayFormulaContext aContext( xTokens, aArrayRange );
        getFormulaParser().importFormula( aContext, rStrm );
    }
}

void BiffSheetDataContext::importSharedFmla( BiffInputStream& rStrm )
{
    getSharedFormulas().importSharedFmla( rStrm, maCurrCell.maAddress );
}

void BiffSheetDataContext::importDataTable( BiffInputStream& rStrm )
{
    BinRange aRange;
    aRange.read( rStrm, false );    // columns always 8-bit
    CellRangeAddress aTableRange;
    if( getAddressConverter().convertToCellRange( aTableRange, aRange, getSheetIndex(), true ) )
    {
        OoxDataTableData aTableData;
        BinAddress aRef1, aRef2;
        switch( rStrm.getRecId() )
        {
            case BIFF2_ID_DATATABLE:
                rStrm.skip( 1 );
                aTableData.mbRowTable = rStrm.readuInt8() != 0;
                aTableData.mb2dTable = false;
                rStrm >> aRef1;
            break;
            case BIFF2_ID_DATATABLE2:
                rStrm.skip( 2 );
                aTableData.mb2dTable = true;
                rStrm >> aRef1 >> aRef2;
            break;
            case BIFF3_ID_DATATABLE:
            {
                sal_uInt16 nFlags;
                rStrm >> nFlags >> aRef1 >> aRef2;
                aTableData.mbRowTable = getFlag( nFlags, BIFF_DATATABLE_ROW );
                aTableData.mb2dTable = getFlag( nFlags, BIFF_DATATABLE_2D );
                aTableData.mbRef1Deleted = getFlag( nFlags, BIFF_DATATABLE_REF1DEL );
                aTableData.mbRef2Deleted = getFlag( nFlags, BIFF_DATATABLE_REF2DEL );
            }
            break;
            default:
                OSL_ENSURE( false, "BiffSheetDataContext::importDataTable - unknown record id" );
        }
        aTableData.maRef1 = FormulaProcessorBase::generateAddress2dString( aRef1, false );
        aTableData.maRef2 = FormulaProcessorBase::generateAddress2dString( aRef2, false );
        setTableOperation( aTableRange, aTableData );
    }
}

// ============================================================================

BiffExternalSheetDataContext::BiffExternalSheetDataContext(
        const WorkbookHelper& rHelper, WorksheetType eSheetType, sal_Int32 nSheet ) :
    WorksheetHelperRoot( rHelper, ISegmentProgressBarRef(), eSheetType, nSheet )
{
}

void BiffExternalSheetDataContext::importCrn( BiffInputStream& rStrm )
{
    sal_uInt8 nCol2, nCol1;
    sal_uInt16 nRow;
    rStrm >> nCol2 >> nCol1 >> nRow;
    bool bLoop = true;
    for( BinAddress aAddr( nCol1, nRow ); bLoop && rStrm.isValid() && (aAddr.mnCol <= nCol2); ++aAddr.mnCol )
    {
        Reference< XCell > xCell = getCell( aAddr );
        bLoop = xCell.is();
        if( bLoop ) switch( rStrm.readuInt8() )
        {
            case BIFF_DATATYPE_EMPTY:
                rStrm.skip( 8 );
                setEmptyStringCell( xCell );
            break;
            case BIFF_DATATYPE_DOUBLE:
                xCell->setValue( rStrm.readDouble() );
            break;
            case BIFF_DATATYPE_STRING:
            {
                OUString aText = (getBiff() == BIFF8) ? rStrm.readUniString() : rStrm.readByteString( false, getTextEncoding() );
                setStringCell( xCell, aText, true );
            }
            break;
            case BIFF_DATATYPE_BOOL:
                setBooleanCell( xCell, rStrm.readuInt8() != 0 );
                rStrm.skip( 7 );
            break;
            case BIFF_DATATYPE_ERROR:
                setErrorCell( xCell, rStrm.readuInt8() );
                rStrm.skip( 7 );
            break;
            default:
                OSL_ENSURE( false, "BiffExternalSheetDataContext::importCrn - unknown data type" );
                bLoop = false;
        }
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

