/*************************************************************************
 *
 *  $RCSfile: XMLChangeTrackingExportHelper.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-01 13:58:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
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

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif

#define SC_CHANGE_ID_PREFIX "ct"

using namespace ::com::sun::star;

ScChangeTrackingExportHelper::ScChangeTrackingExportHelper(ScXMLExport& rTempExport)
    : rExport(rTempExport),
    pChangeTrack(NULL),
    sChangeIDPrefix(RTL_CONSTASCII_USTRINGPARAM(SC_CHANGE_ID_PREFIX)),
    sAccepted(RTL_CONSTASCII_USTRINGPARAM(sXML_accepted)),
    sRejected(RTL_CONSTASCII_USTRINGPARAM(sXML_rejected)),
    sPending(RTL_CONSTASCII_USTRINGPARAM(sXML_pending)),
    pEditTextObj(NULL),
    pDependings(NULL)
{
    pChangeTrack = rExport.GetDocument()->GetChangeTrack();
    pDependings = new ScChangeActionTable();
}

ScChangeTrackingExportHelper::~ScChangeTrackingExportHelper()
{
    if (pDependings)
        delete pDependings;
}

rtl::OUString ScChangeTrackingExportHelper::GetChangeID(const sal_uInt32 nActionNumber)
{
    rtl::OUStringBuffer sBuffer(sChangeIDPrefix);
    SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(nActionNumber));
    return sBuffer.makeStringAndClear();
}

void ScChangeTrackingExportHelper::GetAcceptanceState(const ScChangeAction* pAction)
{
    if (pAction->IsRejected())
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_acceptance_state, sRejected);
    else if (pAction->IsAccepted())
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_acceptance_state, sAccepted);
}

void ScChangeTrackingExportHelper::WriteBigRange(const ScBigRange& rBigRange, const sal_Char *pName)
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
        rtl::OUStringBuffer sBuffer;
        SvXMLUnitConverter::convertNumber(sBuffer, nStartColumn);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_column, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nStartRow);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_row, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nStartSheet);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_table, sBuffer.makeStringAndClear());
    }
    else
    {
        rtl::OUStringBuffer sBuffer;
        SvXMLUnitConverter::convertNumber(sBuffer, nStartColumn);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_start_column, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nStartRow);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_start_row, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nStartSheet);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_start_table, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nEndColumn);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_end_column, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nEndRow);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_end_row, sBuffer.makeStringAndClear());
        SvXMLUnitConverter::convertNumber(sBuffer, nEndSheet);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_end_table, sBuffer.makeStringAndClear());
    }
    SvXMLElementExport aBigRangeElem(rExport, XML_NAMESPACE_TABLE, pName, sal_True, sal_True);
}

void ScChangeTrackingExportHelper::WriteChangeInfo(const ScChangeAction* pAction)
{
    rtl::OUString sAuthor(pAction->GetUser());
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_chg_author, sAuthor);
    rtl::OUStringBuffer sDate;
    ScXMLConverter::ConvertDateTimeToString(pAction->GetDateTimeUTC(), sDate);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_chg_date_time, sDate.makeStringAndClear());
    SvXMLElementExport aElemInfo (rExport, XML_NAMESPACE_OFFICE, sXML_change_info, sal_True, sal_True);
    rtl::OUString sComment(pAction->GetComment());
    if (sComment.getLength())
    {
        SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TEXT, sXML_p, sal_True, sal_False);
        sal_Bool bPrevCharWasSpace(sal_True);
        rExport.GetTextParagraphExport()->exportText(sComment, bPrevCharWasSpace);
    }
}

void ScChangeTrackingExportHelper::WriteGenerated(const ScChangeAction* pGeneratedAction)
{
    sal_uInt32 nActionNumber(pGeneratedAction->GetActionNumber());
    DBG_ASSERT(pChangeTrack->IsGenerated(nActionNumber), "a not generated action found");
    SvXMLElementExport aElemPrev(rExport, XML_NAMESPACE_TABLE, sXML_cell_content_deletion, sal_True, sal_True);
    WriteBigRange(pGeneratedAction->GetBigRange(), sXML_cell_address);
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
                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_id, GetChangeID(nActionNumber));
                SvXMLElementExport aElemPrev(rExport, XML_NAMESPACE_TABLE, sXML_cell_content_deletion, sal_True, sal_True);
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
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_id, GetChangeID(nActionNumber));
        SvXMLElementExport aElemPrev(rExport, XML_NAMESPACE_TABLE, sXML_change_deletion, sal_True, sal_True);
    }
}

void ScChangeTrackingExportHelper::WriteDepending(const ScChangeAction* pDependAction)
{
    sal_uInt32 nActionNumber(pDependAction->GetActionNumber());
    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_id, GetChangeID(nActionNumber));
    SvXMLElementExport aDependElem(rExport, XML_NAMESPACE_TABLE, sXML_dependence, sal_True, sal_True);
}

void ScChangeTrackingExportHelper::WriteDependings(ScChangeAction* pAction)
{
    if (pAction->HasDependent())
    {
        SvXMLElementExport aDependingsElem (rExport, XML_NAMESPACE_TABLE, sXML_dependences, sal_True, sal_True);
        const ScChangeActionLinkEntry* pEntry = pAction->GetFirstDependentEntry();
        while (pEntry)
        {
            WriteDepending(pEntry->GetAction());
            pEntry = pEntry->GetNext();
        }
    }
    if (pAction->HasDeleted())
    {
        SvXMLElementExport aDependingsElem (rExport, XML_NAMESPACE_TABLE, sXML_deletions, sal_True, sal_True);
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
        SvXMLElementExport aDependingsElem (rExport, XML_NAMESPACE_TABLE, sXML_dependences, sal_True, sal_True);
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
    SvXMLElementExport aElemEmptyCell(rExport, XML_NAMESPACE_TABLE, sXML_change_track_table_cell, sal_True, sal_True);
}

void ScChangeTrackingExportHelper::SetValueAttributes(const double& fValue, const String& sValue)
{
    sal_Bool bSetAttributes(sal_False);
    if (sValue.Len())
    {
        sal_uInt32 nIndex;
        double fTempValue;
        if (rExport.GetDocument()->GetFormatTable()->IsNumberFormat(sValue, nIndex, fTempValue))
        {
            sal_uInt16 nType = rExport.GetDocument()->GetFormatTable()->GetType(nIndex);
            switch(nType & NUMBERFORMAT_DEFINED)
            {
                case NUMBERFORMAT_DATE:
                    {
                        if ( rExport.GetMM100UnitConverter().setNullDate(rExport.GetModel()) )
                        {
                            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_value_type, sXML_date);
                            rtl::OUStringBuffer sBuffer;
                            rExport.GetMM100UnitConverter().convertDateTime(sBuffer, fTempValue);
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_date_value, sBuffer.makeStringAndClear());
                            bSetAttributes = sal_True;
                        }
                    }
                    break;
                case NUMBERFORMAT_TIME:
                    {
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_value_type, sXML_time);
                        rtl::OUStringBuffer sBuffer;
                        rExport.GetMM100UnitConverter().convertTime(sBuffer, fTempValue);
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_time_value, sBuffer.makeStringAndClear());
                        bSetAttributes = sal_True;
                    }
                    break;
            }
        }
    }
    if (!bSetAttributes)
    {
        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_value_type, sXML_float);
        rtl::OUStringBuffer sBuffer;
        SvXMLUnitConverter::convertDouble(sBuffer, fValue);
        rtl::OUString sValue(sBuffer.makeStringAndClear());
        if (sValue.getLength())
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_value, sValue);
    }
}


void ScChangeTrackingExportHelper::WriteValueCell(const ScBaseCell* pCell, const String& sValue)
{
    const ScValueCell* pValueCell = static_cast<const ScValueCell*>(pCell);
    if (pValueCell)
    {
        SetValueAttributes(pValueCell->GetValue(), sValue);
        SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, sXML_change_track_table_cell, sal_True, sal_True);
        /*if (sText.getLength())
        {
            SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TEXT, sXML_p, sal_True, sal_False);
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
        rtl::OUString sOUString(sString);
        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_value_type, sXML_string);
        SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, sXML_change_track_table_cell, sal_True, sal_True);
        if (sOUString.getLength())
        {
            SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TEXT, sXML_p, sal_True, sal_False);
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
        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_value_type, sXML_string);
        SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, sXML_change_track_table_cell, sal_True, sal_True);
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
        rtl::OUString sAddress;
        ScXMLConverter::GetStringFromAddress(sAddress, pFormulaCell->aPos, rExport.GetDocument());
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_cell_address, sAddress);
        String sFormula;
        pFormulaCell->GetEnglishFormula(sFormula, sal_True);
        rtl::OUString sOUFormula(sFormula);
        sal_uInt8 nMatrixFlag(pFormulaCell->GetMatrixFlag());
        if (nMatrixFlag)
        {
            if (nMatrixFlag == MM_FORMULA)
            {
                sal_uInt16 nColumns, nRows;
                pFormulaCell->GetMatColsRows(nColumns, nRows);
                rtl::OUStringBuffer sColumns;
                rtl::OUStringBuffer sRows;
                SvXMLUnitConverter::convertNumber(sColumns, static_cast<sal_Int32>(nColumns));
                SvXMLUnitConverter::convertNumber(sRows, static_cast<sal_Int32>(nRows));
                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_number_matrix_columns_spanned, sColumns.makeStringAndClear());
                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_number_matrix_rows_spanned, sRows.makeStringAndClear());
            }
            else
            {
                rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_matrix_covered, sXML_true);
            }
            rtl::OUString sMatrixFormula = sOUFormula.copy(1, sOUFormula.getLength() - 2);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_formula, sMatrixFormula);
        }
        else
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_formula, sFormula);
        if (pFormulaCell->IsValue())
        {
            SetValueAttributes(pFormulaCell->GetValue(), sValue);
            SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, sXML_change_track_table_cell, sal_True, sal_True);
        }
        else
        {
            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_value_type, sXML_string);
            String sValue;
            pFormulaCell->GetString(sValue);
            rtl::OUString sOUValue(sValue);
            SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TABLE, sXML_change_track_table_cell, sal_True, sal_True);
            if (sOUValue.getLength())
            {
                SvXMLElementExport aElemC(rExport, XML_NAMESPACE_TEXT, sXML_p, sal_True, sal_False);
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
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, sXML_cell_content_change, sal_True, sal_True);
    const ScChangeAction* pConstAction = pAction;
    WriteBigRange(pConstAction->GetBigRange(), sXML_cell_address);
    WriteChangeInfo(pAction);
    WriteDependings(pAction);
    {
        ScChangeActionContent* pPrevAction = static_cast<ScChangeActionContent*>(pAction)->GetPrevContent();
        if (pPrevAction)
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_id, GetChangeID(pPrevAction->GetActionNumber()));
        SvXMLElementExport aElemPrev(rExport, XML_NAMESPACE_TABLE, sXML_previous, sal_True, sal_True);
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
            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_type, sXML_column);
            nStartPosition = nStartColumn;
            nEndPosition = nEndColumn;
        }
        break;
        case SC_CAT_INSERT_ROWS :
        {
            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_type, sXML_row);
            nStartPosition = nStartRow;
            nEndPosition = nEndRow;
        }
        break;
        case SC_CAT_INSERT_TABS :
        {
            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_type, sXML_table);
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
    rtl::OUStringBuffer sBuffer;
    SvXMLUnitConverter::convertNumber(sBuffer, nPosition);
    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_position, sBuffer.makeStringAndClear());
    DBG_ASSERT(nCount > 0, "wrong insertion count");
    if (nCount > 1)
    {
        SvXMLUnitConverter::convertNumber(sBuffer, nCount);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_count, sBuffer.makeStringAndClear());
    }
    if (pConstAction->GetType() != SC_CAT_INSERT_TABS)
    {
        SvXMLUnitConverter::convertNumber(sBuffer, nStartSheet);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_table, sBuffer.makeStringAndClear());
    }
}

void ScChangeTrackingExportHelper::WriteInsertion(ScChangeAction* pAction)
{
    AddInsertionAttributes(pAction);
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, sXML_insertion, sal_True, sal_True);
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
            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_type, sXML_column);
            nPosition = nStartColumn;
        }
        break;
        case SC_CAT_DELETE_ROWS :
        {
            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_type, sXML_row);
            nPosition = nStartRow;
        }
        break;
        case SC_CAT_DELETE_TABS :
        {
            //rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_type, sXML_table);
            DBG_ERROR("not implemented feature");
        }
        break;
        default :
        {
            DBG_ERROR("wrong deletion type");
        }
        break;
    }
    rtl::OUStringBuffer sBuffer;
    SvXMLUnitConverter::convertNumber(sBuffer, nPosition);
    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_position, sBuffer.makeStringAndClear());
    if (pDelAction->GetType() != SC_CAT_DELETE_TABS)
    {
        SvXMLUnitConverter::convertNumber(sBuffer, nStartSheet);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_table, sBuffer.makeStringAndClear());
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
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_multi_deletion_spanned, sBuffer.makeStringAndClear());
        }
    }
}

void ScChangeTrackingExportHelper::WriteCutOffs(const ScChangeActionDel* pAction)
{
    const ScChangeActionIns* pCutOffIns = pAction->GetCutOffInsert();
    const ScChangeActionDelMoveEntry* pLinkMove = pAction->GetFirstMoveEntry();
    if (pCutOffIns || pLinkMove)
    {
        SvXMLElementExport aCutOffsElem (rExport, XML_NAMESPACE_TABLE, sXML_cut_offs, sal_True, sal_True);
        rtl::OUStringBuffer sBuffer;
        if (pCutOffIns)
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_id, GetChangeID(pAction->GetActionNumber()));
            SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(pAction->GetCutOffCount()));
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_position, sBuffer.makeStringAndClear());
            SvXMLElementExport aInsertCutOffElem (rExport, XML_NAMESPACE_TABLE, sXML_insertion_cut_off, sal_True, sal_True);
        }
        while (pLinkMove)
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_id, GetChangeID(pLinkMove->GetAction()->GetActionNumber()));
            if (pLinkMove->GetCutOffFrom() == pLinkMove->GetCutOffTo())
            {
                SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(pLinkMove->GetCutOffFrom()));
                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_position, sBuffer.makeStringAndClear());
            }
            else
            {
                SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(pLinkMove->GetCutOffFrom()));
                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_start_position, sBuffer.makeStringAndClear());
                SvXMLUnitConverter::convertNumber(sBuffer, static_cast<sal_Int32>(pLinkMove->GetCutOffTo()));
                rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_end_position, sBuffer.makeStringAndClear());
            }
            SvXMLElementExport aMoveCutOffElem (rExport, XML_NAMESPACE_TABLE, sXML_movement_cut_off, sal_True, sal_True);
            pLinkMove = pLinkMove->GetNext();
        }
    }
}

void ScChangeTrackingExportHelper::WriteDeletion(ScChangeAction* pAction)
{
    ScChangeActionDel* pDelAction = static_cast<ScChangeActionDel*> (pAction);
    AddDeletionAttributes(pDelAction, pDelAction);
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, sXML_deletion, sal_True, sal_True);
    WriteChangeInfo(pDelAction);
    WriteDependings(pDelAction);
    WriteCutOffs(pDelAction);
}

void ScChangeTrackingExportHelper::WriteMovement(ScChangeAction* pAction)
{
    const ScChangeActionMove* pMoveAction = static_cast<ScChangeActionMove*> (pAction);
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, sXML_movement, sal_True, sal_True);
    const ScBigRange& rBigRange = pMoveAction->GetFromRange();
    WriteBigRange(rBigRange, sXML_source_range_address);
    sal_Int32 nStartColumn(0);
    sal_Int32 nEndColumn(0);
    sal_Int32 nStartRow(0);
    sal_Int32 nEndRow(0);
    sal_Int32 nStartSheet(0);
    sal_Int32 nEndSheet(0);
    rBigRange.GetVars(nStartColumn, nStartRow, nStartSheet,
        nEndColumn, nEndRow, nEndSheet);
    sal_Int32 nDx(0);
    sal_Int32 nDy(0);
    sal_Int32 nDz(0);
    pMoveAction->GetDelta(nDx, nDy, nDz);
    nStartColumn += nDx;
    nEndColumn += nDx;
    nStartRow += nDy;
    nEndRow += nDy;
    nStartSheet += nDz;
    nEndSheet += nDz;
    ScBigRange aBigRange(nStartColumn, nStartRow, nStartSheet,
        nEndColumn, nEndRow, nEndSheet);
    WriteBigRange(aBigRange, sXML_target_range_address);
    WriteChangeInfo(pAction);
    WriteDependings(pAction);
}

void ScChangeTrackingExportHelper::WriteRejection(ScChangeAction* pAction)
{
    SvXMLElementExport aElemChange(rExport, XML_NAMESPACE_TABLE, sXML_rejection, sal_True, sal_True);
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
    rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_id, GetChangeID(pAction->GetActionNumber()));
    GetAcceptanceState(pAction);
    if (pAction->IsRejecting())
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_rejecting_change_id, GetChangeID(pAction->GetRejectAction()));
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

void ScChangeTrackingExportHelper::WriteChangeViewSettings()
{
    ScChangeViewSettings* pViewSettings = rExport.GetDocument()->GetChangeViewSettings();
    if (pViewSettings && pChangeTrack)
    {
        if (!pViewSettings->ShowChanges())
            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_show_changes, sXML_false);
        if (pViewSettings->IsShowAccepted())
            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_show_accepted_changes, sXML_true);
        if (pViewSettings->IsShowRejected())
            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_show_rejected_changes, sXML_true);
        SvXMLElementExport aChangeViewSettingsElem(rExport, XML_NAMESPACE_TABLE, sXML_change_view_settings, sal_True, sal_True);
        {
            SvXMLElementExport aChangeViewConditionsElem(rExport, XML_NAMESPACE_TABLE, sXML_change_view_conditions, sal_True, sal_True);
            {
                {
                    if (pViewSettings->HasDate())
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_use_condition, sXML_true);
                    switch (pViewSettings->GetTheDateMode())
                    {
                        case SCDM_DATE_BEFORE:
                        {
                            rtl::OUStringBuffer sDate;
                            ScXMLConverter::ConvertDateTimeToString(pViewSettings->GetTheFirstDateTime(), sDate);
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_date_time, sDate.makeStringAndClear());
                            SvXMLElementExport aDateBeforeElem(rExport, XML_NAMESPACE_TABLE, sXML_before_date_time, sal_True, sal_True);
                        }
                        break;
                        case SCDM_DATE_SINCE:
                        {
                            rtl::OUStringBuffer sDate;
                            ScXMLConverter::ConvertDateTimeToString(pViewSettings->GetTheFirstDateTime(), sDate);
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_date_time, sDate.makeStringAndClear());
                            SvXMLElementExport aDateSinceElem(rExport, XML_NAMESPACE_TABLE, sXML_since_date_time, sal_True, sal_True);
                        }
                        break;
                        case SCDM_DATE_EQUAL:
                        {
                            rtl::OUStringBuffer sDate;
                            ScXMLConverter::ConvertDateTimeToString(pViewSettings->GetTheFirstDateTime(), sDate);
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_date, sDate.makeStringAndClear());
                            SvXMLElementExport aDateEqualElem(rExport, XML_NAMESPACE_TABLE, sXML_equal_date, sal_True, sal_True);
                        }
                        break;
                        case SCDM_DATE_NOTEQUAL:
                        {
                            rtl::OUStringBuffer sDate;
                            ScXMLConverter::ConvertDateTimeToString(pViewSettings->GetTheFirstDateTime(), sDate);
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_date, sDate.makeStringAndClear());
                            SvXMLElementExport aDateNotEqualElem(rExport, XML_NAMESPACE_TABLE, sXML_not_equal_date, sal_True, sal_True);
                        }
                        break;
                        case SCDM_DATE_BETWEEN:
                        {
                            rtl::OUStringBuffer sDate;
                            ScXMLConverter::ConvertDateTimeToString(pViewSettings->GetTheFirstDateTime(), sDate);
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_first_date_time, sDate.makeStringAndClear());
                            ScXMLConverter::ConvertDateTimeToString(pViewSettings->GetTheFirstDateTime(), sDate);
                            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_second_date_time, sDate.makeStringAndClear());
                            SvXMLElementExport aDateBetweenElem(rExport, XML_NAMESPACE_TABLE, sXML_between_date_times, sal_True, sal_True);
                        }
                        break;
                        case SCDM_DATE_SAVE:
                        {
                            SvXMLElementExport aDateSaveElem(rExport, XML_NAMESPACE_TABLE, sXML_since_save, sal_True, sal_True);
                        }
                        break;
                        case SCDM_NO_DATEMODE:
                        {
                            DBG_ERROR("wrong date mode");
                        }
                        break;
                    }
                }
                rtl::OUString sAuthor (pViewSettings->GetTheAuthorToShow());
                if (sAuthor.getLength() && !pViewSettings->HasAuthor())
                {
                    if (pViewSettings->HasAuthor())
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_use_condition, sXML_true);
                    if (sAuthor.getLength())
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_author, sAuthor);
                    SvXMLElementExport aEqualAuthorElem(rExport, XML_NAMESPACE_TABLE, sXML_equal_author, sal_True, sal_True);
                }
                rtl::OUString sComment (pViewSettings->GetTheComment());
                if (sComment.getLength() && !pViewSettings->HasComment())
                {
                    if (pViewSettings->HasComment())
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_use_condition, sXML_true);
                    if (sComment.getLength())
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_comment, sComment);
                    SvXMLElementExport aEqualCommentElem(rExport, XML_NAMESPACE_TABLE, sXML_equal_comment, sal_True, sal_True);
                }
                rtl::OUString sRangeList;
                ScXMLConverter::GetStringFromRangeList(sRangeList, &(pViewSettings->GetTheRangeList()), rExport.GetDocument());
                if (sRangeList.getLength() && !pViewSettings->HasRange())
                {
                    if (pViewSettings->HasRange())
                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_use_condition, sXML_true);
                    if (sRangeList.getLength())
                        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_cell_range_address_list, sRangeList);
                    SvXMLElementExport aInRangeElem(rExport, XML_NAMESPACE_TABLE, sXML_in_range, sal_True, sal_True);
                }
            }
        }
    }
}

void ScChangeTrackingExportHelper::CollectAndWriteChanges()
{
    if (pChangeTrack)
    {
        sal_uInt32 nCount (pChangeTrack->GetActionMax());
        if (nCount)
        {
            SvXMLElementExport aCangeListElem(rExport, XML_NAMESPACE_TABLE, sXML_tracked_changes, sal_True, sal_True);
            {
                ScChangeAction* pAction = pChangeTrack->GetFirst();
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
