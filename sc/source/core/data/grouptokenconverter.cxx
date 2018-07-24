/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <grouptokenconverter.hxx>
#include <compiler.hxx>

#include <formula/token.hxx>
#include <formula/vectortoken.hxx>

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
    {
        // This can end up negative! Was that the original intent, or
        // is it accidental? Was it not like that originally but the
        // surrounding conditions changed?
        nRowLen = nLastRow - nRow + 1;
        // Anyway, let's assume it doesn't make sense to return a
        // negative value here. But should we then return 0 or 1? In
        // the "Column is empty" case below, we return 1, why!? And,
        // at the callsites there are tests for a zero value returned
        // from this function (but not for a negative one).
        if (nRowLen < 0)
            nRowLen = 0;
    }
    else if (nLastRow == 0)
        // Column is empty.
        nRowLen = 1;

    return nRowLen;
}

ScGroupTokenConverter::ScGroupTokenConverter(
    ScTokenArray& rGroupTokens, ScDocument& rDoc, const ScFormulaCell& rCell, const ScAddress& rPos) :
    mrGroupTokens(rGroupTokens),
    mrDoc(rDoc),
    mrCell(rCell),
    mrPos(rPos)
{
}

bool ScGroupTokenConverter::convert( const ScTokenArray& rCode, sc::FormulaLogger::GroupScope& rScope )
{
#if 0
    { // debug to start with:
        ScCompiler aComp( &mrDoc, mrPos, rCode, formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1);
        OUStringBuffer aAsString;
        aComp.CreateStringFromTokenArray(aAsString);
    }
#endif

    const SCROW nLen = mrCell.GetCellGroup()->mnLength;
    formula::FormulaTokenArrayPlainIterator aIter(rCode);
    for (const formula::FormulaToken* p = aIter.First(); p; p = aIter.Next())
    {
        // A reference can be either absolute or relative.  If it's absolute,
        // convert it to a static value token.  If relative, convert it to a
        // vector reference token.  Note: we only care about relative vs
        // absolute reference state for row directions.

        switch (p->GetType())
        {
            case svSingleRef:
            {
                ScSingleRefData aRef = *p->GetSingleRef();
                if( aRef.IsDeleted())
                    return false;
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

                    if (!aArray.isValid())
                        return false;

                    formula::SingleVectorRefToken aTok(aArray, nTrimLen);
                    mrGroupTokens.AddToken(aTok);
                    rScope.addRefMessage(mrPos, aRefPos, nLen, aArray);

                    if (nTrimLen && !mxFormulaGroupContext)
                    {
                        //tdf#98880 if the SingleVectorRefToken relies on the
                        //underlying storage provided by the Document
                        //FormulaGroupContext, take a reference to it here to
                        //ensure that backing storage exists for our lifetime
                        mxFormulaGroupContext = mrDoc.GetFormulaGroupContext();
                    }
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
                    rScope.addRefMessage(mrPos, aRefPos, *pNewToken);
                }
            }
            break;
            case svDoubleRef:
            {
                // This code may break in case of implicit intersection, leading to unnecessarily large
                // matrix operations and possibly incorrect results (=C:C/D:D). That is handled by
                // having ScCompiler check that there are no possible implicit intersections.
                // Additionally some functions such as INDEX() and OFFSET() require a reference,
                // that is handled by blacklisting those opcodes in ScTokenArray::CheckToken().

                ScComplexRefData aRef = *p->GetDoubleRef();
                if( aRef.IsDeleted())
                    return false;
                ScRange aAbs = aRef.toAbs(mrPos);

                // Multiple sheets not handled by vector/matrix.
                if (aRef.Ref1.Tab() != aRef.Ref2.Tab())
                    return false;

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

                    if (!aArray.isValid())
                        return false;

                    aArrays.push_back(aArray);
                }

                formula::DoubleVectorRefToken aTok(aArrays, nArrayLength, nRefRowSize, bAbsFirst, bAbsLast);
                mrGroupTokens.AddToken(aTok);
                rScope.addRefMessage(mrPos, aAbs.aStart, nRequestedLength, aArrays);

                if (nArrayLength && !aArrays.empty() && !mxFormulaGroupContext)
                {
                    //tdf#98880 if the DoubleVectorRefToken relies on the
                    //underlying storage provided by the Document
                    //FormulaGroupContext, take a reference to it here to
                    //ensure that backing storage exists for our lifetime
                    mxFormulaGroupContext = mrDoc.GetFormulaGroupContext();
                }
            }
            break;
            case svIndex:
            {
                if (p->GetOpCode() != ocName)
                {
                    // May be DB-range or TableRef
                    mrGroupTokens.AddToken(*p);
                    break;
                }

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

                if (!convert(*pNamedTokens, rScope))
                    return false;

                mrGroupTokens.AddOpCode(ocClose);
            }
            break;
            default:
                mrGroupTokens.AddToken(*p);
        }
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
