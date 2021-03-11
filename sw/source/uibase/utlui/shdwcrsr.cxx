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

#include <com/sun/star/text/HoriOrientation.hpp>
#include <shdwcrsr.hxx>

using namespace ::com::sun::star;

SwShadowCursor::~SwShadowCursor()
{
    if( USHRT_MAX != nOldMode )
        DrawCursor( aOldPt, nOldHeight, nOldMode );
}

void SwShadowCursor::SetPos( const Point& rPt, tools::Long nHeight, sal_uInt16 nMode )
{
    Point aPt( pWin->LogicToPixel( rPt ));
    nHeight = pWin->LogicToPixel( Size( 0, nHeight )).Height();
    if( aOldPt != aPt || nOldHeight != nHeight || nOldMode != nMode )
    {
        if( USHRT_MAX != nOldMode )
            DrawCursor( aOldPt, nOldHeight, nOldMode );

        DrawCursor( aPt, nHeight, nMode );
        nOldMode = nMode;
        nOldHeight = nHeight;
        aOldPt = aPt;
    }
}

void SwShadowCursor::DrawTri( const Point& rPt, tools::Long nHeight, bool bLeft )
{
    tools::Long nLineDiff = nHeight / 2;
    tools::Long nLineDiffHalf = nLineDiff / 2;

    // Dot above
    Point aPt1( (bLeft ? rPt.X() - 3 : rPt.X() + 3),
                rPt.Y() + nLineDiffHalf );
    // Dot below
    Point aPt2( aPt1.X(), aPt1.Y() + nHeight - nLineDiff - 1 );
    tools::Long nDiff = bLeft ? -1 : 1;
    while( aPt1.Y() <= aPt2.Y() )
    {
        pWin->DrawLine( aPt1, aPt2 );
        aPt1.AdjustY( 1 );
        aPt2.AdjustY( -1 );
        aPt2.setX( aPt1.AdjustX(nDiff ) );
    }
}

void SwShadowCursor::DrawCursor( const Point& rPt, tools::Long nHeight, sal_uInt16 nMode )
{
    nHeight = (((nHeight / 4)+1) * 4) + 1;

    pWin->Push();

    pWin->SetMapMode(MapMode(MapUnit::MapPixel));
    pWin->SetRasterOp( RasterOp::Xor );

    pWin->SetLineColor( Color( ColorTransparency, sal_uInt32(aCol) ^ sal_uInt32(COL_WHITE) ) );

    // 1. The Line:
    pWin->DrawLine( Point( rPt.X(), rPt.Y() + 1),
              Point( rPt.X(), rPt.Y() - 2 + nHeight ));

    // 2. The Triangle
    if( text::HoriOrientation::LEFT == nMode || text::HoriOrientation::CENTER == nMode )    // Arrow to the right
        DrawTri( rPt, nHeight, false );
    if( text::HoriOrientation::RIGHT == nMode || text::HoriOrientation::CENTER == nMode )   // Arrow to the left
        DrawTri( rPt, nHeight, true );

    pWin->Pop();
}

void SwShadowCursor::Paint()
{
    if( USHRT_MAX != nOldMode )
        DrawCursor( aOldPt, nOldHeight, nOldMode );
}

tools::Rectangle SwShadowCursor::GetRect() const
{
    tools::Long nH = nOldHeight;
    Point aPt( aOldPt );

    nH = (((nH / 4)+1) * 4) + 1;
    tools::Long nWidth = nH / 4 + 3 + 1;

    Size aSz( nWidth, nH );

    if( text::HoriOrientation::RIGHT == nOldMode )
        aPt.AdjustX( -(aSz.Width()) );
    else if( text::HoriOrientation::CENTER == nOldMode )
    {
        aPt.AdjustX( -(aSz.Width()) );
        aSz.setWidth( aSz.Width() * 2 );
    }

    return pWin->PixelToLogic( tools::Rectangle( aPt, aSz ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
