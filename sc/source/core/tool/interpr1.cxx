/*************************************************************************
 *
 *  $RCSfile: interpr1.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:36:24 $
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
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif

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
#include "jumpmatrix.hxx"


#define SC_DOUBLE_MAXVALUE  1.7e307

IMPL_FIXEDMEMPOOL_NEWDEL( ScTokenStack, 8, 4 )
IMPL_FIXEDMEMPOOL_NEWDEL( ScErrorStack, 8, 4 )
IMPL_FIXEDMEMPOOL_NEWDEL( ScInterpreter, 32, 16 )

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
    MatrixDoubleRefToMatrix();
    switch ( GetStackType() )
    {
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( !pMat )
                SetIllegalArgument();
            else
            {
                // DoubleError handled by JumpMatrix
                pMat->SetErrorInterpreter( NULL);
                SCSIZE nCols, nRows;
                pMat->GetDimensions( nCols, nRows );
                if ( nCols == 0 || nRows == 0 )
                    SetIllegalParameter();
                else
                {
                    ScJumpMatrix* pJumpMat = new ScJumpMatrix( nCols, nRows );
                    for ( SCSIZE nC=0; nC < nCols; ++nC )
                    {
                        for ( SCSIZE nR=0; nR < nRows; ++nR )
                        {
                            double fVal;
                            bool bTrue;
                            BOOL bIsValue;
                            const MatValue* pMatVal = pMat->Get( nC, nR,
                                    bIsValue);      // bIsValue used as bString
                            bIsValue = !bIsValue;
                            if ( bIsValue )
                            {
                                fVal = pMatVal->fVal;
                                bIsValue = ::rtl::math::isFinite( fVal );
                                bTrue = bIsValue && (fVal != 0.0);
                                if ( bTrue )
                                    fVal = 1.0;
                            }
                            else
                            {
                                bTrue = false;
                                fVal = 0.0;
                            }
                            if ( bTrue )
                            {   // TRUE
                                if( nJumpCount >= 2 )
                                {   // THEN path
                                    pJumpMat->SetJump( nC, nR, fVal,
                                            pJump[ 1 ],
                                            pJump[ nJumpCount ]);
                                }
                                else
                                {   // no parameter given for THEN
                                    pJumpMat->SetJump( nC, nR, fVal,
                                            pJump[ nJumpCount ],
                                            pJump[ nJumpCount ]);
                                }
                            }
                            else
                            {   // FALSE
                                if( nJumpCount == 3 && bIsValue )
                                {   // ELSE path
                                    pJumpMat->SetJump( nC, nR, fVal,
                                            pJump[ 2 ],
                                            pJump[ nJumpCount ]);
                                }
                                else
                                {   // no parameter given for ELSE,
                                    // or DoubleError
                                    pJumpMat->SetJump( nC, nR, fVal,
                                            pJump[ nJumpCount ],
                                            pJump[ nJumpCount ]);
                                }
                            }
                        }
                    }
                    PushTempToken( new ScJumpMatrixToken( pJumpMat ) );
                    // set endpoint of path for main code line
                    aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
                }
            }
        }
        break;
        default:
        {
            if ( GetBool() )
            {   // TRUE
                if( nJumpCount >= 2 )
                {   // THEN path
                    nFuncFmtType = NUMBERFORMAT_UNDEFINED;
                    aCode.Jump( pJump[ 1 ], pJump[ nJumpCount ] );
                }
                else
                {   // no parameter given for THEN
                    nFuncFmtType = NUMBERFORMAT_LOGICAL;
                    PushInt(1);
                    aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
                }
            }
            else
            {   // FALSE
                if( nJumpCount == 3 )
                {   // ELSE path
                    nFuncFmtType = NUMBERFORMAT_UNDEFINED;
                    aCode.Jump( pJump[ 2 ], pJump[ nJumpCount ] );
                }
                else
                {   // no parameter given for ELSE
                    nFuncFmtType = NUMBERFORMAT_LOGICAL;
                    PushInt(0);
                    aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
                }
            }
        }
    }
}


void ScInterpreter::ScChoseJump()
{
    const short* pJump = pCur->GetJump();
    short nJumpCount = pJump[ 0 ];
    MatrixDoubleRefToMatrix();
    switch ( GetStackType() )
    {
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( !pMat )
                SetIllegalArgument();
            else
            {
                // DoubleError handled by JumpMatrix
                pMat->SetErrorInterpreter( NULL);
                SCSIZE nCols, nRows;
                pMat->GetDimensions( nCols, nRows );
                if ( nCols == 0 || nRows == 0 )
                    SetIllegalParameter();
                else
                {
                    ScJumpMatrix* pJumpMat = new ScJumpMatrix( nCols, nRows );
                    for ( SCSIZE nC=0; nC < nCols; ++nC )
                    {
                        for ( SCSIZE nR=0; nR < nRows; ++nR )
                        {
                            double fVal;
                            BOOL bIsValue;
                            const MatValue* pMatVal = pMat->Get( nC, nR,
                                    bIsValue);      // bIsValue used as bString
                            bIsValue = !bIsValue;
                            if ( bIsValue )
                            {
                                fVal = pMatVal->fVal;
                                bIsValue = ::rtl::math::isFinite( fVal );
                                if ( bIsValue )
                                {
                                    fVal = ::rtl::math::approxFloor( fVal);
                                    if ( (fVal < 1) || (fVal >= nJumpCount))
                                    {
                                        bIsValue = FALSE;
                                        fVal = CreateDoubleError(
                                                errIllegalArgument);
                                    }
                                }
                            }
                            else
                            {
                                fVal = CreateDoubleError( errNoValue);
                            }
                            if ( bIsValue )
                            {
                                pJumpMat->SetJump( nC, nR, fVal,
                                        pJump[ (short)fVal ],
                                        pJump[ nJumpCount ]);
                            }
                            else
                            {
                                pJumpMat->SetJump( nC, nR, fVal,
                                        pJump[ nJumpCount ],
                                        pJump[ nJumpCount ]);
                            }
                        }
                    }
                    PushTempToken( new ScJumpMatrixToken( pJumpMat ) );
                    // set endpoint of path for main code line
                    aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
                }
            }
        }
        break;
        default:
        {
            double nJumpIndex = ::rtl::math::approxFloor( GetDouble() );
            if ((nJumpIndex >= 1) && (nJumpIndex < nJumpCount))
                aCode.Jump( pJump[ (short) nJumpIndex ], pJump[ nJumpCount ] );
            else
                SetError(errIllegalArgument);
        }
    }
}


bool ScInterpreter::JumpMatrix( short nStackLevel )
{
    pJumpMatrix = pStack[sp-nStackLevel]->GetJumpMatrix();
    ScMatrixRef pResMat = pJumpMatrix->GetResultMatrix();
    SCSIZE nC, nR;
    if ( nStackLevel == 2 )
    {
        if ( aCode.HasStacked() )
            aCode.Pop();    // pop what Jump() pushed
        else
            DBG_ERRORFILE( "ScInterpreter::JumpMatrix: pop goes the weasel" );

        if ( !pResMat )
        {
            Pop();
            SetError( errUnknownStackVariable );
        }
        else
        {
            pJumpMatrix->GetPos( nC, nR );
            switch ( GetStackType() )
            {
                case svDouble:
                {
                    double fVal = GetDouble();
                    if ( nGlobalError )
                    {
                        fVal = CreateDoubleError( nGlobalError );
                        nGlobalError = 0;
                    }
                    pResMat->PutDouble( fVal, nC, nR );
                }
                break;
                case svString:
                {
                    const String& rStr = GetString();
                    if ( nGlobalError )
                    {
                        pResMat->PutDouble( CreateDoubleError( nGlobalError),
                                nC, nR);
                        nGlobalError = 0;
                    }
                    else
                        pResMat->PutString( rStr, nC, nR );
                }
                break;
                case svSingleRef:
                {
                    ScAddress aAdr;
                    PopSingleRef( aAdr );
                    if ( nGlobalError )
                    {
                        pResMat->PutDouble( CreateDoubleError( nGlobalError),
                                nC, nR);
                        nGlobalError = 0;
                    }
                    else
                    {
                        ScBaseCell* pCell = GetCell( aAdr );
                        switch ( GetCellType( pCell ) )
                        {
                            case CELLTYPE_NONE:
                            case CELLTYPE_NOTE:
                                pResMat->PutEmpty( nC, nR );
                            break;
                            default:
                            {
                                if ( HasCellValueData( pCell ) )
                                {
                                    double fVal = GetCellValue( aAdr, pCell);
                                    if ( nGlobalError )
                                    {
                                        fVal = CreateDoubleError(
                                                nGlobalError);
                                        nGlobalError = 0;
                                    }
                                    pResMat->PutDouble( fVal, nC, nR );
                                }
                                else
                                {
                                    String aStr;
                                    GetCellString( aStr, pCell );
                                    if ( nGlobalError )
                                    {
                                        pResMat->PutDouble( CreateDoubleError(
                                                    nGlobalError), nC, nR);
                                        nGlobalError = 0;
                                    }
                                    else
                                        pResMat->PutString( aStr, nC, nR);
                                }
                            }
                        }
                    }
                }
                break;
                case svDoubleRef:
                {   // upper left plus offset within matrix
                    double fVal;
                    ScRange aRange;
                    PopDoubleRef( aRange );
                    ScAddress& rAdr = aRange.aStart;
                    ULONG nCol = (ULONG)rAdr.Col() + nC;
                    ULONG nRow = (ULONG)rAdr.Row() + nR;
                    if ( nGlobalError )
                    {
                        fVal = CreateDoubleError( nGlobalError );
                        nGlobalError = 0;
                        pResMat->PutDouble( fVal, nC, nR );
                    }
                    else if ( nCol > static_cast<ULONG>(aRange.aEnd.Col()) ||
                            nRow > static_cast<ULONG>(aRange.aEnd.Row()))
                    {
                        fVal = CreateDoubleError( errNoValue );
                        pResMat->PutDouble( fVal, nC, nR );
                    }
                    else
                    {
                        rAdr.SetCol( static_cast<SCCOL>(nCol) );
                        rAdr.SetRow( static_cast<SCROW>(nRow) );
                        ScBaseCell* pCell = GetCell( rAdr );
                        switch ( GetCellType( pCell ) )
                        {
                            case CELLTYPE_NONE:
                            case CELLTYPE_NOTE:
                                pResMat->PutEmpty( nC, nR );
                            break;
                            default:
                            {
                                if ( HasCellValueData( pCell ) )
                                {
                                    double fVal = GetCellValue( rAdr, pCell);
                                    if ( nGlobalError )
                                    {
                                        fVal = CreateDoubleError(
                                                nGlobalError);
                                        nGlobalError = 0;
                                    }
                                    pResMat->PutDouble( fVal, nC, nR );
                                }
                                else
                                {
                                    String aStr;
                                    GetCellString( aStr, pCell );
                                    if ( nGlobalError )
                                    {
                                        pResMat->PutDouble( CreateDoubleError(
                                                    nGlobalError), nC, nR);
                                        nGlobalError = 0;
                                    }
                                    else
                                        pResMat->PutString( aStr, nC, nR );
                                }
                            }
                        }
                    }
                }
                break;
                case svMatrix:
                {   // match matrix offsets
                    double fVal;
                    ScMatrixRef pMat = PopMatrix();
                    if ( nGlobalError )
                    {
                        fVal = CreateDoubleError( nGlobalError );
                        nGlobalError = 0;
                        pResMat->PutDouble( fVal, nC, nR );
                    }
                    else if ( !pMat )
                    {
                        fVal = CreateDoubleError( errUnknownVariable );
                        pResMat->PutDouble( fVal, nC, nR );
                    }
                    else
                    {
                        SCSIZE nCols, nRows;
                        pMat->GetDimensions( nCols, nRows );
                        if ( nCols <= nC || nRows <= nR )
                        {
                            fVal = CreateDoubleError( errNoValue );
                            pResMat->PutDouble( fVal, nC, nR );
                        }
                        else
                        {
                            if ( pMat->IsValue( nC, nR ) )
                            {
                                fVal = pMat->GetDouble( nC, nR );
                                pResMat->PutDouble( fVal, nC, nR );
                            }
                            else if ( pMat->IsEmpty( nC, nR ) )
                                pResMat->PutEmpty( nC, nR );
                            else
                            {
                                const String& rStr = pMat->GetString( nC, nR );
                                pResMat->PutString( rStr, nC, nR );
                            }
                        }
                    }
                }
                break;
                default:
                {
                    Pop();
                    double fVal = CreateDoubleError( errIllegalArgument);
                    pResMat->PutDouble( fVal, nC, nR );
                }
            }
        }
    }
    bool bCont = pJumpMatrix->Next( nC, nR );
    if ( bCont )
    {
        double fBool;
        short nStart, nNext, nStop;
        pJumpMatrix->GetJump( nC, nR, fBool, nStart, nNext, nStop );
        while ( bCont && nStart == nNext )
        {   // push all results that have no jump path
            if ( pResMat )
            {
                // a FALSE without path results in an empty path value
                if ( fBool == 0.0 )
                    pResMat->PutEmptyPath( nC, nR );
                else
                    pResMat->PutDouble( fBool, nC, nR );
            }
            bCont = pJumpMatrix->Next( nC, nR );
            if ( bCont )
                pJumpMatrix->GetJump( nC, nR, fBool, nStart, nNext, nStop );
        }
        if ( bCont && nStart != nNext )
        {
            const ScTokenVec* pParams = pJumpMatrix->GetJumpParameters();
            if ( pParams )
            {
                for ( ScTokenVec::const_iterator i = pParams->begin();
                        i != pParams->end(); ++i )
                {
                    Push( *(*i));
                }
            }
            aCode.Jump( nStart, nNext, nStop );
        }
    }
    if ( !bCont )
    {   // we're done with it, throw away jump matrix, keep result
        pJumpMatrix = NULL;
        Pop();
        PushMatrix( pResMat );
        return true;
    }
    return false;
}


double ScInterpreter::CompareFunc( const ScCompare& rComp )
{
    // Keep DoubleError if encountered
    if ( rComp.bVal[0] && !::rtl::math::isFinite( rComp.nVal[0]))
        return rComp.nVal[0];
    if ( rComp.bVal[1] && !::rtl::math::isFinite( rComp.nVal[1]))
        return rComp.nVal[1];

    double fRes = 0;
    if ( rComp.bEmpty[ 0 ] )
    {
        if ( rComp.bEmpty[ 1 ] )
            ;       // empty cell == empty cell, fRes 0
        else if( rComp.bVal[ 1 ] )
        {
            if ( !::rtl::math::approxEqual( rComp.nVal[ 1 ], 0.0 ) )
            {
                if ( rComp.nVal[ 1 ] < 0.0 )
                    fRes = 1;       // empty cell > -x
                else
                    fRes = -1;      // empty cell < x
            }
            // else: empty cell == 0.0
        }
        else
        {
            if ( rComp.pVal[ 1 ]->Len() )
                fRes = -1;      // empty cell < "..."
            // else: empty cell == ""
        }
    }
    else if ( rComp.bEmpty[ 1 ] )
    {
        if( rComp.bVal[ 0 ] )
        {
            if ( !::rtl::math::approxEqual( rComp.nVal[ 0 ], 0.0 ) )
            {
                if ( rComp.nVal[ 0 ] < 0.0 )
                    fRes = -1;      // -x < empty cell
                else
                    fRes = 1;       // x > empty cell
            }
            // else: empty cell == 0.0
        }
        else
        {
            if ( rComp.pVal[ 0 ]->Len() )
                fRes = 1;       // "..." > empty cell
            // else: "" == empty cell
        }
    }
    else if( rComp.bVal[ 0 ] )
    {
        if( rComp.bVal[ 1 ] )
        {
            if ( !::rtl::math::approxEqual( rComp.nVal[ 0 ], rComp.nVal[ 1 ] ) )
            {
                if( rComp.nVal[ 0 ] - rComp.nVal[ 1 ] < 0 )
                    fRes = -1;
                else
                    fRes = 1;
            }
        }
        else
            fRes = -1;  // number is less than string
    }
    else if( rComp.bVal[ 1 ] )
        fRes = 1;   // number is less than string
    else
    {
        if (pDok->GetDocOptions().IsIgnoreCase())
            fRes = (double) ScGlobal::pCollator->compareString(
                *rComp.pVal[ 0 ], *rComp.pVal[ 1 ] );
        else
            fRes = (double) ScGlobal::pCaseCollator->compareString(
                *rComp.pVal[ 0 ], *rComp.pVal[ 1 ] );
    }
    return fRes;
}


double ScInterpreter::Compare()
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


ScMatrixRef ScInterpreter::CompareMat()
{
    nCurFmtType = nFuncFmtType = NUMBERFORMAT_LOGICAL;
    String aVal1, aVal2;
    ScCompare aComp( &aVal1, &aVal2 );
    ScMatrixRef pMat[2];
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
            case svMatrix:
                pMat[ i ] = GetMatrix();
                if ( !pMat[ i ] )
                    SetError(errIllegalParameter);
                else
                    pMat[i]->SetErrorInterpreter( NULL);
                    // errors are transported as DoubleError inside matrix
                break;
            default:
                SetError(errIllegalParameter);
            break;
        }
    }
    ScMatrixRef pResMat = NULL;
    if( !nGlobalError )
    {
        if ( pMat[0] && pMat[1] )
        {
            SCSIZE nC0, nC1;
            SCSIZE nR0, nR1;
            pMat[0]->GetDimensions( nC0, nR0 );
            pMat[1]->GetDimensions( nC1, nR1 );
            SCSIZE nC = Max( nC0, nC1 );
            SCSIZE nR = Max( nR0, nR1 );
            pResMat = GetNewMat( nC, nR);
            if ( !pResMat )
                return NULL;
            for ( SCSIZE j=0; j<nC; j++ )
            {
                for ( SCSIZE k=0; k<nR; k++ )
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
        }
        else if ( pMat[0] || pMat[1] )
        {
            short i = ( pMat[0] ? 0 : 1);
            SCSIZE nC, nR;
            pMat[i]->GetDimensions( nC, nR );
            pResMat = GetNewMat( nC, nR);
            if ( !pResMat )
                return NULL;
            SCSIZE n = nC * nR;
            for ( SCSIZE j=0; j<n; j++ )
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
        }
    }
    return pResMat;
}


void ScInterpreter::ScEqual()
{
    if ( GetStackType(1) == svMatrix || GetStackType(2) == svMatrix )
    {
        ScMatrixRef pMat = CompareMat();
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
    if ( GetStackType(1) == svMatrix || GetStackType(2) == svMatrix )
    {
        ScMatrixRef pMat = CompareMat();
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
    if ( GetStackType(1) == svMatrix || GetStackType(2) == svMatrix )
    {
        ScMatrixRef pMat = CompareMat();
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
    if ( GetStackType(1) == svMatrix || GetStackType(2) == svMatrix )
    {
        ScMatrixRef pMat = CompareMat();
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
    if ( GetStackType(1) == svMatrix || GetStackType(2) == svMatrix )
    {
        ScMatrixRef pMat = CompareMat();
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
    if ( GetStackType(1) == svMatrix || GetStackType(2) == svMatrix )
    {
        ScMatrixRef pMat = CompareMat();
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
                        ScMatrixRef pMat = GetMatrix();
                        if ( pMat )
                        {
                            bHaveValue = TRUE;
                            double fVal = pMat->And();
                            USHORT nErr = GetDoubleErrorValue( fVal );
                            if ( nErr )
                            {
                                SetError( nErr );
                                nRes = FALSE;
                            }
                            else
                                nRes &= (fVal != 0.0);
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
                        ScMatrixRef pMat = GetMatrix();
                        if ( pMat )
                        {
                            bHaveValue = TRUE;
                            double fVal = pMat->Or();
                            USHORT nErr = GetDoubleErrorValue( fVal );
                            if ( nErr )
                            {
                                SetError( nErr );
                                nRes = FALSE;
                            }
                            else
                                nRes |= (fVal != 0.0);
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
    switch ( GetStackType() )
    {
        case svMatrix :
        {
            ScMatrixRef pMat = GetMatrix();
            if ( !pMat )
                PushError();
            else
            {
                SCSIZE nC, nR;
                pMat->GetDimensions( nC, nR );
                ScMatrixRef pResMat = GetNewMat( nC, nR);
                if ( !pResMat )
                    PushError();
                else
                {
                    SCSIZE nCount = nC * nR;
                    for ( SCSIZE j=0; j<nCount; ++j )
                    {
                        if ( pMat->IsValueOrEmpty(j) )
                            pResMat->PutDouble( -pMat->GetDouble(j), j );
                        else
                            pResMat->PutString(
                                ScGlobal::GetRscString( STR_NO_VALUE ), j );
                    }
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
    switch ( GetStackType() )
    {
        case svMatrix :
        {
            ScMatrixRef pMat = GetMatrix();
            if ( !pMat )
                PushError();
            else
            {
                SCSIZE nC, nR;
                pMat->GetDimensions( nC, nR );
                ScMatrixRef pResMat = GetNewMat( nC, nR);
                if ( !pResMat )
                    PushError();
                else
                {
                    SCSIZE nCount = nC * nR;
                    for ( SCSIZE j=0; j<nCount; ++j )
                    {
                        if ( pMat->IsValueOrEmpty(j) )
                            pResMat->PutDouble( (pMat->GetDouble(j) == 0.0), j );
                        else
                            pResMat->PutString(
                                ScGlobal::GetRscString( STR_NO_VALUE ), j );
                    }
                    PushMatrix( pResMat );
                }
            }
        }
        break;
        default:
            PushInt( GetDouble() == 0.0 );
    }
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
    PushDouble(::rtl::math::sin(GetDouble()));
}


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
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( !pMat )
                ;   // nothing
            else if ( !pJumpMatrix )
                nRes = pMat->IsEmpty( 0 );
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    nRes = pMat->IsEmpty( nC, nR);
                // else: FALSE, not empty (which is what Xcl does)
            }
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
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( !pMat )
                ;   // nothing
            else if ( !pJumpMatrix )
                nRes = pMat->IsString(0) && !pMat->IsEmpty(0);
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    nRes = pMat->IsString( nC, nR) && !pMat->IsEmpty( nC, nR);
            }
        }
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
        case svMatrix:
            // TODO: we don't have type information for arrays except
            // numerical/string.
        // Fall thru
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
        case svMatrix:
            PopMatrix();
            if ( nGlobalError )
            {
                nType = 16;
                nGlobalError = 0;
            }
            else
                nType = 64;
                // we could return the type of one element if in JumpMatrix or
                // ForceArray mode, but Xcl doesn't ...
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
                SCTAB nTab = aCellPos.Tab();
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
                // Yes, passing tab as col is intentional!
                ScAddress( static_cast<SCCOL>(aCellPos.Tab()), 0, 0 ).Format( aResult, (SCA_COL_ABSOLUTE|SCA_VALID_COL) );
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
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( !pMat )
                ;   // nothing
            else if ( !pJumpMatrix )
                nRes = pMat->IsValue( 0 );
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    nRes = pMat->IsValue( nC, nR);
            }
        }
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
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( !pMat )
                ;   // nothing
            else if ( !pJumpMatrix )
                nRes = (GetDoubleErrorValue( pMat->GetDouble( 0 )) == NOVALUE);
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    nRes = (GetDoubleErrorValue( pMat->GetDouble( nC, nR)) == NOVALUE);
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
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( nGlobalError || !pMat )
                nRes = ((nGlobalError && nGlobalError != NOVALUE) || !pMat);
            else if ( !pJumpMatrix )
            {
                USHORT nErr = GetDoubleErrorValue( pMat->GetDouble( 0 ));
                nRes = (nErr && nErr != NOVALUE);
            }
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                {
                    USHORT nErr = GetDoubleErrorValue( pMat->GetDouble( nC, nR));
                    nRes = (nErr && nErr != NOVALUE);
                }
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
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( nGlobalError || !pMat )
                nRes = 1;
            else if ( !pJumpMatrix )
                nRes = (GetDoubleErrorValue( pMat->GetDouble( 0 )) != 0);
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    nRes = (GetDoubleErrorValue( pMat->GetDouble( nC, nR)) != 0);
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
                    break;
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
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( !pMat )
                ;   // nothing
            else if ( !pJumpMatrix )
            {
                nRes = pMat->IsValue( 0 );
                if ( nRes )
                    fVal = pMat->GetDouble( 0 );
            }
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                {
                    nRes = pMat->IsValue( nC, nR);
                    if ( nRes )
                        fVal = pMat->GetDouble( nC, nR);
                }
                else
                    SetError( errNoValue);
            }
        }
        break;
    }
    if ( !nRes )
        SetError( errIllegalParameter);
    else
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
                ScMatrixRef pMat = PopMatrix();
                if (pMat)
                {
                    SCSIZE nC, nR;
                    nFuncFmtType = NUMBERFORMAT_NUMBER;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (SCSIZE i = 0; i < nC; i++)
                            for (SCSIZE j = 0; j < nR; j++)
                            {
                                nVal = pMat->GetDouble(i,j);
                                if (nMin > nVal) nMin = nVal;
                            }
                    }
                    else
                    {
                        for (SCSIZE i = 0; i < nC; i++)
                        {
                            for (SCSIZE j = 0; j < nR; j++)
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
                ScMatrixRef pMat = PopMatrix();
                if (pMat)
                {
                    nFuncFmtType = NUMBERFORMAT_NUMBER;
                    SCSIZE nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (SCSIZE i = 0; i < nC; i++)
                            for (SCSIZE j = 0; j < nR; j++)
                            {
                                nVal = pMat->GetDouble(i,j);
                                if (nMax < nVal) nMax = nVal;
                            }
                    }
                    else
                    {
                        for (SCSIZE i = 0; i < nC; i++)
                        {
                            for (SCSIZE j = 0; j < nR; j++)
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
                ScMatrixRef pMat = PopMatrix();
                if (pMat)
                {
                    SCSIZE nC, nR;
                    nFuncFmtType = NUMBERFORMAT_NUMBER;
                    pMat->GetDimensions(nC, nR);
                    if( eFunc == ifCOUNT2 )
                        nCount += (ULONG) nC * nR;
                    else
                    {
                        for (SCSIZE i = 0; i < nC; i++)
                        {
                            for (SCSIZE j = 0; j < nR; j++)
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
        case ifSUM:     fRes = ::rtl::math::approxAdd( fRes, fMem ); break;
        case ifAVERAGE: fRes = ::rtl::math::approxAdd( fRes, fMem ) / nCount; break;
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
                ScMatrixRef pMat = PopMatrix();
                if (pMat)
                {
                    SCSIZE nC, nR;
                    pMat->GetDimensions(nC, nR);
                    for (SCSIZE i = 0; i < nC; i++)
                    {
                        for (SCSIZE j = 0; j < nR; j++)
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
    rVal = ::rtl::math::approxSub( fSumSqr, fSum*fSum/rValCount );
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
    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
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
                nVal += static_cast<ULONG>(nTab2 - nTab1 + 1) *
                    static_cast<ULONG>(nCol2 - nCol1 + 1);
                break;
            case svMatrix:
            {
                ScMatrixRef pMat = PopMatrix();
                if (pMat)
                {
                    SCSIZE nC, nR;
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
    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
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
                nVal += static_cast<ULONG>(nTab2 - nTab1 + 1) *
                    static_cast<ULONG>(nRow2 - nRow1 + 1);
                break;
            case svMatrix:
            {
                ScMatrixRef pMat = PopMatrix();
                if (pMat)
                {
                    SCSIZE nC, nR;
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
        SCCOL nCol1;
        SCROW nRow1;
        SCTAB nTab1;
        SCCOL nCol2;
        SCROW nRow2;
        SCTAB nTab2;
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
                    nVal += static_cast<ULONG>(nTab2 - nTab1 + 1);
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
                    SCCOL nCol1;
                    SCROW nRow1;
                    SCTAB nTab1;
                    PopSingleRef( nCol1, nRow1, nTab1 );
                    nVal = (double) (nCol1 + 1);
                }
                break;
                case svDoubleRef :
                {
                    SCCOL nCol1;
                    SCROW nRow1;
                    SCTAB nTab1;
                    SCCOL nCol2;
                    SCROW nRow2;
                    SCTAB nTab2;
                    PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                    if (nCol2 > nCol1)
                    {
                        ScMatrixRef pResMat = GetNewMat(
                                static_cast<SCSIZE>(nCol2-nCol1+1), 1);
                        if (pResMat)
                        {
                            for (SCCOL i = nCol1; i <= nCol2; i++)
                                pResMat->PutDouble((double)(i+1),
                                        static_cast<SCSIZE>(i-nCol1), 0);
                            PushMatrix(pResMat);
                            return;
                        }
                        else
                            nVal = 0.0;
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
                    SCCOL nCol1;
                    SCROW nRow1;
                    SCTAB nTab1;
                    PopSingleRef( nCol1, nRow1, nTab1 );
                    nVal = (double) (nRow1 + 1);
                }
                break;
                case svDoubleRef :
                {
                    SCCOL nCol1;
                    SCROW nRow1;
                    SCTAB nTab1;
                    SCCOL nCol2;
                    SCROW nRow2;
                    SCTAB nTab2;
                    PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                    if (nRow2 > nRow1)
                    {
                        ScMatrixRef pResMat = GetNewMat( 1,
                                static_cast<SCSIZE>(nRow2-nRow1+1));
                        if (pResMat)
                        {
                            for (SCROW i = nRow1; i <= nRow2; i++)
                                pResMat->PutDouble((double)(i+1), 0,
                                        static_cast<SCSIZE>(i-nRow1));
                            PushMatrix(pResMat);
                            return;
                        }
                        else
                            nVal = 0.0;
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
        SCTAB nVal;
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
                    SCCOL nCol1;
                    SCROW nRow1;
                    SCTAB nTab1;
                    PopSingleRef( nCol1, nRow1, nTab1 );
                    nVal = nTab1 + 1;
                }
                break;
                case svDoubleRef :
                {
                    SCCOL nCol1;
                    SCROW nRow1;
                    SCTAB nTab1;
                    SCCOL nCol2;
                    SCROW nRow2;
                    SCTAB nTab2;
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
        SCCOL nCol1;
        SCROW nRow1;
        SCTAB nTab1;
        SCCOL nCol2;
        SCROW nRow2;
        SCTAB nTab2;
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
            rParam.bRegExp     = pDok->GetDocOptions().IsFormulaRegexEnabled();
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
                rParam.bRegExp = MayBeRegExp( *rEntry.pStr, pDok );
            SCCOLROW nDelta;
            SCROW nR;
            SCCOL nC;
            if (nCol1 == nCol2)
            {                                           // search row in column
                rParam.nRow2 = nRow2;
                rEntry.nField = nCol1;
                ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
                if (fTyp == 0.0)
                {                                       // EQUAL
                    if (aCellIter.GetFirst())
                        nR = aCellIter.GetRow();
                    else
                    {
                        SetNV();
                        return;
                    }
                }
                else
                {                                       // <= or >=
                    aCellIter.SetStopOnMismatch( TRUE );
                    if ( aCellIter.GetFirst() )
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
                }
                nDelta = nR - nRow1;
            }
            else
            {                                           // search column in row
                rParam.nRow2 = nRow1;
                rEntry.nField = nCol1;
                ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
                // Advance Entry.nField in Iterator if column changed
                aCellIter.SetAdvanceQueryParamEntryField( TRUE );
                if (fTyp == 0.0)
                {                                       // EQUAL
                    if ( aCellIter.GetFirst() )
                        nC = aCellIter.GetCol();
                    else
                    {
                        SetNV();
                        return;
                    }
                }
                else
                {                                       // <= or >=
                    aCellIter.SetStopOnMismatch( TRUE );
                    if ( aCellIter.GetFirst() )
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
        ULONG nMaxCount = 0, nCount = 0;
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
                SCCOL nCol1;
                SCROW nRow1;
                SCTAB nTab1;
                SCCOL nCol2;
                SCROW nRow2;
                SCTAB nTab2;
                PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                nMaxCount = static_cast<ULONG>(nRow2 - nRow1 + 1) *
                    static_cast<ULONG>(nCol2 - nCol1 + 1) *
                    static_cast<ULONG>(nTab2 - nTab1 + 1);
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
        SCCOL nCol1;
        SCROW nRow1;
        SCTAB nTab1;
        SCCOL nCol2;
        SCROW nRow2;
        SCTAB nTab2;
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
                rParam.FillInExcelSyntax(rString, 0);
                ULONG nIndex = 0;
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
        SCCOL nCol3;
        SCROW nRow3;
        SCTAB nTab3;
        SCCOL nCol4;
        SCROW nRow4;
        SCTAB nTab4;
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
        SCCOL nCol1;
        SCROW nRow1;
        SCTAB nTab1;
        SCCOL nCol2;
        SCROW nRow2;
        SCTAB nTab2;
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
                rParam.FillInExcelSyntax(rString, 0);
                ULONG nIndex = 0;
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
            long nColDiff = nCol3 - nCol1;
            long nRowDiff = nRow3 - nRow1;
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
    SCSIZE nC3, nC1;
    SCSIZE nR3, nR1;
    ScMatrixRef pMat3 = NULL;
    ScMatrixRef pMat1 = NULL;
    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
    SCCOL nCol3;
    SCROW nRow3;
    SCTAB nTab3;
    SCCOL nCol4;
    SCROW nRow4;
    SCTAB nTab4;
    SCSIZE nDelta;

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
    SCSIZE nMatCount, nVecCount;
    if (pMat1 == NULL)
    {
        if (nRow1 == nRow2)
        {
            nMatCount = static_cast<SCSIZE>(nCol2 - nCol1 + 1);
            bSpMatrix = FALSE;
        }
        else
        {
            nMatCount = static_cast<SCSIZE>(nRow2 - nRow1 + 1);
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
            nVecCount = static_cast<SCSIZE>(nCol4 - nCol3 + 1);
            bSpVector = FALSE;
        }
        else
        {
            nVecCount = static_cast<SCSIZE>(nRow4 - nRow3 + 1);
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
                SCSIZE i;
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
                SCSIZE i;
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
            SCCOL nC;
            SCROW nR;
            if ( aCellIter.FindEqualOrSortedLastInRange( nC, nR ) )
                nDelta = static_cast<SCSIZE>(nR - nRow1);
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
            SCCOL nC;
            SCROW nR;
            if ( aCellIter.FindEqualOrSortedLastInRange( nC, nR ) )
                nDelta = static_cast<SCSIZE>(nC - nCol1);
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
        ScMatrixRef pMat = NULL;
        SCSIZE nC, nR;
        SCCOL nCol1;
        SCROW nRow1;
        SCTAB nTab1;
        SCCOL nCol2;
        SCROW nRow2;
        SCTAB nTab2;
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
        SCROW nZIndex = static_cast<SCROW>(fIndex);
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
                SCSIZE nMatCount = nC;
                SCSIZE nDelta = SCSIZE_MAX;
                if (rEntry.bQueryByString)
                {
//!!!!!!!
//! TODO: enable regex on matrix strings
//!!!!!!!
                    String aParamStr = *rEntry.pStr;
                    SCSIZE i;
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
                    SCSIZE i;
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
                if ( nDelta != SCSIZE_MAX )
                {
                    if (!pMat->IsString( nDelta, static_cast<SCSIZE>(nZIndex)))
                        PushDouble(pMat->GetDouble( nDelta,
                                    static_cast<SCSIZE>(nZIndex)));
                    else
                        PushString(pMat->GetString( nDelta,
                                    static_cast<SCSIZE>(nZIndex)));
                }
                else
                    SetNV();
            }
            else
            {
                rEntry.nField = nCol1;
                BOOL bFound = FALSE;
                SCCOL nC;
                if ( bSorted )
                    rEntry.eOp = SC_LESS_EQUAL;
                ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
                // advance Entry.nField in Iterator upon switching columns
                aCellIter.SetAdvanceQueryParamEntryField( TRUE );
                if ( bSorted )
                {
                    SCROW nR;
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
        double fIndex = ::rtl::math::approxFloor( GetDouble() ) - 1.0;
        ScMatrixRef pMat = NULL;
        SCSIZE nC, nR;
        SCCOL nCol1;
        SCROW nRow1;
        SCTAB nTab1;
        SCCOL nCol2;
        SCROW nRow2;
        SCTAB nTab2;
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
        SCCOL nSpIndex = static_cast<SCCOL>(fIndex);
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
                SCSIZE nMatCount = nR;
                SCSIZE nDelta = SCSIZE_MAX;
                if (rEntry.bQueryByString)
                {
//!!!!!!!
//! TODO: enable regex on matrix strings
//!!!!!!!
                    String aParamStr = *rEntry.pStr;
                    SCSIZE i;
                    if ( bSorted )
                    {
                        for (i = 0; i < nMatCount; i++)
                        {
                            if (pMat->IsString(0, i))
                            {
                                if ( ScGlobal::pTransliteration->isEqual(
                                        pMat->GetString(0,i), aParamStr ) )
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
                                if ( ScGlobal::pTransliteration->isEqual(
                                        pMat->GetString(0,i), aParamStr ) )
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
                    SCSIZE i;
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
                if ( nDelta != SCSIZE_MAX )
                {
                    if (!pMat->IsString( static_cast<SCSIZE>(nSpIndex), nDelta))
                        PushDouble( pMat->GetDouble(
                                    static_cast<SCSIZE>(nSpIndex), nDelta));
                    else
                        PushString( pMat->GetString(
                                    static_cast<SCSIZE>(nSpIndex), nDelta));
                }
                else
                    SetNV();
            }
            else
            {
                rEntry.nField = nCol1;
                BOOL bFound = FALSE;
                SCROW nR;
                if ( bSorted )
                    rEntry.eOp = SC_LESS_EQUAL;
                ScQueryCellIterator aCellIter(pDok, nTab1, rParam, FALSE);
                if ( bSorted )
                {
                    SCCOL nC;
                    bFound = aCellIter.FindEqualOrSortedLastInRange( nC, nR );
                }
                else if ( aCellIter.GetFirst() )
                {
                    bFound = TRUE;
                    nR = aCellIter.GetRow();
                    if ( bSorted )
                    {
                        while (aCellIter.GetNext())
                            nR = aCellIter.GetRow();
                    }
                }
                if ( bFound )
                {
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
        int nFunc = (int) ::rtl::math::approxFloor( GetDouble() );
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


BOOL ScInterpreter::GetDBParams(SCTAB& rTab, ScQueryParam& rParam,
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

        SCCOL nQCol1;
        SCROW nQRow1;
        SCTAB nQTab1;
        SCCOL nQCol2;
        SCROW nQRow2;
        SCTAB nQTab2;
        PopDoubleRef(nQCol1, nQRow1, nQTab1, nQCol2, nQRow2, nQTab2);

        BOOL    bByVal = TRUE;
        double  nVal;
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

        SCCOL nDBCol1;
        SCROW nDBRow1;
        SCTAB nDBTab1;
        SCCOL nDBCol2;
        SCROW nDBRow2;
        SCTAB nDBTab2;
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
            SCCOL   nField = nDBCol1;
            BOOL    bFound = TRUE;
            if ( rMissingField )
                ;   // special case
            else if ( bByVal )
            {
                if ( nVal <= 0 || nVal > (nDBCol2 - nDBCol1 + 1) )
                    bFound = FALSE;
                else
                    nField = Min(nDBCol2, (SCCOL)(nDBCol1 + (SCCOL)nVal - 1));
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
                        nField = static_cast<SCCOL>(rParam.GetEntry(0).nField);

                    rParam.nCol1 = nField;
                    rParam.nCol2 = nField;
                    rTab = nDBTab1;
                    bRet = TRUE;
                    SCSIZE nCount = rParam.GetEntryCount();
                    for ( SCSIZE i=0; i < nCount; i++ )
                    {
                        ScQueryEntry& rEntry = rParam.GetEntry(i);
                        if ( rEntry.bDoQuery )
                        {
                            ULONG nIndex = 0;
                            rEntry.bQueryByString = !pFormatter->IsNumberFormat(
                                *rEntry.pStr, nIndex, rEntry.nVal );
                            if ( rEntry.bQueryByString && !rParam.bRegExp )
                                rParam.bRegExp = MayBeRegExp( *rEntry.pStr, pDok );
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
    SCTAB nTab1;
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
        case ifSUM:     nErg = ::rtl::math::approxAdd( nErg, fMem ); break;
        case ifAVERAGE: nErg = ::rtl::math::approxAdd( nErg, fMem ) / nCount; break;
    }
    PushDouble( nErg );
}


void ScInterpreter::ScDBSum()
{
    DBIterator( ifSUM );
}


void ScInterpreter::ScDBCount()
{
    SCTAB nTab;
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
    SCTAB nTab;
    ScQueryParam aQueryParam;
    BOOL bMissingField = TRUE;
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
    SCTAB nTab;
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
        SCTAB nTab = aPos.Tab();
        String sRefStr( GetString() );
        ScRefAddress aRefAd, aRefAd2;
        if ( ConvertDoubleRef( pDok, sRefStr, nTab, aRefAd, aRefAd2 ) )
            PushDoubleRef( aRefAd.Col(), aRefAd.Row(), aRefAd.Tab(),
                aRefAd2.Col(), aRefAd2.Row(), aRefAd2.Tab() );
        else if ( ConvertSingleRef( pDok, sRefStr, nTab, aRefAd ) )
            PushSingleRef( aRefAd.Col(), aRefAd.Row(), aRefAd.Tab() );
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
        SCCOL nCol = (SCCOL) ::rtl::math::approxFloor(GetDouble());
        SCROW nRow = (SCROW) ::rtl::math::approxFloor(GetDouble());
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
        ScAddress aAdr( nCol, nRow, 0);
        if (nAbs == 4)
            aRefStr = aAdr.GetColRowString();
        else
        {
            aRefStr = aAdr.GetColRowString(TRUE);
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
        long nColNew, nRowNew, nColPlus, nRowPlus;
        if (nParamCount == 5)
            nColNew = (long) ::rtl::math::approxFloor(GetDouble());
        if (nParamCount >= 4)
            nRowNew = (long) ::rtl::math::approxFloor(GetDouble());
        nColPlus = (long) ::rtl::math::approxFloor(GetDouble());
        nRowPlus = (long) ::rtl::math::approxFloor(GetDouble());
        SCCOL nCol1;
        SCROW nRow1;
        SCTAB nTab1;
        SCCOL nCol2;
        SCROW nRow2;
        SCTAB nTab2;
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
                nCol1 = (SCCOL)((long) nCol1 + nColPlus);
                nRow1 = (SCROW)((long) nRow1 + nRowPlus);
                if (!ValidCol(nCol1) || !ValidRow(nRow1))
                    SetIllegalParameter();
                else
                    PushSingleRef(nCol1, nRow1, nTab1);
            }
            else
            {
                if (nParamCount == 4)
                    nColNew = 1;
                nCol1 = (SCCOL)((long)nCol1+nColPlus);      // ! nCol1 wird veraendert!
                nRow1 = (SCROW)((long)nRow1+nRowPlus);
                nCol2 = (SCCOL)((long)nCol1+nColNew-1);
                nRow2 = (SCROW)((long)nRow1+nRowNew-1);
                if (!ValidCol(nCol1) || !ValidRow(nRow1) ||
                    !ValidCol(nCol2) || !ValidRow(nRow2))
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
            nCol1 = (SCCOL)((long)nCol1+nColPlus);
            nRow1 = (SCROW)((long)nRow1+nRowPlus);
            nCol2 = (SCCOL)((long)nCol1+nColNew-1);
            nRow2 = (SCROW)((long)nRow1+nRowNew-1);
            if (!ValidCol(nCol1) || !ValidRow(nRow1) ||
                !ValidCol(nCol2) || !ValidRow(nRow2) || nTab1 != nTab2)
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
        SCCOL nCol;
        SCROW nRow;
        if (nParamCount == 4)
            nBereich = (short) ::rtl::math::approxFloor(GetDouble());
        else
            nBereich = 1;
        if (nParamCount >= 3)
            nCol = (SCCOL) ::rtl::math::approxFloor(GetDouble());
        else
            nCol = 0;
        if (nParamCount >= 2)
            nRow = (SCROW) ::rtl::math::approxFloor(GetDouble());
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
            USHORT nOldSp = sp;
            ScMatrixRef pMat = GetMatrix();
            if (pMat)
            {
                SCSIZE nC, nR;
                pMat->GetDimensions(nC, nR);
                if (nC == 0 || nR == 0 || static_cast<SCSIZE>(nCol) > nC ||
                        static_cast<SCSIZE>(nRow) > nR)
                    SetIllegalArgument();
                else if (nCol == 0 && nRow == 0)
                    sp = nOldSp;
                else if (nRow == 0)
                {
                    ScMatrixRef pResMat = GetNewMat(nC, 1);
                    if (pResMat)
                    {
                        SCSIZE nColMinus1 = static_cast<SCSIZE>(nCol - 1);
                        for (SCSIZE i = 0; i < nC; i++)
                            if (!pMat->IsString(i, nColMinus1))
                                pResMat->PutDouble(pMat->GetDouble(i,
                                    nColMinus1), i, 0);
                            else
                                pResMat->PutString(pMat->GetString(i,
                                    nColMinus1), i, 0);
                        PushMatrix(pResMat);
                    }
                    else
                        PushError();
                }
                else if (nCol == 0)
                {
                    ScMatrixRef pResMat = GetNewMat(1, nR);
                    if (pResMat)
                    {
                        SCSIZE nRowMinus1 = static_cast<SCSIZE>(nRow - 1);
                        for (SCSIZE i = 0; i < nR; i++)
                            if (!pMat->IsString(nRowMinus1, i))
                                pResMat->PutDouble(pMat->GetDouble(nRowMinus1,
                                    i), i);
                            else
                                pResMat->PutString(pMat->GetString(nRowMinus1,
                                    i), i);
                        PushMatrix(pResMat);
                    }
                    else
                        PushError();
                }
                else
                {
                    if (!pMat->IsString( static_cast<SCSIZE>(nCol-1),
                                static_cast<SCSIZE>(nRow-1)))
                        PushDouble( pMat->GetDouble(
                                    static_cast<SCSIZE>(nCol-1),
                                    static_cast<SCSIZE>(nRow-1)));
                    else
                        PushString( pMat->GetString(
                                    static_cast<SCSIZE>(nCol-1),
                                    static_cast<SCSIZE>(nRow-1)));
                }
            }
        }
        else if (GetStackType() == svSingleRef || GetStackType() == svDoubleRef)
        {
            ScAddress aDummyAdr;
            ScRange aDummyRange;
            SCCOL nCol1;
            SCROW nRow1;
            SCTAB nTab1;
            SCCOL nCol2;
            SCROW nRow2;
            SCTAB nTab2 = MAXTAB+1;
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
            utl::SearchParam::SearchType eSearchType =
                (MayBeRegExp( SearchStr, pDok ) ?
                utl::SearchParam::SRCH_REGEXP : utl::SearchParam::SRCH_NORMAL);
            utl::SearchParam sPar(SearchStr, eSearchType, FALSE, FALSE, FALSE);
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
        double fAnz    = ::rtl::math::approxFloor(GetDouble());
        double fAnfang = ::rtl::math::approxFloor(GetDouble());
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


BOOL ScInterpreter::MayBeRegExp( const String& rStr, const ScDocument* pDoc  )
{
    if ( pDoc && !pDoc->GetDocOptions().IsFormulaRegexEnabled() )
        return FALSE;
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

