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

#include "xmlcelli.hxx"
#include "xmlimprt.hxx"
#include "xmltabi.hxx"
#include "xmlstyli.hxx"
#include "xmlannoi.hxx"
#include "global.hxx"
#include "document.hxx"
#include "cellsuno.hxx"
#include "docuno.hxx"
#include "unonames.hxx"
#include "postit.hxx"
#include "sheetdata.hxx"
#include "docsh.hxx"
#include "cellform.hxx"
#include "validat.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "docpool.hxx"

#include "XMLTableShapeImportHelper.hxx"
#include "XMLTextPContext.hxx"
#include "XMLStylesImportHelper.hxx"

#include "arealink.hxx"
#include <sfx2/linkmgr.hxx>
#include "convuno.hxx"
#include "XMLConverter.hxx"
#include "scerrors.hxx"
#include "editutil.hxx"
#include "cell.hxx"


#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/families.hxx>
#include <xmloff/numehelp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <svl/zforlist.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <svx/unoapi.hxx>
#include <svl/languageoptions.hxx>
#include <sax/tools/converter.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>

#include <com/sun/star/util/XMergeable.hpp>
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>

#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>

#include <rtl/ustrbuf.hxx>
#include <tools/date.hxx>
#include <i18npool/lang.h>
#include <comphelper/extract.hxx>
#include <tools/string.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

using rtl::OUString;

//------------------------------------------------------------------

ScXMLTableRowCellContext::ScXMLTableRowCellContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      const bool bTempIsCovered,
                                      const sal_Int32 nTempRepeatedRows ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pContentValidationName(NULL),
    pDetectiveObjVec(NULL),
    pCellRangeSource(NULL),
    fValue(0.0),
    nMergedRows(1),
    nRepeatedRows(nTempRepeatedRows),
    nMergedCols(1),
    nColsRepeated(1),
    rXMLImport((ScXMLImport&)rImport),
    eGrammar( formula::FormulaGrammar::GRAM_STORAGE_DEFAULT),
    nCellType(util::NumberFormat::TEXT),
    bIsMerged(false),
    bIsMatrix(false),
    bIsCovered(bTempIsCovered),
    bIsEmpty(true),
    bHasTextImport(false),
    bIsFirstTextImport(false),
    bSolarMutexLocked(false),
    bFormulaTextResult(false)
{
    rXMLImport.SetRemoveLastChar(false);
    rXMLImport.GetTables().AddColumn(bTempIsCovered);
    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    rtl::OUString aLocalName;
    rtl::OUString* pStyleName = NULL;
    rtl::OUString* pCurrencySymbol = NULL;
    const SvXMLTokenMap& rTokenMap = rImport.GetTableRowCellAttrTokenMap();
    for (sal_Int16 i = 0; i < nAttrCount; ++i)
    {
        sal_uInt16 nAttrPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(
            xAttrList->getNameByIndex(i), &aLocalName);

        const rtl::OUString& sValue = xAttrList->getValueByIndex(i);
        sal_uInt16 nToken = rTokenMap.Get(nAttrPrefix, aLocalName);
        switch (nToken)
        {
            case XML_TOK_TABLE_ROW_CELL_ATTR_STYLE_NAME:
                pStyleName = new rtl::OUString(sValue);
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_CONTENT_VALIDATION_NAME:
                OSL_ENSURE(!pContentValidationName, "here should be only one Validation Name");
                pContentValidationName = new rtl::OUString(sValue);
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_ROWS:
                bIsMerged = true;
                nMergedRows = static_cast<SCROW>(sValue.toInt32());
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_COLS:
                bIsMerged = true;
                nMergedCols = static_cast<SCCOL>(sValue.toInt32());
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_COLS:
                bIsMatrix = true;
                nMatrixCols = static_cast<SCCOL>(sValue.toInt32());
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_SPANNED_MATRIX_ROWS:
                bIsMatrix = true;
                nMatrixRows = static_cast<SCROW>(sValue.toInt32());
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_REPEATED:
                nColsRepeated = static_cast<SCCOL>(std::max( sValue.toInt32(), static_cast<sal_Int32>(1) ));
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_VALUE_TYPE:
                nCellType = GetScImport().GetCellType(sValue);
                bIsEmpty = false;
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_VALUE:
            {
                if (!sValue.isEmpty())
                {
                    ::sax::Converter::convertDouble(fValue, sValue);
                    bIsEmpty = false;

                    //if office:value="0", let's get the text:p in case this is
                    //a special case in HasSpecialCaseFormulaText(). If it
                    //turns out not to be a special case, we'll use the 0 value.
                    if(fValue == 0.0)
                        bFormulaTextResult = true;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_DATE_VALUE:
            {
                if (!sValue.isEmpty() && rXMLImport.SetNullDateOnUnitConverter())
                {
                    rXMLImport.GetMM100UnitConverter().convertDateTime(fValue, sValue);
                    bIsEmpty = false;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_TIME_VALUE:
            {
                if (!sValue.isEmpty())
                {
                    ::sax::Converter::convertDuration(fValue, sValue);
                    bIsEmpty = false;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_STRING_VALUE:
            {
                if (!sValue.isEmpty())
                {
                    OSL_ENSURE(!pOUTextValue, "here should be only one string value");
                    pOUTextValue.reset(sValue);
                    bIsEmpty = false;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_BOOLEAN_VALUE:
            {
                if (!sValue.isEmpty())
                {
                    if ( IsXMLToken(sValue, XML_TRUE) )
                        fValue = 1.0;
                    else if ( IsXMLToken(sValue, XML_FALSE) )
                        fValue = 0.0;
                    else
                        ::sax::Converter::convertDouble(fValue, sValue);
                    bIsEmpty = false;
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_FORMULA:
            {
                if (!sValue.isEmpty())
                {
                    OSL_ENSURE(!pOUFormula, "here should be only one formula");
                    rtl::OUString aFormula, aFormulaNmsp;
                    rXMLImport.ExtractFormulaNamespaceGrammar( aFormula, aFormulaNmsp, eGrammar, sValue );
                    pOUFormula.reset( FormulaWithNamespace( aFormula, aFormulaNmsp ) );
                }
            }
            break;
            case XML_TOK_TABLE_ROW_CELL_ATTR_CURRENCY:
                pCurrencySymbol = new rtl::OUString(sValue);
            break;
            default:
                ;
        }
    }
    if (pOUFormula)
    {
        if (nCellType == util::NumberFormat::TEXT)
            bFormulaTextResult = true;
        if(nCellType == util::NumberFormat::DATETIME)
            nCellType = util::NumberFormat::UNDEFINED;
        //if bIsEmpty is true at this point, then there is no office value.
        //we must get the text:p (even if it is empty) in case this a special
        //case in HasSpecialCaseFormulaText().
        if(bIsEmpty)
            bFormulaTextResult = true;
    }
    rXMLImport.GetStylesImportHelper()->SetAttributes(pStyleName, pCurrencySymbol, nCellType);
}

ScXMLTableRowCellContext::~ScXMLTableRowCellContext()
{
    delete pContentValidationName;
    delete pDetectiveObjVec;
    delete pCellRangeSource;
}

void ScXMLTableRowCellContext::LockSolarMutex()
{
    if (!bSolarMutexLocked)
    {
        GetScImport().LockSolarMutex();
        bSolarMutexLocked = true;
    }
}

void ScXMLTableRowCellContext::UnlockSolarMutex()
{
    if (bSolarMutexLocked)
    {
        GetScImport().UnlockSolarMutex();
        bSolarMutexLocked = false;
    }
}

namespace {

bool cellExists( const ScAddress& rCellPos )
{
    return( rCellPos.Col() >= 0 && rCellPos.Row() >= 0 &&
            rCellPos.Col() <= MAXCOL && rCellPos.Row() <= MAXROW );
}

}

void ScXMLTableRowCellContext::SetCursorOnTextImport(const rtl::OUString& rOUTempText)
{
    ScAddress aCellPos = rXMLImport.GetTables().GetCurrentCellPos();
    if (cellExists(aCellPos))
    {
        sal_Int32 nCol = static_cast<sal_Int32>( aCellPos.Col() );
        sal_Int32 nRow = static_cast<sal_Int32>( aCellPos.Row() );
        uno::Reference<table::XCellRange> xCellRange(rXMLImport.GetTables().GetCurrentXCellRange());
        if (xCellRange.is())
        {
            xBaseCell.set( xCellRange->getCellByPosition(nCol, nRow) );
            if (xBaseCell.is())
            {
                xLockable.set(xBaseCell, uno::UNO_QUERY);
                if (xLockable.is())
                    xLockable->addActionLock();
                uno::Reference<text::XText> xText(xBaseCell, uno::UNO_QUERY);
                if (xText.is())
                {
                    uno::Reference<text::XTextCursor> xTextCursor(xText->createTextCursor());
                    if (xTextCursor.is())
                    {
                        xTextCursor->setString(rOUTempText);
                        xTextCursor->gotoEnd(false);
                        rXMLImport.GetTextImport()->SetCursor(xTextCursor);
                    }
                }
            }
        }
    }
    else
    {
        OSL_FAIL("this method should only be called for a existing cell");
    }
}

SvXMLImportContext *ScXMLTableRowCellContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = rXMLImport.GetTableRowCellElemTokenMap();
    bool bTextP(false);
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_TABLE_ROW_CELL_P:
        {
            bIsEmpty = false;
            bTextP = true;

            ScAddress aCellPos = rXMLImport.GetTables().GetCurrentCellPos();

            if( ((nCellType == util::NumberFormat::TEXT) || bFormulaTextResult) )
            {
                if (!bHasTextImport)
                {
                    bIsFirstTextImport = true;
                    bHasTextImport = true;
                    pContext = new ScXMLTextPContext(rXMLImport, nPrefix, rLName, xAttrList, this);
                }
                else
                {
                    if (cellExists(aCellPos))
                    {
                        if (bIsFirstTextImport && !rXMLImport.GetRemoveLastChar())
                        {
                            if (pOUTextContent)
                            {
                                SetCursorOnTextImport(*pOUTextContent);
                                pOUTextContent.reset();
                            }
                            else
                                SetCursorOnTextImport(rtl::OUString());
                            rXMLImport.SetRemoveLastChar(true);
                            uno::Reference < text::XTextCursor > xTextCursor(rXMLImport.GetTextImport()->GetCursor());
                            if (xTextCursor.is())
                            {
                                uno::Reference < text::XText > xText (xTextCursor->getText());
                                uno::Reference < text::XTextRange > xTextRange (xTextCursor, uno::UNO_QUERY);
                                if (xText.is() && xTextRange.is())
                                    xText->insertControlCharacter(xTextRange, text::ControlCharacter::PARAGRAPH_BREAK, false);
                            }
                        }
                        pContext = rXMLImport.GetTextImport()->CreateTextChildContext(
                            rXMLImport, nPrefix, rLName, xAttrList);
                        bIsFirstTextImport = false;
                    }
                }
            }
        }
        break;
        case XML_TOK_TABLE_ROW_CELL_TABLE:
        {
            SAL_WARN("sc", "ScXMLTableRowCellContext::CreateChildContext: subtables are not supported");
        }
        break;
        case XML_TOK_TABLE_ROW_CELL_ANNOTATION:
        {
            bIsEmpty = false;
            OSL_ENSURE( !mxAnnotationData.get(), "ScXMLTableRowCellContext::CreateChildContext - multiple annotations in one cell" );
            mxAnnotationData.reset( new ScXMLAnnotationData );
            pContext = new ScXMLAnnotationContext( rXMLImport, nPrefix, rLName,
                                                    xAttrList, *mxAnnotationData, this);
        }
        break;
        case XML_TOK_TABLE_ROW_CELL_DETECTIVE:
        {
            bIsEmpty = false;
            if (!pDetectiveObjVec)
                pDetectiveObjVec = new ScMyImpDetectiveObjVec();
            pContext = new ScXMLDetectiveContext(
                rXMLImport, nPrefix, rLName, pDetectiveObjVec );
        }
        break;
        case XML_TOK_TABLE_ROW_CELL_CELL_RANGE_SOURCE:
        {
            bIsEmpty = false;
            if (!pCellRangeSource)
                pCellRangeSource = new ScMyImpCellRangeSource();
            pContext = new ScXMLCellRangeSourceContext(
                rXMLImport, nPrefix, rLName, xAttrList, pCellRangeSource );
        }
        break;
    }

    if (!pContext && !bTextP)
    {
        ScAddress aCellPos = rXMLImport.GetTables().GetCurrentCellPos();
        uno::Reference<drawing::XShapes> xShapes (rXMLImport.GetTables().GetCurrentXShapes());
        if (xShapes.is())
        {
            if (aCellPos.Col() > MAXCOL)
                aCellPos.SetCol(MAXCOL);
            if (aCellPos.Row() > MAXROW)
                aCellPos.SetRow(MAXROW);
            XMLTableShapeImportHelper* pTableShapeImport =
                    static_cast< XMLTableShapeImportHelper* >( rXMLImport.GetShapeImport().get() );
            pTableShapeImport->SetOnTable(false);
            com::sun::star::table::CellAddress aCellAddress;
            ScUnoConversion::FillApiAddress( aCellAddress, aCellPos );
            pTableShapeImport->SetCell(aCellAddress);
            pContext = rXMLImport.GetShapeImport()->CreateGroupChildContext(
                rXMLImport, nPrefix, rLName, xAttrList, xShapes);
            if (pContext)
            {
                bIsEmpty = false;
                rXMLImport.ProgressBarIncrement(false);
            }
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLTableRowCellContext::DoMerge( const ScAddress& rScAddress, const SCCOL nCols, const SCROW nRows )
{
    SCCOL mergeToCol = rScAddress.Col() + nCols;
    SCROW mergeToRow = rScAddress.Row() + nRows;
    bool bInBounds = rScAddress.Col() <= MAXCOL && rScAddress.Row() <= MAXROW &&
                       mergeToCol <= MAXCOL && mergeToRow <= MAXROW;
    if( bInBounds )
    {
        rXMLImport.GetDocument()->DoMerge( rScAddress.Tab(),
            rScAddress.Col(), rScAddress.Row(), mergeToCol, mergeToRow );
    }
}

namespace {

ScValidationMode validationTypeToMode( const sheet::ValidationType eVType )
{
    ScValidationMode eMode;
    switch( eVType )
    {
        case sheet::ValidationType_WHOLE:               eMode = SC_VALID_WHOLE;     break;
        case sheet::ValidationType_DECIMAL:             eMode = SC_VALID_DECIMAL;   break;
        case sheet::ValidationType_DATE:                eMode = SC_VALID_DATE;      break;
        case sheet::ValidationType_TIME:                eMode = SC_VALID_TIME;      break;
        case sheet::ValidationType_TEXT_LEN:            eMode = SC_VALID_TEXTLEN;   break;
        case sheet::ValidationType_LIST:                eMode = SC_VALID_LIST;      break;
        case sheet::ValidationType_CUSTOM:              eMode = SC_VALID_CUSTOM;    break;
        default:                                        eMode = SC_VALID_ANY;       break;
    }
    return eMode;
}

ScValidErrorStyle validAlertToValidError( const sheet::ValidationAlertStyle eVAlertStyle )
{
    ScValidErrorStyle eVErrStyle;
    switch( eVAlertStyle )
    {
        case sheet::ValidationAlertStyle_STOP:          eVErrStyle = SC_VALERR_STOP;      break;
        case sheet::ValidationAlertStyle_WARNING:       eVErrStyle = SC_VALERR_WARNING;   break;
        case sheet::ValidationAlertStyle_MACRO:         eVErrStyle = SC_VALERR_MACRO;     break;
        default:                                        eVErrStyle = SC_VALERR_INFO;      break;
        //should INFO be the default?  seems to be the most unobtrusive choice.
    }
    return eVErrStyle;
}

ScConditionMode conditionOpToMode( const sheet::ConditionOperator eOp )
{
    ScConditionMode eMode;
    switch( eOp )
    {
        case sheet::ConditionOperator_EQUAL:            eMode = SC_COND_EQUAL;      break;
        case sheet::ConditionOperator_NOT_EQUAL:        eMode = SC_COND_NOTEQUAL;   break;
        case sheet::ConditionOperator_GREATER:          eMode = SC_COND_GREATER;    break;
        case sheet::ConditionOperator_GREATER_EQUAL:    eMode = SC_COND_EQGREATER;  break;
        case sheet::ConditionOperator_LESS:             eMode = SC_COND_LESS;       break;
        case sheet::ConditionOperator_LESS_EQUAL:       eMode = SC_COND_EQLESS;     break;
        case sheet::ConditionOperator_BETWEEN:          eMode = SC_COND_BETWEEN;    break;
        case sheet::ConditionOperator_NOT_BETWEEN:      eMode = SC_COND_NOTBETWEEN; break;
        case sheet::ConditionOperator_FORMULA:          eMode = SC_COND_DIRECT;     break;
        default:                                        eMode = SC_COND_NONE;       break;
    }
    return eMode;
}

}

void ScXMLTableRowCellContext::SetContentValidation( const ScRange& rScRange )
{
    if( pContentValidationName && !pContentValidationName->isEmpty() )
    {
        ScDocument* pDoc = rXMLImport.GetDocument();
        ScMyImportValidation aValidation;
        aValidation.eGrammar1 = aValidation.eGrammar2 = pDoc->GetStorageGrammar();
        if( rXMLImport.GetValidation(*pContentValidationName, aValidation) )
        {
            ScValidationData aScValidationData(
                validationTypeToMode(aValidation.aValidationType),
                conditionOpToMode(aValidation.aOperator),
                aValidation.sFormula1, aValidation.sFormula2, pDoc, ScAddress(),
                aValidation.sFormulaNmsp1, aValidation.sFormulaNmsp2,
                aValidation.eGrammar1, aValidation.eGrammar2
            );

            aScValidationData.SetIgnoreBlank( aValidation.bIgnoreBlanks );
            aScValidationData.SetListType( aValidation.nShowList );

            // set strings for error / input even if disabled (and disable afterwards)
            aScValidationData.SetInput( aValidation.sImputTitle, aValidation.sImputMessage );
            if( !aValidation.bShowImputMessage )
                aScValidationData.ResetInput();
            aScValidationData.SetError( aValidation.sErrorTitle, aValidation.sErrorMessage, validAlertToValidError(aValidation.aAlertStyle) );
            if( !aValidation.bShowErrorMessage )
                aScValidationData.ResetError();

            if( !aValidation.sBaseCellAddress.isEmpty() )
                aScValidationData.SetSrcString( aValidation.sBaseCellAddress );

            sal_uLong nIndex = pDoc->AddValidationEntry( aScValidationData );

            ScPatternAttr aPattern( pDoc->GetPool() );
            aPattern.GetItemSet().Put( SfxUInt32Item( ATTR_VALIDDATA, nIndex ) );
            if( rScRange.aStart == rScRange.aEnd )  //for a single cell
            {
                pDoc->ApplyPattern( rScRange.aStart.Col(), rScRange.aStart.Row(),
                                    rScRange.aStart.Tab(), aPattern );
            }
            else  //for repeating cells
            {
                pDoc->ApplyPatternAreaTab( rScRange.aStart.Col(), rScRange.aStart.Row(),
                                       rScRange.aEnd.Col(), rScRange.aEnd.Row(),
                                       rScRange.aStart.Tab(), aPattern );
            }

            // is the below still needed?
            // For now, any sheet with validity is blocked from stream-copying.
            // Later, the validation names could be stored along with the style names.
            ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetImport().GetModel())->GetSheetSaveData();
            pSheetData->BlockSheet( GetScImport().GetTables().GetCurrentSheet() );
        }
    }
}

void ScXMLTableRowCellContext::SetContentValidation( const ScAddress& rCellPos )
{
    SetContentValidation( ScRange(rCellPos, rCellPos) );
}

void ScXMLTableRowCellContext::SetAnnotation(const ScAddress& rPos)
{
    ScDocument* pDoc = rXMLImport.GetDocument();
    if( !pDoc || !mxAnnotationData.get() )
        return;

    LockSolarMutex();

    ScPostIt* pNote = 0;

    uno::Reference< drawing::XShapes > xShapes = rXMLImport.GetTables().GetCurrentXShapes();
    uno::Reference< container::XIndexAccess > xShapesIA( xShapes, uno::UNO_QUERY );
    sal_Int32 nOldShapeCount = xShapesIA.is() ? xShapesIA->getCount() : 0;

    OSL_ENSURE( !mxAnnotationData->mxShape.is() || mxAnnotationData->mxShapes.is(),
        "ScXMLTableRowCellContext::SetAnnotation - shape without drawing page" );
    if( mxAnnotationData->mxShape.is() && mxAnnotationData->mxShapes.is() )
    {
        OSL_ENSURE( mxAnnotationData->mxShapes.get() == xShapes.get(), "ScXMLTableRowCellContext::SetAnnotation - diffenet drawing pages" );
        SdrObject* pObject = ::GetSdrObjectFromXShape( mxAnnotationData->mxShape );
        OSL_ENSURE( pObject, "ScXMLTableRowCellContext::SetAnnotation - cannot get SdrObject from shape" );

        /*  Try to reuse the drawing object already created (but only if the
            note is visible, and the object is a caption object). */
        if( mxAnnotationData->mbShown && mxAnnotationData->mbUseShapePos )
        {
            if( SdrCaptionObj* pCaption = dynamic_cast< SdrCaptionObj* >( pObject ) )
            {
                OSL_ENSURE( !pCaption->GetLogicRect().IsEmpty(), "ScXMLTableRowCellContext::SetAnnotation - invalid caption rectangle" );
                // create the cell note with the caption object
                pNote = ScNoteUtil::CreateNoteFromCaption( *pDoc, rPos, *pCaption, true );
                // forget pointer to object (do not create note again below)
                pObject = 0;
            }
        }

        // drawing object has not been used to create a note -> use shape data
        if( pObject )
        {
            // rescue settings from drawing object before the shape is removed
            ::std::auto_ptr< SfxItemSet > xItemSet( new SfxItemSet( pObject->GetMergedItemSet() ) );
            ::std::auto_ptr< OutlinerParaObject > xOutlinerObj;
            if( OutlinerParaObject* pOutlinerObj = pObject->GetOutlinerParaObject() )
                xOutlinerObj.reset( new OutlinerParaObject( *pOutlinerObj ) );
            Rectangle aCaptionRect;
            if( mxAnnotationData->mbUseShapePos )
                aCaptionRect = pObject->GetLogicRect();
            // remove the shape from the drawing page, this invalidates pObject
            mxAnnotationData->mxShapes->remove( mxAnnotationData->mxShape );
            pObject = 0;
            // update current number of existing objects
            if( xShapesIA.is() )
                nOldShapeCount = xShapesIA->getCount();

            // an outliner object is required (empty note captions not allowed)
            if( xOutlinerObj.get() )
            {
                // create cell note with all data from drawing object
                pNote = ScNoteUtil::CreateNoteFromObjectData( *pDoc, rPos,
                    xItemSet.release(), xOutlinerObj.release(),
                    aCaptionRect, mxAnnotationData->mbShown, false );
            }
        }
    }
    else if( !mxAnnotationData->maSimpleText.isEmpty() )
    {
        // create note from simple text
        pNote = ScNoteUtil::CreateNoteFromString( *pDoc, rPos,
            mxAnnotationData->maSimpleText, mxAnnotationData->mbShown, false );
    }

    // set author and date
    if( pNote )
    {
        double fDate;
        rXMLImport.GetMM100UnitConverter().convertDateTime( fDate, mxAnnotationData->maCreateDate );
        SvNumberFormatter* pNumForm = pDoc->GetFormatTable();
        sal_uInt32 nfIndex = pNumForm->GetFormatIndex( NF_DATE_SYS_DDMMYYYY, LANGUAGE_SYSTEM );
        String aDate;
        Color* pColor = 0;
        Color** ppColor = &pColor;
        pNumForm->GetOutputString( fDate, nfIndex, aDate, ppColor );
        pNote->SetDate( aDate );
        pNote->SetAuthor( mxAnnotationData->maAuthor );
    }

    // register a shape that has been newly created in the ScNoteUtil functions
    if( xShapesIA.is() && (nOldShapeCount < xShapesIA->getCount()) )
    {
        uno::Reference< drawing::XShape > xShape;
        rXMLImport.GetShapeImport()->shapeWithZIndexAdded( xShape, xShapesIA->getCount() );
    }

    // store the style names for stream copying
    ScSheetSaveData* pSheetData = ScModelObj::getImplementation(rXMLImport.GetModel())->GetSheetSaveData();
    pSheetData->HandleNoteStyles( mxAnnotationData->maStyleName, mxAnnotationData->maTextStyle, rPos );

    std::vector<ScXMLAnnotationStyleEntry>::const_iterator aIter = mxAnnotationData->maContentStyles.begin();
    std::vector<ScXMLAnnotationStyleEntry>::const_iterator aEnd = mxAnnotationData->maContentStyles.end();
    while (aIter != aEnd)
    {
        pSheetData->AddNoteContentStyle( aIter->mnFamily, aIter->maName, rPos, aIter->maSelection );
        ++aIter;
    }
}

// core implementation
void ScXMLTableRowCellContext::SetDetectiveObj( const ScAddress& rPosition )
{
    if( cellExists(rPosition) && pDetectiveObjVec && pDetectiveObjVec->size() )
    {
        LockSolarMutex();
        ScDetectiveFunc aDetFunc( rXMLImport.GetDocument(), rPosition.Tab() );
        uno::Reference<container::XIndexAccess> xShapesIndex (rXMLImport.GetTables().GetCurrentXShapes(), uno::UNO_QUERY); // make draw page
        ScMyImpDetectiveObjVec::iterator aItr(pDetectiveObjVec->begin());
        ScMyImpDetectiveObjVec::iterator aEndItr(pDetectiveObjVec->end());
        while(aItr != aEndItr)
        {
            aDetFunc.InsertObject( aItr->eObjType, rPosition, aItr->aSourceRange, aItr->bHasError );
            if (xShapesIndex.is())
            {
                sal_Int32 nShapes = xShapesIndex->getCount();
                uno::Reference < drawing::XShape > xShape;
                rXMLImport.GetShapeImport()->shapeWithZIndexAdded(xShape, nShapes);
            }
            ++aItr;
        }
    }
}

// core implementation
void ScXMLTableRowCellContext::SetCellRangeSource( const ScAddress& rPosition )
{
    if( cellExists(rPosition) && pCellRangeSource  && !pCellRangeSource->sSourceStr.isEmpty() &&
        !pCellRangeSource->sFilterName.isEmpty() && !pCellRangeSource->sURL.isEmpty() )
    {
        ScDocument* pDoc = rXMLImport.GetDocument();
        if (pDoc)
        {
            LockSolarMutex();
            ScRange aDestRange( rPosition.Col(), rPosition.Row(), rPosition.Tab(),
                rPosition.Col() + static_cast<SCCOL>(pCellRangeSource->nColumns - 1),
                rPosition.Row() + static_cast<SCROW>(pCellRangeSource->nRows - 1), rPosition.Tab() );
            String sFilterName( pCellRangeSource->sFilterName );
            String sSourceStr( pCellRangeSource->sSourceStr );
            ScAreaLink* pLink = new ScAreaLink( pDoc->GetDocumentShell(), pCellRangeSource->sURL,
                sFilterName, pCellRangeSource->sFilterOptions, sSourceStr, aDestRange, pCellRangeSource->nRefresh );
            sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();
            pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, pCellRangeSource->sURL, &sFilterName, &sSourceStr );
        }
    }
}

void ScXMLTableRowCellContext::SetFormulaCell(ScFormulaCell* pFCell) const
{
    if(pFCell)
    {
        if( bFormulaTextResult && pOUTextValue )
            pFCell->SetHybridString( *pOUTextValue );
        else
            pFCell->SetHybridDouble( fValue );
        pFCell->ResetDirty();
    }
}

void ScXMLTableRowCellContext::PutTextCell( const ScAddress& rCurrentPos,
        const SCCOL nCurrentCol, const ::boost::optional< rtl::OUString >& pOUText )
{
    bool bDoIncrement = true;
    //matrix reference cells that contain text formula results;
    //cell was already put in document, just need to set text here.
    if( rXMLImport.GetTables().IsPartOfMatrix(rCurrentPos) )
    {
        ScBaseCell* pCell = rXMLImport.GetDocument()->GetCell( rCurrentPos );
        bDoIncrement = ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA );
        if ( bDoIncrement )
        {
            ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pCell);
            if (pOUTextValue && !pOUTextValue->isEmpty())
                pFCell->SetHybridString( *pOUTextValue );
            else if (pOUTextContent && !pOUTextContent->isEmpty())
                pFCell->SetHybridString( *pOUTextContent );
            else if ( nCurrentCol > 0 && pOUText && !pOUText->isEmpty() )
                pFCell->SetHybridString( *pOUText );
            else
                bDoIncrement = false;
            pFCell->ResetDirty();
        }
    }
    else //regular text cells
    {
        ScBaseCell* pNewCell = NULL;
        ScDocument* pDoc = rXMLImport.GetDocument();
        if (pOUTextValue && !pOUTextValue->isEmpty())
            pNewCell = ScBaseCell::CreateTextCell( *pOUTextValue, pDoc );
        else if (pOUTextContent && !pOUTextContent->isEmpty())
            pNewCell = ScBaseCell::CreateTextCell( *pOUTextContent, pDoc );
        else if ( nCurrentCol > 0 && pOUText && !pOUText->isEmpty() )
            pNewCell = ScBaseCell::CreateTextCell( *pOUText, pDoc );

        bDoIncrement = pNewCell != NULL;
        if ( bDoIncrement )
            pDoc->PutCell( rCurrentPos, pNewCell );
    }
    // #i56027# This is about setting simple text, not edit cells,
    // so ProgressBarIncrement must be called with bEditCell = FALSE.
    // Formatted text that is put into the cell by the child context
    // is handled in AddCellsToTable() (bIsEmpty is true then).
    if (bDoIncrement || bHasTextImport)
        rXMLImport.ProgressBarIncrement(false);
}

void ScXMLTableRowCellContext::PutValueCell( const ScAddress& rCurrentPos )
{
    //matrix reference cells that contain value formula results;
    //cell was already put in document, just need to set value here.
    if( rXMLImport.GetTables().IsPartOfMatrix(rCurrentPos) )
    {
        ScBaseCell* pCell = rXMLImport.GetDocument()->GetCell( rCurrentPos );
        if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
        {
            ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pCell);
            SetFormulaCell(pFCell);
        }
    }
    else  //regular value cell
    {
        // #i62435# Initialize the value cell's script type
        // if the default style's number format is latin-only.
        // If the cell uses a different format, the script type
        // will be reset when the style is applied.

        ScBaseCell* pNewCell = new ScValueCell(fValue);
        if ( rXMLImport.IsLatinDefaultStyle() )
            pNewCell->SetScriptType( SCRIPTTYPE_LATIN );
        rXMLImport.GetDocument()->PutCell(
            rCurrentPos.Col(), rCurrentPos.Row(),
            rCurrentPos.Tab(), pNewCell );
    }
    rXMLImport.ProgressBarIncrement(false);
}

namespace {

bool isEmptyOrNote( ScDocument* pDoc, const ScAddress& rCurrentPos )
{
    ScBaseCell* pCell = pDoc->GetCell( rCurrentPos );
    return ( !pCell || pCell->GetCellType() == CELLTYPE_NOTE );
}

}

void ScXMLTableRowCellContext::AddTextAndValueCells( const ScAddress& rCellPos,
        const ::boost::optional< rtl::OUString >& pOUText, ScAddress& rCurrentPos )
{
    ScMyTables& rTables = rXMLImport.GetTables();
    bool bWasEmpty = bIsEmpty;
    for (SCCOL i = 0; i < nColsRepeated; ++i)
    {
        rCurrentPos.SetCol( rCellPos.Col() + i );
        if (i > 0)
            rTables.AddColumn(false);
        if (!bIsEmpty)
        {
            for (SCROW j = 0; j < nRepeatedRows; ++j)
            {
                rCurrentPos.SetRow( rCellPos.Row() + j );
                if( (rCurrentPos.Col() == 0) && (j > 0) )
                {
                    rTables.AddRow();
                    rTables.AddColumn(false);
                }
                if( cellExists(rCurrentPos) )
                {
                    if(  ( !(bIsCovered) || isEmptyOrNote(rXMLImport.GetDocument(), rCurrentPos) )  )
                    {
                        switch (nCellType)
                        {
                            case util::NumberFormat::TEXT:
                            {
                                PutTextCell( rCurrentPos, i, pOUText );
                            }
                            break;
                            case util::NumberFormat::NUMBER:
                            case util::NumberFormat::PERCENT:
                            case util::NumberFormat::CURRENCY:
                            case util::NumberFormat::TIME:
                            case util::NumberFormat::DATETIME:
                            case util::NumberFormat::LOGICAL:
                            {
                                PutValueCell( rCurrentPos );
                            }
                            break;
                            default:
                            {
                                OSL_FAIL("no cell type given");
                            }
                            break;
                        }
                    }

                    SetAnnotation( rCurrentPos );
                    SetDetectiveObj( rCurrentPos );
                    SetCellRangeSource( rCurrentPos );
                }
                else
                {
                    if (!bWasEmpty || mxAnnotationData.get())
                    {
                        if (rCurrentPos.Row() > MAXROW)
                            rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_ROW_OVERFLOW);
                        else
                            rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_COLUMN_OVERFLOW);
                    }
                }
            }
        }
        else
        {
            // #i56027# If the child context put formatted text into the cell,
            // bIsEmpty is true and ProgressBarIncrement has to be called
            // with bEditCell = TRUE.
            if (bHasTextImport)
                rXMLImport.ProgressBarIncrement(true);
            if ((i == 0) && (rCellPos.Col() == 0))
            {
                for (sal_Int32 j = 1; j < nRepeatedRows; ++j)
                {
                    rTables.AddRow();
                    rTables.AddColumn(false);
                }
            }
        }
    }
}

bool ScXMLTableRowCellContext::HasSpecialContent() const
{
    return ( (pContentValidationName && !pContentValidationName->isEmpty()) ||
              mxAnnotationData.get() || pDetectiveObjVec || pCellRangeSource );
}

bool ScXMLTableRowCellContext::CellsAreRepeated() const
{
    return ( (nColsRepeated > 1) || (nRepeatedRows > 1) );
}

namespace {

// from ScCellObj::GetOutputString_Imp().  all of it may not be necessary.
rtl::OUString getOutputString(ScDocument* pDoc, const ScAddress& aCellPos)
{
    rtl::OUString aVal;
    if ( pDoc )
    {
        ScBaseCell* pCell = pDoc->GetCell( aCellPos );
        if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
        {
            if ( pCell->GetCellType() == CELLTYPE_EDIT )
            {
                //  GetString an der EditCell macht Leerzeichen aus Umbruechen,
                //  hier werden die Umbrueche aber gebraucht
                const EditTextObject* pData = ( static_cast< ScEditCell* >(pCell) )->GetData();
                if (pData)
                {
                    EditEngine& rEngine = pDoc->GetEditEngine();
                    rEngine.SetText( *pData );
                    aVal = rEngine.GetText( LINEEND_LF );
                }
                //  Edit-Zellen auch nicht per NumberFormatter formatieren
                //  (passend zur Ausgabe)
            }
            else
            {
                //  wie in GetString am Dokument (column)
                Color* pColor;
                sal_uLong nNumFmt = pDoc->GetNumberFormat( aCellPos );
                ScCellFormat::GetString( pCell, nNumFmt, aVal, &pColor, *pDoc->GetFormatTable() );
            }
        }
    }
    return aVal;
}

}

void ScXMLTableRowCellContext::AddNonFormulaCells( const ScAddress& rCellPos )
{
    ::boost::optional< rtl::OUString > pOUText;

    if( nCellType == util::NumberFormat::TEXT )
    {
        if( cellExists(rCellPos) && CellsAreRepeated() )
            pOUText.reset( getOutputString(rXMLImport.GetDocument(), rCellPos) );

        if( !pOUTextContent && !pOUText && !pOUTextValue )
                bIsEmpty = true;
    }

    ScAddress aCurrentPos( rCellPos );
    if( HasSpecialContent() )
        bIsEmpty = false;

    AddTextAndValueCells( rCellPos, pOUText, aCurrentPos );

    if( CellsAreRepeated() )
    {
        SCCOL nStartCol( rCellPos.Col() < MAXCOL ? rCellPos.Col() : MAXCOL );
        SCROW nStartRow( rCellPos.Row() < MAXROW ? rCellPos.Row() : MAXROW );
        SCCOL nEndCol( rCellPos.Col() + nColsRepeated - 1 < MAXCOL ? rCellPos.Col() + nColsRepeated - 1 : MAXCOL );
        SCROW nEndRow( rCellPos.Row() + nRepeatedRows - 1 < MAXROW ? rCellPos.Row() + nRepeatedRows - 1 : MAXROW );
        ScRange aScRange( nStartCol, nStartRow, rCellPos.Tab(), nEndCol, nEndRow, rCellPos.Tab() );
        SetContentValidation( aScRange );
        rXMLImport.GetStylesImportHelper()->AddRange( aScRange );
    }
    else if( cellExists(rCellPos) )
    {
        rXMLImport.GetStylesImportHelper()->AddCell(rCellPos);
        SetContentValidation( rCellPos );
    }
}

void ScXMLTableRowCellContext::PutFormulaCell( const ScAddress& rCellPos )
{
    ScDocument* pDoc = rXMLImport.GetDocument();

    rtl::OUString aText = pOUFormula->first;
    rtl::OUString aFormulaNmsp = pOUFormula->second;

    ::boost::scoped_ptr<ScExternalRefManager::ApiGuard> pExtRefGuard;
    pExtRefGuard.reset(new ScExternalRefManager::ApiGuard(pDoc));

    ScBaseCell* pNewCell = NULL;

    if ( !aText.isEmpty() )
    {
        if ( aText[0] == '=' && aText.getLength() > 1 )
        {
            // temporary formula string as string tokens
            ScTokenArray* pCode = new ScTokenArray;
            pCode->AddStringXML( aText );
            if( (eGrammar == formula::FormulaGrammar::GRAM_EXTERNAL) && !aFormulaNmsp.isEmpty() )
                pCode->AddStringXML( aFormulaNmsp );

            pDoc->IncXMLImportedFormulaCount( aText.getLength() );
            pNewCell = new ScFormulaCell( pDoc, rCellPos, pCode, eGrammar, MM_NONE );
            delete pCode;

            ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pNewCell);
            SetFormulaCell(pFCell);
        }
        else if ( aText[0] == '\'' && aText.getLength() > 1 )
        {
            //  for bEnglish, "'" at the beginning is always interpreted as text
            //  marker and stripped
            pNewCell = ScBaseCell::CreateTextCell( aText.copy( 1 ), pDoc );
        }
        else
        {
            SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
            sal_uInt32 nEnglish = pFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US);
            double fVal;
            if ( pFormatter->IsNumberFormat( aText, nEnglish, fVal ) )
                pNewCell = new ScValueCell( fVal );
            //the (english) number format will not be set
            //search matching local format and apply it
            else
                pNewCell = ScBaseCell::CreateTextCell( aText, pDoc );
        }
        pDoc->PutCell( rCellPos, pNewCell );
    }
}

void ScXMLTableRowCellContext::AddFormulaCell( const ScAddress& rCellPos )
{
    if( cellExists(rCellPos) )
    {
        SetContentValidation( rCellPos );
        OSL_ENSURE(((nColsRepeated == 1) && (nRepeatedRows == 1)), "repeated cells with formula not possible now");
        rXMLImport.GetStylesImportHelper()->AddCell(rCellPos);

        //add matrix
        if(bIsMatrix)
        {
            if (nMatrixCols > 0 && nMatrixRows > 0)
            {
                //matrix cells are put in the document, but we must set the
                //value/text of each matrix cell later
                rXMLImport.GetTables().AddMatrixRange(
                        rCellPos.Col(), rCellPos.Row(),
                        rCellPos.Col() + nMatrixCols - 1,
                        rCellPos.Row() + nMatrixRows - 1,
                        pOUFormula->first, pOUFormula->second, eGrammar);

                //set the value/text of the first matrix position (top-left).
                //the value/text of the matrix reference cells will be set later.
                ScFormulaCell* pFCell =
                    static_cast<ScFormulaCell*>( rXMLImport.GetDocument()->GetCell(rCellPos) );
                SetFormulaCell(pFCell);
            }
        }
        else
            PutFormulaCell( rCellPos );

        SetAnnotation( rCellPos );
        SetDetectiveObj( rCellPos );
        SetCellRangeSource( rCellPos );
        rXMLImport.ProgressBarIncrement(false);
    }
    else
    {
        if (rCellPos.Row() > MAXROW)
            rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_ROW_OVERFLOW);
        else
            rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_COLUMN_OVERFLOW);
    }
}

//There are cases where a formula cell is exported with an office:value of 0 or
//no office:value at all, but the formula cell will have a text:p value which
//contains the intended formula result.
//These cases include when a formula result:
// - is blank
// - has a constant error value beginning with "#" (such as "#VALUE!" or "#N/A")
// - has an "Err:[###]" (where "[###]" is an error number)
bool ScXMLTableRowCellContext::HasSpecialCaseFormulaText() const
{
    if(  pOUTextContent &&
         ( pOUTextContent->isEmpty() || (pOUTextContent->indexOf("#") > -1) ||
           (pOUTextContent->indexOf("Err:")  > -1) )
      )
        return true;
    return false;
}


void ScXMLTableRowCellContext::EndElement()
{
    if( bHasTextImport && rXMLImport.GetRemoveLastChar() )
    {
        UniReference< XMLTextImportHelper > aTextImport = rXMLImport.GetTextImport();
        if( aTextImport->GetCursor().is() )
        {
            if( aTextImport->GetCursor()->goLeft(1, true) )
            {
                aTextImport->GetText()->insertString(
                    aTextImport->GetCursorAsRange(), rtl::OUString(), true );
            }
            aTextImport->ResetCursor();
        }
    }

    if( bFormulaTextResult && HasSpecialCaseFormulaText() )
    {
        pOUTextValue.reset(*pOUTextContent);
        nCellType = util::NumberFormat::TEXT;
    }

    ScAddress aCellPos = rXMLImport.GetTables().GetCurrentCellPos();
    if( aCellPos.Col() > 0 && nRepeatedRows > 1 )
        aCellPos.SetRow( aCellPos.Row() - (nRepeatedRows - 1) );
    if( bIsMerged )
        DoMerge( aCellPos, nMergedCols - 1, nMergedRows - 1 );
    if( !pOUFormula )
        AddNonFormulaCells( aCellPos );
    else // if ( pOUFormula )
        AddFormulaCell( aCellPos );

    UnlockSolarMutex(); //if LockSolarMutex got used, we presumably need to ensure an UnlockSolarMutex

    bIsMerged = false;
    nMergedCols = 1;
    nMergedRows = 1;
    nColsRepeated = 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
