/*************************************************************************
 *
 *  $RCSfile: decoview.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:39 $
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

#define _SV_DECOVIEW_CXX

#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif

#pragma hdrstop

// =======================================================================

#define BUTTON_DRAW_FLATTEST    (BUTTON_DRAW_FLAT |             \
                                 BUTTON_DRAW_PRESSED |          \
                                 BUTTON_DRAW_CHECKED |          \
                                 BUTTON_DRAW_HIGHLIGHT)

// =======================================================================

void ImplDrawOS2Symbol( OutputDevice* pDev, const Rectangle& rRect,
                        USHORT nStyle, BOOL bClose )
{
    DecorationView          aView( pDev );
    const StyleSettings&    rStyleSettings = pDev->GetSettings().GetStyleSettings();
    Rectangle               aRect = rRect;
    Color                   aColor1;
    Color                   aColor2;

    pDev->SetFillColor();

    if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
    {
        aColor1 = rStyleSettings.GetShadowColor();
        aColor2 = rStyleSettings.GetLightColor();
    }
    else
    {
        aColor1 = rStyleSettings.GetLightColor();
        aColor2 = rStyleSettings.GetShadowColor();
    }
    aView.DrawFrame( aRect, aColor1, aColor2 );

    aRect.Left()    += 2;
    aRect.Top()     += 2;
    aRect.Right()   -= 2;
    aRect.Bottom()  -= 2;

    if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
        pDev->SetLineColor( rStyleSettings.GetLightColor() );
    else
        pDev->SetLineColor( rStyleSettings.GetShadowColor() );
    if ( bClose )
    {
        pDev->DrawLine( aRect.TopLeft(), Point( aRect.Left(), aRect.Bottom()-2 ) );
        pDev->DrawLine( aRect.TopLeft(), Point( aRect.Right()-2, aRect.Top() ) );
        pDev->DrawLine( Point( aRect.Left()+2, aRect.Bottom()-1 ),
                        Point( aRect.Right()-1, aRect.Top()+2 ) );
    }
    else
    {
        pDev->DrawLine( aRect.TopLeft(), aRect.BottomLeft() );
        pDev->DrawLine( aRect.TopLeft(), Point( aRect.Right()-1, aRect.Top() ) );
    }

    if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
        pDev->SetLineColor( rStyleSettings.GetShadowColor() );
    else
        pDev->SetLineColor( rStyleSettings.GetLightColor() );
    if ( bClose )
    {
        pDev->DrawLine( Point( aRect.Right(), aRect.Top()+2 ), aRect.BottomRight() );
        pDev->DrawLine( Point( aRect.Left()+2, aRect.Bottom() ), aRect.BottomRight() );
        pDev->DrawLine( Point( aRect.Right()-2, aRect.Top()+1 ),
                        Point( aRect.Left()+1, aRect.Bottom()-2 ) );
    }
    else
    {
        pDev->DrawLine( aRect.TopRight(), aRect.BottomRight() );
        pDev->DrawLine( Point( aRect.Left()+1, aRect.Bottom() ), aRect.BottomRight() );
    }
}

// =======================================================================

static void ImplDrawSymbol( OutputDevice* pDev, const Rectangle& rRect,
                            SymbolType eType  )
{
    // Groessen vorberechnen
    long    n;
    long    nHeight = rRect.GetHeight();
    long    nWidth = rRect.GetWidth();
    if ( nWidth < nHeight )
        n = nWidth;
    else
        n = nHeight;
    if ( n & 0x01 )
        n--;
    Point   aCenter = rRect.Center();
    long    nCenterX = aCenter.X();
    long    nCenterY = aCenter.Y();
    long    n2 = n / 2;
    long    n4 = n / 4;
    long    nLeft;
    long    nTop;
    long    nRight;
    long    nBottom;
    long    nTemp;
    long    i;

    switch ( eType )
    {
        case SYMBOL_ARROW_UP:
            {
            if ( !(n & 0x01) )
            {
                n2--;
                n4--;
            }
            nTop = nCenterY-n2;
            nBottom = nCenterY;
            pDev->DrawRect( Rectangle( nCenterX, nTop, nCenterX, nBottom ) );
            i = 1;
            while ( i <= n2 )
            {
                nTop++;
                nTemp = nCenterX-i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                nTemp = nCenterX+i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                i++;
            }
            pDev->DrawRect( Rectangle( nCenterX-n4, nBottom,
                                       nCenterX+n4, nBottom+n2 ) );
            }
            break;

        case SYMBOL_ARROW_DOWN:
            {
            if ( !(n & 0x01) )
            {
                n2--;
                n4--;
            }
            nTop = nCenterY;
            nBottom = nCenterY+n2;
            pDev->DrawRect( Rectangle( nCenterX, nTop, nCenterX, nBottom ) );
            i = 1;
            while ( i <= n2 )
            {
                nBottom--;
                nTemp = nCenterX-i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                nTemp = nCenterX+i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                i++;
            }
            pDev->DrawRect( Rectangle( nCenterX-n4, nTop-n2,
                                       nCenterX+n4, nTop ) );
            }
            break;

        case SYMBOL_ARROW_LEFT:
            {
            if ( !(n & 0x01) )
            {
                n2--;
                n4--;
            }
            nLeft = nCenterX-n2;
            nRight = nCenterX;
            pDev->DrawRect( Rectangle( nLeft, nCenterY, nRight, nCenterY ) );
            i = 1;
            while ( i <= n2 )
            {
                nLeft++;
                nTemp = nCenterY-i;
                pDev->DrawRect( Rectangle( nLeft, nTemp, nRight, nTemp ) );
                nTemp = nCenterY+i;
                pDev->DrawRect( Rectangle( nLeft, nTemp, nRight, nTemp ) );
                i++;
            }
            pDev->DrawRect( Rectangle( nRight, nCenterY-n4,
                                       nRight+n2, nCenterY+n4 ) );
            }
            break;

        case SYMBOL_ARROW_RIGHT:
            {
            if ( !(n & 0x01) )
            {
                n2--;
                n4--;
            }
            nLeft = nCenterX;
            nRight = nCenterX+n2;
            pDev->DrawRect( Rectangle( nLeft, nCenterY, nRight, nCenterY ) );
            i = 1;
            while ( i <= n2 )
            {
                nRight--;
                nTemp = nCenterY-i;
                pDev->DrawRect( Rectangle( nLeft, nTemp, nRight, nTemp ) );
                nTemp = nCenterY+i;
                pDev->DrawRect( Rectangle( nLeft, nTemp, nRight, nTemp ) );
                i++;
            }
            pDev->DrawRect( Rectangle( nLeft-n2, nCenterY-n4,
                                       nLeft, nCenterY+n4 ) );
            }
            break;


        case SYMBOL_SPIN_UP:
            {
            if ( !(n & 0x01) )
                n2--;
            nTop = nCenterY-n4;
            nBottom = nTop+n2;
            pDev->DrawRect( Rectangle( nCenterX, nTop, nCenterX, nBottom ) );
            i = 1;
            while ( i <= n2 )
            {
                nTop++;
                nTemp = nCenterX-i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                nTemp = nCenterX+i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                i++;
            }
            }
            break;

        case SYMBOL_SPIN_DOWN:
            {
            if ( !(n & 0x01) )
                n2--;
            nTop = nCenterY-n4;
            nBottom = nTop+n2;
            pDev->DrawRect( Rectangle( nCenterX, nTop, nCenterX, nBottom ) );
            i = 1;
            while ( i <= n2 )
            {
                nBottom--;
                nTemp = nCenterX-i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                nTemp = nCenterX+i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                i++;
            }
            }
            break;

        case SYMBOL_SPIN_LEFT:
        case SYMBOL_FIRST:
        case SYMBOL_PREV:
        case SYMBOL_REVERSEPLAY:
            {
            if ( !(n & 0x01) )
                n2--;
            nLeft = nCenterX-n4;
            if ( eType == SYMBOL_FIRST )
                nLeft++;
            nRight = nLeft+n2;
            pDev->DrawRect( Rectangle( nLeft, nCenterY, nRight, nCenterY ) );
            i = 1;
            while ( i <= n2 )
            {
                nLeft++;
                nTemp = nCenterY-i;
                pDev->DrawRect( Rectangle( nLeft, nTemp, nRight, nTemp ) );
                nTemp = nCenterY+i;
                pDev->DrawRect( Rectangle( nLeft, nTemp, nRight, nTemp ) );
                i++;
            }
            if ( eType == SYMBOL_FIRST )
            {
                pDev->DrawRect( Rectangle( nCenterX-n4-1, nCenterY-n2,
                                           nCenterX-n4-1, nCenterY+n2 ) );
            }
            }
            break;

        case SYMBOL_SPIN_RIGHT:
        case SYMBOL_LAST:
        case SYMBOL_NEXT:
        case SYMBOL_PLAY:
            {
            if ( !(n & 0x01) )
                n2--;
            nLeft = nCenterX-n4;
            if ( eType == SYMBOL_LAST )
                nLeft--;
            nRight = nLeft+n2;
            pDev->DrawRect( Rectangle( nLeft, nCenterY, nRight, nCenterY ) );
            i = 1;
            while ( i <= n2 )
            {
                nRight--;
                nTemp = nCenterY-i;
                pDev->DrawRect( Rectangle( nLeft, nTemp, nRight, nTemp ) );
                nTemp = nCenterY+i;
                pDev->DrawRect( Rectangle( nLeft, nTemp, nRight, nTemp ) );
                i++;
            }
            if ( eType == SYMBOL_LAST )
            {
                pDev->DrawRect( Rectangle( nCenterX+n4+1, nCenterY-n2,
                                           nCenterX+n4+1, nCenterY+n2 ) );
            }
            }
            break;

        case SYMBOL_PAGEUP:
            {
            nTop = nCenterY-n2;
            nBottom = nCenterY-1;
            pDev->DrawRect( Rectangle( nCenterX, nTop, nCenterX, nBottom ) );
            pDev->DrawRect( Rectangle( nCenterX, nTop+n2+1, nCenterX, nBottom+n2+1 ) );
            i = 1;
            while ( i < n2 )
            {
                nTop++;
                nTemp = nCenterX-i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                pDev->DrawRect( Rectangle( nTemp, nTop+n2+1, nTemp, nBottom+n2+1 ) );
                nTemp = nCenterX+i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                pDev->DrawRect( Rectangle( nTemp, nTop+n2+1, nTemp, nBottom+n2+1 ) );
                i++;
            }
            }
            break;

        case SYMBOL_PAGEDOWN:
            {
            nTop = nCenterY-n2;
            nBottom = nCenterY-1;
            pDev->DrawRect( Rectangle( nCenterX, nTop, nCenterX, nBottom ) );
            pDev->DrawRect( Rectangle( nCenterX, nTop+n2+1, nCenterX, nBottom+n2+1 ) );
            i = 1;
            while ( i < n2 )
            {
                nBottom--;
                nTemp = nCenterX-i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                pDev->DrawRect( Rectangle( nTemp, nTop+n2+1, nTemp, nBottom+n2+1 ) );
                nTemp = nCenterX+i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                pDev->DrawRect( Rectangle( nTemp, nTop+n2+1, nTemp, nBottom+n2+1 ) );
                i++;
            }
            }
            break;

        case SYMBOL_RADIOCHECKMARK:
        case SYMBOL_RECORD:
        {
            const long          nExt = ( n2 << 1 ) + 1;
            Bitmap              aBmp( Size( nExt, nExt ), 1 );
            BitmapWriteAccess*  pWAcc = aBmp.AcquireWriteAccess();

            if( pWAcc )
            {
                const Color aWhite( COL_WHITE );
                const Color aBlack( COL_BLACK );

                pWAcc->Erase( aWhite );
                pWAcc->SetLineColor( aBlack );
                pWAcc->SetFillColor( aBlack );
                pWAcc->DrawPolygon( Polygon( Point( n2, n2 ), n2, n2 ) );
                aBmp.ReleaseAccess( pWAcc );
                pDev->DrawMask( Point( nCenterX - n2, nCenterY - n2 ), aBmp, pDev->GetFillColor() );
            }
            else
                pDev->DrawPolygon( Polygon( Point( nCenterX, nCenterY ), n2, n2 ) );
        }
        break;

        case SYMBOL_STOP:
            {
            nLeft = nCenterX-n2;
            nRight = nCenterX+n2;
            nTop = nCenterY-n2;
            nBottom = nCenterY+n2;
            pDev->DrawRect( Rectangle( nLeft, nTop, nRight, nBottom ) );
            }
            break;

        case SYMBOL_PAUSE:
            {
            nLeft = nCenterX-n2;
            nRight = nCenterX+n2-1;
            nTop = nCenterY-n2;
            nBottom = nCenterY+n2;
            pDev->DrawRect( Rectangle( nLeft, nTop, nCenterX-2, nBottom ) );
            pDev->DrawRect( Rectangle( nCenterX+1, nTop, nRight, nBottom ) );
            }
            break;

        case SYMBOL_WINDSTART:
        case SYMBOL_WINDBACKWARD:
            {
            nLeft = nCenterX-n2+1;
            nRight = nCenterX;
            pDev->DrawRect( Rectangle( nLeft, nCenterY, nRight, nCenterY ) );
            pDev->DrawRect( Rectangle( nLeft+n2, nCenterY, nRight+n2, nCenterY ) );
            i = 1;
            while ( i < n2 )
            {
                nLeft++;
                nTemp = nCenterY-i;
                pDev->DrawRect( Rectangle( nLeft, nTemp, nRight, nTemp ) );
                pDev->DrawRect( Rectangle( nLeft+n2, nTemp, nRight+n2, nTemp ) );
                nTemp = nCenterY+i;
                pDev->DrawRect( Rectangle( nLeft, nTemp, nRight, nTemp ) );
                pDev->DrawRect( Rectangle( nLeft+n2, nTemp, nRight+n2, nTemp ) );
                i++;
            }
            if ( eType == SYMBOL_WINDSTART )
            {
                pDev->DrawRect( Rectangle( nCenterX-n2, nCenterY-n2,
                                           nCenterX-n2, nCenterY+n2 ) );
            }
            }
            break;

        case SYMBOL_WINDEND:
        case SYMBOL_WINDFORWARD:
            {
            nLeft = nCenterX-n2;
            nRight = nCenterX-1;
            pDev->DrawRect( Rectangle( nLeft, nCenterY, nRight, nCenterY ) );
            pDev->DrawRect( Rectangle( nLeft+n2, nCenterY, nRight+n2, nCenterY ) );
            i = 1;
            while ( i < n2 )
            {
                nRight--;
                nTemp = nCenterY-i;
                pDev->DrawRect( Rectangle( nLeft, nTemp, nRight, nTemp ) );
                pDev->DrawRect( Rectangle( nLeft+n2, nTemp, nRight+n2, nTemp ) );
                nTemp = nCenterY+i;
                pDev->DrawRect( Rectangle( nLeft, nTemp, nRight, nTemp ) );
                pDev->DrawRect( Rectangle( nLeft+n2, nTemp, nRight+n2, nTemp ) );
                i++;
            }
            if ( eType == SYMBOL_WINDEND )
            {
                pDev->DrawRect( Rectangle( nCenterX+n2, nCenterY-n2,
                                           nCenterX+n2, nCenterY+n2 ) );
            }
            }
            break;

        case SYMBOL_CLOSE:
            {
            Size aRectSize( 2, 1 );
            if ( n < 8 )
                aRectSize.Width() = 1;
            else if ( n > 20 )
                aRectSize.Width() = n/10;
            nLeft   = nCenterX-n2+1;
            nTop    = nCenterY-n2+1;
            nBottom = nCenterY-n2+n-aRectSize.Width()+1;
            i = 0;
            while ( i < n-aRectSize.Width()+1 )
            {
                pDev->DrawRect( Rectangle( Point( nLeft+i, nTop+i ), aRectSize ) );
                pDev->DrawRect( Rectangle( Point( nLeft+i, nBottom-i ), aRectSize ) );
                i++;
            }
            }
            break;

        case SYMBOL_ROLLUP:
        case SYMBOL_ROLLDOWN:
            {
            Rectangle aRect( nCenterX-n2, nCenterY-n2,
                             nCenterX+n2, nCenterY-n2+1 );
            pDev->DrawRect( aRect );
            if ( eType == SYMBOL_ROLLDOWN )
            {
                Rectangle aTempRect = aRect;
                aTempRect.Bottom() = nCenterY+n2;
                aTempRect.Right() = aRect.Left();
                pDev->DrawRect( aTempRect );
                aTempRect.Left() = aRect.Right();
                aTempRect.Right() = aRect.Right();
                pDev->DrawRect( aTempRect );
                aTempRect.Top() = aTempRect.Bottom();
                aTempRect.Left() = aRect.Left();
                pDev->DrawRect( aTempRect );
            }
            }
            break;
        case SYMBOL_CHECKMARK:
            {
                Point aPos1( rRect.Left(), rRect.Bottom() - rRect.GetHeight() / 3 );
                Point aPos2( rRect.Left() + rRect.GetWidth()/3, rRect.Bottom() );
                Point aPos3( rRect.TopRight() );
                Size aRectSize( 1, 2 );
                long nStepsY = aPos2.Y()-aPos1.Y();
                long nX = aPos1.X();
                long nY = aPos1.Y();
                for ( long n = 0; n <= nStepsY; n++ )
                {
                    pDev->DrawRect( Rectangle( Point( nX, nY++ ), aRectSize ) );
                    nX++;
                }
                nStepsY = aPos2.Y()-aPos3.Y();
                nX = aPos2.X();
                nY = aPos2.Y();
                for ( n = 0; n <= nStepsY; n++ )
                {
                    pDev->DrawRect( Rectangle( Point( nX, nY-- ), aRectSize ) );
                    nX++;
                    if ( nX > rRect.Right() )
                        break;
                }
            }
            break;

        case SYMBOL_SPIN_UPDOWN:
            {
            nTop = nCenterY-n2-1;
            nBottom = nTop+n2;
            pDev->DrawRect( Rectangle( nCenterX, nTop, nCenterX, nBottom ) );
            i = 1;
            while ( i <= n2 )
            {
                nTop++;
                nTemp = nCenterX-i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                nTemp = nCenterX+i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                i++;
            }
            nTop = nCenterY+1;
            nBottom = nTop+n2;
            pDev->DrawRect( Rectangle( nCenterX, nTop, nCenterX, nBottom ) );
            i = 1;
            while ( i <= n2 )
            {
                nBottom--;
                nTemp = nCenterX-i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                nTemp = nCenterX+i;
                pDev->DrawRect( Rectangle( nTemp, nTop, nTemp, nBottom ) );
                i++;
            }
            }
            break;


        case SYMBOL_FLOAT:
            {
            Rectangle aRect( nCenterX-n2, nCenterY-n2+3,
                             nCenterX+n2-2, nCenterY-n2+4 );
            pDev->DrawRect( aRect );
            Rectangle aTempRect = aRect;
            aTempRect.Bottom() = nCenterY+n2;
            aTempRect.Right() = aRect.Left();
            pDev->DrawRect( aTempRect );
            aTempRect.Left() = aRect.Right();
            aTempRect.Right() = aRect.Right();
            pDev->DrawRect( aTempRect );
            aTempRect.Top() = aTempRect.Bottom();
            aTempRect.Left() = aRect.Left();
            pDev->DrawRect( aTempRect );
            aRect = Rectangle( nCenterX-n2+2, nCenterY-n2,
                             nCenterX+n2, nCenterY-n2+1 );
            pDev->DrawRect( aRect );
            aTempRect = aRect;
            aTempRect.Bottom() = nCenterY+n2-3;
            aTempRect.Right() = aRect.Left();
            pDev->DrawRect( aTempRect );
            aTempRect.Left() = aRect.Right();
            aTempRect.Right() = aRect.Right();
            pDev->DrawRect( aTempRect );
            aTempRect.Top() = aTempRect.Bottom();
            aTempRect.Left() = aRect.Left();
            pDev->DrawRect( aTempRect );
            }
            break;
        case SYMBOL_DOCK:
            {
            Rectangle aRect( nCenterX-n2, nCenterY-n2,
                             nCenterX+n2, nCenterY-n2 );
            pDev->DrawRect( aRect );
            Rectangle aTempRect = aRect;
            aTempRect.Bottom() = nCenterY+n2;
            aTempRect.Right() = aRect.Left();
            pDev->DrawRect( aTempRect );
            aTempRect.Left() = aRect.Right();
            aTempRect.Right() = aRect.Right();
            pDev->DrawRect( aTempRect );
            aTempRect.Top() = aTempRect.Bottom();
            aTempRect.Left() = aRect.Left();
            pDev->DrawRect( aTempRect );
            }
            break;
        case SYMBOL_HIDE:
            {
            long nExtra = n / 8;
            Rectangle aRect( nCenterX-n2+nExtra, nCenterY+n2-1,
                             nCenterX+n2-nExtra, nCenterY+n2 );
            pDev->DrawRect( aRect );
            }
            break;

        case SYMBOL_OS2CLOSE:
            {
            Rectangle aRect( nCenterX-n2, nCenterY-n2,
                             nCenterX+n2, nCenterY+n2 );
            ImplDrawOS2Symbol( pDev, aRect, 0, TRUE );
            }
            break;

        case SYMBOL_OS2FLOAT:
            {
            Rectangle aRect( nCenterX-n2+4, nCenterY-n2+4,
                             nCenterX+n2-4, nCenterY+n2-3 );
            ImplDrawOS2Symbol( pDev, aRect, 0, FALSE );
            DecorationView aDecoView( pDev );
            Rectangle aRect2( nCenterX-n2, nCenterY-n2,
                              nCenterX-n2+2, nCenterY+n2 );
            aDecoView.DrawFrame( aRect2,
                                 pDev->GetSettings().GetStyleSettings().GetLightColor(),
                                 pDev->GetSettings().GetStyleSettings().GetShadowColor() );
            Rectangle aRect3( nCenterX+n2-2, nCenterY-n2,
                              nCenterX+n2, nCenterY+n2 );
            aDecoView.DrawFrame( aRect3,
                                 pDev->GetSettings().GetStyleSettings().GetLightColor(),
                                 pDev->GetSettings().GetStyleSettings().GetShadowColor() );
            }
            break;

        case SYMBOL_OS2HIDE:
            {
            Rectangle aRect( nCenterX-n2+3, nCenterY-n2+3,
                             nCenterX+n2-3, nCenterY+n2-3 );
            ImplDrawOS2Symbol( pDev, aRect, 0, FALSE );
            }
            break;
    }
}

// -----------------------------------------------------------------------

void DecorationView::DrawSymbol( const Rectangle& rRect, SymbolType eType,
                                 const Color& rColor, USHORT nStyle )
{
    const StyleSettings&    rStyleSettings  = mpOutDev->GetSettings().GetStyleSettings();
    Rectangle               aRect           = mpOutDev->LogicToPixel( rRect );
    Color                   aOldLineColor   = mpOutDev->GetLineColor();
    Color                   aOldFillColor   = mpOutDev->GetFillColor();
    BOOL                    bOldMapMode     = mpOutDev->IsMapModeEnabled();
    mpOutDev->SetLineColor();
    mpOutDev->SetFillColor( rColor );
    mpOutDev->EnableMapMode( FALSE );

    if ( (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) ||
         (mpOutDev->GetOutDevType() == OUTDEV_PRINTER) )
        nStyle |= BUTTON_DRAW_MONO;

    if ( nStyle & SYMBOL_DRAW_MONO )
    {
        if ( nStyle & SYMBOL_DRAW_DISABLE )
            mpOutDev->SetFillColor( Color( COL_GRAY ) );
        else
            mpOutDev->SetFillColor( Color( COL_BLACK ) );
    }
    else
    {
        if ( nStyle & SYMBOL_DRAW_DISABLE )
        {
            // Als Embosed ausgeben
            mpOutDev->SetFillColor( rStyleSettings.GetLightColor() );
            Rectangle aTempRect = aRect;
            aTempRect.Move( 1, 1 );
            ImplDrawSymbol( mpOutDev, aTempRect, eType );
            mpOutDev->SetFillColor( rStyleSettings.GetShadowColor() );
        }
        else
            mpOutDev->SetFillColor( rColor );
    }

    ImplDrawSymbol( mpOutDev, aRect, eType );

    mpOutDev->SetLineColor( aOldLineColor );
    mpOutDev->SetFillColor( aOldFillColor );
    mpOutDev->EnableMapMode( bOldMapMode );
}

// =======================================================================

void DecorationView::DrawFrame( const Rectangle& rRect,
                                const Color& rLeftTopColor,
                                const Color& rRightBottomColor )
{
    Rectangle   aRect           = mpOutDev->LogicToPixel( rRect );
    Color       aOldLineColor   = mpOutDev->GetLineColor();
    Color       aOldFillColor   = mpOutDev->GetFillColor();
    BOOL        bOldMapMode     = mpOutDev->IsMapModeEnabled();
    mpOutDev->EnableMapMode( FALSE );
    mpOutDev->SetLineColor();
    mpOutDev->ImplDraw2ColorFrame( aRect, rLeftTopColor, rRightBottomColor );
    mpOutDev->SetLineColor( aOldLineColor );
    mpOutDev->SetFillColor( aOldFillColor );
    mpOutDev->EnableMapMode( bOldMapMode );
}

// =======================================================================

void DecorationView::DrawHighlightFrame( const Rectangle& rRect,
                                         USHORT nStyle )
{
    const StyleSettings& rStyleSettings = mpOutDev->GetSettings().GetStyleSettings();
    Color aLightColor = rStyleSettings.GetLightColor();
    Color aShadowColor = rStyleSettings.GetShadowColor();

    if ( (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) ||
         (mpOutDev->GetOutDevType() == OUTDEV_PRINTER) )
    {
        aLightColor = Color( COL_BLACK );
        aShadowColor = Color( COL_BLACK );
    }
    else if ( nStyle & FRAME_HIGHLIGHT_TESTBACKGROUND )
    {
        Wallpaper aBackground = mpOutDev->GetBackground();
        if ( aBackground.IsBitmap() || aBackground.IsGradient() )
        {
            aLightColor = rStyleSettings.GetFaceColor();
            aShadowColor = Color( COL_BLACK );
        }
        else
        {
            Color aBackColor = aBackground.GetColor();
            if ( (aLightColor.GetColorError( aBackColor ) < 32) ||
                 (aShadowColor.GetColorError( aBackColor ) < 32) )
            {
                aLightColor = Color( COL_WHITE );
                aShadowColor = Color( COL_BLACK );

                if ( aLightColor.GetColorError( aBackColor ) < 32 )
                    aLightColor.DecreaseLuminance( 64 );
                if ( aShadowColor.GetColorError( aBackColor ) < 32 )
                    aShadowColor.IncreaseLuminance( 64 );
            }
        }
    }

    if ( (nStyle & FRAME_HIGHLIGHT_STYLE) == FRAME_HIGHLIGHT_IN )
    {
        Color aTempColor = aLightColor;
        aLightColor = aShadowColor;
        aShadowColor = aTempColor;
    }

    DrawFrame( rRect, aLightColor, aShadowColor );
}

// =======================================================================

static void ImplDrawDPILineRect( OutputDevice* pDev, Rectangle& rRect,
                                 const Color* pColor )
{
    long nLineWidth = pDev->ImplGetDPIX()/300;
    long nLineHeight = pDev->ImplGetDPIY()/300;
    if ( !nLineWidth )
        nLineWidth = 1;
    if ( !nLineHeight )
        nLineHeight = 1;

    if ( pColor )
    {
        if ( (nLineWidth == 1) && (nLineHeight == 1) )
        {
            pDev->SetLineColor( *pColor );
            pDev->SetFillColor();
            pDev->DrawRect( rRect );
        }
        else
        {
            long nWidth = rRect.GetWidth();
            long nHeight = rRect.GetHeight();
            pDev->SetLineColor();
            pDev->SetFillColor( *pColor );
            pDev->DrawRect( Rectangle( rRect.TopLeft(), Size( nWidth, nLineHeight ) ) );
            pDev->DrawRect( Rectangle( rRect.TopLeft(), Size( nLineWidth, nHeight ) ) );
            pDev->DrawRect( Rectangle( Point( rRect.Left(), rRect.Bottom()-nLineHeight ),
                                       Size( nWidth, nLineHeight ) ) );
            pDev->DrawRect( Rectangle( Point( rRect.Right()-nLineWidth, rRect.Top() ),
                                       Size( nLineWidth, nHeight ) ) );
        }
    }

    rRect.Left()    += nLineWidth;
    rRect.Top()     += nLineHeight;
    rRect.Right()   -= nLineWidth;
    rRect.Bottom()  -= nLineHeight;
}

// =======================================================================

static void ImplDrawFrame( OutputDevice* pDev, Rectangle& rRect,
                           const StyleSettings& rStyleSettings, USHORT nStyle )
{
    if ( (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) ||
         (pDev->GetOutDevType() == OUTDEV_PRINTER) )
        nStyle |= FRAME_DRAW_MONO;

    if ( nStyle & FRAME_DRAW_NODRAW )
    {
        if ( nStyle & FRAME_DRAW_MONO )
            ImplDrawDPILineRect( pDev, rRect, NULL );
        else
        {
            USHORT nFrameStyle = nStyle & FRAME_DRAW_STYLE;

            if ( nFrameStyle == FRAME_DRAW_GROUP )
            {
                rRect.Left()    += 2;
                rRect.Top()     += 2;
                rRect.Right()   -= 2;
                rRect.Bottom()  -= 2;
            }
            else if ( (nFrameStyle == FRAME_DRAW_IN) ||
                      (nFrameStyle == FRAME_DRAW_OUT) )
            {
                rRect.Left()++;
                rRect.Top()++;
                rRect.Right()--;
                rRect.Bottom()--;
            }
            else // FRAME_DRAW_DOUBLEIN || FRAME_DRAW_DOUBLEOUT
            {
                rRect.Left()    += 2;
                rRect.Top()     += 2;
                rRect.Right()   -= 2;
                rRect.Bottom()  -= 2;
            }
        }
    }
    else
    {
        if ( nStyle & FRAME_DRAW_MONO )
        {
            Color aColor( COL_BLACK );
            ImplDrawDPILineRect( pDev, rRect, &aColor );
        }
        else
        {
            USHORT nFrameStyle = nStyle & FRAME_DRAW_STYLE;
            if ( nFrameStyle == FRAME_DRAW_GROUP )
            {
                pDev->SetFillColor();
                pDev->SetLineColor( rStyleSettings.GetLightColor() );
                rRect.Top()++;
                rRect.Left()++;
                pDev->DrawRect( rRect );
                rRect.Top()--;
                rRect.Left()--;
                pDev->SetLineColor( rStyleSettings.GetShadowColor() );
                rRect.Right()--;
                rRect.Bottom()--;
                pDev->DrawRect( rRect );
                rRect.Right()++;
                rRect.Bottom()++;
            }
            else
            {
                pDev->SetLineColor();

                if ( (nFrameStyle == FRAME_DRAW_IN) ||
                     (nFrameStyle == FRAME_DRAW_OUT) )
                {
                    if ( nFrameStyle == FRAME_DRAW_IN )
                    {
                        pDev->ImplDraw2ColorFrame( rRect,
                                                   rStyleSettings.GetShadowColor(),
                                                   rStyleSettings.GetLightColor() );
                    }
                    else
                    {
                        pDev->ImplDraw2ColorFrame( rRect,
                                                   rStyleSettings.GetLightColor(),
                                                   rStyleSettings.GetShadowColor() );
                    }

                    rRect.Left()++;
                    rRect.Top()++;
                    rRect.Right()--;
                    rRect.Bottom()--;
                }
                else // FRAME_DRAW_DOUBLEIN || FRAME_DRAW_DOUBLEOUT
                {
                    if ( nFrameStyle == FRAME_DRAW_DOUBLEIN )
                    {
                        pDev->ImplDraw2ColorFrame( rRect,
                                                   rStyleSettings.GetShadowColor(),
                                                   rStyleSettings.GetLightColor() );
                    }
                    else
                    {
                        pDev->ImplDraw2ColorFrame( rRect,
                                                   rStyleSettings.GetLightBorderColor(),
                                                   rStyleSettings.GetDarkShadowColor() );
                    }

                    rRect.Left()++;
                    rRect.Top()++;
                    rRect.Right()--;
                    rRect.Bottom()--;

                    if ( nFrameStyle == FRAME_DRAW_DOUBLEIN )
                    {
                        pDev->ImplDraw2ColorFrame( rRect,
                                                   rStyleSettings.GetDarkShadowColor(),
                                                   rStyleSettings.GetLightBorderColor() );
                    }
                    else
                    {
                        pDev->ImplDraw2ColorFrame( rRect,
                                                   rStyleSettings.GetLightColor(),
                                                   rStyleSettings.GetShadowColor() );
                    }

                    rRect.Left()++;
                    rRect.Top()++;
                    rRect.Right()--;
                    rRect.Bottom()--;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

Rectangle DecorationView::DrawFrame( const Rectangle& rRect, USHORT nStyle )
{
    Rectangle   aRect = rRect;
    BOOL        bOldMap = mpOutDev->IsMapModeEnabled();
    if ( bOldMap )
    {
        aRect = mpOutDev->LogicToPixel( aRect );
        mpOutDev->EnableMapMode( FALSE );
    }

    if ( !rRect.IsEmpty() )
    {
        if ( nStyle & FRAME_DRAW_NODRAW )
             ImplDrawFrame( mpOutDev, aRect, mpOutDev->GetSettings().GetStyleSettings(), nStyle );
        else
        {
             Color maOldLineColor  = mpOutDev->GetLineColor();
             Color maOldFillColor  = mpOutDev->GetFillColor();
             ImplDrawFrame( mpOutDev, aRect, mpOutDev->GetSettings().GetStyleSettings(), nStyle );
             mpOutDev->SetLineColor( maOldLineColor );
             mpOutDev->SetFillColor( maOldFillColor );
        }
    }

    if ( bOldMap )
    {
        mpOutDev->EnableMapMode( bOldMap );
        aRect = mpOutDev->PixelToLogic( aRect );
    }

    return aRect;
}

// =======================================================================

static void ImplDrawButton( OutputDevice* pDev, Rectangle& rRect,
                            const StyleSettings& rStyleSettings, USHORT nStyle )
{
    Rectangle aFillRect = rRect;

    if ( nStyle & BUTTON_DRAW_MONO )
    {
        if ( !(nStyle & BUTTON_DRAW_NODRAW) )
        {
            Color aBlackColor( COL_BLACK );

            if ( nStyle & BUTTON_DRAW_DEFAULT )
                ImplDrawDPILineRect( pDev, aFillRect, &aBlackColor );

            ImplDrawDPILineRect( pDev, aFillRect, &aBlackColor );

            Size aBrdSize( 1, 1 );
            if ( pDev->GetOutDevType() == OUTDEV_PRINTER )
            {
                MapMode aResMapMode( MAP_100TH_MM );
                aBrdSize = pDev->LogicToPixel( Size( 20, 20 ), aResMapMode );
                if ( !aBrdSize.Width() )
                    aBrdSize.Width() = 1;
                if ( !aBrdSize.Height() )
                    aBrdSize.Height() = 1;
            }
            pDev->SetLineColor();
            pDev->SetFillColor( aBlackColor );
            Rectangle aRect1;
            Rectangle aRect2;
            aRect1.Left()   = aFillRect.Left();
            aRect1.Right()  = aFillRect.Right(),
            aRect2.Top()    = aFillRect.Top();
            aRect2.Bottom() = aFillRect.Bottom();
            if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
            {
                aRect1.Top()    = aFillRect.Top();
                aRect1.Bottom() = aBrdSize.Height()-1;
                aRect2.Left()   = aFillRect.Left();
                aRect2.Right()  = aFillRect.Left()+aBrdSize.Width()-1;
                aFillRect.Left() += aBrdSize.Width();
                aFillRect.Top()  += aBrdSize.Height();
            }
            else
            {
                aRect1.Top()    = aFillRect.Bottom()-aBrdSize.Height()+1;
                aRect1.Bottom() = aFillRect.Bottom();
                aRect2.Left()   = aFillRect.Right()-aBrdSize.Width()+1;
                aRect2.Right()  = aFillRect.Right(),
                aFillRect.Right()  -= aBrdSize.Width();
                aFillRect.Bottom() -= aBrdSize.Height();
            }
            pDev->DrawRect( aRect1 );
            pDev->DrawRect( aRect2 );
        }
    }
    else
    {
        if ( !(nStyle & BUTTON_DRAW_NODRAW) )
        {
            if ( nStyle & BUTTON_DRAW_DEFAULT )
            {
                Color aDefBtnColor = rStyleSettings.GetDarkShadowColor();
                ImplDrawDPILineRect( pDev, aFillRect, &aDefBtnColor );
            }
        }

        if ( !(nStyle & BUTTON_DRAW_NODRAW) )
        {
            pDev->SetLineColor();
            if ( nStyle & BUTTON_DRAW_NOLEFTLIGHTBORDER )
            {
                pDev->SetFillColor( rStyleSettings.GetLightBorderColor() );
                pDev->DrawRect( Rectangle( aFillRect.Left(), aFillRect.Top(),
                                           aFillRect.Left(), aFillRect.Bottom() ) );
                aFillRect.Left()++;
            }
            if ( (nStyle & BUTTON_DRAW_NOTOPLIGHTBORDER) &&
                 !(nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED)) )
            {
                pDev->SetFillColor( rStyleSettings.GetLightBorderColor() );
                pDev->DrawRect( Rectangle( aFillRect.Left(), aFillRect.Top(),
                                           aFillRect.Right(), aFillRect.Top() ) );
                aFillRect.Top()++;
            }
            if ( ((nStyle & BUTTON_DRAW_NOBOTTOMSHADOWBORDER | BUTTON_DRAW_FLAT) == (BUTTON_DRAW_NOBOTTOMSHADOWBORDER | BUTTON_DRAW_FLAT)) &&
                 !(nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED | BUTTON_DRAW_HIGHLIGHT)) )
            {
                pDev->SetFillColor( rStyleSettings.GetDarkShadowColor() );
                pDev->DrawRect( Rectangle( aFillRect.Left(), aFillRect.Bottom(),
                                           aFillRect.Right(), aFillRect.Bottom() ) );
                aFillRect.Bottom()--;
            }

            Color aColor1;
            Color aColor2;
            if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
            {
                aColor1 = rStyleSettings.GetDarkShadowColor();
                aColor2 = rStyleSettings.GetLightColor();
            }
            else
            {
                if ( nStyle & BUTTON_DRAW_NOLIGHTBORDER )
                    aColor1 = rStyleSettings.GetLightBorderColor();
                else
                    aColor1 = rStyleSettings.GetLightColor();
                if ( (nStyle & BUTTON_DRAW_FLATTEST) == BUTTON_DRAW_FLAT )
                    aColor2 = rStyleSettings.GetShadowColor();
                else
                    aColor2 = rStyleSettings.GetDarkShadowColor();
            }
            pDev->ImplDraw2ColorFrame( aFillRect, aColor1, aColor2 );
            aFillRect.Left()++;
            aFillRect.Top()++;
            aFillRect.Right()--;
            aFillRect.Bottom()--;

            if ( !((nStyle & BUTTON_DRAW_FLATTEST) == BUTTON_DRAW_FLAT) )
            {
                if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
                {
                    aColor1 = rStyleSettings.GetShadowColor();
                    aColor2 = rStyleSettings.GetLightBorderColor();
                }
                else
                {
                    if ( nStyle & BUTTON_DRAW_NOLIGHTBORDER )
                        aColor1 = rStyleSettings.GetLightColor();
                    else
                        aColor1 = rStyleSettings.GetLightBorderColor();
                    aColor2 = rStyleSettings.GetShadowColor();
                }
                pDev->ImplDraw2ColorFrame( aFillRect, aColor1, aColor2 );
                aFillRect.Left()++;
                aFillRect.Top()++;
                aFillRect.Right()--;
                aFillRect.Bottom()--;
            }
        }
    }

    if ( !(nStyle & (BUTTON_DRAW_NOFILL | BUTTON_DRAW_NODRAW)) )
    {
        pDev->SetLineColor();
        if ( nStyle & BUTTON_DRAW_MONO )
        {
            // Hack: Auf Druckern wollen wir im MonoChrom-Modus trotzdem
            // erstmal graue Buttons haben
            if ( pDev->GetOutDevType() == OUTDEV_PRINTER )
                pDev->SetFillColor( Color( COL_LIGHTGRAY ) );
            else
                pDev->SetFillColor( Color( COL_WHITE ) );
        }
        else
        {
            if ( nStyle & (BUTTON_DRAW_CHECKED | BUTTON_DRAW_DONTKNOW) )
                pDev->SetFillColor( rStyleSettings.GetCheckedColor() );
            else
                pDev->SetFillColor( rStyleSettings.GetFaceColor() );
        }
        pDev->DrawRect( aFillRect );
    }

    // Ein Border freilassen, der jedoch bei Default-Darstellung
    // mitbenutzt wird
    rRect.Left()++;
    rRect.Top()++;
    rRect.Right()--;
    rRect.Bottom()--;

    if ( nStyle & BUTTON_DRAW_NOLIGHTBORDER )
    {
        rRect.Left()++;
        rRect.Top()++;
    }
    else if ( nStyle & BUTTON_DRAW_NOLEFTLIGHTBORDER )
        rRect.Left()++;

    if ( nStyle & BUTTON_DRAW_PRESSED )
    {
        if ( (rRect.GetHeight() > 10) && (rRect.GetWidth() > 10) )
        {
            rRect.Left()    += 4;
            rRect.Top()     += 4;
            rRect.Right()   -= 1;
            rRect.Bottom()  -= 1;
        }
        else
        {
            rRect.Left()    += 3;
            rRect.Top()     += 3;
            rRect.Right()   -= 2;
            rRect.Bottom()  -= 2;
        }
    }
    else if ( nStyle & BUTTON_DRAW_CHECKED )
    {
        rRect.Left()    += 3;
        rRect.Top()     += 3;
        rRect.Right()   -= 2;
        rRect.Bottom()  -= 2;
    }
    else
    {
        rRect.Left()    += 2;
        rRect.Top()     += 2;
        rRect.Right()   -= 3;
        rRect.Bottom()  -= 3;
    }
}

// -----------------------------------------------------------------------

Rectangle DecorationView::DrawButton( const Rectangle& rRect, USHORT nStyle )
{
    Rectangle   aRect = rRect;
    BOOL        bOldMap = mpOutDev->IsMapModeEnabled();
    if ( bOldMap )
    {
        aRect = mpOutDev->LogicToPixel( aRect );
        mpOutDev->EnableMapMode( FALSE );
    }

    if ( !rRect.IsEmpty() )
    {
        const StyleSettings& rStyleSettings = mpOutDev->GetSettings().GetStyleSettings();

        if ( (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) ||
             (mpOutDev->GetOutDevType() == OUTDEV_PRINTER) )
            nStyle |= BUTTON_DRAW_MONO;

        if ( nStyle & BUTTON_DRAW_NODRAW )
             ImplDrawButton( mpOutDev, aRect, rStyleSettings, nStyle );
        else
        {
             Color maOldLineColor  = mpOutDev->GetLineColor();
             Color maOldFillColor  = mpOutDev->GetFillColor();
             ImplDrawButton( mpOutDev, aRect, rStyleSettings, nStyle );
             mpOutDev->SetLineColor( maOldLineColor );
             mpOutDev->SetFillColor( maOldFillColor );
        }
    }

    if ( bOldMap )
    {
        mpOutDev->EnableMapMode( bOldMap );
        aRect = mpOutDev->PixelToLogic( aRect );
    }

    return aRect;
}
