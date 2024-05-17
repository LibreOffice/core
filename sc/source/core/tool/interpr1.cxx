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

#include <interpre.hxx>

#include <scitems.hxx>
#include <editeng/langitem.hxx>
#include <editeng/justifyitem.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/temporary.hxx>
#include <osl/thread.h>
#include <unotools/textsearch.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <tools/urlobj.hxx>
#include <unotools/charclass.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <rtl/character.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <unicode/uchar.h>
#include <unicode/regex.h>
#include <i18nlangtag/mslangid.hxx>

#include <patattr.hxx>
#include <global.hxx>
#include <document.hxx>
#include <dociter.hxx>
#include <docsh.hxx>
#include <formulacell.hxx>
#include <scmatrix.hxx>
#include <docoptio.hxx>
#include <attrib.hxx>
#include <jumpmatrix.hxx>
#include <cellkeytranslator.hxx>
#include <lookupcache.hxx>
#include <rangenam.hxx>
#include <rangeutl.hxx>
#include <compiler.hxx>
#include <externalrefmgr.hxx>
#include <doubleref.hxx>
#include <queryparam.hxx>
#include <queryiter.hxx>
#include <tokenarray.hxx>
#include <compare.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <svl/sharedstringpool.hxx>

#include <stdlib.h>
#include <memory>
#include <vector>
#include <limits>
#include <string_view>
#include <cmath>

const sal_uInt64 n2power48 = SAL_CONST_UINT64( 281474976710656); // 2^48

ScCalcConfig *ScInterpreter::mpGlobalConfig = nullptr;

using namespace formula;
using ::std::unique_ptr;

void ScInterpreter::ScIfJump()
{
    const short* pJump = pCur->GetJump();
    short nJumpCount = pJump[ 0 ];
    MatrixJumpConditionToMatrix();
    switch ( GetStackType() )
    {
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( !pMat )
                PushIllegalParameter();
            else
            {
                FormulaConstTokenRef xNew;
                ScTokenMatrixMap::const_iterator aMapIter;
                // DoubleError handled by JumpMatrix
                pMat->SetErrorInterpreter( nullptr);
                SCSIZE nCols, nRows;
                pMat->GetDimensions( nCols, nRows );
                if ( nCols == 0 || nRows == 0 )
                {
                    PushIllegalArgument();
                    return;
                }
                else if ((aMapIter = maTokenMatrixMap.find( pCur)) != maTokenMatrixMap.end())
                    xNew = (*aMapIter).second;
                else
                {
                    std::shared_ptr<ScJumpMatrix> pJumpMat( std::make_shared<ScJumpMatrix>(
                                pCur->GetOpCode(), nCols, nRows));
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
                                bIsValue = std::isfinite(fVal);
                                bTrue = bIsValue && (fVal != 0.0);
                                if (bTrue)
                                    fVal = 1.0;
                            }
                            else
                            {
                                // Treat empty and empty path as 0, but string
                                // as error. ScMatrix::IsValueOrEmpty() returns
                                // true for any empty, empty path, empty cell,
                                // empty result.
                                bIsValue = pMat->IsValueOrEmpty(nC, nR);
                                bTrue = false;
                                fVal = (bIsValue ? 0.0 : CreateDoubleError( FormulaError::NoValue));
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
                    GetTokenMatrixMap().emplace(pCur, xNew);
                }
                if (!xNew)
                {
                    PushIllegalArgument();
                    return;
                }
                PushTokenRef( xNew);
                // set endpoint of path for main code line
                aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
            }
        }
        break;
        default:
        {
            const bool bCondition = GetBool();
            if (nGlobalError != FormulaError::NONE)
            {   // Propagate error, not THEN- or ELSE-path, jump behind.
                PushError(nGlobalError);
                aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
            }
            else if ( bCondition )
            {   // TRUE
                if( nJumpCount >= 2 )
                {   // THEN path
                    aCode.Jump( pJump[ 1 ], pJump[ nJumpCount ] );
                }
                else
                {   // no parameter given for THEN
                    nFuncFmtType = SvNumFormatType::LOGICAL;
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
                    nFuncFmtType = SvNumFormatType::LOGICAL;
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
static void lcl_storeJumpMatResult(
    const ScMatrix* pMat, ScJumpMatrix* pJumpMat, SCSIZE nC, SCSIZE nR )
{
    if ( pMat->IsValue( nC, nR ) )
    {
        double fVal = pMat->GetDouble( nC, nR );
        pJumpMat->PutResultDouble( fVal, nC, nR );
    }
    else if ( pMat->IsEmpty( nC, nR ) )
    {
        pJumpMat->PutResultEmpty( nC, nR );
    }
    else
    {
        pJumpMat->PutResultString(pMat->GetString(nC, nR), nC, nR);
    }
}

void ScInterpreter::ScIfError( bool bNAonly )
{
    const short* pJump = pCur->GetJump();
    short nJumpCount = pJump[ 0 ];
    if (!sp || nJumpCount != 2)
    {
        // Reset nGlobalError here to not propagate the old error, if any.
        nGlobalError = (sp ? FormulaError::ParameterExpected : FormulaError::UnknownStackVariable);
        PushError( nGlobalError);
        aCode.Jump( pJump[ nJumpCount  ], pJump[ nJumpCount ] );
        return;
    }

    FormulaConstTokenRef xToken( pStack[ sp - 1 ] );
    bool bError = false;
    FormulaError nOldGlobalError = nGlobalError;
    nGlobalError = FormulaError::NONE;

    MatrixJumpConditionToMatrix();
    switch (GetStackType())
    {
        default:
            Pop();
            // Act on implicitly propagated error, if any.
            if (nOldGlobalError != FormulaError::NONE)
                nGlobalError = nOldGlobalError;
            if (nGlobalError != FormulaError::NONE)
                bError = true;
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

                    ScRefCellValue aCell(mrDoc, aAdr);
                    nGlobalError = GetCellErrCode(aCell);
                    if (nGlobalError != FormulaError::NONE)
                        bError = true;
                }
            }
            break;
        case svExternalSingleRef:
        case svExternalDoubleRef:
        {
            double fVal;
            svl::SharedString aStr;
            // Handles also existing jump matrix case and sets error on
            // elements.
            GetDoubleOrStringFromMatrix( fVal, aStr);
            if (nGlobalError != FormulaError::NONE)
                bError = true;
        }
        break;
        case svMatrix:
            {
                const ScMatrixRef pMat = PopMatrix();
                if (!pMat || (nGlobalError != FormulaError::NONE && (!bNAonly || nGlobalError == FormulaError::NotAvailable)))
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
                        FormulaError nErr = pMat->GetError( nC, nR );
                        if (nErr != FormulaError::NONE && (!bNAonly || nErr == FormulaError::NotAvailable))
                        {
                            bError = true;
                            nErrorCol = nC;
                            nErrorRow = nR;
                        }
                    }
                }
                if (!bError)
                    break;  // switch, we're done and have the result

                FormulaConstTokenRef xNew;
                ScTokenMatrixMap::const_iterator aMapIter;
                if ((aMapIter = maTokenMatrixMap.find( pCur)) != maTokenMatrixMap.end())
                {
                    xNew = (*aMapIter).second;
                }
                else
                {
                    const ScMatrix* pMatPtr = pMat.get();
                    std::shared_ptr<ScJumpMatrix> pJumpMat( std::make_shared<ScJumpMatrix>(
                                pCur->GetOpCode(), nCols, nRows));
                    // Init all jumps to no error to save single calls. Error
                    // is the exceptional condition.
                    const double fFlagResult = CreateDoubleError( FormulaError::JumpMatHasResult);
                    pJumpMat->SetAllJumps( fFlagResult, pJump[ nJumpCount ], pJump[ nJumpCount ] );
                    // Up to first error position simply store results, no need
                    // to evaluate error conditions again.
                    SCSIZE nC = 0, nR = 0;
                    for ( ; nC < nCols && (nC != nErrorCol || nR != nErrorRow); /*nop*/ )
                    {
                        for (nR = 0 ; nR < nRows && (nC != nErrorCol || nR != nErrorRow); ++nR)
                        {
                            lcl_storeJumpMatResult(pMatPtr, pJumpMat.get(), nC, nR);
                        }
                        if (nC != nErrorCol && nR != nErrorRow)
                            ++nC;
                    }
                    // Now the mixed cases.
                    for ( ; nC < nCols; ++nC)
                    {
                        for ( ; nR < nRows; ++nR)
                        {
                            FormulaError nErr = pMat->GetError( nC, nR );
                            if (nErr != FormulaError::NONE && (!bNAonly || nErr == FormulaError::NotAvailable))
                            {   // TRUE, THEN path
                                pJumpMat->SetJump( nC, nR, 1.0, pJump[ 1 ], pJump[ nJumpCount ] );
                            }
                            else
                            {   // FALSE, EMPTY path, store result instead
                                lcl_storeJumpMatResult(pMatPtr, pJumpMat.get(), nC, nR);
                            }
                        }
                        nR = 0;
                    }
                    xNew = new ScJumpMatrixToken( pJumpMat );
                    GetTokenMatrixMap().emplace( pCur, xNew );
                }
                nGlobalError = nOldGlobalError;
                PushTokenRef( xNew );
                // set endpoint of path for main code line
                aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
                return;
            }
            break;
    }

    if (bError && (!bNAonly || nGlobalError == FormulaError::NotAvailable))
    {
        // error, calculate 2nd argument
        nGlobalError = FormulaError::NONE;
        aCode.Jump( pJump[ 1 ], pJump[ nJumpCount ] );
    }
    else
    {
        // no error, push 1st argument and continue
        nGlobalError = nOldGlobalError;
        PushTokenRef( xToken);
        aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
    }
}

void ScInterpreter::ScChooseJump()
{
    // We have to set a jump, if there was none chosen because of an error set
    // it to endpoint.
    bool bHaveJump = false;
    const short* pJump = pCur->GetJump();
    short nJumpCount = pJump[ 0 ];
    MatrixJumpConditionToMatrix();
    switch ( GetStackType() )
    {
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            if ( !pMat )
                PushIllegalParameter();
            else
            {
                FormulaConstTokenRef xNew;
                ScTokenMatrixMap::const_iterator aMapIter;
                // DoubleError handled by JumpMatrix
                pMat->SetErrorInterpreter( nullptr);
                SCSIZE nCols, nRows;
                pMat->GetDimensions( nCols, nRows );
                if ( nCols == 0 || nRows == 0 )
                    PushIllegalParameter();
                else if ((aMapIter = maTokenMatrixMap.find(
                                    pCur)) != maTokenMatrixMap.end())
                    xNew = (*aMapIter).second;
                else
                {
                    std::shared_ptr<ScJumpMatrix> pJumpMat( std::make_shared<ScJumpMatrix>(
                                pCur->GetOpCode(), nCols, nRows));
                    for ( SCSIZE nC=0; nC < nCols; ++nC )
                    {
                        for ( SCSIZE nR=0; nR < nRows; ++nR )
                        {
                            double fVal;
                            bool bIsValue = pMat->IsValue(nC, nR);
                            if ( bIsValue )
                            {
                                fVal = pMat->GetDouble(nC, nR);
                                bIsValue = std::isfinite( fVal );
                                if ( bIsValue )
                                {
                                    fVal = ::rtl::math::approxFloor( fVal);
                                    if ( (fVal < 1) || (fVal >= nJumpCount))
                                    {
                                        bIsValue = false;
                                        fVal = CreateDoubleError(
                                                FormulaError::IllegalArgument);
                                    }
                                }
                            }
                            else
                            {
                                fVal = CreateDoubleError( FormulaError::NoValue);
                            }
                            if ( bIsValue )
                            {
                                pJumpMat->SetJump( nC, nR, fVal,
                                        pJump[ static_cast<short>(fVal) ],
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
                    GetTokenMatrixMap().emplace(pCur, xNew);
                }
                if (xNew)
                {
                    PushTokenRef( xNew);
                    // set endpoint of path for main code line
                    aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
                    bHaveJump = true;
                }
            }
        }
        break;
        default:
        {
            sal_Int16 nJumpIndex = GetInt16();
            if (nGlobalError == FormulaError::NONE && (nJumpIndex >= 1) && (nJumpIndex < nJumpCount))
            {
                aCode.Jump( pJump[ static_cast<short>(nJumpIndex) ], pJump[ nJumpCount ] );
                bHaveJump = true;
            }
            else
                PushIllegalArgument();
        }
    }
    if (!bHaveJump)
        aCode.Jump( pJump[ nJumpCount ], pJump[ nJumpCount ] );
}

static void lcl_AdjustJumpMatrix( ScJumpMatrix* pJumpM, SCSIZE nParmCols, SCSIZE nParmRows )
{
    SCSIZE nJumpCols, nJumpRows;
    SCSIZE nResCols, nResRows;
    SCSIZE nAdjustCols, nAdjustRows;
    pJumpM->GetDimensions( nJumpCols, nJumpRows );
    pJumpM->GetResMatDimensions( nResCols, nResRows );
    if (!(( nJumpCols == 1 && nParmCols > nResCols ) ||
        ( nJumpRows == 1 && nParmRows > nResRows )))
        return;

    if ( nJumpCols == 1 && nJumpRows == 1 )
    {
        nAdjustCols = std::max(nParmCols, nResCols);
        nAdjustRows = std::max(nParmRows, nResRows);
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
}

bool ScInterpreter::JumpMatrix( short nStackLevel )
{
    pJumpMatrix = pStack[sp-nStackLevel]->GetJumpMatrix();
    bool bHasResMat = pJumpMatrix->HasResultMatrix();
    SCSIZE nC, nR;
    if ( nStackLevel == 2 )
    {
        if ( aCode.HasStacked() )
            aCode.Pop();    // pop what Jump() pushed
        else
        {
            assert(!"pop goes the weasel");
        }

        if ( !bHasResMat )
        {
            Pop();
            SetError( FormulaError::UnknownStackVariable );
        }
        else
        {
            pJumpMatrix->GetPos( nC, nR );
            switch ( GetStackType() )
            {
                case svDouble:
                {
                    double fVal = GetDouble();
                    if ( nGlobalError != FormulaError::NONE )
                    {
                        fVal = CreateDoubleError( nGlobalError );
                        nGlobalError = FormulaError::NONE;
                    }
                    pJumpMatrix->PutResultDouble( fVal, nC, nR );
                }
                break;
                case svString:
                {
                    svl::SharedString aStr = GetString();
                    if ( nGlobalError != FormulaError::NONE )
                    {
                        pJumpMatrix->PutResultDouble( CreateDoubleError( nGlobalError),
                                nC, nR);
                        nGlobalError = FormulaError::NONE;
                    }
                    else
                        pJumpMatrix->PutResultString(aStr, nC, nR);
                }
                break;
                case svSingleRef:
                {
                    FormulaConstTokenRef xRef = pStack[sp-1];
                    ScAddress aAdr;
                    PopSingleRef( aAdr );
                    if ( nGlobalError != FormulaError::NONE )
                    {
                        pJumpMatrix->PutResultDouble( CreateDoubleError( nGlobalError),
                                nC, nR);
                        nGlobalError = FormulaError::NONE;
                    }
                    else
                    {
                        ScRefCellValue aCell(mrDoc, aAdr);
                        if (aCell.hasEmptyValue())
                            pJumpMatrix->PutResultEmpty( nC, nR );
                        else if (aCell.hasNumeric())
                        {
                            double fVal = GetCellValue(aAdr, aCell);
                            if ( nGlobalError != FormulaError::NONE )
                            {
                                fVal = CreateDoubleError(
                                        nGlobalError);
                                nGlobalError = FormulaError::NONE;
                            }
                            pJumpMatrix->PutResultDouble( fVal, nC, nR );
                        }
                        else
                        {
                            svl::SharedString aStr;
                            GetCellString(aStr, aCell);
                            if ( nGlobalError != FormulaError::NONE )
                            {
                                pJumpMatrix->PutResultDouble( CreateDoubleError(
                                            nGlobalError), nC, nR);
                                nGlobalError = FormulaError::NONE;
                            }
                            else
                                pJumpMatrix->PutResultString(aStr, nC, nR);
                        }
                    }

                    formula::ParamClass eReturnType = ScParameterClassification::GetParameterType( pCur, SAL_MAX_UINT16);
                    if (eReturnType == ParamClass::Reference)
                    {
                        /* TODO: What about error handling and do we actually
                         * need the result matrix above at all in this case? */
                        ScComplexRefData aRef;
                        aRef.Ref1 = aRef.Ref2 = *(xRef->GetSingleRef());
                        pJumpMatrix->GetRefList().push_back( aRef);
                    }
                }
                break;
                case svDoubleRef:
                {   // upper left plus offset within matrix
                    FormulaConstTokenRef xRef = pStack[sp-1];
                    double fVal;
                    ScRange aRange;
                    PopDoubleRef( aRange );
                    if ( nGlobalError != FormulaError::NONE )
                    {
                        fVal = CreateDoubleError( nGlobalError );
                        nGlobalError = FormulaError::NONE;
                        pJumpMatrix->PutResultDouble( fVal, nC, nR );
                    }
                    else
                    {
                        // Do not modify the original range because we use it
                        // to adjust the size of the result matrix if necessary.
                        ScAddress aAdr( aRange.aStart);
                        sal_uLong nCol = static_cast<sal_uLong>(aAdr.Col()) + nC;
                        sal_uLong nRow = static_cast<sal_uLong>(aAdr.Row()) + nR;
                        if ((nCol > o3tl::make_unsigned(aRange.aEnd.Col()) &&
                                    aRange.aEnd.Col() != aRange.aStart.Col())
                                || (nRow > o3tl::make_unsigned(aRange.aEnd.Row()) &&
                                    aRange.aEnd.Row() != aRange.aStart.Row()))
                        {
                            fVal = CreateDoubleError( FormulaError::NotAvailable );
                            pJumpMatrix->PutResultDouble( fVal, nC, nR );
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
                            ScRefCellValue aCell(mrDoc, aAdr);
                            if (aCell.hasEmptyValue())
                                pJumpMatrix->PutResultEmpty( nC, nR );
                            else if (aCell.hasNumeric())
                            {
                                double fCellVal = GetCellValue(aAdr, aCell);
                                if ( nGlobalError != FormulaError::NONE )
                                {
                                    fCellVal = CreateDoubleError(
                                            nGlobalError);
                                    nGlobalError = FormulaError::NONE;
                                }
                                pJumpMatrix->PutResultDouble( fCellVal, nC, nR );
                            }
                            else
                            {
                                svl::SharedString aStr;
                                GetCellString(aStr, aCell);
                                if ( nGlobalError != FormulaError::NONE )
                                {
                                    pJumpMatrix->PutResultDouble( CreateDoubleError(
                                                nGlobalError), nC, nR);
                                    nGlobalError = FormulaError::NONE;
                                }
                                else
                                    pJumpMatrix->PutResultString(aStr, nC, nR);
                            }
                        }
                        SCSIZE nParmCols = aRange.aEnd.Col() - aRange.aStart.Col() + 1;
                        SCSIZE nParmRows = aRange.aEnd.Row() - aRange.aStart.Row() + 1;
                        lcl_AdjustJumpMatrix( pJumpMatrix, nParmCols, nParmRows );
                    }

                    formula::ParamClass eReturnType = ScParameterClassification::GetParameterType( pCur, SAL_MAX_UINT16);
                    if (eReturnType == ParamClass::Reference)
                    {
                        /* TODO: What about error handling and do we actually
                         * need the result matrix above at all in this case? */
                        pJumpMatrix->GetRefList().push_back( *(xRef->GetDoubleRef()));
                    }
                }
                break;
                case svExternalSingleRef:
                {
                    ScExternalRefCache::TokenRef pToken;
                    PopExternalSingleRef(pToken);
                    if (nGlobalError != FormulaError::NONE)
                    {
                        pJumpMatrix->PutResultDouble( CreateDoubleError( nGlobalError), nC, nR );
                        nGlobalError = FormulaError::NONE;
                    }
                    else
                    {
                        switch (pToken->GetType())
                        {
                            case svDouble:
                                pJumpMatrix->PutResultDouble( pToken->GetDouble(), nC, nR );
                            break;
                            case svString:
                                pJumpMatrix->PutResultString( pToken->GetString(), nC, nR );
                            break;
                            case svEmptyCell:
                                pJumpMatrix->PutResultEmpty( nC, nR );
                            break;
                            default:
                                // svError was already handled (set by
                                // PopExternalSingleRef()) with nGlobalError
                                // above.
                                assert(!"unhandled svExternalSingleRef case");
                                pJumpMatrix->PutResultDouble( CreateDoubleError(
                                            FormulaError::UnknownStackVariable), nC, nR );
                        }
                    }
                }
                break;
                case svExternalDoubleRef:
                case svMatrix:
                {   // match matrix offsets
                    double fVal;
                    ScMatrixRef pMat = GetMatrix();
                    if ( nGlobalError != FormulaError::NONE )
                    {
                        fVal = CreateDoubleError( nGlobalError );
                        nGlobalError = FormulaError::NONE;
                        pJumpMatrix->PutResultDouble( fVal, nC, nR );
                    }
                    else if ( !pMat )
                    {
                        fVal = CreateDoubleError( FormulaError::UnknownVariable );
                        pJumpMatrix->PutResultDouble( fVal, nC, nR );
                    }
                    else
                    {
                        SCSIZE nCols, nRows;
                        pMat->GetDimensions( nCols, nRows );
                        if ((nCols <= nC && nCols != 1) ||
                            (nRows <= nR && nRows != 1))
                        {
                            fVal = CreateDoubleError( FormulaError::NotAvailable );
                            pJumpMatrix->PutResultDouble( fVal, nC, nR );
                        }
                        else
                        {
                            // GetMatrix() does SetErrorInterpreter() at the
                            // matrix, do not propagate an error from
                            // matrix->GetValue() as global error.
                            pMat->SetErrorInterpreter(nullptr);
                            lcl_storeJumpMatResult(pMat.get(), pJumpMatrix, nC, nR);
                        }
                        lcl_AdjustJumpMatrix( pJumpMatrix, nCols, nRows );
                    }
                }
                break;
                case svError:
                {
                    PopError();
                    double fVal = CreateDoubleError( nGlobalError);
                    nGlobalError = FormulaError::NONE;
                    pJumpMatrix->PutResultDouble( fVal, nC, nR );
                }
                break;
                default:
                {
                    Pop();
                    double fVal = CreateDoubleError( FormulaError::IllegalArgument);
                    pJumpMatrix->PutResultDouble( fVal, nC, nR );
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
            if ( bHasResMat && (GetDoubleErrorValue( fBool) != FormulaError::JumpMatHasResult) )
            {
                // a false without path results in an empty path value
                if ( fBool == 0.0 )
                    pJumpMatrix->PutResultEmptyPath( nC, nR );
                else
                    pJumpMatrix->PutResultDouble( fBool, nC, nR );
            }
            bCont = pJumpMatrix->Next( nC, nR );
            if ( bCont )
                pJumpMatrix->GetJump( nC, nR, fBool, nStart, nNext, nStop );
        }
        if ( bCont && nStart != nNext )
        {
            const ScTokenVec & rParams = pJumpMatrix->GetJumpParameters();
            for ( auto const & i : rParams )
            {
                // This is not the current state of the interpreter, so
                // push without error, and elements' errors are coded into
                // double.
                PushWithoutError(*i);
            }
            aCode.Jump( nStart, nNext, nStop );
        }
    }
    if ( !bCont )
    {   // We're done with it, throw away jump matrix, keep result.
        // For an intermediate result of Reference use the array of references
        // if there are more than one reference and the current ForceArray
        // context is ReferenceOrRefArray.
        // Else (also for a final result of Reference) use the matrix.
        // Treat the result of a jump command as final and use the matrix (see
        // tdf#115493 for why).
        if (pCur->GetInForceArray() == ParamClass::ReferenceOrRefArray &&
                pJumpMatrix->GetRefList().size() > 1 &&
                ScParameterClassification::GetParameterType( pCur, SAL_MAX_UINT16) == ParamClass::Reference &&
                !FormulaCompiler::IsOpCodeJumpCommand( pJumpMatrix->GetOpCode()) &&
                aCode.PeekNextOperator())
        {
            FormulaTokenRef xRef = new ScRefListToken(true);
            *(xRef->GetRefList()) = pJumpMatrix->GetRefList();
            pJumpMatrix = nullptr;
            Pop();
            PushTokenRef( xRef);
            maTokenMatrixMap.erase( pCur);
            // There's no result matrix to remember in this case.
        }
        else
        {
            ScMatrix* pResMat = pJumpMatrix->GetResultMatrix();
            pJumpMatrix = nullptr;
            Pop();
            PushMatrix( pResMat );
            // Remove jump matrix from map and remember result matrix in case it
            // could be reused in another path of the same condition.
            maTokenMatrixMap.erase( pCur);
            maTokenMatrixMap.emplace(pCur, pStack[sp-1]);
        }
        return true;
    }
    return false;
}

double ScInterpreter::Compare( ScQueryOp eOp )
{
    sc::Compare aComp;
    aComp.meOp = eOp;
    aComp.mbIgnoreCase = mrDoc.GetDocOptions().IsIgnoreCase();
    for( short i = 1; i >= 0; i-- )
    {
        sc::Compare::Cell& rCell = aComp.maCells[i];

        switch ( GetRawStackType() )
        {
            case svEmptyCell:
                Pop();
                rCell.mbEmpty = true;
                break;
            case svMissing:
            case svDouble:
                rCell.mfValue = GetDouble();
                rCell.mbValue = true;
                break;
            case svString:
                rCell.maStr = GetString();
                rCell.mbValue = false;
                break;
            case svDoubleRef :
            case svSingleRef :
            {
                ScAddress aAdr;
                if ( !PopDoubleRefOrSingleRef( aAdr ) )
                    break;
                ScRefCellValue aCell(mrDoc, aAdr);
                if (aCell.hasEmptyValue())
                    rCell.mbEmpty = true;
                else if (aCell.hasString())
                {
                    svl::SharedString aStr;
                    GetCellString(aStr, aCell);
                    rCell.maStr = aStr;
                    rCell.mbValue = false;
                }
                else
                {
                    rCell.mfValue = GetCellValue(aAdr, aCell);
                    rCell.mbValue = true;
                }
            }
            break;
            case svExternalSingleRef:
            {
                ScMatrixRef pMat = GetMatrix();
                if (!pMat)
                {
                    SetError( FormulaError::IllegalParameter);
                    break;
                }

                SCSIZE nC, nR;
                pMat->GetDimensions(nC, nR);
                if (!nC || !nR)
                {
                    SetError( FormulaError::IllegalParameter);
                    break;
                }
                if (pMat->IsEmpty(0, 0))
                    rCell.mbEmpty = true;
                else if (pMat->IsStringOrEmpty(0, 0))
                {
                    rCell.maStr = pMat->GetString(0, 0);
                    rCell.mbValue = false;
                }
                else
                {
                    rCell.mfValue = pMat->GetDouble(0, 0);
                    rCell.mbValue = true;
                }
            }
            break;
            case svExternalDoubleRef:
                // TODO: Find out how to handle this...
                // Xcl generates a position dependent intersection using
                // col/row, as it seems to do for all range references, not
                // only in compare context. We'd need a general implementation
                // for that behavior similar to svDoubleRef in scalar and array
                // mode. Which also means we'd have to change all places where
                // it currently is handled along with svMatrix.
            default:
                PopError();
                SetError( FormulaError::IllegalParameter);
            break;
        }
    }
    if( nGlobalError != FormulaError::NONE )
        return 0;
    nCurFmtType = nFuncFmtType = SvNumFormatType::LOGICAL;
    return sc::CompareFunc(aComp);
}

sc::RangeMatrix ScInterpreter::CompareMat( ScQueryOp eOp, sc::CompareOptions* pOptions )
{
    sc::Compare aComp;
    aComp.meOp = eOp;
    aComp.mbIgnoreCase = mrDoc.GetDocOptions().IsIgnoreCase();
    sc::RangeMatrix aMat[2];
    ScAddress aAdr;
    for( short i = 1; i >= 0; i-- )
    {
        sc::Compare::Cell& rCell = aComp.maCells[i];

        switch (GetRawStackType())
        {
            case svEmptyCell:
                Pop();
                rCell.mbEmpty = true;
                break;
            case svMissing:
            case svDouble:
                rCell.mfValue = GetDouble();
                rCell.mbValue = true;
                break;
            case svString:
                rCell.maStr = GetString();
                rCell.mbValue = false;
                break;
            case svSingleRef:
            {
                PopSingleRef( aAdr );
                ScRefCellValue aCell(mrDoc, aAdr);
                if (aCell.hasEmptyValue())
                    rCell.mbEmpty = true;
                else if (aCell.hasString())
                {
                    svl::SharedString aStr;
                    GetCellString(aStr, aCell);
                    rCell.maStr = aStr;
                    rCell.mbValue = false;
                }
                else
                {
                    rCell.mfValue = GetCellValue(aAdr, aCell);
                    rCell.mbValue = true;
                }
            }
            break;
            case svExternalSingleRef:
            case svExternalDoubleRef:
            case svDoubleRef:
            case svMatrix:
                aMat[i] = GetRangeMatrix();
                if (!aMat[i].mpMat)
                    SetError( FormulaError::IllegalParameter);
                else
                    aMat[i].mpMat->SetErrorInterpreter(nullptr);
                    // errors are transported as DoubleError inside matrix
                break;
            default:
                PopError();
                SetError( FormulaError::IllegalParameter);
            break;
        }
    }

    sc::RangeMatrix aRes;

    if (nGlobalError != FormulaError::NONE)
    {
        nCurFmtType = nFuncFmtType = SvNumFormatType::LOGICAL;
        return aRes;
    }

    if (aMat[0].mpMat && aMat[1].mpMat)
    {
        SCSIZE nC0, nC1;
        SCSIZE nR0, nR1;
        aMat[0].mpMat->GetDimensions(nC0, nR0);
        aMat[1].mpMat->GetDimensions(nC1, nR1);
        SCSIZE nC = std::max( nC0, nC1 );
        SCSIZE nR = std::max( nR0, nR1 );
        aRes.mpMat = GetNewMat( nC, nR, /*bEmpty*/true );
        if (!aRes.mpMat)
            return aRes;
        for ( SCSIZE j=0; j<nC; j++ )
        {
            for ( SCSIZE k=0; k<nR; k++ )
            {
                SCSIZE nCol = j, nRow = k;
                if (aMat[0].mpMat->ValidColRowOrReplicated(nCol, nRow) &&
                    aMat[1].mpMat->ValidColRowOrReplicated(nCol, nRow))
                {
                    for ( short i=1; i>=0; i-- )
                    {
                        sc::Compare::Cell& rCell = aComp.maCells[i];

                        if (aMat[i].mpMat->IsStringOrEmpty(j, k))
                        {
                            rCell.mbValue = false;
                            rCell.maStr = aMat[i].mpMat->GetString(j, k);
                            rCell.mbEmpty = aMat[i].mpMat->IsEmpty(j, k);
                        }
                        else
                        {
                            rCell.mbValue = true;
                            rCell.mfValue = aMat[i].mpMat->GetDouble(j, k);
                            rCell.mbEmpty = false;
                        }
                    }
                    aRes.mpMat->PutDouble( sc::CompareFunc( aComp, pOptions), j, k);
                }
                else
                    aRes.mpMat->PutError( FormulaError::NoValue, j, k);
            }
        }

        switch (eOp)
        {
            case SC_EQUAL:
                aRes.mpMat->CompareEqual();
                break;
            case SC_LESS:
                aRes.mpMat->CompareLess();
                break;
            case SC_GREATER:
                aRes.mpMat->CompareGreater();
                break;
            case SC_LESS_EQUAL:
                aRes.mpMat->CompareLessEqual();
                break;
            case SC_GREATER_EQUAL:
                aRes.mpMat->CompareGreaterEqual();
                break;
            case SC_NOT_EQUAL:
                aRes.mpMat->CompareNotEqual();
                break;
            default:
                SAL_WARN("sc",  "ScInterpreter::QueryMat: unhandled comparison operator: " << static_cast<int>(eOp));
                aRes.mpMat.reset();
                return aRes;
        }
    }
    else if (aMat[0].mpMat || aMat[1].mpMat)
    {
        size_t i = ( aMat[0].mpMat ? 0 : 1);

        aRes.mnCol1 = aMat[i].mnCol1;
        aRes.mnRow1 = aMat[i].mnRow1;
        aRes.mnTab1 = aMat[i].mnTab1;
        aRes.mnCol2 = aMat[i].mnCol2;
        aRes.mnRow2 = aMat[i].mnRow2;
        aRes.mnTab2 = aMat[i].mnTab2;

        ScMatrix& rMat = *aMat[i].mpMat;
        aRes.mpMat = rMat.CompareMatrix(aComp, i, pOptions);
        if (!aRes.mpMat)
            return aRes;
    }

    nCurFmtType = nFuncFmtType = SvNumFormatType::LOGICAL;
    return aRes;
}

ScMatrixRef ScInterpreter::QueryMat( const ScMatrixRef& pMat, sc::CompareOptions& rOptions )
{
    SvNumFormatType nSaveCurFmtType = nCurFmtType;
    SvNumFormatType nSaveFuncFmtType = nFuncFmtType;
    PushMatrix( pMat);
    const ScQueryEntry::Item& rItem = rOptions.aQueryEntry.GetQueryItem();
    if (rItem.meType == ScQueryEntry::ByString)
        PushString(rItem.maString.getString());
    else
        PushDouble(rItem.mfVal);
    ScMatrixRef pResultMatrix = CompareMat(rOptions.aQueryEntry.eOp, &rOptions).mpMat;
    nCurFmtType = nSaveCurFmtType;
    nFuncFmtType = nSaveFuncFmtType;
    if (nGlobalError != FormulaError::NONE || !pResultMatrix)
    {
        SetError( FormulaError::IllegalParameter);
        return pResultMatrix;
    }

    return pResultMatrix;
}

void ScInterpreter::ScEqual()
{
    if ( GetStackType(1) == svMatrix || GetStackType(2) == svMatrix )
    {
        sc::RangeMatrix aMat = CompareMat(SC_EQUAL);
        if (!aMat.mpMat)
        {
            PushIllegalParameter();
            return;
        }

        PushMatrix(aMat);
    }
    else
        PushInt( int(Compare( SC_EQUAL) == 0) );
}

void ScInterpreter::ScNotEqual()
{
    if ( GetStackType(1) == svMatrix || GetStackType(2) == svMatrix )
    {
        sc::RangeMatrix aMat = CompareMat(SC_NOT_EQUAL);
        if (!aMat.mpMat)
        {
            PushIllegalParameter();
            return;
        }

        PushMatrix(aMat);
    }
    else
        PushInt( int(Compare( SC_NOT_EQUAL) != 0) );
}

void ScInterpreter::ScLess()
{
    if ( GetStackType(1) == svMatrix || GetStackType(2) == svMatrix )
    {
        sc::RangeMatrix aMat = CompareMat(SC_LESS);
        if (!aMat.mpMat)
        {
            PushIllegalParameter();
            return;
        }

        PushMatrix(aMat);
    }
    else
        PushInt( int(Compare( SC_LESS) < 0) );
}

void ScInterpreter::ScGreater()
{
    if ( GetStackType(1) == svMatrix || GetStackType(2) == svMatrix )
    {
        sc::RangeMatrix aMat = CompareMat(SC_GREATER);
        if (!aMat.mpMat)
        {
            PushIllegalParameter();
            return;
        }

        PushMatrix(aMat);
    }
    else
        PushInt( int(Compare( SC_GREATER) > 0) );
}

void ScInterpreter::ScLessEqual()
{
    if ( GetStackType(1) == svMatrix || GetStackType(2) == svMatrix )
    {
        sc::RangeMatrix aMat = CompareMat(SC_LESS_EQUAL);
        if (!aMat.mpMat)
        {
            PushIllegalParameter();
            return;
        }

        PushMatrix(aMat);
    }
    else
        PushInt( int(Compare( SC_LESS_EQUAL) <= 0) );
}

void ScInterpreter::ScGreaterEqual()
{
    if ( GetStackType(1) == svMatrix || GetStackType(2) == svMatrix )
    {
        sc::RangeMatrix aMat = CompareMat(SC_GREATER_EQUAL);
        if (!aMat.mpMat)
        {
            PushIllegalParameter();
            return;
        }

        PushMatrix(aMat);
    }
    else
        PushInt( int(Compare( SC_GREATER_EQUAL) >= 0) );
}

void ScInterpreter::ScAnd()
{
    nFuncFmtType = SvNumFormatType::LOGICAL;
    short nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1 ) )
        return;

    bool bHaveValue = false;
    bool bRes = true;
    size_t nRefInList = 0;
    while( nParamCount-- > 0)
    {
        if ( nGlobalError == FormulaError::NONE )
        {
            switch ( GetStackType() )
            {
                case svDouble :
                    bHaveValue = true;
                    bRes &= ( PopDouble() != 0.0 );
                break;
                case svString :
                    Pop();
                    SetError( FormulaError::NoValue );
                break;
                case svSingleRef :
                {
                    ScAddress aAdr;
                    PopSingleRef( aAdr );
                    if ( nGlobalError == FormulaError::NONE )
                    {
                        ScRefCellValue aCell(mrDoc, aAdr);
                        if (aCell.hasNumeric())
                        {
                            bHaveValue = true;
                            bRes &= ( GetCellValue(aAdr, aCell) != 0.0 );
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
                    if ( nGlobalError == FormulaError::NONE )
                    {
                        double fVal;
                        FormulaError nErr = FormulaError::NONE;
                        ScValueIterator aValIter( mrContext, aRange );
                        if ( aValIter.GetFirst( fVal, nErr ) && nErr == FormulaError::NONE )
                        {
                            bHaveValue = true;
                            do
                            {
                                bRes &= ( fVal != 0.0 );
                            } while ( (nErr == FormulaError::NONE) &&
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
                        FormulaError nErr = GetDoubleErrorValue( fVal );
                        if ( nErr != FormulaError::NONE )
                        {
                            SetError( nErr );
                            bRes = false;
                        }
                        else
                            bRes &= (fVal != 0.0);
                    }
                    // else: GetMatrix did set FormulaError::IllegalParameter
                }
                break;
                default:
                    PopError();
                    SetError( FormulaError::IllegalParameter);
            }
        }
        else
            Pop();
    }
    if ( bHaveValue )
        PushInt( int(bRes) );
    else
        PushNoValue();
}

void ScInterpreter::ScOr()
{
    nFuncFmtType = SvNumFormatType::LOGICAL;
    short nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1 ) )
        return;

    bool bHaveValue = false;
    bool bRes = false;
    size_t nRefInList = 0;
    while( nParamCount-- > 0)
    {
        if ( nGlobalError == FormulaError::NONE )
        {
            switch ( GetStackType() )
            {
                case svDouble :
                    bHaveValue = true;
                    bRes |= ( PopDouble() != 0.0 );
                break;
                case svString :
                    Pop();
                    SetError( FormulaError::NoValue );
                break;
                case svSingleRef :
                {
                    ScAddress aAdr;
                    PopSingleRef( aAdr );
                    if ( nGlobalError == FormulaError::NONE )
                    {
                        ScRefCellValue aCell(mrDoc, aAdr);
                        if (aCell.hasNumeric())
                        {
                            bHaveValue = true;
                            bRes |= ( GetCellValue(aAdr, aCell) != 0.0 );
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
                    if ( nGlobalError == FormulaError::NONE )
                    {
                        double fVal;
                        FormulaError nErr = FormulaError::NONE;
                        ScValueIterator aValIter( mrContext, aRange );
                        if ( aValIter.GetFirst( fVal, nErr ) )
                        {
                            bHaveValue = true;
                            do
                            {
                                bRes |= ( fVal != 0.0 );
                            } while ( (nErr == FormulaError::NONE) &&
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
                        FormulaError nErr = GetDoubleErrorValue( fVal );
                        if ( nErr != FormulaError::NONE )
                        {
                            SetError( nErr );
                            bRes = false;
                        }
                        else
                            bRes |= (fVal != 0.0);
                    }
                    // else: GetMatrix did set FormulaError::IllegalParameter
                }
                break;
                default:
                    PopError();
                    SetError( FormulaError::IllegalParameter);
            }
        }
        else
            Pop();
    }
    if ( bHaveValue )
        PushInt( int(bRes) );
    else
        PushNoValue();
}

void ScInterpreter::ScXor()
{

    nFuncFmtType = SvNumFormatType::LOGICAL;
    short nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1 ) )
        return;

    bool bHaveValue = false;
    bool bRes = false;
    size_t nRefInList = 0;
    while( nParamCount-- > 0)
    {
        if ( nGlobalError == FormulaError::NONE )
        {
            switch ( GetStackType() )
            {
                case svDouble :
                    bHaveValue = true;
                    bRes ^= ( PopDouble() != 0.0 );
                break;
                case svString :
                    Pop();
                    SetError( FormulaError::NoValue );
                break;
                case svSingleRef :
                {
                    ScAddress aAdr;
                    PopSingleRef( aAdr );
                    if ( nGlobalError == FormulaError::NONE )
                    {
                        ScRefCellValue aCell(mrDoc, aAdr);
                        if (aCell.hasNumeric())
                        {
                            bHaveValue = true;
                            bRes ^= ( GetCellValue(aAdr, aCell) != 0.0 );
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
                    if ( nGlobalError == FormulaError::NONE )
                    {
                        double fVal;
                        FormulaError nErr = FormulaError::NONE;
                        ScValueIterator aValIter( mrContext, aRange );
                        if ( aValIter.GetFirst( fVal, nErr ) )
                        {
                            bHaveValue = true;
                            do
                            {
                                bRes ^= ( fVal != 0.0 );
                            } while ( (nErr == FormulaError::NONE) &&
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
                        FormulaError nErr = GetDoubleErrorValue( fVal );
                        if ( nErr != FormulaError::NONE )
                        {
                            SetError( nErr );
                            bRes = false;
                        }
                        else
                            bRes ^= ( fVal != 0.0 );
                    }
                    // else: GetMatrix did set FormulaError::IllegalParameter
                }
                break;
                default:
                    PopError();
                    SetError( FormulaError::IllegalParameter);
            }
        }
        else
            Pop();
    }
    if ( bHaveValue )
        PushInt( int(bRes) );
    else
        PushNoValue();
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
                ScMatrixRef pResMat = GetNewMat( nC, nR, /*bEmpty*/true );
                if ( !pResMat )
                    PushIllegalArgument();
                else
                {
                    pMat->NegOp( *pResMat);
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
    nFuncFmtType = SvNumFormatType::PERCENT;
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
    nFuncFmtType = SvNumFormatType::LOGICAL;
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
                ScMatrixRef pResMat = GetNewMat( nC, nR, /*bEmpty*/true);
                if ( !pResMat )
                    PushIllegalArgument();
                else
                {
                    pMat->NotOp( *pResMat);
                    PushMatrix( pResMat );
                }
            }
        }
        break;
        default:
            PushInt( int(GetDouble() == 0.0) );
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
        PushDouble (static_cast<sal_uInt64>(num1) & static_cast<sal_uInt64>(num2));
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
        PushDouble (static_cast<sal_uInt64>(num1) | static_cast<sal_uInt64>(num2));
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
        PushDouble (static_cast<sal_uInt64>(num1) ^ static_cast<sal_uInt64>(num2));
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
    PushDouble(M_PI);
}

void ScInterpreter::ScRandomImpl( const std::function<double( double fFirst, double fLast )>& RandomFunc,
        double fFirst, double fLast )
{
    if (bMatrixFormula)
    {
        SCCOL nCols = 0;
        SCROW nRows = 0;
        // In JumpMatrix context use its dimensions for the return matrix; the
        // formula cell range selected may differ, for example if the result is
        // to be transposed.
        if (GetStackType(1) == svJumpMatrix)
        {
            SCSIZE nC, nR;
            pStack[sp-1]->GetJumpMatrix()->GetDimensions( nC, nR);
            nCols = std::max<SCCOL>(0, static_cast<SCCOL>(nC));
            nRows = std::max<SCROW>(0, static_cast<SCROW>(nR));
        }
        else if (pMyFormulaCell)
            pMyFormulaCell->GetMatColsRows( nCols, nRows);

        if (nCols == 1 && nRows == 1)
        {
            // For compatibility with existing
            // com.sun.star.sheet.FunctionAccess.callFunction() calls that per
            // default are executed in array context unless
            // FA.setPropertyValue("IsArrayFunction",False) was set, return a
            // scalar double instead of a 1x1 matrix object. tdf#128218
            PushDouble( RandomFunc( fFirst, fLast));
            return;
        }

        // ScViewFunc::EnterMatrix() might be asking for
        // ScFormulaCell::GetResultDimensions(), which here are none so create
        // a 1x1 matrix at least which exactly is the case when EnterMatrix()
        // asks for a not selected range.
        if (nCols == 0)
            nCols = 1;
        if (nRows == 0)
            nRows = 1;
        ScMatrixRef pResMat = GetNewMat( static_cast<SCSIZE>(nCols), static_cast<SCSIZE>(nRows), /*bEmpty*/true );
        if (!pResMat)
            PushError( FormulaError::MatrixSize);
        else
        {
            for (SCCOL i=0; i < nCols; ++i)
            {
                for (SCROW j=0; j < nRows; ++j)
                {
                    pResMat->PutDouble( RandomFunc( fFirst, fLast),
                            static_cast<SCSIZE>(i), static_cast<SCSIZE>(j));
                }
            }
            PushMatrix( pResMat);
        }
    }
    else
    {
        PushDouble( RandomFunc( fFirst, fLast));
    }
}

void ScInterpreter::ScRandom()
{
    auto RandomFunc = [this]( double, double )
    {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(mrContext.aRNG);
    };
    ScRandomImpl( RandomFunc, 0.0, 0.0 );
}

void ScInterpreter::ScRandArray()
{
    sal_uInt8 nParamCount = GetByte();
    // optional 5th para:
    // TRUE for a whole number
    // FALSE for a decimal number - default.
    bool bWholeNumber = false;
    if (nParamCount == 5)
        bWholeNumber = GetBoolWithDefault(false);

    // optional 4th para: The maximum value of the random numbers
    double fMax = 1.0;
    if (nParamCount >= 4)
        fMax = GetDoubleWithDefault(1.0);

    // optional 3rd para: The minimum value of the random numbers
    double fMin = 0.0;
    if (nParamCount >= 3)
        fMin = GetDoubleWithDefault(0.0);

    // optional 2nd para: The number of columns of the return array
    SCCOL nCols = 1;
    if (nParamCount >= 2)
        nCols = static_cast<SCCOL>(GetInt32WithDefault(1));

    // optional 1st para: The number of rows of the return array
    SCROW nRows = 1;
    if (nParamCount >= 1)
        nRows = static_cast<SCROW>(GetInt32WithDefault(1));

    if (bWholeNumber)
    {
        fMax = rtl::math::round(fMax, 0, rtl_math_RoundingMode_Up);
        fMin = rtl::math::round(fMin, 0, rtl_math_RoundingMode_Up);
    }

    if (nGlobalError != FormulaError::NONE || fMin > fMax || nCols <= 0 || nRows <= 0)
    {
        PushIllegalArgument();
        return;
    }

    if (bWholeNumber)
        fMax = std::nextafter(fMax + 1, -DBL_MAX);
    else
        fMax = std::nextafter(fMax, DBL_MAX);

    auto RandomFunc = [this](double fFirst, double fLast, bool bWholeNum)
        {
            std::uniform_real_distribution<double> dist(fFirst, fLast);
            if (bWholeNum)
                return floor(dist(mrContext.aRNG));
            else
                return dist(mrContext.aRNG);
        };

    if (nCols == 1 && nRows == 1)
    {
        PushDouble(RandomFunc(fMin, fMax, bWholeNumber));
        return;
    }

    ScMatrixRef pResMat = GetNewMat(static_cast<SCSIZE>(nCols), static_cast<SCSIZE>(nRows), /*bEmpty*/true);
    if (!pResMat)
        PushError(FormulaError::MatrixSize);
    else
    {
        for (SCCOL i = 0; i < nCols; ++i)
        {
            for (SCROW j = 0; j < nRows; ++j)
            {
                pResMat->PutDouble(RandomFunc(fMin, fMax, bWholeNumber),
                    static_cast<SCSIZE>(i), static_cast<SCSIZE>(j));
            }
        }
        PushMatrix(pResMat);
    }
}

void ScInterpreter::ScRandbetween()
{
    if (!MustHaveParamCount( GetByte(), 2))
        return;

    // Same like scaddins/source/analysis/analysis.cxx
    // AnalysisAddIn::getRandbetween()
    double fMax = rtl::math::round( GetDouble(), 0, rtl_math_RoundingMode_Up);
    double fMin = rtl::math::round( GetDouble(), 0, rtl_math_RoundingMode_Up);
    if (nGlobalError != FormulaError::NONE || fMin > fMax)
    {
        PushIllegalArgument();
        return;
    }
    fMax = std::nextafter( fMax+1, -DBL_MAX);
    auto RandomFunc = [this]( double fFirst, double fLast )
    {
        std::uniform_real_distribution<double> dist(fFirst, fLast);
        return floor(dist(mrContext.aRNG));
    };
    ScRandomImpl( RandomFunc, fMin, fMax);
}

void ScInterpreter::ScTrue()
{
    nFuncFmtType = SvNumFormatType::LOGICAL;
    PushInt(1);
}

void ScInterpreter::ScFalse()
{
    nFuncFmtType = SvNumFormatType::LOGICAL;
    PushInt(0);
}

void ScInterpreter::ScDeg()
{
    PushDouble(basegfx::rad2deg(GetDouble()));
}

void ScInterpreter::ScRad()
{
    PushDouble(basegfx::deg2rad(GetDouble()));
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
    PushDouble((M_PI_2) - atan(GetDouble()));
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
        PushDouble(::atanh(fVal));
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
    nFuncFmtType = SvNumFormatType::LOGICAL;
    switch ( GetRawStackType() )
    {
        case svEmptyCell:
        {
            FormulaConstTokenRef p = PopToken();
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
            // NOTE: this differs from COUNTBLANK() ScCountEmptyCells() that
            // may treat ="" in the referenced cell as blank for Excel
            // interoperability.
            ScRefCellValue aCell(mrDoc, aAdr);
            if (aCell.getType() == CELLTYPE_NONE)
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
                nRes = pMat->IsEmptyCell( 0, 0) ? 1 : 0;
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    nRes = pMat->IsEmptyCell( nC, nR) ? 1 : 0;
                // else: false, not empty (which is what Xcl does)
            }
        }
        break;
        default:
            Pop();
    }
    nGlobalError = FormulaError::NONE;
    PushInt( nRes );
}

bool ScInterpreter::IsString()
{
    nFuncFmtType = SvNumFormatType::LOGICAL;
    bool bRes = false;
    switch ( GetRawStackType() )
    {
        case svString:
            Pop();
            bRes = true;
        break;
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;

            ScRefCellValue aCell(mrDoc, aAdr);
            if (GetCellErrCode(aCell) == FormulaError::NONE)
            {
                switch (aCell.getType())
                {
                    case CELLTYPE_STRING :
                    case CELLTYPE_EDIT :
                        bRes = true;
                        break;
                    case CELLTYPE_FORMULA :
                        bRes = (!aCell.getFormula()->IsValue() && !aCell.getFormula()->IsEmpty());
                        break;
                    default:
                        ; // nothing
                }
            }
        }
        break;
        case svExternalSingleRef:
        {
            ScExternalRefCache::TokenRef pToken;
            PopExternalSingleRef(pToken);
            if (nGlobalError == FormulaError::NONE && pToken->GetType() == svString)
                bRes = true;
        }
        break;
        case svExternalDoubleRef:
        case svMatrix:
        {
            ScMatrixRef pMat = GetMatrix();
            if ( !pMat )
                ;   // nothing
            else if ( !pJumpMatrix )
                bRes = pMat->IsStringOrEmpty(0, 0) && !pMat->IsEmpty(0, 0);
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    bRes = pMat->IsStringOrEmpty( nC, nR) && !pMat->IsEmpty( nC, nR);
            }
        }
        break;
        default:
            Pop();
    }
    nGlobalError = FormulaError::NONE;
    return bRes;
}

void ScInterpreter::ScIsString()
{
    PushInt( int(IsString()) );
}

void ScInterpreter::ScIsNonString()
{
    PushInt( int(!IsString()) );
}

void ScInterpreter::ScIsLogical()
{
    bool bRes = false;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;

            ScRefCellValue aCell(mrDoc, aAdr);
            if (GetCellErrCode(aCell) == FormulaError::NONE)
            {
                if (aCell.hasNumeric())
                {
                    sal_uInt32 nFormat = GetCellNumberFormat(aAdr, aCell);
                    bRes = (mrContext.NFGetType(nFormat) == SvNumFormatType::LOGICAL);
                }
            }
        }
        break;
        case svMatrix:
        {
            double fVal;
            svl::SharedString aStr;
            ScMatValType nMatValType = GetDoubleOrStringFromMatrix( fVal, aStr);
            bRes = (nMatValType == ScMatValType::Boolean);
        }
        break;
        default:
            PopError();
            if ( nGlobalError == FormulaError::NONE )
                bRes = ( nCurFmtType == SvNumFormatType::LOGICAL );
    }
    nCurFmtType = nFuncFmtType = SvNumFormatType::LOGICAL;
    nGlobalError = FormulaError::NONE;
    PushInt( int(bRes) );
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

            ScRefCellValue aCell(mrDoc, aAdr);
            if (GetCellErrCode(aCell) == FormulaError::NONE)
            {
                switch (aCell.getType())
                {
                    // NOTE: this is Xcl nonsense!
                    case CELLTYPE_STRING :
                    case CELLTYPE_EDIT :
                        nType = 2;
                        break;
                    case CELLTYPE_VALUE :
                    {
                        sal_uInt32 nFormat = GetCellNumberFormat(aAdr, aCell);
                        if (mrContext.NFGetType(nFormat) == SvNumFormatType::LOGICAL)
                            nType = 4;
                        else
                            nType = 1;
                    }
                    break;
                    case CELLTYPE_NONE:
                        // always 1, s. tdf#73078
                        nType = 1;
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
            if ( nGlobalError != FormulaError::NONE )
            {
                nType = 16;
                nGlobalError = FormulaError::NONE;
            }
            else
                nType = 2;
        break;
        case svMatrix:
            PopMatrix();
            if ( nGlobalError != FormulaError::NONE )
            {
                nType = 16;
                nGlobalError = FormulaError::NONE;
            }
            else
                nType = 64;
                // we could return the type of one element if in JumpMatrix or
                // ForceArray mode, but Xcl doesn't ...
        break;
        default:
            PopError();
            if ( nGlobalError != FormulaError::NONE )
            {
                nType = 16;
                nGlobalError = FormulaError::NONE;
            }
            else
                nType = 1;
    }
    PushInt( nType );
}

static bool lcl_FormatHasNegColor( const SvNumberformat* pFormat )
{
    return pFormat && pFormat->GetColor( 1 );
}

static bool lcl_FormatHasOpenPar( const SvNumberformat* pFormat )
{
    return pFormat && (pFormat->GetFormatstring().indexOf('(') != -1);
}

namespace {

void getFormatString(const ScInterpreterContext& rContext, sal_uLong nFormat, OUString& rFmtStr)
{
    rFmtStr = rContext.NFGetCalcCellReturn(nFormat);
}

}

void ScInterpreter::ScCell()
{   // ATTRIBUTE ; [REF]
    sal_uInt8 nParamCount = GetByte();
    if( !MustHaveParamCount( nParamCount, 1, 2 ) )
        return;

    ScAddress aCellPos( aPos );
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
            case svDoubleRef:
            {
                // Exceptionally not an intersecting position but top left.
                // See ODF v1.3 part 4 OpenFormula 6.13.3 CELL
                ScRange aRange;
                PopDoubleRef( aRange);
                aCellPos = aRange.aStart;
            }
            break;
            case svSingleRef:
                PopSingleRef( aCellPos);
            break;
            default:
                PopError();
                SetError( FormulaError::NoRef);
        }
    }
    OUString aInfoType = GetString().getString();
    if (nGlobalError != FormulaError::NONE)
        PushIllegalParameter();
    else
    {
        ScRefCellValue aCell(mrDoc, aCellPos);

        ScCellKeywordTranslator::transKeyword(aInfoType, &ScGlobal::GetLocale(), ocCell);

// *** ADDRESS INFO ***
        if( aInfoType == "COL" )
        {   // column number (1-based)
            PushInt( aCellPos.Col() + 1 );
        }
        else if( aInfoType == "ROW" )
        {   // row number (1-based)
            PushInt( aCellPos.Row() + 1 );
        }
        else if( aInfoType == "SHEET" )
        {   // table number (1-based)
            PushInt( aCellPos.Tab() + 1 );
        }
        else if( aInfoType == "ADDRESS" )
        {   // address formatted as [['FILENAME'#]$TABLE.]$COL$ROW

            // Follow the configurable string reference address syntax as also
            // used by INDIRECT() (and ADDRESS() for the sheet separator).
            FormulaGrammar::AddressConvention eConv = maCalcConfig.meStringRefAddressSyntax;
            switch (eConv)
            {
                default:
                    // Use the current address syntax if unspecified or says
                    // one or the other or one we don't explicitly handle.
                    eConv = mrDoc.GetAddressConvention();
                break;
                case FormulaGrammar::CONV_OOO:
                case FormulaGrammar::CONV_XL_A1:
                case FormulaGrammar::CONV_XL_R1C1:
                    // Use that.
                break;
            }

            ScRefFlags nFlags = (aCellPos.Tab() == aPos.Tab()) ? ScRefFlags::ADDR_ABS : ScRefFlags::ADDR_ABS_3D;
            OUString aStr(aCellPos.Format(nFlags, &mrDoc, eConv));
            PushString(aStr);
        }
        else if( aInfoType == "FILENAME" )
        {
            SCTAB nTab = aCellPos.Tab();
            OUString aFuncResult;
            if( nTab < mrDoc.GetTableCount() )
            {
                if( mrDoc.GetLinkMode( nTab ) == ScLinkMode::VALUE )
                    mrDoc.GetName( nTab, aFuncResult );
                else
                {
                    ScDocShell* pShell = mrDoc.GetDocumentShell();
                    if( pShell && pShell->GetMedium() )
                    {
                        const INetURLObject& rURLObj = pShell->GetMedium()->GetURLObject();
                        OUString aTabName;
                        mrDoc.GetName( nTab, aTabName );

                        FormulaGrammar::AddressConvention eConv = maCalcConfig.meStringRefAddressSyntax;
                        if (eConv == FormulaGrammar::CONV_UNSPECIFIED)
                            eConv = mrDoc.GetAddressConvention();

                        if (eConv == FormulaGrammar::CONV_XL_A1 ||
                            eConv == FormulaGrammar::CONV_XL_R1C1 ||
                            eConv == FormulaGrammar::CONV_XL_OOX)
                        {
                            // file name and table name: FILEPATH/[FILENAME]TABLE
                            if (!comphelper::LibreOfficeKit::isActive())
                                aFuncResult = rURLObj.GetPartBeforeLastName();
                            aFuncResult += "[" + rURLObj.GetLastName(INetURLObject::DecodeMechanism::Unambiguous) +
                                           "]" + aTabName;
                        }
                        else
                        {
                            // file name and table name: 'FILEPATH/FILENAME'#$TABLE
                            aFuncResult = "'";
                            if (!comphelper::LibreOfficeKit::isActive())
                                aFuncResult += rURLObj.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous);
                            else
                                aFuncResult += rURLObj.GetLastName(INetURLObject::DecodeMechanism::Unambiguous);
                            aFuncResult += "'#$" + aTabName;
                        }
                    }
                }
            }
            PushString( aFuncResult );
        }
        else if( aInfoType == "COORD" )
        {   // address, lotus 1-2-3 formatted: $TABLE:$COL$ROW
            // Yes, passing tab as col is intentional!
            OUString aCellStr1 =
                ScAddress( static_cast<SCCOL>(aCellPos.Tab()), 0, 0 ).Format(
                    (ScRefFlags::COL_ABS|ScRefFlags::COL_VALID), nullptr, mrDoc.GetAddressConvention() );
            OUString aCellStr2 =
                aCellPos.Format((ScRefFlags::COL_ABS|ScRefFlags::COL_VALID|ScRefFlags::ROW_ABS|ScRefFlags::ROW_VALID),
                             nullptr, mrDoc.GetAddressConvention());
            OUString aFuncResult = aCellStr1 + ":" + aCellStr2;
            PushString( aFuncResult );
        }

// *** CELL PROPERTIES ***
        else if( aInfoType == "CONTENTS" )
        {   // contents of the cell, no formatting
            if (aCell.hasString())
            {
                svl::SharedString aStr;
                GetCellString(aStr, aCell);
                PushString( aStr );
            }
            else
                PushDouble(GetCellValue(aCellPos, aCell));
        }
        else if( aInfoType == "TYPE" )
        {   // b = blank; l = string (label); v = otherwise (value)
            sal_Unicode c;
            if (aCell.hasString())
                c = 'l';
            else
                c = aCell.hasNumeric() ? 'v' : 'b';
            PushString( OUString(c) );
        }
        else if( aInfoType == "WIDTH" )
        {   // column width (rounded off as count of zero characters in standard font and size)
            Printer*    pPrinter = mrDoc.GetPrinter();
            MapMode     aOldMode( pPrinter->GetMapMode() );
            vcl::Font   aOldFont( pPrinter->GetFont() );
            vcl::Font   aDefFont;

            pPrinter->SetMapMode(MapMode(MapUnit::MapTwip));
            // font color doesn't matter here
            mrDoc.getCellAttributeHelper().getDefaultCellAttribute().fillFontOnly(aDefFont, pPrinter);
            pPrinter->SetFont(aDefFont);
            tools::Long nZeroWidth = pPrinter->GetTextWidth( OUString( '0' ) );
            assert(nZeroWidth != 0);
            pPrinter->SetFont( aOldFont );
            pPrinter->SetMapMode( aOldMode );
            int nZeroCount = static_cast<int>(mrDoc.GetColWidth( aCellPos.Col(), aCellPos.Tab() ) / nZeroWidth);
            PushInt( nZeroCount );
        }
        else if( aInfoType == "PREFIX" )
        {   // ' = left; " = right; ^ = centered
            sal_Unicode c = 0;
            if (aCell.hasString())
            {
                const SvxHorJustifyItem* pJustAttr = mrDoc.GetAttr( aCellPos, ATTR_HOR_JUSTIFY );
                switch( pJustAttr->GetValue() )
                {
                    case SvxCellHorJustify::Standard:
                    case SvxCellHorJustify::Left:
                    case SvxCellHorJustify::Block:     c = '\''; break;
                    case SvxCellHorJustify::Center:    c = '^';  break;
                    case SvxCellHorJustify::Right:     c = '"';  break;
                    case SvxCellHorJustify::Repeat:    c = '\\'; break;
                }
            }
            PushString( OUString(c) );
        }
        else if( aInfoType == "PROTECT" )
        {   // 1 = cell locked
            const ScProtectionAttr* pProtAttr = mrDoc.GetAttr( aCellPos, ATTR_PROTECTION );
            PushInt( pProtAttr->GetProtection() ? 1 : 0 );
        }

// *** FORMATTING ***
        else if( aInfoType == "FORMAT" )
        {   // specific format code for standard formats
            OUString aFuncResult;
            sal_uInt32 nFormat = mrDoc.GetNumberFormat( aCellPos );
            getFormatString(mrContext, nFormat, aFuncResult);
            PushString( aFuncResult );
        }
        else if( aInfoType == "COLOR" )
        {   // 1 = negative values are colored, otherwise 0
            const SvNumberformat* pFormat = mrContext.NFGetFormatEntry( mrDoc.GetNumberFormat( aCellPos ) );
            PushInt( lcl_FormatHasNegColor( pFormat ) ? 1 : 0 );
        }
        else if( aInfoType == "PARENTHESES" )
        {   // 1 = format string contains a '(' character, otherwise 0
            const SvNumberformat* pFormat = mrContext.NFGetFormatEntry( mrDoc.GetNumberFormat( aCellPos ) );
            PushInt( lcl_FormatHasOpenPar( pFormat ) ? 1 : 0 );
        }
        else
            PushIllegalArgument();
    }
}

void ScInterpreter::ScCellExternal()
{
    sal_uInt16 nFileId;
    OUString aTabName;
    ScSingleRefData aRef;
    ScExternalRefCache::TokenRef pToken;
    ScExternalRefCache::CellFormat aFmt;
    PopExternalSingleRef(nFileId, aTabName, aRef, pToken, &aFmt);
    if (nGlobalError != FormulaError::NONE)
    {
        PushError( nGlobalError);
        return;
    }

    OUString aInfoType = GetString().getString();
    if (nGlobalError != FormulaError::NONE)
    {
        PushError( nGlobalError);
        return;
    }

    SCCOL nCol;
    SCROW nRow;
    SCTAB nTab;
    aRef.SetAbsTab(0); // external ref has a tab index of -1, which SingleRefToVars() don't like.
    SingleRefToVars(aRef, nCol, nRow, nTab);
    if (nGlobalError != FormulaError::NONE)
    {
        PushIllegalParameter();
        return;
    }
    aRef.SetAbsTab(-1); // revert the value.

    ScCellKeywordTranslator::transKeyword(aInfoType, &ScGlobal::GetLocale(), ocCell);
    ScExternalRefManager* pRefMgr = mrDoc.GetExternalRefManager();

    if ( aInfoType == "COL" )
        PushInt(nCol + 1);
    else if ( aInfoType == "ROW" )
        PushInt(nRow + 1);
    else if ( aInfoType == "SHEET" )
    {
        // For SHEET, No idea what number we should set, but let's always set
        // 1 if the external sheet exists, no matter what sheet.  Excel does
        // the same.
        if (pRefMgr->getCacheTable(nFileId, aTabName, false))
            PushInt(1);
        else
            SetError(FormulaError::NoName);
    }
    else if ( aInfoType == "ADDRESS" )
    {
        // ODF 1.2 says we need to always display address using the ODF A1 grammar.
        ScTokenArray aArray(mrDoc);
        aArray.AddExternalSingleReference(nFileId, svl::SharedString( aTabName), aRef); // string not interned
        ScCompiler aComp(mrDoc, aPos, aArray, formula::FormulaGrammar::GRAM_ODFF_A1);
        OUString aStr;
        aComp.CreateStringFromTokenArray(aStr);
        PushString(aStr);
    }
    else if ( aInfoType == "FILENAME" )
    {
        const OUString* p = pRefMgr->getExternalFileName(nFileId);
        if (!p)
        {
            // In theory this should never happen...
            SetError(FormulaError::NoName);
            return;
        }

        OUString aBuf;
        FormulaGrammar::AddressConvention eConv = maCalcConfig.meStringRefAddressSyntax;
        if (eConv == FormulaGrammar::CONV_UNSPECIFIED)
            eConv = mrDoc.GetAddressConvention();

        if (eConv == FormulaGrammar::CONV_XL_A1 ||
            eConv == FormulaGrammar::CONV_XL_R1C1 ||
            eConv == FormulaGrammar::CONV_XL_OOX)
        {
            // 'file URI/[FileName]SheetName
            sal_Int32 nPos = p->lastIndexOf('/');
            aBuf = OUString::Concat(p->subView(0, nPos + 1))
                + "[" + p->subView(nPos + 1) + "]"
                + aTabName;
        }
        else
        {
            // 'file URI'#$SheetName
            aBuf = "'" + *p + "'#$" + aTabName;
        }

        PushString(aBuf);
    }
    else if ( aInfoType == "CONTENTS" )
    {
        switch (pToken->GetType())
        {
            case svString:
                PushString(pToken->GetString());
            break;
            case svDouble:
                PushString(OUString::number(pToken->GetDouble()));
            break;
            case svError:
                PushString(ScGlobal::GetErrorString(pToken->GetError()));
            break;
            default:
                PushString(OUString());
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
        getFormatString(mrContext, nFmt, aFmtStr);
        PushString(aFmtStr);
    }
    else if ( aInfoType == "COLOR" )
    {
        // 1 = negative values are colored, otherwise 0
        int nVal = 0;
        if (aFmt.mbIsSet)
        {
            const SvNumberformat* pFormat = mrContext.NFGetFormatEntry(aFmt.mnIndex);
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
            const SvNumberformat* pFormat = mrContext.NFGetFormatEntry(aFmt.mnIndex);
            nVal = lcl_FormatHasOpenPar(pFormat) ? 1 : 0;
        }
        PushInt(nVal);
    }
    else
        PushIllegalParameter();
}

void ScInterpreter::ScIsRef()
{
    nFuncFmtType = SvNumFormatType::LOGICAL;
    bool bRes = false;
    switch ( GetStackType() )
    {
        case svSingleRef :
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            if ( nGlobalError == FormulaError::NONE )
                bRes = true;
        }
        break;
        case svDoubleRef :
        {
            ScRange aRange;
            PopDoubleRef( aRange );
            if ( nGlobalError == FormulaError::NONE )
                bRes = true;
        }
        break;
        case svRefList :
        {
            FormulaConstTokenRef x = PopToken();
            if ( nGlobalError == FormulaError::NONE )
                bRes = !x->GetRefList()->empty();
        }
        break;
        case svExternalSingleRef:
        {
            ScExternalRefCache::TokenRef pToken;
            PopExternalSingleRef(pToken);
            if (nGlobalError == FormulaError::NONE)
                bRes = true;
        }
        break;
        case svExternalDoubleRef:
        {
            ScExternalRefCache::TokenArrayRef pArray;
            PopExternalDoubleRef(pArray);
            if (nGlobalError == FormulaError::NONE)
                bRes = true;
        }
        break;
        default:
            Pop();
    }
    nGlobalError = FormulaError::NONE;
    PushInt( int(bRes) );
}

void ScInterpreter::ScIsValue()
{
    nFuncFmtType = SvNumFormatType::LOGICAL;
    bool bRes = false;
    switch ( GetRawStackType() )
    {
        case svDouble:
            Pop();
            bRes = true;
        break;
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;

            ScRefCellValue aCell(mrDoc, aAdr);
            if (GetCellErrCode(aCell) == FormulaError::NONE)
            {
                switch (aCell.getType())
                {
                    case CELLTYPE_VALUE :
                        bRes = true;
                        break;
                    case CELLTYPE_FORMULA :
                        bRes = (aCell.getFormula()->IsValue() && !aCell.getFormula()->IsEmpty());
                        break;
                    default:
                        ; // nothing
                }
            }
        }
        break;
        case svExternalSingleRef:
        {
            ScExternalRefCache::TokenRef pToken;
            PopExternalSingleRef(pToken);
            if (nGlobalError == FormulaError::NONE && pToken->GetType() == svDouble)
                bRes = true;
        }
        break;
        case svExternalDoubleRef:
        case svMatrix:
        {
            ScMatrixRef pMat = GetMatrix();
            if ( !pMat )
                ;   // nothing
            else if ( !pJumpMatrix )
            {
                if (pMat->GetErrorIfNotString( 0, 0) == FormulaError::NONE)
                    bRes = pMat->IsValue( 0, 0);
            }
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    if (pMat->GetErrorIfNotString( nC, nR) == FormulaError::NONE)
                        bRes = pMat->IsValue( nC, nR);
            }
        }
        break;
        default:
            Pop();
    }
    nGlobalError = FormulaError::NONE;
    PushInt( int(bRes) );
}

void ScInterpreter::ScIsFormula()
{
    nFuncFmtType = SvNumFormatType::LOGICAL;
    bool bRes = false;
    switch ( GetStackType() )
    {
        case svDoubleRef :
            if (IsInArrayContext())
            {
                SCCOL nCol1, nCol2;
                SCROW nRow1, nRow2;
                SCTAB nTab1, nTab2;
                PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                if (nGlobalError != FormulaError::NONE)
                {
                    PushError( nGlobalError);
                    return;
                }
                if (nTab1 != nTab2)
                {
                    PushIllegalArgument();
                    return;
                }

                ScMatrixRef pResMat = GetNewMat( static_cast<SCSIZE>(nCol2 - nCol1 + 1),
                        static_cast<SCSIZE>(nRow2 - nRow1 + 1), true);
                if (!pResMat)
                {
                    PushError( FormulaError::MatrixSize);
                    return;
                }

                /* TODO: we really should have a gap-aware cell iterator. */
                SCSIZE i=0, j=0;
                ScAddress aAdr( 0, 0, nTab1);
                for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
                {
                    aAdr.SetCol(nCol);
                    for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
                    {
                        aAdr.SetRow(nRow);
                        ScRefCellValue aCell(mrDoc, aAdr);
                        pResMat->PutBoolean( (aCell.getType() == CELLTYPE_FORMULA), i,j);
                        ++j;
                    }
                    ++i;
                    j = 0;
                }

                PushMatrix( pResMat);
                return;
            }
        [[fallthrough]];
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;

            bRes = (mrDoc.GetCellType(aAdr) == CELLTYPE_FORMULA);
        }
        break;
        default:
            Pop();
    }
    nGlobalError = FormulaError::NONE;
    PushInt( int(bRes) );
}

void ScInterpreter::ScFormula()
{
    OUString aFormula;
    switch ( GetStackType() )
    {
        case svDoubleRef :
            if (IsInArrayContext())
            {
                SCCOL nCol1, nCol2;
                SCROW nRow1, nRow2;
                SCTAB nTab1, nTab2;
                PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                if (nGlobalError != FormulaError::NONE)
                    break;

                if (nTab1 != nTab2)
                {
                    SetError( FormulaError::IllegalArgument);
                    break;
                }

                ScMatrixRef pResMat = GetNewMat( nCol2 - nCol1 + 1, nRow2 - nRow1 + 1, true);
                if (!pResMat)
                    break;

                /* TODO: use a column iterator instead? */
                SCSIZE i=0, j=0;
                ScAddress aAdr(0,0,nTab1);
                for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
                {
                    aAdr.SetCol(nCol);
                    for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
                    {
                        aAdr.SetRow(nRow);
                        ScRefCellValue aCell(mrDoc, aAdr);
                        switch (aCell.getType())
                        {
                            case CELLTYPE_FORMULA :
                                aFormula = aCell.getFormula()->GetFormula(formula::FormulaGrammar::GRAM_UNSPECIFIED, &mrContext);
                                pResMat->PutString( mrStrPool.intern( aFormula), i,j);
                                break;
                            default:
                                pResMat->PutError( FormulaError::NotAvailable, i,j);
                        }
                        ++j;
                    }
                    ++i;
                    j = 0;
                }

                PushMatrix( pResMat);
                return;
            }
            [[fallthrough]];
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;

            ScRefCellValue aCell(mrDoc, aAdr);
            switch (aCell.getType())
            {
                case CELLTYPE_FORMULA :
                    aFormula = aCell.getFormula()->GetFormula(formula::FormulaGrammar::GRAM_UNSPECIFIED, &mrContext);
                break;
                default:
                    SetError( FormulaError::NotAvailable );
            }
        }
        break;
        default:
            PopError();
            SetError( FormulaError::NotAvailable );
    }
    PushString( aFormula );
}

void ScInterpreter::ScIsNV()
{
    nFuncFmtType = SvNumFormatType::LOGICAL;
    bool bRes = false;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            bool bOk = PopDoubleRefOrSingleRef( aAdr );
            if ( nGlobalError == FormulaError::NotAvailable )
                bRes = true;
            else if (bOk)
            {
                ScRefCellValue aCell(mrDoc, aAdr);
                FormulaError nErr = GetCellErrCode(aCell);
                bRes = (nErr == FormulaError::NotAvailable);
            }
        }
        break;
        case svExternalSingleRef:
        {
            ScExternalRefCache::TokenRef pToken;
            PopExternalSingleRef(pToken);
            if (nGlobalError == FormulaError::NotAvailable ||
                    (pToken && pToken->GetType() == svError && pToken->GetError() == FormulaError::NotAvailable))
                bRes = true;
        }
        break;
        case svExternalDoubleRef:
        case svMatrix:
        {
            ScMatrixRef pMat = GetMatrix();
            if ( !pMat )
                ;   // nothing
            else if ( !pJumpMatrix )
                bRes = (pMat->GetErrorIfNotString( 0, 0) == FormulaError::NotAvailable);
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    bRes = (pMat->GetErrorIfNotString( nC, nR) == FormulaError::NotAvailable);
            }
        }
        break;
        default:
            PopError();
            if ( nGlobalError == FormulaError::NotAvailable )
                bRes = true;
    }
    nGlobalError = FormulaError::NONE;
    PushInt( int(bRes) );
}

void ScInterpreter::ScIsErr()
{
    nFuncFmtType = SvNumFormatType::LOGICAL;
    bool bRes = false;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            bool bOk = PopDoubleRefOrSingleRef( aAdr );
            if ( !bOk || (nGlobalError != FormulaError::NONE && nGlobalError != FormulaError::NotAvailable) )
                bRes = true;
            else
            {
                ScRefCellValue aCell(mrDoc, aAdr);
                FormulaError nErr = GetCellErrCode(aCell);
                bRes = (nErr != FormulaError::NONE && nErr != FormulaError::NotAvailable);
            }
        }
        break;
        case svExternalSingleRef:
        {
            ScExternalRefCache::TokenRef pToken;
            PopExternalSingleRef(pToken);
            if ((nGlobalError != FormulaError::NONE && nGlobalError != FormulaError::NotAvailable) || !pToken ||
                    (pToken->GetType() == svError && pToken->GetError() != FormulaError::NotAvailable))
                bRes = true;
        }
        break;
        case svExternalDoubleRef:
        case svMatrix:
        {
            ScMatrixRef pMat = GetMatrix();
            if ( nGlobalError != FormulaError::NONE || !pMat )
                bRes = ((nGlobalError != FormulaError::NONE && nGlobalError != FormulaError::NotAvailable) || !pMat);
            else if ( !pJumpMatrix )
            {
                FormulaError nErr = pMat->GetErrorIfNotString( 0, 0);
                bRes = (nErr != FormulaError::NONE && nErr != FormulaError::NotAvailable);
            }
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                {
                    FormulaError nErr = pMat->GetErrorIfNotString( nC, nR);
                    bRes = (nErr != FormulaError::NONE && nErr != FormulaError::NotAvailable);
                }
            }
        }
        break;
        default:
            PopError();
            if ( nGlobalError != FormulaError::NONE && nGlobalError != FormulaError::NotAvailable )
                bRes = true;
    }
    nGlobalError = FormulaError::NONE;
    PushInt( int(bRes) );
}

void ScInterpreter::ScIsError()
{
    nFuncFmtType = SvNumFormatType::LOGICAL;
    bool bRes = false;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
            {
                bRes = true;
                break;
            }
            if ( nGlobalError != FormulaError::NONE )
                bRes = true;
            else
            {
                ScRefCellValue aCell(mrDoc, aAdr);
                bRes = (GetCellErrCode(aCell) != FormulaError::NONE);
            }
        }
        break;
        case svExternalSingleRef:
        {
            ScExternalRefCache::TokenRef pToken;
            PopExternalSingleRef(pToken);
            if (nGlobalError != FormulaError::NONE || pToken->GetType() == svError)
                bRes = true;
        }
        break;
        case svExternalDoubleRef:
        case svMatrix:
        {
            ScMatrixRef pMat = GetMatrix();
            if ( nGlobalError != FormulaError::NONE || !pMat )
                bRes = true;
            else if ( !pJumpMatrix )
                bRes = (pMat->GetErrorIfNotString( 0, 0) != FormulaError::NONE);
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                    bRes = (pMat->GetErrorIfNotString( nC, nR) != FormulaError::NONE);
            }
        }
        break;
        default:
            PopError();
            if ( nGlobalError != FormulaError::NONE )
                bRes = true;
    }
    nGlobalError = FormulaError::NONE;
    PushInt( int(bRes) );
}

bool ScInterpreter::IsEven()
{
    nFuncFmtType = SvNumFormatType::LOGICAL;
    bool bRes = false;
    double fVal = 0.0;
    switch ( GetStackType() )
    {
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
                break;

            ScRefCellValue aCell(mrDoc, aAdr);
            FormulaError nErr = GetCellErrCode(aCell);
            if (nErr != FormulaError::NONE)
                SetError(nErr);
            else
            {
                switch (aCell.getType())
                {
                    case CELLTYPE_VALUE :
                        fVal = GetCellValue(aAdr, aCell);
                        bRes = true;
                    break;
                    case CELLTYPE_FORMULA :
                        if (aCell.getFormula()->IsValue())
                        {
                            fVal = GetCellValue(aAdr, aCell);
                            bRes = true;
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
            bRes = true;
        }
        break;
        case svExternalSingleRef:
        {
            ScExternalRefCache::TokenRef pToken;
            PopExternalSingleRef(pToken);
            if (nGlobalError == FormulaError::NONE && pToken->GetType() == svDouble)
            {
                fVal = pToken->GetDouble();
                bRes = true;
            }
        }
        break;
        case svExternalDoubleRef:
        case svMatrix:
        {
            ScMatrixRef pMat = GetMatrix();
            if ( !pMat )
                ;   // nothing
            else if ( !pJumpMatrix )
            {
                bRes = pMat->IsValue( 0, 0);
                if ( bRes )
                    fVal = pMat->GetDouble( 0, 0);
            }
            else
            {
                SCSIZE nCols, nRows, nC, nR;
                pMat->GetDimensions( nCols, nRows);
                pJumpMatrix->GetPos( nC, nR);
                if ( nC < nCols && nR < nRows )
                {
                    bRes = pMat->IsValue( nC, nR);
                    if ( bRes )
                        fVal = pMat->GetDouble( nC, nR);
                }
                else
                    SetError( FormulaError::NoValue);
            }
        }
        break;
        default:
            ; // nothing
    }
    if ( !bRes )
        SetError( FormulaError::IllegalParameter);
    else
        bRes = ( fmod( ::rtl::math::approxFloor( fabs( fVal ) ), 2.0 ) < 0.5 );
    return bRes;
}

void ScInterpreter::ScIsEven()
{
    PushInt( int(IsEven()) );
}

void ScInterpreter::ScIsOdd()
{
    PushInt( int(!IsEven()) );
}

void ScInterpreter::ScN()
{
    FormulaError nErr = nGlobalError;
    nGlobalError = FormulaError::NONE;
    // Temporarily override the ConvertStringToValue() error for
    // GetCellValue() / GetCellValueOrZero()
    FormulaError nSErr = mnStringNoValueError;
    mnStringNoValueError = FormulaError::CellNoValue;
    double fVal = GetDouble();
    mnStringNoValueError = nSErr;
    if (nErr != FormulaError::NONE)
        nGlobalError = nErr;    // preserve previous error if any
    else if (nGlobalError == FormulaError::CellNoValue)
        nGlobalError = FormulaError::NONE;       // reset temporary detection error
    PushDouble(fVal);
}

void ScInterpreter::ScTrim()
{
    // Doesn't only trim but also removes duplicated blanks within!
    OUString aVal = comphelper::string::strip(GetString().getString(), ' ');
    OUStringBuffer aStr;
    const sal_Unicode* p = aVal.getStr();
    const sal_Unicode* const pEnd = p + aVal.getLength();
    while ( p < pEnd )
    {
        if ( *p != ' ' || p[-1] != ' ' )    // first can't be ' ', so -1 is fine
            aStr.append(*p);
        p++;
    }
    PushString(aStr.makeStringAndClear());
}

void ScInterpreter::ScUpper()
{
    OUString aString = ScGlobal::getCharClass().uppercase(GetString().getString());
    PushString(aString);
}

void ScInterpreter::ScProper()
{
//2do: what to do with I18N-CJK ?!?
    OUStringBuffer aStr(GetString().getString());
    const sal_Int32 nLen = aStr.getLength();
    if ( nLen > 0 )
    {
        OUString aUpr(ScGlobal::getCharClass().uppercase(aStr.toString()));
        OUString aLwr(ScGlobal::getCharClass().lowercase(aStr.toString()));
        aStr[0] = aUpr[0];
        sal_Int32 nPos = 1;
        while( nPos < nLen )
        {
            OUString aTmpStr( aStr[nPos-1] );
            if ( !ScGlobal::getCharClass().isLetter( aTmpStr, 0 ) )
                aStr[nPos] = aUpr[nPos];
            else
                aStr[nPos] = aLwr[nPos];
            ++nPos;
        }
    }
    PushString(aStr.makeStringAndClear());
}

void ScInterpreter::ScLower()
{
    OUString aString = ScGlobal::getCharClass().lowercase(GetString().getString());
    PushString(aString);
}

void ScInterpreter::ScLen()
{
    OUString aStr = GetString().getString();
    sal_Int32 nIdx = 0;
    sal_Int32 nCnt = 0;
    while ( nIdx < aStr.getLength() )
    {
        aStr.iterateCodePoints( &nIdx );
        ++nCnt;
    }
    PushDouble( nCnt );
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
            ScRefCellValue aCell(mrDoc, aAdr);
            if (GetCellErrCode(aCell) == FormulaError::NONE)
            {
                switch (aCell.getType())
                {
                    case CELLTYPE_VALUE :
                        bValue = true;
                        break;
                    case CELLTYPE_FORMULA :
                        bValue = aCell.getFormula()->IsValue();
                        break;
                    default:
                        ; // nothing
                }
            }
            if ( bValue )
                PushString(OUString());
            else
            {
                // like GetString()
                svl::SharedString aStr;
                GetCellString(aStr, aCell);
                PushString(aStr);
            }
        }
        break;
        case svMatrix:
        case svExternalSingleRef:
        case svExternalDoubleRef:
        {
            double fVal;
            svl::SharedString aStr;
            ScMatValType nMatValType = GetDoubleOrStringFromMatrix( fVal, aStr);
            if (ScMatrix::IsValueType( nMatValType))
                PushString(svl::SharedString::getEmptyString());
            else
                PushString( aStr);
        }
        break;
        case svDouble :
        {
            PopError();
            PushString( OUString() );
        }
        break;
        case svString :
            ;   // leave on stack
        break;
        default :
            PushError( FormulaError::UnknownOpCode);
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

        case svSingleRef:
        case svDoubleRef:
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
            {
                PushInt(0);
                return;
            }
            ScRefCellValue aCell(mrDoc, aAdr);
            if (aCell.hasString())
            {
                svl::SharedString aSS;
                GetCellString(aSS, aCell);
                aInputString = aSS.getString();
            }
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
                svl::SharedString aSS;
                ScMatValType nType = GetDoubleOrStringFromMatrix( fVal,
                        aSS);
                aInputString = aSS.getString();
                switch (nType)
                {
                    case ScMatValType::Empty:
                        fVal = 0.0;
                        [[fallthrough]];
                    case ScMatValType::Value:
                    case ScMatValType::Boolean:
                        PushDouble( fVal);
                        return;
                    case ScMatValType::String:
                        // evaluated below
                        break;
                    default:
                        PushIllegalArgument();
                }
            }
            break;
        default:
            aInputString = GetString().getString();
            break;
    }

    sal_uInt32 nFIndex = 0;     // 0 for default locale
    if (mrContext.NFIsNumberFormat(aInputString, nFIndex, fVal))
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
    OUString aGroupSeparator;
    sal_Unicode cDecimalSeparator = 0;

    if ( nParamCount == 3 )
        aGroupSeparator = GetString().getString();

    if ( nParamCount >= 2 )
    {
        OUString aDecimalSeparator = GetString().getString();
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
        aInputString = GetString().getString();
    }
    if ( nGlobalError != FormulaError::NONE )
    {
        PushError( nGlobalError );
        return;
    }
    if ( aInputString.isEmpty() )
    {
        if ( maCalcConfig.mbEmptyStringAsZero )
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
        while (nIndex < aGroupSeparator.getLength())
        {
            sal_uInt32 nChar = aGroupSeparator.iterateCodePoints( &nIndex );
            aTemporary = aTemporary.replaceAll( OUString( &nChar, 1 ), "" );
        }
        if ( nDecSep >= 0 )
            aInputString = aTemporary + aInputString.subView( nDecSep );
        else
            aInputString = aTemporary;
    }

    for ( sal_Int32 i = aInputString.getLength(); --i >= 0; )
    {
        sal_Unicode c = aInputString[ i ];
        if ( c == 0x0020 || c == 0x0009 || c == 0x000A || c == 0x000D )
            aInputString = aInputString.replaceAt( i, 1, u"" ); // remove spaces etc.
    }
    sal_Int32 nPercentCount = 0;
    for ( sal_Int32 i = aInputString.getLength() - 1; i >= 0 && aInputString[ i ] == 0x0025; i-- )
    {
        aInputString = aInputString.replaceAt( i, 1, u"" );  // remove and count trailing '%'
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

static bool lcl_ScInterpreter_IsPrintable( sal_uInt32 nCodePoint )
{
    return ( !u_isISOControl(nCodePoint) /*not in Cc*/
             && u_isdefined(nCodePoint)  /*not in Cn*/ );
}


void ScInterpreter::ScClean()
{
    OUString aStr = GetString().getString();

    OUStringBuffer aBuf( aStr.getLength() );
    sal_Int32 nIdx = 0;
    while ( nIdx <  aStr.getLength() )
    {
        sal_uInt32 c = aStr.iterateCodePoints( &nIdx );
        if ( lcl_ScInterpreter_IsPrintable( c ) )
            aBuf.appendUtf32( c );
    }
    PushString( aBuf.makeStringAndClear() );
}


void ScInterpreter::ScCode()
{
//2do: make it full range unicode?
    OUString aStr = GetString().getString();
    if (aStr.isEmpty())
        PushInt(0);
    else
    {
        //"classic" ByteString conversion flags
        const sal_uInt32 convertFlags =
            RTL_UNICODETOTEXT_FLAGS_NONSPACING_IGNORE |
            RTL_UNICODETOTEXT_FLAGS_CONTROL_IGNORE |
            RTL_UNICODETOTEXT_FLAGS_FLUSH |
            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT |
            RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT |
            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_REPLACE;
        PushInt( static_cast<unsigned char>(OUStringToOString(OUStringChar(aStr[0]), osl_getThreadTextEncoding(), convertFlags).toChar()) );
    }
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

        char cEncodedChar = static_cast<char>(fVal);
        OUString aStr(&cEncodedChar, 1,  osl_getThreadTextEncoding(), convertFlags);
        PushString(aStr);
    }
}

/* #i70213# fullwidth/halfwidth conversion provided by
 * Takashi Nakamoto <bluedwarf@ooo>
 * erAck: added Excel compatibility conversions as seen in issue's test case. */

static OUString lcl_convertIntoHalfWidth( const OUString & rStr )
{
    // Make the initialization thread-safe. Since another function needs to be called, move it all to another
    // function and thread-safely initialize a static reference in this function.
    auto init = []() -> utl::TransliterationWrapper&
        {
        static utl::TransliterationWrapper trans( ::comphelper::getProcessComponentContext(), TransliterationFlags::NONE );
        trans.loadModuleByImplName( u"FULLWIDTH_HALFWIDTH_LIKE_ASC"_ustr, LANGUAGE_SYSTEM );
        return trans;
        };
    static utl::TransliterationWrapper& aTrans( init());
    return aTrans.transliterate( rStr, 0, sal_uInt16( rStr.getLength() ) );
}

static OUString lcl_convertIntoFullWidth( const OUString & rStr )
{
    auto init = []() -> utl::TransliterationWrapper&
        {
        static utl::TransliterationWrapper trans( ::comphelper::getProcessComponentContext(), TransliterationFlags::NONE );
        trans.loadModuleByImplName( u"HALFWIDTH_FULLWIDTH_LIKE_JIS"_ustr, LANGUAGE_SYSTEM );
        return trans;
        };
    static utl::TransliterationWrapper& aTrans( init());
    return aTrans.transliterate( rStr, 0, sal_uInt16( rStr.getLength() ) );
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
        PushString( lcl_convertIntoFullWidth( GetString().getString()));
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
        PushString( lcl_convertIntoHalfWidth( GetString().getString()));
}

void ScInterpreter::ScUnicode()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        OUString aStr = GetString().getString();
        if (aStr.isEmpty())
            PushIllegalParameter();
        else
        {
            PushDouble(aStr.iterateCodePoints(&o3tl::temporary(sal_Int32(0))));
        }
    }
}

void ScInterpreter::ScUnichar()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        sal_uInt32 nCodePoint = GetUInt32();
        if (nGlobalError != FormulaError::NONE || !rtl::isUnicodeCodePoint(nCodePoint))
            PushIllegalArgument();
        else
        {
            OUString aStr( &nCodePoint, 1 );
            PushString( aStr );
        }
    }
}

bool ScInterpreter::SwitchToArrayRefList( ScMatrixRef& xResMat, SCSIZE nMatRows, double fCurrent,
        const std::function<void( SCSIZE i, double fCurrent )>& MatOpFunc, bool bDoMatOp )
{
    const ScRefListToken* p = dynamic_cast<const ScRefListToken*>(pStack[sp-1]);
    if (!p || !p->IsArrayResult())
        return false;

    if (!xResMat)
    {
        // Create and init all elements with current value.
        assert(nMatRows > 0);
        xResMat = GetNewMat( 1, nMatRows, true);
        xResMat->FillDouble( fCurrent, 0,0, 0,nMatRows-1);
    }
    else if (bDoMatOp)
    {
        // Current value and values from vector are operands
        // for each vector position.
        for (SCSIZE i=0; i < nMatRows; ++i)
        {
            MatOpFunc( i, fCurrent);
        }
    }
    return true;
}

void ScInterpreter::ScMin( bool bTextAsZero )
{
    short nParamCount = GetByte();
    if (!MustHaveParamCountMin( nParamCount, 1))
        return;

    ScMatrixRef xResMat;
    double nMin = ::std::numeric_limits<double>::max();
    auto MatOpFunc = [&xResMat]( SCSIZE i, double fCurMin )
    {
        double fVecRes = xResMat->GetDouble(0,i);
        if (fVecRes > fCurMin)
            xResMat->PutDouble( fCurMin, 0,i);
    };
    const SCSIZE nMatRows = GetRefListArrayMaxSize( nParamCount);
    size_t nRefArrayPos = std::numeric_limits<size_t>::max();

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
                nFuncFmtType = SvNumFormatType::NUMBER;
            }
            break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScRefCellValue aCell(mrDoc, aAdr);
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
            case svRefList :
            {
                // bDoMatOp only for non-array value when switching to
                // ArrayRefList.
                if (SwitchToArrayRefList( xResMat, nMatRows, nMin, MatOpFunc,
                            nRefArrayPos == std::numeric_limits<size_t>::max()))
                {
                    nRefArrayPos = nRefInList;
                }
            }
            [[fallthrough]];
            case svDoubleRef :
            {
                FormulaError nErr = FormulaError::NONE;
                PopDoubleRef( aRange, nParamCount, nRefInList);
                ScValueIterator aValIter( mrContext, aRange, mnSubTotalFlags, bTextAsZero );
                if (aValIter.GetFirst(nVal, nErr))
                {
                    if (nMin > nVal)
                        nMin = nVal;
                    aValIter.GetCurNumFmtInfo( nFuncFmtType, nFuncFmtIndex );
                    while ((nErr == FormulaError::NONE) && aValIter.GetNext(nVal, nErr))
                    {
                        if (nMin > nVal)
                            nMin = nVal;
                    }
                    SetError(nErr);
                }
                if (nRefArrayPos != std::numeric_limits<size_t>::max())
                {
                    // Update vector element with current value.
                    MatOpFunc( nRefArrayPos, nMin);

                    // Reset.
                    nMin = std::numeric_limits<double>::max();
                    nVal = 0.0;
                    nRefArrayPos = std::numeric_limits<size_t>::max();
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
                    nFuncFmtType = SvNumFormatType::NUMBER;
                    nVal = pMat->GetMinValue(bTextAsZero, bool(mnSubTotalFlags & SubtotalFlags::IgnoreErrVal));
                    if (nMin > nVal)
                        nMin = nVal;
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
                    SetError(FormulaError::IllegalParameter);
            }
            break;
            default :
                PopError();
                SetError(FormulaError::IllegalParameter);
        }
    }

    if (xResMat)
    {
        // Include value of last non-references-array type and calculate final result.
        if (nMin < std::numeric_limits<double>::max())
        {
            for (SCSIZE i=0; i < nMatRows; ++i)
            {
                MatOpFunc( i, nMin);
            }
        }
        else
        {
            /* TODO: the awkward "no value is minimum 0.0" is likely the case
             * if a value is numeric_limits::max. Still, that could be a valid
             * minimum value as well, but nVal and nMin had been reset after
             * the last svRefList... so we may lie here. */
            for (SCSIZE i=0; i < nMatRows; ++i)
            {
                double fVecRes = xResMat->GetDouble(0,i);
                if (fVecRes == std::numeric_limits<double>::max())
                    xResMat->PutDouble( 0.0, 0,i);
            }
        }
        PushMatrix( xResMat);
    }
    else
    {
        if (!std::isfinite(nVal))
            PushError( GetDoubleErrorValue( nVal));
        else if ( nVal < nMin  )
            PushDouble(0.0);    // zero or only empty arguments
        else
            PushDouble(nMin);
    }
}

void ScInterpreter::ScMax( bool bTextAsZero )
{
    short nParamCount = GetByte();
    if (!MustHaveParamCountMin( nParamCount, 1))
        return;

    ScMatrixRef xResMat;
    double nMax = std::numeric_limits<double>::lowest();
    auto MatOpFunc = [&xResMat]( SCSIZE i, double fCurMax )
    {
        double fVecRes = xResMat->GetDouble(0,i);
        if (fVecRes < fCurMax)
            xResMat->PutDouble( fCurMax, 0,i);
    };
    const SCSIZE nMatRows = GetRefListArrayMaxSize( nParamCount);
    size_t nRefArrayPos = std::numeric_limits<size_t>::max();

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
                nFuncFmtType = SvNumFormatType::NUMBER;
            }
            break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScRefCellValue aCell(mrDoc, aAdr);
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
            case svRefList :
            {
                // bDoMatOp only for non-array value when switching to
                // ArrayRefList.
                if (SwitchToArrayRefList( xResMat, nMatRows, nMax, MatOpFunc,
                            nRefArrayPos == std::numeric_limits<size_t>::max()))
                {
                    nRefArrayPos = nRefInList;
                }
            }
            [[fallthrough]];
            case svDoubleRef :
            {
                FormulaError nErr = FormulaError::NONE;
                PopDoubleRef( aRange, nParamCount, nRefInList);
                ScValueIterator aValIter( mrContext, aRange, mnSubTotalFlags, bTextAsZero );
                if (aValIter.GetFirst(nVal, nErr))
                {
                    if (nMax < nVal)
                        nMax = nVal;
                    aValIter.GetCurNumFmtInfo( nFuncFmtType, nFuncFmtIndex );
                    while ((nErr == FormulaError::NONE) && aValIter.GetNext(nVal, nErr))
                    {
                        if (nMax < nVal)
                            nMax = nVal;
                    }
                    SetError(nErr);
                }
                if (nRefArrayPos != std::numeric_limits<size_t>::max())
                {
                    // Update vector element with current value.
                    MatOpFunc( nRefArrayPos, nMax);

                    // Reset.
                    nMax = std::numeric_limits<double>::lowest();
                    nVal = 0.0;
                    nRefArrayPos = std::numeric_limits<size_t>::max();
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
                    nFuncFmtType = SvNumFormatType::NUMBER;
                    nVal = pMat->GetMaxValue(bTextAsZero, bool(mnSubTotalFlags & SubtotalFlags::IgnoreErrVal));
                    if (nMax < nVal)
                        nMax = nVal;
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
                    SetError(FormulaError::IllegalParameter);
            }
            break;
            default :
                PopError();
                SetError(FormulaError::IllegalParameter);
        }
    }

    if (xResMat)
    {
        // Include value of last non-references-array type and calculate final result.
        if (nMax > std::numeric_limits<double>::lowest())
        {
            for (SCSIZE i=0; i < nMatRows; ++i)
            {
                MatOpFunc( i, nMax);
            }
        }
        else
        {
            /* TODO: the awkward "no value is maximum 0.0" is likely the case
             * if a value is numeric_limits::lowest. Still, that could be a
             * valid maximum value as well, but nVal and nMax had been reset
             * after the last svRefList... so we may lie here. */
            for (SCSIZE i=0; i < nMatRows; ++i)
            {
                double fVecRes = xResMat->GetDouble(0,i);
                if (fVecRes == -std::numeric_limits<double>::max())
                    xResMat->PutDouble( 0.0, 0,i);
            }
        }
        PushMatrix( xResMat);
    }
    else
    {
        if (!std::isfinite(nVal))
            PushError( GetDoubleErrorValue( nVal));
        else if ( nVal > nMax  )
            PushDouble(0.0);    // zero or only empty arguments
        else
            PushDouble(nMax);
    }
}

void ScInterpreter::GetStVarParams( bool bTextAsZero, double(*VarResult)( double fVal, size_t nValCount ) )
{
    short nParamCount = GetByte();
    const SCSIZE nMatRows = GetRefListArrayMaxSize( nParamCount);

    struct ArrayRefListValue
    {
        std::vector<double> mvValues;
        KahanSum mfSum;
        ArrayRefListValue() = default;
        double get() const { return mfSum.get(); }
    };
    std::vector<ArrayRefListValue> vArrayValues;

    std::vector<double> values;
    KahanSum fSum    = 0.0;
    double fVal = 0.0;
    ScAddress aAdr;
    ScRange aRange;
    size_t nRefInList = 0;
    while (nGlobalError == FormulaError::NONE && nParamCount-- > 0)
    {
        switch (GetStackType())
        {
            case svDouble :
            {
                fVal = GetDouble();
                if (nGlobalError == FormulaError::NONE)
                {
                    values.push_back(fVal);
                    fSum    += fVal;
                }
            }
            break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScRefCellValue aCell(mrDoc, aAdr);
                if (aCell.hasNumeric())
                {
                    fVal = GetCellValue(aAdr, aCell);
                    if (nGlobalError == FormulaError::NONE)
                    {
                        values.push_back(fVal);
                        fSum += fVal;
                    }
                }
                else if (bTextAsZero && aCell.hasString())
                {
                    values.push_back(0.0);
                }
            }
            break;
            case svRefList :
            {
                const ScRefListToken* p = dynamic_cast<const ScRefListToken*>(pStack[sp-1]);
                if (p && p->IsArrayResult())
                {
                    size_t nRefArrayPos = nRefInList;
                    if (vArrayValues.empty())
                    {
                        // Create and init all elements with current value.
                        assert(nMatRows > 0);
                        vArrayValues.resize(nMatRows);
                        for (ArrayRefListValue & it : vArrayValues)
                        {
                            it.mvValues = values;
                            it.mfSum = fSum;
                        }
                    }
                    else
                    {
                        // Current value and values from vector are operands
                        // for each vector position.
                        for (ArrayRefListValue & it : vArrayValues)
                        {
                            it.mvValues.insert( it.mvValues.end(), values.begin(), values.end());
                            it.mfSum += fSum;
                        }
                    }
                    ArrayRefListValue& rArrayValue = vArrayValues[nRefArrayPos];
                    FormulaError nErr = FormulaError::NONE;
                    PopDoubleRef( aRange, nParamCount, nRefInList);
                    ScValueIterator aValIter( mrContext, aRange, mnSubTotalFlags, bTextAsZero );
                    if (aValIter.GetFirst(fVal, nErr))
                    {
                        do
                        {
                            rArrayValue.mvValues.push_back(fVal);
                            rArrayValue.mfSum += fVal;
                        }
                        while ((nErr == FormulaError::NONE) && aValIter.GetNext(fVal, nErr));
                    }
                    if ( nErr != FormulaError::NONE )
                    {
                        rArrayValue.mfSum = CreateDoubleError( nErr);
                    }
                    // Reset.
                    std::vector<double>().swap(values);
                    fSum = 0.0;
                    break;
                }
            }
            [[fallthrough]];
            case svDoubleRef :
            {
                FormulaError nErr = FormulaError::NONE;
                PopDoubleRef( aRange, nParamCount, nRefInList);
                ScValueIterator aValIter( mrContext, aRange, mnSubTotalFlags, bTextAsZero );
                if (aValIter.GetFirst(fVal, nErr))
                {
                    do
                    {
                        values.push_back(fVal);
                        fSum += fVal;
                    }
                    while ((nErr == FormulaError::NONE) && aValIter.GetNext(fVal, nErr));
                }
                if ( nErr != FormulaError::NONE )
                {
                    SetError(nErr);
                }
            }
            break;
            case svExternalSingleRef :
            case svExternalDoubleRef :
            case svMatrix :
            {
                ScMatrixRef pMat = GetMatrix();
                if (pMat)
                {
                    const bool bIgnoreErrVal = bool(mnSubTotalFlags & SubtotalFlags::IgnoreErrVal);
                    SCSIZE nC, nR;
                    pMat->GetDimensions(nC, nR);
                    for (SCSIZE nMatCol = 0; nMatCol < nC; nMatCol++)
                    {
                        for (SCSIZE nMatRow = 0; nMatRow < nR; nMatRow++)
                        {
                            if (!pMat->IsStringOrEmpty(nMatCol,nMatRow))
                            {
                                fVal= pMat->GetDouble(nMatCol,nMatRow);
                                if (nGlobalError == FormulaError::NONE)
                                {
                                    values.push_back(fVal);
                                    fSum += fVal;
                                }
                                else if (bIgnoreErrVal)
                                    nGlobalError = FormulaError::NONE;
                            }
                            else if ( bTextAsZero )
                            {
                                values.push_back(0.0);
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
                }
                else
                    SetError(FormulaError::IllegalParameter);
            }
            break;
            default :
                PopError();
                SetError(FormulaError::IllegalParameter);
        }
    }

    if (!vArrayValues.empty())
    {
        // Include value of last non-references-array type and calculate final result.
        if (!values.empty())
        {
            for (auto & it : vArrayValues)
            {
                it.mvValues.insert( it.mvValues.end(), values.begin(), values.end());
                it.mfSum += fSum;
            }
        }
        ScMatrixRef xResMat = GetNewMat( 1, nMatRows, true);
        for (SCSIZE r=0; r < nMatRows; ++r)
        {
            ::std::vector<double>::size_type n = vArrayValues[r].mvValues.size();
            if (!n)
                xResMat->PutError( FormulaError::DivisionByZero, 0, r);
            else
            {
                ArrayRefListValue& rArrayValue = vArrayValues[r];
                double vSum = 0.0;
                const double vMean = rArrayValue.get() / n;
                for (::std::vector<double>::size_type i = 0; i < n; i++)
                    vSum += ::rtl::math::approxSub( rArrayValue.mvValues[i], vMean) *
                        ::rtl::math::approxSub( rArrayValue.mvValues[i], vMean);
                xResMat->PutDouble( VarResult( vSum, n), 0, r);
            }
        }
        PushMatrix( xResMat);
    }
    else
    {
        ::std::vector<double>::size_type n = values.size();
        if (!n)
            SetError( FormulaError::DivisionByZero);
        double vSum = 0.0;
        if (nGlobalError == FormulaError::NONE)
        {
            const double vMean = fSum.get() / n;
            for (::std::vector<double>::size_type i = 0; i < n; i++)
                vSum += ::rtl::math::approxSub( values[i], vMean) * ::rtl::math::approxSub( values[i], vMean);
        }
        PushDouble( VarResult( vSum, n));
    }
}

void ScInterpreter::ScVar( bool bTextAsZero )
{
    auto VarResult = []( double fVal, size_t nValCount )
    {
        if (nValCount <= 1)
            return CreateDoubleError( FormulaError::DivisionByZero );
        else
            return fVal / (nValCount - 1);
    };
    GetStVarParams( bTextAsZero, VarResult );
}

void ScInterpreter::ScVarP( bool bTextAsZero )
{
    auto VarResult = []( double fVal, size_t nValCount )
    {
        return sc::div( fVal, nValCount);
    };
    GetStVarParams( bTextAsZero, VarResult );

}

void ScInterpreter::ScStDev( bool bTextAsZero )
{
    auto VarResult = []( double fVal, size_t nValCount )
    {
        if (nValCount <= 1)
            return CreateDoubleError( FormulaError::DivisionByZero );
        else
            return sqrt( fVal / (nValCount - 1));
    };
    GetStVarParams( bTextAsZero, VarResult );
}

void ScInterpreter::ScStDevP( bool bTextAsZero )
{
    auto VarResult = []( double fVal, size_t nValCount )
    {
        if (nValCount == 0)
            return CreateDoubleError( FormulaError::DivisionByZero );
        else
            return sqrt( fVal / nValCount);
    };
    GetStVarParams( bTextAsZero, VarResult );

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
                OUString aTabName;
                ScComplexRefData aRef;
                PopExternalDoubleRef( nFileId, aTabName, aRef);
                ScRange aAbs = aRef.toAbs(mrDoc, aPos);
                nVal += static_cast<sal_uLong>(aAbs.aEnd.Tab() - aAbs.aStart.Tab() + 1) *
                    static_cast<sal_uLong>(aAbs.aEnd.Col() - aAbs.aStart.Col() + 1);
            }
            break;
            default:
                PopError();
                SetError(FormulaError::IllegalParameter);
        }
    }
    PushDouble(static_cast<double>(nVal));
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
                OUString aTabName;
                ScComplexRefData aRef;
                PopExternalDoubleRef( nFileId, aTabName, aRef);
                ScRange aAbs = aRef.toAbs(mrDoc, aPos);
                nVal += static_cast<sal_uLong>(aAbs.aEnd.Tab() - aAbs.aStart.Tab() + 1) *
                    static_cast<sal_uLong>(aAbs.aEnd.Row() - aAbs.aStart.Row() + 1);
            }
            break;
            default:
                PopError();
                SetError(FormulaError::IllegalParameter);
        }
    }
    PushDouble(static_cast<double>(nVal));
}

void ScInterpreter::ScSheets()
{
    sal_uInt8 nParamCount = GetByte();
    sal_uLong nVal;
    if ( nParamCount == 0 )
        nVal = mrDoc.GetTableCount();
    else
    {
        nVal = 0;
        SCCOL nCol1;
        SCROW nRow1;
        SCTAB nTab1;
        SCCOL nCol2;
        SCROW nRow2;
        SCTAB nTab2;
        while (nGlobalError == FormulaError::NONE && nParamCount-- > 0)
        {
            switch ( GetStackType() )
            {
                case svSingleRef:
                case svExternalSingleRef:
                    PopError();
                    nVal++;
                break;
                case svDoubleRef:
                    PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                    nVal += static_cast<sal_uLong>(nTab2 - nTab1 + 1);
                break;
                case svExternalDoubleRef:
                {
                    sal_uInt16 nFileId;
                    OUString aTabName;
                    ScComplexRefData aRef;
                    PopExternalDoubleRef( nFileId, aTabName, aRef);
                    ScRange aAbs = aRef.toAbs(mrDoc, aPos);
                    nVal += static_cast<sal_uLong>(aAbs.aEnd.Tab() - aAbs.aStart.Tab() + 1);
                }
                break;
                default:
                    PopError();
                    SetError( FormulaError::IllegalParameter );
            }
        }
    }
    PushDouble( static_cast<double>(nVal) );
}

void ScInterpreter::ScColumn()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 0, 1 ) )
        return;

    double nVal = 0.0;
    if (nParamCount == 0)
    {
        nVal = aPos.Col() + 1;
        if (bMatrixFormula)
        {
            SCCOL nCols = 0;
            SCROW nRows = 0;
            if (pMyFormulaCell)
                pMyFormulaCell->GetMatColsRows( nCols, nRows);
            bool bMayBeScalar;
            if (nCols == 0)
            {
                // Happens if called via ScViewFunc::EnterMatrix()
                // ScFormulaCell::GetResultDimensions() as of course a
                // matrix result is not available yet.
                nCols = 1;
                bMayBeScalar = false;
            }
            else
            {
                bMayBeScalar = true;
            }
            if (!bMayBeScalar || nCols != 1 || nRows != 1)
            {
                ScMatrixRef pResMat = GetNewMat( static_cast<SCSIZE>(nCols), 1, /*bEmpty*/true );
                if (pResMat)
                {
                    for (SCCOL i=0; i < nCols; ++i)
                        pResMat->PutDouble( nVal + i, static_cast<SCSIZE>(i), 0);
                    PushMatrix( pResMat);
                    return;
                }
            }
        }
    }
    else
    {
        switch ( GetStackType() )
        {
            case svSingleRef :
            {
                SCCOL nCol1(0);
                SCROW nRow1(0);
                SCTAB nTab1(0);
                PopSingleRef( nCol1, nRow1, nTab1 );
                nVal = static_cast<double>(nCol1 + 1);
            }
            break;
            case svExternalSingleRef :
            {
                sal_uInt16 nFileId;
                OUString aTabName;
                ScSingleRefData aRef;
                PopExternalSingleRef( nFileId, aTabName, aRef );
                ScAddress aAbsRef = aRef.toAbs(mrDoc, aPos);
                nVal = static_cast<double>( aAbsRef.Col() + 1 );
            }
            break;

            case svDoubleRef :
            case svExternalDoubleRef :
            {
                SCCOL nCol1;
                SCCOL nCol2;
                if ( GetStackType() == svDoubleRef )
                {
                    SCROW nRow1;
                    SCTAB nTab1;
                    SCROW nRow2;
                    SCTAB nTab2;
                    PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                }
                else
                {
                    sal_uInt16 nFileId;
                    OUString aTabName;
                    ScComplexRefData aRef;
                    PopExternalDoubleRef( nFileId, aTabName, aRef );
                    ScRange aAbs = aRef.toAbs(mrDoc, aPos);
                    nCol1 = aAbs.aStart.Col();
                    nCol2 = aAbs.aEnd.Col();
                }
                if (nCol2 > nCol1)
                {
                    ScMatrixRef pResMat = GetNewMat(
                            static_cast<SCSIZE>(nCol2-nCol1+1), 1, /*bEmpty*/true);
                    if (pResMat)
                    {
                        for (SCCOL i = nCol1; i <= nCol2; i++)
                            pResMat->PutDouble(static_cast<double>(i+1),
                                    static_cast<SCSIZE>(i-nCol1), 0);
                        PushMatrix(pResMat);
                        return;
                    }
                }
                else
                    nVal = static_cast<double>(nCol1 + 1);
            }
            break;
            default:
                SetError( FormulaError::IllegalParameter );
        }
    }
    PushDouble( nVal );
}

void ScInterpreter::ScRow()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 0, 1 ) )
        return;

    double nVal = 0.0;
    if (nParamCount == 0)
    {
        nVal = aPos.Row() + 1;
        if (bMatrixFormula)
        {
            SCCOL nCols = 0;
            SCROW nRows = 0;
            if (pMyFormulaCell)
                pMyFormulaCell->GetMatColsRows( nCols, nRows);
            bool bMayBeScalar;
            if (nRows == 0)
            {
                // Happens if called via ScViewFunc::EnterMatrix()
                // ScFormulaCell::GetResultDimensions() as of course a
                // matrix result is not available yet.
                nRows = 1;
                bMayBeScalar = false;
            }
            else
            {
                bMayBeScalar = true;
            }
            if (!bMayBeScalar || nCols != 1 || nRows != 1)
            {
                ScMatrixRef pResMat = GetNewMat( 1, static_cast<SCSIZE>(nRows), /*bEmpty*/true);
                if (pResMat)
                {
                    for (SCROW i=0; i < nRows; i++)
                        pResMat->PutDouble( nVal + i, 0, static_cast<SCSIZE>(i));
                    PushMatrix( pResMat);
                    return;
                }
            }
        }
    }
    else
    {
        switch ( GetStackType() )
        {
            case svSingleRef :
            {
                SCCOL nCol1(0);
                SCROW nRow1(0);
                SCTAB nTab1(0);
                PopSingleRef( nCol1, nRow1, nTab1 );
                nVal = static_cast<double>(nRow1 + 1);
            }
            break;
            case svExternalSingleRef :
            {
                sal_uInt16 nFileId;
                OUString aTabName;
                ScSingleRefData aRef;
                PopExternalSingleRef( nFileId, aTabName, aRef );
                ScAddress aAbsRef = aRef.toAbs(mrDoc, aPos);
                nVal = static_cast<double>( aAbsRef.Row() + 1 );
            }
            break;
            case svDoubleRef :
            case svExternalDoubleRef :
            {
                SCROW nRow1;
                SCROW nRow2;
                if ( GetStackType() == svDoubleRef )
                {
                    SCCOL nCol1;
                    SCTAB nTab1;
                    SCCOL nCol2;
                    SCTAB nTab2;
                    PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                }
                else
                {
                    sal_uInt16 nFileId;
                    OUString aTabName;
                    ScComplexRefData aRef;
                    PopExternalDoubleRef( nFileId, aTabName, aRef );
                    ScRange aAbs = aRef.toAbs(mrDoc, aPos);
                    nRow1 = aAbs.aStart.Row();
                    nRow2 = aAbs.aEnd.Row();
                }
                if (nRow2 > nRow1)
                {
                    ScMatrixRef pResMat = GetNewMat( 1,
                            static_cast<SCSIZE>(nRow2-nRow1+1), /*bEmpty*/true);
                    if (pResMat)
                    {
                        for (SCROW i = nRow1; i <= nRow2; i++)
                            pResMat->PutDouble(static_cast<double>(i+1), 0,
                                    static_cast<SCSIZE>(i-nRow1));
                        PushMatrix(pResMat);
                        return;
                    }
                }
                else
                    nVal = static_cast<double>(nRow1 + 1);
            }
            break;
            default:
                SetError( FormulaError::IllegalParameter );
        }
    }
    PushDouble( nVal );
}

void ScInterpreter::ScSheet()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 0, 1 ) )
        return;

    SCTAB nVal = 0;
    if ( nParamCount == 0 )
        nVal = aPos.Tab() + 1;
    else
    {
        switch ( GetStackType() )
        {
            case svString :
            {
                svl::SharedString aStr = PopString();
                if ( mrDoc.GetTable(aStr.getString(), nVal))
                    ++nVal;
                else
                    SetError( FormulaError::IllegalArgument );
            }
            break;
            case svSingleRef :
            {
                SCCOL nCol1(0);
                SCROW nRow1(0);
                SCTAB nTab1(0);
                PopSingleRef(nCol1, nRow1, nTab1);
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
                SetError( FormulaError::IllegalParameter );
        }
        if ( nGlobalError != FormulaError::NONE )
            nVal = 0;
    }
    PushDouble( static_cast<double>(nVal) );
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

    bool IsStringOrEmpty(SCSIZE i) const
    {
        return mbColVec ? mrMat.IsStringOrEmpty(0, i) : mrMat.IsStringOrEmpty(i, 0);
    }

    double GetDouble(SCSIZE i) const
    {
        return mbColVec ? mrMat.GetDouble(0, i) : mrMat.GetDouble(i, 0);
    }

    OUString GetString(SCSIZE i) const
    {
        return mbColVec ? mrMat.GetString(0, i).getString() : mrMat.GetString(i, 0).getString();
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
sal_Int32 lcl_CompareMatrix2Query(
    SCSIZE i, const VectorMatrixAccessor& rMat, const ScQueryParam& rParam, const ScQueryEntry& rEntry, bool bMatchWholeCell )
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
        const double nVal1 = rMat.GetDouble(i);
        if (!std::isfinite(nVal1))
        {
            // XXX Querying for error values is not required, otherwise we'd
            // need to check here.
            return 1;   // error always greater than numeric or string
        }

        if (bByString)
            return -1;  // numeric always less than string

        const double nVal2 = rEntry.GetQueryItem().mfVal;
        // XXX Querying for error values is not required, otherwise we'd need
        // to check here and move that check before the bByString check.
        if (nVal1 == nVal2)
            return 0;

        return nVal1 < nVal2 ? -1 : 1;
    }

    if (!bByString)
        return 1;       // string always greater than numeric

    OUString aStr1 = rMat.GetString(i);
    OUString aStr2 = rEntry.GetQueryItem().maString.getString();

    // bRealWildOrRegExp
    if (rParam.eSearchType != utl::SearchParam::SearchType::Normal &&
        ((rEntry.eOp == SC_EQUAL) || (rEntry.eOp == SC_NOT_EQUAL)))
    {
        sal_Int32 nStart = 0;
        sal_Int32 nEnd = aStr1.getLength();

        bool bMatch = rEntry.GetSearchTextPtr(rParam.eSearchType, rParam.bCaseSens, bMatchWholeCell)
            ->SearchForward(aStr1, &nStart, &nEnd);
        // from 614 on, nEnd is behind the found text
        if (bMatch && bMatchWholeCell
            && (nStart != 0 || nEnd != aStr1.getLength()))
            bMatch = false;    // RegExp must match entire cell string

        bool bOk = ((rEntry.eOp == SC_NOT_EQUAL) ? !bMatch : bMatch);

        if (bOk)
            return 0; // we have a WildOrRegExp match
    }

    CollatorWrapper& rCollator = ScGlobal::GetCollator(rParam.bCaseSens);
    return rCollator.compareString(aStr1, aStr2);
}

/** returns -1 when matrix(i) value is smaller than matrix(j) value, 0 when
    they are equal, and 1 when larger */
sal_Int32 lcl_Compare2MatrixCells( SCSIZE i, const VectorMatrixAccessor& rMat, SCSIZE j )
{
    // empty always less than anything else
    if (rMat.IsEmpty(i))
        return ( rMat.IsEmpty(j) ? 0 : -1 );
    else if (rMat.IsEmpty(j))
        return 1;

    bool bByString = rMat.IsStringOrEmpty(j); // string, empty has already been handled
    if (rMat.IsValue(i))
    {
        const double nVal1 = rMat.GetDouble(i);
        if (!std::isfinite(nVal1))
            return 1;   // error always greater than numeric or string

        if (bByString)
            return -1;  // numeric always less than string

        const double nVal2 = rMat.GetDouble(j);
        if (nVal1 == nVal2)
            return 0;

        return ( nVal1 < nVal2 ? -1 : 1 );
    }

    if (!bByString)
        return 1;       // string always greater than numeric

    return ScGlobal::GetCollator().compareString(rMat.GetString(i), rMat.GetString(j)); // case-insensitive
}

/** returns the last item with the identical value as the original item
    value. */
void lcl_GetLastMatch( SCSIZE& rIndex, const VectorMatrixAccessor& rMat,
        SCSIZE nMatCount)
{
    if (rMat.IsValue(rIndex))
    {
        double nVal = rMat.GetDouble(rIndex);
        while (rIndex < nMatCount-1 && rMat.IsValue(rIndex+1) &&
                nVal == rMat.GetDouble(rIndex+1))
            ++rIndex;
    }
    // Order of IsEmptyPath, IsEmpty, IsStringOrEmpty is significant!
    else if (rMat.IsEmptyPath(rIndex))
    {
        while (rIndex < nMatCount-1 && rMat.IsEmptyPath(rIndex+1))
            ++rIndex;
    }
    else if (rMat.IsEmpty(rIndex))
    {
        while (rIndex < nMatCount-1 && rMat.IsEmpty(rIndex+1))
            ++rIndex;
    }
    else if (rMat.IsStringOrEmpty(rIndex))
    {
        OUString aStr( rMat.GetString(rIndex));
        while (rIndex < nMatCount-1 && rMat.IsStringOrEmpty(rIndex+1) &&
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
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return;

    VectorSearchArguments vsa;
    vsa.nSearchOpCode = SC_OPCODE_MATCH;

    // get match mode
    double fType = ( nParamCount == 3 ? GetDouble() : 1.0 );
    switch ( static_cast<int>(fType) )
    {
        case -1 :
            vsa.eMatchMode  = exactorG;
            vsa.eSearchMode = searchbdesc;
            break;
        case 0 :
            vsa.eMatchMode  = exactorNA;
            vsa.eSearchMode = searchfwd;
            break;
        case 1 :
            // default value
            vsa.eMatchMode  = exactorS;
            vsa.eSearchMode = searchbasc;
            break;
        default :
            PushIllegalParameter();
            return;
    }

    // get vector to be searched
    switch (GetStackType())
    {
        case svSingleRef:
            PopSingleRef( vsa.nCol1, vsa.nRow1, vsa.nTab1);
            vsa.nCol2   = vsa.nCol1;
            vsa.nRow2   = vsa.nRow1;
            vsa.pMatSrc = nullptr;
        break;
        case svDoubleRef:
        {
            vsa.pMatSrc = nullptr;
            SCTAB nTab2 = 0;
            PopDoubleRef(vsa.nCol1, vsa.nRow1, vsa.nTab1, vsa.nCol2, vsa.nRow2, nTab2);
            if (vsa.nTab1 != nTab2 || (vsa.nCol1 != vsa.nCol2 && vsa.nRow1 != vsa.nRow2))
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
                vsa.pMatSrc = PopMatrix();
            else
                PopExternalDoubleRef(vsa.pMatSrc);

            if (!vsa.pMatSrc)
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

    // get search value
    if (nGlobalError == FormulaError::NONE)
    {
        switch ( GetStackType() )
        {
            case svDouble:
            {
                vsa.isStringSearch = false;
                vsa.fSearchVal = GetDouble();
            }
            break;
            case svString:
            {
                vsa.isStringSearch = true;
                vsa.sSearchStr = GetString();
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
                ScRefCellValue aCell(mrDoc, aAdr);
                if (aCell.hasNumeric())
                {
                    vsa.isStringSearch = false;
                    vsa.fSearchVal = GetCellValue(aAdr, aCell);
                }
                else
                {
                    vsa.isStringSearch = true;
                    GetCellString(vsa.sSearchStr, aCell);
                }
            }
            break;
            case svExternalSingleRef:
            {
                ScExternalRefCache::TokenRef pToken;
                PopExternalSingleRef(pToken);
                if (nGlobalError != FormulaError::NONE)
                {
                    PushError( nGlobalError);
                    return;
                }
                if (pToken->GetType() == svDouble)
                {
                    vsa.isStringSearch = false;
                    vsa.fSearchVal = pToken->GetDouble();
                }
                else
                {
                    vsa.isStringSearch = true;
                    vsa.sSearchStr = pToken->GetString();
                }
            }
            break;
            case svExternalDoubleRef:
            case svMatrix :
            {
                ScMatValType nType = GetDoubleOrStringFromMatrix(
                        vsa.fSearchVal, vsa.sSearchStr);
                vsa.isStringSearch = ScMatrix::IsNonValueType(nType);
            }
            break;
            default:
            {
                PushIllegalParameter();
                return;
            }
        }

        // execute search
        if ( SearchVectorForValue( vsa ) )
            PushDouble( vsa.nIndex );
        else
        {
            if ( vsa.isResultNA )
                PushNA();
            else
                return; // error occurred and has already been pushed
        }
    }
    else
        PushIllegalParameter();
}

void ScInterpreter::ScXMatch()
{
    sal_uInt8 nParamCount = GetByte();
    if (!MustHaveParamCount(nParamCount, 2, 4))
        return;

    VectorSearchArguments vsa;
    vsa.nSearchOpCode = SC_OPCODE_X_MATCH;

    // get search mode
    if (nParamCount == 4)
    {
        sal_Int16 k = GetInt16();
        if (k >= -2 && k <= 2 && k != 0)
            vsa.eSearchMode = static_cast<SearchMode>(k);
        else
        {
            PushIllegalParameter();
            return;
        }
    }
    else
        vsa.eSearchMode = searchfwd;

    // get match mode
    if (nParamCount >= 3)
    {
        sal_Int16 k = GetInt16();
        if (k >= -1 && k <= 2)
            vsa.eMatchMode = static_cast<MatchMode>(k);
        else
        {
            PushIllegalParameter();
            return;
        }
    }
    else
        vsa.eMatchMode = exactorNA;

    // get vector to be searched
    switch (GetStackType())
    {
        case svSingleRef:
        {
            PopSingleRef(vsa.nCol1, vsa.nRow1, vsa.nTab1);
            vsa.nCol2 = vsa.nCol1;
            vsa.nRow2 = vsa.nRow1;
            vsa.pMatSrc = nullptr;
        }
        break;
        case svDoubleRef:
        {
            vsa.pMatSrc = nullptr;
            SCTAB nTab2 = 0;
            PopDoubleRef(vsa.nCol1, vsa.nRow1, vsa.nTab1, vsa.nCol2, vsa.nRow2, nTab2);
            if (vsa.nTab1 != nTab2 || (vsa.nCol1 != vsa.nCol2 && vsa.nRow1 != vsa.nRow2))
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
                vsa.pMatSrc = PopMatrix();
            else
                PopExternalDoubleRef(vsa.pMatSrc);

            if (!vsa.pMatSrc)
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

    // get search value
    if (nGlobalError == FormulaError::NONE)
    {
        switch (GetRawStackType())
        {
            case svMissing:
            case svEmptyCell:
            {
                vsa.isEmptySearch = true;
                vsa.isStringSearch = false;
                vsa.sSearchStr = GetString();
            }
            break;
            case svDouble:
            {
                vsa.isStringSearch = false;
                vsa.fSearchVal = GetDouble();
            }
            break;
            case svString:
            {
                vsa.isStringSearch = true;
                vsa.sSearchStr = GetString();
            }
            break;
            case svDoubleRef:
            case svSingleRef:
            {
                ScAddress aAdr;
                if (!PopDoubleRefOrSingleRef(aAdr))
                {
                    PushInt(0);
                    return;
                }
                ScRefCellValue aCell(mrDoc, aAdr);
                if (aCell.hasNumeric())
                {
                    vsa.isStringSearch = false;
                    vsa.fSearchVal = GetCellValue(aAdr, aCell);
                }
                else
                {
                    vsa.isStringSearch = true;
                    GetCellString(vsa.sSearchStr, aCell);
                }
            }
            break;
            case svExternalSingleRef:
            {
                ScExternalRefCache::TokenRef pToken;
                PopExternalSingleRef(pToken);
                if (nGlobalError != FormulaError::NONE)
                {
                    PushError(nGlobalError);
                    return;
                }
                if (pToken->GetType() == svDouble)
                {
                    vsa.isStringSearch = false;
                    vsa.fSearchVal = pToken->GetDouble();
                }
                else
                {
                    vsa.isStringSearch = true;
                    vsa.sSearchStr = pToken->GetString();
                }
            }
            break;
            case svExternalDoubleRef:
            case svMatrix:
            {
                ScMatValType nType = GetDoubleOrStringFromMatrix(
                    vsa.fSearchVal, vsa.sSearchStr);
                vsa.isStringSearch = ScMatrix::IsNonValueType(nType);
            }
            break;
            default:
            {
                PushIllegalParameter();
                return;
            }
        }

        // execute search
        if (SearchVectorForValue(vsa))
            PushDouble(vsa.nIndex);
        else
        {
            if (vsa.isResultNA)
                PushNA();
            else
                return; // error occurred and has already been pushed
        }
    }
    else
        PushIllegalParameter();
}

namespace {

bool isCellContentEmpty( const ScRefCellValue& rCell )
{
    switch (rCell.getType())
    {
        case CELLTYPE_VALUE:
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            return false;
        case CELLTYPE_FORMULA:
        {
            // NOTE: Excel treats ="" in a referenced cell as blank in
            // COUNTBLANK() but not in ISBLANK(), which is inconsistent.
            // COUNTBLANK() tests the (display) result whereas ISBLANK() tests
            // the cell content.
            // ODFF allows both for COUNTBLANK().
            // OOo and LibreOffice prior to 4.4 did not treat ="" as blank in
            // COUNTBLANK(), we now do for Excel interoperability.
            /* TODO: introduce yet another compatibility option? */
            sc::FormulaResultValue aRes = rCell.getFormula()->GetResult();
            if (aRes.meType != sc::FormulaResultValue::String)
                return false;
            if (!aRes.maString.isEmpty())
                return false;
        }
        break;
        default:
            ;
    }

    return true;
}

}

void ScInterpreter::ScCountEmptyCells()
{
    if ( !MustHaveParamCount( GetByte(), 1 ) )
        return;

    const SCSIZE nMatRows = GetRefListArrayMaxSize(1);
    // There's either one RefList and nothing else, or none.
    ScMatrixRef xResMat = (nMatRows ? GetNewMat( 1, nMatRows, /*bEmpty*/true ) : nullptr);
    sal_uLong nMaxCount = 0, nCount = 0;
    switch (GetStackType())
    {
        case svSingleRef :
        {
            nMaxCount = 1;
            ScAddress aAdr;
            PopSingleRef( aAdr );
            ScRefCellValue aCell(mrDoc, aAdr);
            if (!isCellContentEmpty(aCell))
                nCount = 1;
        }
        break;
        case svRefList :
        case svDoubleRef :
        {
            ScRange aRange;
            short nParam = 1;
            SCSIZE nRefListArrayPos = 0;
            size_t nRefInList = 0;
            while (nParam-- > 0)
            {
                nRefListArrayPos = nRefInList;
                PopDoubleRef( aRange, nParam, nRefInList);
                nMaxCount +=
                    static_cast<sal_uLong>(aRange.aEnd.Row() - aRange.aStart.Row() + 1) *
                    static_cast<sal_uLong>(aRange.aEnd.Col() - aRange.aStart.Col() + 1) *
                    static_cast<sal_uLong>(aRange.aEnd.Tab() - aRange.aStart.Tab() + 1);

                ScCellIterator aIter( mrDoc, aRange, mnSubTotalFlags);
                for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
                {
                    const ScRefCellValue& rCell = aIter.getRefCellValue();
                    if (!isCellContentEmpty(rCell))
                        ++nCount;
                }
                if (xResMat)
                {
                    xResMat->PutDouble( nMaxCount - nCount, 0, nRefListArrayPos);
                    nMaxCount = nCount = 0;
                }
            }
        }
        break;
        case svMatrix:
        case svExternalSingleRef:
        case svExternalDoubleRef:
        {
            ScMatrixRef xMat = GetMatrix();
            if (!xMat)
                SetError( FormulaError::IllegalParameter);
            else
            {
                SCSIZE nC, nR;
                xMat->GetDimensions( nC, nR);
                nMaxCount = nC * nR;
                // Numbers (implicit), strings and error values, ignore empty
                // strings as those if not entered in an inline array are the
                // result of a formula, to be par with a reference to formula
                // cell as *visual* blank, see isCellContentEmpty() above.
                nCount = xMat->Count( true, true, true);
            }
        }
        break;
        default : SetError(FormulaError::IllegalParameter); break;
    }
    if (xResMat)
        PushMatrix( xResMat);
    else
        PushDouble(nMaxCount - nCount);
}

void ScInterpreter::IterateParametersIf( ScIterFuncIf eFunc )
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return;

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
                        PushError( FormulaError::IllegalParameter);
                        return;
                    }
                }
                break;
            case svSingleRef :
                PopSingleRef( nCol3, nRow3, nTab3 );
                break;
            case svMatrix:
                pSumExtraMatrix = PopMatrix();
                // nCol3, nRow3, nTab3 remain 0
                break;
            case svExternalSingleRef:
                {
                    pSumExtraMatrix = GetNewMat(1,1);
                    ScExternalRefCache::TokenRef pToken;
                    PopExternalSingleRef(pToken);
                    if (nGlobalError != FormulaError::NONE)
                    {
                        PushError( nGlobalError);
                        return;
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
                PushError( FormulaError::IllegalParameter);
                return;
        }
    }

    svl::SharedString aString;
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
                    PushError( nGlobalError);
                    return;
                }

                ScRefCellValue aCell(mrDoc, aAdr);
                switch (aCell.getType())
                {
                    case CELLTYPE_VALUE :
                        fVal = GetCellValue(aAdr, aCell);
                        bIsString = false;
                        break;
                    case CELLTYPE_FORMULA :
                        if (aCell.getFormula()->IsValue())
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
                bIsString = ScMatrix::IsRealStringType( nType);
            }
            break;
        case svExternalSingleRef:
            {
                ScExternalRefCache::TokenRef pToken;
                PopExternalSingleRef(pToken);
                if (nGlobalError == FormulaError::NONE)
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

    KahanSum fSum = 0.0;
    double fRes = 0.0;
    double fCount = 0.0;
    short nParam = 1;
    const SCSIZE nMatRows = GetRefListArrayMaxSize( nParam);
    // There's either one RefList and nothing else, or none.
    ScMatrixRef xResMat = (nMatRows ? GetNewMat( 1, nMatRows, /*bEmpty*/true ) : nullptr);
    SCSIZE nRefListArrayPos = 0;
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
                    /* TODO: this could resolve if all refs are of the same size */
                    SetError( FormulaError::IllegalParameter);
                }
                else
                {
                    nRefListArrayPos = nRefInList;
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
                        PushError( FormulaError::IllegalParameter);
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
                SetError( FormulaError::IllegalParameter);
        }
        if ( nTab1 != nTab2 )
        {
            SetError( FormulaError::IllegalParameter);
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
                nMaxCol = mrDoc.MaxCol();
                nMaxRow = mrDoc.MaxRow();
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

        if (nGlobalError == FormulaError::NONE)
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
                rParam.FillInExcelSyntax(mrDoc.GetSharedStringPool(), aString.getString(), 0, &mrContext);
                if (rItem.meType == ScQueryEntry::ByString)
                    rParam.eSearchType = DetectSearchType(rItem.maString.getString(), mrDoc);
            }
            ScAddress aAdr;
            aAdr.SetTab( nTab3 );
            rParam.nCol1  = nCol1;
            rParam.nCol2  = nCol2;
            rEntry.nField = nCol1;
            SCCOL nColDiff = nCol3 - nCol1;
            SCROW nRowDiff = nRow3 - nRow1;
            if (pQueryMatrix)
            {
                // Never case-sensitive.
                sc::CompareOptions aOptions( mrDoc, rEntry, rParam.eSearchType);
                ScMatrixRef pResultMatrix = QueryMat( pQueryMatrix, aOptions);
                if (nGlobalError != FormulaError::NONE || !pResultMatrix)
                {
                    SetError( FormulaError::IllegalParameter);
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
                                ScRefCellValue aCell(mrDoc, aAdr);
                                if (aCell.hasNumeric())
                                {
                                    fVal = GetCellValue(aAdr, aCell);
                                    ++fCount;
                                    fSum += fVal;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                ScQueryCellIteratorDirect aCellIter(mrDoc, mrContext, nTab1, rParam, false, false);
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
                            ScRefCellValue aCell(mrDoc, aAdr);
                            if (aCell.hasNumeric())
                            {
                                fVal = GetCellValue(aAdr, aCell);
                                ++fCount;
                                fSum += fVal;
                            }
                        } while ( aCellIter.GetNext() );
                    }
                }
            }
        }
        else
        {
            PushError( FormulaError::IllegalParameter);
            return;
        }

        switch( eFunc )
        {
            case ifSUMIF:     fRes = fSum.get(); break;
            case ifAVERAGEIF: fRes = div( fSum.get(), fCount ); break;
        }
        if (xResMat)
        {
            if (nGlobalError == FormulaError::NONE)
                xResMat->PutDouble( fRes, 0, nRefListArrayPos);
            else
            {
                xResMat->PutError( nGlobalError, 0, nRefListArrayPos);
                nGlobalError = FormulaError::NONE;
            }
            fRes = fCount = 0.0;
            fSum = 0;
        }
    }
    if (xResMat)
        PushMatrix( xResMat);
    else
        PushDouble( fRes);
}

void ScInterpreter::ScSumIf()
{
    IterateParametersIf( ifSUMIF);
}

void ScInterpreter::ScAverageIf()
{
    IterateParametersIf( ifAVERAGEIF);
}

void ScInterpreter::ScCountIf()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    svl::SharedString aString;
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
            ScRefCellValue aCell(mrDoc, aAdr);
            switch (aCell.getType())
            {
                case CELLTYPE_VALUE :
                    fVal = GetCellValue(aAdr, aCell);
                    bIsString = false;
                    break;
                case CELLTYPE_FORMULA :
                    if (aCell.getFormula()->IsValue())
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
            ScMatValType nType = GetDoubleOrStringFromMatrix(fVal, aString);
            bIsString = ScMatrix::IsRealStringType( nType);
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
    const SCSIZE nMatRows = GetRefListArrayMaxSize( nParam);
    // There's either one RefList and nothing else, or none.
    ScMatrixRef xResMat = (nMatRows ? GetNewMat( 1, nMatRows, /*bEmpty*/true ) : nullptr);
    SCSIZE nRefListArrayPos = 0;
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
        const ScComplexRefData* refData = nullptr;
        switch ( GetStackType() )
        {
            case svRefList :
                nRefListArrayPos = nRefInList;
                [[fallthrough]];
            case svDoubleRef :
                {
                    refData = GetStackDoubleRef(nRefInList);
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
                PopError(); // Propagate it further
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
        if (nGlobalError == FormulaError::NONE)
        {
            ScQueryParam rParam;
            rParam.nRow1       = nRow1;
            rParam.nRow2       = nRow2;
            rParam.nTab        = nTab1;

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
                rParam.FillInExcelSyntax(mrDoc.GetSharedStringPool(), aString.getString(), 0, &mrContext);
                if (rItem.meType == ScQueryEntry::ByString)
                    rParam.eSearchType = DetectSearchType(rItem.maString.getString(), mrDoc);
            }
            rParam.nCol1  = nCol1;
            rParam.nCol2  = nCol2;
            rEntry.nField = nCol1;
            if (pQueryMatrix)
            {
                // Never case-sensitive.
                sc::CompareOptions aOptions( mrDoc, rEntry, rParam.eSearchType);
                ScMatrixRef pResultMatrix = QueryMat( pQueryMatrix, aOptions);
                if (nGlobalError != FormulaError::NONE || !pResultMatrix)
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
                if(ScCountIfCellIteratorSortedCache::CanBeUsed(mrDoc, rParam, nTab1, pMyFormulaCell,
                        refData, mrContext))
                {
                    ScCountIfCellIteratorSortedCache aCellIter(mrDoc, mrContext, nTab1, rParam, false, false);
                    fCount += aCellIter.GetCount();
                }
                else
                {
                    ScCountIfCellIteratorDirect aCellIter(mrDoc, mrContext, nTab1, rParam, false, false);
                    fCount += aCellIter.GetCount();
                }
            }
        }
        else
        {
            PushIllegalParameter();
            return;
        }
        if (xResMat)
        {
            xResMat->PutDouble( fCount, 0, nRefListArrayPos);
            fCount = 0.0;
        }
    }
    if (xResMat)
        PushMatrix( xResMat);
    else
        PushDouble(fCount);
}

void ScInterpreter::IterateParametersIfs( double(*ResultFunc)( const sc::ParamIfsResult& rRes ) )
{
    sal_uInt8 nParamCount = GetByte();
    sal_uInt8 nQueryCount = nParamCount / 2;

    std::vector<sal_uInt8>& vConditions = mrContext.maConditions;
    // vConditions is cached, although it is clear'ed after every cell is interpreted,
    // if the SUMIFS/COUNTIFS are part of a matrix formula, then that is not enough because
    // with a single InterpretTail() call it results in evaluation of all the cells in the
    // matrix formula.
    vConditions.clear();

    // Range-reduce optimization
    SCCOL nStartColDiff = 0;
    SCCOL nEndColDiff = 0;
    SCROW nStartRowDiff = 0;
    SCROW nEndRowDiff = 0;
    bool bRangeReduce = false;
    ScRange aMainRange;

    bool bHasDoubleRefCriteriaRanges = true;
    // Do not attempt main-range reduce if any of the criteria-ranges are not double-refs.
    // For COUNTIFS queries it's possible to range-reduce too, if the query is not supposed
    // to match empty cells (will be checked and undone later if needed), so simply treat
    // the first criteria range as the main range for purposes of detecting if this can be done.
    for (sal_uInt16 nParamIdx = 2; nParamIdx < nParamCount; nParamIdx += 2 )
    {
        const formula::FormulaToken* pCriteriaRangeToken = pStack[ sp-nParamIdx ];
        if (pCriteriaRangeToken->GetType() != svDoubleRef )
        {
            bHasDoubleRefCriteriaRanges = false;
            break;
        }
    }

    // Probe the main range token, and try if we can shrink the range without altering results.
    const formula::FormulaToken* pMainRangeToken = pStack[ sp-nParamCount ];
    if (pMainRangeToken->GetType() == svDoubleRef && bHasDoubleRefCriteriaRanges)
    {
        const ScComplexRefData* pRefData = pMainRangeToken->GetDoubleRef();
        if (!pRefData->IsDeleted())
        {
            DoubleRefToRange( *pRefData, aMainRange);
            if (aMainRange.aStart.Tab() == aMainRange.aEnd.Tab())
            {
                // Shrink the range to actual data content.
                ScRange aSubRange = aMainRange;
                mrDoc.GetDataAreaSubrange(aSubRange);
                nStartColDiff = aSubRange.aStart.Col() - aMainRange.aStart.Col();
                nStartRowDiff = aSubRange.aStart.Row() - aMainRange.aStart.Row();
                nEndColDiff = aSubRange.aEnd.Col() - aMainRange.aEnd.Col();
                nEndRowDiff = aSubRange.aEnd.Row() - aMainRange.aEnd.Row();
                bRangeReduce = nStartColDiff || nStartRowDiff || nEndColDiff || nEndRowDiff;
            }
        }
    }

    double fVal = 0.0;
    SCCOL nDimensionCols = 0;
    SCROW nDimensionRows = 0;
    const SCSIZE nRefArrayRows = GetRefListArrayMaxSize( nParamCount);
    std::vector<std::vector<sal_uInt8>> vRefArrayConditions;

    while (nParamCount > 1 && nGlobalError == FormulaError::NONE)
    {
        // take criteria
        svl::SharedString aString;
        fVal = 0.0;
        bool bIsString = true;
        switch ( GetStackType() )
        {
            case svDoubleRef :
            case svSingleRef :
                {
                    ScAddress aAdr;
                    if ( !PopDoubleRefOrSingleRef( aAdr ) )
                    {
                        PushError( nGlobalError);
                        return;
                    }

                    ScRefCellValue aCell(mrDoc, aAdr);
                    switch (aCell.getType())
                    {
                        case CELLTYPE_VALUE :
                            fVal = GetCellValue(aAdr, aCell);
                            bIsString = false;
                            break;
                        case CELLTYPE_FORMULA :
                            if (aCell.getFormula()->IsValue())
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
                    bIsString = ScMatrix::IsRealStringType( nType);
                }
                break;
            case svExternalSingleRef:
                {
                    ScExternalRefCache::TokenRef pToken;
                    PopExternalSingleRef(pToken);
                    if (nGlobalError == FormulaError::NONE)
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

        if (nGlobalError != FormulaError::NONE)
        {
            PushError( nGlobalError);
            return;   // and bail out, no need to evaluate other arguments
        }

        // take range
        short nParam = nParamCount;
        size_t nRefInList = 0;
        size_t nRefArrayPos = std::numeric_limits<size_t>::max();
        SCCOL nCol1 = 0;
        SCROW nRow1 = 0;
        SCTAB nTab1 = 0;
        SCCOL nCol2 = 0;
        SCROW nRow2 = 0;
        SCTAB nTab2 = 0;
        ScMatrixRef pQueryMatrix;
        while (nParam-- == nParamCount)
        {
            const ScComplexRefData* refData = nullptr;
            switch ( GetStackType() )
            {
                case svRefList :
                    {
                        const ScRefListToken* p = dynamic_cast<const ScRefListToken*>(pStack[sp-1]);
                        if (p && p->IsArrayResult())
                        {
                            if (nRefInList == 0)
                            {
                                if (vRefArrayConditions.empty())
                                    vRefArrayConditions.resize( nRefArrayRows);
                                if (!vConditions.empty())
                                {
                                    // Similar to other reference list array
                                    // handling, add/op the current value to
                                    // all array positions.
                                    for (auto & rVec : vRefArrayConditions)
                                    {
                                        if (rVec.empty())
                                            rVec = vConditions;
                                        else
                                        {
                                            assert(rVec.size() == vConditions.size());  // see dimensions below
                                            for (size_t i=0, n = rVec.size(); i < n; ++i)
                                            {
                                                rVec[i] += vConditions[i];
                                            }
                                        }
                                    }
                                    // Reset condition results.
                                    std::for_each( vConditions.begin(), vConditions.end(),
                                            [](sal_uInt8 & r){ r = 0.0; } );
                                }
                            }
                            nRefArrayPos = nRefInList;
                        }
                        refData = GetStackDoubleRef(nRefInList);
                        ScRange aRange;
                        PopDoubleRef( aRange, nParam, nRefInList);
                        aRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                    }
                break;
                case svDoubleRef :
                    refData = GetStackDoubleRef();
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
                            PushError( FormulaError::IllegalParameter);
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
                    PushError( FormulaError::IllegalParameter);
                    return;
            }
            if ( nTab1 != nTab2 )
            {
                PushError( FormulaError::IllegalArgument);
                return;
            }

            ScQueryParam rParam;
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
                rParam.FillInExcelSyntax(mrDoc.GetSharedStringPool(), aString.getString(), 0, &mrContext);
                if (rItem.meType == ScQueryEntry::ByString)
                    rParam.eSearchType = DetectSearchType(rItem.maString.getString(), mrDoc);
            }

            // Undo bRangeReduce if asked to match empty cells for COUNTIFS (which should be rare).
            assert(rEntry.GetQueryItems().size() == 1);
            const bool isCountIfs = (nParamCount % 2) == 0;
            if(isCountIfs && (rEntry.IsQueryByEmpty() || rItem.mbMatchEmpty) && bRangeReduce)
            {
                bRangeReduce = false;
                // All criteria ranges are svDoubleRef's, so only vConditions needs adjusting.
                assert(vRefArrayConditions.empty());
                if(!vConditions.empty())
                {
                    std::vector<sal_uInt8> newConditions;
                    SCCOL newDimensionCols = nCol2 - nCol1 + 1;
                    SCROW newDimensionRows = nRow2 - nRow1 + 1;
                    newConditions.reserve( newDimensionCols * newDimensionRows );
                    SCCOL col = nCol1;
                    for(; col < nCol1 + nStartColDiff; ++col)
                        newConditions.insert( newConditions.end(), newDimensionRows, 0 );
                    for(; col <= nCol2 - nStartColDiff; ++col)
                    {
                        newConditions.insert( newConditions.end(), nStartRowDiff, 0 );
                        SCCOL oldCol = col - ( nCol1 + nStartColDiff );
                        size_t nIndex = oldCol * nDimensionRows;
                        if (nIndex < vConditions.size())
                        {
                            auto it = vConditions.begin() + nIndex;
                            newConditions.insert( newConditions.end(), it, it + nDimensionRows );
                        }
                        else
                            newConditions.insert( newConditions.end(), nDimensionRows, 0 );
                        newConditions.insert( newConditions.end(), -nEndRowDiff, 0 );
                    }
                    for(; col <= nCol2; ++col)
                        newConditions.insert( newConditions.end(), newDimensionRows, 0 );
                    assert( newConditions.size() == o3tl::make_unsigned( newDimensionCols * newDimensionRows ));
                    vConditions = std::move( newConditions );
                    nDimensionCols = newDimensionCols;
                    nDimensionRows = newDimensionRows;
                }
            }

            if (bRangeReduce)
            {
                // All reference ranges must be of the same size as the main range.
                if( aMainRange.aEnd.Col() - aMainRange.aStart.Col() != nCol2 - nCol1
                    || aMainRange.aEnd.Row() - aMainRange.aStart.Row() != nRow2 - nRow1)
                {
                    PushError ( FormulaError::IllegalArgument);
                    return;
                }
                nCol1 += nStartColDiff;
                nRow1 += nStartRowDiff;

                nCol2 += nEndColDiff;
                nRow2 += nEndRowDiff;
            }

            // All reference ranges must be of same dimension and size.
            if (!nDimensionCols)
                nDimensionCols = nCol2 - nCol1 + 1;
            if (!nDimensionRows)
                nDimensionRows = nRow2 - nRow1 + 1;
            if ((nDimensionCols != (nCol2 - nCol1 + 1)) || (nDimensionRows != (nRow2 - nRow1 + 1)))
            {
                PushError ( FormulaError::IllegalArgument);
                return;
            }

            // recalculate matrix values
            if (nGlobalError != FormulaError::NONE)
            {
                PushError( nGlobalError);
                return;
            }

            // initialize temporary result matrix
            if (vConditions.empty())
                vConditions.resize( nDimensionCols * nDimensionRows, 0);

            rParam.nRow1  = nRow1;
            rParam.nRow2  = nRow2;
            rParam.nCol1  = nCol1;
            rParam.nCol2  = nCol2;
            rEntry.nField = nCol1;
            SCCOL nColDiff = -nCol1;
            SCROW nRowDiff = -nRow1;
            if (pQueryMatrix)
            {
                // Never case-sensitive.
                sc::CompareOptions aOptions(mrDoc, rEntry, rParam.eSearchType);
                ScMatrixRef pResultMatrix = QueryMat( pQueryMatrix, aOptions);
                if (nGlobalError != FormulaError::NONE || !pResultMatrix)
                {
                    PushError( FormulaError::IllegalParameter);
                    return;
                }

                // result matrix is filled with boolean values.
                std::vector<double> aResValues;
                pResultMatrix->GetDoubleArray(aResValues);
                if (vConditions.size() != aResValues.size())
                {
                    PushError( FormulaError::IllegalParameter);
                    return;
                }

                std::vector<double>::const_iterator itThisRes = aResValues.begin();
                for (auto& rCondition : vConditions)
                {
                    rCondition += *itThisRes;
                    ++itThisRes;
                }
            }
            else
            {
                if( ScQueryCellIteratorSortedCache::CanBeUsed( mrDoc, rParam, nTab1, pMyFormulaCell,
                        refData, mrContext ))
                {
                    ScQueryCellIteratorSortedCache aCellIter(mrDoc, mrContext, nTab1, rParam, false, false);
                    // Increment Entry.nField in iterator when switching to next column.
                    aCellIter.SetAdvanceQueryParamEntryField( true );
                    if ( aCellIter.GetFirst() )
                    {
                        do
                        {
                            size_t nC = aCellIter.GetCol() + nColDiff;
                            size_t nR = aCellIter.GetRow() + nRowDiff;
                            ++vConditions[nC * nDimensionRows + nR];
                        } while ( aCellIter.GetNext() );
                    }
                }
                else
                {
                    ScQueryCellIteratorDirect aCellIter(mrDoc, mrContext, nTab1, rParam, false, false);
                    // Increment Entry.nField in iterator when switching to next column.
                    aCellIter.SetAdvanceQueryParamEntryField( true );
                    if ( aCellIter.GetFirst() )
                    {
                        do
                        {
                            size_t nC = aCellIter.GetCol() + nColDiff;
                            size_t nR = aCellIter.GetRow() + nRowDiff;
                            ++vConditions[nC * nDimensionRows + nR];
                        } while ( aCellIter.GetNext() );
                    }
                }
            }
            if (nRefArrayPos != std::numeric_limits<size_t>::max())
            {
                // Apply condition result to reference list array result position.
                std::vector<sal_uInt8>& rVec = vRefArrayConditions[nRefArrayPos];
                if (rVec.empty())
                    rVec = vConditions;
                else
                {
                    assert(rVec.size() == vConditions.size());  // see dimensions above
                    for (size_t i=0, n = rVec.size(); i < n; ++i)
                    {
                        rVec[i] += vConditions[i];
                    }
                }
                // Reset conditions vector.
                // When leaving an svRefList this has to be emptied not set to
                // 0.0 because it's checked when entering an svRefList.
                if (nRefInList == 0)
                    std::vector<sal_uInt8>().swap( vConditions);
                else
                    std::for_each( vConditions.begin(), vConditions.end(), [](sal_uInt8 & r){ r = 0; } );
            }
        }
        nParamCount -= 2;
    }

    if (!vRefArrayConditions.empty() && !vConditions.empty())
    {
        // Add/op the last current value to all array positions.
        for (auto & rVec : vRefArrayConditions)
        {
            if (rVec.empty())
                rVec = vConditions;
            else
            {
                assert(rVec.size() == vConditions.size());  // see dimensions above
                for (size_t i=0, n = rVec.size(); i < n; ++i)
                {
                    rVec[i] += vConditions[i];
                }
            }
        }
    }

    if (nGlobalError != FormulaError::NONE)
    {
        PushError( nGlobalError);
        return;   // bail out
    }

    sc::ParamIfsResult aRes;
    ScMatrixRef xResMat;

    // main range - only for AVERAGEIFS, SUMIFS, MINIFS and MAXIFS
    if (nParamCount == 1)
    {
        short nParam = nParamCount;
        size_t nRefInList = 0;
        size_t nRefArrayPos = std::numeric_limits<size_t>::max();
        bool bRefArrayMain = false;
        while (nParam-- == nParamCount)
        {
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
                        const ScRefListToken* p = dynamic_cast<const ScRefListToken*>(pStack[sp-1]);
                        if (p && p->IsArrayResult())
                        {
                            if (vRefArrayConditions.empty())
                            {
                                // Replicate conditions if there wasn't a
                                // reference list array for criteria
                                // evaluation.
                                vRefArrayConditions.resize( nRefArrayRows);
                                for (auto & rVec : vRefArrayConditions)
                                {
                                    rVec = vConditions;
                                }
                            }

                            bRefArrayMain = true;
                            nRefArrayPos = nRefInList;
                        }
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
                        pMainMatrix = GetMatrix();
                        if (!pMainMatrix)
                        {
                            PushError( FormulaError::IllegalParameter);
                            return;
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
                // Treat a scalar value as 1x1 matrix.
                case svDouble:
                    pMainMatrix = GetNewMat(1,1);
                    nMainCol1 = nMainCol2 = 0;
                    nMainRow1 = nMainRow2 = 0;
                    nMainTab1 = nMainTab2 = 0;
                    pMainMatrix->PutDouble( GetDouble(), 0, 0);
                break;
                case svString:
                    pMainMatrix = GetNewMat(1,1);
                    nMainCol1 = nMainCol2 = 0;
                    nMainRow1 = nMainRow2 = 0;
                    nMainTab1 = nMainTab2 = 0;
                    pMainMatrix->PutString( GetString(), 0, 0);
                break;
                default:
                    PopError();
                    PushError( FormulaError::IllegalParameter);
                    return;
            }
            if ( nMainTab1 != nMainTab2 )
            {
                PushError( FormulaError::IllegalArgument);
                return;
            }

            if (bRangeReduce)
            {
                nMainCol1 += nStartColDiff;
                nMainRow1 += nStartRowDiff;

                nMainCol2 += nEndColDiff;
                nMainRow2 += nEndRowDiff;
            }

            // All reference ranges must be of same dimension and size.
            if ((nDimensionCols != (nMainCol2 - nMainCol1 + 1)) || (nDimensionRows != (nMainRow2 - nMainRow1 + 1)))
            {
                PushError ( FormulaError::IllegalArgument);
                return;
            }

            if (nGlobalError != FormulaError::NONE)
            {
                PushError( nGlobalError);
                return;   // bail out
            }

            // end-result calculation

            // This gets weird... if conditions were calculated using a
            // reference list array but the main calculation range is not a
            // reference list array, then the conditions of the array are
            // applied to the main range each in turn to form the array result.

            size_t nRefArrayMainPos = (bRefArrayMain ? nRefArrayPos :
                    (vRefArrayConditions.empty() ? std::numeric_limits<size_t>::max() : 0));
            const bool bAppliedArray = (!bRefArrayMain && nRefArrayMainPos == 0);

            if (nRefArrayMainPos == 0)
                xResMat = GetNewMat( 1, nRefArrayRows, /*bEmpty*/true );

            if (pMainMatrix)
            {
                std::vector<double> aMainValues;
                pMainMatrix->GetDoubleArray(aMainValues, false); // Map empty values to NaN's.

                do
                {
                    if (nRefArrayMainPos < vRefArrayConditions.size())
                        vConditions = vRefArrayConditions[nRefArrayMainPos];

                    if (vConditions.size() != aMainValues.size())
                    {
                        PushError( FormulaError::IllegalArgument);
                        return;
                    }

                    std::vector<sal_uInt8>::const_iterator itRes = vConditions.begin(), itResEnd = vConditions.end();
                    std::vector<double>::const_iterator itMain = aMainValues.begin();
                    for (; itRes != itResEnd; ++itRes, ++itMain)
                    {
                        if (*itRes != nQueryCount)
                            continue;

                        fVal = *itMain;
                        if (GetDoubleErrorValue(fVal) == FormulaError::ElementNaN)
                            continue;

                        ++aRes.mfCount;
                        aRes.mfSum += fVal;
                        if ( aRes.mfMin > fVal )
                            aRes.mfMin = fVal;
                        if ( aRes.mfMax < fVal )
                            aRes.mfMax = fVal;
                    }
                    if (nRefArrayMainPos != std::numeric_limits<size_t>::max())
                    {
                        xResMat->PutDouble( ResultFunc( aRes), 0, nRefArrayMainPos);
                        aRes = sc::ParamIfsResult();
                    }
                }
                while (bAppliedArray && ++nRefArrayMainPos < nRefArrayRows);
            }
            else
            {
                ScAddress aAdr;
                aAdr.SetTab( nMainTab1 );
                do
                {
                    if (nRefArrayMainPos < vRefArrayConditions.size())
                        vConditions = vRefArrayConditions[nRefArrayMainPos];

                    SAL_WARN_IF(nDimensionCols && nDimensionRows && vConditions.empty(), "sc",  "ScInterpreter::IterateParametersIfs vConditions is empty");
                    if (!vConditions.empty())
                    {
                        std::vector<sal_uInt8>::const_iterator itRes = vConditions.begin();
                        for (SCCOL nCol = 0; nCol < nDimensionCols; ++nCol)
                        {
                            for (SCROW nRow = 0; nRow < nDimensionRows; ++nRow, ++itRes)
                            {
                                if (*itRes == nQueryCount)
                                {
                                    aAdr.SetCol( nCol + nMainCol1);
                                    aAdr.SetRow( nRow + nMainRow1);
                                    ScRefCellValue aCell(mrDoc, aAdr);
                                    if (aCell.hasNumeric())
                                    {
                                        fVal = GetCellValue(aAdr, aCell);
                                        ++aRes.mfCount;
                                        aRes.mfSum += fVal;
                                        if ( aRes.mfMin > fVal )
                                            aRes.mfMin = fVal;
                                        if ( aRes.mfMax < fVal )
                                            aRes.mfMax = fVal;
                                    }
                                }
                            }
                        }
                    }
                    if (nRefArrayMainPos != std::numeric_limits<size_t>::max())
                    {
                        xResMat->PutDouble( ResultFunc( aRes), 0, nRefArrayMainPos);
                        aRes = sc::ParamIfsResult();
                    }
                }
                while (bAppliedArray && ++nRefArrayMainPos < nRefArrayRows);
            }
        }
    }
    else
    {
        // COUNTIFS only.
        if (vRefArrayConditions.empty())
        {
            // The code below is this but optimized for most elements not matching.
            // for (auto const & rCond : vConditions)
            //     if (rCond == nQueryCount)
            //         ++aRes.mfCount;
            static_assert(sizeof(vConditions[0]) == 1);
            const sal_uInt8* pos = vConditions.data();
            const sal_uInt8* end = pos + vConditions.size();
            for(;;)
            {
                pos = static_cast< const sal_uInt8* >( memchr( pos, nQueryCount, end - pos ));
                if( pos == nullptr )
                    break;
                ++aRes.mfCount;
                ++pos;
            }
        }
        else
        {
            xResMat = GetNewMat( 1, nRefArrayRows, /*bEmpty*/true );
            for (size_t i=0, n = vRefArrayConditions.size(); i < n; ++i)
            {
                double fCount = 0.0;
                for (auto const & rCond : vRefArrayConditions[i])
                {
                    if (rCond == nQueryCount)
                        ++fCount;
                }
                xResMat->PutDouble( fCount, 0, i);
            }
        }
    }

    if (xResMat)
        PushMatrix( xResMat);
    else
        PushDouble( ResultFunc( aRes));
}

void ScInterpreter::ScSumIfs()
{
    // ScMutationGuard aShouldFail(pDok, ScMutationGuardFlags::CORE);
    sal_uInt8 nParamCount = GetByte();

    if (nParamCount < 3 || (nParamCount % 2 != 1))
    {
        PushError( FormulaError::ParameterExpected);
        return;
    }

    auto ResultFunc = []( const sc::ParamIfsResult& rRes )
    {
        return rRes.mfSum.get();
    };
    IterateParametersIfs(ResultFunc);
}

void ScInterpreter::ScAverageIfs()
{
    sal_uInt8 nParamCount = GetByte();

    if (nParamCount < 3 || (nParamCount % 2 != 1))
    {
        PushError( FormulaError::ParameterExpected);
        return;
    }

    auto ResultFunc = []( const sc::ParamIfsResult& rRes )
    {
        return sc::div( rRes.mfSum.get(), rRes.mfCount);
    };
    IterateParametersIfs(ResultFunc);
}

void ScInterpreter::ScCountIfs()
{
    sal_uInt8 nParamCount = GetByte();

    if (nParamCount < 2 || (nParamCount % 2 != 0))
    {
        PushError( FormulaError::ParameterExpected);
        return;
    }

    auto ResultFunc = []( const sc::ParamIfsResult& rRes )
    {
        return rRes.mfCount;
    };
    IterateParametersIfs(ResultFunc);
}

void ScInterpreter::ScMinIfs_MS()
{
    sal_uInt8 nParamCount = GetByte();

    if (nParamCount < 3 || (nParamCount % 2 != 1))
    {
        PushError( FormulaError::ParameterExpected);
        return;
    }

    auto ResultFunc = []( const sc::ParamIfsResult& rRes )
    {
        return (rRes.mfMin < std::numeric_limits<double>::max()) ? rRes.mfMin : 0.0;
    };
    IterateParametersIfs(ResultFunc);
}


void ScInterpreter::ScMaxIfs_MS()
{
    sal_uInt8 nParamCount = GetByte();

    if (nParamCount < 3 || (nParamCount % 2 != 1))
    {
        PushError( FormulaError::ParameterExpected);
        return;
    }

    auto ResultFunc = []( const sc::ParamIfsResult& rRes )
    {
        return (rRes.mfMax > std::numeric_limits<double>::lowest()) ? rRes.mfMax : 0.0;
    };
    IterateParametersIfs(ResultFunc);
}

void ScInterpreter::ScLookup()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return ;

    ScMatrixRef pDataMat = nullptr, pResMat = nullptr;
    SCCOL nCol1 = 0, nCol2 = 0, nResCol1 = 0, nResCol2 = 0;
    SCROW nRow1 = 0, nRow2 = 0, nResRow1 = 0, nResRow2 = 0;
    SCTAB nTab1 = 0, nResTab = 0;
    SCSIZE nLenMajor = 0;   // length of major direction
    bool bVertical = true;  // whether to lookup vertically or horizontally

    // The third parameter, result array, double, string and reference.
    double fResVal = 0.0;
    svl::SharedString aResStr;
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
            case svSingleRef:
                PopSingleRef( nResCol1, nResRow1, nResTab);
                nResCol2 = nResCol1;
                nResRow2 = nResRow1;
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
            default:
                PushIllegalParameter();
                return;
        }
    }

    // For double, string and single reference.
    double fDataVal = 0.0;
    svl::SharedString aDataStr;
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
            ScRefCellValue aCell(mrDoc, aDataAdr);
            if (aCell.hasEmptyValue())
            {
                // Empty cells aren't found anywhere, bail out early.
                SetError( FormulaError::NotAvailable);
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
            SetError( FormulaError::IllegalParameter);
    }

    if (nGlobalError != FormulaError::NONE)
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
                bFound = (ScGlobal::GetCollator().compareString(aDataStr.getString(), rItem.maString.getString()) <= 0);
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
                PushString(pResMat->GetString(0, 0));
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
                case svSingleRef:
                    PushCellResultToken( true, ScAddress( nResCol1, nResRow1, nResTab), nullptr, nullptr);
                    break;
                default:
                    assert(!"ScInterpreter::ScLookup: unhandled eResArrayType, single value data");
                    PushIllegalParameter();
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
                    PushCellResultToken( true, aDataAdr, nullptr, nullptr);
                    break;
                default:
                    assert(!"ScInterpreter::ScLookup: unhandled eDataArrayType, single value data");
                    PushIllegalParameter();
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

        // Do not propagate errors from matrix while copying to vector.
        pDataMat->SetErrorInterpreter( nullptr);

        // Excel has an undocumented behaviour in that it seems to internally
        // sort an interim array (i.e. error values specifically #DIV/0! are
        // sorted to the end) or ignore error values that makes these "get last
        // non-empty" searches work, e.g. =LOOKUP(2,1/NOT(ISBLANK(A:A)),A:A)
        // see tdf#117016
        // Instead of sorting a million entries of which mostly only a bunch of
        // rows are filled and moving error values to the end which most are
        // already anyway, assume the matrix to be sorted except error values
        // and omit the coded DoubleError values.
        // Do this only for a numeric matrix (that includes errors coded as
        // doubles), which covers the case in question.
        /* TODO: it's unclear whether this really matches Excel behaviour in
         * all constellations or if there are cases that include unsorted error
         * values and thus yield arbitrary binary search results or something
         * different or whether there are cases where error values are also
         * omitted from mixed numeric/string arrays or if it's not an interim
         * matrix but a cell range reference instead. */
        const bool bOmitErrorValues = (eDataArrayType == svMatrix && pDataMat->IsNumeric());

        // In case of non-vector matrix, only search the first row or column.
        ScMatrixRef pDataMat2;
        std::vector<SCCOLROW> vIndex;
        if (bOmitErrorValues)
        {
            std::vector<double> vArray;
            VectorMatrixAccessor aMatAcc(*pDataMat, bVertical);
            const SCSIZE nElements = aMatAcc.GetElementCount();
            for (SCSIZE i=0; i < nElements; ++i)
            {
                const double fVal = aMatAcc.GetDouble(i);
                if (std::isfinite(fVal))
                {
                    vArray.push_back(fVal);
                    vIndex.push_back(i);
                }
            }
            if (vArray.empty())
            {
                PushNA();
                return;
            }
            const size_t nElems = vArray.size();
            if (nElems == nElements)
            {
                // No error value omitted, use as is.
                pDataMat2 = pDataMat;
                std::vector<SCCOLROW>().swap( vIndex);
            }
            else
            {
                nLenMajor = nElems;
                if (bVertical)
                {
                    ScMatrixRef pTempMat = GetNewMat( 1, nElems, /*bEmpty*/true );
                    pTempMat->PutDoubleVector( vArray, 0, 0);
                    pDataMat2 = pTempMat;
                }
                else
                {
                    ScMatrixRef pTempMat = GetNewMat( nElems, 1, /*bEmpty*/true );
                    for (size_t i=0; i < nElems; ++i)
                        pTempMat->PutDouble( vArray[i], i, 0);
                    pDataMat2 = pTempMat;
                }
            }
        }
        else
        {
            // Just use as is with the VectorMatrixAccessor.
            pDataMat2 = pDataMat;
        }

        // Do not propagate errors from matrix while searching.
        pDataMat2->SetErrorInterpreter( nullptr);

        VectorMatrixAccessor aMatAcc2(*pDataMat2, bVertical);

        // binary search for non-equality mode (the source data is
        // assumed to be sorted in ascending order).

        SCCOLROW nDelta = -1;
        bool bMatchWholeCell = mrDoc.GetDocOptions().IsMatchWholeCell();

        SCSIZE nFirst = 0, nLast = nLenMajor-1; //, nHitIndex = 0;
        for (SCSIZE nLen = nLast-nFirst; nLen > 0; nLen = nLast-nFirst)
        {
            SCSIZE nMid = nFirst + nLen/2;
            sal_Int32 nCmp = lcl_CompareMatrix2Query( nMid, aMatAcc2, aParam, rEntry, bMatchWholeCell );
            if (nCmp == 0)
            {
                // exact match.  find the last item with the same value.
                lcl_GetLastMatch( nMid, aMatAcc2, nLenMajor);
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
            sal_Int32 nCmp = lcl_CompareMatrix2Query(nDelta+1, aMatAcc2, aParam, rEntry, bMatchWholeCell );
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
            if (!vIndex.empty())
                nDelta = vIndex[nDelta];

            VectorMatrixAccessor aMatAcc(*pDataMat, bVertical);
            SCCOLROW i = nDelta;
            SCSIZE n = aMatAcc.GetElementCount();
            if (o3tl::make_unsigned(i) >= n)
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
            // Result array is matrix.
            // Note this does not replicate the other dimension.
            if (o3tl::make_unsigned(nDelta) >= aResMatAcc.GetElementCount())
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
            /* TODO: the entire switch is a copy of the cell range search
             * result, factor out. */
            switch (eResArrayType)
            {
                case svDoubleRef:
                case svSingleRef:
                {
                    // Use the result array vector.  Note that the result array is assumed
                    // to be a vector (i.e. 1-dimensional array).

                    ScAddress aAdr;
                    aAdr.SetTab(nResTab);
                    bool bResVertical = (nResRow2 - nResRow1) > 0;
                    if (bResVertical)
                    {
                        SCROW nTempRow = static_cast<SCROW>(nResRow1 + nDelta);
                        if (nTempRow > mrDoc.MaxRow())
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
                        if (nTempCol > mrDoc.MaxCol())
                        {
                            PushDouble(0);
                            return;
                        }
                        aAdr.SetCol(nTempCol);
                        aAdr.SetRow(nResRow1);
                    }
                    PushCellResultToken( true, aAdr, nullptr, nullptr);
                }
                break;
                case svDouble:
                case svString:
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
                            default:
                                ;   // nothing
                        }
                    }
                }
                break;
                default:
                    assert(!"ScInterpreter::ScLookup: unhandled eResArrayType, array search");
                    PushIllegalParameter();
            }
        }
        else
        {
            // No result array. Use the data array to get the final value from.
            // Propagate errors from matrix again.
            pDataMat->SetErrorInterpreter( this);
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
        aParam.eSearchType = DetectSearchType(rItem.maString.getString(), mrDoc);

    ScQueryCellIteratorDirect aCellIter(mrDoc, mrContext, nTab1, aParam, false, false);
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
        // Note this does not replicate the other dimension.
        if (o3tl::make_unsigned(nDelta) >= aResMatAcc.GetElementCount())
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
        /* TODO: the entire switch is a copy of the array search result, factor
         * out. */
        switch (eResArrayType)
        {
            case svDoubleRef:
            case svSingleRef:
            {
                // Use the result array vector.  Note that the result array is assumed
                // to be a vector (i.e. 1-dimensional array).

                ScAddress aAdr;
                aAdr.SetTab(nResTab);
                bool bResVertical = (nResRow2 - nResRow1) > 0;
                if (bResVertical)
                {
                    SCROW nTempRow = static_cast<SCROW>(nResRow1 + nDelta);
                    if (nTempRow > mrDoc.MaxRow())
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
                    if (nTempCol > mrDoc.MaxCol())
                    {
                        PushDouble(0);
                        return;
                    }
                    aAdr.SetCol(nTempCol);
                    aAdr.SetRow(nResRow1);
                }
                PushCellResultToken( true, aAdr, nullptr, nullptr);
            }
            break;
            case svDouble:
            case svString:
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
                        default:
                            ;   // nothing
                    }
                }
            }
            break;
            default:
                assert(!"ScInterpreter::ScLookup: unhandled eResArrayType, range search");
                PushIllegalParameter();
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
            if (nTempRow > mrDoc.MaxRow())
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
            if (nTempCol > mrDoc.MaxCol())
            {
                PushDouble(0);
                return;
            }
            aAdr.SetCol(nTempCol);
            aAdr.SetRow(nRow2);
        }
        PushCellResultToken(true, aAdr, nullptr, nullptr);
    }
}

void ScInterpreter::ScHLookup()
{
    CalculateLookup(true);
}

void ScInterpreter::CalculateLookup(bool bHLookup)
{
    sal_uInt8 nParamCount = GetByte();
    if (!MustHaveParamCount(nParamCount, 3, 4))
        return;

    // Optional 4th argument to declare whether or not the range is sorted.
    bool bSorted = true;
    if (nParamCount == 4)
        bSorted = GetBool();

    // Index of column to search.
    double fIndex = ::rtl::math::approxFloor( GetDouble() ) - 1.0;

    ScMatrixRef pMat = nullptr;
    SCSIZE nC = 0, nR = 0;
    SCCOL nCol1 = 0;
    SCROW nRow1 = 0;
    SCTAB nTab1 = 0;
    SCCOL nCol2 = 0;
    SCROW nRow2 = 0;
    const ScComplexRefData* refData = nullptr;
    StackVar eType = GetStackType();
    if (eType == svDoubleRef)
    {
        refData = GetStackDoubleRef(0);
        SCTAB nTab2;
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

    if ( fIndex < 0.0 || (bHLookup ? (pMat ? (fIndex >= nR) : (fIndex+nRow1 > nRow2)) : (pMat ? (fIndex >= nC) : (fIndex+nCol1 > nCol2)) ) )
    {
        PushIllegalArgument();
        return;
    }

    SCROW nZIndex = static_cast<SCROW>(fIndex);
    SCCOL nSpIndex = static_cast<SCCOL>(fIndex);

    if (!pMat)
    {
        nZIndex += nRow1;                       // value row
        nSpIndex = sal::static_int_cast<SCCOL>( nSpIndex + nCol1 );     // value column
    }

    if (nGlobalError != FormulaError::NONE)
    {
        PushIllegalParameter();
        return;
    }

    ScQueryParam aParam;
    aParam.nCol1 = nCol1;
    aParam.nRow1 = nRow1;
    if ( bHLookup )
    {
        aParam.nCol2 = nCol2;
        aParam.nRow2 = nRow1;     // search only in the first row
        aParam.bByRow = false;
    }
    else
    {
        aParam.nCol2 = nCol1;     // search only in the first column
        aParam.nRow2 = nRow2;
        aParam.nTab  = nTab1;
    }

    ScQueryEntry& rEntry = aParam.GetEntry(0);
    rEntry.bDoQuery = true;
    if ( bSorted )
        rEntry.eOp = SC_LESS_EQUAL;
    if ( !FillEntry(rEntry) )
        return;

    ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
    svl::SharedString aParamStr;
    if (rItem.meType == ScQueryEntry::ByString)
    {
        aParam.eSearchType = DetectSearchType(rItem.maString.getString(), mrDoc);
        aParamStr = rItem.maString;
    }

    if (pMat)
    {
        SCSIZE nMatCount = bHLookup ? nC : nR;
        SCSIZE nDelta = SCSIZE_MAX;
        if (rItem.meType == ScQueryEntry::ByString)
        {
//!!!!!!!
//TODO: enable regex on matrix strings
//!!!!!!!
            if ( bSorted )
            {
                CollatorWrapper& rCollator = ScGlobal::GetCollator();
                for (SCSIZE i = 0; i < nMatCount; i++)
                {
                    if (bHLookup ? pMat->IsStringOrEmpty(i, 0) : pMat->IsStringOrEmpty(0, i))
                    {
                        sal_Int32 nRes =
                            rCollator.compareString(
                                bHLookup ? pMat->GetString(i,0).getString() : pMat->GetString(0,i).getString(), aParamStr.getString());
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
                if (bHLookup)
                {
                    for (SCSIZE i = 0; i < nMatCount; i++)
                    {
                        if (pMat->IsStringOrEmpty(i, 0))
                        {
                            if (pMat->GetString(i,0).getDataIgnoreCase() == aParamStr.getDataIgnoreCase())
                            {
                                nDelta = i;
                                i = nMatCount + 1;
                            }
                        }
                    }
                }
                else
                {
                    nDelta = pMat->MatchStringInColumns(aParamStr, 0, 0);
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
                    if (!(bHLookup ? pMat->IsStringOrEmpty(i, 0) : pMat->IsStringOrEmpty(0, i)))
                    {
                        if ((bHLookup ? pMat->GetDouble(i,0) : pMat->GetDouble(0,i)) <= rItem.mfVal)
                            nDelta = i;
                        else
                            i = nMatCount+1;
                    }
                }
            }
            else
            {
                if (bHLookup)
                {
                    for (SCSIZE i = 0; i < nMatCount; i++)
                    {
                        if (! pMat->IsStringOrEmpty(i, 0) )
                        {
                            if ( pMat->GetDouble(i,0) == rItem.mfVal)
                            {
                                nDelta = i;
                                i = nMatCount + 1;
                            }
                        }
                    }
                }
                else
                {
                    nDelta = pMat->MatchDoubleInColumns(rItem.mfVal, 0, 0);
                }
            }
        }
        if ( nDelta != SCSIZE_MAX )
        {
            SCSIZE nX = static_cast<SCSIZE>(nSpIndex);
            SCSIZE nY = nDelta;
            SCSIZE nXs = 0;
            SCSIZE nYs = nY;
            if ( bHLookup )
            {
                nX = nDelta;
                nY = static_cast<SCSIZE>(nZIndex);
                nXs = nX;
                nYs = 0;
            }
            assert( nX < nC && nY < nR );
            if (!(rItem.meType == ScQueryEntry::ByString && pMat->IsValue( nXs, nYs)))
            {
                if (pMat->IsStringOrEmpty( nX, nY))
                    PushString(pMat->GetString( nX, nY).getString());
                else
                    PushDouble(pMat->GetDouble( nX, nY));
            }
            else
                PushNA();
            return;
        }
        else
            PushNA();
    }
    else
    {
        // not a matrix
        rEntry.nField = nCol1;
        bool bFound = false;
        SCCOL nCol = 0;
        SCROW nRow = 0;
        if ( bSorted )
            rEntry.eOp = SC_LESS_EQUAL;
        if ( bHLookup )
        {
            ScQueryCellIteratorDirect aCellIter(mrDoc, mrContext, nTab1, aParam, false, false);
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
        }
        else
        {
            ScAddress aResultPos( nCol1, nRow1, nTab1);
            bFound = LookupQueryWithCache( aResultPos, aParam, refData, 0, SC_OPCODE_V_LOOKUP );
            nRow = aResultPos.Row();
            nCol = nSpIndex;
        }

        if ( bFound )
        {
            ScAddress aAdr( nCol, nRow, nTab1 );
            PushCellResultToken( true, aAdr, nullptr, nullptr);
        }
        else
            PushNA();
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
            rItem.meType = ScQueryEntry::ByString;
            rItem.maString = GetString();
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
            ScRefCellValue aCell(mrDoc, aAdr);
            if (aCell.hasNumeric())
            {
                rItem.meType = ScQueryEntry::ByValue;
                rItem.mfVal = GetCellValue(aAdr, aCell);
            }
            else
            {
                GetCellString(rItem.maString, aCell);
                rItem.meType = ScQueryEntry::ByString;
            }
        }
        break;
        case svExternalDoubleRef:
        case svExternalSingleRef:
        case svMatrix:
        {
            svl::SharedString aStr;
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

void ScInterpreter::ScXLookup()
{
/* TODO
   -use VectorSearchArguments and SearchVectorForValue() with ScLookup, ScHLookup and ScVLookup
    as well to reduce redundant code, can de done later with lots of other MATCH/LOOKUP related code
    that can be unified
   -BinarySearch not supported for columns (horizontal search), now just use linear mode in this case
   -improve efficiency of code
*/
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 6 ) )
        return;

    VectorSearchArguments vsa;
    vsa.nSearchOpCode = SC_OPCODE_X_LOOKUP;

    if ( nParamCount == 6 )
    {
        sal_Int16 k = GetInt16();
        if ( k >= -2 && k <= 2 && k != 0 )
            vsa.eSearchMode = static_cast<SearchMode>(k);
        else
        {
            PushIllegalParameter();
            return;
        }
    }
    else
        vsa.eSearchMode = searchfwd;

    if ( nParamCount >= 5 )
    {
        sal_Int16 k = GetInt16();
        if ( k >= -1 && k <= 2 )
            vsa.eMatchMode = static_cast<MatchMode>(k);
        else
        {
            PushIllegalParameter();
            return;
        }
    }
    else
        vsa.eMatchMode = exactorNA;

    // Optional 4th argument to set return values if not found (default is #N/A)
    formula::FormulaConstTokenRef xNotFound;
    FormulaError nFirstMatchError = FormulaError::NONE;
    if ( nParamCount >= 4 && GetStackType() != svEmptyCell )
    {
        xNotFound = PopToken();
        nFirstMatchError = xNotFound->GetError();
        nGlobalError = FormulaError::NONE; // propagate only for match or active result path
    }

    // 3rd argument is return value array
    ScMatrixRef prMat = nullptr;
    SCCOL nSearchCol1 = 0;
    SCROW nSearchRow1 = 0;
    SCTAB nSearchTab1 = 0;
    SCCOL nSearchCol2 = 0;
    SCROW nSearchRow2 = 0;
    SCTAB nSearchTab2 = 0;
    SCSIZE nrC = 0, nrR = 0;

    switch ( GetStackType() )
    {
        case svSingleRef :
            PopSingleRef(nSearchCol1, nSearchRow1, nSearchTab1);
            nSearchCol2 = nSearchCol1;
            nSearchRow2 = nSearchRow1;
            nrC = nSearchCol2 - nSearchCol1 + 1;
            nrR = nSearchRow2 - nSearchRow1 + 1;
        break;
        case svDoubleRef:
        {
            PopDoubleRef(nSearchCol1, nSearchRow1, nSearchTab1, nSearchCol2, nSearchRow2, nSearchTab2);
            if (nSearchTab1 != nSearchTab2)
            {
                PushIllegalParameter();
                return;
            }
            nrC = nSearchCol2 - nSearchCol1 + 1;
            nrR = nSearchRow2 - nSearchRow1 + 1;
        }
        break;
        case svMatrix :
        case svExternalDoubleRef :
        {
            if (GetStackType() == svMatrix)
                prMat = PopMatrix();
            else
                PopExternalDoubleRef(prMat);

            if (!prMat)
            {
                PushIllegalParameter();
                return;
            }
            prMat->GetDimensions(nrC, nrR);
        }
        break;

        default :
            PushIllegalParameter();
            return;
    }

    // 2nd argument is vector to be searched
    SCSIZE nsC = 0, nsR = 0;
    switch ( GetStackType() )
    {
        case svSingleRef:
            vsa.pMatSrc = nullptr;
            PopSingleRef( vsa.nCol1, vsa.nRow1, vsa.nTab1);
            vsa.nCol2   = vsa.nCol1;
            vsa.nRow2   = vsa.nRow1;
            nsC = vsa.nCol2 - vsa.nCol1 + 1;
            nsR = vsa.nRow2 - vsa.nRow1 + 1;
        break;
        case svDoubleRef:
        {
            vsa.pMatSrc = nullptr;
            SCTAB nTab2 = 0;
            PopDoubleRef(vsa.nCol1, vsa.nRow1, vsa.nTab1, vsa.nCol2, vsa.nRow2, nTab2);
            if (vsa.nTab1 != nTab2 || (vsa.nCol1 != vsa.nCol2 && vsa.nRow1 != vsa.nRow2))
            {
                PushIllegalParameter();
                return;
            }
            nsC = vsa.nCol2 - vsa.nCol1 + 1;
            nsR = vsa.nRow2 - vsa.nRow1 + 1;
        }
        break;
        case svMatrix:
        case svExternalDoubleRef:
        {
            if (GetStackType() == svMatrix)
                vsa.pMatSrc = PopMatrix();
            else
                PopExternalDoubleRef(vsa.pMatSrc);

            if (!vsa.pMatSrc)
            {
                PushIllegalParameter();
                return;
            }
            vsa.pMatSrc->GetDimensions( nsC, nsR);
        }
        break;

        default:
            PushIllegalParameter();
            return;
    }
    if ( ( nsR >= nsC && nsR != nrR ) || ( nsR < nsC && nsC != nrC ) )
    {
        // search matrix must have same number of elements as result matrix in search direction
        PushIllegalParameter();
        return;
    }

    // 1st argument is search value
    if (nGlobalError == FormulaError::NONE)
    {
        switch ( GetRawStackType() )
        {
            case svMissing:
            case svEmptyCell:
            {
                vsa.isEmptySearch = true;
                vsa.isStringSearch = false;
                vsa.sSearchStr = GetString();
            }
            break;

            case svDouble:
            {
                vsa.isStringSearch = false;
                vsa.fSearchVal = GetDouble();
            }
            break;

            case svString:
            {
                vsa.isStringSearch = true;
                vsa.sSearchStr = GetString();
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
                ScRefCellValue aCell(mrDoc, aAdr);
                if (aCell.hasNumeric())
                {
                    vsa.isStringSearch = false;
                    vsa.fSearchVal = GetCellValue(aAdr, aCell);
                }
                else
                {
                    vsa.isStringSearch = true;
                    GetCellString(vsa.sSearchStr, aCell);
                }
            }
            break;

            case svExternalSingleRef:
            {
                ScExternalRefCache::TokenRef pToken;
                PopExternalSingleRef(pToken);
                if (nGlobalError != FormulaError::NONE)
                {
                    PushError( nGlobalError);
                    return;
                }
                if (pToken->GetType() == svDouble)
                {
                    vsa.isStringSearch = false;
                    vsa.fSearchVal = pToken->GetDouble();
                }
                else
                {
                    vsa.isStringSearch = true;
                    vsa.sSearchStr = pToken->GetString();
                }
            }
            break;

            case svExternalDoubleRef:
            case svMatrix :
            {
                ScMatValType nType = GetDoubleOrStringFromMatrix(
                        vsa.fSearchVal, vsa.sSearchStr);
                vsa.isStringSearch = ScMatrix::IsNonValueType(nType);
            }
            break;

            default:
            {
                PushIllegalParameter();
                return;
            }
        }
    }

    // start search
    if ( SearchVectorForValue( vsa ) )
    {
        //  found, output result
        assert( vsa.bVLookup ? ( o3tl::make_unsigned(vsa.nIndex) < nrR ) :
                               ( o3tl::make_unsigned(vsa.nIndex) < nrC ) );
        SCSIZE nX;
        SCSIZE nY;
        SCSIZE nResCols;
        SCSIZE nResRows;
        if ( vsa.bVLookup )
        {
            nX = static_cast<SCSIZE>(0);
            nY = vsa.nIndex;
            nResCols = nrC;
            nResRows = 1;
        }
        else
        {
            nX = vsa.nIndex;
            nY = static_cast<SCSIZE>(0);
            nResCols = 1;
            nResRows = nrR;
        }
        // if result has more than one row or column push double ref or matrix, else push single ref
        if ( nResCols > 1 || nResRows > 1 )
        {
            if (prMat)
            {
                // result is matrix, make / fill matrix with output and push that
                ScMatrixRef pResMat = GetNewMat(nResCols, nResRows, /*bEmpty*/true);
                if (pResMat)
                {
                    for (SCSIZE i = 0; i < nResCols; i++)
                    {
                        for (SCSIZE j = 0; j < nResRows; j++)
                        {
                            SCSIZE ri;
                            SCSIZE rj;
                            if (vsa.bVLookup)
                            {
                                ri = nX + i;
                                rj = nY;
                            }
                            else
                            {
                                ri = nX;
                                rj = nY + j;
                            }
                            if (prMat->IsStringOrEmpty(ri, rj))
                                pResMat->PutString(prMat->GetString(ri, rj), i, j);
                            else
                                pResMat->PutDouble(prMat->GetDouble(ri, rj), i, j);
                        }
                    }
                    PushMatrix(pResMat);
                }
                else
                {
                    PushIllegalParameter();
                    return;
                }
            }
            else
            {
                // result is a double ref
                PushDoubleRef(nSearchCol1 + nX, nSearchRow1 + nY, nSearchTab1,
                    nSearchCol1 + (nResCols - 1) + nX, nSearchRow1 + (nResRows - 1) + nY, nSearchTab1);
            }
        }
        else
        {
            // result is a single ref
            PushSingleRef(nSearchCol1 + nX, nSearchRow1 + nY, nSearchTab1);
        }
    }
    else
    {
        if ( vsa.isResultNA )
        {
            if ( xNotFound && ( xNotFound->GetType() != svMissing ) )
            {
                nGlobalError = nFirstMatchError;
                PushTokenRef(xNotFound);
            }
            else
                PushNA();
        }
    }
}

void ScInterpreter::ScFilter()
{
    sal_uInt8 nParamCount = GetByte();
    if (!MustHaveParamCount(nParamCount, 2, 3))
        return;

    // Optional 3th argument to set the value to return if all values
    // in the included array are empty (filter returns nothing)
    formula::FormulaConstTokenRef xNotFound;
    if (nParamCount == 3 && GetStackType() != svEmptyCell)
        xNotFound = PopToken();

    SCCOL nCondResultColEnd = 0;
    SCROW nCondResultRowEnd = 0;
    ScMatrixRef pCondResultMatrix = nullptr;
    std::vector<double> aResValues;
    size_t nMatch = 0;
    // take 2nd argument criteria bool array
    switch ( GetStackType() )
    {
        case svMatrix :
        case svExternalDoubleRef:
        case svExternalSingleRef:
        case svDoubleRef:
        case svSingleRef:
            {
                pCondResultMatrix = GetMatrix();
                if (!pCondResultMatrix)
                {
                    PushError(FormulaError::IllegalParameter);
                    return;
                }
                SCSIZE nC, nR;
                pCondResultMatrix->GetDimensions(nC, nR);
                nCondResultColEnd = static_cast<SCCOL>(nC - 1);
                nCondResultRowEnd = static_cast<SCROW>(nR - 1);

                // only 1 dimension of filtering allowed (also in excel)
                if (nCondResultColEnd > 0 && nCondResultRowEnd > 0)
                {
                    PushError(FormulaError::NoValue);
                    return;
                }

                // result matrix is filled with boolean values.
                pCondResultMatrix->GetDoubleArray(aResValues);

                FormulaError nError = FormulaError::NONE;
                auto matchNum = [&nMatch, &nError](double i) {
                    nError = GetDoubleErrorValue(i);
                    if (nError != FormulaError::NONE)
                    {
                        return true;
                    }
                    else
                    {
                        if (i > 0)
                            nMatch++;
                        return false;
                    }
                };

                if (auto it = std::find_if(aResValues.begin(), aResValues.end(), matchNum); it != aResValues.end())
                {
                    PushError(nError);
                    return;
                }
            }
            break;

        default:
            {
                PushIllegalParameter();
                return;
            }
    }

    // bail out, no need to evaluate other arguments
    if (nGlobalError != FormulaError::NONE)
    {
        PushError(nGlobalError);
        return;
    }

    SCCOL nQueryCol1 = 0;
    SCROW nQueryRow1 = 0;
    SCCOL nQueryCol2 = 0;
    SCROW nQueryRow2 = 0;
    ScMatrixRef pQueryMatrix = nullptr;
    // take 1st argument range
    switch ( GetStackType() )
    {
        case svSingleRef:
        case svDoubleRef:
        case svMatrix:
        case svExternalSingleRef:
        case svExternalDoubleRef:
            {
                pQueryMatrix = GetMatrix();
                if (!pQueryMatrix)
                {
                    PushError( FormulaError::IllegalParameter);
                    return;
                }
                SCSIZE nC, nR;
                pQueryMatrix->GetDimensions( nC, nR);
                nQueryCol2 = static_cast<SCCOL>(nC - 1);
                nQueryRow2 = static_cast<SCROW>(nR - 1);
            }
        break;
        default:
            PushError( FormulaError::IllegalParameter);
            return;
    }

    // bail out, no need to set a matrix if we have no result
    if (!nMatch)
    {
        if (xNotFound && (xNotFound->GetType() != svMissing))
            PushTokenRef(xNotFound);
        else
            PushError(FormulaError::NestedArray);
        return;
    }

    SCSIZE nResPos = 0;
    ScMatrixRef pResMat = nullptr;
    if (nQueryCol2 == nCondResultColEnd && nCondResultColEnd > 0)
    {
        pResMat = GetNewMat(nMatch, nQueryRow2 + 1 , /*bEmpty*/true);
        for (SCROW iR = nQueryRow1; iR <= nQueryRow2; iR++)
        {
            for (size_t iC = 0; iC < aResValues.size(); iC++)
            {
                if (aResValues[iC] > 0)
                {
                    if (pQueryMatrix->IsStringOrEmpty(iC, iR))
                        pResMat->PutStringTrans(pQueryMatrix->GetString(iC, iR), nResPos++);
                    else
                        pResMat->PutDoubleTrans(pQueryMatrix->GetDouble(iC, iR), nResPos++);
                }
            }
        }
    }
    else if (nQueryRow2 == nCondResultRowEnd && nCondResultRowEnd > 0)
    {
        pResMat = GetNewMat(nQueryCol2 + 1, nMatch, /*bEmpty*/true);
        for (SCCOL iC = nQueryCol1; iC <= nQueryCol2; iC++)
        {
            for (size_t iR = 0; iR < aResValues.size(); iR++)
            {
                if (aResValues[iR] > 0)
                {
                    if (pQueryMatrix->IsStringOrEmpty(iC, iR))
                        pResMat->PutString(pQueryMatrix->GetString(iC, iR), nResPos++);
                    else
                        pResMat->PutDouble(pQueryMatrix->GetDouble(iC, iR), nResPos++);
                }
            }
        }
    }
    else
    {
        PushError(FormulaError::IllegalParameter);
        return;
    }

    if (pResMat)
        PushMatrix(pResMat);
    else
        PushError(FormulaError::NestedArray);
}

void ScInterpreter::ScSort()
{
    sal_uInt8 nParamCount = GetByte();
    if (!MustHaveParamCount(nParamCount, 1, 4))
        return;

    // Create sort data
    ScSortParam aSortData;

    // 4th argument optional
    aSortData.bByRow = true; // default: By_Col = false --> bByRow = true
    if (nParamCount == 4)
        aSortData.bByRow = !GetBool();

    // 3rd argument optional
    std::vector<double> aSortOrderValues{ 1.0 }; // default: 1 = asc, -1 = desc
    if (nParamCount >= 3)
    {
        bool bMissing = IsMissing();
        ScMatrixRef pSortOrder = GetMatrix();
        if (!bMissing)
        {
            aSortOrderValues.clear();
            pSortOrder->GetDoubleArray(aSortOrderValues);
            for (const double& sortOrder : aSortOrderValues)
            {
                if (sortOrder != 1.0 && sortOrder != -1.0)
                {
                    PushIllegalParameter();
                    return;
                }
            }
        }
    }

    // 2nd argument optional
    std::vector<double> aSortIndexValues{ 0.0 }; // default: first column or row
    if (nParamCount >= 2)
    {
        bool bMissing = IsMissing();
        ScMatrixRef pSortIndex = GetMatrix();
        if (!bMissing)
        {
            aSortIndexValues.clear();
            pSortIndex->GetDoubleArray(aSortIndexValues);
            for (double& sortIndex : aSortIndexValues)
            {
                if (sortIndex < 1)
                {
                    PushIllegalParameter();
                    return;
                }
                else
                    sortIndex--;
            }
        }
    }

    if (aSortIndexValues.size() != aSortOrderValues.size() && aSortOrderValues.size() > 1)
    {
        PushIllegalParameter();
        return;
    }

    // 1st argument is vector to be sorted
    SCSIZE nsC = 0, nsR = 0;
    ScMatrixRef pMatSrc = nullptr;
    switch ( GetStackType() )
    {
        case svSingleRef:
            PopSingleRef(aSortData.nCol1, aSortData.nRow1, aSortData.nSourceTab);
            aSortData.nCol2   = aSortData.nCol1;
            aSortData.nRow2   = aSortData.nRow1;
            nsC = aSortData.nCol2 - aSortData.nCol1 + 1;
            nsR = aSortData.nRow2 - aSortData.nRow1 + 1;
        break;
        case svDoubleRef:
        {
            SCTAB nTab2 = 0;
            PopDoubleRef(aSortData.nCol1, aSortData.nRow1, aSortData.nSourceTab, aSortData.nCol2, aSortData.nRow2, nTab2);
            if (aSortData.nSourceTab != nTab2)
            {
                PushIllegalParameter();
                return;
            }
            nsC = aSortData.nCol2 - aSortData.nCol1 + 1;
            nsR = aSortData.nRow2 - aSortData.nRow1 + 1;
        }
        break;
        case svMatrix:
        case svExternalSingleRef:
        case svExternalDoubleRef:
        {
            pMatSrc = GetMatrix();
            if (!pMatSrc)
            {
                PushIllegalParameter();
                return;
            }
            pMatSrc->GetDimensions(nsC, nsR);
            aSortData.nCol2 = nsC - 1; // aSortData.nCol1 = 0
            aSortData.nRow2 = nsR - 1; // aSortData.nRow1 = 0
        }
        break;

        default:
            PushIllegalParameter();
            return;
    }

    aSortData.maKeyState.resize(aSortIndexValues.size());
    for (size_t i = 0; i < aSortIndexValues.size(); i++)
    {
        SCSIZE fIndex = static_cast<SCSIZE>(double_to_int32(aSortIndexValues[i]));
        if ((aSortData.bByRow && fIndex + 1 > nsC) || (!aSortData.bByRow && fIndex + 1 > nsR))
        {
            PushIllegalParameter();
            return;
        }

        aSortData.maKeyState[i].bDoSort = true;
        aSortData.maKeyState[i].nField = (aSortData.bByRow ? aSortData.nCol1 : aSortData.nRow1) + fIndex;

        if (aSortIndexValues.size() == aSortOrderValues.size())
            aSortData.maKeyState[i].bAscending = (aSortOrderValues[i] == 1.0);
        else
            aSortData.maKeyState[i].bAscending = (aSortOrderValues[0] == 1.0);
    }

    // sorting...
    std::vector<SCCOLROW> aOrderIndices = GetSortOrder(aSortData, pMatSrc);
    // create sorted matrix
    ScMatrixRef pResMat = CreateSortedMatrix(aSortData, pMatSrc,
        ScRange(aSortData.nCol1, aSortData.nRow1, aSortData.nSourceTab,
                aSortData.nCol2, aSortData.nRow2, aSortData.nSourceTab),
                aOrderIndices, nsC, nsR);

    if (pResMat)
        PushMatrix(pResMat);
    else
        PushIllegalParameter();
}

void ScInterpreter::ScSortBy()
{
    sal_uInt8 nParamCount = GetByte();

    if (nParamCount < 2/*|| (nParamCount % 2 != 1)*/)
    {
        PushError(FormulaError::ParameterExpected);
        return;
    }

    sal_uInt8 nSortCount = nParamCount / 2;

    ScSortParam aSortData;
    aSortData.maKeyState.resize(nSortCount);

    // 127th, ..., 3rd and 2nd argument: sort by range/array and sort orders pair
    sal_uInt8 nSortBy = nSortCount;
    ScMatrixRef pFullMatSortBy = nullptr;
    while (nSortBy-- > 0 && nGlobalError == FormulaError::NONE)
    {
        // 3rd argument sort_order optional: default ascending
        if (nParamCount >= 3 && (nParamCount % 2 == 1))
        {
            sal_Int8 nSortOrder = static_cast<sal_Int8>(GetInt32WithDefault(1));
            if (nSortOrder != 1 && nSortOrder != -1)
            {
                PushIllegalParameter();
                return;
            }
            aSortData.maKeyState[nSortBy].bAscending = (nSortOrder == 1);
            nParamCount--;
        }

        // 2nd argument: take sort by ranges
        ScMatrixRef pMatSortBy = nullptr;
        SCSIZE nbyC = 0, nbyR = 0;
        switch (GetStackType())
        {
            case svSingleRef:
            case svDoubleRef:
            case svMatrix:
            case svExternalSingleRef:
            case svExternalDoubleRef:
            {
                if (nSortCount == 1)
                {
                    pFullMatSortBy = GetMatrix();
                    if (!pFullMatSortBy)
                    {
                        PushIllegalParameter();
                        return;
                    }
                    pFullMatSortBy->GetDimensions(nbyC, nbyR);
                }
                else
                {
                    pMatSortBy = GetMatrix();
                    if (!pMatSortBy)
                    {
                        PushIllegalParameter();
                        return;
                    }
                    pMatSortBy->GetDimensions(nbyC, nbyR);
                }

                // last->first (backward) sortby array
                if (nSortBy == nSortCount - 1)
                {
                    if (nbyC == 1 && nbyR > 1)
                        aSortData.bByRow = true;
                    else if (nbyR == 1 && nbyC > 1)
                        aSortData.bByRow = false;
                    else
                    {
                        PushIllegalParameter();
                        return;
                    }

                    if (nSortCount > 1)
                    {
                        pFullMatSortBy = GetNewMat(aSortData.bByRow ? (nbyC * nSortCount) : nbyC,
                            aSortData.bByRow ? nbyR : (nbyR * nSortCount), /*bEmpty*/true);
                    }
                }
            }
            break;

            default:
                PushIllegalParameter();
                return;
        }

        // ..., penultimate sortby arrays
        if (nSortCount > 1 && nSortBy <= nSortCount - 1)
        {
            SCSIZE nCheckCol = 0, nCheckRow = 0;
            pFullMatSortBy->GetDimensions(nCheckCol, nCheckRow);
            if ((aSortData.bByRow && nbyR == nCheckRow && nbyC == 1) ||
                (!aSortData.bByRow && nbyC == nCheckCol && nbyR == 1))
            {
                for (SCSIZE ci = 0; ci < nbyC; ci++)//col
                {
                    for (SCSIZE rj = 0; rj < nbyR; rj++)//row
                    {
                        if (pMatSortBy->IsStringOrEmpty(ci, rj))
                        {
                            if (aSortData.bByRow)
                                pFullMatSortBy->PutString(pMatSortBy->GetString(ci, rj), ci + nSortBy, rj);
                            else
                                pFullMatSortBy->PutString(pMatSortBy->GetString(ci, rj), ci, rj + nSortBy);
                        }
                        else
                        {
                            if (aSortData.bByRow)
                                pFullMatSortBy->PutDouble(pMatSortBy->GetDouble(ci, rj), ci + nSortBy, rj);
                            else
                                pFullMatSortBy->PutDouble(pMatSortBy->GetDouble(ci, rj), ci, rj + nSortBy);
                        }
                    }
                }
            }
            else
            {
                PushIllegalParameter();
                return;
            }
        }

        aSortData.maKeyState[nSortBy].bDoSort = true;
        aSortData.maKeyState[nSortBy].nField = nSortBy;

        nParamCount--;
    }

    // 1st argument is the range/array to be sorted
    SCSIZE nsC = 0, nsR = 0;
    SCCOL nSortCol1 = 0, nSortCol2 = 0;
    SCROW nSortRow1 = 0, nSortRow2 = 0;
    SCTAB nSortTab1 = 0, nSortTab2 = 0;
    ScMatrixRef pMatSrc = nullptr;
    switch ( GetStackType() )
    {
        case svSingleRef:
            PopSingleRef(nSortCol1, nSortRow1, nSortTab1);
            nSortCol2 = nSortCol1;
            nSortRow2 = nSortRow1;
            nsC = nSortCol2 - nSortCol1 + 1;
            nsR = nSortRow2 - nSortRow1 + 1;
        break;
        case svDoubleRef:
        {
            PopDoubleRef(nSortCol1, nSortRow1, nSortTab1, nSortCol2, nSortRow2, nSortTab2);
            if (nSortTab1 != nSortTab2)
            {
                PushIllegalParameter();
                return;
            }
            nsC = nSortCol2 - nSortCol1 + 1;
            nsR = nSortRow2 - nSortRow1 + 1;
        }
        break;
        case svMatrix:
        case svExternalSingleRef:
        case svExternalDoubleRef:
        {
            pMatSrc = GetMatrix();
            if (!pMatSrc)
            {
                PushIllegalParameter();
                return;
            }
            pMatSrc->GetDimensions(nsC, nsR);
            nSortCol2 = nsC - 1; // nSortCol1 = 0
            nSortRow2 = nsR - 1; // nSortRow1 = 0
        }
        break;

        default:
            PushIllegalParameter();
            return;
    }

    SCSIZE nCheckMatrixCol = 0, nCheckMatrixRow = 0;
    pFullMatSortBy->GetDimensions(nCheckMatrixCol, nCheckMatrixRow);
    if (nGlobalError != FormulaError::NONE)
    {
        PushError(nGlobalError);
        return;
    }
    else if ((aSortData.bByRow && nsR != nCheckMatrixRow) ||
        (!aSortData.bByRow && nsC != nCheckMatrixCol))
    {
        PushIllegalParameter();
        return;
    }
    else
    {
        aSortData.nCol2 = nCheckMatrixCol - 1;
        aSortData.nRow2 = nCheckMatrixRow - 1;
    }

    // sorting...
    std::vector<SCCOLROW> aOrderIndices = GetSortOrder(aSortData, pFullMatSortBy);
    // create sorted matrix
    ScMatrixRef pResMat = CreateSortedMatrix(aSortData, pMatSrc,
        ScRange(nSortCol1, nSortRow1, nSortTab1, nSortCol2, nSortRow2, nSortTab2),
        aOrderIndices, nsC, nsR);

    if (pResMat)
        PushMatrix(pResMat);
    else
        PushIllegalParameter();
}

void ScInterpreter::ScUnique()
{
    sal_uInt8 nParamCount = GetByte();
    if (!MustHaveParamCount(nParamCount, 1, 3))
        return;

    // 3rd argument optional - Exactly_once: default FALSE
    bool bExactly_once = false;
    if (nParamCount == 3)
        bExactly_once = GetBoolWithDefault(false);

    // 2nd argument optional - default: By_Col = false --> bByRow = true
    bool bByRow = true;
    if (nParamCount >= 2)
        bByRow = !GetBoolWithDefault(false);

    // 1st argument: take unique search range
    ScMatrixRef pMatSource = nullptr;
    SCSIZE nsC = 0, nsR = 0;
    switch (GetStackType())
    {
        case svSingleRef:
        case svDoubleRef:
        case svMatrix:
        case svExternalSingleRef:
        case svExternalDoubleRef:
        {
            pMatSource = GetMatrix();
            if (!pMatSource)
            {
                PushIllegalParameter();
                return;
            }

            pMatSource->GetDimensions(nsC, nsR);
        }
        break;

        default:
            PushIllegalParameter();
            return;
    }

    if (nGlobalError != FormulaError::NONE || nsC < 1 || nsR < 1)
    {
        PushIllegalArgument();
        return;
    }

    // Create unique dataset
    std::unordered_set<OUString> aStrSet;
    std::vector<std::pair<SCSIZE, OUString>> aResPos;
    SCSIZE nOut = bByRow ? nsR : nsC;
    SCSIZE nIn = bByRow ? nsC : nsR;

    for (SCSIZE i = 0; i < nOut; i++)
    {
        OUString aStr;
        for (SCSIZE j = 0; j < nIn; j++)
        {
            OUString aCellStr = bByRow ? pMatSource->GetString(mrContext, j, i).getString() :
                pMatSource->GetString(mrContext, i, j).getString();
            aStr += aCellStr + u"\x0001";
        }

        if (aStrSet.insert(aStr).second) // unique if inserted
        {
            aResPos.emplace_back(std::make_pair(i, aStr));
        }
        else
        {
            if (bExactly_once)
            {
                auto it = std::find_if(aResPos.begin(), aResPos.end(),
                    [&aStr](const std::pair<SCSIZE, OUString>& aRes)
                    {
                        return aRes.second.equals(aStr);
                    }
                );
                if (it != aResPos.end())
                    aResPos.erase(it);
            }
        }
    }
    // No result
    if (aResPos.size() == 0)
    {
        if (nGlobalError != FormulaError::NONE)
        {
            PushIllegalArgument();
        }
        else
        {
            PushNA();
        }
        return;
    }
    // fill result matrix with unique values
    ScMatrixRef pResMat = bByRow ? GetNewMat(nsC, aResPos.size(), /*bEmpty*/true) :
        GetNewMat(aResPos.size(), nsR, /*bEmpty*/true);
    for (SCSIZE iPos = 0; iPos < aResPos.size(); iPos++)
    {
        if (bByRow)
        {
            for (SCSIZE col = 0; col < nsC; col++)
            {
                if (!pMatSource->IsStringOrEmpty(col, aResPos[iPos].first))
                {
                    pResMat->PutDouble(pMatSource->GetDouble(col, aResPos[iPos].first), col, iPos);
                }
                else
                {
                    pResMat->PutString(pMatSource->GetString(col, aResPos[iPos].first), col, iPos);
                }
            }
        }
        else
        {
            for (SCSIZE row = 0; row < nsR; row++)
            {
                if (!pMatSource->IsStringOrEmpty(aResPos[iPos].first, row))
                {
                    pResMat->PutDouble(pMatSource->GetDouble(aResPos[iPos].first, row), iPos, row);
                }
                else
                {
                    pResMat->PutString(pMatSource->GetString(aResPos[iPos].first, row), iPos, row);
                }
            }
        }
    }

    if (!pResMat)
    {
        PushIllegalArgument();
    }
    else
    {
        PushMatrix(pResMat);
    }
}

void ScInterpreter::ScSubTotal()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCountMinWithStackCheck( nParamCount, 2 ) )
        return;

    // We must fish the 1st parameter deep from the stack! And push it on top.
    const FormulaToken* p = pStack[ sp - nParamCount ];
    PushWithoutError( *p );
    sal_Int32 nFunc = GetInt32();
    mnSubTotalFlags |= SubtotalFlags::IgnoreNestedStAg | SubtotalFlags::IgnoreFiltered;
    if (nFunc > 100)
    {
        // For opcodes 101 through 111, we need to skip hidden cells.
        // Other than that these opcodes are identical to 1 through 11.
        mnSubTotalFlags |= SubtotalFlags::IgnoreHidden;
        nFunc -= 100;
    }

    if ( nGlobalError != FormulaError::NONE || nFunc < 1 || nFunc > 11 )
        PushIllegalArgument();  // simulate return on stack, not SetError(...)
    else
    {
        cPar = nParamCount - 1;
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
    }
    mnSubTotalFlags = SubtotalFlags::NONE;
    // Get rid of the 1st (fished) parameter.
    FormulaConstTokenRef xRef( PopToken());
    Pop();
    PushTokenRef( xRef);
}

void ScInterpreter::ScAggregate()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCountMinWithStackCheck( nParamCount, 3 ) )
        return;

    const FormulaError nErr = nGlobalError;
    nGlobalError = FormulaError::NONE;

    // fish the 1st parameter from the stack and push it on top.
    const FormulaToken* p = pStack[ sp - nParamCount ];
    PushWithoutError( *p );
    sal_Int32 nFunc = GetInt32();
    // fish the 2nd parameter from the stack and push it on top.
    const FormulaToken* p2 = pStack[ sp - ( nParamCount - 1 ) ];
    PushWithoutError( *p2 );
    sal_Int32 nOption = GetInt32();

    if ( nGlobalError != FormulaError::NONE || nFunc < 1 || nFunc > 19 )
    {
        nGlobalError = nErr;
        PushIllegalArgument();
    }
    else
    {
        switch ( nOption)
        {
            case 0 : // ignore nested SUBTOTAL and AGGREGATE functions
                mnSubTotalFlags = SubtotalFlags::IgnoreNestedStAg;
                break;
            case 1 : // ignore hidden rows, nested SUBTOTAL and AGGREGATE functions
                mnSubTotalFlags = SubtotalFlags::IgnoreHidden | SubtotalFlags::IgnoreNestedStAg;
                break;
            case 2 : // ignore error values, nested SUBTOTAL and AGGREGATE functions
                mnSubTotalFlags = SubtotalFlags::IgnoreErrVal | SubtotalFlags::IgnoreNestedStAg;
                break;
            case 3 : // ignore hidden rows, error values, nested SUBTOTAL and AGGREGATE functions
                mnSubTotalFlags = SubtotalFlags::IgnoreHidden | SubtotalFlags::IgnoreErrVal | SubtotalFlags::IgnoreNestedStAg;
                break;
            case 4 : // ignore nothing
                mnSubTotalFlags = SubtotalFlags::NONE;
                break;
            case 5 : // ignore hidden rows
                mnSubTotalFlags = SubtotalFlags::IgnoreHidden ;
                break;
            case 6 : // ignore error values
                mnSubTotalFlags = SubtotalFlags::IgnoreErrVal ;
                break;
            case 7 : // ignore hidden rows and error values
                mnSubTotalFlags = SubtotalFlags::IgnoreHidden | SubtotalFlags::IgnoreErrVal ;
                break;
            default :
                nGlobalError = nErr;
                PushIllegalArgument();
                return;
        }

        if ((mnSubTotalFlags & SubtotalFlags::IgnoreErrVal) == SubtotalFlags::NONE)
            nGlobalError = nErr;

        cPar = nParamCount - 2;
        switch ( nFunc )
        {
            case AGGREGATE_FUNC_AVE     : ScAverage(); break;
            case AGGREGATE_FUNC_CNT     : ScCount();   break;
            case AGGREGATE_FUNC_CNT2    : ScCount2();  break;
            case AGGREGATE_FUNC_MAX     : ScMax();     break;
            case AGGREGATE_FUNC_MIN     : ScMin();     break;
            case AGGREGATE_FUNC_PROD    : ScProduct(); break;
            case AGGREGATE_FUNC_STD     : ScStDev();   break;
            case AGGREGATE_FUNC_STDP    : ScStDevP();  break;
            case AGGREGATE_FUNC_SUM     : ScSum();     break;
            case AGGREGATE_FUNC_VAR     : ScVar();     break;
            case AGGREGATE_FUNC_VARP    : ScVarP();    break;
            case AGGREGATE_FUNC_MEDIAN  : ScMedian();            break;
            case AGGREGATE_FUNC_MODSNGL : ScModalValue();        break;
            case AGGREGATE_FUNC_LARGE   : ScLarge();             break;
            case AGGREGATE_FUNC_SMALL   : ScSmall();             break;
            case AGGREGATE_FUNC_PERCINC : ScPercentile( true );  break;
            case AGGREGATE_FUNC_QRTINC  : ScQuartile( true );    break;
            case AGGREGATE_FUNC_PERCEXC : ScPercentile( false ); break;
            case AGGREGATE_FUNC_QRTEXC  : ScQuartile( false );   break;
            default:
                nGlobalError = nErr;
                PushIllegalArgument();
            break;
        }
        mnSubTotalFlags = SubtotalFlags::NONE;
    }
    FormulaConstTokenRef xRef( PopToken());
    // Get rid of the 1st and 2nd (fished) parameters.
    Pop();
    Pop();
    PushTokenRef( xRef);
}

std::unique_ptr<ScDBQueryParamBase> ScInterpreter::GetDBParams( bool& rMissingField )
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
        ::std::unique_ptr<ScDBRangeBase> pQueryRef( PopDBDoubleRef() );
        if (!pQueryRef)
            return nullptr;

        bool    bByVal = true;
        double  nVal = 0.0;
        svl::SharedString  aStr;
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
                    ScRefCellValue aCell(mrDoc, aAdr);
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
                    SetError( FormulaError::IllegalParameter );
                }
                break;
            case svMissing :
                PopError();
                if ( bAllowMissingField )
                    rMissingField = true;
                else
                    SetError( FormulaError::IllegalParameter );
                break;
            default:
                PopError();
                SetError( FormulaError::IllegalParameter );
        }

        if (nGlobalError != FormulaError::NONE)
            return nullptr;

        unique_ptr<ScDBRangeBase> pDBRef( PopDBDoubleRef() );

        if (nGlobalError != FormulaError::NONE || !pDBRef)
            return nullptr;

        if ( bRangeFake )
        {
            // range parameter must match entire database range
            if (pDBRef->isRangeEqual(aMissingRange))
                rMissingField = true;
            else
                SetError( FormulaError::IllegalParameter );
        }

        if (nGlobalError != FormulaError::NONE)
            return nullptr;

        SCCOL nField = pDBRef->getFirstFieldColumn();
        if (rMissingField)
            ; // special case
        else if (bByVal)
            nField = pDBRef->findFieldColumn(static_cast<SCCOL>(nVal));
        else
        {
            FormulaError nErr = FormulaError::NONE;
            nField = pDBRef->findFieldColumn(aStr.getString(), &nErr);
            SetError(nErr);
        }

        if (!mrDoc.ValidCol(nField))
            return nullptr;

        unique_ptr<ScDBQueryParamBase> pParam( pDBRef->createQueryParam(pQueryRef.get()) );

        if (pParam)
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
                OUString aQueryStr = rItem.maString.getString();
                bool bNumber = mrContext.NFIsNumberFormat(
                    aQueryStr, nIndex, rItem.mfVal);
                rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;

                if (!bNumber && pParam->eSearchType == utl::SearchParam::SearchType::Normal)
                    pParam->eSearchType = DetectSearchType(aQueryStr, mrDoc);
            }
            return pParam;
        }
    }
    return nullptr;
}

void ScInterpreter::DBIterator( ScIterFunc eFunc )
{
    double fRes = 0;
    KahanSum fErg = 0;
    sal_uLong nCount = 0;
    bool bMissingField = false;
    unique_ptr<ScDBQueryParamBase> pQueryParam( GetDBParams(bMissingField) );
    if (pQueryParam)
    {
        if (!pQueryParam->IsValidFieldIndex())
        {
            SetError(FormulaError::NoValue);
            return;
        }
        ScDBQueryDataIterator aValIter(mrDoc, mrContext, std::move(pQueryParam));
        ScDBQueryDataIterator::Value aValue;
        if ( aValIter.GetFirst(aValue) && aValue.mnError == FormulaError::NONE )
        {
            switch( eFunc )
            {
                case ifPRODUCT: fRes = 1; break;
                case ifMAX:     fRes = -MAXDOUBLE; break;
                case ifMIN:     fRes = MAXDOUBLE; break;
                default: ; // nothing
            }

            do
            {
                nCount++;
                switch( eFunc )
                {
                    case ifAVERAGE:
                    case ifSUM:
                        fErg += aValue.mfValue;
                        break;
                    case ifSUMSQ:
                        fErg += aValue.mfValue * aValue.mfValue;
                        break;
                    case ifPRODUCT:
                        fRes *= aValue.mfValue;
                        break;
                    case ifMAX:
                        if( aValue.mfValue > fRes ) fRes = aValue.mfValue;
                        break;
                    case ifMIN:
                        if( aValue.mfValue < fRes ) fRes = aValue.mfValue;
                        break;
                    default: ; // nothing
                }
            }
            while ( aValIter.GetNext(aValue) && aValue.mnError == FormulaError::NONE );
        }
        SetError(aValue.mnError);
    }
    else
        SetError( FormulaError::IllegalParameter);
    switch( eFunc )
    {
        case ifCOUNT:   fRes = nCount; break;
        case ifSUM:     fRes = fErg.get(); break;
        case ifSUMSQ:   fRes = fErg.get(); break;
        case ifAVERAGE: fRes = div(fErg.get(), nCount); break;
        default: ; // nothing
    }
    PushDouble( fRes );
}

void ScInterpreter::ScDBSum()
{
    DBIterator( ifSUM );
}

void ScInterpreter::ScDBCount()
{
    bool bMissingField = true;
    unique_ptr<ScDBQueryParamBase> pQueryParam( GetDBParams(bMissingField) );
    if (pQueryParam)
    {
        sal_uLong nCount = 0;
        if ( bMissingField && pQueryParam->GetType() == ScDBQueryParamBase::INTERNAL )
        {   // count all matching records
            // TODO: currently the QueryIterators only return cell pointers of
            // existing cells, so if a query matches an empty cell there's
            // nothing returned, and therefore not counted!
            // Since this has ever been the case and this code here only came
            // into existence to fix #i6899 and it never worked before we'll
            // have to live with it until we reimplement the iterators to also
            // return empty cells, which would mean to adapt all callers of
            // iterators.
            ScDBQueryParamInternal* p = static_cast<ScDBQueryParamInternal*>(pQueryParam.get());
            p->nCol2 = p->nCol1; // Don't forget to select only one column.
            SCTAB nTab = p->nTab;
            // ScQueryCellIteratorDirect doesn't make use of ScDBQueryParamBase::mnField,
            // so the source range has to be restricted, like before the introduction
            // of ScDBQueryParamBase.
            p->nCol1 = p->nCol2 = p->mnField;
            ScQueryCellIteratorDirect aCellIter(mrDoc, mrContext, nTab, *p, true, false);
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
                SetError(FormulaError::NoValue);
                return;
            }
            ScDBQueryDataIterator aValIter(mrDoc, mrContext, std::move(pQueryParam));
            ScDBQueryDataIterator::Value aValue;
            if ( aValIter.GetFirst(aValue) && aValue.mnError == FormulaError::NONE )
            {
                do
                {
                    nCount++;
                }
                while ( aValIter.GetNext(aValue) && aValue.mnError == FormulaError::NONE );
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
    unique_ptr<ScDBQueryParamBase> pQueryParam( GetDBParams(bMissingField) );
    if (pQueryParam)
    {
        if (!pQueryParam->IsValidFieldIndex())
        {
            SetError(FormulaError::NoValue);
            return;
        }
        sal_uLong nCount = 0;
        pQueryParam->mbSkipString = false;
        ScDBQueryDataIterator aValIter(mrDoc, mrContext, std::move(pQueryParam));
        ScDBQueryDataIterator::Value aValue;
        if ( aValIter.GetFirst(aValue) && aValue.mnError == FormulaError::NONE )
        {
            do
            {
                nCount++;
            }
            while ( aValIter.GetNext(aValue) && aValue.mnError == FormulaError::NONE );
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
    KahanSum vSum    = 0.0;
    KahanSum fSum    = 0.0;

    rValCount = 0.0;
    bool bMissingField = false;
    unique_ptr<ScDBQueryParamBase> pQueryParam( GetDBParams(bMissingField) );
    if (pQueryParam)
    {
        if (!pQueryParam->IsValidFieldIndex())
        {
            SetError(FormulaError::NoValue);
            return;
        }
        ScDBQueryDataIterator aValIter(mrDoc, mrContext, std::move(pQueryParam));
        ScDBQueryDataIterator::Value aValue;
        if (aValIter.GetFirst(aValue) && aValue.mnError == FormulaError::NONE)
        {
            do
            {
                rValCount++;
                values.push_back(aValue.mfValue);
                fSum += aValue.mfValue;
            }
            while ((aValue.mnError == FormulaError::NONE) && aValIter.GetNext(aValue));
        }
        SetError(aValue.mnError);
    }
    else
        SetError( FormulaError::IllegalParameter);

    double vMean = fSum.get() / values.size();

    for (double v : values)
        vSum += (v - vMean) * (v - vMean);

    rVal = vSum.get();
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

static bool lcl_IsTableStructuredRef(const OUString& sRefStr, sal_Int32& nIndex)
{
    nIndex = ScGlobal::FindUnquoted(sRefStr, '[');
    return (nIndex > 0 && ScGlobal::FindUnquoted(sRefStr, ']', nIndex + 1) > nIndex);
}

void ScInterpreter::ScIndirect()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 2 )  )
        return;

    // Reference address syntax for INDIRECT is configurable.
    FormulaGrammar::AddressConvention eConv = maCalcConfig.meStringRefAddressSyntax;
    if (eConv == FormulaGrammar::CONV_UNSPECIFIED)
        // Use the current address syntax if unspecified.
        eConv = mrDoc.GetAddressConvention();

    // either CONV_A1_XL_A1 was explicitly configured, or it wasn't possible
    // to determine which syntax to use during doc import
    bool bTryXlA1 = (eConv == FormulaGrammar::CONV_A1_XL_A1);

    if (nParamCount == 2 && 0.0 == GetDouble() )
    {
        // Overwrite the config and try Excel R1C1.
        eConv = FormulaGrammar::CONV_XL_R1C1;
        bTryXlA1 = false;
    }

    svl::SharedString sSharedRefStr = GetString();
    const OUString & sRefStr = sSharedRefStr.getString();
    if (sRefStr.isEmpty())
    {
        // Bail out early for empty cells, rely on "we do have a string" below.
        PushError( FormulaError::NoRef);
        return;
    }

    const ScAddress::Details aDetails( bTryXlA1 ? FormulaGrammar::CONV_OOO : eConv, aPos );
    const ScAddress::Details aDetailsXlA1( FormulaGrammar::CONV_XL_A1, aPos );
    SCTAB nTab = aPos.Tab();

    bool bTableRefNamed = false;
    sal_Int32 nTableRefNamedIndex = -1;
    OUString sTabRefStr;

    // Named expressions and DB range names need to be tried first, as older 1K
    // columns allowed names that would now match a 16k columns cell address.
    do
    {
        ScRangeData* pData = ScRangeStringConverter::GetRangeDataFromString( sRefStr, nTab, mrDoc, eConv);
        if (!pData)
            break;

        // We need this in order to obtain a good range.
        pData->ValidateTabRefs();

        ScRange aRange;

        // This is the usual way to treat named ranges containing
        // relative references.
        if (!pData->IsReference(aRange, aPos))
        {
            sTabRefStr = pData->GetSymbol();
            bTableRefNamed = lcl_IsTableStructuredRef(sTabRefStr, nTableRefNamedIndex);
            // if bTableRefNamed is true, we have a name that maps to a table structured reference.
            // Such a case is handled below.
            break;
        }

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

    do
    {
        if (bTableRefNamed)
            break;

        const OUString & aName( sSharedRefStr.getIgnoreCaseString() );
        ScDBCollection::NamedDBs& rDBs = mrDoc.GetDBCollection()->getNamedDBs();
        const ScDBData* pData = rDBs.findByUpperName( aName);
        if (!pData)
            break;

        ScRange aRange;
        pData->GetArea( aRange);

        // In Excel, specifying a table name without [] resolves to the
        // same as with [], a range that excludes header and totals
        // rows and contains only data rows. Do the same.
        if (pData->HasHeader())
            aRange.aStart.IncRow();
        if (pData->HasTotals())
            aRange.aEnd.IncRow(-1);

        if (aRange.aStart.Row() > aRange.aEnd.Row())
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

    ScRefAddress aRefAd, aRefAd2;
    ScAddress::ExternalInfo aExtInfo;
    if ( !bTableRefNamed &&
         (ConvertDoubleRef(mrDoc, sRefStr, nTab, aRefAd, aRefAd2, aDetails, &aExtInfo) ||
            ( bTryXlA1 && ConvertDoubleRef(mrDoc, sRefStr, nTab, aRefAd,
                                           aRefAd2, aDetailsXlA1, &aExtInfo) ) ) )
    {
        if (aExtInfo.mbExternal)
        {
            PushExternalDoubleRef(
                aExtInfo.mnFileId, aExtInfo.maTabName,
                aRefAd.Col(), aRefAd.Row(), aRefAd.Tab(),
                aRefAd2.Col(), aRefAd2.Row(), aRefAd2.Tab());
        }
        else
            PushDoubleRef( aRefAd, aRefAd2);
    }
    else if ( !bTableRefNamed &&
              (ConvertSingleRef(mrDoc, sRefStr, nTab, aRefAd, aDetails, &aExtInfo) ||
                ( bTryXlA1 && ConvertSingleRef (mrDoc, sRefStr, nTab, aRefAd,
                                                aDetailsXlA1, &aExtInfo) ) ) )
    {
        if (aExtInfo.mbExternal)
        {
            PushExternalSingleRef(
                aExtInfo.mnFileId, aExtInfo.maTabName, aRefAd.Col(), aRefAd.Row(), aRefAd.Tab());
        }
        else
            PushSingleRef( aRefAd);
    }
    else
    {
        // It may be even a TableRef or an external name.
        // Anything else that resolves to one reference could be added
        // here, but we don't want to compile every arbitrary string. This
        // is already nasty enough...
        sal_Int32 nIndex = bTableRefNamed ? nTableRefNamedIndex : -1;
        bool bTableRef = bTableRefNamed;
        if (!bTableRefNamed)
            bTableRef = lcl_IsTableStructuredRef(sRefStr, nIndex);
        bool bExternalName = false;     // External references would had been consumed above already.
        if (!bTableRef)
        {
            // This is our own file name reference representation centric.. but
            // would work also for XL '[doc]'!name and also for
            // '[doc]Sheet1'!name ... sickos.
            if (sRefStr[0] == '\'')
            {
                // Minimum 'a'#name or 'a'!name
                // bTryXlA1 means try both, first our own.
                if (bTryXlA1 || eConv == FormulaGrammar::CONV_OOO)
                {
                    nIndex = ScGlobal::FindUnquoted( sRefStr, '#');
                    if (nIndex >= 3 && sRefStr[nIndex-1] == '\'')
                    {
                        bExternalName = true;
                        eConv = FormulaGrammar::CONV_OOO;
                    }
                }
                if (!bExternalName && (bTryXlA1 || eConv != FormulaGrammar::CONV_OOO))
                {
                    nIndex = ScGlobal::FindUnquoted( sRefStr, '!');
                    if (nIndex >= 3 && sRefStr[nIndex-1] == '\'')
                    {
                        bExternalName = true;
                    }
                }
            }

        }
        if (bExternalName || bTableRef)
        {
            do
            {
                ScCompiler aComp( mrDoc, aPos, mrDoc.GetGrammar());
                aComp.SetRefConvention( eConv);     // must be after grammar
                std::unique_ptr<ScTokenArray> pTokArr( aComp.CompileString(bTableRefNamed ? sTabRefStr : sRefStr));

                if (pTokArr->GetCodeError() != FormulaError::NONE || !pTokArr->GetLen())
                    break;

                // Whatever... use only the specific case.
                if (bExternalName)
                {
                    const formula::FormulaToken* pTok = pTokArr->FirstToken();
                    if (!pTok || pTok->GetType() != svExternalName)
                        break;
                }
                else if (!pTokArr->HasOpCode( ocTableRef))
                    break;

                aComp.CompileTokenArray();

                // A syntactically valid reference will generate exactly
                // one RPN token, a reference or error. Discard everything
                // else as error.
                if (pTokArr->GetCodeLen() != 1)
                    break;

                ScTokenRef xTok( pTokArr->FirstRPNToken());
                if (!xTok)
                    break;

                switch (xTok->GetType())
                {
                    case svSingleRef:
                    case svDoubleRef:
                    case svExternalSingleRef:
                    case svExternalDoubleRef:
                    case svError:
                        PushTokenRef( xTok);
                        // success!
                        return;
                    default:
                        ;   // nothing
                }
            }
            while (false);
        }

        PushError( FormulaError::NoRef);
    }
}

void ScInterpreter::ScAddressFunc()
{
    OUString  sTabStr;

    sal_uInt8    nParamCount = GetByte();
    if( !MustHaveParamCount( nParamCount, 2, 5 ) )
        return;

    if( nParamCount >= 5 )
        sTabStr = GetString().getString();

    FormulaGrammar::AddressConvention eConv = FormulaGrammar::CONV_OOO;      // default
    if (nParamCount >= 4 && 0.0 == GetDoubleWithDefault( 1.0))
        eConv = FormulaGrammar::CONV_XL_R1C1;
    else
    {
        // If A1 syntax is requested then the actual sheet separator and format
        // convention depends on the syntax configured for INDIRECT to match
        // that, and if it is unspecified then the document's address syntax.
        FormulaGrammar::AddressConvention eForceConv = maCalcConfig.meStringRefAddressSyntax;
        if (eForceConv == FormulaGrammar::CONV_UNSPECIFIED)
            eForceConv = mrDoc.GetAddressConvention();
        if (eForceConv == FormulaGrammar::CONV_XL_A1 || eForceConv == FormulaGrammar::CONV_XL_R1C1)
            eConv = FormulaGrammar::CONV_XL_A1;     // for anything Excel use Excel A1
    }

    ScRefFlags  nFlags = ScRefFlags::COL_ABS | ScRefFlags::ROW_ABS;   // default
    if( nParamCount >= 3 )
    {
        sal_Int32 n = GetInt32WithDefault(1);
        switch ( n )
        {
            default :
                PushNoValue();
                return;

            case 5:
            case 1 : break; // default
            case 6:
            case 2 : nFlags = ScRefFlags::ROW_ABS; break;
            case 7:
            case 3 : nFlags = ScRefFlags::COL_ABS; break;
            case 8:
            case 4 : nFlags = ScRefFlags::ZERO; break; // both relative
        }
    }
    nFlags |= ScRefFlags::VALID | ScRefFlags::ROW_VALID | ScRefFlags::COL_VALID;

    SCCOL nCol = static_cast<SCCOL>(GetInt16());
    SCROW nRow = static_cast<SCROW>(GetInt32());
    if( eConv == FormulaGrammar::CONV_XL_R1C1 )
    {
        // YUCK!  The XL interface actually treats rel R1C1 refs differently
        // than A1
        if( !(nFlags & ScRefFlags::COL_ABS) )
            nCol += aPos.Col() + 1;
        if( !(nFlags & ScRefFlags::ROW_ABS) )
            nRow += aPos.Row() + 1;
    }

    --nCol;
    --nRow;
    if (nGlobalError != FormulaError::NONE || !mrDoc.ValidCol( nCol) || !mrDoc.ValidRow( nRow))
    {
        PushIllegalArgument();
        return;
    }

    const ScAddress::Details aDetails( eConv, aPos );
    const ScAddress aAdr( nCol, nRow, 0);
    OUString aRefStr(aAdr.Format(nFlags, &mrDoc, aDetails));

    if( nParamCount >= 5 && !sTabStr.isEmpty() )
    {
        OUString aDoc;
        if (eConv == FormulaGrammar::CONV_OOO)
        {
            // Isolate Tab from 'Doc'#Tab
            sal_Int32 nPos = ScCompiler::GetDocTabPos( sTabStr);
            if (nPos != -1)
            {
                if (sTabStr[nPos+1] == '$')
                    ++nPos;     // also split 'Doc'#$Tab
                aDoc = sTabStr.copy( 0, nPos+1);
                sTabStr = sTabStr.copy( nPos+1);
            }
        }
        /* TODO: yet unsupported external reference in CONV_XL_R1C1 syntax may
         * need some extra handling to isolate Tab from Doc. */
        if (sTabStr[0] != '\'' || !sTabStr.endsWith("'"))
            ScCompiler::CheckTabQuotes( sTabStr, eConv);
        if (!aDoc.isEmpty())
            sTabStr = aDoc + sTabStr;
        sTabStr += (eConv == FormulaGrammar::CONV_XL_R1C1 || eConv == FormulaGrammar::CONV_XL_A1) ?
            std::u16string_view(u"!") : std::u16string_view(u".");
        sTabStr += aRefStr;
        PushString( sTabStr );
    }
    else
        PushString( aRefStr );
}

void ScInterpreter::ScOffset()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;

    bool bNewWidth = false;
    bool bNewHeight = false;
    sal_Int32 nColNew = 1, nRowNew = 1;
    if (nParamCount == 5)
    {
        if (IsMissing())
            PopError();
        else
        {
            nColNew = GetInt32();
            bNewWidth = true;
        }
    }
    if (nParamCount >= 4)
    {
        if (IsMissing())
            PopError();
        else
        {
            nRowNew = GetInt32();
            bNewHeight = true;
        }
    }
    sal_Int32 nColPlus = GetInt32();
    sal_Int32 nRowPlus = GetInt32();
    if (nGlobalError != FormulaError::NONE)
    {
        PushError( nGlobalError);
        return;
    }
    if (nColNew <= 0 || nRowNew <= 0)
    {
        PushIllegalArgument();
        return;
    }
    SCCOL nCol1(0);
    SCROW nRow1(0);
    SCTAB nTab1(0);
    SCCOL nCol2(0);
    SCROW nRow2(0);
    SCTAB nTab2(0);
    switch (GetStackType())
    {
    case svSingleRef:
    {
        PopSingleRef(nCol1, nRow1, nTab1);
        if (!bNewWidth && !bNewHeight)
        {
            nCol1 = static_cast<SCCOL>(static_cast<tools::Long>(nCol1) + nColPlus);
            nRow1 = static_cast<SCROW>(static_cast<tools::Long>(nRow1) + nRowPlus);
            if (!mrDoc.ValidCol(nCol1) || !mrDoc.ValidRow(nRow1))
                PushIllegalArgument();
            else
                PushSingleRef(nCol1, nRow1, nTab1);
        }
        else
        {
            nCol1 = static_cast<SCCOL>(static_cast<tools::Long>(nCol1)+nColPlus);
            nRow1 = static_cast<SCROW>(static_cast<tools::Long>(nRow1)+nRowPlus);
            nCol2 = static_cast<SCCOL>(static_cast<tools::Long>(nCol1)+nColNew-1);
            nRow2 = static_cast<SCROW>(static_cast<tools::Long>(nRow1)+nRowNew-1);
            if (!mrDoc.ValidCol(nCol1) || !mrDoc.ValidRow(nRow1) ||
                !mrDoc.ValidCol(nCol2) || !mrDoc.ValidRow(nRow2))
                PushIllegalArgument();
            else
                PushDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab1);
        }
        break;
    }
    case svExternalSingleRef:
    {
        sal_uInt16 nFileId;
        OUString aTabName;
        ScSingleRefData aRef;
        PopExternalSingleRef(nFileId, aTabName, aRef);
        ScAddress aAbsRef = aRef.toAbs(mrDoc, aPos);
        nCol1 = aAbsRef.Col();
        nRow1 = aAbsRef.Row();
        nTab1 = aAbsRef.Tab();

        if (!bNewWidth && !bNewHeight)
        {
            nCol1 = static_cast<SCCOL>(static_cast<tools::Long>(nCol1) + nColPlus);
            nRow1 = static_cast<SCROW>(static_cast<tools::Long>(nRow1) + nRowPlus);
            if (!mrDoc.ValidCol(nCol1) || !mrDoc.ValidRow(nRow1))
                PushIllegalArgument();
            else
                PushExternalSingleRef(nFileId, aTabName, nCol1, nRow1, nTab1);
        }
        else
        {
            nCol1 = static_cast<SCCOL>(static_cast<tools::Long>(nCol1)+nColPlus);
            nRow1 = static_cast<SCROW>(static_cast<tools::Long>(nRow1)+nRowPlus);
            nCol2 = static_cast<SCCOL>(static_cast<tools::Long>(nCol1)+nColNew-1);
            nRow2 = static_cast<SCROW>(static_cast<tools::Long>(nRow1)+nRowNew-1);
            nTab2 = nTab1;
            if (!mrDoc.ValidCol(nCol1) || !mrDoc.ValidRow(nRow1) ||
                !mrDoc.ValidCol(nCol2) || !mrDoc.ValidRow(nRow2))
                PushIllegalArgument();
            else
                PushExternalDoubleRef(nFileId, aTabName, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
        }
        break;
    }
    case svDoubleRef:
    {
        PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
        if (!bNewWidth)
            nColNew = nCol2 - nCol1 + 1;
        if (!bNewHeight)
            nRowNew = nRow2 - nRow1 + 1;
        nCol1 = static_cast<SCCOL>(static_cast<tools::Long>(nCol1)+nColPlus);
        nRow1 = static_cast<SCROW>(static_cast<tools::Long>(nRow1)+nRowPlus);
        nCol2 = static_cast<SCCOL>(static_cast<tools::Long>(nCol1)+nColNew-1);
        nRow2 = static_cast<SCROW>(static_cast<tools::Long>(nRow1)+nRowNew-1);
        if (!mrDoc.ValidCol(nCol1) || !mrDoc.ValidRow(nRow1) ||
            !mrDoc.ValidCol(nCol2) || !mrDoc.ValidRow(nRow2) || nTab1 != nTab2)
            PushIllegalArgument();
        else
            PushDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab1);
        break;
    }
    case svExternalDoubleRef:
    {
        sal_uInt16 nFileId;
        OUString aTabName;
        ScComplexRefData aRef;
        PopExternalDoubleRef(nFileId, aTabName, aRef);
        ScRange aAbs = aRef.toAbs(mrDoc, aPos);
        nCol1 = aAbs.aStart.Col();
        nRow1 = aAbs.aStart.Row();
        nTab1 = aAbs.aStart.Tab();
        nCol2 = aAbs.aEnd.Col();
        nRow2 = aAbs.aEnd.Row();
        nTab2 = aAbs.aEnd.Tab();
        if (!bNewWidth)
            nColNew = nCol2 - nCol1 + 1;
        if (!bNewHeight)
            nRowNew = nRow2 - nRow1 + 1;
        nCol1 = static_cast<SCCOL>(static_cast<tools::Long>(nCol1)+nColPlus);
        nRow1 = static_cast<SCROW>(static_cast<tools::Long>(nRow1)+nRowPlus);
        nCol2 = static_cast<SCCOL>(static_cast<tools::Long>(nCol1)+nColNew-1);
        nRow2 = static_cast<SCROW>(static_cast<tools::Long>(nRow1)+nRowNew-1);
        if (!mrDoc.ValidCol(nCol1) || !mrDoc.ValidRow(nRow1) ||
            !mrDoc.ValidCol(nCol2) || !mrDoc.ValidRow(nRow2) || nTab1 != nTab2)
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

void ScInterpreter::ScIndex()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 4 ) )
        return;

    sal_uInt32 nArea;
    size_t nAreaCount;
    SCCOL nCol;
    SCROW nRow;
    if (nParamCount == 4)
        nArea = GetUInt32();
    else
        nArea = 1;
    bool bColMissing;
    if (nParamCount >= 3)
    {
        bColMissing = IsMissing();
        nCol = static_cast<SCCOL>(GetInt16());
    }
    else
    {
        bColMissing = false;
        nCol = 0;
    }
    if (nParamCount >= 2)
        nRow = static_cast<SCROW>(GetInt32());
    else
        nRow = 0;
    if (GetStackType() == svRefList)
        nAreaCount = (sp ? pStack[sp-1]->GetRefList()->size() : 0);
    else
        nAreaCount = 1;     // one reference or array or whatever
    if (nGlobalError != FormulaError::NONE || nAreaCount == 0 || static_cast<size_t>(nArea) > nAreaCount)
    {
        PushError( FormulaError::NoRef);
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
                    SetError(FormulaError::IllegalArgument);
                sal_uInt16 nOldSp = sp;
                ScMatrixRef pMat = GetMatrix();
                if (pMat)
                {
                    SCSIZE nC, nR;
                    pMat->GetDimensions(nC, nR);

                    // Access one element of a vector independent of col/row
                    // orientation. Excel documentation does not mention, but
                    // i62850 had a .xls example of a row vector accessed by
                    // row number returning one element. This
                    // INDEX(row_vector;element) behaves the same as
                    // INDEX(row_vector;0;element) and thus contradicts Excel
                    // documentation where the second parameter is always
                    // row_num.
                    //
                    // ODFF v1.3 in 6.14.6 INDEX states "If DataSource is a
                    // one-dimensional row vector, Row is optional, which
                    // effectively makes Row act as the column offset into the
                    // vector". Guess the first Row is a typo and should read
                    // Column instead.

                    const bool bRowVectorSpecial = (nParamCount == 2 || bColMissing);
                    const bool bRowVectorElement = (nR == 1 && (nCol != 0 || (bRowVectorSpecial && nRow != 0)));
                    const bool bVectorElement = (bRowVectorElement || (nC == 1 && nRow != 0));

                    if (nC == 0 || nR == 0 ||
                            (!bVectorElement && (o3tl::make_unsigned(nCol) > nC ||
                                                 o3tl::make_unsigned(nRow) > nR)))
                        PushError( FormulaError::NoRef);
                    else if (nCol == 0 && nRow == 0)
                        sp = nOldSp;
                    else if (bVectorElement)
                    {
                        // Vectors here don't replicate to the other dimension.
                        SCSIZE nElement, nOtherDimension;
                        if (bRowVectorElement && !bRowVectorSpecial)
                        {
                            nElement = o3tl::make_unsigned(nCol);
                            nOtherDimension = o3tl::make_unsigned(nRow);
                        }
                        else
                        {
                            nElement = o3tl::make_unsigned(nRow);
                            nOtherDimension = o3tl::make_unsigned(nCol);
                        }

                        if (nElement == 0 || nElement > nC * nR || nOtherDimension > 1)
                            PushError( FormulaError::NoRef);
                        else
                        {
                            --nElement;
                            if (pMat->IsStringOrEmpty( nElement))
                                PushString( pMat->GetString(nElement).getString());
                            else
                                PushDouble( pMat->GetDouble( nElement));
                        }
                    }
                    else if (nCol == 0)
                    {
                        ScMatrixRef pResMat = GetNewMat(nC, 1, /*bEmpty*/true);
                        if (pResMat)
                        {
                            SCSIZE nRowMinus1 = static_cast<SCSIZE>(nRow - 1);
                            for (SCSIZE i = 0; i < nC; i++)
                                if (!pMat->IsStringOrEmpty(i, nRowMinus1))
                                    pResMat->PutDouble(pMat->GetDouble(i,
                                                nRowMinus1), i, 0);
                                else
                                    pResMat->PutString(pMat->GetString(i, nRowMinus1), i, 0);

                            PushMatrix(pResMat);
                        }
                        else
                            PushError( FormulaError::NoRef);
                    }
                    else if (nRow == 0)
                    {
                        ScMatrixRef pResMat = GetNewMat(1, nR, /*bEmpty*/true);
                        if (pResMat)
                        {
                            SCSIZE nColMinus1 = static_cast<SCSIZE>(nCol - 1);
                            for (SCSIZE i = 0; i < nR; i++)
                                if (!pMat->IsStringOrEmpty(nColMinus1, i))
                                    pResMat->PutDouble(pMat->GetDouble(nColMinus1,
                                                i), i);
                                else
                                    pResMat->PutString(pMat->GetString(nColMinus1, i), i);
                            PushMatrix(pResMat);
                        }
                        else
                            PushError( FormulaError::NoRef);
                    }
                    else
                    {
                        if (!pMat->IsStringOrEmpty( static_cast<SCSIZE>(nCol-1),
                                    static_cast<SCSIZE>(nRow-1)))
                            PushDouble( pMat->GetDouble(
                                        static_cast<SCSIZE>(nCol-1),
                                        static_cast<SCSIZE>(nRow-1)));
                        else
                            PushString( pMat->GetString(
                                        static_cast<SCSIZE>(nCol-1),
                                        static_cast<SCSIZE>(nRow-1)).getString());
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
                    PushError( FormulaError::NoRef);
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
                    FormulaConstTokenRef xRef = PopToken();
                    if (nGlobalError != FormulaError::NONE || !xRef)
                    {
                        PushError( FormulaError::NoRef);
                        return;
                    }
                    ScRange aRange( ScAddress::UNINITIALIZED);
                    DoubleRefToRange( (*(xRef->GetRefList()))[nArea-1], aRange);
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
                    PushError( FormulaError::NoRef);
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
                        nCol =static_cast<SCCOL>(nRow);
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
            PopError();
            PushError( FormulaError::NoRef);
    }
}

void ScInterpreter::ScMultiArea()
{
    // Legacy support, convert to RefList
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCountMin( nParamCount, 1))
    {
        while (nGlobalError == FormulaError::NONE && nParamCount-- > 1)
        {
            ScUnionFunc();
        }
    }
}

void ScInterpreter::ScAreas()
{
    sal_uInt8 nParamCount = GetByte();
    if (!MustHaveParamCount( nParamCount, 1))
        return;

    size_t nCount = 0;
    switch (GetStackType())
    {
        case svSingleRef:
            {
                FormulaConstTokenRef xT = PopToken();
                ValidateRef( *xT->GetSingleRef());
                ++nCount;
            }
            break;
        case svDoubleRef:
            {
                FormulaConstTokenRef xT = PopToken();
                ValidateRef( *xT->GetDoubleRef());
                ++nCount;
            }
            break;
        case svRefList:
            {
                FormulaConstTokenRef xT = PopToken();
                ValidateRef( *(xT->GetRefList()));
                nCount += xT->GetRefList()->size();
            }
            break;
        default:
            SetError( FormulaError::IllegalParameter);
    }
    PushDouble( double(nCount));
}

void ScInterpreter::ScCurrency()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 2 ) )
        return;

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
        fFac = pow( double(10), fDec );
    else
        fFac = 1.0;
    if (fVal < 0.0)
        fVal = ceil(fVal*fFac-0.5)/fFac;
    else
        fVal = floor(fVal*fFac+0.5)/fFac;
    const Color* pColor = nullptr;
    if ( fDec < 0.0 )
        fDec = 0.0;
    sal_uLong nIndex = mrContext.NFGetStandardFormat(
                                    SvNumFormatType::CURRENCY,
                                    ScGlobal::eLnge);
    if ( static_cast<sal_uInt16>(fDec) != mrContext.NFGetFormatPrecision( nIndex ) )
    {
        OUString sFormatString = mrContext.NFGenerateFormat(
                                               nIndex,
                                               ScGlobal::eLnge,
                                               true,        // with thousands separator
                                               false,       // not red
                                               static_cast<sal_uInt16>(fDec));// decimal places
        if (!mrContext.NFGetPreviewString(sFormatString,
                                          fVal,
                                          aStr,
                                          &pColor,
                                          ScGlobal::eLnge))
            SetError(FormulaError::IllegalArgument);
    }
    else
    {
        mrContext.NFGetOutputString(fVal, nIndex, aStr, &pColor);
    }
    PushString(aStr);
}

void ScInterpreter::ScReplace()
{
    if ( !MustHaveParamCount( GetByte(), 4 ) )
        return;

    OUString aNewStr = GetString().getString();
    sal_Int32 nCount = GetStringPositionArgument();
    sal_Int32 nPos   = GetStringPositionArgument();
    OUString aOldStr = GetString().getString();
    if (nPos < 1 || nCount < 0)
        PushIllegalArgument();
    else
    {
        sal_Int32 nLen   = aOldStr.getLength();
        if (nPos > nLen + 1)
            nPos = nLen + 1;
        if (nCount > nLen - nPos + 1)
            nCount = nLen - nPos + 1;
        sal_Int32 nIdx = 0;
        sal_Int32 nCnt = 0;
        while ( nIdx < nLen && nPos > nCnt + 1 )
        {
            aOldStr.iterateCodePoints( &nIdx );
            ++nCnt;
        }
        sal_Int32 nStart = nIdx;
        while ( nIdx < nLen && nPos + nCount - 1 > nCnt )
        {
            aOldStr.iterateCodePoints( &nIdx );
            ++nCnt;
        }
        if ( CheckStringResultLen( aOldStr, aNewStr.getLength() - (nIdx - nStart) ) )
            aOldStr = aOldStr.replaceAt( nStart, nIdx - nStart, aNewStr );
        PushString( aOldStr );
    }
}

void ScInterpreter::ScFixed()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 3 ) )
        return;

    OUString aStr;
    double fDec;
    bool bThousand;
    if (nParamCount == 3)
        bThousand = !GetBool();     // Param true: no thousands separator
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
        fFac = pow( double(10), fDec );
    else
        fFac = 1.0;
    if (fVal < 0.0)
        fVal = ceil(fVal*fFac-0.5)/fFac;
    else
        fVal = floor(fVal*fFac+0.5)/fFac;
    const Color* pColor = nullptr;
    if (fDec < 0.0)
        fDec = 0.0;
    sal_uLong nIndex = mrContext.NFGetStandardFormat(
                                        SvNumFormatType::NUMBER,
                                        ScGlobal::eLnge);
    OUString sFormatString = mrContext.NFGenerateFormat(
                                           nIndex,
                                           ScGlobal::eLnge,
                                           bThousand,   // with thousands separator
                                           false,       // not red
                                           static_cast<sal_uInt16>(fDec));// decimal places
    if (!mrContext.NFGetPreviewString(sFormatString,
                                              fVal,
                                              aStr,
                                              &pColor,
                                              ScGlobal::eLnge))
        PushIllegalArgument();
    else
        PushString(aStr);
}

void ScInterpreter::ScFind()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return;

    sal_Int32 nCnt;
    if (nParamCount == 3)
        nCnt = GetDouble();
    else
        nCnt = 1;
    OUString sStr = GetString().getString();
    if (nCnt < 1 || nCnt > sStr.getLength())
        PushNoValue();
    else
    {
        sal_Int32 nPos = sStr.indexOf(GetString().getString(), nCnt - 1);
        if (nPos == -1)
            PushNoValue();
        else
        {
            sal_Int32 nIdx = 0;
            nCnt = 0;
            while ( nIdx < nPos )
            {
                sStr.iterateCodePoints( &nIdx );
                ++nCnt;
            }
            PushDouble( static_cast<double>(nCnt + 1) );
        }
    }
}

void ScInterpreter::ScExact()
{
    nFuncFmtType = SvNumFormatType::LOGICAL;
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        svl::SharedString s1 = GetString();
        svl::SharedString s2 = GetString();
        PushInt( int(s1.getData() == s2.getData()) );
    }
}

void ScInterpreter::ScLeft()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 2 ) )
        return;

    sal_Int32 n;
    if (nParamCount == 2)
    {
        n = GetStringPositionArgument();
        if (n < 0)
        {
            PushIllegalArgument();
            return ;
        }
    }
    else
        n = 1;
    OUString aStr = GetString().getString();
    sal_Int32 nIdx = 0;
    sal_Int32 nCnt = 0;
    while ( nIdx < aStr.getLength() && n > nCnt++ )
        aStr.iterateCodePoints( &nIdx );
    aStr = aStr.copy( 0, nIdx );
    PushString( aStr );
}

namespace {

struct UBlockScript {
    UBlockCode from;
    UBlockCode to;
};

}

const UBlockScript scriptList[] = {
    {UBLOCK_HANGUL_JAMO, UBLOCK_HANGUL_JAMO},
    {UBLOCK_CJK_RADICALS_SUPPLEMENT, UBLOCK_HANGUL_SYLLABLES},
    {UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS,UBLOCK_CJK_RADICALS_SUPPLEMENT },
    {UBLOCK_IDEOGRAPHIC_DESCRIPTION_CHARACTERS,UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS},
    {UBLOCK_CJK_COMPATIBILITY_FORMS, UBLOCK_CJK_COMPATIBILITY_FORMS},
    {UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS, UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS},
    {UBLOCK_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B, UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT},
    {UBLOCK_CJK_STROKES, UBLOCK_CJK_STROKES}
};
static bool IsDBCS(sal_Unicode currentChar)
{
    // for the locale of ja-JP, character U+0x005c and U+0x20ac should be ScriptType::Asian
    if( (currentChar == 0x005c || currentChar == 0x20ac) &&
          (MsLangId::getConfiguredSystemLanguage() == LANGUAGE_JAPANESE) )
        return true;
    sal_uInt16 i;
    bool bRet = false;
    UBlockCode block = ublock_getCode(currentChar);
    for ( i = 0; i < SAL_N_ELEMENTS(scriptList); i++) {
        if (block <= scriptList[i].to) break;
    }
    bRet = (i < SAL_N_ELEMENTS(scriptList) && block >= scriptList[i].from);
    return bRet;
}
static sal_Int32 lcl_getLengthB( std::u16string_view str, sal_Int32 nPos )
{
    sal_Int32 index = 0;
    sal_Int32 length = 0;
    while ( index < nPos )
    {
        if (IsDBCS(str[index]))
            length += 2;
        else
            length++;
        index++;
    }
    return length;
}
static sal_Int32 getLengthB(std::u16string_view str)
{
    if(str.empty())
        return 0;
    else
        return lcl_getLengthB( str, str.size() );
}
void ScInterpreter::ScLenB()
{
    PushDouble( getLengthB(GetString().getString()) );
}
static OUString lcl_RightB(const OUString &rStr, sal_Int32 n)
{
    if( n < getLengthB(rStr) )
    {
        OUStringBuffer aBuf(rStr);
        sal_Int32 index = aBuf.getLength();
        while(index-- >= 0)
        {
            if(0 == n)
            {
                aBuf.remove( 0, index + 1);
                break;
            }
            if(-1 == n)
            {
                aBuf.remove( 0, index + 2 );
                aBuf.insert( 0, " ");
                break;
            }
            if(IsDBCS(aBuf[index]))
                n -= 2;
            else
                n--;
        }
        return aBuf.makeStringAndClear();
    }
    return rStr;
}
void ScInterpreter::ScRightB()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 2 ) )
        return;

    sal_Int32 n;
    if (nParamCount == 2)
    {
        n = GetStringPositionArgument();
        if (n < 0)
        {
            PushIllegalArgument();
            return ;
        }
    }
    else
        n = 1;
    OUString aStr(lcl_RightB(GetString().getString(), n));
    PushString( aStr );
}
static OUString lcl_LeftB(const OUString &rStr, sal_Int32 n)
{
    if( n < getLengthB(rStr) )
    {
        OUStringBuffer aBuf(rStr);
        sal_Int32 index = -1;
        while(index++ < aBuf.getLength())
        {
            if(0 == n)
            {
                aBuf.truncate(index);
                break;
            }
            if(-1 == n)
            {
                aBuf.truncate( index - 1 );
                aBuf.append(" ");
                break;
            }
            if(IsDBCS(aBuf[index]))
                n -= 2;
            else
                n--;
        }
        return aBuf.makeStringAndClear();
    }
    return rStr;
}
void ScInterpreter::ScLeftB()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 2 ) )
        return;

    sal_Int32 n;
    if (nParamCount == 2)
    {
        n = GetStringPositionArgument();
        if (n < 0)
        {
            PushIllegalArgument();
            return ;
        }
    }
    else
        n = 1;
    OUString aStr(lcl_LeftB(GetString().getString(), n));
    PushString( aStr );
}
void ScInterpreter::ScMidB()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;

    const sal_Int32 nCount = GetStringPositionArgument();
    const sal_Int32 nStart = GetStringPositionArgument();
    OUString aStr = GetString().getString();
    if (nStart < 1 || nCount < 0)
        PushIllegalArgument();
    else
    {

        aStr = lcl_LeftB(aStr, nStart + nCount - 1);
        sal_Int32 nCnt = getLengthB(aStr) - nStart + 1;
        aStr = lcl_RightB(aStr, std::max<sal_Int32>(nCnt,0));
        PushString(aStr);
    }
}

void ScInterpreter::ScReplaceB()
{
    if ( !MustHaveParamCount( GetByte(), 4 ) )
        return;

    OUString aNewStr       = GetString().getString();
    const sal_Int32 nCount = GetStringPositionArgument();
    const sal_Int32 nPos   = GetStringPositionArgument();
    OUString aOldStr       = GetString().getString();
    int nLen               = getLengthB( aOldStr );
    if (nPos < 1.0 || nPos > nLen || nCount < 0.0 || nPos + nCount -1 > nLen)
        PushIllegalArgument();
    else
    {
        // REPLACEB(aOldStr;nPos;nCount;aNewStr) is the same as
        // LEFTB(aOldStr;nPos-1) & aNewStr & RIGHT(aOldStr;LENB(aOldStr)-(nPos - 1)-nCount)
        OUString aStr1 = lcl_LeftB( aOldStr, nPos - 1 );
        OUString aStr3 = lcl_RightB( aOldStr, nLen - nPos - nCount + 1);

        PushString( aStr1 + aNewStr + aStr3 );
    }
}

void ScInterpreter::ScFindB()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return;

    sal_Int32 nStart;
    if ( nParamCount == 3 )
        nStart = GetStringPositionArgument();
    else
        nStart = 1;
    OUString aStr  = GetString().getString();
    int nLen       = getLengthB( aStr );
    OUString asStr = GetString().getString();
    int nsLen      = getLengthB( asStr );
    if ( nStart < 1 || nStart > nLen - nsLen + 1 )
        PushIllegalArgument();
    else
    {
        // create a string from sStr starting at nStart
        OUString aBuf = lcl_RightB( aStr, nLen - nStart + 1 );
        // search aBuf for asStr
        sal_Int32 nPos = aBuf.indexOf( asStr, 0 );
        if ( nPos == -1 )
            PushNoValue();
        else
        {
            // obtain byte value of nPos
            int nBytePos = lcl_getLengthB( aBuf, nPos );
            PushDouble( nBytePos + nStart );
        }
    }
}

void ScInterpreter::ScSearchB()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return;

    sal_Int32 nStart;
    if ( nParamCount == 3 )
    {
        nStart = GetStringPositionArgument();
        if( nStart < 1 )
        {
            PushIllegalArgument();
            return;
        }
    }
    else
        nStart = 1;
    OUString aStr = GetString().getString();
    sal_Int32 nLen = getLengthB( aStr );
    OUString asStr = GetString().getString();
    sal_Int32 nsLen = nStart - 1;
    if( nsLen >= nLen )
        PushNoValue();
    else
    {
        // create a string from sStr starting at nStart
        OUString aSubStr( lcl_RightB( aStr, nLen - nStart + 1 ) );
        // search aSubStr for asStr
        sal_Int32 nPos = 0;
        sal_Int32 nEndPos = aSubStr.getLength();
        utl::SearchParam::SearchType eSearchType = DetectSearchType( asStr, mrDoc );
        utl::SearchParam sPar( asStr, eSearchType, false, '~', false );
        utl::TextSearch sT( sPar, ScGlobal::getCharClass() );
        if ( !sT.SearchForward( aSubStr, &nPos, &nEndPos ) )
            PushNoValue();
        else
        {
            // obtain byte value of nPos
            int nBytePos = lcl_getLengthB( aSubStr, nPos );
            PushDouble( nBytePos + nStart );
        }
    }
}

void ScInterpreter::ScRight()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 2 ) )
        return;

    sal_Int32 n;
    if (nParamCount == 2)
    {
        n = GetStringPositionArgument();
        if (n < 0)
        {
            PushIllegalArgument();
            return ;
        }
    }
    else
        n = 1;
    OUString aStr = GetString().getString();
    sal_Int32 nLen = aStr.getLength();
    if ( nLen <= n )
        PushString( aStr );
    else
    {
        sal_Int32 nIdx = nLen;
        sal_Int32 nCnt = 0;
        while ( nIdx > 0 && n > nCnt )
        {
            aStr.iterateCodePoints( &nIdx, -1 );
            ++nCnt;
        }
        aStr = aStr.copy( nIdx, nLen - nIdx );
        PushString( aStr );
    }
}

void ScInterpreter::ScSearch()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return;

    sal_Int32 nStart;
    if (nParamCount == 3)
    {
        nStart = GetStringPositionArgument();
        if( nStart < 1 )
        {
            PushIllegalArgument();
            return;
        }
    }
    else
        nStart = 1;
    OUString sStr = GetString().getString();
    OUString SearchStr = GetString().getString();
    sal_Int32 nPos = nStart - 1;
    sal_Int32 nEndPos = sStr.getLength();
    if( nPos >= nEndPos )
        PushNoValue();
    else
    {
        utl::SearchParam::SearchType eSearchType = DetectSearchType( SearchStr, mrDoc );
        utl::SearchParam sPar(SearchStr, eSearchType, false, '~', false);
        utl::TextSearch sT( sPar, ScGlobal::getCharClass() );
        bool bBool = sT.SearchForward(sStr, &nPos, &nEndPos);
        if (!bBool)
            PushNoValue();
        else
        {
            sal_Int32 nIdx = 0;
            sal_Int32 nCnt = 0;
            while ( nIdx < nPos )
            {
                sStr.iterateCodePoints( &nIdx );
                ++nCnt;
            }
            PushDouble( static_cast<double>(nCnt + 1) );
        }
    }
}

void ScInterpreter::ScRegex()
{
    const sal_uInt8 nParamCount = GetByte();
    if (!MustHaveParamCount( nParamCount, 2, 4))
        return;

    // Flags are supported only for replacement, search match flags can be
    // individually and much more flexible set in the regular expression
    // pattern using (?ismwx-ismwx)
    bool bGlobalReplacement = false;
    sal_Int32 nOccurrence = 1;  // default first occurrence, if any
    if (nParamCount == 4)
    {
        // Argument can be either string or double.
        double fOccurrence;
        svl::SharedString aFlagsString;
        bool bDouble;
        if (!IsMissing())
            bDouble = GetDoubleOrString( fOccurrence, aFlagsString);
        else
        {
            // For an omitted argument keep the default.
            PopError();
            bDouble = true;
            fOccurrence = nOccurrence;
        }
        if (nGlobalError != FormulaError::NONE)
        {
            PushError( nGlobalError);
            return;
        }
        if (bDouble)
        {
            if (!CheckStringPositionArgument( fOccurrence))
            {
                PushError( FormulaError::IllegalArgument);
                return;
            }
            nOccurrence = static_cast<sal_Int32>(fOccurrence);
        }
        else
        {
            const OUString aFlags( aFlagsString.getString());
            // Empty flags string is valid => no flag set.
            if (aFlags.getLength() > 1)
            {
                // Only one flag supported.
                PushIllegalArgument();
                return;
            }
            if (aFlags.getLength() == 1)
            {
                if (aFlags.indexOf('g') >= 0)
                    bGlobalReplacement = true;
                else
                {
                    // Unsupported flag.
                    PushIllegalArgument();
                    return;
                }
            }
        }
    }

    bool bReplacement = false;
    OUString aReplacement;
    if (nParamCount >= 3)
    {
        // A missing argument is not an empty string to replace the match.
        // nOccurrence==0 forces no replacement, so simply discard the
        // argument.
        if (IsMissing() || nOccurrence == 0)
            PopError();
        else
        {
            aReplacement = GetString().getString();
            bReplacement = true;
        }
    }
    // If bGlobalReplacement==true and bReplacement==false then
    // bGlobalReplacement is silently ignored.

    const OUString aExpression = GetString().getString();
    const OUString aText = GetString().getString();

    if (nGlobalError != FormulaError::NONE)
    {
        PushError( nGlobalError);
        return;
    }

    // 0-th match or replacement is none, return original string early.
    if (nOccurrence == 0)
    {
        PushString( aText);
        return;
    }

    const icu::UnicodeString aIcuExpression(
        false, reinterpret_cast<const UChar*>(aExpression.getStr()), aExpression.getLength());
    UErrorCode status = U_ZERO_ERROR;
    icu::RegexMatcher aRegexMatcher( aIcuExpression, 0, status);
    if (U_FAILURE(status))
    {
        // Invalid regex.
        PushIllegalArgument();
        return;
    }
    // Guard against pathological patterns, limit steps of engine, see
    // https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1RegexMatcher.html#a6ebcfcab4fe6a38678c0291643a03a00
    aRegexMatcher.setTimeLimit( 23*1000, status);

    const icu::UnicodeString aIcuText(false, reinterpret_cast<const UChar*>(aText.getStr()), aText.getLength());
    aRegexMatcher.reset( aIcuText);

    if (!bReplacement)
    {
        // Find n-th occurrence.
        sal_Int32 nCount = 0;
        while (aRegexMatcher.find(status) && U_SUCCESS(status) && ++nCount < nOccurrence)
            ;
        if (U_FAILURE(status))
        {
            // Some error.
            PushIllegalArgument();
            return;
        }
        // n-th match found?
        if (nCount != nOccurrence)
        {
            PushError( FormulaError::NotAvailable);
            return;
        }
        // Extract matched text.
        icu::UnicodeString aMatch( aRegexMatcher.group( status));
        if (U_FAILURE(status))
        {
            // Some error.
            PushIllegalArgument();
            return;
        }
        OUString aResult( reinterpret_cast<const sal_Unicode*>(aMatch.getBuffer()), aMatch.length());
        PushString( aResult);
        return;
    }

    const icu::UnicodeString aIcuReplacement(
        false, reinterpret_cast<const UChar*>(aReplacement.getStr()), aReplacement.getLength());
    icu::UnicodeString aReplaced;
    if (bGlobalReplacement)
        // Replace all occurrences of match with replacement.
        aReplaced = aRegexMatcher.replaceAll( aIcuReplacement, status);
    else if (nOccurrence == 1)
        // Replace first occurrence of match with replacement.
        aReplaced = aRegexMatcher.replaceFirst( aIcuReplacement, status);
    else
    {
        // Replace n-th occurrence of match with replacement.
        sal_Int32 nCount = 0;
        while (aRegexMatcher.find(status) && U_SUCCESS(status))
        {
            // XXX NOTE: After several RegexMatcher::find() the
            // RegexMatcher::appendReplacement() still starts at the
            // beginning (or after the last appendReplacement() position
            // which is none here) and copies the original text up to the
            // current found match and then replaces the found match.
            if (++nCount == nOccurrence)
            {
                aRegexMatcher.appendReplacement( aReplaced, aIcuReplacement, status);
                break;
            }
        }
        aRegexMatcher.appendTail( aReplaced);
    }
    if (U_FAILURE(status))
    {
        // Some error, e.g. extraneous $1 without group.
        PushIllegalArgument();
        return;
    }
    OUString aResult( reinterpret_cast<const sal_Unicode*>(aReplaced.getBuffer()), aReplaced.length());
    PushString( aResult);
}

void ScInterpreter::ScMid()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;

    const sal_Int32 nSubLen = GetStringPositionArgument();
    const sal_Int32 nStart  = GetStringPositionArgument();
    OUString aStr = GetString().getString();
    if ( nStart < 1 || nSubLen < 0 )
        PushIllegalArgument();
    else if (nStart > kScInterpreterMaxStrLen || nSubLen > kScInterpreterMaxStrLen)
        PushError(FormulaError::StringOverflow);
    else
    {
        sal_Int32 nLen = aStr.getLength();
        sal_Int32 nIdx = 0;
        sal_Int32 nCnt = 0;
        while ( nIdx < nLen && nStart - 1 > nCnt )
        {
            aStr.iterateCodePoints( &nIdx );
            ++nCnt;
        }
        sal_Int32 nIdx0 = nIdx;  //start position

        while ( nIdx < nLen && nStart + nSubLen - 1 > nCnt )
        {
            aStr.iterateCodePoints( &nIdx );
            ++nCnt;
        }
        aStr = aStr.copy( nIdx0, nIdx - nIdx0 );
        PushString( aStr );
    }
}

void ScInterpreter::ScText()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    OUString sFormatString = GetString().getString();
    svl::SharedString aStr;
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
                FormulaConstTokenRef xTok( PopToken());
                if (nGlobalError == FormulaError::NONE)
                {
                    PushTokenRef( xTok);
                    // Temporarily override the ConvertStringToValue()
                    // error for GetCellValue() / GetCellValueOrZero()
                    FormulaError nSErr = mnStringNoValueError;
                    mnStringNoValueError = FormulaError::NotNumericString;
                    fVal = GetDouble();
                    mnStringNoValueError = nSErr;
                    if (nGlobalError == FormulaError::NotNumericString)
                    {
                        // Not numeric.
                        nGlobalError = FormulaError::NONE;
                        PushTokenRef( xTok);
                        aStr = GetString();
                        bString = true;
                    }
                }
            }
    }
    if (nGlobalError != FormulaError::NONE)
        PushError( nGlobalError);
    else if (sFormatString.isEmpty())
    {
        // Mimic the Excel behaviour that
        // * anything numeric returns an empty string
        // * text convertible to numeric returns an empty string
        // * any other text returns that text
        // Conversion was detected above.
        if (bString)
            PushString( aStr);
        else
            PushString( OUString());
    }
    else
    {
        OUString aResult;
        const Color* pColor = nullptr;
        LanguageType eCellLang;
        const ScPatternAttr* pPattern = mrDoc.GetPattern(
                aPos.Col(), aPos.Row(), aPos.Tab() );
        if ( pPattern )
            eCellLang = pPattern->GetItem( ATTR_LANGUAGE_FORMAT ).GetValue();
        else
            eCellLang = ScGlobal::eLnge;
        if (bString)
        {
            if (!mrContext.NFGetPreviewString( sFormatString, aStr.getString(),
                        aResult, &pColor, eCellLang))
                PushIllegalArgument();
            else
                PushString( aResult);
        }
        else
        {
            if (!mrContext.NFGetPreviewStringGuess( sFormatString, fVal,
                        aResult, &pColor, eCellLang))
                PushIllegalArgument();
            else
                PushString( aResult);
        }
    }
}

void ScInterpreter::ScSubstitute()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 4 ) )
        return;

    sal_Int32 nCnt;
    if (nParamCount == 4)
    {
        nCnt = GetStringPositionArgument();
        if (nCnt < 1)
        {
            PushIllegalArgument();
            return;
        }
    }
    else
        nCnt = 0;
    OUString sNewStr = GetString().getString();
    OUString sOldStr = GetString().getString();
    OUString sStr    = GetString().getString();
    sal_Int32 nPos = 0;
    sal_Int32 nCount = 0;
    std::optional<OUStringBuffer> oResult;
    for (sal_Int32 nEnd = sStr.indexOf(sOldStr); nEnd >= 0; nEnd = sStr.indexOf(sOldStr, nEnd))
    {
        if (nCnt == 0 || ++nCount == nCnt) // Found a replacement cite
        {
            if (!oResult) // Only allocate buffer when needed
                oResult.emplace(sStr.getLength() + sNewStr.getLength() - sOldStr.getLength());

            oResult->append(sStr.subView(nPos, nEnd - nPos)); // Copy leading unchanged text
            if (!CheckStringResultLen(*oResult, sNewStr.getLength()))
                return PushError(GetError());
            oResult->append(sNewStr); // Copy  the replacement
            nPos = nEnd + sOldStr.getLength();
            if (nCnt > 0) // Found the single replacement site - end the loop
                break;
        }
        nEnd += sOldStr.getLength();
    }
    if (oResult) // If there were prior replacements, copy the rest, otherwise use original
        oResult->append(sStr.subView(nPos, sStr.getLength() - nPos));
    PushString(oResult ? oResult->makeStringAndClear() : sStr);
}

void ScInterpreter::ScRept()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    sal_Int32 nCnt = GetStringPositionArgument();
    OUString aStr = GetString().getString();
    if (nCnt < 0)
        PushIllegalArgument();
    else if (static_cast<double>(nCnt) * aStr.getLength() > kScInterpreterMaxStrLen)
    {
        PushError( FormulaError::StringOverflow );
    }
    else if (nCnt == 0)
        PushString( OUString() );
    else
    {
        const sal_Int32 nLen = aStr.getLength();
        OUStringBuffer aRes(nCnt*nLen);
        while( nCnt-- )
            aRes.append(aStr);
        PushString( aRes.makeStringAndClear() );
    }
}

void ScInterpreter::ScConcat()
{
    sal_uInt8 nParamCount = GetByte();

    //reverse order of parameter stack to simplify processing
    ReverseStack(nParamCount);

    OUStringBuffer aRes;
    while( nParamCount-- > 0)
    {
        OUString aStr = GetString().getString();
        if (CheckStringResultLen(aRes, aStr.getLength()))
            aRes.append(aStr);
        else
            break;
    }
    PushString( aRes.makeStringAndClear() );
}

FormulaError ScInterpreter::GetErrorType()
{
    FormulaError nErr;
    FormulaError nOldError = nGlobalError;
    nGlobalError = FormulaError::NONE;
    switch ( GetStackType() )
    {
        case svRefList :
        {
            FormulaConstTokenRef x = PopToken();
            if (nGlobalError != FormulaError::NONE)
                nErr = nGlobalError;
            else
            {
                const ScRefList* pRefList = x->GetRefList();
                size_t n = pRefList->size();
                if (!n)
                    nErr = FormulaError::NoRef;
                else if (n > 1)
                    nErr = FormulaError::NoValue;
                else
                {
                    ScRange aRange;
                    DoubleRefToRange( (*pRefList)[0], aRange);
                    if (nGlobalError != FormulaError::NONE)
                        nErr = nGlobalError;
                    else
                    {
                        ScAddress aAdr;
                        if ( DoubleRefToPosSingleRef( aRange, aAdr ) )
                            nErr = mrDoc.GetErrCode( aAdr );
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
            if ( nGlobalError != FormulaError::NONE )
                nErr = nGlobalError;
            else
            {
                ScAddress aAdr;
                if ( DoubleRefToPosSingleRef( aRange, aAdr ) )
                    nErr = mrDoc.GetErrCode( aAdr );
                else
                    nErr = nGlobalError;
            }
        }
        break;
        case svSingleRef :
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            if ( nGlobalError != FormulaError::NONE )
                nErr = nGlobalError;
            else
                nErr = mrDoc.GetErrCode( aAdr );
        }
        break;
        default:
            PopError();
            nErr = nGlobalError;
    }
    nGlobalError = nOldError;
    return nErr;
}

void ScInterpreter::ScErrorType()
{
    FormulaError nErr = GetErrorType();
    if ( nErr != FormulaError::NONE )
    {
        nGlobalError = FormulaError::NONE;
        PushDouble( static_cast<double>(nErr) );
    }
    else
    {
        PushNA();
    }
}

void ScInterpreter::ScErrorType_ODF()
{
    FormulaError nErr = GetErrorType();
    sal_uInt16 nErrType;

    switch ( nErr )
    {
        case FormulaError::NoCode :             // #NULL!
            nErrType = 1;
            break;
        case FormulaError::DivisionByZero :     // #DIV/0!
            nErrType = 2;
            break;
        case FormulaError::NoValue :            // #VALUE!
            nErrType = 3;
            break;
        case FormulaError::NoRef :              // #REF!
            nErrType = 4;
            break;
        case FormulaError::NoName :             // #NAME?
            nErrType = 5;
            break;
        case FormulaError::IllegalFPOperation : // #NUM!
            nErrType = 6;
            break;
        case FormulaError::NotAvailable :       // #N/A
            nErrType = 7;
            break;
        /*
        #GETTING_DATA is a message that can appear in Excel when a large or
        complex worksheet is being calculated. In Excel 2007 and newer,
        operations are grouped so more complicated cells may finish after
        earlier ones do. While the calculations are still processing, the
        unfinished cells may display #GETTING_DATA.
        Because the message is temporary and disappears when the calculations
        complete, this isn’t a true error.
        No calc error code known (yet).

        case :                       // GETTING_DATA
            nErrType = 8;
            break;
        */
        default :
            nErrType = 0;
            break;
    }

    if ( nErrType )
    {
        nGlobalError =FormulaError::NONE;
        PushDouble( nErrType );
    }
    else
        PushNA();
}

static bool MayBeRegExp( std::u16string_view rStr )
{
    if ( rStr.empty() || (rStr.size() == 1 && rStr[0] != '.') )
        return false;   // single meta characters can not be a regexp
    // First two characters are wildcard '?' and '*' characters.
    std::u16string_view cre(u"?*+.[]^$\\<>()|");
    return rStr.find_first_of(cre) != std::u16string_view::npos;
}

static bool MayBeWildcard( std::u16string_view rStr )
{
    // Wildcards with '~' escape, if there are no wildcards then an escaped
    // character does not make sense, but it modifies the search pattern in an
    // Excel compatible wildcard search...
    std::u16string_view cw(u"*?~");
    return rStr.find_first_of(cw) != std::u16string_view::npos;
}

utl::SearchParam::SearchType ScInterpreter::DetectSearchType( std::u16string_view rStr, const ScDocument& rDoc )
{
    const auto eType = rDoc.GetDocOptions().GetFormulaSearchType();
    if ((eType == utl::SearchParam::SearchType::Wildcard && MayBeWildcard(rStr))
        || (eType == utl::SearchParam::SearchType::Regexp && MayBeRegExp(rStr)))
        return eType;
    return utl::SearchParam::SearchType::Normal;
}

bool ScInterpreter::SearchMatrixForValue( VectorSearchArguments& vsa, ScQueryParam& rParam, ScQueryEntry& rEntry, ScQueryEntry::Item& rItem )
{
    SCSIZE nC, nR;
    vsa.pMatSrc->GetDimensions( nC, nR);
    if (nC > 1 && nR > 1)
    {
        // The source matrix must be a vector.
        PushIllegalParameter();
        return false;
    }
    vsa.bVLookup = ( nC == 1 );

    // Do not propagate errors from matrix while searching.
    vsa.pMatSrc->SetErrorInterpreter( nullptr );

    SCSIZE nMatCount = (vsa.bVLookup ? nR : nC);
    VectorMatrixAccessor aMatAcc(*(vsa.pMatSrc), vsa.bVLookup);
    bool bMatchWholeCell = mrDoc.GetDocOptions().IsMatchWholeCell();

    switch ( vsa.eSearchMode )
    {
        case searchfwd :
        {
            switch ( vsa.eMatchMode )
            {
                case exactorNA :
                case wildcard :
                    // simple serial search for equality mode (source data doesn't
                    // need to be sorted).
                    for (SCSIZE i = 0; i < nMatCount; ++i)
                    {
                        if (lcl_CompareMatrix2Query( i, aMatAcc, rParam, rEntry, bMatchWholeCell ) == 0)
                        {
                            vsa.nHitIndex = i+1; // found !
                            break;
                        }
                    }
                    break;

                case exactorS :
                case exactorG :
                    for (SCSIZE i = 0; i < nMatCount; ++i)
                    {
                        sal_Int32 result = lcl_CompareMatrix2Query( i, aMatAcc, rParam, rEntry, bMatchWholeCell );
                        if (result == 0)
                        {
                            vsa.nHitIndex = i+1; // found !
                            break;
                        }
                        else if (vsa.eMatchMode == exactorS && result == -1)
                        {
                            if ( vsa.nBestFit == SCSIZE_MAX )
                                vsa.nBestFit = i;
                            else
                            {
                                // replace value of vsa.nBestFit if value(i) > value(vsa.nBestFit)
                                if ( lcl_Compare2MatrixCells( i, aMatAcc, vsa.nBestFit) == 1 )
                                    vsa.nBestFit = i;
                            }
                        }
                        else if (vsa.eMatchMode == exactorG && result == 1)
                        {
                            if ( vsa.nBestFit == SCSIZE_MAX )
                                vsa.nBestFit = i;
                            else
                            {
                                // replace value of vsa.nBestFit if value(i) < value(vsa.nBestFit)
                                if ( lcl_Compare2MatrixCells( i, aMatAcc, vsa.nBestFit) == -1 )
                                    vsa.nBestFit = i;
                            }
                        }
                        // else do nothing
                    }
                    break;

                default :
                    PushIllegalParameter();
                    return false;
            }
        }
        break;

        case searchrev:
            {
                switch ( vsa.eMatchMode )
                {
                    case exactorNA :
                    case wildcard :
                        // simple serial search for equality mode (source data doesn't
                        // need to be sorted).
                        for ( SCSIZE i = nMatCount - 1; i > 0; i-- )
                        {
                            if (lcl_CompareMatrix2Query(i, aMatAcc, rParam, rEntry, bMatchWholeCell) == 0)
                            {
                                vsa.nHitIndex = i + 1; // found !
                                break;
                            }
                        }
                        break;

                    case exactorS :
                    case exactorG :
                        for (SCSIZE i = nMatCount - 1; i-- > 0; )
                        {
                            sal_Int32 result = lcl_CompareMatrix2Query( i, aMatAcc, rParam, rEntry, bMatchWholeCell );
                            if (result == 0)
                            {
                                vsa.nHitIndex = i + 1; // found !
                                break;
                            }
                            else if (vsa.eMatchMode == exactorS && result == -1)
                            {
                                if ( vsa.nBestFit == SCSIZE_MAX )
                                    vsa.nBestFit = i;
                                else
                                {
                                    // replace value of vsa.nBestFit if value(i) > value(vsa.nBestFit)
                                    if ( lcl_Compare2MatrixCells( i, aMatAcc, vsa.nBestFit) == 1 )
                                        vsa.nBestFit = i;
                                }
                            }
                            else if (vsa.eMatchMode == exactorG && result == 1)
                            {
                                if ( vsa.nBestFit == SCSIZE_MAX )
                                    vsa.nBestFit = i;
                                else
                                {
                                    // replace value of vsa.nBestFit if value(i) < value(vsa.nBestFit)
                                    if ( lcl_Compare2MatrixCells( i, aMatAcc, vsa.nBestFit) == -1 )
                                        vsa.nBestFit = i;
                                }
                            }
                            // else do nothing
                        }
                        break;

                    default :
                        PushIllegalParameter();
                        return false;
                }
            }
            break;

        case searchbasc:
        case searchbdesc:
            {
                // binary search for non-equality mode (the source data is sorted)
                bool bAscOrder = ( vsa.eSearchMode == searchbasc );
                SCSIZE nFirst = 0;
                SCSIZE nLast  = nMatCount - 1;
                for ( SCSIZE nLen = nLast - nFirst; nLen > 0; nLen = nLast - nFirst )
                {
                    SCSIZE nMid = nFirst + nLen / 2;
                    sal_Int32 nCmp = lcl_CompareMatrix2Query( nMid, aMatAcc, rParam, rEntry, bMatchWholeCell );
                    if ( nCmp == 0 )
                    {
                        // exact match.  find the last item with the same value.
                        lcl_GetLastMatch( nMid, aMatAcc, nMatCount);
                        vsa.nHitIndex = nMid + 1;
                        break;
                    }

                    if ( nLen == 1 ) // first and last items are next to each other.
                    {
                        if ( bAscOrder && vsa.eMatchMode == exactorS )
                            vsa.nHitIndex = ( nCmp > 0 ? nFirst : nLast );
                        else if ( !bAscOrder && vsa.eMatchMode == exactorG )
                            vsa.nHitIndex = ( nCmp < 0 ? nFirst : nLast );
                        break;
                    }
                    else
                    {
                        if ( nCmp < 0 )
                        {
                            if ( bAscOrder )
                                nFirst = nMid;
                            else
                                nLast = nMid;
                        }
                        else
                        {
                            if ( bAscOrder )
                                nLast = nMid;
                            else
                                nFirst = nMid;
                        }

                        if ( vsa.nHitIndex == nMatCount - 1 ) // last item
                        {
                            nCmp = lcl_CompareMatrix2Query( vsa.nHitIndex, aMatAcc, rParam, rEntry, bMatchWholeCell );
                            if ( ( vsa.eMatchMode == exactorS && nCmp <= 0 ) ||
                                 ( vsa.eMatchMode == exactorG && nCmp >= 0 ) )
                            {
                                // either the last item is an exact match or the real
                                // hit is beyond the last item.
                                vsa.nHitIndex++;
                            }
                            else
                                vsa.nHitIndex = 0;
                        }
                    }
                }
            }
            break;

        default:
            PushIllegalParameter();
            return false;
    }

    if ((vsa.nHitIndex > 0) && ((rItem.meType == ScQueryEntry::ByString && aMatAcc.IsValue(vsa.nHitIndex - 1)) ||
        (rItem.meType == ScQueryEntry::ByValue && !aMatAcc.IsValue(vsa.nHitIndex - 1))))
    {
        vsa.nHitIndex = 0;
        vsa.isResultNA = true;
        return false;
    }
    return true;
}

bool ScInterpreter::SearchRangeForValue( VectorSearchArguments& vsa, ScQueryParam& rParam, ScQueryEntry& rEntry )
{
    vsa.bVLookup = ( vsa.nCol1 == vsa.nCol2 );
    switch ( vsa.eSearchMode )
    {
        case searchfwd:
        case searchrev:
        case searchbasc:
        case searchbdesc:
            {
                if (vsa.bVLookup)
                {
                    // search of rows in column
                    rParam.bByRow = true;
                    ScAddress aResultPos( vsa.nCol1, vsa.nRow1, vsa.nTab1 );
                    const ScComplexRefData* refData = nullptr;
                    if ( LookupQueryWithCache( aResultPos, rParam, refData, vsa.eSearchMode, vsa.nSearchOpCode ) )
                        vsa.nHitIndex = aResultPos.Row() - vsa.nRow1 + 1;
                }
                else
                {
                    rParam.bByRow = false;
                    bool bBinarySearch = vsa.eSearchMode == searchbasc || vsa.eSearchMode == searchbdesc;
                    if (bBinarySearch && (vsa.nSearchOpCode == SC_OPCODE_X_LOOKUP || vsa.nSearchOpCode == SC_OPCODE_X_MATCH))
                    {
                        ScQueryCellIteratorSortedCache aCellIter(mrDoc, mrContext, rParam.nTab, rParam, false, false);
                        // Advance Entry.nField in Iterator if column changed
                        aCellIter.SetAdvanceQueryParamEntryField(true);
                        aCellIter.SetSortedBinarySearchMode(vsa.eSearchMode);
                        aCellIter.SetLookupMode(vsa.nSearchOpCode);
                        if (aCellIter.GetFirst())
                        {
                            vsa.nHitIndex = aCellIter.GetCol() - vsa.nCol1 + 1;
                        }
                    }
                    else
                    {
                        // search of columns in row
                        bool bReverseSearch = (vsa.eSearchMode == searchrev);
                        ScQueryCellIteratorDirect aCellIter(mrDoc, mrContext, vsa.nTab1, rParam, false, bReverseSearch);
                        // Advance Entry.nField in Iterator if column changed
                        aCellIter.SetAdvanceQueryParamEntryField(true);
                        aCellIter.SetLookupMode(vsa.nSearchOpCode);
                        aCellIter.SetSortedBinarySearchMode(vsa.eSearchMode);
                        if (rEntry.eOp == SC_EQUAL)
                        {
                            if (aCellIter.GetFirst())
                                vsa.nHitIndex = aCellIter.GetCol() - vsa.nCol1 + 1;
                        }
                        else
                        {
                            SCCOL nC;
                            SCROW nR;
                            if (aCellIter.FindEqualOrSortedLastInRange(nC, nR))
                                vsa.nHitIndex = nC - vsa.nCol1 + 1;
                        }
                    }
                }
            }
            break;

        default :
            PushIllegalParameter();
            return false;
    }
    return true;
}


/** When search value is found, the index is stored in struct VectorSearchArguments.nIndex
    and SearchVectorForValue() returns true. When search value is not found or an error
    occurs, SearchVectorForValue() pushes the relevant (error)message and returns false,
    expect when SearchVectorForValue() is called by ScXLookup and the search value is not
    found.
    This difference in behaviour is because MATCH returns the found index and XLOOKUP
    uses the found index to determine the result(s) to be pushed and may return a custom
    value when the search value is not found.
*/
bool ScInterpreter::SearchVectorForValue( VectorSearchArguments& vsa )
{
    // preparations
    ScQueryParam rParam;
    rParam.nCol1 = vsa.nCol1;
    rParam.nRow1 = vsa.nRow1;
    rParam.nCol2 = vsa.nCol2;
    rParam.nRow2 = vsa.nRow2;
    rParam.nTab  = vsa.nTab1;

    ScQueryEntry& rEntry = rParam.GetEntry(0);
    rEntry.nField = vsa.eSearchMode != searchrev ? vsa.nCol1 : vsa.nCol2;
    rEntry.bDoQuery = true;
    switch ( vsa.eMatchMode )
    {
        case exactorNA :
            rEntry.eOp = SC_EQUAL;
            break;

        case exactorS :
            rEntry.eOp = SC_LESS_EQUAL;
            break;

        case exactorG :
            rEntry.eOp = SC_GREATER_EQUAL;
            break;

        case wildcard :
            // this mode can only used with XLOOKUP/XMATCH
            if ( vsa.nSearchOpCode == SC_OPCODE_X_LOOKUP || vsa.nSearchOpCode == SC_OPCODE_X_MATCH )
            {
                rEntry.eOp = SC_EQUAL;
                if ( vsa.isStringSearch )
                {
                    if ( mrDoc.IsInVBAMode() )
                        rParam.eSearchType = utl::SearchParam::SearchType::Wildcard;
                    else
                        rParam.eSearchType = DetectSearchType(vsa.sSearchStr.getString(), mrDoc);
                }
            }
            else
            {
                PushIllegalParameter();
                return false;
            }
            break;

        default :
            PushIllegalParameter();
            return false;
    }

    ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
    // allow to match empty cells as result if we are looking for the next smaller
    // or larger values in case of the new lookup functions
    if (rEntry.eOp != SC_EQUAL && (vsa.nSearchOpCode == SC_OPCODE_X_LOOKUP ||
        vsa.nSearchOpCode == SC_OPCODE_X_MATCH))
        rItem.mbMatchEmpty = true;

    if ( vsa.isStringSearch )
    {
        rItem.meType   = ScQueryEntry::ByString;
        rItem.maString = vsa.sSearchStr;
        if ( vsa.nSearchOpCode == SC_OPCODE_MATCH )
        {
            if ( mrDoc.IsInVBAMode() )
                rParam.eSearchType = utl::SearchParam::SearchType::Wildcard;
            else
                rParam.eSearchType = DetectSearchType(rEntry.GetQueryItem().maString.getString(), mrDoc);
        }
    }
    else if ( vsa.isEmptySearch && (vsa.nSearchOpCode == SC_OPCODE_X_LOOKUP ||
        vsa.nSearchOpCode == SC_OPCODE_X_MATCH) )
    {
        rEntry.SetQueryByEmpty();
        rItem.mbMatchEmpty = true;
    }
    else
    {
        rItem.mfVal = vsa.fSearchVal;
        rItem.meType = ScQueryEntry::ByValue;
    }

    // execute search
    if (vsa.pMatSrc) // The source data is matrix array.
    {
        // matrix
        if ( !SearchMatrixForValue( vsa, rParam, rEntry, rItem ) )
            return false;
    }
    else
    {
        // not a matrix
        if ( !SearchRangeForValue( vsa, rParam, rEntry ) )
            return false;
    }

    // MATCH expects index starting with 1, XLOOKUP expects index starting with 0
    if ( vsa.nHitIndex > 0 )
    {
        vsa.nIndex = ( vsa.nSearchOpCode == SC_OPCODE_X_LOOKUP ? --vsa.nHitIndex : vsa.nHitIndex );
        return true;
    }
    else  if ( vsa.nHitIndex == 0 && vsa.nBestFit != SCSIZE_MAX )
    {
        if ( vsa.nSearchOpCode == SC_OPCODE_X_LOOKUP )
        {
            vsa.nIndex = vsa.nBestFit;
            if ( !vsa.pMatSrc )
            {
                vsa.nIndex -= ( vsa.bVLookup ? vsa.nRow1 : vsa.nCol1 );
            }
        }
        else
        {
            vsa.nIndex = ++vsa.nBestFit;
        }
        return true;
    }

    // nomatch
    vsa.isResultNA = true;
    return false;
}

static bool lcl_LookupQuery( ScAddress & o_rResultPos, ScDocument& rDoc, ScInterpreterContext& rContext,
        const ScQueryParam & rParam, const ScQueryEntry & rEntry, const ScFormulaCell* cell,
        const ScComplexRefData* refData, sal_Int8 nSearchMode, sal_uInt16 nOpCode )
{
    if (rEntry.eOp != SC_EQUAL)
    {
        // range lookup <= or >=
        SCCOL nCol;
        SCROW nRow;
        bool bBinarySearch = static_cast<SearchMode>(nSearchMode) == searchbasc || static_cast<SearchMode>(nSearchMode) == searchbdesc;
        if ((bBinarySearch && (nOpCode == SC_OPCODE_X_LOOKUP || nOpCode == SC_OPCODE_X_MATCH)) ||
            ScQueryCellIteratorSortedCache::CanBeUsed(rDoc, rParam, rParam.nTab, cell, refData, rContext))
        {
            ScQueryCellIteratorSortedCache aCellIter(rDoc, rContext, rParam.nTab, rParam, false, false);
            aCellIter.SetSortedBinarySearchMode(nSearchMode);
            aCellIter.SetLookupMode(nOpCode);
            if (aCellIter.GetFirst())
            {
                o_rResultPos.SetCol(aCellIter.GetCol());
                o_rResultPos.SetRow(aCellIter.GetRow());
                return true;
            }
        }
        else
        {
            bool bReverse = (static_cast<SearchMode>(nSearchMode) == searchrev);
            ScQueryCellIteratorDirect aCellIter(rDoc, rContext, rParam.nTab, rParam, false, bReverse);

            aCellIter.SetSortedBinarySearchMode(nSearchMode);
            aCellIter.SetLookupMode(nOpCode);
            if (aCellIter.FindEqualOrSortedLastInRange(nCol, nRow))
            {
                o_rResultPos.SetCol(nCol);
                o_rResultPos.SetRow(nRow);
                return true;
            }
        }
    }
    else // EQUAL
    {
        // we can use binary search for rows if the SearchMode is searchbasc or searchbdesc
        bool bLiteral = rParam.eSearchType == utl::SearchParam::SearchType::Normal &&
            rEntry.GetQueryItem().meType == ScQueryEntry::ByString;
        bool bBinary = rParam.bByRow &&
            (bLiteral || rEntry.GetQueryItem().meType == ScQueryEntry::ByValue);

        if( bBinary && (static_cast<SearchMode>(nSearchMode) == searchbasc || static_cast<SearchMode>(nSearchMode) == searchbdesc ||
            ScQueryCellIteratorSortedCache::CanBeUsed(rDoc, rParam, rParam.nTab, cell, refData, rContext)))
        {
            ScQueryCellIteratorSortedCache aCellIter( rDoc, rContext, rParam.nTab, rParam, false, false );
            aCellIter.SetSortedBinarySearchMode(nSearchMode);
            aCellIter.SetLookupMode(nOpCode);
            if (aCellIter.GetFirst())
            {
                o_rResultPos.SetCol( aCellIter.GetCol());
                o_rResultPos.SetRow( aCellIter.GetRow());
                return true;
            }
        }
        else
        {
            ScQueryCellIteratorDirect aCellIter( rDoc, rContext, rParam.nTab, rParam, false,
                static_cast<SearchMode>(nSearchMode) == searchrev);
            aCellIter.SetSortedBinarySearchMode(nSearchMode);
            aCellIter.SetLookupMode(nOpCode);
            if (aCellIter.GetFirst())
            {
                o_rResultPos.SetCol( aCellIter.GetCol());
                o_rResultPos.SetRow( aCellIter.GetRow());
                return true;
            }
        }
    }
    return false;
}

// tdf#121052:
// =VLOOKUP(SearchCriterion; RangeArray; Index; Sorted)
//  [SearchCriterion] is the value searched for in the first column of the array.
//  [RangeArray] is the reference, which is to comprise at least two columns.
//  [Index] is the number of the column in the array that contains the value to be returned. The first column has the number 1.
//
// Prerequisite of lcl_getPrevRowWithEmptyValueLookup():
//      Value referenced by [SearchCriterion] is empty.
// lcl_getPrevRowWithEmptyValueLookup() performs following checks:
// - if we run query with "exact match" mode (i.e. VLOOKUP)
// - and if we already have the same lookup done before but for another row
//   which is also had empty [SearchCriterion]
//
// then
//   we could say, that for current row we could reuse results of the cached call which was done for the row2
//   In this case we return row index, which is >= 0.
//
// Elsewhere
//   -1 is returned, which will lead to default behavior =>
//   complete lookup will be done in RangeArray inside lcl_LookupQuery() method.
//
// This method was added only for speed up to avoid several useless complete
// lookups inside [RangeArray] for searching empty strings.
//
static SCROW lcl_getPrevRowWithEmptyValueLookup( const ScLookupCache& rCache,
        const ScLookupCache::QueryCriteria& rCriteria, const ScQueryParam & rParam)
{
    // is lookup value empty?
    const ScQueryEntry& rEntry = rParam.GetEntry(0);
    const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
    if (! rItem.maString.getString().isEmpty())
        return -1; // not found

    // try to find the row index for which we have already performed lookup
    // and have some result of it inside cache
    return rCache.lookup( rCriteria );
}

bool ScInterpreter::LookupQueryWithCache( ScAddress & o_rResultPos,
        const ScQueryParam & rParam, const ScComplexRefData* refData,
        sal_Int8 nSearchMode, sal_uInt16 nOpCode ) const
{
    bool bFound = false;
    const ScQueryEntry& rEntry = rParam.GetEntry(0);
    bool bColumnsMatch = (rParam.nCol1 == rEntry.nField);
    // At least all volatile functions that generate indirect references have
    // to force non-cached lookup.
    /* TODO: We could further classify volatile functions into reference
     * generating and not reference generating functions to have to force less
     * direct lookups here. We could even further attribute volatility per
     * parameter so it would affect only the lookup range parameter. */
    if (!bColumnsMatch || GetVolatileType() != NOT_VOLATILE)
        bFound = lcl_LookupQuery( o_rResultPos, mrDoc, mrContext, rParam, rEntry, pMyFormulaCell,
            refData, nSearchMode, nOpCode );
    else
    {
        ScRange aLookupRange( rParam.nCol1, rParam.nRow1, rParam.nTab,
                rParam.nCol2, rParam.nRow2, rParam.nTab);
        ScLookupCache& rCache = mrDoc.GetLookupCache( aLookupRange, &mrContext );
        ScLookupCache::QueryCriteria aCriteria( rEntry );
        ScLookupCache::Result eCacheResult = rCache.lookup( o_rResultPos,
                aCriteria, aPos);

        // tdf#121052: Slow load of cells with VLOOKUP with references to empty cells
        // This check was added only for speed up to avoid several useless complete
        // lookups inside [RangeArray] for searching empty strings.
        if (eCacheResult == ScLookupCache::NOT_CACHED && aCriteria.isEmptyStringQuery())
        {
            const SCROW nPrevRowWithEmptyValueLookup = lcl_getPrevRowWithEmptyValueLookup(rCache, aCriteria, rParam);
            if (nPrevRowWithEmptyValueLookup >= 0)
            {
                // make the same lookup using cache with different row index
                // (this lookup was already cached)
                ScAddress aPosPrev(aPos);
                aPosPrev.SetRow(nPrevRowWithEmptyValueLookup);

                eCacheResult = rCache.lookup( o_rResultPos, aCriteria, aPosPrev );
            }
        }

        switch (eCacheResult)
        {
            case ScLookupCache::NOT_CACHED :
            case ScLookupCache::CRITERIA_DIFFERENT :
                bFound = lcl_LookupQuery( o_rResultPos, mrDoc, mrContext, rParam, rEntry,
                    pMyFormulaCell, refData, nSearchMode, nOpCode );
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
