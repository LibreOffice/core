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

#include "worksheetfragment.hxx"

#include <oox/core/filterbase.hxx>
#include <oox/core/relations.hxx>
#include <oox/helper/attributelist.hxx>
#include "addressconverter.hxx"
#include "autofilterbuffer.hxx"
#include "autofiltercontext.hxx"
#include "biffinputstream.hxx"
#include "commentsfragment.hxx"
#include "condformatcontext.hxx"
#include "drawingfragment.hxx"
#include "drawingmanager.hxx"
#include "externallinkbuffer.hxx"
#include "pagesettings.hxx"
#include "pivottablefragment.hxx"
#include "querytablefragment.hxx"
#include "scenariobuffer.hxx"
#include "scenariocontext.hxx"
#include "sheetdatabuffer.hxx"
#include "sheetdatacontext.hxx"
#include "tablefragment.hxx"
#include "extlstcontext.hxx"
#include "viewsettings.hxx"
#include "workbooksettings.hxx"
#include "worksheetsettings.hxx"

namespace oox {
namespace xls {

using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;
using namespace ::oox::core;

namespace {

const sal_uInt16 BIFF_COLINFO_HIDDEN        = 0x0001;
const sal_uInt16 BIFF_COLINFO_SHOWPHONETIC  = 0x0008;
const sal_uInt16 BIFF_COLINFO_COLLAPSED     = 0x1000;

const sal_uInt16 BIFF_DEFROW_CUSTOMHEIGHT   = 0x0001;
const sal_uInt16 BIFF_DEFROW_HIDDEN         = 0x0002;
const sal_uInt16 BIFF_DEFROW_THICKTOP       = 0x0004;
const sal_uInt16 BIFF_DEFROW_THICKBOTTOM    = 0x0008;

const sal_uInt32 BIFF_DATAVAL_STRINGLIST    = 0x00000080;
const sal_uInt32 BIFF_DATAVAL_ALLOWBLANK    = 0x00000100;
const sal_uInt32 BIFF_DATAVAL_NODROPDOWN    = 0x00000200;
const sal_uInt32 BIFF_DATAVAL_SHOWINPUT     = 0x00040000;
const sal_uInt32 BIFF_DATAVAL_SHOWERROR     = 0x00080000;

const sal_Int32 BIFF12_OLEOBJECT_ICON       = 4;
const sal_Int32 BIFF12_OLEOBJECT_ALWAYS     = 1;
const sal_uInt16 BIFF12_OLEOBJECT_LINKED    = 0x0001;
const sal_uInt16 BIFF12_OLEOBJECT_AUTOLOAD  = 0x0002;

} // namespace

void DataValidationsContext_Base::SetValidation(::oox::xls::WorksheetHelper& rTarget)
{
    if (!mxValModel.get())
        return;

    rTarget.getAddressConverter().convertToCellRangeList(mxValModel->maRanges, mSqref, rTarget.getSheetIndex(), true);
    mxValModel->msRef = mSqref;

    mxValModel->maTokens1 = rTarget.getFormulaParser().importFormula(mxValModel->maRanges.getBaseAddress(), mFormula1);
    // process string list of a list validation (convert to list of string tokens)
    if (mxValModel->mnType == XML_list)
        rTarget.getFormulaParser().convertStringToStringList(mxValModel->maTokens1, ',', true);

    mxValModel->maTokens2 = rTarget.getFormulaParser().importFormula(mxValModel->maRanges.getBaseAddress(), mFormula2);

    rTarget.setValidation(*mxValModel);
    mxValModel.reset();
}

void DataValidationsContext_Base::importDataValidation(const AttributeList& rAttribs)
{
    mxValModel.reset(new ValidationModel);
    mFormula1.clear();
    mFormula2.clear();
    mSqref = rAttribs.getString(XML_sqref, OUString());
    mxValModel->maInputTitle = rAttribs.getXString(XML_promptTitle, OUString());
    mxValModel->maInputMessage = rAttribs.getXString(XML_prompt, OUString());
    mxValModel->maErrorTitle = rAttribs.getXString(XML_errorTitle, OUString());
    mxValModel->maErrorMessage = rAttribs.getXString(XML_error, OUString());
    mxValModel->mnType = rAttribs.getToken(XML_type, XML_none);
    mxValModel->mnOperator = rAttribs.getToken(XML_operator, XML_between);
    mxValModel->mnErrorStyle = rAttribs.getToken(XML_errorStyle, XML_stop);
    mxValModel->mbShowInputMsg = rAttribs.getBool(XML_showInputMessage, false);
    mxValModel->mbShowErrorMsg = rAttribs.getBool(XML_showErrorMessage, false);
    /*  The attribute showDropDown@dataValidation is in fact a "suppress
    dropdown" flag, as it was in the BIFF format! ECMA specification
    and attribute name are plain wrong! */
    mxValModel->mbNoDropDown = rAttribs.getBool(XML_showDropDown, false);
    mxValModel->mbAllowBlank = rAttribs.getBool(XML_allowBlank, false);
}

void DataValidationsContext_Base::importDataValidation(SequenceInputStream& rStrm, ::oox::xls::WorksheetHelper& rTarget)
{
    ValidationModel aModel;

    sal_uInt32 nFlags;
    BinRangeList aRanges;
    nFlags = rStrm.readuInt32();
    rStrm >> aRanges >> aModel.maErrorTitle >> aModel.maErrorMessage >> aModel.maInputTitle >> aModel.maInputMessage;

    // equal flags in all BIFFs
    aModel.setBiffType(extractValue< sal_uInt8 >(nFlags, 0, 4));
    aModel.setBiffOperator(extractValue< sal_uInt8 >(nFlags, 20, 4));
    aModel.setBiffErrorStyle(extractValue< sal_uInt8 >(nFlags, 4, 3));
    aModel.mbAllowBlank = getFlag(nFlags, BIFF_DATAVAL_ALLOWBLANK);
    aModel.mbNoDropDown = getFlag(nFlags, BIFF_DATAVAL_NODROPDOWN);
    aModel.mbShowInputMsg = getFlag(nFlags, BIFF_DATAVAL_SHOWINPUT);
    aModel.mbShowErrorMsg = getFlag(nFlags, BIFF_DATAVAL_SHOWERROR);

    // cell range list
    rTarget.getAddressConverter().convertToCellRangeList(aModel.maRanges, aRanges, rTarget.getSheetIndex(), true);

    // condition formula(s)
    FormulaParser& rParser = rTarget.getFormulaParser();
    CellAddress aBaseAddr = aModel.maRanges.getBaseAddress();
    aModel.maTokens1 = rParser.importFormula(aBaseAddr, FORMULATYPE_VALIDATION, rStrm);
    aModel.maTokens2 = rParser.importFormula(aBaseAddr, FORMULATYPE_VALIDATION, rStrm);
    // process string list of a list validation (convert to list of string tokens)
    if ((aModel.mnType == XML_list) && getFlag(nFlags, BIFF_DATAVAL_STRINGLIST))
        rParser.convertStringToStringList(aModel.maTokens1, ',', true);

    // set validation data
    rTarget.setValidation(aModel);
}

DataValidationsContext::DataValidationsContext( WorksheetFragmentBase& rFragment ) :
    WorksheetContextBase( rFragment )
{
}

ContextHandlerRef DataValidationsContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElementWithMce() )
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
                case MCE_TOKEN( AlternateContent ):
                case XLS_TOKEN( formula1 ):
                case XLS_TOKEN( formula2 ):
                    return this;    // collect formulas in onCharacters()
            }
        break;
        case MCE_TOKEN( AlternateContent ):
            switch( nElement )
            {
                case MCE_TOKEN( Choice ):
                case MCE_TOKEN( Fallback ):
                    return this;
            }
        break;
        case MCE_TOKEN( Choice ):
            switch( nElement )
            {
                case X12AC_TOKEN( list ):
                    return this;
            }
        break;
        case MCE_TOKEN( Fallback ):
            switch( nElement )
            {
                case XLS_TOKEN( formula1 ):
                    if (!isFormula1Set()) // only if more preferable choice was not used
                        return this;      // collect formulas in onCharacters()
                break;
                case XLS_TOKEN( formula2 ):
                    if (!isFormula2Set()) // only if more preferable choice was not used
                        return this;      // collect formulas in onCharacters()
                break;
            }
        break;
    }
    return nullptr;
}

namespace {
// Convert strings like 1,"2,3",4 to form "1","2,3","4"
OUString NormalizeOoxList(const OUString& aList)
{
    OUStringBuffer aResult("\"");
    bool bInsideQuotes = false;
    const sal_Int32 nLen = aList.getLength();
    for (sal_Int32 i = 0; i < nLen; ++i)
    {
        sal_Unicode ch = aList[i];

        switch (ch)
        {
            case L'"':
                bInsideQuotes = !bInsideQuotes;
                break;
            case L',':
                if (!bInsideQuotes)
                {
                    aResult.append("\",\"");
                    break;
                }
                SAL_FALLTHROUGH;
            default:
                aResult.append(ch);
                break;
        }
    }
    return aResult.append('"').makeStringAndClear();
}
}

void DataValidationsContext::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( formula1 ):
            SetFormula1( rChars );
        break;
        case XLS_TOKEN( formula2 ):
            SetFormula2( rChars );
        break;
        case X12AC_TOKEN( list ):
            SetFormula1( NormalizeOoxList( rChars ) );
        break;
    }
}

void DataValidationsContext::onEndElement()
{
    if( getCurrentElementWithMce() == XLS_TOKEN( dataValidation ) )
    {
        SetValidation( *this );
    }
}

ContextHandlerRef DataValidationsContext::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    if( nRecId == BIFF12_ID_DATAVALIDATION )
        importDataValidation( rStrm, *this );
    return nullptr;
}

ExtDataValidationsContext::ExtDataValidationsContext( WorksheetContextBase& rFragment ) :
    WorksheetContextBase( rFragment ), mCurrFormula( 0 )
{
}

ContextHandlerRef ExtDataValidationsContext::onCreateContext(sal_Int32 nElement, const AttributeList& rAttribs)
{
    switch( getCurrentElement() )
    {
    case XLS14_TOKEN( dataValidations ):
        if ( nElement == XLS14_TOKEN( dataValidation ) )
        {
            importDataValidation( rAttribs );
            return this;
        }
        break;
    case XLS14_TOKEN( dataValidation ):
        switch ( nElement )
        {
        case XLS14_TOKEN( formula1 ):
        case XLS14_TOKEN( formula2 ):
            mCurrFormula = nElement;
            return this;
        case XM_TOKEN( sqref ):
            return this;    // collect sqref in onCharacters()
        }
        break;
    case XLS14_TOKEN( formula1 ):
    case XLS14_TOKEN( formula2 ):
        switch( nElement )
        {
        case XM_TOKEN( f ):
            return this;    // collect formulas in onCharacters()
        }
        break;
    }
    return nullptr;
}

void ExtDataValidationsContext::onCharacters( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XM_TOKEN( f ):
            switch( mCurrFormula )
            {
                case XLS14_TOKEN( formula1 ):
                    SetFormula1( rChars );
                break;
                case XLS14_TOKEN( formula2 ):
                    SetFormula2( rChars );
                break;
            }
        break;
        case XM_TOKEN( sqref ):
            SetSqref( rChars );
        break;
    }
}

void ExtDataValidationsContext::onEndElement()
{
    if( isCurrentElement( XLS14_TOKEN( dataValidation ) ) )
    {
        SetValidation( *this );
    }
}

WorksheetFragment::WorksheetFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    WorksheetFragmentBase( rHelper, rFragmentPath )
{
    // import data tables related to this worksheet
    RelationsRef xTableRels = getRelations().getRelationsFromTypeFromOfficeDoc( "table" );
    for( ::std::map< OUString, Relation >::const_iterator aIt = xTableRels->begin(), aEnd = xTableRels->end(); aIt != aEnd; ++aIt )
        importOoxFragment( new TableFragment( *this, getFragmentPathFromRelation( aIt->second ) ) );

    // import comments related to this worksheet
    OUString aCommentsFragmentPath = getFragmentPathFromFirstTypeFromOfficeDoc( "comments" );
    if( !aCommentsFragmentPath.isEmpty() )
        importOoxFragment( new CommentsFragment( *this, aCommentsFragmentPath ) );
}

ContextHandlerRef WorksheetFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT: switch( getSheetType() )
        {
            case SHEETTYPE_WORKSHEET:   return (nElement == XLS_TOKEN( worksheet )) ? this : nullptr;
            case SHEETTYPE_CHARTSHEET:  return nullptr;
            case SHEETTYPE_MACROSHEET:  return (nElement == XM_TOKEN( macrosheet )) ? this : nullptr;
            case SHEETTYPE_DIALOGSHEET: return (nElement == XLS_TOKEN( dialogsheet )) ? this : nullptr;
            case SHEETTYPE_MODULESHEET: return nullptr;
            case SHEETTYPE_EMPTYSHEET:  return nullptr;
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
                case XLS_TOKEN( extLst ):                   return new ExtLstGlobalContext( *this );

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
                case XLS_TOKEN( protectedRanges ):  WorksheetSettings::importProtectedRanges( rAttribs );       return this;
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

        case XLS_TOKEN( protectedRanges ):
            switch( nElement )
            {
                case XLS_TOKEN( protectedRange ):   getWorksheetSettings().importProtectedRange( rAttribs );    return this;
            }
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
        // Only process an oleObject or control if outside a mc:AlternateContent
        // element OR if within a mc:Fallback. I suppose ideally we
        // should process the stuff within 'mc:Choice'
	case XLS_TOKEN( controls ):
        case XLS_TOKEN( oleObjects ):
            if ( getCurrentElement() == XLS_TOKEN( controls ) )
            {
                if( aMceState.empty() || aMceState.back() == MCE_STARTED )
                {
                    if ( getCurrentElement() == XLS_TOKEN( oleObjects ) ) importOleObject( rAttribs );
                    else
                        importControl( rAttribs );
                }
                else if ( !aMceState.empty() && aMceState.back() == MCE_FOUND_CHOICE )
                {
                    // reset the handling within 'Choice'
                    // this will force attempted handling in Fallback
                    aMceState.back() = MCE_STARTED;
                }
            }
        break;
    }
    return nullptr;
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
    return nullptr;
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
    RelationsRef xQueryRels = getRelations().getRelationsFromTypeFromOfficeDoc( "queryTable" );
    for( ::std::map< OUString, Relation >::const_iterator aIt = xQueryRels->begin(), aEnd = xQueryRels->end(); aIt != aEnd; ++aIt )
        importOoxFragment( new QueryTableFragment( *this, getFragmentPathFromRelation( aIt->second ) ) );

    // import pivot table fragments related to this worksheet
    RelationsRef xPivotRels = getRelations().getRelationsFromTypeFromOfficeDoc( "pivotTable" );
    for( ::std::map< OUString, Relation >::const_iterator aIt = xPivotRels->begin(), aEnd = xPivotRels->end(); aIt != aEnd; ++aIt )
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
    AddressConverter::convertToCellRangeUnchecked( aRange, rAttribs.getString( XML_ref, OUString() ), getSheetIndex() );
    /*  OOXML stores the used area, if existing, or "A1" if the sheet is empty.
        In case of "A1", the dimension at the WorksheetHelper object will not
        be set. If the cell A1 exists, the used area will be updated while
        importing the cell. */
    if( (aRange.EndColumn > 0) || (aRange.EndRow > 0) )
    {
        extendUsedArea( aRange );
    }
}

void WorksheetFragment::importSheetFormatPr( const AttributeList& rAttribs )
{
    // default column settings
    setBaseColumnWidth( rAttribs.getInteger( XML_baseColWidth, 8 ) );
    setDefaultColumnWidth( rAttribs.getDouble( XML_defaultColWidth, 0.0 ) );
    // default row settings

    // We don't need to import:
    // XML_outlineLevelRow
    // XML_outlineLevelCol
    // as it will be updated during export to OOXML
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
    aModel.maRange.mnFirst = rAttribs.getInteger( XML_min, -1 );
    aModel.maRange.mnLast  = rAttribs.getInteger( XML_max, -1 );
    aModel.mfWidth         = rAttribs.getDouble( XML_width, 0.0 );
    aModel.mnXfId          = rAttribs.getInteger( XML_style, -1 );
    aModel.mnLevel         = rAttribs.getInteger( XML_outlineLevel, 0 );
    aModel.mbShowPhonetic  = rAttribs.getBool( XML_phonetic, false );
    aModel.mbHidden        = rAttribs.getBool( XML_hidden, false );
    aModel.mbCollapsed     = rAttribs.getBool( XML_collapsed, false );
    // set column properties in the current sheet
    setColumnModel( aModel );
}

void WorksheetFragment::importMergeCell( const AttributeList& rAttribs )
{
    CellRangeAddress aRange;
    if( getAddressConverter().convertToCellRange( aRange, rAttribs.getString( XML_ref, OUString() ), getSheetIndex(), true, true ) )
        getSheetData().setMergedRange( aRange );
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
    AddressConverter::convertToCellRangeUnchecked( aRange, aBinRange, getSheetIndex() );
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
    nDefaultWidth = rStrm.readInt32();
    nBaseWidth = rStrm.readuInt16();
    nDefaultHeight = rStrm.readuInt16();
    nFlags = rStrm.readuInt16();

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
    aModel.maRange.mnFirst = rStrm.readInt32();
    aModel.maRange.mnLast = rStrm.readInt32();
    nWidth = rStrm.readInt32();
    aModel.mnXfId = rStrm.readInt32();
    nFlags = rStrm.readuInt16();

    // column indexes are 0-based in BIFF12, but ColumnModel expects 1-based
    ++aModel.maRange.mnFirst;
    ++aModel.maRange.mnLast;
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
        getSheetData().setMergedRange( aRange );
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
    aModel.mnColRow = rStrm.readInt32();
    aModel.mnMin = rStrm.readInt32();
    aModel.mnMax = rStrm.readInt32();
    nManual = rStrm.readInt32();
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
    nAspect = rStrm.readInt32();
    nUpdateMode = rStrm.readInt32();
    nShapeId = rStrm.readInt32();
    nFlags = rStrm.readuInt16();
    rStrm >> aInfo.maProgId;
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
    if( !aFragmentPath.isEmpty() )
        getBaseFilter().importBinaryData( orEmbeddedData, aFragmentPath );
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
