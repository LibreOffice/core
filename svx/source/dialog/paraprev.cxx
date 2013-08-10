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

#include <sfx2/dialoghelper.hxx>
#include <svx/paraprev.hxx>
#include <vcl/builder.hxx>

// class SvxParaPrevWindow -----------------------------------------------

SvxParaPrevWindow::SvxParaPrevWindow( Window* pParent,  WinBits nBits) :

    Window( pParent, nBits),

    nLeftMargin     ( 0 ),
    nRightMargin    ( 0 ),
    nFirstLineOfst  ( 0 ),
    nUpper          ( 0 ),
    nLower          ( 0 ),
    eAdjust         ( SVX_ADJUST_LEFT ),
    eLastLine       ( SVX_ADJUST_LEFT ),
    eLine           ( SVX_PREV_LINESPACE_1 ),
    nLineVal        ( 0 )

{
    // Count in Twips by default
    SetMapMode( MapMode( MAP_TWIP ) );

    aSize = Size( 11905, 16837 );

    SetBorderStyle( WINDOW_BORDER_MONO );
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxParaPrevWindow(Window *pParent, VclBuilder::stringmap &)
{
    return new SvxParaPrevWindow(pParent, WB_BORDER);
}

Size SvxParaPrevWindow::GetOptimalSize() const
{
    return getParagraphPreviewOptimalSize(this);
}

// -----------------------------------------------------------------------

void SvxParaPrevWindow::Paint( const Rectangle& )
{
    DrawParagraph( sal_True );
}

// -----------------------------------------------------------------------

#define DEF_MARGIN  120

void SvxParaPrevWindow::DrawParagraph( sal_Bool bAll )
{
    Size aWinSize = GetOutputSizePixel();
    aWinSize = PixelToLogic( aWinSize );
    Size aTmp(1, 1);
    aTmp = PixelToLogic(aTmp);
    aWinSize.Width() -= aTmp.Width() /2;
    aWinSize.Height() -= aTmp.Height() /2;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    const Color& rWinColor = rStyleSettings.GetWindowColor();
    Color aGrayColor(COL_LIGHTGRAY);

    SetFillColor( Color( rWinColor ) );
    if( bAll )
        DrawRect( Rectangle( Point(), aWinSize ) );

    SetLineColor();

    long nH = aWinSize.Height() / 19;
    Size aLineSiz( aWinSize.Width() - DEF_MARGIN, nH ),
         aSiz = aLineSiz;
    Point aPnt;
    aPnt.X() = DEF_MARGIN / 2;
    SetFillColor( aGrayColor );

    for ( sal_uInt16 i = 0; i < 9; ++i )
    {
        if ( 3 == i )
        {
            SetFillColor( Color( COL_GRAY ) );
            long nTop = nUpper * aLineSiz.Height() / aSize.Height();
            aPnt.Y() += nTop * 2;
        }

        if ( 6 == i )
            SetFillColor( aGrayColor );

        if ( 3 <= i && 6 > i )
        {
            long nLeft = nLeftMargin * aLineSiz.Width() / aSize.Width();
            long nFirst = nFirstLineOfst * aLineSiz.Width() / aSize.Width();
            long nTmp = nLeft + nFirst;

            if ( 3 == i )
            {
                aPnt.X() += nTmp;
                aSiz.Width() -= nTmp;
            }
            else
            {
                aPnt.X() += nLeft;
                aSiz.Width() -= nLeft;
            }
            long nRight = nRightMargin * aLineSiz.Width() / aSize.Width();
            aSiz.Width() -= nRight;
        }

        if ( 4 == i || 5 == i || 6 == i )
        {
            switch ( eLine )
            {
                case SVX_PREV_LINESPACE_1:                      break;
                case SVX_PREV_LINESPACE_15: aPnt.Y() += nH / 2; break;
                case SVX_PREV_LINESPACE_2:  aPnt.Y() += nH;     break;

                case SVX_PREV_LINESPACE_PROP:
                case SVX_PREV_LINESPACE_MIN:
                case SVX_PREV_LINESPACE_DURCH:                  break;
            }
        }

        aPnt.Y() += nH;

        if ( (3 <= i) && (5 >= i) )
        {
            long nLW;
            switch( i )
            {
                default:
                case 3: nLW = aLineSiz.Width() * 8 / 10;    break;
                case 4: nLW = aLineSiz.Width() * 9 / 10;    break;
                case 5: nLW = aLineSiz.Width() / 2;         break;
            }

            if ( nLW > aSiz.Width() )
                nLW = aSiz.Width();

            switch ( eAdjust )
            {
                case SVX_ADJUST_LEFT:
                    break;
                case SVX_ADJUST_RIGHT:
                    aPnt.X() += ( aSiz.Width() - nLW );
                    break;
                case SVX_ADJUST_CENTER:
                    aPnt.X() += ( aSiz.Width() - nLW ) / 2;
                    break;
                default: ; //prevent warning
            }
            if( SVX_ADJUST_BLOCK == eAdjust )
            {
                if( 5 == i )
                {
                    switch( eLastLine )
                    {
                        case SVX_ADJUST_LEFT:
                            break;
                        case SVX_ADJUST_RIGHT:
                            aPnt.X() += ( aSiz.Width() - nLW );
                            break;
                        case SVX_ADJUST_CENTER:
                            aPnt.X() += ( aSiz.Width() - nLW ) / 2;
                            break;
                        case SVX_ADJUST_BLOCK:
                            nLW = aSiz.Width();
                            break;
                        default: ; //prevent warning
                    }
                }
                else
                    nLW = aSiz.Width();
            }
            aSiz.Width() = nLW;
        }

        Rectangle aRect( aPnt, aSiz );

        if ( Lines[i] != aRect || bAll )
        {
            if ( !bAll )
            {
                Color aFillCol = GetFillColor();
                SetFillColor( rWinColor );
                DrawRect( Lines[i] );
                SetFillColor( aFillCol );
            }
            DrawRect( aRect );
            Lines[i] = aRect;
        }

        if ( 5 == i )
        {
            long nBottom = nLower * aLineSiz.Height() / aSize.Height();
            aPnt.Y() += nBottom * 2;
        }

        aPnt.Y() += nH;
        // Reset, recalculate for each line
        aPnt.X() = DEF_MARGIN / 2;
        aSiz = aLineSiz;
    }
}

#undef DEF_MARGIN

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
