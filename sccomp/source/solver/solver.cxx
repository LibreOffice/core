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

#include "solver.hxx"
#include "solver.hrc"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vector>
#include <unordered_map>

#include <tools/resmgr.hxx>

using namespace com::sun::star;

#define C2U(constAsciiStr) (::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( constAsciiStr ) ))

#define STR_NONNEGATIVE   "NonNegative"
#define STR_INTEGER       "Integer"
#define STR_TIMEOUT       "Timeout"
#define STR_EPSILONLEVEL  "EpsilonLevel"
#define STR_LIMITBBDEPTH  "LimitBBDepth"

// -----------------------------------------------------------------------
//  Resources from tools are used for translated strings

static ResMgr* pSolverResMgr = NULL;

OUString lcl_GetResourceString( sal_uInt32 nId )
{
    if (!pSolverResMgr)
        pSolverResMgr = ResMgr::CreateResMgr( "solver" );

    return ResId( nId, *pSolverResMgr );
}

// -----------------------------------------------------------------------

namespace
{
    enum
    {
        PROP_NONNEGATIVE,
        PROP_INTEGER,
        PROP_TIMEOUT,
        PROP_EPSILONLEVEL,
        PROP_LIMITBBDEPTH
    };
}

// -----------------------------------------------------------------------

// hash map for the coefficients of a dependent cell (objective or constraint)
// The size of each vector is the number of columns (variable cells) plus one, first entry is initial value.

struct ScSolverCellHash
{
    size_t operator()( const table::CellAddress& rAddress ) const
    {
        return ( rAddress.Sheet << 24 ) | ( rAddress.Column << 16 ) | rAddress.Row;
    }
};

inline bool AddressEqual( const table::CellAddress& rAddr1, const table::CellAddress& rAddr2 )
{
    return rAddr1.Sheet == rAddr2.Sheet && rAddr1.Column == rAddr2.Column && rAddr1.Row == rAddr2.Row;
}

struct ScSolverCellEqual
{
    bool operator()( const table::CellAddress& rAddr1, const table::CellAddress& rAddr2 ) const
    {
        return AddressEqual( rAddr1, rAddr2 );
    }
};

typedef std::unordered_map< table::CellAddress, std::vector<double>, ScSolverCellHash, ScSolverCellEqual > ScSolverCellHashMap;

// -----------------------------------------------------------------------

uno::Reference<table::XCell> lcl_GetCell( const uno::Reference<sheet::XSpreadsheetDocument>& xDoc,
                                          const table::CellAddress& rPos )
{
    uno::Reference<container::XIndexAccess> xSheets( xDoc->getSheets(), uno::UNO_QUERY );
    uno::Reference<sheet::XSpreadsheet> xSheet( xSheets->getByIndex( rPos.Sheet ), uno::UNO_QUERY );
    return xSheet->getCellByPosition( rPos.Column, rPos.Row );
}

void lcl_SetValue( const uno::Reference<sheet::XSpreadsheetDocument>& xDoc,
                   const table::CellAddress& rPos, double fValue )
{
    lcl_GetCell( xDoc, rPos )->setValue( fValue );
}

double lcl_GetValue( const uno::Reference<sheet::XSpreadsheetDocument>& xDoc,
                     const table::CellAddress& rPos )
{
    return lcl_GetCell( xDoc, rPos )->getValue();
}

// -------------------------------------------------------------------------

SolverComponent::SolverComponent( const uno::Reference<uno::XComponentContext>& /* rSMgr */ ) :
    OPropertyContainer( GetBroadcastHelper() ),
    mbMaximize( sal_True ),
    mbNonNegative( sal_False ),
    mbInteger( sal_False ),
    mnTimeout( 100 ),
    mnEpsilonLevel( 0 ),
    mbLimitBBDepth( sal_True ),
    mbSuccess( sal_False ),
    mfResultValue( 0.0 )
{
    // for XPropertySet implementation:
    registerProperty( C2U(STR_NONNEGATIVE),  PROP_NONNEGATIVE,  0, &mbNonNegative,  getCppuType( &mbNonNegative )  );
    registerProperty( C2U(STR_INTEGER),      PROP_INTEGER,      0, &mbInteger,      getCppuType( &mbInteger )      );
    registerProperty( C2U(STR_TIMEOUT),      PROP_TIMEOUT,      0, &mnTimeout,      getCppuType( &mnTimeout )      );
    registerProperty( C2U(STR_EPSILONLEVEL), PROP_EPSILONLEVEL, 0, &mnEpsilonLevel, getCppuType( &mnEpsilonLevel ) );
    registerProperty( C2U(STR_LIMITBBDEPTH), PROP_LIMITBBDEPTH, 0, &mbLimitBBDepth, getCppuType( &mbLimitBBDepth ) );
}

SolverComponent::~SolverComponent()
{
}

IMPLEMENT_FORWARD_XINTERFACE2( SolverComponent, SolverComponent_Base, OPropertyContainer )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SolverComponent, SolverComponent_Base, OPropertyContainer )

cppu::IPropertyArrayHelper* SolverComponent::createArrayHelper() const
{
    uno::Sequence<beans::Property> aProps;
    describeProperties( aProps );
    return new cppu::OPropertyArrayHelper( aProps );
}

cppu::IPropertyArrayHelper& SAL_CALL SolverComponent::getInfoHelper()
{
    return *getArrayHelper();
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL SolverComponent::getPropertySetInfo() throw(uno::RuntimeException, std::exception)
{
    return createPropertySetInfo( getInfoHelper() );
}

// XSolverDescription

OUString SAL_CALL SolverComponent::getComponentDescription() throw (uno::RuntimeException, std::exception)
{
    return lcl_GetResourceString( RID_SOLVER_COMPONENT );
}

OUString SAL_CALL SolverComponent::getStatusDescription() throw (uno::RuntimeException, std::exception)
{
    return maStatus;
}

OUString SAL_CALL SolverComponent::getPropertyDescription( const OUString& rPropertyName ) throw (uno::RuntimeException, std::exception)
{
    sal_uInt32 nResId = 0;
    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );
    switch (nHandle)
    {
        case PROP_NONNEGATIVE:
            nResId = RID_PROPERTY_NONNEGATIVE;
            break;
        case PROP_INTEGER:
            nResId = RID_PROPERTY_INTEGER;
            break;
        case PROP_TIMEOUT:
            nResId = RID_PROPERTY_TIMEOUT;
            break;
        case PROP_EPSILONLEVEL:
            nResId = RID_PROPERTY_EPSILONLEVEL;
            break;
        case PROP_LIMITBBDEPTH:
            nResId = RID_PROPERTY_LIMITBBDEPTH;
            break;
        default:
            {
                // unknown - leave empty
            }
    }
    OUString aRet;
    if ( nResId )
        aRet = lcl_GetResourceString( nResId );
    return aRet;
}

// XSolver: settings

uno::Reference<sheet::XSpreadsheetDocument> SAL_CALL SolverComponent::getDocument() throw(uno::RuntimeException, std::exception)
{
    return mxDoc;
}

void SAL_CALL SolverComponent::setDocument( const uno::Reference<sheet::XSpreadsheetDocument>& _document )
                                throw(uno::RuntimeException, std::exception)
{
    mxDoc = _document;
}

table::CellAddress SAL_CALL SolverComponent::getObjective() throw(uno::RuntimeException, std::exception)
{
    return maObjective;
}

void SAL_CALL SolverComponent::setObjective( const table::CellAddress& _objective ) throw(uno::RuntimeException, std::exception)
{
    maObjective = _objective;
}

uno::Sequence<table::CellAddress> SAL_CALL SolverComponent::getVariables() throw(uno::RuntimeException, std::exception)
{
    return maVariables;
}

void SAL_CALL SolverComponent::setVariables( const uno::Sequence<table::CellAddress>& _variables )
                                throw(uno::RuntimeException, std::exception)
{
    maVariables = _variables;
}

uno::Sequence<sheet::SolverConstraint> SAL_CALL SolverComponent::getConstraints() throw(uno::RuntimeException, std::exception)
{
    return maConstraints;
}

void SAL_CALL SolverComponent::setConstraints( const uno::Sequence<sheet::SolverConstraint>& _constraints )
                                throw(uno::RuntimeException, std::exception)
{
    maConstraints = _constraints;
}

sal_Bool SAL_CALL SolverComponent::getMaximize() throw(uno::RuntimeException, std::exception)
{
    return mbMaximize;
}

void SAL_CALL SolverComponent::setMaximize( sal_Bool _maximize ) throw(uno::RuntimeException, std::exception)
{
    mbMaximize = _maximize;
}

// XSolver: get results

sal_Bool SAL_CALL SolverComponent::getSuccess() throw(uno::RuntimeException, std::exception)
{
    return mbSuccess;
}

double SAL_CALL SolverComponent::getResultValue() throw(uno::RuntimeException, std::exception)
{
    return mfResultValue;
}

uno::Sequence<double> SAL_CALL SolverComponent::getSolution() throw(uno::RuntimeException, std::exception)
{
    return maSolution;
}

// -------------------------------------------------------------------------

void SAL_CALL SolverComponent::solve() throw(uno::RuntimeException, std::exception)
{
    uno::Reference<frame::XModel> xModel( mxDoc, uno::UNO_QUERY );
    if ( !xModel.is() )
        throw uno::RuntimeException();

    maStatus = OUString();
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
        lcl_SetValue( mxDoc, *aVarIter, 0.0 );
    }

    // read initial values from all dependent cells
    ScSolverCellHashMap::iterator aCellsIter;
    for ( aCellsIter = aCellsHash.begin(); aCellsIter != aCellsHash.end(); ++aCellsIter )
    {
        double fValue = lcl_GetValue( mxDoc, aCellsIter->first );
        aCellsIter->second.push_back( fValue );                         // store as first element, as-is
    }

    // loop through variables
    for ( aVarIter = aVariableCells.begin(); aVarIter != aVariableCells.end(); ++aVarIter )
    {
        lcl_SetValue( mxDoc, *aVarIter, 1.0 );      // set to 1 to examine influence

        // read value change from all dependent cells
        for ( aCellsIter = aCellsHash.begin(); aCellsIter != aCellsHash.end(); ++aCellsIter )
        {
            double fChanged = lcl_GetValue( mxDoc, aCellsIter->first );
            double fInitial = aCellsIter->second.front();
            aCellsIter->second.push_back( fChanged - fInitial );
        }

        lcl_SetValue( mxDoc, *aVarIter, 2.0 );      // minimal test for linearity

        for ( aCellsIter = aCellsHash.begin(); aCellsIter != aCellsHash.end(); ++aCellsIter )
        {
            double fInitial = aCellsIter->second.front();
            double fCoeff   = aCellsIter->second.back();       // last appended: coefficient for this variable
            double fTwo     = lcl_GetValue( mxDoc, aCellsIter->first );

            bool bLinear = rtl::math::approxEqual( fTwo, fInitial + 2.0 * fCoeff ) ||
                           rtl::math::approxEqual( fInitial, fTwo - 2.0 * fCoeff );
            // second comparison is needed in case fTwo is zero
            if ( !bLinear )
                maStatus = lcl_GetResourceString( RID_ERROR_NONLINEAR );
        }

        lcl_SetValue( mxDoc, *aVarIter, 0.0 );      // set back to zero for examining next variable
    }

    xModel->unlockControllers();

    if ( maStatus.getLength() )
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
    pCompMatrix = NULL;

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
                    pLowerBounds, pUpperBounds, pRowType, pRHS, NULL,
                    pMatrixBegin, pMatrixCount, pMatrixIndex, pMatrix,
                    NULL, NULL, NULL );
    nResult = CoinLoadInteger( hProb, pColType );

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

    nResult = CoinCheckProblem( hProb );
    nResult = CoinOptimizeProblem( hProb, 0 );

    mbSuccess = ( nResult == SOLV_CALL_SUCCESS );
    if ( mbSuccess )
    {
        // get solution

        maSolution.realloc( nVariables );
        CoinGetSolutionValues( hProb, maSolution.getArray(), NULL, NULL, NULL );
        mfResultValue = CoinGetObjectValue( hProb );
    }
    else
    {
        int nSolutionStatus = CoinGetSolutionStatus( hProb );
        if ( nSolutionStatus == 1 )
            maStatus = lcl_GetResourceString( RID_ERROR_INFEASIBLE );
        else if ( nSolutionStatus == 2 )
            maStatus = lcl_GetResourceString( RID_ERROR_UNBOUNDED );
        // TODO: detect timeout condition and report as RID_ERROR_TIMEOUT
        // (currently reported as infeasible)
    }

    CoinUnloadProblem( hProb );
}

// XServiceInfo

uno::Sequence< OUString > SolverComponent_getSupportedServiceNames()
{
    uno::Sequence< OUString > aServiceNames( 1 );
    aServiceNames[ 0 ] = OUString::createFromAscii( "com.sun.star.sheet.Solver" );
    return aServiceNames;
}

OUString SolverComponent_getImplementationName()
{
    return OUString::createFromAscii( "com.sun.star.comp.Calc.Solver" );
}

OUString SAL_CALL SolverComponent::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return SolverComponent_getImplementationName();
}

sal_Bool SAL_CALL SolverComponent::supportsService( const OUString& rServiceName ) throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}

uno::Sequence<OUString> SAL_CALL SolverComponent::getSupportedServiceNames() throw(uno::RuntimeException, std::exception)
{
    return SolverComponent_getSupportedServiceNames();
}

uno::Reference<uno::XInterface> SolverComponent_createInstance( const uno::Reference<uno::XComponentContext>& rSMgr )
    throw(uno::Exception)
{
    return (cppu::OWeakObject*) new SolverComponent( rSMgr );
}

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void* SAL_CALL solver_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
    {
        OUString    aImplName( OUString::createFromAscii( pImplName ) );
        void*       pRet = 0;

        if( pServiceManager )
        {
            uno::Reference< lang::XSingleComponentFactory > xFactory;
            if( aImplName.equals( SolverComponent_getImplementationName() ) )
                xFactory = cppu::createSingleComponentFactory(
                        SolverComponent_createInstance,
                        OUString::createFromAscii( pImplName ),
                        SolverComponent_getSupportedServiceNames() );

            if( xFactory.is() )
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
        return pRet;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
