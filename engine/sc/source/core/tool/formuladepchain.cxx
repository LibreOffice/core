/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <formuladepchain.hxx>
#include <document.hxx>
#include <externalrefmgr.hxx>
#include <formulacell.hxx>
#include <tokenarray.hxx>
#include <token.hxx>
#include <refdata.hxx>
#include <global.hxx>

#include <formula/token.hxx>
#include <formula/errorcodes.hxx>
#include <tools/json_writer.hxx>

#include <set>

namespace
{
constexpr int MAX_DEPTH = 10;
constexpr int MAX_CELLS = 50;
constexpr int MAX_RANGE_CELLS = 20;
constexpr int MAX_RANGE_SCAN = 1000;
constexpr int MAX_ERROR_CELLS = 5;

class DepChainWriter
{
    ScDocument& m_rDoc;
    tools::JsonWriter& m_rJsonWriter;
    std::set<ScAddress> m_aVisited;
    int m_nCellCount = 0;

public:
    DepChainWriter(ScDocument& rDoc, tools::JsonWriter& rJsonWriter)
        : m_rDoc(rDoc)
        , m_rJsonWriter(rJsonWriter)
    {
    }

    void addVisited(const ScAddress& rPos) { m_aVisited.insert(rPos); }

    void writeDependencies(ScTokenArray* pCode, const ScAddress& rPos, int nDepth);
    void writeCellInfo(const ScAddress& rPos, int nDepth);

private:
    void writeExternalRefInfo(formula::FormulaToken* pToken);
};

void DepChainWriter::writeExternalRefInfo(formula::FormulaToken* pToken)
{
    m_rJsonWriter.put("type", "external");

    OUString aTabName = pToken->GetString().getString();
    if (!aTabName.isEmpty())
        m_rJsonWriter.put("sheet", aTabName);

    sal_uInt16 nFileId = pToken->GetIndex();
    ScExternalRefManager* pRefMgr = m_rDoc.GetExternalRefManager();
    if (pRefMgr)
    {
        const OUString* pFileName = pRefMgr->getExternalFileName(nFileId);
        if (pFileName)
            m_rJsonWriter.put("file", *pFileName);
    }
}

void DepChainWriter::writeDependencies(ScTokenArray* pCode, const ScAddress& rPos, int nDepth)
{
    if (!pCode)
        return;

    formula::FormulaTokenArrayPlainIterator aIter(*pCode);
    bool bHasDeps = (aIter.GetNextReference() != nullptr);

    if (!bHasDeps)
        return;

    auto aDeps = m_rJsonWriter.startArray("dependencies");
    aIter.Reset();
    formula::FormulaToken* pToken;

    while ((pToken = aIter.GetNextReference()) != nullptr)
    {
        if (m_nCellCount > MAX_CELLS)
            break;

        if (pToken->GetType() == formula::svSingleRef)
        {
            const ScSingleRefData* pRef = pToken->GetSingleRef();
            ScAddress aAbs = pRef->toAbs(m_rDoc, rPos);

            if (aAbs.IsValid())
            {
                auto aStruct = m_rJsonWriter.startStruct();
                m_nCellCount++;
                writeCellInfo(aAbs, nDepth + 1);
            }
        }
        else if (pToken->GetType() == formula::svDoubleRef)
        {
            const ScComplexRefData& rRef = static_cast<ScDoubleRefToken*>(pToken)->GetDoubleRef();
            ScRange aRange = rRef.toAbs(m_rDoc, rPos);

            SCROW nRows = aRange.aEnd.Row() - aRange.aStart.Row() + 1;
            SCCOL nCols = aRange.aEnd.Col() - aRange.aStart.Col() + 1;
            sal_Int64 nRangeSize = static_cast<sal_Int64>(nRows) * nCols;

            if (nRangeSize <= MAX_RANGE_CELLS)
            {
                // Enumerate individual cells
                for (SCTAB nTab = aRange.aStart.Tab();
                     nTab <= aRange.aEnd.Tab() && m_nCellCount <= MAX_CELLS; ++nTab)
                {
                    for (SCCOL nCol = aRange.aStart.Col();
                         nCol <= aRange.aEnd.Col() && m_nCellCount <= MAX_CELLS; ++nCol)
                    {
                        for (SCROW nRow = aRange.aStart.Row();
                             nRow <= aRange.aEnd.Row() && m_nCellCount <= MAX_CELLS; ++nRow)
                        {
                            ScAddress aCellAddr(nCol, nRow, nTab);
                            auto aStruct = m_rJsonWriter.startStruct();
                            m_nCellCount++;
                            writeCellInfo(aCellAddr, nDepth + 1);
                        }
                    }
                }
            }
            else
            {
                // Summarize the range
                auto aStruct = m_rJsonWriter.startStruct();
                OUString aRangeStr = aRange.Format(m_rDoc, ScRefFlags::VALID | ScRefFlags::TAB_3D);
                m_rJsonWriter.put("address", aRangeStr);
                m_rJsonWriter.put("type", "range");
                m_rJsonWriter.put("rows", static_cast<sal_Int64>(nRows));
                m_rJsonWriter.put("cols", static_cast<sal_Int64>(nCols));

                // Scan for error cells within the range (bounded)
                OUString aErrorCells;
                int nErrorCount = 0;
                int nScanCount = 0;
                for (SCCOL nCol = aRange.aStart.Col();
                     nCol <= aRange.aEnd.Col() && nErrorCount < MAX_ERROR_CELLS
                     && nScanCount < MAX_RANGE_SCAN;
                     ++nCol)
                {
                    for (SCROW nRow = aRange.aStart.Row();
                         nRow <= aRange.aEnd.Row() && nErrorCount < MAX_ERROR_CELLS
                         && nScanCount < MAX_RANGE_SCAN;
                         ++nRow)
                    {
                        nScanCount++;
                        ScAddress aCellAddr(nCol, nRow, aRange.aStart.Tab());
                        ScFormulaCell* pRefCell = m_rDoc.GetFormulaCell(aCellAddr);
                        if (pRefCell && pRefCell->GetRawError() != FormulaError::NONE)
                        {
                            if (!aErrorCells.isEmpty())
                                aErrorCells += ", ";
                            aErrorCells += aCellAddr.Format(ScRefFlags::VALID, &m_rDoc);
                            nErrorCount++;
                        }
                    }
                }
                if (!aErrorCells.isEmpty())
                    m_rJsonWriter.put("errorCells", aErrorCells);
                if (nScanCount >= MAX_RANGE_SCAN && nRangeSize > MAX_RANGE_SCAN)
                    m_rJsonWriter.put("scanLimited", true);
            }
        }
        else if (pToken->GetType() == formula::svExternalSingleRef
                 || pToken->GetType() == formula::svExternalDoubleRef)
        {
            auto aStruct = m_rJsonWriter.startStruct();
            writeExternalRefInfo(pToken);
        }
    }
}

void DepChainWriter::writeCellInfo(const ScAddress& rPos, int nDepth)
{
    if (nDepth > MAX_DEPTH || m_nCellCount > MAX_CELLS)
    {
        m_rJsonWriter.put("address", rPos.Format(ScRefFlags::VALID, &m_rDoc));
        m_rJsonWriter.put("type", "truncated");
        return;
    }

    if (m_aVisited.count(rPos))
    {
        m_rJsonWriter.put("address", rPos.Format(ScRefFlags::VALID, &m_rDoc));
        m_rJsonWriter.put("type", "circular");
        return;
    }

    m_aVisited.insert(rPos);

    OUString aAddr = rPos.Format(ScRefFlags::VALID, &m_rDoc);
    m_rJsonWriter.put("address", aAddr);

    OUString aTabName;
    m_rDoc.GetName(rPos.Tab(), aTabName);
    m_rJsonWriter.put("sheet", aTabName);

    CellType eCellType = m_rDoc.GetCellType(rPos);

    if (eCellType == CELLTYPE_FORMULA)
    {
        ScFormulaCell* pCell = m_rDoc.GetFormulaCell(rPos);
        if (!pCell)
        {
            m_rJsonWriter.put("type", "error");
            m_aVisited.erase(rPos);
            return;
        }

        m_rJsonWriter.put("type", "formula");
        m_rJsonWriter.put("formula", pCell->GetFormula());

        FormulaError nErr = pCell->GetRawError();
        if (nErr != FormulaError::NONE)
        {
            m_rJsonWriter.put("error", ScGlobal::GetErrorString(nErr));
            m_rJsonWriter.put("errorCode", static_cast<int>(nErr));
        }
        else
        {
            if (pCell->IsValue())
                m_rJsonWriter.put("value", pCell->GetRawValue());
            else
                m_rJsonWriter.put("value", pCell->GetRawString().getString());
        }

        writeDependencies(pCell->GetCode(), rPos, nDepth);
    }
    else if (eCellType == CELLTYPE_VALUE)
    {
        m_rJsonWriter.put("type", "value");
        m_rJsonWriter.put("value", m_rDoc.GetValue(rPos));
    }
    else if (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT)
    {
        m_rJsonWriter.put("type", "string");
        m_rJsonWriter.put("value", m_rDoc.GetString(rPos));
    }
    else
    {
        m_rJsonWriter.put("type", "empty");
    }

    m_aVisited.erase(rPos);
}

} // anonymous namespace

namespace sc
{
void getFormulaDependencyChain(ScDocument& rDoc, const ScAddress& rPos,
                               tools::JsonWriter& rJsonWriter)
{
    ScFormulaCell* pCell = rDoc.GetFormulaCell(rPos);
    if (!pCell)
    {
        rJsonWriter.put("hasError", false);
        rJsonWriter.put("reason", "not_formula");
        return;
    }

    FormulaError nErr = pCell->GetRawError();
    if (nErr == FormulaError::NONE)
    {
        rJsonWriter.put("hasError", false);
        rJsonWriter.put("reason", "no_error");
        return;
    }

    rJsonWriter.put("hasError", true);

    // Write root cell info
    {
        auto aCell = rJsonWriter.startNode("cell");
        OUString aAddr = rPos.Format(ScRefFlags::VALID, &rDoc);
        rJsonWriter.put("address", aAddr);

        OUString aTabName;
        rDoc.GetName(rPos.Tab(), aTabName);
        rJsonWriter.put("sheet", aTabName);

        rJsonWriter.put("formula", pCell->GetFormula());
        rJsonWriter.put("error", ScGlobal::GetErrorString(nErr));
        rJsonWriter.put("errorCode", static_cast<int>(nErr));
    }

    // Walk dependencies
    DepChainWriter aWriter(rDoc, rJsonWriter);
    aWriter.addVisited(rPos);
    aWriter.writeDependencies(pCell->GetCode(), rPos, 0);
}

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
