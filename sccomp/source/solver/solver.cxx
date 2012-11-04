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
 ************************************************************************/

#undef LANGUAGE_NONE
#define WINAPI __stdcall
#define LoadInverseLib FALSE
#define LoadLanguageLib FALSE
#include <lpsolve/lp_lib.h>
#undef LANGUAGE_NONE

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
#include <vector>
#include <boost/unordered_map.hpp>

#include <tools/resmgr.hxx>

using namespace com::sun::star;

using ::rtl::OUString;

#define C2U(constAsciiStr) (::rtl::OUString( constAsciiStr  ))

#define STR_NONNEGATIVE   "NonNegative"
#define STR_INTEGER       "Integer"
#define STR_TIMEOUT       "Timeout"
#define STR_EPSILONLEVEL  "EpsilonLevel"
#define STR_LIMITBBDEPTH  "LimitBBDepth"

// -----------------------------------------------------------------------
//  Resources from tools are used for translated strings

static ResMgr* pSolverResMgr = NULL;

static OUString lcl_GetResourceString( sal_uInt32 nId )
{
    if (!pSolverResMgr)
        pSolverResMgr = ResMgr::CreateResMgr("solver");

    return String( ResId( nId, *pSolverResMgr ) );
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

typedef boost::unordered_map< table::CellAddress, std::vector<double>, ScSolverCellHash, ScSolverCellEqual > ScSolverCellHashMap;

// -----------------------------------------------------------------------

static uno::Reference<table::XCell> lcl_GetCell( const uno::Reference<sheet::XSpreadsheetDocument>& xDoc,
                                          const table::CellAddress& rPos )
{
    uno::Reference<container::XIndexAccess> xSheets( xDoc->getSheets(), uno::UNO_QUERY );
    uno::Reference<sheet::XSpreadsheet> xSheet( xSheets->getByIndex( rPos.Sheet ), uno::UNO_QUERY );
    return xSheet->getCellByPosition( rPos.Column, rPos.Row );
}

static void lcl_SetValue( const uno::Reference<sheet::XSpreadsheetDocument>& xDoc,
                   const table::CellAddress& rPos, double fValue )
{
    lcl_GetCell( xDoc, rPos )->setValue( fValue );
}

static double lcl_GetValue( const uno::Reference<sheet::XSpreadsheetDocument>& xDoc,
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

uno::Reference<beans::XPropertySetInfo> SAL_CALL SolverComponent::getPropertySetInfo() throw(uno::RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() );
}

// XSolverDescription

OUString SAL_CALL SolverComponent::getComponentDescription() throw (uno::RuntimeException)
{
    return lcl_GetResourceString( RID_SOLVER_COMPONENT );
}

OUString SAL_CALL SolverComponent::getStatusDescription() throw (uno::RuntimeException)
{
    return maStatus;
}

OUString SAL_CALL SolverComponent::getPropertyDescription( const OUString& rPropertyName ) throw (uno::RuntimeException)
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

uno::Reference<sheet::XSpreadsheetDocument> SAL_CALL SolverComponent::getDocument() throw(uno::RuntimeException)
{
    return mxDoc;
}

void SAL_CALL SolverComponent::setDocument( const uno::Reference<sheet::XSpreadsheetDocument>& _document )
                                throw(uno::RuntimeException)
{
    mxDoc = _document;
}

table::CellAddress SAL_CALL SolverComponent::getObjective() throw(uno::RuntimeException)
{
    return maObjective;
}

void SAL_CALL SolverComponent::setObjective( const table::CellAddress& _objective ) throw(uno::RuntimeException)
{
    maObjective = _objective;
}

uno::Sequence<table::CellAddress> SAL_CALL SolverComponent::getVariables() throw(uno::RuntimeException)
{
    return maVariables;
}

void SAL_CALL SolverComponent::setVariables( const uno::Sequence<table::CellAddress>& _variables )
                                throw(uno::RuntimeException)
{
    maVariables = _variables;
}

uno::Sequence<sheet::SolverConstraint> SAL_CALL SolverComponent::getConstraints() throw(uno::RuntimeException)
{
    return maConstraints;
}

void SAL_CALL SolverComponent::setConstraints( const uno::Sequence<sheet::SolverConstraint>& _constraints )
                                throw(uno::RuntimeException)
{
    maConstraints = _constraints;
}

sal_Bool SAL_CALL SolverComponent::getMaximize() throw(uno::RuntimeException)
{
    return mbMaximize;
}

void SAL_CALL SolverComponent::setMaximize( sal_Bool _maximize ) throw(uno::RuntimeException)
{
    mbMaximize = _maximize;
}

// XSolver: get results

sal_Bool SAL_CALL SolverComponent::getSuccess() throw(uno::RuntimeException)
{
    return mbSuccess;
}

double SAL_CALL SolverComponent::getResultValue() throw(uno::RuntimeException)
{
    return mfResultValue;
}

uno::Sequence<double> SAL_CALL SolverComponent::getSolution() throw(uno::RuntimeException)
{
    return maSolution;
}

// -------------------------------------------------------------------------

void SAL_CALL SolverComponent::solve() throw(uno::RuntimeException)
{
    uno::Reference<frame::XModel> xModel( mxDoc, uno::UNO_QUERY );
    if ( !xModel.is() )
        throw uno::RuntimeException();

    maStatus = OUString();
    mbSuccess = false;

    if ( mnEpsilonLevel < EPS_TIGHT || mnEpsilonLevel > EPS_BAGGY )
    {
        maStatus = lcl_GetResourceString( RID_ERROR_EPSILONLEVEL );
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

    if ( !maStatus.isEmpty() )
        return;

    //
    // build lp_solve model
    //

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
            REAL* pValues = new REAL[nVariables+1];
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
            add_constraint( lp, pValues, nConstrType, fRightValue );

            delete[] pValues;
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

        REAL* pResultVar = NULL;
        get_ptr_variables( lp, &pResultVar );
        for (nVar=0; nVar<nVariables; nVar++)
            maSolution[nVar] = pResultVar[nVar];

        mfResultValue = get_objective( lp );
    }
    else if ( nResult == INFEASIBLE )
        maStatus = lcl_GetResourceString( RID_ERROR_INFEASIBLE );
    else if ( nResult == UNBOUNDED )
        maStatus = lcl_GetResourceString( RID_ERROR_UNBOUNDED );
    else if ( nResult == TIMEOUT || nResult == SUBOPTIMAL )
        maStatus = lcl_GetResourceString( RID_ERROR_TIMEOUT );
    // SUBOPTIMAL is assumed to be caused by a timeout, and reported as an error

    delete_lp( lp );
}

// -------------------------------------------------------------------------

// XServiceInfo

uno::Sequence< OUString > SolverComponent_getSupportedServiceNames()
{
    uno::Sequence< OUString > aServiceNames( 1 );
    aServiceNames[ 0 ] = OUString("com.sun.star.sheet.Solver" );
    return aServiceNames;
}

OUString SolverComponent_getImplementationName()
{
    return OUString("com.sun.star.comp.Calc.Solver" );
}

OUString SAL_CALL SolverComponent::getImplementationName() throw(uno::RuntimeException)
{
    return SolverComponent_getImplementationName();
}

sal_Bool SAL_CALL SolverComponent::supportsService( const OUString& rServiceName ) throw(uno::RuntimeException)
{
    const uno::Sequence< OUString > aServices = SolverComponent_getSupportedServiceNames();
    const OUString* pArray = aServices.getConstArray();
    const OUString* pArrayEnd = pArray + aServices.getLength();
    return ::std::find( pArray, pArrayEnd, rServiceName ) != pArrayEnd;
}

uno::Sequence<OUString> SAL_CALL SolverComponent::getSupportedServiceNames() throw(uno::RuntimeException)
{
    return SolverComponent_getSupportedServiceNames();
}

uno::Reference<uno::XInterface> SolverComponent_createInstance( const uno::Reference<uno::XComponentContext>& rSMgr )
    throw(uno::Exception)
{
    return (cppu::OWeakObject*) new SolverComponent( rSMgr );
}

// -------------------------------------------------------------------------

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
