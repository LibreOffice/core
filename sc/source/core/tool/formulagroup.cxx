/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formulagroup.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "compiler.hxx"
#include "interpre.hxx"

#include "formula/vectortoken.hxx"

namespace sc {

FormulaGroupInterpreter::FormulaGroupInterpreter(
    ScDocument& rDoc, const ScAddress& rTopPos, const ScFormulaCellGroupRef& xGroup, ScTokenArray& rCode) :
    mrDoc(rDoc), maTopPos(rTopPos), mxGroup(xGroup), mrCode(rCode) {}

bool FormulaGroupInterpreter::interpret()
{
    // Until we implement group calculation for real, decompose the group into
    // individual formula token arrays for individual calculation.
    ScAddress aTmpPos = maTopPos;
    for (sal_Int32 i = 0; i < mxGroup->mnLength; ++i)
    {
        aTmpPos.SetRow(mxGroup->mnStart + i);
        ScTokenArray aCode2;
        for (const formula::FormulaToken* p = mrCode.First(); p; p = mrCode.Next())
        {
            switch (p->GetType())
            {
                case formula::svSingleVectorRef:
                {
                    const formula::SingleVectorRefToken* p2 = static_cast<const formula::SingleVectorRefToken*>(p);
                    const double* pArray = p2->GetArray();
                    aCode2.AddDouble(i < p2->GetArrayLength() ? pArray[i] : 0.0);
                }
                break;
                case formula::svDoubleVectorRef:
                {
                    const formula::DoubleVectorRefToken* p2 = static_cast<const formula::DoubleVectorRefToken*>(p);
                    const std::vector<const double*>& rArrays = p2->GetArrays();
                    size_t nColSize = rArrays.size();
                    size_t nRowStart = p2->IsStartFixed() ? 0 : i;
                    size_t nRowEnd = p2->GetRefRowSize() - 1;
                    if (!p2->IsEndFixed())
                        nRowEnd += i;

                    size_t nRowSize = nRowEnd - nRowStart + 1;
                    ScMatrixRef pMat(new ScMatrix(nColSize, nRowSize, 0.0));
                    for (size_t nCol = 0; nCol < nColSize; ++nCol)
                    {
                        const double* pArray = rArrays[nCol];
                        for (size_t nRow = 0; nRow < nRowSize; ++nRow)
                        {
                            if (nRowStart + nRow < p2->GetArrayLength())
                            {
                                double fVal = pArray[nRowStart+nRow];
                                pMat->PutDouble(fVal, nCol, nRow);
                            }
                        }
                    }

                    ScMatrixToken aTok(pMat);
                    aCode2.AddToken(aTok);
                }
                break;
                default:
                    aCode2.AddToken(*p);
            }
        }

        ScFormulaCell* pDest = mrDoc.GetFormulaCell(aTmpPos);
        if (!pDest)
            return false;

        ScCompiler aComp(&mrDoc, aTmpPos, aCode2);
        aComp.SetGrammar(mrDoc.GetGrammar());
        aComp.CompileTokenArray(); // Create RPN token array.
        ScInterpreter aInterpreter(pDest, &mrDoc, aTmpPos, aCode2);
        aInterpreter.Interpret();

        pDest->SetResultToken(aInterpreter.GetResultToken().get());
        pDest->ResetDirty();
        pDest->SetChanged(true);
    }

    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
