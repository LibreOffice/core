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
#include "formulacell.hxx"
#include "docoptio.hxx"
#include "globalnames.hxx"
#include "mtvelements.hxx"

struct ScDocumentImportImpl
{
    ScDocument& mrDoc;
    sc::ColumnBlockPositionSet maBlockPosSet;

    ScDocumentImportImpl(ScDocument& rDoc) : mrDoc(rDoc), maBlockPosSet(rDoc) {}
};

ScDocumentImport::ScDocumentImport(ScDocument& rDoc) : mpImpl(new ScDocumentImportImpl(rDoc)) {}
ScDocumentImport::~ScDocumentImport()
{
    delete mpImpl;
}

ScDocument& ScDocumentImport::getDoc()
{
    return mpImpl->mrDoc;
}

const ScDocument& ScDocumentImport::getDoc() const
{
    return mpImpl->mrDoc;
}

SCTAB ScDocumentImport::getSheetIndex(const OUString& rName) const
{
    SCTAB nTab = -1;
    if (!mpImpl->mrDoc.GetTable(rName, nTab))
        return -1;

    return nTab;
}

SCTAB ScDocumentImport::getSheetCount() const
{
    return mpImpl->mrDoc.maTabs.size();
}

bool ScDocumentImport::appendSheet(const OUString& rName)
{
    SCTAB nTabCount = mpImpl->mrDoc.maTabs.size();
    if (!ValidTab(nTabCount))
        return false;

    mpImpl->mrDoc.maTabs.push_back(new ScTable(&mpImpl->mrDoc, nTabCount, rName));
    return true;
}

void ScDocumentImport::setOriginDate(sal_uInt16 nYear, sal_uInt16 nMonth, sal_uInt16 nDay)
{
    if (!mpImpl->mrDoc.pDocOptions)
        mpImpl->mrDoc.pDocOptions = new ScDocOptions;

    mpImpl->mrDoc.pDocOptions->SetDate(nDay, nMonth, nYear);
}

void ScDocumentImport::setAutoInput(const ScAddress& rPos, const OUString& rStr)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    pTab->aCol[rPos.Col()].SetString(
        rPos.Row(), rPos.Tab(), rStr, mpImpl->mrDoc.GetAddressConvention());
}

void ScDocumentImport::setNumericCell(const ScAddress& rPos, double fVal)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    pTab->aCol[rPos.Col()].SetValue(rPos.Row(), fVal);
}

void ScDocumentImport::setStringCell(const ScAddress& rPos, const OUString& rStr)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    pTab->aCol[rPos.Col()].SetRawString(rPos.Row(), rStr);
}

void ScDocumentImport::setFormulaCell(
    const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    pTab->aCol[rPos.Col()].SetFormulaCell(
        rPos.Row(), new ScFormulaCell(&mpImpl->mrDoc, rPos, rFormula, eGrammar));
}

void ScDocumentImport::setFormulaCell(const ScAddress& rPos, const ScTokenArray& rArray)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    pTab->aCol[rPos.Col()].SetFormulaCell(
        rPos.Row(), new ScFormulaCell(&mpImpl->mrDoc, rPos, &rArray));
}

void ScDocumentImport::finalize()
{
    // Populate the text width and script type arrays in all columns.
    ScDocument::TableContainer::iterator itTab = mpImpl->mrDoc.maTabs.begin(), itTabEnd = mpImpl->mrDoc.maTabs.end();
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
            rCol.ResetCellTextAttrs();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
