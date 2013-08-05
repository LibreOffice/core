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
#include "mtvelements.hxx"
#include "tokenarray.hxx"
#include "cellvalue.hxx"

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

    ScCellValue aCell;
    pTab->aCol[rPos.Col()].ParseString(
        aCell, rPos.Row(), rPos.Tab(), rStr, mpImpl->mrDoc.GetAddressConvention(), NULL);

    ScColumn& rCol = pTab->aCol[rPos.Col()];
    switch (aCell.meType)
    {
        case CELLTYPE_STRING:
            // string is copied.
            setCell(rCol, rPos.Row(), new ScStringCell(*aCell.mpString));
        break;
        case CELLTYPE_EDIT:
            // Cell takes the ownership of the text object.
            setCell(rCol, rPos.Row(), new ScEditCell(aCell.mpEditText, &mpImpl->mrDoc));
            aCell.mpEditText = NULL;
        break;
        case CELLTYPE_VALUE:
            setCell(rCol, rPos.Row(), new ScValueCell(aCell.mfValue));
        break;
        case CELLTYPE_FORMULA:
            // This formula cell instance is directly placed in the document without copying.
            setCell(rCol, rPos.Row(), aCell.mpFormula);
            aCell.mpFormula = NULL;
        break;
        default:
            ;
    }
}

void ScDocumentImport::setNumericCell(const ScAddress& rPos, double fVal)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    ScColumn& rCol = pTab->aCol[rPos.Col()];
    setCell(rCol, rPos.Row(), new ScValueCell(fVal));
}

void ScDocumentImport::setStringCell(const ScAddress& rPos, const OUString& rStr)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    ScColumn& rCol = pTab->aCol[rPos.Col()];
    setCell(rCol, rPos.Row(), new ScStringCell(rStr));
}

void ScDocumentImport::setEditCell(const ScAddress& rPos, EditTextObject* pEditText)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    ScColumn& rCol = pTab->aCol[rPos.Col()];
    setCell(rCol, rPos.Row(), new ScEditCell(pEditText, &mpImpl->mrDoc));
}

void ScDocumentImport::setFormulaCell(
    const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    ScColumn& rCol = pTab->aCol[rPos.Col()];
    setCell(rCol, rPos.Row(), new ScFormulaCell(&mpImpl->mrDoc, rPos, rFormula, eGrammar));
}

void ScDocumentImport::setFormulaCell(const ScAddress& rPos, const ScTokenArray& rArray)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    ScColumn& rCol = pTab->aCol[rPos.Col()];
    setCell(rCol, rPos.Row(), new ScFormulaCell(&mpImpl->mrDoc, rPos, &rArray));
}

void ScDocumentImport::setFormulaCell(const ScAddress& rPos, ScFormulaCell* pCell)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    ScColumn& rCol = pTab->aCol[rPos.Col()];
    setCell(rCol, rPos.Row(), pCell);
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

    ScColumn& rCol = pTab->aCol[rBasePos.Col()];

    // Set the master cell.
    ScFormulaCell* pCell = new ScFormulaCell(&mpImpl->mrDoc, rBasePos, &rArray, eGram, MM_FORMULA);
    setCell(rCol, rBasePos.Row(), pCell);

    // Set the reference cells.
    ScSingleRefData aRefData;
    aRefData.InitFlags();
    aRefData.SetColRel(true);
    aRefData.SetRowRel(true);
    aRefData.SetTabRel(true);
    aRefData.nRelCol = 0;
    aRefData.nRelRow = 0;
    aRefData.nRelTab = 0;

    ScTokenArray aArr; // consists only of one single reference token.
    ScToken* t = static_cast<ScToken*>(aArr.AddMatrixSingleReference(aRefData));

    ScAddress aPos = rBasePos;
    for (SCROW nRow = rRange.aStart.Row()+1; nRow <= rRange.aEnd.Row(); ++nRow)
    {
        // Token array must be cloned so that each formula cell receives its own copy.
        aPos.SetRow(nRow);
        // Reference in each cell must point to the origin cell relative to the current cell.
        aRefData.nRelRow = rBasePos.Row() - nRow;
        t->GetSingleRef() = aRefData;
        boost::scoped_ptr<ScTokenArray> pTokArr(aArr.Clone());
        pCell = new ScFormulaCell(&mpImpl->mrDoc, aPos, pTokArr.get(), eGram, MM_REFERENCE);
        setCell(rCol, aPos.Row(), pCell);
    }

    for (SCCOL nCol = rRange.aStart.Col()+1; nCol <= rRange.aEnd.Col(); ++nCol)
    {
        pBlockPos = mpImpl->maBlockPosSet.getBlockPosition(rBasePos.Tab(), nCol);
        if (!pBlockPos)
            return;

        ScColumn& rCol2 = pTab->aCol[nCol];

        aPos.SetCol(nCol);
        aRefData.nRelRow = 0;
        aRefData.nRelCol = rBasePos.Col() - nCol;

        for (SCROW nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
        {
            aPos.SetRow(nRow);
            aRefData.nRelRow = rBasePos.Row() - nRow;
            t->GetSingleRef() = aRefData;
            boost::scoped_ptr<ScTokenArray> pTokArr(aArr.Clone());
            pCell = new ScFormulaCell(&mpImpl->mrDoc, aPos, pTokArr.get(), eGram, MM_REFERENCE);
            setCell(rCol2, aPos.Row(), pCell);
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

    void operator() (const ColEntry& rEntry)
    {
        sc::CellTextAttr aDefault;
        if (rEntry.pCell->GetCellType() == CELLTYPE_VALUE)
            aDefault.mnScriptType = mpImpl->mnScriptNumeric;
        mpImpl->miPos = mpImpl->maAttrs.set(mpImpl->miPos, rEntry.nRow, aDefault);
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

void ScDocumentImport::setCell(ScColumn& rCol, SCROW nRow, ScBaseCell* pCell)
{
    if (pCell->GetCellType() == CELLTYPE_FORMULA)
    {
        ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pCell);
        sal_uInt32 nCellFormat = rCol.GetNumberFormat(nRow);
        if ((nCellFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
            pFCell->SetNeedNumberFormat(true);
    }

    std::vector<ColEntry>& rItems = rCol.maItems;
    if (!rItems.empty())
    {
        if (rItems.back().nRow < nRow)
        {
            rItems.push_back(ColEntry());
            rItems.back().pCell = pCell;
            rItems.back().nRow  = nRow;
            return;
        }
    }

    SCSIZE nIndex;
    if (rCol.Search(nRow, nIndex))
    {
        ScBaseCell* pOldCell = rItems[nIndex].pCell;
        pOldCell->Delete();
        rItems[nIndex].pCell = pCell;
    }
    else
    {
        rItems.insert(rItems.begin() + nIndex, ColEntry());
        rItems[nIndex].pCell = pCell;
        rItems[nIndex].nRow  = nRow;
    }
}

void ScDocumentImport::initColumn(ScColumn& rCol)
{
    CellTextAttrInitializer aFunc(mpImpl->mnDefaultScriptNumeric);
    std::for_each(rCol.maItems.begin(), rCol.maItems.end(), aFunc);
    aFunc.swap(rCol.maCellTextAttrs);
    rCol.CellStorageModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
