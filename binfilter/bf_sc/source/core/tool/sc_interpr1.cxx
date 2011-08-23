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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#ifdef RS6000
#pragma options FLTTRAP
#include <fptrap.h>
#include <fpxcp.h>
#endif

#include "scitems.hxx"
#include <bf_svx/langitem.hxx>
#include <bf_svx/algitem.hxx>
#include <unotools/textsearch.hxx>
#include <bf_svtools/zformat.hxx>
#include <tools/solar.h>
#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/printer.hxx>

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "interpre.hxx"
#include "patattr.hxx"
#include "dociter.hxx"
#include "scmatrix.hxx"
#include "docoptio.hxx"
#include "globstr.hrc"
#include "attrib.hxx"
namespace binfilter {


// PI jetzt als F_PI aus solar.h
//#define	PI			  3.1415926535897932
//#define	MINVALUE	  1.7e-307
//#define SQRT_2_PI   2.506628274631000

//	globale Variablen

/*N*/ #ifdef _MSC_VER
/*N*/ #pragma code_seg("SCSTATICS")
/*N*/ #endif

/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScTokenStack, 8, 4 )
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScErrorStack, 8, 4 )
/*N*/ IMPL_FIXEDMEMPOOL_NEWDEL( ScInterpreter, 32, 16 )

/*N*/ #ifdef _MSC_VER
/*N*/ #pragma code_seg()
/*N*/ #endif

/*N*/ ScTokenStack* ScInterpreter::pGlobalStack = NULL;
/*N*/ ScErrorStack* ScInterpreter::pGlobalErrorStack = NULL;
/*N*/ BOOL ScInterpreter::bGlobalStackInUse = FALSE;


//-----------------------------------------------------------------------------
// Funktionen
//-----------------------------------------------------------------------------


/*N*/ void ScInterpreter::ScIfJump()
/*N*/ {
/*N*/ 	const short* pJump = pCur->GetJump();
/*N*/ 	short nJumpCount = pJump[ 0 ];
/*N*/ 	if ( GetBool() )
/*N*/ 	{	// TRUE
/*N*/ 		if( nJumpCount >= 2 )
/*N*/ 		{	// then Zweig
/*N*/ 			nFuncFmtType = NUMBERFORMAT_UNDEFINED;
/*N*/ 			aCode.Jump( pJump[ 1 ], pJump[ nJumpCount ] );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{	// kein Parameter fuer then
/*?*/ 			nFuncFmtType = NUMBERFORMAT_LOGICAL;
/*?*/ 			PushInt(1);
/*?*/ 			aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{	// FALSE
/*N*/ 		if( nJumpCount == 3 )
/*N*/ 		{	// else Zweig
/*N*/ 			nFuncFmtType = NUMBERFORMAT_UNDEFINED;
/*N*/ 			aCode.Jump( pJump[ 2 ], pJump[ nJumpCount ] );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{	// kein Parameter fuer else
/*?*/ 			nFuncFmtType = NUMBERFORMAT_LOGICAL;
/*?*/ 			PushInt(0);
/*?*/ 			aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
/*N*/ 		}
/*N*/ 	}
/*N*/ }


void ScInterpreter::ScChoseJump()
{
    const short* pJump = pCur->GetJump();
    short nJumpCount = pJump[ 0 ];
    double nJumpIndex = ::rtl::math::approxFloor( GetDouble() );
    if ((nJumpIndex >= 1) && (nJumpIndex < nJumpCount))
        aCode.Jump( pJump[ (short) nJumpIndex ], pJump[ nJumpCount ] );
    else
        SetError(errIllegalArgument);
}


/*N*/ short ScInterpreter::CompareFunc( const ScCompare& rComp )
/*N*/ {
/*N*/ 	short nRes = 0;
/*N*/ 	if ( rComp.bEmpty[ 0 ] )
/*N*/ 	{
/*?*/ 		if ( rComp.bEmpty[ 1 ] )
/*?*/ 			;		// leere Zelle == leere Zelle, nRes 0
/*?*/ 		else if( rComp.bVal[ 1 ] )
/*?*/ 		{
/*?*/ 			if ( !::rtl::math::approxEqual( rComp.nVal[ 1 ], 0.0 ) )
/*?*/ 			{
/*?*/ 				if ( rComp.nVal[ 1 ] < 0.0 )
/*?*/ 					nRes = 1;		// leere Zelle > -x
/*?*/ 				else
/*?*/ 					nRes = -1;		// leere Zelle < x
/*?*/ 			}
/*?*/ 			// else: leere Zelle == 0.0
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			if ( rComp.pVal[ 1 ]->Len() )
/*?*/ 				nRes = -1;		// leere Zelle < "..."
/*?*/ 			// else: leere Zelle == ""
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else if ( rComp.bEmpty[ 1 ] )
/*N*/ 	{
/*N*/ 		if( rComp.bVal[ 0 ] )
/*N*/ 		{
/*N*/ 			if ( !::rtl::math::approxEqual( rComp.nVal[ 0 ], 0.0 ) )
/*N*/ 			{
/*N*/ 				if ( rComp.nVal[ 0 ] < 0.0 )
/*N*/ 					nRes = -1;		// -x < leere Zelle
/*N*/ 				else
/*N*/ 					nRes = 1;		// x > leere Zelle
/*N*/ 			}
/*N*/ 			// else: leere Zelle == 0.0
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			if ( rComp.pVal[ 0 ]->Len() )
/*?*/ 				nRes = 1;		// "..." > leere Zelle
/*?*/ 			// else: "" == leere Zelle
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if( rComp.bVal[ 0 ] )
/*N*/ 	{
/*N*/ 		if( rComp.bVal[ 1 ] )
/*N*/ 		{
/*N*/ 			if ( !::rtl::math::approxEqual( rComp.nVal[ 0 ], rComp.nVal[ 1 ] ) )
/*N*/ 			{
/*N*/ 				if( rComp.nVal[ 0 ] - rComp.nVal[ 1 ] < 0 )
/*N*/ 					nRes = -1;
/*N*/ 				else
/*N*/ 					nRes = 1;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			nRes = -1;	// Zahl ist kleiner als String
/*N*/ 	}
/*N*/ 	else if( rComp.bVal[ 1 ] )
/*N*/ 		nRes = 1;	// Zahl ist kleiner als String
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if (pDok->GetDocOptions().IsIgnoreCase())
/*?*/ 			nRes = (short) ScGlobal::pCollator->compareString(
/*?*/ 				*rComp.pVal[ 0 ], *rComp.pVal[ 1 ] );
/*N*/ 		else
/*?*/             nRes = (short) ScGlobal::pCaseCollator->compareString(
/*?*/ 				*rComp.pVal[ 0 ], *rComp.pVal[ 1 ] );
/*N*/ 	}
/*N*/ 	return nRes;
/*N*/ }


/*N*/ short ScInterpreter::Compare()
/*N*/ {
/*N*/ 	nCurFmtType = nFuncFmtType = NUMBERFORMAT_LOGICAL;
/*N*/ 	String aVal1, aVal2;
/*N*/ 	ScCompare aComp( &aVal1, &aVal2 );
/*N*/ 	for( short i = 1; i >= 0; i-- )
/*N*/ 	{
/*N*/ 		switch ( GetStackType() )
/*N*/ 		{
/*N*/ 			case svDouble:
/*N*/ 				aComp.nVal[ i ] = GetDouble();
/*N*/ 				aComp.bVal[ i ] = TRUE;
/*N*/ 				break;
/*N*/ 			case svString:
/*N*/ 				*aComp.pVal[ i ] = GetString();
/*N*/ 				aComp.bVal[ i ] = FALSE;
/*N*/ 				break;
/*N*/ 			case svDoubleRef :
/*N*/ 			case svSingleRef :
/*N*/ 			{
/*N*/ 				ScAddress aAdr;
/*N*/ 				if ( !PopDoubleRefOrSingleRef( aAdr ) )
/*N*/ 					break;
/*N*/ 				ScBaseCell* pCell = GetCell( aAdr );
/*N*/ 				if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
/*N*/ 				{
/*N*/ 					if (HasCellStringData(pCell))
/*N*/ 					{
/*N*/ 						GetCellString(*aComp.pVal[ i ], pCell);
/*N*/ 						aComp.bVal[ i ] = FALSE;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						aComp.nVal[ i ] = GetCellValue( aAdr, pCell );
/*N*/ 						aComp.bVal[ i ] = TRUE;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 					aComp.bEmpty[ i ] = TRUE;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			default:
/*N*/ 				SetError(errIllegalParameter);
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( nGlobalError )
/*N*/ 		return 0;
/*N*/ 	return CompareFunc( aComp );
/*N*/ }


ScMatrix* ScInterpreter::CompareMat()
{
    nCurFmtType = nFuncFmtType = NUMBERFORMAT_LOGICAL;
    String aVal1, aVal2;
    ScCompare aComp( &aVal1, &aVal2 );
    ScMatrix* pMat[2];
    pMat[0] = pMat[1] = NULL;
    USHORT nMatInd[2];
    BOOL bTmpMat[2];
    bTmpMat[0] = bTmpMat[1] = FALSE;
    ScAddress aAdr;
    for( short i = 1; i >= 0; i-- )
    {
        switch (GetStackType())
        {
            case svDouble:
                aComp.nVal[ i ] = GetDouble();
                aComp.bVal[ i ] = TRUE;
                break;
            case svString:
                *aComp.pVal[ i ] = GetString();
                aComp.bVal[ i ] = FALSE;
                break;
            case svSingleRef:
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
                {
                    if (HasCellStringData(pCell))
                    {
                        GetCellString(*aComp.pVal[ i ], pCell);
                        aComp.bVal[ i ] = FALSE;
                    }
                    else
                    {
                        aComp.nVal[ i ] = GetCellValue( aAdr, pCell );
                        aComp.bVal[ i ] = TRUE;
                    }
                }
                else
                    aComp.bEmpty[ i ] = TRUE;
            }
            break;
            case svDoubleRef:
                bTmpMat[ i ] = TRUE;
            case svMatrix:
                pMat[ i ] = GetMatrix( nMatInd[ i ] );
                if ( !pMat[ i ] )
                    SetError(errIllegalParameter);
                break;
            default:
                SetError(errIllegalParameter);
            break;
        }
    }
    ScMatrix* pResMat = NULL;
    if( !nGlobalError )
    {
        USHORT nResMatInd;
        if ( pMat[0] && pMat[1] )
        {
            USHORT nC0, nR0, nC1, nR1;
            pMat[0]->GetDimensions( nC0, nR0 );
            pMat[1]->GetDimensions( nC1, nR1 );
            USHORT nC = Max( nC0, nC1 );
            USHORT nR = Max( nR0, nR1 );
            pResMat = GetNewMat( nC, nR, nResMatInd );
            if ( !pResMat )
                return NULL;
            for ( USHORT j=0; j<nC; j++ )
            {
                for ( USHORT k=0; k<nR; k++ )
                {
                    if ( j < nC0 && j < nC1 && k < nR0 && k < nR1 )
                    {
                        for ( short i=1; i>=0; i-- )
                        {
                            if ( pMat[i]->IsString(j,k) )
                            {
                                aComp.bVal[i] = FALSE;
                                *aComp.pVal[i] = pMat[i]->GetString(j,k);
                                aComp.bEmpty[i] = pMat[i]->IsEmpty(j,k);
                            }
                            else
                            {
                                aComp.bVal[i] = TRUE;
                                aComp.nVal[i] = pMat[i]->GetDouble(j,k);
                                aComp.bEmpty[i] = FALSE;
                            }
                        }
                        pResMat->PutDouble( CompareFunc( aComp ), j,k );
                    }
                    else
                        pResMat->PutString( ScGlobal::GetRscString(STR_NO_VALUE), j,k );
                }
            }
            nRetMat = nResMatInd;
        }
        else if ( pMat[0] || pMat[1] )
        {
            short i = ( pMat[0] ? 0 : 1);
            USHORT nC, nR;
            pMat[i]->GetDimensions( nC, nR );
            pResMat = GetNewMat( nC, nR, nResMatInd );
            if ( !pResMat )
                return NULL;
            ULONG n = (ULONG) nC * nR;
            for ( ULONG j=0; j<n; j++ )
            {
                if ( pMat[i]->IsValue(j) )
                {
                    aComp.bVal[i] = TRUE;
                    aComp.nVal[i] = pMat[i]->GetDouble(j);
                    aComp.bEmpty[i] = FALSE;
                }
                else
                {
                    aComp.bVal[i] = FALSE;
                    *aComp.pVal[i] = pMat[i]->GetString(j);
                    aComp.bEmpty[i] = pMat[i]->IsEmpty(j);
                }
                pResMat->PutDouble( CompareFunc( aComp ), j );
            }
            nRetMat = nResMatInd;
        }
    }
    for( short x=1; x >= 0; x-- )
    {
        if ( bTmpMat[x] && pMat[x] )
        {
            delete pMat[x];
            ResetNewMat( nMatInd[x] );
        }
    }
    return pResMat;
}


/*N*/ void ScInterpreter::ScEqual()
/*N*/ {
/*N*/ 	StackVar eType;
/*N*/ 	if ( ((eType = GetStackType(2)) == svMatrix) ||
/*N*/ 			(bMatrixFormula && eType == svDoubleRef) ||
/*N*/ 			((eType = GetStackType(1)) == svMatrix) ||
/*N*/ 			(bMatrixFormula && eType == svDoubleRef)
/*N*/ 		)
/*N*/ 	{
/*?*/ 		ScMatrix* pMat = CompareMat();
/*?*/ 		if ( !pMat )
/*?*/ 			SetIllegalParameter();
/*?*/ 		else
/*?*/ 		{
/*?*/ 			pMat->CompareEqual();
/*?*/ 			PushMatrix( pMat );
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		PushInt( Compare() == 0 );
/*N*/ }


/*N*/ void ScInterpreter::ScNotEqual()
/*N*/ {
/*N*/ 	StackVar eType;
/*N*/ 	if ( ((eType = GetStackType(2)) == svMatrix) ||
/*N*/ 			(bMatrixFormula && eType == svDoubleRef) ||
/*N*/ 			((eType = GetStackType(1)) == svMatrix) ||
/*N*/ 			(bMatrixFormula && eType == svDoubleRef)
/*N*/ 		)
/*N*/ 	{
/*?*/ 		ScMatrix* pMat = CompareMat();
/*?*/ 		if ( !pMat )
/*?*/ 			SetIllegalParameter();
/*?*/ 		else
/*?*/ 		{
/*?*/ 			pMat->CompareNotEqual();
/*?*/ 			PushMatrix( pMat );
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		PushInt( Compare() != 0 );
/*N*/ }


void ScInterpreter::ScLess()
{
    StackVar eType;
    if ( ((eType = GetStackType(2)) == svMatrix) ||
            (bMatrixFormula && eType == svDoubleRef) ||
            ((eType = GetStackType(1)) == svMatrix) ||
            (bMatrixFormula && eType == svDoubleRef)
        )
    {
        ScMatrix* pMat = CompareMat();
        if ( !pMat )
            SetIllegalParameter();
        else
        {
            pMat->CompareLess();
            PushMatrix( pMat );
        }
    }
    else
        PushInt( Compare() < 0 );
}


void ScInterpreter::ScGreater()
{
    StackVar eType;
    if ( ((eType = GetStackType(2)) == svMatrix) ||
            (bMatrixFormula && eType == svDoubleRef) ||
            ((eType = GetStackType(1)) == svMatrix) ||
            (bMatrixFormula && eType == svDoubleRef)
        )
    {
        ScMatrix* pMat = CompareMat();
        if ( !pMat )
            SetIllegalParameter();
        else
        {
            pMat->CompareGreater();
            PushMatrix( pMat );
        }
    }
    else
        PushInt( Compare() > 0 );
}


void ScInterpreter::ScLessEqual()
{
    StackVar eType;
    if ( ((eType = GetStackType(2)) == svMatrix) ||
            (bMatrixFormula && eType == svDoubleRef) ||
            ((eType = GetStackType(1)) == svMatrix) ||
            (bMatrixFormula && eType == svDoubleRef)
        )
    {
        ScMatrix* pMat = CompareMat();
        if ( !pMat )
            SetIllegalParameter();
        else
        {
            pMat->CompareLessEqual();
            PushMatrix( pMat );
        }
    }
    else
        PushInt( Compare() <= 0 );
}


void ScInterpreter::ScGreaterEqual()
{
    StackVar eType;
    if ( ((eType = GetStackType(2)) == svMatrix) ||
            (bMatrixFormula && eType == svDoubleRef) ||
            ((eType = GetStackType(1)) == svMatrix) ||
            (bMatrixFormula && eType == svDoubleRef)
        )
    {
        ScMatrix* pMat = CompareMat();
        if ( !pMat )
            SetIllegalParameter();
        else
        {
            pMat->CompareGreaterEqual();
            PushMatrix( pMat );
        }
    }
    else
        PushInt( Compare() >= 0 );
}


void ScInterpreter::ScAnd()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCountMin( nParamCount, 1 ) )
    {
        BOOL bHaveValue = FALSE;
        short nRes = TRUE;
        while( nParamCount-- )
        {
            if ( !nGlobalError )
            {
                switch ( GetStackType() )
                {
                    case svDouble :
                        bHaveValue = TRUE;
                        nRes &= ( PopDouble() != 0.0 );
                    break;
                    case svString :
                        Pop();
                        SetError( errNoValue );
                    break;
                    case svSingleRef :
                    {
                        ScAddress aAdr;
                        PopSingleRef( aAdr );
                        if ( !nGlobalError )
                        {
                            ScBaseCell* pCell = GetCell( aAdr );
                            if ( HasCellValueData( pCell ) )
                            {
                                bHaveValue = TRUE;
                                nRes &= ( GetCellValue( aAdr, pCell ) != 0.0 );
                            }
                            // else: Xcl setzt hier keinen Fehler
                        }
                    }
                    break;
                    case svDoubleRef:
                    {
                        ScRange aRange;
                        PopDoubleRef( aRange );
                        if ( !nGlobalError )
                        {
                            double fVal;
                            USHORT nErr = 0;
                            ScValueIterator aValIter( pDok, aRange );
                            if ( aValIter.GetFirst( fVal, nErr ) )
                            {
                                bHaveValue = TRUE;
                                do
                                {
                                    nRes &= ( fVal != 0.0 );
                                } while ( (nErr == 0) &&
                                    aValIter.GetNext( fVal, nErr ) );
                            }
                            SetError( nErr );
                        }
                    }
                    break;
                    case svMatrix:
                    {
                        USHORT nMatInd;
                        ScMatrix* pMat = GetMatrix( nMatInd );
                        if ( pMat )
                        {
                            bHaveValue = TRUE;
                            nRes &= pMat->And();
                        }
                        // else: GetMatrix hat errIllegalParameter gesetzt
                    }
                    break;
                    default:
                        Pop();
                        SetError( errIllegalParameter );
                }
            }
            else
                Pop();
        }
        if ( bHaveValue )
            PushInt( nRes );
        else
            SetNoValue();
    }
}


void ScInterpreter::ScOr()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCountMin( nParamCount, 1 ) )
    {
        BOOL bHaveValue = FALSE;
        short nRes = FALSE;
        while( nParamCount-- )
        {
            if ( !nGlobalError )
            {
                switch ( GetStackType() )
                {
                    case svDouble :
                        bHaveValue = TRUE;
                        nRes |= ( PopDouble() != 0.0 );
                    break;
                    case svString :
                        Pop();
                        SetError( errNoValue );
                    break;
                    case svSingleRef :
                    {
                        ScAddress aAdr;
                        PopSingleRef( aAdr );
                        if ( !nGlobalError )
                        {
                            ScBaseCell* pCell = GetCell( aAdr );
                            if ( HasCellValueData( pCell ) )
                            {
                                bHaveValue = TRUE;
                                nRes |= ( GetCellValue( aAdr, pCell ) != 0.0 );
                            }
                            // else: Xcl setzt hier keinen Fehler
                        }
                    }
                    break;
                    case svDoubleRef:
                    {
                        ScRange aRange;
                        PopDoubleRef( aRange );
                        if ( !nGlobalError )
                        {
                            double fVal;
                            USHORT nErr = 0;
                            ScValueIterator aValIter( pDok, aRange );
                            if ( aValIter.GetFirst( fVal, nErr ) )
                            {
                                bHaveValue = TRUE;
                                do
                                {
                                    nRes |= ( fVal != 0.0 );
                                } while ( (nErr == 0) &&
                                    aValIter.GetNext( fVal, nErr ) );
                            }
                            SetError( nErr );
                        }
                    }
                    break;
                    case svMatrix:
                    {
                        bHaveValue = TRUE;
                        USHORT nMatInd;
                        ScMatrix* pMat = GetMatrix( nMatInd );
                        if ( pMat )
                        {
                            bHaveValue = TRUE;
                            nRes |= pMat->Or();
                        }
                        // else: GetMatrix hat errIllegalParameter gesetzt
                    }
                    break;
                    default:
                        Pop();
                        SetError( errIllegalParameter );
                }
            }
            else
                Pop();
        }
        if ( bHaveValue )
            PushInt( nRes );
        else
            SetNoValue();
    }
}


void ScInterpreter::ScNeg()
{
    MatrixDoubleRefToMatrix();
    switch ( GetStackType() )
    {
        case svMatrix :
        {
            USHORT nMatInd;
            ScMatrix* pMat = GetMatrix( nMatInd );
            if ( pMat )
            {
                USHORT nC, nR;
                pMat->GetDimensions( nC, nR );
                USHORT nResMat;
                ScMatrix* pResMat = GetNewMat( nC, nR, nResMat );
                if ( !pResMat )
                    SetNoValue();
                else
                {
                    ULONG nCount = nC * nR;
                    for ( ULONG j=0; j<nCount; ++j )
                    {
                        if ( pMat->IsValueOrEmpty(j) )
                            pResMat->PutDouble( -pMat->GetDouble(j), j );
                        else
                            pResMat->PutString(
                                ScGlobal::GetRscString( STR_NO_VALUE ), j );
                    }
                    nRetMat = nResMat;
                    PushMatrix( pResMat );
                }
            }
        }
        break;
        default:
            PushDouble( -GetDouble() );
    }
}


void ScInterpreter::ScPercentSign()
{
    nFuncFmtType = NUMBERFORMAT_PERCENT;
    const ScToken* pSaveCur = pCur;
    BYTE nSavePar = cPar;
    PushInt( 100 );
    cPar = 2;
    ScByteToken aDivOp( ocDiv, cPar );
    pCur = &aDivOp;
    ScDiv();
    pCur = pSaveCur;
    cPar = nSavePar;
}


void ScInterpreter::ScNot()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    PushInt( GetDouble() == 0.0 );
}


/*N*/ void ScInterpreter::ScPi()
/*N*/ {
/*N*/ 	PushDouble(F_PI);
/*N*/ }


void ScInterpreter::ScRandom()
{
    PushDouble((double)rand() / RAND_MAX);
}


void ScInterpreter::ScTrue()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    PushInt(1);
}


void ScInterpreter::ScFalse()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    PushInt(0);
}


void ScInterpreter::ScDeg()
{
    PushDouble((GetDouble() / F_PI) * 180.0);
}


void ScInterpreter::ScRad()
{
    PushDouble(GetDouble() * (F_PI / 180));
}


/*N*/ void ScInterpreter::ScSin()
/*N*/ {
/*N*/     PushDouble(::rtl::math::sin(GetDouble()));
/*N*/ }


void ScInterpreter::ScCos()
{
    PushDouble(::rtl::math::cos(GetDouble()));
}


void ScInterpreter::ScTan()
{
    PushDouble(::rtl::math::tan(GetDouble()));
}


void ScInterpreter::ScCot()
{
    PushDouble(1.0 / ::rtl::math::tan(GetDouble()));
}


void ScInterpreter::ScArcSin()
{
    PushDouble(asin(GetDouble()));
}


void ScInterpreter::ScArcCos()
{
    PushDouble(acos(GetDouble()));
}


void ScInterpreter::ScArcTan()
{
    PushDouble(atan(GetDouble()));
}


void ScInterpreter::ScArcCot()
{
    PushDouble((F_PI2) - atan(GetDouble()));
}


void ScInterpreter::ScSinHyp()
{
    PushDouble(sinh(GetDouble()));
}


void ScInterpreter::ScCosHyp()
{
    PushDouble(cosh(GetDouble()));
}


void ScInterpreter::ScTanHyp()
{
    PushDouble(tanh(GetDouble()));
}


void ScInterpreter::ScCotHyp()
{
    PushDouble(1.0 / tanh(GetDouble()));
}


void ScInterpreter::ScArcSinHyp()
{
    double nVal = GetDouble();
    PushDouble(log(nVal + sqrt((nVal * nVal) + 1.0)));
}


void ScInterpreter::ScArcCosHyp()
{
    double nVal = GetDouble();
    if (nVal < 1.0)
        SetIllegalArgument();
    else
        PushDouble(log(nVal + sqrt((nVal * nVal) - 1.0)));
}


void ScInterpreter::ScArcTanHyp()
{
    double nVal = GetDouble();
    if (fabs(nVal) >= 1.0)
        SetIllegalArgument();
    else
        PushDouble(0.5 * log((1.0 + nVal) / (1.0 - nVal)));
}


void ScInterpreter::ScArcCotHyp()
{
    double nVal = GetDouble();
    if (fabs(nVal) <= 1.0)
        SetIllegalArgument();
    else
        PushDouble(0.5 * log((nVal + 1.0) / (nVal - 1.0)));
}


void ScInterpreter::ScExp()
{
    PushDouble(exp(GetDouble()));
}


void ScInterpreter::ScSqrt()
{
    double fVal = GetDouble();
    if (fVal >= 0.0)
        PushDouble(sqrt(fVal));
    else
        SetIllegalArgument();
}


void ScInterpreter::ScIsEmpty()
{
    short nRes = 0;
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;
            CellType eCellType = GetCellType( GetCell( aAdr ) );
            if((eCellType == CELLTYPE_NONE) || (eCellType == CELLTYPE_NOTE))
                nRes = 1;
        }
        break;
        default:
            Pop();
    }
    nGlobalError = 0;
    PushInt( nRes );
}


short ScInterpreter::IsString()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    short nRes = 0;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;
            ScBaseCell* pCell = GetCell( aAdr );
            if (GetCellErrCode( pCell ) == 0)
            {
                switch ( GetCellType( pCell ) )
                {
                    case CELLTYPE_STRING :
                    case CELLTYPE_EDIT :
                        nRes = 1;
                        break;
                    case CELLTYPE_FORMULA :
                        nRes = !((ScFormulaCell*)pCell)->IsValue();
                        break;
                }
            }
        }
        break;
        case svString:
            PopError();
            if ( !nGlobalError )
                nRes = 1;
            break;
        default:
            Pop();
    }
    nGlobalError = 0;
    return nRes;
}


void ScInterpreter::ScIsString()
{
    PushInt( IsString() );
}


void ScInterpreter::ScIsNonString()
{
    PushInt( !IsString() );
}


void ScInterpreter::ScIsLogical(UINT16 aOldNumType)
{
    short nRes = 0;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;
            ScBaseCell* pCell = GetCell( aAdr );
            if (GetCellErrCode( pCell ) == 0)
            {
                if (HasCellValueData(pCell))
                {
                    ULONG nFormat = GetCellNumberFormat( aAdr, pCell );
                    nRes = ( pFormatter->GetType(nFormat)
                                                 == NUMBERFORMAT_LOGICAL);
                }
            }
        }
        break;
        default:
            PopError();
            if ( !nGlobalError )
                nRes = ( nCurFmtType == NUMBERFORMAT_LOGICAL );
    }
    nCurFmtType = nFuncFmtType = NUMBERFORMAT_LOGICAL;
    nGlobalError = 0;
    PushInt( nRes );
}


void ScInterpreter::ScType()
{
    short nType = 0;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;
            ScBaseCell* pCell = GetCell( aAdr );
            if (GetCellErrCode( pCell ) == 0)
            {
                switch ( GetCellType( pCell ) )
                {
                    case CELLTYPE_STRING :
                    case CELLTYPE_EDIT :
                        nType = 2;
                        break;
                    case CELLTYPE_VALUE :
                        {
                            ULONG nFormat = GetCellNumberFormat( aAdr, pCell );
                            if (pFormatter->GetType(nFormat)
                                                     == NUMBERFORMAT_LOGICAL)
                                nType = 4;
                            else
                                nType = 1;
                        }
                        break;
                    case CELLTYPE_FORMULA :
                        nType = 8;
                        break;
                    default:
                        SetIllegalParameter();
                }
            }
            else
                nType = 16;
        }
        break;
        case svString:
            PopError();
            if ( nGlobalError )
            {
                nType = 16;
                nGlobalError = 0;
            }
            else
                nType = 2;
            break;
        default:
            PopError();
            if ( nGlobalError )
            {
                nType = 16;
                nGlobalError = 0;
            }
            else
                nType = 1;
    }
    PushInt( nType );
}


inline BOOL lcl_FormatHasNegColor( const SvNumberformat* pFormat )
{
    return pFormat && pFormat->GetColor( 1 );
}


inline BOOL lcl_FormatHasOpenPar( const SvNumberformat* pFormat )
{
    return pFormat && (pFormat->GetFormatstring().Search( '(' ) != STRING_NOTFOUND);
}


void ScInterpreter::ScCell()
{	// ATTRIBUTE ; [REF]
    BYTE nParamCount = GetByte();
    if( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        ScAddress aCellPos( aPos );
        BOOL bError = FALSE;
        if( nParamCount == 2 )
            bError = !PopDoubleRefOrSingleRef( aCellPos );
        String aInfoType( GetString() );
        if( bError || nGlobalError )
            SetIllegalParameter();
        else
        {
            String			aResult;
            ScBaseCell*		pCell = GetCell( aCellPos );

            aInfoType.ToUpperAscii();

// *** ADDRESS INFO ***
            if( aInfoType.EqualsAscii( "COL" ) )
            {	// column number (1-based)
                PushInt( aCellPos.Col() + 1 );
            }
            else if( aInfoType.EqualsAscii( "ROW" ) )
            {	// row number (1-based)
                PushInt( aCellPos.Row() + 1 );
            }
            else if( aInfoType.EqualsAscii( "SHEET" ) )
            {	// table number (1-based)
                PushInt( aCellPos.Tab() + 1 );
            }
            else if( aInfoType.EqualsAscii( "ADDRESS" ) )
            {	// address formatted as [['FILENAME'#]$TABLE.]$COL$ROW
                USHORT nFlags = (aCellPos.Tab() == aPos.Tab()) ? (SCA_ABS) : (SCA_ABS_3D);
                aCellPos.Format( aResult, nFlags, pDok );
                PushString( aResult );
            }
            else if( aInfoType.EqualsAscii( "FILENAME" ) )
            {	// file name and table name: 'FILENAME'#$TABLE
                USHORT nTab = aCellPos.Tab();
                if( nTab < pDok->GetTableCount() )
                {
                    if( pDok->GetLinkMode( nTab ) == SC_LINK_VALUE )
                        pDok->GetName( nTab, aResult );
                    else
                    {
                        SfxObjectShell* pShell = pDok->GetDocumentShell();
                        if( pShell && pShell->GetMedium() )
                        {
                            aResult = (sal_Unicode) '\'';
                            aResult += pShell->GetMedium()->GetName();
                            aResult.AppendAscii( "'#$" );
                            String aTabName;
                            pDok->GetName( nTab, aTabName );
                            aResult += aTabName;
                        }
                    }
                }
                PushString( aResult );
            }
            else if( aInfoType.EqualsAscii( "COORD" ) )
            {	// address, lotus 1-2-3 formatted: $TABLE:$COL$ROW
                ScAddress( aCellPos.Tab(), 0, 0 ).Format( aResult, (SCA_COL_ABSOLUTE|SCA_VALID_COL) );
                aResult += ':';
                String aCellStr;
                aCellPos.Format( aCellStr, (SCA_COL_ABSOLUTE|SCA_VALID_COL|SCA_ROW_ABSOLUTE|SCA_VALID_ROW) );
                aResult += aCellStr;
                PushString( aResult );
            }

// *** CELL PROPERTIES ***
            else if( aInfoType.EqualsAscii( "CONTENTS" ) )
            {	// contents of the cell, no formatting
                if( pCell && pCell->HasStringData() )
                {
                    GetCellString( aResult, pCell );
                    PushString( aResult );
                }
                else
                    PushDouble( GetCellValue( aCellPos, pCell ) );
            }
            else if( aInfoType.EqualsAscii( "TYPE" ) )
            {	// b = blank; l = string (label); v = otherwise (value)
                if( HasCellStringData( pCell ) )
                    aResult = 'l';
                else
                    aResult = HasCellValueData( pCell ) ? 'v' : 'b';
                PushString( aResult );
            }
            else if( aInfoType.EqualsAscii( "WIDTH" ) )
            {	// column width (rounded off as count of zero characters in standard font and size)
                Printer*	pPrinter = pDok->GetPrinter();
                MapMode 	aOldMode( pPrinter->GetMapMode() );
                Font		aOldFont( pPrinter->GetFont() );
                Font		aDefFont;

                pPrinter->SetMapMode( MAP_TWIP );
                // font color doesn't matter here
                pDok->GetDefPattern()->GetFont( aDefFont, SC_AUTOCOL_BLACK, pPrinter );
                pPrinter->SetFont( aDefFont );
                long nZeroWidth = pPrinter->GetTextWidth( String( '0' ) );
                pPrinter->SetFont( aOldFont );
                pPrinter->SetMapMode( aOldMode );
                int nZeroCount = (int)(pDok->GetColWidth( aCellPos.Col(), aCellPos.Tab() ) / nZeroWidth);
                PushInt( nZeroCount );
            }
            else if( aInfoType.EqualsAscii( "PREFIX" ) )
            {	// ' = left; " = right; ^ = centered
                if( HasCellStringData( pCell ) )
                {
                    const SvxHorJustifyItem* pJustAttr = (const SvxHorJustifyItem*)
                        pDok->GetAttr( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), ATTR_HOR_JUSTIFY );
                    switch( pJustAttr->GetValue() )
                    {
                        case SVX_HOR_JUSTIFY_STANDARD:
                        case SVX_HOR_JUSTIFY_LEFT:
                        case SVX_HOR_JUSTIFY_BLOCK:		aResult = '\'';	break;
                        case SVX_HOR_JUSTIFY_CENTER:	aResult = '^';	break;
                        case SVX_HOR_JUSTIFY_RIGHT:		aResult = '"';	break;
                        case SVX_HOR_JUSTIFY_REPEAT:	aResult = '\\';	break;
                    }
                }
                PushString( aResult );
            }
            else if( aInfoType.EqualsAscii( "PROTECT" ) )
            {	// 1 = cell locked
                const ScProtectionAttr* pProtAttr = (const ScProtectionAttr*)
                    pDok->GetAttr( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), ATTR_PROTECTION );
                PushInt( pProtAttr->GetProtection() ? 1 : 0 );
            }

// *** FORMATTING ***
            else if( aInfoType.EqualsAscii( "FORMAT" ) )
            {	// specific format code for standard formats
                ULONG	nFormat = pDok->GetNumberFormat( aCellPos );
                BOOL	bAppendPrec = TRUE;
                USHORT	nPrec, nLeading;
                BOOL	bThousand, bIsRed;
                pFormatter->GetFormatSpecialInfo( nFormat, bThousand, bIsRed, nPrec, nLeading );

                switch( pFormatter->GetType( nFormat ) )
                {
                    case NUMBERFORMAT_NUMBER:		aResult = (bThousand ? ',' : 'F');	break;
                    case NUMBERFORMAT_CURRENCY:		aResult = 'C';						break;
                    case NUMBERFORMAT_SCIENTIFIC:	aResult = 'S';						break;
                    case NUMBERFORMAT_PERCENT:		aResult = 'P';						break;
                    default:
                    {
                        bAppendPrec = FALSE;
                        switch( pFormatter->GetIndexTableOffset( nFormat ) )
                        {
                            case NF_DATE_SYSTEM_SHORT:
                            case NF_DATE_SYS_DMMMYY:
                            case NF_DATE_SYS_DDMMYY:
                            case NF_DATE_SYS_DDMMYYYY:
                            case NF_DATE_SYS_DMMMYYYY:
                            case NF_DATE_DIN_DMMMYYYY:
                            case NF_DATE_SYS_DMMMMYYYY:
                            case NF_DATE_DIN_DMMMMYYYY:	aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D1" ) );	break;
                            case NF_DATE_SYS_DDMMM:		aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D2" ) );	break;
                            case NF_DATE_SYS_MMYY:		aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D3" ) );	break;
                            case NF_DATETIME_SYSTEM_SHORT_HHMM:
                            case NF_DATETIME_SYS_DDMMYYYY_HHMMSS:
                                                        aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D4" ) );	break;
                            case NF_DATE_DIN_MMDD:		aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D5" ) );	break;
                            case NF_TIME_HHMMSSAMPM:	aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D6" ) );	break;
                            case NF_TIME_HHMMAMPM:		aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D7" ) );	break;
                            case NF_TIME_HHMMSS:		aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D8" ) );	break;
                            case NF_TIME_HHMM:			aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D9" ) );	break;
                            default:					aResult = 'G';
                        }
                    }
                }
                if( bAppendPrec )
                    aResult += String::CreateFromInt32( nPrec );
                const SvNumberformat* pFormat = pFormatter->GetEntry( nFormat );
                if( lcl_FormatHasNegColor( pFormat ) )
                    aResult += '-';
                if( lcl_FormatHasOpenPar( pFormat ) )
                    aResult.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "()" ) );
                PushString( aResult );
            }
            else if( aInfoType.EqualsAscii( "COLOR" ) )
            {	// 1 = negative values are colored, otherwise 0
                const SvNumberformat* pFormat = pFormatter->GetEntry( pDok->GetNumberFormat( aCellPos ) );
                PushInt( lcl_FormatHasNegColor( pFormat ) ? 1 : 0 );
            }
            else if( aInfoType.EqualsAscii( "PARENTHESES" ) )
            {	// 1 = format string contains a '(' character, otherwise 0
                const SvNumberformat* pFormat = pFormatter->GetEntry( pDok->GetNumberFormat( aCellPos ) );
                PushInt( lcl_FormatHasOpenPar( pFormat ) ? 1 : 0 );
            }
            else
                SetIllegalArgument();
        }
    }
}


void ScInterpreter::ScIsRef()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    short nRes = 0;
    switch ( GetStackType() )
    {
        case svSingleRef :
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            if ( !nGlobalError )
                nRes = 1;
        }
        break;
        case svDoubleRef :
        {
            ScRange aRange;
            PopDoubleRef( aRange );
            if ( !nGlobalError )
                nRes = 1;
        }
        break;
        default:
            Pop();
    }
    nGlobalError = 0;
    PushInt( nRes );
}


void ScInterpreter::ScIsValue()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    short nRes = 0;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;
            ScBaseCell* pCell = GetCell( aAdr );
            if (GetCellErrCode( pCell ) == 0)
            {
                switch ( GetCellType( pCell ) )
                {
                    case CELLTYPE_VALUE :
                        nRes = 1;
                        break;
                    case CELLTYPE_FORMULA :
                        nRes = ((ScFormulaCell*)pCell)->IsValue();
                        break;
                }
            }
        }
        break;
        case svString:
            Pop();
            break;
        default:
            PopError();
            if ( !nGlobalError )
                nRes = 1;
    }
    nGlobalError = 0;
    PushInt( nRes );
}


void ScInterpreter::ScIsFormula()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    short nRes = 0;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;
            nRes = (GetCellType( GetCell( aAdr ) ) == CELLTYPE_FORMULA);
        }
        break;
        default:
            Pop();
    }
    nGlobalError = 0;
    PushInt( nRes );
}


void ScInterpreter::ScFormula()
{
    String aFormula;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;
            ScBaseCell* pCell = GetCell( aAdr );
            switch ( GetCellType( pCell ) )
            {
                case CELLTYPE_FORMULA :
                    ((ScFormulaCell*)pCell)->GetFormula( aFormula );
                break;
                default:
                    SetError( NOVALUE );
            }
        }
        break;
        default:
            Pop();
            SetError( NOVALUE );
    }
    PushString( aFormula );
}



void ScInterpreter::ScIsNV()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    short nRes = 0;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            PopDoubleRefOrSingleRef( aAdr );
            if ( nGlobalError == NOVALUE )
                nRes = 1;
            else
            {
                ScBaseCell* pCell = GetCell( aAdr );
                USHORT nErr = GetCellErrCode( pCell );
                nRes = (nErr == NOVALUE);
            }
        }
        break;
        default:
            PopError();
            if ( nGlobalError == NOVALUE )
                nRes = 1;
    }
    nGlobalError = 0;
    PushInt( nRes );
}


void ScInterpreter::ScIsErr()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    short nRes = 0;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            PopDoubleRefOrSingleRef( aAdr );
            if ( nGlobalError && nGlobalError != NOVALUE )
                nRes = 1;
            else
            {
                ScBaseCell* pCell = GetCell( aAdr );
                USHORT nErr = GetCellErrCode( pCell );
                nRes = (nErr && nErr != NOVALUE);
            }
        }
        break;
        default:
            PopError();
            if ( nGlobalError && nGlobalError != NOVALUE )
                nRes = 1;
    }
    nGlobalError = 0;
    PushInt( nRes );
}


void ScInterpreter::ScIsError()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    short nRes = 0;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
            {
                nRes = 1;
                break;
            }
            if ( nGlobalError )
                nRes = 1;
            else
            {
                ScBaseCell* pCell = GetCell( aAdr );
                nRes = (GetCellErrCode( pCell ) != 0);
            }
        }
        break;
        default:
            PopError();
            if ( nGlobalError )
                nRes = 1;
    }
    nGlobalError = 0;
    PushInt( nRes );
}


short ScInterpreter::IsEven()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    short nRes = 0;
    double fVal;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;
            ScBaseCell* pCell = GetCell( aAdr );
            USHORT nErr = GetCellErrCode( pCell );
            if (nErr != 0)
                SetError(nErr);
            else
            {
                switch ( GetCellType( pCell ) )
                {
                    case CELLTYPE_VALUE :
                        fVal = GetCellValue( aAdr, pCell );
                        nRes = 1;
                        break;
                    case CELLTYPE_FORMULA :
                        if( ((ScFormulaCell*)pCell)->IsValue() )
                        {
                            fVal = GetCellValue( aAdr, pCell );
                            nRes = 1;
                        }
                        else
                            SetIllegalParameter();
                        break;
                    default:
                        SetIllegalParameter();
                }
            }
        }
        break;
        case svDouble:
        {
            fVal = PopDouble();
            nRes = 1;
        }
        break;
        default:
            SetIllegalParameter();
    }
    if (nRes)
        nRes = ( fmod( ::rtl::math::approxFloor( fabs( fVal ) ), 2.0 ) < 0.5 );
    return nRes;
}


void ScInterpreter::ScIsEven()
{
    PushInt( IsEven() );
}


void ScInterpreter::ScIsOdd()
{
    PushInt( !IsEven() );
}


void ScInterpreter::ScN()
{
    USHORT nErr = nGlobalError;
    nGlobalError = 0;
    double fVal = GetDouble();
    if ( nGlobalError == NOVALUE || nGlobalError == errIllegalArgument )
        nGlobalError = 0;		// N(#NV) und N("text") sind ok
    if ( !nGlobalError && nErr != NOVALUE )
        nGlobalError = nErr;
    PushDouble( fVal );
}


void ScInterpreter::ScTrim()
{   // trimmt nicht nur sondern schnibbelt auch doppelte raus!
    String aVal( GetString() );
    aVal.EraseLeadingChars();
    aVal.EraseTrailingChars();
    String aStr;
    register const sal_Unicode* p = aVal.GetBuffer();
    register const sal_Unicode* const pEnd = p + aVal.Len();
    while ( p < pEnd )
    {
        if ( *p != ' ' || p[-1] != ' ' )	// erster kann kein ' ' sein, -1 ist also ok
            aStr += *p;
        p++;
    }
    PushString( aStr );
}


void ScInterpreter::ScUpper()
{
    String aString = GetString();
    ScGlobal::pCharClass->toUpper(aString);
    PushString(aString);
}


void ScInterpreter::ScPropper()
{
//2do: what to do with I18N-CJK ?!?
    String aStr( GetString() );
    const xub_StrLen nLen = aStr.Len();
    // #i82487#,#i89963# don't try to write to empty string's BufferAccess
    // (would crash now that the empty string is const)
    if ( nLen > 0 )
    {
        String aUpr( ScGlobal::pCharClass->upper( aStr ) );
        String aLwr( ScGlobal::pCharClass->lower( aStr ) );
        register sal_Unicode* pStr = aStr.GetBufferAccess();
        const sal_Unicode* pUpr = aUpr.GetBuffer();
        const sal_Unicode* pLwr = aLwr.GetBuffer();
        *pStr = *pUpr;
        String aTmpStr( 'x' );
        xub_StrLen nPos = 1;
        while( nPos < nLen )
        {
            aTmpStr.SetChar( 0, pStr[nPos-1] );
            if ( !ScGlobal::pCharClass->isLetter( aTmpStr, 0 ) )
                pStr[nPos] = pUpr[nPos];
            else
                pStr[nPos] = pLwr[nPos];
            nPos++;
        }
        aStr.ReleaseBufferAccess( nLen );
    }
    PushString( aStr );
}


void ScInterpreter::ScLower()
{
    String aString( GetString() );
    ScGlobal::pCharClass->toLower(aString);
    PushString(aString);
}


void ScInterpreter::ScLen()
{
    String aStr( GetString() );
    PushDouble( aStr.Len() );
}


void ScInterpreter::ScT()
{
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
            {
                PushInt(0);
                return ;
            }
            BOOL bValue = FALSE;
            ScBaseCell* pCell = GetCell( aAdr );
            if ( GetCellErrCode( pCell ) == 0 )
            {
                switch ( GetCellType( pCell ) )
                {
                    case CELLTYPE_VALUE :
                        bValue = TRUE;
                        break;
                    case CELLTYPE_FORMULA :
                        bValue = ((ScFormulaCell*)pCell)->IsValue();
                        break;
                }
            }
            if ( bValue )
                PushString( EMPTY_STRING );
            else
            {
                //	wie GetString()
                GetCellString( aTempStr, pCell );
                PushString( aTempStr );
            }
        }
        break;
        case svDouble :
        {
            PopError();
            PushString( EMPTY_STRING );
        }
        break;
        case svString :
        break;
        default :
            SetError(errUnknownOpCode);
            PushInt(0);

    }
}


void ScInterpreter::ScValue()
{
    String aInputString = GetString();
    sal_uInt32 nFIndex = 0;					// damit default Land/Spr.
    double fVal;
    if (pFormatter->IsNumberFormat(aInputString, nFIndex, fVal))
        PushDouble(fVal);
    else
        SetIllegalArgument();
}


//2do: this should be a proper unicode string method
inline BOOL lcl_ScInterpreter_IsPrintable( sal_Unicode c )
{
    return 0x20 <= c && c != 0x7f;
}

void ScInterpreter::ScClean()
{
    String aStr( GetString() );
    for ( xub_StrLen i = 0; i < aStr.Len(); i++ )
    {
        if ( !lcl_ScInterpreter_IsPrintable( aStr.GetChar( i ) ) )
            aStr.Erase(i,1);
    }
    PushString(aStr);
}


void ScInterpreter::ScCode()
{
//2do: make it full range unicode?
    const String& rStr = GetString();
    PushInt( (sal_uChar) ByteString::ConvertFromUnicode( rStr.GetChar(0), gsl_getSystemTextEncoding() ) );
}


void ScInterpreter::ScChar()
{
//2do: make it full range unicode?
    double fVal = GetDouble();
    if (fVal < 0.0 || fVal >= 256.0)
        SetIllegalArgument();
    else
    {
        String aStr( '0' );
        aStr.SetChar( 0, ByteString::ConvertToUnicode( (sal_Char) fVal, gsl_getSystemTextEncoding() ) );
        PushString( aStr );
    }
}


/*N*/ void ScInterpreter::ScMin( BOOL bTextAsZero )
/*N*/ {
/*N*/ 	BYTE nParamCount = GetByte();
/*N*/ 	double nMin = SC_DOUBLE_MAXVALUE;
/*N*/ 	double nVal = 0.0;
/*N*/ 	ScAddress aAdr;
/*N*/ 	ScRange aRange;
/*N*/ 	for (short i = 0; i < nParamCount; i++)
/*N*/ 	{
/*N*/ 		switch (GetStackType())
/*N*/ 		{
/*?*/ 			case svDouble :
/*?*/ 			{
/*?*/ 				nVal = GetDouble();
/*?*/ 				if (nMin > nVal) nMin = nVal;
/*?*/ 				nFuncFmtType = NUMBERFORMAT_NUMBER;
/*?*/ 			}
/*?*/ 			break;
/*?*/ 			case svSingleRef :
/*?*/ 			{
/*?*/ 				PopSingleRef( aAdr );
/*?*/ 				ScBaseCell* pCell = GetCell( aAdr );
/*?*/ 				if (HasCellValueData(pCell))
/*?*/ 				{
/*?*/ 					nVal = GetCellValue( aAdr, pCell );
/*?*/ 					CurFmtToFuncFmt();
/*?*/ 					if (nMin > nVal) nMin = nVal;
/*?*/ 				}
/*?*/ 				else if ( bTextAsZero && HasCellStringData( pCell ) )
/*?*/ 				{
/*?*/ 					if ( nMin > 0.0 )
/*?*/ 						nMin = 0.0;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*N*/ 			case svDoubleRef :
/*N*/ 			{
/*N*/ 				USHORT nErr = 0;
/*N*/ 				PopDoubleRef( aRange );
/*N*/ 				ScValueIterator aValIter( pDok, aRange, glSubTotal, bTextAsZero );
/*N*/ 				if (aValIter.GetFirst(nVal, nErr))
/*N*/ 				{
/*N*/ 					if (nMin > nVal)
/*N*/ 						nMin = nVal;
/*N*/ 					aValIter.GetCurNumFmtInfo( nFuncFmtType, nFuncFmtIndex );
/*N*/ 					while ((nErr == 0) && aValIter.GetNext(nVal, nErr))
/*N*/ 					{
/*N*/ 						if (nMin > nVal)
/*N*/ 							nMin = nVal;
/*N*/ 					}
/*N*/ 					SetError(nErr);
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*?*/ 			case svMatrix :
/*?*/ 			{
/*?*/ 				ScMatrix* pMat = PopMatrix();
/*?*/ 				if (pMat)
/*?*/ 				{
/*?*/ 					USHORT nC, nR;
/*?*/ 					nFuncFmtType = NUMBERFORMAT_NUMBER;
/*?*/ 					pMat->GetDimensions(nC, nR);
/*?*/ 					if (pMat->IsNumeric())
/*?*/ 					{
/*?*/ 						for (USHORT i = 0; i < nC; i++)
/*?*/ 							for (USHORT j = 0; j < nR; j++)
/*?*/ 							{
/*?*/ 								nVal = pMat->GetDouble(i,j);
/*?*/ 								if (nMin > nVal) nMin = nVal;
/*?*/ 							}
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						for (USHORT i = 0; i < nC; i++)
/*?*/ 						{
/*?*/ 							for (USHORT j = 0; j < nR; j++)
/*?*/ 							{
/*?*/ 								if (!pMat->IsString(i,j))
/*?*/ 								{
/*?*/ 									nVal = pMat->GetDouble(i,j);
/*?*/ 									if (nMin > nVal) nMin = nVal;
/*?*/ 								}
/*?*/ 								else if ( bTextAsZero )
/*?*/ 								{
/*?*/ 									if ( nMin > 0.0 )
/*?*/ 										nMin = 0.0;
/*?*/ 								}
/*?*/ 							}
/*?*/ 						 }
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*?*/ 			case svString :
/*?*/ 			{
/*?*/ 				Pop();
/*?*/ 				if ( bTextAsZero )
/*?*/ 				{
/*?*/ 					if ( nMin > 0.0 )
/*?*/ 						nMin = 0.0;
/*?*/ 				}
/*?*/ 				else
/*?*/ 					SetError(errIllegalParameter);
/*?*/ 			}
/*?*/ 			break;
/*?*/ 			default :
/*?*/ 				Pop();
/*?*/ 				SetError(errIllegalParameter);
/*?*/ 		}
/*N*/ 	}
/*N*/ 	if (nMin == SC_DOUBLE_MAXVALUE)
/*?*/ 		SetIllegalArgument();
/*N*/ 	else
/*N*/ 		PushDouble(nMin);
/*N*/ }
/*N*/ 
/*N*/ #if defined(WIN) && defined(MSC)
/*N*/ #pragma optimize("",off)
/*N*/ #endif

/*N*/ void ScInterpreter::ScMax( BOOL bTextAsZero )
/*N*/ {
/*N*/ 	BYTE nParamCount = GetByte();
/*N*/ 	double nMax = -SC_DOUBLE_MAXVALUE;
/*N*/ 	double nVal = 0.0;
/*N*/ 	ScAddress aAdr;
/*N*/ 	ScRange aRange;
/*N*/ 	for (short i = 0; i < nParamCount; i++)
/*N*/ 	{
/*N*/ 		switch (GetStackType())
/*N*/ 		{
/*?*/ 			case svDouble :
/*?*/ 			{
/*?*/ 				nVal = GetDouble();
/*?*/ 				if (nMax < nVal) nMax = nVal;
/*?*/ 				nFuncFmtType = NUMBERFORMAT_NUMBER;
/*?*/ 			}
/*?*/ 			break;
/*N*/ 			case svSingleRef :
/*N*/ 			{
/*N*/ 				PopSingleRef( aAdr );
/*N*/ 				ScBaseCell* pCell = GetCell( aAdr );
/*N*/ 				if (HasCellValueData(pCell))
/*N*/ 				{
/*N*/ 					nVal = GetCellValue( aAdr, pCell );
/*N*/ 					CurFmtToFuncFmt();
/*N*/ 					if (nMax < nVal) nMax = nVal;
/*N*/ 				}
/*N*/ 				else if ( bTextAsZero && HasCellStringData( pCell ) )
/*N*/ 				{
/*N*/ 					if ( nMax < 0.0 )
/*N*/ 						nMax = 0.0;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case svDoubleRef :
/*N*/ 			{
/*N*/ 				USHORT nErr = 0;
/*N*/ 				PopDoubleRef( aRange );
/*N*/ 				ScValueIterator aValIter( pDok, aRange, glSubTotal, bTextAsZero );
/*N*/ 				if (aValIter.GetFirst(nVal, nErr))
/*N*/ 				{
/*N*/ 					if (nMax < nVal)
/*N*/ 						nMax = nVal;
/*N*/ 					aValIter.GetCurNumFmtInfo( nFuncFmtType, nFuncFmtIndex );
/*N*/ 					while ((nErr == 0) && aValIter.GetNext(nVal, nErr))
/*N*/ 					{
/*N*/ 						if (nMax < nVal)
/*N*/ 							nMax = nVal;
/*N*/ 					}
/*N*/ 					SetError(nErr);
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*?*/ 			case svMatrix :
/*?*/ 			{
/*?*/ 				ScMatrix* pMat = PopMatrix();
/*?*/ 				if (pMat)
/*?*/ 				{
/*?*/ 					nFuncFmtType = NUMBERFORMAT_NUMBER;
/*?*/ 					USHORT nC, nR;
/*?*/ 					pMat->GetDimensions(nC, nR);
/*?*/ 					if (pMat->IsNumeric())
/*?*/ 					{
/*?*/ 						for (USHORT i = 0; i < nC; i++)
/*?*/ 							for (USHORT j = 0; j < nR; j++)
/*?*/ 							{
/*?*/ 								nVal = pMat->GetDouble(i,j);
/*?*/ 								if (nMax < nVal) nMax = nVal;
/*?*/ 							}
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						for (USHORT i = 0; i < nC; i++)
/*?*/ 						{
/*?*/ 							for (USHORT j = 0; j < nR; j++)
/*?*/ 							{
/*?*/ 								if (!pMat->IsString(i,j))
/*?*/ 								{
/*?*/ 									nVal = pMat->GetDouble(i,j);
/*?*/ 									if (nMax < nVal) nMax = nVal;
/*?*/ 								}
/*?*/ 								else if ( bTextAsZero )
/*?*/ 								{
/*?*/ 									if ( nMax < 0.0 )
/*?*/ 										nMax = 0.0;
/*?*/ 								}
/*?*/ 							}
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*?*/ 			case svString :
/*?*/ 			{
/*?*/ 				Pop();
/*?*/ 				if ( bTextAsZero )
/*?*/ 				{
/*?*/ 					if ( nMax < 0.0 )
/*?*/ 						nMax = 0.0;
/*?*/ 				}
/*?*/ 				else
/*?*/ 					SetError(errIllegalParameter);
/*?*/ 			}
/*?*/ 			break;
/*?*/ 			default :
/*?*/ 				Pop();
/*?*/ 				SetError(errIllegalParameter);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (nMax == -SC_DOUBLE_MAXVALUE)
/*N*/ 		SetIllegalArgument();
/*N*/ 	else
/*N*/ 		PushDouble(nMax);
/*N*/ }
/*N*/ #if defined(WIN) && defined(MSC)
/*N*/ #pragma optimize("",on)
/*N*/ #endif


/*N*/ double ScInterpreter::IterateParameters( ScIterFunc eFunc, BOOL bTextAsZero )
/*N*/ {
/*N*/ 	BYTE nParamCount = GetByte();
/*N*/ 	double fRes = ( eFunc == ifPRODUCT ) ? 1.0 : 0.0;
/*N*/ 	double fVal = 0.0;
/*N*/ 	double fMem = 0.0;
/*N*/ 	BOOL bNull = TRUE;
/*N*/ 	ULONG nCount = 0;
/*N*/ 	ScAddress aAdr;
/*N*/ 	ScRange aRange;
/*N*/ 	for (short i = 0; i < nParamCount; i++)
/*N*/ 	{
/*N*/ 		switch (GetStackType())
/*N*/ 		{
/*N*/ 
/*?*/ 			case svString:
/*?*/ 			{
/*?*/ 				if( eFunc == ifCOUNT )
/*?*/ 				{
/*?*/ 					String aStr( PopString() );
/*?*/ 					sal_uInt32 nFIndex = 0;					// damit default Land/Spr.
/*?*/ 					if ( bTextAsZero || pFormatter->IsNumberFormat(aStr, nFIndex, fVal))
/*?*/ 						nCount++;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					switch ( eFunc )
/*?*/ 					{
/*?*/ 						case ifAVERAGE:
/*?*/ 						case ifSUM:
/*?*/ 						case ifSUMSQ:
/*?*/ 						case ifPRODUCT:
/*?*/ 						{
/*?*/ 							if ( bTextAsZero )
/*?*/ 							{
/*?*/ 								Pop();
/*?*/ 								nCount++;
/*?*/ 								if ( eFunc == ifPRODUCT )
/*?*/ 									fRes = 0.0;
/*?*/ 							}
/*?*/ 							else
/*?*/ 							{
/*?*/ 								for ( ; i < nParamCount; i++ )
/*?*/ 									Pop();
/*?*/ 								SetError( errNoValue );
/*?*/ 							}
/*?*/ 						}
/*?*/ 						break;
/*?*/ 						default:
/*?*/ 							Pop();
/*?*/ 							nCount++;
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*?*/ 			case svDouble    :
/*?*/ 				fVal = GetDouble();
/*?*/ 				nCount++;
/*?*/ 				switch( eFunc )
/*?*/ 				{
/*?*/ 					case ifAVERAGE:
/*?*/ 					case ifSUM:
/*?*/ 						if ( bNull && fVal != 0.0 )
/*?*/ 						{
/*?*/ 							bNull = FALSE;
/*?*/ 							fMem = fVal;
/*?*/ 						}
/*?*/ 						else
/*?*/ 							fRes += fVal;
/*?*/ 						break;
/*?*/ 					case ifSUMSQ:	fRes += fVal * fVal; break;
/*?*/ 					case ifPRODUCT:	fRes *= fVal; break;
/*?*/ 				}
/*?*/ 				nFuncFmtType = NUMBERFORMAT_NUMBER;
/*?*/ 				break;
/*?*/ 			case svSingleRef :
/*?*/ 			{
/*?*/ 				PopSingleRef( aAdr );
/*?*/ 				ScBaseCell* pCell = GetCell( aAdr );
/*?*/ 				if ( pCell )
/*?*/ 				{
/*?*/ 					if( eFunc == ifCOUNT2 )
/*?*/ 					{
/*?*/ 						CellType eCellType = pCell->GetCellType();
/*?*/ 						if (eCellType != CELLTYPE_NONE && eCellType != CELLTYPE_NOTE)
/*?*/ 							nCount++;
/*?*/ 					}
/*?*/ 					else if ( pCell->HasValueData() )
/*?*/ 					{
/*?*/ 						nCount++;
/*?*/ 						fVal = GetCellValue( aAdr, pCell );
/*?*/ 						CurFmtToFuncFmt();
/*?*/ 						switch( eFunc )
/*?*/ 						{
/*?*/ 							case ifAVERAGE:
/*?*/ 							case ifSUM:
/*?*/ 								if ( bNull && fVal != 0.0 )
/*?*/ 								{
/*?*/ 									bNull = FALSE;
/*?*/ 									fMem = fVal;
/*?*/ 								}
/*?*/ 								else
/*?*/ 									fRes += fVal;
/*?*/ 								break;
/*?*/ 							case ifSUMSQ:	fRes += fVal * fVal; break;
/*?*/ 							case ifPRODUCT:	fRes *= fVal; break;
/*?*/ 						}
/*?*/ 					}
/*?*/ 					else if ( bTextAsZero && pCell->HasStringData() )
/*?*/ 					{
/*?*/ 						nCount++;
/*?*/ 						if ( eFunc == ifPRODUCT )
/*?*/ 							fRes = 0.0;
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*N*/ 			case svDoubleRef :
/*N*/ 			{
/*N*/ 				USHORT nErr = 0;
/*N*/ 				PopDoubleRef( aRange );
/*N*/ 				if( eFunc == ifCOUNT2 )
/*N*/ 				{
/*N*/ 					ScBaseCell* pCell;
/*N*/ 					ScCellIterator aIter( pDok, aRange, glSubTotal );
/*N*/ 					if (pCell = aIter.GetFirst())
/*N*/ 					{
/*N*/ 						do
/*N*/ 						{
/*N*/ 							CellType eType = pCell->GetCellType();
/*N*/ 							if( eType != CELLTYPE_NONE && eType != CELLTYPE_NOTE )
/*N*/ 								nCount++;
/*N*/ 						}
/*N*/ 						while ( pCell = aIter.GetNext());
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					ScValueIterator aValIter( pDok, aRange, glSubTotal, bTextAsZero );
/*N*/ 					if (aValIter.GetFirst(fVal, nErr))
/*N*/ 					{
/*N*/ 						//	Schleife aus Performance-Gruenden nach innen verlegt:
/*N*/ 						aValIter.GetCurNumFmtInfo( nFuncFmtType, nFuncFmtIndex );
/*N*/ 						switch( eFunc )
/*N*/ 						{
/*N*/ 							case ifAVERAGE:
/*N*/ 							case ifSUM:
/*N*/ 									do
/*N*/ 									{
/*N*/ 										SetError(nErr);
/*N*/ 										if ( bNull && fVal != 0.0 )
/*N*/ 										{
/*N*/ 											bNull = FALSE;
/*N*/ 											fMem = fVal;
/*N*/ 										}
/*N*/ 										else
/*N*/ 											fRes += fVal;
/*N*/ 										nCount++;
/*N*/ 									}
/*N*/ 									while (aValIter.GetNext(fVal, nErr));
/*N*/ 									break;
/*N*/ 							case ifSUMSQ:
/*N*/ 									do
/*N*/ 									{
/*N*/ 										SetError(nErr);
/*N*/ 										fRes += fVal * fVal;
/*N*/ 										nCount++;
/*N*/ 									}
/*N*/ 									while (aValIter.GetNext(fVal, nErr));
/*N*/ 									break;
/*N*/ 							case ifPRODUCT:
/*N*/ 									do
/*N*/ 									{
/*N*/ 										SetError(nErr);
/*N*/ 										fRes *= fVal;
/*N*/ 										nCount++;
/*N*/ 									}
/*N*/ 									while (aValIter.GetNext(fVal, nErr));
/*N*/ 									break;
/*N*/ 							default:				// count
/*N*/ 									do
/*N*/ 									{
/*N*/ 										SetError(nErr);
/*N*/ 										nCount++;
/*N*/ 									}
/*N*/ 									while (aValIter.GetNext(fVal, nErr));
/*N*/ 									break;
/*N*/ 						}
/*N*/ 						SetError( nErr );
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*?*/ 			case svMatrix :
/*?*/ 			{
/*?*/ 				ScMatrix* pMat = PopMatrix();
/*?*/ 				if (pMat)
/*?*/ 				{
/*?*/ 					USHORT nC, nR;
/*?*/ 					nFuncFmtType = NUMBERFORMAT_NUMBER;
/*?*/ 					pMat->GetDimensions(nC, nR);
/*?*/ 					if( eFunc == ifCOUNT2 )
/*?*/ 						nCount += (ULONG) nC * nR;
/*?*/ 					else
/*?*/ 					{
/*?*/ 						for (USHORT i = 0; i < nC; i++)
/*?*/ 						{
/*?*/ 							for (USHORT j = 0; j < nR; j++)
/*?*/ 							{
/*?*/ 								if (!pMat->IsString(i,j))
/*?*/ 								{
/*?*/ 									nCount++;
/*?*/ 									fVal = pMat->GetDouble(i,j);
/*?*/ 									switch( eFunc )
/*?*/ 									{
/*?*/ 										case ifAVERAGE:
/*?*/ 										case ifSUM:
/*?*/ 											if ( bNull && fVal != 0.0 )
/*?*/ 											{
/*?*/ 												bNull = FALSE;
/*?*/ 												fMem = fVal;
/*?*/ 											}
/*?*/ 											else
/*?*/ 												fRes += fVal;
/*?*/ 											break;
/*?*/ 										case ifSUMSQ:	fRes += fVal * fVal; break;
/*?*/ 										case ifPRODUCT:	fRes *= fVal; break;
/*?*/ 									}
/*?*/ 								}
/*?*/ 								else if ( bTextAsZero )
/*?*/ 								{
/*?*/ 									nCount++;
/*?*/ 									if ( eFunc == ifPRODUCT )
/*?*/ 										fRes = 0.0;
/*?*/ 								}
/*?*/ 							}
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*?*/ 			default :
/*?*/ 				for ( ; i < nParamCount; i++ )
/*?*/ 					Pop();
/*?*/ 				SetError(errIllegalParameter);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	switch( eFunc )
/*N*/ 	{
/*N*/ 		case ifSUM:		fRes = ::rtl::math::approxAdd( fRes, fMem ); break;
/*N*/ 		case ifAVERAGE:	fRes = ::rtl::math::approxAdd( fRes, fMem ) / nCount; break;
/*N*/ 		case ifCOUNT2:
/*N*/ 		case ifCOUNT:	fRes  = nCount; break;
/*N*/ 		case ifPRODUCT:	if ( !nCount ) fRes = 0.0; break;
/*N*/ 	}
/*N*/ 	// Bei Summen etc. macht ein BOOL-Ergebnis keinen Sinn
/*N*/ 	// und Anzahl ist immer Number (#38345#)
/*N*/ 	if( eFunc == ifCOUNT || nFuncFmtType == NUMBERFORMAT_LOGICAL )
/*N*/ 		nFuncFmtType = NUMBERFORMAT_NUMBER;
/*N*/ 	return fRes;
/*N*/ }


void ScInterpreter::ScSumSQ()
{
    PushDouble( IterateParameters( ifSUMSQ ) );
}


/*N*/ void ScInterpreter::ScSum()
/*N*/ {
/*N*/ 	PushDouble( IterateParameters( ifSUM ) );
/*N*/ }


void ScInterpreter::ScProduct()
{
    PushDouble( IterateParameters( ifPRODUCT ) );
}


/*N*/ void ScInterpreter::ScAverage( BOOL bTextAsZero )
/*N*/ {
/*N*/ 	PushDouble( IterateParameters( ifAVERAGE, bTextAsZero ) );
/*N*/ }


void ScInterpreter::ScCount()
{
    PushDouble( IterateParameters( ifCOUNT ) );
}


/*N*/ void ScInterpreter::ScCount2()
/*N*/ {
/*N*/ 	PushDouble( IterateParameters( ifCOUNT2 ) );
/*N*/ }


/*N*/ void ScInterpreter::GetStVarParams( double& rVal, double& rValCount,
/*N*/ 				BOOL bTextAsZero )
/*N*/ {
/*N*/ 	BYTE nParamCount = GetByte();
/*N*/ 	USHORT i;
/*N*/ 	double fSum    = 0.0;
/*N*/ 	double fSumSqr = 0.0;
/*N*/ 	double fVal;
/*N*/ 	rValCount = 0.0;
/*N*/ 	ScAddress aAdr;
/*N*/ 	ScRange aRange;
/*N*/ 	for (i = 0; i < nParamCount; i++)
/*N*/ 	{
/*N*/ 		switch (GetStackType())
/*N*/ 		{
/*N*/ 			case svDouble :
/*N*/ 			{
/*?*/ 				fVal = GetDouble();
/*?*/ 				fSum    += fVal;
/*?*/ 				fSumSqr += fVal*fVal;
/*?*/ 				rValCount++;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case svSingleRef :
/*N*/ 			{
/*?*/ 				PopSingleRef( aAdr );
/*?*/ 				ScBaseCell* pCell = GetCell( aAdr );
/*?*/ 				if (HasCellValueData(pCell))
/*?*/ 				{
/*?*/ 					fVal = GetCellValue( aAdr, pCell );
/*?*/ 					fSum += fVal;
/*?*/ 					fSumSqr += fVal*fVal;
/*?*/ 					rValCount++;
/*?*/ 				}
/*?*/ 				else if ( bTextAsZero && HasCellStringData( pCell ) )
/*?*/ 					rValCount++;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case svDoubleRef :
/*N*/ 			{
/*N*/ 				USHORT nErr = 0;
/*N*/ 				PopDoubleRef( aRange );
/*N*/ 				ScValueIterator aValIter( pDok, aRange, glSubTotal, bTextAsZero );
/*N*/ 				if (aValIter.GetFirst(fVal, nErr))
/*N*/ 				{
/*N*/ 					do
/*N*/ 					{
/*N*/ 						fSum += fVal;
/*N*/ 						fSumSqr += fVal*fVal;
/*N*/ 						rValCount++;
/*N*/ 					}
/*N*/ 					while ((nErr == 0) && aValIter.GetNext(fVal, nErr));
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case svMatrix :
/*N*/ 			{
/*?*/ 				ScMatrix* pMat = PopMatrix();
/*?*/ 				if (pMat)
/*?*/ 				{
/*?*/ 					USHORT nC, nR;
/*?*/ 					pMat->GetDimensions(nC, nR);
/*?*/ 					for (USHORT i = 0; i < nC; i++)
/*?*/ 					{
/*?*/ 						for (USHORT j = 0; j < nR; j++)
/*?*/ 						{
/*?*/ 							if (!pMat->IsString(i,j))
/*?*/ 							{
/*?*/ 								fVal= pMat->GetDouble(i,j);
/*?*/ 								fSum += fVal;
/*?*/ 								fSumSqr += fVal * fVal;
/*?*/ 								rValCount++;
/*?*/ 							}
/*?*/ 							else if ( bTextAsZero )
/*?*/ 								rValCount++;
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case svString :
/*N*/ 			{
/*?*/ 				if ( bTextAsZero )
/*?*/ 					rValCount++;
/*?*/ 				else
/*?*/ 				{
/*?*/ 					Pop();
/*?*/ 					SetError(errIllegalParameter);
/*?*/ 				}
/*N*/ 			}
/*N*/ 			default :
/*?*/ 				Pop();
/*?*/ 				SetError(errIllegalParameter);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	rVal = ::rtl::math::approxSub( fSumSqr, fSum*fSum/rValCount );
/*N*/ }


void ScInterpreter::ScVar( BOOL bTextAsZero )
{
    double nVal;
    double nValCount;
    GetStVarParams( nVal, nValCount, bTextAsZero );
    PushDouble(nVal / (nValCount - 1.0));
}


void ScInterpreter::ScVarP( BOOL bTextAsZero )
{
    double nVal;
    double nValCount;
    GetStVarParams( nVal, nValCount, bTextAsZero );
    PushDouble(nVal / nValCount);
}


void ScInterpreter::ScStDev( BOOL bTextAsZero )
{
    double nVal;
    double nValCount;
    GetStVarParams( nVal, nValCount, bTextAsZero );
    PushDouble(sqrt(nVal / (nValCount - 1.0)));
}


/*N*/ void ScInterpreter::ScStDevP( BOOL bTextAsZero )
/*N*/ {
/*N*/ 	double nVal;
/*N*/ 	double nValCount;
/*N*/ 	GetStVarParams( nVal, nValCount, bTextAsZero );
/*N*/ 	PushDouble(sqrt(nVal / nValCount));
/*N*/ }


void ScInterpreter::ScColumns()
{
    BYTE nParamCount = GetByte();
    ULONG nVal = 0;
    USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
    for (USHORT i = 1; i <= nParamCount; i++)
    {
        switch ( GetStackType() )
        {
            case svSingleRef:
                PopError();
                nVal++;
                break;
            case svDoubleRef:
                PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                nVal += (nTab2 - nTab1 + 1) * (nCol2 - nCol1 + 1);
                break;
            case svMatrix:
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    nVal += nC;
                }
            }
            break;
            default:
                PopError();
                SetError(errIllegalParameter);
        }
    }
    PushDouble((double)nVal);
}


void ScInterpreter::ScRows()
{
    BYTE nParamCount = GetByte();
    ULONG nVal = 0;
    USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
    for (USHORT i = 1; i <= nParamCount; i++)
    {
        switch ( GetStackType() )
        {
            case svSingleRef:
                PopError();
                nVal++;
                break;
            case svDoubleRef:
                PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                nVal += (nTab2 - nTab1 + 1) * (nRow2 - nRow1 + 1);
                break;
            case svMatrix:
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    nVal += nR;
                }
            }
            break;
            default:
                PopError();
                SetError(errIllegalParameter);
        }
    }
    PushDouble((double)nVal);
}

void ScInterpreter::ScTables()
{
    BYTE nParamCount = GetByte();
    ULONG nVal;
    if ( nParamCount == 0 )
        nVal = pDok->GetTableCount();
    else
    {
        nVal = 0;
        USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
        for (USHORT i = 1; i <= nParamCount; i++)
        {
            switch ( GetStackType() )
            {
                case svSingleRef:
                    PopError();
                    nVal++;
                break;
                case svDoubleRef:
                    PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                    nVal += (nTab2 - nTab1 + 1);
                break;
                case svMatrix:
                    PopError();
                    nVal++;
                break;
                default:
                    PopError();
                    SetError( errIllegalParameter );
            }
        }
    }
    PushDouble( (double) nVal );
}


void ScInterpreter::ScColumn()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 0, 1 ) )
    {
        double nVal;
        if (nParamCount == 0)
            nVal = aPos.Col() + 1;
        else
        {
            switch ( GetStackType() )
            {
                case svSingleRef :
                {
                    USHORT nCol1, nRow1, nTab1;
                    PopSingleRef( nCol1, nRow1, nTab1 );
                    nVal = (double) (nCol1 + 1);
                }
                break;
                case svDoubleRef :
                {
                    USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
                    PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                    if (nCol2 > nCol1)
                    {
                        USHORT nMatInd;
                        ScMatrix* pResMat = GetNewMat(nCol2-nCol1+1, 1, nMatInd);
                        if (pResMat)
                        {
                            for (USHORT i = nCol1; i <= nCol2; i++)
                                pResMat->PutDouble((double)(i+1), i-nCol1, 0);
                            PushMatrix(pResMat);
                            nRetMat = nMatInd;
                            return;
                        }
                        else
                        {
                            SetError( errIllegalParameter );
                            nVal = 0.0;
                        }
                    }
                    else
                        nVal = (double) (nCol1 + 1);
                }
                break;
                default:
                    SetError( errIllegalParameter );
                    nVal = 0.0;
            }
        }
        PushDouble( nVal );
    }
}


void ScInterpreter::ScRow()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 0, 1 ) )
    {
        double nVal;
        if (nParamCount == 0)
            nVal = aPos.Row() + 1;
        else
        {
            switch ( GetStackType() )
            {
                case svSingleRef :
                {
                    USHORT nCol1, nRow1, nTab1;
                    PopSingleRef( nCol1, nRow1, nTab1 );
                    nVal = (double) (nRow1 + 1);
                }
                break;
                case svDoubleRef :
                {
                    USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
                    PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                    if (nRow2 > nRow1)
                    {
                        USHORT nMatInd;
                        ScMatrix* pResMat = GetNewMat(1, nRow2-nRow1+1, nMatInd);
                        if (pResMat)
                        {
                            for (USHORT i = nRow1; i <= nRow2; i++)
                                pResMat->PutDouble((double)(i+1), 0, i-nRow1);
                            PushMatrix(pResMat);
                            nRetMat = nMatInd;
                            return;
                        }
                        else
                        {
                            SetError( errIllegalParameter );
                            nVal = 0.0;
                        }
                    }
                    else
                        nVal = (double) (nRow1 + 1);
                }
                break;
                default:
                    SetError( errIllegalParameter );
                    nVal = 0.0;
            }
        }
        PushDouble( nVal );
    }
}

void ScInterpreter::ScTable()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 0, 1 ) )
    {
        USHORT nVal = 0;
        if ( nParamCount == 0 )
            nVal = aPos.Tab() + 1;
        else
        {
            switch ( GetStackType() )
            {
                case svString :
                {
                    String aStr( PopString() );
                    if ( pDok->GetTable( aStr, nVal ) )
                        ++nVal;
                    else
                        SetError( errIllegalArgument );
                }
                break;
                case svSingleRef :
                {
                    USHORT nCol1, nRow1, nTab1;
                    PopSingleRef( nCol1, nRow1, nTab1 );
                    nVal = nTab1 + 1;
                }
                break;
                case svDoubleRef :
                {
                    USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
                    PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                    nVal = nTab1 + 1;
                }
                break;
                default:
                    SetError( errIllegalParameter );
            }
            if ( nGlobalError )
                nVal = 0;
        }
        PushDouble( (double) nVal );
    }
}


/*N*/  void ScInterpreter::ScMatch()
/*N*/  {
/*N*/  	BYTE nParamCount = GetByte();
/*N*/  	if ( MustHaveParamCount( nParamCount, 2, 3 ) )
/*N*/  	{
/*N*/  		double fTyp;
/*N*/  		if (nParamCount == 3)
/*N*/  			fTyp = GetDouble();
/*N*/  		else
/*N*/  			fTyp = 1.0;
/*N*/  		USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*N*/  		if (GetStackType() == svDoubleRef)
/*N*/  		{
/*N*/  			PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
/*N*/  			if (nTab1 != nTab2 || (nCol1 != nCol2 && nRow1 != nRow2))
/*N*/  			{
/*N*/  				SetIllegalParameter();
/*N*/  				return;
/*N*/  			}
/*N*/  		}
/*N*/  		else
/*N*/  		{
/*N*/  			SetIllegalParameter();
/*N*/  			return;
/*N*/  		}
/*N*/  		if (nGlobalError == 0)
/*N*/  		{
/*N*/  			double fVal;
/*N*/  			String sStr;
/*N*/  			ScQueryParam rParam;
/*N*/  			rParam.nCol1       = nCol1;
/*N*/  			rParam.nRow1       = nRow1;
/*N*/  			rParam.nCol2       = nCol2;
/*N*/  			rParam.bHasHeader  = FALSE;
/*N*/  			rParam.bInplace    = TRUE;
/*N*/  			rParam.bCaseSens   = FALSE;
/*N*/              rParam.bRegExp     = pDok->GetDocOptions().IsFormulaRegexEnabled();
/*N*/  			rParam.bDuplicate  = FALSE;
/*N*/  
/*N*/  			ScQueryEntry& rEntry = rParam.GetEntry(0);
/*N*/  			rEntry.bDoQuery = TRUE;
/*N*/  			if (fTyp < 0.0)
/*N*/  				rEntry.eOp = SC_GREATER_EQUAL;
/*N*/  			else if (fTyp > 0.0)
/*N*/  				rEntry.eOp = SC_LESS_EQUAL;
/*N*/  			switch ( GetStackType() )
/*N*/  			{
/*N*/  				case svDouble:
/*N*/  				{
/*N*/  					fVal = GetDouble();
/*N*/  					rEntry.bQueryByString = FALSE;
/*N*/  					rEntry.nVal = fVal;
/*N*/  				}
/*N*/  				break;
/*N*/  				case svString:
/*N*/  				{
/*N*/  					sStr = GetString();
/*N*/  					rEntry.bQueryByString = TRUE;
/*N*/  					*rEntry.pStr = sStr;
/*N*/  				}
/*N*/  				break;
/*N*/  				case svDoubleRef :
/*N*/  				case svSingleRef :
/*N*/  				{
/*N*/  					ScAddress aAdr;
/*N*/  					if ( !PopDoubleRefOrSingleRef( aAdr ) )
/*N*/  					{
/*N*/  						PushInt(0);
/*N*/  						return ;
/*N*/  					}
/*N*/  					ScBaseCell* pCell = GetCell( aAdr );
/*N*/  					if (HasCellValueData(pCell))
/*N*/  					{
/*N*/  						fVal = GetCellValue( aAdr, pCell );
/*N*/  						rEntry.bQueryByString = FALSE;
/*N*/  						rEntry.nVal = fVal;
/*N*/  					}
/*N*/  					else
/*N*/  					{
/*N*/  						GetCellString(sStr, pCell);
/*N*/  						rEntry.bQueryByString = TRUE;
/*N*/  						*rEntry.pStr = sStr;
/*N*/  					}
/*N*/  				}
/*N*/  				break;
/*N*/  				default:
/*N*/  				{
/*N*/  					SetIllegalParameter();
/*N*/  					return;
/*N*/  				}
/*N*/  			}
/*N*/  			if ( rEntry.bQueryByString )
/*N*/                  rParam.bRegExp = MayBeRegExp( *rEntry.pStr, pDok );
/*N*/  			USHORT nDelta, nR, nC;
/*N*/              if (nCol1 == nCol2)
/*N*/              {                                           // search row in column
/*N*/  				rParam.nRow2 = nRow2;
/*N*/  				rEntry.nField = nCol1;
/*N*/  				ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
/*N*/                  if (fTyp == 0.0)
/*N*/                  {                                       // EQUAL
/*N*/  					if (aCellIter.GetFirst())
/*N*/  						nR = aCellIter.GetRow();
/*N*/  					else
/*N*/  					{
/*N*/  						SetNV();
/*N*/  						return;
/*N*/  					}
/*N*/  				}
/*N*/                  else
/*N*/                  {                                       // <= or >=
/*N*/                      aCellIter.SetStopOnMismatch( TRUE );
/*N*/                      if ( aCellIter.GetFirst() )
/*N*/                      {
/*N*/                          do
/*N*/                          {
/*N*/                              nR = aCellIter.GetRow();
/*N*/                          } while ( aCellIter.GetNext() );
/*N*/                      }
/*N*/                      else
/*N*/                      {
/*N*/                          SetNV();
/*N*/                          return;
/*N*/                      }
/*N*/                  }
/*N*/  				nDelta = nR - nRow1;
/*N*/  			}
/*N*/              else
/*N*/              {                                           // search column in row
/*N*/  				rParam.nRow2 = nRow1;
/*N*/  				rEntry.nField = nCol1;
/*N*/  				ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
/*N*/                  // Advance Entry.nField in Iterator if column changed
/*N*/  				aCellIter.SetAdvanceQueryParamEntryField( TRUE );
/*N*/                  if (fTyp == 0.0)
/*N*/                  {                                       // EQUAL
/*N*/  					if ( aCellIter.GetFirst() )
/*N*/  						nC = aCellIter.GetCol();
/*N*/  					else
/*N*/                      {
/*N*/  						SetNV();
/*N*/  						return;
/*N*/  					}
/*N*/  				}
/*N*/                  else
/*N*/                  {                                       // <= or >=
/*N*/                      aCellIter.SetStopOnMismatch( TRUE );
/*N*/                      if ( aCellIter.GetFirst() )
/*N*/                      {
/*N*/                          do
/*N*/                          {
/*N*/                              nC = aCellIter.GetCol();
/*N*/                          } while ( aCellIter.GetNext() );
/*N*/                      }
/*N*/                      else
/*N*/                      {
/*N*/                          SetNV();
/*N*/                          return;
/*N*/                      }
/*N*/                  }
/*N*/  				nDelta = nC - nCol1;
/*N*/  			}
/*N*/  			PushDouble((double) (nDelta + 1));
/*N*/  		}
/*N*/  		else
/*N*/  			SetIllegalParameter();
/*N*/  	}
/*N*/  }


void ScInterpreter::ScCountEmptyCells()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        long nMaxCount = 0, nCount = 0;
        CellType eCellType;
        switch (GetStackType())
        {
            case svSingleRef :
            {
                nMaxCount = 1;
                ScAddress aAdr;
                PopSingleRef( aAdr );
                eCellType = GetCellType( GetCell( aAdr ) );
                if (eCellType != CELLTYPE_NONE && eCellType != CELLTYPE_NOTE)
                    nCount = 1;
            }
            break;
            case svDoubleRef :
            {
                USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
                PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                nMaxCount = (nRow2 - nRow1 + 1) * (nCol2 - nCol1 + 1)
                                                * (nTab2 - nTab1 + 1);
                ScBaseCell* pCell;
                ScCellIterator aDocIter(pDok, nCol1, nRow1, nTab1,
                                              nCol2, nRow2, nTab2, glSubTotal);
                if (pCell = aDocIter.GetFirst())
                {
                    do
                    {
                        if ((eCellType = pCell->GetCellType()) != CELLTYPE_NONE
                                && eCellType != CELLTYPE_NOTE)
                            nCount++;
                    } while ( pCell = aDocIter.GetNext() );
                }
            }
            break;
            default : SetError(errIllegalParameter); break;
        }
        PushDouble(nMaxCount - nCount);
    }
}


 void ScInterpreter::ScCountIf()
 {
     if ( MustHaveParamCount( GetByte(), 2 ) )
     {
         String rString;
         double fVal;
         BOOL bIsString = TRUE;
         switch ( GetStackType() )
         {
             case svDoubleRef :
             case svSingleRef :
             {
                 ScAddress aAdr;
                 if ( !PopDoubleRefOrSingleRef( aAdr ) )
                 {
                     PushInt(0);
                     return ;
                 }
                 ScBaseCell* pCell = GetCell( aAdr );
                 switch ( GetCellType( pCell ) )
                 {
                     case CELLTYPE_VALUE :
                         fVal = GetCellValue( aAdr, pCell );
                         bIsString = FALSE;
                         break;
                     case CELLTYPE_FORMULA :
                         if( ((ScFormulaCell*)pCell)->IsValue() )
                         {
                             fVal = GetCellValue( aAdr, pCell );
                             bIsString = FALSE;
                         }
                         else
                             GetCellString(rString, pCell);
                         break;
                     case CELLTYPE_STRING :
                     case CELLTYPE_EDIT :
                         GetCellString(rString, pCell);
                         break;
                     default:
                         fVal = 0.0;
                         bIsString = FALSE;
                 }
             }
             break;
             case svString:
                 rString = GetString();
             break;
             default:
             {
                 fVal = GetDouble();
                 bIsString = FALSE;
             }
         }
         USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
         switch ( GetStackType() )
         {
             case svDoubleRef :
                 PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
             break;
             case svSingleRef :
                 PopSingleRef( nCol1, nRow1, nTab1 );
                 nCol2 = nCol1;
                 nRow2 = nRow1;
                 nTab2 = nTab1;
             break;
             default:
                 SetIllegalParameter();
                 return ;
         }
         if ( nTab1 != nTab2 )
         {
             SetIllegalParameter();
             return;
         }
         if (nCol1 > nCol2)
         {
             SetIllegalParameter();
             return;
         }
         if (nGlobalError == 0)
         {
             ScQueryParam rParam;
             rParam.nRow1       = nRow1;
             rParam.nRow2       = nRow2;
             rParam.bHasHeader  = FALSE;
             rParam.bInplace    = TRUE;
             rParam.bCaseSens   = FALSE;
             rParam.bRegExp     = pDok->GetDocOptions().IsFormulaRegexEnabled();
             rParam.bDuplicate  = FALSE;
 
             ScQueryEntry& rEntry = rParam.GetEntry(0);
             rEntry.bDoQuery = TRUE;
             if (!bIsString)
             {
                 rEntry.bQueryByString = FALSE;
                 rEntry.nVal = fVal;
                 rEntry.eOp = SC_EQUAL;
             }
             else if( rString.Len() )
             {
                 rParam.FillInExcelSyntax(rString,(USHORT) 0);
                 sal_uInt32 nIndex = 0;
                 rEntry.bQueryByString =
                     !(pFormatter->IsNumberFormat(
                         *rEntry.pStr, nIndex, rEntry.nVal));
                 if ( rEntry.bQueryByString )
                     rParam.bRegExp = MayBeRegExp( *rEntry.pStr, pDok );
             }
             else
             {
                 PushInt( 0 );
                 return;
             }
             double fSum = 0.0;
             rParam.nCol1  = nCol1;
             rParam.nCol2  = nCol2;
             rEntry.nField = nCol1;
             ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
             // Entry.nField im Iterator bei Spaltenwechsel weiterschalten
             aCellIter.SetAdvanceQueryParamEntryField( TRUE );
             if ( aCellIter.GetFirst() )
             {
                 do
                 {
                     fSum++;
                 } while ( aCellIter.GetNext() );
             }
             PushDouble(fSum);
         }
         else
             SetIllegalParameter();
     }
 }


 void ScInterpreter::ScSumIf()
 {
     BYTE nParamCount = GetByte();
     if ( MustHaveParamCount( nParamCount, 2, 3 ) )
     {
         USHORT nCol3, nRow3, nTab3, nCol4, nRow4, nTab4;
         if (nParamCount == 3)
         {
             switch ( GetStackType() )
             {
                 case svDoubleRef :
                     PopDoubleRef( nCol3, nRow3, nTab3, nCol4, nRow4, nTab4 );
                 break;
                 case svSingleRef :
                     PopSingleRef( nCol3, nRow3, nTab3 );
                     nCol4 = nCol3;
                     nRow4 = nRow3;
                     nTab4 = nTab3;
                 break;
                 default:
                     SetIllegalParameter();
                     return ;
             }
             if ( nTab3 != nTab4 )
             {
                 SetIllegalParameter();
                 return;
             }
         }
         String rString;
         double fVal;
         BOOL bIsString = TRUE;
         switch ( GetStackType() )
         {
             case svDoubleRef :
             case svSingleRef :
             {
                 ScAddress aAdr;
                 if ( !PopDoubleRefOrSingleRef( aAdr ) )
                 {
                     PushInt(0);
                     return ;
                 }
                 ScBaseCell* pCell = GetCell( aAdr );
                 switch ( GetCellType( pCell ) )
                 {
                     case CELLTYPE_VALUE :
                         fVal = GetCellValue( aAdr, pCell );
                         bIsString = FALSE;
                         break;
                     case CELLTYPE_FORMULA :
                         if( ((ScFormulaCell*)pCell)->IsValue() )
                         {
                             fVal = GetCellValue( aAdr, pCell );
                             bIsString = FALSE;
                         }
                         else
                             GetCellString(rString, pCell);
                         break;
                     case CELLTYPE_STRING :
                     case CELLTYPE_EDIT :
                         GetCellString(rString, pCell);
                         break;
                     default:
                         fVal = 0.0;
                         bIsString = FALSE;
                 }
             }
             break;
             case svString:
                 rString = GetString();
             break;
             default:
             {
                 fVal = GetDouble();
                 bIsString = FALSE;
             }
         }
         USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
         switch ( GetStackType() )
         {
             case svDoubleRef :
                 PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
             break;
             case svSingleRef :
                 PopSingleRef( nCol1, nRow1, nTab1 );
                 nCol2 = nCol1;
                 nRow2 = nRow1;
                 nTab2 = nTab1;
             break;
             default:
                 SetIllegalParameter();
                 return ;
         }
         if ( nTab1 != nTab2 )
         {
             SetIllegalParameter();
             return;
         }
         if (nParamCount != 3)
         {
             nCol3 = nCol1;
             nRow3 = nRow1;
             nTab3 = nTab1;
             nCol4 = nCol2;
             nRow4 = nRow2;
             nTab4 = nTab2;
         }
         else if (nCol4 - nCol3 != nCol2 - nCol1 ||
                  nRow4 - nRow3 != nRow2 - nRow1 || nCol1 > nCol2)
         {
             SetIllegalParameter();
             return;
         }
         if (nGlobalError == 0)
         {
             ScQueryParam rParam;
             rParam.nRow1       = nRow1;
             rParam.nRow2       = nRow2;
             rParam.bHasHeader  = FALSE;
             rParam.bInplace    = TRUE;
             rParam.bCaseSens   = FALSE;
             rParam.bRegExp     = pDok->GetDocOptions().IsFormulaRegexEnabled();
             rParam.bDuplicate  = FALSE;
 
             ScQueryEntry& rEntry = rParam.GetEntry(0);
             rEntry.bDoQuery = TRUE;
             if (!bIsString)
             {
                 rEntry.bQueryByString = FALSE;
                 rEntry.nVal = fVal;
                 rEntry.eOp = SC_EQUAL;
             }
             else
             {
                 rParam.FillInExcelSyntax(rString,(USHORT) 0);
                 sal_uInt32 nIndex = 0;
                 rEntry.bQueryByString =
                     !(pFormatter->IsNumberFormat(
                         *rEntry.pStr, nIndex, rEntry.nVal));
                 if ( rEntry.bQueryByString )
                     rParam.bRegExp = MayBeRegExp( *rEntry.pStr, pDok );
             }
             double fSum = 0.0;
             double fMem = 0.0;
             BOOL bNull = TRUE;
             ScAddress aAdr;
             aAdr.SetTab( nTab3 );
             rParam.nCol1  = nCol1;
             rParam.nCol2  = nCol2;
             rEntry.nField = nCol1;
             short nColDiff = nCol3 - nCol1;
             short nRowDiff = nRow3 - nRow1;
             ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
             // Entry.nField im Iterator bei Spaltenwechsel weiterschalten
             aCellIter.SetAdvanceQueryParamEntryField( TRUE );
             if ( aCellIter.GetFirst() )
             {
                 do
                 {
                     aAdr.SetCol( aCellIter.GetCol() + nColDiff );
                     aAdr.SetRow( aCellIter.GetRow() + nRowDiff );
                     ScBaseCell* pCell = GetCell( aAdr );
                     if ( HasCellValueData(pCell) )
                     {
                         fVal = GetCellValue( aAdr, pCell );
                         if ( bNull && fVal != 0.0 )
                         {
                             bNull = FALSE;
                             fMem = fVal;
                         }
                         else
                             fSum += fVal;
                     }
                 } while ( aCellIter.GetNext() );
             }
             PushDouble( ::rtl::math::approxAdd( fSum, fMem ) );
         }
         else
             SetIllegalParameter();
     }
 }


 void ScInterpreter::ScLookup()
 {
     BYTE nParamCount = GetByte();
     if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
         return ;
     USHORT nC3 = 0, nC1 = 0;
    USHORT nR3 = 0, nR1 = 0;
     ScMatrix* pMat3 = NULL;
     ScMatrix* pMat1 = NULL;
     USHORT nCol1 = 0; 
    USHORT nRow1 = 0; 
    USHORT nTab1 = 0; 
        USHORT nCol2 = 0; 
    USHORT nRow2 = 0; 
    USHORT nTab2 = 0;
     USHORT nCol3 = 0; 
    USHORT nRow3 = 0; 
    USHORT nTab3 = 0; 
    USHORT nCol4 = 0; 
    USHORT nRow4 = 0; 
    USHORT nTab4 = 0;
     USHORT nDelta;
 
     // param 3: data range
     if ( nParamCount == 3 )
     {
         if (GetStackType() == svDoubleRef)
         {
             PopDoubleRef(nCol3, nRow3, nTab3, nCol4, nRow4, nTab4);
             if (nTab3 != nTab4 || (nCol3 != nCol4 && nRow3 != nRow4))
             {
                 SetIllegalParameter();
                 return;
             }
         }
         else if (GetStackType() == svMatrix)
         {
             pMat3 = PopMatrix();
             if (pMat3)
             {
                 pMat3->GetDimensions(nC3, nR3);
                 if (nC3 != 1 && nR3 != 1)
                 {
                     SetIllegalParameter();
                     return;
                 }
             }
             else
             {
                 SetIllegalParameter();
                 return;
             }
         }
         else
         {
             SetIllegalParameter();
             return;
         }
     }
 
     // param 2: key range, or key range and data range
     if (GetStackType() == svDoubleRef)
     {
         PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
         if ( nTab1 != nTab2 || (nParamCount == 3 && nCol1 != nCol2 && nRow1 != nRow2) )
         {
             SetIllegalParameter();
             return;
         }
     }
     else if (GetStackType() == svMatrix)
     {
         pMat1 = PopMatrix();
         if (pMat1)
         {
             pMat1->GetDimensions(nC1, nR1);
             if (nC1 != 1 && nR1 != 1)
             {
                 SetIllegalParameter();
                 return;
             }
         }
         else
         {
             SetIllegalParameter();
             return;
         }
     }
     else
     {
         SetIllegalParameter();
         return;
     }
     BOOL bSpMatrix, bSpVector;
     USHORT nMatCount, nVecCount;
     if (pMat1 == NULL)
     {
         if (nRow1 == nRow2)
         {
             nMatCount = nCol2 - nCol1 + 1;
             bSpMatrix = FALSE;
         }
         else
         {
             nMatCount = nRow2 - nRow1 + 1;
             bSpMatrix = TRUE;
         }
     }
     else
     {
         if (nR1 == 1)
         {
             nMatCount = nC1;
             bSpMatrix = FALSE;
         }
         else
         {
             nMatCount = nR1;
             bSpMatrix = TRUE;
         }
     }
     if ( nParamCount < 3 )
     {
         nVecCount = nMatCount;
         bSpVector = bSpMatrix;
     }
     else if (pMat3 == NULL)
     {
         if (nRow3 == nRow4)
         {
             nVecCount = nCol4 - nCol3 + 1;
             bSpVector = FALSE;
         }
         else
         {
             nVecCount = nRow4 - nRow3 + 1;
             bSpVector = TRUE;
         }
     }
     else
     {
         if (nR3 == 1)
         {
             nVecCount = nC3;
             bSpVector = FALSE;
         }
         else
         {
             nVecCount = nR3;
             bSpVector = TRUE;
         }
     }
     if (nGlobalError == 0 && nVecCount == nMatCount)
     {
         String sStr;
         ScQueryParam rParam;
         rParam.nCol1       = nCol1;
         rParam.nRow1       = nRow1;
         rParam.nCol2       = (bSpMatrix ? nCol1 : nCol2);
         rParam.nRow2       = (bSpMatrix ? nRow2 : nRow1);
         rParam.bHasHeader  = FALSE;
         rParam.bInplace    = TRUE;
         rParam.bCaseSens   = FALSE;
         rParam.bRegExp     = pDok->GetDocOptions().IsFormulaRegexEnabled();
         rParam.bDuplicate  = FALSE;
 
         ScQueryEntry& rEntry = rParam.GetEntry(0);
         rEntry.bDoQuery = TRUE;
         rEntry.eOp = SC_LESS_EQUAL;
         rEntry.nField = nCol1;
         switch ( GetStackType() )
         {
             case svDouble:
             {
                 rEntry.bQueryByString = FALSE;
                 rEntry.nVal = GetDouble();
             }
             break;
             case svString:
             {
                 sStr = GetString();
                 rEntry.bQueryByString = TRUE;
                 *rEntry.pStr = sStr;
             }
             break;
             case svDoubleRef :
             case svSingleRef :
             {
                 ScAddress aAdr;
                 if ( !PopDoubleRefOrSingleRef( aAdr ) )
                 {
                     PushInt(0);
                     return ;
                 }
                 ScBaseCell* pCell = GetCell( aAdr );
                 if (HasCellValueData(pCell))
                 {
                     rEntry.bQueryByString = FALSE;
                     rEntry.nVal = GetCellValue( aAdr, pCell );
                 }
                 else
                 {
                     if ( GetCellType( pCell ) == CELLTYPE_NOTE )
                     {
                         rEntry.bQueryByString = FALSE;
                         rEntry.nVal = 0.0;
                     }
                     else
                     {
                         GetCellString(sStr, pCell);
                         rEntry.bQueryByString = TRUE;
                         *rEntry.pStr = sStr;
                     }
                 }
             }
             break;
             default:
             {
                 SetIllegalParameter();
                 return;
             }
         }
         if ( rEntry.bQueryByString )
             rParam.bRegExp = MayBeRegExp( *rEntry.pStr, pDok );
         if (pMat1)
         {
             if (rEntry.bQueryByString)
             {
 //!!!!!!!
 //! TODO: enable regex on matrix strings
 //!!!!!!!
                 BOOL bFound = FALSE;
                 sal_Int32 nRes;
                 String aParamStr = *rEntry.pStr;
                 USHORT i = 0;
                 for ( i = 0; i < nMatCount; i++)
                 {
                     if (!pMat1->IsValue(i))
                     {
                         nRes = ScGlobal::pCollator->compareString(
                             pMat1->GetString(i), aParamStr );
                         if (nRes == COMPARE_EQUAL)
                         {
                             bFound = TRUE;
                             nDelta = i;
                         }
                         else if (nRes == COMPARE_LESS)
                             i = nMatCount+1;
                     }
                 }
                 if (i == nMatCount+2 && !bFound)
                 {
                     SetNV();
                     return;
                 }
                 else if (!bFound)
                     nDelta = i-1;
             }
             else
             {
                 BOOL bFound = FALSE;
                 double fVal1;
                 USHORT i = 0;
                 for ( i = 0; i < nMatCount; i++)
                 {
                     if (pMat1->IsValue(i))
                         fVal1 = pMat1->GetDouble(i);
                     else
                         fVal1 = MAXDOUBLE;
                     if (fVal1 == rEntry.nVal)
                     {
                         bFound = TRUE;
                         nDelta = i;
                     }
                     else if (fVal1 > rEntry.nVal)
                         i = nMatCount+1;
                 }
                 if (i == nMatCount+2 && !bFound)
                 {
                     SetNV();
                     return;
                 }
                 else if (!bFound)
                     nDelta = i-1;
             }
         }
         else if (bSpMatrix)             // lookup in column
         {
             rEntry.eOp = SC_LESS_EQUAL;
             ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
             USHORT nC, nR;
             if ( aCellIter.FindEqualOrSortedLastInRange( nC, nR ) )
                 nDelta = nR - nRow1;
             else
             {
                 SetNV();
                 return;
             }
         }
         else                            // lookup in row
         {
             rEntry.eOp = SC_LESS_EQUAL;
             ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
             // advance Entry.nField in Iterator upon switching columns
             aCellIter.SetAdvanceQueryParamEntryField( TRUE );
             USHORT nC, nR;
             if ( aCellIter.FindEqualOrSortedLastInRange( nC, nR ) )
                 nDelta = nC - nCol1;
             else
             {
                 SetNV();
                 return;
             }
         }
     }
     else
     {
         SetIllegalParameter();
         return;
     }
     if (pMat3)
     {
         if (pMat3->IsValue(nDelta))
             PushDouble(pMat3->GetDouble(nDelta));
         else
             PushString(pMat3->GetString(nDelta));
     }
     else
     {
         ScAddress aAdr;
         if ( nParamCount < 3 )
         {
             if (bSpVector)
             {
                 aAdr.SetCol( nCol2 );   // data in right col of key/data range
                 aAdr.SetRow( nRow1 + nDelta );
             }
             else
             {
                 aAdr.SetCol( nCol1 + nDelta );
                 aAdr.SetRow( nRow2 );   // data in lower row of key/data range
             }
             aAdr.SetTab( nTab1 );
         }
         else
         {
             if (bSpVector)
             {
                 aAdr.SetCol( nCol3 );
                 aAdr.SetRow( nRow3 + nDelta );
             }
             else
             {
                 aAdr.SetCol( nCol3 + nDelta );
                 aAdr.SetRow( nRow3 );
             }
             aAdr.SetTab( nTab3 );
         }
         ScBaseCell* pCell = GetCell( aAdr );
         if (HasCellValueData(pCell))
             PushDouble(GetCellValue( aAdr, pCell ));
         else
         {
             String aStr;
             GetCellString(aStr, pCell);
             PushString(aStr);
         }
     }
 }


 void ScInterpreter::ScHLookup()
 {
     BYTE nParamCount = GetByte();
     if ( MustHaveParamCount( nParamCount, 3, 4 ) )
     {
         BOOL bSorted;
         if (nParamCount == 4)
             bSorted = GetBool();
         else
             bSorted = TRUE;
         double fIndex = ::rtl::math::approxFloor( GetDouble() ) - 1.0;
         ScMatrix* pMat = NULL;
         USHORT nC = 0, nR = 0;
         USHORT nCol1 = 0, nRow1 = 0, nTab1 = 0, nCol2 = 0, nRow2 = 0, nTab2 = 0;
         if (GetStackType() == svDoubleRef)
         {
             PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
             if (nTab1 != nTab2)
             {
                 SetIllegalParameter();
                 return;
             }
         }
         else if (GetStackType() == svMatrix)
         {
             pMat = PopMatrix();
             if (pMat)
                 pMat->GetDimensions(nC, nR);
             else
             {
                 SetIllegalParameter();
                 return;
             }
         }
         else
         {
             SetIllegalParameter();
             return;
         }
         if ( fIndex < 0.0 || (pMat ? (fIndex >= nR) : (fIndex+nRow1 > nRow2)) )
         {
             SetIllegalArgument();
             return;
         }
         USHORT nZIndex = (USHORT) fIndex;
         if (!pMat)
             nZIndex += nRow1;						// Wertzeile
         if (nGlobalError == 0)
         {
             String sStr;
             ScQueryParam rParam;
             rParam.nCol1       = nCol1;
             rParam.nRow1       = nRow1;
             rParam.nCol2       = nCol2;
             rParam.nRow2       = nRow1;		// nur in der ersten Zeile suchen
             rParam.bHasHeader  = FALSE;
             rParam.bInplace    = TRUE;
             rParam.bCaseSens   = FALSE;
             rParam.bRegExp     = pDok->GetDocOptions().IsFormulaRegexEnabled();
             rParam.bDuplicate  = FALSE;
 
             ScQueryEntry& rEntry = rParam.GetEntry(0);
             rEntry.bDoQuery = TRUE;
             if ( bSorted )
                 rEntry.eOp = SC_LESS_EQUAL;
             switch ( GetStackType() )
             {
                 case svDouble:
                 {
                     rEntry.bQueryByString = FALSE;
                     rEntry.nVal = GetDouble();
                 }
                 break;
                 case svString:
                 {
                     sStr = GetString();
                     rEntry.bQueryByString = TRUE;
                     *rEntry.pStr = sStr;
                 }
                 break;
                 case svDoubleRef :
                 case svSingleRef :
                 {
                     ScAddress aAdr;
                     if ( !PopDoubleRefOrSingleRef( aAdr ) )
                     {
                         PushInt(0);
                         return ;
                     }
                     ScBaseCell* pCell = GetCell( aAdr );
                     if (HasCellValueData(pCell))
                     {
                         rEntry.bQueryByString = FALSE;
                         rEntry.nVal = GetCellValue( aAdr, pCell );
                     }
                     else
                     {
                         if ( GetCellType( pCell ) == CELLTYPE_NOTE )
                         {
                             rEntry.bQueryByString = FALSE;
                             rEntry.nVal = 0.0;
                         }
                         else
                         {
                             GetCellString(sStr, pCell);
                             rEntry.bQueryByString = TRUE;
                             *rEntry.pStr = sStr;
                         }
                     }
                 }
                 break;
                 default:
                 {
                     SetIllegalParameter();
                     return;
                 }
             }
             if ( rEntry.bQueryByString )
                 rParam.bRegExp = MayBeRegExp( *rEntry.pStr, pDok );
             if (pMat)
             {
                 USHORT nMatCount = nC;
                 short nDelta = -1;
                 if (rEntry.bQueryByString)
                 {
 //!!!!!!!
 //! TODO: enable regex on matrix strings
 //!!!!!!!
                     String aParamStr = *rEntry.pStr;
                     USHORT i;
                     if ( bSorted )
                     {
                         for (i = 0; i < nMatCount; i++)
                         {
                             if (pMat->IsString(i, 0))
                             {
                                 if ( ScGlobal::pTransliteration->isEqual(
                                         pMat->GetString(i,0), aParamStr ) )
                                     i = nMatCount+1;
                                 else
                                     nDelta = i;
                             }
                             else
                                 nDelta = i;
                         }
                     }
                     else
                     {
                         for (i = 0; i < nMatCount; i++)
                         {
                             if (pMat->IsString(i, 0))
                             {
                                 if ( ScGlobal::pTransliteration->isEqual(
                                         pMat->GetString(i,0), aParamStr ) )
                                 {
                                     nDelta = i;
                                     i = nMatCount + 1;
                                 }
                             }
                         }
                     }
                 }
                 else
                 {
                     double fVal1;
                     USHORT i;
                     if ( bSorted )
                     {
                         for (i = 0; i < nMatCount; i++)
                         {
                             if (!pMat->IsString(i, 0))
                                 fVal1 = pMat->GetDouble(i,0);
                             else
                                 fVal1 = MAXDOUBLE;
                             if (fVal1 <= rEntry.nVal)
                                 nDelta = i;
                             else
                                 i = nMatCount+1;
                         }
                     }
                     else
                     {
                         for (i = 0; i < nMatCount; i++)
                         {
                             if (!pMat->IsString(i, 0))
                                 fVal1 = pMat->GetDouble(i,0);
                             else
                                 fVal1 = MAXDOUBLE;
                             if (fVal1 == rEntry.nVal)
                             {
                                 nDelta = i;
                                 i = nMatCount + 1;
                             }
                         }
                     }
                 }
                 if ( nDelta >= 0 )
                 {
                     if (!pMat->IsString(nDelta, nZIndex))
                         PushDouble(pMat->GetDouble(nDelta, nZIndex));
                     else
                         PushString(pMat->GetString(nDelta, nZIndex));
                 }
                 else
                     SetNV();
             }
             else
             {
                 rEntry.nField = nCol1;
                 BOOL bFound = FALSE;
                 USHORT nC;
                 if ( bSorted )
                     rEntry.eOp = SC_LESS_EQUAL;
                 ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
                 // advance Entry.nField in Iterator upon switching columns
                 aCellIter.SetAdvanceQueryParamEntryField( TRUE );
                 if ( bSorted )
                 {
                     USHORT nR;
                     bFound = aCellIter.FindEqualOrSortedLastInRange( nC, nR );
                 }
                 else if ( aCellIter.GetFirst() )
                 {
                     bFound = TRUE;
                     nC = aCellIter.GetCol();
                     if ( bSorted )
                     {
                         while ( aCellIter.GetNext() )
                             nC = aCellIter.GetCol();
                     }
                 }
                 if ( bFound )
                 {
                     ScBaseCell* pCell;
                     ScAddress aAdr( nC, nZIndex, nTab1 );
                     if ( HasCellValueData( pCell = GetCell( aAdr ) ) )
                         PushDouble(GetCellValue( aAdr, pCell ));
                     else
                     {
                         String aStr;
                         GetCellString(aStr, pCell);
                         PushString(aStr);
                     }
                 }
                 else
                     SetNV();
             }
         }
         else
             SetIllegalParameter();
     }
 }


/*N*/ void ScInterpreter::ScVLookup()
/*N*/ {
/*N*/ 	BYTE nParamCount = GetByte();
/*N*/ 	if ( MustHaveParamCount( nParamCount, 3, 4 ) )
/*N*/ 	{
/*N*/ 		BOOL bSorted;
/*N*/ 		if (nParamCount == 4)
/*N*/ 			bSorted = GetBool();
/*N*/ 		else
/*N*/ 			bSorted = TRUE;
/*N*/ 		double fIndex = ::rtl::math::approxFloor( GetDouble() ) - 1.0;
/*N*/ 		ScMatrix* pMat = NULL;
/*N*/ 		USHORT nC = 0, nR = 0;
/*N*/ 		USHORT nCol1 = 0, nRow1 = 0, nTab1 = 0, nCol2 = 0, nRow2 = 0, nTab2 = 0;
/*N*/ 		if (GetStackType() == svDoubleRef)
/*N*/ 		{
/*N*/ 			PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
/*N*/ 			if (nTab1 != nTab2)
/*N*/ 			{
/*?*/ 				SetIllegalParameter();
/*?*/ 				return;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if (GetStackType() == svMatrix)
/*N*/ 		{
/*?*/ 			pMat = PopMatrix();
/*?*/ 			if (pMat)
/*?*/ 				pMat->GetDimensions(nC, nR);
/*?*/ 			else
/*?*/ 			{
/*?*/ 				SetIllegalParameter();
/*?*/ 				return;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			SetIllegalParameter();
/*?*/ 			return;
/*N*/ 		}
/*N*/ 		if ( fIndex < 0.0 || (pMat ? (fIndex >= nC) : (fIndex+nCol1 > nCol2)) )
/*N*/ 		{
/*?*/ 			SetIllegalArgument();
/*?*/ 			return;
/*N*/ 		}
/*N*/ 		USHORT nSpIndex = (USHORT) fIndex;
/*N*/ 		if (!pMat)
/*N*/ 			nSpIndex += nCol1;						// Wertspalte
/*N*/ 		if (nGlobalError == 0)
/*N*/ 		{
/*N*/ 			String sStr;
/*N*/ 			ScQueryParam rParam;
/*N*/ 			rParam.nCol1       = nCol1;
/*N*/ 			rParam.nRow1       = nRow1;
/*N*/ 			rParam.nCol2       = nCol1;		// nur in der ersten Spalte suchen
/*N*/ 			rParam.nRow2       = nRow2;
/*N*/ 			rParam.bHasHeader  = FALSE;
/*N*/ 			rParam.bInplace    = TRUE;
/*N*/ 			rParam.bCaseSens   = FALSE;
/*N*/             rParam.bRegExp     = pDok->GetDocOptions().IsFormulaRegexEnabled();
/*N*/ 			rParam.bDuplicate  = FALSE;
/*N*/ 
/*N*/ 			ScQueryEntry& rEntry = rParam.GetEntry(0);
/*N*/ 			rEntry.bDoQuery = TRUE;
/*N*/ 			if ( bSorted )
/*N*/ 				rEntry.eOp = SC_LESS_EQUAL;
/*N*/ 			switch ( GetStackType() )
/*N*/ 			{
/*N*/ 				case svDouble:
/*N*/ 				{
/*N*/ 					rEntry.bQueryByString = FALSE;
/*N*/ 					rEntry.nVal = GetDouble();
/*N*/ 				}
/*N*/ 				break;
/*N*/ 				case svString:
/*N*/ 				{
/*?*/ 					sStr = GetString();
/*?*/ 					rEntry.bQueryByString = TRUE;
/*?*/ 					*rEntry.pStr = sStr;
/*?*/ 				}
/*?*/ 				break;
/*N*/ 				case svDoubleRef :
/*N*/ 				case svSingleRef :
/*N*/ 				{
/*N*/ 					ScAddress aAdr;
/*N*/ 					if ( !PopDoubleRefOrSingleRef( aAdr ) )
/*N*/ 					{
/*?*/ 						PushInt(0);
/*?*/ 						return ;
/*N*/ 					}
/*N*/ 					ScBaseCell* pCell = GetCell( aAdr );
/*N*/ 					if (HasCellValueData(pCell))
/*N*/ 					{
/*?*/ 						rEntry.bQueryByString = FALSE;
/*?*/ 						rEntry.nVal = GetCellValue( aAdr, pCell );
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						if ( GetCellType( pCell ) == CELLTYPE_NOTE )
/*N*/ 						{
/*?*/ 							rEntry.bQueryByString = FALSE;
/*?*/ 							rEntry.nVal = 0.0;
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							GetCellString(sStr, pCell);
/*N*/ 							rEntry.bQueryByString = TRUE;
/*N*/ 							*rEntry.pStr = sStr;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				break;
/*N*/ 				default:
/*N*/ 				{
/*?*/ 					SetIllegalParameter();
/*?*/ 					return;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if ( rEntry.bQueryByString )
/*N*/                 rParam.bRegExp = MayBeRegExp( *rEntry.pStr, pDok );
/*N*/ 			if (pMat)
/*N*/ 			{
/*?*/ 				USHORT nMatCount = nR;
/*?*/ 				short nDelta = -1;
/*?*/ 				if (rEntry.bQueryByString)
/*?*/ 				{
/*?*/ //!!!!!!!
/*?*/ //! TODO: enable regex on matrix strings
/*?*/ //!!!!!!!
/*?*/ 					String aParamStr = *rEntry.pStr;
/*?*/ 					USHORT i;
/*?*/ 					if ( bSorted )
/*?*/ 					{
/*?*/ 						for (i = 0; i < nMatCount; i++)
/*?*/ 						{
/*?*/ 							if (pMat->IsString(0, i))
/*?*/ 							{
/*?*/                                 if ( ScGlobal::pTransliteration->isEqual(
/*?*/                                         pMat->GetString(0,i), aParamStr ) )
/*?*/ 									i = nMatCount+1;
/*?*/ 								else
/*?*/ 									nDelta = i;
/*?*/ 							}
/*?*/ 							else
/*?*/ 								nDelta = i;
/*?*/ 						}
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						for (i = 0; i < nMatCount; i++)
/*?*/ 						{
/*?*/ 							if (pMat->IsString(0, i))
/*?*/ 							{
/*?*/                                 if ( ScGlobal::pTransliteration->isEqual(
/*?*/                                         pMat->GetString(0,i), aParamStr ) )
/*?*/ 								{
/*?*/ 									nDelta = i;
/*?*/ 									i = nMatCount + 1;
/*?*/ 								}
/*?*/ 							}
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					double fVal1;
/*?*/ 					USHORT i;
/*?*/ 					if ( bSorted )
/*?*/ 					{
/*?*/ 						for (i = 0; i < nMatCount; i++)
/*?*/ 						{
/*?*/ 							if (!pMat->IsString(0, i))
/*?*/ 								fVal1 = pMat->GetDouble(0, i);
/*?*/ 							else
/*?*/ 								fVal1 = MAXDOUBLE;
/*?*/ 							if (fVal1 <= rEntry.nVal)
/*?*/ 								nDelta = i;
/*?*/ 							else
/*?*/ 								i = nMatCount+1;
/*?*/ 						}
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						for (i = 0; i < nMatCount; i++)
/*?*/ 						{
/*?*/ 							if (!pMat->IsString(0, i))
/*?*/ 								fVal1 = pMat->GetDouble(0, i);
/*?*/ 							else
/*?*/ 								fVal1 = MAXDOUBLE;
/*?*/ 							if (fVal1 == rEntry.nVal)
/*?*/ 							{
/*?*/ 								nDelta = i;
/*?*/ 								i = nMatCount + 1;
/*?*/ 							}
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 				if ( nDelta >= 0 )
/*?*/ 				{
/*?*/ 					if (!pMat->IsString(nSpIndex, nDelta))
/*?*/ 						PushDouble(pMat->GetDouble(nSpIndex, nDelta));
/*?*/ 					else
/*?*/ 						PushString(pMat->GetString(nSpIndex, nDelta));
/*?*/ 				}
/*?*/ 				else
/*?*/ 					SetNV();
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				rEntry.nField = nCol1;
/*N*/                 BOOL bFound = FALSE;
/*N*/                 USHORT nR;
/*N*/                 if ( bSorted )
/*N*/                     rEntry.eOp = SC_LESS_EQUAL;
/*N*/                 ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
/*N*/                 if ( bSorted )
/*N*/                 {
/*N*/                     USHORT nC;
/*N*/                     bFound = aCellIter.FindEqualOrSortedLastInRange( nC, nR );
/*N*/                 }
/*N*/                 else if ( aCellIter.GetFirst() )
/*N*/                 {
/*N*/                     bFound = TRUE;
/*N*/                     nR = aCellIter.GetRow();
/*N*/                     if ( bSorted )
/*N*/                     {
/*?*/                         while (aCellIter.GetNext())
/*?*/                             nR = aCellIter.GetRow();
/*N*/                     }
/*N*/                 }
/*N*/                 if ( bFound )
/*N*/                 {
/*N*/                     ScBaseCell* pCell;
/*N*/                     ScAddress aAdr( nSpIndex, nR, nTab1 );
/*N*/                     if ( HasCellValueData( pCell = GetCell( aAdr ) ) )
/*N*/                         PushDouble(GetCellValue( aAdr, pCell ));
/*N*/                     else
/*N*/                     {
/*N*/                         String aStr;
/*N*/                         GetCellString(aStr, pCell);
/*N*/                         PushString(aStr);
/*N*/                     }
/*N*/                 }
/*N*/ 				else
/*N*/ 					SetNV();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*?*/ 			SetIllegalParameter();
/*N*/ 	}
/*N*/ }

#if defined(WIN) && defined(MSC)
#pragma optimize("",off)
#endif

/*N*/ void ScInterpreter::ScSubTotal()
/*N*/ {
/*N*/ 	BYTE nParamCount = GetByte();
/*N*/ 	if ( MustHaveParamCountMin( nParamCount, 2 ) )
/*N*/ 	{
/*N*/ 		// Wir muessen den 1. Parameter tief aus dem Stack herausfischen!
/*N*/ 		const ScToken* p = pStack[ sp - nParamCount ];
/*N*/ 		PushTempToken( *p );
/*N*/ 		int nFunc = (int) ::rtl::math::approxFloor( GetDouble() );
/*N*/ 		if( nFunc < 1 || nFunc > 11 )
/*N*/ 			SetIllegalParameter();
/*N*/ 		else
/*N*/ 		{
/*N*/ 			cPar = nParamCount - 1;
/*N*/ 			glSubTotal = TRUE;
/*N*/ 			switch( nFunc )
/*N*/ 			{
/*?*/ 				case SUBTOTAL_FUNC_AVE  : ScAverage(); break;
/*?*/ 				case SUBTOTAL_FUNC_CNT  : ScCount();   break;
/*?*/ 				case SUBTOTAL_FUNC_CNT2 : ScCount2();  break;
/*?*/ 				case SUBTOTAL_FUNC_MAX  : ScMax();     break;
/*?*/ 				case SUBTOTAL_FUNC_MIN  : ScMin();     break;
/*?*/ 				case SUBTOTAL_FUNC_PROD : ScProduct(); break;
/*?*/ 				case SUBTOTAL_FUNC_STD  : ScStDev();   break;
/*N*/ 				case SUBTOTAL_FUNC_STDP : ScStDevP();  break;
/*?*/ 				case SUBTOTAL_FUNC_SUM  : ScSum();     break;
/*?*/ 				case SUBTOTAL_FUNC_VAR  : ScVar();     break;
/*?*/ 				case SUBTOTAL_FUNC_VARP : ScVarP();    break;
/*?*/ 				default : SetIllegalParameter();       break;
/*N*/ 			}
/*N*/ 			glSubTotal = FALSE;
/*N*/ 		}
/*N*/ 		// den abgefischten 1. Parameter entfernen
/*N*/ 		double nVal = GetDouble();
/*N*/ 		Pop();
/*N*/ 		PushDouble( nVal );
/*N*/ 	}
/*N*/ }
#if defined(WIN) && defined(MSC)
#pragma optimize("",on)
#endif


BOOL ScInterpreter::GetDBParams(USHORT& rTab, ScQueryParam& rParam,
        BOOL& rMissingField )
{
    BOOL bRet = FALSE;
    BOOL bAllowMissingField = FALSE;
    if ( rMissingField )
    {
        bAllowMissingField = TRUE;
        rMissingField = FALSE;
    }
    if ( GetByte() == 3 )
    {

        USHORT nQCol1, nQRow1, nQTab1, nQCol2, nQRow2, nQTab2;
        PopDoubleRef(nQCol1, nQRow1, nQTab1, nQCol2, nQRow2, nQTab2);

        BOOL	bByVal = TRUE;
        double	nVal = 0.0;
        String  aStr;
        ScRange aMissingRange;
        BOOL bRangeFake = FALSE;
        switch (GetStackType())
        {
            case svDouble :
                nVal = ::rtl::math::approxFloor( GetDouble() );
                if ( bAllowMissingField && nVal == 0.0 )
                    rMissingField = TRUE;   // fake missing parameter
                break;
            case svString :
                bByVal = FALSE;
                aStr = GetString();
                break;
            case svSingleRef :
                {
                    ScAddress aAdr;
                    PopSingleRef( aAdr );
                    ScBaseCell* pCell = GetCell( aAdr );
                    if (HasCellValueData(pCell))
                        nVal = GetCellValue( aAdr, pCell );
                    else
                    {
                        bByVal = FALSE;
                        GetCellString(aStr, pCell);
                    }
                }
                break;
            case svDoubleRef :
                if ( bAllowMissingField )
                {   // fake missing parameter for old SO compatibility
                    bRangeFake = TRUE;
                    PopDoubleRef( aMissingRange );
                }
                else
                {
                    PopError();
                    SetError( errIllegalParameter );
                }
                break;
            case svMissing :
                PopError();
                if ( bAllowMissingField )
                    rMissingField = TRUE;
                else
                    SetError( errIllegalParameter );
                break;
            default:
                PopError();
                SetError( errIllegalParameter );
        }

        USHORT nDBCol1, nDBRow1, nDBTab1, nDBCol2, nDBRow2, nDBTab2;
        PopDoubleRef(nDBCol1, nDBRow1, nDBTab1, nDBCol2, nDBRow2, nDBTab2);

        if ( nGlobalError == 0 && bRangeFake )
        {
            // range parameter must match entire database range
            if ( aMissingRange == ScRange( nDBCol1, nDBRow1, nDBTab1, nDBCol2,
                        nDBRow2, nDBTab2) )
                rMissingField = TRUE;
            else
                SetError( errIllegalParameter );
        }
        
        if (nGlobalError == 0)
        {
            USHORT	nField = nDBCol1;
            BOOL	bFound = TRUE;
            if ( rMissingField )
                ;   // special case
            else if ( bByVal )
            {
                if ( nVal <= 0 || nVal > (nDBCol2 - nDBCol1 + 1) )
                    bFound = FALSE;
                else
                    nField = Min(nDBCol2, (USHORT)(nDBCol1 + (USHORT)nVal - 1));
            }
            else
            {
                bFound = FALSE;
                String aCellStr;
                ScAddress aLook( nDBCol1, nDBRow1, nDBTab1 );
                while (!bFound && (aLook.Col() <= nDBCol2))
                {
                    ScBaseCell* pCell = GetCell( aLook );
                    GetCellString( aCellStr, pCell );
                    bFound = ScGlobal::pTransliteration->isEqual( aCellStr, aStr );
                    if (!bFound)
                        aLook.IncCol();
                }
                nField = aLook.Col();
            }
            if (bFound)
            {
                rParam.nCol1 = nDBCol1;
                rParam.nRow1 = nDBRow1;
                rParam.nCol2 = nDBCol2;
                rParam.nRow2 = nDBRow2;
                rParam.nTab  = nDBTab1;
                rParam.bHasHeader = TRUE;
                rParam.bByRow = TRUE;
                rParam.bInplace = TRUE;
                rParam.bCaseSens = FALSE;
                rParam.bRegExp = FALSE;
                rParam.bDuplicate = TRUE;
                if (pDok->CreateQueryParam(nQCol1, nQRow1, nQCol2, nQRow2, nQTab1, rParam))
                {
                    // An allowed missing field parameter sets the result field
                    // to any of the query fields, just to be able to return
                    // some cell from the iterator.
                    if ( rMissingField )
                        nField = rParam.GetEntry(0).nField;

                    rParam.nCol1 = nField;
                    rParam.nCol2 = nField;
                    rTab = nDBTab1;
                    bRet = TRUE;
                    USHORT nCount = rParam.GetEntryCount();
                    for ( USHORT i=0; i < nCount; i++ )
                    {
                        ScQueryEntry& rEntry = rParam.GetEntry(i);
                        if ( rEntry.bDoQuery )
                        {
                            sal_uInt32 nIndex = 0;
                            rEntry.bQueryByString = !pFormatter->IsNumberFormat(
                                *rEntry.pStr, nIndex, rEntry.nVal );
                            if ( rEntry.bQueryByString && !rParam.bRegExp )
                                rParam.bRegExp = MayBeRegExp( *rEntry.pStr, pDok );
                        }
                        else
                            break;	// for
                    }
                }
            }
        }
    }
    return bRet;
}


 void ScInterpreter::DBIterator( ScIterFunc eFunc )
 {
     USHORT nTab1;
     double nErg = 0.0;
     double fMem = 0.0;
     BOOL bNull = TRUE;
     ULONG nCount = 0;
     ScQueryParam aQueryParam;
     BOOL bMissingField = FALSE;
     if ( GetDBParams( nTab1, aQueryParam, bMissingField) )
     {
         double nVal;
         USHORT nErr;
         ScQueryValueIterator aValIter(pDok, nTab1, aQueryParam);
         if ( aValIter.GetFirst(nVal, nErr) && !nErr )
         {
             switch( eFunc )
             {
                 case ifPRODUCT:	nErg = 1; break;
                 case ifMAX:		nErg = MINDOUBLE; break;
                 case ifMIN:		nErg = MAXDOUBLE; break;
             }
             do
             {
                 nCount++;
                 switch( eFunc )
                 {
                     case ifAVERAGE:
                     case ifSUM:
                         if ( bNull && nVal != 0.0 )
                         {
                             bNull = FALSE;
                             fMem = nVal;
                         }
                         else
                             nErg += nVal;
                         break;
                     case ifSUMSQ:	nErg += nVal * nVal; break;
                     case ifPRODUCT:	nErg *= nVal; break;
                     case ifMAX:		if( nVal > nErg ) nErg = nVal; break;
                     case ifMIN:		if( nVal < nErg ) nErg = nVal; break;
                 }
             }
             while ( aValIter.GetNext(nVal, nErr) && !nErr );
         }
         SetError(nErr);
     }
     else
         SetIllegalParameter();
     switch( eFunc )
     {
         case ifCOUNT:	nErg = nCount; break;
         case ifSUM:		nErg = ::rtl::math::approxAdd( nErg, fMem ); break;
         case ifAVERAGE:	nErg = ::rtl::math::approxAdd( nErg, fMem ) / nCount; break;
     }
     PushDouble( nErg );
 }


 void ScInterpreter::ScDBSum()
 {
     DBIterator( ifSUM );
 }


 void ScInterpreter::ScDBCount()
 {
     USHORT nTab;
     ScQueryParam aQueryParam;
     BOOL bMissingField = TRUE;
     if ( GetDBParams( nTab, aQueryParam, bMissingField) )
     {
         ULONG nCount = 0;
         if ( bMissingField )
         {   // count all matching records
             // TODO: currently the QueryIterators only return cell pointers of
             // existing cells, so if a query matches an empty cell there's
             // nothing returned, and therefor not counted!
             // Since this has ever been the case and this code here only came
             // into existance to fix #i6899 and it never worked before we'll
             // have to live with it until we reimplement the iterators to also
             // return empty cells, which would mean to adapt all callers of
             // iterators.
             ScQueryCellIterator aCellIter( pDok, nTab, aQueryParam);
             if ( aCellIter.GetFirst() )
             {
                 do
                 {
                     nCount++;
                 } while ( aCellIter.GetNext() );
             }
         }
         else
         {   // count only matching records with a value in the "result" field
             double nVal;
             USHORT nErr = 0;
             ScQueryValueIterator aValIter( pDok, nTab, aQueryParam);
             if ( aValIter.GetFirst( nVal, nErr) && !nErr )
             {
                 do
                 {
                     nCount++;
                 } while ( aValIter.GetNext( nVal, nErr) && !nErr );
             }
             SetError( nErr );
         }
         PushDouble( nCount );
     }
     else
         SetIllegalParameter();
 }


 void ScInterpreter::ScDBCount2()
 {
     USHORT nTab;
     ScQueryParam aQueryParam;
     BOOL bMissingField = FALSE;
     if (GetDBParams( nTab, aQueryParam, bMissingField))
     {
         ULONG nCount = 0;
         ScQueryCellIterator aCellIter(pDok, nTab, aQueryParam);
         if ( aCellIter.GetFirst() )
         {
             do
             {
                 nCount++;
             } while ( aCellIter.GetNext() );
         }
         PushDouble(nCount);
     }
     else
         SetIllegalParameter();
 }


 void ScInterpreter::ScDBAverage()
 {
     DBIterator( ifAVERAGE );
 }


 void ScInterpreter::ScDBMax()
 {
     DBIterator( ifMAX );
 }


 void ScInterpreter::ScDBMin()
 {
     DBIterator( ifMIN );
 }


 void ScInterpreter::ScDBProduct()
 {
     DBIterator( ifPRODUCT );
 }


 void ScInterpreter::GetDBStVarParams( double& rVal, double& rValCount )
 {
     rValCount = 0.0;
     double fSum    = 0.0;
     double fSumSqr = 0.0;
     USHORT nTab;
     ScQueryParam aQueryParam;
     BOOL bMissingField = FALSE;
     if (GetDBParams( nTab, aQueryParam, bMissingField))
     {
         double fVal;
         USHORT nErr;
         ScQueryValueIterator aValIter(pDok, nTab, aQueryParam);
         if (aValIter.GetFirst(fVal, nErr) && !nErr)
         {
             do
             {
                 rValCount++;
                 fSum += fVal;
                 fSumSqr += fVal*fVal;
             }
             while ((nErr == 0) && aValIter.GetNext(fVal, nErr));
         }
         SetError(nErr);
     }
     else
         SetIllegalParameter();
     rVal = ::rtl::math::approxSub( fSumSqr, fSum*fSum/rValCount );
 }


 void ScInterpreter::ScDBStdDev()
 {
     double fVal, fCount;
     GetDBStVarParams( fVal, fCount );
     PushDouble( sqrt(fVal/(fCount-1)));
 }


 void ScInterpreter::ScDBStdDevP()
 {
     double fVal, fCount;
     GetDBStVarParams( fVal, fCount );
     PushDouble( sqrt(fVal/fCount));
 }


 void ScInterpreter::ScDBVar()
 {
     double fVal, fCount;
     GetDBStVarParams( fVal, fCount );
     PushDouble(fVal/(fCount-1));
 }


 void ScInterpreter::ScDBVarP()
 {
     double fVal, fCount;
     GetDBStVarParams( fVal, fCount );
     PushDouble(fVal/fCount);
 }


void ScInterpreter::ScIndirect()
{
    BYTE nParamCount = GetByte();
/*
    if (nParamCount == 2)
    {
        double fBool = GetDouble();
        if (fBool == 0.0)							// nur TRUE erlaubt!!
        {
            SetIllegalParameter();
            return;
        }
        else
            nParamCount = 1;
    }
*/
    if ( MustHaveParamCount( nParamCount, 1 )  )
    {
        USHORT nTab = aPos.Tab();
        String sRefStr( GetString() );
        ScRefTripel aRefTr, aRefTr2;
        if ( ConvertDoubleRef( pDok, sRefStr, nTab, aRefTr, aRefTr2 ) )
            PushDoubleRef( aRefTr.GetCol(), aRefTr.GetRow(), aRefTr.GetTab(),
                aRefTr2.GetCol(), aRefTr2.GetRow(), aRefTr2.GetTab() );
        else if ( ConvertSingleRef( pDok, sRefStr, nTab, aRefTr ) )
            PushSingleRef( aRefTr.GetCol(), aRefTr.GetRow(), aRefTr.GetTab() );
        else
            SetIllegalArgument();
    }
}


void ScInterpreter::ScAdress()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 4 ) )
    {
        String sTabStr;
        USHORT nAbs = 1;
        if (nParamCount == 4)
            sTabStr = GetString();
        if (nParamCount >= 3)
            nAbs = (USHORT) ::rtl::math::approxFloor(GetDouble());
        USHORT nCol = (USHORT) ::rtl::math::approxFloor(GetDouble());
        USHORT nRow = (USHORT) ::rtl::math::approxFloor(GetDouble());
        if (nCol < 1 || nCol > MAXCOL + 1 || nRow < 1 || nRow > MAXROW + 1)
        {
            SetIllegalParameter();
            return;
        }
        else
        {
            nRow--;
            nCol--;
        }
        String aRefStr;
        ScTripel aScTr;
        aScTr.SetCol(nCol);
        aScTr.SetRow(nRow);
        if (nAbs == 4)
            aRefStr = aScTr.GetColRowString();
        else
        {
            aRefStr = aScTr.GetColRowString(TRUE);
            if (nAbs == 2)
                aRefStr.EraseLeadingChars('$');
            else if (nAbs == 3)
                aRefStr.Erase(aRefStr.Search('$',1),1);
        }
        if ( sTabStr.Len() )
        {
            aRefStr.Insert('.',0);
            aRefStr.Insert(sTabStr,0);
        }
        PushString(aRefStr);
    }
}


void ScInterpreter::ScOffset()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 3, 5 ) )
    {
        short nColNew, nRowNew, nColPlus, nRowPlus;
        if (nParamCount == 5)
            nColNew = (short) ::rtl::math::approxFloor(GetDouble());
        if (nParamCount >= 4)
            nRowNew = (short) ::rtl::math::approxFloor(GetDouble());
        nColPlus = (short) ::rtl::math::approxFloor(GetDouble());
        nRowPlus = (short) ::rtl::math::approxFloor(GetDouble());
        USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
        if ( (nParamCount == 5 && nColNew == 0)
          || (nParamCount >= 4 && nRowNew == 0) )
        {
            SetIllegalParameter();
            return;
        }
        if (GetStackType() == svSingleRef)
        {
            PopSingleRef(nCol1, nRow1, nTab1);
            if (nParamCount == 3)
            {
                nCol1 = (USHORT)((short) nCol1 + nColPlus);
                nRow1 = (USHORT)((short) nRow1 + nRowPlus);
                if (nCol1 > MAXCOL || nRow1 > MAXROW)
                    SetIllegalParameter();
                else
                    PushSingleRef(nCol1, nRow1, nTab1);
            }
            else
            {
                if (nParamCount == 4)
                    nColNew = 1;
                nCol1 = (USHORT)((short)nCol1+nColPlus);		// ! nCol1 wird veraendert!
                nRow1 = (USHORT)((short)nRow1+nRowPlus);
                nCol2 = (USHORT)((short)nCol1+nColNew-1);
                nRow2 = (USHORT)((short)nRow1+nRowNew-1);
                if (nCol1 > MAXCOL || nRow1 > MAXROW ||
                    nCol2 > MAXCOL || nRow2 > MAXROW)
                    SetIllegalParameter();
                else
                    PushDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab1);
            }
        }
        else if (GetStackType() == svDoubleRef)
        {
            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            if (nParamCount < 5)
                nColNew = nCol2 - nCol1 + 1;
            if (nParamCount < 4)
                nRowNew = nRow2 - nRow1 + 1;
            nCol1 = (USHORT)((short)nCol1+nColPlus);
            nRow1 = (USHORT)((short)nRow1+nRowPlus);
            nCol2 = (USHORT)((short)nCol1+nColNew-1);
            nRow2 = (USHORT)((short)nRow1+nRowNew-1);
            if (nCol1 > MAXCOL || nRow1 > MAXROW ||
                nCol2 > MAXCOL || nRow2 > MAXROW || nTab1 != nTab2)
                SetIllegalParameter();
            else
                PushDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab1);
        }
        else
            SetIllegalParameter();
    }
}


void ScInterpreter::ScIndex()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 4 ) )
    {
        short nBereich, nMaxAnz, nCount;
        USHORT nCol, nRow;
        if (nParamCount == 4)
            nBereich = (short) ::rtl::math::approxFloor(GetDouble());
        else
            nBereich = 1;
        if (nParamCount >= 3)
            nCol = (USHORT) ::rtl::math::approxFloor(GetDouble());
        else
            nCol = 0;
        if (nParamCount >= 2)
            nRow = (USHORT) ::rtl::math::approxFloor(GetDouble());
        else
            nRow = 0;
        if (GetStackType() == svByte)					// vorher MultiSelektion?
            nMaxAnz = (short) PopByte();
        else											// sonst Einzelselektion
            nMaxAnz = 1;
        if (nBereich > nMaxAnz || nBereich < 1)
        {
            SetIllegalParameter();
            return;
        }
        if (GetStackType() == svMatrix)
        {
            if (nBereich != 1)
                SetError(errIllegalParameter);
            USHORT nMatInd1;
            USHORT nOldSp = sp;
            ScMatrix* pMat = GetMatrix(nMatInd1);
            if (pMat)
            {
                USHORT nC, nR, nMatInd;
                pMat->GetDimensions(nC, nR);
                if (nC == 0 || nR == 0 || nCol > nC || nRow > nR)
                    SetIllegalArgument();
                else if (nCol == 0 && nRow == 0)
                    sp = nOldSp;
                else if (nRow == 0)
                {
                    ScMatrix* pResMat = GetNewMat(nC, 1, nMatInd);
                    if (pResMat)
                    {
                        USHORT nColMinus1 = nCol - 1;
                        for (USHORT i = 0; i < nC; i++)
                            if (!pMat->IsString(i, nColMinus1))
                                pResMat->PutDouble(pMat->GetDouble(i,
                                    nColMinus1), i, 0);
                            else
                                pResMat->PutString(pMat->GetString(i,
                                    nColMinus1), i, 0);
                        PushMatrix(pResMat);
                        nRetMat = nMatInd;
                    }
                    else
                        SetNoValue();
                }
                else if (nCol == 0)
                {
                    ScMatrix* pResMat = GetNewMat(1, nR, nMatInd);
                    if (pResMat)
                    {
                        USHORT nRowMinus1 = nRow - 1;
                        for (USHORT i = 0; i < nR; i++)
                            if (!pMat->IsString(nRowMinus1, i))
                                pResMat->PutDouble(pMat->GetDouble(nRowMinus1,
                                    i), i);
                            else
                                pResMat->PutString(pMat->GetString(nRowMinus1,
                                    i), i);
                        PushMatrix(pResMat);
                        nRetMat = nMatInd;
                    }
                    else
                        SetNoValue();
                }
                else
                {
                    if (!pMat->IsString(nCol-1, nRow-1))
                        PushDouble(pMat->GetDouble(nCol-1, nRow-1));
                    else
                        PushString(pMat->GetString(nCol-1, nRow-1));
                }
                ResetNewMat(nMatInd1);
            }
        }
        else if (GetStackType() == svSingleRef || GetStackType() == svDoubleRef)
        {
            ScAddress aDummyAdr;
            ScRange aDummyRange;
            USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 = MAXTAB+1;
            nCount = nMaxAnz;	// Refs liegen umgekehrt auf dem Stack!
            while (nCount > nBereich && !nGlobalError)		// erste Refs weg
            {
                nCount--;
                if ( GetStackType() == svSingleRef )
                    PopSingleRef( aDummyAdr );
                else if ( GetStackType() == svDoubleRef )
                    PopDoubleRef( aDummyRange );
            }
            while (nCount > nBereich-1 && !nGlobalError)		// richtigen Teilbezug
            {
                nCount--;
                if (GetStackType() == svSingleRef)
                    PopSingleRef(nCol1, nRow1, nTab1);
                else if (GetStackType() == svDoubleRef)
                    PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            }
            while (nCount > 0 && !nGlobalError)		// restliche Refs weg
            {
                nCount--;
                if ( GetStackType() == svSingleRef )
                    PopSingleRef( aDummyAdr );
                else if ( GetStackType() == svDoubleRef )
                    PopDoubleRef( aDummyRange );
            }
            if (nTab2 == MAXTAB+1)							// SingleRef
            {
                if (nCol > 1 || nRow > 1)
                    SetIllegalParameter();
                else
                    PushSingleRef(nCol1, nRow1, nTab1);
            }
            else											// DoubleRef
            {
                if ( nTab1 != nTab2 ||
                    (nCol > 0 && nCol1+nCol-1 > nCol2) ||
                    (nRow > 0 && nRow1+nRow-1 > nRow2) )
                    SetIllegalParameter();
                else if (nCol == 0 && nRow == 0)
                {
                    if ( nCol1 == nCol2 && nRow1 == nRow2 )
                        PushSingleRef( nCol1, nRow1, nTab1 );
                    else
                        PushDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab1 );
                }
                else if (nRow == 0)
                {
                    if ( nRow1 == nRow2 )
                        PushSingleRef( nCol1+nCol-1, nRow1, nTab1 );
                    else
                        PushDoubleRef( nCol1+nCol-1, nRow1, nTab1,
                                      nCol1+nCol-1, nRow2, nTab1 );
                }
                else if (nCol == 0)
                {
                    if ( nCol1 == nCol2 )
                        PushSingleRef( nCol1, nRow1+nRow-1, nTab1 );
                    else
                        PushDoubleRef(nCol1, nRow1+nRow-1, nTab1,
                                      nCol2, nRow1+nRow-1, nTab1);
                }
                else
                    PushSingleRef(nCol1+nCol-1, nRow1+nRow-1, nTab1);
            }
        }
        else
            SetIllegalParameter();
    }
}


void ScInterpreter::ScMultiArea()
{
    // Hier ist nichts zu tun, der paramCount von ScMultiSelektion
    // bleibt auf dem Stack !!
    // Den muessen die nachfolgenden Funktionen (Index ...) wegraeumen !!
}


void ScInterpreter::ScAreas()
{
    BYTE nParamCount = GetByte();
    double fMaxAnz = 1.0;
    ScAddress aDummyAdr;
    ScRange aDummyRange;
    for (USHORT i = 0; i < nParamCount && nGlobalError == 0; i++)
    {
        if (GetStackType() == svByte)					// vorher MultiSelektion?
        {
            double fCount = 0.0;
            fMaxAnz = (double) GetByte();
            while (fCount < fMaxAnz && !nGlobalError)		// mehrere Refs
            {
                fCount++;
                if (GetStackType() == svSingleRef)
                    PopSingleRef( aDummyAdr );
                else if (GetStackType() == svDoubleRef)
                    PopDoubleRef( aDummyRange );
                else
                    SetIllegalParameter();
            }
        }
        else if (GetStackType() == svSingleRef)
            PopSingleRef( aDummyAdr );
        else if (GetStackType() == svDoubleRef)
            PopDoubleRef( aDummyRange );
        else
            SetIllegalParameter();
    }
    if (nGlobalError == 0)
        PushDouble((double)nParamCount + fMaxAnz - 1.0);
}


void ScInterpreter::ScCurrency()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        String aStr;
        double fDec;
        if (nParamCount == 2)
        {
            fDec = ::rtl::math::approxFloor(GetDouble());
            if (fDec < -15.0 || fDec > 15.0)
            {
                SetIllegalArgument();
                return;
            }
        }
        else
            fDec = 2.0;
        double fVal = GetDouble();
        double fFac;
        if ( fDec != 0.0 )
            fFac = pow( (double)10, fDec );
        else
            fFac = 1.0;
        if (fVal < 0.0)
            fVal = ceil(fVal*fFac-0.5)/fFac;
        else
            fVal = floor(fVal*fFac+0.5)/fFac;
        Color* pColor = NULL;
        if ( fDec < 0.0 )
            fDec = 0.0;
        ULONG nIndex = pFormatter->GetStandardFormat(
                                        NUMBERFORMAT_CURRENCY,
                                        ScGlobal::eLnge);
        if ( (USHORT) fDec != pFormatter->GetFormatPrecision( nIndex ) )
        {
            String sFormatString;
            pFormatter->GenerateFormat(sFormatString,
                                                   nIndex,
                                                   ScGlobal::eLnge,
                                                   TRUE,		// mit Tausenderpunkt
                                                   FALSE,		// nicht rot
                                                  (USHORT) fDec,// Nachkommastellen
                                                   1);			// 1 Vorkommanull
            if (!pFormatter->GetPreviewString(sFormatString,
                                                  fVal,
                                                  aStr,
                                                  &pColor,
                                                  ScGlobal::eLnge))
                SetError(errIllegalParameter);
        }
        else
        {
            pFormatter->GetOutputString(fVal, nIndex, aStr, &pColor);
        }
        PushString(aStr);
    }
}


void ScInterpreter::ScReplace()
{
    if ( MustHaveParamCount( GetByte(), 4 ) )
    {
        String aNewStr( GetString() );
        short nCount = (short) GetDouble();
        short nPos   = (short) GetDouble();
        String aOldStr( GetString() );
        if( nPos < 1 || nCount < 1 )
            SetIllegalArgument();
        else
        {
            aOldStr.Erase( nPos-1, nCount );
            if ( CheckStringResultLen( aOldStr, aNewStr ) )
                aOldStr.Insert( aNewStr, nPos-1 );
            PushString( aOldStr );
        }
    }
}


void ScInterpreter::ScFixed()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 3 ) )
    {
        String aStr;
        double fDec;
        BOOL bThousand;
        if (nParamCount == 3)
            bThousand = !GetBool();		// Param TRUE: keine Tausenderpunkte
        else
            bThousand = TRUE;
        if (nParamCount >= 2)
        {
            fDec = ::rtl::math::approxFloor(GetDouble());
            if (fDec < -15.0 || fDec > 15.0)
            {
                SetIllegalArgument();
                return;
            }
        }
        else
            fDec = 2.0;
        double fVal = GetDouble();
        double fFac;
        if ( fDec != 0.0 )
            fFac = pow( (double)10, fDec );
        else
            fFac = 1.0;
        if (fVal < 0.0)
            fVal = ceil(fVal*fFac-0.5)/fFac;
        else
            fVal = floor(fVal*fFac+0.5)/fFac;
        Color* pColor = NULL;
        String sFormatString;
        if (fDec < 0.0)
            fDec = 0.0;
        ULONG nIndex = pFormatter->GetStandardFormat(
                                            NUMBERFORMAT_NUMBER,
                                            ScGlobal::eLnge);
        pFormatter->GenerateFormat(sFormatString,
                                               nIndex,
                                               ScGlobal::eLnge,
                                               bThousand,	// mit Tausenderpunkt
                                               FALSE,		// nicht rot
                                               (USHORT) fDec,// Nachkommastellen
                                               1);			// 1 Vorkommanull
        if (!pFormatter->GetPreviewString(sFormatString,
                                                  fVal,
                                                  aStr,
                                                  &pColor,
                                                  ScGlobal::eLnge))
            SetIllegalParameter();
        else
            PushString(aStr);
    }
}


void ScInterpreter::ScFind()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        double fAnz;
        if (nParamCount == 3)
            fAnz = GetDouble();
        else
            fAnz = 1.0;
        String sStr = GetString();
        if( fAnz < 1.0 || fAnz > (double) sStr.Len() )
            SetNoValue();
        else
        {
            xub_StrLen nPos = sStr.Search( GetString(), (xub_StrLen) fAnz - 1 );
            if (nPos == STRING_NOTFOUND)
                SetNoValue();
            else
                PushDouble((double)(nPos + 1));
        }
    }
}


void ScInterpreter::ScExact()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        String s1( GetString() );
        String s2( GetString() );
        PushInt( s1 == s2 );
    }
}


void ScInterpreter::ScLeft()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        xub_StrLen n;
        if (nParamCount == 2)
        {
            double nVal = ::rtl::math::approxFloor(GetDouble());
            if ( nVal < 0.0 || nVal > STRING_MAXLEN )
            {
                SetIllegalParameter();
                return ;
            }
            else
                n = (xub_StrLen) nVal;
        }
        else
            n = 1;
        String aStr( GetString() );
        aStr.Erase( n );
        PushString( aStr );
    }
}


void ScInterpreter::ScRight()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        xub_StrLen n;
        if (nParamCount == 2)
        {
            double nVal = ::rtl::math::approxFloor(GetDouble());
            if ( nVal < 0.0 || nVal > STRING_MAXLEN )
            {
                SetIllegalParameter();
                return ;
            }
            else
                n = (xub_StrLen) nVal;
        }
        else
            n = 1;
        String aStr( GetString() );
        if( n < aStr.Len() )
            aStr.Erase( 0, aStr.Len() - n );
        PushString( aStr );
    }
}


void ScInterpreter::ScSearch()
{
    double fAnz;
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        if (nParamCount == 3)
        {
            fAnz = ::rtl::math::approxFloor(GetDouble());
            if (fAnz > double(STRING_MAXLEN))
            {
                SetIllegalParameter();
                return;
            }
        }
        else
            fAnz = 1.0;
        String sStr = GetString();
        String SearchStr = GetString();
        xub_StrLen nPos = (xub_StrLen) fAnz - 1;
        xub_StrLen nEndPos = sStr.Len();
        if( nPos >= nEndPos )
            SetNoValue();
        else
        {
            ::utl::SearchParam::SearchType eSearchType =
                (MayBeRegExp( SearchStr, pDok ) ?
                ::utl::SearchParam::SRCH_REGEXP : ::utl::SearchParam::SRCH_NORMAL);
            ::utl::SearchParam sPar(SearchStr, eSearchType, FALSE, FALSE, FALSE);
            ::utl::TextSearch sT( sPar, *ScGlobal::pCharClass );
            int nBool = sT.SearchFrwrd(sStr, &nPos, &nEndPos);
            if (!nBool)
                SetNoValue();
            else
                PushDouble((double)(nPos) + 1);
        }
    }
}


void ScInterpreter::ScMid()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double fAnz    = ::rtl::math::approxFloor(GetDouble());
        double fAnfang = ::rtl::math::approxFloor(GetDouble());
        const String& rStr = GetString();
        if (fAnfang < 1.0 || fAnz < 0.0 || fAnfang > double(STRING_MAXLEN) || fAnz > double(STRING_MAXLEN))
            SetIllegalParameter();
        else
            PushString(rStr.Copy( (xub_StrLen) fAnfang - 1, (xub_StrLen) fAnz ));
    }
}


/*N*/ void ScInterpreter::ScText()
/*N*/ {
/*N*/ 	if ( MustHaveParamCount( GetByte(), 2 ) )
/*N*/ 	{
/*N*/ 		String sFormatString = GetString();
/*N*/ 		double fVal = GetDouble();
/*N*/ 		String aStr;
/*N*/ 		Color* pColor = NULL;
/*N*/ 		LanguageType eCellLang;
/*N*/ 		const ScPatternAttr* pPattern = pDok->GetPattern(
/*N*/ 			aPos.Col(), aPos.Row(), aPos.Tab() );
/*N*/ 		if ( pPattern )
/*N*/ 			eCellLang = ((const SvxLanguageItem&)
/*N*/ 				pPattern->GetItem( ATTR_LANGUAGE_FORMAT )).GetValue();
/*N*/ 		else
/*?*/ 			eCellLang = ScGlobal::eLnge;
/*N*/ 		if ( !pFormatter->GetPreviewStringGuess( sFormatString, fVal, aStr,
/*N*/ 				&pColor, eCellLang ) )
/*?*/ 			SetIllegalParameter();
/*N*/ 		else
/*N*/ 			PushString(aStr);
/*N*/ 	}
/*N*/ }


void ScInterpreter::ScSubstitute()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 3, 4 ) )
    {
        xub_StrLen nAnz;
        if (nParamCount == 4)
        {
            double fAnz = ::rtl::math::approxFloor(GetDouble());
            if( fAnz < 1 || fAnz > STRING_MAXLEN )
            {
                SetIllegalParameter();
                return;
            }
            else
                nAnz = (xub_StrLen) fAnz;
        }
        else
            nAnz = 0;
        String sNewStr = GetString();
        String sOldStr = GetString();
        String sStr    = GetString();
        xub_StrLen nPos = 0;
        xub_StrLen nCount = 0;
        xub_StrLen nNewLen = sNewStr.Len();
        xub_StrLen nOldLen = sOldStr.Len();
        while( TRUE )
        {
            nPos = sStr.Search( sOldStr, nPos );
            if (nPos != STRING_NOTFOUND)
            {
                nCount++;
                if( !nAnz || nCount == nAnz )
                {
                    sStr.Erase(nPos,nOldLen);
                    if ( CheckStringResultLen( sStr, sNewStr ) )
                    {
                        sStr.Insert(sNewStr,nPos);
                        nPos += nNewLen;
                    }
                    else
                        break;
                }
                else
                    nPos++;
            }
            else
                break;
        }
        PushString( sStr );
    }
}


void ScInterpreter::ScRept()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double fAnz = ::rtl::math::approxFloor(GetDouble());
        String aStr( GetString() );
        if ( fAnz < 0.0 )
            SetIllegalParameter();
        else if ( fAnz * aStr.Len() > STRING_MAXLEN )
        {
            SetError( errStringOverflow );
            PushInt(0);
        }
        else if ( fAnz == 0.0 )
            PushString( EMPTY_STRING );
        else
        {
            xub_StrLen n = (xub_StrLen) fAnz;
            const xub_StrLen nLen = aStr.Len();
            String aRes;
            const sal_Unicode* const pSrc = aStr.GetBuffer();
            sal_Unicode* pDst = aRes.AllocBuffer( n * nLen );
            while( n-- )
            {
                memcpy( pDst, pSrc, nLen * sizeof(sal_Unicode) );
                pDst += nLen;
            }
            PushString( aRes );
        }
    }
}


void ScInterpreter::ScConcat()
{
    BYTE nParamCount = GetByte();
    String aRes;
    while( nParamCount-- )
    {
        const String& rStr = GetString();
        aRes.Insert( rStr, 0 );
    }
    PushString( aRes );
}


/*N*/  void ScInterpreter::ScErrorType()
/*N*/  {
/*N*/  	USHORT nErr;
/*N*/  	USHORT nOldError = nGlobalError;
/*N*/  	nGlobalError = 0;
/*N*/  	switch ( GetStackType() )
/*N*/  	{
/*N*/  		case svDoubleRef :
/*N*/  		{
/*N*/  			ScRange aRange;
/*N*/  			PopDoubleRef( aRange );
/*N*/  			if ( nGlobalError )
/*N*/  				nErr = nGlobalError;
/*N*/  			else
/*N*/  			{
/*N*/  				ScAddress aAdr;
/*N*/  				if ( DoubleRefToPosSingleRef( aRange, aAdr ) )
/*N*/  					nErr = pDok->GetErrCode( aAdr );
/*N*/  				else
/*N*/  					nErr = nGlobalError;
/*N*/  			}
/*N*/  		}
/*N*/  		break;
/*N*/  		case svSingleRef :
/*N*/  		{
/*N*/  			ScAddress aAdr;
/*N*/  			PopSingleRef( aAdr );
/*N*/  			if ( nGlobalError )
/*N*/  				nErr = nGlobalError;
/*N*/  			else
/*N*/  				nErr = pDok->GetErrCode( aAdr );
/*N*/  		}
/*N*/  		break;
/*N*/  		default:
/*N*/  			PopError();
/*N*/  			nErr = nGlobalError;
/*N*/  	}
/*N*/  	if ( nErr )
/*N*/  	{
/*N*/  		nGlobalError = 0;
/*N*/  		PushDouble( nErr );
/*N*/  	}
/*N*/  	else
/*N*/  	{
/*N*/  		nGlobalError = nOldError;
/*N*/  		SetNV();
/*N*/  	}
/*N*/  }


/*N*/ BOOL ScInterpreter::MayBeRegExp( const String& rStr, const ScDocument* pDoc  )
/*N*/ {
/*N*/     if ( pDoc && !pDoc->GetDocOptions().IsFormulaRegexEnabled() )
/*N*/         return FALSE;
/*N*/ 	if ( !rStr.Len() || (rStr.Len() == 1 && rStr.GetChar(0) != '.') )
/*N*/ 		return FALSE;	// einzelnes Metazeichen kann keine RegExp sein
/*N*/ 	static const sal_Unicode cre[] = { '.','*','+','?','[',']','^','$','\\','<','>','(',')','|', 0 };
/*N*/ 	const sal_Unicode* p1 = rStr.GetBuffer();
/*N*/ 	sal_Unicode c1;
/*N*/ 	while ( c1 = *p1++ )
/*N*/ 	{
/*N*/ 		const sal_Unicode* p2 = cre;
/*N*/ 		while ( *p2 )
/*N*/ 		{
/*N*/ 			if ( c1 == *p2++ )
/*N*/ 				return TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
