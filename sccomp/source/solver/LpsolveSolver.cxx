/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
 *
 ************************************************************************/

#include "sal/config.h"
#include <config_lgpl.h>

#undef LANGUAGE_NONE
#if defined SAL_W32
#define WINAPI __stdcall
#endif
#define LoadInverseLib FALSE
#define LoadLanguageLib FALSE
#ifdef SYSTEM_LPSOLVE
#include <lpsolve/lp_lib.h>
#else
#include <lp_lib.h>
#endif
#undef LANGUAGE_NONE

#include "SolverComponent.hxx"
#include "strings.hrc"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/math.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <memory>
#include <vector>

using namespace com::sun::star;

class LpsolveSolver : public SolverComponent
{
public:
    LpsolveSolver() {}

private:
    virtual void SAL_CALL solve() override;
    virtual OUString SAL_CALL getImplementationName() override
    {
        return OUString("com.sun.star.comp.Calc.LpsolveSolver");
    }
    virtual OUString SAL_CALL getComponentDescription() override
    {
        return SolverComponent::GetResourceString( RID_SOLVER_COMPONENT );
    }
};

void SAL_CALL LpsolveSolver::solve()
{
    uno::Reference<frame::XModel> xModel( mxDoc, uno::UNO_QUERY_THROW );

    maStatus.clear();
    mbSuccess = false;

    if ( mnEpsilonLevel < EPS_TIGHT || mnEpsilonLevel > EPS_BAGGY )
    {
        maStatus = SolverComponent::GetResourceString( RID_ERROR_EPSILONLEVEL );
        return;
    }

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


    // build lp_solve model


    lprec* lp = make_lp( 0, nVariables );
    if ( !lp )
        return;

    set_outputfile( lp, const_cast<char*>( "" ) );  // no output

    // set objective function

    const std::vector<double>& rObjCoeff = aCellsHash[maObjective];
    REAL* pObjVal = new REAL[nVariables+1];
    pObjVal[0] = 0.0;                           // ignored
    for (nVar=0; nVar<nVariables; nVar++)
        pObjVal[nVar+1] = rObjCoeff[nVar+1];
    set_obj_fn( lp, pObjVal );
    delete[] pObjVal;
    set_rh( lp, 0, rObjCoeff[0] );              // constant term of objective

    // add rows

    set_add_rowmode(lp, TRUE);

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
            std::unique_ptr<REAL[]> pValues(new REAL[nVariables+1] );
            pValues[0] = 0.0;                               // ignored?
            for (nVar=0; nVar<nVariables; nVar++)
                pValues[nVar+1] = rLeftCoeff[nVar+1];

            // if left hand cell has a constant term, put into rhs value
            double fRightValue = -rLeftCoeff[0];

            if ( bRightCell )
            {
                const std::vector<double>& rRightCoeff = aCellsHash[aRightAddr];
                // modify pValues with rhs coefficients
                for (nVar=0; nVar<nVariables; nVar++)
                    pValues[nVar+1] -= rRightCoeff[nVar+1];

                fRightValue += rRightCoeff[0];      // constant term
            }
            else
                fRightValue += fDirectValue;

            int nConstrType = LE;
            switch ( eOp )
            {
                case sheet::SolverConstraintOperator_LESS_EQUAL:    nConstrType = LE; break;
                case sheet::SolverConstraintOperator_GREATER_EQUAL: nConstrType = GE; break;
                case sheet::SolverConstraintOperator_EQUAL:         nConstrType = EQ; break;
                default:
                    OSL_FAIL( "unexpected enum type" );
            }
            add_constraint( lp, pValues.get(), nConstrType, fRightValue );
        }
    }

    set_add_rowmode(lp, FALSE);

    // apply settings to all variables

    for (nVar=0; nVar<nVariables; nVar++)
    {
        if ( !mbNonNegative )
            set_unbounded(lp, nVar+1);          // allow negative (default is non-negative)
                                                //! collect bounds from constraints?
        if ( mbInteger )
            set_int(lp, nVar+1, TRUE);
    }

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
                        set_int(lp, nVar+1, TRUE);
                    else
                        set_binary(lp, nVar+1, TRUE);
                }
        }
    }

    if ( mbMaximize )
        set_maxim(lp);
    else
        set_minim(lp);

    if ( !mbLimitBBDepth )
        set_bb_depthlimit( lp, 0 );

    set_epslevel( lp, mnEpsilonLevel );
    set_timeout( lp, mnTimeout );

    // solve model

    int nResult = ::solve( lp );

    mbSuccess = ( nResult == OPTIMAL );
    if ( mbSuccess )
    {
        // get solution

        maSolution.realloc( nVariables );

        REAL* pResultVar = nullptr;
        get_ptr_variables( lp, &pResultVar );
        for (nVar=0; nVar<nVariables; nVar++)
            maSolution[nVar] = pResultVar[nVar];

        mfResultValue = get_objective( lp );
    }
    else if ( nResult == INFEASIBLE )
        maStatus = SolverComponent::GetResourceString( RID_ERROR_INFEASIBLE );
    else if ( nResult == UNBOUNDED )
        maStatus = SolverComponent::GetResourceString( RID_ERROR_UNBOUNDED );
    else if ( nResult == TIMEOUT || nResult == SUBOPTIMAL )
        maStatus = SolverComponent::GetResourceString( RID_ERROR_TIMEOUT );
    // SUBOPTIMAL is assumed to be caused by a timeout, and reported as an error

    delete_lp( lp );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_Calc_LpsolveSolver_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new LpsolveSolver());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
