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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include "xmlcelli.hxx"
#include "xmltabi.hxx"
#include "xmlstyli.hxx"
#include "xmlannoi.hxx"
#include "document.hxx"
#include "cellsuno.hxx"
#include "XMLTableShapeImportHelper.hxx"
#include "XMLTextPContext.hxx"
#include "XMLStylesImportHelper.hxx"

#include "unonames.hxx"
// core implementation
#include "arealink.hxx"
// core implementation
#include <bf_svx/linkmgr.hxx>
#include "convuno.hxx"
#include "XMLConverter.hxx"
#include "scerrors.hxx"

#include <bf_xmloff/nmspmap.hxx>
#include <bf_xmloff/xmluconv.hxx>
#include <bf_xmloff/xmlnmspe.hxx>
#include <bf_svtools/zforlist.hxx>


#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <comphelper/extract.hxx>
namespace binfilter {

#define SC_CURRENCYSYMBOL	"CurrencySymbol"

using namespace ::com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLTableRowCellContext::ScXMLTableRowCellContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                                      const sal_Bool bTempIsCovered,
                                      const sal_Int32 nTempRepeatedRows ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    bIsMerged(sal_False),
    bIsMatrix(sal_False),
    bHasSubTable(sal_False),
    bIsCovered(bTempIsCovered),
    nRepeatedRows(nTempRepeatedRows),
    bIsEmpty(sal_True),
    bHasTextImport(sal_False),
    bIsFirstTextImport(sal_False),
    pDetectiveObjVec(NULL),
    pCellRangeSource(NULL),
    pMyAnnotation(NULL),
    pOUTextValue(NULL),
    pOUTextContent(NULL),
    pOUFormula(NULL),
    pContentValidationName(NULL),
    nCellType(util::NumberFormat::TEXT),
    nMergedCols(1),
    nMergedRows(1),
    nCellsRepeated(1),
    fValue(0.0),
    rXMLImport((ScXMLImport&)rImport),
    bSolarMutexLocked(sal_False),
    bFormulaTextResult(sal_False)
{
    rXMLImport.SetRemoveLastChar(sal_False);
    rXMLImport.GetTables().AddColumn(bTempIsCovered);
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    ::rtl::OUString aLocalName;
    ::rtl::OUString sValue;
    ::rtl::OUString* pStyleName = NULL;
    ::rtl::OUString* pCurrencySymbol = NULL;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        sal_uInt16 nPrefix = rXMLImport.GetNamespaceMap().GetKeyByAttrName(
                                            xAttrList->getNameByIndex( i ), &aLocalName );
        sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            sal_uInt32 nLength(aLocalName.getLength());

            switch (nLength)
            {
            case 5 :
                {
                    if (IsXMLToken(aLocalName, XML_VALUE))
                    {
                        if (sValue.getLength())
                        {
                            rXMLImport.GetMM100UnitConverter().convertDouble(fValue, sValue);
                            bIsEmpty = sal_False;
                        }
                    }
                }
                break;
            case 7 :
                {
                    if (IsXMLToken(aLocalName, XML_FORMULA))
                    {
                        if (sValue.getLength())
                        {
                            DBG_ASSERT(!pOUFormula, "here should be only one formula");
                            pOUFormula = new ::rtl::OUString(sValue);
                        }
                    }
                }
                break;
            case 8 :
                {
                    if (IsXMLToken(aLocalName, XML_CURRENCY))
                        pCurrencySymbol = new ::rtl::OUString(sValue);
                }
                break;
            case 10 :
                {
                    if (IsXMLToken(aLocalName, XML_VALUE_TYPE))
                    {
                        nCellType = GetCellType(sValue);
                        bIsEmpty = sal_False;
                    }
                    else if (IsXMLToken(aLocalName, XML_STYLE_NAME))
                        pStyleName = new ::rtl::OUString(sValue);
                    else if (IsXMLToken(aLocalName, XML_DATE_VALUE))
                    {
                        if (sValue.getLength() && rXMLImport.SetNullDateOnUnitConverter())
                        {
                            rXMLImport.GetMM100UnitConverter().convertDateTime(fValue, sValue);
                            bIsEmpty = sal_False;
                        }
                    }
                    else if (IsXMLToken(aLocalName, XML_TIME_VALUE))
                    {
                        if (sValue.getLength())
                        {
                            rXMLImport.GetMM100UnitConverter().convertTime(fValue, sValue);
                            bIsEmpty = sal_False;
                        }
                    }
                }
                break;
            case 12 :
                {
                    if (IsXMLToken(aLocalName, XML_STRING_VALUE))
                    {
                        if (sValue.getLength())
                        {
                            DBG_ASSERT(!pOUTextValue, "here should be only one string value");
                            pOUTextValue = new ::rtl::OUString(sValue);
                            bIsEmpty = sal_False;
                        }
                    }
                }
                break;
            case 13 :
                {
                    if (IsXMLToken(aLocalName, XML_BOOLEAN_VALUE))
                    {
                        if (sValue.getLength())
                        {
                            if ( IsXMLToken(sValue, XML_TRUE) )
                                fValue = 1.0;
                            else if ( IsXMLToken(sValue, XML_FALSE) )
                                fValue = 0.0;
                            else
                                rXMLImport.GetMM100UnitConverter().convertDouble(fValue, sValue);
                            bIsEmpty = sal_False;
                        }
                    }
                }
                break;
            case 15 :
                {
                    if (IsXMLToken(aLocalName, XML_CONTENT_VALIDATION_NAME)) //#109340# the string in this constant is different to the name; should be changed ASAP
                    {
                        DBG_ASSERT(!pContentValidationName, "here should be only one Validation Name");
                        pContentValidationName = new ::rtl::OUString(sValue);
                    }
                }
                break;
            case 19 :
                {
                    if (IsXMLToken(aLocalName, XML_NUMBER_ROWS_SPANNED))
                    {
                        bIsMerged = sal_True;
                        nMergedRows = sValue.toInt32();
                    }
                }
                break;
            case 22 :
                {
                    if (IsXMLToken(aLocalName, XML_NUMBER_COLUMNS_SPANNED))
                    {
                        bIsMerged = sal_True;
                        nMergedCols = sValue.toInt32();
                    }
                }
                break;
            case 23 :
                {
                    if (IsXMLToken(aLocalName, XML_NUMBER_COLUMNS_REPEATED))
                        nCellsRepeated = sValue.toInt32();
                }
                break;
            case 26 :
                {
                    if (IsXMLToken(aLocalName, XML_NUMBER_MATRIX_ROWS_SPANNED))
                    {
                        bIsMatrix = sal_True;
                        nMatrixRows = sValue.toInt32();
                    }
                }
                break;
            case 29 :
                {
                    if (IsXMLToken(aLocalName, XML_NUMBER_MATRIX_COLUMNS_SPANNED))
                    {
                        bIsMatrix = sal_True;
                        nMatrixCols = sValue.toInt32();
                    }
                }
                break;
            }
        }
    }
    if (pOUFormula)
    {
        if (nCellType == util::NumberFormat::TEXT)
            bFormulaTextResult = sal_True;
        nCellType = util::NumberFormat::UNDEFINED;
    }
    rXMLImport.GetStylesImportHelper()->SetAttributes(pStyleName, pCurrencySymbol, nCellType);
}

sal_Int16 ScXMLTableRowCellContext::GetCellType(const ::rtl::OUString& sOUValue) const
{
    if (IsXMLToken(sOUValue, XML_FLOAT))
        return util::NumberFormat::NUMBER;
    else
        if (IsXMLToken(sOUValue, XML_STRING))
            return util::NumberFormat::TEXT;
        else
            if (IsXMLToken(sOUValue, XML_TIME))
                return util::NumberFormat::TIME;
            else
                if (IsXMLToken(sOUValue, XML_DATE))
                    return util::NumberFormat::DATETIME;
                else
                    if (IsXMLToken(sOUValue, XML_PERCENTAGE))
                        return util::NumberFormat::PERCENT;
                    else
                        if (IsXMLToken(sOUValue, XML_CURRENCY))
                            return util::NumberFormat::CURRENCY;
                        else
                            if (IsXMLToken(sOUValue, XML_BOOLEAN))
                                return util::NumberFormat::LOGICAL;
                            else
                                return util::NumberFormat::UNDEFINED;
}

ScXMLTableRowCellContext::~ScXMLTableRowCellContext()
{
    if (pOUTextValue)
        delete pOUTextValue;
    if (pOUTextContent)
        delete pOUTextContent;
    if (pOUFormula)
        delete pOUFormula;
    if (pContentValidationName)
        delete pContentValidationName;
    if (pMyAnnotation)
        delete pMyAnnotation;
    if (pDetectiveObjVec)
        delete pDetectiveObjVec;
    if (pCellRangeSource)
        delete pCellRangeSource;
}

void ScXMLTableRowCellContext::LockSolarMutex()
{
    if (!bSolarMutexLocked)
    {
        GetScImport().LockSolarMutex();
        bSolarMutexLocked = sal_True;
    }
}

void ScXMLTableRowCellContext::UnlockSolarMutex()
{
    if (bSolarMutexLocked)
    {
        GetScImport().UnlockSolarMutex();
        bSolarMutexLocked = sal_False;
    }
}

void ScXMLTableRowCellContext::SetCursorOnTextImport(const ::rtl::OUString& rOUTempText)
{
    ::com::sun::star::table::CellAddress aCellPos = rXMLImport.GetTables().GetRealCellPos();
    if (CellExists(aCellPos))
    {
        uno::Reference<table::XCellRange> xCellRange = rXMLImport.GetTables().GetCurrentXCellRange();
        if (xCellRange.is())
        {
            xBaseCell = xCellRange->getCellByPosition(aCellPos.Column, aCellPos.Row);
            if (xBaseCell.is())
            {
                xLockable = uno::Reference<document::XActionLockable>(xBaseCell, uno::UNO_QUERY);
                if (xLockable.is())
                    xLockable->addActionLock();
                uno::Reference<text::XText> xText(xBaseCell, uno::UNO_QUERY);
                if (xText.is())
                {
                    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
                    if (xTextCursor.is())
                    {
                        xTextCursor->setString(rOUTempText);
                        xTextCursor->gotoEnd(sal_False);
                        rXMLImport.GetTextImport()->SetCursor(xTextCursor);
                    }
                }
            }
        }
    }
    else
        DBG_ERRORFILE("this method should only be called for a existing cell");
}

SvXMLImportContext *ScXMLTableRowCellContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = rXMLImport.GetTableRowCellElemTokenMap();
    sal_Bool bHeader(sal_False);
    sal_Bool bTextP(sal_False);
    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
    case XML_TOK_TABLE_ROW_CELL_P:
        {
            bIsEmpty = sal_False;
            bTextP = sal_True;
            ::com::sun::star::table::CellAddress aCellPos = rXMLImport.GetTables().GetRealCellPos();
            if (((nCellType == util::NumberFormat::TEXT) || bFormulaTextResult) && 
                !rXMLImport.GetTables().IsPartOfMatrix(aCellPos.Column, aCellPos.Row))
            {
                if (!bHasTextImport)
                {
                    bIsFirstTextImport = sal_True;
                    bHasTextImport = sal_True;
                    pContext = new ScXMLTextPContext(rXMLImport, nPrefix, rLName, xAttrList, this);
                }
                else
                {
                    ::com::sun::star::table::CellAddress aCellPos = rXMLImport.GetTables().GetRealCellPos();
                    if (CellExists(aCellPos))
                    {
                        if (bIsFirstTextImport && !rXMLImport.GetRemoveLastChar())
                        {
                            if (pOUTextContent)
                            {
                                SetCursorOnTextImport(*pOUTextContent);
                                delete pOUTextContent;
                                pOUTextContent = NULL;
                            }
                            else
                                SetCursorOnTextImport(::rtl::OUString());
                            rXMLImport.SetRemoveLastChar(sal_True);
                            uno::Reference<text::XTextCursor> xTextCursor = rXMLImport.GetTextImport()->GetCursor();
                            uno::Reference < text::XText > xText (xTextCursor->getText());
                            uno::Reference < text::XTextRange > xTextRange (xTextCursor, uno::UNO_QUERY);
                            if (xText.is() && xTextRange.is())
                                xText->insertControlCharacter(xTextRange, text::ControlCharacter::PARAGRAPH_BREAK, sal_False);
                        }
                        pContext = rXMLImport.GetTextImport()->CreateTextChildContext(
                            rXMLImport, nPrefix, rLName, xAttrList);
                        bIsFirstTextImport = sal_False;
                    }
                }
            }
        }
        break;
    case XML_TOK_TABLE_ROW_CELL_SUBTABLE:
        {
            bHasSubTable = sal_True;
            pContext = new ScXMLTableContext( rXMLImport , nPrefix,
                                                        rLName, xAttrList,
                                                        sal_True, nMergedCols);
            nMergedCols = 1;
            bIsMerged = sal_False;
        }
        break;
    case XML_TOK_TABLE_ROW_CELL_ANNOTATION:
        {
            bIsEmpty = sal_False;
            pContext = new ScXMLAnnotationContext( rXMLImport, nPrefix, rLName,
                                                    xAttrList, this);
        }
        break;
    case XML_TOK_TABLE_ROW_CELL_DETECTIVE:
        {
            bIsEmpty = sal_False;
            if (!pDetectiveObjVec)
                pDetectiveObjVec = new ScMyImpDetectiveObjVec();
            pContext = new ScXMLDetectiveContext(
                rXMLImport, nPrefix, rLName, pDetectiveObjVec );
        }
        break;
    case XML_TOK_TABLE_ROW_CELL_CELL_RANGE_SOURCE:
        {
            bIsEmpty = sal_False;
            if (!pCellRangeSource)
                pCellRangeSource = new ScMyImpCellRangeSource();
            pContext = new ScXMLCellRangeSourceContext(
                rXMLImport, nPrefix, rLName, xAttrList, pCellRangeSource );
        }
        break;
    }

    if (!pContext && !bTextP)
    {
        ::com::sun::star::table::CellAddress aCellPos = rXMLImport.GetTables().GetRealCellPos();
        uno::Reference<drawing::XShapes> xShapes (rXMLImport.GetTables().GetCurrentXShapes());
        if (xShapes.is())
        {
            if (aCellPos.Column > MAXCOL)
                aCellPos.Column = MAXCOL;
            if (aCellPos.Row > MAXROW)
                aCellPos.Row = MAXROW;
            XMLTableShapeImportHelper* pTableShapeImport = (XMLTableShapeImportHelper*)rXMLImport.GetShapeImport().get();
            pTableShapeImport->SetOnTable(sal_False);
            pTableShapeImport->SetCell(aCellPos);
            pContext = rXMLImport.GetShapeImport()->CreateGroupChildContext(
                rXMLImport, nPrefix, rLName, xAttrList, xShapes);
            if (pContext)
            {
                bIsEmpty = sal_False;
                rXMLImport.GetProgressBarHelper()->Increment();
            }
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

sal_Bool ScXMLTableRowCellContext::IsMerged (const uno::Reference <table::XCellRange>& xCellRange, const sal_Int32 nCol, const sal_Int32 nRow,
                            table::CellRangeAddress& aCellAddress) const
{
    table::CellAddress aCell; // don't need to set the sheet, because every sheet can contain the same count of cells.
    aCell.Column = nCol;
    aCell.Row = nRow;
    if (CellExists(aCell))
    {
        uno::Reference <table::XCellRange> xMergeCellRange = xCellRange->getCellRangeByPosition(nCol,nRow,nCol,nRow);
        uno::Reference <util::XMergeable> xMergeable (xMergeCellRange, uno::UNO_QUERY);
        if (xMergeable.is())
        {
            uno::Reference<sheet::XSheetCellRange> xMergeSheetCellRange (xMergeCellRange, uno::UNO_QUERY);
            uno::Reference<sheet::XSpreadsheet> xTable = xMergeSheetCellRange->getSpreadsheet();
            uno::Reference<sheet::XSheetCellCursor> xMergeSheetCursor = xTable->createCursorByRange(xMergeSheetCellRange);
            if (xMergeSheetCursor.is())
            {
                xMergeSheetCursor->collapseToMergedArea();
                uno::Reference<sheet::XCellRangeAddressable> xMergeCellAddress (xMergeSheetCursor, uno::UNO_QUERY);
                if (xMergeCellAddress.is())
                {
                    aCellAddress = xMergeCellAddress->getRangeAddress();
                    if (aCellAddress.StartColumn == nCol && aCellAddress.EndColumn == nCol &&
                        aCellAddress.StartRow == nRow && aCellAddress.EndRow == nRow)
                        return sal_False;
                    else
                        return sal_True;
                }
            }
        }
    }
    return sal_False;
}

void ScXMLTableRowCellContext::DoMerge(const ::com::sun::star::table::CellAddress& aCellPos,
                 const sal_Int32 nCols, const sal_Int32 nRows)
{
    if (CellExists(aCellPos))
    {
        uno::Reference<table::XCellRange> xCellRange = rXMLImport.GetTables().GetCurrentXCellRange();
        if ( xCellRange.is() )
        {
            table::CellRangeAddress aCellAddress;
            if (IsMerged(xCellRange, aCellPos.Column, aCellPos.Row, aCellAddress))
            {
                //unmerge
                uno::Reference <table::XCellRange> xMergeCellRange =
                    xCellRange->getCellRangeByPosition(aCellAddress.StartColumn, aCellAddress.StartRow,
                                                        aCellAddress.EndColumn, aCellAddress.EndRow);
                uno::Reference <util::XMergeable> xMergeable (xMergeCellRange, uno::UNO_QUERY);
                if (xMergeable.is())
                    xMergeable->merge(sal_False);
            }

            //merge
            uno::Reference <table::XCellRange> xMergeCellRange =
                xCellRange->getCellRangeByPosition(aCellAddress.StartColumn, aCellAddress.StartRow,
                                                    aCellAddress.EndColumn + nCols, aCellAddress.EndRow + nRows);
            uno::Reference <util::XMergeable> xMergeable (xMergeCellRange, uno::UNO_QUERY);
            if (xMergeable.is())
                xMergeable->merge(sal_True);
        }
    }
}

void ScXMLTableRowCellContext::SetContentValidation(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xPropSet)
{
    if (pContentValidationName)
    {
        ScMyImportValidation aValidation;
        if (rXMLImport.GetValidation(*pContentValidationName, aValidation))
        {
            uno::Any aAny = xPropSet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_VALIDAT)));
            uno::Reference<beans::XPropertySet> xPropertySet;
            if (aAny >>= xPropertySet)
            {
                if (aValidation.sErrorMessage.getLength())
                {
                    aAny <<= aValidation.sErrorMessage;
                    xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ERRMESS)), aAny);
                }
                if (aValidation.sErrorTitle.getLength())
                {
                    aAny <<= aValidation.sErrorTitle;
                    xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ERRTITLE)), aAny);
                }
                if (aValidation.sImputMessage.getLength())
                {
                    aAny <<= aValidation.sImputMessage;
                    xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_INPMESS)), aAny);
                }
                if (aValidation.sImputTitle.getLength())
                {
                    aAny <<= aValidation.sImputTitle;
                    xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_INPTITLE)), aAny);
                }
                aAny = ::cppu::bool2any(aValidation.bShowErrorMessage);
                xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SHOWERR)), aAny);
                aAny = ::cppu::bool2any(aValidation.bShowImputMessage);
                xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SHOWINP)), aAny);
                aAny <<= aValidation.aValidationType;
                xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_TYPE)), aAny);
                aAny = ::cppu::bool2any(aValidation.bIgnoreBlanks);
                xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_IGNOREBL)), aAny);
                aAny <<= aValidation.aAlertStyle;
                xPropertySet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ERRALSTY)), aAny);
                uno::Reference<sheet::XSheetCondition> xCondition(xPropertySet, uno::UNO_QUERY);
                if (xCondition.is())
                {
                        xCondition->setFormula1(aValidation.sFormula1);
                        xCondition->setFormula2(aValidation.sFormula2);
                        xCondition->setOperator(aValidation.aOperator);
                        xCondition->setSourcePosition(aValidation.aBaseCellAddress);
                }
            }
            aAny <<= xPropertySet;
            xPropSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_VALIDAT)), aAny);
        }
    }
}

void ScXMLTableRowCellContext::SetCellProperties(const uno::Reference<table::XCellRange>& xCellRange,
                                                const table::CellAddress& aCellAddress)
{
    if (CellExists(aCellAddress) && pContentValidationName && pContentValidationName->getLength())
    {
        sal_Int32 nBottom = aCellAddress.Row + nRepeatedRows - 1;
        sal_Int32 nRight = aCellAddress.Column + nCellsRepeated - 1;
        if (nBottom > MAXROW)
            nBottom = MAXROW;
        if (nRight > MAXCOL)
            nRight = MAXCOL;
        uno::Reference <table::XCellRange> xPropCellRange = xCellRange->getCellRangeByPosition(aCellAddress.Column, aCellAddress.Row,
            nRight, nBottom);
        if (xPropCellRange.is())
        {
            uno::Reference <beans::XPropertySet> xProperties (xPropCellRange, uno::UNO_QUERY);
            if (xProperties.is())
                SetContentValidation(xProperties);
        }
    }
}

void ScXMLTableRowCellContext::SetCellProperties(const uno::Reference<table::XCell>& xCell)
{
    if (pContentValidationName && pContentValidationName->getLength())
    {
        uno::Reference <beans::XPropertySet> xProperties (xCell, uno::UNO_QUERY);
        if (xProperties.is())
            SetContentValidation(xProperties);
    }
}

void ScXMLTableRowCellContext::SetAnnotation(const uno::Reference<table::XCell>& xCell)
{
    /*uno::Reference<sheet::XSheetAnnotationAnchor> xSheetAnnotationAnchor(xCell, uno::UNO_QUERY);
    if (xSheetAnnotationAnchor.is())
    {
        uno::Reference <sheet::XSheetAnnotation> xSheetAnnotation = xSheetAnnotationAnchor->getAnnotation();
        uno::Reference<text::XSimpleText> xSimpleText(xSheetAnnotation, uno::UNO_QUERY);
        if (xSheetAnnotation.is() && xSimpleText.is())
        {
            xSimpleText->setString(aMyAnnotation.sText);
            //xSheetAnnotation->setAuthor(aMyAnnotation.sAuthor);
            //xSheetAnnotation->setDate();
            xSheetAnnotation->setIsVisible(aMyAnnotation.bDisplay);
        }
    }*/
    if( pMyAnnotation )
    {
        uno::Reference<sheet::XCellAddressable> xCellAddressable(xCell, uno::UNO_QUERY);
        if (xCellAddressable.is())
        {
            table::CellAddress aCellAddress = xCellAddressable->getCellAddress();
            double fDate;
            rXMLImport.GetMM100UnitConverter().convertDateTime(fDate, pMyAnnotation->sCreateDate);
            ScDocument* pDoc = rXMLImport.GetDocument();
            if (pDoc)
            {
                LockSolarMutex();
                SvNumberFormatter* pNumForm = pDoc->GetFormatTable();
                sal_uInt32 nfIndex = pNumForm->GetFormatIndex(NF_DATE_SYS_DDMMYYYY, LANGUAGE_SYSTEM);
                String sDate;
                Color* pColor = NULL;
                Color** ppColor = &pColor;
                pNumForm->GetOutputString(fDate, nfIndex, sDate, ppColor);
                ScPostIt aNote(String(pMyAnnotation->sText), sDate, String(pMyAnnotation->sAuthor));
                aNote.SetShown(pMyAnnotation->bDisplay);
                pDoc->SetNote(static_cast<USHORT>(aCellAddress.Column), static_cast<USHORT>(aCellAddress.Row), aCellAddress.Sheet, aNote);
                if (pMyAnnotation->bDisplay)
                {
                    uno::Reference < drawing::XShapes > xShapes (rXMLImport.GetTables().GetCurrentXShapes());	// make draw page
                    ScDetectiveFunc aDetFunc(pDoc, aCellAddress.Sheet);
                    aDetFunc.ShowComment(static_cast<USHORT>(aCellAddress.Column), static_cast<USHORT>(aCellAddress.Row), sal_False);
                    uno::Reference<container::XIndexAccess> xShapesIndex (xShapes, uno::UNO_QUERY);
                    if (xShapesIndex.is())
                    {
                        sal_Int32 nShapes = xShapesIndex->getCount();
                        uno::Reference < drawing::XShape > xShape;
                        rXMLImport.GetShapeImport()->shapeWithZIndexAdded(xShape, nShapes);
                    }
                }
            }
        }
    }
}

// core implementation
void ScXMLTableRowCellContext::SetDetectiveObj( const table::CellAddress& rPosition )
{
    if( CellExists(rPosition) && pDetectiveObjVec && pDetectiveObjVec->size() )
    {
        LockSolarMutex();
        ScDetectiveFunc	aDetFunc( rXMLImport.GetDocument(), rPosition.Sheet );
        uno::Reference < drawing::XShapes > xShapes (rXMLImport.GetTables().GetCurrentXShapes());	// make draw page
        for( ScMyImpDetectiveObjVec::iterator aItr = pDetectiveObjVec->begin(); aItr != pDetectiveObjVec->end(); aItr++ )
        {
            ScAddress aScAddress;
            ScUnoConversion::FillScAddress( aScAddress, rPosition );
            aDetFunc.InsertObject( aItr->eObjType, aScAddress, aItr->aSourceRange, aItr->bHasError );
            uno::Reference<container::XIndexAccess> xShapesIndex (xShapes, uno::UNO_QUERY);
            if (xShapesIndex.is())
            {
                sal_Int32 nShapes = xShapesIndex->getCount();
                uno::Reference < drawing::XShape > xShape;
                rXMLImport.GetShapeImport()->shapeWithZIndexAdded(xShape, nShapes);
            }
        }
    }
}

// core implementation
void ScXMLTableRowCellContext::SetCellRangeSource( const table::CellAddress& rPosition )
{
    if( CellExists(rPosition) && pCellRangeSource  && pCellRangeSource->sSourceStr.getLength() &&
        pCellRangeSource->sFilterName.getLength() && pCellRangeSource->sURL.getLength() )
    {
        ScDocument* pDoc = rXMLImport.GetDocument();
        if (pDoc)
        {
            LockSolarMutex();
            ScRange aDestRange( static_cast<USHORT>(rPosition.Column), static_cast<USHORT>(rPosition.Row), rPosition.Sheet,
                rPosition.Column + pCellRangeSource->nColumns - 1,
                rPosition.Row + pCellRangeSource->nRows - 1, rPosition.Sheet );
            String sFilterName( pCellRangeSource->sFilterName );
            String sSourceStr( pCellRangeSource->sSourceStr );
            ScAreaLink* pLink = new ScAreaLink( pDoc->GetDocumentShell(), pCellRangeSource->sURL,
                sFilterName, pCellRangeSource->sFilterOptions, sSourceStr, aDestRange, pCellRangeSource->nRefresh );
            SvxLinkManager* pLinkManager = pDoc->GetLinkManager();
            pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, pCellRangeSource->sURL, &sFilterName, &sSourceStr );
        }
    }
}

void ScXMLTableRowCellContext::EndElement()
{
    if (!bHasSubTable)
    {
        if (bHasTextImport && rXMLImport.GetRemoveLastChar())
        {
            if (GetImport().GetTextImport()->GetCursor().is())
            {
                //GetImport().GetTextImport()->GetCursor()->gotoEnd(sal_False);
                if( GetImport().GetTextImport()->GetCursor()->goLeft( 1, sal_True ) )
                {
                    GetImport().GetTextImport()->GetText()->insertString(
                        GetImport().GetTextImport()->GetCursorAsRange(), ::rtl::OUString(),
                        sal_True );
                }
                rXMLImport.GetTextImport()->ResetCursor();
            }
        }
        table::CellAddress aCellPos = rXMLImport.GetTables().GetRealCellPos();
        if (aCellPos.Column > 0 && nRepeatedRows > 1)
            aCellPos.Row -= (nRepeatedRows - 1);
        uno::Reference<table::XCellRange> xCellRange = rXMLImport.GetTables().GetCurrentXCellRange();
        if (xCellRange.is())
        {
            if (bIsMerged)
                DoMerge(aCellPos, nMergedCols - 1, nMergedRows - 1);
            if ( !pOUFormula )
            {
                ::rtl::OUString* pOUText = NULL;
                if(nCellType == util::NumberFormat::TEXT)
                {
                    if (xLockable.is())
                        xLockable->removeActionLock();
                    if (CellExists(aCellPos) && ((nCellsRepeated > 1) || (nRepeatedRows > 1)))
                    {
                        if (!xBaseCell.is())
                        {
                            try
                            {
                                xBaseCell = xCellRange->getCellByPosition(aCellPos.Column, aCellPos.Row);
                            }
                            catch (lang::IndexOutOfBoundsException&)
                            {
                                DBG_ERRORFILE("It seems here are to many columns or rows");
                            }
                        }
                        uno::Reference <text::XText> xTempText (xBaseCell, uno::UNO_QUERY);
                        if (xTempText.is())
                        {
                            ::rtl::OUString sBla(xTempText->getString());
                            pOUText = new ::rtl::OUString(sBla);
                        }
                    }
                    if (!pOUTextContent && !pOUText && !pOUTextValue)
                        bIsEmpty = sal_True;
                    else if ( (pOUTextContent && !pOUTextContent->getLength()) || !pOUTextContent )
                        if ( (pOUText && !pOUText->getLength()) || !pOUText )
                            if ( (pOUTextValue && !pOUTextValue->getLength()) || !pOUTextValue )
                                bIsEmpty = sal_True;
                }
                sal_Bool bWasEmpty = bIsEmpty;
                uno::Reference <table::XCell> xCell;
                table::CellAddress aCurrentPos( aCellPos );
                if ((pContentValidationName && pContentValidationName->getLength()) ||
                    pMyAnnotation || pDetectiveObjVec || pCellRangeSource)
                    bIsEmpty = sal_False;
                for (sal_Int32 i = 0; i < nCellsRepeated; i++)
                {
                    aCurrentPos.Column = aCellPos.Column + i;
                    if (i > 0)
                        rXMLImport.GetTables().AddColumn(sal_False);
                    if (!bIsEmpty)
                    {
                        for (sal_Int32 j = 0; j < nRepeatedRows; j++)
                        {
                            aCurrentPos.Row = aCellPos.Row + j;
                            if ((aCurrentPos.Column == 0) && (j > 0))
                            {
                                rXMLImport.GetTables().AddRow();
                                rXMLImport.GetTables().AddColumn(sal_False);
                            }
                            if (CellExists(aCurrentPos))
                            {
                                if (xBaseCell.is() && (aCurrentPos == aCellPos))
                                    xCell = xBaseCell;
                                else
                                {
                                    try
                                    {
                                        xCell = xCellRange->getCellByPosition(aCurrentPos.Column, aCurrentPos.Row);
                                    }
                                    catch (lang::IndexOutOfBoundsException&)
                                    {
                                        DBG_ERRORFILE("It seems here are to many columns or rows");
                                    }
                                }
                                if ((!(bIsCovered) || (xCell->getType() == table::CellContentType_EMPTY)))
                                {
                                    switch (nCellType)
                                    {
                                    case util::NumberFormat::TEXT:
                                        {
                                            sal_Bool bDoIncrement = sal_True;
                                            if (rXMLImport.GetTables().IsPartOfMatrix(aCurrentPos.Column, aCurrentPos.Row))
                                            {
                                                LockSolarMutex();
                                                ScCellObj* pCellObj = (ScCellObj*)ScCellRangesBase::getImplementation(xCell);
                                                if (pCellObj)
                                                {
                                                    if(pOUTextValue && pOUTextValue->getLength())
                                                        pCellObj->SetFormulaResultString(*pOUTextValue);
                                                    else if (pOUTextContent && pOUTextContent->getLength())
                                                        pCellObj->SetFormulaResultString(*pOUTextContent);
                                                    else if ( i > 0 && pOUText && pOUText->getLength() )
                                                    {
                                                        pCellObj->SetFormulaResultString(*pOUText);
                                                    }
                                                    else
                                                        bDoIncrement = sal_False;
                                                }
                                                else
                                                    bDoIncrement = sal_False;
                                            }
                                            else
                                            {
                                                uno::Reference <text::XText> xText (xCell, uno::UNO_QUERY);
                                                if (xText.is())
                                                {
                                                    if(pOUTextValue && pOUTextValue->getLength())
                                                        xText->setString(*pOUTextValue);
                                                    else if (pOUTextContent && pOUTextContent->getLength())
                                                        xText->setString(*pOUTextContent);
                                                    else if ( i > 0 && pOUText && pOUText->getLength() )
                                                    {
                                                        xText->setString(*pOUText);
                                                    }
                                                    else
                                                        bDoIncrement = sal_False;
                                                }
                                            }
                                            if (bDoIncrement || bHasTextImport)
                                                rXMLImport.GetProgressBarHelper()->Increment();
                                        }
                                        break;
                                    case util::NumberFormat::NUMBER:
                                    case util::NumberFormat::PERCENT:
                                    case util::NumberFormat::CURRENCY:
                                    case util::NumberFormat::TIME:
                                    case util::NumberFormat::DATETIME:
                                    case util::NumberFormat::LOGICAL:
                                        {
                                            if (rXMLImport.GetTables().IsPartOfMatrix(aCurrentPos.Column, aCurrentPos.Row))
                                            {
                                                LockSolarMutex();
                                                ScCellObj* pCellObj = (ScCellObj*)ScCellRangesBase::getImplementation(xCell);
                                                if (pCellObj)
                                                    pCellObj->SetFormulaResultDouble(fValue);
                                            }
                                            else
                                            {
                                                xCell->setValue(fValue);
                                            }
                                            rXMLImport.GetProgressBarHelper()->Increment();
                                        }
                                        break;
                                    default:
                                        {
                                            DBG_ERROR("no cell type given");
                                        }
                                        break;
                                    }
                                }
                                SetAnnotation(xCell);
                                SetDetectiveObj( aCurrentPos );
                                SetCellRangeSource( aCurrentPos );
                            }
                            else
                            {
                                if (!bWasEmpty || (pMyAnnotation))
                                {
                                    if (aCurrentPos.Row > MAXROW)
                                        rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_ROW_OVERFLOW);
                                    else
                                        rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_COLUMN_OVERFLOW);
                                }
                            }
                        }
                    }
                    else
                    {
                        if (bHasTextImport)
                            rXMLImport.GetProgressBarHelper()->Increment();
                        if ((i == 0) && (aCellPos.Column == 0))
                            for (sal_Int32 j = 1; j < nRepeatedRows; j++)
                            {
                                    rXMLImport.GetTables().AddRow();
                                    rXMLImport.GetTables().AddColumn(sal_False);
                            }
                    }
                }
                if (nCellsRepeated > 1 || nRepeatedRows > 1)
                {
                    SetCellProperties(xCellRange, aCellPos); // set now only the validation for the complete range with the given cell as start cell
                    //SetType(xCellRange, aCellPos);
                    USHORT nStartCol(aCellPos.Column < MAXCOL ? aCellPos.Column : MAXCOL);
                    USHORT nStartRow(aCellPos.Row < MAXROW ? aCellPos.Row : MAXROW);
                    USHORT nEndCol(aCellPos.Column + nCellsRepeated - 1 < MAXCOL ? aCellPos.Column + nCellsRepeated - 1 : MAXCOL);
                    USHORT nEndRow(aCellPos.Row + nRepeatedRows - 1 < MAXROW ? aCellPos.Row + nRepeatedRows - 1 : MAXROW);
                    ScRange aScRange( nStartCol, nStartRow, aCellPos.Sheet,
                        nEndCol, nEndRow, aCellPos.Sheet );
                    rXMLImport.GetStylesImportHelper()->AddRange(aScRange);
                }
                else if (CellExists(aCellPos))
                {
                    rXMLImport.GetStylesImportHelper()->AddCell(aCellPos);
                    SetCellProperties(xCell); // set now only the validation
                    //SetType(xTempCell);
                }
                if (pOUText)
                    delete pOUText;
            }
            else
            {
                if (CellExists(aCellPos))
                {
                    uno::Reference <table::XCell> xCell;
                    try
                    {
                        xCell = xCellRange->getCellByPosition(aCellPos.Column , aCellPos.Row);
                    }
                    catch (lang::IndexOutOfBoundsException&)
                    {
                        DBG_ERRORFILE("It seems here are to many columns or rows");
                    }
                    if (xCell.is())
                    {
                        SetCellProperties(xCell); // set now only the validation
                        DBG_ASSERT(((nCellsRepeated == 1) && (nRepeatedRows == 1)), "repeated cells with formula not possible now");
                        rXMLImport.GetStylesImportHelper()->AddCell(aCellPos);
                        ScXMLConverter::ParseFormula(*pOUFormula);
                        if (!bIsMatrix)
                        {
                            xCell->setFormula(*pOUFormula);
                            if (bFormulaTextResult && pOUTextValue && pOUTextValue->getLength())
                            {
                                LockSolarMutex();
                                ScCellObj* pCellObj = (ScCellObj*)ScCellRangesBase::getImplementation(xCell);
                                if (pCellObj)
                                    pCellObj->SetFormulaResultString(*pOUTextValue);
                            }
                            else if (fValue != 0.0)
                            {
                                LockSolarMutex();
                                ScCellObj* pCellObj = (ScCellObj*)ScCellRangesBase::getImplementation(xCell);
                                if (pCellObj)
                                    pCellObj->SetFormulaResultDouble(fValue);
                            }
                        }
                        else
                        {
                            if (nMatrixCols > 0 && nMatrixRows > 0)
                            {
                                rXMLImport.GetTables().AddMatrixRange(aCellPos.Column, aCellPos.Row,
                                                aCellPos.Column + nMatrixCols - 1, aCellPos.Row + nMatrixRows - 1);
                                uno::Reference <table::XCellRange> xMatrixCellRange =
                                    xCellRange->getCellRangeByPosition(aCellPos.Column, aCellPos.Row,
                                                aCellPos.Column + nMatrixCols - 1, aCellPos.Row + nMatrixRows - 1);
                                if (xMatrixCellRange.is())
                                {
                                    uno::Reference <sheet::XArrayFormulaRange> xArrayFormulaRange(xMatrixCellRange, uno::UNO_QUERY);
                                    if (xArrayFormulaRange.is())
                                        xArrayFormulaRange->setArrayFormula(*pOUFormula);
                                }
                            }
                        }
                        SetAnnotation(xCell);
                        SetDetectiveObj( aCellPos );
                        SetCellRangeSource( aCellPos );
                        rXMLImport.GetProgressBarHelper()->Increment();
                    }
                }
                else
                {
                    if (aCellPos.Row > MAXROW)
                        rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_ROW_OVERFLOW);
                    else
                        rXMLImport.SetRangeOverflowType(SCWARN_IMPORT_COLUMN_OVERFLOW);
                }

            }
        }
        UnlockSolarMutex();
    }
    bIsMerged = sal_False;
    bHasSubTable = sal_False;
    nMergedCols = 1;
    nMergedRows = 1;
    nCellsRepeated = 1;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
