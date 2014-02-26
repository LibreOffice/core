/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <formula/token.hxx>
#include <formula/vectortoken.hxx>

#include "compiler.hxx"
#include "grouptokenconverter.hxx"

using namespace formula;

bool ScGroupTokenConverter::isSelfReferenceRelative(const ScAddress& rRefPos, SCROW nRelRow)
{
    if (rRefPos.Col() != mrPos.Col())
        return false;

    SCROW nLen = mrCell.GetCellGroup()->mnLength;
    SCROW nEndRow = mrPos.Row() + nLen - 1;

    if (nRelRow < 0)
    {
        SCROW nTest = nEndRow;
        nTest += nRelRow;
        if (nTest >= mrPos.Row())
            return true;
    }
    else if (nRelRow > 0)
    {
        SCROW nTest = mrPos.Row(); // top row.
        nTest += nRelRow;
        if (nTest <= nEndRow)
            return true;
    }

    return false;
}

bool ScGroupTokenConverter::isSelfReferenceAbsolute(const ScAddress& rRefPos)
{
    if (rRefPos.Col() != mrPos.Col())
        return false;

    SCROW nLen = mrCell.GetCellGroup()->mnLength;
    SCROW nEndRow = mrPos.Row() + nLen - 1;

    if (rRefPos.Row() < mrPos.Row())
        return false;

    if (rRefPos.Row() > nEndRow)
        return false;

    return true;
}

SCROW ScGroupTokenConverter::trimLength(SCTAB nTab, SCCOL nCol1, SCCOL nCol2, SCROW nRow, SCROW nRowLen)
{
    SCROW nLastRow = nRow + nRowLen - 1; // current last row.
    nLastRow = mrDoc.GetLastDataRow(nTab, nCol1, nCol2, nLastRow);
    if (nLastRow < (nRow + nRowLen - 1))
        nRowLen = nLastRow - nRow + 1;
    else if (nLastRow == 0)
        // Column is empty.
        nRowLen = 1;

    return nRowLen;
}

ScGroupTokenConverter::ScGroupTokenConverter(ScTokenArray& rGroupTokens, ScDocument& rDoc, ScFormulaCell& rCell, const ScAddress& rPos) :
        mrGroupTokens(rGroupTokens), mrDoc(rDoc), mrCell(rCell), mrPos(rPos)

{
}

bool ScGroupTokenConverter::convert(ScTokenArray& rCode)
{
#if 0
    { // debug to start with:
        ScCompiler aComp( &mrDoc, mrPos, rCode);
        aComp.SetGrammar(formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1);
        OUStringBuffer aAsString;
        aComp.CreateStringFromTokenArray(aAsString);
    }
#endif

    rCode.Reset();
    for (const formula::FormulaToken* p = rCode.First(); p; p = rCode.Next())
    {
        // A reference can be either absolute or relative.  If it's absolute,
        // convert it to a static value token.  If relative, convert it to a
        // vector reference token.  Note: we only care about relative vs
        // absolute reference state for row directions.

        const ScToken* pToken = static_cast<const ScToken*>(p);
        SCROW nLen = mrCell.GetCellGroup()->mnLength;
        switch (pToken->GetType())
        {
            case svSingleRef:
            {
                ScSingleRefData aRef = pToken->GetSingleRef();
                ScAddress aRefPos = aRef.toAbs(mrPos);
                if (aRef.IsRowRel())
                {
                    if (isSelfReferenceRelative(aRefPos, aRef.Row()))
                        return false;

                    // Trim data array length to actual data range.
                    SCROW nTrimLen = trimLength(aRefPos.Tab(), aRefPos.Col(), aRefPos.Col(), aRefPos.Row(), nLen);
                    // Fetch double array guarantees that the length of the
                    // returned array equals or greater than the requested
                    // length.

                    formula::VectorRefArray aArray;
                    if (nTrimLen)
                        aArray = mrDoc.FetchVectorRefArray(aRefPos, nTrimLen);

                    formula::SingleVectorRefToken aTok(aArray, nLen, nTrimLen);
                    mrGroupTokens.AddToken(aTok);
                }
                else
                {
                    // Absolute row reference.
                    if (isSelfReferenceAbsolute(aRefPos))
                        return false;

                    formula::FormulaTokenRef pNewToken = mrDoc.ResolveStaticReference(aRefPos);
                    if (!pNewToken)
                        return false;

                    mrGroupTokens.AddToken(*pNewToken);
                }
            }
            break;
            case svDoubleRef:
            {
                ScComplexRefData aRef = pToken->GetDoubleRef();
                ScRange aAbs = aRef.toAbs(mrPos);

                // Check for self reference.
                if (aRef.Ref1.IsRowRel())
                {
                    if (isSelfReferenceRelative(aAbs.aStart, aRef.Ref1.Row()))
                        return false;
                }
                else if (isSelfReferenceAbsolute(aAbs.aStart))
                    return false;

                if (aRef.Ref2.IsRowRel())
                {
                    if (isSelfReferenceRelative(aAbs.aEnd, aRef.Ref2.Row()))
                        return false;
                }
                else if (isSelfReferenceAbsolute(aAbs.aEnd))
                    return false;

                // Row reference is relative.
                bool bAbsFirst = !aRef.Ref1.IsRowRel();
                bool bAbsLast = !aRef.Ref2.IsRowRel();
                ScAddress aRefPos = aAbs.aStart;
                size_t nCols = aAbs.aEnd.Col() - aAbs.aStart.Col() + 1;
                std::vector<formula::VectorRefArray> aArrays;
                aArrays.reserve(nCols);
                SCROW nRefRowSize = aAbs.aEnd.Row() - aAbs.aStart.Row() + 1;
                SCROW nArrayLength = nRefRowSize;
                if (!bAbsLast)
                {
                    // range end position is relative. Extend the array length.
                    SCROW nLastRefRowOffset = aAbs.aEnd.Row() - mrPos.Row();
                    SCROW nLastRefRow = mrPos.Row() + nLen - 1 + nLastRefRowOffset;
                    SCROW nNewLength = nLastRefRow - aAbs.aStart.Row() + 1;
                    if (nNewLength > nArrayLength)
                        nArrayLength = nNewLength;
                }

                // Trim trailing empty rows.
                SCROW nRequestedLength = nArrayLength; // keep the original length.
                nArrayLength = trimLength(aRefPos.Tab(), aAbs.aStart.Col(), aAbs.aEnd.Col(), aRefPos.Row(), nArrayLength);

                for (SCCOL i = aAbs.aStart.Col(); i <= aAbs.aEnd.Col(); ++i)
                {
                    aRefPos.SetCol(i);
                    formula::VectorRefArray aArray;
                    if (nArrayLength)
                        aArray = mrDoc.FetchVectorRefArray(aRefPos, nArrayLength);

                    aArrays.push_back(aArray);
                }

                formula::DoubleVectorRefToken aTok(aArrays, nRequestedLength, nArrayLength, nRefRowSize, bAbsFirst, bAbsLast);
                mrGroupTokens.AddToken(aTok);
            }
            break;
            case svIndex:
            {
                // Named range.
                ScRangeName* pNames = mrDoc.GetRangeName();
                if (!pNames)
                    // This should never fail.
                    return false;

                ScRangeData* pRange = pNames->findByIndex(p->GetIndex());
                if (!pRange)
                    // No named range exists by that index.
                    return false;

                ScTokenArray* pNamedTokens = pRange->GetCode();
                if (!pNamedTokens)
                    // This named range is empty.
                    return false;

                mrGroupTokens.AddOpCode(ocOpen);

                if (!convert(*pNamedTokens))
                    return false;

                mrGroupTokens.AddOpCode(ocClose);
            }
            break;
            default:
                mrGroupTokens.AddToken(*pToken);
        }
    }

    ScCompiler aComp(&mrDoc, mrPos, mrGroupTokens);
    aComp.SetGrammar(mrDoc.GetGrammar());
    aComp.CompileTokenArray(); // Regenerate RPN tokens.

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
