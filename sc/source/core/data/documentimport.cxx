/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
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

#include "svl/sharedstringpool.hxx"

struct ScDocumentImportImpl
{
    ScDocument& mrDoc;
    sc::StartListeningContext maListenCxt;
    sc::ColumnBlockPositionSet maBlockPosSet;
    sal_uInt16 mnDefaultScriptNumeric;

    ScDocumentImportImpl(ScDocument& rDoc) :
        mrDoc(rDoc),
        maListenCxt(rDoc),
        maBlockPosSet(rDoc),
        mnDefaultScriptNumeric(SC_SCRIPTTYPE_UNKNOWN) {}
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

void ScDocumentImport::setCellStyleToSheet(SCTAB nTab, const ScStyleSheet& rStyle)
{
    ScTable* pTab = mpImpl->mrDoc.FetchTable(nTab);
    if (!pTab)
        return;

    pTab->ApplyStyleArea(0, 0, MAXCOL, MAXROW, rStyle);
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

    sc::ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rPos.Tab(), rPos.Col());

    if (!pBlockPos)
        return;

    ScCellValue aCell;
    pTab->aCol[rPos.Col()].ParseString(
        aCell, rPos.Row(), rPos.Tab(), rStr, mpImpl->mrDoc.GetAddressConvention(), pStringParam);

    sc::CellStoreType& rCells = pTab->aCol[rPos.Col()].maCells;
    switch (aCell.meType)
    {
        case CELLTYPE_STRING:
            
            pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), *aCell.mpString);
        break;
        case CELLTYPE_EDIT:
            
            pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), aCell.mpEditText);
            aCell.mpEditText = NULL;
        break;
        case CELLTYPE_VALUE:
            pBlockPos->miCellPos = rCells.set(pBlockPos->miCellPos, rPos.Row(), aCell.mfValue);
        break;
        case CELLTYPE_FORMULA:
            
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

    sc::ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rPos.Tab(), rPos.Col());

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

    sc::ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rPos.Tab(), rPos.Col());

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

    sc::ColumnBlockPosition* pBlockPos =
        mpImpl->maBlockPosSet.getBlockPosition(rPos.Tab(), rPos.Col());

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

    
    ScFormulaCell* pCell = new ScFormulaCell(&mpImpl->mrDoc, rBasePos, rArray, eGram, MM_FORMULA);

    pBlockPos->miCellPos =
        rCells.set(pBlockPos->miCellPos, rBasePos.Row(), pCell);

    
    pCell->SetMatColsRows(
        rRange.aEnd.Col()-rRange.aStart.Col()+1, rRange.aEnd.Row()-rRange.aStart.Row()+1, true);

    
    ScSingleRefData aRefData;
    aRefData.InitFlags();
    aRefData.SetColRel(true);
    aRefData.SetRowRel(true);
    aRefData.SetTabRel(true);
    aRefData.SetAddress(rBasePos, rBasePos);

    ScTokenArray aArr; 
    ScToken* t = static_cast<ScToken*>(aArr.AddMatrixSingleReference(aRefData));

    ScAddress aPos = rBasePos;
    for (SCROW nRow = rRange.aStart.Row()+1; nRow <= rRange.aEnd.Row(); ++nRow)
    {
        
        aPos.SetRow(nRow);
        
        aRefData.SetAddress(rBasePos, aPos);
        t->GetSingleRef() = aRefData;
        boost::scoped_ptr<ScTokenArray> pTokArr(aArr.Clone());
        pCell = new ScFormulaCell(&mpImpl->mrDoc, aPos, *pTokArr, eGram, MM_REFERENCE);
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
    OUStringBuffer aFormulaBuf('=');
    aFormulaBuf.append(ScCompiler::GetNativeSymbol(ocTableOp));
    aFormulaBuf.append(ScCompiler::GetNativeSymbol(ocOpen));

    OUString aSep = ScCompiler::GetNativeSymbol(ocSep);
    if (rParam.meMode == ScTabOpParam::Column) 
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
    else if (rParam.meMode == ScTabOpParam::Row) 
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
    else 
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
        sc::ColumnBlockPosition* pBlockPos =
            mpImpl->maBlockPosSet.getBlockPosition(nTab, nCol);

        if (!pBlockPos)
            
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

namespace {

class CellStoreInitializer
{
    
    //
    
    
    
    
    
    //
    
    
    
    struct Impl
    {
        sc::CellTextAttrStoreType maAttrs;
        sc::CellTextAttrStoreType::iterator miPos;
        sal_uInt16 mnScriptNumeric;

        Impl(const sal_uInt32 nMaxRowCount, const sal_uInt16 nScriptNumeric)
            : maAttrs(nMaxRowCount), miPos(maAttrs.begin()), mnScriptNumeric(nScriptNumeric)
        {}
    };

    ScDocument& mrDoc;
    sc::StartListeningContext& mrListenCxt;

public:
    CellStoreInitializer(ScDocument& rDoc, sc::StartListeningContext& rCxt, sal_uInt16 nScriptNumeric) :
        mrDoc(rDoc),
        mrListenCxt(rCxt),
        mpImpl(new Impl(MAXROWCOUNT, nScriptNumeric))
    {}

    boost::shared_ptr<Impl> mpImpl;

    void operator() (const sc::CellStoreType::value_type& node)
    {
        if (node.type == sc::element_type_empty)
            return;

        
        sc::CellTextAttr aDefault;
        if (node.type == sc::element_type_numeric)
            aDefault.mnScriptType = mpImpl->mnScriptNumeric;
        std::vector<sc::CellTextAttr> aDefaults(node.size, aDefault);
        mpImpl->miPos = mpImpl->maAttrs.set(mpImpl->miPos, node.position, aDefaults.begin(), aDefaults.end());

        if (node.type == sc::element_type_formula)
        {
            
            sc::formula_block::iterator it = sc::formula_block::begin(*node.data);
            sc::formula_block::iterator itEnd = sc::formula_block::end(*node.data);
            for (; it != itEnd; ++it)
            {
                ScFormulaCell& rFC = **it;
                rFC.StartListeningTo(mrListenCxt);
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
    CellStoreInitializer aFunc(mpImpl->mrDoc, mpImpl->maListenCxt, mpImpl->mnDefaultScriptNumeric);
    std::for_each(rCol.maCells.begin(), rCol.maCells.end(), aFunc);
    aFunc.swap(rCol.maCellTextAttrs);
    rCol.RegroupFormulaCells();
    rCol.CellStorageModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
