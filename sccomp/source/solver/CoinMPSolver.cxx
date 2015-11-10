/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <CoinMP.h>
#include <CoinError.hpp>

#include "SolverComponent.hxx"
#include "solver.hrc"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <rtl/math.hxx>
#include <stdexcept>
#include <vector>

using namespace com::sun::star;

class CoinMPSolver : public SolverComponent
{
public:
    CoinMPSolver() {}
    virtual ~CoinMPSolver() {}

private:
    virtual void SAL_CALL solve() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getImplementationName()
        throw(css::uno::RuntimeException, std::exception) override
    {
        return OUString("com.sun.star.comp.Calc.CoinMPSolver");
    }
    virtual OUString SAL_CALL getComponentDescription()
        throw (uno::RuntimeException, std::exception) override
    {
        return SolverComponent::GetResourceString( RID_COINMP_SOLVER_COMPONENT );
    }
};

void SAL_CALL CoinMPSolver::solve() throw(uno::RuntimeException, std::exception)
{
    uno::Reference<frame::XModel> xModel( mxDoc, uno::UNO_QUERY );
    if ( !xModel.is() )
        throw uno::RuntimeException();

    maStatus.clear();
    mbSuccess = false;

    xModel->lockControllers();

    // collect variables in vector (?)

    std::vector<table::CellAddress> aVariableCells;
    for (sal_Int32 nPos=0; nPos<maVariables.getLength(); nPos++)
        aVariableCells.push_back( maVariables[nPos] );
    size_t nVariables = aVariableCells.size();
    size_t nVar = 0;

    // collect all dependent cells

    ScSolverCellHashMap aCellsHash;
    aCellsHash[maObjective].reserve( nVariables + 1 );                  // objective function

    for (sal_Int32 nConstrPos = 0; nConstrPos < maConstraints.getLength(); ++nConstrPos)
    {
        table::CellAddress aCellAddr = maConstraints[nConstrPos].Left;
        aCellsHash[aCellAddr].reserve( nVariables + 1 );                // constraints: left hand side

        if ( maConstraints[nConstrPos].Right >>= aCellAddr )
            aCellsHash[aCellAddr].reserve( nVariables + 1 );            // constraints: right hand side
    }

    // set all variables to zero
    //! store old values?
    //! use old values as initial values?
    std::vector<table::CellAddress>::const_iterator aVarIter;
    for ( aVarIter = aVariableCells.begin(); aVarIter != aVariableCells.end(); ++aVarIter )
    {
        SolverComponent::SetValue( mxDoc, *aVarIter, 0.0 );
    }

    // read initial values from all dependent cells
    ScSolverCellHashMap::iterator aCellsIter;
    for ( aCellsIter = aCellsHash.begin(); aCellsIter != aCellsHash.end(); ++aCellsIter )
    {
        double fValue = SolverComponent::GetValue( mxDoc, aCellsIter->first );
        aCellsIter->second.push_back( fValue );                         // store as first element, as-is
    }

    // loop through variables
    for ( aVarIter = aVariableCells.begin(); aVarIter != aVariableCells.end(); ++aVarIter )
    {
        SolverComponent::SetValue( mxDoc, *aVarIter, 1.0 );      // set to 1 to examine influence

        // read value change from all dependent cells
        for ( aCellsIter = aCellsHash.begin(); aCellsIter != aCellsHash.end(); ++aCellsIter )
        {
            double fChanged = SolverComponent::GetValue( mxDoc, aCellsIter->first );
            double fInitial = aCellsIter->second.front();
            aCellsIter->second.push_back( fChanged - fInitial );
        }

        SolverComponent::SetValue( mxDoc, *aVarIter, 2.0 );      // minimal test for linearity

        for ( aCellsIter = aCellsHash.begin(); aCellsIter != aCellsHash.end(); ++aCellsIter )
        {
            double fInitial = aCellsIter->second.front();
            double fCoeff   = aCellsIter->second.back();       // last appended: coefficient for this variable
            double fTwo     = SolverComponent::GetValue( mxDoc, aCellsIter->first );

            bool bLinear = rtl::math::approxEqual( fTwo, fInitial + 2.0 * fCoeff ) ||
                           rtl::math::approxEqual( fInitial, fTwo - 2.0 * fCoeff );
            // second comparison is needed in case fTwo is zero
            if ( !bLinear )
                maStatus = SolverComponent::GetResourceString( RID_ERROR_NONLINEAR );
        }

        SolverComponent::SetValue( mxDoc, *aVarIter, 0.0 );      // set back to zero for examining next variable
    }

    xModel->unlockControllers();

    if ( !maStatus.isEmpty() )
        return;

    //
    // build parameter arrays for CoinMP
    //

    // set objective function

    const std::vector<double>& rObjCoeff = aCellsHash[maObjective];
    double* pObjectCoeffs = new double[nVariables];
    for (nVar=0; nVar<nVariables; nVar++)
        pObjectCoeffs[nVar] = rObjCoeff[nVar+1];
    double nObjectConst = rObjCoeff[0];             // constant term of objective

    // add rows

    size_t nRows = maConstraints.getLength();
    size_t nCompSize = nVariables * nRows;
    double* pCompMatrix = new double[nCompSize];    // first collect all coefficients, row-wise
    for (size_t i=0; i<nCompSize; i++)
        pCompMatrix[i] = 0.0;

    double* pRHS = new double[nRows];
    char* pRowType = new char[nRows];
    for (size_t i=0; i<nRows; i++)
    {
        pRHS[i] = 0.0;
        pRowType[i] = 'N';
    }

    for (sal_Int32 nConstrPos = 0; nConstrPos < maConstraints.getLength(); ++nConstrPos)
    {
        // integer constraints are set later
        sheet::SolverConstraintOperator eOp = maConstraints[nConstrPos].Operator;
        if ( eOp == sheet::SolverConstraintOperator_LESS_EQUAL ||
             eOp == sheet::SolverConstraintOperator_GREATER_EQUAL ||
             eOp == sheet::SolverConstraintOperator_EQUAL )
        {
            double fDirectValue = 0.0;
            bool bRightCell = false;
            table::CellAddress aRightAddr;
            const uno::Any& rRightAny = maConstraints[nConstrPos].Right;
            if ( rRightAny >>= aRightAddr )
                bRightCell = true;                  // cell specified as right-hand side
            else
                rRightAny >>= fDirectValue;         // constant value

            table::CellAddress aLeftAddr = maConstraints[nConstrPos].Left;

            const std::vector<double>& rLeftCoeff = aCellsHash[aLeftAddr];
            double* pValues = &pCompMatrix[nConstrPos * nVariables];
            for (nVar=0; nVar<nVariables; nVar++)
                pValues[nVar] = rLeftCoeff[nVar+1];

            // if left hand cell has a constant term, put into rhs value
            double fRightValue = -rLeftCoeff[0];

            if ( bRightCell )
            {
                const std::vector<double>& rRightCoeff = aCellsHash[aRightAddr];
                // modify pValues with rhs coefficients
                for (nVar=0; nVar<nVariables; nVar++)
                    pValues[nVar] -= rRightCoeff[nVar+1];

                fRightValue += rRightCoeff[0];      // constant term
            }
            else
                fRightValue += fDirectValue;

            switch ( eOp )
            {
                case sheet::SolverConstraintOperator_LESS_EQUAL:    pRowType[nConstrPos] = 'L'; break;
                case sheet::SolverConstraintOperator_GREATER_EQUAL: pRowType[nConstrPos] = 'G'; break;
                case sheet::SolverConstraintOperator_EQUAL:         pRowType[nConstrPos] = 'E'; break;
                default:
                    OSL_ENSURE( false, "unexpected enum type" );
            }
            pRHS[nConstrPos] = fRightValue;
        }
    }

    // Find non-zero coefficients, column-wise

    int* pMatrixBegin = new int[nVariables+1];
    int* pMatrixCount = new int[nVariables];
    double* pMatrix = new double[nCompSize];    // not always completely used
    int* pMatrixIndex = new int[nCompSize];
    int nMatrixPos = 0;
    for (nVar=0; nVar<nVariables; nVar++)
    {
        int nBegin = nMatrixPos;
        for (size_t nRow=0; nRow<nRows; nRow++)
        {
            double fCoeff = pCompMatrix[ nRow * nVariables + nVar ];    // row-wise
            if ( fCoeff != 0.0 )
            {
                pMatrix[nMatrixPos] = fCoeff;
                pMatrixIndex[nMatrixPos] = nRow;
                ++nMatrixPos;
            }
        }
        pMatrixBegin[nVar] = nBegin;
        pMatrixCount[nVar] = nMatrixPos - nBegin;
    }
    pMatrixBegin[nVariables] = nMatrixPos;
    delete[] pCompMatrix;
    pCompMatrix = nullptr;

    // apply settings to all variables

    double* pLowerBounds = new double[nVariables];
    double* pUpperBounds = new double[nVariables];
    for (nVar=0; nVar<nVariables; nVar++)
    {
        pLowerBounds[nVar] = mbNonNegative ? 0.0 : -DBL_MAX;
        pUpperBounds[nVar] = DBL_MAX;

        // bounds could possibly be further restricted from single-cell constraints
    }

    char* pColType = new char[nVariables];
    for (nVar=0; nVar<nVariables; nVar++)
        pColType[nVar] = mbInteger ? 'I' : 'C';

    // apply single-var integer constraints

    for (sal_Int32 nConstrPos = 0; nConstrPos < maConstraints.getLength(); ++nConstrPos)
    {
        sheet::SolverConstraintOperator eOp = maConstraints[nConstrPos].Operator;
        if ( eOp == sheet::SolverConstraintOperator_INTEGER ||
             eOp == sheet::SolverConstraintOperator_BINARY )
        {
            table::CellAddress aLeftAddr = maConstraints[nConstrPos].Left;
            // find variable index for cell
            for (nVar=0; nVar<nVariables; nVar++)
                if ( AddressEqual( aVariableCells[nVar], aLeftAddr ) )
                {
                    if ( eOp == sheet::SolverConstraintOperator_INTEGER )
                        pColType[nVar] = 'I';
                    else
                    {
                        pColType[nVar] = 'B';
                        pLowerBounds[nVar] = 0.0;
                        pUpperBounds[nVar] = 1.0;
                    }
                }
        }
    }

    int nObjectSense = mbMaximize ? SOLV_OBJSENS_MAX : SOLV_OBJSENS_MIN;

    HPROB hProb = CoinCreateProblem("");
    int nResult = CoinLoadProblem( hProb, nVariables, nRows, nMatrixPos, 0,
                    nObjectSense, nObjectConst, pObjectCoeffs,
                    pLowerBounds, pUpperBounds, pRowType, pRHS, nullptr,
                    pMatrixBegin, pMatrixCount, pMatrixIndex, pMatrix,
                    nullptr, nullptr, nullptr );
    if (nResult == SOLV_CALL_SUCCESS)
    {
        nResult = CoinLoadInteger( hProb, pColType );
    }

    delete[] pColType;
    delete[] pMatrixIndex;
    delete[] pMatrix;
    delete[] pMatrixCount;
    delete[] pMatrixBegin;
    delete[] pUpperBounds;
    delete[] pLowerBounds;
    delete[] pRowType;
    delete[] pRHS;
    delete[] pObjectCoeffs;

    CoinSetRealOption( hProb, COIN_REAL_MAXSECONDS, mnTimeout );
    CoinSetRealOption( hProb, COIN_REAL_MIPMAXSEC, mnTimeout );

    // TODO: handle (or remove) settings: epsilon, B&B depth

    // solve model

    if (nResult == SOLV_CALL_SUCCESS)
    {
        nResult = CoinCheckProblem( hProb );
    }

    if (nResult == SOLV_CALL_SUCCESS)
    {
        try
        {
            nResult = CoinOptimizeProblem( hProb, 0 );
        }
        catch (const CoinError& e)
        {
            throw std::runtime_error(e.message());
        }
    }

    mbSuccess = ( nResult == SOLV_CALL_SUCCESS );
    if ( mbSuccess )
    {
        // get solution

        maSolution.realloc( nVariables );
        CoinGetSolutionValues( hProb, maSolution.getArray(), nullptr, nullptr, nullptr );
        mfResultValue = CoinGetObjectValue( hProb );
    }
    else
    {
        int nSolutionStatus = CoinGetSolutionStatus( hProb );
        if ( nSolutionStatus == 1 )
            maStatus = SolverComponent::GetResourceString( RID_ERROR_INFEASIBLE );
        else if ( nSolutionStatus == 2 )
            maStatus = SolverComponent::GetResourceString( RID_ERROR_UNBOUNDED );
        // TODO: detect timeout condition and report as RID_ERROR_TIMEOUT
        // (currently reported as infeasible)
    }

    CoinUnloadProblem( hProb );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_Calc_CoinMPSolver_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new CoinMPSolver());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
