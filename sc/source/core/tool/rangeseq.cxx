/*************************************************************************
 *
 *  $RCSfile: rangeseq.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-18 18:23:33 $
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

#include <svtools/zforlist.hxx>
#include <tools/solmath.hxx>
#include <tools/debug.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "rangeseq.hxx"
#include "document.hxx"
#include "scmatrix.hxx"
#include "cell.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

long lcl_DoubleToLong( double fVal )
{
    double fInt = (fVal >= 0.0) ? SolarMath::ApproxFloor( fVal ) :
                                  SolarMath::ApproxCeil( fVal );
    if ( fInt >= LONG_MIN && fInt <= LONG_MAX )
        return (long)fInt;
    else
        return 0.0;     // out of range
}

BOOL ScRangeToSequence::FillLongArray( uno::Any& rAny, ScDocument* pDoc, const ScRange& rRange )
{
    USHORT nTab = rRange.aStart.Tab();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    long nColCount = rRange.aEnd.Col() + 1 - rRange.aStart.Col();
    long nRowCount = rRange.aEnd.Row() + 1 - rRange.aStart.Row();

    uno::Sequence< uno::Sequence<INT32> > aRowSeq( nRowCount );
    uno::Sequence<INT32>* pRowAry = aRowSeq.getArray();
    for (long nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<INT32> aColSeq( nColCount );
        INT32* pColAry = aColSeq.getArray();
        for (long nCol = 0; nCol < nColCount; nCol++)
            pColAry[nCol] = lcl_DoubleToLong( pDoc->GetValue(
                ScAddress( (USHORT)(nStartCol+nCol), (USHORT)(nStartRow+nRow), nTab ) ) );

        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return TRUE;        //! check for errors
}


BOOL ScRangeToSequence::FillLongArray( uno::Any& rAny, const ScMatrix* pMatrix )
{
    if (!pMatrix)
        return FALSE;

    USHORT nColCount, nRowCount;
    pMatrix->GetDimensions( nColCount, nRowCount );

    uno::Sequence< uno::Sequence<INT32> > aRowSeq( nRowCount );
    uno::Sequence<INT32>* pRowAry = aRowSeq.getArray();
    for (USHORT nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<INT32> aColSeq( nColCount );
        INT32* pColAry = aColSeq.getArray();
        for (USHORT nCol = 0; nCol < nColCount; nCol++)
            if ( pMatrix->IsString( nCol, nRow ) )
                pColAry[nCol] = 0;
            else
                pColAry[nCol] = lcl_DoubleToLong( pMatrix->GetDouble( nCol, nRow ) );

        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScRangeToSequence::FillDoubleArray( uno::Any& rAny, ScDocument* pDoc, const ScRange& rRange )
{
    USHORT nTab = rRange.aStart.Tab();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
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
                ScAddress( (USHORT)(nStartCol+nCol), (USHORT)(nStartRow+nRow), nTab ) );

        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return TRUE;        //! check for errors
}


BOOL ScRangeToSequence::FillDoubleArray( uno::Any& rAny, const ScMatrix* pMatrix )
{
    if (!pMatrix)
        return FALSE;

    USHORT nColCount, nRowCount;
    pMatrix->GetDimensions( nColCount, nRowCount );

    uno::Sequence< uno::Sequence<double> > aRowSeq( nRowCount );
    uno::Sequence<double>* pRowAry = aRowSeq.getArray();
    for (USHORT nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<double> aColSeq( nColCount );
        double* pColAry = aColSeq.getArray();
        for (USHORT nCol = 0; nCol < nColCount; nCol++)
            if ( pMatrix->IsString( nCol, nRow ) )
                pColAry[nCol] = 0.0;
            else
                pColAry[nCol] = pMatrix->GetDouble( nCol, nRow );

        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return TRUE;
}

//------------------------------------------------------------------------

BOOL ScRangeToSequence::FillStringArray( uno::Any& rAny, ScDocument* pDoc, const ScRange& rRange )
{
    USHORT nTab = rRange.aStart.Tab();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    long nColCount = rRange.aEnd.Col() + 1 - rRange.aStart.Col();
    long nRowCount = rRange.aEnd.Row() + 1 - rRange.aStart.Row();

    String aDocStr;

    uno::Sequence< uno::Sequence<rtl::OUString> > aRowSeq( nRowCount );
    uno::Sequence<rtl::OUString>* pRowAry = aRowSeq.getArray();
    for (long nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<rtl::OUString> aColSeq( nColCount );
        rtl::OUString* pColAry = aColSeq.getArray();
        for (long nCol = 0; nCol < nColCount; nCol++)
        {
            pDoc->GetString( (USHORT)(nStartCol+nCol), (USHORT)(nStartRow+nRow), nTab, aDocStr );
            pColAry[nCol] = rtl::OUString( aDocStr );
        }
        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return TRUE;        //! check for errors
}


BOOL ScRangeToSequence::FillStringArray( uno::Any& rAny, const ScMatrix* pMatrix,
                                            SvNumberFormatter* pFormatter )
{
    if (!pMatrix)
        return FALSE;

    USHORT nColCount, nRowCount;
    pMatrix->GetDimensions( nColCount, nRowCount );

    uno::Sequence< uno::Sequence<rtl::OUString> > aRowSeq( nRowCount );
    uno::Sequence<rtl::OUString>* pRowAry = aRowSeq.getArray();
    for (USHORT nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<rtl::OUString> aColSeq( nColCount );
        rtl::OUString* pColAry = aColSeq.getArray();
        for (USHORT nCol = 0; nCol < nColCount; nCol++)
        {
            String aStr;
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
            pColAry[nCol] = rtl::OUString( aStr );
        }

        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return TRUE;
}

//------------------------------------------------------------------------

double lcl_GetValueFromCell( ScBaseCell& rCell )
{
    //! ScBaseCell member function?

    CellType eType = rCell.GetCellType();
    if ( eType == CELLTYPE_VALUE )
        return ((ScValueCell&)rCell).GetValue();
    else if ( eType == CELLTYPE_FORMULA )
        return ((ScFormulaCell&)rCell).GetValue();      // called only if result is value

    DBG_ERROR( "GetValueFromCell: wrong type" );
    return 0;
}

BOOL ScRangeToSequence::FillMixedArray( uno::Any& rAny, ScDocument* pDoc, const ScRange& rRange,
                                        BOOL bAllowNV )
{
    USHORT nTab = rRange.aStart.Tab();
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    long nColCount = rRange.aEnd.Col() + 1 - rRange.aStart.Col();
    long nRowCount = rRange.aEnd.Row() + 1 - rRange.aStart.Row();

    String aDocStr;
    BOOL bHasErrors = FALSE;

    uno::Sequence< uno::Sequence<uno::Any> > aRowSeq( nRowCount );
    uno::Sequence<uno::Any>* pRowAry = aRowSeq.getArray();
    for (long nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<uno::Any> aColSeq( nColCount );
        uno::Any* pColAry = aColSeq.getArray();
        for (long nCol = 0; nCol < nColCount; nCol++)
        {
            uno::Any& rElement = pColAry[nCol];

            ScAddress aPos( (USHORT)(nStartCol+nCol), (USHORT)(nStartRow+nRow), nTab );
            ScBaseCell* pCell = pDoc->GetCell( aPos );
            if ( pCell )
            {
                if ( pCell->GetCellType() == CELLTYPE_FORMULA &&
                        ((ScFormulaCell*)pCell)->GetErrCode() != 0 )
                {
                    // if NV is allowed, leave empty for errors
                    bHasErrors = TRUE;
                }
                else if ( pCell->HasValueData() )
                    rElement <<= (double) lcl_GetValueFromCell( *pCell );
                else
                    rElement <<= rtl::OUString( pCell->GetStringData() );
            }
            else
                rElement <<= rtl::OUString();       // empty: empty string
        }
        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return bAllowNV || !bHasErrors;
}


BOOL ScRangeToSequence::FillMixedArray( uno::Any& rAny, const ScMatrix* pMatrix )
{
    if (!pMatrix)
        return FALSE;

    USHORT nColCount, nRowCount;
    pMatrix->GetDimensions( nColCount, nRowCount );

    uno::Sequence< uno::Sequence<uno::Any> > aRowSeq( nRowCount );
    uno::Sequence<uno::Any>* pRowAry = aRowSeq.getArray();
    for (USHORT nRow = 0; nRow < nRowCount; nRow++)
    {
        uno::Sequence<uno::Any> aColSeq( nColCount );
        uno::Any* pColAry = aColSeq.getArray();
        for (USHORT nCol = 0; nCol < nColCount; nCol++)
        {
            if ( pMatrix->IsString( nCol, nRow ) )
            {
                String aStr;
                if ( !pMatrix->IsEmpty( nCol, nRow ) )
                    aStr = pMatrix->GetString( nCol, nRow );
                pColAry[nCol] <<= rtl::OUString( aStr );
            }
            else
                pColAry[nCol] <<= (double) pMatrix->GetDouble( nCol, nRow );
        }

        pRowAry[nRow] = aColSeq;
    }

    rAny <<= aRowSeq;
    return TRUE;
}


//------------------------------------------------------------------------



