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
#include "tokenarray.hxx"

struct ScDocumentImportImpl
{
    ScDocument& mrDoc;
    sc::ColumnBlockPositionSet maBlockPosSet;
    sal_uInt16 mnDefaultScriptNumeric;

    ScDocumentImportImpl(ScDocument& rDoc) : mrDoc(rDoc), maBlockPosSet(rDoc), mnDefaultScriptNumeric(SC_SCRIPTTYPE_UNKNOWN) {}
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

void ScDocumentImport::setDefaultNumericScript(sal_uInt16 nScript)
{
    mpImpl->mnDefaultScriptNumeric = nScript;
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

    sc::ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    ScCellValue aCell;
    pTab->aCol[rPos.Col()].ParseString(
        aCell, rPos.Row(), rPos.Tab(), rStr, mpImpl->mrDoc.GetAddressConvention(), NULL);

    sc::CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    switch (aCell.meType)
    {
        case CELLTYPE_STRING:
            // string is copied.
            pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), *aCell.mpString);
        break;
        case CELLTYPE_EDIT:
            // Cell takes the ownership of the text object.
            pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), aCell.mpEditText);
            aCell.mpEditText = NULL;
        break;
        case CELLTYPE_VALUE:
            pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), aCell.mfValue);
        break;
        case CELLTYPE_FORMULA:
            // This formula cell instance is directly placed in the document without copying.
            pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), aCell.mpFormula);
            aCell.mpFormula = NULL;
        break;
        default:
            pBlockPos->miCellPos = rCells.set_empty(pBlockPos->miCellPos, rPos.Row(), rPos.Row());
    }
}

void ScDocumentImport::setNumericCell(const ScAddress& rPos, double fVal)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    sc::ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    sc::CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), fVal);
}

void ScDocumentImport::setStringCell(const ScAddress& rPos, const OUString& rStr)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    sc::ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    sc::CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), rStr);
}

void ScDocumentImport::setEditCell(const ScAddress& rPos, EditTextObject* pEditText)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    sc::ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    sc::CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), pEditText);
}

void ScDocumentImport::setFormulaCell(
    const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    sc::ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    sc::CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    pBlockPos->miCellPos =
        rCells.set(pBlockPos->miCellPos, rPos.Row(), new ScFormulaCell(&mpImpl->mrDoc, rPos, rFormula, eGrammar));
}

void ScDocumentImport::setFormulaCell(const ScAddress& rPos, const ScTokenArray& rArray)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    sc::ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    sc::CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    pBlockPos->miCellPos =
        rCells.set(pBlockPos->miCellPos, rPos.Row(), new ScFormulaCell(&mpImpl->mrDoc, rPos, &rArray));
}

void ScDocumentImport::setFormulaCell(const ScAddress& rPos, ScFormulaCell* pCell)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    sc::ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    sc::CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    pBlockPos->miCellPos =
        rCells.set(pBlockPos->miCellPos, rPos.Row(), pCell);
}

void ScDocumentImport::setMatrixCells(
    const ScRange& rRange, const ScTokenArray& rArray, formula::FormulaGrammar::Grammar eGram)
{
    const ScAddress& rBasePos = rRange.aStart;

    ScTable* pTab = mpImpl->mrDoc.FetchTable(rBasePos.Tab());
    if (!pTab)
        return;

    sc::ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rBasePos.Tab(), rBasePos.Col());

    if (!pBlockPos)
        return;

    sc::CellStoreType& rCells = pTab->aCol[rBasePos.Col()].maCells;

    // Set the master cell.
    ScFormulaCell* pCell = new ScFormulaCell(&mpImpl->mrDoc, rBasePos, &rArray, eGram, MM_FORMULA);

    pBlockPos->miCellPos =
        rCells.set(pBlockPos->miCellPos, rBasePos.Row(), pCell);

    // Set the reference cells.
    ScSingleRefData aRefData;
    aRefData.InitFlags();
    aRefData.SetColRel(true);
    aRefData.SetRowRel(true);
    aRefData.SetTabRel(true);
    aRefData.SetAddress(rBasePos, rBasePos);

    ScTokenArray aArr; // consists only of one single reference token.
    ScToken* t = static_cast<ScToken*>(aArr.AddMatrixSingleReference(aRefData));

    ScAddress aPos = rBasePos;
    for (SCROW nRow = rRange.aStart.Row()+1; nRow <= rRange.aEnd.Row(); ++nRow)
    {
        // Token array must be cloned so that each formula cell receives its own copy.
        aPos.SetRow(nRow);
        // Reference in each cell must point to the origin cell relative to the current cell.
        aRefData.SetAddress(rBasePos, aPos);
        t->GetSingleRef() = aRefData;
        boost::scoped_ptr<ScTokenArray> pTokArr(aArr.Clone());
        pCell = new ScFormulaCell(&mpImpl->mrDoc, aPos, pTokArr.get(), eGram, MM_REFERENCE);
        pBlockPos->miCellPos =
            rCells.set(pBlockPos->miCellPos, aPos.Row(), pCell);
    }

    for (SCCOL nCol = rRange.aStart.Col()+1; nCol <= rRange.aEnd.Col(); ++nCol)
    {
        pBlockPos = mpImpl->maBlockPosSet.getBlockPosition(rBasePos.Tab(), nCol);
        if (!pBlockPos)
            return;

        sc::CellStoreType& rColCells = pTab->aCol[nCol].maCells;

        aPos.SetCol(nCol);
        for (SCROW nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
        {
            aPos.SetRow(nRow);
            aRefData.SetAddress(rBasePos, aPos);
            t->GetSingleRef() = aRefData;
            boost::scoped_ptr<ScTokenArray> pTokArr(aArr.Clone());
            pCell = new ScFormulaCell(&mpImpl->mrDoc, aPos, pTokArr.get(), eGram, MM_REFERENCE);
            pBlockPos->miCellPos =
                rColCells.set(pBlockPos->miCellPos, aPos.Row(), pCell);
        }
    }
}

namespace {

class CellTextAttrInitializer
{
    struct Impl
    {
        sc::CellTextAttrStoreType maAttrs;
        sc::CellTextAttrStoreType::iterator miPos;
        sal_uInt16 mnScriptNumeric;

        Impl(const sal_uInt32 nMaxRowCount, const sal_uInt16 nScriptNumeric)
            : maAttrs(nMaxRowCount), miPos(maAttrs.begin()), mnScriptNumeric(nScriptNumeric)
        {}
    };

    boost::shared_ptr<Impl> mpImpl;

public:
    CellTextAttrInitializer(sal_uInt16 nScriptNumeric) : mpImpl(new Impl(MAXROWCOUNT, nScriptNumeric)) {}

    void operator() (const sc::CellStoreType::value_type& node)
    {
        if (node.type == sc::element_type_empty)
            return;

        // Fill with default values for non-empty cell segments.
        sc::CellTextAttr aDefault;
        if (node.type == sc::element_type_numeric)
            aDefault.mnScriptType = mpImpl->mnScriptNumeric;
        std::vector<sc::CellTextAttr> aDefaults(node.size, aDefault);
        mpImpl->miPos = mpImpl->maAttrs.set(mpImpl->miPos, node.position, aDefaults.begin(), aDefaults.end());
    }

    void swap(sc::CellTextAttrStoreType& rAttrs)
    {
        mpImpl->maAttrs.swap(rAttrs);
    }
};

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
            initColumn(*pCol);
    }
}

void ScDocumentImport::initColumn(ScColumn& rCol)
{
    CellTextAttrInitializer aFunc(mpImpl->mnDefaultScriptNumeric);
    std::for_each(rCol.maCells.begin(), rCol.maCells.end(), aFunc);
    aFunc.swap(rCol.maCellTextAttrs);
    rCol.CellStorageModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
