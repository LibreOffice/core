/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>
#include "formulagroup.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "compiler.hxx"
#include "interpre.hxx"
#include "scmatrix.hxx"

#include "formula/vectortoken.hxx"

namespace sc {

ScMatrixRef FormulaGroupInterpreterSoftware::inverseMatrix(const ScMatrix& /*rMat*/)
{
    return ScMatrixRef();
}

bool FormulaGroupInterpreterSoftware::interpret(ScDocument& rDoc, const ScAddress& rTopPos,
                                                const ScFormulaCellGroupRef& xGroup,
                                                ScTokenArray& rCode)
{
    // Until we implement group calculation for real, decompose the group into
    // individual formula token arrays for individual calculation.
    ScAddress aTmpPos = rTopPos;
    for (sal_Int32 i = 0; i < xGroup->mnLength; ++i)
    {
        aTmpPos.SetRow(xGroup->mnStart + i);
        ScTokenArray aCode2;
        for (const formula::FormulaToken* p = rCode.First(); p; p = rCode.Next())
        {
            switch (p->GetType())
            {
                case formula::svSingleVectorRef:
                {
                    const formula::SingleVectorRefToken* p2 = static_cast<const formula::SingleVectorRefToken*>(p);
                    const double* pArray = p2->GetArray();
                    aCode2.AddDouble(static_cast<size_t>(i) < p2->GetArrayLength() ? pArray[i] : 0.0);
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

        ScFormulaCell* pDest = rDoc.GetFormulaCell(aTmpPos);
        if (!pDest)
            return false;

        ScCompiler aComp(&rDoc, aTmpPos, aCode2);
        aComp.SetGrammar(rDoc.GetGrammar());
        aComp.CompileTokenArray(); // Create RPN token array.
        ScInterpreter aInterpreter(pDest, &rDoc, aTmpPos, aCode2);
        aInterpreter.Interpret();
        pDest->SetResultToken(aInterpreter.GetResultToken().get());
        pDest->ResetDirty();
        pDest->SetChanged(true);
    } // for loop end (xGroup->mnLength)

    return true;
}

// TODO: load module, hook symbol out, check it works, UI on failure etc.
namespace opencl {
    extern sc::FormulaGroupInterpreter *createFormulaGroupInterpreter();
}

FormulaGroupInterpreter *FormulaGroupInterpreter::msInstance = NULL;

/// load and/or configure the correct formula group interpreter
FormulaGroupInterpreter *FormulaGroupInterpreter::getStatic()
{
    static bool bOpenCLEnabled = false;

    if ( msInstance &&
         bOpenCLEnabled != ScInterpreter::GetGlobalConfig().mbOpenCLEnabled )
    {
        delete msInstance;
        msInstance = NULL;
    }

    if ( !msInstance )
    {
#if HAVE_FEATURE_OPENCL
        if ( ScInterpreter::GetGlobalConfig().mbOpenCLEnabled )
            msInstance = sc::opencl::createFormulaGroupInterpreter();
#endif
        if ( !msInstance ) // software fallback
            msInstance = new sc::FormulaGroupInterpreterSoftware();
    }

    return msInstance;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
