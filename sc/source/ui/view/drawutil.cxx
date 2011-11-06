/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <vcl/outdev.hxx>

#include "drawutil.hxx"
#include "document.hxx"
#include "global.hxx"
#include "viewdata.hxx"

// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------


inline Fraction MakeFraction( long nA, long nB )
{
    return ( nA && nB ) ? Fraction(nA,nB) : Fraction(1,1);
}

void ScDrawUtil::CalcScale( ScDocument* pDoc, SCTAB nTab,
                            SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                            OutputDevice* pDev,
                            const Fraction& rZoomX, const Fraction& rZoomY,
                            double nPPTX, double nPPTY,
                            Fraction& rScaleX, Fraction& rScaleY )
{
    long nPixelX = 0;
    long nTwipsX = 0;
    long nPixelY = 0;
    long nTwipsY = 0;
    for (SCCOL i=nStartCol; i<nEndCol; i++)
    {
        sal_uInt16 nWidth = pDoc->GetColWidth(i,nTab);
        nTwipsX += (long) nWidth;
        nPixelX += ScViewData::ToPixel( nWidth, nPPTX );
    }

    for (SCROW nRow = nStartRow; nRow <= nEndRow-1; ++nRow)
    {
        SCROW nLastRow = nRow;
        if (pDoc->RowHidden(nRow, nTab, NULL, &nLastRow))
        {
            nRow = nLastRow;
            continue;
        }

        sal_uInt16 nHeight = pDoc->GetRowHeight(nRow, nTab);
        nTwipsY += static_cast<long>(nHeight);
        nPixelY += ScViewData::ToPixel(nHeight, nPPTY);
    }

    // #i116848# To get a large-enough number for PixelToLogic, multiply the integer values
    // instead of using a larger number of rows
    if ( nTwipsY )
    {
        long nMultiply = 2000000 / nTwipsY;
        if ( nMultiply > 1 )
        {
            nTwipsY *= nMultiply;
            nPixelY *= nMultiply;
        }
    }

    MapMode aHMMMode( MAP_100TH_MM, Point(), rZoomX, rZoomY );
    Point aPixelLog = pDev->PixelToLogic( Point( nPixelX,nPixelY ), aHMMMode );

    //  Fraction(double) ctor can be used here (and avoid overflows of PixelLog * Zoom)
    //  because ReduceInaccurate is called later anyway.

    if ( aPixelLog.X() && nTwipsX )
        rScaleX = Fraction( ((double)aPixelLog.X()) *
                            ((double)rZoomX.GetNumerator()) /
                            ((double)nTwipsX) /
                            ((double)HMM_PER_TWIPS) /
                            ((double)rZoomX.GetDenominator()) );
    else
        rScaleX = Fraction( 1, 1 );

    if ( aPixelLog.Y() && nTwipsY )
        rScaleY = Fraction( ((double)aPixelLog.Y()) *
                            ((double)rZoomY.GetNumerator()) /
                            ((double)nTwipsY) /
                            ((double)HMM_PER_TWIPS) /
                            ((double)rZoomY.GetDenominator()) );
    else
        rScaleY = Fraction( 1, 1 );

    //  25 bits of accuracy are needed to always hit the right part of
    //  cells in the last rows (was 17 before 1M rows).
    rScaleX.ReduceInaccurate( 25 );
    rScaleY.ReduceInaccurate( 25 );
}




