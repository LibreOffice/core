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

#include "oox/xls/worksheetfragment.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/core/relations.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/autofiltercontext.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/commentsfragment.hxx"
#include "oox/xls/condformatcontext.hxx"
#include "oox/xls/drawingfragment.hxx"
#include "oox/xls/externallinkbuffer.hxx"
#include "oox/xls/pagesettings.hxx"
#include "oox/xls/pivottablebuffer.hxx"
#include "oox/xls/pivottablefragment.hxx"
#include "oox/xls/querytablefragment.hxx"
#include "oox/xls/scenariobuffer.hxx"
#include "oox/xls/scenariocontext.hxx"
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
using ::oox::core::ContextHandlerRef;
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

const sal_uInt32 BIFF_SHRFEATHEAD_SHEETPROT = 2;

const sal_Int32 OOBIN_OLEOBJECT_CONTENT     = 1;
const sal_Int32 OOBIN_OLEOBJECT_ICON        = 4;
const sal_Int32 OOBIN_OLEOBJECT_ALWAYS      = 1;
const sal_Int32 OOBIN_OLEOBJECT_ONCALL      = 3;
const sal_uInt16 OOBIN_OLEOBJECT_LINKED     = 0x0001;
const sal_uInt16 OOBIN_OLEOBJECT_AUTOLOAD   = 0x0002;

} // namespace

// ============================================================================

OoxDataValidationsContext::OoxDataValidationsContext( OoxWorksheetFragmentBase& rFragment ) :
    OoxWorksheetContextBase( rFragment )
{
}

ContextHandlerRef OoxDataValidationsContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( dataValidations ):
            if( nElement == XLS_TOKEN( dataValidation ) )
            {
                importDataValidation( rAttribs );
                return this;
            }
        break;
        case XLS_TOKEN( dataValidation ):
            switch( nElement )
            {
                case XLS_TOKEN( formula1 ):
                case XLS_TOKEN( formula2 ):
                    return this;    // collect formulas in onEndElement()
            }
        break;
    }
    return 0;
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

void OoxDataValidationsContext::onEndElement( const OUString& rChars )
{
    if( mxValModel.get() ) switch( getCurrentElement() )
    {
        case XLS_TOKEN( formula1 ):
            mxValModel->maTokens1 = lclImportDataValFormula(
                getFormulaParser(), rChars, mxValModel->maRanges.getBaseAddress() );
            // process string list of a list validation (convert to list of string tokens)
            if( mxValModel->mnType == XML_list )
                getFormulaParser().convertStringToStringList( mxValModel->maTokens1, ',', true );
        break;
        case XLS_TOKEN( formula2 ):
            mxValModel->maTokens2 = lclImportDataValFormula(
                getFormulaParser(), rChars, mxValModel->maRanges.getBaseAddress() );
        break;
        case XLS_TOKEN( dataValidation ):
            setValidation( *mxValModel );
            mxValModel.reset();
        break;
    }
}


ContextHandlerRef OoxDataValidationsContext::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    if( nRecId == OOBIN_ID_DATAVALIDATION )
        importDataValidation( rStrm );
    return 0;
}

void OoxDataValidationsContext::importDataValidation( const AttributeList& rAttribs )
{
    mxValModel.reset( new ValidationModel );
    getAddressConverter().convertToCellRangeList( mxValModel->maRanges, rAttribs.getString( XML_sqref, OUString() ), getSheetIndex(), true );
    mxValModel->maInputTitle   = rAttribs.getXString( XML_promptTitle, OUString() );
    mxValModel->maInputMessage = rAttribs.getXString( XML_prompt, OUString() );
    mxValModel->maErrorTitle   = rAttribs.getXString( XML_errorTitle, OUString() );
    mxValModel->maErrorMessage = rAttribs.getXString( XML_error, OUString() );
    mxValModel->mnType         = rAttribs.getToken( XML_type, XML_none );
    mxValModel->mnOperator     = rAttribs.getToken( XML_operator, XML_between );
    mxValModel->mnErrorStyle   = rAttribs.getToken( XML_errorStyle, XML_stop );
    mxValModel->mbShowInputMsg = rAttribs.getBool( XML_showInputMessage, false );
    mxValModel->mbShowErrorMsg = rAttribs.getBool( XML_showErrorMessage, false );
    /*  The attribute showDropDown@dataValidation is in fact a "suppress
        dropdown" flag, as it was in the BIFF format! ECMA specification
        and attribute name are plain wrong! */
    mxValModel->mbNoDropDown   = rAttribs.getBool( XML_showDropDown, false );
    mxValModel->mbAllowBlank   = rAttribs.getBool( XML_allowBlank, false );
}

void OoxDataValidationsContext::importDataValidation( RecordInputStream& rStrm )
{
    ValidationModel aModel;

    sal_uInt32 nFlags;
    BinRangeList aRanges;
    rStrm >> nFlags >> aRanges >> aModel.maErrorTitle >> aModel.maErrorMessage >> aModel.maInputTitle >> aModel.maInputMessage;

    // equal flags in BIFF and OOBIN
    aModel.setBinType( extractValue< sal_uInt8 >( nFlags, 0, 4 ) );
    aModel.setBinOperator( extractValue< sal_uInt8 >( nFlags, 20, 4 ) );
    aModel.setBinErrorStyle( extractValue< sal_uInt8 >( nFlags, 4, 3 ) );
    aModel.mbAllowBlank   = getFlag( nFlags, BIFF_DATAVAL_ALLOWBLANK );
    aModel.mbNoDropDown   = getFlag( nFlags, BIFF_DATAVAL_NODROPDOWN );
    aModel.mbShowInputMsg = getFlag( nFlags, BIFF_DATAVAL_SHOWINPUT );
    aModel.mbShowErrorMsg = getFlag( nFlags, BIFF_DATAVAL_SHOWERROR );

    // cell range list
    getAddressConverter().convertToCellRangeList( aModel.maRanges, aRanges, getSheetIndex(), true );

    // condition formula(s)
    FormulaParser& rParser = getFormulaParser();
    TokensFormulaContext aContext( true, false );
    aContext.setBaseAddress( aModel.maRanges.getBaseAddress() );
    rParser.importFormula( aContext, rStrm );
    aModel.maTokens1 = aContext.getTokens();
    rParser.importFormula( aContext, rStrm );
    aModel.maTokens2 = aContext.getTokens();
    // process string list of a list validation (convert to list of string tokens)
    if( (aModel.mnType == XML_list) && getFlag( nFlags, BIFF_DATAVAL_STRINGLIST ) )
        rParser.convertStringToStringList( aModel.maTokens1, ',', true );

    // set validation data
    setValidation( aModel );
}

// ============================================================================

OoxWorksheetFragment::OoxWorksheetFragment( const WorkbookHelper& rHelper,
        const OUString& rFragmentPath, ISegmentProgressBarRef xProgressBar, WorksheetType eSheetType, sal_Int16 nSheet ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath, xProgressBar, eSheetType, nSheet )
{
    // import data tables related to this worksheet
    RelationsRef xTableRels = getRelations().getRelationsFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "table" ) );
    for( Relations::const_iterator aIt = xTableRels->begin(), aEnd = xTableRels->end(); aIt != aEnd; ++aIt )
        importOoxFragment( new OoxTableFragment( *this, getFragmentPathFromRelation( aIt->second ) ) );

    // import comments related to this worksheet
    OUString aCommentsFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATIONSTYPE( "comments" ) );
    if( aCommentsFragmentPath.getLength() > 0 )
        importOoxFragment( new OoxCommentsFragment( *this, aCommentsFragmentPath ) );
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextHandlerRef OoxWorksheetFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT: switch( getSheetType() )
        {
            case SHEETTYPE_WORKSHEET:   return (nElement == XLS_TOKEN( worksheet )) ? this : 0;
            case SHEETTYPE_CHARTSHEET:  return 0;
            case SHEETTYPE_MACROSHEET:  return (nElement == XM_TOKEN( macrosheet )) ? this : 0;
            case SHEETTYPE_DIALOGSHEET: return (nElement == XM_TOKEN( dialogsheet )) ? this : 0;
            case SHEETTYPE_MODULESHEET: return 0;
            case SHEETTYPE_EMPTYSHEET:  return 0;
        }
        break;

        case XLS_TOKEN( worksheet ):
        case XM_TOKEN( macrosheet ):
            switch( nElement )
            {
                case XLS_TOKEN( sheetData ):                return new OoxSheetDataContext( *this );
                case XLS_TOKEN( autoFilter ):               return new OoxAutoFilterContext( *this );
                case XLS_TOKEN( conditionalFormatting ):    return new OoxCondFormatContext( *this );
                case XLS_TOKEN( dataValidations ):          return new OoxDataValidationsContext( *this );

                case XLS_TOKEN( sheetViews ):
                case XLS_TOKEN( cols ):
                case XLS_TOKEN( mergeCells ):
                case XLS_TOKEN( hyperlinks ):
                case XLS_TOKEN( rowBreaks ):
                case XLS_TOKEN( colBreaks ):
                case XLS_TOKEN( oleObjects ):
                case XLS_TOKEN( controls ):         return this;

                case XLS_TOKEN( sheetPr ):          getWorksheetSettings().importSheetPr( rAttribs );               return this;
                case XLS_TOKEN( dimension ):        importDimension( rAttribs );                                    break;
                case XLS_TOKEN( sheetFormatPr ):    importSheetFormatPr( rAttribs );                                break;
                case XLS_TOKEN( sheetProtection ):  getWorksheetSettings().importSheetProtection( rAttribs );       break;
                case XLS_TOKEN( phoneticPr ):       getWorksheetSettings().importPhoneticPr( rAttribs );            break;
                case XLS_TOKEN( printOptions ):     getPageSettings().importPrintOptions( rAttribs );               break;
                case XLS_TOKEN( pageMargins ):      getPageSettings().importPageMargins( rAttribs );                break;
                case XLS_TOKEN( pageSetup ):        getPageSettings().importPageSetup( getRelations(), rAttribs );  break;
                case XLS_TOKEN( headerFooter ):     getPageSettings().importHeaderFooter( rAttribs );               return this;
                case XLS_TOKEN( picture ):          getPageSettings().importPicture( getRelations(), rAttribs );    break;
                case XLS_TOKEN( drawing ):          importDrawing( rAttribs );                                      break;
                case XLS_TOKEN( legacyDrawing ):    importLegacyDrawing( rAttribs );                                break;
                case XLS_TOKEN( scenarios ):
                    return new OoxScenariosContext( *this );
            }
        break;

        case XLS_TOKEN( sheetPr ):
            switch( nElement )
            {
                case XLS_TOKEN( tabColor ):         getWorksheetSettings().importTabColor( rAttribs );              break;
                case XLS_TOKEN( outlinePr ):        getWorksheetSettings().importOutlinePr( rAttribs );             break;
                case XLS_TOKEN( pageSetUpPr ):      importPageSetUpPr( rAttribs );                                  break;
            }
        break;

        case XLS_TOKEN( sheetViews ):
            switch( nElement )
            {
                case XLS_TOKEN( sheetView ):        getSheetViewSettings().importSheetView( rAttribs );             return this;
            }
        break;
        case XLS_TOKEN( sheetView ):
            switch( nElement )
            {
                case XLS_TOKEN( pane ):             getSheetViewSettings().importPane( rAttribs );                  break;
                case XLS_TOKEN( selection ):        getSheetViewSettings().importSelection( rAttribs );             break;
            }
        break;

        case XLS_TOKEN( cols ):
            if( nElement == XLS_TOKEN( col ) ) importCol( rAttribs );
        break;
        case XLS_TOKEN( mergeCells ):
            if( nElement == XLS_TOKEN( mergeCell ) ) importMergeCell( rAttribs );
        break;
        case XLS_TOKEN( hyperlinks ):
            if( nElement == XLS_TOKEN( hyperlink ) ) importHyperlink( rAttribs );
        break;
        case XLS_TOKEN( rowBreaks ):
            if( nElement == XLS_TOKEN( brk ) ) importBrk( rAttribs, true );
        break;
        case XLS_TOKEN( colBreaks ):
            if( nElement == XLS_TOKEN( brk ) ) importBrk( rAttribs, false );
        break;

        case XLS_TOKEN( headerFooter ):
            switch( nElement )
            {
                case XLS_TOKEN( firstHeader ):
                case XLS_TOKEN( firstFooter ):
                case XLS_TOKEN( oddHeader ):
                case XLS_TOKEN( oddFooter ):
                case XLS_TOKEN( evenHeader ):
                case XLS_TOKEN( evenFooter ):       return this;    // collect h/f contents in onEndElement()
            }
        break;

        case XLS_TOKEN( oleObjects ):
            if( nElement == XLS_TOKEN( oleObject ) ) importOleObject( rAttribs );
        break;
        case XLS_TOKEN( controls ):
            if( nElement == XLS_TOKEN( control ) ) importControl( rAttribs );
        break;
    }
    return 0;
}

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
    }
}

ContextHandlerRef OoxWorksheetFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == OOBIN_ID_WORKSHEET ) return this;
        break;

        case OOBIN_ID_WORKSHEET:
            switch( nRecId )
            {
                case OOBIN_ID_SHEETDATA:        return new OoxSheetDataContext( *this );
                case OOBIN_ID_CONDFORMATTING:   return new OoxCondFormatContext( *this );
                case OOBIN_ID_DATAVALIDATIONS:  return new OoxDataValidationsContext( *this );

                case OOBIN_ID_SHEETVIEWS:
                case OOBIN_ID_COLS:
                case OOBIN_ID_MERGECELLS:
                case OOBIN_ID_ROWBREAKS:
                case OOBIN_ID_COLBREAKS:
                case OOBIN_ID_OLEOBJECTS:
                case OOBIN_ID_CONTROLS:         return this;

                case OOBIN_ID_SHEETPR:          getWorksheetSettings().importSheetPr( rStrm );              break;
                case OOBIN_ID_DIMENSION:        importDimension( rStrm );                                   break;
                case OOBIN_ID_SHEETFORMATPR:    importSheetFormatPr( rStrm );                               break;
                case OOBIN_ID_HYPERLINK:        importHyperlink( rStrm );                                   break;
                case OOBIN_ID_PAGEMARGINS:      getPageSettings().importPageMargins( rStrm );               break;
                case OOBIN_ID_PAGESETUP:        getPageSettings().importPageSetup( getRelations(), rStrm ); break;
                case OOBIN_ID_PRINTOPTIONS:     getPageSettings().importPrintOptions( rStrm );              break;
                case OOBIN_ID_HEADERFOOTER:     getPageSettings().importHeaderFooter( rStrm );              break;
                case OOBIN_ID_PICTURE:          getPageSettings().importPicture( getRelations(), rStrm );   break;
                case OOBIN_ID_SHEETPROTECTION:  getWorksheetSettings().importSheetProtection( rStrm );      break;
                case OOBIN_ID_PHONETICPR:       getWorksheetSettings().importPhoneticPr( rStrm );           break;
                case OOBIN_ID_DRAWING:          importDrawing( rStrm );                                     break;
                case OOBIN_ID_LEGACYDRAWING:    importLegacyDrawing( rStrm );                               break;
                case OOBIN_ID_SCENARIOS:
                    return new OoxScenariosContext( *this );
            }
        break;

        case OOBIN_ID_SHEETVIEWS:
            switch( nRecId )
            {
                case OOBIN_ID_SHEETVIEW:        getSheetViewSettings().importSheetView( rStrm );            return this;
            }
        break;
        case OOBIN_ID_SHEETVIEW:
            switch( nRecId )
            {
                case OOBIN_ID_PANE:             getSheetViewSettings().importPane( rStrm );                 break;
                case OOBIN_ID_SELECTION:        getSheetViewSettings().importSelection( rStrm );            break;
            }
        break;

        case OOBIN_ID_COLS:
            if( nRecId == OOBIN_ID_COL ) importCol( rStrm );
        break;
        case OOBIN_ID_MERGECELLS:
            if( nRecId == OOBIN_ID_MERGECELL ) importMergeCell( rStrm );
        break;
        case OOBIN_ID_ROWBREAKS:
            if( nRecId == OOBIN_ID_BRK ) importBrk( rStrm, true );
        break;
        case OOBIN_ID_COLBREAKS:
            if( nRecId == OOBIN_ID_BRK ) importBrk( rStrm, false );
        break;
        case OOBIN_ID_OLEOBJECTS:
            if( nRecId == OOBIN_ID_OLEOBJECT ) importOleObject( rStrm );
        break;
        case OOBIN_ID_CONTROLS:
            if( nRecId == OOBIN_ID_CONTROL ) importControl( rStrm );
        break;
    }
    return 0;
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
        { OOBIN_ID_SCENARIO,            OOBIN_ID_SCENARIO + 1           },
        { OOBIN_ID_SCENARIOS,           OOBIN_ID_SCENARIOS + 1          },
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
        importOoxFragment( new OoxQueryTableFragment( *this, getFragmentPathFromRelation( aIt->second ) ) );

    // import pivot table fragments related to this worksheet
    RelationsRef xPivotRels = getRelations().getRelationsFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "pivotTable" ) );
    for( Relations::const_iterator aIt = xPivotRels->begin(), aEnd = xPivotRels->end(); aIt != aEnd; ++aIt )
        importOoxFragment( new OoxPivotTableFragment( *this, getFragmentPathFromRelation( aIt->second ) ) );
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
    /*  OOXML stores the used area, if existing, or "A1" if the sheet is empty.
        In case of "A1", the dimension at the WorksheetHelper object will not
        be set. If the cell A1 exists, the used area will be updated while
        importing the cell. */
    if( (aRange.EndColumn > 0) || (aRange.EndRow > 0) )
        extendUsedArea( aRange );
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
    ColumnModel aModel;
    aModel.mnFirstCol     = rAttribs.getInteger( XML_min, -1 );
    aModel.mnLastCol      = rAttribs.getInteger( XML_max, -1 );
    aModel.mfWidth        = rAttribs.getDouble( XML_width, 0.0 );
    aModel.mnXfId         = rAttribs.getInteger( XML_style, -1 );
    aModel.mnLevel        = rAttribs.getInteger( XML_outlineLevel, 0 );
    aModel.mbShowPhonetic = rAttribs.getBool( XML_phonetic, false );
    aModel.mbHidden       = rAttribs.getBool( XML_hidden, false );
    aModel.mbCollapsed    = rAttribs.getBool( XML_collapsed, false );
    // set column properties in the current sheet
    setColumnModel( aModel );
}

void OoxWorksheetFragment::importMergeCell( const AttributeList& rAttribs )
{
    CellRangeAddress aRange;
    if( getAddressConverter().convertToCellRange( aRange, rAttribs.getString( XML_ref, OUString() ), getSheetIndex(), true, true ) )
        setMergedRange( aRange );
}

void OoxWorksheetFragment::importHyperlink( const AttributeList& rAttribs )
{
    HyperlinkModel aModel;
    if( getAddressConverter().convertToCellRange( aModel.maRange, rAttribs.getString( XML_ref, OUString() ), getSheetIndex(), true, true ) )
    {
        aModel.maTarget   = getRelations().getExternalTargetFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
        aModel.maLocation = rAttribs.getXString( XML_location, OUString() );
        aModel.maDisplay  = rAttribs.getXString( XML_display, OUString() );
        aModel.maTooltip  = rAttribs.getXString( XML_tooltip, OUString() );
        setHyperlink( aModel );
    }
}

void OoxWorksheetFragment::importBrk( const AttributeList& rAttribs, bool bRowBreak )
{
    PageBreakModel aModel;
    aModel.mnColRow = rAttribs.getInteger( XML_id, 0 );
    aModel.mnMin    = rAttribs.getInteger( XML_min, aModel.mnColRow );
    aModel.mnMax    = rAttribs.getInteger( XML_max, aModel.mnColRow );
    aModel.mbManual = rAttribs.getBool( XML_man, false );
    setPageBreak( aModel, bRowBreak );
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
    ::oox::vml::OleObjectInfo aInfo;
    aInfo.setShapeId( rAttribs.getInteger( XML_shapeId, 0 ) );
    OSL_ENSURE( rAttribs.hasAttribute( XML_link ) != rAttribs.hasAttribute( R_TOKEN( id ) ),
        "OoxWorksheetFragment::importOleObject - OLE object must be either linked or embedded" );
    aInfo.mbLinked = rAttribs.hasAttribute( XML_link );
    if( aInfo.mbLinked )
        aInfo.maTargetLink = getFormulaParser().importOleTargetLink( rAttribs.getString( XML_link, OUString() ) );
    else if( rAttribs.hasAttribute( R_TOKEN( id ) ) )
        importEmbeddedOleData( aInfo.maEmbeddedData, rAttribs.getString( R_TOKEN( id ), OUString() ) );
    aInfo.maProgId = rAttribs.getString( XML_progId, OUString() );
    aInfo.mbShowAsIcon = rAttribs.getToken( XML_dvAspect, XML_DVASPECT_CONTENT ) == XML_DVASPECT_ICON;
    aInfo.mbAutoUpdate = rAttribs.getToken( XML_oleUpdate, XML_OLEUPDATE_ONCALL ) == XML_OLEUPDATE_ALWAYS;
    aInfo.mbAutoLoad = rAttribs.getBool( XML_autoLoad, false );
    getVmlDrawing().registerOleObject( aInfo );
}

void OoxWorksheetFragment::importControl( const AttributeList& rAttribs )
{
    ::oox::vml::ControlInfo aInfo;
    aInfo.setShapeId( rAttribs.getInteger( XML_shapeId, 0 ) );
    aInfo.maFragmentPath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
    aInfo.maName = rAttribs.getString( XML_name, OUString() );
    getVmlDrawing().registerControl( aInfo );
}

void OoxWorksheetFragment::importDimension( RecordInputStream& rStrm )
{
    BinRange aBinRange;
    aBinRange.read( rStrm );
    CellRangeAddress aRange;
    getAddressConverter().convertToCellRangeUnchecked( aRange, aBinRange, getSheetIndex() );
    /*  BIFF12 stores the used area, if existing, or "A1" if the sheet is
        empty. In case of "A1", the dimension at the WorksheetHelper object
        will not be set. If the cell A1 exists, the used area will be updated
        while importing the cell. */
    if( (aRange.EndColumn > 0) || (aRange.EndRow > 0) )
        extendUsedArea( aRange );
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
    ColumnModel aModel;

    sal_Int32 nWidth;
    sal_uInt16 nFlags;
    rStrm >> aModel.mnFirstCol >> aModel.mnLastCol >> nWidth >> aModel.mnXfId >> nFlags;

    // column indexes are 0-based in OOBIN, but ColumnModel expects 1-based
    ++aModel.mnFirstCol;
    ++aModel.mnLastCol;
    // width is stored as 1/256th of a character in OOBIN, convert to entire character
    aModel.mfWidth        = static_cast< double >( nWidth ) / 256.0;
    // equal flags in BIFF and OOBIN
    aModel.mnLevel        = extractValue< sal_Int32 >( nFlags, 8, 3 );
    aModel.mbShowPhonetic = getFlag( nFlags, BIFF_COLINFO_SHOWPHONETIC );
    aModel.mbHidden       = getFlag( nFlags, BIFF_COLINFO_HIDDEN );
    aModel.mbCollapsed    = getFlag( nFlags, BIFF_COLINFO_COLLAPSED );
    // set column properties in the current sheet
    setColumnModel( aModel );
}

void OoxWorksheetFragment::importMergeCell( RecordInputStream& rStrm )
{
    BinRange aBinRange;
    rStrm >> aBinRange;
    CellRangeAddress aRange;
    if( getAddressConverter().convertToCellRange( aRange, aBinRange, getSheetIndex(), true, true ) )
        setMergedRange( aRange );
}

void OoxWorksheetFragment::importHyperlink( RecordInputStream& rStrm )
{
    BinRange aBinRange;
    rStrm >> aBinRange;
    HyperlinkModel aModel;
    if( getAddressConverter().convertToCellRange( aModel.maRange, aBinRange, getSheetIndex(), true, true ) )
    {
        aModel.maTarget = getRelations().getExternalTargetFromRelId( rStrm.readString() );
        rStrm >> aModel.maLocation >> aModel.maTooltip >> aModel.maDisplay;
        setHyperlink( aModel );
    }
}

void OoxWorksheetFragment::importBrk( RecordInputStream& rStrm, bool bRowBreak )
{
    PageBreakModel aModel;
    sal_Int32 nManual;
    rStrm >> aModel.mnColRow >> aModel.mnMin >> aModel.mnMax >> nManual;
    aModel.mbManual = nManual != 0;
    setPageBreak( aModel, bRowBreak );
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
    ::oox::vml::OleObjectInfo aInfo;
    sal_Int32 nAspect, nUpdateMode, nShapeId;
    sal_uInt16 nFlags;
    rStrm >> nAspect >> nUpdateMode >> nShapeId >> nFlags >> aInfo.maProgId;
    aInfo.mbLinked = getFlag( nFlags, OOBIN_OLEOBJECT_LINKED );
    if( aInfo.mbLinked )
        aInfo.maTargetLink = getFormulaParser().importOleTargetLink( rStrm );
    else
        importEmbeddedOleData( aInfo.maEmbeddedData, rStrm.readString() );
    aInfo.setShapeId( nShapeId );
    aInfo.mbShowAsIcon = nAspect == OOBIN_OLEOBJECT_ICON;
    aInfo.mbAutoUpdate = nUpdateMode == OOBIN_OLEOBJECT_ALWAYS;
    aInfo.mbAutoLoad = getFlag( nFlags, OOBIN_OLEOBJECT_AUTOLOAD );
    getVmlDrawing().registerOleObject( aInfo );
}

void OoxWorksheetFragment::importControl( RecordInputStream& rStrm )
{
    ::oox::vml::ControlInfo aInfo;
    aInfo.setShapeId( rStrm.readInt32() );
    aInfo.maFragmentPath = getFragmentPathFromRelId( rStrm.readString() );
    rStrm >> aInfo.maName;
    getVmlDrawing().registerControl( aInfo );
}

void OoxWorksheetFragment::importEmbeddedOleData( StreamDataSequence& orEmbeddedData, const OUString& rRelId )
{
    OUString aFragmentPath = getFragmentPathFromRelId( rRelId );
    if( aFragmentPath.getLength() > 0 )
        getBaseFilter().importBinaryData( orEmbeddedData, aFragmentPath );
}

// ============================================================================

BiffWorksheetFragment::BiffWorksheetFragment( const BiffWorkbookFragmentBase& rParent, ISegmentProgressBarRef xProgressBar, WorksheetType eSheetType, sal_Int16 nSheet ) :
    BiffWorksheetFragmentBase( rParent, xProgressBar, eSheetType, nSheet )
{
}

BiffWorksheetFragment::~BiffWorksheetFragment()
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
                        case BIFF_ID_PTDEFINITION:  importPTDefinition();                           break;
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
                        case BIFF_ID_CODENAME:          rWorksheetSett.importCodeName( mrStrm );        break;
                        case BIFF_ID_COLINFO:           importColInfo();                                break;
                        case BIFF_ID_DATAVALIDATION:    importDataValidation();                         break;
                        case BIFF_ID_DATAVALIDATIONS:   importDataValidations();                        break;
                        case BIFF3_ID_DEFROWHEIGHT:     importDefRowHeight();                           break;
                        case BIFF_ID_HCENTER:           rPageSett.importHorCenter( mrStrm );            break;
                        case BIFF_ID_HYPERLINK:         importHyperlink();                              break;
                        case BIFF_ID_LABELRANGES:       importLabelRanges();                            break;
                        case BIFF_ID_MERGEDCELLS:       importMergedCells();                            break;
                        case BIFF_ID_OBJECTPROTECT:     rWorksheetSett.importObjectProtect( mrStrm );   break;
                        case BIFF_ID_PAGESETUP:         rPageSett.importPageSetup( mrStrm );            break;
                        case BIFF_ID_PHONETICPR:        rWorksheetSett.importPhoneticPr( mrStrm );      break;
                        case BIFF_ID_PICTURE:           rPageSett.importPicture( mrStrm );              break;
                        case BIFF_ID_PTDEFINITION:      importPTDefinition();                           break;
                        case BIFF_ID_SAVERECALC:        rWorkbookSett.importSaveRecalc( mrStrm );       break;
                        case BIFF_ID_SCENARIOS:         importScenarios();                              break;
                        case BIFF_ID_SCENPROTECT:       rWorksheetSett.importScenProtect( mrStrm );     break;
                        case BIFF_ID_SCL:               rSheetViewSett.importScl( mrStrm );             break;
                        case BIFF_ID_SHEETPR:           rWorksheetSett.importSheetPr( mrStrm );         break;
                        case BIFF_ID_SHAREDFEATHEAD:    importSharedFeatHead();                         break;
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
            // record still not processed, try pivot table records
            if( mxPTContext.get() && (mrStrm.tellBase() == nStrmPos) )
                mxPTContext->importRecord();
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

    ColumnModel aModel;
    // column indexes are 0-based in BIFF, but ColumnModel expects 1-based
    aModel.mnFirstCol     = static_cast< sal_Int32 >( nFirstCol ) + 1;
    aModel.mnLastCol      = static_cast< sal_Int32 >( nLastCol ) + 1;
    // width is stored as 1/256th of a character in BIFF, convert to entire character
    aModel.mfWidth        = static_cast< double >( nWidth ) / 256.0;
    aModel.mnXfId         = nXfId;
    aModel.mnLevel        = extractValue< sal_Int32 >( nFlags, 8, 3 );
    aModel.mbShowPhonetic = getFlag( nFlags, BIFF_COLINFO_SHOWPHONETIC );
    aModel.mbHidden       = getFlag( nFlags, BIFF_COLINFO_HIDDEN );
    aModel.mbCollapsed    = getFlag( nFlags, BIFF_COLINFO_COLLAPSED );
    // set column properties in the current sheet
    setColumnModel( aModel );
}

void BiffWorksheetFragment::importColumnDefault()
{
    sal_uInt16 nFirstCol, nLastCol, nXfId;
    mrStrm >> nFirstCol >> nLastCol >> nXfId;
    setDefaultColumnFormat( nFirstCol, nLastCol, nXfId );
}

void BiffWorksheetFragment::importColWidth()
{
    sal_uInt8 nFirstCol, nLastCol;
    sal_uInt16 nWidth;
    mrStrm >> nFirstCol >> nLastCol >> nWidth;

    ColumnModel aModel;
    // column indexes are 0-based in BIFF, but ColumnModel expects 1-based
    aModel.mnFirstCol = static_cast< sal_Int32 >( nFirstCol ) + 1;
    aModel.mnLastCol = static_cast< sal_Int32 >( nLastCol ) + 1;
    // width is stored as 1/256th of a character in BIFF, convert to entire character
    aModel.mfWidth = static_cast< double >( nWidth ) / 256.0;
    // set column properties in the current sheet
    setColumnModel( aModel );
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
    OUString aMessage = rStrm.readUniString( true );
    if( (aMessage.getLength() == 1) && (aMessage[ 0 ] == 0) )
        aMessage = OUString();
    return aMessage;
}

ApiTokenSequence lclReadDataValFormula( BiffInputStream& rStrm, FormulaParser& rParser )
{
    sal_uInt16 nFmlaSize = rStrm.readuInt16();
    rStrm.skip( 2 );
    // enable NUL characters, string list is single tStr token with NUL separators
    TokensFormulaContext aContext( true, false, true );
    rParser.importFormula( aContext, rStrm, &nFmlaSize );
    return aContext.getTokens();
}

} // namespace

void BiffWorksheetFragment::importDataValidation()
{
    ValidationModel aModel;

    // flags
    sal_uInt32 nFlags;
    mrStrm >> nFlags;
    aModel.setBinType( extractValue< sal_uInt8 >( nFlags, 0, 4 ) );
    aModel.setBinOperator( extractValue< sal_uInt8 >( nFlags, 20, 4 ) );
    aModel.setBinErrorStyle( extractValue< sal_uInt8 >( nFlags, 4, 3 ) );
    aModel.mbAllowBlank   = getFlag( nFlags, BIFF_DATAVAL_ALLOWBLANK );
    aModel.mbNoDropDown   = getFlag( nFlags, BIFF_DATAVAL_NODROPDOWN );
    aModel.mbShowInputMsg = getFlag( nFlags, BIFF_DATAVAL_SHOWINPUT );
    aModel.mbShowErrorMsg = getFlag( nFlags, BIFF_DATAVAL_SHOWERROR );

    // message strings
    aModel.maInputTitle   = lclReadDataValMessage( mrStrm );
    aModel.maErrorTitle   = lclReadDataValMessage( mrStrm );
    aModel.maInputMessage = lclReadDataValMessage( mrStrm );
    aModel.maErrorMessage = lclReadDataValMessage( mrStrm );

    // condition formula(s)
    FormulaParser& rParser = getFormulaParser();
    aModel.maTokens1 = lclReadDataValFormula( mrStrm, rParser );
    aModel.maTokens2 = lclReadDataValFormula( mrStrm, rParser );
    // process string list of a list validation (convert to list of string tokens)
    if( (aModel.mnType == XML_list) && getFlag( nFlags, BIFF_DATAVAL_STRINGLIST ) )
        rParser.convertStringToStringList( aModel.maTokens1, '\0', true );

    // cell range list
    BinRangeList aRanges;
    mrStrm >> aRanges;
    getAddressConverter().convertToCellRangeList( aModel.maRanges, aRanges, getSheetIndex(), true );

    // set validation data
    setValidation( aModel );
}

void BiffWorksheetFragment::importDimension()
{
    // 32-bit row indexes in BIFF8
    bool bInt32Rows = (mrStrm.getRecId() == BIFF3_ID_DIMENSION) && (getBiff() == BIFF8);
    BinRange aBinRange;
    aBinRange.read( mrStrm, true, bInt32Rows );
    /*  BIFF stores the used area with end column and end row increased by 1
        (first unused column and row). */
    if( (aBinRange.maFirst.mnCol < aBinRange.maLast.mnCol) && (aBinRange.maFirst.mnRow < aBinRange.maLast.mnRow) )
    {
        // reduce range to used area
        --aBinRange.maLast.mnCol;
        --aBinRange.maLast.mnRow;
        CellRangeAddress aRange;
        getAddressConverter().convertToCellRangeUnchecked( aRange, aBinRange, getSheetIndex() );
        extendUsedArea( aRange );
    }
}

void BiffWorksheetFragment::importHyperlink()
{
    HyperlinkModel aModel;

    // read cell range for the hyperlink
    BinRange aBiffRange;
    mrStrm >> aBiffRange;
    // #i80006# Excel silently ignores invalid hi-byte of column index (TODO: everywhere?)
    aBiffRange.maFirst.mnCol &= 0xFF;
    aBiffRange.maLast.mnCol &= 0xFF;
    if( !getAddressConverter().convertToCellRange( aModel.maRange, aBiffRange, getSheetIndex(), true, true ) )
        return;

    // try to read the StdHlink data
    if( !::oox::ole::OleHelper::importStdHlink( aModel, mrStrm, true ) )
        return;

    // try to read the optional following SCREENTIP record
    if( (mrStrm.getNextRecId() == BIFF_ID_SCREENTIP) && mrStrm.startNextRecord() )
    {
        mrStrm.skip( 2 );      // repeated record id
        // the cell range, again
        mrStrm >> aBiffRange;
        CellRangeAddress aRange;
        if( getAddressConverter().convertToCellRange( aRange, aBiffRange, getSheetIndex(), true, true ) &&
            (aRange.StartColumn == aModel.maRange.StartColumn) &&
            (aRange.StartRow == aModel.maRange.StartRow) &&
            (aRange.EndColumn == aModel.maRange.EndColumn) &&
            (aRange.EndRow == aModel.maRange.EndRow) )
        {
            /*  This time, we have no string length, no flag field, and a
                null-terminated 16-bit character array. */
            aModel.maTooltip = mrStrm.readNulUnicodeArray();
        }
    }

    // store the hyperlink settings
    setHyperlink( aModel );
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
    PageBreakModel aModel;
    aModel.mbManual = true;             // only manual breaks stored in BIFF
    bool bBiff8 = getBiff() == BIFF8;   // skip start/end columns or rows in BIFF8

    sal_uInt16 nCount;
    mrStrm >> nCount;
    for( sal_uInt16 nIndex = 0; !mrStrm.isEof() && (nIndex < nCount); ++nIndex )
    {
        aModel.mnColRow = mrStrm.readuInt16();
        setPageBreak( aModel, bRowBreak );
        if( bBiff8 )
            mrStrm.skip( 4 );
    }
}

void BiffWorksheetFragment::importPTDefinition()
{
    mxPTContext.reset( new BiffPivotTableContext( *this, getPivotTables().createPivotTable() ) );
    mxPTContext->importRecord();
}

void BiffWorksheetFragment::importScenarios()
{
    getScenarios().createSheetScenarios( getSheetIndex() ).importScenarios( mrStrm );
}

void BiffWorksheetFragment::importSharedFeatHead()
{
    mrStrm.skip( 12 );
    sal_uInt16 nType = mrStrm.readuInt16();
    mrStrm.skip( 5 );
    switch( nType )
    {
        case BIFF_SHRFEATHEAD_SHEETPROT:
            if( mrStrm.getRemaining() >= 4 )
                getWorksheetSettings().importSheetProtection( mrStrm );
        break;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
