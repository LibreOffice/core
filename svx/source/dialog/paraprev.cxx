/*************************************************************************
 *
 *  $RCSfile: paraprev.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:10 $
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

// include ---------------------------------------------------------------

#pragma hdrstop

#include "paraprev.hxx"

// STATIC DATA -----------------------------------------------------------

#define FOUR_POINTS 80

// class SvxParaPrevWindow -----------------------------------------------

SvxParaPrevWindow::SvxParaPrevWindow( Window* pParent, const ResId& rId ) :

    Window( pParent, rId ),

    nFirstLineOfst  ( 0 ),
    nLeftMargin     ( 0 ),
    nRightMargin    ( 0 ),
    nUpper          ( 0 ),
    nLower          ( 0 ),
    eAdjust         ( SVX_ADJUST_LEFT ),
#if  SUPD>352
    eLastLine       ( SVX_ADJUST_LEFT ),
#endif
    eLine           ( SVX_PREV_LINESPACE_1 ),
    nLineVal        ( 0 )

{
    // defaultmaessing in Twips rechnen
    SetMapMode( MapMode( MAP_TWIP ) );
    aWinSize = GetOutputSizePixel();
    aWinSize = PixelToLogic( aWinSize );
    aSize = Size( 11905, 16837 );
}

// -----------------------------------------------------------------------

void SvxParaPrevWindow::Paint( const Rectangle& rRect )
{
    DrawParagraph( TRUE );
}

// -----------------------------------------------------------------------

#define DEF_MARGIN  120

void SvxParaPrevWindow::DrawParagraph( BOOL bAll )
{
    SetLineColor();
    SetFillColor( Color( COL_WHITE ) );

    if ( bAll )
        DrawRect( Rectangle( Point(), aWinSize ) );

    long nH = aWinSize.Height() / 19;
    Size aLineSiz( aWinSize.Width() - DEF_MARGIN, nH ),
         aSiz = aLineSiz;
    Point aPnt;
    aPnt.X() = DEF_MARGIN / 2;
    SetFillColor( Color( COL_LIGHTGRAY ) );

    for ( USHORT i = 0; i < 9; ++i )
    {
        if ( 3 == i )
        {
            SetFillColor( Color( COL_GRAY ) );
            long nTop = nUpper * aLineSiz.Height() / aSize.Height();
            aPnt.Y() += nTop * 2;
        }

        if ( 6 == i )
            SetFillColor( Color( COL_LIGHTGRAY ) );

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

        if ( 5 == i )
        {
            long nLW = aLineSiz.Width() / 2;

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
                case SVX_ADJUST_BLOCK:
                    break;
            }

            if ( SVX_ADJUST_BLOCK != eAdjust )
                aSiz.Width() = nLW;
        }

        Rectangle aRect( aPnt, aSiz );

        if ( Lines[i] != aRect || bAll )
        {
            if ( !bAll )
            {
                Color aFillCol = GetFillColor();
                SetFillColor( Color( COL_WHITE ) );
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
        // wieder zuruecksetzen, fuer jede Linie neu berechnen
        aPnt.X() = DEF_MARGIN / 2;
        aSiz = aLineSiz;
    }
}

#undef DEF_MARGIN

// -----------------------------------------------------------------------

void SvxParaPrevWindow::OutputSizeChanged()
{
    aWinSize = GetOutputSizePixel();
    aWinSize = PixelToLogic( aWinSize );
    Invalidate();
}

