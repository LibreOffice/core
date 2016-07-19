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

#include <config_features.h>

#include "interpre.hxx"

#include <rangelst.hxx>
#include <rtl/math.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfilt.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbx.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbuno.hxx>
#include <svl/zforlist.hxx>
#include <svl/sharedstringpool.hxx>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <comphelper/processfactory.hxx>

#include "global.hxx"
#include "dbdata.hxx"
#include "formulacell.hxx"
#include "callform.hxx"
#include "addincol.hxx"
#include "document.hxx"
#include "dociter.hxx"
#include "docoptio.hxx"
#include "scmatrix.hxx"
#include "adiasync.hxx"
#include "sc.hrc"
#include "cellsuno.hxx"
#include "optuno.hxx"
#include "rangeseq.hxx"
#include "addinlis.hxx"
#include "jumpmatrix.hxx"
#include "parclass.hxx"
#include "externalrefmgr.hxx"
#include <formula/FormulaCompiler.hxx>
#include "macromgr.hxx"
#include "doubleref.hxx"
#include "queryparam.hxx"
#include "tokenarray.hxx"

#include <math.h>
#include <float.h>
#include <map>
#include <algorithm>
#include <functional>
#include <basic/basmgr.hxx>
#include <vbahelper/vbaaccesshelper.hxx>
#include <memory>
#include <stack>

using namespace com::sun::star;
using namespace formula;
using ::std::unique_ptr;

#define ADDIN_MAXSTRLEN 256

//-----------------------------static data -----------------

// document access functions

void ScInterpreter::ReplaceCell( ScAddress& rPos )
{
    size_t ListSize = pDok->m_TableOpList.size();
    for ( size_t i = 0; i < ListSize; ++i )
    {
        ScInterpreterTableOpParams *const pTOp = pDok->m_TableOpList[ i ].get();
        if ( rPos == pTOp->aOld1 )
        {
            rPos = pTOp->aNew1;
            return ;
        }
        else if ( rPos == pTOp->aOld2 )
        {
            rPos = pTOp->aNew2;
            return ;
        }
    }
}

void ScInterpreter::ReplaceCell( SCCOL& rCol, SCROW& rRow, SCTAB& rTab )
{
    ScAddress aCellPos( rCol, rRow, rTab );
    size_t ListSize = pDok->m_TableOpList.size();
    for ( size_t i = 0; i < ListSize; ++i )
    {
        ScInterpreterTableOpParams *const pTOp = pDok->m_TableOpList[ i ].get();
        if ( aCellPos == pTOp->aOld1 )
        {
            rCol = pTOp->aNew1.Col();
            rRow = pTOp->aNew1.Row();
            rTab = pTOp->aNew1.Tab();
            return ;
        }
        else if ( aCellPos == pTOp->aOld2 )
        {
            rCol = pTOp->aNew2.Col();
            rRow = pTOp->aNew2.Row();
            rTab = pTOp->aNew2.Tab();
            return ;
        }
    }
}

bool ScInterpreter::IsTableOpInRange( const ScRange& rRange )
{
    if ( rRange.aStart == rRange.aEnd )
        return false;   // not considered to be a range in TableOp sense

    // we can't replace a single cell in a range
    size_t ListSize = pDok->m_TableOpList.size();
    for ( size_t i = 0; i < ListSize; ++i )
    {
        ScInterpreterTableOpParams *const pTOp = pDok->m_TableOpList[ i ].get();
        if ( rRange.In( pTOp->aOld1 ) )
            return true;
        if ( rRange.In( pTOp->aOld2 ) )
            return true;
    }
    return false;
}

sal_uLong ScInterpreter::GetCellNumberFormat( const ScAddress& rPos, ScRefCellValue& rCell )
{
    sal_uLong nFormat;
    sal_uInt16 nErr;
    if (rCell.isEmpty())
    {
        nFormat = pDok->GetNumberFormat( rPos );
        nErr = 0;
    }
    else
    {
        if (rCell.meType == CELLTYPE_FORMULA)
            nErr = rCell.mpFormula->GetErrCode();
        else
            nErr = 0;
        nFormat = pDok->GetNumberFormat( rPos );
    }

    SetError(nErr);
    return nFormat;
}

/// Only ValueCell, formula cells already store the result rounded.
double ScInterpreter::GetValueCellValue( const ScAddress& rPos, double fOrig )
{
    if ( bCalcAsShown && fOrig != 0.0 )
    {
        sal_uLong nFormat = pDok->GetNumberFormat( rPos );
        fOrig = pDok->RoundValueAsShown( fOrig, nFormat );
    }
    return fOrig;
}

sal_uInt16 ScInterpreter::GetCellErrCode( const ScRefCellValue& rCell )
{
    return rCell.meType == CELLTYPE_FORMULA ? rCell.mpFormula->GetErrCode() : 0;
}

double ScInterpreter::ConvertStringToValue( const OUString& rStr )
{
    sal_uInt16 nError = 0;
    double fValue = ScGlobal::ConvertStringToValue( rStr, maCalcConfig, nError, mnStringNoValueError,
            pFormatter, nCurFmtType);
    if (nError)
        SetError(nError);
    return fValue;
}

double ScInterpreter::ConvertStringToValue( const OUString& rStr, sal_uInt16& rError, short& rCurFmtType )
{
    return ScGlobal::ConvertStringToValue( rStr, maCalcConfig, rError, mnStringNoValueError, pFormatter, rCurFmtType);
}

double ScInterpreter::GetCellValue( const ScAddress& rPos, ScRefCellValue& rCell )
{
    sal_uInt16 nErr = nGlobalError;
    nGlobalError = 0;
    double nVal = GetCellValueOrZero(rPos, rCell);
    if ( !nGlobalError || nGlobalError == errCellNoValue )
        nGlobalError = nErr;
    return nVal;
}

double ScInterpreter::GetCellValueOrZero( const ScAddress& rPos, ScRefCellValue& rCell )
{
    double fValue = 0.0;

    CellType eType = rCell.meType;
    switch (eType)
    {
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell* pFCell = rCell.mpFormula;
            sal_uInt16 nErr = pFCell->GetErrCode();
            if( !nErr )
            {
                if (pFCell->IsValue())
                {
                    fValue = pFCell->GetValue();
                    pDok->GetNumberFormatInfo( nCurFmtType, nCurFmtIndex,
                        rPos );
                }
                else
                {
                    fValue = ConvertStringToValue(pFCell->GetString().getString());
                }
            }
            else
            {
                fValue = 0.0;
                SetError(nErr);
            }
        }
        break;
        case CELLTYPE_VALUE:
        {
            fValue = rCell.mfValue;
            nCurFmtIndex = pDok->GetNumberFormat( rPos );
            nCurFmtType = pFormatter->GetType( nCurFmtIndex );
            if ( bCalcAsShown && fValue != 0.0 )
                fValue = pDok->RoundValueAsShown( fValue, nCurFmtIndex );
        }
        break;
        case  CELLTYPE_STRING:
        case  CELLTYPE_EDIT:
        {
            // SUM(A1:A2) differs from A1+A2. No good. But people insist on
            // it ... #i5658#
            OUString aStr = rCell.getString(pDok);
            fValue = ConvertStringToValue( aStr );
        }
        break;
        case CELLTYPE_NONE:
            fValue = 0.0;       // empty or broadcaster cell
        break;
    }

    return fValue;
}

void ScInterpreter::GetCellString( svl::SharedString& rStr, ScRefCellValue& rCell )
{
    sal_uInt16 nErr = 0;

    switch (rCell.meType)
    {
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            rStr = mrStrPool.intern(rCell.getString(pDok));
        break;
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell* pFCell = rCell.mpFormula;
            nErr = pFCell->GetErrCode();
            if (pFCell->IsValue())
            {
                double fVal = pFCell->GetValue();
                sal_uLong nIndex = pFormatter->GetStandardFormat(
                                    css::util::NumberFormat::NUMBER,
                                    ScGlobal::eLnge);
                OUString aStr;
                pFormatter->GetInputLineString(fVal, nIndex, aStr);
                rStr = mrStrPool.intern(aStr);
            }
            else
                rStr = pFCell->GetString();
        }
        break;
        case CELLTYPE_VALUE:
        {
            double fVal = rCell.mfValue;
            sal_uLong nIndex = pFormatter->GetStandardFormat(
                                    css::util::NumberFormat::NUMBER,
                                    ScGlobal::eLnge);
            OUString aStr;
            pFormatter->GetInputLineString(fVal, nIndex, aStr);
            rStr = mrStrPool.intern(aStr);
        }
        break;
        default:
            rStr = svl::SharedString::getEmptyString();
        break;
    }

    SetError(nErr);
}

bool ScInterpreter::CreateDoubleArr(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2, sal_uInt8* pCellArr)
{

    // Old Add-Ins are hard limited to sal_uInt16 values.
#if MAXCOLCOUNT_DEFINE > USHRT_MAX
#error Add check for columns > USHRT_MAX!
#endif
    if (nRow1 > USHRT_MAX || nRow2 > USHRT_MAX)
        return false;

    sal_uInt16 nCount = 0;
    sal_uInt16* p = reinterpret_cast<sal_uInt16*>(pCellArr);
    *p++ = static_cast<sal_uInt16>(nCol1);
    *p++ = static_cast<sal_uInt16>(nRow1);
    *p++ = static_cast<sal_uInt16>(nTab1);
    *p++ = static_cast<sal_uInt16>(nCol2);
    *p++ = static_cast<sal_uInt16>(nRow2);
    *p++ = static_cast<sal_uInt16>(nTab2);
    sal_uInt16* pCount = p;
    *p++ = 0;
    sal_uInt16 nPos = 14;
    SCTAB nTab = nTab1;
    ScAddress aAdr;
    while (nTab <= nTab2)
    {
        aAdr.SetTab( nTab );
        SCROW nRow = nRow1;
        while (nRow <= nRow2)
        {
            aAdr.SetRow( nRow );
            SCCOL nCol = nCol1;
            while (nCol <= nCol2)
            {
                aAdr.SetCol( nCol );

                ScRefCellValue aCell(*pDok, aAdr);
                if (!aCell.isEmpty())
                {
                    sal_uInt16  nErr = 0;
                    double  nVal = 0.0;
                    bool    bOk = true;
                    switch (aCell.meType)
                    {
                        case CELLTYPE_VALUE :
                            nVal = GetValueCellValue(aAdr, aCell.mfValue);
                            break;
                        case CELLTYPE_FORMULA :
                            if (aCell.mpFormula->IsValue())
                            {
                                nErr = aCell.mpFormula->GetErrCode();
                                nVal = aCell.mpFormula->GetValue();
                            }
                            else
                                bOk = false;
                            break;
                        default :
                            bOk = false;
                            break;
                    }
                    if (bOk)
                    {
                        if ((nPos + (4 * sizeof(sal_uInt16)) + sizeof(double)) > MAXARRSIZE)
                            return false;
                        *p++ = static_cast<sal_uInt16>(nCol);
                        *p++ = static_cast<sal_uInt16>(nRow);
                        *p++ = static_cast<sal_uInt16>(nTab);
                        *p++ = nErr;
                        memcpy( p, &nVal, sizeof(double));
                        nPos += 8 + sizeof(double);
                        p = reinterpret_cast<sal_uInt16*>( pCellArr + nPos );
                        nCount++;
                    }
                }
                nCol++;
            }
            nRow++;
        }
        nTab++;
    }
    *pCount = nCount;
    return true;
}

bool ScInterpreter::CreateStringArr(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    sal_uInt8* pCellArr)
{

    // Old Add-Ins are hard limited to sal_uInt16 values.
#if MAXCOLCOUNT_DEFINE > USHRT_MAX
#error Add check for columns > USHRT_MAX!
#endif
    if (nRow1 > USHRT_MAX || nRow2 > USHRT_MAX)
        return false;

    sal_uInt16 nCount = 0;
    sal_uInt16* p = reinterpret_cast<sal_uInt16*>(pCellArr);
    *p++ = static_cast<sal_uInt16>(nCol1);
    *p++ = static_cast<sal_uInt16>(nRow1);
    *p++ = static_cast<sal_uInt16>(nTab1);
    *p++ = static_cast<sal_uInt16>(nCol2);
    *p++ = static_cast<sal_uInt16>(nRow2);
    *p++ = static_cast<sal_uInt16>(nTab2);
    sal_uInt16* pCount = p;
    *p++ = 0;
    sal_uInt16 nPos = 14;
    SCTAB nTab = nTab1;
    while (nTab <= nTab2)
    {
        SCROW nRow = nRow1;
        while (nRow <= nRow2)
        {
            SCCOL nCol = nCol1;
            while (nCol <= nCol2)
            {
                ScRefCellValue aCell(*pDok, ScAddress(nCol, nRow, nTab));
                if (!aCell.isEmpty())
                {
                    OUString  aStr;
                    sal_uInt16  nErr = 0;
                    bool    bOk = true;
                    switch (aCell.meType)
                    {
                        case CELLTYPE_STRING:
                        case CELLTYPE_EDIT:
                            aStr = aCell.getString(pDok);
                            break;
                        case CELLTYPE_FORMULA:
                            if (!aCell.mpFormula->IsValue())
                            {
                                nErr = aCell.mpFormula->GetErrCode();
                                aStr = aCell.mpFormula->GetString().getString();
                            }
                            else
                                bOk = false;
                            break;
                        default :
                            bOk = false;
                            break;
                    }
                    if (bOk)
                    {
                        OString aTmp(OUStringToOString(aStr,
                            osl_getThreadTextEncoding()));
                        // Old Add-Ins are limited to sal_uInt16 string
                        // lengths, and room for pad byte check.
                        if ( aTmp.getLength() > SAL_MAX_UINT16 - 2 )
                            return false;
                        // Append a 0-pad-byte if string length is odd
                        // MUST be sal_uInt16
                        sal_uInt16 nStrLen = (sal_uInt16) aTmp.getLength();
                        sal_uInt16 nLen = ( nStrLen + 2 ) & ~1;

                        if (((sal_uLong)nPos + (5 * sizeof(sal_uInt16)) + nLen) > MAXARRSIZE)
                            return false;
                        *p++ = static_cast<sal_uInt16>(nCol);
                        *p++ = static_cast<sal_uInt16>(nRow);
                        *p++ = static_cast<sal_uInt16>(nTab);
                        *p++ = nErr;
                        *p++ = nLen;
                        memcpy( p, aTmp.getStr(), nStrLen + 1);
                        nPos += 10 + nStrLen + 1;
                        sal_uInt8* q = ( pCellArr + nPos );
                        if( (nStrLen & 1) == 0 )
                        {
                            *q++ = 0;
                            nPos++;
                        }
                        p = reinterpret_cast<sal_uInt16*>( pCellArr + nPos );
                        nCount++;
                    }
                }
                nCol++;
            }
            nRow++;
        }
        nTab++;
    }
    *pCount = nCount;
    return true;
}

bool ScInterpreter::CreateCellArr(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                  SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                  sal_uInt8* pCellArr)
{

    // Old Add-Ins are hard limited to sal_uInt16 values.
#if MAXCOLCOUNT_DEFINE > USHRT_MAX
#error Add check for columns > USHRT_MAX!
#endif
    if (nRow1 > USHRT_MAX || nRow2 > USHRT_MAX)
        return false;

    sal_uInt16 nCount = 0;
    sal_uInt16* p = reinterpret_cast<sal_uInt16*>(pCellArr);
    *p++ = static_cast<sal_uInt16>(nCol1);
    *p++ = static_cast<sal_uInt16>(nRow1);
    *p++ = static_cast<sal_uInt16>(nTab1);
    *p++ = static_cast<sal_uInt16>(nCol2);
    *p++ = static_cast<sal_uInt16>(nRow2);
    *p++ = static_cast<sal_uInt16>(nTab2);
    sal_uInt16* pCount = p;
    *p++ = 0;
    sal_uInt16 nPos = 14;
    SCTAB nTab = nTab1;
    ScAddress aAdr;
    while (nTab <= nTab2)
    {
        aAdr.SetTab( nTab );
        SCROW nRow = nRow1;
        while (nRow <= nRow2)
        {
            aAdr.SetRow( nRow );
            SCCOL nCol = nCol1;
            while (nCol <= nCol2)
            {
                aAdr.SetCol( nCol );
                ScRefCellValue aCell(*pDok, aAdr);
                if (!aCell.isEmpty())
                {
                    sal_uInt16  nErr = 0;
                    sal_uInt16  nType = 0; // 0 = Zahl; 1 = String
                    double  nVal = 0.0;
                    OUString  aStr;
                    bool    bOk = true;
                    switch (aCell.meType)
                    {
                        case CELLTYPE_STRING :
                        case CELLTYPE_EDIT :
                            aStr = aCell.getString(pDok);
                            nType = 1;
                            break;
                        case CELLTYPE_VALUE :
                            nVal = GetValueCellValue(aAdr, aCell.mfValue);
                            break;
                        case CELLTYPE_FORMULA :
                            nErr = aCell.mpFormula->GetErrCode();
                            if (aCell.mpFormula->IsValue())
                                nVal = aCell.mpFormula->GetValue();
                            else
                                aStr = aCell.mpFormula->GetString().getString();
                            break;
                        default :
                            bOk = false;
                            break;
                    }
                    if (bOk)
                    {
                        if ((nPos + (5 * sizeof(sal_uInt16))) > MAXARRSIZE)
                            return false;
                        *p++ = static_cast<sal_uInt16>(nCol);
                        *p++ = static_cast<sal_uInt16>(nRow);
                        *p++ = static_cast<sal_uInt16>(nTab);
                        *p++ = nErr;
                        *p++ = nType;
                        nPos += 10;
                        if (nType == 0)
                        {
                            if ((nPos + sizeof(double)) > MAXARRSIZE)
                                return false;
                            memcpy( p, &nVal, sizeof(double));
                            nPos += sizeof(double);
                        }
                        else
                        {
                            OString aTmp(OUStringToOString(aStr,
                                osl_getThreadTextEncoding()));
                            // Old Add-Ins are limited to sal_uInt16 string
                            // lengths, and room for pad byte check.
                            if ( aTmp.getLength() > SAL_MAX_UINT16 - 2 )
                                return false;
                            // Append a 0-pad-byte if string length is odd
                            // MUST be sal_uInt16
                            sal_uInt16 nStrLen = (sal_uInt16) aTmp.getLength();
                            sal_uInt16 nLen = ( nStrLen + 2 ) & ~1;
                            if ( ((sal_uLong)nPos + 2 + nLen) > MAXARRSIZE)
                                return false;
                            *p++ = nLen;
                            memcpy( p, aTmp.getStr(), nStrLen + 1);
                            nPos += 2 + nStrLen + 1;
                            sal_uInt8* q = ( pCellArr + nPos );
                            if( (nStrLen & 1) == 0 )
                            {
                                *q++ = 0;
                                nPos++;
                            }
                        }
                        nCount++;
                        p = reinterpret_cast<sal_uInt16*>( pCellArr + nPos );
                    }
                }
                nCol++;
            }
            nRow++;
        }
        nTab++;
    }
    *pCount = nCount;
    return true;
}

// Stack operations

// Also releases a TempToken if appropriate.

void ScInterpreter::PushWithoutError( FormulaToken& r )
{
    if ( sp >= MAXSTACK )
        SetError( errStackOverflow );
    else
    {
        nCurFmtType = css::util::NumberFormat::UNDEFINED;
        r.IncRef();
        if( sp >= maxsp )
            maxsp = sp + 1;
        else
            pStack[ sp ]->DecRef();
        pStack[ sp ] = &r;
        ++sp;
    }
}

void ScInterpreter::Push( FormulaToken& r )
{
    if ( sp >= MAXSTACK )
        SetError( errStackOverflow );
    else
    {
        if (nGlobalError)
        {
            if (r.GetType() == svError)
            {
                r.SetError( nGlobalError);
                PushWithoutError( r);
            }
            else
                PushWithoutError( *(new FormulaErrorToken( nGlobalError)));
        }
        else
            PushWithoutError( r);
    }
}

void ScInterpreter::PushTempToken( FormulaToken* p )
{
    if ( sp >= MAXSTACK )
    {
        SetError( errStackOverflow );
        // p may be a dangling pointer hereafter!
        p->DeleteIfZeroRef();
    }
    else
    {
        if (nGlobalError)
        {
            if (p->GetType() == svError)
            {
                p->SetError( nGlobalError);
                PushTempTokenWithoutError( p);
            }
            else
            {
                // p may be a dangling pointer hereafter!
                p->DeleteIfZeroRef();
                PushTempTokenWithoutError( new FormulaErrorToken( nGlobalError));
            }
        }
        else
            PushTempTokenWithoutError( p);
    }
}

void ScInterpreter::PushTempTokenWithoutError( FormulaToken* p )
{
    p->IncRef();
    if ( sp >= MAXSTACK )
    {
        SetError( errStackOverflow );
        // p may be a dangling pointer hereafter!
        p->DecRef();
    }
    else
    {
        if( sp >= maxsp )
            maxsp = sp + 1;
        else
            pStack[ sp ]->DecRef();
        pStack[ sp ] = p;
        ++sp;
    }
}

void ScInterpreter::PushTempToken( const FormulaToken& r )
{
    if (!IfErrorPushError())
        PushTempTokenWithoutError( r.Clone());
}

void ScInterpreter::PushCellResultToken( bool bDisplayEmptyAsString,
        const ScAddress & rAddress, short * pRetTypeExpr, sal_uLong * pRetIndexExpr, bool bFinalResult )
{
    ScRefCellValue aCell(*pDok, rAddress);
    if (aCell.hasEmptyValue())
    {
        bool bInherited = (aCell.meType == CELLTYPE_FORMULA);
        if (pRetTypeExpr && pRetIndexExpr)
            pDok->GetNumberFormatInfo(*pRetTypeExpr, *pRetIndexExpr, rAddress);
        PushTempToken( new ScEmptyCellToken( bInherited, bDisplayEmptyAsString));
        return;
    }

    sal_uInt16 nErr = 0;
    if (aCell.meType == CELLTYPE_FORMULA)
        nErr = aCell.mpFormula->GetErrCode();

    if (nErr)
    {
        PushError( nErr);
        if (pRetTypeExpr)
            *pRetTypeExpr = css::util::NumberFormat::UNDEFINED;
        if (pRetIndexExpr)
            *pRetIndexExpr = 0;
    }
    else if (aCell.hasString())
    {
        svl::SharedString aRes;
        GetCellString( aRes, aCell);
        PushString( aRes);
        if (pRetTypeExpr)
            *pRetTypeExpr = css::util::NumberFormat::TEXT;
        if (pRetIndexExpr)
            *pRetIndexExpr = 0;
    }
    else
    {
        double fVal = GetCellValue(rAddress, aCell);
        if (bFinalResult)
        {
            TreatDoubleError( fVal);
            if (!IfErrorPushError())
                PushTempTokenWithoutError( new FormulaDoubleToken( fVal));
        }
        else
        {
            PushDouble( fVal);
        }
        if (pRetTypeExpr)
            *pRetTypeExpr = nCurFmtType;
        if (pRetIndexExpr)
            *pRetIndexExpr = nCurFmtIndex;
    }
}

// Simply throw away TOS.

void ScInterpreter::Pop()
{
    if( sp )
        sp--;
    else
        SetError(errUnknownStackVariable);
}

// Simply throw away TOS and set error code, used with ocIsError et al.

void ScInterpreter::PopError()
{
    if( sp )
    {
        sp--;
        if (pStack[sp]->GetType() == svError)
            nGlobalError = pStack[sp]->GetError();
    }
    else
        SetError(errUnknownStackVariable);
}

FormulaTokenRef ScInterpreter::PopToken()
{
    if (sp)
    {
        sp--;
        FormulaToken* p = pStack[ sp ];
        if (p->GetType() == svError)
            nGlobalError = p->GetError();
        return p;
    }
    else
        SetError(errUnknownStackVariable);
    return nullptr;
}

double ScInterpreter::PopDouble()
{
    nCurFmtType = css::util::NumberFormat::NUMBER;
    nCurFmtIndex = 0;
    if( sp )
    {
        --sp;
        FormulaToken* p = pStack[ sp ];
        switch (p->GetType())
        {
            case svError:
                nGlobalError = p->GetError();
                break;
            case svDouble:
                {
                    short nType = p->GetDoubleType();
                    if (nType && nType != css::util::NumberFormat::UNDEFINED)
                        nCurFmtType = nType;
                    return p->GetDouble();
                }
            case svEmptyCell:
            case svMissing:
                return 0.0;
            default:
                SetError( errIllegalArgument);
        }
    }
    else
        SetError( errUnknownStackVariable);
    return 0.0;
}

svl::SharedString ScInterpreter::PopString()
{
    nCurFmtType = css::util::NumberFormat::TEXT;
    nCurFmtIndex = 0;
    if( sp )
    {
        --sp;
        FormulaToken* p = pStack[ sp ];
        switch (p->GetType())
        {
            case svError:
                nGlobalError = p->GetError();
                break;
            case svString:
                return p->GetString();
            case svEmptyCell:
            case svMissing:
                return svl::SharedString::getEmptyString();
            default:
                SetError( errIllegalArgument);
        }
    }
    else
        SetError( errUnknownStackVariable);

    return svl::SharedString::getEmptyString();
}

void ScInterpreter::ValidateRef( const ScSingleRefData & rRef )
{
    SCCOL nCol;
    SCROW nRow;
    SCTAB nTab;
    SingleRefToVars( rRef, nCol, nRow, nTab);
}

void ScInterpreter::ValidateRef( const ScComplexRefData & rRef )
{
    ValidateRef( rRef.Ref1);
    ValidateRef( rRef.Ref2);
}

void ScInterpreter::ValidateRef( const ScRefList & rRefList )
{
    ScRefList::const_iterator it( rRefList.begin());
    ScRefList::const_iterator end( rRefList.end());
    for ( ; it != end; ++it)
    {
        ValidateRef( *it);
    }
}

void ScInterpreter::SingleRefToVars( const ScSingleRefData & rRef,
        SCCOL & rCol, SCROW & rRow, SCTAB & rTab )
{
    if ( rRef.IsColRel() )
        rCol = aPos.Col() + rRef.Col();
    else
        rCol = rRef.Col();

    if ( rRef.IsRowRel() )
        rRow = aPos.Row() + rRef.Row();
    else
        rRow = rRef.Row();

    if ( rRef.IsTabRel() )
        rTab = aPos.Tab() + rRef.Tab();
    else
        rTab = rRef.Tab();

    if( !ValidCol( rCol) || rRef.IsColDeleted() )
    {
        SetError( errNoRef );
        rCol = 0;
    }
    if( !ValidRow( rRow) || rRef.IsRowDeleted() )
    {
        SetError( errNoRef );
        rRow = 0;
    }
    if( !ValidTab( rTab, pDok->GetTableCount() - 1) || rRef.IsTabDeleted() )
    {
        SetError( errNoRef );
        rTab = 0;
    }
}

void ScInterpreter::PopSingleRef(SCCOL& rCol, SCROW &rRow, SCTAB& rTab)
{
    if( sp )
    {
        --sp;
        FormulaToken* p = pStack[ sp ];
        switch (p->GetType())
        {
            case svError:
                nGlobalError = p->GetError();
                break;
            case svSingleRef:
                SingleRefToVars( *p->GetSingleRef(), rCol, rRow, rTab);
                if (!pDok->m_TableOpList.empty())
                    ReplaceCell( rCol, rRow, rTab );
                break;
            default:
                SetError( errIllegalParameter);
        }
    }
    else
        SetError( errUnknownStackVariable);
}

void ScInterpreter::PopSingleRef( ScAddress& rAdr )
{
    if( sp )
    {
        --sp;
        FormulaToken* p = pStack[ sp ];
        switch (p->GetType())
        {
            case svError:
                nGlobalError = p->GetError();
                break;
            case svSingleRef:
                {
                    SCCOL nCol;
                    SCROW nRow;
                    SCTAB nTab;
                    SingleRefToVars( *p->GetSingleRef(), nCol, nRow, nTab);
                    rAdr.Set( nCol, nRow, nTab );
                    if (!pDok->m_TableOpList.empty())
                        ReplaceCell( rAdr );
                }
                break;
            default:
                SetError( errIllegalParameter);
        }
    }
    else
        SetError( errUnknownStackVariable);
}

void ScInterpreter::DoubleRefToVars( const formula::FormulaToken* p,
        SCCOL& rCol1, SCROW &rRow1, SCTAB& rTab1,
        SCCOL& rCol2, SCROW &rRow2, SCTAB& rTab2 )
{
    const ScComplexRefData& rCRef = *p->GetDoubleRef();
    SingleRefToVars( rCRef.Ref1, rCol1, rRow1, rTab1);
    SingleRefToVars( rCRef.Ref2, rCol2, rRow2, rTab2);
    if (!pDok->m_TableOpList.empty())
    {
        ScRange aRange( rCol1, rRow1, rTab1, rCol2, rRow2, rTab2 );
        if ( IsTableOpInRange( aRange ) )
            SetError( errIllegalParameter );
    }
}

ScDBRangeBase* ScInterpreter::PopDBDoubleRef()
{
    StackVar eType = GetStackType();
    switch (eType)
    {
        case svUnknown:
            SetError(errUnknownStackVariable);
        break;
        case svError:
            PopError();
        break;
        case svDoubleRef:
        {
            SCCOL nCol1, nCol2;
            SCROW nRow1, nRow2;
            SCTAB nTab1, nTab2;
            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            if (nGlobalError)
                break;
            return new ScDBInternalRange(pDok,
                ScRange(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2));
        }
        case svMatrix:
        case svExternalDoubleRef:
        {
            ScMatrixRef pMat;
            if (eType == svMatrix)
                pMat = PopMatrix();
            else
                PopExternalDoubleRef(pMat);
            if (nGlobalError)
                break;
            return new ScDBExternalRange(pDok, pMat);
        }
        default:
            SetError( errIllegalParameter);
    }

    return nullptr;
}

void ScInterpreter::PopDoubleRef(SCCOL& rCol1, SCROW &rRow1, SCTAB& rTab1,
                                 SCCOL& rCol2, SCROW &rRow2, SCTAB& rTab2)
{
    if( sp )
    {
        --sp;
        FormulaToken* p = pStack[ sp ];
        switch (p->GetType())
        {
            case svError:
                nGlobalError = p->GetError();
                break;
            case svDoubleRef:
                DoubleRefToVars( p, rCol1, rRow1, rTab1, rCol2, rRow2, rTab2);
                break;
            default:
                SetError( errIllegalParameter);
        }
    }
    else
        SetError( errUnknownStackVariable);
}

void ScInterpreter::DoubleRefToRange( const ScComplexRefData & rCRef,
        ScRange & rRange, bool bDontCheckForTableOp )
{
    SCCOL nCol;
    SCROW nRow;
    SCTAB nTab;
    SingleRefToVars( rCRef.Ref1, nCol, nRow, nTab);
    rRange.aStart.Set( nCol, nRow, nTab );
    SingleRefToVars( rCRef.Ref2, nCol, nRow, nTab);
    rRange.aEnd.Set( nCol, nRow, nTab );
    rRange.PutInOrder();
    if (!pDok->m_TableOpList.empty() && !bDontCheckForTableOp)
    {
        if ( IsTableOpInRange( rRange ) )
            SetError( errIllegalParameter );
    }
}

void ScInterpreter::PopDoubleRef( ScRange & rRange, short & rParam, size_t & rRefInList )
{
    if (sp)
    {
        formula::FormulaToken* pToken = pStack[ sp-1 ];
        switch (pToken->GetType())
        {
            case svError:
                nGlobalError = pToken->GetError();
                break;
            case svDoubleRef:
                --sp;
                DoubleRefToRange( *pToken->GetDoubleRef(), rRange);
                break;
            case svRefList:
                {
                    const ScRefList* pList = pToken->GetRefList();
                    if (rRefInList < pList->size())
                    {
                        DoubleRefToRange( (*pList)[rRefInList], rRange);
                        if (++rRefInList < pList->size())
                            ++rParam;
                        else
                        {
                            --sp;
                            rRefInList = 0;
                        }
                    }
                    else
                    {
                        --sp;
                        rRefInList = 0;
                        SetError( errIllegalParameter);
                    }
                }
                break;
            default:
                SetError( errIllegalParameter);
        }
    }
    else
        SetError( errUnknownStackVariable);
}

void ScInterpreter::PopDoubleRef( ScRange& rRange, bool bDontCheckForTableOp )
{
    if( sp )
    {
        --sp;
        FormulaToken* p = pStack[ sp ];
        switch (p->GetType())
        {
            case svError:
                nGlobalError = p->GetError();
                break;
            case svDoubleRef:
                DoubleRefToRange( *p->GetDoubleRef(), rRange, bDontCheckForTableOp);
                break;
            default:
                SetError( errIllegalParameter);
        }
    }
    else
        SetError( errUnknownStackVariable);
}

void ScInterpreter::PopExternalSingleRef(sal_uInt16& rFileId, OUString& rTabName, ScSingleRefData& rRef)
{
    if (!sp)
    {
        SetError(errUnknownStackVariable);
        return;
    }

    --sp;
    FormulaToken* p = pStack[sp];
    StackVar eType = p->GetType();

    if (eType == svError)
    {
        nGlobalError = p->GetError();
        return;
    }

    if (eType != svExternalSingleRef)
    {
        SetError( errIllegalParameter);
        return;
    }

    rFileId = p->GetIndex();
    rTabName = p->GetString().getString();
    rRef = *p->GetSingleRef();
}

void ScInterpreter::PopExternalSingleRef(ScExternalRefCache::TokenRef& rToken, ScExternalRefCache::CellFormat* pFmt)
{
    sal_uInt16 nFileId;
    OUString aTabName;
    ScSingleRefData aData;
    PopExternalSingleRef(nFileId, aTabName, aData, rToken, pFmt);
}

void ScInterpreter::PopExternalSingleRef(
    sal_uInt16& rFileId, OUString& rTabName, ScSingleRefData& rRef,
    ScExternalRefCache::TokenRef& rToken, ScExternalRefCache::CellFormat* pFmt)
{
    PopExternalSingleRef(rFileId, rTabName, rRef);
    if (nGlobalError)
        return;

    ScExternalRefManager* pRefMgr = pDok->GetExternalRefManager();
    const OUString* pFile = pRefMgr->getExternalFileName(rFileId);
    if (!pFile)
    {
        SetError(errNoName);
        return;
    }

    if (rRef.IsTabRel())
    {
        OSL_FAIL("ScCompiler::GetToken: external single reference must have an absolute table reference!");
        SetError(errNoRef);
        return;
    }

    ScAddress aAddr = rRef.toAbs(aPos);
    ScExternalRefCache::CellFormat aFmt;
    ScExternalRefCache::TokenRef xNew = pRefMgr->getSingleRefToken(
        rFileId, rTabName, aAddr, &aPos, nullptr, &aFmt);

    if (!xNew)
    {
        SetError(errNoRef);
        return;
    }

    if (xNew->GetType() == svError)
        SetError( xNew->GetError());

    rToken = xNew;
    if (pFmt)
        *pFmt = aFmt;
}

void ScInterpreter::PopExternalDoubleRef(sal_uInt16& rFileId, OUString& rTabName, ScComplexRefData& rRef)
{
    if (!sp)
    {
        SetError(errUnknownStackVariable);
        return;
    }

    --sp;
    FormulaToken* p = pStack[sp];
    StackVar eType = p->GetType();

    if (eType == svError)
    {
        nGlobalError = p->GetError();
        return;
    }

    if (eType != svExternalDoubleRef)
    {
        SetError( errIllegalParameter);
        return;
    }

    rFileId = p->GetIndex();
    rTabName = p->GetString().getString();
    rRef = *p->GetDoubleRef();
}

void ScInterpreter::PopExternalDoubleRef(ScExternalRefCache::TokenArrayRef& rArray)
{
    sal_uInt16 nFileId;
    OUString aTabName;
    ScComplexRefData aData;
    PopExternalDoubleRef(nFileId, aTabName, aData);
    if (nGlobalError)
        return;

    GetExternalDoubleRef(nFileId, aTabName, aData, rArray);
    if (nGlobalError)
        return;
}

void ScInterpreter::PopExternalDoubleRef(ScMatrixRef& rMat)
{
    ScExternalRefCache::TokenArrayRef pArray;
    PopExternalDoubleRef(pArray);
    if (nGlobalError)
        return;

    // For now, we only support single range data for external
    // references, which means the array should only contain a
    // single matrix token.
    formula::FormulaToken* p = pArray->First();
    if (!p || p->GetType() != svMatrix)
        SetError( errIllegalParameter);
    else
    {
        rMat = p->GetMatrix();
        if (!rMat)
            SetError( errUnknownVariable);
    }
}

void ScInterpreter::GetExternalDoubleRef(
    sal_uInt16 nFileId, const OUString& rTabName, const ScComplexRefData& rData, ScExternalRefCache::TokenArrayRef& rArray)
{
    ScExternalRefManager* pRefMgr = pDok->GetExternalRefManager();
    const OUString* pFile = pRefMgr->getExternalFileName(nFileId);
    if (!pFile)
    {
        SetError(errNoName);
        return;
    }
    if (rData.Ref1.IsTabRel() || rData.Ref2.IsTabRel())
    {
        OSL_FAIL("ScCompiler::GetToken: external double reference must have an absolute table reference!");
        SetError(errNoRef);
        return;
    }

    ScComplexRefData aData(rData);
    ScRange aRange = aData.toAbs(aPos);
    ScExternalRefCache::TokenArrayRef pArray = pRefMgr->getDoubleRefTokens(
        nFileId, rTabName, aRange, &aPos);

    if (!pArray)
    {
        SetError(errIllegalArgument);
        return;
    }

    formula::FormulaToken* pToken = pArray->First();
    if (pToken->GetType() == svError)
    {
        SetError( pToken->GetError());
        return;
    }
    if (pToken->GetType() != svMatrix)
    {
        SetError(errIllegalArgument);
        return;
    }

    if (pArray->Next())
    {
        // Can't handle more than one matrix per parameter.
        SetError( errIllegalArgument);
        return;
    }

    rArray = pArray;
}

bool ScInterpreter::PopDoubleRefOrSingleRef( ScAddress& rAdr )
{
    switch ( GetStackType() )
    {
        case svDoubleRef :
        {
            ScRange aRange;
            PopDoubleRef( aRange, true );
            return DoubleRefToPosSingleRef( aRange, rAdr );
        }
        case svSingleRef :
        {
            PopSingleRef( rAdr );
            return true;
        }
        default:
            PopError();
            SetError( errNoRef );
    }
    return false;
}

void ScInterpreter::PopDoubleRefPushMatrix()
{
    if ( GetStackType() == svDoubleRef )
    {
        ScMatrixRef pMat = GetMatrix();
        if ( pMat )
            PushMatrix( pMat );
        else
            PushIllegalParameter();
    }
    else
        SetError( errNoRef );
}

ScTokenMatrixMap* ScInterpreter::CreateTokenMatrixMap()
{
    return new ScTokenMatrixMap;
}

bool ScInterpreter::ConvertMatrixParameters()
{
    sal_uInt16 nParams = pCur->GetParamCount();
    OSL_ENSURE( nParams <= sp, "ConvertMatrixParameters: stack/param count mismatch");
    SCSIZE nJumpCols = 0, nJumpRows = 0;
    for ( sal_uInt16 i=1; i <= nParams && i <= sp; ++i )
    {
        FormulaToken* p = pStack[ sp - i ];
        if ( p->GetOpCode() != ocPush && p->GetOpCode() != ocMissing)
        {
            OSL_FAIL( "ConvertMatrixParameters: not a push");
        }
        else
        {
            switch ( p->GetType() )
            {
                case svDouble:
                case svString:
                case svSingleRef:
                case svExternalSingleRef:
                case svMissing:
                case svError:
                case svEmptyCell:
                    // nothing to do
                break;
                case svMatrix:
                {
                    if ( ScParameterClassification::GetParameterType( pCur, nParams - i)
                            == ScParameterClassification::Value )
                    {   // only if single value expected
                        ScMatrixRef pMat = p->GetMatrix();
                        if ( !pMat )
                            SetError( errUnknownVariable);
                        else
                        {
                            SCSIZE nCols, nRows;
                            pMat->GetDimensions( nCols, nRows);
                            if ( nJumpCols < nCols )
                                nJumpCols = nCols;
                            if ( nJumpRows < nRows )
                                nJumpRows = nRows;
                        }
                    }
                }
                break;
                case svDoubleRef:
                {
                    ScParameterClassification::Type eType =
                        ScParameterClassification::GetParameterType( pCur, nParams - i);
                    if ( eType != ScParameterClassification::Reference &&
                            eType != ScParameterClassification::ReferenceOrForceArray &&
                            // For scalar Value: convert to Array/JumpMatrix
                            // only if in array formula context, else (function
                            // has ForceArray or ReferenceOrForceArray
                            // parameter *somewhere else*) pick a normal
                            // position dependent implicit intersection later.
                            (eType != ScParameterClassification::Value || bMatrixFormula || pCur->IsInForceArray()))
                    {
                        SCCOL nCol1, nCol2;
                        SCROW nRow1, nRow2;
                        SCTAB nTab1, nTab2;
                        DoubleRefToVars( p, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                        // Make sure the map exists, created if not.
                        GetTokenMatrixMap();
                        ScMatrixRef pMat = CreateMatrixFromDoubleRef( p,
                                nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                        if (pMat)
                        {
                            if ( eType == ScParameterClassification::Value )
                            {   // only if single value expected
                                if ( nJumpCols < static_cast<SCSIZE>(nCol2 - nCol1 + 1) )
                                    nJumpCols = static_cast<SCSIZE>(nCol2 - nCol1 + 1);
                                if ( nJumpRows < static_cast<SCSIZE>(nRow2 - nRow1 + 1) )
                                    nJumpRows = static_cast<SCSIZE>(nRow2 - nRow1 + 1);
                            }
                            formula::FormulaToken* pNew = new ScMatrixToken( pMat);
                            pNew->IncRef();
                            pStack[ sp - i ] = pNew;
                            p->DecRef();    // p may be dead now!
                        }
                    }
                }
                break;
                case svExternalDoubleRef:
                {
                    ScParameterClassification::Type eType =
                        ScParameterClassification::GetParameterType( pCur, nParams - i);
                    if (eType == ScParameterClassification::Array)
                    {
                        sal_uInt16 nFileId = p->GetIndex();
                        OUString aTabName = p->GetString().getString();
                        const ScComplexRefData& rRef = *p->GetDoubleRef();
                        ScExternalRefCache::TokenArrayRef pArray;
                        GetExternalDoubleRef(nFileId, aTabName, rRef, pArray);
                        if (nGlobalError || !pArray)
                            break;
                        formula::FormulaToken* pTemp = pArray->First();
                        if (!pTemp)
                            break;

                        ScMatrixRef pMat = pTemp->GetMatrix();
                        if (pMat)
                        {
                            formula::FormulaToken* pNew = new ScMatrixToken( pMat);
                            pNew->IncRef();
                            pStack[ sp - i ] = pNew;
                            p->DecRef();    // p may be dead now!
                        }
                    }
                }
                break;
                case svRefList:
                {
                    ScParameterClassification::Type eType =
                        ScParameterClassification::GetParameterType( pCur, nParams - i);
                    if ( eType != ScParameterClassification::Reference &&
                            eType != ScParameterClassification::ReferenceOrForceArray)
                    {
                        // can't convert to matrix
                        SetError( errNoValue);
                    }
                }
                break;
                default:
                    OSL_FAIL( "ConvertMatrixParameters: unknown parameter type");
            }
        }
    }
    if( nJumpCols && nJumpRows )
    {
        short nPC = aCode.GetPC();
        short nStart = nPC - 1;     // restart on current code (-1)
        short nNext = nPC;          // next instruction after subroutine
        short nStop = nPC + 1;      // stop subroutine before reaching that
        FormulaTokenRef xNew;
        ScTokenMatrixMap::const_iterator aMapIter;
        if (pTokenMatrixMap && ((aMapIter = pTokenMatrixMap->find( pCur)) !=
                    pTokenMatrixMap->end()))
            xNew = (*aMapIter).second;
        else
        {
            ScJumpMatrix* pJumpMat = new ScJumpMatrix( nJumpCols, nJumpRows);
            pJumpMat->SetAllJumps( 1.0, nStart, nNext, nStop);
            // pop parameters and store in ScJumpMatrix, push in JumpMatrix()
            ScTokenVec* pParams = new ScTokenVec( nParams);
            for ( sal_uInt16 i=1; i <= nParams && sp > 0; ++i )
            {
                FormulaToken* p = pStack[ --sp ];
                p->IncRef();
                // store in reverse order such that a push may simply iterate
                (*pParams)[ nParams - i ] = p;
            }
            pJumpMat->SetJumpParameters( pParams);
            xNew = new ScJumpMatrixToken( pJumpMat );
            GetTokenMatrixMap().insert( ScTokenMatrixMap::value_type( pCur,
                        xNew));
        }
        PushTempTokenWithoutError( xNew.get());
        // set continuation point of path for main code line
        aCode.Jump( nNext, nNext);
        return true;
    }
    return false;
}

ScMatrixRef ScInterpreter::PopMatrix()
{
    if( sp )
    {
        --sp;
        FormulaToken* p = pStack[ sp ];
        switch (p->GetType())
        {
            case svError:
                nGlobalError = p->GetError();
                break;
            case svMatrix:
                {
                    ScMatrix* pMat = p->GetMatrix();
                    if ( pMat )
                        pMat->SetErrorInterpreter( this);
                    else
                        SetError( errUnknownVariable);
                    return pMat;
                }
            default:
                SetError( errIllegalParameter);
        }
    }
    else
        SetError( errUnknownStackVariable);
    return nullptr;
}

sc::RangeMatrix ScInterpreter::PopRangeMatrix()
{
    sc::RangeMatrix aRet;
    if (sp)
    {
        switch (pStack[sp-1]->GetType())
        {
            case svMatrix:
            {
                --sp;
                FormulaToken* p = pStack[sp];
                aRet.mpMat = p->GetMatrix();
                if (aRet.mpMat)
                {
                    aRet.mpMat->SetErrorInterpreter(this);
                    if (p->GetByte() == MATRIX_TOKEN_HAS_RANGE)
                    {
                        const ScComplexRefData& rRef = *p->GetDoubleRef();
                        if (!rRef.Ref1.IsColRel() && !rRef.Ref1.IsRowRel() && !rRef.Ref2.IsColRel() && !rRef.Ref2.IsRowRel())
                        {
                            aRet.mnCol1 = rRef.Ref1.Col();
                            aRet.mnRow1 = rRef.Ref1.Row();
                            aRet.mnTab1 = rRef.Ref1.Tab();
                            aRet.mnCol2 = rRef.Ref2.Col();
                            aRet.mnRow2 = rRef.Ref2.Row();
                            aRet.mnTab2 = rRef.Ref2.Tab();
                        }
                    }
                }
                else
                    SetError( errUnknownVariable);
            }
            break;
            default:
                aRet.mpMat = PopMatrix();
        }
    }
    return aRet;
}

void ScInterpreter::QueryMatrixType(ScMatrixRef& xMat, short& rRetTypeExpr, sal_uLong& rRetIndexExpr)
{
    if (xMat)
    {
        SCSIZE nCols, nRows;
        xMat->GetDimensions(nCols, nRows);
        ScMatrixValue nMatVal = xMat->Get(0, 0);
        ScMatValType nMatValType = nMatVal.nType;
        if (ScMatrix::IsNonValueType( nMatValType))
        {
            if ( xMat->IsEmptyPath( 0, 0))
            {   // result of empty FALSE jump path
                FormulaTokenRef xRes = new FormulaDoubleToken( 0.0);
                PushTempToken( new ScMatrixFormulaCellToken(nCols, nRows, xMat, xRes.get()));
                rRetTypeExpr = css::util::NumberFormat::LOGICAL;
            }
            else if ( xMat->IsEmptyResult( 0, 0))
            {   // empty formula result
                FormulaTokenRef xRes = new ScEmptyCellToken( true, true);   // inherited, display empty
                PushTempToken( new ScMatrixFormulaCellToken(nCols, nRows, xMat, xRes.get()));
            }
            else if ( xMat->IsEmpty( 0, 0))
            {   // empty or empty cell
                FormulaTokenRef xRes = new ScEmptyCellToken( false, true);  // not inherited, display empty
                PushTempToken( new ScMatrixFormulaCellToken(nCols, nRows, xMat, xRes.get()));
            }
            else
            {
                svl::SharedString aStr( nMatVal.GetString());
                FormulaTokenRef xRes = new FormulaStringToken( aStr);
                PushTempToken( new ScMatrixFormulaCellToken(nCols, nRows, xMat, xRes.get()));
                rRetTypeExpr = css::util::NumberFormat::TEXT;
            }
        }
        else
        {
            sal_uInt16 nErr = GetDoubleErrorValue( nMatVal.fVal);
            FormulaTokenRef xRes;
            if (nErr)
                xRes = new FormulaErrorToken( nErr);
            else
                xRes = new FormulaDoubleToken( nMatVal.fVal);
            PushTempToken( new ScMatrixFormulaCellToken(nCols, nRows, xMat, xRes.get()));
            if ( rRetTypeExpr != css::util::NumberFormat::LOGICAL )
                rRetTypeExpr = css::util::NumberFormat::NUMBER;
        }
        rRetIndexExpr = 0;
        xMat->SetErrorInterpreter( nullptr);
    }
    else
        SetError( errUnknownStackVariable);
}

formula::FormulaToken* ScInterpreter::CreateDoubleOrTypedToken( double fVal )
{
    // NumberFormat::NUMBER is the default untyped double.
    if (nFuncFmtType && nFuncFmtType != css::util::NumberFormat::NUMBER &&
            nFuncFmtType != css::util::NumberFormat::UNDEFINED)
        return new FormulaTypedDoubleToken( fVal, nFuncFmtType);
    else
        return new FormulaDoubleToken( fVal);
}

void ScInterpreter::PushDouble(double nVal)
{
    TreatDoubleError( nVal );
    if (!IfErrorPushError())
        PushTempTokenWithoutError( CreateDoubleOrTypedToken( nVal));
}

void ScInterpreter::PushInt(int nVal)
{
    if (!IfErrorPushError())
        PushTempTokenWithoutError( CreateDoubleOrTypedToken( nVal));
}

void ScInterpreter::PushStringBuffer( const sal_Unicode* pString )
{
    if ( pString )
    {
        svl::SharedString aSS = pDok->GetSharedStringPool().intern(OUString(pString));
        PushString(aSS);
    }
    else
        PushString(svl::SharedString::getEmptyString());
}

void ScInterpreter::PushString( const OUString& rStr )
{
    PushString(pDok->GetSharedStringPool().intern(rStr));
}

void ScInterpreter::PushString( const svl::SharedString& rString )
{
    if (!IfErrorPushError())
        PushTempTokenWithoutError( new FormulaStringToken( rString ) );
}

void ScInterpreter::PushSingleRef(SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    if (!IfErrorPushError())
    {
        ScSingleRefData aRef;
        aRef.InitAddress(ScAddress(nCol,nRow,nTab));
        PushTempTokenWithoutError( new ScSingleRefToken( aRef ) );
    }
}

void ScInterpreter::PushDoubleRef(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                  SCCOL nCol2, SCROW nRow2, SCTAB nTab2)
{
    if (!IfErrorPushError())
    {
        ScComplexRefData aRef;
        aRef.InitRange(ScRange(nCol1,nRow1,nTab1,nCol2,nRow2,nTab2));
        PushTempTokenWithoutError( new ScDoubleRefToken( aRef ) );
    }
}

void ScInterpreter::PushExternalSingleRef(
    sal_uInt16 nFileId, const OUString& rTabName, SCCOL nCol, SCROW nRow, SCTAB nTab)
{
    if (!IfErrorPushError())
    {
        ScSingleRefData aRef;
        aRef.InitAddress(ScAddress(nCol,nRow,nTab));
        PushTempTokenWithoutError( new ScExternalSingleRefToken(nFileId,
                    pDok->GetSharedStringPool().intern( rTabName), aRef)) ;
    }
}

void ScInterpreter::PushExternalDoubleRef(
    sal_uInt16 nFileId, const OUString& rTabName,
    SCCOL nCol1, SCROW nRow1, SCTAB nTab1, SCCOL nCol2, SCROW nRow2, SCTAB nTab2)
{
    if (!IfErrorPushError())
    {
        ScComplexRefData aRef;
        aRef.InitRange(ScRange(nCol1,nRow1,nTab1,nCol2,nRow2,nTab2));
        PushTempTokenWithoutError( new ScExternalDoubleRefToken(nFileId,
                    pDok->GetSharedStringPool().intern( rTabName), aRef) );
    }
}

void ScInterpreter::PushSingleRef( const ScRefAddress& rRef )
{
    if (!IfErrorPushError())
    {
        ScSingleRefData aRef;
        aRef.InitFromRefAddress( rRef, aPos);
        PushTempTokenWithoutError( new ScSingleRefToken( aRef ) );
    }
}

void ScInterpreter::PushDoubleRef( const ScRefAddress& rRef1, const ScRefAddress& rRef2 )
{
    if (!IfErrorPushError())
    {
        ScComplexRefData aRef;
        aRef.InitFromRefAddresses( rRef1, rRef2, aPos);
        PushTempTokenWithoutError( new ScDoubleRefToken( aRef ) );
    }
}

void ScInterpreter::PushMatrix( const sc::RangeMatrix& rMat )
{
    if (!rMat.isRangeValid())
    {
        // Just push the matrix part only.
        PushMatrix(rMat.mpMat);
        return;
    }

    rMat.mpMat->SetErrorInterpreter(nullptr);
    nGlobalError = 0;
    PushTempTokenWithoutError(new ScMatrixRangeToken(rMat));
}

void ScInterpreter::PushMatrix(const ScMatrixRef& pMat)
{
    pMat->SetErrorInterpreter( nullptr);
    // No   if (!IfErrorPushError())   because ScMatrix stores errors itself,
    // but with notifying ScInterpreter via nGlobalError, substituting it would
    // mean to inherit the error on all array elements in all following
    // operations.
    nGlobalError = 0;
    PushTempTokenWithoutError( new ScMatrixToken( pMat ) );
}

void ScInterpreter::PushError( sal_uInt16 nError )
{
    SetError( nError );     // only sets error if not already set
    PushTempTokenWithoutError( new FormulaErrorToken( nGlobalError));
}

void ScInterpreter::PushParameterExpected()
{
    PushError( errParameterExpected);
}

void ScInterpreter::PushIllegalParameter()
{
    PushError( errIllegalParameter);
}

void ScInterpreter::PushIllegalArgument()
{
    PushError( errIllegalArgument);
}

void ScInterpreter::PushNA()
{
    PushError( NOTAVAILABLE);
}

void ScInterpreter::PushNoValue()
{
    PushError( errNoValue);
}

bool ScInterpreter::IsMissing()
{
    return sp && pStack[sp - 1]->GetType() == svMissing;
}

StackVar ScInterpreter::GetRawStackType()
{
    StackVar eRes;
    if( sp )
    {
        eRes = pStack[sp - 1]->GetType();
    }
    else
    {
        SetError(errUnknownStackVariable);
        eRes = svUnknown;
    }
    return eRes;
}

StackVar ScInterpreter::GetStackType()
{
    StackVar eRes;
    if( sp )
    {
        eRes = pStack[sp - 1]->GetType();
        if( eRes == svMissing || eRes == svEmptyCell )
            eRes = svDouble;    // default!
    }
    else
    {
        SetError(errUnknownStackVariable);
        eRes = svUnknown;
    }
    return eRes;
}

StackVar ScInterpreter::GetStackType( sal_uInt8 nParam )
{
    StackVar eRes;
    if( sp > nParam-1 )
    {
        eRes = pStack[sp - nParam]->GetType();
        if( eRes == svMissing || eRes == svEmptyCell )
            eRes = svDouble;    // default!
    }
    else
        eRes = svUnknown;
    return eRes;
}

void ScInterpreter::ReverseStack( sal_uInt8 nParamCount )
{
    //reverse order of parameter stack
    FormulaToken* p;
    assert( sp >= nParamCount && " less stack elements than parameters");
    short nStackParams = std::min<short>( sp, nParamCount);
    for ( short i = 0; i < short( nStackParams / 2 ); i++ )
    {
        p = pStack[ sp - ( nStackParams - i ) ];
        pStack[ sp - ( nStackParams - i ) ] = pStack[ sp - 1 - i ];
        pStack[ sp - 1 - i ] = p;
    }
}

bool ScInterpreter::DoubleRefToPosSingleRef( const ScRange& rRange, ScAddress& rAdr )
{
    // Check for a singleton first - no implicit intersection for them.
    if( rRange.aStart == rRange.aEnd )
    {
        rAdr = rRange.aStart;
        return true;
    }

    bool bOk = false;

    if ( pJumpMatrix )
    {
        bOk = rRange.aStart.Tab() == rRange.aEnd.Tab();
        if ( !bOk )
            SetError( errIllegalArgument);
        else
        {
            SCSIZE nC, nR;
            pJumpMatrix->GetPos( nC, nR);
            rAdr.SetCol( sal::static_int_cast<SCCOL>( rRange.aStart.Col() + nC ) );
            rAdr.SetRow( sal::static_int_cast<SCROW>( rRange.aStart.Row() + nR ) );
            rAdr.SetTab( rRange.aStart.Tab());
            bOk = rRange.aStart.Col() <= rAdr.Col() && rAdr.Col() <=
                rRange.aEnd.Col() && rRange.aStart.Row() <= rAdr.Row() &&
                rAdr.Row() <= rRange.aEnd.Row();
            if ( !bOk )
                SetError( errNoValue);
        }
        return bOk;
    }

    SCCOL nMyCol = aPos.Col();
    SCROW nMyRow = aPos.Row();
    SCTAB nMyTab = aPos.Tab();
    SCCOL nCol = 0;
    SCROW nRow = 0;
    SCTAB nTab;
    nTab = rRange.aStart.Tab();
    if ( rRange.aStart.Col() <= nMyCol && nMyCol <= rRange.aEnd.Col() )
    {
        nRow = rRange.aStart.Row();
        if ( nRow == rRange.aEnd.Row() )
        {
            bOk = true;
            nCol = nMyCol;
        }
        else if ( nTab != nMyTab && nTab == rRange.aEnd.Tab()
                && rRange.aStart.Row() <= nMyRow && nMyRow <= rRange.aEnd.Row() )
        {
            bOk = true;
            nCol = nMyCol;
            nRow = nMyRow;
        }
    }
    else if ( rRange.aStart.Row() <= nMyRow && nMyRow <= rRange.aEnd.Row() )
    {
        nCol = rRange.aStart.Col();
        if ( nCol == rRange.aEnd.Col() )
        {
            bOk = true;
            nRow = nMyRow;
        }
        else if ( nTab != nMyTab && nTab == rRange.aEnd.Tab()
                && rRange.aStart.Col() <= nMyCol && nMyCol <= rRange.aEnd.Col() )
        {
            bOk = true;
            nCol = nMyCol;
            nRow = nMyRow;
        }
    }
    if ( bOk )
    {
        if ( nTab == rRange.aEnd.Tab() )
            ;   // all done
        else if ( nTab <= nMyTab && nMyTab <= rRange.aEnd.Tab() )
            nTab = nMyTab;
        else
            bOk = false;
        if ( bOk )
            rAdr.Set( nCol, nRow, nTab );
    }
    if ( !bOk )
        SetError( errNoValue );
    return bOk;
}

double ScInterpreter::GetDoubleFromMatrix(const ScMatrixRef& pMat)
{
    if (!pMat)
        return 0.0;

    if ( !pJumpMatrix )
        return pMat->GetDoubleWithStringConversion( 0, 0);

    SCSIZE nCols, nRows, nC, nR;
    pMat->GetDimensions( nCols, nRows);
    pJumpMatrix->GetPos( nC, nR);
    // Use vector replication for single row/column arrays.
    if ( (nC < nCols || nCols == 1) && (nR < nRows || nRows == 1) )
        return pMat->GetDoubleWithStringConversion( nC, nR);

    SetError( errNoValue);
    return 0.0;
}

double ScInterpreter::GetDouble()
{
    double nVal(0.0);
    switch( GetRawStackType() )
    {
        case svDouble:
            nVal = PopDouble();
        break;
        case svString:
            nVal = ConvertStringToValue( PopString().getString());
        break;
        case svSingleRef:
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            ScRefCellValue aCell(*pDok, aAdr);
            nVal = GetCellValue(aAdr, aCell);
        }
        break;
        case svDoubleRef:
        {   // generate position dependent SingleRef
            ScRange aRange;
            PopDoubleRef( aRange );
            ScAddress aAdr;
            if ( !nGlobalError && DoubleRefToPosSingleRef( aRange, aAdr ) )
            {
                ScRefCellValue aCell(*pDok, aAdr);
                nVal = GetCellValue(aAdr, aCell);
            }
            else
                nVal = 0.0;
        }
        break;
        case svExternalSingleRef:
        {
            ScExternalRefCache::TokenRef pToken;
            PopExternalSingleRef(pToken);
            if (!nGlobalError)
            {
                if (pToken->GetType() == svDouble || pToken->GetType() == svEmptyCell)
                    nVal = pToken->GetDouble();
                else
                    nVal = ConvertStringToValue( pToken->GetString().getString());
            }
        }
        break;
        case svExternalDoubleRef:
        {
            ScMatrixRef pMat;
            PopExternalDoubleRef(pMat);
            if (nGlobalError)
                break;

            nVal = GetDoubleFromMatrix(pMat);
        }
        break;
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            nVal = GetDoubleFromMatrix(pMat);
        }
        break;
        case svError:
            PopError();
            nVal = 0.0;
        break;
        case svEmptyCell:
        case svMissing:
            Pop();
            nVal = 0.0;
        break;
        default:
            PopError();
            SetError( errIllegalParameter);
            nVal = 0.0;
    }
    if ( nFuncFmtType == nCurFmtType )
        nFuncFmtIndex = nCurFmtIndex;
    return nVal;
}

double ScInterpreter::GetDoubleWithDefault(double nDefault)
{
    bool bMissing = IsMissing();
    double nResultVal = GetDouble();
    if ( bMissing )
        nResultVal = nDefault;
    return nResultVal;
}

sal_Int32 ScInterpreter::GetInt32()
{
    double fVal = GetDouble();
    if (rtl::math::isNan(fVal))
    {
        SetError(errIllegalArgument);
        return SAL_MAX_INT32;
    }
    if (fVal > 0.0)
    {
        fVal = rtl::math::approxFloor( fVal);
        if (fVal > SAL_MAX_INT32)
        {
            SetError( errIllegalArgument);
            return SAL_MAX_INT32;
        }
    }
    else if (fVal < 0.0)
    {
        fVal = rtl::math::approxCeil( fVal);
        if (fVal < SAL_MIN_INT32)
        {
            SetError( errIllegalArgument);
            return SAL_MAX_INT32;
        }
    }
    return static_cast<sal_Int32>(fVal);
}

sal_Int32 ScInterpreter::GetInt32WithDefault( sal_Int32 nDefault )
{
    double fVal = GetDoubleWithDefault( nDefault);
    if (rtl::math::isNan(fVal))
    {
        SetError(errIllegalArgument);
        return SAL_MAX_INT32;
    }
    if (fVal > 0.0)
    {
        fVal = rtl::math::approxFloor( fVal);
        if (fVal > SAL_MAX_INT32)
        {
            SetError( errIllegalArgument);
            return SAL_MAX_INT32;
        }
    }
    else if (fVal < 0.0)
    {
        fVal = rtl::math::approxCeil( fVal);
        if (fVal < SAL_MIN_INT32)
        {
            SetError( errIllegalArgument);
            return SAL_MAX_INT32;
        }
    }
    return static_cast<sal_Int32>(fVal);
}

sal_Int16 ScInterpreter::GetInt16()
{
    double fVal = GetDouble();
    if (rtl::math::isNan(fVal))
    {
        SetError(errIllegalArgument);
        return SAL_MAX_INT16;
    }
    if (fVal > 0.0)
    {
        fVal = rtl::math::approxFloor( fVal);
        if (fVal > SAL_MAX_INT16)
        {
            SetError( errIllegalArgument);
            return SAL_MAX_INT16;
        }
    }
    else if (fVal < 0.0)
    {
        fVal = rtl::math::approxCeil( fVal);
        if (fVal < SAL_MIN_INT16)
        {
            SetError( errIllegalArgument);
            return SAL_MAX_INT16;
        }
    }
    return static_cast<sal_Int16>(fVal);
}

sal_uInt32 ScInterpreter::GetUInt32()
{
    double fVal = rtl::math::approxFloor( GetDouble());
    if (rtl::math::isNan(fVal) || fVal < 0.0 || fVal > SAL_MAX_UINT32)
    {
        SetError( errIllegalArgument);
        return SAL_MAX_UINT32;
    }
    return static_cast<sal_uInt32>(fVal);
}

bool ScInterpreter::GetDoubleOrString( double& rDouble, svl::SharedString& rString )
{
    bool bDouble = true;
    switch( GetRawStackType() )
    {
        case svDouble:
            rDouble = PopDouble();
        break;
        case svString:
            rString = PopString();
            bDouble = false;
        break;
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if (!PopDoubleRefOrSingleRef( aAdr))
            {
                rDouble = 0.0;
                return true;    // caller needs to check nGlobalError
            }
            ScRefCellValue aCell( *pDok, aAdr);
            if (aCell.hasNumeric())
            {
                rDouble = GetCellValue( aAdr, aCell);
            }
            else
            {
                GetCellString( rString, aCell);
                bDouble = false;
            }
        }
        break;
        case svExternalSingleRef:
        case svExternalDoubleRef:
        case svMatrix:
        {
            ScMatValType nType = GetDoubleOrStringFromMatrix( rDouble, rString);
            bDouble = ScMatrix::IsValueType( nType);
        }
        break;
        case svError:
            PopError();
            rDouble = 0.0;
        break;
        case svEmptyCell:
        case svMissing:
            Pop();
            rDouble = 0.0;
        break;
        default:
            PopError();
            SetError( errIllegalParameter);
            rDouble = 0.0;
    }
    if ( nFuncFmtType == nCurFmtType )
        nFuncFmtIndex = nCurFmtIndex;
    return bDouble;
}

svl::SharedString ScInterpreter::GetString()
{
    switch (GetRawStackType())
    {
        case svError:
            PopError();
            return svl::SharedString::getEmptyString();
        case svMissing:
        case svEmptyCell:
            Pop();
            return svl::SharedString::getEmptyString();
        case svDouble:
        {
            double fVal = PopDouble();
            sal_uLong nIndex = pFormatter->GetStandardFormat(
                                    css::util::NumberFormat::NUMBER,
                                    ScGlobal::eLnge);
            OUString aStr;
            pFormatter->GetInputLineString(fVal, nIndex, aStr);
            return mrStrPool.intern(aStr);
        }
        case svString:
            return PopString();
        case svSingleRef:
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            if (nGlobalError == 0)
            {
                ScRefCellValue aCell(*pDok, aAdr);
                svl::SharedString aSS;
                GetCellString(aSS, aCell);
                return aSS;
            }
            else
                return svl::SharedString::getEmptyString();
        }
        case svDoubleRef:
        {   // generate position dependent SingleRef
            ScRange aRange;
            PopDoubleRef( aRange );
            ScAddress aAdr;
            if ( !nGlobalError && DoubleRefToPosSingleRef( aRange, aAdr ) )
            {
                ScRefCellValue aCell(*pDok, aAdr);
                svl::SharedString aSS;
                GetCellString(aSS, aCell);
                return aSS;
            }
            else
                return svl::SharedString::getEmptyString();
        }
        case svExternalSingleRef:
        {
            ScExternalRefCache::TokenRef pToken;
            PopExternalSingleRef(pToken);
            if (nGlobalError)
                return svl::SharedString::getEmptyString();

            return pToken->GetString();
        }
        case svExternalDoubleRef:
        {
            ScMatrixRef pMat;
            PopExternalDoubleRef(pMat);
            return GetStringFromMatrix(pMat);
        }
        case svMatrix:
        {
            ScMatrixRef pMat = PopMatrix();
            return GetStringFromMatrix(pMat);
        }
        break;
        default:
            PopError();
            SetError( errIllegalArgument);
    }
    return svl::SharedString::getEmptyString();
}

svl::SharedString ScInterpreter::GetStringFromMatrix(const ScMatrixRef& pMat)
{
    if ( !pMat )
        ;   // nothing
    else if ( !pJumpMatrix )
    {
        return pMat->GetString( *pFormatter, 0, 0);
    }
    else
    {
        SCSIZE nCols, nRows, nC, nR;
        pMat->GetDimensions( nCols, nRows);
        pJumpMatrix->GetPos( nC, nR);
        // Use vector replication for single row/column arrays.
        if ( (nC < nCols || nCols == 1) && (nR < nRows || nRows == 1) )
            return pMat->GetString( *pFormatter, nC, nR);

        SetError( errNoValue);
    }
    return svl::SharedString::getEmptyString();
}

ScMatValType ScInterpreter::GetDoubleOrStringFromMatrix(
    double& rDouble, svl::SharedString& rString )
{

    rDouble = 0.0;
    rString = svl::SharedString::getEmptyString();
    ScMatValType nMatValType = SC_MATVAL_EMPTY;

    ScMatrixRef pMat;
    StackVar eType = GetStackType();
    if (eType == svExternalDoubleRef || eType == svExternalSingleRef || eType == svMatrix)
    {
        pMat = GetMatrix();
    }
    else
    {
        PopError();
        SetError( errIllegalParameter);
        return nMatValType;
    }

    ScMatrixValue nMatVal;
    if (!pMat)
    {
        // nothing
    }
    else if (!pJumpMatrix)
    {
        nMatVal = pMat->Get(0, 0);
        nMatValType = nMatVal.nType;
    }
    else
    {
        SCSIZE nCols, nRows, nC, nR;
        pMat->GetDimensions( nCols, nRows);
        pJumpMatrix->GetPos( nC, nR);
        // Use vector replication for single row/column arrays.
        if ( (nC < nCols || nCols == 1) && (nR < nRows || nRows == 1) )
        {
            nMatVal = pMat->Get( nC, nR);
            nMatValType = nMatVal.nType;
        }
        else
            SetError( errNoValue);
    }

    if (ScMatrix::IsValueType( nMatValType))
    {
        rDouble = nMatVal.fVal;
        sal_uInt16 nError = nMatVal.GetError();
        if (nError)
            SetError( nError);
    }
    else
    {
        rString = nMatVal.GetString();
    }

    return nMatValType;
}

void ScInterpreter::ScDBGet()
{
    bool bMissingField = false;
    unique_ptr<ScDBQueryParamBase> pQueryParam( GetDBParams(bMissingField) );
    if (!pQueryParam.get())
    {
        // Failed to create query param.
        PushIllegalParameter();
        return;
    }

    pQueryParam->mbSkipString = false;
    ScDBQueryDataIterator aValIter(pDok, pQueryParam.release());
    ScDBQueryDataIterator::Value aValue;
    if (!aValIter.GetFirst(aValue) || aValue.mnError)
    {
        // No match found.
        PushNoValue();
        return;
    }

    ScDBQueryDataIterator::Value aValNext;
    if (aValIter.GetNext(aValNext) && !aValNext.mnError)
    {
        // There should be only one unique match.
        PushIllegalArgument();
        return;
    }

    if (aValue.mbIsNumber)
        PushDouble(aValue.mfValue);
    else
        PushString(aValue.maString);
}

void ScInterpreter::ScExternal()
{
    sal_uInt8 nParamCount = GetByte();
    OUString aUnoName;
    OUString aFuncName( ScGlobal::pCharClass->uppercase( pCur->GetExternal() ) );
    LegacyFuncData* pLegacyFuncData = ScGlobal::GetLegacyFuncCollection()->findByName(aFuncName);
    if (pLegacyFuncData)
    {
        // Old binary non-UNO add-in function.
        // NOTE: parameter count is 1-based with the 0th "parameter" being the
        // return value, included in pLegacyFuncDatat->GetParamCount()
        if (nParamCount < MAXFUNCPARAM && nParamCount == pLegacyFuncData->GetParamCount() - 1)
        {
            ParamType   eParamType[MAXFUNCPARAM];
            void*       ppParam[MAXFUNCPARAM];
            double      nVal[MAXFUNCPARAM];
            sal_Char*   pStr[MAXFUNCPARAM];
            sal_uInt8*  pCellArr[MAXFUNCPARAM];
            short       i;

            for (i = 0; i < MAXFUNCPARAM; i++)
            {
                eParamType[i] = pLegacyFuncData->GetParamType(i);
                ppParam[i] = nullptr;
                nVal[i] = 0.0;
                pStr[i] = nullptr;
                pCellArr[i] = nullptr;
            }

            for (i = nParamCount; (i > 0) && (nGlobalError == 0); i--)
            {
                if (IsMissing())
                {
                    // Old binary Add-In can't distinguish between missing
                    // omitted argument and 0 (or any other value). Force
                    // error.
                    SetError( errParameterExpected);
                    break;  // for
                }
                switch (eParamType[i])
                {
                    case ParamType::PTR_DOUBLE :
                        {
                            nVal[i-1] = GetDouble();
                            ppParam[i] = &nVal[i-1];
                        }
                        break;
                    case ParamType::PTR_STRING :
                        {
                            OString aStr(OUStringToOString(GetString().getString(),
                                osl_getThreadTextEncoding()));
                            if ( aStr.getLength() >= ADDIN_MAXSTRLEN )
                                SetError( errStringOverflow );
                            else
                            {
                                pStr[i-1] = new sal_Char[ADDIN_MAXSTRLEN];
                                strncpy( pStr[i-1], aStr.getStr(), ADDIN_MAXSTRLEN );
                                pStr[i-1][ADDIN_MAXSTRLEN-1] = 0;
                                ppParam[i] = pStr[i-1];
                            }
                        }
                        break;
                    case ParamType::PTR_DOUBLE_ARR :
                        {
                            SCCOL nCol1;
                            SCROW nRow1;
                            SCTAB nTab1;
                            SCCOL nCol2;
                            SCROW nRow2;
                            SCTAB nTab2;
                            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                            pCellArr[i-1] = new sal_uInt8[MAXARRSIZE];
                            if (!CreateDoubleArr(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, pCellArr[i-1]))
                                SetError(errCodeOverflow);
                            else
                                ppParam[i] = pCellArr[i-1];
                        }
                        break;
                    case ParamType::PTR_STRING_ARR :
                        {
                            SCCOL nCol1;
                            SCROW nRow1;
                            SCTAB nTab1;
                            SCCOL nCol2;
                            SCROW nRow2;
                            SCTAB nTab2;
                            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                            pCellArr[i-1] = new sal_uInt8[MAXARRSIZE];
                            if (!CreateStringArr(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, pCellArr[i-1]))
                                SetError(errCodeOverflow);
                            else
                                ppParam[i] = pCellArr[i-1];
                        }
                        break;
                    case ParamType::PTR_CELL_ARR :
                        {
                            SCCOL nCol1;
                            SCROW nRow1;
                            SCTAB nTab1;
                            SCCOL nCol2;
                            SCROW nRow2;
                            SCTAB nTab2;
                            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                            pCellArr[i-1] = new sal_uInt8[MAXARRSIZE];
                            if (!CreateCellArr(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, pCellArr[i-1]))
                                SetError(errCodeOverflow);
                            else
                                ppParam[i] = pCellArr[i-1];
                        }
                        break;
                    default :
                        SetError(errIllegalParameter);
                        break;
                }
            }
            while ( i-- )
                Pop();      // In case of error (otherwise i==0) pop all parameters

            if (nGlobalError == 0)
            {
                if ( pLegacyFuncData->GetAsyncType() == ParamType::NONE )
                {
                    switch ( eParamType[0] )
                    {
                        case ParamType::PTR_DOUBLE :
                        {
                            double nErg = 0.0;
                            ppParam[0] = &nErg;
                            pLegacyFuncData->Call(ppParam);
                            PushDouble(nErg);
                        }
                        break;
                        case ParamType::PTR_STRING :
                        {
                            std::unique_ptr<sal_Char[]> pcErg(new sal_Char[ADDIN_MAXSTRLEN]);
                            ppParam[0] = pcErg.get();
                            pLegacyFuncData->Call(ppParam);
                            OUString aUni( pcErg.get(), strlen(pcErg.get()), osl_getThreadTextEncoding() );
                            PushString( aUni );
                        }
                        break;
                        default:
                            PushError( errUnknownState );
                    }
                }
                else
                {
                    // enable asyncs after loading
                    if ( rArr.IsRecalcModeNormal() )
                        rArr.SetExclusiveRecalcModeOnLoad();
                    // assure identical handler with identical call?
                    double nErg = 0.0;
                    ppParam[0] = &nErg;
                    pLegacyFuncData->Call(ppParam);
                    sal_uLong nHandle = sal_uLong( nErg );
                    if ( nHandle >= 65536 )
                    {
                        ScAddInAsync* pAs = ScAddInAsync::Get( nHandle );
                        if ( !pAs )
                        {
                            pAs = new ScAddInAsync(nHandle, pLegacyFuncData, pDok);
                            pMyFormulaCell->StartListening( *pAs );
                        }
                        else
                        {
                            pMyFormulaCell->StartListening( *pAs );
                            if ( !pAs->HasDocument( pDok ) )
                                pAs->AddDocument( pDok );
                        }
                        if ( pAs->IsValid() )
                        {
                            switch ( pAs->GetType() )
                            {
                                case ParamType::PTR_DOUBLE :
                                    PushDouble( pAs->GetValue() );
                                    break;
                                case ParamType::PTR_STRING :
                                    PushString( pAs->GetString() );
                                    break;
                                default:
                                    PushError( errUnknownState );
                            }
                        }
                        else
                            PushNA();
                    }
                    else
                        PushNoValue();
                }
            }

            for (i = 0; i < MAXFUNCPARAM; i++)
            {
                delete[] pStr[i];
                delete[] pCellArr[i];
            }
        }
        else
        {
            while( nParamCount-- > 0)
                Pop();
            PushIllegalParameter();
        }
    }
    else if ( !( aUnoName = ScGlobal::GetAddInCollection()->FindFunction(aFuncName, false) ).isEmpty()  )
    {
        //  bLocalFirst=false in FindFunction, cFunc should be the stored
        //  internal name

        ScUnoAddInCall aCall( *ScGlobal::GetAddInCollection(), aUnoName, nParamCount );

        if ( !aCall.ValidParamCount() )
            SetError( errIllegalParameter );

        if ( aCall.NeedsCaller() && !GetError() )
        {
            SfxObjectShell* pShell = pDok->GetDocumentShell();
            if (pShell)
                aCall.SetCallerFromObjectShell( pShell );
            else
            {
                // use temporary model object (without document) to supply options
                aCall.SetCaller( static_cast<beans::XPropertySet*>(
                                    new ScDocOptionsObj( pDok->GetDocOptions() ) ) );
            }
        }

        short nPar = nParamCount;
        while ( nPar > 0 && !GetError() )
        {
            --nPar;     // 0 .. (nParamCount-1)

            uno::Any aParam;
            if (IsMissing())
            {
                // Add-In has to explicitly handle an omitted empty missing
                // argument, do not default to anything like GetDouble() would
                // do (e.g. 0).
                Pop();
                aCall.SetParam( nPar, aParam );
                continue;   // while
            }

            sal_uInt8 nStackType = sal::static_int_cast<sal_uInt8>( GetStackType() );
            ScAddInArgumentType eType = aCall.GetArgType( nPar );
            switch (eType)
            {
                case SC_ADDINARG_INTEGER:
                    {
                        sal_Int32 nVal = GetInt32();
                        if (!nGlobalError)
                            aParam <<= nVal;
                    }
                    break;

                case SC_ADDINARG_DOUBLE:
                    aParam <<= (double) GetDouble();
                    break;

                case SC_ADDINARG_STRING:
                    aParam <<= GetString().getString();
                    break;

                case SC_ADDINARG_INTEGER_ARRAY:
                    switch( nStackType )
                    {
                        case svDouble:
                        case svString:
                        case svSingleRef:
                            {
                                sal_Int32 nVal = GetInt32();
                                if (!nGlobalError)
                                {
                                    uno::Sequence<sal_Int32> aInner( &nVal, 1 );
                                    uno::Sequence< uno::Sequence<sal_Int32> > aOuter( &aInner, 1 );
                                    aParam <<= aOuter;
                                }
                            }
                            break;
                        case svDoubleRef:
                            {
                                ScRange aRange;
                                PopDoubleRef( aRange );
                                if (!ScRangeToSequence::FillLongArray( aParam, pDok, aRange ))
                                    SetError(errIllegalParameter);
                            }
                            break;
                        case svMatrix:
                            if (!ScRangeToSequence::FillLongArray( aParam, PopMatrix().get() ))
                                SetError(errIllegalParameter);
                            break;
                        default:
                            PopError();
                            SetError(errIllegalParameter);
                    }
                    break;

                case SC_ADDINARG_DOUBLE_ARRAY:
                    switch( nStackType )
                    {
                        case svDouble:
                        case svString:
                        case svSingleRef:
                            {
                                double fVal = GetDouble();
                                uno::Sequence<double> aInner( &fVal, 1 );
                                uno::Sequence< uno::Sequence<double> > aOuter( &aInner, 1 );
                                aParam <<= aOuter;
                            }
                            break;
                        case svDoubleRef:
                            {
                                ScRange aRange;
                                PopDoubleRef( aRange );
                                if (!ScRangeToSequence::FillDoubleArray( aParam, pDok, aRange ))
                                    SetError(errIllegalParameter);
                            }
                            break;
                        case svMatrix:
                            if (!ScRangeToSequence::FillDoubleArray( aParam, PopMatrix().get() ))
                                SetError(errIllegalParameter);
                            break;
                        default:
                            PopError();
                            SetError(errIllegalParameter);
                    }
                    break;

                case SC_ADDINARG_STRING_ARRAY:
                    switch( nStackType )
                    {
                        case svDouble:
                        case svString:
                        case svSingleRef:
                            {
                                OUString aString = GetString().getString();
                                uno::Sequence<OUString> aInner( &aString, 1 );
                                uno::Sequence< uno::Sequence<OUString> > aOuter( &aInner, 1 );
                                aParam <<= aOuter;
                            }
                            break;
                        case svDoubleRef:
                            {
                                ScRange aRange;
                                PopDoubleRef( aRange );
                                if (!ScRangeToSequence::FillStringArray( aParam, pDok, aRange ))
                                    SetError(errIllegalParameter);
                            }
                            break;
                        case svMatrix:
                            if (!ScRangeToSequence::FillStringArray( aParam, PopMatrix().get(), pFormatter ))
                                SetError(errIllegalParameter);
                            break;
                        default:
                            PopError();
                            SetError(errIllegalParameter);
                    }
                    break;

                case SC_ADDINARG_MIXED_ARRAY:
                    switch( nStackType )
                    {
                        case svDouble:
                        case svString:
                        case svSingleRef:
                            {
                                uno::Any aElem;
                                if ( nStackType == svDouble )
                                    aElem <<= (double) GetDouble();
                                else if ( nStackType == svString )
                                    aElem <<= GetString().getString();
                                else
                                {
                                    ScAddress aAdr;
                                    if ( PopDoubleRefOrSingleRef( aAdr ) )
                                    {
                                        ScRefCellValue aCell(*pDok, aAdr);
                                        if (aCell.hasString())
                                        {
                                            svl::SharedString aStr;
                                            GetCellString(aStr, aCell);
                                            aElem <<= aStr.getString();
                                        }
                                        else
                                            aElem <<= GetCellValue(aAdr, aCell);
                                    }
                                }
                                uno::Sequence<uno::Any> aInner( &aElem, 1 );
                                uno::Sequence< uno::Sequence<uno::Any> > aOuter( &aInner, 1 );
                                aParam <<= aOuter;
                            }
                            break;
                        case svDoubleRef:
                            {
                                ScRange aRange;
                                PopDoubleRef( aRange );
                                if (!ScRangeToSequence::FillMixedArray( aParam, pDok, aRange ))
                                    SetError(errIllegalParameter);
                            }
                            break;
                        case svMatrix:
                            if (!ScRangeToSequence::FillMixedArray( aParam, PopMatrix().get() ))
                                SetError(errIllegalParameter);
                            break;
                        default:
                            PopError();
                            SetError(errIllegalParameter);
                    }
                    break;

                case SC_ADDINARG_VALUE_OR_ARRAY:
                    switch( nStackType )
                    {
                        case svDouble:
                            aParam <<= (double) GetDouble();
                            break;
                        case svString:
                            aParam <<= GetString().getString();
                            break;
                        case svSingleRef:
                            {
                                ScAddress aAdr;
                                if ( PopDoubleRefOrSingleRef( aAdr ) )
                                {
                                    ScRefCellValue aCell(*pDok, aAdr);
                                    if (aCell.hasString())
                                    {
                                        svl::SharedString aStr;
                                        GetCellString(aStr, aCell);
                                        aParam <<= aStr.getString();
                                    }
                                    else
                                        aParam <<= GetCellValue(aAdr, aCell);
                                }
                            }
                            break;
                        case svDoubleRef:
                            {
                                ScRange aRange;
                                PopDoubleRef( aRange );
                                if (!ScRangeToSequence::FillMixedArray( aParam, pDok, aRange ))
                                    SetError(errIllegalParameter);
                            }
                            break;
                        case svMatrix:
                            if (!ScRangeToSequence::FillMixedArray( aParam, PopMatrix().get() ))
                                SetError(errIllegalParameter);
                            break;
                        default:
                            PopError();
                            SetError(errIllegalParameter);
                    }
                    break;

                case SC_ADDINARG_CELLRANGE:
                    switch( nStackType )
                    {
                        case svSingleRef:
                            {
                                ScAddress aAdr;
                                PopSingleRef( aAdr );
                                ScRange aRange( aAdr );
                                uno::Reference<table::XCellRange> xObj =
                                        ScCellRangeObj::CreateRangeFromDoc( pDok, aRange );
                                if (xObj.is())
                                    aParam <<= xObj;
                                else
                                    SetError(errIllegalParameter);
                            }
                            break;
                        case svDoubleRef:
                            {
                                ScRange aRange;
                                PopDoubleRef( aRange );
                                uno::Reference<table::XCellRange> xObj =
                                        ScCellRangeObj::CreateRangeFromDoc( pDok, aRange );
                                if (xObj.is())
                                {
                                    aParam <<= xObj;
                                }
                                else
                                {
                                    SetError(errIllegalParameter);
                                }
                            }
                            break;
                        default:
                            PopError();
                            SetError(errIllegalParameter);
                    }
                    break;

                default:
                    PopError();
                    SetError(errIllegalParameter);
            }
            aCall.SetParam( nPar, aParam );
        }

        while (nPar-- > 0)
        {
            Pop();                  // in case of error, remove remaining args
        }
        if ( !GetError() )
        {
            aCall.ExecuteCall();

            if ( aCall.HasVarRes() )                        // handle async functions
            {
                if ( rArr.IsRecalcModeNormal() )
                {
                    rArr.SetExclusiveRecalcModeOnLoad();
                }
                uno::Reference<sheet::XVolatileResult> xRes = aCall.GetVarRes();
                ScAddInListener* pLis = ScAddInListener::Get( xRes );
                if ( !pLis )
                {
                    pLis = ScAddInListener::CreateListener( xRes, pDok );
                    pMyFormulaCell->StartListening( *pLis );
                }
                else
                {
                    pMyFormulaCell->StartListening( *pLis );
                    if ( !pLis->HasDocument( pDok ) )
                    {
                        pLis->AddDocument( pDok );
                    }
                }

                aCall.SetResult( pLis->GetResult() );       // use result from async
            }

            if ( aCall.GetErrCode() )
            {
                PushError( aCall.GetErrCode() );
            }
            else if ( aCall.HasMatrix() )
            {
                ScMatrixRef xMat = aCall.GetMatrix();
                PushMatrix( xMat );
            }
            else if ( aCall.HasString() )
            {
                PushString( aCall.GetString() );
            }
            else
            {
                PushDouble( aCall.GetValue() );
            }
        }
        else                // error...
            PushError( GetError());
    }
    else
    {
        while( nParamCount-- > 0)
        {
            Pop();
        }
        PushError( errNoAddin );
    }
}

void ScInterpreter::ScMissing()
{
    if ( aCode.IsEndOfPath() )
        PushTempToken( new ScEmptyCellToken( false, false ) );
    else
        PushTempToken( new FormulaMissingToken );
}

#if HAVE_FEATURE_SCRIPTING

static uno::Any lcl_getSheetModule( const uno::Reference<table::XCellRange>& xCellRange, ScDocument* pDok )
{
    uno::Reference< sheet::XSheetCellRange > xSheetRange( xCellRange, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xProps( xSheetRange->getSpreadsheet(), uno::UNO_QUERY_THROW );
    OUString sCodeName;
    xProps->getPropertyValue("CodeName") >>= sCodeName;
    // #TODO #FIXME ideally we should 'throw' here if we don't get a valid parent, but... it is possible
    // to create a module ( and use 'Option VBASupport 1' ) for a calc document, in this scenario there
    // are *NO* special document module objects ( of course being able to switch between vba/non vba mode at
    // the document in the future could fix this, especially IF the switching of the vba mode takes care to
    // create the special document module objects if they don't exist.
    BasicManager* pBasMgr = pDok->GetDocumentShell()->GetBasicManager();

    uno::Reference< uno::XInterface > xIf;
    if ( pBasMgr && !pBasMgr->GetName().isEmpty() )
    {
        OUString sProj( "Standard" );
        if ( !pDok->GetDocumentShell()->GetBasicManager()->GetName().isEmpty() )
        {
            sProj = pDok->GetDocumentShell()->GetBasicManager()->GetName();
        }
        StarBASIC* pBasic = pDok->GetDocumentShell()->GetBasicManager()->GetLib( sProj );
        if ( pBasic )
        {
            SbModule* pMod = pBasic->FindModule( sCodeName );
            if ( pMod )
            {
                xIf = pMod->GetUnoModule();
            }
        }
    }
    return uno::makeAny( xIf );
}

static bool lcl_setVBARange( ScRange& aRange, ScDocument* pDok, SbxVariable* pPar )
{
    bool bOk = false;
    try
    {
        uno::Reference< uno::XInterface > xVBARange;
        uno::Reference<table::XCellRange> xCellRange = ScCellRangeObj::CreateRangeFromDoc( pDok, aRange );
        uno::Sequence< uno::Any > aArgs(2);
        aArgs[0] = lcl_getSheetModule( xCellRange, pDok );
        aArgs[1] = uno::Any( xCellRange );
        xVBARange = ooo::vba::createVBAUnoAPIServiceWithArgs( pDok->GetDocumentShell(), "ooo.vba.excel.Range", aArgs );
        if ( xVBARange.is() )
        {
            SbxObjectRef aObj = GetSbUnoObject( "A-Range", uno::Any( xVBARange ) );
            SetSbUnoObjectDfltPropName( aObj );
            bOk = pPar->PutObject( aObj );
        }
    }
    catch( uno::Exception& )
    {
    }
    return bOk;
}

#endif

void ScInterpreter::ScMacro()
{

#if !HAVE_FEATURE_SCRIPTING
    PushNoValue();      // without DocShell no CallBasic
    return;
#else
    SbxBase::ResetError();

    sal_uInt8 nParamCount = GetByte();
    OUString aMacro( pCur->GetExternal() );

    SfxObjectShell* pDocSh = pDok->GetDocumentShell();
    if ( !pDocSh || !ScDocument::CheckMacroWarn() )
    {
        PushNoValue();      // without DocShell no CallBasic
        return;
    }

    //  no security queue beforehand (just CheckMacroWarn), moved to  CallBasic

    //  If the  Dok was loaded during a Basic-Calls,
    //  is the  Sbx-Objekt created(?)
//  pDocSh->GetSbxObject();

    //  search function with the name,
    //  then assemble  SfxObjectShell::CallBasic from aBasicStr, aMacroStr

    StarBASIC* pRoot;

    try
    {
        pRoot = pDocSh->GetBasic();
    }
    catch (...)
    {
        pRoot = nullptr;
    }

    SbxVariable* pVar = pRoot ? pRoot->Find(aMacro, SbxClassType::Method) : nullptr;
    if( !pVar || pVar->GetType() == SbxVOID || dynamic_cast<const SbMethod*>( pVar) ==  nullptr )
    {
        PushError( errNoMacro );
        return;
    }

    bool bVolatileMacro = false;
    SbMethod* pMethod = static_cast<SbMethod*>(pVar);

    SbModule* pModule = pMethod->GetModule();
    bool bUseVBAObjects = pModule->IsVBACompat();
    SbxObject* pObject = pModule->GetParent();
    OSL_ENSURE(dynamic_cast<const StarBASIC *>(pObject) != nullptr, "No Basic found!");
    OUString aMacroStr = pObject->GetName() + "." + pModule->GetName() + "." + pMethod->GetName();
    OUString aBasicStr;
    if (pObject->GetParent())
    {
        aBasicStr = pObject->GetParent()->GetName();    // Dokumentenbasic
    }
    else
    {
        aBasicStr = SfxGetpApp()->GetName();               // Applikationsbasic
    }
    //  Parameter-Array zusammenbauen

    SbxArrayRef refPar = new SbxArray;
    bool bOk = true;
    for( short i = nParamCount; i && bOk ; i-- )
    {
        SbxVariable* pPar = refPar->Get( (sal_uInt16) i );
        sal_uInt8 nStackType = sal::static_int_cast<sal_uInt8>( GetStackType() );
        switch( nStackType )
        {
            case svDouble:
                pPar->PutDouble( GetDouble() );
            break;
            case svString:
                pPar->PutString( GetString().getString() );
            break;
            case svExternalSingleRef:
            {
                ScExternalRefCache::TokenRef pToken;
                PopExternalSingleRef(pToken);
                if (nGlobalError)
                    bOk = false;
                else
                {
                    if ( pToken->GetType() == svString )
                        pPar->PutString( pToken->GetString().getString() );
                    else if ( pToken->GetType() == svDouble )
                        pPar->PutDouble( pToken->GetDouble() );
                    else
                    {
                        SetError( errIllegalArgument );
                        bOk = false;
                    }
                }
            }
            break;
            case svSingleRef:
            {
                ScAddress aAdr;
                PopSingleRef( aAdr );
                if ( bUseVBAObjects )
                {
                    ScRange aRange( aAdr );
                    bOk = lcl_setVBARange( aRange, pDok, pPar );
                }
                else
                {
                    bOk = SetSbxVariable( pPar, aAdr );
                }
            }
            break;
            case svDoubleRef:
            {
                SCCOL nCol1;
                SCROW nRow1;
                SCTAB nTab1;
                SCCOL nCol2;
                SCROW nRow2;
                SCTAB nTab2;
                PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                if( nTab1 != nTab2 )
                {
                    SetError( errIllegalParameter );
                    bOk = false;
                }
                else
                {
                    if ( bUseVBAObjects )
                    {
                        ScRange aRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                        bOk = lcl_setVBARange( aRange, pDok, pPar );
                    }
                    else
                    {
                        SbxDimArrayRef refArray = new SbxDimArray;
                        refArray->AddDim32( 1, nRow2 - nRow1 + 1 );
                        refArray->AddDim32( 1, nCol2 - nCol1 + 1 );
                        ScAddress aAdr( nCol1, nRow1, nTab1 );
                        for( SCROW nRow = nRow1; bOk && nRow <= nRow2; nRow++ )
                        {
                            aAdr.SetRow( nRow );
                            sal_Int32 nIdx[ 2 ];
                            nIdx[ 0 ] = nRow-nRow1+1;
                            for( SCCOL nCol = nCol1; bOk && nCol <= nCol2; nCol++ )
                            {
                                aAdr.SetCol( nCol );
                                nIdx[ 1 ] = nCol-nCol1+1;
                                SbxVariable* p = refArray->Get32( nIdx );
                                bOk = SetSbxVariable( p, aAdr );
                            }
                        }
                        pPar->PutObject( refArray );
                    }
                }
            }
            break;
            case svExternalDoubleRef:
            case svMatrix:
            {
                ScMatrixRef pMat = GetMatrix();
                SCSIZE nC, nR;
                if (pMat && !nGlobalError)
                {
                    pMat->GetDimensions(nC, nR);
                    SbxDimArrayRef refArray = new SbxDimArray;
                    refArray->AddDim32( 1, static_cast<sal_Int32>(nR) );
                    refArray->AddDim32( 1, static_cast<sal_Int32>(nC) );
                    for( SCSIZE nMatRow = 0; nMatRow < nR; nMatRow++ )
                    {
                        sal_Int32 nIdx[ 2 ];
                        nIdx[ 0 ] = static_cast<sal_Int32>(nMatRow+1);
                        for( SCSIZE nMatCol = 0; nMatCol < nC; nMatCol++ )
                        {
                            nIdx[ 1 ] = static_cast<sal_Int32>(nMatCol+1);
                            SbxVariable* p = refArray->Get32( nIdx );
                            if (pMat->IsString(nMatCol, nMatRow))
                            {
                                p->PutString( pMat->GetString(nMatCol, nMatRow).getString() );
                            }
                            else
                            {
                                p->PutDouble( pMat->GetDouble(nMatCol, nMatRow));
                            }
                        }
                    }
                    pPar->PutObject( refArray );
                }
                else
                {
                    SetError( errIllegalParameter );
                }
            }
            break;
            default:
                SetError( errIllegalParameter );
                bOk = false;
        }
    }
    if( bOk )
    {
        pDok->LockTable( aPos.Tab() );
        SbxVariableRef refRes = new SbxVariable;
        pDok->IncMacroInterpretLevel();
        ErrCode eRet = pDocSh->CallBasic( aMacroStr, aBasicStr, refPar, refRes );
        pDok->DecMacroInterpretLevel();
        pDok->UnlockTable( aPos.Tab() );

        ScMacroManager* pMacroMgr = pDok->GetMacroManager();
        if (pMacroMgr)
        {
            bVolatileMacro = pMacroMgr->GetUserFuncVolatile( pMethod->GetName() );
            pMacroMgr->AddDependentCell(pModule->GetName(), pMyFormulaCell);
        }

        SbxDataType eResType = refRes->GetType();
        if( SbxBase::GetError() )
        {
            SetError( errNoValue);
        }
        if ( eRet != ERRCODE_NONE )
        {
            PushNoValue();
        }
        else if( eResType >= SbxINTEGER && eResType <= SbxDOUBLE )
        {
            PushDouble( refRes->GetDouble() );
        }
        else if ( eResType & SbxARRAY )
        {
            SbxBase* pElemObj = refRes->GetObject();
            SbxDimArray* pDimArray = dynamic_cast< SbxDimArray *>( pElemObj );
            short nDim = pDimArray->GetDims();
            if ( 1 <= nDim && nDim <= 2 )
            {
                sal_Int32 nCs, nCe, nRs, nRe;
                SCSIZE nC, nR;
                SCCOL nColIdx;
                SCROW nRowIdx;
                if ( nDim == 1 )
                {   // array( cols )  one line, several columns
                    pDimArray->GetDim32( 1, nCs, nCe );
                    nC = static_cast<SCSIZE>(nCe - nCs + 1);
                    nRs = nRe = 0;
                    nR = 1;
                    nColIdx = 0;
                    nRowIdx = 1;
                }
                else
                {   // array( rows, cols )
                    pDimArray->GetDim32( 1, nRs, nRe );
                    nR = static_cast<SCSIZE>(nRe - nRs + 1);
                    pDimArray->GetDim32( 2, nCs, nCe );
                    nC = static_cast<SCSIZE>(nCe - nCs + 1);
                    nColIdx = 1;
                    nRowIdx = 0;
                }
                ScMatrixRef pMat = GetNewMat( nC, nR);
                if ( pMat )
                {
                    SbxVariable* pV;
                    SbxDataType eType;
                    for ( SCSIZE j=0; j < nR; j++ )
                    {
                        sal_Int32 nIdx[ 2 ];
                        //  in one-dimensional array( cols )  nIdx[1]
                        // from SbxDimArray::Get is ignored
                        nIdx[ nRowIdx ] = nRs + static_cast<sal_Int32>(j);
                        for ( SCSIZE i=0; i < nC; i++ )
                        {
                            nIdx[ nColIdx ] = nCs + static_cast<sal_Int32>(i);
                            pV = pDimArray->Get32( nIdx );
                            eType = pV->GetType();
                            if ( eType >= SbxINTEGER && eType <= SbxDOUBLE )
                            {
                                pMat->PutDouble( pV->GetDouble(), i, j );
                            }
                            else
                            {
                                pMat->PutString(mrStrPool.intern(pV->GetOUString()), i, j);
                            }
                        }
                    }
                    PushMatrix( pMat );
                }
                else
                {
                    PushIllegalArgument();
                }
            }
            else
            {
                PushNoValue();
            }
        }
        else
        {
            PushString( refRes->GetOUString() );
        }
    }

    if (bVolatileMacro && meVolatileType == NOT_VOLATILE)
        meVolatileType = VOLATILE_MACRO;
#endif
}

#if HAVE_FEATURE_SCRIPTING

bool ScInterpreter::SetSbxVariable( SbxVariable* pVar, const ScAddress& rPos )
{
    bool bOk = true;
    ScRefCellValue aCell(*pDok, rPos);
    if (!aCell.isEmpty())
    {
        sal_uInt16 nErr;
        double nVal;
        switch (aCell.meType)
        {
            case CELLTYPE_VALUE :
                nVal = GetValueCellValue(rPos, aCell.mfValue);
                pVar->PutDouble( nVal );
            break;
            case CELLTYPE_STRING :
            case CELLTYPE_EDIT :
                pVar->PutString(aCell.getString(pDok));
            break;
            case CELLTYPE_FORMULA :
                nErr = aCell.mpFormula->GetErrCode();
                if( !nErr )
                {
                    if (aCell.mpFormula->IsValue())
                    {
                        nVal = aCell.mpFormula->GetValue();
                        pVar->PutDouble( nVal );
                    }
                    else
                        pVar->PutString(aCell.mpFormula->GetString().getString());
                }
                else
                {
                    SetError( nErr );
                    bOk = false;
                }
                break;
            default :
                pVar->PutDouble( 0.0 );
        }
    }
    else
        pVar->PutDouble( 0.0 );
    return bOk;
}

#endif

namespace {

class FindByPointer : public ::std::unary_function<ScInterpreterTableOpParams, bool>
{
    const ScInterpreterTableOpParams* mpTableOp;
public:
    explicit FindByPointer(const ScInterpreterTableOpParams* p) : mpTableOp(p) {}
    bool operator() (std::unique_ptr<ScInterpreterTableOpParams> const& val) const
    {
        return val.get() == mpTableOp;
    }
};

}

void ScInterpreter::ScTableOp()
{
    sal_uInt8 nParamCount = GetByte();
    if (nParamCount != 3 && nParamCount != 5)
    {
        PushIllegalParameter();
        return;
    }
    ScInterpreterTableOpParams* pTableOp = new ScInterpreterTableOpParams;
    if (nParamCount == 5)
    {
        PopSingleRef( pTableOp->aNew2 );
        PopSingleRef( pTableOp->aOld2 );
    }
    PopSingleRef( pTableOp->aNew1 );
    PopSingleRef( pTableOp->aOld1 );
    PopSingleRef( pTableOp->aFormulaPos );

    pTableOp->bValid = true;
    pDok->m_TableOpList.push_back(
            std::unique_ptr<ScInterpreterTableOpParams>(pTableOp));
    pDok->IncInterpreterTableOpLevel();

    bool bReuseLastParams = (pDok->aLastTableOpParams == *pTableOp);
    if ( bReuseLastParams )
    {
        pTableOp->aNotifiedFormulaPos = pDok->aLastTableOpParams.aNotifiedFormulaPos;
        pTableOp->bRefresh = true;
        for ( ::std::vector< ScAddress >::const_iterator iBroadcast(
                    pTableOp->aNotifiedFormulaPos.begin() );
                iBroadcast != pTableOp->aNotifiedFormulaPos.end();
                ++iBroadcast )
        {   // emulate broadcast and indirectly collect cell pointers
            ScRefCellValue aCell(*pDok, *iBroadcast);
            if (aCell.meType == CELLTYPE_FORMULA)
                aCell.mpFormula->SetTableOpDirty();
        }
    }
    else
    {   // broadcast and indirectly collect cell pointers and positions
        pDok->SetTableOpDirty( pTableOp->aOld1 );
        if ( nParamCount == 5 )
            pDok->SetTableOpDirty( pTableOp->aOld2 );
    }
    pTableOp->bCollectNotifications = false;

    ScRefCellValue aCell(*pDok, pTableOp->aFormulaPos);
    if (aCell.meType == CELLTYPE_FORMULA)
        aCell.mpFormula->SetDirtyVar();
    if (aCell.hasNumeric())
    {
        PushDouble(GetCellValue(pTableOp->aFormulaPos, aCell));
    }
    else
    {
        svl::SharedString aCellString;
        GetCellString(aCellString, aCell);
        PushString( aCellString );
    }

    auto const itr =
        ::std::find_if(pDok->m_TableOpList.begin(), pDok->m_TableOpList.end(), FindByPointer(pTableOp));
    if (itr != pDok->m_TableOpList.end())
    {
        pTableOp = itr->release();
        pDok->m_TableOpList.erase(itr);
    }

    // set dirty again once more to be able to recalculate original
    for ( ::std::vector< ScFormulaCell* >::const_iterator iBroadcast(
                pTableOp->aNotifiedFormulaCells.begin() );
            iBroadcast != pTableOp->aNotifiedFormulaCells.end();
            ++iBroadcast )
    {
        (*iBroadcast)->SetTableOpDirty();
    }

    // save these params for next incarnation
    if ( !bReuseLastParams )
        pDok->aLastTableOpParams = *pTableOp;

    if (aCell.meType == CELLTYPE_FORMULA)
    {
        aCell.mpFormula->SetDirtyVar();
        aCell.mpFormula->GetErrCode();     // recalculate original
    }

    // Reset all dirty flags so next incarnation does really collect all cell
    // pointers during notifications and not just non-dirty ones, which may
    // happen if a formula cell is used by more than one TableOp block.
    for ( ::std::vector< ScFormulaCell* >::const_iterator iBroadcast2(
                pTableOp->aNotifiedFormulaCells.begin() );
            iBroadcast2 != pTableOp->aNotifiedFormulaCells.end();
            ++iBroadcast2 )
    {
        (*iBroadcast2)->ResetTableOpDirtyVar();
    }
    delete pTableOp;

    pDok->DecInterpreterTableOpLevel();
}

void ScInterpreter::ScDBArea()
{
    ScDBData* pDBData = pDok->GetDBCollection()->getNamedDBs().findByIndex(pCur->GetIndex());
    if (pDBData)
    {
        ScComplexRefData aRefData;
        aRefData.InitFlags();
        ScRange aRange;
        pDBData->GetArea(aRange);
        aRange.aEnd.SetTab(aRange.aStart.Tab());
        aRefData.SetRange(aRange, aPos);
        PushTempToken( new ScDoubleRefToken( aRefData ) );
    }
    else
        PushError( errNoName);
}

void ScInterpreter::ScColRowNameAuto()
{
    ScComplexRefData aRefData( *pCur->GetDoubleRef() );
    ScRange aAbs = aRefData.toAbs(aPos);
    if (!ValidRange(aAbs))
    {
        PushError( errNoRef );
        return;
    }

    SCsCOL nStartCol;
    SCsROW nStartRow;

    // maybe remember limit by using defined ColRowNameRange
    SCsCOL nCol2 = aAbs.aEnd.Col();
    SCsROW nRow2 = aAbs.aEnd.Row();
    // DataArea of the first cell
    nStartCol = aAbs.aStart.Col();
    nStartRow = aAbs.aStart.Row();
    aAbs.aEnd = aAbs.aStart; // Shrink to the top-left cell.

    {
        // Expand to the data area. Only modify the end position.
        SCCOL nDACol1 = aAbs.aStart.Col(), nDACol2 = aAbs.aEnd.Col();
        SCROW nDARow1 = aAbs.aStart.Row(), nDARow2 = aAbs.aEnd.Row();
        pDok->GetDataArea(aAbs.aStart.Tab(), nDACol1, nDARow1, nDACol2, nDARow2, true, false);
        aAbs.aEnd.SetCol(nDACol2);
        aAbs.aEnd.SetRow(nDARow2);
    }

    // corresponds with ScCompiler::GetToken
    if ( aRefData.Ref1.IsColRel() )
    {   // ColName
        aAbs.aEnd.SetCol(nStartCol);
        // maybe get previous limit by using defined ColRowNameRange
        if (aAbs.aEnd.Row() > nRow2)
            aAbs.aEnd.SetRow(nRow2);
        SCROW nMyRow;
        if ( aPos.Col() == nStartCol
          && nStartRow <= (nMyRow = aPos.Row()) && nMyRow <= aAbs.aEnd.Row())
        {   //Formula in the same column and within the range
            if ( nMyRow == nStartRow )
            {   // take the rest under the name
                nStartRow++;
                if ( nStartRow > MAXROW )
                    nStartRow = MAXROW;
                aAbs.aStart.SetRow(nStartRow);
            }
            else
            {   // below the name to the formula cell
                aAbs.aEnd.SetRow(nMyRow - 1);
            }
        }
    }
    else
    {   // RowName
        aAbs.aEnd.SetRow(nStartRow);
        // maybe get previous limit by using defined ColRowNameRange
        if (aAbs.aEnd.Col() > nCol2)
            aAbs.aEnd.SetCol(nCol2);
        SCCOL nMyCol;
        if ( aPos.Row() == nStartRow
          && nStartCol <= (nMyCol = aPos.Col()) && nMyCol <= aAbs.aEnd.Col())
        {   //Formula in the same column and within the range
            if ( nMyCol == nStartCol )
            {    // take the rest under the name
                nStartCol++;
                if ( nStartCol > MAXCOL )
                    nStartCol = MAXCOL;
                aAbs.aStart.SetCol(nStartCol);
            }
            else
            {   // below the name to the formula cell
                aAbs.aEnd.SetCol(nMyCol - 1);
            }
        }
    }
    aRefData.SetRange(aAbs, aPos);
    PushTempToken( new ScDoubleRefToken( aRefData ) );
}

// --- internals ------------------------------------------------------------

void ScInterpreter::ScTTT()
{   // temporary test, testing functions etc.
    sal_uInt8 nParamCount = GetByte();
    // do something, count down nParamCount with Pops!

    // clean up Stack
    while ( nParamCount-- > 0)
        Pop();
    PushError(errNoValue);
}

ScInterpreter::ScInterpreter( ScFormulaCell* pCell, ScDocument* pDoc,
        const ScAddress& rPos, ScTokenArray& r )
    : aCode(r)
    , aPos(rPos)
    , rArr(r)
    , pDok(pDoc)
    , mpLinkManager(pDok->GetLinkManager())
    , mrStrPool(pDoc->GetSharedStringPool())
    , pJumpMatrix(nullptr)
    , pTokenMatrixMap(nullptr)
    , pMyFormulaCell(pCell)
    , pFormatter(pDoc->GetFormatTable())
    , pCur(nullptr)
    , nGlobalError(0)
    , sp(0)
    , maxsp(0)
    , nFuncFmtIndex(0)
    , nCurFmtIndex(0)
    , nRetFmtIndex(0)
    , nFuncFmtType(0)
    , nCurFmtType(0)
    , nRetFmtType(0)
    , mnStringNoValueError(errNoValue)
    , mnSubTotalFlags(0)
    , cPar(0)
    , bCalcAsShown(pDoc->GetDocOptions().IsCalcAsShown())
    , meVolatileType(r.IsRecalcModeAlways() ? VOLATILE : NOT_VOLATILE)
{
    MergeCalcConfig();

    if(pMyFormulaCell)
    {
        sal_uInt8 cMatFlag = pMyFormulaCell->GetMatrixFlag();
        bMatrixFormula = ( cMatFlag == MM_FORMULA );
    }
    else
        bMatrixFormula = false;

    if (!bGlobalStackInUse)
    {
        bGlobalStackInUse = true;
        if (!pGlobalStack)
            pGlobalStack = new ScTokenStack;
        pStackObj = pGlobalStack;
    }
    else
    {
        pStackObj = new ScTokenStack;
    }
    pStack = pStackObj->pPointer;
}

ScInterpreter::~ScInterpreter()
{
    if ( pStackObj == pGlobalStack )
        bGlobalStackInUse = false;
    else
        delete pStackObj;
    delete pTokenMatrixMap;
}

ScCalcConfig& ScInterpreter::GetOrCreateGlobalConfig()
{
    if (!mpGlobalConfig)
        mpGlobalConfig = new ScCalcConfig();
    return *mpGlobalConfig;
}

void ScInterpreter::SetGlobalConfig(const ScCalcConfig& rConfig)
{
    GetOrCreateGlobalConfig() = rConfig;
}

const ScCalcConfig& ScInterpreter::GetGlobalConfig()
{
    return GetOrCreateGlobalConfig();
}

void ScInterpreter::MergeCalcConfig()
{
    maCalcConfig = GetOrCreateGlobalConfig();
    maCalcConfig.MergeDocumentSpecific( pDok->GetCalcConfig());
}

void ScInterpreter::GlobalExit()
{
    OSL_ENSURE(!bGlobalStackInUse, "who is still using the TokenStack?");
    DELETEZ(pGlobalStack);
}

namespace {

double applyImplicitIntersection(const sc::RangeMatrix& rMat, const ScAddress& rPos)
{
    if (rMat.mnRow1 <= rPos.Row() && rPos.Row() <= rMat.mnRow2 && rMat.mnCol1 == rMat.mnCol2)
    {
        SCROW nOffset = rPos.Row() - rMat.mnRow1;
        return rMat.mpMat->GetDouble(0, nOffset);
    }

    if (rMat.mnCol1 <= rPos.Col() && rPos.Col() <= rMat.mnCol2 && rMat.mnRow1 == rMat.mnRow2)
    {
        SCROW nOffset = rPos.Col() - rMat.mnCol1;
        return rMat.mpMat->GetDouble(nOffset, 0);
    }

    double fVal;
    rtl::math::setNan(&fVal);
    return fVal;
}

// Test for Functions that evaluate an error code and directly set nGlobalError to 0
bool IsErrFunc(OpCode oc)
{
    switch (oc)
    {
        case ocCount :
        case ocCount2 :
        case ocErrorType :
        case ocIsEmpty :
        case ocIsErr :
        case ocIsError :
        case ocIsFormula :
        case ocIsLogical :
        case ocIsNA :
        case ocIsNonString :
        case ocIsRef :
        case ocIsString :
        case ocIsValue :
        case ocN :
        case ocType :
        case ocIfError :
        case ocIfNA :
        case ocErrorType_ODF :
        case ocIfs_MS:
        case ocSwitch_MS:
            return true;
        default:
            return false;
    }
}

} //namespace

StackVar ScInterpreter::Interpret()
{
    short nRetTypeExpr = css::util::NumberFormat::UNDEFINED;
    sal_uLong nRetIndexExpr = 0;
    sal_uInt16 nErrorFunction = 0;
    sal_uInt16 nErrorFunctionCount = 0;
    std::stack<sal_uInt16> aErrorFunctionStack;
    sal_uInt16 nStackBase;

    nGlobalError = 0;
    nStackBase = sp = maxsp = 0;
    nRetFmtType = css::util::NumberFormat::UNDEFINED;
    nFuncFmtType = css::util::NumberFormat::UNDEFINED;
    nFuncFmtIndex = nCurFmtIndex = nRetFmtIndex = 0;
    xResult = nullptr;
    pJumpMatrix = nullptr;
    mnSubTotalFlags = 0x00;
    ScTokenMatrixMap::const_iterator aTokenMatrixMapIter;

    // Once upon a time we used to have FP exceptions on, and there was a
    // Windows printer driver that kept switching off exceptions, so we had to
    // switch them back on again every time. Who knows if there isn't a driver
    // that keeps switching exceptions on, now that we run with exceptions off,
    // so reassure exceptions are really off.
    SAL_MATH_FPEXCEPTIONS_OFF();

    aCode.Reset();
    while( ( pCur = aCode.Next() ) != nullptr
            && (!nGlobalError || nErrorFunction <= nErrorFunctionCount) )
    {
        OpCode eOp = pCur->GetOpCode();
        cPar = pCur->GetByte();
        if ( eOp == ocPush )
        {
            // RPN code push without error
            PushWithoutError( (FormulaToken&) *pCur );
        }
        else if (pTokenMatrixMap &&
                 !(eOp == ocIf || eOp == ocIfError || eOp == ocIfNA || eOp == ocChoose) &&
                ((aTokenMatrixMapIter = pTokenMatrixMap->find( pCur)) !=
                 pTokenMatrixMap->end()) &&
                (*aTokenMatrixMapIter).second->GetType() != svJumpMatrix)
        {
            // Path already calculated, reuse result.
            nStackBase = sp - pCur->GetParamCount();
            if ( nStackBase > sp )
                nStackBase = sp;        // underflow?!?
            sp = nStackBase;
            PushTempToken( (*aTokenMatrixMapIter).second.get());
        }
        else
        {
            // previous expression determines the current number format
            nCurFmtType = nRetTypeExpr;
            nCurFmtIndex = nRetIndexExpr;
            // default function's format, others are set if needed
            nFuncFmtType = css::util::NumberFormat::NUMBER;
            nFuncFmtIndex = 0;

            if ( eOp == ocIf || eOp == ocChoose || eOp == ocIfError || eOp == ocIfNA )
                nStackBase = sp;        // don't mess around with the jumps
            else
            {
                // Convert parameters to matrix if in array/matrix formula and
                // parameters of function indicate doing so. Create JumpMatrix
                // if necessary.
                if ( MatrixParameterConversion() )
                {
                    eOp = ocNone;       // JumpMatrix created
                    nStackBase = sp;
                }
                else
                    nStackBase = sp - pCur->GetParamCount();
            }
            if ( nStackBase > sp )
                nStackBase = sp;        // underflow?!?

            switch( eOp )
            {
                case ocSep:
                case ocClose:           // pushed by the compiler
                case ocMissing          : ScMissing();                  break;
                case ocMacro            : ScMacro();                    break;
                case ocDBArea           : ScDBArea();                   break;
                case ocColRowNameAuto   : ScColRowNameAuto();           break;
                case ocIf               : ScIfJump();                   break;
                case ocIfError          : ScIfError( false );           break;
                case ocIfNA             : ScIfError( true );            break;
                case ocChoose           : ScChooseJump();                break;
                case ocAdd              : ScAdd();                      break;
                case ocSub              : ScSub();                      break;
                case ocMul              : ScMul();                      break;
                case ocDiv              : ScDiv();                      break;
                case ocAmpersand        : ScAmpersand();                break;
                case ocPow              : ScPow();                      break;
                case ocEqual            : ScEqual();                    break;
                case ocNotEqual         : ScNotEqual();                 break;
                case ocLess             : ScLess();                     break;
                case ocGreater          : ScGreater();                  break;
                case ocLessEqual        : ScLessEqual();                break;
                case ocGreaterEqual     : ScGreaterEqual();             break;
                case ocAnd              : ScAnd();                      break;
                case ocOr               : ScOr();                       break;
                case ocXor              : ScXor();                      break;
                case ocIntersect        : ScIntersect();                break;
                case ocRange            : ScRangeFunc();                break;
                case ocUnion            : ScUnionFunc();                break;
                case ocNot              : ScNot();                      break;
                case ocNegSub           :
                case ocNeg              : ScNeg();                      break;
                case ocPercentSign      : ScPercentSign();              break;
                case ocPi               : ScPi();                       break;
                case ocRandom           : ScRandom();                   break;
                case ocTrue             : ScTrue();                     break;
                case ocFalse            : ScFalse();                    break;
                case ocGetActDate       : ScGetActDate();               break;
                case ocGetActTime       : ScGetActTime();               break;
                case ocNotAvail         : PushError( NOTAVAILABLE);     break;
                case ocDeg              : ScDeg();                      break;
                case ocRad              : ScRad();                      break;
                case ocSin              : ScSin();                      break;
                case ocCos              : ScCos();                      break;
                case ocTan              : ScTan();                      break;
                case ocCot              : ScCot();                      break;
                case ocArcSin           : ScArcSin();                   break;
                case ocArcCos           : ScArcCos();                   break;
                case ocArcTan           : ScArcTan();                   break;
                case ocArcCot           : ScArcCot();                   break;
                case ocSinHyp           : ScSinHyp();                   break;
                case ocCosHyp           : ScCosHyp();                   break;
                case ocTanHyp           : ScTanHyp();                   break;
                case ocCotHyp           : ScCotHyp();                   break;
                case ocArcSinHyp        : ScArcSinHyp();                break;
                case ocArcCosHyp        : ScArcCosHyp();                break;
                case ocArcTanHyp        : ScArcTanHyp();                break;
                case ocArcCotHyp        : ScArcCotHyp();                break;
                case ocCosecant         : ScCosecant();                 break;
                case ocSecant           : ScSecant();                   break;
                case ocCosecantHyp      : ScCosecantHyp();              break;
                case ocSecantHyp        : ScSecantHyp();                break;
                case ocExp              : ScExp();                      break;
                case ocLn               : ScLn();                       break;
                case ocLog10            : ScLog10();                    break;
                case ocSqrt             : ScSqrt();                     break;
                case ocFact             : ScFact();                     break;
                case ocGetYear          : ScGetYear();                  break;
                case ocGetMonth         : ScGetMonth();                 break;
                case ocGetDay           : ScGetDay();                   break;
                case ocGetDayOfWeek     : ScGetDayOfWeek();             break;
                case ocWeek             : ScGetWeekOfYear();            break;
                case ocIsoWeeknum       : ScGetIsoWeekOfYear();         break;
                case ocWeeknumOOo       : ScWeeknumOOo();               break;
                case ocEasterSunday     : ScEasterSunday();             break;
                case ocNetWorkdays      : ScNetWorkdays( false);        break;
                case ocNetWorkdays_MS   : ScNetWorkdays( true );        break;
                case ocWorkday_MS       : ScWorkday_MS();               break;
                case ocGetHour          : ScGetHour();                  break;
                case ocGetMin           : ScGetMin();                   break;
                case ocGetSec           : ScGetSec();                   break;
                case ocPlusMinus        : ScPlusMinus();                break;
                case ocAbs              : ScAbs();                      break;
                case ocInt              : ScInt();                      break;
                case ocEven             : ScEven();                     break;
                case ocOdd              : ScOdd();                      break;
                case ocPhi              : ScPhi();                      break;
                case ocGauss            : ScGauss();                    break;
                case ocStdNormDist      : ScStdNormDist();              break;
                case ocStdNormDist_MS   : ScStdNormDist_MS();           break;
                case ocFisher           : ScFisher();                   break;
                case ocFisherInv        : ScFisherInv();                break;
                case ocIsEmpty          : ScIsEmpty();                  break;
                case ocIsString         : ScIsString();                 break;
                case ocIsNonString      : ScIsNonString();              break;
                case ocIsLogical        : ScIsLogical();                break;
                case ocType             : ScType();                     break;
                case ocCell             : ScCell();                     break;
                case ocIsRef            : ScIsRef();                    break;
                case ocIsValue          : ScIsValue();                  break;
                case ocIsFormula        : ScIsFormula();                break;
                case ocFormula          : ScFormula();                  break;
                case ocIsNA             : ScIsNV();                     break;
                case ocIsErr            : ScIsErr();                    break;
                case ocIsError          : ScIsError();                  break;
                case ocIsEven           : ScIsEven();                   break;
                case ocIsOdd            : ScIsOdd();                    break;
                case ocN                : ScN();                        break;
                case ocGetDateValue     : ScGetDateValue();             break;
                case ocGetTimeValue     : ScGetTimeValue();             break;
                case ocCode             : ScCode();                     break;
                case ocTrim             : ScTrim();                     break;
                case ocUpper            : ScUpper();                    break;
                case ocProper           : ScProper();                   break;
                case ocLower            : ScLower();                    break;
                case ocLen              : ScLen();                      break;
                case ocT                : ScT();                        break;
                case ocClean            : ScClean();                    break;
                case ocValue            : ScValue();                    break;
                case ocNumberValue      : ScNumberValue();              break;
                case ocChar             : ScChar();                     break;
                case ocArcTan2          : ScArcTan2();                  break;
                case ocMod              : ScMod();                      break;
                case ocPower            : ScPower();                    break;
                case ocRound            : ScRound();                    break;
                case ocRoundUp          : ScRoundUp();                  break;
                case ocTrunc            :
                case ocRoundDown        : ScRoundDown();                break;
                case ocCeil             : ScCeil( true );               break;
                case ocCeil_MS          : ScCeil_MS();                  break;
                case ocCeil_Precise     :
                case ocCeil_ISO         : ScCeil_Precise();             break;
                case ocCeil_Math        : ScCeil( false );              break;
                case ocFloor            : ScFloor( true );              break;
                case ocFloor_MS         : ScFloor_MS();                 break;
                case ocFloor_Precise    : ScFloor_Precise();            break;
                case ocFloor_Math       : ScFloor( false );             break;
                case ocSumProduct       : ScSumProduct();               break;
                case ocSumSQ            : ScSumSQ();                    break;
                case ocSumX2MY2         : ScSumX2MY2();                 break;
                case ocSumX2DY2         : ScSumX2DY2();                 break;
                case ocSumXMY2          : ScSumXMY2();                  break;
                case ocRawSubtract      : ScRawSubtract();              break;
                case ocLog              : ScLog();                      break;
                case ocGCD              : ScGCD();                      break;
                case ocLCM              : ScLCM();                      break;
                case ocGetDate          : ScGetDate();                  break;
                case ocGetTime          : ScGetTime();                  break;
                case ocGetDiffDate      : ScGetDiffDate();              break;
                case ocGetDiffDate360   : ScGetDiffDate360();           break;
                case ocGetDateDif       : ScGetDateDif();               break;
                case ocMin              : ScMin()       ;               break;
                case ocMinA             : ScMin( true );                break;
                case ocMax              : ScMax();                      break;
                case ocMaxA             : ScMax( true );                break;
                case ocSum              : ScSum();                      break;
                case ocProduct          : ScProduct();                  break;
                case ocNPV              : ScNPV();                      break;
                case ocIRR              : ScIRR();                      break;
                case ocMIRR             : ScMIRR();                     break;
                case ocISPMT            : ScISPMT();                    break;
                case ocAverage          : ScAverage()       ;           break;
                case ocAverageA         : ScAverage( true );            break;
                case ocCount            : ScCount();                    break;
                case ocCount2           : ScCount2();                   break;
                case ocVar              :
                case ocVarS             : ScVar();                      break;
                case ocVarA             : ScVar( true );                break;
                case ocVarP             :
                case ocVarP_MS          : ScVarP();                     break;
                case ocVarPA            : ScVarP( true );               break;
                case ocStDev            :
                case ocStDevS           : ScStDev();                    break;
                case ocStDevA           : ScStDev( true );              break;
                case ocStDevP           :
                case ocStDevP_MS        : ScStDevP();                   break;
                case ocStDevPA          : ScStDevP( true );             break;
                case ocPV               : ScPV();                       break;
                case ocSYD              : ScSYD();                      break;
                case ocDDB              : ScDDB();                      break;
                case ocDB               : ScDB();                       break;
                case ocVBD              : ScVDB();                      break;
                case ocDuration         : ScDuration();                 break;
                case ocSLN              : ScSLN();                      break;
                case ocPMT              : ScPMT();                      break;
                case ocColumns          : ScColumns();                  break;
                case ocRows             : ScRows();                     break;
                case ocSheets           : ScSheets();                   break;
                case ocColumn           : ScColumn();                   break;
                case ocRow              : ScRow();                      break;
                case ocSheet            : ScSheet();                    break;
                case ocRRI              : ScRRI();                      break;
                case ocFV               : ScFV();                       break;
                case ocNper             : ScNper();                     break;
                case ocRate             : ScRate();                     break;
                case ocFilterXML        : ScFilterXML();                break;
                case ocWebservice       : ScWebservice();               break;
                case ocEncodeURL        : ScEncodeURL();                break;
                case ocColor            : ScColor();                    break;
                case ocErf_MS           : ScErf();                      break;
                case ocErfc_MS          : ScErfc();                     break;
                case ocIpmt             : ScIpmt();                     break;
                case ocPpmt             : ScPpmt();                     break;
                case ocCumIpmt          : ScCumIpmt();                  break;
                case ocCumPrinc         : ScCumPrinc();                 break;
                case ocEffect           : ScEffect();                   break;
                case ocNominal          : ScNominal();                  break;
                case ocSubTotal         : ScSubTotal();                 break;
                case ocAggregate        : ScAggregate();                break;
                case ocDBSum            : ScDBSum();                    break;
                case ocDBCount          : ScDBCount();                  break;
                case ocDBCount2         : ScDBCount2();                 break;
                case ocDBAverage        : ScDBAverage();                break;
                case ocDBGet            : ScDBGet();                    break;
                case ocDBMax            : ScDBMax();                    break;
                case ocDBMin            : ScDBMin();                    break;
                case ocDBProduct        : ScDBProduct();                break;
                case ocDBStdDev         : ScDBStdDev();                 break;
                case ocDBStdDevP        : ScDBStdDevP();                break;
                case ocDBVar            : ScDBVar();                    break;
                case ocDBVarP           : ScDBVarP();                   break;
                case ocIndirect         : ScIndirect();                 break;
                case ocAddress          : ScAddressFunc();              break;
                case ocMatch            : ScMatch();                    break;
                case ocCountEmptyCells  : ScCountEmptyCells();          break;
                case ocCountIf          : ScCountIf();                  break;
                case ocSumIf            : ScSumIf();                    break;
                case ocAverageIf        : ScAverageIf();                break;
                case ocSumIfs           : ScSumIfs();                   break;
                case ocAverageIfs       : ScAverageIfs();               break;
                case ocCountIfs         : ScCountIfs();                 break;
                case ocLookup           : ScLookup();                   break;
                case ocVLookup          : ScVLookup();                  break;
                case ocHLookup          : ScHLookup();                  break;
                case ocIndex            : ScIndex();                    break;
                case ocMultiArea        : ScMultiArea();                break;
                case ocOffset           : ScOffset();                   break;
                case ocAreas            : ScAreas();                    break;
                case ocCurrency         : ScCurrency();                 break;
                case ocReplace          : ScReplace();                  break;
                case ocFixed            : ScFixed();                    break;
                case ocFind             : ScFind();                     break;
                case ocExact            : ScExact();                    break;
                case ocLeft             : ScLeft();                     break;
                case ocRight            : ScRight();                    break;
                case ocSearch           : ScSearch();                   break;
                case ocMid              : ScMid();                      break;
                case ocText             : ScText();                     break;
                case ocSubstitute       : ScSubstitute();               break;
                case ocRept             : ScRept();                     break;
                case ocConcat           : ScConcat();                   break;
                case ocConcat_MS        : ScConcat_MS();                break;
                case ocTextJoin_MS      : ScTextJoin_MS();              break;
                case ocIfs_MS           : ScIfs_MS();                   break;
                case ocSwitch_MS        : ScSwitch_MS();                break;
                case ocMinIfs_MS        : ScMinIfs_MS();                break;
                case ocMaxIfs_MS        : ScMaxIfs_MS();                break;
                case ocMatValue         : ScMatValue();                 break;
                case ocMatrixUnit       : ScEMat();                     break;
                case ocMatDet           : ScMatDet();                   break;
                case ocMatInv           : ScMatInv();                   break;
                case ocMatMult          : ScMatMult();                  break;
                case ocMatTrans         : ScMatTrans();                 break;
                case ocMatRef           : ScMatRef();                   break;
                case ocB                : ScB();                        break;
                case ocNormDist         : ScNormDist( 3 );              break;
                case ocNormDist_MS      : ScNormDist( 4 );              break;
                case ocExpDist          :
                case ocExpDist_MS       : ScExpDist();                  break;
                case ocBinomDist        :
                case ocBinomDist_MS     : ScBinomDist();                break;
                case ocPoissonDist      :
                case ocPoissonDist_MS   : ScPoissonDist();              break;
                case ocCombin           : ScCombin();                   break;
                case ocCombinA          : ScCombinA();                  break;
                case ocPermut           : ScPermut();                   break;
                case ocPermutationA     : ScPermutationA();             break;
                case ocHypGeomDist      : ScHypGeomDist();              break;
                case ocHypGeomDist_MS   : ScHypGeomDist_MS();           break;
                case ocLogNormDist      : ScLogNormDist( 1 );           break;
                case ocLogNormDist_MS   : ScLogNormDist( 4 );           break;
                case ocTDist            : ScTDist();                    break;
                case ocTDist_MS         : ScTDist_MS();                 break;
                case ocTDist_RT         : ScTDist_T( 1 );               break;
                case ocTDist_2T         : ScTDist_T( 2 );               break;
                case ocFDist            :
                case ocFDist_RT         : ScFDist();                    break;
                case ocFDist_LT         : ScFDist_LT();                 break;
                case ocChiDist          :
                case ocChiDist_MS       : ScChiDist();                  break;
                case ocChiSqDist        : ScChiSqDist();                break;
                case ocChiSqDist_MS     : ScChiSqDist_MS();             break;
                case ocStandard         : ScStandard();                 break;
                case ocAveDev           : ScAveDev();                   break;
                case ocDevSq            : ScDevSq();                    break;
                case ocKurt             : ScKurt();                     break;
                case ocSkew             : ScSkew();                     break;
                case ocSkewp            : ScSkewp();                    break;
                case ocModalValue       : ScModalValue();               break;
                case ocModalValue_MS    : ScModalValue();               break;
                case ocModalValue_Multi : ScModalValue_Multi();         break;
                case ocMedian           : ScMedian();                   break;
                case ocGeoMean          : ScGeoMean();                  break;
                case ocHarMean          : ScHarMean();                  break;
                case ocWeibull          :
                case ocWeibull_MS       : ScWeibull();                  break;
                case ocBinomInv         :
                case ocCritBinom        : ScCritBinom();                break;
                case ocNegBinomVert     : ScNegBinomDist();             break;
                case ocNegBinomDist_MS  : ScNegBinomDist_MS();          break;
                case ocNoName           : ScNoName();                   break;
                case ocBad              : ScBadName();                  break;
                case ocZTest            :
                case ocZTest_MS         : ScZTest();                    break;
                case ocTTest            :
                case ocTTest_MS         : ScTTest();                    break;
                case ocFTest            :
                case ocFTest_MS         : ScFTest();                    break;
                case ocRank             :
                case ocRank_Eq          : ScRank( false );              break;
                case ocRank_Avg         : ScRank( true );               break;
                case ocPercentile       :
                case ocPercentile_Inc   : ScPercentile( true );         break;
                case ocPercentile_Exc   : ScPercentile( false );        break;
                case ocPercentrank      :
                case ocPercentrank_Inc  : ScPercentrank( true );        break;
                case ocPercentrank_Exc  : ScPercentrank( false );       break;
                case ocLarge            : ScLarge();                    break;
                case ocSmall            : ScSmall();                    break;
                case ocFrequency        : ScFrequency();                break;
                case ocQuartile         :
                case ocQuartile_Inc     : ScQuartile( true );           break;
                case ocQuartile_Exc     : ScQuartile( false );          break;
                case ocNormInv          :
                case ocNormInv_MS       : ScNormInv();                  break;
                case ocSNormInv         :
                case ocSNormInv_MS      : ScSNormInv();                 break;
                case ocConfidence       :
                case ocConfidence_N     : ScConfidence();               break;
                case ocConfidence_T     : ScConfidenceT();              break;
                case ocTrimMean         : ScTrimMean();                 break;
                case ocProb             : ScProbability();              break;
                case ocCorrel           : ScCorrel();                   break;
                case ocCovar            :
                case ocCovarianceP      : ScCovarianceP();              break;
                case ocCovarianceS      : ScCovarianceS();              break;
                case ocPearson          : ScPearson();                  break;
                case ocRSQ              : ScRSQ();                      break;
                case ocSTEYX            : ScSTEYX();                    break;
                case ocSlope            : ScSlope();                    break;
                case ocIntercept        : ScIntercept();                break;
                case ocTrend            : ScTrend();                    break;
                case ocGrowth           : ScGrowth();                   break;
                case ocLinest           : ScLinest();                   break;
                case ocLogest           : ScLogest();                   break;
                case ocForecast_LIN     :
                case ocForecast         : ScForecast();                   break;
                case ocForecast_ETS_ADD : ScForecast_Ets( etsAdd );       break;
                case ocForecast_ETS_SEA : ScForecast_Ets( etsSeason );    break;
                case ocForecast_ETS_MUL : ScForecast_Ets( etsMult );      break;
                case ocForecast_ETS_PIA : ScForecast_Ets( etsPIAdd );     break;
                case ocForecast_ETS_PIM : ScForecast_Ets( etsPIMult );    break;
                case ocForecast_ETS_STA : ScForecast_Ets( etsStatAdd );   break;
                case ocForecast_ETS_STM : ScForecast_Ets( etsStatMult );  break;
                case ocGammaLn          :
                case ocGammaLn_MS       : ScLogGamma();                 break;
                case ocGamma            : ScGamma();                    break;
                case ocGammaDist        : ScGammaDist( 3 );             break;
                case ocGammaDist_MS     : ScGammaDist( 4 );             break;
                case ocGammaInv         :
                case ocGammaInv_MS      : ScGammaInv();                 break;
                case ocChiTest          :
                case ocChiTest_MS       : ScChiTest();                  break;
                case ocChiInv           :
                case ocChiInv_MS        : ScChiInv();                   break;
                case ocChiSqInv         :
                case ocChiSqInv_MS      : ScChiSqInv();                 break;
                case ocTInv             :
                case ocTInv_2T          : ScTInv( 2 );                  break;
                case ocTInv_MS          : ScTInv( 4 );                  break;
                case ocFInv             :
                case ocFInv_RT          : ScFInv();                     break;
                case ocFInv_LT          : ScFInv_LT();                  break;
                case ocLogInv           :
                case ocLogInv_MS        : ScLogNormInv();               break;
                case ocBetaDist         : ScBetaDist();                 break;
                case ocBetaDist_MS      : ScBetaDist_MS();              break;
                case ocBetaInv          :
                case ocBetaInv_MS       : ScBetaInv();                  break;
                case ocExternal         : ScExternal();                 break;
                case ocTableOp          : ScTableOp();                  break;
                case ocStop :                                           break;
                case ocErrorType        : ScErrorType();                break;
                case ocErrorType_ODF    : ScErrorType_ODF();            break;
                case ocCurrent          : ScCurrent();                  break;
                case ocStyle            : ScStyle();                    break;
                case ocDde              : ScDde();                      break;
                case ocBase             : ScBase();                     break;
                case ocDecimal          : ScDecimal();                  break;
                case ocConvert          : ScConvert();                  break;
                case ocEuroConvert      : ScEuroConvert();              break;
                case ocRoman            : ScRoman();                    break;
                case ocArabic           : ScArabic();                   break;
                case ocInfo             : ScInfo();                     break;
                case ocHyperLink        : ScHyperLink();                break;
                case ocBahtText         : ScBahtText();                 break;
                case ocGetPivotData     : ScGetPivotData();             break;
                case ocJis              : ScJis();                      break;
                case ocAsc              : ScAsc();                      break;
                case ocLenB             : ScLenB();                     break;
                case ocRightB           : ScRightB();                   break;
                case ocLeftB            : ScLeftB();                    break;
                case ocMidB             : ScMidB();                     break;
                case ocUnicode          : ScUnicode();                  break;
                case ocUnichar          : ScUnichar();                  break;
                case ocBitAnd           : ScBitAnd();                   break;
                case ocBitOr            : ScBitOr();                    break;
                case ocBitXor           : ScBitXor();                   break;
                case ocBitRshift        : ScBitRshift();                break;
                case ocBitLshift        : ScBitLshift();                break;
                case ocTTT              : ScTTT();                      break;
                case ocDebugVar         : ScDebugVar();                 break;
                case ocNone : nFuncFmtType = css::util::NumberFormat::UNDEFINED;    break;
                default : PushError( errUnknownOpCode);                 break;
            }

            // If the function pushed a subroutine as result, continue with
            // execution of the subroutine.
            if (sp > nStackBase && pStack[sp-1]->GetOpCode() == ocCall
                /* && pStack[sp-1]->GetType() == svSubroutine */)
            {
                Pop(); continue;
            }

            if (FormulaCompiler::IsOpCodeVolatile(eOp))
                meVolatileType = VOLATILE;

            // Remember result matrix in case it could be reused.
            if (pTokenMatrixMap && sp && GetStackType() == svMatrix)
                pTokenMatrixMap->insert( ScTokenMatrixMap::value_type( pCur,
                            pStack[sp-1]));

            // outer function determines format of an expression
            if ( nFuncFmtType != css::util::NumberFormat::UNDEFINED )
            {
                nRetTypeExpr = nFuncFmtType;
                // inherit the format index only for currency formats
                nRetIndexExpr = ( nFuncFmtType == css::util::NumberFormat::CURRENCY ?
                    nFuncFmtIndex : 0 );
            }
        }

        // Need a clean stack environment for the JumpMatrix to work.
        if (nGlobalError && eOp != ocPush && sp > nStackBase + 1)
        {
            // Not all functions pop all parameters in case an error is
            // generated. Clean up stack. Assumes that every function pushes a
            // result, may be arbitrary in case of error.
            const FormulaToken* pLocalResult = pStack[ sp - 1 ];
            while (sp > nStackBase)
                Pop();
            PushTempToken( *pLocalResult );
        }

        bool bGotResult;
        do
        {
            bGotResult = false;
            sal_uInt8 nLevel = 0;
            if ( GetStackType( ++nLevel ) == svJumpMatrix )
                ;   // nothing
            else if ( GetStackType( ++nLevel ) == svJumpMatrix )
                ;   // nothing
            else
                nLevel = 0;
            if ( nLevel == 1 || (nLevel == 2 && aCode.IsEndOfPath()) )
            {
                if (nLevel == 1)
                    aErrorFunctionStack.push( nErrorFunction);
                // Restrict nLevel==1 to not prematurely discard a path result.
                if (nLevel == 2 || (!aCode.HasStacked() || aCode.IsEndOfPath()))
                {
                    bGotResult = JumpMatrix( nLevel );
                    if (aErrorFunctionStack.empty())
                        assert(!"ScInterpreter::Interpret - aErrorFunctionStack empty in JumpMatrix context");
                    else
                    {
                        nErrorFunction = aErrorFunctionStack.top();
                        if (bGotResult)
                            aErrorFunctionStack.pop();
                    }
                }
            }
            else
                pJumpMatrix = nullptr;
        } while ( bGotResult );

        if( IsErrFunc(eOp) )
            ++nErrorFunction;

        if ( nGlobalError )
        {
            if ( !nErrorFunctionCount )
            {   // count of errorcode functions in formula
                for ( FormulaToken* t = rArr.FirstRPN(); t; t = rArr.NextRPN() )
                {
                    if ( IsErrFunc(t->GetOpCode()) )
                        ++nErrorFunctionCount;
                }
            }
            if ( nErrorFunction >= nErrorFunctionCount )
                ++nErrorFunction;   // that's it, error => terminate
        }
    }

    // End: obtain result

    if( sp )
    {
        pCur = pStack[ sp-1 ];
        if( pCur->GetOpCode() == ocPush )
        {
            switch( pCur->GetType() )
            {
                case svEmptyCell:
                    ;   // nothing
                break;
                case svError:
                    nGlobalError = pCur->GetError();
                break;
                case svDouble :
                    {
                        // If typed, pop token to obtain type information and
                        // push a plain untyped double so the result token to
                        // be transfered to the formula cell result does not
                        // unnecessarily duplicate the information.
                        if (pCur->GetDoubleType())
                        {
                            const double fVal = PopDouble();
                            if (nCurFmtType != nFuncFmtType)
                                nRetIndexExpr = 0;  // carry format index only for matching type
                            nRetTypeExpr = nFuncFmtType = nCurFmtType;
                            PushTempToken( new FormulaDoubleToken( fVal));
                        }
                        if ( nFuncFmtType == css::util::NumberFormat::UNDEFINED )
                        {
                            nRetTypeExpr = css::util::NumberFormat::NUMBER;
                            nRetIndexExpr = 0;
                        }
                    }
                break;
                case svString :
                    nRetTypeExpr = css::util::NumberFormat::TEXT;
                    nRetIndexExpr = 0;
                break;
                case svSingleRef :
                {
                    ScAddress aAdr;
                    PopSingleRef( aAdr );
                    if( !nGlobalError )
                        PushCellResultToken( false, aAdr, &nRetTypeExpr, &nRetIndexExpr, true);
                }
                break;
                case svRefList :
                    PopError();     // maybe #REF! takes precedence over #VALUE!
                    PushError( errNoValue);
                break;
                case svDoubleRef :
                {
                    if ( bMatrixFormula )
                    {   // create matrix for {=A1:A5}
                        PopDoubleRefPushMatrix();
                        ScMatrixRef xMat = PopMatrix();
                        QueryMatrixType(xMat, nRetTypeExpr, nRetIndexExpr);
                    }
                    else
                    {
                        ScRange aRange;
                        PopDoubleRef( aRange );
                        ScAddress aAdr;
                        if ( !nGlobalError && DoubleRefToPosSingleRef( aRange, aAdr))
                            PushCellResultToken( false, aAdr, &nRetTypeExpr, &nRetIndexExpr, true);
                    }
                }
                break;
                case svExternalDoubleRef:
                {
                    ScMatrixRef xMat;
                    PopExternalDoubleRef(xMat);
                    QueryMatrixType(xMat, nRetTypeExpr, nRetIndexExpr);
                }
                break;
                case svMatrix :
                {
                    sc::RangeMatrix aMat = PopRangeMatrix();
                    if (aMat.isRangeValid())
                    {
                        // This matrix represents a range reference. Apply implicit intersection.
                        double fVal = applyImplicitIntersection(aMat, aPos);
                        if (rtl::math::isNan(fVal))
                            PushNoValue();
                        else
                            PushInt(fVal);
                    }
                    else
                        // This is a normal matrix.
                        QueryMatrixType(aMat.mpMat, nRetTypeExpr, nRetIndexExpr);
                }
                break;
                case svExternalSingleRef:
                {
                    ScExternalRefCache::TokenRef pToken;
                    ScExternalRefCache::CellFormat aFmt;
                    PopExternalSingleRef(pToken, &aFmt);
                    if (nGlobalError)
                        break;

                    PushTempToken(*pToken);

                    if (aFmt.mbIsSet)
                    {
                        nFuncFmtType = aFmt.mnType;
                        nFuncFmtIndex = aFmt.mnIndex;
                    }
                }
                break;
                default :
                    SetError( errUnknownStackVariable);
            }
        }
        else
            SetError( errUnknownStackVariable);
    }
    else
        SetError( errNoCode);

    if( nRetTypeExpr != css::util::NumberFormat::UNDEFINED )
    {
        nRetFmtType = nRetTypeExpr;
        nRetFmtIndex = nRetIndexExpr;
    }
    else if( nFuncFmtType != css::util::NumberFormat::UNDEFINED )
    {
        nRetFmtType = nFuncFmtType;
        nRetFmtIndex = nFuncFmtIndex;
    }
    else
        nRetFmtType = css::util::NumberFormat::NUMBER;

    if (nGlobalError && GetStackType() != svError )
        PushError( nGlobalError);

    // THE final result.
    xResult = PopToken();
    if (!xResult)
        xResult = new FormulaErrorToken( errUnknownStackVariable);

    // release tokens in expression stack
    FormulaToken** p = pStack;
    while( maxsp-- )
        (*p++)->DecRef();

    StackVar eType = xResult->GetType();
    if (eType == svMatrix)
        // Results are immutable in case they would be reused as input for new
        // interpreters.
        xResult.get()->GetMatrix()->SetImmutable( true);
    return eType;
}

void ScInterpreter::AssertFormulaMatrix()
{
    bMatrixFormula = true;
}

svl::SharedString ScInterpreter::GetStringResult() const
{
    return xResult->GetString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
