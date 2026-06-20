/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include "CoinMPModel.hxx"
#include "SolverComponent.hxx"
#include "strings.hrc"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/table/CellAddress.hpp>

#include <rtl/math.hxx>
#include <algorithm>
#include <vector>
#include <float.h>

namespace com::sun::star::uno { class XComponentContext; }

using namespace com::sun::star;

namespace {

class CoinMPSolver : public SolverComponent
{
public:
    CoinMPSolver() {}

private:
    virtual void SAL_CALL solve() override;
    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.Calc.CoinMPSolver"_ustr;
    }
    virtual OUString SAL_CALL getComponentDescription() override
    {
        return SolverComponent::GetResourceString( RID_COINMP_SOLVER_COMPONENT );
    }
};

}

void SAL_CALL CoinMPSolver::solve()
{
    uno::Reference<frame::XModel> xModel( mxDoc, uno::UNO_QUERY_THROW );

    maStatus.clear();
    mbSuccess = false;

    xModel->lockControllers();

    // collect variables in vector (?)

    const auto & aVariableCells = maVariables;
    size_t nVariables = aVariableCells.size();
    size_t nVar = 0;

    // collect all dependent cells

    ScSolverCellHashMap aCellsHash;
    aCellsHash[maObjective].reserve( nVariables + 1 );                  // objective function

    for (const auto& rConstr : maConstraints)
    {
        table::CellAddress aCellAddr = rConstr.Left;
        aCellsHash[aCellAddr].reserve( nVariables + 1 );                // constraints: left hand side

        if ( rConstr.Right >>= aCellAddr )
            aCellsHash[aCellAddr].reserve( nVariables + 1 );            // constraints: right hand side
    }

    // set all variables to zero
    //! store old values?
    //! use old values as initial values?
    for ( const auto& rVarCell : aVariableCells )
    {
        SetValue(rVarCell, 0.0);
    }

    // read initial values from all dependent cells
    for ( auto& rEntry : aCellsHash )
    {
        double fValue = GetValue(rEntry.first);
        rEntry.second.push_back( fValue );                         // store as first element, as-is
    }

    // loop through variables
    for ( const auto& rVarCell : aVariableCells )
    {
        SetValue(rVarCell, 1.0); // set to 1 to examine influence

        // read value change from all dependent cells
        for ( auto& rEntry : aCellsHash )
        {
            double fChanged = GetValue(rEntry.first);
            double fInitial = rEntry.second.front();
            rEntry.second.push_back( fChanged - fInitial );
        }

        SetValue(rVarCell, 2.0); // minimal test for linearity

        for ( const auto& rEntry : aCellsHash )
        {
            double fInitial = rEntry.second.front();
            double fCoeff   = rEntry.second.back();       // last appended: coefficient for this variable
            double fTwo = GetValue(rEntry.first);

            bool bLinear = rtl::math::approxEqual( fTwo, fInitial + 2.0 * fCoeff ) ||
                           rtl::math::approxEqual( fInitial, fTwo - 2.0 * fCoeff );
            // second comparison is needed in case fTwo is zero
            if ( !bLinear )
                maStatus = SolverComponent::GetResourceString( RID_ERROR_NONLINEAR );
        }

        SetValue(rVarCell, 0.0); // set back to zero for examining next variable
    }

    xModel->unlockControllers();

    if ( !maStatus.isEmpty() )
        return;

    // variable bounds and column types. The default range is unbounded unless
    // non-negativity was requested. Integer and binary single-variable
    // constraints refine these.
    std::vector<double> aLowerBounds(nVariables, mbNonNegative ? 0.0 : -DBL_MAX);
    std::vector<double> aUpperBounds(nVariables, DBL_MAX);
    std::vector<char> aColumnType(nVariables, mbInteger ? 'I' : 'C');

    for (const auto& rConstr : maConstraints)
    {
        sheet::SolverConstraintOperator eOp = rConstr.Operator;
        if ( eOp == sheet::SolverConstraintOperator_INTEGER ||
             eOp == sheet::SolverConstraintOperator_BINARY )
        {
            table::CellAddress aLeftAddr = rConstr.Left;
            for (nVar=0; nVar<nVariables; nVar++)
                if ( AddressEqual( aVariableCells[nVar], aLeftAddr ) )
                {
                    if ( eOp == sheet::SolverConstraintOperator_INTEGER )
                        aColumnType[nVar] = 'I';
                    else
                    {
                        aColumnType[nVar] = 'B';
                        aLowerBounds[nVar] = 0.0;
                        aUpperBounds[nVar] = 1.0;
                    }
                }
        }
    }

    // TODO: handle (or remove) settings: epsilon, B&B depth

    CoinMpSolveResult aSolveResult
        = coinmpSolveLinearModel(maVariables, maConstraints, maObjective, aCellsHash, aLowerBounds,
                                 aUpperBounds, aColumnType, mbMaximize, mnTimeout);

    mbSuccess = aSolveResult.bSuccess;
    if ( mbSuccess )
    {
        maSolution.realloc( nVariables );
        std::copy(aSolveResult.aSolution.begin(), aSolveResult.aSolution.end(),
                  maSolution.getArray());
        mfResultValue = aSolveResult.fObjective;
    }
    else
    {
        if (aSolveResult.nStatus == 1)
            maStatus = SolverComponent::GetResourceString( RID_ERROR_INFEASIBLE );
        else if (aSolveResult.nStatus == 2)
            maStatus = SolverComponent::GetResourceString( RID_ERROR_UNBOUNDED );
        // TODO: detect timeout condition and report as RID_ERROR_TIMEOUT
        // (currently reported as infeasible)
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_Calc_CoinMPSolver_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new CoinMPSolver());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
