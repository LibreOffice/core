/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <CoinMP.h>
#include <CoinError.hpp>

#include "CoinMPModel.hxx"

#include <memory>
#include <stdexcept>

using namespace com::sun::star;

CoinMpSolveResult coinmpSolveLinearModel(const uno::Sequence<table::CellAddress>& rVariables,
                                         const uno::Sequence<sheet::SolverConstraint>& rConstraints,
                                         const table::CellAddress& rObjective,
                                         const ScSolverCellHashMap& rCells,
                                         const std::vector<double>& rLowerBounds,
                                         const std::vector<double>& rUpperBounds,
                                         const std::vector<char>& rColumnType, bool bMaximize,
                                         double fTimeoutSeconds)
{
    CoinMpSolveResult aResult;

    const size_t nVariables = rVariables.size();
    size_t nVar = 0;

    // objective: slope of each variable, plus the constant term at the origin
    const std::vector<double>& rObjectCoefficients = rCells.at(rObjective);
    std::unique_ptr<double[]> pObjectCoefficients(new double[nVariables]);
    for (nVar = 0; nVar < nVariables; nVar++)
        pObjectCoefficients[nVar] = rObjectCoefficients[nVar + 1];
    double fObjectConstant = rObjectCoefficients[0];

    // rows: collect all coefficients row-wise first
    size_t nRows = rConstraints.getLength();
    size_t nDenseSize = nVariables * nRows;
    std::unique_ptr<double[]> pDenseMatrix(new double[nDenseSize]);
    for (size_t i = 0; i < nDenseSize; i++)
        pDenseMatrix[i] = 0.0;

    std::unique_ptr<double[]> pRHS(new double[nRows]);
    std::unique_ptr<char[]> pRowType(new char[nRows]);
    for (size_t i = 0; i < nRows; i++)
    {
        pRHS[i] = 0.0;
        pRowType[i] = 'N';
    }

    for (sal_Int32 nConstraintPosition = 0; nConstraintPosition < rConstraints.getLength();
         ++nConstraintPosition)
    {
        // integer and binary constraints are reflected in the column types
        sheet::SolverConstraintOperator eOperator = rConstraints[nConstraintPosition].Operator;
        if (eOperator == sheet::SolverConstraintOperator_LESS_EQUAL
            || eOperator == sheet::SolverConstraintOperator_GREATER_EQUAL
            || eOperator == sheet::SolverConstraintOperator_EQUAL)
        {
            double fDirectValue = 0.0;
            bool bRightCell = false;
            table::CellAddress aRightAddress;
            const cpo::uno::Any& rRightAny = rConstraints[nConstraintPosition].Right;
            if (rRightAny >>= aRightAddress)
                bRightCell = true;
            else
                rRightAny >>= fDirectValue;

            table::CellAddress aLeftAddress = rConstraints[nConstraintPosition].Left;

            const std::vector<double>& rLeftCoefficients = rCells.at(aLeftAddress);
            double* pValues = &pDenseMatrix[nConstraintPosition * nVariables];
            for (nVar = 0; nVar < nVariables; nVar++)
                pValues[nVar] = rLeftCoefficients[nVar + 1];

            // if the left hand cell has a constant term, move it to the right
            double fRightValue = -rLeftCoefficients[0];

            if (bRightCell)
            {
                const std::vector<double>& rRightCoefficients = rCells.at(aRightAddress);
                for (nVar = 0; nVar < nVariables; nVar++)
                    pValues[nVar] -= rRightCoefficients[nVar + 1];

                fRightValue += rRightCoefficients[0];
            }
            else
                fRightValue += fDirectValue;

            switch (eOperator)
            {
                case sheet::SolverConstraintOperator_LESS_EQUAL:
                    pRowType[nConstraintPosition] = 'L';
                    break;
                case sheet::SolverConstraintOperator_GREATER_EQUAL:
                    pRowType[nConstraintPosition] = 'G';
                    break;
                case sheet::SolverConstraintOperator_EQUAL:
                    pRowType[nConstraintPosition] = 'E';
                    break;
                default:
                    break;
            }
            pRHS[nConstraintPosition] = fRightValue;
        }
    }

    // turn the row-wise matrix into the column-wise form CoinMP wants
    std::unique_ptr<int[]> pMatrixBegin(new int[nVariables + 1]);
    std::unique_ptr<int[]> pMatrixCount(new int[nVariables]);
    std::unique_ptr<double[]> pMatrix(new double[nDenseSize]);
    std::unique_ptr<int[]> pMatrixIndex(new int[nDenseSize]);
    int nMatrixPosition = 0;
    for (nVar = 0; nVar < nVariables; nVar++)
    {
        int nBegin = nMatrixPosition;
        for (size_t nRow = 0; nRow < nRows; nRow++)
        {
            double fCoefficient = pDenseMatrix[nRow * nVariables + nVar];
            if (fCoefficient != 0.0)
            {
                pMatrix[nMatrixPosition] = fCoefficient;
                pMatrixIndex[nMatrixPosition] = nRow;
                ++nMatrixPosition;
            }
        }
        pMatrixBegin[nVar] = nBegin;
        pMatrixCount[nVar] = nMatrixPosition - nBegin;
    }
    pMatrixBegin[nVariables] = nMatrixPosition;
    pDenseMatrix.reset();

    std::unique_ptr<double[]> pLowerBounds(new double[nVariables]);
    std::unique_ptr<double[]> pUpperBounds(new double[nVariables]);
    std::unique_ptr<char[]> pColumnType(new char[nVariables]);
    for (nVar = 0; nVar < nVariables; nVar++)
    {
        pLowerBounds[nVar] = rLowerBounds[nVar];
        pUpperBounds[nVar] = rUpperBounds[nVar];
        pColumnType[nVar] = rColumnType[nVar];
    }

    int nObjectSense = bMaximize ? SOLV_OBJSENS_MAX : SOLV_OBJSENS_MIN;

    HPROB hProblem = CoinCreateProblem("");
    int nResult = CoinLoadProblem(hProblem, nVariables, nRows, nMatrixPosition, 0, nObjectSense,
                                  fObjectConstant, pObjectCoefficients.get(), pLowerBounds.get(),
                                  pUpperBounds.get(), pRowType.get(), pRHS.get(), nullptr,
                                  pMatrixBegin.get(), pMatrixCount.get(), pMatrixIndex.get(),
                                  pMatrix.get(), nullptr, nullptr, nullptr);
    if (nResult == SOLV_CALL_SUCCESS)
        nResult = CoinLoadInteger(hProblem, pColumnType.get());

    pColumnType.reset();
    pMatrixIndex.reset();
    pMatrix.reset();
    pMatrixCount.reset();
    pMatrixBegin.reset();
    pUpperBounds.reset();
    pLowerBounds.reset();
    pRowType.reset();
    pRHS.reset();
    pObjectCoefficients.reset();

    CoinSetRealOption(hProblem, COIN_REAL_MAXSECONDS, fTimeoutSeconds);
    CoinSetRealOption(hProblem, COIN_REAL_MIPMAXSEC, fTimeoutSeconds);

    if (nResult == SOLV_CALL_SUCCESS)
        nResult = CoinCheckProblem(hProblem);

    if (nResult == SOLV_CALL_SUCCESS)
    {
        try
        {
            nResult = CoinOptimizeProblem(hProblem, 0);
        }
        catch (const CoinError& rError)
        {
            CoinUnloadProblem(hProblem);
            throw std::runtime_error(rError.message());
        }
    }

    aResult.bSuccess = (nResult == SOLV_CALL_SUCCESS);
    if (aResult.bSuccess)
    {
        aResult.aSolution.resize(nVariables);
        CoinGetSolutionValues(hProblem, aResult.aSolution.data(), nullptr, nullptr, nullptr);
        aResult.fObjective = CoinGetObjectValue(hProblem);
    }
    else
    {
        aResult.nStatus = CoinGetSolutionStatus(hProblem);
    }

    CoinUnloadProblem(hProblem);

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
