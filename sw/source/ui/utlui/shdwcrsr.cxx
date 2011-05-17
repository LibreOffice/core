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
#include "precompiled_sw.hxx"


#include <com/sun/star/text/HoriOrientation.hpp>
#include <vcl/window.hxx>

#include "swtypes.hxx"
#include "shdwcrsr.hxx"

using namespace ::com::sun::star;


SwShadowCursor::~SwShadowCursor()
{
    if( USHRT_MAX != nOldMode )
        DrawCrsr( aOldPt, nOldHeight, nOldMode );
}

void SwShadowCursor::SetPos( const Point& rPt, long nHeight, sal_uInt16 nMode )
{
    Point aPt( pWin->LogicToPixel( rPt ));
    nHeight = pWin->LogicToPixel( Size( 0, nHeight )).Height();
    if( aOldPt != aPt || nOldHeight != nHeight || nOldMode != nMode )
    {
        if( USHRT_MAX != nOldMode )
            DrawCrsr( aOldPt, nOldHeight, nOldMode );

        DrawCrsr( aPt, nHeight, nMode );
        nOldMode = nMode;
        nOldHeight = nHeight;
        aOldPt = aPt;
    }
}

void SwShadowCursor::DrawTri( const Point& rPt, long nHeight, sal_Bool bLeft )
{
    long nLineDiff = ( nHeight / 2 );
    long nLineDiffHalf = nLineDiff / 2;

    // Punkt oben
    Point aPt1( (bLeft ? rPt.X() - 3 : rPt.X() + 3),
                rPt.Y() + nLineDiffHalf );
    // Punkt unten
    Point aPt2( aPt1.X(), aPt1.Y() + nHeight - nLineDiff - 1 );
    long nDiff = bLeft ? -1 : 1;
    while( aPt1.Y() <= aPt2.Y() )
    {
        pWin->DrawLine( aPt1, aPt2 );
        aPt1.Y()++, aPt2.Y()--;
        aPt2.X() = aPt1.X() += nDiff;
    }
}

void SwShadowCursor::DrawCrsr( const Point& rPt, long nHeight, sal_uInt16 nMode )
{
    nHeight = (((nHeight / 4)+1) * 4) + 1;

    pWin->Push();

    pWin->SetMapMode( MAP_PIXEL );
    pWin->SetRasterOp( ROP_XOR );

    pWin->SetLineColor( Color( aCol.GetColor() ^ COL_WHITE ) );

    // 1. der Strich:
    pWin->DrawLine( Point( rPt.X(), rPt.Y() + 1),
              Point( rPt.X(), rPt.Y() - 2 + nHeight ));

    // 2. das Dreieck
    if( text::HoriOrientation::LEFT == nMode || text::HoriOrientation::CENTER == nMode )    // Pfeil nach rechts
        DrawTri( rPt, nHeight, sal_False );
    if( text::HoriOrientation::RIGHT == nMode || text::HoriOrientation::CENTER == nMode )   // Pfeil nach links
        DrawTri( rPt, nHeight, sal_True );

    pWin->Pop();
}

void SwShadowCursor::Paint()
{
    if( USHRT_MAX != nOldMode )
        DrawCrsr( aOldPt, nOldHeight, nOldMode );
}

Rectangle SwShadowCursor::GetRect() const
{
    long nH = nOldHeight;
    Point aPt( aOldPt );

    nH = (((nH / 4)+1) * 4) + 1;
    long nWidth = nH / 4 + 3 + 1;

    Size aSz( nWidth, nH );

    if( text::HoriOrientation::RIGHT == nOldMode )
        aPt.X() -= aSz.Width();
    else if( text::HoriOrientation::CENTER == nOldMode )
    {
        aPt.X() -= aSz.Width();
        aSz.Width() *= 2;
    }

    return pWin->PixelToLogic( Rectangle( aPt, aSz ) );
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
