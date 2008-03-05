/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: worksheetfragment.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:09:27 $
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

#include "oox/xls/worksheetfragment.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/core/relations.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/autofiltercontext.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/condformatcontext.hxx"
#include "oox/xls/externallinkbuffer.hxx"
#include "oox/xls/pagesettings.hxx"
#include "oox/xls/pivottablefragment.hxx"
#include "oox/xls/querytablefragment.hxx"
#include "oox/xls/sheetdatacontext.hxx"
#include "oox/xls/tablefragment.hxx"
#include "oox/xls/viewsettings.hxx"
#include "oox/xls/workbooksettings.hxx"
#include "oox/xls/worksheetsettings.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::oox::core::RecordInfo;
using ::oox::core::Relations;
using ::oox::core::RelationsRef;

namespace oox {
namespace xls {

// ============================================================================

namespace {

const sal_uInt16 BIFF_COLINFO_HIDDEN        = 0x0001;
const sal_uInt16 BIFF_COLINFO_SHOWPHONETIC  = 0x0008;
const sal_uInt16 BIFF_COLINFO_COLLAPSED     = 0x1000;

const sal_uInt16 BIFF_DEFROW_CUSTOMHEIGHT   = 0x0001;
const sal_uInt16 BIFF_DEFROW_HIDDEN         = 0x0002;
const sal_uInt16 BIFF_DEFROW_THICKTOP       = 0x0004;
const sal_uInt16 BIFF_DEFROW_THICKBOTTOM    = 0x0008;
const sal_uInt16 BIFF2_DEFROW_DEFHEIGHT     = 0x8000;
const sal_uInt16 BIFF2_DEFROW_MASK          = 0x7FFF;

const sal_uInt32 BIFF_DATAVAL_STRINGLIST    = 0x00000080;
const sal_uInt32 BIFF_DATAVAL_ALLOWBLANK    = 0x00000100;
const sal_uInt32 BIFF_DATAVAL_NODROPDOWN    = 0x00000200;
const sal_uInt32 BIFF_DATAVAL_SHOWINPUT     = 0x00040000;
const sal_uInt32 BIFF_DATAVAL_SHOWERROR     = 0x00080000;

const sal_uInt32 BIFF_HYPERLINK_TARGET      = 0x00000001;   /// File name or URL.
const sal_uInt32 BIFF_HYPERLINK_ABS         = 0x00000002;   /// Absolute path.
const sal_uInt32 BIFF_HYPERLINK_DISPLAY     = 0x00000014;   /// Display string.
const sal_uInt32 BIFF_HYPERLINK_LOC         = 0x00000008;   /// Target location.
const sal_uInt32 BIFF_HYPERLINK_FRAME       = 0x00000080;   /// Target frame.
const sal_uInt32 BIFF_HYPERLINK_UNC         = 0x00000100;   /// UNC path.

} // namespace

// ============================================================================

OoxWorksheetFragment::OoxWorksheetFragment( const WorkbookHelper& rHelper,
        const OUString& rFragmentPath, ISegmentProgressBarRef xProgressBar, WorksheetType eSheetType, sal_Int32 nSheet ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath, xProgressBar, eSheetType, nSheet )
{
    // import data tables related to this worksheet
    RelationsRef xTableRels = getRelations().getRelationsFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "table" ) );
    for( Relations::const_iterator aIt = xTableRels->begin(), aEnd = xTableRels->end(); aIt != aEnd; ++aIt )
        importOoxFragment( new OoxTableFragment( *this, getFragmentPathFromTarget( aIt->second.maTarget ) ) );
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper OoxWorksheetFragment::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT: switch( getSheetType() )
        {
            case SHEETTYPE_WORKSHEET:   return  (nElement == XLS_TOKEN( worksheet ));
            case SHEETTYPE_CHARTSHEET:  return  false;
            case SHEETTYPE_MACROSHEET:  return  (nElement == XM_TOKEN( macrosheet ));
            case SHEETTYPE_DIALOGSHEET: return  false;
            case SHEETTYPE_MODULESHEET: return  false;
        }
        break;

        case XLS_TOKEN( worksheet ):
        case XM_TOKEN( macrosheet ):
            switch( nElement )
            {
                case XLS_TOKEN( sheetData ):
                    return new OoxSheetDataContext( *this );
                case XLS_TOKEN( autoFilter ):
                    return new OoxAutoFilterContext( *this );
                case XLS_TOKEN( conditionalFormatting ):
                    return new OoxCondFormatContext( *this );
            }
            return  (nElement == XLS_TOKEN( sheetPr )) ||
                    (nElement == XLS_TOKEN( dimension )) ||
                    (nElement == XLS_TOKEN( sheetViews )) ||
                    (nElement == XLS_TOKEN( sheetFormatPr )) ||
                    (nElement == XLS_TOKEN( cols )) ||
                    (nElement == XLS_TOKEN( sheetProtection )) ||
                    (nElement == XLS_TOKEN( mergeCells )) ||
                    (nElement == XLS_TOKEN( phoneticPr )) ||
                    (nElement == XLS_TOKEN( dataValidations )) ||
                    (nElement == XLS_TOKEN( hyperlinks )) ||
                    (nElement == XLS_TOKEN( printOptions )) ||
                    (nElement == XLS_TOKEN( pageMargins )) ||
                    (nElement == XLS_TOKEN( pageSetup )) ||
                    (nElement == XLS_TOKEN( headerFooter )) ||
                    (nElement == XLS_TOKEN( picture )) ||
                    (nElement == XLS_TOKEN( rowBreaks )) ||
                    (nElement == XLS_TOKEN( colBreaks )) ||
                    (nElement == XLS_TOKEN( drawing ));

        case XLS_TOKEN( sheetPr ):
            return  (nElement == XLS_TOKEN( tabColor )) ||
                    (nElement == XLS_TOKEN( outlinePr )) ||
                    (nElement == XLS_TOKEN( pageSetUpPr ));

        case XLS_TOKEN( sheetViews ):
            return  (nElement == XLS_TOKEN( sheetView ));
        case XLS_TOKEN( sheetView ):
            return  (nElement == XLS_TOKEN( pane )) ||
                    (nElement == XLS_TOKEN( selection ));

        case XLS_TOKEN( cols ):
            return  (nElement == XLS_TOKEN( col ));

        case XLS_TOKEN( mergeCells ):
            return  (nElement == XLS_TOKEN( mergeCell ));

        case XLS_TOKEN( dataValidations ):
            return  (nElement == XLS_TOKEN( dataValidation ));
        case XLS_TOKEN( dataValidation ):
            return  (nElement == XLS_TOKEN( formula1 )) ||
                    (nElement == XLS_TOKEN( formula2 ));

        case XLS_TOKEN( hyperlinks ):
            return  (nElement == XLS_TOKEN( hyperlink ));

        case XLS_TOKEN( headerFooter ):
            return  (nElement == XLS_TOKEN( firstHeader )) ||
                    (nElement == XLS_TOKEN( firstFooter )) ||
                    (nElement == XLS_TOKEN( oddHeader )) ||
                    (nElement == XLS_TOKEN( oddFooter )) ||
                    (nElement == XLS_TOKEN( evenHeader )) ||
                    (nElement == XLS_TOKEN( evenFooter ));
        case XLS_TOKEN( rowBreaks ):
        case XLS_TOKEN( colBreaks ):
            return  (nElement == XLS_TOKEN( brk ));
    }
    return false;
}

void OoxWorksheetFragment::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( sheetPr ):          getWorksheetSettings().importSheetPr( rAttribs );               break;
        case XLS_TOKEN( tabColor ):         getWorksheetSettings().importTabColor( rAttribs );              break;
        case XLS_TOKEN( outlinePr ):        getWorksheetSettings().importOutlinePr( rAttribs );             break;
        case XLS_TOKEN( pageSetUpPr ):      importPageSetUpPr( rAttribs );                                  break;
        case XLS_TOKEN( dimension ):        importDimension( rAttribs );                                    break;
        case XLS_TOKEN( sheetView ):        getSheetViewSettings().importSheetView( rAttribs );             break;
        case XLS_TOKEN( pane ):             getSheetViewSettings().importPane( rAttribs );                  break;
        case XLS_TOKEN( selection ):        getSheetViewSettings().importSelection( rAttribs );             break;
        case XLS_TOKEN( sheetFormatPr ):    importSheetFormatPr( rAttribs );                                break;
        case XLS_TOKEN( col ):              importCol( rAttribs );                                          break;
        case XLS_TOKEN( sheetProtection ):  getWorksheetSettings().importSheetProtection( rAttribs );       break;
        case XLS_TOKEN( mergeCell ):        importMergeCell( rAttribs );                                    break;
        case XLS_TOKEN( phoneticPr ):       getWorksheetSettings().importPhoneticPr( rAttribs );            break;
        case XLS_TOKEN( dataValidation ):   importDataValidation( rAttribs );                               break;
        case XLS_TOKEN( hyperlink ):        importHyperlink( rAttribs );                                    break;
        case XLS_TOKEN( printOptions ):     getPageSettings().importPrintOptions( rAttribs );               break;
        case XLS_TOKEN( pageMargins ):      getPageSettings().importPageMargins( rAttribs );                break;
        case XLS_TOKEN( pageSetup ):        getPageSettings().importPageSetup( getRelations(), rAttribs );  break;
        case XLS_TOKEN( headerFooter ):     getPageSettings().importHeaderFooter( rAttribs );               break;
        case XLS_TOKEN( picture ):          getPageSettings().importPicture( getRelations(), rAttribs );    break;
        case XLS_TOKEN( brk ):              importBrk( rAttribs );                                          break;
        case XLS_TOKEN( drawing ):          importDrawing( rAttribs );                                      break;
    }
}

namespace {

ApiTokenSequence lclImportDataValFormula( FormulaParser& rParser, const OUString& rFormula, const CellAddress& rBaseAddress )
{
    TokensFormulaContext aContext( true, false );
    aContext.setBaseAddress( rBaseAddress );
    rParser.importFormula( aContext, rFormula );
    return aContext.getTokens();
}

} // namespace

void OoxWorksheetFragment::onEndElement( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( firstHeader ):
        case XLS_TOKEN( firstFooter ):
        case XLS_TOKEN( oddHeader ):
        case XLS_TOKEN( oddFooter ):
        case XLS_TOKEN( evenHeader ):
        case XLS_TOKEN( evenFooter ):
            getPageSettings().importHeaderFooterCharacters( rChars, getCurrentElement() );
        break;
        case XLS_TOKEN( formula1 ):
            if( mxValData.get() )
            {
                mxValData->maTokens1 = lclImportDataValFormula(
                    getFormulaParser(), rChars, mxValData->maRanges.getBaseAddress() );
                // process string list of a list validation (convert to list of string tokens)
                if( mxValData->mnType == XML_list )
                    getFormulaParser().convertStringToStringList( mxValData->maTokens1, ',', true );
            }
        break;
        case XLS_TOKEN( formula2 ):
            if( mxValData.get() )
                mxValData->maTokens2 = lclImportDataValFormula(
                    getFormulaParser(), rChars, mxValData->maRanges.getBaseAddress() );
        break;
        case XLS_TOKEN( dataValidation ):
            if( mxValData.get() )
                setValidation( *mxValData );
            mxValData.reset();
        break;
    }
}

ContextWrapper OoxWorksheetFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nRecId == OOBIN_ID_WORKSHEET);
        case OOBIN_ID_WORKSHEET:
            switch( nRecId )
            {
                case OOBIN_ID_SHEETDATA:
                    return new OoxSheetDataContext( *this );
                case OOBIN_ID_CONDFORMATTING:
                    return new OoxCondFormatContext( *this );
            }
            return  (nRecId == OOBIN_ID_SHEETPR) ||
                    (nRecId == OOBIN_ID_DIMENSION) ||
                    (nRecId == OOBIN_ID_SHEETFORMATPR) ||
                    (nRecId == OOBIN_ID_SHEETVIEWS) ||
                    (nRecId == OOBIN_ID_COLS) ||
                    (nRecId == OOBIN_ID_MERGECELLS) ||
                    (nRecId == OOBIN_ID_HYPERLINK) ||
                    (nRecId == OOBIN_ID_DATAVALIDATIONS) ||
                    (nRecId == OOBIN_ID_PAGEMARGINS) ||
                    (nRecId == OOBIN_ID_PAGESETUP) ||
                    (nRecId == OOBIN_ID_PRINTOPTIONS) ||
                    (nRecId == OOBIN_ID_HEADERFOOTER) ||
                    (nRecId == OOBIN_ID_PICTURE) ||
                    (nRecId == OOBIN_ID_ROWBREAKS) ||
                    (nRecId == OOBIN_ID_COLBREAKS) ||
                    (nRecId == OOBIN_ID_SHEETPROTECTION) ||
                    (nRecId == OOBIN_ID_PHONETICPR) ||
                    (nRecId == OOBIN_ID_DRAWING);
        case OOBIN_ID_SHEETVIEWS:
            return  (nRecId == OOBIN_ID_SHEETVIEW);
        case OOBIN_ID_SHEETVIEW:
            return  (nRecId == OOBIN_ID_PANE) ||
                    (nRecId == OOBIN_ID_SELECTION);
        case OOBIN_ID_COLS:
            return  (nRecId == OOBIN_ID_COL);
        case OOBIN_ID_MERGECELLS:
            return  (nRecId == OOBIN_ID_MERGECELL);
        case OOBIN_ID_DATAVALIDATIONS:
            return  (nRecId == OOBIN_ID_DATAVALIDATION);
        case OOBIN_ID_ROWBREAKS:
        case OOBIN_ID_COLBREAKS:
            return  (nRecId == OOBIN_ID_BRK);
    }
    return false;
}

void OoxWorksheetFragment::onStartRecord( RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_SHEETPR:          getWorksheetSettings().importSheetPr( rStrm );              break;
        case OOBIN_ID_DIMENSION:        importDimension( rStrm );                                   break;
        case OOBIN_ID_SHEETPROTECTION:  getWorksheetSettings().importSheetProtection( rStrm );      break;
        case OOBIN_ID_PHONETICPR:       getWorksheetSettings().importPhoneticPr( rStrm );           break;
        case OOBIN_ID_SHEETFORMATPR:    importSheetFormatPr( rStrm );                               break;
        case OOBIN_ID_SHEETVIEW:        getSheetViewSettings().importSheetView( rStrm );            break;
        case OOBIN_ID_PANE:             getSheetViewSettings().importPane( rStrm );                 break;
        case OOBIN_ID_SELECTION:        getSheetViewSettings().importSelection( rStrm );            break;
        case OOBIN_ID_COL:              importCol( rStrm );                                         break;
        case OOBIN_ID_MERGECELL:        importMergeCell( rStrm );                                   break;
        case OOBIN_ID_HYPERLINK:        importHyperlink( rStrm );                                   break;
        case OOBIN_ID_DATAVALIDATION:   importDataValidation( rStrm );                              break;
        case OOBIN_ID_PAGEMARGINS:      getPageSettings().importPageMargins( rStrm );               break;
        case OOBIN_ID_PAGESETUP:        getPageSettings().importPageSetup( getRelations(), rStrm ); break;
        case OOBIN_ID_PRINTOPTIONS:     getPageSettings().importPrintOptions( rStrm );              break;
        case OOBIN_ID_HEADERFOOTER:     getPageSettings().importHeaderFooter( rStrm );              break;
        case OOBIN_ID_PICTURE:          getPageSettings().importPicture( getRelations(), rStrm );   break;
        case OOBIN_ID_BRK:              importBrk( rStrm );                                         break;
        case OOBIN_ID_DRAWING:          importDrawing( rStrm );                                     break;
    }
}

// oox.core.FragmentHandler2 interface ----------------------------------------

const RecordInfo* OoxWorksheetFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { OOBIN_ID_CFRULE,              OOBIN_ID_CFRULE + 1             },
        { OOBIN_ID_COLBREAKS,           OOBIN_ID_COLBREAKS + 1          },
        { OOBIN_ID_COLORSCALE,          OOBIN_ID_COLORSCALE + 1         },
        { OOBIN_ID_COLS,                OOBIN_ID_COLS + 1               },
        { OOBIN_ID_CONDFORMATTING,      OOBIN_ID_CONDFORMATTING + 1     },
        { OOBIN_ID_CUSTOMSHEETVIEW,     OOBIN_ID_CUSTOMSHEETVIEW + 1    },
        { OOBIN_ID_CUSTOMSHEETVIEWS,    OOBIN_ID_CUSTOMSHEETVIEWS + 3   },
        { OOBIN_ID_DATABAR,             OOBIN_ID_DATABAR + 1            },
        { OOBIN_ID_DATAVALIDATIONS,     OOBIN_ID_DATAVALIDATIONS + 1    },
        { OOBIN_ID_HEADERFOOTER,        OOBIN_ID_HEADERFOOTER + 1       },
        { OOBIN_ID_ICONSET,             OOBIN_ID_ICONSET + 1            },
        { OOBIN_ID_MERGECELLS,          OOBIN_ID_MERGECELLS + 1         },
        { OOBIN_ID_ROW,                 -1                              },
        { OOBIN_ID_ROWBREAKS,           OOBIN_ID_ROWBREAKS + 1          },
        { OOBIN_ID_SHEETDATA,           OOBIN_ID_SHEETDATA + 1          },
        { OOBIN_ID_SHEETVIEW,           OOBIN_ID_SHEETVIEW + 1          },
        { OOBIN_ID_SHEETVIEWS,          OOBIN_ID_SHEETVIEWS + 1         },
        { OOBIN_ID_TABLEPARTS,          OOBIN_ID_TABLEPARTS + 2         },
        { OOBIN_ID_WORKSHEET,           OOBIN_ID_WORKSHEET + 1          },
        { -1,                           -1                              }
    };
    return spRecInfos;
}

void OoxWorksheetFragment::initializeImport()
{
    // initial processing in base class WorksheetHelper
    initializeWorksheetImport();

    // import query table fragments related to this worksheet
    RelationsRef xQueryRels = getRelations().getRelationsFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "queryTable" ) );
    for( Relations::const_iterator aIt = xQueryRels->begin(), aEnd = xQueryRels->end(); aIt != aEnd; ++aIt )
        importOoxFragment( new OoxQueryTableFragment( *this, getFragmentPathFromTarget( aIt->second.maTarget ) ) );

    // import pivot table fragments related to this worksheet
    RelationsRef xPivotRels = getRelations().getRelationsFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "pivotTable" ) );
    for( Relations::const_iterator aIt = xPivotRels->begin(), aEnd = xPivotRels->end(); aIt != aEnd; ++aIt )
        importOoxFragment( new OoxPivotTableFragment( *this, getFragmentPathFromTarget( aIt->second.maTarget ) ) );
}

void OoxWorksheetFragment::finalizeImport()
{
    // final processing in base class WorksheetHelper
    finalizeWorksheetImport();
}

// private --------------------------------------------------------------------

void OoxWorksheetFragment::importPageSetUpPr( const AttributeList& rAttribs )
{
    // for whatever reason, this flag is still stored separated from the page settings
    getPageSettings().setFitToPagesMode( rAttribs.getBool( XML_fitToPage, false ) );
}

void OoxWorksheetFragment::importDimension( const AttributeList& rAttribs )
{
    CellRangeAddress aRange;
    getAddressConverter().convertToCellRangeUnchecked( aRange, rAttribs.getString( XML_ref ), getSheetIndex() );
    setDimension( aRange );
}

void OoxWorksheetFragment::importSheetFormatPr( const AttributeList& rAttribs )
{
    // default column settings
    setBaseColumnWidth( rAttribs.getInteger( XML_baseColWidth, 8 ) );
    setDefaultColumnWidth( rAttribs.getDouble( XML_defaultColWidth, 0.0 ) );
    // default row settings
    setDefaultRowSettings(
        rAttribs.getDouble( XML_defaultRowHeight, 0.0 ),
        rAttribs.getBool( XML_customHeight, false ),
        rAttribs.getBool( XML_zeroHeight, false ),
        rAttribs.getBool( XML_thickTop, false ),
        rAttribs.getBool( XML_thickBottom, false ) );
}

void OoxWorksheetFragment::importCol( const AttributeList& rAttribs )
{
    OoxColumnData aData;
    aData.mnFirstCol     = rAttribs.getInteger( XML_min, -1 );
    aData.mnLastCol      = rAttribs.getInteger( XML_max, -1 );
    aData.mfWidth        = rAttribs.getDouble( XML_width, 0.0 );
    aData.mnXfId         = rAttribs.getInteger( XML_style, -1 );
    aData.mnLevel        = rAttribs.getInteger( XML_outlineLevel, 0 );
    aData.mbShowPhonetic = rAttribs.getBool( XML_phonetic, false );
    aData.mbHidden       = rAttribs.getBool( XML_hidden, false );
    aData.mbCollapsed    = rAttribs.getBool( XML_collapsed, false );
    // set column properties in the current sheet
    setColumnData( aData );
}

void OoxWorksheetFragment::importMergeCell( const AttributeList& rAttribs )
{
    CellRangeAddress aRange;
    if( getAddressConverter().convertToCellRange( aRange, rAttribs.getString( XML_ref ), getSheetIndex(), true ) )
        setMergedRange( aRange );
}

void OoxWorksheetFragment::importDataValidation( const AttributeList& rAttribs )
{
    mxValData.reset( new OoxValidationData );
    getAddressConverter().convertToCellRangeList( mxValData->maRanges, rAttribs.getString( XML_sqref ), getSheetIndex(), true );
    mxValData->maInputTitle   = rAttribs.getString( XML_promptTitle );
    mxValData->maInputMessage = rAttribs.getString( XML_prompt );
    mxValData->maErrorTitle   = rAttribs.getString( XML_errorTitle );
    mxValData->maErrorMessage = rAttribs.getString( XML_error );
    mxValData->mnType         = rAttribs.getToken( XML_type, XML_none );
    mxValData->mnOperator     = rAttribs.getToken( XML_operator, XML_between );
    mxValData->mnErrorStyle   = rAttribs.getToken( XML_errorStyle, XML_stop );
    mxValData->mbShowInputMsg = rAttribs.getBool( XML_showInputMessage, false );
    mxValData->mbShowErrorMsg = rAttribs.getBool( XML_showErrorMessage, false );
    /*  The attribute showDropDown@dataValidation is in fact a "suppress
        dropdown" flag, as it was in the BIFF format! ECMA specification
        and attribute name are plain wrong! */
    mxValData->mbNoDropDown   = rAttribs.getBool( XML_showDropDown, false );
    mxValData->mbAllowBlank   = rAttribs.getBool( XML_allowBlank, false );
}

void OoxWorksheetFragment::importHyperlink( const AttributeList& rAttribs )
{
    OoxHyperlinkData aData;
    if( getAddressConverter().convertToCellRange( aData.maRange, rAttribs.getString( XML_ref ), getSheetIndex(), true ) )
    {
        aData.maTarget   = getRelations().getTargetFromRelId( rAttribs.getString( R_TOKEN( id ) ) );
        aData.maLocation = rAttribs.getString( XML_location );
        aData.maDisplay  = rAttribs.getString( XML_display );
        aData.maTooltip  = rAttribs.getString( XML_tooltip );
        setHyperlink( aData );
    }
}

void OoxWorksheetFragment::importBrk( const AttributeList& rAttribs )
{
    OoxPageBreakData aData;
    aData.mnColRow = rAttribs.getInteger( XML_id, 0 );
    aData.mnMin    = rAttribs.getInteger( XML_id, 0 );
    aData.mnMax    = rAttribs.getInteger( XML_id, 0 );
    aData.mbManual = rAttribs.getBool( XML_man, false );
    switch( getPreviousElement() )
    {
        case XLS_TOKEN( rowBreaks ):    setPageBreak( aData, true );    break;
        case XLS_TOKEN( colBreaks ):    setPageBreak( aData, false );   break;
    }
}

void OoxWorksheetFragment::importDrawing( const AttributeList& rAttribs )
{
    setDrawingPath( getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ) ) ) );
}

void OoxWorksheetFragment::importDimension( RecordInputStream& rStrm )
{
    BinRange aBinRange;
    aBinRange.read( rStrm );
    CellRangeAddress aRange;
    getAddressConverter().convertToCellRangeUnchecked( aRange, aBinRange, getSheetIndex() );
    setDimension( aRange );
}

void OoxWorksheetFragment::importSheetFormatPr( RecordInputStream& rStrm )
{
    sal_Int32 nDefaultWidth;
    sal_uInt16 nBaseWidth, nDefaultHeight, nFlags;
    rStrm >> nDefaultWidth >> nBaseWidth >> nDefaultHeight >> nFlags;

    // base column with
    setBaseColumnWidth( nBaseWidth );
    // default width is stored as 1/256th of a character in OOBIN, convert to entire character
    setDefaultColumnWidth( static_cast< double >( nDefaultWidth ) / 256.0 );
    // row height is in twips in OOBIN, convert to points; equal flags in BIFF and OOBIN
    setDefaultRowSettings(
        nDefaultHeight / 20.0,
        getFlag( nFlags, BIFF_DEFROW_CUSTOMHEIGHT ),
        getFlag( nFlags, BIFF_DEFROW_HIDDEN ),
        getFlag( nFlags, BIFF_DEFROW_THICKTOP ),
        getFlag( nFlags, BIFF_DEFROW_THICKBOTTOM ) );
}

void OoxWorksheetFragment::importCol( RecordInputStream& rStrm )
{
    OoxColumnData aData;

    sal_Int32 nWidth;
    sal_uInt16 nFlags;
    rStrm >> aData.mnFirstCol >> aData.mnLastCol >> nWidth >> aData.mnXfId >> nFlags;

    // column indexes are 0-based in OOBIN, but OoxColumnData expects 1-based
    ++aData.mnFirstCol;
    ++aData.mnLastCol;
    // width is stored as 1/256th of a character in OOBIN, convert to entire character
    aData.mfWidth        = static_cast< double >( nWidth ) / 256.0;
    // equal flags in BIFF and OOBIN
    aData.mnLevel        = extractValue< sal_Int32 >( nFlags, 8, 3 );
    aData.mbShowPhonetic = getFlag( nFlags, BIFF_COLINFO_SHOWPHONETIC );
    aData.mbHidden       = getFlag( nFlags, BIFF_COLINFO_HIDDEN );
    aData.mbCollapsed    = getFlag( nFlags, BIFF_COLINFO_COLLAPSED );
    // set column properties in the current sheet
    setColumnData( aData );
}

void OoxWorksheetFragment::importMergeCell( RecordInputStream& rStrm )
{
    BinRange aBinRange;
    rStrm >> aBinRange;
    CellRangeAddress aRange;
    if( getAddressConverter().convertToCellRange( aRange, aBinRange, getSheetIndex(), true ) )
        setMergedRange( aRange );
}

void OoxWorksheetFragment::importHyperlink( RecordInputStream& rStrm )
{
    BinRange aBinRange;
    rStrm >> aBinRange;
    OoxHyperlinkData aData;
    if( getAddressConverter().convertToCellRange( aData.maRange, aBinRange, getSheetIndex(), true ) )
    {
        aData.maTarget = getRelations().getTargetFromRelId( rStrm.readString() );
        rStrm >> aData.maLocation >> aData.maTooltip >> aData.maDisplay;
        setHyperlink( aData );
    }
}

void OoxWorksheetFragment::importDataValidation( RecordInputStream& rStrm )
{
    OoxValidationData aData;

    sal_uInt32 nFlags;
    BinRangeList aRanges;
    rStrm >> nFlags >> aRanges >> aData.maErrorTitle >> aData.maErrorMessage >> aData.maInputTitle >> aData.maInputMessage;

    // equal flags in BIFF and OOBIN
    aData.setBinType( extractValue< sal_uInt8 >( nFlags, 0, 4 ) );
    aData.setBinOperator( extractValue< sal_uInt8 >( nFlags, 20, 4 ) );
    aData.setBinErrorStyle( extractValue< sal_uInt8 >( nFlags, 4, 3 ) );
    aData.mbAllowBlank   = getFlag( nFlags, BIFF_DATAVAL_ALLOWBLANK );
    aData.mbNoDropDown   = getFlag( nFlags, BIFF_DATAVAL_NODROPDOWN );
    aData.mbShowInputMsg = getFlag( nFlags, BIFF_DATAVAL_SHOWINPUT );
    aData.mbShowErrorMsg = getFlag( nFlags, BIFF_DATAVAL_SHOWERROR );

    // cell range list
    getAddressConverter().convertToCellRangeList( aData.maRanges, aRanges, getSheetIndex(), true );

    // condition formula(s)
    FormulaParser& rParser = getFormulaParser();
    TokensFormulaContext aContext( true, false );
    aContext.setBaseAddress( aData.maRanges.getBaseAddress() );
    rParser.importFormula( aContext, rStrm );
    aData.maTokens1 = aContext.getTokens();
    rParser.importFormula( aContext, rStrm );
    aData.maTokens2 = aContext.getTokens();
    // process string list of a list validation (convert to list of string tokens)
    if( (aData.mnType == XML_list) && getFlag( nFlags, BIFF_DATAVAL_STRINGLIST ) )
        rParser.convertStringToStringList( aData.maTokens1, ',', true );

    // set validation data
    setValidation( aData );
}

void OoxWorksheetFragment::importBrk( RecordInputStream& rStrm )
{
    OoxPageBreakData aData;
    sal_Int32 nManual;
    rStrm >> aData.mnColRow >> aData.mnMin >> aData.mnMax >> nManual;
    aData.mbManual = nManual != 0;
    switch( getPreviousElement() )
    {
        case OOBIN_ID_ROWBREAKS:    setPageBreak( aData, true );    break;
        case OOBIN_ID_COLBREAKS:    setPageBreak( aData, false );   break;
    }
}

void OoxWorksheetFragment::importDrawing( RecordInputStream& rStrm )
{
    setDrawingPath( getFragmentPathFromRelId( rStrm.readString() ) );
}

// ============================================================================

BiffWorksheetFragment::BiffWorksheetFragment( const WorkbookHelper& rHelper, ISegmentProgressBarRef xProgressBar, WorksheetType eSheetType, sal_Int32 nSheet ) :
    BiffWorksheetFragmentBase( rHelper, xProgressBar, eSheetType, nSheet )
{
}

bool BiffWorksheetFragment::importFragment( BiffInputStream& rStrm )
{
    // initial processing in base class WorksheetHelper
    initializeWorksheetImport();

    // create a SheetDataContext object that implements cell import
    BiffSheetDataContext aSheetData( *this );

    WorkbookSettings& rWorkbookSett   = getWorkbookSettings();
    WorksheetSettings& rWorksheetSett = getWorksheetSettings();
    SheetViewSettings& rSheetViewSett = getSheetViewSettings();
    CondFormatBuffer& rCondFormats    = getCondFormats();
    PageSettings& rPageSett           = getPageSettings();

    // process all record in this sheet fragment
    while( rStrm.startNextRecord() && (rStrm.getRecId() != BIFF_ID_EOF) )
    {
        sal_uInt16 nRecId = rStrm.getRecId();

        if( isBofRecord( nRecId ) )
        {
            // skip unknown embedded fragments (BOF/EOF blocks)
            skipFragment( rStrm );
        }
        else
        {
            // cache core stream position to detect if record is already processed
            sal_Int64 nStrmPos = rStrm.getCoreStreamPos();

            switch( nRecId )
            {
                // records in all BIFF versions
                case BIFF_ID_BOTTOMMARGIN:      rPageSett.importBottomMargin( rStrm );      break;
                case BIFF_ID_CALCCOUNT:         rWorkbookSett.importCalcCount( rStrm );     break;
                case BIFF_ID_CALCMODE:          rWorkbookSett.importCalcMode( rStrm );      break;
                case BIFF_ID_DEFCOLWIDTH:       importDefColWidth( rStrm );                 break;
                case BIFF_ID_DELTA:             rWorkbookSett.importDelta( rStrm );         break;
                case BIFF2_ID_DIMENSION:        importDimension( rStrm );                   break;
                case BIFF3_ID_DIMENSION:        importDimension( rStrm );                   break;
                case BIFF_ID_FOOTER:            rPageSett.importFooter( rStrm );            break;
                case BIFF_ID_HEADER:            rPageSett.importHeader( rStrm );            break;
                case BIFF_ID_HORPAGEBREAKS:     importPageBreaks( rStrm, true );            break;
                case BIFF_ID_ITERATION:         rWorkbookSett.importIteration( rStrm );     break;
                case BIFF_ID_LEFTMARGIN:        rPageSett.importLeftMargin( rStrm );        break;
                case BIFF_ID_PANE:              rSheetViewSett.importPane( rStrm );         break;
                case BIFF_ID_PASSWORD:          rWorksheetSett.importPassword( rStrm );     break;
                case BIFF_ID_PRINTGRIDLINES:    rPageSett.importPrintGridLines( rStrm );    break;
                case BIFF_ID_PRINTHEADERS:      rPageSett.importPrintHeaders( rStrm );      break;
                case BIFF_ID_PROTECT:           rWorksheetSett.importProtect( rStrm );      break;
                case BIFF_ID_REFMODE:           rWorkbookSett.importRefMode( rStrm );       break;
                case BIFF_ID_RIGHTMARGIN:       rPageSett.importRightMargin( rStrm );       break;
                case BIFF_ID_SELECTION:         rSheetViewSett.importSelection( rStrm );    break;
                case BIFF_ID_TOPMARGIN:         rPageSett.importTopMargin( rStrm );         break;
                case BIFF_ID_VERPAGEBREAKS:     importPageBreaks( rStrm, false );           break;

                // BIFF specific records
                default: switch( getBiff() )
                {
                    case BIFF2: switch( nRecId )
                    {
                        case BIFF_ID_COLUMNDEFAULT: importColumnDefault( rStrm );           break;
                        case BIFF_ID_COLWIDTH:      importColWidth( rStrm );                break;
                        case BIFF2_ID_DEFROWHEIGHT: importDefRowHeight( rStrm );            break;
                        case BIFF2_ID_WINDOW2:      rSheetViewSett.importWindow2( rStrm );  break;
                    }
                    break;

                    case BIFF3: switch( nRecId )
                    {
                        case BIFF_ID_COLINFO:       importColInfo( rStrm );                         break;
                        case BIFF_ID_DEFCOLWIDTH:   importDefColWidth( rStrm );                     break;
                        case BIFF3_ID_DEFROWHEIGHT: importDefRowHeight( rStrm );                    break;
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( rStrm );             break;
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( rStrm );    break;
                        case BIFF_ID_SAVERECALC:    rWorkbookSett.importSaveRecalc( rStrm );        break;
                        case BIFF_ID_SHEETPR:       rWorksheetSett.importSheetPr( rStrm );          break;
                        case BIFF_ID_UNCALCED:      rWorkbookSett.importUncalced( rStrm );          break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( rStrm );             break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( rStrm );          break;

                    }
                    break;

                    case BIFF4: switch( nRecId )
                    {
                        case BIFF_ID_COLINFO:       importColInfo( rStrm );                         break;
                        case BIFF3_ID_DEFROWHEIGHT: importDefRowHeight( rStrm );                    break;
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( rStrm );             break;
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( rStrm );    break;
                        case BIFF_ID_PAGESETUP:     rPageSett.importPageSetup( rStrm );             break;
                        case BIFF_ID_SAVERECALC:    rWorkbookSett.importSaveRecalc( rStrm );        break;
                        case BIFF_ID_SHEETPR:       rWorksheetSett.importSheetPr( rStrm );          break;
                        case BIFF_ID_STANDARDWIDTH: importStandardWidth( rStrm );                   break;
                        case BIFF_ID_UNCALCED:      rWorkbookSett.importUncalced( rStrm );          break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( rStrm );             break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( rStrm );          break;
                    }
                    break;

                    case BIFF5: switch( nRecId )
                    {
                        case BIFF_ID_COLINFO:       importColInfo( rStrm );                         break;
                        case BIFF3_ID_DEFROWHEIGHT: importDefRowHeight( rStrm );                    break;
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( rStrm );             break;
                        case BIFF_ID_MERGEDCELLS:   importMergedCells( rStrm );                     break;  // #i62300# also in BIFF5
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( rStrm );    break;
                        case BIFF_ID_PAGESETUP:     rPageSett.importPageSetup( rStrm );             break;
                        case BIFF_ID_SAVERECALC:    rWorkbookSett.importSaveRecalc( rStrm );        break;
                        case BIFF_ID_SCENPROTECT:   rWorksheetSett.importScenProtect( rStrm );      break;
                        case BIFF_ID_SCL:           rSheetViewSett.importScl( rStrm );              break;
                        case BIFF_ID_SHEETPR:       rWorksheetSett.importSheetPr( rStrm );          break;
                        case BIFF_ID_STANDARDWIDTH: importStandardWidth( rStrm );                   break;
                        case BIFF_ID_UNCALCED:      rWorkbookSett.importUncalced( rStrm );          break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( rStrm );             break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( rStrm );          break;
                    }
                    break;

                    case BIFF8: switch( nRecId )
                    {
                        case BIFF_ID_CFHEADER:          rCondFormats.importCfHeader( rStrm );           break;
                        case BIFF_ID_COLINFO:           importColInfo( rStrm );                         break;
                        case BIFF_ID_DATAVALIDATION:    importDataValidation( rStrm );                  break;
                        case BIFF_ID_DATAVALIDATIONS:   importDataValidations( rStrm );                 break;
                        case BIFF3_ID_DEFROWHEIGHT:     importDefRowHeight( rStrm );                    break;
                        case BIFF_ID_HCENTER:           rPageSett.importHorCenter( rStrm );             break;
                        case BIFF_ID_HYPERLINK:         importHyperlink( rStrm );                       break;
                        case BIFF_ID_LABELRANGES:       importLabelRanges( rStrm );                     break;
                        case BIFF_ID_MERGEDCELLS:       importMergedCells( rStrm );                     break;
                        case BIFF_ID_OBJECTPROTECT:     rWorksheetSett.importObjectProtect( rStrm );    break;
                        case BIFF_ID_PICTURE:           rPageSett.importPicture( rStrm );               break;
                        case BIFF_ID_SAVERECALC:        rWorkbookSett.importSaveRecalc( rStrm );        break;
                        case BIFF_ID_SCENPROTECT:       rWorksheetSett.importScenProtect( rStrm );      break;
                        case BIFF_ID_SCL:               rSheetViewSett.importScl( rStrm );              break;
                        case BIFF_ID_SHEETPR:           rWorksheetSett.importSheetPr( rStrm );          break;
                        case BIFF_ID_SHEETPROTECTION:   rWorksheetSett.importSheetProtection( rStrm );  break;
                        case BIFF_ID_PAGESETUP:         rPageSett.importPageSetup( rStrm );             break;
                        case BIFF_ID_PHONETICPR:        rWorksheetSett.importPhoneticPr( rStrm );       break;
                        case BIFF_ID_STANDARDWIDTH:     importStandardWidth( rStrm );                   break;
                        case BIFF_ID_UNCALCED:          rWorkbookSett.importUncalced( rStrm );          break;
                        case BIFF_ID_VCENTER:           rPageSett.importVerCenter( rStrm );             break;
                        case BIFF3_ID_WINDOW2:          rSheetViewSett.importWindow2( rStrm );          break;
                    }
                    break;

                    case BIFF_UNKNOWN: break;
                }
            }

            // record not processed, try cell records
            if( rStrm.getCoreStreamPos() == nStrmPos )
                aSheetData.importRecord( rStrm );
        }
    }

    // final processing in base class WorksheetHelper
    finalizeWorksheetImport();
    return rStrm.getRecId() == BIFF_ID_EOF;
}

// private --------------------------------------------------------------------

void BiffWorksheetFragment::importColInfo( BiffInputStream& rStrm )
{
    sal_uInt16 nFirstCol, nLastCol, nWidth, nXfId, nFlags;
    rStrm >> nFirstCol >> nLastCol >> nWidth >> nXfId >> nFlags;

    OoxColumnData aData;
    // column indexes are 0-based in BIFF, but OoxColumnData expects 1-based
    aData.mnFirstCol     = static_cast< sal_Int32 >( nFirstCol ) + 1;
    aData.mnLastCol      = static_cast< sal_Int32 >( nLastCol ) + 1;
    // width is stored as 1/256th of a character in BIFF, convert to entire character
    aData.mfWidth        = static_cast< double >( nWidth ) / 256.0;
    aData.mnXfId         = nXfId;
    aData.mnLevel        = extractValue< sal_Int32 >( nFlags, 8, 3 );
    aData.mbShowPhonetic = getFlag( nFlags, BIFF_COLINFO_SHOWPHONETIC );
    aData.mbHidden       = getFlag( nFlags, BIFF_COLINFO_HIDDEN );
    aData.mbCollapsed    = getFlag( nFlags, BIFF_COLINFO_COLLAPSED );
    // set column properties in the current sheet
    setColumnData( aData );
}

void BiffWorksheetFragment::importColumnDefault( BiffInputStream& rStrm )
{
    sal_uInt16 nFirstCol, nLastCol, nXfId;
    rStrm >> nFirstCol >> nLastCol >> nXfId;
    convertColumnFormat( nFirstCol, nLastCol, nXfId );
}

void BiffWorksheetFragment::importColWidth( BiffInputStream& rStrm )
{
    sal_uInt8 nFirstCol, nLastCol;
    sal_uInt16 nWidth;
    rStrm >> nFirstCol >> nLastCol >> nWidth;

    OoxColumnData aData;
    // column indexes are 0-based in BIFF, but OoxColumnData expects 1-based
    aData.mnFirstCol = static_cast< sal_Int32 >( nFirstCol ) + 1;
    aData.mnLastCol = static_cast< sal_Int32 >( nLastCol ) + 1;
    // width is stored as 1/256th of a character in BIFF, convert to entire character
    aData.mfWidth = static_cast< double >( nWidth ) / 256.0;
    // set column properties in the current sheet
    setColumnData( aData );
}

void BiffWorksheetFragment::importDefColWidth( BiffInputStream& rStrm )
{
    /*  Stored as entire number of characters without padding pixels, which
        will be added in setBaseColumnWidth(). Call has no effect, if a
        width has already been set from the STANDARDWIDTH record. */
    setBaseColumnWidth( rStrm.readuInt16() );
}

void BiffWorksheetFragment::importDefRowHeight( BiffInputStream& rStrm )
{
    sal_uInt16 nFlags = BIFF_DEFROW_CUSTOMHEIGHT, nHeight;
    if( getBiff() != BIFF2 )
        rStrm >> nFlags;
    rStrm >> nHeight;
    if( getBiff() == BIFF2 )
        nHeight &= BIFF2_DEFROW_MASK;
    // row height is in twips in BIFF, convert to points
    setDefaultRowSettings(
        nHeight / 20.0,
        getFlag( nFlags, BIFF_DEFROW_CUSTOMHEIGHT ),
        getFlag( nFlags, BIFF_DEFROW_HIDDEN ),
        getFlag( nFlags, BIFF_DEFROW_THICKTOP ),
        getFlag( nFlags, BIFF_DEFROW_THICKBOTTOM ) );
}

void BiffWorksheetFragment::importDataValidations( BiffInputStream& rStrm )
{
    sal_Int32 nObjId;
    rStrm.skip( 10 );
    rStrm >> nObjId;
    //! TODO: invalidate object id in drawing object manager
}

namespace {

OUString lclReadDataValMessage( BiffInputStream& rStrm )
{
    // empty strings are single NUL characters (string length is 1)
    rStrm.enableNulChars( true );
    OUString aMessage = rStrm.readUniString();
    rStrm.enableNulChars( false );
    if( (aMessage.getLength() == 1) && (aMessage[ 0 ] == 0) )
        aMessage = OUString();
    return aMessage;
}

ApiTokenSequence lclReadDataValFormula( BiffInputStream& rStrm, FormulaParser& rParser )
{
    sal_uInt16 nFmlaSize = rStrm.readuInt16();
    rStrm.skip( 2 );
    TokensFormulaContext aContext( true, false );
    // enable NUL characters, string list is single tStr token with NUL separators
    rStrm.enableNulChars( true );
    rParser.importFormula( aContext, rStrm, &nFmlaSize );
    rStrm.enableNulChars( false );
    return aContext.getTokens();
}

} // namespace

void BiffWorksheetFragment::importDataValidation( BiffInputStream& rStrm )
{
    OoxValidationData aData;

    // flags
    sal_uInt32 nFlags;
    rStrm >> nFlags;
    aData.setBinType( extractValue< sal_uInt8 >( nFlags, 0, 4 ) );
    aData.setBinOperator( extractValue< sal_uInt8 >( nFlags, 20, 4 ) );
    aData.setBinErrorStyle( extractValue< sal_uInt8 >( nFlags, 4, 3 ) );
    aData.mbAllowBlank   = getFlag( nFlags, BIFF_DATAVAL_ALLOWBLANK );
    aData.mbNoDropDown   = getFlag( nFlags, BIFF_DATAVAL_NODROPDOWN );
    aData.mbShowInputMsg = getFlag( nFlags, BIFF_DATAVAL_SHOWINPUT );
    aData.mbShowErrorMsg = getFlag( nFlags, BIFF_DATAVAL_SHOWERROR );

    // message strings
    aData.maInputTitle   = lclReadDataValMessage( rStrm );
    aData.maErrorTitle   = lclReadDataValMessage( rStrm );
    aData.maInputMessage = lclReadDataValMessage( rStrm );
    aData.maErrorMessage = lclReadDataValMessage( rStrm );

    // condition formula(s)
    FormulaParser& rParser = getFormulaParser();
    aData.maTokens1 = lclReadDataValFormula( rStrm, rParser );
    aData.maTokens2 = lclReadDataValFormula( rStrm, rParser );
    // process string list of a list validation (convert to list of string tokens)
    if( (aData.mnType == XML_list) && getFlag( nFlags, BIFF_DATAVAL_STRINGLIST ) )
        rParser.convertStringToStringList( aData.maTokens1, '\0', true );

    // cell range list
    BinRangeList aRanges;
    rStrm >> aRanges;
    getAddressConverter().convertToCellRangeList( aData.maRanges, aRanges, getSheetIndex(), true );

    // set validation data
    setValidation( aData );
}

void BiffWorksheetFragment::importDimension( BiffInputStream& rStrm )
{
    BinRange aBinRange;
    aBinRange.read( rStrm, true, (rStrm.getRecId() == BIFF3_ID_DIMENSION) && (getBiff() == BIFF8) );
    // first unused row/column index in BIFF, not last used
    if( aBinRange.maFirst.mnCol < aBinRange.maLast.mnCol ) --aBinRange.maLast.mnCol;
    if( aBinRange.maFirst.mnRow < aBinRange.maLast.mnRow ) --aBinRange.maLast.mnRow;
    // set dimension
    CellRangeAddress aRange;
    getAddressConverter().convertToCellRangeUnchecked( aRange, aBinRange, getSheetIndex() );
    setDimension( aRange );
}

namespace {

OUString lclReadHyperlinkString( BiffInputStream& rStrm, sal_Int32 nChars, rtl_TextEncoding eTextEnc, bool bUnicode )
{
    OUString aRet;
    if( nChars > 0 )
    {
        sal_uInt16 nReadChars = getLimitedValue< sal_uInt16, sal_Int32 >( nChars, 0, SAL_MAX_UINT16 );
        // strings are NUL terminated with trailing garbage
        rStrm.enableNulChars( true );
        aRet = bUnicode ?
            rStrm.readUnicodeArray( nReadChars ) :
            rStrm.readCharArray( nReadChars, eTextEnc );
        rStrm.enableNulChars( false );
        // remove trailing garbage
        sal_Int32 nNulPos = aRet.indexOf( '\0' );
        if( nNulPos >= 0 )
            aRet = aRet.copy( 0, nNulPos );
        // skip remaining chars
        sal_uInt32 nSkip = static_cast< sal_uInt32 >( nChars - nReadChars );
        rStrm.skip( bUnicode ? (nSkip * 2) : nSkip );
    }
    return aRet;
}

OUString lclReadHyperlinkString( BiffInputStream& rStrm, rtl_TextEncoding eTextEnc, bool bUnicode )
{
    return lclReadHyperlinkString( rStrm, rStrm.readInt32(), eTextEnc, bUnicode );
}

void lclSkipHyperlinkString( BiffInputStream& rStrm, sal_Int32 nChars, bool bUnicode )
{
    if( nChars > 0 )
        rStrm.skip( static_cast< sal_uInt32 >( bUnicode ? (nChars * 2) : nChars ) );
}

void lclSkipHyperlinkString( BiffInputStream& rStrm, bool bUnicode )
{
    lclSkipHyperlinkString( rStrm, rStrm.readInt32(), bUnicode );
}

} // namespace

void BiffWorksheetFragment::importHyperlink( BiffInputStream& rStrm )
{
    OoxHyperlinkData aData;

    // read cell range for the hyperlink
    BinRange aBiffRange;
    rStrm >> aBiffRange;
    // #i80006# Excel silently ignores invalid hi-byte of column index (TODO: everywhere?)
    aBiffRange.maFirst.mnCol &= 0xFF;
    aBiffRange.maLast.mnCol &= 0xFF;
    if( !getAddressConverter().convertToCellRange( aData.maRange, aBiffRange, getSheetIndex(), true ) )
        return;

    BiffGuid aGuid;
    sal_uInt32 nId, nFlags;
    rStrm >> aGuid >> nId >> nFlags;

    OSL_ENSURE( aGuid == BiffHelper::maGuidStdHlink, "BiffWorksheetFragment::importHyperlink - unexpected header GUID" );
    OSL_ENSURE( nId == 2, "BiffWorksheetFragment::importHyperlink - unexpected header identifier" );
    if( !(aGuid == BiffHelper::maGuidStdHlink) )
        return;

    // display string
    if( getFlag( nFlags, BIFF_HYPERLINK_DISPLAY ) )
        aData.maDisplay = lclReadHyperlinkString( rStrm, getTextEncoding(), true );
    // target frame (ignore) !TODO: DISPLAY/FRAME - right order? (never seen them together)
    if( getFlag( nFlags, BIFF_HYPERLINK_FRAME ) )
        lclSkipHyperlinkString( rStrm, true );

    // target
    if( getFlag( nFlags, BIFF_HYPERLINK_TARGET ) )
    {
        if( getFlag( nFlags, BIFF_HYPERLINK_UNC ) )
        {
            // UNC path
            OSL_ENSURE( getFlag( nFlags, BIFF_HYPERLINK_ABS ), "BiffWorksheetFragment::importHyperlink - UNC link not absolute" );
            aData.maTarget = lclReadHyperlinkString( rStrm, getTextEncoding(), true );
        }
        else
        {
            rStrm >> aGuid;
            if( aGuid == BiffHelper::maGuidFileMoniker )
            {
                // file name, maybe relative and with directory up-count
                sal_Int16 nUpLevels;
                rStrm >> nUpLevels;
                OSL_ENSURE( (nUpLevels == 0) || !getFlag( nFlags, BIFF_HYPERLINK_ABS ), "BiffWorksheetFragment::importHyperlink - absolute filename with upcount" );
                OUString aShortName = lclReadHyperlinkString( rStrm, getTextEncoding(), false );
                if( rStrm.skip( 24 ).readInt32() > 0 )
                {
                    sal_Int32 nStrLen = rStrm.readInt32() / 2;  // byte count to char count
                    rStrm.skip( 2 );
                    aData.maTarget = lclReadHyperlinkString( rStrm, nStrLen, getTextEncoding(), true );
                }
                if( aData.maTarget.getLength() == 0 )
                    aData.maTarget = aShortName;
                if( !getFlag( nFlags, BIFF_HYPERLINK_ABS ) )
                    for( sal_Int16 nLevel = 0; nLevel < nUpLevels; ++nLevel )
                        aData.maTarget = CREATE_OUSTRING( "../" ) + aData.maTarget;
            }
            else if( aGuid == BiffHelper::maGuidUrlMoniker )
            {
                // URL, maybe relative and with leading '../'
                sal_Int32 nStrLen = rStrm.readInt32() / 2;  // byte count to char count
                aData.maTarget = lclReadHyperlinkString( rStrm, nStrLen, getTextEncoding(), true );
            }
            else
            {
                OSL_ENSURE( false, "BiffWorksheetFragment::importHyperlink - unknown content GUID" );
                return;
            }
        }
    }

    // target location
    if( getFlag( nFlags, BIFF_HYPERLINK_LOC ) )
        aData.maLocation = lclReadHyperlinkString( rStrm, getTextEncoding(), true );

    OSL_ENSURE( rStrm.getRecLeft() == 0, "BiffWorksheetFragment::importHyperlink - unknown record data" );

    // try to read the SCREENTIP record
    if( (rStrm.getNextRecId() == BIFF_ID_SCREENTIP) && rStrm.startNextRecord() )
    {
        rStrm.skip( 2 );      // repeated record id
        // the cell range, again
        rStrm >> aBiffRange;
        CellRangeAddress aRange;
        if( getAddressConverter().convertToCellRange( aRange, aBiffRange, getSheetIndex(), true ) &&
            (aRange.StartColumn == aData.maRange.StartColumn) &&
            (aRange.StartRow == aData.maRange.StartRow) &&
            (aRange.EndColumn == aData.maRange.EndColumn) &&
            (aRange.EndRow == aData.maRange.EndRow) )
        {
            /*  This time, we have no string length, no flag field, and a
                null-terminated 16-bit character array. */
            aData.maTooltip = rStrm.readUnicodeArray( static_cast< sal_uInt16 >( rStrm.getRecLeft() / 2 ) );
        }
    }

    // store the hyperlink settings
    setHyperlink( aData );
}

void BiffWorksheetFragment::importLabelRanges( BiffInputStream& rStrm )
{
    BinRangeList aBiffRowRanges, aBiffColRanges;
    rStrm >> aBiffRowRanges >> aBiffColRanges;
    ApiCellRangeList aColRanges, aRowRanges;
    getAddressConverter().convertToCellRangeList( aColRanges, aBiffColRanges, getSheetIndex(), true );
    getAddressConverter().convertToCellRangeList( aRowRanges, aBiffRowRanges, getSheetIndex(), true );
    setLabelRanges( aColRanges, aRowRanges );
}

void BiffWorksheetFragment::importMergedCells( BiffInputStream& rStrm )
{
    BinRangeList aBiffRanges;
    rStrm >> aBiffRanges;
    ApiCellRangeList aRanges;
    getAddressConverter().convertToCellRangeList( aRanges, aBiffRanges, getSheetIndex(), true );
    for( ApiCellRangeList::const_iterator aIt = aRanges.begin(), aEnd = aRanges.end(); aIt != aEnd; ++aIt )
        setMergedRange( *aIt );
}

void BiffWorksheetFragment::importPageBreaks( BiffInputStream& rStrm, bool bRowBreak )
{
    OoxPageBreakData aData;
    aData.mbManual = true;              // only manual breaks stored in BIFF
    bool bBiff8 = getBiff() == BIFF8;   // skip start/end columns or rows in BIFF8

    sal_uInt16 nCount;
    rStrm >> nCount;
    for( sal_uInt16 nIndex = 0; rStrm.isValid() && (nIndex < nCount); ++nIndex )
    {
        aData.mnColRow = rStrm.readuInt16();
        setPageBreak( aData, bRowBreak );
        if( bBiff8 )
            rStrm.skip( 4 );
    }
}

void BiffWorksheetFragment::importStandardWidth( BiffInputStream& rStrm )
{
    sal_uInt16 nWidth;
    rStrm >> nWidth;
    // width is stored as 1/256th of a character in BIFF, convert to entire character
    double fWidth = static_cast< double >( nWidth ) / 256.0;
    // set as default width, will override the width from DEFCOLWIDTH record
    setDefaultColumnWidth( fWidth );
}

// ============================================================================

} // namespace xls
} // namespace oox

