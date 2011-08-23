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

#ifndef _SC_XMLCHANGETRACKINGEXPORTHELPER_HXX
#include "XMLChangeTrackingExportHelper.hxx"
#endif

#ifndef SC_XMLEXPRT_HXX
#include "xmlexprt.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif
#ifndef SC_CHGTRACK_HXX
#include "chgtrack.hxx"
#endif
#ifndef SC_CHGVISET_HXX
#include "chgviset.hxx"
#endif
#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif
#ifndef SC_TEXTSUNO_HXX
#include "textuno.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <bf_xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <bf_xmloff/xmluconv.hxx>
#endif

#ifndef _ZFORLIST_HXX
#include <bf_svtools/zforlist.hxx>
#endif
namespace binfilter {

#define SC_CHANGE_ID_PREFIX "ct"

using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

ScChangeTrackingExportHelper::ScChangeTrackingExportHelper(ScXMLExport& rTempExport)
    : rExport(rTempExport),
    pChangeTrack(NULL),
    sChangeIDPrefix(RTL_CONSTASCII_USTRINGPARAM(SC_CHANGE_ID_PREFIX)),
    pEditTextObj(NULL),
    pDependings(NULL)
{
    pChangeTrack = rExport.GetDocument() ? rExport.GetDocument()->GetChangeTrack() : NULL;
    pDependings = new ScChangeActionTable();
}

ScChangeTrackingExportHelper::~ScChangeTrackingExportHelper()
{
    if (pDependings)
        delete pDependings;
}

rtl::OUString ScChangeTrackingExportHelper::GetChangeID(const sal_uInt32 nActionNumber)
{
    ::rtl::OUStringBuffer sBuffer(sChangeIDPrefix);
    SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(nActionNumber));
    return sBuffer.makeStringAndClear();
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
    sal_Int32 nStartColumn;
    sal_Int32 nEndColumn;
    sal_Int32 nStartRow;
    sal_Int32 nEndRow;
    sal_Int32 nStartSheet;
    sal_Int32 nEndSheet;
    rBigRange.GetVars(nStartColumn, nStartRow, nStartSheet,
        nEndColumn, nEndRow, nEndSheet);
    if ((nStartColumn == nEndColumn) && (nStartRow == nEndRow) && (nStartSheet == nEndSheet))
    {
        ::rtl::OUStringBuffer sBuffer;
        SvXMLUnitConverter::convertNumber(sBuffer, nStartColumn);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_COLUMN, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nStartRow);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ROW, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nStartSheet);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE, sBuffer.makeStringAndClear());
    }
    else
    {
        ::rtl::OUStringBuffer sBuffer;
        SvXMLUnitConverter::convertNumber(sBuffer, nStartColumn);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_START_COLUMN, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nStartRow);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_START_ROW, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nStartSheet);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_START_TABLE, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nEndColumn);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_END_COLUMN, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nEndRow);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_END_ROW, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nEndSheet);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_END_TABLE, sBuffer.makeStringAndClear());
    }
    SvXMLElementExport aBigRangeElem(rExport, XML_NAMESPACE_TABLE, aName, sal_True, sal_True);
}

void ScChangeTrackingExportHelper::WriteChangeInfo(const ScChangeAction* pAction)
{
    ::rtl::OUString sAuthor(pAction->GetUser());
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_CHG_AUTHOR, sAuthor);
    ::rtl::OUStringBuffer sDate;
    ScXMLConverter::ConvertDateTimeToString(pAction->GetDateTimeUTC(), sDate);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_CHG_DATE_TIME, sDate.makeStringAndClear());
    SvXMLElementExport aElemInfo (rExport, XML_NAMESPACE_OFFICE, XML_CHANGE_INFO, sal_True, sal_True);
    ::rtl::OUString sComment(pAction->GetComment());
    if (sComment.getLength())
    {
        SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TEXT, XML_P, sal_True, sal_False);
        sal_Bool bPrevCharWasSpace(sal_True);
        rExport.GetTextParagraphExport()->exportText(sComment, bPrevCharWasSpace);
    }
}

void ScChangeTrackingExportHelper::WriteGenerated(const ScChangeAction* pGeneratedAction)
{
    sal_uInt32 nActionNumber(pGeneratedAction->GetActionNumber());
    DBG_ASSERT(pChangeTrack->IsGenerated(nActionNumber), "a not generated action found");
    SvXMLElementExport aElemPrev(rExport, XML_NAMESPACE_TABLE, XML_CELL_CONTENT_DELETION, sal_True, sal_True);
    WriteBigRange(pGeneratedAction->GetBigRange(), XML_CELL_ADDRESS);
    String sValue;
    static_cast<const ScChangeActionContent*>(pGeneratedAction)->GetNewString(sValue);
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
                SvXMLElementExport aElemPrev(rExport, XML_NAMESPACE_TABLE, XML_CELL_CONTENT_DELETION, sal_True, sal_True);
                if (static_cast<const ScChangeActionContent*>(pDeletedAction)->IsTopContent() && pDeletedAction->IsDeletedIn())
                {
                    String sValue;
                    pContentAction->GetNewString(sValue);
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
        SvXMLElementExport aElemPrev(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_DELETION, sal_True, sal_True);
    }
}

void ScChangeTrackingExportHelper::WriteDepending(const ScChangeAction* pDependAction)
{
    sal_uInt32 nActionNumber(pDependAction->GetActionNumber());
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ID, GetChangeID(nActionNumber));
    SvXMLElementExport aDependElem(rExport, XML_NAMESPACE_TABLE, XML_DEPENDENCE, sal_True, sal_True);
}

void ScChangeTrackingExportHelper::WriteDependings(ScChangeAction* pAction)
{
    if (pAction->HasDependent())
    {
        SvXMLElementExport aDependingsElem (rExport, XML_NAMESPACE_TABLE, XML_DEPENDENCES, sal_True, sal_True);
        const ScChangeActionLinkEntry* pEntry = pAction->GetFirstDependentEntry();
        while (pEntry)
        {
            WriteDepending(pEntry->GetAction());
            pEntry = pEntry->GetNext();
        }
    }
    if (pAction->HasDeleted())
    {
        SvXMLElementExport aDependingsElem (rExport, XML_NAMESPACE_TABLE, XML_DELETIONS, sal_True, sal_True);
        const ScChangeActionLinkEntry* pEntry = pAction->GetFirstDeletedEntry();
        while (pEntry)
        {
            WriteDeleted(pEntry->GetAction());
            pEntry = pEntry->GetNext();
        }
        /*if (pAction->IsDeleteType())
        {
            ScChangeActionDel* pDelAction = static_cast<ScChangeActionDel*> (pAction);
            if (pDelAction)
            {
                const ScChangeActionCellListEntry* pCellEntry = pDelAction->GetFirstCellEntry();
                while (pCellEntry)
                {
                    WriteGenerated(pCellEntry->GetContent());
                    pCellEntry = pCellEntry->GetNext();
                }
            }
        }
        else if (pAction->GetType() == SC_CAT_MOVE)
        {
            ScChangeActionMove* pMoveAction = static_cast<ScChangeActionMove*> (pAction);
            if (pMoveAction)
            {
                const ScChangeActionCellListEntry* pCellEntry = pMoveAction->GetFirstCellEntry();
                while (pCellEntry)
                {
                    WriteGenerated(pCellEntry->GetContent());
                    pCellEntry = pCellEntry->GetNext();
                }
            }
        }*/
    }
}

/*void ScChangeTrackingExportHelper::WriteDependings(ScChangeAction* pAction)
{
    pChangeTrack->GetDependents(pAction, *pDependings);
    if (pDependings->Count())
    {
        SvXMLElementExport aDependingsElem (rExport, XML_NAMESPACE_TABLE, XML_DEPENDENCES, sal_True, sal_True);
        ScChangeAction* pDependAction = pDependings->First();
        while (pDependAction != NULL)
        {
            WriteDepending(pDependAction);
            pDependAction = pDependings->Next();
        }
    }
}*/

void ScChangeTrackingExportHelper::WriteEmptyCell()
{
    SvXMLElementExport aElemEmptyCell(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_TRACK_TABLE_CELL, sal_True, sal_True);
}

void ScChangeTrackingExportHelper::SetValueAttributes(const double& fValue, const String& sValue)
{
    sal_Bool bSetAttributes(sal_False);
    if (sValue.Len())
    {
        sal_uInt32 nIndex;
        double fTempValue;
        if (rExport.GetDocument() && rExport.GetDocument()->GetFormatTable()->IsNumberFormat(sValue, nIndex, fTempValue))
        {
            sal_uInt16 nType = rExport.GetDocument()->GetFormatTable()->GetType(nIndex);
            switch(nType & NUMBERFORMAT_DEFINED)
            {
                case NUMBERFORMAT_DATE:
                    {
                        if ( rExport.GetMM100UnitConverter().setNullDate(rExport.GetModel()) )
                        {
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE_TYPE, XML_DATE);
                            ::rtl::OUStringBuffer sBuffer;
                            rExport.GetMM100UnitConverter().convertDateTime(sBuffer, fTempValue);
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DATE_VALUE, sBuffer.makeStringAndClear());
                            bSetAttributes = sal_True;
                        }
                    }
                    break;
                case NUMBERFORMAT_TIME:
                    {
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE_TYPE, XML_TIME);
                        ::rtl::OUStringBuffer sBuffer;
                        rExport.GetMM100UnitConverter().convertTime(sBuffer, fTempValue);
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TIME_VALUE, sBuffer.makeStringAndClear());
                        bSetAttributes = sal_True;
                    }
                    break;
            }
        }
    }
    if (!bSetAttributes)
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE_TYPE, XML_FLOAT);
        ::rtl::OUStringBuffer sBuffer;
        SvXMLUnitConverter::convertDouble(sBuffer, fValue);
        ::rtl::OUString sValue(sBuffer.makeStringAndClear());
        if (sValue.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, sValue);
    }
}


void ScChangeTrackingExportHelper::WriteValueCell(const ScBaseCell* pCell, const String& sValue)
{
    const ScValueCell* pValueCell = static_cast<const ScValueCell*>(pCell);
    if (pValueCell)
    {
        SetValueAttributes(pValueCell->GetValue(), sValue);
        SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_TRACK_TABLE_CELL, sal_True, sal_True);
        /*if (sText.getLength())
        {
            SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TEXT, XML_P, sal_True, sal_False);
            sal_Bool bPrevCharWasSpace(sal_True);
            rExport.GetTextParagraphExport()->exportText(sText, bPrevCharWasSpace);
        }*/
    }
}

void ScChangeTrackingExportHelper::WriteStringCell(const ScBaseCell* pCell)
{
    const ScStringCell* pStringCell = static_cast<const ScStringCell*>(pCell);
    if (pStringCell)
    {
        String sString;
        pStringCell->GetString(sString);
        ::rtl::OUString sOUString(sString);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE_TYPE, XML_STRING);
        SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_TRACK_TABLE_CELL, sal_True, sal_True);
        if (sOUString.getLength())
        {
            SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TEXT, XML_P, sal_True, sal_False);
            sal_Bool bPrevCharWasSpace(sal_True);
            rExport.GetTextParagraphExport()->exportText(sOUString, bPrevCharWasSpace);
        }
    }
}

void ScChangeTrackingExportHelper::WriteEditCell(const ScBaseCell* pCell)
{
    const ScEditCell* pEditCell = static_cast<const ScEditCell*>(pCell);
    if (pEditCell)
    {
        String sString;
        pEditCell->GetString(sString);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE_TYPE, XML_STRING);
        SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_TRACK_TABLE_CELL, sal_True, sal_True);
        if (sString.Len())
        {
            if (!pEditTextObj)
            {
                pEditTextObj = new ScEditEngineTextObj();
                xText = pEditTextObj;
            }
            pEditTextObj->SetText(*(pEditCell->GetData()));
            if (xText.is())
                rExport.GetTextParagraphExport()->exportText(xText, sal_False, sal_False);
        }
    }
}

void ScChangeTrackingExportHelper::WriteFormulaCell(const ScBaseCell* pCell, const String& sValue)
{
    ScBaseCell* pBaseCell = const_cast<ScBaseCell*>(pCell);
    ScFormulaCell* pFormulaCell = static_cast<ScFormulaCell*>(pBaseCell);
    if (pFormulaCell)
    {
        ::rtl::OUString sAddress;
        ScXMLConverter::GetStringFromAddress(sAddress, pFormulaCell->aPos, rExport.GetDocument());
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CELL_ADDRESS, sAddress);
        String sFormula;
        pFormulaCell->GetEnglishFormula(sFormula, sal_True);
        ::rtl::OUString sOUFormula(sFormula);
        sal_uInt8 nMatrixFlag(pFormulaCell->GetMatrixFlag());
        if (nMatrixFlag)
        {
            if (nMatrixFlag == MM_FORMULA)
            {
                sal_uInt16 nColumns, nRows;
                pFormulaCell->GetMatColsRows(nColumns, nRows);
                ::rtl::OUStringBuffer sColumns;
                ::rtl::OUStringBuffer sRows;
                SvXMLUnitConverter::convertNumber(sColumns, static_cast<sal_Int32>(nColumns));
                SvXMLUnitConverter::convertNumber(sRows, static_cast<sal_Int32>(nRows));
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_COLUMNS_SPANNED, sColumns.makeStringAndClear());
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_MATRIX_ROWS_SPANNED, sRows.makeStringAndClear());
            }
            else
            {
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MATRIX_COVERED, XML_TRUE);
            }
            ::rtl::OUString sMatrixFormula = sOUFormula.copy(1, sOUFormula.getLength() - 2);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FORMULA, sMatrixFormula);
        }
        else
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_FORMULA, sFormula);
        if (pFormulaCell->IsValue())
        {
            SetValueAttributes(pFormulaCell->GetValue(), sValue);
            SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_TRACK_TABLE_CELL, sal_True, sal_True);
        }
        else
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE_TYPE, XML_STRING);
            String sValue;
            pFormulaCell->GetString(sValue);
            ::rtl::OUString sOUValue(sValue);
            SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, XML_CHANGE_TRACK_TABLE_CELL, sal_True, sal_True);
            if (sOUValue.getLength())
            {
                SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TEXT, XML_P, sal_True, sal_False);
                sal_Bool bPrevCharWasSpace(sal_True);
                rExport.GetTextParagraphExport()->exportText(sOUValue, bPrevCharWasSpace);
            }
        }
    }
}

void ScChangeTrackingExportHelper::WriteCell(const ScBaseCell* pCell, const String& sValue)
{
    if (pCell)
    {
        switch (pCell->GetCellType())
        {
            case CELLTYPE_NONE:
                WriteEmptyCell();
                break;
            case CELLTYPE_VALUE:
                WriteValueCell(pCell, sValue);
                break;
            case CELLTYPE_STRING:
                WriteStringCell(pCell);
                break;
            case CELLTYPE_EDIT:
                WriteEditCell(pCell);
                break;
            case CELLTYPE_FORMULA:
                WriteFormulaCell(pCell, sValue);
                break;
        }
    }
    else
        WriteEmptyCell();
}

void ScChangeTrackingExportHelper::WriteContentChange(ScChangeAction* pAction)
{
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, XML_CELL_CONTENT_CHANGE, sal_True, sal_True);
    const ScChangeAction* pConstAction = pAction;
    WriteBigRange(pConstAction->GetBigRange(), XML_CELL_ADDRESS);
    WriteChangeInfo(pAction);
    WriteDependings(pAction);
    {
        ScChangeActionContent* pPrevAction = static_cast<ScChangeActionContent*>(pAction)->GetPrevContent();
        if (pPrevAction)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ID, GetChangeID(pPrevAction->GetActionNumber()));
        SvXMLElementExport aElemPrev(rExport, XML_NAMESPACE_TABLE, XML_PREVIOUS, sal_True, sal_True);
        String sValue;
        static_cast<ScChangeActionContent*>(pAction)->GetOldString(sValue);
        WriteCell(static_cast<ScChangeActionContent*>(pAction)->GetOldCell(), sValue);
    }
}

void ScChangeTrackingExportHelper::AddInsertionAttributes(const ScChangeAction* pConstAction)
{
    sal_Int32 nPosition(0);
    sal_Int32 nCount(0);
    sal_Int32 nStartPosition(0);
    sal_Int32 nEndPosition(0);
    sal_Int32 nStartColumn;
    sal_Int32 nEndColumn;
    sal_Int32 nStartRow;
    sal_Int32 nEndRow;
    sal_Int32 nStartSheet;
    sal_Int32 nEndSheet;
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
            DBG_ERROR("wrong insertion type");
        }
        break;
    }
    nPosition = nStartPosition;
    nCount = nEndPosition - nStartPosition + 1;
    ::rtl::OUStringBuffer sBuffer;
    SvXMLUnitConverter::convertNumber(sBuffer, nPosition);
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_POSITION, sBuffer.makeStringAndClear());
    DBG_ASSERT(nCount > 0, "wrong insertion count");
    if (nCount > 1)
    {
        SvXMLUnitConverter::convertNumber(sBuffer, nCount);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_COUNT, sBuffer.makeStringAndClear());
    }
    if (pConstAction->GetType() != SC_CAT_INSERT_TABS)
    {
        SvXMLUnitConverter::convertNumber(sBuffer, nStartSheet);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE, sBuffer.makeStringAndClear());
    }
}

void ScChangeTrackingExportHelper::WriteInsertion(ScChangeAction* pAction)
{
    AddInsertionAttributes(pAction);
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, XML_INSERTION, sal_True, sal_True);
    WriteChangeInfo(pAction);
    WriteDependings(pAction);
}

void ScChangeTrackingExportHelper::AddDeletionAttributes(const ScChangeActionDel* pDelAction, const ScChangeActionDel* pLastAction)
{
    sal_Int32 nPosition(0);
    const ScBigRange& rBigRange = pDelAction->GetBigRange();
    sal_Int32 nStartColumn(0);
    sal_Int32 nEndColumn(0);
    sal_Int32 nStartRow(0);
    sal_Int32 nEndRow(0);
    sal_Int32 nStartSheet(0);
    sal_Int32 nEndSheet(0);
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
            //DBG_ERROR("not implemented feature");
        }
        break;
        default :
        {
            DBG_ERROR("wrong deletion type");
        }
        break;
    }
    ::rtl::OUStringBuffer sBuffer;
    SvXMLUnitConverter::convertNumber(sBuffer, nPosition);
    rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_POSITION, sBuffer.makeStringAndClear());
    if (pDelAction->GetType() != SC_CAT_DELETE_TABS)
    {
        SvXMLUnitConverter::convertNumber(sBuffer, nStartSheet);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_TABLE, sBuffer.makeStringAndClear());
        if (pDelAction->IsMultiDelete() && !pDelAction->GetDx() && !pDelAction->GetDy())
        {
            const ScChangeAction* p = pDelAction->GetNext();
            sal_Bool bAll(sal_False);
            sal_Int32 nSlavesCount (1);
            while (!bAll && p)
            {
                if ( !p || p->GetType() != pDelAction->GetType() )
                    bAll = sal_True;
                else
                {
                    const ScChangeActionDel* pDel = (const ScChangeActionDel*) p;
                    if ( (pDel->GetDx() > pDelAction->GetDx() || pDel->GetDy() > pDelAction->GetDy()) &&
                            pDel->GetBigRange() == pDelAction->GetBigRange() )
                    {
                        nSlavesCount++;
                        p = p->GetNext();
                    }
                    else
                        bAll = sal_True;
                }
            }

            SvXMLUnitConverter::convertNumber(sBuffer, nSlavesCount);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_MULTI_DELETION_SPANNED, sBuffer.makeStringAndClear());
        }
    }
}

void ScChangeTrackingExportHelper::WriteCutOffs(const ScChangeActionDel* pAction)
{
    const ScChangeActionIns* pCutOffIns = pAction->GetCutOffInsert();
    const ScChangeActionDelMoveEntry* pLinkMove = pAction->GetFirstMoveEntry();
    if (pCutOffIns || pLinkMove)
    {
        SvXMLElementExport aCutOffsElem (rExport, XML_NAMESPACE_TABLE, XML_CUT_OFFS, sal_True, sal_True);
        ::rtl::OUStringBuffer sBuffer;
        if (pCutOffIns)
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ID, GetChangeID(pCutOffIns->GetActionNumber()));
            SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(pAction->GetCutOffCount()));
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_POSITION, sBuffer.makeStringAndClear());
            SvXMLElementExport aInsertCutOffElem (rExport, XML_NAMESPACE_TABLE, XML_INSERTION_CUT_OFF, sal_True, sal_True);
        }
        while (pLinkMove)
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_ID, GetChangeID(pLinkMove->GetAction()->GetActionNumber()));
            if (pLinkMove->GetCutOffFrom() == pLinkMove->GetCutOffTo())
            {
                SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(pLinkMove->GetCutOffFrom()));
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_POSITION, sBuffer.makeStringAndClear());
            }
            else
            {
                SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(pLinkMove->GetCutOffFrom()));
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_START_POSITION, sBuffer.makeStringAndClear());
                SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(pLinkMove->GetCutOffTo()));
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_END_POSITION, sBuffer.makeStringAndClear());
            }
            SvXMLElementExport aMoveCutOffElem (rExport, XML_NAMESPACE_TABLE, XML_MOVEMENT_CUT_OFF, sal_True, sal_True);
            pLinkMove = pLinkMove->GetNext();
        }
    }
}

void ScChangeTrackingExportHelper::WriteDeletion(ScChangeAction* pAction)
{
    ScChangeActionDel* pDelAction = static_cast<ScChangeActionDel*> (pAction);
    AddDeletionAttributes(pDelAction, pDelAction);
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, XML_DELETION, sal_True, sal_True);
    WriteChangeInfo(pDelAction);
    WriteDependings(pDelAction);
    WriteCutOffs(pDelAction);
}

void ScChangeTrackingExportHelper::WriteMovement(ScChangeAction* pAction)
{
    const ScChangeActionMove* pMoveAction = static_cast<ScChangeActionMove*> (pAction);
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, XML_MOVEMENT, sal_True, sal_True);
    WriteBigRange(pMoveAction->GetFromRange(), XML_SOURCE_RANGE_ADDRESS);
    WriteBigRange(pMoveAction->GetBigRange(), XML_TARGET_RANGE_ADDRESS);
    WriteChangeInfo(pAction);
    WriteDependings(pAction);
}

void ScChangeTrackingExportHelper::WriteRejection(ScChangeAction* pAction)
{
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, XML_REJECTION, sal_True, sal_True);
    WriteChangeInfo(pAction);
    WriteDependings(pAction);
}

void ScChangeTrackingExportHelper::CollectCellAutoStyles(const ScBaseCell* pBaseCell)
{
    if (pBaseCell && (pBaseCell->GetCellType() == CELLTYPE_EDIT))
    {
        const ScEditCell* pEditCell = static_cast<const ScEditCell*>(pBaseCell);
        if (pEditCell)
        {
            if (!pEditTextObj)
            {
                pEditTextObj = new ScEditEngineTextObj();
                xText = pEditTextObj;
            }
            pEditTextObj->SetText(*(pEditCell->GetData()));
            if (xText.is())
                rExport.GetTextParagraphExport()->collectTextAutoStyles(xText, sal_False, sal_False);
        }
    }
}

void ScChangeTrackingExportHelper::CollectActionAutoStyles(ScChangeAction* pAction)
{
    if (pAction->GetType() == SC_CAT_CONTENT)
    {
        if (pChangeTrack->IsGenerated(pAction->GetActionNumber()))
             CollectCellAutoStyles(static_cast<ScChangeActionContent*>(pAction)->GetNewCell());
        else
        {
             CollectCellAutoStyles(static_cast<ScChangeActionContent*>(pAction)->GetOldCell());
            if (static_cast<ScChangeActionContent*>(pAction)->IsTopContent() && pAction->IsDeletedIn())
                 CollectCellAutoStyles(static_cast<ScChangeActionContent*>(pAction)->GetNewCell());
        }
    }
}

void ScChangeTrackingExportHelper::WorkWithChangeAction(ScChangeAction* pAction)
{
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
        DBG_ERROR("not a writeable type");
    rExport.CheckAttrList();
}

void ScChangeTrackingExportHelper::CollectAutoStyles()
{
    if (pChangeTrack)
    {
        sal_uInt32 nCount (pChangeTrack->GetActionMax());
        if (nCount)
        {
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
    }
}

void ScChangeTrackingExportHelper::CollectAndWriteChanges()
{
    if (pChangeTrack)
    {
        if (pChangeTrack->IsProtected())
        {
            ::rtl::OUStringBuffer aBuffer;
            SvXMLUnitConverter::encodeBase64(aBuffer, pChangeTrack->GetProtection());
            if (aBuffer.getLength())
                rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_PROTECTION_KEY, aBuffer.makeStringAndClear());
        }
        SvXMLElementExport aCangeListElem(rExport, XML_NAMESPACE_TABLE, XML_TRACKED_CHANGES, sal_True, sal_True);
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
}
}
