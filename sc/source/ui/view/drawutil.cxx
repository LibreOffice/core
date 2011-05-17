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




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
