/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#if defined _MSC_VER
#pragma warning(push, 0)
#endif
#include <CbcModel.hpp>
#include <CbcSolver.hpp>
#include <ClpSimplex.hpp>
#include <CoinError.hpp>
#include <CoinFinite.hpp>
#include <OsiClpSolverInterface.hpp>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include "CoinMPModel.hxx"

#include <iterator>
#include <memory>
#include <stdexcept>

using namespace com::sun::star;

namespace
{
int noCallBack(CbcModel*, int) { return 0; }

// Clp's simplex method solves a problem without integer columns.
void solveContinuous(OsiClpSolverInterface& rSolver, size_t nVariables, double fTimeoutSeconds,
                     CoinMpSolveResult& rResult)
{
    ClpSimplex* pSimplex = rSolver.getModelPtr();
    pSimplex->setLogLevel(0);
    pSimplex->setMaximumSeconds(fTimeoutSeconds);
    pSimplex->initialSolve();

    if (pSimplex->isProvenOptimal())
        rResult.eStatus = CoinMpSolveStatus::Optimal;
    else if (pSimplex->isProvenPrimalInfeasible())
        rResult.eStatus = CoinMpSolveStatus::Infeasible;
    else if (pSimplex->isProvenDualInfeasible())
        rResult.eStatus = CoinMpSolveStatus::Unbounded;
    else if (pSimplex->hitMaximumIterations())
        rResult.eStatus = CoinMpSolveStatus::TimeLimit;

    rResult.bSuccess = rResult.eStatus == CoinMpSolveStatus::Optimal;
    if (rResult.bSuccess)
    {
        const double* pSolution = pSimplex->primalColumnSolution();
        rResult.aSolution.assign(pSolution, pSolution + nVariables);
        rResult.fObjective = pSimplex->objectiveValue();
    }
}

// Cbc's standalone solver runs branch and bound with its default cut generators and heuristics.
void solveInteger(const OsiClpSolverInterface& rSolver, size_t nVariables, double fTimeoutSeconds,
                  CoinMpSolveResult& rResult)
{
    CbcModel aModel(rSolver);
    CbcSolverUsefulData aSolverData;
    CbcMain0(aModel, aSolverData);
    aModel.setMaximumSeconds(fTimeoutSeconds);
    const char* aArguments[] = { "calc", "-log", "0", "-solve", "-quit" };
    CbcMain1(std::size(aArguments), aArguments, aModel, noCallBack, aSolverData);

    if (aModel.isProvenOptimal())
        rResult.eStatus = CoinMpSolveStatus::Optimal;
    else if (aModel.isProvenInfeasible())
        rResult.eStatus = CoinMpSolveStatus::Infeasible;
    else if (aModel.isContinuousUnbounded())
        rResult.eStatus = CoinMpSolveStatus::Unbounded;
    else if (aModel.isSecondsLimitReached())
        rResult.eStatus = CoinMpSolveStatus::TimeLimit;

    // At the time limit the best solution found so far is the result.
    const double* pSolution = aModel.bestSolution();
    rResult.bSuccess = pSolution
                       && (rResult.eStatus == CoinMpSolveStatus::Optimal
                           || rResult.eStatus == CoinMpSolveStatus::TimeLimit);
    if (rResult.bSuccess)
    {
        rResult.aSolution.assign(pSolution, pSolution + nVariables);
        rResult.fObjective = aModel.getObjValue();
    }
}
}

CoinMpSolveResult
coinmpSolveLinearModel(const cpo::uno::Sequence<table::CellAddress>& rVariables,
                       const cpo::uno::Sequence<sheet::SolverConstraint>& rConstraints,
                       const table::CellAddress& rObjective, const ScSolverCellHashMap& rCells,
                       const std::vector<double>& rLowerBounds,
                       const std::vector<double>& rUpperBounds,
                       const std::vector<char>& rColumnType, bool bMaximize, double fTimeoutSeconds)
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

    // a row without a comparison operator is free
    std::vector<double> aRowLower(nRows, -COIN_DBL_MAX);
    std::vector<double> aRowUpper(nRows, COIN_DBL_MAX);

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

            if (eOperator != sheet::SolverConstraintOperator_LESS_EQUAL)
                aRowLower[nConstraintPosition] = fRightValue;
            if (eOperator != sheet::SolverConstraintOperator_GREATER_EQUAL)
                aRowUpper[nConstraintPosition] = fRightValue;
        }
    }

    // turn the row-wise matrix into the column-wise form the solver wants
    std::unique_ptr<int[]> pMatrixBegin(new int[nVariables + 1]);
    std::unique_ptr<double[]> pMatrix(new double[nDenseSize]);
    std::unique_ptr<int[]> pMatrixIndex(new int[nDenseSize]);
    int nMatrixPosition = 0;
    for (nVar = 0; nVar < nVariables; nVar++)
    {
        pMatrixBegin[nVar] = nMatrixPosition;
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
    }
    pMatrixBegin[nVariables] = nMatrixPosition;
    pDenseMatrix.reset();

    try
    {
        OsiClpSolverInterface aSolver;
        aSolver.messageHandler()->setLogLevel(0);
        aSolver.loadProblem(nVariables, nRows, pMatrixBegin.get(), pMatrixIndex.get(),
                            pMatrix.get(), rLowerBounds.data(), rUpperBounds.data(),
                            pObjectCoefficients.get(), aRowLower.data(), aRowUpper.data());
        aSolver.setObjSense(bMaximize ? -1.0 : 1.0);

        bool bInteger = false;
        for (nVar = 0; nVar < nVariables; nVar++)
        {
            if (rColumnType[nVar] == 'I' || rColumnType[nVar] == 'B')
            {
                aSolver.setInteger(nVar);
                bInteger = true;
            }
        }

        pMatrixIndex.reset();
        pMatrix.reset();
        pMatrixBegin.reset();
        pObjectCoefficients.reset();

        if (bInteger)
            solveInteger(aSolver, nVariables, fTimeoutSeconds, aResult);
        else
            solveContinuous(aSolver, nVariables, fTimeoutSeconds, aResult);
    }
    catch (const CoinError& rError)
    {
        throw std::runtime_error(rError.message());
    }

    if (aResult.bSuccess)
        aResult.fObjective += fObjectConstant;

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
