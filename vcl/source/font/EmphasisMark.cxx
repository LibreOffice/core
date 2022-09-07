/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <font/EmphasisMark.hxx>

namespace vcl::font
{
EmphasisMark::EmphasisMark(FontEmphasisMark eEmphasis, tools::Long nHeight, sal_Int32 nDPIY)
{
    static const PolyFlags aAccentPolyFlags[24] =
    {
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Normal,  PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control
    };

    static const Point aAccentPos[24] =
    {
        {  78,    0 },
        { 348,   79 },
        { 599,  235 },
        { 843,  469 },
        { 938,  574 },
        { 990,  669 },
        { 990,  773 },
        { 990,  843 },
        { 964,  895 },
        { 921,  947 },
        { 886,  982 },
        { 860,  999 },
        { 825,  999 },
        { 764,  999 },
        { 721,  964 },
        { 686,  895 },
        { 625,  791 },
        { 556,  660 },
        { 469,  504 },
        { 400,  400 },
        { 261,  252 },
        {  61,   61 },
        {   0,   27 },
        {   9,    0 }
    };

    mnWidth      = 0;
    mnYOff       = 0;
    mbIsPolyLine   = false;

    if ( !nHeight )
        return;

    FontEmphasisMark    nEmphasisStyle = eEmphasis & FontEmphasisMark::Style;
    tools::Long                nDotSize = 0;
    switch ( nEmphasisStyle )
    {
        case FontEmphasisMark::Dot:
            // Dot has 55% of the height
            nDotSize = (nHeight*550)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
                maRect1 = tools::Rectangle( Point(), Size( nDotSize, nDotSize ) );
            else
            {
                tools::Long nRad = nDotSize/2;
                tools::Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
                maPolyPoly.Insert( aPoly );
            }
            mnYOff = ((nHeight*250)/1000)/2; // Center to the another EmphasisMarks
            mnWidth = nDotSize;
            break;

        case FontEmphasisMark::Circle:
            // Dot has 80% of the height
            nDotSize = (nHeight*800)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
                maRect1 = tools::Rectangle( Point(), Size( nDotSize, nDotSize ) );
            else
            {
                tools::Long nRad = nDotSize/2;
                tools::Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
                maPolyPoly.Insert( aPoly );
                // Border mnWidth is 15%
                tools::Long nBorder = (nDotSize*150)/1000;
                if ( nBorder <= 1 )
                    mbIsPolyLine = true;
                else
                {
                    tools::Polygon aPoly2( Point( nRad, nRad ),
                                           nRad-nBorder, nRad-nBorder );
                    maPolyPoly.Insert( aPoly2 );
                }
            }
            mnWidth = nDotSize;
            break;

        case FontEmphasisMark::Disc:
            // Dot has 80% of the height
            nDotSize = (nHeight*800)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
                maRect1 = tools::Rectangle( Point(), Size( nDotSize, nDotSize ) );
            else
            {
                tools::Long nRad = nDotSize/2;
                tools::Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
                maPolyPoly.Insert( aPoly );
            }
            mnWidth = nDotSize;
            break;

        case FontEmphasisMark::Accent:
            // Dot has 80% of the height
            nDotSize = (nHeight*800)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
            {
                if ( nDotSize == 1 )
                {
                    maRect1 = tools::Rectangle( Point(), Size( nDotSize, nDotSize ) );
                    mnWidth = nDotSize;
                }
                else
                {
                    maRect1 = tools::Rectangle( Point(), Size( 1, 1 ) );
                    maRect2 = tools::Rectangle( Point( 1, 1 ), Size( 1, 1 ) );
                }
            }
            else
            {
                tools::Polygon aPoly( SAL_N_ELEMENTS(aAccentPos), aAccentPos,
                                      aAccentPolyFlags );
                double dScale = static_cast<double>(nDotSize)/1000.0;
                aPoly.Scale( dScale, dScale );
                tools::Polygon aTemp;
                aPoly.AdaptiveSubdivide( aTemp );
                tools::Rectangle aBoundRect = aTemp.GetBoundRect();
                mnWidth = aBoundRect.GetWidth();
                nDotSize = aBoundRect.GetHeight();
                maPolyPoly.Insert( aTemp );
            }
            break;
        default: break;
    }

    // calculate position
    tools::Long nOffY = 1+(nDPIY/300); // one visible pixel space
    tools::Long nSpaceY = nHeight-nDotSize;
    if ( nSpaceY >= nOffY*2 )
        mnYOff += nOffY;

    if ( !(eEmphasis & FontEmphasisMark::PosBelow) )
        mnYOff += nDotSize;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
