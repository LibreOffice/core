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

#include <svl/zforlist.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/string.hxx>
#include "rangeseq.hxx"
#include "document.hxx"
#include "dociter.hxx"
#include "scmatrix.hxx"
#include "formulacell.hxx"

using namespace com::sun::star;

static bool lcl_HasErrors( ScDocument* pDoc, const ScRange& rRange )
{
    // no need to look at empty cells - just use ScCellIterator
    ScCellIterator aIter( pDoc, rRange );
    for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
    {
        if (aIter.getType() != CELLTYPE_FORMULA)
            continue;

        ScFormulaCell* pCell = aIter.getFormulaCell();
        if (pCell->GetErrCode() != 0)
            return true;
    }
    return false;   // no error found
}

static long lcl_DoubleToLong( double fVal )
{
    double fInt = (fVal >= 0.0) ? ::rtl::math::approxFloor( fVal ) :
                                  ::rtl::math::approxCeil( fVal );
    if ( fInt >= LONG_MIN && fInt <= LONG_MAX )
        return (long)fInt;
    else
        return 0;       // out of range
}

sal_Bool ScRangeToSequence::FillLongArray( uno::Any& rAny, ScDocument* pDoc, const ScRange& rRange )
{
    SCTAB nTab = rRange.aStart.Tab();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    long nColCount = rRange.aEnd.Col() + 1 - rRange.aStart.Col();
    long nRowCount = rRange.aEnd.Row() + 1 - rRange.aStart.Row();

    uno::Sequence< uno::Sequence<sal_Int32> > aRowSeq( nRowCount );
    uno::Sequence<sal_Int32>* pRowAry = aRowSeq.getArray();
    for (long nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<sal_Int32> aColSeq( nColCount );
        sal_Int32* pColAry = aColSeq.getArray();
        for (long nCol = 0; nCol < nColCount; nCol++)
            pColAry[nCol] = lcl_DoubleToLong( pDoc->GetValue(
                ScAddress( (SCCOL)(nStartCol+nCol), (SCROW)(nStartRow+nRow), nTab ) ) );

        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return !lcl_HasErrors( pDoc, rRange );
}

sal_Bool ScRangeToSequence::FillLongArray( uno::Any& rAny, const ScMatrix* pMatrix )
{
    if (!pMatrix)
        return false;

    SCSIZE nColCount;
    SCSIZE nRowCount;
    pMatrix->GetDimensions( nColCount, nRowCount );

    uno::Sequence< uno::Sequence<sal_Int32> > aRowSeq( static_cast<sal_Int32>(nRowCount) );
    uno::Sequence<sal_Int32>* pRowAry = aRowSeq.getArray();
    for (SCSIZE nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<sal_Int32> aColSeq( static_cast<sal_Int32>(nColCount) );
        sal_Int32* pColAry = aColSeq.getArray();
        for (SCSIZE nCol = 0; nCol < nColCount; nCol++)
            if ( pMatrix->IsString( nCol, nRow ) )
                pColAry[nCol] = 0;
            else
                pColAry[nCol] = lcl_DoubleToLong( pMatrix->GetDouble( nCol, nRow ) );

        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return sal_True;
}

sal_Bool ScRangeToSequence::FillDoubleArray( uno::Any& rAny, ScDocument* pDoc, const ScRange& rRange )
{
    SCTAB nTab = rRange.aStart.Tab();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    long nColCount = rRange.aEnd.Col() + 1 - rRange.aStart.Col();
    long nRowCount = rRange.aEnd.Row() + 1 - rRange.aStart.Row();

    uno::Sequence< uno::Sequence<double> > aRowSeq( nRowCount );
    uno::Sequence<double>* pRowAry = aRowSeq.getArray();
    for (long nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<double> aColSeq( nColCount );
        double* pColAry = aColSeq.getArray();
        for (long nCol = 0; nCol < nColCount; nCol++)
            pColAry[nCol] = pDoc->GetValue(
                ScAddress( (SCCOL)(nStartCol+nCol), (SCROW)(nStartRow+nRow), nTab ) );

        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return !lcl_HasErrors( pDoc, rRange );
}

sal_Bool ScRangeToSequence::FillDoubleArray( uno::Any& rAny, const ScMatrix* pMatrix )
{
    if (!pMatrix)
        return false;

    SCSIZE nColCount;
    SCSIZE nRowCount;
    pMatrix->GetDimensions( nColCount, nRowCount );

    uno::Sequence< uno::Sequence<double> > aRowSeq( static_cast<sal_Int32>(nRowCount) );
    uno::Sequence<double>* pRowAry = aRowSeq.getArray();
    for (SCSIZE nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<double> aColSeq( static_cast<sal_Int32>(nColCount) );
        double* pColAry = aColSeq.getArray();
        for (SCSIZE nCol = 0; nCol < nColCount; nCol++)
            if ( pMatrix->IsString( nCol, nRow ) )
                pColAry[nCol] = 0.0;
            else
                pColAry[nCol] = pMatrix->GetDouble( nCol, nRow );

        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return sal_True;
}

sal_Bool ScRangeToSequence::FillStringArray( uno::Any& rAny, ScDocument* pDoc, const ScRange& rRange )
{
    SCTAB nTab = rRange.aStart.Tab();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    long nColCount = rRange.aEnd.Col() + 1 - rRange.aStart.Col();
    long nRowCount = rRange.aEnd.Row() + 1 - rRange.aStart.Row();

    bool bHasErrors = false;

    uno::Sequence< uno::Sequence<OUString> > aRowSeq( nRowCount );
    uno::Sequence<OUString>* pRowAry = aRowSeq.getArray();
    for (long nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<OUString> aColSeq( nColCount );
        OUString* pColAry = aColSeq.getArray();
        for (long nCol = 0; nCol < nColCount; nCol++)
        {
            sal_uInt16 nErrCode = pDoc->GetStringForFormula(
                        ScAddress((SCCOL)(nStartCol+nCol), (SCROW)(nStartRow+nRow), nTab),
                        pColAry[nCol] );
            if ( nErrCode != 0 )
                bHasErrors = true;
        }
        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return !bHasErrors;
}

sal_Bool ScRangeToSequence::FillStringArray( uno::Any& rAny, const ScMatrix* pMatrix,
                                            SvNumberFormatter* pFormatter )
{
    if (!pMatrix)
        return false;

    SCSIZE nColCount;
    SCSIZE nRowCount;
    pMatrix->GetDimensions( nColCount, nRowCount );

    uno::Sequence< uno::Sequence<OUString> > aRowSeq( static_cast<sal_Int32>(nRowCount) );
    uno::Sequence<OUString>* pRowAry = aRowSeq.getArray();
    for (SCSIZE nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<OUString> aColSeq( static_cast<sal_Int32>(nColCount) );
        OUString* pColAry = aColSeq.getArray();
        for (SCSIZE nCol = 0; nCol < nColCount; nCol++)
        {
            OUString aStr;
            if ( pMatrix->IsString( nCol, nRow ) )
            {
                if ( !pMatrix->IsEmpty( nCol, nRow ) )
                    aStr = pMatrix->GetString( nCol, nRow );
            }
            else if ( pFormatter )
            {
                double fVal = pMatrix->GetDouble( nCol, nRow );
                Color* pColor;
                pFormatter->GetOutputString( fVal, 0, aStr, &pColor );
            }
            pColAry[nCol] = aStr;
        }

        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return sal_True;
}

sal_Bool ScRangeToSequence::FillMixedArray( uno::Any& rAny, ScDocument* pDoc, const ScRange& rRange,
                                        sal_Bool bAllowNV )
{
    SCTAB nTab = rRange.aStart.Tab();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    long nColCount = rRange.aEnd.Col() + 1 - rRange.aStart.Col();
    long nRowCount = rRange.aEnd.Row() + 1 - rRange.aStart.Row();

    sal_Bool bHasErrors = false;

    uno::Sequence< uno::Sequence<uno::Any> > aRowSeq( nRowCount );
    uno::Sequence<uno::Any>* pRowAry = aRowSeq.getArray();
    for (long nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<uno::Any> aColSeq( nColCount );
        uno::Any* pColAry = aColSeq.getArray();
        for (long nCol = 0; nCol < nColCount; nCol++)
        {
            uno::Any& rElement = pColAry[nCol];

            ScAddress aPos( (SCCOL)(nStartCol+nCol), (SCROW)(nStartRow+nRow), nTab );
            ScRefCellValue aCell;
            aCell.assign(*pDoc, aPos);

            if (aCell.isEmpty())
            {
                rElement <<= EMPTY_OUSTRING;
                continue;
            }

            if (aCell.meType == CELLTYPE_FORMULA && aCell.mpFormula->GetErrCode() != 0)
            {
                // if NV is allowed, leave empty for errors
                bHasErrors = true;
            }
            else if (aCell.hasNumeric())
                rElement <<= aCell.getValue();
            else
                rElement <<= aCell.getString(pDoc);
        }
        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return bAllowNV || !bHasErrors;
}

sal_Bool ScRangeToSequence::FillMixedArray( uno::Any& rAny, const ScMatrix* pMatrix, bool bDataTypes )
{
    if (!pMatrix)
        return false;

    SCSIZE nColCount;
    SCSIZE nRowCount;
    pMatrix->GetDimensions( nColCount, nRowCount );

    uno::Sequence< uno::Sequence<uno::Any> > aRowSeq( static_cast<sal_Int32>(nRowCount) );
    uno::Sequence<uno::Any>* pRowAry = aRowSeq.getArray();
    for (SCSIZE nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<uno::Any> aColSeq( static_cast<sal_Int32>(nColCount) );
        uno::Any* pColAry = aColSeq.getArray();
        for (SCSIZE nCol = 0; nCol < nColCount; nCol++)
        {
            if ( pMatrix->IsString( nCol, nRow ) )
            {
                String aStr;
                if ( !pMatrix->IsEmpty( nCol, nRow ) )
                    aStr = pMatrix->GetString( nCol, nRow );
                pColAry[nCol] <<= OUString( aStr );
            }
            else
            {
                double fVal = pMatrix->GetDouble( nCol, nRow );
                if (bDataTypes && pMatrix->IsBoolean( nCol, nRow ))
                    pColAry[nCol] <<= (fVal ? true : false);
                else
                    pColAry[nCol] <<= fVal;
            }
        }

        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return sal_True;
}

bool ScApiTypeConversion::ConvertAnyToDouble( double & o_fVal,
        com::sun::star::uno::TypeClass & o_eClass,
        const com::sun::star::uno::Any & rAny )
{
    bool bRet = false;
    o_eClass = rAny.getValueTypeClass();
    switch (o_eClass)
    {
        //! extract integer values
        case uno::TypeClass_ENUM:
        case uno::TypeClass_BOOLEAN:
        case uno::TypeClass_CHAR:
        case uno::TypeClass_BYTE:
        case uno::TypeClass_SHORT:
        case uno::TypeClass_UNSIGNED_SHORT:
        case uno::TypeClass_LONG:
        case uno::TypeClass_UNSIGNED_LONG:
        case uno::TypeClass_FLOAT:
        case uno::TypeClass_DOUBLE:
            rAny >>= o_fVal;
            bRet = true;
            break;
        default:
            ;   // nothing, avoid warning
    }
    if (!bRet)
        o_fVal = 0.0;
    return bRet;
}

ScMatrixRef ScSequenceToMatrix::CreateMixedMatrix( const com::sun::star::uno::Any & rAny )
{
    ScMatrixRef xMatrix;
    uno::Sequence< uno::Sequence< uno::Any > > aSequence;
    if ( rAny >>= aSequence )
    {
        sal_Int32 nRowCount = aSequence.getLength();
        const uno::Sequence<uno::Any>* pRowArr = aSequence.getConstArray();
        sal_Int32 nMaxColCount = 0;
        sal_Int32 nCol, nRow;
        for (nRow=0; nRow<nRowCount; nRow++)
        {
            sal_Int32 nTmp = pRowArr[nRow].getLength();
            if ( nTmp > nMaxColCount )
                nMaxColCount = nTmp;
        }
        if ( nMaxColCount && nRowCount )
        {
            OUString aUStr;
            xMatrix = new ScMatrix(
                    static_cast<SCSIZE>(nMaxColCount),
                    static_cast<SCSIZE>(nRowCount), 0.0);
            SCSIZE nCols, nRows;
            xMatrix->GetDimensions( nCols, nRows);
            if (nCols != static_cast<SCSIZE>(nMaxColCount) || nRows != static_cast<SCSIZE>(nRowCount))
            {
                OSL_FAIL( "ScSequenceToMatrix::CreateMixedMatrix: matrix exceeded max size, returning NULL matrix");
                return NULL;
            }
            for (nRow=0; nRow<nRowCount; nRow++)
            {
                sal_Int32 nColCount = pRowArr[nRow].getLength();
                const uno::Any* pColArr = pRowArr[nRow].getConstArray();
                for (nCol=0; nCol<nColCount; nCol++)
                {
                    double fVal;
                    uno::TypeClass eClass;
                    if (ScApiTypeConversion::ConvertAnyToDouble( fVal, eClass, pColArr[nCol]))
                    {
                        if (eClass == uno::TypeClass_BOOLEAN)
                            xMatrix->PutBoolean( (fVal ? true : false),
                                    static_cast<SCSIZE>(nCol),
                                    static_cast<SCSIZE>(nRow) );
                        else
                            xMatrix->PutDouble( fVal,
                                    static_cast<SCSIZE>(nCol),
                                    static_cast<SCSIZE>(nRow) );
                    }
                    else
                    {
                        // Try string, else use empty as last resort.

                        if ( pColArr[nCol] >>= aUStr )
                            xMatrix->PutString( String( aUStr ),
                                    static_cast<SCSIZE>(nCol),
                                    static_cast<SCSIZE>(nRow) );
                        else
                            xMatrix->PutEmpty(
                                    static_cast<SCSIZE>(nCol),
                                    static_cast<SCSIZE>(nRow) );
                    }
                }
                for (nCol=nColCount; nCol<nMaxColCount; nCol++)
                {
                    xMatrix->PutEmpty(
                            static_cast<SCSIZE>(nCol),
                            static_cast<SCSIZE>(nRow) );
                }
            }
        }
    }
    return xMatrix;
}

sal_Bool ScByteSequenceToString::GetString( OUString& rString, const uno::Any& rAny,
                                        sal_uInt16 nEncoding )
{
    uno::Sequence<sal_Int8> aSeq;
    if ( rAny >>= aSeq )
    {
        rString = OUString( (const sal_Char*)aSeq.getConstArray(),
                            aSeq.getLength(), nEncoding );
        rString = comphelper::string::stripEnd(rString, 0);
        return sal_True;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
