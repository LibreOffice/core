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

#include <vcl/outdev.hxx>

#include "drawutil.hxx"
#include "document.hxx"
#include "global.hxx"
#include "viewdata.hxx"

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
        if (pDoc->RowHidden(nRow, nTab, nullptr, &nLastRow))
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
