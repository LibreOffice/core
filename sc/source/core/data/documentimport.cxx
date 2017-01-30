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
#include "stringutil.hxx"
#include "compiler.hxx"
#include "paramisc.hxx"
#include "listenercontext.hxx"
#include <attarray.hxx>
#include <sharedformula.hxx>

#include <svl/sharedstringpool.hxx>
#include <svl/languageoptions.hxx>

#include <vector>

namespace {

struct ColAttr
{
    bool mbLatinNumFmtOnly;

    ColAttr() : mbLatinNumFmtOnly(false) {}
};

struct TabAttr
{
    std::vector<ColAttr> maCols;
};

}

struct ScDocumentImportImpl
{
    ScDocument& mrDoc;
    sc::StartListeningContext maListenCxt;
    std::vector<sc::TableColumnBlockPositionSet> maBlockPosSet;
    SvtScriptType mnDefaultScriptNumeric;
    std::vector<TabAttr> maTabAttrs;

    explicit ScDocumentImportImpl(ScDocument& rDoc) :
        mrDoc(rDoc),
        maListenCxt(rDoc),
        mnDefaultScriptNumeric(SvtScriptType::UNKNOWN) {}

    static bool isValid( size_t nTab, size_t nCol )
    {
        return (nTab <= size_t(MAXTAB) && nCol <= size_t(MAXCOL));
    }

    ColAttr* getColAttr( size_t nTab, size_t nCol )
    {
        if (!isValid(nTab, nCol))
            return nullptr;

        if (nTab >= maTabAttrs.size())
            maTabAttrs.resize(nTab+1);

        TabAttr& rTab = maTabAttrs[nTab];
        if (nCol >= rTab.maCols.size())
            rTab.maCols.resize(nCol+1);

        return &rTab.maCols[nCol];
    }

    sc::ColumnBlockPosition* getBlockPosition( SCTAB nTab, SCCOL nCol )
    {
        if (!isValid(nTab, nCol))
            return nullptr;

        if (size_t(nTab) >= maBlockPosSet.size())
        {
            for (SCTAB i = maBlockPosSet.size(); i <= nTab; ++i)
                maBlockPosSet.emplace_back(mrDoc, i);
        }

        sc::TableColumnBlockPositionSet& rTab = maBlockPosSet[nTab];
        return rTab.getBlockPosition(nCol);
    }

    void initForSheets()
    {
        size_t n = mrDoc.GetTableCount();
        for (size_t i = maBlockPosSet.size(); i < n; ++i)
            maBlockPosSet.emplace_back(mrDoc, i);

        if (maTabAttrs.size() < n)
            maTabAttrs.resize(n);
    }
};

ScDocumentImport::Attrs::Attrs() : mpData(nullptr), mnSize(0), mbLatinNumFmtOnly(false) {}

ScDocumentImport::ScDocumentImport(ScDocument& rDoc) : mpImpl(new ScDocumentImportImpl(rDoc)) {}

ScDocumentImport::~ScDocumentImport()
{
}

ScDocument& ScDocumentImport::getDoc()
{
    return mpImpl->mrDoc;
}

const ScDocument& ScDocumentImport::getDoc() const
{
    return mpImpl->mrDoc;
}

void ScDocumentImport::initForSheets()
{
    mpImpl->initForSheets();
}

void ScDocumentImport::setDefaultNumericScript(SvtScriptType nScript)
{
    mpImpl->mnDefaultScriptNumeric = nScript;
}

void ScDocumentImport::setCellStyleToSheet(SCTAB nTab, const ScStyleSheet& rStyle)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(nTab);
    if (!pTab)
        return;

    pTab->ApplyStyleArea(0, 0, pTab->aCol.size() - 1, MAXROW, rStyle);
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

void ScDocumentImport::setAutoInput(const ScAddress& rPos, const OUString& rStr, ScSetStringParam* pStringParam)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    sc::ColumnBlockPosition* pBlockPos = mpImpl->getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    ScCellValue aCell;
    pTab->aCol[rPos.Col()].ParseString(
        aCell, rPos.Row(), rPos.Tab(), rStr, mpImpl->mrDoc.GetAddressConvention(), pStringParam);

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
            aCell.mpEditText = nullptr;
        break;
        case CELLTYPE_VALUE:
            pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), aCell.mfValue);
        break;
        case CELLTYPE_FORMULA:
            // This formula cell instance is directly placed in the document without copying.
            pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), aCell.mpFormula);
            aCell.mpFormula = nullptr;
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

    sc::ColumnBlockPosition* pBlockPos = mpImpl->getBlockPosition(rPos.Tab(), rPos.Col());

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

    sc::ColumnBlockPosition* pBlockPos = mpImpl->getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    svl::SharedString aSS = mpImpl->mrDoc.GetSharedStringPool().intern(rStr);
    if (!aSS.getData())
        return;

    sc::CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), aSS);
}

void ScDocumentImport::setEditCell(const ScAddress& rPos, EditTextObject* pEditText)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    sc::ColumnBlockPosition* pBlockPos = mpImpl->getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    pEditText->NormalizeString(mpImpl->mrDoc.GetSharedStringPool());
    sc::CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), pEditText);
}

void ScDocumentImport::setFormulaCell(
    const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    sc::ColumnBlockPosition* pBlockPos = mpImpl->getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    sc::CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    pBlockPos->miCellPos =
        rCells.set(pBlockPos->miCellPos, rPos.Row(), new ScFormulaCell(&mpImpl->mrDoc, rPos, rFormula, eGrammar));
}

void ScDocumentImport::setFormulaCell(const ScAddress& rPos, ScTokenArray* pArray)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    sc::ColumnBlockPosition* pBlockPos = mpImpl->getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    sc::CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    pBlockPos->miCellPos =
        rCells.set(pBlockPos->miCellPos, rPos.Row(), new ScFormulaCell(&mpImpl->mrDoc, rPos, pArray));
}

void ScDocumentImport::setFormulaCell(const ScAddress& rPos, ScFormulaCell* pCell)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(rPos.Tab());
    if (!pTab)
        return;

    sc::ColumnBlockPosition* pBlockPos = mpImpl->getBlockPosition(rPos.Tab(), rPos.Col());

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

    sc::ColumnBlockPosition* pBlockPos = mpImpl->getBlockPosition(rBasePos.Tab(), rBasePos.Col());

    if (!pBlockPos)
        return;

    sc::CellStoreType& rCells = pTab->aCol[rBasePos.Col()].maCells;

    // Set the master cell.
    ScFormulaCell* pCell = new ScFormulaCell(&mpImpl->mrDoc, rBasePos, rArray, eGram, MM_FORMULA);

    pBlockPos->miCellPos =
        rCells.set(pBlockPos->miCellPos, rBasePos.Row(), pCell);

    // Matrix formulas currently need re-calculation on import.
    pCell->SetMatColsRows(
        rRange.aEnd.Col()-rRange.aStart.Col()+1, rRange.aEnd.Row()-rRange.aStart.Row()+1);

    // Set the reference cells.
    ScSingleRefData aRefData;
    aRefData.InitFlags();
    aRefData.SetColRel(true);
    aRefData.SetRowRel(true);
    aRefData.SetTabRel(true);
    aRefData.SetAddress(rBasePos, rBasePos);

    ScTokenArray aArr; // consists only of one single reference token.
    formula::FormulaToken* t = aArr.AddMatrixSingleReference(aRefData);

    ScAddress aPos = rBasePos;
    for (SCROW nRow = rRange.aStart.Row()+1; nRow <= rRange.aEnd.Row(); ++nRow)
    {
        // Token array must be cloned so that each formula cell receives its own copy.
        aPos.SetRow(nRow);
        // Reference in each cell must point to the origin cell relative to the current cell.
        aRefData.SetAddress(rBasePos, aPos);
        *t->GetSingleRef() = aRefData;
        std::unique_ptr<ScTokenArray> pTokArr(aArr.Clone());
        pCell = new ScFormulaCell(&mpImpl->mrDoc, aPos, *pTokArr, eGram, MM_REFERENCE);
        pBlockPos->miCellPos =
            rCells.set(pBlockPos->miCellPos, aPos.Row(), pCell);
    }

    for (SCCOL nCol = rRange.aStart.Col()+1; nCol <= rRange.aEnd.Col(); ++nCol)
    {
        pBlockPos = mpImpl->getBlockPosition(rBasePos.Tab(), nCol);
        if (!pBlockPos)
            return;

        sc::CellStoreType& rColCells = pTab->aCol[nCol].maCells;

        aPos.SetCol(nCol);
        for (SCROW nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
        {
            aPos.SetRow(nRow);
            aRefData.SetAddress(rBasePos, aPos);
            *t->GetSingleRef() = aRefData;
            std::unique_ptr<ScTokenArray> pTokArr(aArr.Clone());
            pCell = new ScFormulaCell(&mpImpl->mrDoc, aPos, *pTokArr, eGram, MM_REFERENCE);
            pBlockPos->miCellPos =
                rColCells.set(pBlockPos->miCellPos, aPos.Row(), pCell);
        }
    }
}

void ScDocumentImport::setTableOpCells(const ScRange& rRange, const ScTabOpParam& rParam)
{
    SCTAB nTab = rRange.aStart.Tab();
    SCCOL nCol1 = rRange.aStart.Col();
    SCROW nRow1 = rRange.aStart.Row();
    SCCOL nCol2 = rRange.aEnd.Col();
    SCROW nRow2 = rRange.aEnd.Row();

    ScTable* pTab = mpImpl->mrDoc.FetchTable(nTab);
    if (!pTab)
        return;

    ScDocument* pDoc = &mpImpl->mrDoc;
    ScRefAddress aRef;
    OUStringBuffer aFormulaBuf;
    aFormulaBuf.append('=');
    aFormulaBuf.append(ScCompiler::GetNativeSymbol(ocTableOp));
    aFormulaBuf.append(ScCompiler::GetNativeSymbol(ocOpen));

    OUString aSep = ScCompiler::GetNativeSymbol(ocSep);
    if (rParam.meMode == ScTabOpParam::Column) // column only
    {
        aRef.Set(rParam.aRefFormulaCell.GetAddress(), true, false, false);
        aFormulaBuf.append(aRef.GetRefString(pDoc, nTab));
        aFormulaBuf.append(aSep);
        aFormulaBuf.append(rParam.aRefColCell.GetRefString(pDoc, nTab));
        aFormulaBuf.append(aSep);
        aRef.Set(nCol1, nRow1, nTab, false, true, true);
        aFormulaBuf.append(aRef.GetRefString(pDoc, nTab));
        nCol1++;
        nCol2 = std::min( nCol2, (SCCOL)(rParam.aRefFormulaEnd.Col() -
                    rParam.aRefFormulaCell.Col() + nCol1 + 1));
    }
    else if (rParam.meMode == ScTabOpParam::Row) // row only
    {
        aRef.Set(rParam.aRefFormulaCell.GetAddress(), false, true, false);
        aFormulaBuf.append(aRef.GetRefString(pDoc, nTab));
        aFormulaBuf.append(aSep);
        aFormulaBuf.append(rParam.aRefRowCell.GetRefString(pDoc, nTab));
        aFormulaBuf.append(aSep);
        aRef.Set(nCol1, nRow1, nTab, true, false, true);
        aFormulaBuf.append(aRef.GetRefString(pDoc, nTab));
        ++nRow1;
        nRow2 = std::min(
            nRow2, rParam.aRefFormulaEnd.Row() - rParam.aRefFormulaCell.Row() + nRow1 + 1);
    }
    else // both
    {
        aFormulaBuf.append(rParam.aRefFormulaCell.GetRefString(pDoc, nTab));
        aFormulaBuf.append(aSep);
        aFormulaBuf.append(rParam.aRefColCell.GetRefString(pDoc, nTab));
        aFormulaBuf.append(aSep);
        aRef.Set(nCol1, nRow1 + 1, nTab, false, true, true);
        aFormulaBuf.append(aRef.GetRefString(pDoc, nTab));
        aFormulaBuf.append(aSep);
        aFormulaBuf.append(rParam.aRefRowCell.GetRefString(pDoc, nTab));
        aFormulaBuf.append(aSep);
        aRef.Set(nCol1 + 1, nRow1, nTab, true, false, true);
        aFormulaBuf.append(aRef.GetRefString(pDoc, nTab));
        ++nCol1;
        ++nRow1;
    }

    aFormulaBuf.append(ScCompiler::GetNativeSymbol(ocClose));

    ScFormulaCell aRefCell(
        pDoc, ScAddress(nCol1, nRow1, nTab), aFormulaBuf.makeStringAndClear(),
        formula::FormulaGrammar::GRAM_NATIVE, MM_NONE);

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
    {
        sc::ColumnBlockPosition* pBlockPos = mpImpl->getBlockPosition(nTab, nCol);

        if (!pBlockPos)
            // Something went horribly wrong.
            return;

        sc::CellStoreType& rColCells = pTab->aCol[nCol].maCells;

        for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
        {
            ScAddress aPos(nCol, nRow, nTab);
            ScFormulaCell* pCell = new ScFormulaCell(aRefCell, *pDoc, aPos);
            pBlockPos->miCellPos =
                rColCells.set(pBlockPos->miCellPos, nRow, pCell);
        }
    }
}

void ScDocumentImport::setAttrEntries( SCTAB nTab, SCCOL nCol, Attrs& rAttrs )
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(nTab);
    if (!pTab)
        return;

    ScColumn* pCol = pTab->FetchColumn(nCol);
    if (!pCol)
        return;

    ColAttr* pColAttr = mpImpl->getColAttr(nTab, nCol);
    if (pColAttr)
        pColAttr->mbLatinNumFmtOnly = rAttrs.mbLatinNumFmtOnly;

    pCol->pAttrArray->SetAttrEntries(rAttrs.mpData, rAttrs.mnSize);
}

void ScDocumentImport::setRowsVisible(SCTAB nTab, SCROW nRowStart, SCROW nRowEnd, bool bVisible)
{
    if (!bVisible)
    {
        getDoc().ShowRows(nRowStart, nRowEnd, nTab, false);
        getDoc().SetDrawPageSize(nTab);
        getDoc().UpdatePageBreaks( nTab );
    }
    else
    {
        assert(false);
    }
}

namespace {

class CellStoreInitializer
{
    // The pimpl pattern here is intentional.
    //
    // The problem with having the attributes in CellStoreInitializer
    // directly is that, as a functor, it might be copied around. In
    // that case miPos in _copied_ object points ot maAttrs in the
    // original object, not in the copy. So later, deep in mdds, we end
    // up comparing iterators from different sequences.
    //
    // This could be solved by defining copy constructor and operator=,
    // but given the limited usage of the class, I think it is simpler
    // to let copies share the state.
    struct Impl
    {
        sc::CellTextAttrStoreType maAttrs;
        sc::CellTextAttrStoreType::iterator miPos;
        SvtScriptType mnScriptNumeric;

        explicit Impl(const SvtScriptType nScriptNumeric)
            : maAttrs(MAXROWCOUNT), miPos(maAttrs.begin()), mnScriptNumeric(nScriptNumeric)
        {}
    };

    ScDocumentImportImpl& mrDocImpl;
    SCTAB mnTab;
    SCCOL mnCol;

public:
    CellStoreInitializer( ScDocumentImportImpl& rDocImpl, SCTAB nTab, SCCOL nCol ) :
        mrDocImpl(rDocImpl),
        mnTab(nTab),
        mnCol(nCol),
        mpImpl(new Impl(mrDocImpl.mnDefaultScriptNumeric))
    {}

    std::shared_ptr<Impl> mpImpl;

    void operator() (const sc::CellStoreType::value_type& node)
    {
        if (node.type == sc::element_type_empty)
            return;

        // Fill with default values for non-empty cell segments.
        sc::CellTextAttr aDefault;
        switch (node.type)
        {
            case sc::element_type_numeric:
            {
                aDefault.mnScriptType = mpImpl->mnScriptNumeric;
                const ColAttr* p = mrDocImpl.getColAttr(mnTab, mnCol);
                if (p && p->mbLatinNumFmtOnly)
                    aDefault.mnScriptType = SvtScriptType::LATIN;
            }
            break;
            case sc::element_type_formula:
            {
                const ColAttr* p = mrDocImpl.getColAttr(mnTab, mnCol);
                if (p && p->mbLatinNumFmtOnly)
                {
                    // We can assume latin script type if the block only
                    // contains formula cells with numeric results.
                    ScFormulaCell** pp = &sc::formula_block::at(*node.data, 0);
                    ScFormulaCell** ppEnd = pp + node.size;
                    bool bNumResOnly = true;
                    for (; pp != ppEnd; ++pp)
                    {
                        const ScFormulaCell& rCell = **pp;
                        if (!rCell.IsValueNoError())
                        {
                            bNumResOnly = false;
                            break;
                        }
                    }

                    if (bNumResOnly)
                        aDefault.mnScriptType = SvtScriptType::LATIN;
                }
            }
            break;
            default:
                ;
        }

        std::vector<sc::CellTextAttr> aDefaults(node.size, aDefault);
        mpImpl->miPos = mpImpl->maAttrs.set(mpImpl->miPos, node.position, aDefaults.begin(), aDefaults.end());

        if (node.type == sc::element_type_formula)
        {
            // Have all formula cells start listening to the document.
            ScFormulaCell** pp = &sc::formula_block::at(*node.data, 0);
            ScFormulaCell** ppEnd = pp + node.size;
            for (; pp != ppEnd; ++pp)
            {
                ScFormulaCell& rFC = **pp;
                if (rFC.IsSharedTop())
                {
                    // Register formula cells as a group.
                    sc::SharedFormulaUtil::startListeningAsGroup(mrDocImpl.maListenCxt, pp);
                    pp += rFC.GetSharedLength() - 1; // Move to the last one in the group.
                }
                else
                    rFC.StartListeningTo(mrDocImpl.maListenCxt);
            }
        }
    }

    void swap(sc::CellTextAttrStoreType& rAttrs)
    {
        mpImpl->maAttrs.swap(rAttrs);
    }
};

}

void ScDocumentImport::finalize()
{
    // Populate the text width and script type arrays in all columns. Also
    // activate all formula cells.
    ScDocument::TableContainer::iterator itTab = mpImpl->mrDoc.maTabs.begin(), itTabEnd = mpImpl->mrDoc.maTabs.end();
    for (; itTab != itTabEnd; ++itTab)
    {
        if (!*itTab)
            continue;

        ScTable& rTab = **itTab;
        SCCOL nNumCols = rTab.aCol.size();
        for (SCCOL nColIdx = 0; nColIdx < nNumCols; ++nColIdx)
            initColumn(rTab.aCol[nColIdx]);
    }

    mpImpl->mrDoc.finalizeOutlineImport();
}

void ScDocumentImport::initColumn(ScColumn& rCol)
{
    rCol.RegroupFormulaCells();

    CellStoreInitializer aFunc(*mpImpl, rCol.nTab, rCol.nCol);
    std::for_each(rCol.maCells.begin(), rCol.maCells.end(), aFunc);
    aFunc.swap(rCol.maCellTextAttrs);

    rCol.CellStorageModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
