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

#include <vcl/lineinfo.hxx>
#include <vcl/outdev.hxx>

#include <gridmerg.hxx>

#define PAGEBREAK_LINE_DISTANCE_PIXEL 5
#define PAGEBREAK_LINE_DASH_LEN_PIXEL 5
#define PAGEBREAK_LINE_DASH_COUNT 1

ScGridMerger::ScGridMerger( OutputDevice* pOutDev, long nOnePixelX, long nOnePixelY )
    : pDev(pOutDev)
    , nOneX(nOnePixelX)
    , nOneY(nOnePixelY)
    , nFixStart(0)
    , nFixEnd(0)
    , nVarStart(0)
    , nVarDiff(0)
    , nCount(0)
    , bVertical(false)
{
    //  optimize (DrawGrid) only for pixel MapMode,
    //  to avoid rounding errors

    bOptimize = ( pDev->GetMapMode().GetMapUnit() == MapUnit::MapPixel );
}

ScGridMerger::~ScGridMerger()
{
    Flush();
}

void ScGridMerger::AddLine( long nStart, long nEnd, long nPos )
{
    if ( nCount )
    {
        //  not first line - test fix position
        //  more than one previous line - test distance

        if ( nStart != nFixStart || nEnd != nFixEnd )
        {
            if ( nCount == 1 && nPos == nVarStart &&
                    ( nStart == nFixEnd ||
                        nStart == nFixEnd + ( bVertical ? nOneY : nOneX ) ) )
            {
                //  additional optimization: extend connected lines
                //  keep nCount at 1
                nFixEnd = nEnd;
            }
            else
                Flush();
        }
        else if ( nCount == 1 )
        {
            nVarDiff = nPos - nVarStart;
            ++nCount;
        }
        else if ( nPos != nVarStart + nCount * nVarDiff )       //! keep VarEnd?
            Flush();
        else
            ++nCount;
    }

    if ( !nCount )
    {
        //  first line (or flushed above) - just store

        nFixStart = nStart;
        nFixEnd   = nEnd;
        nVarStart = nPos;
        nVarDiff  = 0;
        nCount    = 1;
    }
}

void ScGridMerger::AddHorLine(bool bWorksInPixels, long nX1, long nX2, long nY, bool bDashed)
{
    if ( bWorksInPixels )
    {
        Point aPoint(pDev->PixelToLogic(Point(nX1, nY)));
        nX1 = aPoint.X();
        nY = aPoint.Y();
        nX2 = pDev->PixelToLogic(Point(nX2, 0)).X();
    }

    if ( bDashed )
    {
        // If there are some unflushed lines they must be flushed since
        // new line is of different style
        if (bOptimize) {
            Flush();
            bVertical = false;
        }

        LineInfo aLineInfo(LineStyle::Dash, 1);
        aLineInfo.SetDashCount( PAGEBREAK_LINE_DASH_COUNT );

        // Calculating logic values of DashLen and Distance from fixed pixel values
        Size aDashDistanceLen( pDev->PixelToLogic( Size( PAGEBREAK_LINE_DISTANCE_PIXEL,
                                                         PAGEBREAK_LINE_DASH_LEN_PIXEL )));

        aLineInfo.SetDistance( aDashDistanceLen.Width() );
        aLineInfo.SetDashLen( aDashDistanceLen.Height() );

        pDev->DrawLine( Point( nX1, nY ), Point( nX2, nY ), aLineInfo );
    }
    else if ( bOptimize )
    {
        if ( bVertical )
        {
            Flush();
            bVertical = false;
        }
        AddLine( nX1, nX2, nY );
    }
    else
        pDev->DrawLine( Point( nX1, nY ), Point( nX2, nY ) );
}

void ScGridMerger::AddVerLine(bool bWorksInPixels, long nX, long nY1, long nY2, bool bDashed)
{
    if (bWorksInPixels)
    {
        Point aPoint(pDev->PixelToLogic(Point(nX, nY1)));
        nX = aPoint.X();
        nY1 = aPoint.Y();
        nY2 = pDev->PixelToLogic(Point(0, nY2)).Y();
    }

    if ( bDashed )
    {
        // If there are some unflushed lines they must be flushed since
        // new line is of different style
        if (bOptimize) {
            Flush();
            bVertical = false;
        }

        LineInfo aLineInfo(LineStyle::Dash, 1);
        aLineInfo.SetDashCount( PAGEBREAK_LINE_DASH_COUNT );

        // Calculating logic values of DashLen and Distance from fixed pixel values
        Size aDashDistanceLen( pDev->PixelToLogic( Size( PAGEBREAK_LINE_DISTANCE_PIXEL,
                                                         PAGEBREAK_LINE_DASH_LEN_PIXEL )));

        aLineInfo.SetDistance( aDashDistanceLen.Width() );
        aLineInfo.SetDashLen( aDashDistanceLen.Height() );

        pDev->DrawLine( Point( nX, nY1 ), Point( nX, nY2 ), aLineInfo);
    }
    else if ( bOptimize )
    {
        if ( !bVertical )
        {
            Flush();
            bVertical = true;
        }
        AddLine( nY1, nY2, nX );
    }
    else
        pDev->DrawLine( Point( nX, nY1 ), Point( nX, nY2 ) );
}

void ScGridMerger::Flush()
{
    if (!nCount)
        return;

    if (bVertical)
    {
        if ( nCount == 1 )
            pDev->DrawLine( Point( nVarStart, nFixStart ), Point( nVarStart, nFixEnd ) );
        else
        {
            long nVarEnd = nVarStart + ( nCount - 1 ) * nVarDiff;
            if ( nVarDiff < 0 )
            {
                //  nVarDiff is negative in RTL layout mode
                //  Change the positions so DrawGrid is called with a positive distance
                //  (nVarStart / nVarDiff can be modified, aren't used after Flush)

                nVarDiff = -nVarDiff;
                long nTemp = nVarStart;
                nVarStart = nVarEnd;
                nVarEnd = nTemp;
            }
            pDev->DrawGrid( tools::Rectangle( nVarStart, nFixStart, nVarEnd, nFixEnd ),
                            Size( nVarDiff, nFixEnd - nFixStart ),
                            DrawGridFlags::VertLines );
        }
    }
    else
    {
        if ( nCount == 1 )
            pDev->DrawLine( Point( nFixStart, nVarStart ), Point( nFixEnd, nVarStart ) );
        else
        {
            long nVarEnd = nVarStart + ( nCount - 1 ) * nVarDiff;
            pDev->DrawGrid( tools::Rectangle( nFixStart, nVarStart, nFixEnd, nVarEnd ),
                            Size( nFixEnd - nFixStart, nVarDiff ),
                            DrawGridFlags::HorzLines );
        }
    }
    nCount = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
