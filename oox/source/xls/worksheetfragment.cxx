/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: worksheetfragment.cxx,v $
 * $Revision: 1.5.4.5 $
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

const sal_uInt32 OLE_HYPERLINK_HASTARGET    = 0x00000001;   /// Has hyperlink moniker.
const sal_uInt32 OLE_HYPERLINK_ABSOLUTE     = 0x00000002;   /// Absolute path.
const sal_uInt32 OLE_HYPERLINK_HASLOCATION  = 0x00000008;   /// Has target location.
const sal_uInt32 OLE_HYPERLINK_HASDISPLAY   = 0x00000010;   /// Has display string.
const sal_uInt32 OLE_HYPERLINK_HASGUID      = 0x00000020;   /// Has identification GUID.
const sal_uInt32 OLE_HYPERLINK_HASTIME      = 0x00000040;   /// Has creation time.
const sal_uInt32 OLE_HYPERLINK_HASFRAME     = 0x00000080;   /// Has frame.
const sal_uInt32 OLE_HYPERLINK_ASSTRING     = 0x00000100;   /// Hyperlink as simple string.

const sal_Int32 OOBIN_OLEOBJECT_CONTENT     = 1;
const sal_Int32 OOBIN_OLEOBJECT_ICON        = 4;
const sal_Int32 OOBIN_OLEOBJECT_ALWAYS      = 1;
const sal_Int32 OOBIN_OLEOBJECT_ONCALL      = 3;
const sal_uInt16 OOBIN_OLEOBJECT_AUTOLOAD   = 0x0001;
const sal_uInt16 OOBIN_OLEOBJECT_LINKED     = 0x0002;

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
            case SHEETTYPE_DIALOGSHEET: return  (nElement == XM_TOKEN( dialogsheet ));
            case SHEETTYPE_MODULESHEET: return  false;
            case SHEETTYPE_EMPTYSHEET:  return  false;
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
                    (nElement == XLS_TOKEN( drawing )) ||
                    (nElement == XLS_TOKEN( legacyDrawing )) ||
                    (nElement == XLS_TOKEN( oleObjects )) ||
                    (nElement == XLS_TOKEN( controls ));

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

        case XLS_TOKEN( oleObjects ):
            return  (nElement == XLS_TOKEN( oleObject ));
        case XLS_TOKEN( controls ):
            return  (nElement == XLS_TOKEN( control ));
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
        case XLS_TOKEN( legacyDrawing ):    importLegacyDrawing( rAttribs );                                break;
        case XLS_TOKEN( oleObject ):        importOleObject( rAttribs );                                    break;
        case XLS_TOKEN( control ):          importControl( rAttribs );                                      break;
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
                    (nRecId == OOBIN_ID_DRAWING) ||
                    (nRecId == OOBIN_ID_LEGACYDRAWING) ||
                    (nRecId == OOBIN_ID_OLEOBJECTS) ||
                    (nRecId == OOBIN_ID_CONTROLS);
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
        case OOBIN_ID_OLEOBJECTS:
            return  (nRecId == OOBIN_ID_OLEOBJECT);
        case OOBIN_ID_CONTROLS:
            return  (nRecId == OOBIN_ID_CONTROL);
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
        case OOBIN_ID_LEGACYDRAWING:    importLegacyDrawing( rStrm );                               break;
        case OOBIN_ID_OLEOBJECT:        importOleObject( rStrm );                                   break;
        case OOBIN_ID_CONTROL:          importControl( rStrm );                                     break;
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
        { OOBIN_ID_CONTROLS,            OOBIN_ID_CONTROLS + 2           },
        { OOBIN_ID_CUSTOMSHEETVIEW,     OOBIN_ID_CUSTOMSHEETVIEW + 1    },
        { OOBIN_ID_CUSTOMSHEETVIEWS,    OOBIN_ID_CUSTOMSHEETVIEWS + 3   },
        { OOBIN_ID_DATABAR,             OOBIN_ID_DATABAR + 1            },
        { OOBIN_ID_DATAVALIDATIONS,     OOBIN_ID_DATAVALIDATIONS + 1    },
        { OOBIN_ID_HEADERFOOTER,        OOBIN_ID_HEADERFOOTER + 1       },
        { OOBIN_ID_ICONSET,             OOBIN_ID_ICONSET + 1            },
        { OOBIN_ID_MERGECELLS,          OOBIN_ID_MERGECELLS + 1         },
        { OOBIN_ID_OLEOBJECTS,          OOBIN_ID_OLEOBJECTS + 2         },
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
    getAddressConverter().convertToCellRangeUnchecked( aRange, rAttribs.getString( XML_ref, OUString() ), getSheetIndex() );
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
    if( getAddressConverter().convertToCellRange( aRange, rAttribs.getString( XML_ref, OUString() ), getSheetIndex(), true ) )
        setMergedRange( aRange );
}

void OoxWorksheetFragment::importDataValidation( const AttributeList& rAttribs )
{
    mxValData.reset( new OoxValidationData );
    getAddressConverter().convertToCellRangeList( mxValData->maRanges, rAttribs.getString( XML_sqref, OUString() ), getSheetIndex(), true );
    mxValData->maInputTitle   = rAttribs.getString( XML_promptTitle, OUString() );
    mxValData->maInputMessage = rAttribs.getString( XML_prompt, OUString() );
    mxValData->maErrorTitle   = rAttribs.getString( XML_errorTitle, OUString() );
    mxValData->maErrorMessage = rAttribs.getString( XML_error, OUString() );
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
    if( getAddressConverter().convertToCellRange( aData.maRange, rAttribs.getString( XML_ref, OUString() ), getSheetIndex(), true ) )
    {
        aData.maTarget   = getRelations().getTargetFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
        aData.maLocation = rAttribs.getString( XML_location, OUString() );
        aData.maDisplay  = rAttribs.getString( XML_display, OUString() );
        aData.maTooltip  = rAttribs.getString( XML_tooltip, OUString() );
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
    setDrawingPath( getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) ) );
}

void OoxWorksheetFragment::importLegacyDrawing( const AttributeList& rAttribs )
{
    setVmlDrawingPath( getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) ) );
}

void OoxWorksheetFragment::importOleObject( const AttributeList& rAttribs )
{
    OoxOleObjectData aData;
    aData.maProgId = rAttribs.getString( XML_progId, OUString() );
    OSL_ENSURE( rAttribs.hasAttribute( XML_link ) != rAttribs.hasAttribute( R_TOKEN( id ) ),
        "OoxWorksheetFragment::importOleObject - either linked or embedded" );
    if( rAttribs.hasAttribute( XML_link ) )
        (void)0;
    if( rAttribs.hasAttribute( R_TOKEN( id ) ) )
        aData.maStoragePath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
    aData.mnAspect = rAttribs.getToken( XML_dvAspect, XML_DVASPECT_CONTENT );
    aData.mnUpdateMode = rAttribs.getToken( XML_oleUpdate, XML_OLEUPDATE_ALWAYS );
    aData.mnShapeId = rAttribs.getInteger( XML_shapeId, 0 );
    aData.mbAutoLoad = rAttribs.getBool( XML_autoLoad, false );
    setOleObject( aData );
}

void OoxWorksheetFragment::importControl( const AttributeList& rAttribs )
{
    OoxFormControlData aData;
    aData.maStoragePath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
    aData.maName = rAttribs.getString( XML_name, OUString() );
    aData.mnShapeId = rAttribs.getInteger( XML_shapeId, 0 );
    setFormControl( aData );
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

void OoxWorksheetFragment::importLegacyDrawing( RecordInputStream& rStrm )
{
    setVmlDrawingPath( getFragmentPathFromRelId( rStrm.readString() ) );
}

void OoxWorksheetFragment::importOleObject( RecordInputStream& rStrm )
{
    OoxOleObjectData aData;
    sal_Int32 nAspect, nUpdateMode;
    sal_uInt16 nFlags;
    rStrm >> nAspect >> nUpdateMode >> aData.mnShapeId >> nFlags >> aData.maProgId;
    if( getFlag( nFlags, OOBIN_OLEOBJECT_LINKED ) )
        (void)0;
    else
        aData.maStoragePath = getFragmentPathFromRelId( rStrm.readString() );
    aData.mnAspect = (nAspect == OOBIN_OLEOBJECT_ICON) ? XML_DVASPECT_ICON : XML_DVASPECT_CONTENT;
    aData.mnUpdateMode = (nUpdateMode == OOBIN_OLEOBJECT_ONCALL) ? XML_OLEUPDATE_ONCALL : XML_OLEUPDATE_ALWAYS;
    aData.mbAutoLoad = getFlag( nFlags, OOBIN_OLEOBJECT_AUTOLOAD );
    setOleObject( aData );
}

void OoxWorksheetFragment::importControl( RecordInputStream& rStrm )
{
    OoxFormControlData aData;
    rStrm >> aData.mnShapeId;
    aData.maStoragePath = getFragmentPathFromRelId( rStrm.readString() );
    rStrm >> aData.maName;
    setFormControl( aData );
}

// ============================================================================

BiffWorksheetFragment::BiffWorksheetFragment( const BiffWorkbookFragmentBase& rParent, ISegmentProgressBarRef xProgressBar, WorksheetType eSheetType, sal_Int32 nSheet ) :
    BiffWorksheetFragmentBase( rParent, xProgressBar, eSheetType, nSheet )
{
}

bool BiffWorksheetFragment::importFragment()
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
    while( mrStrm.startNextRecord() && (mrStrm.getRecId() != BIFF_ID_EOF) )
    {
        if( isBofRecord() )
        {
            // skip unknown embedded fragments (BOF/EOF blocks)
            skipFragment();
        }
        else
        {
            // cache base stream position to detect if record is already processed
            sal_Int64 nStrmPos = mrStrm.tellBase();
            sal_uInt16 nRecId = mrStrm.getRecId();

            switch( nRecId )
            {
                // records in all BIFF versions
                case BIFF_ID_BOTTOMMARGIN:      rPageSett.importBottomMargin( mrStrm );     break;
                case BIFF_ID_CALCCOUNT:         rWorkbookSett.importCalcCount( mrStrm );    break;
                case BIFF_ID_CALCMODE:          rWorkbookSett.importCalcMode( mrStrm );     break;
                case BIFF_ID_DEFCOLWIDTH:       importDefColWidth();                        break;
                case BIFF_ID_DELTA:             rWorkbookSett.importDelta( mrStrm );        break;
                case BIFF2_ID_DIMENSION:        importDimension();                          break;
                case BIFF3_ID_DIMENSION:        importDimension();                          break;
                case BIFF_ID_FOOTER:            rPageSett.importFooter( mrStrm );           break;
                case BIFF_ID_HEADER:            rPageSett.importHeader( mrStrm );           break;
                case BIFF_ID_HORPAGEBREAKS:     importPageBreaks( true );                   break;
                case BIFF_ID_ITERATION:         rWorkbookSett.importIteration( mrStrm );    break;
                case BIFF_ID_LEFTMARGIN:        rPageSett.importLeftMargin( mrStrm );       break;
                case BIFF_ID_PANE:              rSheetViewSett.importPane( mrStrm );        break;
                case BIFF_ID_PASSWORD:          rWorksheetSett.importPassword( mrStrm );    break;
                case BIFF_ID_PRINTGRIDLINES:    rPageSett.importPrintGridLines( mrStrm );   break;
                case BIFF_ID_PRINTHEADERS:      rPageSett.importPrintHeaders( mrStrm );     break;
                case BIFF_ID_PROTECT:           rWorksheetSett.importProtect( mrStrm );     break;
                case BIFF_ID_REFMODE:           rWorkbookSett.importRefMode( mrStrm );      break;
                case BIFF_ID_RIGHTMARGIN:       rPageSett.importRightMargin( mrStrm );      break;
                case BIFF_ID_SELECTION:         rSheetViewSett.importSelection( mrStrm );   break;
                case BIFF_ID_TOPMARGIN:         rPageSett.importTopMargin( mrStrm );        break;
                case BIFF_ID_VERPAGEBREAKS:     importPageBreaks( false );                  break;

                // BIFF specific records
                default: switch( getBiff() )
                {
                    case BIFF2: switch( nRecId )
                    {
                        case BIFF_ID_COLUMNDEFAULT: importColumnDefault();                  break;
                        case BIFF_ID_COLWIDTH:      importColWidth();                       break;
                        case BIFF2_ID_DEFROWHEIGHT: importDefRowHeight();                   break;
                        case BIFF2_ID_WINDOW2:      rSheetViewSett.importWindow2( mrStrm ); break;
                    }
                    break;

                    case BIFF3: switch( nRecId )
                    {
                        case BIFF_ID_COLINFO:       importColInfo();                                break;
                        case BIFF_ID_DEFCOLWIDTH:   importDefColWidth();                            break;
                        case BIFF3_ID_DEFROWHEIGHT: importDefRowHeight();                           break;
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( mrStrm );            break;
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( mrStrm );   break;
                        case BIFF_ID_SAVERECALC:    rWorkbookSett.importSaveRecalc( mrStrm );       break;
                        case BIFF_ID_SHEETPR:       rWorksheetSett.importSheetPr( mrStrm );         break;
                        case BIFF_ID_UNCALCED:      rWorkbookSett.importUncalced( mrStrm );         break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( mrStrm );            break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( mrStrm );         break;

                    }
                    break;

                    case BIFF4: switch( nRecId )
                    {
                        case BIFF_ID_COLINFO:       importColInfo();                                break;
                        case BIFF3_ID_DEFROWHEIGHT: importDefRowHeight();                           break;
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( mrStrm );            break;
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( mrStrm );   break;
                        case BIFF_ID_PAGESETUP:     rPageSett.importPageSetup( mrStrm );            break;
                        case BIFF_ID_SAVERECALC:    rWorkbookSett.importSaveRecalc( mrStrm );       break;
                        case BIFF_ID_SHEETPR:       rWorksheetSett.importSheetPr( mrStrm );         break;
                        case BIFF_ID_STANDARDWIDTH: importStandardWidth();                          break;
                        case BIFF_ID_UNCALCED:      rWorkbookSett.importUncalced( mrStrm );         break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( mrStrm );            break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( mrStrm );         break;
                    }
                    break;

                    case BIFF5: switch( nRecId )
                    {
                        case BIFF_ID_COLINFO:       importColInfo();                                break;
                        case BIFF3_ID_DEFROWHEIGHT: importDefRowHeight();                           break;
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( mrStrm );            break;
                        case BIFF_ID_MERGEDCELLS:   importMergedCells();                            break;  // #i62300# also in BIFF5
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( mrStrm );   break;
                        case BIFF_ID_PAGESETUP:     rPageSett.importPageSetup( mrStrm );            break;
                        case BIFF_ID_SAVERECALC:    rWorkbookSett.importSaveRecalc( mrStrm );       break;
                        case BIFF_ID_SCENPROTECT:   rWorksheetSett.importScenProtect( mrStrm );     break;
                        case BIFF_ID_SCL:           rSheetViewSett.importScl( mrStrm );             break;
                        case BIFF_ID_SHEETPR:       rWorksheetSett.importSheetPr( mrStrm );         break;
                        case BIFF_ID_STANDARDWIDTH: importStandardWidth();                          break;
                        case BIFF_ID_UNCALCED:      rWorkbookSett.importUncalced( mrStrm );         break;
                        case BIFF_ID_VCENTER:       rPageSett.importVerCenter( mrStrm );            break;
                        case BIFF3_ID_WINDOW2:      rSheetViewSett.importWindow2( mrStrm );         break;
                    }
                    break;

                    case BIFF8: switch( nRecId )
                    {
                        case BIFF_ID_CFHEADER:          rCondFormats.importCfHeader( mrStrm );          break;
                        case BIFF_ID_COLINFO:           importColInfo();                                break;
                        case BIFF_ID_DATAVALIDATION:    importDataValidation();                         break;
                        case BIFF_ID_DATAVALIDATIONS:   importDataValidations();                        break;
                        case BIFF3_ID_DEFROWHEIGHT:     importDefRowHeight();                           break;
                        case BIFF_ID_HCENTER:           rPageSett.importHorCenter( mrStrm );            break;
                        case BIFF_ID_HYPERLINK:         importHyperlink();                              break;
                        case BIFF_ID_LABELRANGES:       importLabelRanges();                            break;
                        case BIFF_ID_MERGEDCELLS:       importMergedCells();                            break;
                        case BIFF_ID_OBJECTPROTECT:     rWorksheetSett.importObjectProtect( mrStrm );   break;
                        case BIFF_ID_PICTURE:           rPageSett.importPicture( mrStrm );              break;
                        case BIFF_ID_SAVERECALC:        rWorkbookSett.importSaveRecalc( mrStrm );       break;
                        case BIFF_ID_SCENPROTECT:       rWorksheetSett.importScenProtect( mrStrm );     break;
                        case BIFF_ID_SCL:               rSheetViewSett.importScl( mrStrm );             break;
                        case BIFF_ID_SHEETPR:           rWorksheetSett.importSheetPr( mrStrm );         break;
                        case BIFF_ID_SHEETPROTECTION:   rWorksheetSett.importSheetProtection( mrStrm ); break;
                        case BIFF_ID_PAGESETUP:         rPageSett.importPageSetup( mrStrm );            break;
                        case BIFF_ID_PHONETICPR:        rWorksheetSett.importPhoneticPr( mrStrm );      break;
                        case BIFF_ID_STANDARDWIDTH:     importStandardWidth();                          break;
                        case BIFF_ID_UNCALCED:          rWorkbookSett.importUncalced( mrStrm );         break;
                        case BIFF_ID_VCENTER:           rPageSett.importVerCenter( mrStrm );            break;
                        case BIFF3_ID_WINDOW2:          rSheetViewSett.importWindow2( mrStrm );         break;
                    }
                    break;

                    case BIFF_UNKNOWN: break;
                }
            }

            // record not processed, try cell records
            if( mrStrm.tellBase() == nStrmPos )
                aSheetData.importRecord();
        }
    }

    // final processing in base class WorksheetHelper
    finalizeWorksheetImport();
    return mrStrm.getRecId() == BIFF_ID_EOF;
}

// private --------------------------------------------------------------------

void BiffWorksheetFragment::importColInfo()
{
    sal_uInt16 nFirstCol, nLastCol, nWidth, nXfId, nFlags;
    mrStrm >> nFirstCol >> nLastCol >> nWidth >> nXfId >> nFlags;

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

void BiffWorksheetFragment::importColumnDefault()
{
    sal_uInt16 nFirstCol, nLastCol, nXfId;
    mrStrm >> nFirstCol >> nLastCol >> nXfId;
    convertColumnFormat( nFirstCol, nLastCol, nXfId );
}

void BiffWorksheetFragment::importColWidth()
{
    sal_uInt8 nFirstCol, nLastCol;
    sal_uInt16 nWidth;
    mrStrm >> nFirstCol >> nLastCol >> nWidth;

    OoxColumnData aData;
    // column indexes are 0-based in BIFF, but OoxColumnData expects 1-based
    aData.mnFirstCol = static_cast< sal_Int32 >( nFirstCol ) + 1;
    aData.mnLastCol = static_cast< sal_Int32 >( nLastCol ) + 1;
    // width is stored as 1/256th of a character in BIFF, convert to entire character
    aData.mfWidth = static_cast< double >( nWidth ) / 256.0;
    // set column properties in the current sheet
    setColumnData( aData );
}

void BiffWorksheetFragment::importDefColWidth()
{
    /*  Stored as entire number of characters without padding pixels, which
        will be added in setBaseColumnWidth(). Call has no effect, if a
        width has already been set from the STANDARDWIDTH record. */
    setBaseColumnWidth( mrStrm.readuInt16() );
}

void BiffWorksheetFragment::importDefRowHeight()
{
    sal_uInt16 nFlags = BIFF_DEFROW_CUSTOMHEIGHT, nHeight;
    if( getBiff() != BIFF2 )
        mrStrm >> nFlags;
    mrStrm >> nHeight;
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

void BiffWorksheetFragment::importDataValidations()
{
    sal_Int32 nObjId;
    mrStrm.skip( 10 );
    mrStrm >> nObjId;
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

void BiffWorksheetFragment::importDataValidation()
{
    OoxValidationData aData;

    // flags
    sal_uInt32 nFlags;
    mrStrm >> nFlags;
    aData.setBinType( extractValue< sal_uInt8 >( nFlags, 0, 4 ) );
    aData.setBinOperator( extractValue< sal_uInt8 >( nFlags, 20, 4 ) );
    aData.setBinErrorStyle( extractValue< sal_uInt8 >( nFlags, 4, 3 ) );
    aData.mbAllowBlank   = getFlag( nFlags, BIFF_DATAVAL_ALLOWBLANK );
    aData.mbNoDropDown   = getFlag( nFlags, BIFF_DATAVAL_NODROPDOWN );
    aData.mbShowInputMsg = getFlag( nFlags, BIFF_DATAVAL_SHOWINPUT );
    aData.mbShowErrorMsg = getFlag( nFlags, BIFF_DATAVAL_SHOWERROR );

    // message strings
    aData.maInputTitle   = lclReadDataValMessage( mrStrm );
    aData.maErrorTitle   = lclReadDataValMessage( mrStrm );
    aData.maInputMessage = lclReadDataValMessage( mrStrm );
    aData.maErrorMessage = lclReadDataValMessage( mrStrm );

    // condition formula(s)
    FormulaParser& rParser = getFormulaParser();
    aData.maTokens1 = lclReadDataValFormula( mrStrm, rParser );
    aData.maTokens2 = lclReadDataValFormula( mrStrm, rParser );
    // process string list of a list validation (convert to list of string tokens)
    if( (aData.mnType == XML_list) && getFlag( nFlags, BIFF_DATAVAL_STRINGLIST ) )
        rParser.convertStringToStringList( aData.maTokens1, '\0', true );

    // cell range list
    BinRangeList aRanges;
    mrStrm >> aRanges;
    getAddressConverter().convertToCellRangeList( aData.maRanges, aRanges, getSheetIndex(), true );

    // set validation data
    setValidation( aData );
}

void BiffWorksheetFragment::importDimension()
{
    BinRange aBinRange;
    aBinRange.read( mrStrm, true, (mrStrm.getRecId() == BIFF3_ID_DIMENSION) && (getBiff() == BIFF8) );
    // first unused row/column index in BIFF, not last used
    if( aBinRange.maFirst.mnCol < aBinRange.maLast.mnCol ) --aBinRange.maLast.mnCol;
    if( aBinRange.maFirst.mnRow < aBinRange.maLast.mnRow ) --aBinRange.maLast.mnRow;
    // set dimension
    CellRangeAddress aRange;
    getAddressConverter().convertToCellRangeUnchecked( aRange, aBinRange, getSheetIndex() );
    setDimension( aRange );
}

OUString BiffWorksheetFragment::readHyperlinkString( rtl_TextEncoding eTextEnc, bool bUnicode )
{
    OUString aRet;
    sal_Int32 nChars = mrStrm.readInt32();
    if( nChars > 0 )
    {
        sal_uInt16 nReadChars = getLimitedValue< sal_uInt16, sal_Int32 >( nChars, 0, SAL_MAX_UINT16 );
        // strings are NUL terminated
        mrStrm.enableNulChars( true );
        aRet = bUnicode ?
            mrStrm.readUnicodeArray( nReadChars ) :
            mrStrm.readCharArray( nReadChars, eTextEnc );
        mrStrm.enableNulChars( false );
        // remove trailing NUL and possible other garbage
        sal_Int32 nNulPos = aRet.indexOf( '\0' );
        if( nNulPos >= 0 )
            aRet = aRet.copy( 0, nNulPos );
        // skip remaining chars
        sal_uInt32 nSkip = static_cast< sal_uInt32 >( nChars - nReadChars );
        mrStrm.skip( (bUnicode ? 2 : 1) * nSkip );
    }
    return aRet;
}

void BiffWorksheetFragment::importHyperlink()
{
    OoxHyperlinkData aData;

    // read cell range for the hyperlink
    BinRange aBiffRange;
    mrStrm >> aBiffRange;
    // #i80006# Excel silently ignores invalid hi-byte of column index (TODO: everywhere?)
    aBiffRange.maFirst.mnCol &= 0xFF;
    aBiffRange.maLast.mnCol &= 0xFF;
    if( !getAddressConverter().convertToCellRange( aData.maRange, aBiffRange, getSheetIndex(), true ) )
        return;

    BiffGuid aGuid;
    sal_uInt32 nVersion, nFlags;
    mrStrm >> aGuid >> nVersion >> nFlags;

    OSL_ENSURE( aGuid == BiffHelper::maGuidStdHlink, "BiffWorksheetFragment::importHyperlink - unexpected header GUID" );
    OSL_ENSURE( nVersion == 2, "BiffWorksheetFragment::importHyperlink - unexpected header version" );
    if( !(aGuid == BiffHelper::maGuidStdHlink) || (nVersion != 2) )
        return;

    // display string
    if( getFlag( nFlags, OLE_HYPERLINK_HASDISPLAY ) )
        aData.maDisplay = readHyperlinkString( getTextEncoding(), true );
    // frame string
    if( getFlag( nFlags, OLE_HYPERLINK_HASFRAME ) )
        aData.maFrame = readHyperlinkString( getTextEncoding(), true );

    // target
    if( getFlag( nFlags, OLE_HYPERLINK_HASTARGET ) )
    {
        if( getFlag( nFlags, OLE_HYPERLINK_ASSTRING ) )
        {
            OSL_ENSURE( getFlag( nFlags, OLE_HYPERLINK_ABSOLUTE ), "BiffWorksheetFragment::importHyperlink - link not absolute" );
            aData.maTarget = readHyperlinkString( getTextEncoding(), true );
        }
        else // hyperlink moniker
        {
            mrStrm >> aGuid;
            if( aGuid == BiffHelper::maGuidFileMoniker )
            {
                // file name, maybe relative and with directory up-count
                sal_Int16 nUpLevels;
                mrStrm >> nUpLevels;
                OSL_ENSURE( (nUpLevels == 0) || !getFlag( nFlags, OLE_HYPERLINK_ABSOLUTE ), "BiffWorksheetFragment::importHyperlink - absolute filename with upcount" );
                aData.maTarget = readHyperlinkString( getTextEncoding(), false );
                mrStrm.skip( 24 );
                sal_Int32 nBytes = mrStrm.readInt32();
                if( nBytes > 0 )
                {
                    sal_Int64 nEndPos = mrStrm.tell() + ::std::max< sal_Int32 >( nBytes, 0 );
                    sal_uInt16 nChars = getLimitedValue< sal_uInt16, sal_Int32 >( mrStrm.readInt32() / 2, 0, SAL_MAX_UINT16 );
                    mrStrm.skip( 2 );   // key value
                    aData.maTarget = mrStrm.readUnicodeArray( nChars ); // NOT null terminated
                    mrStrm.seek( nEndPos );
                }
                if( !getFlag( nFlags, OLE_HYPERLINK_ABSOLUTE ) )
                    for( sal_Int16 nLevel = 0; nLevel < nUpLevels; ++nLevel )
                        aData.maTarget = CREATE_OUSTRING( "../" ) + aData.maTarget;
            }
            else if( aGuid == BiffHelper::maGuidUrlMoniker )
            {
                // URL, maybe relative and with leading '../'
                sal_Int32 nBytes = mrStrm.readInt32();
                sal_Int64 nEndPos = mrStrm.tell() + ::std::max< sal_Int32 >( nBytes, 0 );
                aData.maTarget = mrStrm.readNulUnicodeArray();
                mrStrm.seek( nEndPos );
            }
            else
            {
                OSL_ENSURE( false, "BiffWorksheetFragment::importHyperlink - unsupported hyperlink moniker" );
                return;
            }
        }
    }

    // target location
    if( getFlag( nFlags, OLE_HYPERLINK_HASLOCATION ) )
        aData.maLocation = readHyperlinkString( getTextEncoding(), true );

    // try to read the SCREENTIP record
    if( (mrStrm.getNextRecId() == BIFF_ID_SCREENTIP) && mrStrm.startNextRecord() )
    {
        mrStrm.skip( 2 );      // repeated record id
        // the cell range, again
        mrStrm >> aBiffRange;
        CellRangeAddress aRange;
        if( getAddressConverter().convertToCellRange( aRange, aBiffRange, getSheetIndex(), true ) &&
            (aRange.StartColumn == aData.maRange.StartColumn) &&
            (aRange.StartRow == aData.maRange.StartRow) &&
            (aRange.EndColumn == aData.maRange.EndColumn) &&
            (aRange.EndRow == aData.maRange.EndRow) )
        {
            /*  This time, we have no string length, no flag field, and a
                null-terminated 16-bit character array. */
            aData.maTooltip = mrStrm.readNulUnicodeArray();
        }
    }

    // store the hyperlink settings
    setHyperlink( aData );
}

void BiffWorksheetFragment::importLabelRanges()
{
    BinRangeList aBiffRowRanges, aBiffColRanges;
    mrStrm >> aBiffRowRanges >> aBiffColRanges;
    ApiCellRangeList aColRanges, aRowRanges;
    getAddressConverter().convertToCellRangeList( aColRanges, aBiffColRanges, getSheetIndex(), true );
    getAddressConverter().convertToCellRangeList( aRowRanges, aBiffRowRanges, getSheetIndex(), true );
    setLabelRanges( aColRanges, aRowRanges );
}

void BiffWorksheetFragment::importMergedCells()
{
    BinRangeList aBiffRanges;
    mrStrm >> aBiffRanges;
    ApiCellRangeList aRanges;
    getAddressConverter().convertToCellRangeList( aRanges, aBiffRanges, getSheetIndex(), true );
    for( ApiCellRangeList::const_iterator aIt = aRanges.begin(), aEnd = aRanges.end(); aIt != aEnd; ++aIt )
        setMergedRange( *aIt );
}

void BiffWorksheetFragment::importPageBreaks( bool bRowBreak )
{
    OoxPageBreakData aData;
    aData.mbManual = true;              // only manual breaks stored in BIFF
    bool bBiff8 = getBiff() == BIFF8;   // skip start/end columns or rows in BIFF8

    sal_uInt16 nCount;
    mrStrm >> nCount;
    for( sal_uInt16 nIndex = 0; !mrStrm.isEof() && (nIndex < nCount); ++nIndex )
    {
        aData.mnColRow = mrStrm.readuInt16();
        setPageBreak( aData, bRowBreak );
        if( bBiff8 )
            mrStrm.skip( 4 );
    }
}

void BiffWorksheetFragment::importStandardWidth()
{
    sal_uInt16 nWidth;
    mrStrm >> nWidth;
    // width is stored as 1/256th of a character in BIFF, convert to entire character
    double fWidth = static_cast< double >( nWidth ) / 256.0;
    // set as default width, will override the width from DEFCOLWIDTH record
    setDefaultColumnWidth( fWidth );
}

// ============================================================================

} // namespace xls
} // namespace oox

