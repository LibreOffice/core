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

#include "interpre.hxx"

#include "scitems.hxx"
#include <editeng/langitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svx/algitem.hxx>
#include <unotools/textsearch.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <tools/urlobj.hxx>
#include <unotools/charclass.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <rtl/ustring.hxx>
#include <unicode/uchar.h>

#include "patattr.hxx"
#include "global.hxx"
#include "document.hxx"
#include "dociter.hxx"
#include "formulacell.hxx"
#include "scmatrix.hxx"
#include "docoptio.hxx"
#include "globstr.hrc"
#include "attrib.hxx"
#include "jumpmatrix.hxx"
#include "random.hxx"
#include "cellkeytranslator.hxx"
#include "lookupcache.hxx"
#include "rangenam.hxx"
#include "rangeutl.hxx"
#include "compiler.hxx"
#include "externalrefmgr.hxx"
#include <basic/sbstar.hxx>
#include "doubleref.hxx"
#include "queryparam.hxx"
#include "queryentry.hxx"
#include "tokenarray.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <memory>
#include <limits>

static const sal_uInt64 n2power48 = SAL_CONST_UINT64( 281474976710656); // 2^48

IMPL_FIXEDMEMPOOL_NEWDEL( ScTokenStack )
IMPL_FIXEDMEMPOOL_NEWDEL( ScInterpreter )

ScCalcConfig ScInterpreter::maGlobalConfig;
ScTokenStack* ScInterpreter::pGlobalStack = NULL;
bool ScInterpreter::bGlobalStackInUse = false;

using namespace formula;
using ::std::auto_ptr;

struct ScCompare
{
    double  nVal[2];
    String* pVal[2];
    bool    bVal[2];
    bool    bEmpty[2];
        ScCompare( String* p1, String* p2 )
        {
            pVal[ 0 ] = p1;
            pVal[ 1 ] = p2;
            bEmpty[0] = false;
            bEmpty[1] = false;
        }
};

struct ScCompareOptions
{
    ScQueryEntry        aQueryEntry;
    bool                bRegEx;
    bool                bMatchWholeCell;
    bool                bIgnoreCase;

                        ScCompareOptions( ScDocument* pDoc, const ScQueryEntry& rEntry, bool bReg );
private:
                        // Not implemented, prevent usage.
                        ScCompareOptions();
                        ScCompareOptions( const ScCompareOptions & );
     ScCompareOptions&  operator=( const ScCompareOptions & );
};

//-----------------------------------------------------------------------------
// Functions
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
                PushIllegalParameter();
            else
            {
                FormulaTokenRef xNew;
                ScTokenMatrixMap::const_iterator aMapIter;
                // DoubleError handled by JumpMatrix
                pMat->SetErrorInterpreter( NULL);
                SCSIZE nCols, nRows;
                pMat->GetDimensions( nCols, nRows );
                if ( nCols == 0 || nRows == 0 )
                {
                    PushIllegalArgument();
                    return;
                }
                else if (pTokenMatrixMap && ((aMapIter = pTokenMatrixMap->find(
                                    pCur)) != pTokenMatrixMap->end()))
                    xNew = (*aMapIter).second;
                else
                {
                    ScJumpMatrix* pJumpMat = new ScJumpMatrix( nCols, nRows );
                    for ( SCSIZE nC=0; nC < nCols; ++nC )
                    {
                        for ( SCSIZE nR=0; nR < nRows; ++nR )
                        {
                            double fVal;
                            bool bTrue;
                            bool bIsValue = pMat->IsValue(nC, nR);
                            if (bIsValue)
                            {
                                fVal = pMat->GetDouble(nC, nR);
                                bIsValue = ::rtl::math::isFinite(fVal);
                                bTrue = bIsValue && (fVal != 0.0);
                                if (bTrue)
                                    fVal = 1.0;
                            }
                            else
                            {
                                // Treat empty and empty path as 0, but string
                                // as error.
                                bIsValue = (!pMat->IsString(nC, nR) || pMat->IsEmpty(nC, nR));
                                bTrue = false;
                                fVal = (bIsValue ? 0.0 : CreateDoubleError( errNoValue));
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
                    xNew = new ScJumpMatrixToken( pJumpMat );
                    GetTokenMatrixMap().insert( ScTokenMatrixMap::value_type(pCur, xNew));
                }
                PushTempToken( xNew.get());
                // set endpoint of path for main code line
                aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
            }
        }
        break;
        default:
        {
            if ( GetBool() )
            {   // TRUE
                if( nJumpCount >= 2 )
                {   // THEN path
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


/** Store a matrix value in another matrix in the context of that other matrix
    is the result matrix of a jump matrix. All arguments must be valid and are
    not checked. */
static void lcl_storeJumpMatResult( const ScMatrix* pMat, ScMatrix* pResMat, SCSIZE nC, SCSIZE nR )
{
    if ( pMat->IsValue( nC, nR ) )
    {
        double fVal = pMat->GetDouble( nC, nR );
        pResMat->PutDouble( fVal, nC, nR );
    }
    else if ( pMat->IsEmpty( nC, nR ) )
    {
        pResMat->PutEmpty( nC, nR );
    }
    else
    {
        const String& rStr = pMat->GetString( nC, nR );
        pResMat->PutString( rStr, nC, nR );
    }
}


void ScInterpreter::ScIfError( bool bNAonly )
{
    const short* pJump = pCur->GetJump();
    short nJumpCount = pJump[ 0 ];
    if (!sp)
    {
        PushError( errUnknownStackVariable);
        aCode.Jump( pJump[ nJumpCount  ], pJump[ nJumpCount ] );
        return;
    }

    FormulaTokenRef xToken( pStack[ sp - 1 ] );
    bool bError = false;
    sal_uInt16 nOldGlobalError = nGlobalError;
    nGlobalError = 0;

    MatrixDoubleRefToMatrix();
    switch (GetStackType())
    {
        default:
            Pop();
            break;
        case svError:
            PopError();
            bError = true;
            break;
        case svDoubleRef:
        case svSingleRef:
            {
                ScAddress aAdr;
                if (!PopDoubleRefOrSingleRef( aAdr))
                    bError = true;
                else
                {

                    ScRefCellValue aCell;
                    aCell.assign(*pDok, aAdr);
                    nGlobalError = GetCellErrCode(aCell);
                    if (nGlobalError)
                        bError = true;
                }
            }
            break;
        case svExternalSingleRef:
        case svExternalDoubleRef:
        {
            double fVal;
            OUString aStr;
            // Handles also existing jump matrix case and sets error on
            // elements.
            GetDoubleOrStringFromMatrix( fVal, aStr);
            if (nGlobalError)
                bError = true;
        }
        break;
        case svMatrix:
            {
                const ScMatrixRef pMat = PopMatrix();
                if (!pMat || (nGlobalError && (!bNAonly || nGlobalError == NOTAVAILABLE)))
                {
                    bError = true;
                    break;  // switch
                }
                // If the matrix has no queried error at all we can simply use
                // it as result and don't need to bother with jump matrix.
                SCSIZE nErrorCol = ::std::numeric_limits<SCSIZE>::max(),
                       nErrorRow = ::std::numeric_limits<SCSIZE>::max();
                SCSIZE nCols, nRows;
                pMat->GetDimensions( nCols, nRows );
                if (nCols == 0 || nRows == 0)
                {
                    bError = true;
                    break;  // switch
                }
                for (SCSIZE nC=0; nC < nCols && !bError; ++nC)
                {
                    for (SCSIZE nR=0; nR < nRows && !bError; ++nR)
                    {
                        sal_uInt16 nErr = pMat->GetError( nC, nR );
                        if (nErr && (!bNAonly || nErr == NOTAVAILABLE))
                        {
                            bError = true;
                            nErrorCol = nC;
                            nErrorRow = nR;
                        }
                    }
                }
                if (!bError)
                    break;  // switch, we're done and have the result

                FormulaTokenRef xNew;
                ScTokenMatrixMap::const_iterator aMapIter;
                if (pTokenMatrixMap && ((aMapIter = pTokenMatrixMap->find( pCur)) != pTokenMatrixMap->end()))
                {
                    xNew = (*aMapIter).second;
                }
                else
                {
                    const ScMatrix* pMatPtr = pMat.get();
                    ScJumpMatrix* pJumpMat = new ScJumpMatrix( nCols, nRows );
                    ScMatrix* pResMatPtr = pJumpMat->GetResultMatrix();
                    // Init all jumps to no error to save single calls. Error
                    // is the exceptional condition.
                    const double fFlagResult = CreateDoubleError( errJumpMatHasResult);
                    pJumpMat->SetAllJumps( fFlagResult, pJump[ nJumpCount ], pJump[ nJumpCount ] );
                    // Up to first error position simply store results, no need
                    // to evaluate error conditions again.
                    SCSIZE nC = 0, nR = 0;
                    for ( ; nC < nCols && (nC != nErrorCol || nR != nErrorRow); /*nop*/ )
                    {
                        for ( ; nR < nRows && (nC != nErrorCol || nR != nErrorRow); ++nR)
                        {
                            lcl_storeJumpMatResult( pMatPtr, pResMatPtr, nC, nR);
                        }
                        if (nC != nErrorCol || nR != nErrorRow)
                            ++nC;
                    }
                    // Now the mixed cases.
                    for ( ; nC < nCols; ++nC)
                    {
                        for ( ; nR < nRows; ++nR)
                        {
                            sal_uInt16 nErr = pMat->GetError( nC, nR );
                            if (nErr && (!bNAonly || nErr == NOTAVAILABLE))
                            {   // TRUE, THEN path
                                pJumpMat->SetJump( nC, nR, 1.0, pJump[ 1 ], pJump[ nJumpCount ] );
                            }
                            else
                            {   // FALSE, EMPTY path, store result instead
                                lcl_storeJumpMatResult( pMatPtr, pResMatPtr, nC, nR);
                            }
                        }
                    }
                    xNew = new ScJumpMatrixToken( pJumpMat );
                    GetTokenMatrixMap().insert( ScTokenMatrixMap::value_type( pCur, xNew ));
                }
                nGlobalError = nOldGlobalError;
                PushTempToken( xNew.get() );
                // set endpoint of path for main code line
                aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
                return;
            }
            break;
    }

    if (bError && (!bNAonly || nGlobalError == NOTAVAILABLE))
    {
        // error, calculate 2nd argument
        nGlobalError = 0;
        aCode.Jump( pJump[ 1 ], pJump[ nJumpCount ] );
    }
    else
    {
        // no error, push 1st argument and continue
        nGlobalError = nOldGlobalError;
        PushTempToken( xToken.get());
        aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
    }
}


void ScInterpreter::ScChoseJump()
{
    // We have to set a jump, if there was none chosen because of an error set
    // it to endpoint.
    bool bHaveJump = false;
    const short* pJump = pCur->GetJump();
    short nJumpCount = pJump[ 0 ];
    MatrixDoubleRefToMatrix();
    switch ( GetStackType() )
    {
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( !pMat )
                PushIllegalParameter();
            else
            {
                FormulaTokenRef xNew;
                ScTokenMatrixMap::const_iterator aMapIter;
                // DoubleError handled by JumpMatrix
                pMat->SetErrorInterpreter( NULL);
                SCSIZE nCols, nRows;
                pMat->GetDimensions( nCols, nRows );
                if ( nCols == 0 || nRows == 0 )
                    PushIllegalParameter();
                else if (pTokenMatrixMap && ((aMapIter = pTokenMatrixMap->find(
                                    pCur)) != pTokenMatrixMap->end()))
                    xNew = (*aMapIter).second;
                else
                {
                    ScJumpMatrix* pJumpMat = new ScJumpMatrix( nCols, nRows );
                    for ( SCSIZE nC=0; nC < nCols; ++nC )
                    {
                        for ( SCSIZE nR=0; nR < nRows; ++nR )
                        {
                            double fVal;
                            bool bIsValue = pMat->IsValue(nC, nR);
                            if ( bIsValue )
                            {
                                fVal = pMat->GetDouble(nC, nR);
                                bIsValue = ::rtl::math::isFinite( fVal );
                                if ( bIsValue )
                                {
                                    fVal = ::rtl::math::approxFloor( fVal);
                                    if ( (fVal < 1) || (fVal >= nJumpCount))
                                    {
                                        bIsValue = false;
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
                    xNew = new ScJumpMatrixToken( pJumpMat );
                    GetTokenMatrixMap().insert( ScTokenMatrixMap::value_type(
                                pCur, xNew));
                }
                PushTempToken( xNew.get());
                // set endpoint of path for main code line
                aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
                bHaveJump = true;
            }
        }
        break;
        default:
        {
            double nJumpIndex = ::rtl::math::approxFloor( GetDouble() );
            if (!nGlobalError && (nJumpIndex >= 1) && (nJumpIndex < nJumpCount))
            {
                aCode.Jump( pJump[ (short) nJumpIndex ], pJump[ nJumpCount ] );
                bHaveJump = true;
            }
            else
                PushIllegalArgument();
        }
    }
    if (!bHaveJump)
        aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
}

static void lcl_AdjustJumpMatrix( ScJumpMatrix* pJumpM, ScMatrixRef& pResMat, SCSIZE nParmCols, SCSIZE nParmRows )
{
    SCSIZE nJumpCols, nJumpRows;
    SCSIZE nResCols, nResRows;
    SCSIZE nAdjustCols, nAdjustRows;
    pJumpM->GetDimensions( nJumpCols, nJumpRows );
    pJumpM->GetResMatDimensions( nResCols, nResRows );
    if (( nJumpCols == 1 && nParmCols > nResCols ) ||
        ( nJumpRows == 1 && nParmRows > nResRows ))
    {
        if ( nJumpCols == 1 && nJumpRows == 1 )
        {
            nAdjustCols = nParmCols > nResCols ? nParmCols : nResCols;
            nAdjustRows = nParmRows > nResRows ? nParmRows : nResRows;
        }
        else if ( nJumpCols == 1 )
        {
            nAdjustCols = nParmCols;
            nAdjustRows = nResRows;
        }
        else
        {
            nAdjustCols = nResCols;
            nAdjustRows = nParmRows;
        }
        pJumpM->SetNewResMat( nAdjustCols, nAdjustRows );
        pResMat = pJumpM->GetResultMatrix();
    }
}

bool ScInterpreter::JumpMatrix( short nStackLevel )
{
    pJumpMatrix = static_cast<ScToken*>(pStack[sp-nStackLevel])->GetJumpMatrix();
    ScMatrixRef pResMat = pJumpMatrix->GetResultMatrix();
    SCSIZE nC, nR;
    if ( nStackLevel == 2 )
    {
        if ( aCode.HasStacked() )
            aCode.Pop();    // pop what Jump() pushed
        else
        {
            OSL_FAIL( "ScInterpreter::JumpMatrix: pop goes the weasel" );
        }

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
                        ScRefCellValue aCell;
                        aCell.assign(*pDok, aAdr);
                        if (aCell.hasEmptyValue())
                            pResMat->PutEmpty( nC, nR );
                        else if (aCell.hasNumeric())
                        {
                            double fVal = GetCellValue(aAdr, aCell);
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
                            OUString aStr;
                            GetCellString(aStr, aCell);
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
                break;
                case svDoubleRef:
                {   // upper left plus offset within matrix
                    double fVal;
                    ScRange aRange;
                    PopDoubleRef( aRange );
                    if ( nGlobalError )
                    {
                        fVal = CreateDoubleError( nGlobalError );
                        nGlobalError = 0;
                        pResMat->PutDouble( fVal, nC, nR );
                    }
                    else
                    {
                        // Do not modify the original range because we use it
                        // to adjust the size of the result matrix if necessary.
                        ScAddress aAdr( aRange.aStart);
                        sal_uLong nCol = (sal_uLong)aAdr.Col() + nC;
                        sal_uLong nRow = (sal_uLong)aAdr.Row() + nR;
                        if ((nCol > static_cast<sal_uLong>(aRange.aEnd.Col()) &&
                                    aRange.aEnd.Col() != aRange.aStart.Col())
                                || (nRow > static_cast<sal_uLong>(aRange.aEnd.Row()) &&
                                    aRange.aEnd.Row() != aRange.aStart.Row()))
                          {
                            fVal = CreateDoubleError( NOTAVAILABLE );
                            pResMat->PutDouble( fVal, nC, nR );
                          }
                          else
                          {
                            // Replicate column and/or row of a vector if it is
                            // one. Note that this could be a range reference
                            // that in fact consists of only one cell, e.g. A1:A1
                            if (aRange.aEnd.Col() == aRange.aStart.Col())
                                nCol = aRange.aStart.Col();
                            if (aRange.aEnd.Row() == aRange.aStart.Row())
                                nRow = aRange.aStart.Row();
                            aAdr.SetCol( static_cast<SCCOL>(nCol) );
                            aAdr.SetRow( static_cast<SCROW>(nRow) );
                            ScRefCellValue aCell;
                            aCell.assign(*pDok, aAdr);
                            if (aCell.hasEmptyValue())
                                pResMat->PutEmpty( nC, nR );
                            else if (aCell.hasNumeric())
                              {
                                double fCellVal = GetCellValue(aAdr, aCell);
                                if ( nGlobalError )
                                {
                                    fCellVal = CreateDoubleError(
                                            nGlobalError);
                                    nGlobalError = 0;
                                }
                                pResMat->PutDouble( fCellVal, nC, nR );
                              }
                              else
                            {
                                OUString aStr;
                                GetCellString(aStr, aCell);
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
                        SCSIZE nParmCols = aRange.aEnd.Col() - aRange.aStart.Col() + 1;
                        SCSIZE nParmRows = aRange.aEnd.Row() - aRange.aStart.Row() + 1;
                        lcl_AdjustJumpMatrix( pJumpMatrix, pResMat, nParmCols, nParmRows );
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
                        if ((nCols <= nC && nCols != 1) ||
                            (nRows <= nR && nRows != 1))
                        {
                            fVal = CreateDoubleError( NOTAVAILABLE );
                            pResMat->PutDouble( fVal, nC, nR );
                        }
                        else
                        {
                            lcl_storeJumpMatResult( pMat.get(), pResMat.get(), nC, nR);
                        }
                        lcl_AdjustJumpMatrix( pJumpMatrix, pResMat, nCols, nRows );
                    }
                }
                break;
                case svError:
                {
                    PopError();
                    double fVal = CreateDoubleError( nGlobalError);
                    nGlobalError = 0;
                    pResMat->PutDouble( fVal, nC, nR );
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
            if ( pResMat && (GetDoubleErrorValue( fBool) != errJumpMatHasResult) )
            {
                // a false without path results in an empty path value
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
                    // This is not the current state of the interpreter, so
                    // push without error, and elements' errors are coded into
                    // double.
                    PushWithoutError( *(*i));
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
        // Remove jump matrix from map and remember result matrix in case it
        // could be reused in another path of the same condition.
        if (pTokenMatrixMap)
        {
            pTokenMatrixMap->erase( pCur);
            pTokenMatrixMap->insert( ScTokenMatrixMap::value_type( pCur,
                        pStack[sp-1]));
        }
        return true;
    }
    return false;
}


ScCompareOptions::ScCompareOptions( ScDocument* pDoc, const ScQueryEntry& rEntry, bool bReg ) :
    aQueryEntry(rEntry),
    bRegEx(bReg),
    bMatchWholeCell(pDoc->GetDocOptions().IsMatchWholeCell()),
    bIgnoreCase(true)
{
    bRegEx = (bRegEx && (aQueryEntry.eOp == SC_EQUAL || aQueryEntry.eOp == SC_NOT_EQUAL));
    // Interpreter functions usually are case insensitive, except the simple
    // comparison operators, for which these options aren't used. Override in
    // struct if needed.
}


double ScInterpreter::CompareFunc( const ScCompare& rComp, ScCompareOptions* pOptions )
{
    // Keep DoubleError if encountered
    // #i40539# if bEmpty is set, bVal/nVal are uninitialized
    if ( !rComp.bEmpty[0] && rComp.bVal[0] && !::rtl::math::isFinite( rComp.nVal[0]))
        return rComp.nVal[0];
    if ( !rComp.bEmpty[1] && rComp.bVal[1] && !::rtl::math::isFinite( rComp.nVal[1]))
        return rComp.nVal[1];

    size_t nStringQuery = 0;    // 0:=no, 1:=0, 2:=1
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
        {
            fRes = -1;          // number is less than string
            nStringQuery = 2;   // 1+1
        }
    }
    else if( rComp.bVal[ 1 ] )
    {
        fRes = 1;               // string is greater than number
        nStringQuery = 1;       // 0+1
    }
    else
    {
        // Both strings.
        if (pOptions)
        {
            // All similar to ScTable::ValidQuery(), *rComp.pVal[1] actually
            // is/must be identical to *rEntry.pStr, which is essential for
            // regex to work through GetSearchTextPtr().
            ScQueryEntry& rEntry = pOptions->aQueryEntry;
            OSL_ENSURE(rEntry.GetQueryItem().maString.equals(*rComp.pVal[1]), "ScInterpreter::CompareFunc: broken options");
            if (pOptions->bRegEx)
            {
                sal_Int32 nStart = 0;
                sal_Int32 nStop  = rComp.pVal[0]->Len();
                bool bMatch = rEntry.GetSearchTextPtr(
                        !pOptions->bIgnoreCase)->SearchForward( *rComp.pVal[0],
                            &nStart, &nStop);
                if (bMatch && pOptions->bMatchWholeCell && (nStart != 0 || nStop != rComp.pVal[0]->Len()))
                    bMatch = false;     // RegEx must match entire string.
                fRes = (bMatch ? 0 : 1);
            }
            else if (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL)
            {
                ::utl::TransliterationWrapper* pTransliteration =
                    (pOptions->bIgnoreCase ? ScGlobal::GetpTransliteration() :
                     ScGlobal::GetCaseTransliteration());
                bool bMatch;
                if (pOptions->bMatchWholeCell)
                    bMatch = pTransliteration->isEqual( *rComp.pVal[0], *rComp.pVal[1]);
                else
                {
                    String aCell( pTransliteration->transliterate(
                                *rComp.pVal[0], ScGlobal::eLnge, 0,
                                rComp.pVal[0]->Len(), NULL));
                    String aQuer( pTransliteration->transliterate(
                                *rComp.pVal[1], ScGlobal::eLnge, 0,
                                rComp.pVal[1]->Len(), NULL));
                    bMatch = (aCell.Search( aQuer ) != STRING_NOTFOUND);
                }
                fRes = (bMatch ? 0 : 1);
            }
            else if (pOptions->bIgnoreCase)
                fRes = (double) ScGlobal::GetCollator()->compareString(
                        *rComp.pVal[ 0 ], *rComp.pVal[ 1 ] );
            else
                fRes = (double) ScGlobal::GetCaseCollator()->compareString(
                        *rComp.pVal[ 0 ], *rComp.pVal[ 1 ] );
        }
        else if (pDok->GetDocOptions().IsIgnoreCase())
            fRes = (double) ScGlobal::GetCollator()->compareString(
                *rComp.pVal[ 0 ], *rComp.pVal[ 1 ] );
        else
            fRes = (double) ScGlobal::GetCaseCollator()->compareString(
                *rComp.pVal[ 0 ], *rComp.pVal[ 1 ] );
    }

    if (nStringQuery && pOptions)
    {
        const ScQueryEntry& rEntry = pOptions->aQueryEntry;
        const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        if (!rItems.empty())
        {
            const ScQueryEntry::Item& rItem = rItems[0];
            if (rItem.meType != ScQueryEntry::ByString && !rItem.maString.isEmpty() &&
                (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL))
            {
                // As in ScTable::ValidQuery() match a numeric string for a
                // number query that originated from a string, e.g. in SUMIF
                // and COUNTIF. Transliteration is not needed here.
                bool bEqual = rComp.pVal[nStringQuery-1]->Equals(rItem.maString);
                // match => fRes=0, else fRes=1
                fRes = (rEntry.eOp == SC_NOT_EQUAL) ? bEqual : !bEqual;
            }
        }
    }

    return fRes;
}


double ScInterpreter::Compare()
{
    String aVal1, aVal2;
    ScCompare aComp( &aVal1, &aVal2 );
    for( short i = 1; i >= 0; i-- )
    {
        switch ( GetRawStackType() )
        {
            case svEmptyCell:
                Pop();
                aComp.bEmpty[ i ] = true;
                break;
            case svMissing:
            case svDouble:
                aComp.nVal[ i ] = GetDouble();
                aComp.bVal[ i ] = true;
                break;
            case svString:
                *aComp.pVal[ i ] = GetString();
                aComp.bVal[ i ] = false;
                break;
            case svDoubleRef :
            case svSingleRef :
            {
                ScAddress aAdr;
                if ( !PopDoubleRefOrSingleRef( aAdr ) )
                    break;
                ScRefCellValue aCell;
                aCell.assign(*pDok, aAdr);
                if (aCell.hasEmptyValue())
                    aComp.bEmpty[i] = true;
                else if (aCell.hasString())
                {
                    OUString aStr;
                    GetCellString(aStr, aCell);
                    *aComp.pVal[i] = aStr;
                    aComp.bVal[i] = false;
                }
                else
                {
                    aComp.nVal[i] = GetCellValue(aAdr, aCell);
                    aComp.bVal[i] = true;
                }
            }
            break;
            case svExternalSingleRef:
            {
                ScMatrixRef pMat = GetMatrix();
                if (!pMat)
                {
                    SetError( errIllegalParameter);
                    break;
                }

                SCSIZE nC, nR;
                pMat->GetDimensions(nC, nR);
                if (!nC || !nR)
                {
                    SetError( errIllegalParameter);
                    break;
                }
                if (pMat->IsEmpty(0, 0))
                    aComp.bEmpty[i] = true;
                else if (pMat->IsString(0, 0))
                {
                    *aComp.pVal[i] = pMat->GetString(0, 0);
                    aComp.bVal[i] = false;
                }
                else
                {
                    aComp.nVal[i] = pMat->GetDouble(0, 0);
                    aComp.bVal[i] = true;
                }
            }
            break;
            case svExternalDoubleRef:
                // TODO: Find out how to handle this...
            default:
                SetError( errIllegalParameter);
            break;
        }
    }
    if( nGlobalError )
        return 0;
    nCurFmtType = nFuncFmtType = NUMBERFORMAT_LOGICAL;
    return CompareFunc( aComp );
}


ScMatrixRef ScInterpreter::CompareMat( ScCompareOptions* pOptions )
{
    String aVal1, aVal2;
    ScCompare aComp( &aVal1, &aVal2 );
    ScMatrixRef pMat[2];
    ScAddress aAdr;
    for( short i = 1; i >= 0; i-- )
    {
        switch (GetRawStackType())
        {
            case svEmptyCell:
                Pop();
                aComp.bEmpty[ i ] = true;
                break;
            case svMissing:
            case svDouble:
                aComp.nVal[ i ] = GetDouble();
                aComp.bVal[ i ] = true;
                break;
            case svString:
                *aComp.pVal[ i ] = GetString();
                aComp.bVal[ i ] = false;
                break;
            case svSingleRef:
            {
                PopSingleRef( aAdr );
                ScRefCellValue aCell;
                aCell.assign(*pDok, aAdr);
                if (aCell.hasEmptyValue())
                    aComp.bEmpty[i] = true;
                else if (aCell.hasString())
                {
                    OUString aStr;
                    GetCellString(aStr, aCell);
                    *aComp.pVal[i] = aStr;
                    aComp.bVal[i] = false;
                }
                else
                {
                    aComp.nVal[i] = GetCellValue(aAdr, aCell);
                    aComp.bVal[i] = true;
                }
            }
            break;
            case svDoubleRef:
            case svMatrix:
                pMat[ i ] = GetMatrix();
                if ( !pMat[ i ] )
                    SetError( errIllegalParameter);
                else
                    pMat[i]->SetErrorInterpreter( NULL);
                    // errors are transported as DoubleError inside matrix
                break;
            default:
                SetError( errIllegalParameter);
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
            SCSIZE nC = std::max( nC0, nC1 );
            SCSIZE nR = std::max( nR0, nR1 );
            pResMat = GetNewMat( nC, nR);
            if ( !pResMat )
                return NULL;
            for ( SCSIZE j=0; j<nC; j++ )
            {
                for ( SCSIZE k=0; k<nR; k++ )
                {
                    SCSIZE nCol = j, nRow = k;
                    if (    pMat[0]->ValidColRowOrReplicated( nCol, nRow ) &&
                            pMat[1]->ValidColRowOrReplicated( nCol, nRow ))
                    {
                        for ( short i=1; i>=0; i-- )
                        {
                            if ( pMat[i]->IsString(j,k) )
                            {
                                aComp.bVal[i] = false;
                                *aComp.pVal[i] = pMat[i]->GetString(j,k);
                                aComp.bEmpty[i] = pMat[i]->IsEmpty(j,k);
                            }
                            else
                            {
                                aComp.bVal[i] = true;
                                aComp.nVal[i] = pMat[i]->GetDouble(j,k);
                                aComp.bEmpty[i] = false;
                            }
                        }
                        pResMat->PutDouble( CompareFunc( aComp, pOptions ), j,k );
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

            for (SCSIZE j = 0; j < nC; ++j)
            {
                for (SCSIZE k = 0; k < nR; ++k)
                {
                    if ( pMat[i]->IsValue(j,k) )
                    {
                        aComp.bVal[i] = true;
                        aComp.nVal[i] = pMat[i]->GetDouble(j,k);
                        aComp.bEmpty[i] = false;
                    }
                    else
                    {
                        aComp.bVal[i] = false;
                        *aComp.pVal[i] = pMat[i]->GetString(j,k);
                        aComp.bEmpty[i] = pMat[i]->IsEmpty(j,k);
                    }
                    pResMat->PutDouble( CompareFunc(aComp, pOptions), j, k);
                }
            }
        }
    }
    nCurFmtType = nFuncFmtType = NUMBERFORMAT_LOGICAL;
    return pResMat;
}


ScMatrixRef ScInterpreter::QueryMat( const ScMatrixRef& pMat, ScCompareOptions& rOptions )
{
    short nSaveCurFmtType = nCurFmtType;
    short nSaveFuncFmtType = nFuncFmtType;
    PushMatrix( pMat);
    const ScQueryEntry::Item& rItem = rOptions.aQueryEntry.GetQueryItem();
    if (rItem.meType == ScQueryEntry::ByString)
        PushString(rItem.maString);
    else
        PushDouble(rItem.mfVal);
    ScMatrixRef pResultMatrix = CompareMat( &rOptions);
    nCurFmtType = nSaveCurFmtType;
    nFuncFmtType = nSaveFuncFmtType;
    if (nGlobalError || !pResultMatrix)
    {
        SetError( errIllegalParameter);
        return pResultMatrix;
    }

    switch (rOptions.aQueryEntry.eOp)
    {
        case SC_EQUAL:
            pResultMatrix->CompareEqual();
            break;
        case SC_LESS:
            pResultMatrix->CompareLess();
            break;
        case SC_GREATER:
            pResultMatrix->CompareGreater();
            break;
        case SC_LESS_EQUAL:
            pResultMatrix->CompareLessEqual();
            break;
        case SC_GREATER_EQUAL:
            pResultMatrix->CompareGreaterEqual();
            break;
        case SC_NOT_EQUAL:
            pResultMatrix->CompareNotEqual();
            break;
        default:
            SetError( errIllegalArgument);
            OSL_TRACE( "ScInterpreter::QueryMat: unhandled comparison operator: %d", (int)rOptions.aQueryEntry.eOp);
    }
    return pResultMatrix;
}


void ScInterpreter::ScEqual()
{
    if ( GetStackType(1) == svMatrix || GetStackType(2) == svMatrix )
    {
        ScMatrixRef pMat = CompareMat();
        if ( !pMat )
            PushIllegalParameter();
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
            PushIllegalParameter();
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
            PushIllegalParameter();
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
            PushIllegalParameter();
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
            PushIllegalParameter();
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
            PushIllegalParameter();
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
    short nParamCount = GetByte();
    if ( MustHaveParamCountMin( nParamCount, 1 ) )
    {
        bool bHaveValue = false;
        short nRes = true;
        size_t nRefInList = 0;
        while( nParamCount-- > 0)
        {
            if ( !nGlobalError )
            {
                switch ( GetStackType() )
                {
                    case svDouble :
                        bHaveValue = true;
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
                            ScRefCellValue aCell;
                            aCell.assign(*pDok, aAdr);
                            if (aCell.hasNumeric())
                            {
                                bHaveValue = true;
                                nRes &= ( GetCellValue(aAdr, aCell) != 0.0 );
                            }
                            // else: Xcl raises no error here
                        }
                    }
                    break;
                    case svDoubleRef:
                    case svRefList:
                    {
                        ScRange aRange;
                        PopDoubleRef( aRange, nParamCount, nRefInList);
                        if ( !nGlobalError )
                        {
                            double fVal;
                            sal_uInt16 nErr = 0;
                            ScValueIterator aValIter( pDok, aRange );
                            if ( aValIter.GetFirst( fVal, nErr ) )
                            {
                                bHaveValue = true;
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
                    case svExternalSingleRef:
                    case svExternalDoubleRef:
                    case svMatrix:
                    {
                        ScMatrixRef pMat = GetMatrix();
                        if ( pMat )
                        {
                            bHaveValue = true;
                            double fVal = pMat->And();
                            sal_uInt16 nErr = GetDoubleErrorValue( fVal );
                            if ( nErr )
                            {
                                SetError( nErr );
                                nRes = false;
                            }
                            else
                                nRes &= (fVal != 0.0);
                        }
                        // else: GetMatrix did set errIllegalParameter
                    }
                    break;
                    default:
                        Pop();
                        SetError( errIllegalParameter);
                }
            }
            else
                Pop();
        }
        if ( bHaveValue )
            PushInt( nRes );
        else
            PushNoValue();
    }
}


void ScInterpreter::ScOr()
{
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    short nParamCount = GetByte();
    if ( MustHaveParamCountMin( nParamCount, 1 ) )
    {
        bool bHaveValue = false;
        short nRes = false;
        size_t nRefInList = 0;
        while( nParamCount-- > 0)
        {
            if ( !nGlobalError )
            {
                switch ( GetStackType() )
                {
                    case svDouble :
                        bHaveValue = true;
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
                            ScRefCellValue aCell;
                            aCell.assign(*pDok, aAdr);
                            if (aCell.hasNumeric())
                            {
                                bHaveValue = true;
                                nRes |= ( GetCellValue(aAdr, aCell) != 0.0 );
                            }
                            // else: Xcl raises no error here
                        }
                    }
                    break;
                    case svDoubleRef:
                    case svRefList:
                    {
                        ScRange aRange;
                        PopDoubleRef( aRange, nParamCount, nRefInList);
                        if ( !nGlobalError )
                        {
                            double fVal;
                            sal_uInt16 nErr = 0;
                            ScValueIterator aValIter( pDok, aRange );
                            if ( aValIter.GetFirst( fVal, nErr ) )
                            {
                                bHaveValue = true;
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
                    case svExternalSingleRef:
                    case svExternalDoubleRef:
                    case svMatrix:
                    {
                        bHaveValue = true;
                        ScMatrixRef pMat = GetMatrix();
                        if ( pMat )
                        {
                            bHaveValue = true;
                            double fVal = pMat->Or();
                            sal_uInt16 nErr = GetDoubleErrorValue( fVal );
                            if ( nErr )
                            {
                                SetError( nErr );
                                nRes = false;
                            }
                            else
                                nRes |= (fVal != 0.0);
                        }
                        // else: GetMatrix did set errIllegalParameter
                    }
                    break;
                    default:
                        Pop();
                        SetError( errIllegalParameter);
                }
            }
            else
                Pop();
        }
        if ( bHaveValue )
            PushInt( nRes );
        else
            PushNoValue();
    }
}


void ScInterpreter::ScXor()
{

    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    short nParamCount = GetByte();
    if ( MustHaveParamCountMin( nParamCount, 1 ) )
    {
        bool bHaveValue = false;
        short nRes = false;
        size_t nRefInList = 0;
        while( nParamCount-- > 0)
        {
            if ( !nGlobalError )
            {
                switch ( GetStackType() )
                {
                    case svDouble :
                        bHaveValue = true;
                        nRes ^= ( PopDouble() != 0.0 );
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
                            ScRefCellValue aCell;
                            aCell.assign(*pDok, aAdr);
                            if (aCell.hasNumeric())
                            {
                                bHaveValue = true;
                                nRes ^= ( GetCellValue(aAdr, aCell) != 0.0 );
                            }
                            /* TODO: set error? Excel doesn't have XOR, but
                             * doesn't set an error in this case for AND and
                             * OR. */
                        }
                    }
                    break;
                    case svDoubleRef:
                    case svRefList:
                    {
                        ScRange aRange;
                        PopDoubleRef( aRange, nParamCount, nRefInList);
                        if ( !nGlobalError )
                        {
                            double fVal;
                            sal_uInt16 nErr = 0;
                            ScValueIterator aValIter( pDok, aRange );
                            if ( aValIter.GetFirst( fVal, nErr ) )
                            {
                                bHaveValue = true;
                                do
                                {
                                    nRes ^= ( fVal != 0.0 );
                                } while ( (nErr == 0) &&
                                    aValIter.GetNext( fVal, nErr ) );
                            }
                            SetError( nErr );
                        }
                    }
                    break;
                    case svExternalSingleRef:
                    case svExternalDoubleRef:
                    case svMatrix:
                    {
                        bHaveValue = true;
                        ScMatrixRef pMat = GetMatrix();
                        if ( pMat )
                        {
                            bHaveValue = true;
                            double fVal = pMat->Xor();
                            sal_uInt16 nErr = GetDoubleErrorValue( fVal );
                            if ( nErr )
                            {
                                SetError( nErr );
                                nRes = 0;
                            }
                            else
                                nRes ^= ( fVal != 0.0 );
                        }
                        // else: GetMatrix did set errIllegalParameter
                    }
                    break;
                    default:
                        Pop();
                        SetError( errIllegalParameter);
                }
            }
            else
                Pop();
        }
        if ( bHaveValue )
            PushInt( nRes );
        else
            PushNoValue();
    }
}


void ScInterpreter::ScNeg()
{
    // Simple negation doesn't change current format type to number, keep
    // current type.
    nFuncFmtType = nCurFmtType;
    switch ( GetStackType() )
    {
        case svMatrix :
        {
            ScMatrixRef pMat = GetMatrix();
            if ( !pMat )
                PushIllegalParameter();
            else
            {
                SCSIZE nC, nR;
                pMat->GetDimensions( nC, nR );
                ScMatrixRef pResMat = GetNewMat( nC, nR);
                if ( !pResMat )
                    PushIllegalArgument();
                else
                {
                    for (SCSIZE i = 0; i < nC; ++i)
                    {
                        for (SCSIZE j = 0; j < nR; ++j)
                        {
                            if ( pMat->IsValueOrEmpty(i,j) )
                                pResMat->PutDouble( -pMat->GetDouble(i,j), i, j );
                            else
                                pResMat->PutString(
                                    ScGlobal::GetRscString( STR_NO_VALUE ), i, j );
                        }
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
    const FormulaToken* pSaveCur = pCur;
    sal_uInt8 nSavePar = cPar;
    PushInt( 100 );
    cPar = 2;
    FormulaByteToken aDivOp( ocDiv, cPar );
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
                PushIllegalParameter();
            else
            {
                SCSIZE nC, nR;
                pMat->GetDimensions( nC, nR );
                ScMatrixRef pResMat = GetNewMat( nC, nR);
                if ( !pResMat )
                    PushIllegalArgument();
                else
                {
                    for (SCSIZE i = 0; i < nC; ++i)
                    {
                        for (SCSIZE j = 0; j < nR; ++j)
                        {
                            if ( pMat->IsValueOrEmpty(i,j) )
                                pResMat->PutDouble( (pMat->GetDouble(i,j) == 0.0), i, j );
                            else
                                pResMat->PutString(
                                    ScGlobal::GetRscString( STR_NO_VALUE ), i, j );
                        }
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


void ScInterpreter::ScBitAnd()
{

    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    double num1 = ::rtl::math::approxFloor( GetDouble());
    double num2 = ::rtl::math::approxFloor( GetDouble());
    if (    (num1 >= n2power48) || (num1 < 0) ||
            (num2 >= n2power48) || (num2 < 0))
        PushIllegalArgument();
    else
        PushDouble ((sal_uInt64) num1 & (sal_uInt64) num2);
}


void ScInterpreter::ScBitOr()
{

    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    double num1 = ::rtl::math::approxFloor( GetDouble());
    double num2 = ::rtl::math::approxFloor( GetDouble());
    if (    (num1 >= n2power48) || (num1 < 0) ||
            (num2 >= n2power48) || (num2 < 0))
        PushIllegalArgument();
    else
        PushDouble ((sal_uInt64) num1 | (sal_uInt64) num2);
}


void ScInterpreter::ScBitXor()
{

    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    double num1 = ::rtl::math::approxFloor( GetDouble());
    double num2 = ::rtl::math::approxFloor( GetDouble());
    if (    (num1 >= n2power48) || (num1 < 0) ||
            (num2 >= n2power48) || (num2 < 0))
        PushIllegalArgument();
    else
        PushDouble ((sal_uInt64) num1 ^ (sal_uInt64) num2);
}


void ScInterpreter::ScBitLshift()
{

    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    double fShift = ::rtl::math::approxFloor( GetDouble());
    double num = ::rtl::math::approxFloor( GetDouble());
    if ((num >= n2power48) || (num < 0))
        PushIllegalArgument();
    else
    {
        double fRes;
        if (fShift < 0)
            fRes = ::rtl::math::approxFloor( num / pow( 2.0, -fShift));
        else if (fShift == 0)
            fRes = num;
        else
            fRes = num * pow( 2.0, fShift);
        PushDouble( fRes);
    }
}


void ScInterpreter::ScBitRshift()
{

    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    double fShift = ::rtl::math::approxFloor( GetDouble());
    double num = ::rtl::math::approxFloor( GetDouble());
    if ((num >= n2power48) || (num < 0))
        PushIllegalArgument();
    else
    {
        double fRes;
        if (fShift < 0)
            fRes = num * pow( 2.0, -fShift);
        else if (fShift == 0)
            fRes = num;
        else
            fRes = ::rtl::math::approxFloor( num / pow( 2.0, fShift));
        PushDouble( fRes);
    }
}


void ScInterpreter::ScPi()
{
    PushDouble(F_PI);
}


void ScInterpreter::ScRandom()
{
    PushDouble(sc::rng::uniform());
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
    PushDouble( ::rtl::math::asinh( GetDouble()));
}

void ScInterpreter::ScArcCosHyp()
{
    double fVal = GetDouble();
    if (fVal < 1.0)
        PushIllegalArgument();
    else
        PushDouble( ::rtl::math::acosh( fVal));
}

void ScInterpreter::ScArcTanHyp()
{
    double fVal = GetDouble();
    if (fabs(fVal) >= 1.0)
        PushIllegalArgument();
    else
        PushDouble( ::rtl::math::atanh( fVal));
}


void ScInterpreter::ScArcCotHyp()
{
    double nVal = GetDouble();
    if (fabs(nVal) <= 1.0)
        PushIllegalArgument();
    else
        PushDouble(0.5 * log((nVal + 1.0) / (nVal - 1.0)));
}

void ScInterpreter::ScCosecant()
{
    PushDouble(1.0 / ::rtl::math::sin(GetDouble()));
}

void ScInterpreter::ScSecant()
{
    PushDouble(1.0 / ::rtl::math::cos(GetDouble()));
}

void ScInterpreter::ScCosecantHyp()
{
    PushDouble(1.0 / sinh(GetDouble()));
}

void ScInterpreter::ScSecantHyp()
{
    PushDouble(1.0 / cosh(GetDouble()));
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
        PushIllegalArgument();
}


void ScInterpreter::ScIsEmpty()
{
    short nRes = 0;
    nFuncFmtType = NUMBERFORMAT_LOGICAL;
    switch ( GetRawStackType() )
    {
        case svEmptyCell:
        {
            FormulaTokenRef p = PopToken();
            if (!static_cast<const ScEmptyCellToken*>(p.get())->IsInherited())
                nRes = 1;
        }
        break;
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;
            // NOTE: this could test also on inherited emptiness, but then the
            // cell tested wouldn't be empty. Must correspond with
            // ScCountEmptyCells().
            ScRefCellValue aCell;
            aCell.assign(*pDok, aAdr);
            if (aCell.meType == CELLTYPE_NONE)
                nRes = 1;
        }
        break;
        case svExternalSingleRef:
        case svExternalDoubleRef:
        case svMatrix:
        {
            ScMatrixRef pMat = GetMatrix();
            if ( !pMat )
                ;   // nothing
            else if ( !pJumpMatrix )
                nRes = pMat->IsEmpty( 0, 0);
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    nRes = pMat->IsEmpty( nC, nR);
                // else: false, not empty (which is what Xcl does)
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
    switch ( GetRawStackType() )
    {
        case svString:
            Pop();
            nRes = 1;
        break;
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;

            ScRefCellValue aCell;
            aCell.assign(*pDok, aAdr);
            if (GetCellErrCode(aCell) == 0)
            {
                switch (aCell.meType)
                {
                    case CELLTYPE_STRING :
                    case CELLTYPE_EDIT :
                        nRes = 1;
                        break;
                    case CELLTYPE_FORMULA :
                        nRes = (!aCell.mpFormula->IsValue() && !aCell.mpFormula->IsEmpty());
                        break;
                    default:
                        ; // nothing
                }
            }
        }
        break;
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( !pMat )
                ;   // nothing
            else if ( !pJumpMatrix )
                nRes = pMat->IsString(0, 0) && !pMat->IsEmpty(0, 0);
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


void ScInterpreter::ScIsLogical()
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

            ScRefCellValue aCell;
            aCell.assign(*pDok, aAdr);
            if (GetCellErrCode(aCell) == 0)
            {
                if (aCell.hasNumeric())
                {
                    sal_uLong nFormat = GetCellNumberFormat(aAdr, aCell);
                    nRes = (pFormatter->GetType(nFormat) == NUMBERFORMAT_LOGICAL);
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

            ScRefCellValue aCell;
            aCell.assign(*pDok, aAdr);
            if (GetCellErrCode(aCell) == 0)
            {
                switch (aCell.meType)
                {
                    // NOTE: this is Xcl nonsense!
                    case CELLTYPE_STRING :
                    case CELLTYPE_EDIT :
                        nType = 2;
                        break;
                    case CELLTYPE_VALUE :
                    {
                        sal_uLong nFormat = GetCellNumberFormat(aAdr, aCell);
                        if (pFormatter->GetType(nFormat) == NUMBERFORMAT_LOGICAL)
                            nType = 4;
                        else
                            nType = 1;
                    }
                    break;
                    case CELLTYPE_FORMULA :
                        nType = 8;
                        break;
                    default:
                        PushIllegalArgument();
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


static inline bool lcl_FormatHasNegColor( const SvNumberformat* pFormat )
{
    return pFormat && pFormat->GetColor( 1 );
}


static inline bool lcl_FormatHasOpenPar( const SvNumberformat* pFormat )
{
    return pFormat && (pFormat->GetFormatstring().indexOf('(') != -1);
}

namespace {

void getFormatString(SvNumberFormatter* pFormatter, sal_uLong nFormat, OUString& rFmtStr)
{
    bool        bAppendPrec = true;
    sal_uInt16  nPrec, nLeading;
    bool        bThousand, bIsRed;
    pFormatter->GetFormatSpecialInfo( nFormat, bThousand, bIsRed, nPrec, nLeading );

    switch( pFormatter->GetType( nFormat ) )
    {
        case NUMBERFORMAT_NUMBER:       if(bThousand) rFmtStr = ","; else rFmtStr = "F"; break;
        case NUMBERFORMAT_CURRENCY:     rFmtStr = "C";                                   break;
        case NUMBERFORMAT_SCIENTIFIC:   rFmtStr = "S";                                   break;
        case NUMBERFORMAT_PERCENT:      rFmtStr = "P";                                   break;
        default:
        {
            bAppendPrec = false;
            switch( pFormatter->GetIndexTableOffset( nFormat ) )
            {
                case NF_DATE_SYSTEM_SHORT:
                case NF_DATE_SYS_DMMMYY:
                case NF_DATE_SYS_DDMMYY:
                case NF_DATE_SYS_DDMMYYYY:
                case NF_DATE_SYS_DMMMYYYY:
                case NF_DATE_DIN_DMMMYYYY:
                case NF_DATE_SYS_DMMMMYYYY:
                case NF_DATE_DIN_DMMMMYYYY: rFmtStr = "D1"; break;
                case NF_DATE_SYS_DDMMM:     rFmtStr = "D2"; break;
                case NF_DATE_SYS_MMYY:      rFmtStr = "D3"; break;
                case NF_DATETIME_SYSTEM_SHORT_HHMM:
                case NF_DATETIME_SYS_DDMMYYYY_HHMMSS:
                                            rFmtStr = "D4"; break;
                case NF_DATE_DIN_MMDD:      rFmtStr = "D5"; break;
                case NF_TIME_HHMMSSAMPM:    rFmtStr = "D6"; break;
                case NF_TIME_HHMMAMPM:      rFmtStr = "D7"; break;
                case NF_TIME_HHMMSS:        rFmtStr = "D8"; break;
                case NF_TIME_HHMM:          rFmtStr = "D9"; break;
                default:                    rFmtStr = "G";
            }
        }
    }
    if( bAppendPrec )
        rFmtStr += OUString::number(nPrec);
    const SvNumberformat* pFormat = pFormatter->GetEntry( nFormat );
    if( lcl_FormatHasNegColor( pFormat ) )
        rFmtStr += "-";
    if( lcl_FormatHasOpenPar( pFormat ) )
        rFmtStr += "()";
}

}

void ScInterpreter::ScCell()
{   // ATTRIBUTE ; [REF]
    sal_uInt8 nParamCount = GetByte();
    if( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        ScAddress aCellPos( aPos );
        bool bError = false;
        if( nParamCount == 2 )
        {
            switch (GetStackType())
            {
                case svExternalSingleRef:
                case svExternalDoubleRef:
                {
                    // Let's handle external reference separately...
                    ScCellExternal();
                    return;
                }
                default:
                    ;
            }
            bError = !PopDoubleRefOrSingleRef( aCellPos );
        }
        String aInfoType( GetString() );
        if( bError || nGlobalError )
            PushIllegalParameter();
        else
        {
            ScRefCellValue aCell;
            aCell.assign(*pDok, aCellPos);

            ScCellKeywordTranslator::transKeyword(aInfoType, ScGlobal::GetLocale(), ocCell);

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
                sal_uInt16 nFlags = (aCellPos.Tab() == aPos.Tab()) ? (SCA_ABS) : (SCA_ABS_3D);
                OUString aStr;
                aCellPos.Format( aStr, nFlags, pDok, pDok->GetAddressConvention() );
                PushString(aStr);
            }
            else if( aInfoType.EqualsAscii( "FILENAME" ) )
            {   // file name and table name: 'FILENAME'#$TABLE
                SCTAB nTab = aCellPos.Tab();
                OUString aFuncResult;
                if( nTab < pDok->GetTableCount() )
                {
                    if( pDok->GetLinkMode( nTab ) == SC_LINK_VALUE )
                        pDok->GetName( nTab, aFuncResult );
                    else
                    {
                        SfxObjectShell* pShell = pDok->GetDocumentShell();
                        if( pShell && pShell->GetMedium() )
                        {
                            OUStringBuffer aBuf;
                            aBuf.append(sal_Unicode('\''));
                            const INetURLObject& rURLObj = pShell->GetMedium()->GetURLObject();
                            aBuf.append(rURLObj.GetMainURL(INetURLObject::DECODE_UNAMBIGUOUS));
                            aBuf.appendAscii("'#$");
                            OUString aTabName;
                            pDok->GetName( nTab, aTabName );
                            aBuf.append(aTabName);
                            aFuncResult = aBuf.makeStringAndClear();
                        }
                    }
                }
                PushString( aFuncResult );
            }
            else if( aInfoType.EqualsAscii( "COORD" ) )
            {   // address, lotus 1-2-3 formatted: $TABLE:$COL$ROW
                // Yes, passing tab as col is intentional!
                OUStringBuffer aFuncResult;
                OUString aCellStr;
                ScAddress( static_cast<SCCOL>(aCellPos.Tab()), 0, 0 ).Format(
                    aCellStr, (SCA_COL_ABSOLUTE|SCA_VALID_COL), NULL, pDok->GetAddressConvention() );
                aFuncResult.append(aCellStr);
                aFuncResult.append(sal_Unicode(':'));
                aCellPos.Format( aCellStr, (SCA_COL_ABSOLUTE|SCA_VALID_COL|SCA_ROW_ABSOLUTE|SCA_VALID_ROW),
                                 NULL, pDok->GetAddressConvention() );
                aFuncResult.append(aCellStr);
                PushString( aFuncResult.makeStringAndClear() );
            }

// *** CELL PROPERTIES ***
            else if( aInfoType.EqualsAscii( "CONTENTS" ) )
            {   // contents of the cell, no formatting
                if (aCell.hasString())
                {
                    OUString aStr;
                    GetCellString(aStr, aCell);
                    PushString( aStr );
                }
                else
                    PushDouble(GetCellValue(aCellPos, aCell));
            }
            else if( aInfoType.EqualsAscii( "TYPE" ) )
            {   // b = blank; l = string (label); v = otherwise (value)
                sal_Unicode c;
                if (aCell.hasString())
                    c = 'l';
                else
                    c = aCell.hasNumeric() ? 'v' : 'b';
                PushString( OUString(c) );
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
                long nZeroWidth = pPrinter->GetTextWidth( OUString( '0' ) );
                pPrinter->SetFont( aOldFont );
                pPrinter->SetMapMode( aOldMode );
                int nZeroCount = (int)(pDok->GetColWidth( aCellPos.Col(), aCellPos.Tab() ) / nZeroWidth);
                PushInt( nZeroCount );
            }
            else if( aInfoType.EqualsAscii( "PREFIX" ) )
            {   // ' = left; " = right; ^ = centered
                sal_Unicode c = 0;
                if (aCell.hasString())
                {
                    const SvxHorJustifyItem* pJustAttr = (const SvxHorJustifyItem*)
                        pDok->GetAttr( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), ATTR_HOR_JUSTIFY );
                    switch( pJustAttr->GetValue() )
                    {
                        case SVX_HOR_JUSTIFY_STANDARD:
                        case SVX_HOR_JUSTIFY_LEFT:
                        case SVX_HOR_JUSTIFY_BLOCK:     c = '\''; break;
                        case SVX_HOR_JUSTIFY_CENTER:    c = '^';  break;
                        case SVX_HOR_JUSTIFY_RIGHT:     c = '"';  break;
                        case SVX_HOR_JUSTIFY_REPEAT:    c = '\\'; break;
                    }
                }
                PushString( OUString(c) );
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
                OUString aFuncResult;
                sal_uLong   nFormat = pDok->GetNumberFormat( aCellPos );
                getFormatString(pFormatter, nFormat, aFuncResult);
                PushString( aFuncResult );
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
                PushIllegalArgument();
        }
    }
}

void ScInterpreter::ScCellExternal()
{
    sal_uInt16 nFileId;
    String aTabName;
    ScSingleRefData aRef;
    ScExternalRefCache::TokenRef pToken;
    ScExternalRefCache::CellFormat aFmt;
    PopExternalSingleRef(nFileId, aTabName, aRef, pToken, &aFmt);
    if (nGlobalError)
    {
        PushIllegalParameter();
        return;
    }

    OUString aInfoType = GetString();
    if (nGlobalError)
    {
        PushIllegalParameter();
        return;
    }

    SCCOL nCol;
    SCROW nRow;
    SCTAB nTab;
    aRef.nTab = 0; // external ref has a tab index of -1, which SingleRefToVars() don't like.
    SingleRefToVars(aRef, nCol, nRow, nTab);
    if (nGlobalError)
    {
        PushIllegalParameter();
        return;
    }
    aRef.nTab = -1; // revert the value.

    ScCellKeywordTranslator::transKeyword(aInfoType, ScGlobal::GetLocale(), ocCell);
    ScExternalRefManager* pRefMgr = pDok->GetExternalRefManager();

    if ( aInfoType == "COL" )
        PushInt(nCol + 1);
    else if ( aInfoType == "ROW" )
        PushInt(nRow + 1);
    else if ( aInfoType == "SHEET" )
    {
        // For SHEET, No idea what number we should set, but let's always set
        // 1 if the external sheet exists, no matter what sheet.  Excel does
        // the same.
        if (pRefMgr->getCacheTable(nFileId, aTabName, false, NULL).get())
            PushInt(1);
        else
            SetError(errNoName);
    }
    else if ( aInfoType == "ADDRESS" )
    {
        // ODF 1.2 says we need to always display address using the ODF A1 grammar.
        ScTokenArray aArray;
        aArray.AddExternalSingleReference(nFileId, aTabName, aRef);
        ScCompiler aComp(pDok, aPos, aArray);
        aComp.SetGrammar(formula::FormulaGrammar::GRAM_ODFF_A1);
        String aStr;
        aComp.CreateStringFromTokenArray(aStr);
        PushString(aStr);
    }
    else if ( aInfoType == "FILENAME" )
    {
        // 'file URI'#$SheetName

        const OUString* p = pRefMgr->getExternalFileName(nFileId);
        if (!p)
        {
            // In theory this should never happen...
            SetError(errNoName);
            return;
        }

        OUStringBuffer aBuf;
        aBuf.append(sal_Unicode('\''));
        aBuf.append(*p);
        aBuf.appendAscii("'#$");
        aBuf.append(aTabName);
        PushString(aBuf.makeStringAndClear());
    }
    else if ( aInfoType == "CONTENTS" )
    {
        switch (pToken->GetType())
        {
            case svString:
                PushString(pToken->GetString());
            break;
            case svDouble:
                PushString(OUString::valueOf(pToken->GetDouble()));
            break;
            case svError:
                PushString(ScGlobal::GetErrorString(pToken->GetError()));
            break;
            default:
                PushString(ScGlobal::GetEmptyString());
        }
    }
    else if ( aInfoType == "TYPE" )
    {
        sal_Unicode c = 'v';
        switch (pToken->GetType())
        {
            case svString:
                c = 'l';
            break;
            case svEmptyCell:
                c = 'b';
            break;
            default:
                ;
        }
        PushString(OUString(c));
    }
    else if ( aInfoType == "FORMAT" )
    {
        OUString aFmtStr;
        sal_uLong nFmt = aFmt.mbIsSet ? aFmt.mnIndex : 0;
        getFormatString(pFormatter, nFmt, aFmtStr);
        PushString(aFmtStr);
    }
    else if ( aInfoType == "COLOR" )
    {
        // 1 = negative values are colored, otherwise 0
        int nVal = 0;
        if (aFmt.mbIsSet)
        {
            const SvNumberformat* pFormat = pFormatter->GetEntry(aFmt.mnIndex);
            nVal = lcl_FormatHasNegColor(pFormat) ? 1 : 0;
        }
        PushInt(nVal);
    }
    else if ( aInfoType == "PARENTHESES" )
    {
        // 1 = format string contains a '(' character, otherwise 0
        int nVal = 0;
        if (aFmt.mbIsSet)
        {
            const SvNumberformat* pFormat = pFormatter->GetEntry(aFmt.mnIndex);
            nVal = lcl_FormatHasOpenPar(pFormat) ? 1 : 0;
        }
        PushInt(nVal);
    }
    else
        PushIllegalParameter();
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
        case svRefList :
        {
            FormulaTokenRef x = PopToken();
            if ( !nGlobalError )
                nRes = !static_cast<ScToken*>(x.get())->GetRefList()->empty();
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
    switch ( GetRawStackType() )
    {
        case svDouble:
            Pop();
            nRes = 1;
        break;
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;

            ScRefCellValue aCell;
            aCell.assign(*pDok, aAdr);
            if (GetCellErrCode(aCell) == 0)
            {
                switch (aCell.meType)
                {
                    case CELLTYPE_VALUE :
                        nRes = 1;
                        break;
                    case CELLTYPE_FORMULA :
                        nRes = (aCell.mpFormula->IsValue() && !aCell.mpFormula->IsEmpty());
                        break;
                    default:
                        ; // nothing
                }
            }
        }
        break;
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( !pMat )
                ;   // nothing
            else if ( !pJumpMatrix )
            {
                if (pMat->GetErrorIfNotString( 0, 0) == 0)
                    nRes = pMat->IsValue( 0, 0);
            }
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    if (pMat->GetErrorIfNotString( nC, nR) == 0)
                        nRes = pMat->IsValue( nC, nR);
            }
        }
        break;
        default:
            Pop();
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

            nRes = (pDok->GetCellType(aAdr) == CELLTYPE_FORMULA);
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
    OUString aFormula;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;

            ScRefCellValue aCell;
            aCell.assign(*pDok, aAdr);
            switch (aCell.meType)
            {
                case CELLTYPE_FORMULA :
                    aCell.mpFormula->GetFormula(aFormula);
                break;
                default:
                    SetError( NOTAVAILABLE );
            }
        }
        break;
        default:
            Pop();
            SetError( NOTAVAILABLE );
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
            if ( nGlobalError == NOTAVAILABLE )
                nRes = 1;
            else
            {
                ScRefCellValue aCell;
                aCell.assign(*pDok, aAdr);
                sal_uInt16 nErr = GetCellErrCode(aCell);
                nRes = (nErr == NOTAVAILABLE);
            }
        }
        break;
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( !pMat )
                ;   // nothing
            else if ( !pJumpMatrix )
                nRes = (pMat->GetErrorIfNotString( 0, 0) == NOTAVAILABLE);
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    nRes = (pMat->GetErrorIfNotString( nC, nR) == NOTAVAILABLE);
            }
        }
        break;
        default:
            PopError();
            if ( nGlobalError == NOTAVAILABLE )
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
            if ( nGlobalError && nGlobalError != NOTAVAILABLE )
                nRes = 1;
            else
            {
                ScRefCellValue aCell;
                aCell.assign(*pDok, aAdr);
                sal_uInt16 nErr = GetCellErrCode(aCell);
                nRes = (nErr && nErr != NOTAVAILABLE);
            }
        }
        break;
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( nGlobalError || !pMat )
                nRes = ((nGlobalError && nGlobalError != NOTAVAILABLE) || !pMat);
            else if ( !pJumpMatrix )
            {
                sal_uInt16 nErr = pMat->GetErrorIfNotString( 0, 0);
                nRes = (nErr && nErr != NOTAVAILABLE);
            }
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                {
                    sal_uInt16 nErr = pMat->GetErrorIfNotString( nC, nR);
                    nRes = (nErr && nErr != NOTAVAILABLE);
                }
            }
        }
        break;
        default:
            PopError();
            if ( nGlobalError && nGlobalError != NOTAVAILABLE )
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
                ScRefCellValue aCell;
                aCell.assign(*pDok, aAdr);
                nRes = (GetCellErrCode(aCell) != 0);
            }
        }
        break;
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( nGlobalError || !pMat )
                nRes = 1;
            else if ( !pJumpMatrix )
                nRes = (pMat->GetErrorIfNotString( 0, 0) != 0);
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    nRes = (pMat->GetErrorIfNotString( nC, nR) != 0);
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
    double fVal = 0.0;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;

            ScRefCellValue aCell;
            aCell.assign(*pDok, aAdr);
            sal_uInt16 nErr = GetCellErrCode(aCell);
            if (nErr != 0)
                SetError(nErr);
            else
            {
                switch (aCell.meType)
                {
                    case CELLTYPE_VALUE :
                        fVal = GetCellValue(aAdr, aCell);
                        nRes = 1;
                    break;
                    case CELLTYPE_FORMULA :
                        if (aCell.mpFormula->IsValue())
                        {
                            fVal = GetCellValue(aAdr, aCell);
                            nRes = 1;
                        }
                    break;
                    default:
                        ; // nothing
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
                nRes = pMat->IsValue( 0, 0);
                if ( nRes )
                    fVal = pMat->GetDouble( 0, 0);
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
        default:
            ; // nothing
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
    sal_uInt16 nErr = nGlobalError;
    nGlobalError = 0;
    // Temporarily override the ConvertStringToValue() error for
    // GetCellValue() / GetCellValueOrZero()
    sal_uInt16 nSErr = mnStringNoValueError;
    mnStringNoValueError = errCellNoValue;
    double fVal = GetDouble();
    mnStringNoValueError = nSErr;
    if (nErr)
        nGlobalError = nErr;    // preserve previous error if any
    else if (nGlobalError == errCellNoValue)
        nGlobalError = 0;       // reset temporary detection error
    PushDouble(fVal);
}

void ScInterpreter::ScTrim()
{
    // Doesn't only trim but also removes duplicated blanks within!
    String aVal = comphelper::string::strip(GetString(), ' ');
    String aStr;
    const sal_Unicode* p = aVal.GetBuffer();
    const sal_Unicode* const pEnd = p + aVal.Len();
    while ( p < pEnd )
    {
        if ( *p != ' ' || p[-1] != ' ' )    // first can't be ' ', so -1 is fine
            aStr += *p;
        p++;
    }
    PushString( aStr );
}


void ScInterpreter::ScUpper()
{
    String aString = ScGlobal::pCharClass->uppercase(GetString());
    PushString(aString);
}


void ScInterpreter::ScPropper()
{
//2do: what to do with I18N-CJK ?!?
    String aStr( GetString() );
    const xub_StrLen nLen = aStr.Len();
    // #i82487# don't try to write to empty string's BufferAccess
    // (would crash now that the empty string is const)
    if ( nLen > 0 )
    {
        String aUpr( ScGlobal::pCharClass->uppercase( aStr ) );
        String aLwr( ScGlobal::pCharClass->lowercase( aStr ) );
        sal_Unicode* pStr = aStr.GetBufferAccess();
        const sal_Unicode* pUpr = aUpr.GetBuffer();
        const sal_Unicode* pLwr = aLwr.GetBuffer();
        *pStr = *pUpr;
        xub_StrLen nPos = 1;
        while( nPos < nLen )
        {
            OUString aTmpStr( pStr[nPos-1] );
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
    String aString = ScGlobal::pCharClass->lowercase(GetString());
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
            bool bValue = false;
            ScRefCellValue aCell;
            aCell.assign(*pDok, aAdr);
            if (GetCellErrCode(aCell) == 0)
            {
                switch (aCell.meType)
                {
                    case CELLTYPE_VALUE :
                        bValue = true;
                        break;
                    case CELLTYPE_FORMULA :
                        bValue = aCell.mpFormula->IsValue();
                        break;
                    default:
                        ; // nothing
                }
            }
            if ( bValue )
                PushString(EMPTY_OUSTRING);
            else
            {
                // like GetString()
                GetCellString(aTempStr, aCell);
                PushString( aTempStr );
            }
        }
        break;
        case svMatrix:
        case svExternalSingleRef:
        case svExternalDoubleRef:
        {
            double fVal;
            OUString aStr;
            ScMatValType nMatValType = GetDoubleOrStringFromMatrix( fVal, aStr);
            if (ScMatrix::IsValueType( nMatValType))
                PushString( EMPTY_STRING);
            else
                PushString( aStr);
        }
        break;
        case svDouble :
        {
            PopError();
            PushString( EMPTY_STRING );
        }
        break;
        case svString :
            ;   // leave on stack
        break;
        default :
            PushError( errUnknownOpCode);
    }
}


void ScInterpreter::ScValue()
{
    OUString aInputString;
    double fVal;

    switch ( GetRawStackType() )
    {
        case svMissing:
        case svEmptyCell:
            Pop();
            PushInt(0);
            return;
        case svDouble:
            return;     // leave on stack
            //break;

        case svSingleRef:
        case svDoubleRef:
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
            {
                PushInt(0);
                return;
            }
            ScRefCellValue aCell;
            aCell.assign(*pDok, aAdr);
            if (aCell.hasString())
                GetCellString(aInputString, aCell);
            else if (aCell.hasNumeric())
            {
                PushDouble( GetCellValue(aAdr, aCell) );
                return;
            }
            else
            {
                PushDouble(0.0);
                return;
            }
        }
        break;
        case svMatrix:
            {
                ScMatValType nType = GetDoubleOrStringFromMatrix( fVal,
                        aInputString);
                switch (nType)
                {
                    case SC_MATVAL_EMPTY:
                        fVal = 0.0;
                        // fallthru
                    case SC_MATVAL_VALUE:
                    case SC_MATVAL_BOOLEAN:
                        PushDouble( fVal);
                        return;
                        //break;
                    case SC_MATVAL_STRING:
                        // evaluated below
                        break;
                    default:
                        PushIllegalArgument();
                }
            }
            break;
        default:
            aInputString = GetString();
            break;
    }

    sal_uInt32 nFIndex = 0;     // 0 for default locale
    if (pFormatter->IsNumberFormat(aInputString, nFIndex, fVal))
        PushDouble(fVal);
    else
        PushIllegalArgument();
}


// fdo#57180
void ScInterpreter::ScNumberValue()
{

    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 3 ) )
        return;

    OUString aInputString;
    OUString aDecimalSeparator, aGroupSeparator;
    sal_Unicode cDecimalSeparator = 0;

    if ( nParamCount == 3 )
        aGroupSeparator = GetString();

    if ( nParamCount >= 2 )
    {
        aDecimalSeparator = GetString();
        if ( aDecimalSeparator.getLength() == 1  )
            cDecimalSeparator = aDecimalSeparator[ 0 ];
        else
        {
            PushIllegalArgument();  //if given, separator length must be 1
            return;
        }
    }

    if ( cDecimalSeparator && aGroupSeparator.indexOf( cDecimalSeparator ) != -1 )
    {
        PushIllegalArgument(); //decimal separator cannot appear in group separator
        return;
    }

    switch (GetStackType())
    {
        case svDouble:
        return; // leave on stack
        default:
        aInputString = GetString();
    }
    if ( nGlobalError )
    {
        PushError( nGlobalError );
        return;
    }
    if ( aInputString.isEmpty() )
    {
        if ( GetGlobalConfig().mbEmptyStringAsZero )
            PushDouble( 0.0 );
        else
            PushNoValue();
        return;
    }

    sal_Int32 nDecSep = aInputString.indexOf( cDecimalSeparator );
    if ( nDecSep != 0 )
    {
        OUString aTemporary( nDecSep >= 0 ? aInputString.copy( 0, nDecSep ) : aInputString );
        sal_Int32 nIndex = 0;
        do
        {
            sal_uInt32 nChar = aGroupSeparator.iterateCodePoints( &nIndex );
            aTemporary = aTemporary.replaceAll( OUString( &nChar, 1 ), "" );
        } while ( nIndex < aGroupSeparator.getLength() );
        if ( nDecSep >= 0 )
            aInputString = aTemporary + aInputString.copy( nDecSep );
        else
            aInputString = aTemporary;
    }

    for ( sal_Int32 i = aInputString.getLength(); --i >= 0; )
    {
        sal_Unicode c = aInputString[ i ];
        if ( c == 0x0020 || c == 0x0009 || c == 0x000A || c == 0x000D )
            aInputString = aInputString.replaceAt( i, 1, "" ); // remove spaces etc.
    }
    sal_Int32 nPercentCount = 0;
    for ( sal_Int32 i = aInputString.getLength() - 1; i >= 0 && aInputString[ i ] == 0x0025; i-- )
    {
        aInputString = aInputString.replaceAt( i, 1, "" );  // remove and count trailing '%'
        nPercentCount++;
    }

    rtl_math_ConversionStatus eStatus;
    sal_Int32 nParseEnd;
    double fVal = ::rtl::math::stringToDouble( aInputString, cDecimalSeparator, 0, &eStatus, &nParseEnd );
    if ( eStatus == rtl_math_ConversionStatus_Ok && nParseEnd == aInputString.getLength() )
    {
        if (nPercentCount)
            fVal *= pow( 10.0, -(nPercentCount * 2));    // process '%' from input string
        PushDouble(fVal);
        return;
    }
    PushNoValue();
}


//2do: this should be a proper unicode string method
static inline bool lcl_ScInterpreter_IsPrintable( sal_Unicode c )
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
    //"classic" ByteString conversion flags
    const sal_uInt32 convertFlags =
        RTL_UNICODETOTEXT_FLAGS_NONSPACING_IGNORE |
        RTL_UNICODETOTEXT_FLAGS_CONTROL_IGNORE |
        RTL_UNICODETOTEXT_FLAGS_FLUSH |
        RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT |
        RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT |
        RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE;
    PushInt( (sal_uChar) OUStringToOString(OUString(rStr.GetChar(0)), osl_getThreadTextEncoding(), convertFlags).toChar() );
}


void ScInterpreter::ScChar()
{
//2do: make it full range unicode?
    double fVal = GetDouble();
    if (fVal < 0.0 || fVal >= 256.0)
        PushIllegalArgument();
    else
    {
        //"classic" ByteString conversion flags
        const sal_uInt32 convertFlags =
            RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT |
            RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
            RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT;

        sal_Char cEncodedChar = static_cast<sal_Char>(fVal);
        OUString aStr(&cEncodedChar, 1,  osl_getThreadTextEncoding(), convertFlags);
        PushString(aStr);
    }
}


/* #i70213# fullwidth/halfwidth conversion provided by
 * Takashi Nakamoto <bluedwarf@ooo>
 * erAck: added Excel compatibility conversions as seen in issue's test case. */

static OUString lcl_convertIntoHalfWidth( const OUString & rStr )
{
    static bool bFirstASCCall = true;
    static utl::TransliterationWrapper aTrans( ::comphelper::getProcessComponentContext(), 0 );

    if( bFirstASCCall )
    {
        aTrans.loadModuleByImplName( OUString( "FULLWIDTH_HALFWIDTH_LIKE_ASC" ), LANGUAGE_SYSTEM );
        bFirstASCCall = false;
    }

    return aTrans.transliterate( rStr, 0, sal_uInt16( rStr.getLength() ), NULL );
}


static OUString lcl_convertIntoFullWidth( const OUString & rStr )
{
    static bool bFirstJISCall = true;
    static utl::TransliterationWrapper aTrans( ::comphelper::getProcessComponentContext(), 0 );

    if( bFirstJISCall )
    {
        aTrans.loadModuleByImplName( OUString( "HALFWIDTH_FULLWIDTH_LIKE_JIS" ), LANGUAGE_SYSTEM );
        bFirstJISCall = false;
    }

    return aTrans.transliterate( rStr, 0, sal_uInt16( rStr.getLength() ), NULL );
}


/* ODFF:
 * Summary: Converts half-width to full-width ASCII and katakana characters.
 * Semantics: Conversion is done for half-width ASCII and katakana characters,
 * other characters are simply copied from T to the result. This is the
 * complementary function to ASC.
 * For references regarding halfwidth and fullwidth characters see
 * http://www.unicode.org/reports/tr11/
 * http://www.unicode.org/charts/charindex2.html#H
 * http://www.unicode.org/charts/charindex2.html#F
 */
void ScInterpreter::ScJis()
{
    if (MustHaveParamCount( GetByte(), 1))
        PushString( lcl_convertIntoFullWidth( GetString()));
}


/* ODFF:
 * Summary: Converts full-width to half-width ASCII and katakana characters.
 * Semantics: Conversion is done for full-width ASCII and katakana characters,
 * other characters are simply copied from T to the result. This is the
 * complementary function to JIS.
 */
void ScInterpreter::ScAsc()
{
    if (MustHaveParamCount( GetByte(), 1))
        PushString( lcl_convertIntoHalfWidth( GetString()));
}

void ScInterpreter::ScUnicode()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        const OUString& rStr = GetString();
        if (rStr.isEmpty())
            PushIllegalParameter();
        else
        {
            sal_Int32 i = 0;
            PushDouble( rStr.iterateCodePoints(&i) );
        }
    }
}

void ScInterpreter::ScUnichar()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        double dVal = ::rtl::math::approxFloor( GetDouble() );
        if ((dVal < 0x000000) || (dVal > 0x10FFFF))
            PushIllegalArgument();
        else
        {
            sal_uInt32 nCodePoint = static_cast<sal_uInt32>( dVal );
            OUString aStr( &nCodePoint, 1 );
            PushString( aStr );
        }
    }
}


void ScInterpreter::ScMin( bool bTextAsZero )
{
    short nParamCount = GetByte();
    if (!MustHaveParamCountMin( nParamCount, 1))
        return;
    double nMin = ::std::numeric_limits<double>::max();
    double nVal = 0.0;
    ScAddress aAdr;
    ScRange aRange;
    size_t nRefInList = 0;
    while (nParamCount-- > 0)
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
                ScRefCellValue aCell;
                aCell.assign(*pDok, aAdr);
                if (aCell.hasNumeric())
                {
                    nVal = GetCellValue(aAdr, aCell);
                    CurFmtToFuncFmt();
                    if (nMin > nVal) nMin = nVal;
                }
                else if (bTextAsZero && aCell.hasString())
                {
                    if ( nMin > 0.0 )
                        nMin = 0.0;
                }
            }
            break;
            case svDoubleRef :
            case svRefList :
            {
                sal_uInt16 nErr = 0;
                PopDoubleRef( aRange, nParamCount, nRefInList);
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
            case svExternalSingleRef:
            case svExternalDoubleRef:
            {
                ScMatrixRef pMat = GetMatrix();
                if (pMat)
                    nMin = pMat->GetMinValue(bTextAsZero);
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
    if ( nVal < nMin  )
        PushDouble(0.0);
    else
        PushDouble(nMin);
}

void ScInterpreter::ScMax( bool bTextAsZero )
{
    short nParamCount = GetByte();
    if (!MustHaveParamCountMin( nParamCount, 1))
        return;
    double nMax = -(::std::numeric_limits<double>::max());
    double nVal = 0.0;
    ScAddress aAdr;
    ScRange aRange;
    size_t nRefInList = 0;
    while (nParamCount-- > 0)
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
                ScRefCellValue aCell;
                aCell.assign(*pDok, aAdr);
                if (aCell.hasNumeric())
                {
                    nVal = GetCellValue(aAdr, aCell);
                    CurFmtToFuncFmt();
                    if (nMax < nVal) nMax = nVal;
                }
                else if (bTextAsZero && aCell.hasString())
                {
                    if ( nMax < 0.0 )
                        nMax = 0.0;
                }
            }
            break;
            case svDoubleRef :
            case svRefList :
            {
                sal_uInt16 nErr = 0;
                PopDoubleRef( aRange, nParamCount, nRefInList);
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
            case svExternalSingleRef:
            case svExternalDoubleRef:
            {
                ScMatrixRef pMat = GetMatrix();
                if (pMat)
                {
                    nFuncFmtType = NUMBERFORMAT_NUMBER;
                    nMax = pMat->GetMaxValue(bTextAsZero);
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
    if ( nVal > nMax  )
        PushDouble(0.0);
    else
        PushDouble(nMax);
}

void ScInterpreter::GetStVarParams( double& rVal, double& rValCount,
                bool bTextAsZero )
{
    short nParamCount = GetByte();

    std::vector<double> values;
    double fSum    = 0.0;
    double vSum    = 0.0;
    double vMean    = 0.0;
    double fVal = 0.0;
    rValCount = 0.0;
    ScAddress aAdr;
    ScRange aRange;
    size_t nRefInList = 0;
    while (nParamCount-- > 0)
    {
        switch (GetStackType())
        {
            case svDouble :
            {
                fVal = GetDouble();
                values.push_back(fVal);
                fSum    += fVal;
                rValCount++;
            }
            break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScRefCellValue aCell;
                aCell.assign(*pDok, aAdr);
                if (aCell.hasNumeric())
                {
                    fVal = GetCellValue(aAdr, aCell);
                    values.push_back(fVal);
                    fSum += fVal;
                    rValCount++;
                }
                else if (bTextAsZero && aCell.hasString())
                {
                    values.push_back(0.0);
                    rValCount++;
                }
            }
            break;
            case svDoubleRef :
            case svRefList :
            {
                sal_uInt16 nErr = 0;
                PopDoubleRef( aRange, nParamCount, nRefInList);
                ScValueIterator aValIter( pDok, aRange, glSubTotal, bTextAsZero );
                if (aValIter.GetFirst(fVal, nErr))
                {
                    do
                    {
                        values.push_back(fVal);
                        fSum += fVal;
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
                    for (SCSIZE nMatCol = 0; nMatCol < nC; nMatCol++)
                    {
                        for (SCSIZE nMatRow = 0; nMatRow < nR; nMatRow++)
                        {
                            if (!pMat->IsString(nMatCol,nMatRow))
                            {
                                fVal= pMat->GetDouble(nMatCol,nMatRow);
                                values.push_back(fVal);
                                fSum += fVal;
                                rValCount++;
                            }
                            else if ( bTextAsZero )
                            {
                                values.push_back(0.0);
                                rValCount++;
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
                    values.push_back(0.0);
                    rValCount++;
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

    ::std::vector<double>::size_type n = values.size();
    vMean = fSum / n;
    for (::std::vector<double>::size_type i = 0; i < n; i++)
        vSum += ::rtl::math::approxSub( values[i], vMean) * ::rtl::math::approxSub( values[i], vMean);
    rVal = vSum;
}


void ScInterpreter::ScVar( bool bTextAsZero )
{
    double nVal;
    double nValCount;
    GetStVarParams( nVal, nValCount, bTextAsZero );

    if (nValCount <= 1.0)
        PushError( errDivisionByZero );
    else
        PushDouble( nVal / (nValCount - 1.0));
}


void ScInterpreter::ScVarP( bool bTextAsZero )
{
    double nVal;
    double nValCount;
    GetStVarParams( nVal, nValCount, bTextAsZero );

    PushDouble( div( nVal, nValCount));
}


void ScInterpreter::ScStDev( bool bTextAsZero )
{
    double nVal;
    double nValCount;
    GetStVarParams( nVal, nValCount, bTextAsZero );
    if (nValCount <= 1.0)
        PushError( errDivisionByZero );
    else
        PushDouble( sqrt( nVal / (nValCount - 1.0)));
}


void ScInterpreter::ScStDevP( bool bTextAsZero )
{
    double nVal;
    double nValCount;
    GetStVarParams( nVal, nValCount, bTextAsZero );
    if (nValCount == 0.0)
        PushError( errDivisionByZero );
    else
        PushDouble( sqrt( nVal / nValCount));

    /* this was: PushDouble( sqrt( div( nVal, nValCount)));
     *
     * Besides that the special NAN gets lost in the call through sqrt(),
     * unxlngi6.pro then looped back and forth somewhere between div() and
     * ::rtl::math::setNan(). Tests showed that
     *
     *      sqrt( div( 1, 0));
     *
     * produced a loop, but
     *
     *      double f1 = div( 1, 0);
     *      sqrt( f1 );
     *
     * was fine. There seems to be some compiler optimization problem. It does
     * not occur when compiled with debug=t.
     */
}


void ScInterpreter::ScColumns()
{
    sal_uInt8 nParamCount = GetByte();
    sal_uLong nVal = 0;
    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
    while (nParamCount-- > 0)
    {
        switch ( GetStackType() )
        {
            case svSingleRef:
                PopError();
                nVal++;
                break;
            case svDoubleRef:
                PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                nVal += static_cast<sal_uLong>(nTab2 - nTab1 + 1) *
                    static_cast<sal_uLong>(nCol2 - nCol1 + 1);
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
            case svExternalSingleRef:
                PopError();
                nVal++;
            break;
            case svExternalDoubleRef:
            {
                sal_uInt16 nFileId;
                String aTabName;
                ScComplexRefData aRef;
                PopExternalDoubleRef( nFileId, aTabName, aRef);
                ScRange aAbs = aRef.toAbs(aPos);
                nVal += static_cast<sal_uLong>(aAbs.aEnd.Tab() - aAbs.aStart.Tab() + 1) *
                    static_cast<sal_uLong>(aAbs.aEnd.Col() - aAbs.aStart.Col() + 1);
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
    sal_uInt8 nParamCount = GetByte();
    sal_uLong nVal = 0;
    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
    while (nParamCount-- > 0)
    {
        switch ( GetStackType() )
        {
            case svSingleRef:
                PopError();
                nVal++;
                break;
            case svDoubleRef:
                PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                nVal += static_cast<sal_uLong>(nTab2 - nTab1 + 1) *
                    static_cast<sal_uLong>(nRow2 - nRow1 + 1);
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
            case svExternalSingleRef:
                PopError();
                nVal++;
            break;
            case svExternalDoubleRef:
            {
                sal_uInt16 nFileId;
                String aTabName;
                ScComplexRefData aRef;
                PopExternalDoubleRef( nFileId, aTabName, aRef);
                nVal += static_cast<sal_uLong>(aRef.Ref2.nTab - aRef.Ref1.nTab + 1) *
                    static_cast<sal_uLong>(aRef.Ref2.nRow - aRef.Ref1.nRow + 1);
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
    sal_uInt8 nParamCount = GetByte();
    sal_uLong nVal;
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
        while (nParamCount-- > 0)
        {
            switch ( GetStackType() )
            {
                case svSingleRef:
                    PopError();
                    nVal++;
                break;
                case svDoubleRef:
                    PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                    nVal += static_cast<sal_uLong>(nTab2 - nTab1 + 1);
                break;
                case svMatrix:
                    PopError();
                    nVal++;
                break;
                case svExternalSingleRef:
                    PopError();
                    nVal++;
                break;
                case svExternalDoubleRef:
                {
                    sal_uInt16 nFileId;
                    String aTabName;
                    ScComplexRefData aRef;
                    PopExternalDoubleRef( nFileId, aTabName, aRef);
                    nVal += static_cast<sal_uLong>(aRef.Ref2.nTab - aRef.Ref1.nTab + 1);
                }
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
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 0, 1 ) )
    {
        double nVal = 0;
        if (nParamCount == 0)
        {
            nVal = aPos.Col() + 1;
            if (bMatrixFormula)
            {
                SCCOL nCols;
                SCROW nRows;
                pMyFormulaCell->GetMatColsRows( nCols, nRows);
                if (nCols == 0)
                {
                    // Happens if called via ScViewFunc::EnterMatrix()
                    // ScFormulaCell::GetResultDimensions() as of course a
                    // matrix result is not available yet.
                    nCols = 1;
                }
                ScMatrixRef pResMat = GetNewMat( static_cast<SCSIZE>(nCols), 1);
                if (pResMat)
                {
                    for (SCCOL i=0; i < nCols; ++i)
                        pResMat->PutDouble( nVal + i, static_cast<SCSIZE>(i), 0);
                    PushMatrix( pResMat);
                    return;
                }
            }
        }
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
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 0, 1 ) )
    {
        double nVal = 0;
        if (nParamCount == 0)
        {
            nVal = aPos.Row() + 1;
            if (bMatrixFormula)
            {
                SCCOL nCols;
                SCROW nRows;
                pMyFormulaCell->GetMatColsRows( nCols, nRows);
                if (nRows == 0)
                {
                    // Happens if called via ScViewFunc::EnterMatrix()
                    // ScFormulaCell::GetResultDimensions() as of course a
                    // matrix result is not available yet.
                    nRows = 1;
                }
                ScMatrixRef pResMat = GetNewMat( 1, static_cast<SCSIZE>(nRows));
                if (pResMat)
                {
                    for (SCROW i=0; i < nRows; i++)
                        pResMat->PutDouble( nVal + i, 0, static_cast<SCSIZE>(i));
                    PushMatrix( pResMat);
                    return;
                }
            }
        }
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
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 0, 1 ) )
    {
        SCTAB nVal = 0;
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

namespace {

class VectorMatrixAccessor
{
public:
    VectorMatrixAccessor(const ScMatrix& rMat, bool bColVec) :
        mrMat(rMat), mbColVec(bColVec) {}

    bool IsEmpty(SCSIZE i) const
    {
        return mbColVec ? mrMat.IsEmpty(0, i) : mrMat.IsEmpty(i, 0);
    }

    bool IsEmptyPath(SCSIZE i) const
    {
        return mbColVec ? mrMat.IsEmptyPath(0, i) : mrMat.IsEmptyPath(i, 0);
    }

    bool IsValue(SCSIZE i) const
    {
        return mbColVec ? mrMat.IsValue(0, i) : mrMat.IsValue(i, 0);
    }

    bool IsString(SCSIZE i) const
    {
        return mbColVec ? mrMat.IsString(0, i) : mrMat.IsString(i, 0);
    }

    double GetDouble(SCSIZE i) const
    {
        return mbColVec ? mrMat.GetDouble(0, i) : mrMat.GetDouble(i, 0);
    }

    OUString GetString(SCSIZE i) const
    {
        return mbColVec ? mrMat.GetString(0, i) : mrMat.GetString(i, 0);
    }

    SCSIZE GetElementCount() const
    {
        SCSIZE nC, nR;
        mrMat.GetDimensions(nC, nR);
        return mbColVec ? nR : nC;
    }

private:
    const ScMatrix& mrMat;
    bool mbColVec;
};

/** returns -1 when the matrix value is smaller than the query value, 0 when
    they are equal, and 1 when the matrix value is larger than the query
    value. */
static sal_Int32 lcl_CompareMatrix2Query(
    SCSIZE i, const VectorMatrixAccessor& rMat, const ScQueryEntry& rEntry)
{
    if (rMat.IsEmpty(i))
    {
        /* TODO: in case we introduced query for real empty this would have to
         * be changed! */
        return -1;      // empty always less than anything else
    }

    /* FIXME: what is an empty path (result of IF(false;true_path) in
     * comparisons? */

    bool bByString = rEntry.GetQueryItem().meType == ScQueryEntry::ByString;
    if (rMat.IsValue(i))
    {
        if (bByString)
            return -1;  // numeric always less than string

        const double nVal1 = rMat.GetDouble(i);
        const double nVal2 = rEntry.GetQueryItem().mfVal;
        if (nVal1 == nVal2)
            return 0;

        return nVal1 < nVal2 ? -1 : 1;
    }

    if (!bByString)
        return 1;       // string always greater than numeric

    const OUString aStr1 = rMat.GetString(i);
    const OUString& rStr2 = rEntry.GetQueryItem().maString;

    return ScGlobal::GetCollator()->compareString(aStr1, rStr2); // case-insensitive
}

/** returns the last item with the identical value as the original item
    value. */
static void lcl_GetLastMatch( SCSIZE& rIndex, const VectorMatrixAccessor& rMat,
        SCSIZE nMatCount, bool bReverse)
{
    if (rMat.IsValue(rIndex))
    {
        double nVal = rMat.GetDouble(rIndex);
        if (bReverse)
            while (rIndex > 0 && rMat.IsValue(rIndex-1) &&
                    nVal == rMat.GetDouble(rIndex-1))
                --rIndex;
        else
            while (rIndex < nMatCount-1 && rMat.IsValue(rIndex+1) &&
                    nVal == rMat.GetDouble(rIndex+1))
                ++rIndex;
    }
    //! Order of IsEmptyPath, IsEmpty, IsString is significant!
    else if (rMat.IsEmptyPath(rIndex))
    {
        if (bReverse)
            while (rIndex > 0 && rMat.IsEmptyPath(rIndex-1))
                --rIndex;
        else
            while (rIndex < nMatCount-1 && rMat.IsEmptyPath(rIndex+1))
                ++rIndex;
    }
    else if (rMat.IsEmpty(rIndex))
    {
        if (bReverse)
            while (rIndex > 0 && rMat.IsEmpty(rIndex-1))
                --rIndex;
        else
            while (rIndex < nMatCount-1 && rMat.IsEmpty(rIndex+1))
                ++rIndex;
    }
    else if (rMat.IsString(rIndex))
    {
        OUString aStr( rMat.GetString(rIndex));
        if (bReverse)
            while (rIndex > 0 && rMat.IsString(rIndex-1) &&
                    aStr == rMat.GetString(rIndex-1))
                --rIndex;
        else
            while (rIndex < nMatCount-1 && rMat.IsString(rIndex+1) &&
                    aStr == rMat.GetString(rIndex+1))
                ++rIndex;
    }
    else
    {
        OSL_FAIL("lcl_GetLastMatch: unhandled matrix type");
    }
}

}

void ScInterpreter::ScMatch()
{

    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        double fTyp;
        if (nParamCount == 3)
            fTyp = GetDouble();
        else
            fTyp = 1.0;
        SCCOL nCol1 = 0;
        SCROW nRow1 = 0;
        SCTAB nTab1 = 0;
        SCCOL nCol2 = 0;
        SCROW nRow2 = 0;
        SCTAB nTab2 = 0;
        ScMatrixRef pMatSrc = NULL;

        switch (GetStackType())
        {
            case svSingleRef:
                PopSingleRef( nCol1, nRow1, nTab1);
                nCol2 = nCol1;
                nRow2 = nRow1;
                nTab2 = nTab1;
            break;
            case svDoubleRef:
            {
                PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                if (nTab1 != nTab2 || (nCol1 != nCol2 && nRow1 != nRow2))
                {
                    PushIllegalParameter();
                    return;
                }
            }
            break;
            case svMatrix:
            case svExternalDoubleRef:
            {
                if (GetStackType() == svMatrix)
                    pMatSrc = PopMatrix();
                else
                    PopExternalDoubleRef(pMatSrc);

                if (!pMatSrc)
                {
                    PushIllegalParameter();
                    return;
                }
            }
            break;
            default:
                PushIllegalParameter();
                return;
        }

        if (nGlobalError == 0)
        {
            double fVal;
            OUString sStr;
            ScQueryParam rParam;
            rParam.nCol1       = nCol1;
            rParam.nRow1       = nRow1;
            rParam.nCol2       = nCol2;
            rParam.nTab        = nTab1;

            ScQueryEntry& rEntry = rParam.GetEntry(0);
            ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
            rEntry.bDoQuery = true;
            if (fTyp < 0.0)
                rEntry.eOp = SC_GREATER_EQUAL;
            else if (fTyp > 0.0)
                rEntry.eOp = SC_LESS_EQUAL;
            switch ( GetStackType() )
            {
                case svDouble:
                {
                    fVal = GetDouble();
                    rItem.mfVal = fVal;
                    rItem.meType = ScQueryEntry::ByValue;
                }
                break;
                case svString:
                {
                    sStr = GetString();
                    rItem.meType = ScQueryEntry::ByString;
                    rItem.maString = sStr;
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
                    ScRefCellValue aCell;
                    aCell.assign(*pDok, aAdr);
                    if (aCell.hasNumeric())
                    {
                        fVal = GetCellValue(aAdr, aCell);
                        rItem.meType = ScQueryEntry::ByValue;
                        rItem.mfVal = fVal;
                    }
                    else
                    {
                        GetCellString(sStr, aCell);
                        rItem.meType = ScQueryEntry::ByString;
                        rItem.maString = sStr;
                    }
                }
                break;
                case svExternalSingleRef:
                {
                    ScExternalRefCache::TokenRef pToken;
                    PopExternalSingleRef(pToken);
                    if (!pToken)
                    {
                        PushInt(0);
                        return;
                    }
                    if (pToken->GetType() == svDouble)
                    {
                        rItem.meType = ScQueryEntry::ByValue;
                        rItem.mfVal = pToken->GetDouble();
                    }
                    else
                    {
                        rItem.meType = ScQueryEntry::ByString;
                        rItem.maString = pToken->GetString();
                    }
                }
                break;
                case svExternalDoubleRef:
                    // TODO: Implement this.
                    PushIllegalParameter();
                    return;
                break;
                case svMatrix :
                {
                    OUString aStr;
                    ScMatValType nType = GetDoubleOrStringFromMatrix(
                            rItem.mfVal, aStr);
                    rItem.maString = aStr;
                    rItem.meType = ScMatrix::IsNonValueType(nType) ?
                        ScQueryEntry::ByString : ScQueryEntry::ByValue;
                }
                break;
                default:
                {
                    PushIllegalParameter();
                    return;
                }
            }
            if (rItem.meType == ScQueryEntry::ByString)
            {
                bool bIsVBAMode = false;
                if ( pDok )
                    bIsVBAMode = pDok->IsInVBAMode();

                // #TODO handle MSO wildcards
                if ( bIsVBAMode )
                    rParam.bRegExp = false;
                else
                    rParam.bRegExp = MayBeRegExp(rEntry.GetQueryItem().maString, pDok);
            }

            if (pMatSrc) // The source data is matrix array.
            {
                SCSIZE nC, nR;
                pMatSrc->GetDimensions( nC, nR);
                if (nC > 1 && nR > 1)
                {
                    // The source matrix must be a vector.
                    PushIllegalParameter();
                    return;
                }
                SCSIZE nMatCount = (nC == 1) ? nR : nC;
                VectorMatrixAccessor aMatAcc(*pMatSrc, nC == 1);

                // simple serial search for equality mode (source data doesn't
                // need to be sorted).

                if (rEntry.eOp == SC_EQUAL)
                {
                    for (SCSIZE i = 0; i < nMatCount; ++i)
                    {
                        if (lcl_CompareMatrix2Query( i, aMatAcc, rEntry) == 0)
                        {
                            PushDouble(i+1); // found !
                            return;
                        }
                    }
                    PushNA(); // not found
                    return;
                }

                // binary search for non-equality mode (the source data is
                // assumed to be sorted).

                bool bAscOrder = (rEntry.eOp == SC_LESS_EQUAL);
                SCSIZE nFirst = 0, nLast = nMatCount-1, nHitIndex = 0;
                for (SCSIZE nLen = nLast-nFirst; nLen > 0; nLen = nLast-nFirst)
                {
                    SCSIZE nMid = nFirst + nLen/2;
                    sal_Int32 nCmp = lcl_CompareMatrix2Query( nMid, aMatAcc, rEntry);
                    if (nCmp == 0)
                    {
                        // exact match.  find the last item with the same value.
                        lcl_GetLastMatch( nMid, aMatAcc, nMatCount, !bAscOrder);
                        PushDouble( nMid+1);
                        return;
                    }

                    if (nLen == 1) // first and last items are next to each other.
                    {
                        if (nCmp < 0)
                            nHitIndex = bAscOrder ? nLast : nFirst;
                        else
                            nHitIndex = bAscOrder ? nFirst : nLast;
                        break;
                    }

                    if (nCmp < 0)
                    {
                        if (bAscOrder)
                            nFirst = nMid;
                        else
                            nLast = nMid;
                    }
                    else
                    {
                        if (bAscOrder)
                            nLast = nMid;
                        else
                            nFirst = nMid;
                    }
                }

                if (nHitIndex == nMatCount-1) // last item
                {
                    sal_Int32 nCmp = lcl_CompareMatrix2Query( nHitIndex, aMatAcc, rEntry);
                    if ((bAscOrder && nCmp <= 0) || (!bAscOrder && nCmp >= 0))
                    {
                        // either the last item is an exact match or the real
                        // hit is beyond the last item.
                        PushDouble( nHitIndex+1);
                        return;
                    }
                }

                if (nHitIndex > 0) // valid hit must be 2nd item or higher
                {
                    PushDouble( nHitIndex); // non-exact match
                    return;
                }

                PushNA();
                return;
            }

            SCCOLROW nDelta = 0;
            if (nCol1 == nCol2)
            {                                           // search row in column
                rParam.nRow2 = nRow2;
                rEntry.nField = nCol1;
                ScAddress aResultPos( nCol1, nRow1, nTab1);
                if (!LookupQueryWithCache( aResultPos, rParam))
                {
                    PushNA();
                    return;
                }
                nDelta = aResultPos.Row() - nRow1;
            }
            else
            {                                           // search column in row
                SCCOL nC;
                rParam.bByRow = false;
                rParam.nRow2 = nRow1;
                rEntry.nField = nCol1;
                ScQueryCellIterator aCellIter(pDok, nTab1, rParam, false);
                // Advance Entry.nField in Iterator if column changed
                aCellIter.SetAdvanceQueryParamEntryField( true );
                if (fTyp == 0.0)
                {                                       // EQUAL
                    if ( aCellIter.GetFirst() )
                        nC = aCellIter.GetCol();
                    else
                    {
                        PushNA();
                        return;
                    }
                }
                else
                {                                       // <= or >=
                    SCROW nR;
                    if ( !aCellIter.FindEqualOrSortedLastInRange( nC, nR ) )
                    {
                        PushNA();
                        return;
                    }
                }
                nDelta = nC - nCol1;
            }
            PushDouble((double) (nDelta + 1));
        }
        else
            PushIllegalParameter();
    }
}


void ScInterpreter::ScCountEmptyCells()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        sal_uLong nMaxCount = 0, nCount = 0;
        CellType eCellType;
        switch (GetStackType())
        {
            case svSingleRef :
            {
                nMaxCount = 1;
                ScAddress aAdr;
                PopSingleRef( aAdr );
                eCellType = pDok->GetCellType(aAdr);
                if (eCellType != CELLTYPE_NONE)
                    nCount = 1;
            }
            break;
            case svDoubleRef :
            case svRefList :
            {
                ScRange aRange;
                short nParam = 1;
                size_t nRefInList = 0;
                while (nParam-- > 0)
                {
                    PopDoubleRef( aRange, nParam, nRefInList);
                    nMaxCount +=
                        static_cast<sal_uLong>(aRange.aEnd.Row() - aRange.aStart.Row() + 1) *
                        static_cast<sal_uLong>(aRange.aEnd.Col() - aRange.aStart.Col() + 1) *
                        static_cast<sal_uLong>(aRange.aEnd.Tab() - aRange.aStart.Tab() + 1);

                    ScCellIterator aIter( pDok, aRange, glSubTotal);
                    for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
                    {
                        if (!aIter.hasEmptyData())
                            ++nCount;
                    }
                }
            }
            break;
            default : SetError(errIllegalParameter); break;
        }
        PushDouble(nMaxCount - nCount);
    }
}


double ScInterpreter::IterateParametersIf( ScIterFuncIf eFunc )
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return 0;

    SCCOL nCol3 = 0;
    SCROW nRow3 = 0;
    SCTAB nTab3 = 0;

    ScMatrixRef pSumExtraMatrix;
    bool bSumExtraRange = (nParamCount == 3);
    if (bSumExtraRange)
    {
        // Save only the upperleft cell in case of cell range.  The geometry
        // of the 3rd parameter is taken from the 1st parameter.

        switch ( GetStackType() )
        {
            case svDoubleRef :
                {
                    SCCOL nColJunk = 0;
                    SCROW nRowJunk = 0;
                    SCTAB nTabJunk = 0;
                    PopDoubleRef( nCol3, nRow3, nTab3, nColJunk, nRowJunk, nTabJunk );
                    if ( nTabJunk != nTab3 )
                    {
                        SetError( errIllegalParameter);
                        return 0;
                    }
                }
                break;
            case svSingleRef :
                PopSingleRef( nCol3, nRow3, nTab3 );
                break;
            case svMatrix:
                pSumExtraMatrix = PopMatrix();
                //! nCol3, nRow3, nTab3 remain 0
                break;
            case svExternalSingleRef:
                {
                    pSumExtraMatrix = new ScMatrix(1, 1, 0.0);
                    ScExternalRefCache::TokenRef pToken;
                    PopExternalSingleRef(pToken);
                    if (!pToken)
                    {
                        SetError( errIllegalParameter);
                        return 0;
                    }

                    if (pToken->GetType() == svDouble)
                        pSumExtraMatrix->PutDouble(pToken->GetDouble(), 0, 0);
                    else
                        pSumExtraMatrix->PutString(pToken->GetString(), 0, 0);
                }
                break;
            case svExternalDoubleRef:
                PopExternalDoubleRef(pSumExtraMatrix);
                break;
            default:
                SetError( errIllegalParameter);
                return 0;
        }
    }

    OUString aString;
    double fVal = 0.0;
    bool bIsString = true;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
            {
                ScAddress aAdr;
                if ( !PopDoubleRefOrSingleRef( aAdr ) )
                    return 0;

                ScRefCellValue aCell;
                aCell.assign(*pDok, aAdr);
                switch (aCell.meType)
                {
                    case CELLTYPE_VALUE :
                        fVal = GetCellValue(aAdr, aCell);
                        bIsString = false;
                        break;
                    case CELLTYPE_FORMULA :
                        if (aCell.mpFormula->IsValue())
                        {
                            fVal = GetCellValue(aAdr, aCell);
                            bIsString = false;
                        }
                        else
                            GetCellString(aString, aCell);
                        break;
                    case CELLTYPE_STRING :
                    case CELLTYPE_EDIT :
                        GetCellString(aString, aCell);
                        break;
                    default:
                        fVal = 0.0;
                        bIsString = false;
                }
            }
            break;
        case svString:
            aString = GetString();
            break;
        case svMatrix :
        case svExternalDoubleRef:
            {
                ScMatValType nType = GetDoubleOrStringFromMatrix( fVal, aString);
                bIsString = ScMatrix::IsNonValueType( nType);
            }
            break;
        case svExternalSingleRef:
            {
                ScExternalRefCache::TokenRef pToken;
                PopExternalSingleRef(pToken);
                if (pToken)
                {
                    if (pToken->GetType() == svDouble)
                    {
                        fVal = pToken->GetDouble();
                        bIsString = false;
                    }
                    else
                        aString = pToken->GetString();
                }
            }
            break;
        default:
            {
                fVal = GetDouble();
                bIsString = false;
            }
    }

    double fSum = 0.0;
    double fMem = 0.0;
    double fRes = 0.0;
    double fCount = 0.0;
    bool bNull = true;
    short nParam = 1;
    size_t nRefInList = 0;
    while (nParam-- > 0)
    {
        SCCOL nCol1 = 0;
        SCROW nRow1 = 0;
        SCTAB nTab1 = 0;
        SCCOL nCol2 = 0;
        SCROW nRow2 = 0;
        SCTAB nTab2 = 0;
        ScMatrixRef pQueryMatrix;
        switch ( GetStackType() )
        {
            case svRefList :
                if (bSumExtraRange)
                {
                    SetError( errIllegalParameter);
                }
                else
                {
                    ScRange aRange;
                    PopDoubleRef( aRange, nParam, nRefInList);
                    aRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                }
                break;
            case svDoubleRef :
                PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                break;
            case svSingleRef :
                PopSingleRef( nCol1, nRow1, nTab1 );
                nCol2 = nCol1;
                nRow2 = nRow1;
                nTab2 = nTab1;
                break;
            case svMatrix:
            case svExternalSingleRef:
            case svExternalDoubleRef:
                {
                    pQueryMatrix = GetMatrix();
                    if (!pQueryMatrix)
                    {
                        SetError( errIllegalParameter);
                        return 0;
                    }
                    nCol1 = 0;
                    nRow1 = 0;
                    nTab1 = 0;
                    SCSIZE nC, nR;
                    pQueryMatrix->GetDimensions( nC, nR);
                    nCol2 = static_cast<SCCOL>(nC - 1);
                    nRow2 = static_cast<SCROW>(nR - 1);
                    nTab2 = 0;
                }
                break;
            default:
                SetError( errIllegalParameter);
        }
        if ( nTab1 != nTab2 )
        {
            SetError( errIllegalParameter);
        }

        if (bSumExtraRange)
        {
            // Take the range geometry of the 1st parameter and apply it to
            // the 3rd. If parts of the resulting range would point outside
            // the sheet, don't complain but silently ignore and simply cut
            // them away, this is what Xcl does :-/

            // For the cut-away part we also don't need to determine the
            // criteria match, so shrink the source range accordingly,
            // instead of the result range.
            SCCOL nColDelta = nCol2 - nCol1;
            SCROW nRowDelta = nRow2 - nRow1;
            SCCOL nMaxCol;
            SCROW nMaxRow;
            if (pSumExtraMatrix)
            {
                SCSIZE nC, nR;
                pSumExtraMatrix->GetDimensions( nC, nR);
                nMaxCol = static_cast<SCCOL>(nC - 1);
                nMaxRow = static_cast<SCROW>(nR - 1);
            }
            else
            {
                nMaxCol = MAXCOL;
                nMaxRow = MAXROW;
            }
            if (nCol3 + nColDelta > nMaxCol)
            {
                SCCOL nNewDelta = nMaxCol - nCol3;
                nCol2 = nCol1 + nNewDelta;
            }

            if (nRow3 + nRowDelta > nMaxRow)
            {
                SCROW nNewDelta = nMaxRow - nRow3;
                nRow2 = nRow1 + nNewDelta;
            }
        }
        else
        {
            nCol3 = nCol1;
            nRow3 = nRow1;
            nTab3 = nTab1;
        }

        if (nGlobalError == 0)
        {
            ScQueryParam rParam;
            rParam.nRow1       = nRow1;
            rParam.nRow2       = nRow2;

            ScQueryEntry& rEntry = rParam.GetEntry(0);
            ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
            rEntry.bDoQuery = true;
            if (!bIsString)
            {
                rItem.meType = ScQueryEntry::ByValue;
                rItem.mfVal = fVal;
                rEntry.eOp = SC_EQUAL;
            }
            else
            {
                rParam.FillInExcelSyntax(aString, 0);
                sal_uInt32 nIndex = 0;
                bool bNumber = pFormatter->IsNumberFormat(
                        rItem.maString, nIndex, rItem.mfVal);
                rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
                if (rItem.meType == ScQueryEntry::ByString)
                    rParam.bRegExp = MayBeRegExp(rItem.maString, pDok);
            }
            ScAddress aAdr;
            aAdr.SetTab( nTab3 );
            rParam.nCol1  = nCol1;
            rParam.nCol2  = nCol2;
            rEntry.nField = nCol1;
            SCsCOL nColDiff = nCol3 - nCol1;
            SCsROW nRowDiff = nRow3 - nRow1;
            if (pQueryMatrix)
            {
                // Never case-sensitive.
                ScCompareOptions aOptions( pDok, rEntry, rParam.bRegExp);
                ScMatrixRef pResultMatrix = QueryMat( pQueryMatrix, aOptions);
                if (nGlobalError || !pResultMatrix)
                {
                    SetError( errIllegalParameter);
                }

                if (pSumExtraMatrix)
                {
                    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
                    {
                        for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
                        {
                            if (pResultMatrix->IsValue( nCol, nRow) &&
                                    pResultMatrix->GetDouble( nCol, nRow))
                            {
                                SCSIZE nC = nCol + nColDiff;
                                SCSIZE nR = nRow + nRowDiff;
                                if (pSumExtraMatrix->IsValue( nC, nR))
                                {
                                    fVal = pSumExtraMatrix->GetDouble( nC, nR);
                                    ++fCount;
                                    if ( bNull && fVal != 0.0 )
                                    {
                                        bNull = false;
                                        fMem = fVal;
                                    }
                                    else
                                        fSum += fVal;
                                }
                            }
                        }
                    }
                }
                else
                {
                    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
                    {
                        for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
                        {
                            if (pResultMatrix->GetDouble( nCol, nRow))
                            {
                                aAdr.SetCol( nCol + nColDiff);
                                aAdr.SetRow( nRow + nRowDiff);
                                ScRefCellValue aCell;
                                aCell.assign(*pDok, aAdr);
                                if (aCell.hasNumeric())
                                {
                                    fVal = GetCellValue(aAdr, aCell);
                                    ++fCount;
                                    if ( bNull && fVal != 0.0 )
                                    {
                                        bNull = false;
                                        fMem = fVal;
                                    }
                                    else
                                        fSum += fVal;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                ScQueryCellIterator aCellIter(pDok, nTab1, rParam, false);
                // Increment Entry.nField in iterator when switching to next column.
                aCellIter.SetAdvanceQueryParamEntryField( true );
                if ( aCellIter.GetFirst() )
                {
                    if (pSumExtraMatrix)
                    {
                        do
                        {
                            SCSIZE nC = aCellIter.GetCol() + nColDiff;
                            SCSIZE nR = aCellIter.GetRow() + nRowDiff;
                            if (pSumExtraMatrix->IsValue( nC, nR))
                            {
                                fVal = pSumExtraMatrix->GetDouble( nC, nR);
                                ++fCount;
                                if ( bNull && fVal != 0.0 )
                                {
                                    bNull = false;
                                    fMem = fVal;
                                }
                                else
                                    fSum += fVal;
                            }
                        } while ( aCellIter.GetNext() );
                    }
                    else
                    {
                        do
                        {
                            aAdr.SetCol( aCellIter.GetCol() + nColDiff);
                            aAdr.SetRow( aCellIter.GetRow() + nRowDiff);
                            ScRefCellValue aCell;
                            aCell.assign(*pDok, aAdr);
                            if (aCell.hasNumeric())
                            {
                                fVal = GetCellValue(aAdr, aCell);
                                ++fCount;
                                if ( bNull && fVal != 0.0 )
                                {
                                    bNull = false;
                                    fMem = fVal;
                                }
                                else
                                    fSum += fVal;
                            }
                        } while ( aCellIter.GetNext() );
                    }
                }
            }
        }
        else
        {
            SetError( errIllegalParameter);
            return 0;
        }
    }

    switch( eFunc )
    {
        case ifSUMIF:     fRes = ::rtl::math::approxAdd( fSum, fMem ); break;
        case ifAVERAGEIF: fRes = div( ::rtl::math::approxAdd( fSum, fMem ), fCount); break;
    }
    return fRes;
}

void ScInterpreter::ScSumIf()
{
    PushDouble( IterateParametersIf( ifSUMIF));
}

void ScInterpreter::ScAverageIf()
{
    PushDouble( IterateParametersIf( ifAVERAGEIF));
}

void ScInterpreter::ScCountIf()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        OUString aString;
        double fVal = 0.0;
        bool bIsString = true;
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
                ScRefCellValue aCell;
                aCell.assign(*pDok, aAdr);
                switch (aCell.meType)
                {
                    case CELLTYPE_VALUE :
                        fVal = GetCellValue(aAdr, aCell);
                        bIsString = false;
                        break;
                    case CELLTYPE_FORMULA :
                        if (aCell.mpFormula->IsValue())
                        {
                            fVal = GetCellValue(aAdr, aCell);
                            bIsString = false;
                        }
                        else
                            GetCellString(aString, aCell);
                        break;
                    case CELLTYPE_STRING :
                    case CELLTYPE_EDIT :
                        GetCellString(aString, aCell);
                        break;
                    default:
                        fVal = 0.0;
                        bIsString = false;
                }
            }
            break;
            case svMatrix:
            case svExternalSingleRef:
            case svExternalDoubleRef:
            {
                ScMatValType nType = GetDoubleOrStringFromMatrix( fVal,
                        aString);
                bIsString = ScMatrix::IsNonValueType( nType);
            }
            break;
            case svString:
                aString = GetString();
            break;
            default:
            {
                fVal = GetDouble();
                bIsString = false;
            }
        }
        double fCount = 0.0;
        short nParam = 1;
        size_t nRefInList = 0;
        while (nParam-- > 0)
        {
            SCCOL nCol1 = 0;
            SCROW nRow1 = 0;
            SCTAB nTab1 = 0;
            SCCOL nCol2 = 0;
            SCROW nRow2 = 0;
            SCTAB nTab2 = 0;
            ScMatrixRef pQueryMatrix;
            switch ( GetStackType() )
            {
                case svDoubleRef :
                case svRefList :
                    {
                        ScRange aRange;
                        PopDoubleRef( aRange, nParam, nRefInList);
                        aRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                    }
                    break;
                case svSingleRef :
                    PopSingleRef( nCol1, nRow1, nTab1 );
                    nCol2 = nCol1;
                    nRow2 = nRow1;
                    nTab2 = nTab1;
                    break;
                case svMatrix:
                case svExternalSingleRef:
                case svExternalDoubleRef:
                {
                    pQueryMatrix = GetMatrix();
                    if (!pQueryMatrix)
                    {
                        PushIllegalParameter();
                        return;
                    }
                    nCol1 = 0;
                    nRow1 = 0;
                    nTab1 = 0;
                    SCSIZE nC, nR;
                    pQueryMatrix->GetDimensions( nC, nR);
                    nCol2 = static_cast<SCCOL>(nC - 1);
                    nRow2 = static_cast<SCROW>(nR - 1);
                    nTab2 = 0;
                }
                break;
                default:
                    PushIllegalParameter();
                    return ;
            }
            if ( nTab1 != nTab2 )
            {
                PushIllegalParameter();
                return;
            }
            if (nCol1 > nCol2)
            {
                PushIllegalParameter();
                return;
            }
            if (nGlobalError == 0)
            {
                ScQueryParam rParam;
                rParam.nRow1       = nRow1;
                rParam.nRow2       = nRow2;

                ScQueryEntry& rEntry = rParam.GetEntry(0);
                ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
                rEntry.bDoQuery = true;
                if (!bIsString)
                {
                    rItem.meType = ScQueryEntry::ByValue;
                    rItem.mfVal = fVal;
                    rEntry.eOp = SC_EQUAL;
                }
                else
                {
                    rParam.FillInExcelSyntax(aString, 0);
                    sal_uInt32 nIndex = 0;
                    bool bNumber = pFormatter->IsNumberFormat(
                            rItem.maString, nIndex, rItem.mfVal);
                    rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
                    if (rItem.meType == ScQueryEntry::ByString)
                        rParam.bRegExp = MayBeRegExp(rItem.maString, pDok);
                }
                rParam.nCol1  = nCol1;
                rParam.nCol2  = nCol2;
                rEntry.nField = nCol1;
                if (pQueryMatrix)
                {
                    // Never case-sensitive.
                    ScCompareOptions aOptions( pDok, rEntry, rParam.bRegExp);
                    ScMatrixRef pResultMatrix = QueryMat( pQueryMatrix, aOptions);
                    if (nGlobalError || !pResultMatrix)
                    {
                        PushIllegalParameter();
                        return;
                    }

                    SCSIZE nSize = pResultMatrix->GetElementCount();
                    for (SCSIZE nIndex = 0; nIndex < nSize; ++nIndex)
                    {
                        if (pResultMatrix->IsValue( nIndex) &&
                                pResultMatrix->GetDouble( nIndex))
                            ++fCount;
                    }
                }
                else
                {
                    ScQueryCellIterator aCellIter(pDok, nTab1, rParam, false);
                    // Keep Entry.nField in iterator on column change
                    aCellIter.SetAdvanceQueryParamEntryField( true );
                    if ( aCellIter.GetFirst() )
                    {
                        do
                        {
                            fCount++;
                        } while ( aCellIter.GetNext() );
                    }
                }
            }
            else
            {
                PushIllegalParameter();
                return;
            }
        }
        PushDouble(fCount);
    }
}

double ScInterpreter::IterateParametersIfs( ScIterFuncIfs eFunc )
{
    sal_uInt8 nParamCount = GetByte();
    sal_uInt8 nQueryCount = nParamCount / 2;

    bool bCheck;
    if ( eFunc == ifCOUNTIFS )
        bCheck = (nParamCount >= 2) && (nParamCount % 2 == 0);
    else
        bCheck = (nParamCount >= 3) && (nParamCount % 2 == 1);

    if ( !bCheck )
    {
        SetError( errParameterExpected);
    }
    else
    {
        ScMatrixRef pResMat;
        double fVal = 0.0;
        double fSum = 0.0;
        double fMem = 0.0;
        double fRes = 0.0;
        double fCount = 0.0;
        short nParam = 1;
        size_t nRefInList = 0;
        SCCOL nDimensionCols = 0;
        SCROW nDimensionRows = 0;

        while (nParamCount > 1 && !nGlobalError)
        {
            // take criteria
            OUString aString;
            fVal = 0.0;
            bool bIsString = true;
            switch ( GetStackType() )
            {
                case svDoubleRef :
                case svSingleRef :
                    {
                        ScAddress aAdr;
                        if ( !PopDoubleRefOrSingleRef( aAdr ) )
                            return 0;

                        ScRefCellValue aCell;
                        aCell.assign(*pDok, aAdr);
                        switch (aCell.meType)
                        {
                            case CELLTYPE_VALUE :
                                fVal = GetCellValue(aAdr, aCell);
                                bIsString = false;
                                break;
                            case CELLTYPE_FORMULA :
                                if (aCell.mpFormula->IsValue())
                                {
                                    fVal = GetCellValue(aAdr, aCell);
                                    bIsString = false;
                                }
                                else
                                    GetCellString(aString, aCell);
                                break;
                            case CELLTYPE_STRING :
                            case CELLTYPE_EDIT :
                                GetCellString(aString, aCell);
                                break;
                            default:
                                fVal = 0.0;
                                bIsString = false;
                        }
                    }
                    break;
                case svString:
                    aString = GetString();
                    break;
                case svMatrix :
                case svExternalDoubleRef:
                    {
                        ScMatValType nType = GetDoubleOrStringFromMatrix( fVal, aString);
                        bIsString = ScMatrix::IsNonValueType( nType);
                    }
                    break;
                case svExternalSingleRef:
                    {
                        ScExternalRefCache::TokenRef pToken;
                        PopExternalSingleRef(pToken);
                        if (pToken)
                        {
                            if (pToken->GetType() == svDouble)
                            {
                                fVal = pToken->GetDouble();
                                bIsString = false;
                            }
                            else
                                aString = pToken->GetString();
                        }
                    }
                    break;
                default:
                    {
                        fVal = GetDouble();
                        bIsString = false;
                    }
            }

            if (nGlobalError)
                continue;   // and bail out, no need to evaluate other arguments

            // take range
            nParam = 1;
            nRefInList = 0;
            SCCOL nCol1 = 0;
            SCROW nRow1 = 0;
            SCTAB nTab1 = 0;
            SCCOL nCol2 = 0;
            SCROW nRow2 = 0;
            SCTAB nTab2 = 0;
            ScMatrixRef pQueryMatrix;
            switch ( GetStackType() )
            {
                case svRefList :
                    {
                        ScRange aRange;
                        PopDoubleRef( aRange, nParam, nRefInList);
                        aRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                    }
                    break;
                case svDoubleRef :
                    PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                    break;
                case svSingleRef :
                    PopSingleRef( nCol1, nRow1, nTab1 );
                    nCol2 = nCol1;
                    nRow2 = nRow1;
                    nTab2 = nTab1;
                    break;
                case svMatrix:
                case svExternalSingleRef:
                case svExternalDoubleRef:
                    {
                        pQueryMatrix = PopMatrix();
                        if (!pQueryMatrix)
                        {
                            SetError( errIllegalParameter);
                        }
                        nCol1 = 0;
                        nRow1 = 0;
                        nTab1 = 0;
                        SCSIZE nC, nR;
                        pQueryMatrix->GetDimensions( nC, nR);
                        nCol2 = static_cast<SCCOL>(nC - 1);
                        nRow2 = static_cast<SCROW>(nR - 1);
                        nTab2 = 0;
                    }
                    break;
                default:
                    SetError( errIllegalParameter);
            }
            if ( nTab1 != nTab2 )
                SetError( errIllegalArgument);

            // All reference ranges must be of same dimension and size.
            if (!nDimensionCols)
                nDimensionCols = nCol2 - nCol1 + 1;
            if (!nDimensionRows)
                nDimensionRows = nRow2 - nRow1 + 1;
            if ((nDimensionCols != (nCol2 - nCol1 + 1)) || (nDimensionRows != (nRow2 - nRow1 + 1)))
                SetError ( errIllegalArgument);

            // recalculate matrix values
            if (nGlobalError == 0)
            {
                // initialize temporary result matrix
                if (!pResMat)
                {
                    SCSIZE nResC, nResR;
                    nResC = nCol2 - nCol1 + 1;
                    nResR = nRow2 - nRow1 + 1;
                    pResMat = GetNewMat(nResC, nResR);
                    if (!pResMat)
                        SetError( errIllegalParameter);
                    else
                        pResMat->FillDouble( 0.0, 0, 0, nResC-1, nResR-1);
                }

                ScQueryParam rParam;
                rParam.nRow1       = nRow1;
                rParam.nRow2       = nRow2;

                ScQueryEntry& rEntry = rParam.GetEntry(0);
                ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
                rEntry.bDoQuery = true;
                if (!bIsString)
                {
                    rItem.meType = ScQueryEntry::ByValue;
                    rItem.mfVal = fVal;
                    rEntry.eOp = SC_EQUAL;
                }
                else
                {
                    rParam.FillInExcelSyntax(aString, 0);
                    sal_uInt32 nIndex = 0;
                    bool bNumber = pFormatter->IsNumberFormat(
                            rItem.maString, nIndex, rItem.mfVal);
                    rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
                    if (rItem.meType == ScQueryEntry::ByString)
                        rParam.bRegExp = MayBeRegExp(rItem.maString, pDok);
                }
                ScAddress aAdr;
                aAdr.SetTab( nTab1 );
                rParam.nCol1  = nCol1;
                rParam.nCol2  = nCol2;
                rEntry.nField = nCol1;
                SCsCOL nColDiff = -nCol1;
                SCsROW nRowDiff = -nRow1;
                if (pQueryMatrix)
                {
                    // Never case-sensitive.
                    ScCompareOptions aOptions( pDok, rEntry, rParam.bRegExp);
                    ScMatrixRef pResultMatrix = QueryMat( pQueryMatrix, aOptions);
                    if (nGlobalError || !pResultMatrix)
                    {
                        SetError( errIllegalParameter);
                    }

                    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
                    {
                        for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
                        {
                            if (pResultMatrix->IsValue( nCol, nRow) &&
                                    pResultMatrix->GetDouble( nCol, nRow))
                            {
                                SCSIZE nC = nCol + nColDiff;
                                SCSIZE nR = nRow + nRowDiff;
                                pResMat->PutDouble(pResMat->GetDouble(nC, nR)+1.0, nC, nR);
                            }
                        }
                    }
                }
                else
                {
                    ScQueryCellIterator aCellIter(pDok, nTab1, rParam, false);
                    // Increment Entry.nField in iterator when switching to next column.
                    aCellIter.SetAdvanceQueryParamEntryField( true );
                    if ( aCellIter.GetFirst() )
                    {
                        do
                        {
                            SCSIZE nC = aCellIter.GetCol() + nColDiff;
                            SCSIZE nR = aCellIter.GetRow() + nRowDiff;
                            pResMat->PutDouble(pResMat->GetDouble(nC, nR)+1.0, nC, nR);
                        } while ( aCellIter.GetNext() );
                    }
                }
            }
            nParamCount -= 2;
        }

        if (nGlobalError)
            return 0;   // bail out

        // main range - only for AVERAGEIFS and SUMIFS
        if (nParamCount == 1)
        {
            nParam = 1;
            nRefInList = 0;
            bool bNull = true;
            SCCOL nMainCol1 = 0;
            SCROW nMainRow1 = 0;
            SCTAB nMainTab1 = 0;
            SCCOL nMainCol2 = 0;
            SCROW nMainRow2 = 0;
            SCTAB nMainTab2 = 0;
            ScMatrixRef pMainMatrix;
            switch ( GetStackType() )
            {
                case svRefList :
                    {
                        ScRange aRange;
                        PopDoubleRef( aRange, nParam, nRefInList);
                        aRange.GetVars( nMainCol1, nMainRow1, nMainTab1, nMainCol2, nMainRow2, nMainTab2);
                    }
                    break;
                case svDoubleRef :
                    PopDoubleRef( nMainCol1, nMainRow1, nMainTab1, nMainCol2, nMainRow2, nMainTab2 );
                    break;
                case svSingleRef :
                    PopSingleRef( nMainCol1, nMainRow1, nMainTab1 );
                    nMainCol2 = nMainCol1;
                    nMainRow2 = nMainRow1;
                    nMainTab2 = nMainTab1;
                    break;
                case svMatrix:
                case svExternalSingleRef:
                case svExternalDoubleRef:
                    {
                        pMainMatrix = PopMatrix();
                        if (!pMainMatrix)
                        {
                            SetError( errIllegalParameter);
                        }
                        nMainCol1 = 0;
                        nMainRow1 = 0;
                        nMainTab1 = 0;
                        SCSIZE nC, nR;
                        pMainMatrix->GetDimensions( nC, nR);
                        nMainCol2 = static_cast<SCCOL>(nC - 1);
                        nMainRow2 = static_cast<SCROW>(nR - 1);
                        nMainTab2 = 0;
                    }
                    break;
                default:
                    SetError( errIllegalParameter);
            }
            if ( nMainTab1 != nMainTab2 )
                SetError( errIllegalArgument);

            // All reference ranges must be of same dimension and size.
            if ((nDimensionCols != (nMainCol2 - nMainCol1 + 1)) || (nDimensionRows != (nMainRow2 - nMainRow1 + 1)))
                SetError ( errIllegalArgument);

            if (nGlobalError)
                return 0;   // bail out

            // end-result calculation
            ScAddress aAdr;
            aAdr.SetTab( nMainTab1 );
            if (pMainMatrix)
            {
                SCSIZE nC, nR;
                pResMat->GetDimensions(nC, nR);
                for (SCSIZE nCol = 0; nCol < nC; ++nCol)
                {
                    for (SCSIZE nRow = 0; nRow < nR; ++nRow)
                    {
                        if (pResMat->GetDouble( nCol, nRow) == nQueryCount)
                        {
                            if (pMainMatrix->IsValue( nCol, nRow))
                            {
                                fVal = pMainMatrix->GetDouble( nCol, nRow);
                                ++fCount;
                                if ( bNull && fVal != 0.0 )
                                {
                                    bNull = false;
                                    fMem = fVal;
                                }
                                else
                                    fSum += fVal;
                            }
                        }
                    }
                }
            }
            else
            {
                SCSIZE nC, nR;
                pResMat->GetDimensions(nC, nR);
                for (SCSIZE nCol = 0; nCol < nC; ++nCol)
                {
                    for (SCSIZE nRow = 0; nRow < nR; ++nRow)
                    {
                        if (pResMat->GetDouble( nCol, nRow) == nQueryCount)
                        {
                            aAdr.SetCol( static_cast<SCCOL>(nCol) + nMainCol1);
                            aAdr.SetRow( static_cast<SCROW>(nRow) + nMainRow1);
                            ScRefCellValue aCell;
                            aCell.assign(*pDok, aAdr);
                            if (aCell.hasNumeric())
                            {
                                fVal = GetCellValue(aAdr, aCell);
                                ++fCount;
                                if ( bNull && fVal != 0.0 )
                                {
                                    bNull = false;
                                    fMem = fVal;
                                }
                                else
                                    fSum += fVal;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            SCSIZE nC, nR;
            pResMat->GetDimensions(nC, nR);
            for (SCSIZE nCol = 0; nCol < nC; ++nCol)
            {
                for (SCSIZE nRow = 0; nRow < nR; ++nRow)
                    if (pResMat->GetDouble( nCol, nRow) == nQueryCount)
                        ++fCount;
            }
        }

        switch( eFunc )
        {
            case ifSUMIFS:     fRes = ::rtl::math::approxAdd( fSum, fMem ); break;
            case ifAVERAGEIFS: fRes = div( ::rtl::math::approxAdd( fSum, fMem ), fCount); break;
            case ifCOUNTIFS:   fRes = fCount; break;
            default: ; // nothing
        }
        return fRes;
    }
    return 0;
}

void ScInterpreter::ScSumIfs()
{
    PushDouble( IterateParametersIfs( ifSUMIFS));
}

void ScInterpreter::ScAverageIfs()
{
    PushDouble( IterateParametersIfs( ifAVERAGEIFS));
}

void ScInterpreter::ScCountIfs()
{
    PushDouble( IterateParametersIfs( ifCOUNTIFS));
}

void ScInterpreter::ScLookup()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return ;

    ScMatrixRef pDataMat = NULL, pResMat = NULL;
    SCCOL nCol1 = 0, nCol2 = 0, nResCol1 = 0, nResCol2 = 0;
    SCROW nRow1 = 0, nRow2 = 0, nResRow1 = 0, nResRow2 = 0;
    SCTAB nTab1 = 0, nResTab = 0;
    SCSIZE nLenMajor = 0;   // length of major direction
    bool bVertical = true;  // whether to lookup vertically or horizontally

    // The third parameter, result array, for double, string and single reference.
    double fResVal = 0.0;
    String aResStr;
    ScAddress aResAdr;
    StackVar eResArrayType = svUnknown;

    if (nParamCount == 3)
    {
        eResArrayType = GetStackType();
        switch (eResArrayType)
        {
            case svDoubleRef:
            {
                SCTAB nTabJunk;
                PopDoubleRef(nResCol1, nResRow1, nResTab,
                             nResCol2, nResRow2, nTabJunk);
                if (nResTab != nTabJunk ||
                    ((nResRow2 - nResRow1) > 0 && (nResCol2 - nResCol1) > 0))
                {
                    // The result array must be a vector.
                    PushIllegalParameter();
                    return;
                }
            }
            break;
            case svMatrix:
            case svExternalSingleRef:
            case svExternalDoubleRef:
            {
                pResMat = GetMatrix();
                if (!pResMat)
                {
                    PushIllegalParameter();
                    return;
                }
                SCSIZE nC, nR;
                pResMat->GetDimensions(nC, nR);
                if (nC != 1 && nR != 1)
                {
                    // Result matrix must be a vector.
                    PushIllegalParameter();
                    return;
                }
            }
            break;
            case svDouble:
                fResVal = GetDouble();
            break;
            case svString:
                aResStr = GetString();
            break;
            case svSingleRef:
                PopSingleRef( aResAdr );
            break;
            default:
                PushIllegalParameter();
                return;
        }
    }

    // For double, string and single reference.
    double fDataVal = 0.0;
    OUString aDataStr;
    ScAddress aDataAdr;
    bool bValueData = false;

    // Get the data-result range and also determine whether this is vertical
    // lookup or horizontal lookup.

    StackVar eDataArrayType = GetStackType();
    switch (eDataArrayType)
    {
        case svDoubleRef:
        {
            SCTAB nTabJunk;
            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTabJunk);
            if (nTab1 != nTabJunk)
            {
                PushIllegalParameter();
                return;
            }
            bVertical = (nRow2 - nRow1) >= (nCol2 - nCol1);
            nLenMajor = bVertical ? nRow2 - nRow1 + 1 : nCol2 - nCol1 + 1;
        }
        break;
        case svMatrix:
        case svExternalSingleRef:
        case svExternalDoubleRef:
        {
            pDataMat = GetMatrix();
            if (!pDataMat)
            {
                PushIllegalParameter();
                return;
            }

            SCSIZE nC, nR;
            pDataMat->GetDimensions(nC, nR);
            bVertical = (nR >= nC);
            nLenMajor = bVertical ? nR : nC;
        }
        break;
        case svDouble:
        {
            fDataVal = GetDouble();
            bValueData = true;
        }
        break;
        case svString:
        {
            aDataStr = GetString();
        }
        break;
        case svSingleRef:
        {
            PopSingleRef( aDataAdr );
            ScRefCellValue aCell;
            aCell.assign(*pDok, aDataAdr);
            if (aCell.hasEmptyValue())
            {
                // Empty cells aren't found anywhere, bail out early.
                SetError( NOTAVAILABLE);
            }
            else if (aCell.hasNumeric())
            {
                fDataVal = GetCellValue(aDataAdr, aCell);
                bValueData = true;
            }
            else
                GetCellString(aDataStr, aCell);
        }
        break;
        default:
            SetError( errIllegalParameter);
    }


    if (nGlobalError)
    {
        PushError( nGlobalError);
        return;
    }

    // Get the lookup value.

    ScQueryParam aParam;
    ScQueryEntry& rEntry = aParam.GetEntry(0);
    if ( !FillEntry(rEntry) )
        return;

    if ( eDataArrayType == svDouble || eDataArrayType == svString ||
            eDataArrayType == svSingleRef )
    {
        // Delta position for a single value is always 0.

        // Found if data <= query, but not if query is string and found data is
        // numeric or vice versa. This is how Excel does it but doesn't
        // document it.

        bool bFound = false;
        ScQueryEntry::Item& rItem = rEntry.GetQueryItem();

        if ( bValueData )
        {
            if (rItem.meType == ScQueryEntry::ByString)
                bFound = false;
            else
                bFound = (fDataVal <= rItem.mfVal);
        }
        else
        {
            if (rItem.meType != ScQueryEntry::ByString)
                bFound = false;
            else
                bFound = (ScGlobal::GetCollator()->compareString(aDataStr, rItem.maString) <= 0);
        }

        if (!bFound)
        {
            PushNA();
            return;
        }

        if (pResMat)
        {
            if (pResMat->IsValue( 0, 0 ))
                PushDouble(pResMat->GetDouble( 0, 0 ));
            else
                PushString(pResMat->GetString( 0, 0 ));
        }
        else if (nParamCount == 3)
        {
            switch (eResArrayType)
            {
                case svDouble:
                    PushDouble( fResVal );
                    break;
                case svString:
                    PushString( aResStr );
                    break;
                case svDoubleRef:
                    aResAdr.Set( nResCol1, nResRow1, nResTab);
                    // fallthru
                case svSingleRef:
                    PushCellResultToken( true, aResAdr, NULL, NULL);
                    break;
                default:
                    OSL_FAIL( "ScInterpreter::ScLookup: unhandled eResArrayType, single value data");
            }
        }
        else
        {
            switch (eDataArrayType)
            {
                case svDouble:
                    PushDouble( fDataVal );
                    break;
                case svString:
                    PushString( aDataStr );
                    break;
                case svSingleRef:
                    PushCellResultToken( true, aDataAdr, NULL, NULL);
                    break;
                default:
                    OSL_FAIL( "ScInterpreter::ScLookup: unhandled eDataArrayType, single value data");
            }
        }
        return;
    }

    // Now, perform the search to compute the delta position (nDelta).

    if (pDataMat)
    {
        // Data array is given as a matrix.
        rEntry.bDoQuery = true;
        rEntry.eOp = SC_LESS_EQUAL;
        bool bFound = false;

        SCSIZE nC, nR;
        pDataMat->GetDimensions(nC, nR);

        // In case of non-vector matrix, only search the first row or column.
        ScMatrixRef pDataMat2;
        if (bVertical)
        {
            ScMatrixRef pTempMat(new ScMatrix(1, nR, 0.0));
            for (SCSIZE i = 0; i < nR; ++i)
                if (pDataMat->IsValue(0, i))
                    pTempMat->PutDouble(pDataMat->GetDouble(0, i), 0, i);
                else
                    pTempMat->PutString(pDataMat->GetString(0, i), 0, i);
            pDataMat2 = pTempMat;
        }
        else
        {
            ScMatrixRef pTempMat(new ScMatrix(nC, 1, 0.0));
            for (SCSIZE i = 0; i < nC; ++i)
                if (pDataMat->IsValue(i, 0))
                    pTempMat->PutDouble(pDataMat->GetDouble(i, 0), i, 0);
                else
                    pTempMat->PutString(pDataMat->GetString(i, 0), i, 0);
            pDataMat2 = pTempMat;
        }

        VectorMatrixAccessor aMatAcc2(*pDataMat2, bVertical);

        // binary search for non-equality mode (the source data is
        // assumed to be sorted in ascending order).

        SCCOLROW nDelta = -1;

        SCSIZE nFirst = 0, nLast = nLenMajor-1; //, nHitIndex = 0;
        for (SCSIZE nLen = nLast-nFirst; nLen > 0; nLen = nLast-nFirst)
        {
            SCSIZE nMid = nFirst + nLen/2;
            sal_Int32 nCmp = lcl_CompareMatrix2Query( nMid, aMatAcc2, rEntry);
            if (nCmp == 0)
            {
                // exact match.  find the last item with the same value.
                lcl_GetLastMatch( nMid, aMatAcc2, nLenMajor, false);
                nDelta = nMid;
                bFound = true;
                break;
            }

            if (nLen == 1) // first and last items are next to each other.
            {
                nDelta = nCmp < 0 ? nLast - 1 : nFirst - 1;
                // If already the 1st item is greater there's nothing found.
                bFound = (nDelta >= 0);
                break;
            }

            if (nCmp < 0)
                nFirst = nMid;
            else
                nLast = nMid;
        }

        if (nDelta == static_cast<SCCOLROW>(nLenMajor-2)) // last item
        {
            sal_Int32 nCmp = lcl_CompareMatrix2Query(nDelta+1, aMatAcc2, rEntry);
            if (nCmp <= 0)
            {
                // either the last item is an exact match or the real
                // hit is beyond the last item.
                nDelta += 1;
                bFound = true;
            }
        }
        else if (nDelta > 0) // valid hit must be 2nd item or higher
        {
            // non-exact match
            bFound = true;
        }

        // With 0-9 < A-Z, if query is numeric and data found is string, or
        // vice versa, the (yet another undocumented) Excel behavior is to
        // return #N/A instead.

        if (bFound)
        {
            VectorMatrixAccessor aMatAcc(*pDataMat, bVertical);
            SCCOLROW i = nDelta;
            SCSIZE n = aMatAcc.GetElementCount();
            if (static_cast<SCSIZE>(i) >= n)
                i = static_cast<SCCOLROW>(n);
            bool bByString = rEntry.GetQueryItem().meType == ScQueryEntry::ByString;
            if (bByString == aMatAcc.IsValue(i))
                bFound = false;
        }

        if (!bFound)
        {
            PushNA();
            return;
        }

        // Now that we've found the delta, push the result back to the cell.

        if (pResMat)
        {
            VectorMatrixAccessor aResMatAcc(*pResMat, bVertical);
            // result array is matrix.
            if (static_cast<SCSIZE>(nDelta) >= aResMatAcc.GetElementCount())
            {
                PushNA();
                return;
            }
            if (aResMatAcc.IsValue(nDelta))
                PushDouble(aResMatAcc.GetDouble(nDelta));
            else
                PushString(aResMatAcc.GetString(nDelta));
        }
        else if (nParamCount == 3)
        {
            // result array is cell range.
            ScAddress aAdr;
            aAdr.SetTab(nResTab);
            bool bResVertical = (nResRow2 - nResRow1) > 0;
            if (bResVertical)
            {
                SCROW nTempRow = static_cast<SCROW>(nResRow1 + nDelta);
                if (nTempRow > MAXROW)
                {
                    PushDouble(0);
                    return;
                }
                aAdr.SetCol(nResCol1);
                aAdr.SetRow(nTempRow);
            }
            else
            {
                SCCOL nTempCol = static_cast<SCCOL>(nResCol1 + nDelta);
                if (nTempCol > MAXCOL)
                {
                    PushDouble(0);
                    return;
                }
                aAdr.SetCol(nTempCol);
                aAdr.SetRow(nResRow1);
            }
            PushCellResultToken(true, aAdr, NULL, NULL);
        }
        else
        {
            // no result array.  Use the data array to get the final value from.
            if (bVertical)
            {
                if (pDataMat->IsValue(nC-1, nDelta))
                    PushDouble(pDataMat->GetDouble(nC-1, nDelta));
                else
                    PushString(pDataMat->GetString(nC-1, nDelta));
            }
            else
            {
                if (pDataMat->IsValue(nDelta, nR-1))
                    PushDouble(pDataMat->GetDouble(nDelta, nR-1));
                else
                    PushString(pDataMat->GetString(nDelta, nR-1));
            }
        }

        return;
    }

    // Perform cell range search.

    aParam.nCol1            = nCol1;
    aParam.nRow1            = nRow1;
    aParam.nCol2            = bVertical ? nCol1 : nCol2;
    aParam.nRow2            = bVertical ? nRow2 : nRow1;
    aParam.bByRow           = bVertical;

    rEntry.bDoQuery = true;
    rEntry.eOp = SC_LESS_EQUAL;
    rEntry.nField = nCol1;
    ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
    if (rItem.meType == ScQueryEntry::ByString)
        aParam.bRegExp = MayBeRegExp(rItem.maString, pDok);

    ScQueryCellIterator aCellIter(pDok, nTab1, aParam, false);
    SCCOL nC;
    SCROW nR;
    // Advance Entry.nField in iterator upon switching columns if
    // lookup in row.
    aCellIter.SetAdvanceQueryParamEntryField(!bVertical);
    if ( !aCellIter.FindEqualOrSortedLastInRange(nC, nR) )
    {
        PushNA();
        return;
    }

    SCCOLROW nDelta = bVertical ? static_cast<SCSIZE>(nR-nRow1) : static_cast<SCSIZE>(nC-nCol1);

    if (pResMat)
    {
        VectorMatrixAccessor aResMatAcc(*pResMat, bVertical);
        // Use the matrix result array.
        if (aResMatAcc.IsValue(nDelta))
            PushDouble(aResMatAcc.GetDouble(nDelta));
        else
            PushString(aResMatAcc.GetString(nDelta));
    }
    else if (nParamCount == 3)
    {
        switch (eResArrayType)
        {
            case svDoubleRef:
            {
                // Use the result array vector.  Note that the result array is assumed
                // to be a vector (i.e. 1-dimensinoal array).

                ScAddress aAdr;
                aAdr.SetTab(nResTab);
                bool bResVertical = (nResRow2 - nResRow1) > 0;
                if (bResVertical)
                {
                    SCROW nTempRow = static_cast<SCROW>(nResRow1 + nDelta);
                    if (nTempRow > MAXROW)
                    {
                        PushDouble(0);
                        return;
                    }
                    aAdr.SetCol(nResCol1);
                    aAdr.SetRow(nTempRow);
                }
                else
                {
                    SCCOL nTempCol = static_cast<SCCOL>(nResCol1 + nDelta);
                    if (nTempCol > MAXCOL)
                    {
                        PushDouble(0);
                        return;
                    }
                    aAdr.SetCol(nTempCol);
                    aAdr.SetRow(nResRow1);
                }
                PushCellResultToken( true, aAdr, NULL, NULL);
            }
            break;
            case svDouble:
            case svString:
            case svSingleRef:
            {
                if (nDelta != 0)
                    PushNA();
                else
                {
                    switch (eResArrayType)
                    {
                        case svDouble:
                            PushDouble( fResVal );
                            break;
                        case svString:
                            PushString( aResStr );
                            break;
                        case svSingleRef:
                            PushCellResultToken( true, aResAdr, NULL, NULL);
                            break;
                        default:
                            ;   // nothing
                    }
                }
            }
            break;
            default:
                OSL_FAIL( "ScInterpreter::ScLookup: unhandled eResArrayType, range search");
        }
    }
    else
    {
        // Regardless of whether or not the result array exists, the last
        // array is always used as the "result" array.

        ScAddress aAdr;
        aAdr.SetTab(nTab1);
        if (bVertical)
        {
            SCROW nTempRow = static_cast<SCROW>(nRow1 + nDelta);
            if (nTempRow > MAXROW)
            {
                PushDouble(0);
                return;
            }
            aAdr.SetCol(nCol2);
            aAdr.SetRow(nTempRow);
        }
        else
        {
            SCCOL nTempCol = static_cast<SCCOL>(nCol1 + nDelta);
            if (nTempCol > MAXCOL)
            {
                PushDouble(0);
                return;
            }
            aAdr.SetCol(nTempCol);
            aAdr.SetRow(nRow2);
        }
        PushCellResultToken(true, aAdr, NULL, NULL);
    }
}


void ScInterpreter::ScHLookup()
{
    CalculateLookup(true);
}
void ScInterpreter::CalculateLookup(bool HLookup)
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 3, 4 ) )
    {
        bool bSorted;
        if (nParamCount == 4)
            bSorted = GetBool();
        else
            bSorted = true;
        double fIndex = ::rtl::math::approxFloor( GetDouble() ) - 1.0;
        ScMatrixRef pMat = NULL;
        SCSIZE nC = 0, nR = 0;
        SCCOL nCol1 = 0;
        SCROW nRow1 = 0;
        SCTAB nTab1 = 0;
        SCCOL nCol2 = 0;
        SCROW nRow2 = 0;
        SCTAB nTab2;
        StackVar eType = GetStackType();
        if (eType == svDoubleRef)
        {
            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            if (nTab1 != nTab2)
            {
                PushIllegalParameter();
                return;
            }
        }
        else if (eType == svSingleRef)
        {
            PopSingleRef(nCol1, nRow1, nTab1);
            nCol2 = nCol1;
            nRow2 = nRow1;
        }
        else if (eType == svMatrix || eType == svExternalDoubleRef || eType == svExternalSingleRef)
        {
            pMat = GetMatrix();

            if (pMat)
                pMat->GetDimensions(nC, nR);
            else
            {
                PushIllegalParameter();
                return;
            }
        }
        else
        {
            PushIllegalParameter();
            return;
        }
        if ( fIndex < 0.0 || (HLookup ? (pMat ? (fIndex >= nR) : (fIndex+nRow1 > nRow2)) : (pMat ? (fIndex >= nC) : (fIndex+nCol1 > nCol2)) ) )
        {
            PushIllegalArgument();
            return;
        }
        SCROW nZIndex = static_cast<SCROW>(fIndex);
        SCCOL nSpIndex = static_cast<SCCOL>(fIndex);

        if (!pMat)
        {
            nZIndex += nRow1;                       // Wertzeile
            nSpIndex = sal::static_int_cast<SCCOL>( nSpIndex + nCol1 );     // value column
        }

        if (nGlobalError == 0)
        {
            ScQueryParam rParam;
            rParam.nCol1       = nCol1;
            rParam.nRow1       = nRow1;
            if ( HLookup )
            {
                rParam.nCol2       = nCol2;
                rParam.nRow2       = nRow1;     // nur in der ersten Zeile suchen
                rParam.bByRow      = false;
            } // if ( HLookup )
            else
            {
                rParam.nCol2       = nCol1;     // nur in der ersten Spalte suchen
                rParam.nRow2       = nRow2;
                rParam.nTab        = nTab1;
            }

            ScQueryEntry& rEntry = rParam.GetEntry(0);
            rEntry.bDoQuery = true;
            if ( bSorted )
                rEntry.eOp = SC_LESS_EQUAL;
            if ( !FillEntry(rEntry) )
                return;

            ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
            if (rItem.meType == ScQueryEntry::ByString)
                rParam.bRegExp = MayBeRegExp(rItem.maString, pDok);
            if (pMat)
            {
                SCSIZE nMatCount = HLookup ? nC : nR;
                SCSIZE nDelta = SCSIZE_MAX;
                if (rItem.meType == ScQueryEntry::ByString)
                {
        //!!!!!!!
        //! TODO: enable regex on matrix strings
        //!!!!!!!
                    const OUString& rParamStr = rItem.maString;
                    if ( bSorted )
                    {
                        static CollatorWrapper* pCollator = ScGlobal::GetCollator();
                        for (SCSIZE i = 0; i < nMatCount; i++)
                        {
                            if (HLookup ? pMat->IsString(i, 0) : pMat->IsString(0, i))
                            {
                                sal_Int32 nRes =
                                    pCollator->compareString( HLookup ? pMat->GetString(i,0) : pMat->GetString(0,i), rParamStr);
                                if (nRes <= 0)
                                    nDelta = i;
                                else if (i>0)   // #i2168# ignore first mismatch
                                    i = nMatCount+1;
                            }
                            else
                                nDelta = i;
                        }
                    }
                    else
                    {
                        for (SCSIZE i = 0; i < nMatCount; i++)
                        {
                            if (HLookup ? pMat->IsString(i, 0) : pMat->IsString(0, i))
                            {
                                if ( ScGlobal::GetpTransliteration()->isEqual(
                                    HLookup ? pMat->GetString(i,0) : pMat->GetString(0,i), rParamStr))
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
                    if ( bSorted )
                    {
                        // #i2168# ignore strings
                        for (SCSIZE i = 0; i < nMatCount; i++)
                        {
                            if (!(HLookup ? pMat->IsString(i, 0) : pMat->IsString(0, i)))
                            {
                                if ((HLookup ? pMat->GetDouble(i,0) : pMat->GetDouble(0,i)) <= rItem.mfVal)
                                    nDelta = i;
                                else
                                    i = nMatCount+1;
                            }
                        }
                    }
                    else
                    {
                        for (SCSIZE i = 0; i < nMatCount; i++)
                        {
                            if (!(HLookup ? pMat->IsString(i, 0) : pMat->IsString(0, i)))
                            {
                                if ((HLookup ? pMat->GetDouble(i,0) : pMat->GetDouble(0,i)) == rItem.mfVal)
                                {
                                    nDelta = i;
                                    i = nMatCount + 1;
                                }
                            }
                        }
                    }
                }
                if ( nDelta != SCSIZE_MAX )
                {
                    SCSIZE nX = static_cast<SCSIZE>(nSpIndex);
                    SCSIZE nY = nDelta;
                    if ( HLookup )
                    {
                        nX = nDelta;
                        nY = static_cast<SCSIZE>(nZIndex);
                    }
                    if ( pMat->IsString( nX, nY) )
                        PushString(pMat->GetString( nX,nY));
                    else
                        PushDouble(pMat->GetDouble( nX,nY));
                }
                else
                    PushNA();
            }
            else
            {
                rEntry.nField = nCol1;
                bool bFound = false;
                SCCOL nCol = 0;
                SCROW nRow = 0;
                if ( bSorted )
                    rEntry.eOp = SC_LESS_EQUAL;
                if ( HLookup )
                {
                    ScQueryCellIterator aCellIter(pDok, nTab1, rParam, false);
                    // advance Entry.nField in Iterator upon switching columns
                    aCellIter.SetAdvanceQueryParamEntryField( true );
                    if ( bSorted )
                    {
                        SCROW nRow1_temp;
                        bFound = aCellIter.FindEqualOrSortedLastInRange( nCol, nRow1_temp );
                    }
                    else if ( aCellIter.GetFirst() )
                    {
                        bFound = true;
                        nCol = aCellIter.GetCol();
                    }
                    nRow = nZIndex;
                } // if ( HLookup )
                else
                {
                    ScAddress aResultPos( nCol1, nRow1, nTab1);
                    bFound = LookupQueryWithCache( aResultPos, rParam);
                    nRow = aResultPos.Row();
                    nCol = nSpIndex;
                }
                if ( bFound )
                {
                    ScAddress aAdr( nCol, nRow, nTab1 );
                    PushCellResultToken( true, aAdr, NULL, NULL);
                }
                else
                    PushNA();
            }
        }
        else
            PushIllegalParameter();
    }
}

bool ScInterpreter::FillEntry(ScQueryEntry& rEntry)
{
    ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
    switch ( GetStackType() )
    {
        case svDouble:
        {
            rItem.meType = ScQueryEntry::ByValue;
            rItem.mfVal = GetDouble();
        }
        break;
        case svString:
        {
            const String& sStr = GetString();
            rItem.meType = ScQueryEntry::ByString;
            rItem.maString = sStr;
        }
        break;
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
            {
                PushInt(0);
                return false;
            }
            ScRefCellValue aCell;
            aCell.assign(*pDok, aAdr);
            if (aCell.hasNumeric())
            {
                rItem.meType = ScQueryEntry::ByValue;
                rItem.mfVal = GetCellValue(aAdr, aCell);
            }
            else
            {
                OUString aStr;
                GetCellString(aStr, aCell);
                rItem.meType = ScQueryEntry::ByString;
                rItem.maString = aStr;
            }
        }
        break;
        case svMatrix :
        {
            OUString aStr;
            const ScMatValType nType = GetDoubleOrStringFromMatrix(rItem.mfVal, aStr);
            rItem.maString = aStr;
            rItem.meType = ScMatrix::IsNonValueType(nType) ?
                ScQueryEntry::ByString : ScQueryEntry::ByValue;
        }
        break;
        default:
        {
            PushIllegalParameter();
            return false;
        }
    } // switch ( GetStackType() )
    return true;
}
void ScInterpreter::ScVLookup()
{
    CalculateLookup(false);
}

void ScInterpreter::ScSubTotal()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCountMin( nParamCount, 2 ) )
    {
        // We must fish the 1st parameter deep from the stack! And push it on top.
        const FormulaToken* p = pStack[ sp - nParamCount ];
        PushTempToken( *p );
        int nFunc = (int) ::rtl::math::approxFloor( GetDouble() );
        bool bIncludeHidden = true;
        if (nFunc > 100)
        {
            // For opcodes 101 through 111, we need to skip hidden cells.
            // Other than that these opcodes are identical to 1 through 11.
            bIncludeHidden = false;
            nFunc -= 100;
        }

        if (nFunc < 1 || nFunc > 11 || !bIncludeHidden)
            PushIllegalArgument();  // simulate return on stack, not SetError(...)
        else
        {
            // TODO: Make use of bIncludeHidden flag. Then it's false, we do need to skip hidden cells.
            cPar = nParamCount - 1;
            glSubTotal = true;
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
                default : PushIllegalArgument();       break;
            }
            glSubTotal = false;
        }
        // Get rid of the 1st (fished) parameter.
        double nVal = GetDouble();
        Pop();
        PushDouble( nVal );
    }
}

ScDBQueryParamBase* ScInterpreter::GetDBParams( bool& rMissingField )
{
    bool bAllowMissingField = false;
    if ( rMissingField )
    {
        bAllowMissingField = true;
        rMissingField = false;
    }
    if ( GetByte() == 3 )
    {
        // First, get the query criteria range.
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<ScDBRangeBase> pQueryRef( PopDBDoubleRef() );
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if (!pQueryRef.get())
            return NULL;

        bool    bByVal = true;
        double  nVal = 0.0;
        OUString  aStr;
        ScRange aMissingRange;
        bool bRangeFake = false;
        switch (GetStackType())
        {
            case svDouble :
                nVal = ::rtl::math::approxFloor( GetDouble() );
                if ( bAllowMissingField && nVal == 0.0 )
                    rMissingField = true;   // fake missing parameter
                break;
            case svString :
                bByVal = false;
                aStr = GetString();
                break;
            case svSingleRef :
                {
                    ScAddress aAdr;
                    PopSingleRef( aAdr );
                    ScRefCellValue aCell;
                    aCell.assign(*pDok, aAdr);
                    if (aCell.hasNumeric())
                        nVal = GetCellValue(aAdr, aCell);
                    else
                    {
                        bByVal = false;
                        GetCellString(aStr, aCell);
                    }
                }
                break;
            case svDoubleRef :
                if ( bAllowMissingField )
                {   // fake missing parameter for old SO compatibility
                    bRangeFake = true;
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
                    rMissingField = true;
                else
                    SetError( errIllegalParameter );
                break;
            default:
                PopError();
                SetError( errIllegalParameter );
        }

        if (nGlobalError)
            return NULL;

        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        auto_ptr<ScDBRangeBase> pDBRef( PopDBDoubleRef() );
        SAL_WNODEPRECATED_DECLARATIONS_POP

        if (nGlobalError || !pDBRef.get())
            return NULL;

        if ( bRangeFake )
        {
            // range parameter must match entire database range
            if (pDBRef->isRangeEqual(aMissingRange))
                rMissingField = true;
            else
                SetError( errIllegalParameter );
        }

        if (nGlobalError)
            return NULL;

        SCCOL nField = pDBRef->getFirstFieldColumn();
        if (rMissingField)
            ; // special case
        else if (bByVal)
            nField = pDBRef->findFieldColumn(static_cast<SCCOL>(nVal));
        else
        {
            sal_uInt16 nErr = 0;
            nField = pDBRef->findFieldColumn(aStr, &nErr);
            SetError(nErr);
        }

        if (!ValidCol(nField))
            return NULL;

        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        auto_ptr<ScDBQueryParamBase> pParam( pDBRef->createQueryParam(pQueryRef.get()) );
        SAL_WNODEPRECATED_DECLARATIONS_POP

        if (pParam.get())
        {
            // An allowed missing field parameter sets the result field
            // to any of the query fields, just to be able to return
            // some cell from the iterator.
            if ( rMissingField )
                nField = static_cast<SCCOL>(pParam->GetEntry(0).nField);
            pParam->mnField = nField;

            SCSIZE nCount = pParam->GetEntryCount();
            for ( SCSIZE i=0; i < nCount; i++ )
            {
                ScQueryEntry& rEntry = pParam->GetEntry(i);
                if (!rEntry.bDoQuery)
                    break;

                ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
                sal_uInt32 nIndex = 0;
                const OUString& rQueryStr = rItem.maString;
                bool bNumber = pFormatter->IsNumberFormat(
                    rQueryStr, nIndex, rItem.mfVal);
                rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;

                if (!bNumber && !pParam->bRegExp)
                    pParam->bRegExp = MayBeRegExp(rQueryStr, pDok);
            }
            return pParam.release();
        }
    }
    return NULL;
}


void ScInterpreter::DBIterator( ScIterFunc eFunc )
{
    double nErg = 0.0;
    double fMem = 0.0;
    bool bNull = true;
    sal_uLong nCount = 0;
    bool bMissingField = false;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    auto_ptr<ScDBQueryParamBase> pQueryParam( GetDBParams(bMissingField) );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (pQueryParam.get())
    {
        if (!pQueryParam->IsValidFieldIndex())
        {
            SetError(errNoValue);
            return;
        }
        ScDBQueryDataIterator aValIter(pDok, pQueryParam.release());
        ScDBQueryDataIterator::Value aValue;
        if ( aValIter.GetFirst(aValue) && !aValue.mnError )
        {
            switch( eFunc )
            {
                case ifPRODUCT: nErg = 1; break;
                case ifMAX:     nErg = -MAXDOUBLE; break;
                case ifMIN:     nErg = MAXDOUBLE; break;
                default: ; // nothing
            }
            do
            {
                nCount++;
                switch( eFunc )
                {
                    case ifAVERAGE:
                    case ifSUM:
                        if ( bNull && aValue.mfValue != 0.0 )
                        {
                            bNull = false;
                            fMem = aValue.mfValue;
                        }
                        else
                            nErg += aValue.mfValue;
                        break;
                    case ifSUMSQ:   nErg += aValue.mfValue * aValue.mfValue; break;
                    case ifPRODUCT: nErg *= aValue.mfValue; break;
                    case ifMAX:     if( aValue.mfValue > nErg ) nErg = aValue.mfValue; break;
                    case ifMIN:     if( aValue.mfValue < nErg ) nErg = aValue.mfValue; break;
                    default: ; // nothing
                }
            }
            while ( aValIter.GetNext(aValue) && !aValue.mnError );
        }
        SetError(aValue.mnError);
    }
    else
        SetError( errIllegalParameter);
    switch( eFunc )
    {
        case ifCOUNT:   nErg = nCount; break;
        case ifSUM:     nErg = ::rtl::math::approxAdd( nErg, fMem ); break;
        case ifAVERAGE: nErg = ::rtl::math::approxAdd( nErg, fMem ) / nCount; break;
        default: ; // nothing
    }
    PushDouble( nErg );
}


void ScInterpreter::ScDBSum()
{
    DBIterator( ifSUM );
}


void ScInterpreter::ScDBCount()
{
    bool bMissingField = true;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    auto_ptr<ScDBQueryParamBase> pQueryParam( GetDBParams(bMissingField) );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (pQueryParam.get())
    {
        sal_uLong nCount = 0;
        if ( bMissingField && pQueryParam->GetType() == ScDBQueryParamBase::INTERNAL )
        {   // count all matching records
            // TODO: currently the QueryIterators only return cell pointers of
            // existing cells, so if a query matches an empty cell there's
            // nothing returned, and therefor not counted!
            // Since this has ever been the case and this code here only came
            // into existance to fix #i6899 and it never worked before we'll
            // have to live with it until we reimplement the iterators to also
            // return empty cells, which would mean to adapt all callers of
            // iterators.
            ScDBQueryParamInternal* p = static_cast<ScDBQueryParamInternal*>(pQueryParam.get());
            p->nCol2 = p->nCol1; // Don't forget to select only one column.
            SCTAB nTab = p->nTab;
            // ScQueryCellIterator doesn't make use of ScDBQueryParamBase::mnField,
            // so the source range has to be restricted, like before the introduction
            // of ScDBQueryParamBase.
            p->nCol1 = p->nCol2 = p->mnField;
            ScQueryCellIterator aCellIter( pDok, nTab, *p);
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
            if (!pQueryParam->IsValidFieldIndex())
            {
                SetError(errNoValue);
                return;
            }
            ScDBQueryDataIterator aValIter( pDok, pQueryParam.release());
            ScDBQueryDataIterator::Value aValue;
            if ( aValIter.GetFirst(aValue) && !aValue.mnError )
            {
                do
                {
                    nCount++;
                }
                while ( aValIter.GetNext(aValue) && !aValue.mnError );
            }
            SetError(aValue.mnError);
        }
        PushDouble( nCount );
    }
    else
        PushIllegalParameter();
}


void ScInterpreter::ScDBCount2()
{
    bool bMissingField = true;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    auto_ptr<ScDBQueryParamBase> pQueryParam( GetDBParams(bMissingField) );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (pQueryParam.get())
    {
        if (!pQueryParam->IsValidFieldIndex())
        {
            SetError(errNoValue);
            return;
        }
        sal_uLong nCount = 0;
        pQueryParam->mbSkipString = false;
        ScDBQueryDataIterator aValIter( pDok, pQueryParam.release());
        ScDBQueryDataIterator::Value aValue;
        if ( aValIter.GetFirst(aValue) && !aValue.mnError )
        {
            do
            {
                nCount++;
            }
            while ( aValIter.GetNext(aValue) && !aValue.mnError );
        }
        SetError(aValue.mnError);
        PushDouble( nCount );
    }
    else
        PushIllegalParameter();
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
    std::vector<double> values;
    double vSum    = 0.0;
    double vMean    = 0.0;

    rValCount = 0.0;
    double fSum    = 0.0;
    bool bMissingField = false;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    auto_ptr<ScDBQueryParamBase> pQueryParam( GetDBParams(bMissingField) );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (pQueryParam.get())
    {
        if (!pQueryParam->IsValidFieldIndex())
        {
            SetError(errNoValue);
            return;
        }
        ScDBQueryDataIterator aValIter(pDok, pQueryParam.release());
        ScDBQueryDataIterator::Value aValue;
        if (aValIter.GetFirst(aValue) && !aValue.mnError)
        {
            do
            {
                rValCount++;
                values.push_back(aValue.mfValue);
                fSum += aValue.mfValue;
            }
            while ((aValue.mnError == 0) && aValIter.GetNext(aValue));
        }
        SetError(aValue.mnError);
    }
    else
        SetError( errIllegalParameter);

    vMean = fSum / values.size();

    for (size_t i = 0; i < values.size(); i++)
        vSum += (values[i] - vMean) * (values[i] - vMean);

    rVal = vSum;
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
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 )  )
    {
        // Reference address syntax for INDIRECT is configurable.
        FormulaGrammar::AddressConvention eConv = GetGlobalConfig().meStringRefAddressSyntax;
        if (eConv == FormulaGrammar::CONV_UNSPECIFIED)
            // Use the current address syntax if unspecified.
            eConv = pDok->GetAddressConvention();

        if (nParamCount == 2 && 0.0 == ::rtl::math::approxFloor( GetDouble()))
        {
            // Overwrite the config and try Excel R1C1.
            eConv = FormulaGrammar::CONV_XL_R1C1;
        }
        const ScAddress::Details aDetails( eConv, aPos );
        SCTAB nTab = aPos.Tab();
        String sRefStr( GetString() );
        ScRefAddress aRefAd, aRefAd2;
        ScAddress::ExternalInfo aExtInfo;
        if (ConvertDoubleRef(pDok, sRefStr, nTab, aRefAd, aRefAd2, aDetails, &aExtInfo))
        {
            if (aExtInfo.mbExternal)
            {
                PushExternalDoubleRef(
                    aExtInfo.mnFileId, aExtInfo.maTabName,
                    aRefAd.Col(), aRefAd.Row(), aRefAd.Tab(),
                    aRefAd2.Col(), aRefAd2.Row(), aRefAd2.Tab());
            }
            else
                PushDoubleRef( aRefAd.Col(), aRefAd.Row(), aRefAd.Tab(),
                        aRefAd2.Col(), aRefAd2.Row(), aRefAd2.Tab() );
        }
        else if (ConvertSingleRef(pDok, sRefStr, nTab, aRefAd, aDetails, &aExtInfo))
        {
            if (aExtInfo.mbExternal)
            {
                PushExternalSingleRef(
                    aExtInfo.mnFileId, aExtInfo.maTabName, aRefAd.Col(), aRefAd.Row(), aRefAd.Tab());
            }
            else
                PushSingleRef( aRefAd.Col(), aRefAd.Row(), aRefAd.Tab() );
        }
        else
        {
            do
            {
                ScRangeData* pData = ScRangeStringConverter::GetRangeDataFromString(sRefStr, nTab, pDok);
                if (!pData)
                    break;

                // We need this in order to obtain a good range.
                pData->ValidateTabRefs();

                ScRange aRange;

                // This is the usual way to treat named ranges containing
                // relative references.
                if (!pData->IsReference( aRange, aPos))
                    break;

                if (aRange.aStart == aRange.aEnd)
                    PushSingleRef( aRange.aStart.Col(), aRange.aStart.Row(),
                            aRange.aStart.Tab());
                else
                    PushDoubleRef( aRange.aStart.Col(), aRange.aStart.Row(),
                            aRange.aStart.Tab(), aRange.aEnd.Col(),
                            aRange.aEnd.Row(), aRange.aEnd.Tab());

                // success!
                return;
            }
            while (false);

            PushError( errNoRef);
        }
    }
}


void ScInterpreter::ScAddressFunc()
{
    String  sTabStr;

    sal_uInt8    nParamCount = GetByte();
    if( !MustHaveParamCount( nParamCount, 2, 5 ) )
        return;

    if( nParamCount >= 5 )
        sTabStr = GetString();

    FormulaGrammar::AddressConvention eConv = FormulaGrammar::CONV_OOO;      // default
    if( nParamCount >= 4 && 0.0 == ::rtl::math::approxFloor( GetDoubleWithDefault( 1.0)))
        eConv = FormulaGrammar::CONV_XL_R1C1;

    sal_uInt16  nFlags = SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE;   // default
    if( nParamCount >= 3 )
    {
        sal_uInt16 n = (sal_uInt16) ::rtl::math::approxFloor( GetDoubleWithDefault( 1.0));
        switch ( n )
        {
            default :
                PushNoValue();
                return;

            case 5:
            case 1 : break; // default
            case 6:
            case 2 : nFlags = SCA_ROW_ABSOLUTE; break;
            case 7:
            case 3 : nFlags = SCA_COL_ABSOLUTE; break;
            case 8:
            case 4 : nFlags = 0; break; // both relative
        }
    }
    nFlags |= SCA_VALID | SCA_VALID_ROW | SCA_VALID_COL;

    SCCOL nCol = (SCCOL) ::rtl::math::approxFloor(GetDouble());
    SCROW nRow = (SCROW) ::rtl::math::approxFloor(GetDouble());
    if( eConv == FormulaGrammar::CONV_XL_R1C1 )
    {
        // YUCK!  The XL interface actually treats rel R1C1 refs differently
        // than A1
        if( !(nFlags & SCA_COL_ABSOLUTE) )
            nCol += aPos.Col() + 1;
        if( !(nFlags & SCA_ROW_ABSOLUTE) )
            nRow += aPos.Row() + 1;
    }

    --nCol;
    --nRow;
    if(!ValidCol( nCol) || !ValidRow( nRow))
    {
        PushIllegalArgument();
        return;
    }

    String aRefStr;
    const ScAddress::Details aDetails( eConv, aPos );
    const ScAddress aAdr( nCol, nRow, 0);
    aAdr.Format( aRefStr, nFlags, pDok, aDetails );

    if( nParamCount >= 5 && sTabStr.Len() )
    {
        String aDoc;
        if (eConv == FormulaGrammar::CONV_OOO)
        {
            // Isolate Tab from 'Doc'#Tab
            xub_StrLen nPos = ScCompiler::GetDocTabPos( sTabStr);
            if (nPos != STRING_NOTFOUND)
            {
                if (sTabStr.GetChar(nPos+1) == '$')
                    ++nPos;     // also split 'Doc'#$Tab
                aDoc = sTabStr.Copy( 0, nPos+1);
                sTabStr.Erase( 0, nPos+1);
            }
        }
        /* TODO: yet unsupported external reference in CONV_XL_R1C1 syntax may
         * need some extra handling to isolate Tab from Doc. */
        if (sTabStr.GetChar(0) != '\'' || sTabStr.GetChar(sTabStr.Len()-1) != '\'')
            ScCompiler::CheckTabQuotes( sTabStr, eConv);
        if (aDoc.Len())
            sTabStr.Insert( aDoc, 0);
        sTabStr += static_cast<sal_Unicode>(eConv == FormulaGrammar::CONV_XL_R1C1 ? '!' : '.');
        sTabStr += aRefStr;
        PushString( sTabStr );
    }
    else
        PushString( aRefStr );
}


void ScInterpreter::ScOffset()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 3, 5 ) )
    {
        long nColNew = -1, nRowNew = -1, nColPlus, nRowPlus;
        if (nParamCount == 5)
            nColNew = (long) ::rtl::math::approxFloor(GetDouble());
        if (nParamCount >= 4)
            nRowNew = (long) ::rtl::math::approxFloor(GetDoubleWithDefault( -1.0 ));
        nColPlus = (long) ::rtl::math::approxFloor(GetDouble());
        nRowPlus = (long) ::rtl::math::approxFloor(GetDouble());
        SCCOL nCol1(0);
        SCROW nRow1(0);
        SCTAB nTab1(0);
        SCCOL nCol2(0);
        SCROW nRow2(0);
        SCTAB nTab2(0);
        if (nColNew == 0 || nRowNew == 0)
        {
            PushIllegalArgument();
            return;
        }
        switch (GetStackType())
        {
        case svSingleRef:
        {
            PopSingleRef(nCol1, nRow1, nTab1);
            if (nParamCount == 3 || (nColNew < 0 && nRowNew < 0))
            {
                nCol1 = (SCCOL)((long) nCol1 + nColPlus);
                nRow1 = (SCROW)((long) nRow1 + nRowPlus);
                if (!ValidCol(nCol1) || !ValidRow(nRow1))
                    PushIllegalArgument();
                else
                    PushSingleRef(nCol1, nRow1, nTab1);
            }
            else
            {
                if (nColNew < 0)
                    nColNew = 1;
                if (nRowNew < 0)
                    nRowNew = 1;
                nCol1 = (SCCOL)((long)nCol1+nColPlus);
                nRow1 = (SCROW)((long)nRow1+nRowPlus);
                nCol2 = (SCCOL)((long)nCol1+nColNew-1);
                nRow2 = (SCROW)((long)nRow1+nRowNew-1);
                if (!ValidCol(nCol1) || !ValidRow(nRow1) ||
                    !ValidCol(nCol2) || !ValidRow(nRow2))
                    PushIllegalArgument();
                else
                    PushDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab1);
            }
            break;
        }
        case svExternalSingleRef:
        {
            sal_uInt16 nFileId;
            String aTabName;
            ScSingleRefData aRef;
            PopExternalSingleRef(nFileId, aTabName, aRef);
            ScAddress aAbsRef = aRef.toAbs(aPos);
            nCol1 = aAbsRef.Col();
            nRow1 = aAbsRef.Row();
            nTab1 = aAbsRef.Tab();

            if (nParamCount == 3 || (nColNew < 0 && nRowNew < 0))
            {
                nCol1 = (SCCOL)((long) nCol1 + nColPlus);
                nRow1 = (SCROW)((long) nRow1 + nRowPlus);
                if (!ValidCol(nCol1) || !ValidRow(nRow1))
                    PushIllegalArgument();
                else
                    PushExternalSingleRef(nFileId, aTabName, nCol1, nRow1, nTab1);
            }
            else
            {
                if (nColNew < 0)
                    nColNew = 1;
                if (nRowNew < 0)
                    nRowNew = 1;
                nCol1 = (SCCOL)((long)nCol1+nColPlus);
                nRow1 = (SCROW)((long)nRow1+nRowPlus);
                nCol2 = (SCCOL)((long)nCol1+nColNew-1);
                nRow2 = (SCROW)((long)nRow1+nRowNew-1);
                nTab2 = nTab1;
                if (!ValidCol(nCol1) || !ValidRow(nRow1) ||
                    !ValidCol(nCol2) || !ValidRow(nRow2))
                    PushIllegalArgument();
                else
                    PushExternalDoubleRef(nFileId, aTabName, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            }
            break;
        }
        case svDoubleRef:
        {
            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            if (nColNew < 0)
                nColNew = nCol2 - nCol1 + 1;
            if (nRowNew < 0)
                nRowNew = nRow2 - nRow1 + 1;
            nCol1 = (SCCOL)((long)nCol1+nColPlus);
            nRow1 = (SCROW)((long)nRow1+nRowPlus);
            nCol2 = (SCCOL)((long)nCol1+nColNew-1);
            nRow2 = (SCROW)((long)nRow1+nRowNew-1);
            if (!ValidCol(nCol1) || !ValidRow(nRow1) ||
                !ValidCol(nCol2) || !ValidRow(nRow2) || nTab1 != nTab2)
                PushIllegalArgument();
            else
                PushDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab1);
            break;
        }
        case svExternalDoubleRef:
        {
            sal_uInt16 nFileId;
            String aTabName;
            ScComplexRefData aRef;
            PopExternalDoubleRef(nFileId, aTabName, aRef);
            ScRange aAbs = aRef.toAbs(aPos);
            nCol1 = aAbs.aStart.Col();
            nRow1 = aAbs.aStart.Row();
            nTab1 = aAbs.aStart.Tab();
            nCol2 = aAbs.aEnd.Col();
            nRow2 = aAbs.aEnd.Row();
            nTab2 = aAbs.aEnd.Tab();
            if (nColNew < 0)
                nColNew = nCol2 - nCol1 + 1;
            if (nRowNew < 0)
                nRowNew = nRow2 - nRow1 + 1;
            nCol1 = (SCCOL)((long)nCol1+nColPlus);
            nRow1 = (SCROW)((long)nRow1+nRowPlus);
            nCol2 = (SCCOL)((long)nCol1+nColNew-1);
            nRow2 = (SCROW)((long)nRow1+nRowNew-1);
            if (!ValidCol(nCol1) || !ValidRow(nRow1) ||
                !ValidCol(nCol2) || !ValidRow(nRow2) || nTab1 != nTab2)
                PushIllegalArgument();
            else
                PushExternalDoubleRef(nFileId, aTabName, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            break;
        }
        default:
            PushIllegalParameter();
            break;
        } // end switch
    }
}


void ScInterpreter::ScIndex()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 4 ) )
    {
        long nArea;
        size_t nAreaCount;
        SCCOL nCol;
        SCROW nRow;
        if (nParamCount == 4)
            nArea = (long) ::rtl::math::approxFloor(GetDouble());
        else
            nArea = 1;
        if (nParamCount >= 3)
            nCol = (SCCOL) ::rtl::math::approxFloor(GetDouble());
        else
            nCol = 0;
        if (nParamCount >= 2)
            nRow = (SCROW) ::rtl::math::approxFloor(GetDouble());
        else
            nRow = 0;
        if (GetStackType() == svRefList)
            nAreaCount = (sp ? static_cast<ScToken*>(pStack[sp-1])->GetRefList()->size() : 0);
        else
            nAreaCount = 1;     // one reference or array or whatever
        if (nAreaCount == 0 || (size_t)nArea > nAreaCount)
        {
            PushError( errNoRef);
            return;
        }
        else if (nArea < 1 || nCol < 0 || nRow < 0)
        {
            PushIllegalArgument();
            return;
        }
        switch (GetStackType())
        {
            case svMatrix:
            case svExternalSingleRef:
            case svExternalDoubleRef:
                {
                    if (nArea != 1)
                        SetError(errIllegalArgument);
                    sal_uInt16 nOldSp = sp;
                    ScMatrixRef pMat = GetMatrix();
                    if (pMat)
                    {
                        SCSIZE nC, nR;
                        pMat->GetDimensions(nC, nR);
                        // Access one element of a vector independent of col/row
                        // orientation?
                        bool bVector = ((nCol == 0 || nRow == 0) && (nC == 1 || nR == 1));
                        SCSIZE nElement = ::std::max( static_cast<SCSIZE>(nCol),
                                static_cast<SCSIZE>(nRow));
                        if (nC == 0 || nR == 0 ||
                                (!bVector && (static_cast<SCSIZE>(nCol) > nC ||
                                              static_cast<SCSIZE>(nRow) > nR)) ||
                                (bVector && nElement > nC * nR))
                            PushIllegalArgument();
                        else if (nCol == 0 && nRow == 0)
                            sp = nOldSp;
                        else if (bVector)
                        {
                            --nElement;
                            if (pMat->IsString( nElement))
                                PushString( pMat->GetString( nElement));
                            else
                                PushDouble( pMat->GetDouble( nElement));
                        }
                        else if (nCol == 0)
                        {
                            ScMatrixRef pResMat = GetNewMat(nC, 1);
                            if (pResMat)
                            {
                                SCSIZE nRowMinus1 = static_cast<SCSIZE>(nRow - 1);
                                for (SCSIZE i = 0; i < nC; i++)
                                    if (!pMat->IsString(i, nRowMinus1))
                                        pResMat->PutDouble(pMat->GetDouble(i,
                                                    nRowMinus1), i, 0);
                                    else
                                        pResMat->PutString(pMat->GetString(i,
                                                    nRowMinus1), i, 0);
                                PushMatrix(pResMat);
                            }
                            else
                                PushIllegalArgument();
                        }
                        else if (nRow == 0)
                        {
                            ScMatrixRef pResMat = GetNewMat(1, nR);
                            if (pResMat)
                            {
                                SCSIZE nColMinus1 = static_cast<SCSIZE>(nCol - 1);
                                for (SCSIZE i = 0; i < nR; i++)
                                    if (!pMat->IsString(nColMinus1, i))
                                        pResMat->PutDouble(pMat->GetDouble(nColMinus1,
                                                    i), i);
                                    else
                                        pResMat->PutString(pMat->GetString(nColMinus1,
                                                    i), i);
                                PushMatrix(pResMat);
                            }
                            else
                                PushIllegalArgument();
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
                break;
            case svSingleRef:
                {
                    SCCOL nCol1 = 0;
                    SCROW nRow1 = 0;
                    SCTAB nTab1 = 0;
                    PopSingleRef( nCol1, nRow1, nTab1);
                    if (nCol > 1 || nRow > 1)
                        PushIllegalArgument();
                    else
                        PushSingleRef( nCol1, nRow1, nTab1);
                }
                break;
            case svDoubleRef:
            case svRefList:
                {
                    SCCOL nCol1 = 0;
                    SCROW nRow1 = 0;
                    SCTAB nTab1 = 0;
                    SCCOL nCol2 = 0;
                    SCROW nRow2 = 0;
                    SCTAB nTab2 = 0;
                    bool bRowArray = false;
                    if (GetStackType() == svRefList)
                    {
                        FormulaTokenRef xRef = PopToken();
                        if (nGlobalError || !xRef)
                        {
                            PushIllegalParameter();
                            return;
                        }
                        ScRange aRange( ScAddress::UNINITIALIZED);
                        DoubleRefToRange( (*(static_cast<ScToken*>(xRef.get())->GetRefList()))[nArea-1], aRange);
                        aRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                        if ( nParamCount == 2 && nRow1 == nRow2 )
                            bRowArray = true;
                    }
                    else
                    {
                        PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                        if ( nParamCount == 2 && nRow1 == nRow2 )
                            bRowArray = true;
                    }
                    if ( nTab1 != nTab2 ||
                            (nCol > 0 && nCol1+nCol-1 > nCol2) ||
                            (nRow > 0 && nRow1+nRow-1 > nRow2 && !bRowArray ) ||
                            ( nRow > nCol2 - nCol1 + 1 && bRowArray ))
                        PushIllegalArgument();
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
                        else if ( bRowArray )
                        {
                            nCol =(SCCOL) nRow;
                            nRow = 1;
                            PushSingleRef( nCol1+nCol-1, nRow1+nRow-1, nTab1);
                        }
                        else
                            PushDoubleRef( nCol1, nRow1+nRow-1, nTab1,
                                    nCol2, nRow1+nRow-1, nTab1);
                    }
                    else
                        PushSingleRef( nCol1+nCol-1, nRow1+nRow-1, nTab1);
                }
                break;
            default:
                PushIllegalParameter();
        }
    }
}


void ScInterpreter::ScMultiArea()
{
    // Legacy support, convert to RefList
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCountMin( nParamCount, 1))
    {
        while (!nGlobalError && nParamCount-- > 1)
        {
            ScUnionFunc();
        }
    }
}


void ScInterpreter::ScAreas()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 1))
    {
        size_t nCount = 0;
        switch (GetStackType())
        {
            case svSingleRef:
                {
                    FormulaTokenRef xT = PopToken();
                    ValidateRef( static_cast<ScToken*>(xT.get())->GetSingleRef());
                    ++nCount;
                }
                break;
            case svDoubleRef:
                {
                    FormulaTokenRef xT = PopToken();
                    ValidateRef( static_cast<ScToken*>(xT.get())->GetDoubleRef());
                    ++nCount;
                }
                break;
            case svRefList:
                {
                    FormulaTokenRef xT = PopToken();
                    ValidateRef( *(static_cast<ScToken*>(xT.get())->GetRefList()));
                    nCount += static_cast<ScToken*>(xT.get())->GetRefList()->size();
                }
                break;
            default:
                SetError( errIllegalParameter);
        }
        PushDouble( double(nCount));
    }
}


void ScInterpreter::ScCurrency()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        OUString aStr;
        double fDec;
        if (nParamCount == 2)
        {
            fDec = ::rtl::math::approxFloor(GetDouble());
            if (fDec < -15.0 || fDec > 15.0)
            {
                PushIllegalArgument();
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
        sal_uLong nIndex = pFormatter->GetStandardFormat(
                                        NUMBERFORMAT_CURRENCY,
                                        ScGlobal::eLnge);
        if ( (sal_uInt16) fDec != pFormatter->GetFormatPrecision( nIndex ) )
        {
            OUString sFormatString = pFormatter->GenerateFormat(
                                                   nIndex,
                                                   ScGlobal::eLnge,
                                                   true,        // mit Tausenderpunkt
                                                   false,       // nicht rot
                                                  (sal_uInt16) fDec,// Nachkommastellen
                                                   1);          // 1 Vorkommanull
            if (!pFormatter->GetPreviewString(sFormatString,
                                              fVal,
                                              aStr,
                                              &pColor,
                                              ScGlobal::eLnge))
                SetError(errIllegalArgument);
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
        double fCount = ::rtl::math::approxFloor( GetDouble());
        double fPos   = ::rtl::math::approxFloor( GetDouble());
        String aOldStr( GetString() );
        if (fPos < 1.0 || fPos > static_cast<double>(STRING_MAXLEN)
                || fCount < 0.0 || fCount > static_cast<double>(STRING_MAXLEN))
            PushIllegalArgument();
        else
        {
            xub_StrLen nCount = static_cast<xub_StrLen>(fCount);
            xub_StrLen nPos   = static_cast<xub_StrLen>(fPos);
            xub_StrLen nLen   = aOldStr.Len();
            if (nPos > nLen + 1)
                nPos = nLen + 1;
            if (nCount > nLen - nPos + 1)
                nCount = nLen - nPos + 1;
            aOldStr.Erase( nPos-1, nCount );
            if ( CheckStringResultLen( aOldStr, aNewStr ) )
                aOldStr.Insert( aNewStr, nPos-1 );
            PushString( aOldStr );
        }
    }
}


void ScInterpreter::ScFixed()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 3 ) )
    {
        OUString aStr;
        double fDec;
        bool bThousand;
        if (nParamCount == 3)
            bThousand = !GetBool();     // Param TRUE: keine Tausenderpunkte
        else
            bThousand = true;
        if (nParamCount >= 2)
        {
            fDec = ::rtl::math::approxFloor(GetDoubleWithDefault( 2.0 ));
            if (fDec < -15.0 || fDec > 15.0)
            {
                PushIllegalArgument();
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
        if (fDec < 0.0)
            fDec = 0.0;
        sal_uLong nIndex = pFormatter->GetStandardFormat(
                                            NUMBERFORMAT_NUMBER,
                                            ScGlobal::eLnge);
        OUString sFormatString = pFormatter->GenerateFormat(
                                               nIndex,
                                               ScGlobal::eLnge,
                                               bThousand,   // mit Tausenderpunkt
                                               false,       // nicht rot
                                               (sal_uInt16) fDec,// Nachkommastellen
                                               1);          // 1 Vorkommanull
        if (!pFormatter->GetPreviewString(sFormatString,
                                                  fVal,
                                                  aStr,
                                                  &pColor,
                                                  ScGlobal::eLnge))
            PushIllegalArgument();
        else
            PushString(aStr);
    }
}


void ScInterpreter::ScFind()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        double fAnz;
        if (nParamCount == 3)
            fAnz = GetDouble();
        else
            fAnz = 1.0;
        String sStr = GetString();
        if( fAnz < 1.0 || fAnz > (double) sStr.Len() )
            PushNoValue();
        else
        {
            xub_StrLen nPos = sStr.Search( GetString(), (xub_StrLen) fAnz - 1 );
            if (nPos == STRING_NOTFOUND)
                PushNoValue();
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
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        xub_StrLen n;
        if (nParamCount == 2)
        {
            double nVal = ::rtl::math::approxFloor(GetDouble());
            if ( nVal < 0.0 || nVal > STRING_MAXLEN )
            {
                PushIllegalArgument();
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

typedef struct {
    UBlockCode from;
    UBlockCode to;
} UBlockScript;

static UBlockScript scriptList[] = {
    {UBLOCK_HANGUL_JAMO, UBLOCK_HANGUL_JAMO},
    {UBLOCK_CJK_RADICALS_SUPPLEMENT, UBLOCK_HANGUL_SYLLABLES},
    {UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS,UBLOCK_CJK_RADICALS_SUPPLEMENT },
    {UBLOCK_IDEOGRAPHIC_DESCRIPTION_CHARACTERS,UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS},
    {UBLOCK_CJK_COMPATIBILITY_FORMS, UBLOCK_CJK_COMPATIBILITY_FORMS},
    {UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS, UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS},
    {UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B, UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT},
    {UBLOCK_CJK_STROKES, UBLOCK_CJK_STROKES}
};
#define scriptListCount sizeof (scriptList) / sizeof (UBlockScript)
bool SAL_CALL lcl_getScriptClass(sal_uInt32 currentChar)
{
    // for the locale of ja-JP, character U+0x005c and U+0x20ac should be ScriptType::Asian
    if( (currentChar == 0x005c || currentChar == 0x20ac) &&
          (MsLangId::getSystemLanguage() == LANGUAGE_JAPANESE) )
        return true;
    sal_uInt16 i;
    static sal_Int16 nRet = 0;
    UBlockCode block = (UBlockCode)ublock_getCode((sal_uInt32)currentChar);
    for ( i = 0; i < scriptListCount; i++) {
        if (block <= scriptList[i].to) break;
    }
    nRet = (i < scriptListCount && block >= scriptList[i].from);
    return nRet;
}
bool IsDBCS(sal_Unicode ch)
{
    return lcl_getScriptClass(ch);
}
sal_Int32 getLengthB(String &str)
{
    sal_Int32 index = 0;
    sal_Int32 length = 0;
    if(0 == str.Len())
        return 0;
    while(index < str.Len()){
        if(IsDBCS(str.GetChar(index)))
            length += 2;
        else
            length++;
        index++;
    }
    return length;
}
void ScInterpreter::ScLenB()
{
    String aStr( GetString() );
    PushDouble( getLengthB(aStr) );
}
void lcl_RightB(String &aStr, sal_Int32 n)
{
    if( n < getLengthB(aStr) )
    {
        sal_Int32 index = aStr.Len();
        while(index-- >= 0)
        {
            if(0 == n)
            {
                aStr.Erase( 0, index + 1);
                break;
            }
            if(-1 == n)
            {
                aStr.Erase( 0, index + 2 );
                aStr.InsertAscii(" ", 0);
                break;
            }
            if(IsDBCS(aStr.GetChar(index)))
                n -= 2;
            else
                n--;
        }
    }
}
void ScInterpreter::ScRightB()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        sal_Int32 n;
        if (nParamCount == 2)
        {
            double nVal = ::rtl::math::approxFloor(GetDouble());
            if ( nVal < 0.0 || nVal > STRING_MAXLEN )
            {
                PushIllegalArgument();
                return ;
            }
            else
                n = (xub_StrLen) nVal;
        }
        else
            n = 1;
        String aStr( GetString() );
        lcl_RightB(aStr, n);
        PushString( aStr );
    }
}
void lcl_LeftB(String &aStr, sal_Int32 n)
{
    if( n < getLengthB(aStr) )
    {
        sal_Int32 index = -1;
        while(index++ < aStr.Len())
        {
            if(0 == n)
            {
                aStr.Erase( index );
                break;
            }
            if(-1 == n)
            {
                aStr.Erase( index - 1 );
                aStr.InsertAscii(" ");
                break;
            }
            if(IsDBCS(aStr.GetChar(index)))
                n -= 2;
            else
                n--;
        }
    }
}
void ScInterpreter::ScLeftB()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        sal_Int32 n;
        if (nParamCount == 2)
        {
            double nVal = ::rtl::math::approxFloor(GetDouble());
            if ( nVal < 0.0 || nVal > STRING_MAXLEN )
            {
                PushIllegalArgument();
                return ;
            }
            else
                n = (xub_StrLen) nVal;
        }
        else
            n = 1;
        String aStr( GetString() );
        lcl_LeftB(aStr, n);
        PushString( aStr );
    }
}
void ScInterpreter::ScMidB()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double fAnz    = ::rtl::math::approxFloor(GetDouble());
        double fAnfang = ::rtl::math::approxFloor(GetDouble());
        String rStr( GetString() );
        if (fAnfang < 1.0 || fAnz < 0.0 || fAnfang > double(STRING_MAXLEN) || fAnz > double(STRING_MAXLEN))
            PushIllegalArgument();
        else
        {

            lcl_LeftB(rStr, (xub_StrLen)fAnfang + (xub_StrLen)fAnz - 1);
            sal_Int32 nCnt = getLengthB(rStr) - (xub_StrLen)fAnfang + 1;
            lcl_RightB(rStr, nCnt>0 ? nCnt:0);
            PushString(rStr);
        }
    }
}

void ScInterpreter::ScRight()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        xub_StrLen n;
        if (nParamCount == 2)
        {
            double nVal = ::rtl::math::approxFloor(GetDouble());
            if ( nVal < 0.0 || nVal > STRING_MAXLEN )
            {
                PushIllegalArgument();
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
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        double fAnz;
        if (nParamCount == 3)
        {
            fAnz = ::rtl::math::approxFloor(GetDouble());
            if (fAnz > double(STRING_MAXLEN))
            {
                PushIllegalArgument();
                return;
            }
        }
        else
            fAnz = 1.0;
        OUString sStr = GetString();
        OUString SearchStr = GetString();
        sal_Int32 nPos = fAnz - 1;
        sal_Int32 nEndPos = sStr.getLength();
        if( nPos >= nEndPos )
            PushNoValue();
        else
        {
            utl::SearchParam::SearchType eSearchType =
                (MayBeRegExp( SearchStr, pDok ) ?
                utl::SearchParam::SRCH_REGEXP : utl::SearchParam::SRCH_NORMAL);
            utl::SearchParam sPar(SearchStr, eSearchType, false, false, false);
            utl::TextSearch sT( sPar, *ScGlobal::pCharClass );
            int nBool = sT.SearchForward(sStr, &nPos, &nEndPos);
            if (!nBool)
                PushNoValue();
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
            PushIllegalArgument();
        else
            PushString(rStr.Copy( (xub_StrLen) fAnfang - 1, (xub_StrLen) fAnz ));
    }
}


void ScInterpreter::ScText()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        OUString sFormatString = GetString();
        OUString aStr;
        bool bString = false;
        double fVal = 0.0;
        switch (GetStackType())
        {
            case svError:
                PopError();
                break;
            case svDouble:
                fVal = PopDouble();
                break;
            default:
                {
                    FormulaTokenRef xTok( PopToken());
                    if (!nGlobalError)
                    {
                        PushTempToken( xTok.get());
                        // Temporarily override the ConvertStringToValue()
                        // error for GetCellValue() / GetCellValueOrZero()
                        sal_uInt16 nSErr = mnStringNoValueError;
                        mnStringNoValueError = errNotNumericString;
                        fVal = GetDouble();
                        mnStringNoValueError = nSErr;
                        if (nGlobalError == errNotNumericString)
                        {
                            // Not numeric.
                            nGlobalError = 0;
                            PushTempToken( xTok.get());
                            aStr = GetString();
                            bString = true;
                        }
                    }
                }
        }
        if (nGlobalError)
            PushError( nGlobalError);
        else
        {
            OUString aResult;
            Color* pColor = NULL;
            LanguageType eCellLang;
            const ScPatternAttr* pPattern = pDok->GetPattern(
                    aPos.Col(), aPos.Row(), aPos.Tab() );
            if ( pPattern )
                eCellLang = ((const SvxLanguageItem&)
                        pPattern->GetItem( ATTR_LANGUAGE_FORMAT )).GetValue();
            else
                eCellLang = ScGlobal::eLnge;
            if (bString)
            {
                if (!pFormatter->GetPreviewString( sFormatString, aStr,
                            aResult, &pColor, eCellLang))
                    PushIllegalArgument();
                else
                    PushString( aResult);
            }
            else
            {
                if (!pFormatter->GetPreviewStringGuess( sFormatString, fVal,
                            aResult, &pColor, eCellLang))
                    PushIllegalArgument();
                else
                    PushString( aResult);
            }
        }
    }
}


void ScInterpreter::ScSubstitute()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 3, 4 ) )
    {
        xub_StrLen nAnz;
        if (nParamCount == 4)
        {
            double fAnz = ::rtl::math::approxFloor(GetDouble());
            if( fAnz < 1 || fAnz > STRING_MAXLEN )
            {
                PushIllegalArgument();
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
        while( true )
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
                        nPos = sal::static_int_cast<xub_StrLen>( nPos + nNewLen );
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
            PushIllegalArgument();
        else if ( fAnz * aStr.Len() > STRING_MAXLEN )
        {
            PushError( errStringOverflow );
        }
        else if ( fAnz == 0.0 )
            PushString( EMPTY_STRING );
        else
        {
            const xub_StrLen nLen = aStr.Len();
            xub_StrLen n = (xub_StrLen) fAnz;
            OUStringBuffer aRes(n*nLen);
            while( n-- )
                aRes.append(aStr);
            PushString( aRes.makeStringAndClear() );
        }
    }
}


void ScInterpreter::ScConcat()
{
    sal_uInt8 nParamCount = GetByte();
    String aRes;
    while( nParamCount-- > 0)
    {
        const String& rStr = GetString();
        aRes.Insert( rStr, 0 );
    }
    PushString( aRes );
}


void ScInterpreter::ScErrorType()
{
    sal_uInt16 nErr;
    sal_uInt16 nOldError = nGlobalError;
    nGlobalError = 0;
    switch ( GetStackType() )
    {
        case svRefList :
        {
            FormulaTokenRef x = PopToken();
            if (nGlobalError)
                nErr = nGlobalError;
            else
            {
                const ScRefList* pRefList = static_cast<ScToken*>(x.get())->GetRefList();
                size_t n = pRefList->size();
                if (!n)
                    nErr = errNoRef;
                else if (n > 1)
                    nErr = errNoValue;
                else
                {
                    ScRange aRange;
                    DoubleRefToRange( (*pRefList)[0], aRange);
                    if (nGlobalError)
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
            }
        }
        break;
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
        PushNA();
    }
}


bool ScInterpreter::MayBeRegExp( const OUString& rStr, const ScDocument* pDoc  )
{
    if ( pDoc && !pDoc->GetDocOptions().IsFormulaRegexEnabled() )
        return false;
    if ( rStr.isEmpty() || (rStr.getLength() == 1 && rStr[0] != '.') )
        return false;   // single meta characters can not be a regexp
    static const sal_Unicode cre[] = { '.','*','+','?','[',']','^','$','\\','<','>','(',')','|', 0 };
    const sal_Unicode* p1 = rStr.getStr();
    sal_Unicode c1;
    while ( ( c1 = *p1++ ) != 0 )
    {
        const sal_Unicode* p2 = cre;
        while ( *p2 )
        {
            if ( c1 == *p2++ )
                return true;
        }
    }
    return false;
}

static bool lcl_LookupQuery( ScAddress & o_rResultPos, ScDocument * pDoc,
        const ScQueryParam & rParam, const ScQueryEntry & rEntry )
{
    bool bFound = false;
    ScQueryCellIterator aCellIter( pDoc, rParam.nTab, rParam, false);
    if (rEntry.eOp != SC_EQUAL)
    {
        // range lookup <= or >=
        SCCOL nCol;
        SCROW nRow;
        bFound = aCellIter.FindEqualOrSortedLastInRange( nCol, nRow);
        if (bFound)
        {
            o_rResultPos.SetCol( nCol);
            o_rResultPos.SetRow( nRow);
        }
    }
    else if (aCellIter.GetFirst())
    {
        // EQUAL
        bFound = true;
        o_rResultPos.SetCol( aCellIter.GetCol());
        o_rResultPos.SetRow( aCellIter.GetRow());
    }
    return bFound;
}

bool ScInterpreter::LookupQueryWithCache( ScAddress & o_rResultPos,
        const ScQueryParam & rParam ) const
{
    bool bFound = false;
    const ScQueryEntry& rEntry = rParam.GetEntry(0);
    bool bColumnsMatch = (rParam.nCol1 == rEntry.nField);
    OSL_ENSURE( bColumnsMatch, "ScInterpreter::LookupQueryWithCache: columns don't match");
    if (!bColumnsMatch)
        bFound = lcl_LookupQuery( o_rResultPos, pDok, rParam, rEntry);
    else
    {
        ScRange aLookupRange( rParam.nCol1, rParam.nRow1, rParam.nTab,
                rParam.nCol2, rParam.nRow2, rParam.nTab);
        ScLookupCache& rCache = pDok->GetLookupCache( aLookupRange);
        ScLookupCache::QueryCriteria aCriteria( rEntry);
        ScLookupCache::Result eCacheResult = rCache.lookup( o_rResultPos,
                aCriteria, aPos);
        switch (eCacheResult)
        {
            case ScLookupCache::NOT_CACHED :
            case ScLookupCache::CRITERIA_DIFFERENT :
                bFound = lcl_LookupQuery( o_rResultPos, pDok, rParam, rEntry);
                if (eCacheResult == ScLookupCache::NOT_CACHED)
                    rCache.insert( o_rResultPos, aCriteria, aPos, bFound);
                break;
            case ScLookupCache::FOUND :
                bFound = true;
                break;
            case ScLookupCache::NOT_AVAILABLE :
                ;   // nothing, bFound remains FALSE
                break;
        }
    }
    return bFound;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
