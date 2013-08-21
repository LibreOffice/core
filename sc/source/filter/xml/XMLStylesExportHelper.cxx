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

#include "XMLStylesExportHelper.hxx"
#include "global.hxx"
#include "unonames.hxx"
#include "XMLConverter.hxx"
#include "xmlexprt.hxx"
#include "document.hxx"
#include "rangeutl.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/XMLEventExport.hxx>
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSheetCondition.hpp>
#include <com/sun/star/sheet/TableValidationVisibility.hpp>
#include <comphelper/extract.hxx>
#include <sfx2/app.hxx>

#include <algorithm>

using namespace com::sun::star;
using namespace xmloff::token;

ScMyValidation::ScMyValidation()
    : sName(),
    sErrorMessage(),
    sErrorTitle(),
    sImputMessage(),
    sImputTitle(),
    sFormula1(),
    sFormula2(),
    bShowErrorMessage(false),
    bShowImputMessage(false),
    bIgnoreBlanks(false)
{
}

ScMyValidation::~ScMyValidation()
{
}

bool ScMyValidation::IsEqual(const ScMyValidation& aVal) const
{
    if (aVal.bIgnoreBlanks == bIgnoreBlanks &&
        aVal.bShowImputMessage == bShowImputMessage &&
        aVal.bShowErrorMessage == bShowErrorMessage &&
        aVal.aBaseCell.Sheet == aBaseCell.Sheet &&
        aVal.aBaseCell.Column == aBaseCell.Column &&
        aVal.aBaseCell.Row == aBaseCell.Row &&
        aVal.aAlertStyle == aAlertStyle &&
        aVal.aValidationType == aValidationType &&
        aVal.aOperator == aOperator &&
        aVal.sErrorTitle == sErrorTitle &&
        aVal.sImputTitle == sImputTitle &&
        aVal.sErrorMessage == sErrorMessage &&
        aVal.sImputMessage == sImputMessage &&
        aVal.sFormula1 == sFormula1 &&
        aVal.sFormula2 == sFormula2)
        return true;
    else
        return false;
}

ScMyValidationsContainer::ScMyValidationsContainer()
    : aValidationVec(),
    sEmptyString(),
    sERRALSTY(SC_UNONAME_ERRALSTY),
    sIGNOREBL(SC_UNONAME_IGNOREBL),
    sSHOWLIST(SC_UNONAME_SHOWLIST),
    sTYPE(SC_UNONAME_TYPE),
    sSHOWINP(SC_UNONAME_SHOWINP),
    sSHOWERR(SC_UNONAME_SHOWERR),
    sINPTITLE(SC_UNONAME_INPTITLE),
    sINPMESS(SC_UNONAME_INPMESS),
    sERRTITLE(SC_UNONAME_ERRTITLE),
    sERRMESS(SC_UNONAME_ERRMESS),
    sOnError("OnError"),
    sEventType("EventType"),
    sStarBasic("StarBasic"),
    sScript("Script"),
    sLibrary("Library"),
    sMacroName("MacroName")
{
}

ScMyValidationsContainer::~ScMyValidationsContainer()
{
}

bool ScMyValidationsContainer::AddValidation(const uno::Any& aTempAny,
    sal_Int32& nValidationIndex)
{
    bool bAdded(false);
    uno::Reference<beans::XPropertySet> xPropertySet(aTempAny, uno::UNO_QUERY);
    if (xPropertySet.is())
    {
        OUString sErrorMessage;
        xPropertySet->getPropertyValue(sERRMESS) >>= sErrorMessage;
        OUString sErrorTitle;
        xPropertySet->getPropertyValue(sERRTITLE) >>= sErrorTitle;
        OUString sImputMessage;
        xPropertySet->getPropertyValue(sINPMESS) >>= sImputMessage;
        OUString sImputTitle;
        xPropertySet->getPropertyValue(sINPTITLE) >>= sImputTitle;
        bool bShowErrorMessage = ::cppu::any2bool(xPropertySet->getPropertyValue(sSHOWERR));
        bool bShowImputMessage = ::cppu::any2bool(xPropertySet->getPropertyValue(sSHOWINP));
        sheet::ValidationType aValidationType;
        xPropertySet->getPropertyValue(sTYPE) >>= aValidationType;
        if (bShowErrorMessage || bShowImputMessage || aValidationType != sheet::ValidationType_ANY ||
            !sErrorMessage.isEmpty() || !sErrorTitle.isEmpty() || !sImputMessage.isEmpty() || !sImputTitle.isEmpty())
        {
            ScMyValidation aValidation;
            aValidation.sErrorMessage = sErrorMessage;
            aValidation.sErrorTitle = sErrorTitle;
            aValidation.sImputMessage = sImputMessage;
            aValidation.sImputTitle = sImputTitle;
            aValidation.bShowErrorMessage = bShowErrorMessage;
            aValidation.bShowImputMessage = bShowImputMessage;
            aValidation.aValidationType = aValidationType;
            aValidation.bIgnoreBlanks = ::cppu::any2bool(xPropertySet->getPropertyValue(sIGNOREBL));
            xPropertySet->getPropertyValue(sSHOWLIST) >>= aValidation.nShowList;
            xPropertySet->getPropertyValue(sERRALSTY) >>= aValidation.aAlertStyle;
            uno::Reference<sheet::XSheetCondition> xCondition(xPropertySet, uno::UNO_QUERY);
            if (xCondition.is())
            {
                aValidation.sFormula1 = xCondition->getFormula1();
                aValidation.sFormula2 = xCondition->getFormula2();
                aValidation.aOperator = xCondition->getOperator();
                aValidation.aBaseCell = xCondition->getSourcePosition();
            }
            //ScMyValidationRange aValidationRange;
            bool bEqualFound(false);
            sal_Int32 i(0);
            sal_Int32 nCount(aValidationVec.size());
            while (i < nCount && !bEqualFound)
            {
                bEqualFound = aValidationVec[i].IsEqual(aValidation);
                if (!bEqualFound)
                    ++i;
            }
            if (bEqualFound)
                nValidationIndex = i;
            else
            {
                sal_Int32 nNameIndex(nCount + 1);
                OUString sCount(OUString::number(nNameIndex));
                OUString sPrefix("val");
                aValidation.sName += sPrefix;
                aValidation.sName += sCount;
                aValidationVec.push_back(aValidation);
                nValidationIndex = nCount;
                bAdded = true;
            }
        }
    }
    return bAdded;
}

OUString ScMyValidationsContainer::GetCondition(ScXMLExport& rExport, const ScMyValidation& aValidation)
{
    /* ATTENTION! Should the condition to not write sheet::ValidationType_ANY
     * ever be changed, adapt the conditional call of
     * MarkUsedExternalReferences() in
     * ScTableValidationObj::ScTableValidationObj() accordingly! */
    OUString sCondition;
    if (aValidation.aValidationType != sheet::ValidationType_ANY)
    {
        switch (aValidation.aValidationType)
        {
            //case sheet::ValidationType_CUSTOM
            case sheet::ValidationType_DATE :
                sCondition += OUString("cell-content-is-date()");
            break;
            case sheet::ValidationType_DECIMAL :
                sCondition += OUString("cell-content-is-decimal-number()");
            break;
            case sheet::ValidationType_LIST :
                sCondition += OUString("cell-content-is-in-list(");
                sCondition += aValidation.sFormula1;
                sCondition += OUString(")");
            break;
            case sheet::ValidationType_TEXT_LEN :
                if (aValidation.aOperator != sheet::ConditionOperator_BETWEEN &&
                    aValidation.aOperator != sheet::ConditionOperator_NOT_BETWEEN)
                    sCondition += OUString("cell-content-text-length()");
            break;
            case sheet::ValidationType_TIME :
                sCondition += OUString("cell-content-is-time()");
            break;
            case sheet::ValidationType_WHOLE :
                sCondition += OUString("cell-content-is-whole-number()");
            break;
            default:
            {
                // added to avoid warnings
            }
        }
        if (aValidation.aValidationType != sheet::ValidationType_LIST &&
            (!aValidation.sFormula1.isEmpty() ||
            (aValidation.aOperator == sheet::ConditionOperator_BETWEEN &&
            aValidation.aOperator == sheet::ConditionOperator_NOT_BETWEEN &&
            !aValidation.sFormula2.isEmpty())))
        {
            if (aValidation.aValidationType != sheet::ValidationType_TEXT_LEN)
                sCondition += OUString(" and ");
            if (aValidation.aOperator != sheet::ConditionOperator_BETWEEN &&
                aValidation.aOperator != sheet::ConditionOperator_NOT_BETWEEN)
            {
                if (aValidation.aValidationType != sheet::ValidationType_TEXT_LEN)
                    sCondition += OUString("cell-content()");
                switch (aValidation.aOperator)
                {
                    case sheet::ConditionOperator_EQUAL :
                        sCondition += OUString("=");
                    break;
                    case sheet::ConditionOperator_GREATER :
                        sCondition += OUString(">");
                    break;
                    case sheet::ConditionOperator_GREATER_EQUAL :
                        sCondition += OUString(">=");
                    break;
                    case sheet::ConditionOperator_LESS :
                        sCondition += OUString("<");
                    break;
                    case sheet::ConditionOperator_LESS_EQUAL :
                        sCondition += OUString("<=");
                    break;
                    case sheet::ConditionOperator_NOT_EQUAL :
                        sCondition += OUString("!=");
                    break;
                    default:
                    {
                        // added to avoid warnings
                    }
                }
                sCondition += aValidation.sFormula1;
            }
            else
            {
                if (aValidation.aValidationType == sheet::ValidationType_TEXT_LEN)
                {
                    if (aValidation.aOperator == sheet::ConditionOperator_BETWEEN)
                        sCondition += OUString("cell-content-text-length-is-between(");
                    else
                        sCondition += OUString("cell-content-text-length-is-not-between(");
                }
                else
                {
                    if (aValidation.aOperator == sheet::ConditionOperator_BETWEEN)
                        sCondition += OUString("cell-content-is-between(");
                    else
                        sCondition += OUString("cell-content-is-not-between(");
                }
                sCondition += aValidation.sFormula1;
                sCondition += OUString(",");
                sCondition += aValidation.sFormula2;
                sCondition += OUString(")");
            }
        }
        else
            if (aValidation.aValidationType == sheet::ValidationType_TEXT_LEN)
                sCondition = OUString();
    }
    if (!sCondition.isEmpty())
    {
        const formula::FormulaGrammar::Grammar eGrammar = rExport.GetDocument()->GetStorageGrammar();
        sal_uInt16 nNamespacePrefix = (eGrammar == formula::FormulaGrammar::GRAM_ODFF ? XML_NAMESPACE_OF : XML_NAMESPACE_OOOC);
        sCondition = rExport.GetNamespaceMap().GetQNameByKey( nNamespacePrefix, sCondition, false );
    }

    return sCondition;
}

OUString ScMyValidationsContainer::GetBaseCellAddress(ScDocument* pDoc, const table::CellAddress& aCell)
{
    OUString sAddress;
    ScRangeStringConverter::GetStringFromAddress( sAddress, aCell, pDoc, ::formula::FormulaGrammar::CONV_OOO );
    return sAddress;
}

void ScMyValidationsContainer::WriteMessage(ScXMLExport& rExport,
    const OUString& sTitle, const OUString& sOUMessage,
    const bool bShowMessage, const bool bIsHelpMessage)
{
    if (!sTitle.isEmpty())
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TITLE, sTitle);
    if (bShowMessage)
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY, XML_TRUE);
    else
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY, XML_FALSE);
    SvXMLElementExport* pMessage(NULL);
    if (bIsHelpMessage)
        pMessage = new SvXMLElementExport(rExport, XML_NAMESPACE_TABLE, XML_HELP_MESSAGE, true, true);
    else
        pMessage = new SvXMLElementExport(rExport, XML_NAMESPACE_TABLE, XML_ERROR_MESSAGE, true, true);
    if (!sOUMessage.isEmpty())
    {
        sal_Int32 i(0);
        OUStringBuffer sTemp;
        OUString sText(convertLineEnd(sOUMessage, LINEEND_LF));
        bool bPrevCharWasSpace(true);
        while(i < sText.getLength())
        {
            if ((sText[i] == '\n'))
            {
                SvXMLElementExport aElemP(rExport, XML_NAMESPACE_TEXT, XML_P, true, false);
                rExport.GetTextParagraphExport()->exportText(sTemp.makeStringAndClear(), bPrevCharWasSpace);
            }
            else
                sTemp.append(sText[i]);
            ++i;
        }
        if (!sTemp.isEmpty())
        {
            SvXMLElementExport aElemP(rExport, XML_NAMESPACE_TEXT, XML_P, true, false);
            rExport.GetTextParagraphExport()->exportText(sTemp.makeStringAndClear(), bPrevCharWasSpace);
        }
    }
    if (pMessage)
        delete pMessage;
}

void ScMyValidationsContainer::WriteValidations(ScXMLExport& rExport)
{
    if (!aValidationVec.empty())
    {
        SvXMLElementExport aElemVs(rExport, XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATIONS, true, true);
        ScMyValidationVec::iterator aItr(aValidationVec.begin());
        ScMyValidationVec::iterator aEndItr(aValidationVec.end());
        while (aItr != aEndItr)
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, aItr->sName);
            OUString sCondition(GetCondition(rExport, *aItr));
            if (!sCondition.isEmpty())
            {
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CONDITION, sCondition);
                if (aItr->bIgnoreBlanks)
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ALLOW_EMPTY_CELL, XML_TRUE);
                else
                    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ALLOW_EMPTY_CELL, XML_FALSE);
                if (aItr->aValidationType == sheet::ValidationType_LIST)
                {
                    switch (aItr->nShowList)
                    {
                    case sheet::TableValidationVisibility::INVISIBLE:
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_LIST, XML_NO);
                    break;
                    case sheet::TableValidationVisibility::UNSORTED:
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_LIST, XML_UNSORTED);
                    break;
                    case sheet::TableValidationVisibility::SORTEDASCENDING:
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY_LIST, XML_SORTED_ASCENDING);
                    break;
                    default:
                        OSL_FAIL("unknown ListType");
                    }
                }
            }
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_BASE_CELL_ADDRESS, GetBaseCellAddress(rExport.GetDocument(), aItr->aBaseCell));
            SvXMLElementExport aElemV(rExport, XML_NAMESPACE_TABLE, XML_CONTENT_VALIDATION, true, true);
            if (aItr->bShowImputMessage || !aItr->sImputMessage.isEmpty() || !aItr->sImputTitle.isEmpty())
            {
                WriteMessage(rExport, aItr->sImputTitle, aItr->sImputMessage, aItr->bShowImputMessage, true);
            }
            if (aItr->bShowErrorMessage || !aItr->sErrorMessage.isEmpty() || !aItr->sErrorTitle.isEmpty())
            {
                switch (aItr->aAlertStyle)
                {
                    case sheet::ValidationAlertStyle_INFO :
                    {
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MESSAGE_TYPE, XML_INFORMATION);
                        WriteMessage(rExport, aItr->sErrorTitle, aItr->sErrorMessage, aItr->bShowErrorMessage, false);
                    }
                    break;
                    case sheet::ValidationAlertStyle_WARNING :
                    {
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MESSAGE_TYPE, XML_WARNING);
                        WriteMessage(rExport, aItr->sErrorTitle, aItr->sErrorMessage, aItr->bShowErrorMessage, false);
                    }
                    break;
                    case sheet::ValidationAlertStyle_STOP :
                    {
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MESSAGE_TYPE, XML_STOP);
                        WriteMessage(rExport, aItr->sErrorTitle, aItr->sErrorMessage, aItr->bShowErrorMessage, false);
                    }
                    break;
                    case sheet::ValidationAlertStyle_MACRO :
                    {
                        {
                            //rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, aItr->sErrorTitle);
                            if (aItr->bShowErrorMessage)
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_EXECUTE, XML_TRUE);
                            else
                                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_EXECUTE, XML_FALSE);
                            SvXMLElementExport aEMElem(rExport, XML_NAMESPACE_TABLE, XML_ERROR_MACRO, true, true);
                        }
                        {
                            // #i47525# for a script URL the type and the property name for the URL
                            // are both "Script", for a simple macro name the type is "StarBasic"
                            // and the property name is "MacroName".
                            bool bScriptURL = SfxApplication::IsXScriptURL( aItr->sErrorTitle );

                            uno::Sequence<beans::PropertyValue> aSeq(3);
                            beans::PropertyValue* pArr(aSeq.getArray());
                            pArr[0].Name = sEventType;
                            pArr[0].Value <<= bScriptURL ? sScript : sStarBasic;
                            pArr[1].Name = sLibrary;
                            pArr[1].Value <<= sEmptyString;
                            pArr[2].Name = bScriptURL ? sScript : sMacroName;
                            pArr[2].Value <<= aItr->sErrorTitle;

                            // 2) export the sequence
                            rExport.GetEventExport().ExportSingleEvent( aSeq, sOnError);
                        }
                    }
                    break;
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
            ++aItr;
        }
    }
}

const OUString& ScMyValidationsContainer::GetValidationName(const sal_Int32 nIndex)
{
    OSL_ENSURE( static_cast<size_t>(nIndex) < aValidationVec.size(), "out of range" );
    return aValidationVec[nIndex].sName;
}

//==============================================================================

sal_Int32 ScMyDefaultStyles::GetStyleNameIndex(const ScFormatRangeStyles* pCellStyles,
    const sal_Int32 nTable, const sal_Int32 nPos,
    const sal_Int32 i, const bool bRow, bool& bIsAutoStyle)
{
    if (bRow)
        return pCellStyles->GetStyleNameIndex(nTable, nPos, i,
                                bIsAutoStyle);
    else
        return pCellStyles->GetStyleNameIndex(nTable, i, nPos,
                                bIsAutoStyle);
}

void ScMyDefaultStyles::FillDefaultStyles(const sal_Int32 nTable,
    const sal_Int32 nLastRow, const sal_Int32 nLastCol,
    const ScFormatRangeStyles* pCellStyles, ScDocument* pDoc,
    const bool bRow)
{
    if (pDoc)
    {
        SCTAB nTab = static_cast<SCTAB>(nTable);
        sal_Int32 nPos;
        sal_Int32 nLast;
        ScMyDefaultStyleList* pDefaults;
        if (bRow)
        {
            pDefaults = &maRowDefaults;
            nLast = nLastRow;
        }
        else
        {
            pDefaults = &maColDefaults;
            nLast = nLastCol;
        }
        bool bPrevAutoStyle(false);
        bool bIsAutoStyle;
        bool bResult;
        sal_Int32 nPrevIndex(0);
        sal_Int32 nIndex;
        sal_Int32 nRepeat(0);
        sal_Int32 nEmptyRepeat(0);
        for (sal_Int32 i = nLast; i >= 0; --i)
        {
            if (bRow)
            {
                SCCOL nCol;
                bResult = pDoc->GetRowDefault(nTab,
                    static_cast<SCROW>(i), static_cast<SCCOL>(nLastCol), nCol);
                nPos = static_cast<sal_Int32>(nCol);
            }
            else
            {
                SCROW nRow;
                bResult = pDoc->GetColDefault(nTab,
                    static_cast<SCCOL>(i), static_cast<SCROW>(nLastRow), nRow);
                nPos = static_cast<sal_Int32>(nRow);
            }
            if (bResult)
            {
                nEmptyRepeat = 0;
                if (!nRepeat)
                {
                    nPrevIndex = GetStyleNameIndex(pCellStyles, nTab, nPos, i,
                                                bRow, bPrevAutoStyle);
                    (*pDefaults)[i].nIndex = nPrevIndex;
                    (*pDefaults)[i].bIsAutoStyle = bPrevAutoStyle;
                    nRepeat = 1;
                }
                else
                {
                    nIndex = GetStyleNameIndex(pCellStyles, nTab, nPos, i,
                                            bRow, bIsAutoStyle);
                    if ((nIndex != nPrevIndex) || (bIsAutoStyle != bPrevAutoStyle))
                    {
                        nRepeat = 1;
                        nPrevIndex = GetStyleNameIndex(pCellStyles, nTab, nPos, i,
                                                bRow, bPrevAutoStyle);
                        (*pDefaults)[i].nIndex = nPrevIndex;
                        (*pDefaults)[i].bIsAutoStyle = bPrevAutoStyle;
                    }
                    else
                    {
                        (*pDefaults)[i].nIndex = nPrevIndex;
                        (*pDefaults)[i].bIsAutoStyle = bPrevAutoStyle;
                        ++nRepeat;
                        if (nRepeat > 1)
                            (*pDefaults)[i].nRepeat = nRepeat;
                    }
                }
            }
            else
            {
                nRepeat = 0;
                if (!nEmptyRepeat)
                    nEmptyRepeat = 1;
                else
                {
                    ++nEmptyRepeat;
                    if (nEmptyRepeat > 1)
                        (*pDefaults)[i].nRepeat = nEmptyRepeat;
                }
            }
        }
    }
}

void ScMyDefaultStyles::FillDefaultStyles(const sal_Int32 nTable,
    const sal_Int32 nLastRow, const sal_Int32 nLastCol,
    const ScFormatRangeStyles* pCellStyles, ScDocument* pDoc)
{
    maRowDefaults.clear();
    maRowDefaults.resize(nLastRow + 1);
    FillDefaultStyles(nTable, nLastRow, nLastCol, pCellStyles, pDoc, true);
    maColDefaults.clear();
    maColDefaults.resize(nLastCol + 1);
    FillDefaultStyles(nTable, nLastRow, nLastCol, pCellStyles, pDoc, false);
}

ScMyRowFormatRange::ScMyRowFormatRange()
    : nStartColumn(0),
    nRepeatColumns(0),
    nRepeatRows(0),
    nIndex(-1),
    nValidationIndex(-1),
    bIsAutoStyle(true)
{
}

bool ScMyRowFormatRange::operator< (const ScMyRowFormatRange& rRange) const
{
    return (nStartColumn < rRange.nStartColumn);
}

ScRowFormatRanges::ScRowFormatRanges()
    : aRowFormatRanges(),
    pRowDefaults(NULL),
    pColDefaults(NULL),
    nSize(0)
{
}

ScRowFormatRanges::ScRowFormatRanges(const ScRowFormatRanges* pRanges)
    : aRowFormatRanges(pRanges->aRowFormatRanges),
    pRowDefaults(pRanges->pRowDefaults),
    pColDefaults(pRanges->pColDefaults),
    nSize(pRanges->nSize)
{
}

ScRowFormatRanges::~ScRowFormatRanges()
{
}

void ScRowFormatRanges::Clear()
{
    aRowFormatRanges.clear();
    nSize = 0;
}

void ScRowFormatRanges::AddRange(const sal_Int32 nPrevStartCol, const sal_Int32 nRepeat, const sal_Int32 nPrevIndex,
    const bool bPrevAutoStyle, const ScMyRowFormatRange& rFormatRange)
{
    sal_Int32 nIndex(-1);
    if ((nPrevIndex != rFormatRange.nIndex) ||
        (bPrevAutoStyle != rFormatRange.bIsAutoStyle))
        nIndex = rFormatRange.nIndex;

    bool bInserted(false);
    if (!aRowFormatRanges.empty())
    {
        ScMyRowFormatRange* pRange(&aRowFormatRanges.back());
        if (pRange)
        {
            if ((nPrevStartCol == (pRange->nStartColumn + pRange->nRepeatColumns)) &&
                (pRange->bIsAutoStyle == rFormatRange.bIsAutoStyle) &&
                (pRange->nIndex == nIndex) &&
                (pRange->nValidationIndex == rFormatRange.nValidationIndex))
            {
                if (rFormatRange.nRepeatRows < pRange->nRepeatRows)
                    pRange->nRepeatRows = rFormatRange.nRepeatRows;
                pRange->nRepeatColumns += nRepeat;
                bInserted = true;
            }
        }
    }
    if (!bInserted)
    {
        ScMyRowFormatRange aRange;
        aRange.nStartColumn = nPrevStartCol;
        aRange.nRepeatColumns = nRepeat;
        aRange.nRepeatRows = rFormatRange.nRepeatRows;
        aRange.nValidationIndex = rFormatRange.nValidationIndex;
        aRange.bIsAutoStyle = rFormatRange.bIsAutoStyle;
        aRange.nIndex = nIndex;
        aRowFormatRanges.push_back(aRange);
        ++nSize;
    }
}

void ScRowFormatRanges::AddRange(ScMyRowFormatRange& rFormatRange,
    const sal_Int32 nRow)
{
    OSL_ENSURE(pRowDefaults, "no row defaults");
    if (!pRowDefaults)
        return;
    OSL_ENSURE(pColDefaults, "no column defaults");
    if (!pColDefaults)
        return;
    sal_Int32 nPrevIndex;
    bool bPrevAutoStyle;
    OSL_ENSURE( static_cast<size_t>(nRow) < pRowDefaults->size(), "nRow out of bounds");
    if (!(static_cast<size_t>(nRow) < pRowDefaults->size()))
    {
        /* This is only to prevent out-of-bounds accesses, once reached here
         * there's something else going wrong, so FIXME there! */
        if (pRowDefaults->empty())
        {
            nPrevIndex = -1;
            bPrevAutoStyle = false;
        }
        else
        {
            nPrevIndex = (*pRowDefaults)[pRowDefaults->size()-1].nIndex;
            bPrevAutoStyle = (*pRowDefaults)[pRowDefaults->size()-1].bIsAutoStyle;
        }
    }
    else
    {
        nPrevIndex = (*pRowDefaults)[nRow].nIndex;
        bPrevAutoStyle = (*pRowDefaults)[nRow].bIsAutoStyle;
    }
    sal_uInt32 nEnd (rFormatRange.nRepeatRows + nRow - 1);
    sal_uInt32 i(nRow + 1);
    bool bReady(false);
    while ((i < nEnd) && !bReady && (i < pRowDefaults->size()))
    {
        if ((nPrevIndex != (*pRowDefaults)[i].nIndex) ||
            (bPrevAutoStyle != (*pRowDefaults)[i].bIsAutoStyle))
            bReady = true;
        else
            i += (*pRowDefaults)[i].nRepeat;
    }
    if (i > nEnd)
        i = nEnd;
    if (bReady)
        rFormatRange.nRepeatRows = i - nRow + 1;
    if (nPrevIndex == -1)
    {
        sal_uInt32 nPrevStartCol(rFormatRange.nStartColumn);
        OSL_ENSURE( static_cast<size_t>(nPrevStartCol) < pColDefaults->size(), "nPrevStartCol out of bounds");
        sal_uInt32 nRepeat;
        if (static_cast<size_t>(nPrevStartCol) < pColDefaults->size())
        {
            nRepeat = (*pColDefaults)[nPrevStartCol].nRepeat;
            nPrevIndex = (*pColDefaults)[nPrevStartCol].nIndex;
            bPrevAutoStyle = (*pColDefaults)[nPrevStartCol].bIsAutoStyle;
        }
        else
        {
            /* Again, this is to prevent out-of-bounds accesses, so FIXME
             * elsewhere! */
            if (pColDefaults->empty())
            {
                nRepeat = 1;
                nPrevIndex = -1;
                bPrevAutoStyle = false;
            }
            else
            {
                nRepeat = (*pColDefaults)[pColDefaults->size()-1].nRepeat;
                nPrevIndex = (*pColDefaults)[pColDefaults->size()-1].nIndex;
                bPrevAutoStyle = (*pColDefaults)[pColDefaults->size()-1].bIsAutoStyle;
            }
        }
        nEnd = nPrevStartCol + rFormatRange.nRepeatColumns;
        for(i = nPrevStartCol + nRepeat; i < nEnd && i < pColDefaults->size(); i += (*pColDefaults)[i].nRepeat)
        {
            OSL_ENSURE(sal_uInt32(nPrevStartCol + nRepeat) <= nEnd, "something wents wrong");
            if ((nPrevIndex != (*pColDefaults)[i].nIndex) ||
                (bPrevAutoStyle != (*pColDefaults)[i].bIsAutoStyle))
            {
                AddRange(nPrevStartCol, nRepeat, nPrevIndex, bPrevAutoStyle, rFormatRange);
                nPrevStartCol = i;
                nRepeat = (*pColDefaults)[i].nRepeat;
                nPrevIndex = (*pColDefaults)[i].nIndex;
                bPrevAutoStyle = (*pColDefaults)[i].bIsAutoStyle;
            }
            else
                nRepeat += (*pColDefaults)[i].nRepeat;
        }
        if (sal_uInt32(nPrevStartCol + nRepeat) > nEnd)
            nRepeat = nEnd - nPrevStartCol;
        AddRange(nPrevStartCol, nRepeat, nPrevIndex, bPrevAutoStyle, rFormatRange);
    }
    else if ((nPrevIndex == rFormatRange.nIndex) &&
        (bPrevAutoStyle == rFormatRange.bIsAutoStyle))
    {
        rFormatRange.nIndex = -1;
        aRowFormatRanges.push_back(rFormatRange);
        ++nSize;
    }
}

bool ScRowFormatRanges::GetNext(ScMyRowFormatRange& aFormatRange)
{
    ScMyRowFormatRangesList::iterator aItr(aRowFormatRanges.begin());
    if (aItr != aRowFormatRanges.end())
    {
        aFormatRange = (*aItr);
        aRowFormatRanges.erase(aItr);
        --nSize;
        return true;
    }
    return false;
}

sal_Int32 ScRowFormatRanges::GetMaxRows() const
{
    ScMyRowFormatRangesList::const_iterator aItr(aRowFormatRanges.begin());
    ScMyRowFormatRangesList::const_iterator aEndItr(aRowFormatRanges.end());
    sal_Int32 nMaxRows = MAXROW + 1;
    if (aItr != aEndItr)
    {
        while (aItr != aEndItr)
        {
            if ((*aItr).nRepeatRows < nMaxRows)
                nMaxRows = (*aItr).nRepeatRows;
            ++aItr;
        }
    }
    else
    {
        OSL_FAIL("no ranges found");
    }
    return nMaxRows;
}

sal_Int32 ScRowFormatRanges::GetSize() const
{
    return nSize;
}

void ScRowFormatRanges::Sort()
{
    aRowFormatRanges.sort();
}

// ============================================================================
ScMyFormatRange::ScMyFormatRange()
    : nStyleNameIndex(-1),
    nValidationIndex(-1),
    bIsAutoStyle(true)
{
}

bool ScMyFormatRange::operator<(const ScMyFormatRange& rRange) const
{
    if (aRangeAddress.StartRow < rRange.aRangeAddress.StartRow)
        return true;
    else
        if (aRangeAddress.StartRow == rRange.aRangeAddress.StartRow)
            return (aRangeAddress.StartColumn < rRange.aRangeAddress.StartColumn);
        else
            return false;
}

ScFormatRangeStyles::ScFormatRangeStyles()
    : aTables(),
    aStyleNames(),
    aAutoStyleNames(),
    pRowDefaults(0),
    pColDefaults(0)
{
}

ScFormatRangeStyles::~ScFormatRangeStyles()
{
    ScMyOUStringVec::iterator i(aStyleNames.begin());
    ScMyOUStringVec::iterator endi(aStyleNames.end());
    while (i != endi)
    {
        delete *i;
        ++i;
    }
    i = aAutoStyleNames.begin();
    endi = aAutoStyleNames.end();
    while (i != endi)
    {
        delete *i;
        ++i;
    }
    ScMyFormatRangeListVec::iterator j(aTables.begin());
    ScMyFormatRangeListVec::iterator endj(aTables.end());
    while (j != endj)
    {
        delete *j;
        ++j;
    }
}

void ScFormatRangeStyles::AddNewTable(const sal_Int32 nTable)
{
    sal_Int32 nSize = aTables.size() - 1;
    if (nTable > nSize)
        for (sal_Int32 i = nSize; i < nTable; ++i)
        {
            ScMyFormatRangeAddresses* aRangeAddresses(new ScMyFormatRangeAddresses);
            aTables.push_back(aRangeAddresses);
        }
}

bool ScFormatRangeStyles::AddStyleName(OUString* rpString, sal_Int32& rIndex, const bool bIsAutoStyle)
{
    if (bIsAutoStyle)
    {
        aAutoStyleNames.push_back(rpString);
        rIndex = aAutoStyleNames.size() - 1;
        return true;
    }
    else
    {
        sal_Int32 nCount(aStyleNames.size());
        bool bFound(false);
        sal_Int32 i(nCount - 1);
        while ((i >= 0) && (!bFound))
        {
            if (aStyleNames.at(i)->equals(*rpString))
                bFound = true;
            else
                i--;
        }
        if (bFound)
        {
            rIndex = i;
            return false;
        }
        else
        {
            aStyleNames.push_back(rpString);
            rIndex = aStyleNames.size() - 1;
            return true;
        }
    }
}

sal_Int32 ScFormatRangeStyles::GetIndexOfStyleName(const OUString& rString, const OUString& rPrefix, bool& bIsAutoStyle)
{
    sal_Int32 nPrefixLength(rPrefix.getLength());
    OUString sTemp(rString.copy(nPrefixLength));
    sal_Int32 nIndex(sTemp.toInt32());
    if (nIndex > 0 && static_cast<size_t>(nIndex-1) < aAutoStyleNames.size() && aAutoStyleNames.at(nIndex - 1)->equals(rString))
    {
        bIsAutoStyle = true;
        return nIndex - 1;
    }
    else
    {
        sal_Int32 i(0);
        bool bFound(false);
        while (!bFound && static_cast<size_t>(i) < aStyleNames.size())
        {
            if (aStyleNames[i]->equals(rString))
                bFound = true;
            else
                ++i;
        }
        if (bFound)
        {
            bIsAutoStyle = false;
            return i;
        }
        else
        {
            i = 0;
            while (!bFound && static_cast<size_t>(i) < aAutoStyleNames.size())
            {
                if (aAutoStyleNames[i]->equals(rString))
                    bFound = true;
                else
                    ++i;
            }
            if (bFound)
            {
                bIsAutoStyle = true;
                return i;
            }
            else
                return -1;
        }
    }
}

sal_Int32 ScFormatRangeStyles::GetStyleNameIndex(const sal_Int32 nTable,
    const sal_Int32 nColumn, const sal_Int32 nRow, bool& bIsAutoStyle) const
{
    OSL_ENSURE(static_cast<size_t>(nTable) < aTables.size(), "wrong table");
    if (!(static_cast<size_t>(nTable) < aTables.size()))
        return -1;
    ScMyFormatRangeAddresses* pFormatRanges(aTables[nTable]);
    ScMyFormatRangeAddresses::iterator aItr(pFormatRanges->begin());
    ScMyFormatRangeAddresses::iterator aEndItr(pFormatRanges->end());
    while (aItr != aEndItr)
    {
        if (((*aItr).aRangeAddress.StartColumn <= nColumn) &&
            ((*aItr).aRangeAddress.EndColumn >= nColumn) &&
            ((*aItr).aRangeAddress.StartRow <= nRow) &&
            ((*aItr).aRangeAddress.EndRow >= nRow))
        {
            bIsAutoStyle = aItr->bIsAutoStyle;
            return (*aItr).nStyleNameIndex;
        }
        else
            ++aItr;
    }
    return -1;
}

sal_Int32 ScFormatRangeStyles::GetStyleNameIndex(const sal_Int32 nTable, const sal_Int32 nColumn, const sal_Int32 nRow,
    bool& bIsAutoStyle, sal_Int32& nValidationIndex, sal_Int32& nNumberFormat, const sal_Int32 nRemoveBeforeRow)
{
    OSL_ENSURE(static_cast<size_t>(nTable) < aTables.size(), "wrong table");
    if (!(static_cast<size_t>(nTable) < aTables.size()))
        return -1;
    ScMyFormatRangeAddresses* pFormatRanges(aTables[nTable]);
    ScMyFormatRangeAddresses::iterator aItr(pFormatRanges->begin());
    ScMyFormatRangeAddresses::iterator aEndItr(pFormatRanges->end());
    while (aItr != aEndItr)
    {
        if (((*aItr).aRangeAddress.StartColumn <= nColumn) &&
            ((*aItr).aRangeAddress.EndColumn >= nColumn) &&
            ((*aItr).aRangeAddress.StartRow <= nRow) &&
            ((*aItr).aRangeAddress.EndRow >= nRow))
        {
            bIsAutoStyle = aItr->bIsAutoStyle;
            nValidationIndex = aItr->nValidationIndex;
            nNumberFormat = aItr->nNumberFormat;
            /* out-of-bounds is an error elsewhere, so FIXME there! */
            OSL_ENSURE( static_cast<size_t>(nRow) < pRowDefaults->size(), "nRow out of bounds");
            if (static_cast<size_t>(nRow) < pRowDefaults->size() &&
                    ((*pRowDefaults)[nRow].nIndex != -1))
            {
                if (((*pRowDefaults)[nRow].nIndex == (*aItr).nStyleNameIndex) &&
                    ((*pRowDefaults)[nRow].bIsAutoStyle == (*aItr).bIsAutoStyle))
                    return -1;
                else
                    return (*aItr).nStyleNameIndex;
            }
            else
            {
                OSL_ENSURE( static_cast<size_t>(nColumn) < pColDefaults->size(), "nColumn out of bounds");
                if (static_cast<size_t>(nColumn) < pColDefaults->size() &&
                        ((*pColDefaults)[nColumn].nIndex != -1) &&
                        ((*pColDefaults)[nColumn].nIndex == (*aItr).nStyleNameIndex) &&
                        ((*pColDefaults)[nColumn].bIsAutoStyle == (*aItr).bIsAutoStyle))
                    return -1;
                else
                    return (*aItr).nStyleNameIndex;
            }
        }
        else
        {
            if ((*aItr).aRangeAddress.EndRow < nRemoveBeforeRow)
                aItr = pFormatRanges->erase(aItr);
            else
                ++aItr;
        }
    }
    return -1;
}

void ScFormatRangeStyles::GetFormatRanges(const sal_Int32 nStartColumn, const sal_Int32 nEndColumn, const sal_Int32 nRow,
                    const sal_Int32 nTable, ScRowFormatRanges* pRowFormatRanges)
{
    sal_Int32 nTotalColumns(nEndColumn - nStartColumn + 1);
    OSL_ENSURE(static_cast<size_t>(nTable) < aTables.size(), "wrong table");
    ScMyFormatRangeAddresses* pFormatRanges(aTables[nTable]);
    ScMyFormatRangeAddresses::iterator aItr(pFormatRanges->begin());
    ScMyFormatRangeAddresses::iterator aEndItr(pFormatRanges->end());
    sal_Int32 nColumns = 0;
    while (aItr != aEndItr && nColumns < nTotalColumns)
    {
#if OSL_DEBUG_LEVEL > 1
        table::CellRangeAddress aTempRangeAddress((*aItr).aRangeAddress);
#endif
        if (((*aItr).aRangeAddress.StartRow <= nRow) &&
            ((*aItr).aRangeAddress.EndRow >= nRow))
        {
            if ((((*aItr).aRangeAddress.StartColumn <= nStartColumn) &&
                ((*aItr).aRangeAddress.EndColumn >= nStartColumn)) ||
                (((*aItr).aRangeAddress.StartColumn <= nEndColumn) &&
                ((*aItr).aRangeAddress.EndColumn >= nEndColumn)) ||
                (((*aItr).aRangeAddress.StartColumn >= nStartColumn) &&
                ((*aItr).aRangeAddress.EndColumn <= nEndColumn)))
            {
                ScMyRowFormatRange aRange;
                aRange.nIndex = aItr->nStyleNameIndex;
                aRange.nValidationIndex = aItr->nValidationIndex;
                aRange.bIsAutoStyle = aItr->bIsAutoStyle;
                if ((aItr->aRangeAddress.StartColumn < nStartColumn) &&
                    (aItr->aRangeAddress.EndColumn >= nStartColumn))
                {
                    if (aItr->aRangeAddress.EndColumn >= nEndColumn)
                        aRange.nRepeatColumns = nTotalColumns;
                    else
                        aRange.nRepeatColumns = aItr->aRangeAddress.EndColumn - nStartColumn + 1;
                    aRange.nStartColumn = nStartColumn;
                }
                else if ((aItr->aRangeAddress.StartColumn >= nStartColumn) &&
                    (aItr->aRangeAddress.EndColumn <= nEndColumn))
                {
                    aRange.nRepeatColumns = aItr->aRangeAddress.EndColumn - aItr->aRangeAddress.StartColumn + 1;
                    aRange.nStartColumn = aItr->aRangeAddress.StartColumn;
                }
                else if ((aItr->aRangeAddress.StartColumn >= nStartColumn) &&
                    (aItr->aRangeAddress.StartColumn <= nEndColumn) &&
                    (aItr->aRangeAddress.EndColumn > nEndColumn))
                {
                    aRange.nRepeatColumns = nEndColumn - aItr->aRangeAddress.StartColumn + 1;
                    aRange.nStartColumn = aItr->aRangeAddress.StartColumn;
                }
                aRange.nRepeatRows = aItr->aRangeAddress.EndRow - nRow + 1;
                pRowFormatRanges->AddRange(aRange, nRow);
                nColumns += aRange.nRepeatColumns;
            }
            ++aItr;
        }
        else
            if(aItr->aRangeAddress.EndRow < nRow)
                aItr = pFormatRanges->erase(aItr);
            else
                ++aItr;
    }
    pRowFormatRanges->Sort();
}

void ScFormatRangeStyles::AddRangeStyleName(const table::CellRangeAddress aCellRangeAddress,
    const sal_Int32 nStringIndex, const bool bIsAutoStyle, const sal_Int32 nValidationIndex,
    const sal_Int32 nNumberFormat)
{
    ScMyFormatRange aFormatRange;
    aFormatRange.aRangeAddress = aCellRangeAddress;
    aFormatRange.nStyleNameIndex = nStringIndex;
    aFormatRange.nValidationIndex = nValidationIndex;
    aFormatRange.nNumberFormat = nNumberFormat;
    aFormatRange.bIsAutoStyle = bIsAutoStyle;
    OSL_ENSURE(static_cast<size_t>(aCellRangeAddress.Sheet) < aTables.size(), "wrong table");
    ScMyFormatRangeAddresses* pFormatRanges(aTables[aCellRangeAddress.Sheet]);
    pFormatRanges->push_back(aFormatRange);
}

OUString* ScFormatRangeStyles::GetStyleNameByIndex(const sal_Int32 nIndex, const bool bIsAutoStyle)
{
    if (bIsAutoStyle)
        return aAutoStyleNames[nIndex];
    else
        return aStyleNames[nIndex];
}

void ScFormatRangeStyles::Sort()
{
    sal_Int32 nTables = aTables.size();
    for (sal_Int16 i = 0; i < nTables; ++i)
        if (!aTables[i]->empty())
            aTables[i]->sort();
}

//===========================================================================

ScColumnRowStylesBase::ScColumnRowStylesBase()
    : aStyleNames()
{
}

ScColumnRowStylesBase::~ScColumnRowStylesBase()
{
    ScMyOUStringVec::iterator i(aStyleNames.begin());
    ScMyOUStringVec::iterator endi(aStyleNames.end());
    while (i != endi)
    {
        delete *i;
        ++i;
    }
}

sal_Int32 ScColumnRowStylesBase::AddStyleName(OUString* pString)
{
    aStyleNames.push_back(pString);
    return aStyleNames.size() - 1;
}

sal_Int32 ScColumnRowStylesBase::GetIndexOfStyleName(const OUString& rString, const OUString& rPrefix)
{
    sal_Int32 nPrefixLength(rPrefix.getLength());
    OUString sTemp(rString.copy(nPrefixLength));
    sal_Int32 nIndex(sTemp.toInt32());
    if (nIndex > 0 && static_cast<size_t>(nIndex-1) < aStyleNames.size() && aStyleNames.at(nIndex - 1)->equals(rString))
        return nIndex - 1;
    else
    {
        sal_Int32 i(0);
        bool bFound(false);
        while (!bFound && static_cast<size_t>(i) < aStyleNames.size())
        {
            if (aStyleNames.at(i)->equals(rString))
                bFound = true;
            else
                ++i;
        }
        if (bFound)
            return i;
        else
            return -1;
    }
}

OUString* ScColumnRowStylesBase::GetStyleNameByIndex(const sal_Int32 nIndex)
{
    if ( nIndex < 0 || nIndex >= sal::static_int_cast<sal_Int32>( aStyleNames.size() ) )
    {
        // should no longer happen, use first style then
        OSL_FAIL("GetStyleNameByIndex: invalid index");
        return aStyleNames[0];
    }

    return aStyleNames[nIndex];
}

//===========================================================================

ScColumnStyles::ScColumnStyles()
    : ScColumnRowStylesBase(),
    aTables()
{
}

ScColumnStyles::~ScColumnStyles()
{
}

void ScColumnStyles::AddNewTable(const sal_Int32 nTable, const sal_Int32 nFields)
{
    sal_Int32 nSize(aTables.size() - 1);
    if (nTable > nSize)
        for (sal_Int32 i = nSize; i < nTable; ++i)
        {
            ScMyColumnStyleVec aFieldsVec(nFields + 1, ScColumnStyle());
            aTables.push_back(aFieldsVec);
        }
}

sal_Int32 ScColumnStyles::GetStyleNameIndex(const sal_Int32 nTable, const sal_Int32 nField,
    bool& bIsVisible)
{
    OSL_ENSURE(static_cast<size_t>(nTable) < aTables.size(), "wrong table");
    if (static_cast<size_t>(nField) < aTables[nTable].size())
    {
        bIsVisible = aTables[nTable][nField].bIsVisible;
        return aTables[nTable][nField].nIndex;
    }
    else
    {
        bIsVisible = aTables[nTable][aTables[nTable].size() - 1].bIsVisible;
        return aTables[nTable][aTables[nTable].size() - 1].nIndex;
    }
}

void ScColumnStyles::AddFieldStyleName(const sal_Int32 nTable, const sal_Int32 nField,
    const sal_Int32 nStringIndex, const bool bIsVisible)
{
    OSL_ENSURE(static_cast<size_t>(nTable) < aTables.size(), "wrong table");
    OSL_ENSURE(aTables[nTable].size() >= static_cast<sal_uInt32>(nField), "wrong field");
    ScColumnStyle aStyle;
    aStyle.nIndex = nStringIndex;
    aStyle.bIsVisible = bIsVisible;
    if (aTables[nTable].size() == static_cast<sal_uInt32>(nField))
        aTables[nTable].push_back(aStyle);
    aTables[nTable][nField] = aStyle;
}

OUString* ScColumnStyles::GetStyleName(const sal_Int32 nTable, const sal_Int32 nField)
{
    bool bTemp;
    return GetStyleNameByIndex(GetStyleNameIndex(nTable, nField, bTemp));
}

//===========================================================================

ScRowStyles::Cache::Cache() :
    mnTable(-1), mnStart(-1), mnEnd(-1), mnStyle(-1) {}

bool ScRowStyles::Cache::hasCache(sal_Int32 nTable, sal_Int32 nField) const
{
    return mnTable == nTable && mnStart <= nField && nField < mnEnd;
}

ScRowStyles::ScRowStyles()
    : ScColumnRowStylesBase()
{
}

ScRowStyles::~ScRowStyles()
{
}

void ScRowStyles::AddNewTable(const sal_Int32 nTable, const sal_Int32 nFields)
{
    sal_Int32 nSize(aTables.size() - 1);
    if (nTable > nSize)
        for (sal_Int32 i = nSize; i < nTable; ++i)
        {
            aTables.push_back(new StylesType(0, nFields+1, -1));
        }
}

sal_Int32 ScRowStyles::GetStyleNameIndex(const sal_Int32 nTable, const sal_Int32 nField)
{
    OSL_ENSURE(static_cast<size_t>(nTable) < aTables.size(), "wrong table");
    if (!(static_cast<size_t>(nTable) < aTables.size()))
        return -1;

    if (maCache.hasCache(nTable, nField))
        // Cache hit !
        return maCache.mnStyle;

    StylesType& r = aTables[nTable];
    if (!r.is_tree_valid())
        r.build_tree();
    sal_Int32 nStyle(0);
    sal_Int32 nStart(0), nEnd(0);
    if (r.search_tree(nField, nStyle, &nStart, &nEnd).second)
    {
        // Cache this value for better performance.
        maCache.mnTable = nTable;
        maCache.mnStart = nStart;
        maCache.mnEnd = nEnd;
        maCache.mnStyle = nStyle;
        return nStyle;
    }

    return -1;
}

void ScRowStyles::AddFieldStyleName(const sal_Int32 nTable, const sal_Int32 nField,
    const sal_Int32 nStringIndex)
{
    OSL_ENSURE(static_cast<size_t>(nTable) < aTables.size(), "wrong table");
    StylesType& r = aTables[nTable];
    r.insert_back(nField, nField+1, nStringIndex);
}

void ScRowStyles::AddFieldStyleName(const sal_Int32 nTable, const sal_Int32 nStartField,
        const sal_Int32 nStringIndex, const sal_Int32 nEndField)
{
    OSL_ENSURE( nStartField <= nEndField, "bad field range");
    OSL_ENSURE(static_cast<size_t>(nTable) < aTables.size(), "wrong table");
    StylesType& r = aTables[nTable];
    r.insert_back(nStartField, nEndField+1, nStringIndex);
}

OUString* ScRowStyles::GetStyleName(const sal_Int32 nTable, const sal_Int32 nField)
{
    return GetStyleNameByIndex(GetStyleNameIndex(nTable, nField));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
