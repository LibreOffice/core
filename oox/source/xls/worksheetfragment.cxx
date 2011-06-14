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

#include "oox/core/filterbase.hxx"
#include "oox/core/relations.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/xls/addressconverter.hxx"
#include "oox/xls/autofilterbuffer.hxx"
#include "oox/xls/autofiltercontext.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/commentsfragment.hxx"
#include "oox/xls/condformatcontext.hxx"
#include "oox/xls/drawingfragment.hxx"
#include "oox/xls/externallinkbuffer.hxx"
#include "oox/xls/pagesettings.hxx"
#include "oox/xls/pivottablefragment.hxx"
#include "oox/xls/querytablefragment.hxx"
#include "oox/xls/scenariobuffer.hxx"
#include "oox/xls/scenariocontext.hxx"
#include "oox/xls/sheetdatacontext.hxx"
#include "oox/xls/tablefragment.hxx"
#include "oox/xls/viewsettings.hxx"
#include "oox/xls/workbooksettings.hxx"
#include "oox/xls/worksheetsettings.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;
using namespace ::oox::core;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

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

const sal_Int32 BIFF12_OLEOBJECT_CONTENT    = 1;
const sal_Int32 BIFF12_OLEOBJECT_ICON       = 4;
const sal_Int32 BIFF12_OLEOBJECT_ALWAYS     = 1;
const sal_Int32 BIFF12_OLEOBJECT_ONCALL     = 3;
const sal_uInt16 BIFF12_OLEOBJECT_LINKED    = 0x0001;
const sal_uInt16 BIFF12_OLEOBJECT_AUTOLOAD  = 0x0002;

} // namespace

// ============================================================================

DataValidationsContext::DataValidationsContext( WorksheetFragmentBase& rFragment ) :
    WorksheetContextBase( rFragment )
{
}

ContextHandlerRef DataValidationsContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
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
                    return this;    // collect formulas in onCharacters()
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

void DataValidationsContext::onCharacters( const OUString& rChars )
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
    }
}

void DataValidationsContext::onEndElement()
{
    if( isCurrentElement( XLS_TOKEN( dataValidation ) ) && mxValModel.get() )
    {
        setValidation( *mxValModel );
        mxValModel.reset();
    }
}


ContextHandlerRef DataValidationsContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    if( nRecId == BIFF12_ID_DATAVALIDATION )
        importDataValidation( rStrm );
    return 0;
}

void DataValidationsContext::importDataValidation( const AttributeList& rAttribs )
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

void DataValidationsContext::importDataValidation( SequenceInputStream& rStrm )
{
    ValidationModel aModel;

    sal_uInt32 nFlags;
    BinRangeList aRanges;
    rStrm >> nFlags >> aRanges >> aModel.maErrorTitle >> aModel.maErrorMessage >> aModel.maInputTitle >> aModel.maInputMessage;

    // equal flags in all BIFFs
    aModel.setBiffType( extractValue< sal_uInt8 >( nFlags, 0, 4 ) );
    aModel.setBiffOperator( extractValue< sal_uInt8 >( nFlags, 20, 4 ) );
    aModel.setBiffErrorStyle( extractValue< sal_uInt8 >( nFlags, 4, 3 ) );
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

WorksheetFragment::WorksheetFragment( const WorkbookHelper& rHelper,
        const OUString& rFragmentPath, const ISegmentProgressBarRef& rxProgressBar, WorksheetType eSheetType, sal_Int16 nSheet ) :
    WorksheetFragmentBase( rHelper, rFragmentPath, rxProgressBar, eSheetType, nSheet )
{
    // import data tables related to this worksheet
    RelationsRef xTableRels = getRelations().getRelationsFromType( CREATE_OFFICEDOC_RELATION_TYPE( "table" ) );
    for( Relations::const_iterator aIt = xTableRels->begin(), aEnd = xTableRels->end(); aIt != aEnd; ++aIt )
        importOoxFragment( new TableFragment( *this, getFragmentPathFromRelation( aIt->second ) ) );

    // import comments related to this worksheet
    OUString aCommentsFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "comments" ) );
    if( aCommentsFragmentPath.getLength() > 0 )
        importOoxFragment( new CommentsFragment( *this, aCommentsFragmentPath ) );
}

ContextHandlerRef WorksheetFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT: switch( getSheetType() )
        {
            case SHEETTYPE_WORKSHEET:   return (nElement == XLS_TOKEN( worksheet )) ? this : 0;
            case SHEETTYPE_CHARTSHEET:  return 0;
            case SHEETTYPE_MACROSHEET:  return (nElement == XM_TOKEN( macrosheet )) ? this : 0;
            case SHEETTYPE_DIALOGSHEET: return (nElement == XLS_TOKEN( dialogsheet )) ? this : 0;
            case SHEETTYPE_MODULESHEET: return 0;
            case SHEETTYPE_EMPTYSHEET:  return 0;
        }
        break;

        case XLS_TOKEN( worksheet ):
        case XM_TOKEN( macrosheet ):
        case XLS_TOKEN( dialogsheet ):
            switch( nElement )
            {
                case XLS_TOKEN( sheetData ):                return new SheetDataContext( *this );
                case XLS_TOKEN( conditionalFormatting ):    return new CondFormatContext( *this );
                case XLS_TOKEN( dataValidations ):          return new DataValidationsContext( *this );
                case XLS_TOKEN( autoFilter ):               return new AutoFilterContext( *this, getAutoFilters().createAutoFilter() );
                case XLS_TOKEN( scenarios ):                return new ScenariosContext( *this );

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
                case XLS_TOKEN( evenFooter ):       return this;    // collect h/f contents in onCharacters()
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

void WorksheetFragment::onCharacters( const OUString& rChars )
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

ContextHandlerRef WorksheetFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_WORKSHEET ) return this;
        break;

        case BIFF12_ID_WORKSHEET:
            switch( nRecId )
            {
                case BIFF12_ID_SHEETDATA:       return new SheetDataContext( *this );
                case BIFF12_ID_CONDFORMATTING:  return new CondFormatContext( *this );
                case BIFF12_ID_DATAVALIDATIONS: return new DataValidationsContext( *this );
                case BIFF12_ID_AUTOFILTER:      return new AutoFilterContext( *this, getAutoFilters().createAutoFilter() );
                case BIFF12_ID_SCENARIOS:       return new ScenariosContext( *this );

                case BIFF12_ID_SHEETVIEWS:
                case BIFF12_ID_COLS:
                case BIFF12_ID_MERGECELLS:
                case BIFF12_ID_ROWBREAKS:
                case BIFF12_ID_COLBREAKS:
                case BIFF12_ID_OLEOBJECTS:
                case BIFF12_ID_CONTROLS:        return this;

                case BIFF12_ID_SHEETPR:         getWorksheetSettings().importSheetPr( rStrm );              break;
                case BIFF12_ID_DIMENSION:       importDimension( rStrm );                                   break;
                case BIFF12_ID_SHEETFORMATPR:   importSheetFormatPr( rStrm );                               break;
                case BIFF12_ID_HYPERLINK:       importHyperlink( rStrm );                                   break;
                case BIFF12_ID_PAGEMARGINS:     getPageSettings().importPageMargins( rStrm );               break;
                case BIFF12_ID_PAGESETUP:       getPageSettings().importPageSetup( getRelations(), rStrm ); break;
                case BIFF12_ID_PRINTOPTIONS:    getPageSettings().importPrintOptions( rStrm );              break;
                case BIFF12_ID_HEADERFOOTER:    getPageSettings().importHeaderFooter( rStrm );              break;
                case BIFF12_ID_PICTURE:         getPageSettings().importPicture( getRelations(), rStrm );   break;
                case BIFF12_ID_SHEETPROTECTION: getWorksheetSettings().importSheetProtection( rStrm );      break;
                case BIFF12_ID_PHONETICPR:      getWorksheetSettings().importPhoneticPr( rStrm );           break;
                case BIFF12_ID_DRAWING:         importDrawing( rStrm );                                     break;
                case BIFF12_ID_LEGACYDRAWING:   importLegacyDrawing( rStrm );                               break;
            }
        break;

        case BIFF12_ID_SHEETVIEWS:
            switch( nRecId )
            {
                case BIFF12_ID_SHEETVIEW:       getSheetViewSettings().importSheetView( rStrm );            return this;
            }
        break;
        case BIFF12_ID_SHEETVIEW:
            switch( nRecId )
            {
                case BIFF12_ID_PANE:            getSheetViewSettings().importPane( rStrm );                 break;
                case BIFF12_ID_SELECTION:       getSheetViewSettings().importSelection( rStrm );            break;
            }
        break;

        case BIFF12_ID_COLS:
            if( nRecId == BIFF12_ID_COL ) importCol( rStrm );
        break;
        case BIFF12_ID_MERGECELLS:
            if( nRecId == BIFF12_ID_MERGECELL ) importMergeCell( rStrm );
        break;
        case BIFF12_ID_ROWBREAKS:
            if( nRecId == BIFF12_ID_BRK ) importBrk( rStrm, true );
        break;
        case BIFF12_ID_COLBREAKS:
            if( nRecId == BIFF12_ID_BRK ) importBrk( rStrm, false );
        break;
        case BIFF12_ID_OLEOBJECTS:
            if( nRecId == BIFF12_ID_OLEOBJECT ) importOleObject( rStrm );
        break;
        case BIFF12_ID_CONTROLS:
            if( nRecId == BIFF12_ID_CONTROL ) importControl( rStrm );
        break;
    }
    return 0;
}

const RecordInfo* WorksheetFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_AUTOFILTER,         BIFF12_ID_AUTOFILTER + 1        },
        { BIFF12_ID_CFRULE,             BIFF12_ID_CFRULE + 1            },
        { BIFF12_ID_COLBREAKS,          BIFF12_ID_COLBREAKS + 1         },
        { BIFF12_ID_COLORSCALE,         BIFF12_ID_COLORSCALE + 1        },
        { BIFF12_ID_COLS,               BIFF12_ID_COLS + 1              },
        { BIFF12_ID_CONDFORMATTING,     BIFF12_ID_CONDFORMATTING + 1    },
        { BIFF12_ID_CONTROLS,           BIFF12_ID_CONTROLS + 2          },
        { BIFF12_ID_CUSTOMFILTERS,      BIFF12_ID_CUSTOMFILTERS + 1     },
        { BIFF12_ID_CUSTOMSHEETVIEW,    BIFF12_ID_CUSTOMSHEETVIEW + 1   },
        { BIFF12_ID_CUSTOMSHEETVIEWS,   BIFF12_ID_CUSTOMSHEETVIEWS + 3  },
        { BIFF12_ID_DATABAR,            BIFF12_ID_DATABAR + 1           },
        { BIFF12_ID_DATAVALIDATIONS,    BIFF12_ID_DATAVALIDATIONS + 1   },
        { BIFF12_ID_DISCRETEFILTERS,    BIFF12_ID_DISCRETEFILTERS + 1   },
        { BIFF12_ID_FILTERCOLUMN,       BIFF12_ID_FILTERCOLUMN + 1      },
        { BIFF12_ID_HEADERFOOTER,       BIFF12_ID_HEADERFOOTER + 1      },
        { BIFF12_ID_ICONSET,            BIFF12_ID_ICONSET + 1           },
        { BIFF12_ID_MERGECELLS,         BIFF12_ID_MERGECELLS + 1        },
        { BIFF12_ID_OLEOBJECTS,         BIFF12_ID_OLEOBJECTS + 2        },
        { BIFF12_ID_ROW,                -1                              },
        { BIFF12_ID_ROWBREAKS,          BIFF12_ID_ROWBREAKS + 1         },
        { BIFF12_ID_SCENARIO,           BIFF12_ID_SCENARIO + 1          },
        { BIFF12_ID_SCENARIOS,          BIFF12_ID_SCENARIOS + 1         },
        { BIFF12_ID_SHEETDATA,          BIFF12_ID_SHEETDATA + 1         },
        { BIFF12_ID_SHEETVIEW,          BIFF12_ID_SHEETVIEW + 1         },
        { BIFF12_ID_SHEETVIEWS,         BIFF12_ID_SHEETVIEWS + 1        },
        { BIFF12_ID_TABLEPARTS,         BIFF12_ID_TABLEPARTS + 2        },
        { BIFF12_ID_WORKSHEET,          BIFF12_ID_WORKSHEET + 1         },
        { -1,                           -1                              }
    };
    return spRecInfos;
}

void WorksheetFragment::initializeImport()
{
    // initial processing in base class WorksheetHelper
    initializeWorksheetImport();

    // import query table fragments related to this worksheet
    RelationsRef xQueryRels = getRelations().getRelationsFromType( CREATE_OFFICEDOC_RELATION_TYPE( "queryTable" ) );
    for( Relations::const_iterator aIt = xQueryRels->begin(), aEnd = xQueryRels->end(); aIt != aEnd; ++aIt )
        importOoxFragment( new QueryTableFragment( *this, getFragmentPathFromRelation( aIt->second ) ) );

    // import pivot table fragments related to this worksheet
    RelationsRef xPivotRels = getRelations().getRelationsFromType( CREATE_OFFICEDOC_RELATION_TYPE( "pivotTable" ) );
    for( Relations::const_iterator aIt = xPivotRels->begin(), aEnd = xPivotRels->end(); aIt != aEnd; ++aIt )
        importOoxFragment( new PivotTableFragment( *this, getFragmentPathFromRelation( aIt->second ) ) );
}

void WorksheetFragment::finalizeImport()
{
    // final processing in base class WorksheetHelper
    finalizeWorksheetImport();
}

// private --------------------------------------------------------------------

void WorksheetFragment::importPageSetUpPr( const AttributeList& rAttribs )
{
    // for whatever reason, this flag is still stored separated from the page settings
    getPageSettings().setFitToPagesMode( rAttribs.getBool( XML_fitToPage, false ) );
}

void WorksheetFragment::importDimension( const AttributeList& rAttribs )
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

void WorksheetFragment::importSheetFormatPr( const AttributeList& rAttribs )
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

void WorksheetFragment::importCol( const AttributeList& rAttribs )
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

void WorksheetFragment::importMergeCell( const AttributeList& rAttribs )
{
    CellRangeAddress aRange;
    if( getAddressConverter().convertToCellRange( aRange, rAttribs.getString( XML_ref, OUString() ), getSheetIndex(), true, true ) )
        setMergedRange( aRange );
}

void WorksheetFragment::importHyperlink( const AttributeList& rAttribs )
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

void WorksheetFragment::importBrk( const AttributeList& rAttribs, bool bRowBreak )
{
    PageBreakModel aModel;
    aModel.mnColRow = rAttribs.getInteger( XML_id, 0 );
    aModel.mnMin    = rAttribs.getInteger( XML_min, aModel.mnColRow );
    aModel.mnMax    = rAttribs.getInteger( XML_max, aModel.mnColRow );
    aModel.mbManual = rAttribs.getBool( XML_man, false );
    setPageBreak( aModel, bRowBreak );
}

void WorksheetFragment::importDrawing( const AttributeList& rAttribs )
{
    setDrawingPath( getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) ) );
}

void WorksheetFragment::importLegacyDrawing( const AttributeList& rAttribs )
{
    setVmlDrawingPath( getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) ) );
}

void WorksheetFragment::importOleObject( const AttributeList& rAttribs )
{
    ::oox::vml::OleObjectInfo aInfo;
    aInfo.setShapeId( rAttribs.getInteger( XML_shapeId, 0 ) );
    OSL_ENSURE( rAttribs.hasAttribute( XML_link ) != rAttribs.hasAttribute( R_TOKEN( id ) ),
        "WorksheetFragment::importOleObject - OLE object must be either linked or embedded" );
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

void WorksheetFragment::importControl( const AttributeList& rAttribs )
{
    ::oox::vml::ControlInfo aInfo;
    aInfo.setShapeId( rAttribs.getInteger( XML_shapeId, 0 ) );
    aInfo.maFragmentPath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
    aInfo.maName = rAttribs.getString( XML_name, OUString() );
    getVmlDrawing().registerControl( aInfo );
}

void WorksheetFragment::importDimension( SequenceInputStream& rStrm )
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

void WorksheetFragment::importSheetFormatPr( SequenceInputStream& rStrm )
{
    sal_Int32 nDefaultWidth;
    sal_uInt16 nBaseWidth, nDefaultHeight, nFlags;
    rStrm >> nDefaultWidth >> nBaseWidth >> nDefaultHeight >> nFlags;

    // base column with
    setBaseColumnWidth( nBaseWidth );
    // default width is stored as 1/256th of a character in BIFF12, convert to entire character
    setDefaultColumnWidth( static_cast< double >( nDefaultWidth ) / 256.0 );
    // row height is in twips in BIFF12, convert to points; equal flags in all BIFFs
    setDefaultRowSettings(
        nDefaultHeight / 20.0,
        getFlag( nFlags, BIFF_DEFROW_CUSTOMHEIGHT ),
        getFlag( nFlags, BIFF_DEFROW_HIDDEN ),
        getFlag( nFlags, BIFF_DEFROW_THICKTOP ),
        getFlag( nFlags, BIFF_DEFROW_THICKBOTTOM ) );
}

void WorksheetFragment::importCol( SequenceInputStream& rStrm )
{
    ColumnModel aModel;

    sal_Int32 nWidth;
    sal_uInt16 nFlags;
    rStrm >> aModel.mnFirstCol >> aModel.mnLastCol >> nWidth >> aModel.mnXfId >> nFlags;

    // column indexes are 0-based in BIFF12, but ColumnModel expects 1-based
    ++aModel.mnFirstCol;
    ++aModel.mnLastCol;
    // width is stored as 1/256th of a character in BIFF12, convert to entire character
    aModel.mfWidth        = static_cast< double >( nWidth ) / 256.0;
    // equal flags in all BIFFs
    aModel.mnLevel        = extractValue< sal_Int32 >( nFlags, 8, 3 );
    aModel.mbShowPhonetic = getFlag( nFlags, BIFF_COLINFO_SHOWPHONETIC );
    aModel.mbHidden       = getFlag( nFlags, BIFF_COLINFO_HIDDEN );
    aModel.mbCollapsed    = getFlag( nFlags, BIFF_COLINFO_COLLAPSED );
    // set column properties in the current sheet
    setColumnModel( aModel );
}

void WorksheetFragment::importMergeCell( SequenceInputStream& rStrm )
{
    BinRange aBinRange;
    rStrm >> aBinRange;
    CellRangeAddress aRange;
    if( getAddressConverter().convertToCellRange( aRange, aBinRange, getSheetIndex(), true, true ) )
        setMergedRange( aRange );
}

void WorksheetFragment::importHyperlink( SequenceInputStream& rStrm )
{
    BinRange aBinRange;
    rStrm >> aBinRange;
    HyperlinkModel aModel;
    if( getAddressConverter().convertToCellRange( aModel.maRange, aBinRange, getSheetIndex(), true, true ) )
    {
        aModel.maTarget = getRelations().getExternalTargetFromRelId( BiffHelper::readString( rStrm ) );
        rStrm >> aModel.maLocation >> aModel.maTooltip >> aModel.maDisplay;
        setHyperlink( aModel );
    }
}

void WorksheetFragment::importBrk( SequenceInputStream& rStrm, bool bRowBreak )
{
    PageBreakModel aModel;
    sal_Int32 nManual;
    rStrm >> aModel.mnColRow >> aModel.mnMin >> aModel.mnMax >> nManual;
    aModel.mbManual = nManual != 0;
    setPageBreak( aModel, bRowBreak );
}

void WorksheetFragment::importDrawing( SequenceInputStream& rStrm )
{
    setDrawingPath( getFragmentPathFromRelId( BiffHelper::readString( rStrm ) ) );
}

void WorksheetFragment::importLegacyDrawing( SequenceInputStream& rStrm )
{
    setVmlDrawingPath( getFragmentPathFromRelId( BiffHelper::readString( rStrm ) ) );
}

void WorksheetFragment::importOleObject( SequenceInputStream& rStrm )
{
    ::oox::vml::OleObjectInfo aInfo;
    sal_Int32 nAspect, nUpdateMode, nShapeId;
    sal_uInt16 nFlags;
    rStrm >> nAspect >> nUpdateMode >> nShapeId >> nFlags >> aInfo.maProgId;
    aInfo.mbLinked = getFlag( nFlags, BIFF12_OLEOBJECT_LINKED );
    if( aInfo.mbLinked )
        aInfo.maTargetLink = getFormulaParser().importOleTargetLink( rStrm );
    else
        importEmbeddedOleData( aInfo.maEmbeddedData, BiffHelper::readString( rStrm ) );
    aInfo.setShapeId( nShapeId );
    aInfo.mbShowAsIcon = nAspect == BIFF12_OLEOBJECT_ICON;
    aInfo.mbAutoUpdate = nUpdateMode == BIFF12_OLEOBJECT_ALWAYS;
    aInfo.mbAutoLoad = getFlag( nFlags, BIFF12_OLEOBJECT_AUTOLOAD );
    getVmlDrawing().registerOleObject( aInfo );
}

void WorksheetFragment::importControl( SequenceInputStream& rStrm )
{
    ::oox::vml::ControlInfo aInfo;
    aInfo.setShapeId( rStrm.readInt32() );
    aInfo.maFragmentPath = getFragmentPathFromRelId( BiffHelper::readString( rStrm ) );
    rStrm >> aInfo.maName;
    getVmlDrawing().registerControl( aInfo );
}

void WorksheetFragment::importEmbeddedOleData( StreamDataSequence& orEmbeddedData, const OUString& rRelId )
{
    OUString aFragmentPath = getFragmentPathFromRelId( rRelId );
    if( aFragmentPath.getLength() > 0 )
        getBaseFilter().importBinaryData( orEmbeddedData, aFragmentPath );
}

// ============================================================================

BiffWorksheetFragment::BiffWorksheetFragment( const BiffWorkbookFragmentBase& rParent,
        const ISegmentProgressBarRef& rxProgressBar, WorksheetType eSheetType, sal_Int16 nSheet ) :
    BiffWorksheetFragmentBase( rParent, rxProgressBar, eSheetType, nSheet )
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
    BiffInputStream& rStrm = getInputStream();
    while( rStrm.startNextRecord() && (rStrm.getRecId() != BIFF_ID_EOF) )
    {
        if( BiffHelper::isBofRecord( rStrm ) )
        {
            // skip unknown embedded fragments (BOF/EOF blocks)
            skipFragment();
        }
        else
        {
            // cache base stream position to detect if record is already processed
            sal_Int64 nStrmPos = rStrm.tellBase();
            sal_uInt16 nRecId = rStrm.getRecId();

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
                        case BIFF_ID_AUTOFILTER:    importAutoFilter( rStrm );                      break;
                        case BIFF_ID_COLINFO:       importColInfo( rStrm );                         break;
                        case BIFF3_ID_DEFROWHEIGHT: importDefRowHeight( rStrm );                    break;
                        case BIFF_ID_HCENTER:       rPageSett.importHorCenter( rStrm );             break;
                        case BIFF_ID_MERGEDCELLS:   importMergedCells( rStrm );                     break;  // #i62300# also in BIFF5
                        case BIFF_ID_OBJECTPROTECT: rWorksheetSett.importObjectProtect( rStrm );    break;
                        case BIFF_ID_PAGESETUP:     rPageSett.importPageSetup( rStrm );             break;
                        case BIFF_ID_PTDEFINITION:  importPTDefinition( rStrm );                    break;
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
                        case BIFF_ID_AUTOFILTER:        importAutoFilter( rStrm );                      break;
                        case BIFF_ID_CFHEADER:          rCondFormats.importCfHeader( rStrm );           break;
                        case BIFF_ID_CODENAME:          rWorksheetSett.importCodeName( rStrm );         break;
                        case BIFF_ID_COLINFO:           importColInfo( rStrm );                         break;
                        case BIFF_ID_DATAVALIDATION:    importDataValidation( rStrm );                  break;
                        case BIFF_ID_DATAVALIDATIONS:   importDataValidations( rStrm );                 break;
                        case BIFF3_ID_DEFROWHEIGHT:     importDefRowHeight( rStrm );                    break;
                        case BIFF_ID_HCENTER:           rPageSett.importHorCenter( rStrm );             break;
                        case BIFF_ID_HYPERLINK:         importHyperlink( rStrm );                       break;
                        case BIFF_ID_LABELRANGES:       importLabelRanges( rStrm );                     break;
                        case BIFF_ID_MERGEDCELLS:       importMergedCells( rStrm );                     break;
                        case BIFF_ID_OBJECTPROTECT:     rWorksheetSett.importObjectProtect( rStrm );    break;
                        case BIFF_ID_PAGESETUP:         rPageSett.importPageSetup( rStrm );             break;
                        case BIFF_ID_PHONETICPR:        rWorksheetSett.importPhoneticPr( rStrm );       break;
                        case BIFF_ID_PICTURE:           rPageSett.importPicture( rStrm );               break;
                        case BIFF_ID_PTDEFINITION:      importPTDefinition( rStrm );                    break;
                        case BIFF_ID_QUERYTABLE:        importQueryTable( rStrm );                      break;
                        case BIFF_ID_SAVERECALC:        rWorkbookSett.importSaveRecalc( rStrm );        break;
                        case BIFF_ID_SCENARIOS:         importScenarios( rStrm );                       break;
                        case BIFF_ID_SCENPROTECT:       rWorksheetSett.importScenProtect( rStrm );      break;
                        case BIFF_ID_SCL:               rSheetViewSett.importScl( rStrm );              break;
                        case BIFF_ID_SHEETEXT:          rWorksheetSett.importSheetExt( rStrm );         break;
                        case BIFF_ID_SHEETPR:           rWorksheetSett.importSheetPr( rStrm );          break;
                        case BIFF_ID_SHAREDFEATHEAD:    importSharedFeatHead( rStrm );                  break;
                        case BIFF_ID_STANDARDWIDTH:     importStandardWidth( rStrm );                   break;
                        case BIFF_ID_UNCALCED:          rWorkbookSett.importUncalced( rStrm );          break;
                        case BIFF_ID_VCENTER:           rPageSett.importVerCenter( rStrm );             break;
                        case BIFF3_ID_WINDOW2:          rSheetViewSett.importWindow2( rStrm );          break;
                    }
                    break;

                    case BIFF_UNKNOWN: break;
                }
            }

            // record not processed, try record context objects
            if( rStrm.tellBase() == nStrmPos )
            {
                // first, try cell table records
                aSheetData.importRecord( rStrm );
                // handle another open record context
                if( mxContext.get() )
                {
                    // if it was a cell table record, forget the other record context
                    if( rStrm.tellBase() == nStrmPos )
                        mxContext->importRecord( rStrm );
                    else
                        mxContext.reset();
                }
            }
        }
    }

    // final processing in base class WorksheetHelper
    finalizeWorksheetImport();
    return rStrm.getRecId() == BIFF_ID_EOF;
}

// private --------------------------------------------------------------------

void BiffWorksheetFragment::importAutoFilter( BiffInputStream& rStrm )
{
    mxContext.reset( new BiffAutoFilterContext( *this, getAutoFilters().createAutoFilter() ) );
    mxContext->importRecord( rStrm );
}

void BiffWorksheetFragment::importColInfo( BiffInputStream& rStrm )
{
    sal_uInt16 nFirstCol, nLastCol, nWidth, nXfId, nFlags;
    rStrm >> nFirstCol >> nLastCol >> nWidth >> nXfId >> nFlags;

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

void BiffWorksheetFragment::importColumnDefault( BiffInputStream& rStrm )
{
    sal_uInt16 nFirstCol, nLastCol, nXfId;
    rStrm >> nFirstCol >> nLastCol >> nXfId;
    setDefaultColumnFormat( nFirstCol, nLastCol, nXfId );
}

void BiffWorksheetFragment::importColWidth( BiffInputStream& rStrm )
{
    sal_uInt8 nFirstCol, nLastCol;
    sal_uInt16 nWidth;
    rStrm >> nFirstCol >> nLastCol >> nWidth;

    ColumnModel aModel;
    // column indexes are 0-based in BIFF, but ColumnModel expects 1-based
    aModel.mnFirstCol = static_cast< sal_Int32 >( nFirstCol ) + 1;
    aModel.mnLastCol = static_cast< sal_Int32 >( nLastCol ) + 1;
    // width is stored as 1/256th of a character in BIFF, convert to entire character
    aModel.mfWidth = static_cast< double >( nWidth ) / 256.0;
    // set column properties in the current sheet
    setColumnModel( aModel );
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

void BiffWorksheetFragment::importDataValidation( BiffInputStream& rStrm )
{
    ValidationModel aModel;

    // flags
    sal_uInt32 nFlags;
    rStrm >> nFlags;
    aModel.setBiffType( extractValue< sal_uInt8 >( nFlags, 0, 4 ) );
    aModel.setBiffOperator( extractValue< sal_uInt8 >( nFlags, 20, 4 ) );
    aModel.setBiffErrorStyle( extractValue< sal_uInt8 >( nFlags, 4, 3 ) );
    aModel.mbAllowBlank   = getFlag( nFlags, BIFF_DATAVAL_ALLOWBLANK );
    aModel.mbNoDropDown   = getFlag( nFlags, BIFF_DATAVAL_NODROPDOWN );
    aModel.mbShowInputMsg = getFlag( nFlags, BIFF_DATAVAL_SHOWINPUT );
    aModel.mbShowErrorMsg = getFlag( nFlags, BIFF_DATAVAL_SHOWERROR );

    // message strings
    aModel.maInputTitle   = lclReadDataValMessage( rStrm );
    aModel.maErrorTitle   = lclReadDataValMessage( rStrm );
    aModel.maInputMessage = lclReadDataValMessage( rStrm );
    aModel.maErrorMessage = lclReadDataValMessage( rStrm );

    // condition formula(s)
    FormulaParser& rParser = getFormulaParser();
    aModel.maTokens1 = lclReadDataValFormula( rStrm, rParser );
    aModel.maTokens2 = lclReadDataValFormula( rStrm, rParser );
    // process string list of a list validation (convert to list of string tokens)
    if( (aModel.mnType == XML_list) && getFlag( nFlags, BIFF_DATAVAL_STRINGLIST ) )
        rParser.convertStringToStringList( aModel.maTokens1, '\0', true );

    // cell range list
    BinRangeList aRanges;
    rStrm >> aRanges;
    getAddressConverter().convertToCellRangeList( aModel.maRanges, aRanges, getSheetIndex(), true );

    // set validation data
    setValidation( aModel );
}

void BiffWorksheetFragment::importDimension( BiffInputStream& rStrm )
{
    // 32-bit row indexes in BIFF8
    bool bInt32Rows = (rStrm.getRecId() == BIFF3_ID_DIMENSION) && (getBiff() == BIFF8);
    BinRange aBinRange;
    aBinRange.read( rStrm, true, bInt32Rows );
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

void BiffWorksheetFragment::importHyperlink( BiffInputStream& rStrm )
{
    HyperlinkModel aModel;

    // read cell range for the hyperlink
    BinRange aBiffRange;
    rStrm >> aBiffRange;
    // #i80006# Excel silently ignores invalid hi-byte of column index (TODO: everywhere?)
    aBiffRange.maFirst.mnCol &= 0xFF;
    aBiffRange.maLast.mnCol &= 0xFF;
    if( !getAddressConverter().convertToCellRange( aModel.maRange, aBiffRange, getSheetIndex(), true, true ) )
        return;

    // try to read the StdHlink data
    if( !::oox::ole::OleHelper::importStdHlink( aModel, rStrm, true ) )
        return;

    // try to read the optional following SCREENTIP record
    if( (rStrm.getNextRecId() == BIFF_ID_SCREENTIP) && rStrm.startNextRecord() )
    {
        rStrm.skip( 2 );      // repeated record id
        // the cell range, again
        rStrm >> aBiffRange;
        CellRangeAddress aRange;
        if( getAddressConverter().convertToCellRange( aRange, aBiffRange, getSheetIndex(), true, true ) &&
            (aRange.StartColumn == aModel.maRange.StartColumn) &&
            (aRange.StartRow == aModel.maRange.StartRow) &&
            (aRange.EndColumn == aModel.maRange.EndColumn) &&
            (aRange.EndRow == aModel.maRange.EndRow) )
        {
            /*  This time, we have no string length, no flag field, and a
                null-terminated 16-bit character array. */
            aModel.maTooltip = rStrm.readNulUnicodeArray();
        }
    }

    // store the hyperlink settings
    setHyperlink( aModel );
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
    PageBreakModel aModel;
    aModel.mbManual = true;             // only manual breaks stored in BIFF
    bool bBiff8 = getBiff() == BIFF8;   // skip start/end columns or rows in BIFF8

    sal_uInt16 nCount;
    rStrm >> nCount;
    for( sal_uInt16 nIndex = 0; !rStrm.isEof() && (nIndex < nCount); ++nIndex )
    {
        aModel.mnColRow = rStrm.readuInt16();
        setPageBreak( aModel, bRowBreak );
        if( bBiff8 )
            rStrm.skip( 4 );
    }
}

void BiffWorksheetFragment::importPTDefinition( BiffInputStream& rStrm )
{
    mxContext.reset( new BiffPivotTableContext( *this ) );
    mxContext->importRecord( rStrm );
}

void BiffWorksheetFragment::importQueryTable( BiffInputStream& rStrm )
{
    mxContext.reset( new BiffQueryTableContext( *this ) );
    mxContext->importRecord( rStrm );
}

void BiffWorksheetFragment::importScenarios( BiffInputStream& rStrm )
{
    getScenarios().createSheetScenarios( getSheetIndex() ).importScenarios( rStrm );
}

void BiffWorksheetFragment::importSharedFeatHead( BiffInputStream& rStrm )
{
    rStrm.skip( 12 );
    sal_uInt16 nType = rStrm.readuInt16();
    rStrm.skip( 5 );
    switch( nType )
    {
        case BIFF_SHRFEATHEAD_SHEETPROT:
            if( rStrm.getRemaining() >= 4 )
                getWorksheetSettings().importSheetProtection( rStrm );
        break;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */