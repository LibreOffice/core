/*************************************************************************
 *
 *  $RCSfile: interpr1.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: er $ $Date: 2001-03-14 18:10:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifdef RS6000
#pragma options FLTTRAP
#include <fptrap.h>
#include <fpxcp.h>
#endif

#include "scitems.hxx"
#include <svx/langitem.hxx>
#include <svx/algitem.hxx>
#include <unotools/textsearch.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <tools/solar.h>
#include <unotools/charclass.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <unotools/collatorwrapper.hxx>

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "interpre.hxx"
#include "patattr.hxx"
#include "global.hxx"
#include "document.hxx"
#include "dociter.hxx"
#include "cell.hxx"
#include "scmatrix.hxx"
#include "docoptio.hxx"
#include "globstr.hrc"
#include "attrib.hxx"


// PI jetzt als F_PI aus solar.h
//#define   PI            3.1415926535897932
//#define   MINVALUE      1.7e-307
//#define SQRT_2_PI   2.506628274631000

//  globale Variablen

#pragma code_seg("SCSTATICS")

IMPL_FIXEDMEMPOOL_NEWDEL( ScTokenStack, 8, 4 )
IMPL_FIXEDMEMPOOL_NEWDEL( ScErrorStack, 8, 4 )
IMPL_FIXEDMEMPOOL_NEWDEL( ScInterpreter, 32, 16 )

#pragma code_seg()

ScTokenStack* ScInterpreter::pGlobalStack = NULL;
ScErrorStack* ScInterpreter::pGlobalErrorStack = NULL;
BOOL ScInterpreter::bGlobalStackInUse = FALSE;


//-----------------------------------------------------------------------------
// Funktionen
//-----------------------------------------------------------------------------


void ScInterpreter::ScIfJump()
{
    const short* pJump = pCur->GetJump();
    short nJumpCount = pJump[ 0 ];
    if ( GetBool() )
    {   // TRUE
        if( nJumpCount >= 2 )
        {   // then Zweig
            nFuncFmtType = NUMBERFORMAT_UNDEFINED;
            aCode.Jump( pJump[ 1 ], pJump[ nJumpCount ] );
        }
        else
        {   // kein Parameter fuer then
            nFuncFmtType = NUMBERFORMAT_LOGICAL;
            PushInt(1);
            aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
        }
    }
    else
    {   // FALSE
        if( nJumpCount == 3 )
        {   // else Zweig
            nFuncFmtType = NUMBERFORMAT_UNDEFINED;
            aCode.Jump( pJump[ 2 ], pJump[ nJumpCount ] );
        }
        else
        {   // kein Parameter fuer else
            nFuncFmtType = NUMBERFORMAT_LOGICAL;
            PushInt(0);
            aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
        }
    }
}


void ScInterpreter::ScChoseJump()
{
    const short* pJump = pCur->GetJump();
    short nJumpCount = pJump[ 0 ];
    double nJumpIndex = SolarMath::ApproxFloor( GetDouble() );
    if ((nJumpIndex >= 1) && (nJumpIndex < nJumpCount))
        aCode.Jump( pJump[ (short) nJumpIndex ], pJump[ nJumpCount ] );
    else
        SetError(errIllegalArgument);
}


short ScInterpreter::CompareFunc( const ScCompare& rComp )
{
    short nRes = 0;
    if ( rComp.bEmpty[ 0 ] )
    {
        if ( rComp.bEmpty[ 1 ] )
            ;       // leere Zelle == leere Zelle, nRes 0
        else if( rComp.bVal[ 1 ] )
        {
            if ( !SolarMath::ApproxEqual( rComp.nVal[ 1 ], 0.0 ) )
            {
                if ( rComp.nVal[ 1 ] < 0.0 )
                    nRes = 1;       // leere Zelle > -x
                else
                    nRes = -1;      // leere Zelle < x
            }
            // else: leere Zelle == 0.0
        }
        else
        {
            if ( rComp.pVal[ 1 ]->Len() )
                nRes = -1;      // leere Zelle < "..."
            // else: leere Zelle == ""
        }
    }
    else if ( rComp.bEmpty[ 1 ] )
    {
        if( rComp.bVal[ 0 ] )
        {
            if ( !SolarMath::ApproxEqual( rComp.nVal[ 0 ], 0.0 ) )
            {
                if ( rComp.nVal[ 0 ] < 0.0 )
                    nRes = -1;      // -x < leere Zelle
                else
                    nRes = 1;       // x > leere Zelle
            }
            // else: leere Zelle == 0.0
        }
        else
        {
            if ( rComp.pVal[ 0 ]->Len() )
                nRes = 1;       // "..." > leere Zelle
            // else: "" == leere Zelle
        }
    }
    else if( rComp.bVal[ 0 ] )
    {
        if( rComp.bVal[ 1 ] )
        {
            if ( !SolarMath::ApproxEqual( rComp.nVal[ 0 ], rComp.nVal[ 1 ] ) )
            {
                if( rComp.nVal[ 0 ] - rComp.nVal[ 1 ] < 0 )
                    nRes = -1;
                else
                    nRes = 1;
            }
        }
        else
            nRes = -1;  // Zahl ist kleiner als String
    }
    else if( rComp.bVal[ 1 ] )
        nRes = 1;   // Zahl ist kleiner als String
    else
    {
        if (pDok->GetDocOptions().IsIgnoreCase())
            nRes = (short) ScGlobal::pCollator->compareString(
                *rComp.pVal[ 0 ], *rComp.pVal[ 1 ] );
        else
            nRes = (*rComp.pVal[ 0 ]).CompareTo( *rComp.pVal[ 1 ] );
    }
    return nRes;
}


short ScInterpreter::Compare()
{
    nCurFmtType = nFuncFmtType = NUMBERFORMAT_LOGICAL;
    String aVal1, aVal2;
    ScCompare aComp( &aVal1, &aVal2 );
    for( short i = 1; i >= 0; i-- )
    {
        switch ( GetStackType() )
        {
            case svDouble:
                aComp.nVal[ i ] = GetDouble();
                aComp.bVal[ i ] = TRUE;
                break;
            case svString:
                *aComp.pVal[ i ] = GetString();
                aComp.bVal[ i ] = FALSE;
                break;
            case svDoubleRef :
            case svSingleRef :
            {
                ScAddress aAdr;
                if ( !PopDoubleRefOrSingleRef( aAdr ) )
                    break;
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
            default:
                SetError(errIllegalParameter);
            break;
        }
    }
    if( nGlobalError )
        return 0;
    return CompareFunc( aComp );
}


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


void ScInterpreter::ScEqual()
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
            pMat->CompareEqual();
            PushMatrix( pMat );
        }
    }
    else
        PushInt( Compare() == 0 );
}


void ScInterpreter::ScNotEqual()
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
            pMat->CompareNotEqual();
            PushMatrix( pMat );
        }
    }
    else
        PushInt( Compare() != 0 );
}


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


void ScInterpreter::ScNot()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    PushInt( GetDouble() == 0.0 );
}


void ScInterpreter::ScNeg()
{
    PushDouble(-GetDouble());
}


void ScInterpreter::ScPi()
{
    PushDouble(F_PI);
}


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


void ScInterpreter::ScSin()
{
    PushDouble(sin(GetDouble()));
}


void ScInterpreter::ScCos()
{
    PushDouble(cos(GetDouble()));
}


void ScInterpreter::ScTan()
{
    PushDouble(tan(GetDouble()));
}


void ScInterpreter::ScCot()
{
    PushDouble(1.0 / tan(GetDouble()));
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
{   // ATTRIBUTE ; [REF]
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
            String          aResult;
            ScBaseCell*     pCell = GetCell( aCellPos );

            aInfoType.ToUpperAscii();

// *** ADDRESS INFO ***
            if( aInfoType.EqualsAscii( "COL" ) )
            {   // column number (1-based)
                PushInt( aCellPos.Col() + 1 );
            }
            else if( aInfoType.EqualsAscii( "ROW" ) )
            {   // row number (1-based)
                PushInt( aCellPos.Row() + 1 );
            }
            else if( aInfoType.EqualsAscii( "SHEET" ) )
            {   // table number (1-based)
                PushInt( aCellPos.Tab() + 1 );
            }
            else if( aInfoType.EqualsAscii( "ADDRESS" ) )
            {   // address formatted as [['FILENAME'#]$TABLE.]$COL$ROW
                USHORT nFlags = (aCellPos.Tab() == aPos.Tab()) ? (SCA_ABS) : (SCA_ABS_3D);
                aCellPos.Format( aResult, nFlags, pDok );
                PushString( aResult );
            }
            else if( aInfoType.EqualsAscii( "FILENAME" ) )
            {   // file name and table name: 'FILENAME'#$TABLE
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
            {   // address, lotus 1-2-3 formatted: $TABLE:$COL$ROW
                ScAddress( aCellPos.Tab(), 0, 0 ).Format( aResult, (SCA_COL_ABSOLUTE|SCA_VALID_COL) );
                aResult += ':';
                String aCellStr;
                aCellPos.Format( aCellStr, (SCA_COL_ABSOLUTE|SCA_VALID_COL|SCA_ROW_ABSOLUTE|SCA_VALID_ROW) );
                aResult += aCellStr;
                PushString( aResult );
            }

// *** CELL PROPERTIES ***
            else if( aInfoType.EqualsAscii( "CONTENTS" ) )
            {   // contents of the cell, no formatting
                if( pCell && pCell->HasStringData() )
                {
                    GetCellString( aResult, pCell );
                    PushString( aResult );
                }
                else
                    PushDouble( GetCellValue( aCellPos, pCell ) );
            }
            else if( aInfoType.EqualsAscii( "TYPE" ) )
            {   // b = blank; l = string (label); v = otherwise (value)
                if( HasCellStringData( pCell ) )
                    aResult = 'l';
                else
                    aResult = HasCellValueData( pCell ) ? 'v' : 'b';
                PushString( aResult );
            }
            else if( aInfoType.EqualsAscii( "WIDTH" ) )
            {   // column width (rounded off as count of zero characters in standard font and size)
                Printer*    pPrinter = pDok->GetPrinter();
                MapMode     aOldMode( pPrinter->GetMapMode() );
                Font        aOldFont( pPrinter->GetFont() );
                Font        aDefFont;

                pPrinter->SetMapMode( MAP_TWIP );
                pDok->GetDefPattern()->GetFont( aDefFont, pPrinter );
                pPrinter->SetFont( aDefFont );
                long nZeroWidth = pPrinter->GetTextWidth( String( '0' ) );
                pPrinter->SetFont( aOldFont );
                pPrinter->SetMapMode( aOldMode );
                int nZeroCount = (int)(pDok->GetColWidth( aCellPos.Col(), aCellPos.Tab() ) / nZeroWidth);
                PushInt( nZeroCount );
            }
            else if( aInfoType.EqualsAscii( "PREFIX" ) )
            {   // ' = left; " = right; ^ = centered
                if( HasCellStringData( pCell ) )
                {
                    const SvxHorJustifyItem* pJustAttr = (const SvxHorJustifyItem*)
                        pDok->GetAttr( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), ATTR_HOR_JUSTIFY );
                    switch( pJustAttr->GetValue() )
                    {
                        case SVX_HOR_JUSTIFY_STANDARD:
                        case SVX_HOR_JUSTIFY_LEFT:
                        case SVX_HOR_JUSTIFY_BLOCK:     aResult = '\''; break;
                        case SVX_HOR_JUSTIFY_CENTER:    aResult = '^';  break;
                        case SVX_HOR_JUSTIFY_RIGHT:     aResult = '"';  break;
                        case SVX_HOR_JUSTIFY_REPEAT:    aResult = '\\'; break;
                    }
                }
                PushString( aResult );
            }
            else if( aInfoType.EqualsAscii( "PROTECT" ) )
            {   // 1 = cell locked
                const ScProtectionAttr* pProtAttr = (const ScProtectionAttr*)
                    pDok->GetAttr( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), ATTR_PROTECTION );
                PushInt( pProtAttr->GetProtection() ? 1 : 0 );
            }

// *** FORMATTING ***
            else if( aInfoType.EqualsAscii( "FORMAT" ) )
            {   // specific format code for standard formats
                ULONG   nFormat = pDok->GetNumberFormat( aCellPos );
                BOOL    bAppendPrec = TRUE;
                USHORT  nPrec, nLeading;
                BOOL    bThousand, bIsRed;
                pFormatter->GetFormatSpecialInfo( nFormat, bThousand, bIsRed, nPrec, nLeading );

                switch( pFormatter->GetType( nFormat ) )
                {
                    case NUMBERFORMAT_NUMBER:       aResult = (bThousand ? ',' : 'F');  break;
                    case NUMBERFORMAT_CURRENCY:     aResult = 'C';                      break;
                    case NUMBERFORMAT_SCIENTIFIC:   aResult = 'S';                      break;
                    case NUMBERFORMAT_PERCENT:      aResult = 'P';                      break;
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
                            case NF_DATE_DIN_DMMMMYYYY: aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D1" ) );  break;
                            case NF_DATE_SYS_DDMMM:     aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D2" ) );  break;
                            case NF_DATE_SYS_MMYY:      aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D3" ) );  break;
                            case NF_DATETIME_SYSTEM_SHORT_HHMM:
                            case NF_DATETIME_SYS_DDMMYYYY_HHMMSS:
                                                        aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D4" ) );  break;
                            case NF_DATE_DIN_MMDD:      aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D5" ) );  break;
                            case NF_TIME_HHMMSSAMPM:    aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D6" ) );  break;
                            case NF_TIME_HHMMAMPM:      aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D7" ) );  break;
                            case NF_TIME_HHMMSS:        aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D8" ) );  break;
                            case NF_TIME_HHMM:          aResult.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "D9" ) );  break;
                            default:                    aResult = 'G';
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
            {   // 1 = negative values are colored, otherwise 0
                const SvNumberformat* pFormat = pFormatter->GetEntry( pDok->GetNumberFormat( aCellPos ) );
                PushInt( lcl_FormatHasNegColor( pFormat ) ? 1 : 0 );
            }
            else if( aInfoType.EqualsAscii( "PARENTHESES" ) )
            {   // 1 = format string contains a '(' character, otherwise 0
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
        nRes = ( fmod( SolarMath::ApproxFloor( fabs( fVal ) ), 2.0 ) < 0.5 );
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
        nGlobalError = 0;       // N(#NV) und N("text") sind ok
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
        if ( *p != ' ' || p[-1] != ' ' )    // erster kann kein ' ' sein, -1 ist also ok
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
    String aUpr( ScGlobal::pCharClass->upper( aStr ) );
    String aLwr( ScGlobal::pCharClass->lower( aStr ) );
    register sal_Unicode* pStr = aStr.GetBufferAccess();
    const sal_Unicode* pUpr = aUpr.GetBuffer();
    const sal_Unicode* pLwr = aLwr.GetBuffer();
    *pStr = *pUpr;
    String aTmpStr( 'x' );
    xub_StrLen nPos = 1;
    const xub_StrLen nLen = aStr.Len();
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
                //  wie GetString()
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
    ULONG nFIndex = 0;                  // damit default Land/Spr.
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


void ScInterpreter::ScMin( BOOL bTextAsZero )
{
    BYTE nParamCount = GetByte();
    double nMin = SC_DOUBLE_MAXVALUE;
    double nVal = 0.0;
    ScAddress aAdr;
    ScRange aRange;
    for (short i = 0; i < nParamCount; i++)
    {
        switch (GetStackType())
        {
            case svDouble :
            {
                nVal = GetDouble();
                if (nMin > nVal) nMin = nVal;
                nFuncFmtType = NUMBERFORMAT_NUMBER;
            }
            break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellValueData(pCell))
                {
                    nVal = GetCellValue( aAdr, pCell );
                    CurFmtToFuncFmt();
                    if (nMin > nVal) nMin = nVal;
                }
                else if ( bTextAsZero && HasCellStringData( pCell ) )
                {
                    if ( nMin > 0.0 )
                        nMin = 0.0;
                }
            }
            break;
            case svDoubleRef :
            {
                USHORT nErr = 0;
                PopDoubleRef( aRange );
                ScValueIterator aValIter( pDok, aRange, glSubTotal, bTextAsZero );
                if (aValIter.GetFirst(nVal, nErr))
                {
                    if (nMin > nVal)
                        nMin = nVal;
                    aValIter.GetCurNumFmtInfo( nFuncFmtType, nFuncFmtIndex );
                    while ((nErr == 0) && aValIter.GetNext(nVal, nErr))
                    {
                        if (nMin > nVal)
                            nMin = nVal;
                    }
                    SetError(nErr);
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    nFuncFmtType = NUMBERFORMAT_NUMBER;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                            {
                                nVal = pMat->GetDouble(i,j);
                                if (nMin > nVal) nMin = nVal;
                            }
                    }
                    else
                    {
                        for (USHORT i = 0; i < nC; i++)
                        {
                            for (USHORT j = 0; j < nR; j++)
                            {
                                if (!pMat->IsString(i,j))
                                {
                                    nVal = pMat->GetDouble(i,j);
                                    if (nMin > nVal) nMin = nVal;
                                }
                                else if ( bTextAsZero )
                                {
                                    if ( nMin > 0.0 )
                                        nMin = 0.0;
                                }
                            }
                         }
                    }
                }
            }
            break;
            case svString :
            {
                Pop();
                if ( bTextAsZero )
                {
                    if ( nMin > 0.0 )
                        nMin = 0.0;
                }
                else
                    SetError(errIllegalParameter);
            }
            break;
            default :
                Pop();
                SetError(errIllegalParameter);
        }
    }
    if (nMin == SC_DOUBLE_MAXVALUE)
        SetIllegalArgument();
    else
        PushDouble(nMin);
}

#if defined(WIN) && defined(MSC)
#pragma optimize("",off)
#endif

void ScInterpreter::ScMax( BOOL bTextAsZero )
{
    BYTE nParamCount = GetByte();
    double nMax = -SC_DOUBLE_MAXVALUE;
    double nVal = 0.0;
    ScAddress aAdr;
    ScRange aRange;
    for (short i = 0; i < nParamCount; i++)
    {
        switch (GetStackType())
        {
            case svDouble :
            {
                nVal = GetDouble();
                if (nMax < nVal) nMax = nVal;
                nFuncFmtType = NUMBERFORMAT_NUMBER;
            }
            break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellValueData(pCell))
                {
                    nVal = GetCellValue( aAdr, pCell );
                    CurFmtToFuncFmt();
                    if (nMax < nVal) nMax = nVal;
                }
                else if ( bTextAsZero && HasCellStringData( pCell ) )
                {
                    if ( nMax < 0.0 )
                        nMax = 0.0;
                }
            }
            break;
            case svDoubleRef :
            {
                USHORT nErr = 0;
                PopDoubleRef( aRange );
                ScValueIterator aValIter( pDok, aRange, glSubTotal, bTextAsZero );
                if (aValIter.GetFirst(nVal, nErr))
                {
                    if (nMax < nVal)
                        nMax = nVal;
                    aValIter.GetCurNumFmtInfo( nFuncFmtType, nFuncFmtIndex );
                    while ((nErr == 0) && aValIter.GetNext(nVal, nErr))
                    {
                        if (nMax < nVal)
                            nMax = nVal;
                    }
                    SetError(nErr);
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    nFuncFmtType = NUMBERFORMAT_NUMBER;
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                            {
                                nVal = pMat->GetDouble(i,j);
                                if (nMax < nVal) nMax = nVal;
                            }
                    }
                    else
                    {
                        for (USHORT i = 0; i < nC; i++)
                        {
                            for (USHORT j = 0; j < nR; j++)
                            {
                                if (!pMat->IsString(i,j))
                                {
                                    nVal = pMat->GetDouble(i,j);
                                    if (nMax < nVal) nMax = nVal;
                                }
                                else if ( bTextAsZero )
                                {
                                    if ( nMax < 0.0 )
                                        nMax = 0.0;
                                }
                            }
                        }
                    }
                }
            }
            break;
            case svString :
            {
                Pop();
                if ( bTextAsZero )
                {
                    if ( nMax < 0.0 )
                        nMax = 0.0;
                }
                else
                    SetError(errIllegalParameter);
            }
            break;
            default :
                Pop();
                SetError(errIllegalParameter);
        }
    }
    if (nMax == -SC_DOUBLE_MAXVALUE)
        SetIllegalArgument();
    else
        PushDouble(nMax);
}
#if defined(WIN) && defined(MSC)
#pragma optimize("",on)
#endif


double ScInterpreter::IterateParameters( ScIterFunc eFunc, BOOL bTextAsZero )
{
    BYTE nParamCount = GetByte();
    double fRes = ( eFunc == ifPRODUCT ) ? 1.0 : 0.0;
    double fVal = 0.0;
    double fMem = 0.0;
    BOOL bNull = TRUE;
    ULONG nCount = 0;
    ScAddress aAdr;
    ScRange aRange;
    for (short i = 0; i < nParamCount; i++)
    {
        switch (GetStackType())
        {

            case svString:
            {
                if( eFunc == ifCOUNT )
                {
                    String aStr( PopString() );
                    ULONG nFIndex = 0;                  // damit default Land/Spr.
                    if ( bTextAsZero || pFormatter->IsNumberFormat(aStr, nFIndex, fVal))
                        nCount++;
                }
                else
                {
                    switch ( eFunc )
                    {
                        case ifAVERAGE:
                        case ifSUM:
                        case ifSUMSQ:
                        case ifPRODUCT:
                        {
                            if ( bTextAsZero )
                            {
                                Pop();
                                nCount++;
                                if ( eFunc == ifPRODUCT )
                                    fRes = 0.0;
                            }
                            else
                            {
                                for ( ; i < nParamCount; i++ )
                                    Pop();
                                SetError( errNoValue );
                            }
                        }
                        break;
                        default:
                            Pop();
                            nCount++;
                    }
                }
            }
            break;
            case svDouble    :
                fVal = GetDouble();
                nCount++;
                switch( eFunc )
                {
                    case ifAVERAGE:
                    case ifSUM:
                        if ( bNull && fVal != 0.0 )
                        {
                            bNull = FALSE;
                            fMem = fVal;
                        }
                        else
                            fRes += fVal;
                        break;
                    case ifSUMSQ:   fRes += fVal * fVal; break;
                    case ifPRODUCT: fRes *= fVal; break;
                }
                nFuncFmtType = NUMBERFORMAT_NUMBER;
                break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if ( pCell )
                {
                    if( eFunc == ifCOUNT2 )
                    {
                        CellType eCellType = pCell->GetCellType();
                        if (eCellType != CELLTYPE_NONE && eCellType != CELLTYPE_NOTE)
                            nCount++;
                    }
                    else if ( pCell->HasValueData() )
                    {
                        nCount++;
                        fVal = GetCellValue( aAdr, pCell );
                        CurFmtToFuncFmt();
                        switch( eFunc )
                        {
                            case ifAVERAGE:
                            case ifSUM:
                                if ( bNull && fVal != 0.0 )
                                {
                                    bNull = FALSE;
                                    fMem = fVal;
                                }
                                else
                                    fRes += fVal;
                                break;
                            case ifSUMSQ:   fRes += fVal * fVal; break;
                            case ifPRODUCT: fRes *= fVal; break;
                        }
                    }
                    else if ( bTextAsZero && pCell->HasStringData() )
                    {
                        nCount++;
                        if ( eFunc == ifPRODUCT )
                            fRes = 0.0;
                    }
                }
            }
            break;
            case svDoubleRef :
            {
                USHORT nErr = 0;
                PopDoubleRef( aRange );
                if( eFunc == ifCOUNT2 )
                {
                    ScBaseCell* pCell;
                    ScCellIterator aIter( pDok, aRange, glSubTotal );
                    if (pCell = aIter.GetFirst())
                    {
                        do
                        {
                            CellType eType = pCell->GetCellType();
                            if( eType != CELLTYPE_NONE && eType != CELLTYPE_NOTE )
                                nCount++;
                        }
                        while ( pCell = aIter.GetNext());
                    }
                }
                else
                {
                    ScValueIterator aValIter( pDok, aRange, glSubTotal, bTextAsZero );
                    if (aValIter.GetFirst(fVal, nErr))
                    {
                        //  Schleife aus Performance-Gruenden nach innen verlegt:
                        aValIter.GetCurNumFmtInfo( nFuncFmtType, nFuncFmtIndex );
                        switch( eFunc )
                        {
                            case ifAVERAGE:
                            case ifSUM:
                                    do
                                    {
                                        SetError(nErr);
                                        if ( bNull && fVal != 0.0 )
                                        {
                                            bNull = FALSE;
                                            fMem = fVal;
                                        }
                                        else
                                            fRes += fVal;
                                        nCount++;
                                    }
                                    while (aValIter.GetNext(fVal, nErr));
                                    break;
                            case ifSUMSQ:
                                    do
                                    {
                                        SetError(nErr);
                                        fRes += fVal * fVal;
                                        nCount++;
                                    }
                                    while (aValIter.GetNext(fVal, nErr));
                                    break;
                            case ifPRODUCT:
                                    do
                                    {
                                        SetError(nErr);
                                        fRes *= fVal;
                                        nCount++;
                                    }
                                    while (aValIter.GetNext(fVal, nErr));
                                    break;
                            default:                // count
                                    do
                                    {
                                        SetError(nErr);
                                        nCount++;
                                    }
                                    while (aValIter.GetNext(fVal, nErr));
                                    break;
                        }
                        SetError( nErr );
                    }
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    nFuncFmtType = NUMBERFORMAT_NUMBER;
                    pMat->GetDimensions(nC, nR);
                    if( eFunc == ifCOUNT2 )
                        nCount += (ULONG) nC * nR;
                    else
                    {
                        for (USHORT i = 0; i < nC; i++)
                        {
                            for (USHORT j = 0; j < nR; j++)
                            {
                                if (!pMat->IsString(i,j))
                                {
                                    nCount++;
                                    fVal = pMat->GetDouble(i,j);
                                    switch( eFunc )
                                    {
                                        case ifAVERAGE:
                                        case ifSUM:
                                            if ( bNull && fVal != 0.0 )
                                            {
                                                bNull = FALSE;
                                                fMem = fVal;
                                            }
                                            else
                                                fRes += fVal;
                                            break;
                                        case ifSUMSQ:   fRes += fVal * fVal; break;
                                        case ifPRODUCT: fRes *= fVal; break;
                                    }
                                }
                                else if ( bTextAsZero )
                                {
                                    nCount++;
                                    if ( eFunc == ifPRODUCT )
                                        fRes = 0.0;
                                }
                            }
                        }
                    }
                }
            }
            break;
            default :
                for ( ; i < nParamCount; i++ )
                    Pop();
                SetError(errIllegalParameter);
        }
    }
    switch( eFunc )
    {
        case ifSUM:     fRes = SolarMath::ApproxAdd( fRes, fMem ); break;
        case ifAVERAGE: fRes = SolarMath::ApproxAdd( fRes, fMem ) / nCount; break;
        case ifCOUNT2:
        case ifCOUNT:   fRes  = nCount; break;
        case ifPRODUCT: if ( !nCount ) fRes = 0.0; break;
    }
    // Bei Summen etc. macht ein BOOL-Ergebnis keinen Sinn
    // und Anzahl ist immer Number (#38345#)
    if( eFunc == ifCOUNT || nFuncFmtType == NUMBERFORMAT_LOGICAL )
        nFuncFmtType = NUMBERFORMAT_NUMBER;
    return fRes;
}


void ScInterpreter::ScSumSQ()
{
    PushDouble( IterateParameters( ifSUMSQ ) );
}


void ScInterpreter::ScSum()
{
    PushDouble( IterateParameters( ifSUM ) );
}


void ScInterpreter::ScProduct()
{
    PushDouble( IterateParameters( ifPRODUCT ) );
}


void ScInterpreter::ScAverage( BOOL bTextAsZero )
{
    PushDouble( IterateParameters( ifAVERAGE, bTextAsZero ) );
}


void ScInterpreter::ScCount()
{
    PushDouble( IterateParameters( ifCOUNT ) );
}


void ScInterpreter::ScCount2()
{
    PushDouble( IterateParameters( ifCOUNT2 ) );
}


void ScInterpreter::GetStVarParams( double& rVal, double& rValCount,
                BOOL bTextAsZero )
{
    BYTE nParamCount = GetByte();
    USHORT i;
    double fSum    = 0.0;
    double fSumSqr = 0.0;
    double fVal;
    rValCount = 0.0;
    ScAddress aAdr;
    ScRange aRange;
    for (i = 0; i < nParamCount; i++)
    {
        switch (GetStackType())
        {
            case svDouble :
            {
                fVal = GetDouble();
                fSum    += fVal;
                fSumSqr += fVal*fVal;
                rValCount++;
            }
            break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellValueData(pCell))
                {
                    fVal = GetCellValue( aAdr, pCell );
                    fSum += fVal;
                    fSumSqr += fVal*fVal;
                    rValCount++;
                }
                else if ( bTextAsZero && HasCellStringData( pCell ) )
                    rValCount++;
            }
            break;
            case svDoubleRef :
            {
                USHORT nErr = 0;
                PopDoubleRef( aRange );
                ScValueIterator aValIter( pDok, aRange, glSubTotal, bTextAsZero );
                if (aValIter.GetFirst(fVal, nErr))
                {
                    do
                    {
                        fSum += fVal;
                        fSumSqr += fVal*fVal;
                        rValCount++;
                    }
                    while ((nErr == 0) && aValIter.GetNext(fVal, nErr));
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    for (USHORT i = 0; i < nC; i++)
                    {
                        for (USHORT j = 0; j < nR; j++)
                        {
                            if (!pMat->IsString(i,j))
                            {
                                fVal= pMat->GetDouble(i,j);
                                fSum += fVal;
                                fSumSqr += fVal * fVal;
                                rValCount++;
                            }
                            else if ( bTextAsZero )
                                rValCount++;
                        }
                    }
                }
            }
            break;
            case svString :
            {
                if ( bTextAsZero )
                    rValCount++;
                else
                {
                    Pop();
                    SetError(errIllegalParameter);
                }
            }
            default :
                Pop();
                SetError(errIllegalParameter);
        }
    }
    rVal = fSumSqr - fSum*fSum/rValCount;
}


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


void ScInterpreter::ScStDevP( BOOL bTextAsZero )
{
    double nVal;
    double nValCount;
    GetStVarParams( nVal, nValCount, bTextAsZero );
    PushDouble(sqrt(nVal / nValCount));
}


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
        USHORT nVal;
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


void ScInterpreter::ScMatch()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        double fTyp;
        if (nParamCount == 3)
            fTyp = GetDouble();
        else
            fTyp = 1.0;
        USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
        if (GetStackType() == svDoubleRef)
        {
            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            if (nTab1 != nTab2 || (nCol1 != nCol2 && nRow1 != nRow2))
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
        if (nGlobalError == 0)
        {
            double fVal;
            String sStr;
            ScQueryParam rParam;
            rParam.nCol1       = nCol1;
            rParam.nRow1       = nRow1;
            rParam.nCol2       = nCol2;
            rParam.bHasHeader  = FALSE;
            rParam.bInplace    = TRUE;
            rParam.bCaseSens   = FALSE;
            rParam.bRegExp     = TRUE;
            rParam.bDuplicate  = FALSE;

            ScQueryEntry& rEntry = rParam.GetEntry(0);
            rEntry.bDoQuery = TRUE;
            if (fTyp < 0.0)
                rEntry.eOp = SC_GREATER_EQUAL;
            else if (fTyp > 0.0)
                rEntry.eOp = SC_LESS_EQUAL;
            switch ( GetStackType() )
            {
                case svDouble:
                {
                    fVal = GetDouble();
                    rEntry.bQueryByString = FALSE;
                    rEntry.nVal = fVal;
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
                        fVal = GetCellValue( aAdr, pCell );
                        rEntry.bQueryByString = FALSE;
                        rEntry.nVal = fVal;
                    }
                    else
                    {
                        GetCellString(sStr, pCell);
                        rEntry.bQueryByString = TRUE;
                        *rEntry.pStr = sStr;
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
                rParam.bRegExp = MayBeRegExp( *rEntry.pStr );
            USHORT nDelta, nR, nC;
            if (nCol1 == nCol2)                         // spaltenweise
            {
                rParam.nRow2 = nRow2;
                rEntry.nField = nCol1;
                ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
                if (fTyp == 0.0)                        // exakte Suche
                {
                    if (aCellIter.GetFirst())
                    {
                        nR = aCellIter.GetRow();
                        if (aCellIter.GetNext())
                        {
                            SetNV();
                            return;
                        }
                    }
                    else
                    {
                        SetNV();
                        return;
                    }
                }
                else if (aCellIter.GetFirst())          // <= bzw. >= Suche
                {
                    do
                    {
                        nR = aCellIter.GetRow();
                    } while ( aCellIter.GetNext() );
                }
                else
                {
                    SetNV();
                    return;
                }
                nDelta = nR - nRow1;
            }
            else                                        // zeilenweise
            {
                rParam.nRow2 = nRow1;
                rEntry.nField = nCol1;
                ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
                // Entry.nField im Iterator bei Spaltenwechsel weiterschalten
                aCellIter.SetAdvanceQueryParamEntryField( TRUE );
                if (fTyp == 0.0)                        // exakte Suche
                {
                    if ( aCellIter.GetFirst() )
                    {
                        nC = aCellIter.GetCol();
                        if ( aCellIter.GetNext() )
                        {   // doppelt gefunden
                            SetNV();
                            return;
                        }
                    }
                    else
                    {   // gar nicht gefunden
                        SetNV();
                        return;
                    }
                }
                else if ( aCellIter.GetFirst() )        // <= bzw. >= Suche
                {
                    do
                    {
                        nC = aCellIter.GetCol();
                    } while ( aCellIter.GetNext() );
                }
                else
                {
                    SetNV();
                    return;
                }
                nDelta = nC - nCol1;
            }
            PushDouble((double) (nDelta + 1));
        }
        else
            SetIllegalParameter();
    }
}


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
            rParam.bRegExp     = TRUE;
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
                ULONG nIndex = 0;
                rEntry.bQueryByString =
                    !(pFormatter->IsNumberFormat(
                        *rEntry.pStr, nIndex, rEntry.nVal));
                if ( rEntry.bQueryByString )
                    rParam.bRegExp = MayBeRegExp( *rEntry.pStr );
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
            rParam.bRegExp     = TRUE;
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
                ULONG nIndex = 0;
                rEntry.bQueryByString =
                    !(pFormatter->IsNumberFormat(
                        *rEntry.pStr, nIndex, rEntry.nVal));
                if ( rEntry.bQueryByString )
                    rParam.bRegExp = MayBeRegExp( *rEntry.pStr );
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
            PushDouble( SolarMath::ApproxAdd( fSum, fMem ) );
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
    USHORT nC3, nR3, nC1, nR1;
    ScMatrix* pMat3 = NULL;
    ScMatrix* pMat1 = NULL;
    USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
    USHORT nCol3, nRow3, nTab3, nCol4, nRow4, nTab4;
    USHORT nDelta;
    if (nParamCount == 3)
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
        if (GetStackType() == svDoubleRef)
        {
            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            if (nTab1 != nTab2 || (nCol1 != nCol2 && nRow1 != nRow2))
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
            if (nCol1 == nCol2)
            {
                nMatCount = nRow2 - nRow1 + 1;
                bSpMatrix = TRUE;
            }
            else
            {
                nMatCount = nCol2 - nCol1 + 1;
                bSpMatrix = FALSE;
            }
        }
        else
        {
            if (nR1 == 1)
            {
                nMatCount = nC1;
                bSpMatrix = TRUE;
            }
            else
            {
                nMatCount = nR1;
                bSpMatrix = FALSE;
            }
        }
        if (pMat3 == NULL)
        {
            if (nCol3 == nCol4)
            {
                nVecCount = nRow4 - nRow3 + 1;
                bSpVector = TRUE;
            }
            else
            {
                nVecCount = nCol4 - nCol3 + 1;
                bSpVector = FALSE;
            }
        }
        else
        {
            if (nR3 == 1)
            {
                nVecCount = nC3;
                bSpVector = TRUE;
            }
            else
            {
                nVecCount = nR3;
                bSpVector = FALSE;
            }
        }
        if (nGlobalError == 0 && nVecCount == nMatCount)
        {
            String sStr;
            ScQueryParam rParam;
            rParam.nCol1       = nCol1;
            rParam.nRow1       = nRow1;
            rParam.nCol2       = nCol2;
            rParam.bHasHeader  = FALSE;
            rParam.bInplace    = TRUE;
            rParam.bCaseSens   = FALSE;
            rParam.bRegExp     = TRUE;
            rParam.bDuplicate  = FALSE;

            ScQueryEntry& rEntry = rParam.GetEntry(0);
            rEntry.bDoQuery = TRUE;
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
                rParam.bRegExp = MayBeRegExp( *rEntry.pStr );
            if (pMat1)
            {
                if (rEntry.bQueryByString)
                {
                    BOOL bFound = FALSE;
                    sal_Int32 nRes;
                    String aParamStr = *rEntry.pStr;
                    for (USHORT i = 0; i < nMatCount; i++)
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
                    for (USHORT i = 0; i < nMatCount; i++)
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
            else if (bSpMatrix)                         // spaltenweise
            {
                rParam.nRow2 = nRow2;
                rEntry.nField = nCol1;
                ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
                if ( aCellIter.GetFirst() )
                {
                    USHORT nR;
                    do
                    {
                        nR = aCellIter.GetRow();
                    } while ( aCellIter.GetNext() );
                    nDelta = nR - nRow1;
                }
                else
                {
                    SetNV();
                    return;
                }
            }
            else                                    // zeilenweise
            {
                rParam.nRow2 = nRow1;
                rEntry.nField = nCol1;
                ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
                // Entry.nField im Iterator bei Spaltenwechsel weiterschalten
                aCellIter.SetAdvanceQueryParamEntryField( TRUE );
                if ( aCellIter.GetFirst() )
                {
                    USHORT nC;
                    do
                    {
                        nC = aCellIter.GetCol();
                    } while ( aCellIter.GetNext() );
                    nDelta = nC - nCol1;
                }
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
    else if (nParamCount == 2)
    {
        USHORT nMatInd;
        BOOL bSpMatrix;
        USHORT nMatCount;
        pMat1 = GetMatrix(nMatInd);
        if (!pMat1)
        {
            SetIllegalParameter();
            return;
        }
        pMat1->GetDimensions(nC1, nR1);
        if (nR1 >= nC1)
        {
            bSpMatrix = FALSE;
            nMatCount = nR1;
        }
        else
        {
            bSpMatrix = TRUE;
            nMatCount = nC1;
        }
        BOOL bIsStr;
        double fVal;
        String sStr;
        switch ( GetStackType() )
        {
            case svDouble:
            {
                fVal = GetDouble();
                bIsStr = FALSE;
            }
            break;
            case svString:
            {
                sStr = GetString();
                bIsStr = TRUE;
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
                    fVal = GetCellValue( aAdr, pCell );
                    bIsStr = FALSE;
                }
                else
                {
                    if ( GetCellType( pCell ) == CELLTYPE_NOTE )
                    {
                        fVal = 0.0;
                        bIsStr = FALSE;
                    }
                    else
                    {
                        GetCellString(sStr, pCell);
                        bIsStr = TRUE;
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
        if (bIsStr)
        {
            BOOL bFound = FALSE;
            for (USHORT i = 0; i < nMatCount; i++)
            {
                sal_Int32 nRes;
                if (bSpMatrix)
                {
                    if (pMat1->IsString(i, 0))
                        nRes = ScGlobal::pCollator->compareString(
                            pMat1->GetString(i,0), sStr );
                    else
                        nRes = COMPARE_GREATER;
                }
                else
                {
                    if (pMat1->IsString(0, i))
                        nRes = ScGlobal::pCollator->compareString(
                            pMat1->GetString(0,i), sStr );
                    else
                        nRes = COMPARE_GREATER;
                }
                if (nRes == COMPARE_EQUAL)
                {
                    bFound = TRUE;
                    nDelta = i;
                }
                else if (nRes == COMPARE_LESS)
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
        else
        {
            BOOL bFound = FALSE;
            double fVal1;
            for (USHORT i = 0; i < nMatCount; i++)
            {
                if (bSpMatrix)
                {
                    if (!pMat1->IsString(i, 0))
                        fVal1 = pMat1->GetDouble(i, 0);
                    else
                        fVal1 = MAXDOUBLE;
                }
                else
                {
                    if (!pMat1->IsString(0, i))
                        fVal1 = pMat1->GetDouble(0, i);
                    else
                        fVal1 = MAXDOUBLE;
                }
                if (fVal1 == fVal)
                {
                    bFound = TRUE;
                    nDelta = i;
                }
                else if (fVal1 > fVal)
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
        if (bSpMatrix)
        {
            if (pMat1->IsString(nDelta, nR1-1))
                PushString(pMat1->GetString(nDelta, nR1-1));
            else
                PushDouble(pMat1->GetDouble(nDelta, nR1-1));
        }
        else
        {
            if (pMat1->IsString(nC1-1, nDelta))
                PushString(pMat1->GetString(nC1-1, nDelta));
            else
                PushDouble(pMat1->GetDouble(nC1-1, nDelta));
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
        double fIndex = SolarMath::ApproxFloor( GetDouble() ) - 1.0;
        ScMatrix* pMat = NULL;
        USHORT nC, nR;
        USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
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
            nZIndex += nRow1;                       // Wertzeile
        if (nGlobalError == 0)
        {
            String sStr;
            ScQueryParam rParam;
            rParam.nCol1       = nCol1;
            rParam.nRow1       = nRow1;
            rParam.nCol2       = nCol2;
            rParam.nRow2       = nRow1;     // nur in der ersten Zeile suchen
            rParam.bHasHeader  = FALSE;
            rParam.bInplace    = TRUE;
            rParam.bCaseSens   = FALSE;
            rParam.bRegExp     = TRUE;
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
                rParam.bRegExp = MayBeRegExp( *rEntry.pStr );
            if (pMat)
            {
                USHORT nMatCount = nC;
                short nDelta = -1;
                if (rEntry.bQueryByString)
                {
                    sal_Int32 nRes;
                    String aParamStr = *rEntry.pStr;
                    USHORT i;
                    if ( bSorted )
                    {
                        for (i = 0; i < nMatCount; i++)
                        {
                            if (pMat->IsString(i, 0))
                            {
                                nRes = ScGlobal::pCollator->compareString(
                                    pMat->GetString(i,0), aParamStr );
                                if (nRes == COMPARE_LESS)
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
                                nRes = ScGlobal::pCollator->compareString(
                                    pMat->GetString(i,0), aParamStr );
                                if (nRes == COMPARE_EQUAL)
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
                ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
                // Entry.nField im Iterator bei Spaltenwechsel weiterschalten
                aCellIter.SetAdvanceQueryParamEntryField( TRUE );
                if ( aCellIter.GetFirst() )
                {
                    USHORT nC = aCellIter.GetCol();
                    if ( bSorted )
                    {   // <= Suche, #37661#: nicht abbrechen wenn nC > nCol1
                        // weil leere Zelle
                        while ( aCellIter.GetNext() )
                            nC = aCellIter.GetCol();
                    }
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


void ScInterpreter::ScVLookup()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 3, 4 ) )
    {
        BOOL bSorted;
        if (nParamCount == 4)
            bSorted = GetBool();
        else
            bSorted = TRUE;
        double fIndex = SolarMath::ApproxFloor( GetDouble() ) - 1.0;
        ScMatrix* pMat = NULL;
        USHORT nC, nR;
        USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
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
        if ( fIndex < 0.0 || (pMat ? (fIndex >= nC) : (fIndex+nCol1 > nCol2)) )
        {
            SetIllegalArgument();
            return;
        }
        USHORT nSpIndex = (USHORT) fIndex;
        if (!pMat)
            nSpIndex += nCol1;                      // Wertspalte
        if (nGlobalError == 0)
        {
            String sStr;
            ScQueryParam rParam;
            rParam.nCol1       = nCol1;
            rParam.nRow1       = nRow1;
            rParam.nCol2       = nCol1;     // nur in der ersten Spalte suchen
            rParam.nRow2       = nRow2;
            rParam.bHasHeader  = FALSE;
            rParam.bInplace    = TRUE;
            rParam.bCaseSens   = FALSE;
            rParam.bRegExp     = TRUE;
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
                rParam.bRegExp = MayBeRegExp( *rEntry.pStr );
            if (pMat)
            {
                USHORT nMatCount = nR;
                short nDelta = -1;
                if (rEntry.bQueryByString)
                {
                    sal_Int32 nRes;
                    String aParamStr = *rEntry.pStr;
                    USHORT i;
                    if ( bSorted )
                    {
                        for (i = 0; i < nMatCount; i++)
                        {
                            if (pMat->IsString(0, i))
                            {
                                nRes = ScGlobal::pCollator->compareString(
                                    pMat->GetString(0,i), aParamStr );
                                if (nRes == COMPARE_LESS)
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
                            if (pMat->IsString(0, i))
                            {
                                nRes = ScGlobal::pCollator->compareString(
                                    pMat->GetString(0,i), aParamStr );
                                if (nRes == COMPARE_EQUAL)
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
                            if (!pMat->IsString(0, i))
                                fVal1 = pMat->GetDouble(0, i);
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
                            if (!pMat->IsString(0, i))
                                fVal1 = pMat->GetDouble(0, i);
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
                    if (!pMat->IsString(nSpIndex, nDelta))
                        PushDouble(pMat->GetDouble(nSpIndex, nDelta));
                    else
                        PushString(pMat->GetString(nSpIndex, nDelta));
                }
                else
                    SetNV();
            }
            else
            {
                rEntry.nField = nCol1;
                ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
                if (aCellIter.GetFirst())
                {
                    USHORT nR = aCellIter.GetRow();
                    if ( bSorted )
                    {   // <= Suche, #37661#: nicht abbrechen wenn nR > nRow1
                        // weil leere Zelle
                        while (aCellIter.GetNext())
                            nR = aCellIter.GetRow();
                    }
                    ScBaseCell* pCell;
                    ScAddress aAdr( nSpIndex, nR, nTab1 );
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

#if defined(WIN) && defined(MSC)
#pragma optimize("",off)
#endif

void ScInterpreter::ScSubTotal()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCountMin( nParamCount, 2 ) )
    {
        // Wir muessen den 1. Parameter tief aus dem Stack herausfischen!
        const ScToken* p = pStack[ sp - nParamCount ];
        PushTempToken( *p );
        int nFunc = (int) SolarMath::ApproxFloor( GetDouble() );
        if( nFunc < 1 || nFunc > 11 )
            SetIllegalParameter();
        else
        {
            cPar = nParamCount - 1;
            glSubTotal = TRUE;
            switch( nFunc )
            {
                case SUBTOTAL_FUNC_AVE  : ScAverage(); break;
                case SUBTOTAL_FUNC_CNT  : ScCount();   break;
                case SUBTOTAL_FUNC_CNT2 : ScCount2();  break;
                case SUBTOTAL_FUNC_MAX  : ScMax();     break;
                case SUBTOTAL_FUNC_MIN  : ScMin();     break;
                case SUBTOTAL_FUNC_PROD : ScProduct(); break;
                case SUBTOTAL_FUNC_STD  : ScStDev();   break;
                case SUBTOTAL_FUNC_STDP : ScStDevP();  break;
                case SUBTOTAL_FUNC_SUM  : ScSum();     break;
                case SUBTOTAL_FUNC_VAR  : ScVar();     break;
                case SUBTOTAL_FUNC_VARP : ScVarP();    break;
                default : SetIllegalParameter();       break;
            }
            glSubTotal = FALSE;
        }
        // den abgefischten 1. Parameter entfernen
        double nVal = GetDouble();
        Pop();
        PushDouble( nVal );
    }
}
#if defined(WIN) && defined(MSC)
#pragma optimize("",on)
#endif


BOOL ScInterpreter::GetDBParams(USHORT& rTab, ScQueryParam& rParam)
{
    BOOL bRet = FALSE;
    if ( GetByte() == 3 )
    {

        USHORT nQCol1, nQRow1, nQTab1, nQCol2, nQRow2, nQTab2;
        PopDoubleRef(nQCol1, nQRow1, nQTab1, nQCol2, nQRow2, nQTab2);

        BOOL    bByVal = TRUE;
        double  nVal;
        String  aStr;
        switch (GetStackType())
        {
            case svDouble :
                nVal = GetDouble();
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
        }

        USHORT nDBCol1, nDBRow1, nDBTab1, nDBCol2, nDBRow2, nDBTab2;
        PopDoubleRef(nDBCol1, nDBRow1, nDBTab1, nDBCol2, nDBRow2, nDBTab2);

        if (nGlobalError == 0)
        {
            USHORT  nField = nDBCol1;
            BOOL    bFound = TRUE;
            if (bByVal)
                nField = Min(nDBCol2, (USHORT)(nDBCol1 + (USHORT)SolarMath::ApproxFloor(nVal) - 1));
            else
            {
                bFound = FALSE;
                String aCellStr;
                nField = nDBCol1;
                while (!bFound && (nField <= nDBCol2))
                {
                    pDok->GetString(nField, nDBRow1, nDBTab1, aCellStr);
                    bFound = (ScGlobal::pCollator->compareString(
                        aCellStr, aStr ) == COMPARE_EQUAL);
                    if (!bFound)
                        nField++;
                }
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
                            ULONG nIndex = 0;
                            rEntry.bQueryByString = !pFormatter->IsNumberFormat(
                                *rEntry.pStr, nIndex, rEntry.nVal );
                            if ( rEntry.bQueryByString && !rParam.bRegExp )
                                rParam.bRegExp = MayBeRegExp( *rEntry.pStr );
                        }
                        else
                            break;  // for
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
    long nCount = 0;
    ScQueryParam aQueryParam;
    if (GetDBParams(nTab1, aQueryParam))
    {
        double nVal;
        USHORT nErr;
        ScQueryValueIterator aValIter(pDok, nTab1, aQueryParam);
        if ( aValIter.GetFirst(nVal, nErr) && !nErr )
        {
            switch( eFunc )
            {
                case ifPRODUCT: nErg = 1; break;
                case ifMAX:     nErg = MINDOUBLE; break;
                case ifMIN:     nErg = MAXDOUBLE; break;
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
                    case ifSUMSQ:   nErg += nVal * nVal; break;
                    case ifPRODUCT: nErg *= nVal; break;
                    case ifMAX:     if( nVal > nErg ) nErg = nVal; break;
                    case ifMIN:     if( nVal < nErg ) nErg = nVal; break;
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
        case ifCOUNT:   nErg = nCount; break;
        case ifSUM:     nErg = SolarMath::ApproxAdd( nErg, fMem ); break;
        case ifAVERAGE: nErg = (nCount ? SolarMath::ApproxAdd( nErg, fMem ) / nCount : 0); break;
    }
    PushDouble( nErg );
}


void ScInterpreter::ScDBSum()
{
    DBIterator( ifSUM );
}


void ScInterpreter::ScDBCount()
{
    DBIterator( ifCOUNT );
}


void ScInterpreter::ScDBCount2()
{
    USHORT nTab;
    ScQueryParam aQueryParam;
    if (GetDBParams(nTab, aQueryParam))
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


ULONG ScInterpreter::StdDev( double& rSum, double& rSum2 )
{
    USHORT nTab;
    ULONG nCount = 0;
    ScQueryParam aQueryParam;
    if (GetDBParams(nTab, aQueryParam))
    {
        double fVal, fSum = 0.0, fSumSqr = 0.0;
        USHORT nErr;
        ScQueryValueIterator aValIter(pDok, nTab, aQueryParam);
        if (aValIter.GetFirst(fVal, nErr) && !nErr)
        {
            do
            {
                nCount++;
                fSum += fVal;
                fSumSqr += fVal*fVal;
            }
            while ((nErr == 0) && aValIter.GetNext(fVal, nErr));
        }
        SetError(nErr);
        rSum = fSum;
        rSum2 = fSumSqr;
    }
    else
        SetIllegalParameter();
    return nCount;
}


void ScInterpreter::ScDBStdDev()
{
    ULONG nCount;
    double fSum, fSumSqr;
    nCount = StdDev( fSum, fSumSqr );
    PushDouble( sqrt((fSumSqr - fSum*fSum/nCount)/(nCount-1)));
}


void ScInterpreter::ScDBStdDevP()
{
    ULONG nCount;
    double fSum, fSumSqr;
    nCount = StdDev( fSum, fSumSqr );
    PushDouble( sqrt((fSumSqr - fSum*fSum/nCount)/nCount));
}


void ScInterpreter::ScDBVar()
{
    ULONG nCount;
    double fSum, fSumSqr;
    nCount = StdDev( fSum, fSumSqr );
    PushDouble((fSumSqr - fSum*fSum/nCount)/(nCount-1));
}


void ScInterpreter::ScDBVarP()
{
    ULONG nCount;
    double fSum, fSumSqr;
    nCount = StdDev( fSum, fSumSqr );
    PushDouble((fSumSqr - fSum*fSum/nCount)/nCount);
}


void ScInterpreter::ScIndirect()
{
    BYTE nParamCount = GetByte();
/*
    if (nParamCount == 2)
    {
        double fBool = GetDouble();
        if (fBool == 0.0)                           // nur TRUE erlaubt!!
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
            nAbs = (USHORT) SolarMath::ApproxFloor(GetDouble());
        USHORT nCol = (USHORT) SolarMath::ApproxFloor(GetDouble());
        USHORT nRow = (USHORT) SolarMath::ApproxFloor(GetDouble());
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
            nColNew = (short) SolarMath::ApproxFloor(GetDouble());
        if (nParamCount >= 4)
            nRowNew = (short) SolarMath::ApproxFloor(GetDouble());
        nColPlus = (short) SolarMath::ApproxFloor(GetDouble());
        nRowPlus = (short) SolarMath::ApproxFloor(GetDouble());
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
                nCol1 = (USHORT)((short)nCol1+nColPlus);        // ! nCol1 wird veraendert!
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
            nBereich = (short) SolarMath::ApproxFloor(GetDouble());
        else
            nBereich = 1;
        if (nParamCount >= 3)
            nCol = (USHORT) SolarMath::ApproxFloor(GetDouble());
        else
            nCol = 0;
        if (nParamCount >= 2)
            nRow = (USHORT) SolarMath::ApproxFloor(GetDouble());
        else
            nRow = 0;
        if (GetStackType() == svByte)                   // vorher MultiSelektion?
            nMaxAnz = (short) PopByte();
        else                                            // sonst Einzelselektion
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
                                    i), 0, i);
                            else
                                pResMat->PutString(pMat->GetString(nRowMinus1,
                                    i), 0, i);
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
            nCount = nMaxAnz;   // Refs liegen umgekehrt auf dem Stack!
            while (nCount > nBereich && !nGlobalError)      // erste Refs weg
            {
                nCount--;
                if ( GetStackType() == svSingleRef )
                    PopSingleRef( aDummyAdr );
                else if ( GetStackType() == svDoubleRef )
                    PopDoubleRef( aDummyRange );
            }
            while (nCount > nBereich-1 && !nGlobalError)        // richtigen Teilbezug
            {
                nCount--;
                if (GetStackType() == svSingleRef)
                    PopSingleRef(nCol1, nRow1, nTab1);
                else if (GetStackType() == svDoubleRef)
                    PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            }
            while (nCount > 0 && !nGlobalError)     // restliche Refs weg
            {
                nCount--;
                if ( GetStackType() == svSingleRef )
                    PopSingleRef( aDummyAdr );
                else if ( GetStackType() == svDoubleRef )
                    PopDoubleRef( aDummyRange );
            }
            if (nTab2 == MAXTAB+1)                          // SingleRef
            {
                if (nCol > 1 || nRow > 1)
                    SetIllegalParameter();
                else
                    PushSingleRef(nCol1, nRow1, nTab1);
            }
            else                                            // DoubleRef
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
        if (GetStackType() == svByte)                   // vorher MultiSelektion?
        {
            double fCount = 0.0;
            fMaxAnz = (double) GetByte();
            while (fCount < fMaxAnz && !nGlobalError)       // mehrere Refs
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
            fDec = SolarMath::ApproxFloor(GetDouble());
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
                                                   TRUE,        // mit Tausenderpunkt
                                                   FALSE,       // nicht rot
                                                  (USHORT) fDec,// Nachkommastellen
                                                   1);          // 1 Vorkommanull
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
            bThousand = !GetBool();     // Param TRUE: keine Tausenderpunkte
        else
            bThousand = TRUE;
        if (nParamCount >= 2)
        {
            fDec = SolarMath::ApproxFloor(GetDouble());
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
                                               bThousand,   // mit Tausenderpunkt
                                               FALSE,       // nicht rot
                                               (USHORT) fDec,// Nachkommastellen
                                               1);          // 1 Vorkommanull
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
            double nVal = SolarMath::ApproxFloor(GetDouble());
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
            double nVal = SolarMath::ApproxFloor(GetDouble());
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
            fAnz = SolarMath::ApproxFloor(GetDouble());
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
            utl::SearchParam sPar(SearchStr, utl::SearchParam::SRCH_REGEXP, FALSE, FALSE, FALSE);
            utl::TextSearch sT( sPar, *ScGlobal::pCharClass );
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
        double fAnz    = SolarMath::ApproxFloor(GetDouble());
        double fAnfang = SolarMath::ApproxFloor(GetDouble());
        const String& rStr = GetString();
        if (fAnfang < 1.0 || fAnz < 0.0 || fAnfang > double(STRING_MAXLEN) || fAnz > double(STRING_MAXLEN))
            SetIllegalParameter();
        else
            PushString(rStr.Copy( (xub_StrLen) fAnfang - 1, (xub_StrLen) fAnz ));
    }
}


void ScInterpreter::ScText()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        String sFormatString = GetString();
        double fVal = GetDouble();
        String aStr;
        Color* pColor = NULL;
        LanguageType eCellLang;
        const ScPatternAttr* pPattern = pDok->GetPattern(
            aPos.Col(), aPos.Row(), aPos.Tab() );
        if ( pPattern )
            eCellLang = ((const SvxLanguageItem&)
                pPattern->GetItem( ATTR_LANGUAGE_FORMAT )).GetValue();
        else
            eCellLang = ScGlobal::eLnge;
        if ( !pFormatter->GetPreviewStringGuess( sFormatString, fVal, aStr,
                &pColor, eCellLang ) )
            SetIllegalParameter();
        else
            PushString(aStr);
    }
}


void ScInterpreter::ScSubstitute()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 3, 4 ) )
    {
        xub_StrLen nAnz;
        if (nParamCount == 4)
        {
            double fAnz = SolarMath::ApproxFloor(GetDouble());
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
        double fAnz = SolarMath::ApproxFloor(GetDouble());
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


void ScInterpreter::ScErrorType()
{
    USHORT nErr;
    USHORT nOldError = nGlobalError;
    nGlobalError = 0;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        {
            ScRange aRange;
            PopDoubleRef( aRange );
            if ( nGlobalError )
                nErr = nGlobalError;
            else
            {
                ScAddress aAdr;
                if ( DoubleRefToPosSingleRef( aRange, aAdr ) )
                    nErr = pDok->GetErrCode( aAdr );
                else
                    nErr = nGlobalError;
            }
        }
        break;
        case svSingleRef :
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            if ( nGlobalError )
                nErr = nGlobalError;
            else
                nErr = pDok->GetErrCode( aAdr );
        }
        break;
        default:
            PopError();
            nErr = nGlobalError;
    }
    if ( nErr )
    {
        nGlobalError = 0;
        PushDouble( nErr );
    }
    else
    {
        nGlobalError = nOldError;
        SetNV();
    }
}


BOOL ScInterpreter::MayBeRegExp( const String& rStr )
{
    if ( !rStr.Len() || (rStr.Len() == 1 && rStr.GetChar(0) != '.') )
        return FALSE;   // einzelnes Metazeichen kann keine RegExp sein
    static const sal_Unicode cre[] = { '.','*','+','?','[',']','^','$','\\','<','>','(',')','|', 0 };
    const sal_Unicode* p1 = rStr.GetBuffer();
    sal_Unicode c1;
    while ( c1 = *p1++ )
    {
        const sal_Unicode* p2 = cre;
        while ( *p2 )
        {
            if ( c1 == *p2++ )
                return TRUE;
        }
    }
    return FALSE;
}

