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

#include <sal/config.h>

#include <o3tl/unit_conversion.hxx>
#include <vcl/outdev.hxx>

#include <drawutil.hxx>
#include <document.hxx>
#include <global.hxx>
#include <viewdata.hxx>

void ScDrawUtil::CalcScale( const ScDocument& rDoc, SCTAB nTab,
                            SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                            const OutputDevice* pDev,
                            const Fraction& rZoomX, const Fraction& rZoomY,
                            double nPPTX, double nPPTY,
                            Fraction& rScaleX, Fraction& rScaleY )
{
    tools::Long nPixelX = 0;
    tools::Long nTwipsX = 0;
    tools::Long nPixelY = 0;
    tools::Long nTwipsY = 0;
    for (SCCOL i=nStartCol; i<nEndCol; i++)
    {
        sal_uInt16 nWidth = rDoc.GetColWidth(i,nTab);
        nTwipsX += static_cast<tools::Long>(nWidth);
        nPixelX += ScViewData::ToPixel( nWidth, nPPTX );
    }

    for (SCROW nRow = nStartRow; nRow <= nEndRow-1; ++nRow)
    {
        SCROW nLastRow = nRow;
        if (rDoc.RowHidden(nRow, nTab, nullptr, &nLastRow))
        {
            nRow = nLastRow;
            continue;
        }

        sal_uInt16 nHeight = rDoc.GetRowHeight(nRow, nTab);
        nTwipsY += static_cast<tools::Long>(nHeight);
        nPixelY += ScViewData::ToPixel(nHeight, nPPTY);
    }

    MapMode aHMMMode( MapUnit::Map100thMM, Point(), rZoomX, rZoomY );
    Point aPixelLog = pDev->PixelToLogic( Point( nPixelX,nPixelY ), aHMMMode );

    //  Fraction(double) ctor can be used here (and avoid overflows of PixelLog * Zoom)
    //  because ReduceInaccurate is called later anyway.

    if ( aPixelLog.X() && nTwipsX )
        rScaleX = Fraction( static_cast<double>(aPixelLog.X()) *
                            static_cast<double>(rZoomX.GetNumerator()) /
                            o3tl::convert<double>(nTwipsX, o3tl::Length::twip, o3tl::Length::mm100) /
                            static_cast<double>(rZoomX.GetDenominator()) );
    else
        rScaleX = Fraction( 1, 1 );

    if ( aPixelLog.Y() && nTwipsY )
        rScaleY = Fraction( static_cast<double>(aPixelLog.Y()) *
                            static_cast<double>(rZoomY.GetNumerator()) /
                            o3tl::convert<double>(nTwipsY, o3tl::Length::twip, o3tl::Length::mm100) /
                            static_cast<double>(rZoomY.GetDenominator()) );
    else
        rScaleY = Fraction( 1, 1 );

    //  25 bits of accuracy are needed to always hit the right part of
    //  cells in the last rows (was 17 before 1M rows).
    rScaleX.ReduceInaccurate( 25 );
    rScaleY.ReduceInaccurate( 25 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
