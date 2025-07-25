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

#include "XMLChangeTrackingExportHelper.hxx"
#include "xmlexprt.hxx"
#include "XMLConverter.hxx"
#include <document.hxx>
#include <chgtrack.hxx>
#include <formulacell.hxx>
#include <textuno.hxx>
#include <rangeutl.hxx>
#include <cellvalue.hxx>
#include <editutil.hxx>

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmluconv.hxx>
#include <sax/tools/converter.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/sharedstring.hxx>
#include <sal/log.hxx>

#include <com/sun/star/util/DateTime.hpp>

using namespace ::com::sun::star;
using namespace xmloff::token;

ScChangeTrackingExportHelper::ScChangeTrackingExportHelper(ScDocument& rDoc, ScXMLExport& rTempExport)
    : m_rDoc(rDoc)
    , rExport(rTempExport)
    , pChangeTrack(nullptr)
{
    pChangeTrack = m_rDoc.GetChangeTrack();
}

ScChangeTrackingExportHelper::~ScChangeTrackingExportHelper()
{
}

OUString ScChangeTrackingExportHelper::GetChangeID(const sal_uInt32 nActionNumber)
{
    return "ct" + OUString::number(nActionNumber);
}

void ScChangeTrackingExportHelper::GetAcceptanceState(const ScChangeAction* pAction)
{
    if (pAction->IsRejected())
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ACCEPTANCE_STATE, XML_REJECTED);
    else if (pAction->IsAccepted())
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ACCEPTANCE_STATE, XML_ACCEPTED);
}

void ScChangeTrackingExportHelper::WriteBigRange(const ScBigRange& rBigRange, XMLTokenEnum aName)
{
    sal_Int64 nStartColumn;
    sal_Int64 nEndColumn;
    sal_Int64 nStartRow;
    sal_Int64 nEndRow;
    sal_Int64 nStartSheet;
    sal_Int64 nEndSheet;
    rBigRange.GetVars(nStartColumn, nStartRow, nStartSheet,
        nEndColumn, nEndRow, nEndSheet);
    if ((nStartColumn == nEndColumn) && (nStartRow == nEndRow) && (nStartSheet == nEndSheet))
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_COLUMN, OUString::number(nStartColumn));
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ROW, OUString::number(nStartRow));
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE, OUString::number(nStartSheet));
    }
    else
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_START_COLUMN, OUString::number(nStartColumn));
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_START_ROW, OUString::number(nStartRow));
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_START_TABLE, OUString::number(nStartSheet));
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_END_COLUMN, OUString::number(nEndColumn));
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_END_ROW, OUString::number(nEndRow));
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_END_TABLE, OUString::number(nEndSheet));
    }
    SvXMLElementExport aBigRangeElem(rExport, XML_NAMESPACE_TABLE, aName, true, true);
}

void ScChangeTrackingExportHelper::WriteChangeInfo(const ScChangeAction* pAction)
{
    bool bRemovePersonalInfo
        = SvtSecurityOptions::IsOptionSet(SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo)
          && !SvtSecurityOptions::IsOptionSet(SvtSecurityOptions::EOption::DocWarnKeepRedlineInfo);

    SvXMLElementExport aElemInfo (rExport, XML_NAMESPACE_OFFICE, XML_CHANGE_INFO, true, true);

    {
        SvXMLElementExport aCreatorElem( rExport, XML_NAMESPACE_DC,
                                            XML_CREATOR, true,
                                            false );
        rExport.Characters(bRemovePersonalInfo
                               ? "Author" + OUString::number(rExport.GetInfoID(pAction->GetUser()))
                               : pAction->GetUser());
    }

    {
        OUStringBuffer sDate;
        ScXMLConverter::ConvertDateTimeToString(bRemovePersonalInfo
                                                    ? DateTime( util::DateTime(0, 0, 0, 12, 1, 1, 1970, true) )
                                                    : pAction->GetDateTimeUTC(),
                                                sDate);
        SvXMLElementExport aDateElem(rExport, XML_NAMESPACE_DC, XML_DATE, true, false);
        rExport.Characters(sDate.makeStringAndClear());
    }

    const OUString& sComment(pAction->GetComment());
    if (!sComment.isEmpty())
    {
        SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TEXT, XML_P, true, false);
        bool bPrevCharWasSpace(true);
        rExport.GetTextParagraphExport()->exportCharacterData(sComment, bPrevCharWasSpace);
    }
}

void ScChangeTrackingExportHelper::WriteGenerated(const ScChangeAction* pGeneratedAction)
{
#if OSL_DEBUG_LEVEL > 0
    sal_uInt32 nActionNumber(pGeneratedAction->GetActionNumber());
    OSL_ENSURE(pChangeTrack->IsGenerated(nActionNumber), "a not generated action found");
#endif
    SvXMLElementExport aElemPrev(rExport, XML_NAMESPACE_TABLE, XML_CELL_CONTENT_DELETION, true, true);
    WriteBigRange(pGeneratedAction->GetBigRange(), XML_CELL_ADDRESS);
    OUString sValue = static_cast<const ScChangeActionContent*>(pGeneratedAction)->GetNewString(m_rDoc);
    WriteCell(static_cast<const ScChangeActionContent*>(pGeneratedAction)->GetNewCell(), sValue);
}

void ScChangeTrackingExportHelper::WriteDeleted(const ScChangeAction* pDeletedAction)
{
    sal_uInt32 nActionNumber(pDeletedAction->GetActionNumber());
    if (pDeletedAction->GetType() == SC_CAT_CONTENT)
    {
        const ScChangeActionContent* pContentAction = static_cast<const ScChangeActionContent*>(pDeletedAction);
        if (pContentAction)
        {
            if (!pChangeTrack->IsGenerated(nActionNumber))
            {
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ID, GetChangeID(nActionNumber));
                SvXMLElementExport aElemPrev(rExport, XML_NAMESPACE_TABLE, XML_CELL_CONTENT_DELETION, true, true);
                if (static_cast<const ScChangeActionContent*>(pDeletedAction)->IsTopContent() && pDeletedAction->IsDeletedIn())
                {
                    OUString sValue = pContentAction->GetNewString(m_rDoc);
                    WriteCell(pContentAction->GetNewCell(), sValue);
                }
            }
            else
                WriteGenerated(pContentAction);
        }
    }
    else
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ID, GetChangeID(nActionNumber));
        SvXMLElementExport aElemPrev(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_DELETION, true, true);
    }
}

void ScChangeTrackingExportHelper::WriteDepending(const ScChangeAction* pDependAction)
{
    sal_uInt32 nActionNumber(pDependAction->GetActionNumber());
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ID, GetChangeID(nActionNumber));

    SvXMLElementExport aDependElem(rExport, XML_NAMESPACE_TABLE,
        XML_DEPENDENCY, true, true);
}

void ScChangeTrackingExportHelper::WriteDependings(const ScChangeAction* pAction)
{
    if (pAction->HasDependent())
    {
        SvXMLElementExport aDependingsElem (rExport, XML_NAMESPACE_TABLE, XML_DEPENDENCIES, true, true);
        const ScChangeActionLinkEntry* pEntry = pAction->GetFirstDependentEntry();
        while (pEntry)
        {
            WriteDepending(pEntry->GetAction());
            pEntry = pEntry->GetNext();
        }
    }
    if (pAction->HasDeleted())
    {
        SvXMLElementExport aDependingsElem (rExport, XML_NAMESPACE_TABLE, XML_DELETIONS, true, true);
        const ScChangeActionLinkEntry* pEntry = pAction->GetFirstDeletedEntry();
        while (pEntry)
        {
            WriteDeleted(pEntry->GetAction());
            pEntry = pEntry->GetNext();
        }
    }
}

void ScChangeTrackingExportHelper::WriteEmptyCell()
{
    SvXMLElementExport aElemEmptyCell(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_TRACK_TABLE_CELL, true, true);
}

void ScChangeTrackingExportHelper::SetValueAttributes(const double& fValue, const OUString& sValue)
{
    bool bSetAttributes(false);
    if (!sValue.isEmpty())
    {
        sal_uInt32 nIndex = 0;
        double fTempValue = 0.0;
        if (m_rDoc.GetFormatTable()->IsNumberFormat(sValue, nIndex, fTempValue))
        {
            SvNumFormatType nType = m_rDoc.GetFormatTable()->GetType(nIndex);
            if (nType & SvNumFormatType::DEFINED)
                nType &= ~SvNumFormatType::DEFINED;
            switch(nType)
            {
                case SvNumFormatType::DATE:
                    {
                        if ( rExport.GetMM100UnitConverter().setNullDate(rExport.GetModel()) )
                        {
                            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_DATE);
                            OUStringBuffer sBuffer;
                            rExport.GetMM100UnitConverter().convertDateTime(sBuffer, fTempValue);
                            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_DATE_VALUE, sBuffer.makeStringAndClear());
                            bSetAttributes = true;
                        }
                    }
                    break;
                case SvNumFormatType::TIME:
                    {
                        rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_TIME);
                        OUStringBuffer sBuffer;
                        ::sax::Converter::convertDuration(sBuffer, fTempValue);
                        rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_TIME_VALUE, sBuffer.makeStringAndClear());
                        bSetAttributes = true;
                    }
                    break;
                default: break;
            }
        }
    }
    if (!bSetAttributes)
    {
        rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT);
        OUStringBuffer sBuffer;
        ::sax::Converter::convertDouble(sBuffer, fValue);
        OUString sNumValue(sBuffer.makeStringAndClear());
        if (!sNumValue.isEmpty())
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE, sNumValue);
    }
}

void ScChangeTrackingExportHelper::WriteValueCell(const ScCellValue& rCell, const OUString& sValue)
{
    assert(rCell.getType() == CELLTYPE_VALUE);

    SetValueAttributes(rCell.getDouble(), sValue);
    SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_TRACK_TABLE_CELL, true, true);
}

void ScChangeTrackingExportHelper::WriteStringCell(const ScCellValue& rCell)
{
    assert(rCell.getType() == CELLTYPE_STRING);

    rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_STRING);
    SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_TRACK_TABLE_CELL, true, true);
    if (!rCell.getSharedString()->isEmpty())
    {
        SvXMLElementExport aElemP(rExport, XML_NAMESPACE_TEXT, XML_P, true, false);
        bool bPrevCharWasSpace(true);
        rExport.GetTextParagraphExport()->exportCharacterData(rCell.getSharedString()->getString(), bPrevCharWasSpace);
    }
}

void ScChangeTrackingExportHelper::WriteEditCell(const ScCellValue& rCell)
{
    assert(rCell.getType() == CELLTYPE_EDIT);

    OUString sString;
    if (rCell.getEditText())
        sString = ScEditUtil::GetString(*rCell.getEditText(), m_rDoc);

    rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_STRING);
    SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_TRACK_TABLE_CELL, true, true);
    if (rCell.getEditText() && !sString.isEmpty())
    {
        if (!pEditTextObj)
            pEditTextObj = new ScEditEngineTextObj(m_rDoc.GetEditEnginePool());
        pEditTextObj->SetText(*rCell.getEditText());
        rExport.GetTextParagraphExport()->exportText(pEditTextObj, false, false);
    }
}

void ScChangeTrackingExportHelper::WriteFormulaCell(const ScCellValue& rCell, const OUString& sValue)
{
    assert(rCell.getType() == CELLTYPE_FORMULA);

    ScFormulaCell* pFormulaCell = rCell.getFormula();
    OUString sAddress;
    ScRangeStringConverter::GetStringFromAddress(sAddress, pFormulaCell->aPos, &m_rDoc, ::formula::FormulaGrammar::CONV_OOO);
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CELL_ADDRESS, sAddress);
    const formula::FormulaGrammar::Grammar eGrammar = m_rDoc.GetStorageGrammar();
    sal_uInt16 nNamespacePrefix = (eGrammar == formula::FormulaGrammar::GRAM_ODFF ? XML_NAMESPACE_OF : XML_NAMESPACE_OOOC);
    OUString sFormula = pFormulaCell->GetFormula(eGrammar);
    ScMatrixMode nMatrixFlag(pFormulaCell->GetMatrixFlag());
    if (nMatrixFlag != ScMatrixMode::NONE)
    {
        if (nMatrixFlag == ScMatrixMode::Formula)
        {
            SCCOL nColumns;
            SCROW nRows;
            pFormulaCell->GetMatColsRows(nColumns, nRows);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_COLUMNS_SPANNED, OUString::number(nColumns));
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_ROWS_SPANNED, OUString::number(nRows));
        }
        else
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MATRIX_COVERED, XML_TRUE);
        }
        OUString sMatrixFormula = sFormula.copy(1, sFormula.getLength() - 2);
        OUString sQValue = rExport.GetNamespaceMap().GetQNameByKey( nNamespacePrefix, sMatrixFormula, false );
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FORMULA, sQValue);
    }
    else
    {
        OUString sQValue = rExport.GetNamespaceMap().GetQNameByKey( nNamespacePrefix, sFormula, false );
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FORMULA, sQValue);
    }
    if (pFormulaCell->IsValue())
    {
        SetValueAttributes(pFormulaCell->GetValue(), sValue);
        SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_TRACK_TABLE_CELL, true, true);
    }
    else
    {
        rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_STRING);
        OUString sCellValue = pFormulaCell->GetString().getString();
        SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_TRACK_TABLE_CELL, true, true);
        if (!sCellValue.isEmpty())
        {
            SvXMLElementExport aElemP(rExport, XML_NAMESPACE_TEXT, XML_P, true, false);
            bool bPrevCharWasSpace(true);
            rExport.GetTextParagraphExport()->exportCharacterData(sCellValue, bPrevCharWasSpace);
        }
    }
}

void ScChangeTrackingExportHelper::WriteCell(const ScCellValue& rCell, const OUString& sValue)
{
    if (rCell.isEmpty())
    {
        WriteEmptyCell();
        return;
    }

    switch (rCell.getType())
    {
        case CELLTYPE_VALUE:
            WriteValueCell(rCell, sValue);
            break;
        case CELLTYPE_STRING:
            WriteStringCell(rCell);
            break;
        case CELLTYPE_EDIT:
            WriteEditCell(rCell);
            break;
        case CELLTYPE_FORMULA:
            WriteFormulaCell(rCell, sValue);
            break;
        default:
            WriteEmptyCell();
    }
}

void ScChangeTrackingExportHelper::WriteContentChange(const ScChangeAction* pAction)
{
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, XML_CELL_CONTENT_CHANGE, true, true);
    const ScChangeAction* pConstAction = pAction;
    WriteBigRange(pConstAction->GetBigRange(), XML_CELL_ADDRESS);
    WriteChangeInfo(pAction);
    WriteDependings(pAction);
    {
        const ScChangeActionContent* pPrevAction = static_cast<const ScChangeActionContent*>(pAction)->GetPrevContent();
        if (pPrevAction)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ID, GetChangeID(pPrevAction->GetActionNumber()));
        SvXMLElementExport aElemPrev(rExport, XML_NAMESPACE_TABLE, XML_PREVIOUS, true, true);
        OUString sValue = static_cast<const ScChangeActionContent*>(pAction)->GetOldString(m_rDoc);
        WriteCell(static_cast<const ScChangeActionContent*>(pAction)->GetOldCell(), sValue);
    }
}

void ScChangeTrackingExportHelper::AddInsertionAttributes(const ScChangeAction* pConstAction)
{
    sal_Int64 nPosition(0);
    sal_Int64 nCount(0);
    sal_Int64 nStartPosition(0);
    sal_Int64 nEndPosition(0);
    sal_Int64 nStartColumn;
    sal_Int64 nEndColumn;
    sal_Int64 nStartRow;
    sal_Int64 nEndRow;
    sal_Int64 nStartSheet;
    sal_Int64 nEndSheet;
    const ScBigRange& rBigRange = pConstAction->GetBigRange();
    rBigRange.GetVars(nStartColumn, nStartRow, nStartSheet,
        nEndColumn, nEndRow, nEndSheet);
    switch (pConstAction->GetType())
    {
        case SC_CAT_INSERT_COLS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TYPE, XML_COLUMN);
            nStartPosition = nStartColumn;
            nEndPosition = nEndColumn;
        }
        break;
        case SC_CAT_INSERT_ROWS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TYPE, XML_ROW);
            nStartPosition = nStartRow;
            nEndPosition = nEndRow;
        }
        break;
        case SC_CAT_INSERT_TABS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TYPE, XML_TABLE);
            nStartPosition = nStartSheet;
            nEndPosition = nEndSheet;
        }
        break;
        default :
        {
            OSL_FAIL("wrong insertion type");
        }
        break;
    }
    nPosition = nStartPosition;
    nCount = nEndPosition - nStartPosition + 1;
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_POSITION, OUString::number(nPosition));
    OSL_ENSURE(nCount > 0, "wrong insertion count");
    if (nCount > 1)
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_COUNT, OUString::number(nCount));
    }
    if (pConstAction->GetType() != SC_CAT_INSERT_TABS)
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE, OUString::number(nStartSheet));
    }
}

void ScChangeTrackingExportHelper::WriteInsertion(const ScChangeAction* pAction)
{
    AddInsertionAttributes(pAction);
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, XML_INSERTION, true, true);
    WriteChangeInfo(pAction);
    WriteDependings(pAction);
}

void ScChangeTrackingExportHelper::AddDeletionAttributes(const ScChangeActionDel* pDelAction)
{
    sal_Int32 nPosition(0);
    const ScBigRange& rBigRange = pDelAction->GetBigRange();
    sal_Int64 nStartColumn(0);
    sal_Int64 nEndColumn(0);
    sal_Int64 nStartRow(0);
    sal_Int64 nEndRow(0);
    sal_Int64 nStartSheet(0);
    sal_Int64 nEndSheet(0);
    rBigRange.GetVars(nStartColumn, nStartRow, nStartSheet,
        nEndColumn, nEndRow, nEndSheet);
    switch (pDelAction->GetType())
    {
        case SC_CAT_DELETE_COLS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TYPE, XML_COLUMN);
            nPosition = nStartColumn;
        }
        break;
        case SC_CAT_DELETE_ROWS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TYPE, XML_ROW);
            nPosition = nStartRow;
        }
        break;
        case SC_CAT_DELETE_TABS :
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TYPE, XML_TABLE);
            nPosition = nStartSheet;
        }
        break;
        default :
        {
            OSL_FAIL("wrong deletion type");
        }
        break;
    }
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_POSITION, OUString::number(nPosition));
    if (pDelAction->GetType() == SC_CAT_DELETE_TABS)
        return;

    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE, OUString::number(nStartSheet));
    if (!pDelAction->IsMultiDelete() || pDelAction->GetDx() || pDelAction->GetDy())
        return;

    const ScChangeAction* p = pDelAction->GetNext();
    sal_Int32 nSlavesCount (1);
    while (p)
    {
        if (p->GetType() != pDelAction->GetType())
            break;
        else
        {
            const ScChangeActionDel* pDel = static_cast<const ScChangeActionDel*>(p);
            if ( (pDel->GetDx() > pDelAction->GetDx() || pDel->GetDy() > pDelAction->GetDy()) &&
                    pDel->GetBigRange() == pDelAction->GetBigRange() )
            {
                ++nSlavesCount;
                p = p->GetNext();
            }
            else
                break;
        }
    }

    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MULTI_DELETION_SPANNED, OUString::number(nSlavesCount));
}

void ScChangeTrackingExportHelper::WriteCutOffs(const ScChangeActionDel* pAction)
{
    const ScChangeActionIns* pCutOffIns = pAction->GetCutOffInsert();
    const ScChangeActionDelMoveEntry* pLinkMove = pAction->GetFirstMoveEntry();
    if (!(pCutOffIns || pLinkMove))
        return;

    SvXMLElementExport aCutOffsElem (rExport, XML_NAMESPACE_TABLE, XML_CUT_OFFS, true, true);
    if (pCutOffIns)
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ID, GetChangeID(pCutOffIns->GetActionNumber()));
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_POSITION, OUString::number(pAction->GetCutOffCount()));
        SvXMLElementExport aInsertCutOffElem (rExport, XML_NAMESPACE_TABLE, XML_INSERTION_CUT_OFF, true, true);
    }
    while (pLinkMove)
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ID, GetChangeID(pLinkMove->GetAction()->GetActionNumber()));
        if (pLinkMove->GetCutOffFrom() == pLinkMove->GetCutOffTo())
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_POSITION, OUString::number(pLinkMove->GetCutOffFrom()));
        }
        else
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_START_POSITION, OUString::number(pLinkMove->GetCutOffFrom()));
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_END_POSITION, OUString::number(pLinkMove->GetCutOffTo()));
        }
        SvXMLElementExport aMoveCutOffElem (rExport, XML_NAMESPACE_TABLE, XML_MOVEMENT_CUT_OFF, true, true);
        pLinkMove = pLinkMove->GetNext();
    }
}

void ScChangeTrackingExportHelper::WriteDeletion(ScChangeAction* pAction)
{
    ScChangeActionDel* pDelAction = static_cast<ScChangeActionDel*> (pAction);
    AddDeletionAttributes(pDelAction);
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, XML_DELETION, true, true);
    WriteChangeInfo(pDelAction);
    WriteDependings(pDelAction);
    WriteCutOffs(pDelAction);
}

void ScChangeTrackingExportHelper::WriteMovement(const ScChangeAction* pAction)
{
    const ScChangeActionMove* pMoveAction = static_cast<const ScChangeActionMove*> (pAction);
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, XML_MOVEMENT, true, true);
    WriteBigRange(pMoveAction->GetFromRange(), XML_SOURCE_RANGE_ADDRESS);
    WriteBigRange(pMoveAction->GetBigRange(), XML_TARGET_RANGE_ADDRESS);
    WriteChangeInfo(pAction);
    WriteDependings(pAction);
}

void ScChangeTrackingExportHelper::WriteRejection(const ScChangeAction* pAction)
{
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, XML_REJECTION, true, true);
    WriteChangeInfo(pAction);
    WriteDependings(pAction);
}

void ScChangeTrackingExportHelper::CollectCellAutoStyles(const ScCellValue& rCell)
{
    if (rCell.getType() != CELLTYPE_EDIT)
        return;

    if (!pEditTextObj)
        pEditTextObj = new ScEditEngineTextObj(m_rDoc.GetEditEnginePool());

    pEditTextObj->SetText(*rCell.getEditText());
    rExport.GetTextParagraphExport()->collectTextAutoStyles(pEditTextObj, false, false);
}

void ScChangeTrackingExportHelper::CollectActionAutoStyles(const ScChangeAction* pAction)
{
    if (pAction->GetType() != SC_CAT_CONTENT)
        return;

    if (pChangeTrack->IsGenerated(pAction->GetActionNumber()))
        CollectCellAutoStyles(static_cast<const ScChangeActionContent*>(pAction)->GetNewCell());
    else
    {
        CollectCellAutoStyles(static_cast<const ScChangeActionContent*>(pAction)->GetOldCell());
        if (static_cast<const ScChangeActionContent*>(pAction)->IsTopContent() && pAction->IsDeletedIn())
            CollectCellAutoStyles(static_cast<const ScChangeActionContent*>(pAction)->GetNewCell());
    }
}

void ScChangeTrackingExportHelper::WorkWithChangeAction(ScChangeAction* pAction)
{
    if (pAction->GetType() == SC_CAT_NONE)
    {
        SAL_WARN("sc.filter", "WorkWithChangeAction: type is not writable");
        return;
    }
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ID, GetChangeID(pAction->GetActionNumber()));
    GetAcceptanceState(pAction);
    if (pAction->IsRejecting())
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_REJECTING_CHANGE_ID, GetChangeID(pAction->GetRejectAction()));
    if (pAction->GetType() == SC_CAT_CONTENT)
        WriteContentChange(pAction);
    else if (pAction->IsInsertType())
        WriteInsertion(pAction);
    else if (pAction->IsDeleteType())
        WriteDeletion(pAction);
    else if (pAction->GetType() == SC_CAT_MOVE)
        WriteMovement(pAction);
    else if (pAction->GetType() == SC_CAT_REJECT)
        WriteRejection(pAction);
    else
    {
        assert(false); // tdf#73335 this would create duplicate attributes
    }
    rExport.CheckAttrList();
}

void ScChangeTrackingExportHelper::CollectAutoStyles()
{
    if (!pChangeTrack)
        return;

    sal_uInt32 nCount (pChangeTrack->GetActionMax());
    if (!nCount)
        return;

    ScChangeAction* pAction = pChangeTrack->GetFirst();
    CollectActionAutoStyles(pAction);
    ScChangeAction* pLastAction = pChangeTrack->GetLast();
    while (pAction != pLastAction)
    {
        pAction = pAction->GetNext();
        CollectActionAutoStyles(pAction);
    }
    pAction = pChangeTrack->GetFirstGenerated();
    while (pAction)
    {
        CollectActionAutoStyles(pAction);
        pAction = pAction->GetNext();
    }
}

void ScChangeTrackingExportHelper::CollectAndWriteChanges()
{
    if (!pChangeTrack)
        return;

    SvXMLElementExport aCangeListElem(rExport, XML_NAMESPACE_TABLE, XML_TRACKED_CHANGES, true, true);
    {
        ScChangeAction* pAction = pChangeTrack->GetFirst();
        if (pAction)
        {
            WorkWithChangeAction(pAction);
            ScChangeAction* pLastAction = pChangeTrack->GetLast();
            while (pAction != pLastAction)
            {
                pAction = pAction->GetNext();
                WorkWithChangeAction(pAction);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
