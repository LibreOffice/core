/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "documentimport.hxx"
#include "document.hxx"
#include "table.hxx"
#include "column.hxx"
#include "cell.hxx"
#include "formulacell.hxx"
#include "docoptio.hxx"
#include "globalnames.hxx"

ScDocumentImport::ScDocumentImport(ScDocument& rDoc) : mrDoc(rDoc) {}
ScDocumentImport::ScDocumentImport(const ScDocumentImport& r) : mrDoc(r.mrDoc) {}

ScDocument& ScDocumentImport::getDoc()
{
    return mrDoc;
}

const ScDocument& ScDocumentImport::getDoc() const
{
    return mrDoc;
}

SCTAB ScDocumentImport::getSheetIndex(const OUString& rName) const
{
    SCTAB nTab = -1;
    if (!mrDoc.GetTable(rName, nTab))
        return -1;

    return nTab;
}

SCTAB ScDocumentImport::getSheetCount() const
{
    return mrDoc.maTabs.size();
}

bool ScDocumentImport::appendSheet(const OUString& rName)
{
    SCTAB nTabCount = mrDoc.maTabs.size();
    if (!ValidTab(nTabCount))
        return false;

    mrDoc.maTabs.push_back(new ScTable(&mrDoc, nTabCount, rName));
    return true;
}

void ScDocumentImport::setOriginDate(sal_uInt16 nYear, sal_uInt16 nMonth, sal_uInt16 nDay)
{
    if (!mrDoc.pDocOptions)
        mrDoc.pDocOptions = new ScDocOptions;

    mrDoc.pDocOptions->SetDate(nDay, nMonth, nYear);
}

void ScDocumentImport::setAutoInput(const ScAddress& rPos, const OUString& rStr)
{
    if (!mrDoc.TableExists(rPos.Tab()))
        return;

    mrDoc.maTabs[rPos.Tab()]->aCol[rPos.Col()].SetString(rPos.Row(), rPos.Tab(), rStr, mrDoc.GetAddressConvention());
}

void ScDocumentImport::setNumericCell(const ScAddress& rPos, double fVal)
{
    insertCell(rPos, new ScValueCell(fVal));
}

void ScDocumentImport::setStringCell(const ScAddress& rPos, const OUString& rStr)
{
    insertCell(rPos, new ScStringCell(rStr));
}

void ScDocumentImport::setFormulaCell(
    const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar)
{
    insertCell(rPos, new ScFormulaCell(&mrDoc, rPos, rFormula, eGrammar));
}

void ScDocumentImport::setFormulaCell(const ScAddress& rPos, const ScTokenArray& rArray)
{
    insertCell(rPos, new ScFormulaCell(&mrDoc, rPos, &rArray));
}

void ScDocumentImport::finalize()
{
    // Populate the text width and script type arrays in all columns.
    ScDocument::TableContainer::iterator itTab = mrDoc.maTabs.begin(), itTabEnd = mrDoc.maTabs.end();
    for (; itTab != itTabEnd; ++itTab)
    {
        if (!*itTab)
            continue;

        ScTable& rTab = **itTab;
        ScColumn* pCol = &rTab.aCol[0];
        ScColumn* pColEnd = pCol + static_cast<size_t>(MAXCOLCOUNT);
        for (; pCol != pColEnd; ++pCol)
        {
            ScColumn& rCol = *pCol;
            if (rCol.maItems.empty())
                // Column has no cells. Skip it.
                continue;

            ScColumn::TextWidthType::iterator itWidthPos = rCol.maTextWidths.begin();
            ScColumn::ScriptType::iterator itScriptPos = rCol.maScriptTypes.begin();
            std::vector<ColEntry>::iterator itCell = rCol.maItems.begin(), itCellEnd = rCol.maItems.end();
            for (; itCell != itCellEnd; ++itCell)
            {
                itWidthPos = rCol.maTextWidths.set<unsigned short>(itWidthPos, itCell->nRow, TEXTWIDTH_DIRTY);
                itScriptPos = rCol.maScriptTypes.set<unsigned short>(itScriptPos, itCell->nRow, SC_SCRIPTTYPE_UNKNOWN);
            }
        }
    }
}

void ScDocumentImport::insertCell(const ScAddress& rPos, ScBaseCell* pCell)
{
    if (!mrDoc.TableExists(rPos.Tab()))
    {
        pCell->Delete();
        return;
    }

    ScColumn& rCol = mrDoc.maTabs[rPos.Tab()]->aCol[rPos.Col()];
    rCol.SetCell(rPos.Row(), pCell);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
