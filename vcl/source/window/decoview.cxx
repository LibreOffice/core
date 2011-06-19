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
#include "precompiled_vcl.hxx"
#include <vcl/settings.hxx>
#include <tools/poly.hxx>
#include <vcl/outdev.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/decoview.hxx>
#include <vcl/window.hxx>
#include <vcl/ctrl.hxx>

// =======================================================================

#define BUTTON_DRAW_FLATTEST    (BUTTON_DRAW_FLAT |             \
                                 BUTTON_DRAW_PRESSED |          \
                                 BUTTON_DRAW_CHECKED |          \
                                 BUTTON_DRAW_HIGHLIGHT)

// =======================================================================

static void ImplDrawSymbol( OutputDevice* pDev, const Rectangle& rRect,
                            SymbolType eType  )
{
    // Groessen vorberechnen
    long    nMin    = Min( rRect.GetWidth(), rRect.GetHeight() );
    long    nSize   = nMin;

    if ( nMin & 0x01 )
        nMin--;
    Point   aCenter = rRect.Center();
    long    nCenterX = aCenter.X();
    long    nCenterY = aCenter.Y();
    long    n2 = nMin / 2;
    long    n4 = nMin / 4;
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
            if ( !(nMin & 0x01) )
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
            if ( !(nMin & 0x01) )
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
            if ( !(nMin & 0x01) )
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
            if ( !(nMin & 0x01) )
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
            if ( !(nMin & 0x01) )
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
            if ( !(nMin & 0x01) )
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
            if ( !(nMin & 0x01) )
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
            if ( !(nMin & 0x01) )
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
        case SYMBOL_PAGEDOWN:
        {
            if ( !( nSize & 0x01 ))
            {
                // An even rectangle size means we have to use a smaller size for
                // our arrows as we want to use one pixel for the spearhead! Otherwise
                // it will be clipped!
                nCenterX++;
                n2 = ( nMin-1 ) / 2;
                n4 = ( nMin-1 ) / 4;
            }

            nTop = nCenterY-n2;
            nBottom = nCenterY-1;
            pDev->DrawRect( Rectangle( nCenterX, nTop, nCenterX, nBottom ) );
            pDev->DrawRect( Rectangle( nCenterX, nTop+n2+1, nCenterX, nBottom+n2+1 ) );
            i = 1;
            while ( i < n2 )
            {
                ( eType == SYMBOL_PAGEUP ) ? nTop++ : nBottom--;
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
            if ( nMin < 8 )
                aRectSize.Width() = 1;
            else if ( nMin > 20 )
                aRectSize.Width() = nMin/10;
            nLeft   = nCenterX-n2+1;
            nTop    = nCenterY-n2+1;
            nBottom = nCenterY-n2+nMin-aRectSize.Width()+1;
            i = 0;
            while ( i < nMin-aRectSize.Width()+1 )
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
                // #106953# never mirror checkmarks
                sal_Bool bRTL = pDev->ImplHasMirroredGraphics() && pDev->IsRTLEnabled();
                Point aPos1( bRTL ? rRect.Right() : rRect.Left(),
                    rRect.Bottom() - rRect.GetHeight() / 3 );
                Point aPos2( bRTL ? rRect.Right() - rRect.GetWidth()/3 : rRect.Left() + rRect.GetWidth()/3,
                    rRect.Bottom() );
                Point aPos3( bRTL ? rRect.TopLeft() : rRect.TopRight() );
                Size aRectSize( 1, 2 );
                long nStepsY = aPos2.Y()-aPos1.Y();
                long nX = aPos1.X();
                long nY = aPos1.Y();
                long n;
                for ( n = 0; n <= nStepsY; n++ )
                {
                    if( bRTL )
                        nX--;
                    pDev->DrawRect( Rectangle( Point( nX, nY++ ), aRectSize ) );
                    if( !bRTL )
                        nX++;
                }
                nStepsY = aPos2.Y()-aPos3.Y();
                nX = aPos2.X();
                nY = aPos2.Y();
                for ( n = 0; n <= nStepsY; n++ )
                {
                    if( bRTL )
                        if ( --nX < rRect.Left() )
                            break;
                    pDev->DrawRect( Rectangle( Point( nX, nY-- ), aRectSize ) );
                    if( !bRTL )
                        if ( ++nX > rRect.Right() )
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
            long nExtra = nMin / 8;
            Rectangle aRect( nCenterX-n2+nExtra, nCenterY+n2-1,
                             nCenterX+n2-nExtra, nCenterY+n2 );
            pDev->DrawRect( aRect );
            }
            break;
    }
}

// -----------------------------------------------------------------------

void DecorationView::DrawSymbol( const Rectangle& rRect, SymbolType eType,
                                 const Color& rColor, sal_uInt16 nStyle )
{
    const StyleSettings&    rStyleSettings  = mpOutDev->GetSettings().GetStyleSettings();
    Rectangle               aRect           = mpOutDev->LogicToPixel( rRect );
    Color                   aOldLineColor   = mpOutDev->GetLineColor();
    Color                   aOldFillColor   = mpOutDev->GetFillColor();
    sal_Bool                    bOldMapMode     = mpOutDev->IsMapModeEnabled();
    mpOutDev->SetLineColor();
    mpOutDev->SetFillColor( rColor );
    mpOutDev->EnableMapMode( sal_False );

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
    sal_Bool        bOldMapMode     = mpOutDev->IsMapModeEnabled();
    mpOutDev->EnableMapMode( sal_False );
    mpOutDev->SetLineColor();
    mpOutDev->ImplDraw2ColorFrame( aRect, rLeftTopColor, rRightBottomColor );
    mpOutDev->SetLineColor( aOldLineColor );
    mpOutDev->SetFillColor( aOldFillColor );
    mpOutDev->EnableMapMode( bOldMapMode );
}

// =======================================================================

void DecorationView::DrawHighlightFrame( const Rectangle& rRect,
                                         sal_uInt16 nStyle )
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
                                 const Color* pColor, sal_Bool bRound = sal_False )
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
            if( bRound )
            {
                pDev->DrawLine( Point( rRect.Left()+1, rRect.Top()), Point( rRect.Right()-1, rRect.Top()) );
                pDev->DrawLine( Point( rRect.Left()+1, rRect.Bottom()), Point( rRect.Right()-1, rRect.Bottom()) );
                pDev->DrawLine( Point( rRect.Left(), rRect.Top()+1), Point( rRect.Left(), rRect.Bottom()-1) );
                pDev->DrawLine( Point( rRect.Right(), rRect.Top()+1), Point( rRect.Right(), rRect.Bottom()-1) );
            }
            else
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
                           const StyleSettings& rStyleSettings, sal_uInt16 nStyle )
{
    // mask menu style
    sal_Bool bMenuStyle = (nStyle & FRAME_DRAW_MENU) ? sal_True : sal_False;
    nStyle &= ~FRAME_DRAW_MENU;

    Window *pWin = NULL;
    if( pDev->GetOutDevType() == OUTDEV_WINDOW )
        pWin = (Window*) pDev;

    // UseFlatBorders disables 3D style for all frames except menus
    // menus may use different border colors (eg on XP)
    // normal frames will be drawn using the shadow color
    // whereas window frame borders will use black
    sal_Bool bFlatBorders = ( !bMenuStyle && rStyleSettings.GetUseFlatBorders() );

    // no flat borders for standard VCL controls (ie formcontrols that keep their classic look)
    // will not affect frame windows (like dropdowns)
    if( bFlatBorders && pWin && pWin->GetType() == WINDOW_BORDERWINDOW && (pWin != pWin->ImplGetFrameWindow()) )
    {
        // check for formcontrol, i.e., a control without NWF enabled
        Control *pControl = dynamic_cast< Control* >( pWin->GetWindow( WINDOW_CLIENT ) );
        if( pControl && pControl->IsNativeWidgetEnabled() )
            bFlatBorders = sal_True;
        else
            bFlatBorders = sal_False;
    }

    // no round corners for window frame borders
    sal_Bool bRound = (bFlatBorders && !(nStyle & FRAME_DRAW_WINDOWBORDER));

    if ( (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) ||
         (pDev->GetOutDevType() == OUTDEV_PRINTER) ||
         bFlatBorders )
        nStyle |= FRAME_DRAW_MONO;

    if ( nStyle & FRAME_DRAW_NODRAW )
    {
        sal_uInt16 nValueStyle = bMenuStyle ? nStyle | FRAME_DRAW_MENU : nStyle;
        if( pWin->GetType() == WINDOW_BORDERWINDOW )
            nValueStyle |= FRAME_DRAW_BORDERWINDOWBORDER;
        ImplControlValue aControlValue( nValueStyle );
        Rectangle aBound, aContent;
        Rectangle aNatRgn( rRect );
        if(pWin && pWin->GetNativeControlRegion(CTRL_FRAME, PART_BORDER,
            aNatRgn, 0, aControlValue, rtl::OUString(), aBound, aContent) )
        {
            rRect = aContent;
        }
        else if ( nStyle & FRAME_DRAW_MONO )
            ImplDrawDPILineRect( pDev, rRect, NULL, bRound );
        else
        {
            sal_uInt16 nFrameStyle = nStyle & FRAME_DRAW_STYLE;

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
        if( pWin && pWin->IsNativeControlSupported(CTRL_FRAME, PART_BORDER) )
        {
            sal_uInt16 nValueStyle = bMenuStyle ? nStyle | FRAME_DRAW_MENU : nStyle;
            if( pWin->GetType() == WINDOW_BORDERWINDOW )
                nValueStyle |= FRAME_DRAW_BORDERWINDOWBORDER;
            ImplControlValue aControlValue( nValueStyle );
            Rectangle aBound, aContent;
            Rectangle aNatRgn( rRect );
            if( pWin->GetNativeControlRegion(CTRL_FRAME, PART_BORDER,
                aNatRgn, 0, aControlValue, rtl::OUString(), aBound, aContent) )
            {
                if( pWin->DrawNativeControl( CTRL_FRAME, PART_BORDER, aContent, CTRL_STATE_ENABLED,
                             aControlValue, rtl::OUString()) )
                {
                    rRect = aContent;
                    return;
                }
            }
        }

        if ( nStyle & FRAME_DRAW_MONO )
        {
            Color aColor = bRound ? rStyleSettings.GetShadowColor()
                                  : pDev->GetSettings().GetStyleSettings().GetMonoColor();
            // when the MonoColor wasn't set, check face color
            if (
                (bRound && aColor.IsDark()) ||
                (
                  (aColor == Color(COL_BLACK)) &&
                  (pDev->GetSettings().GetStyleSettings().GetFaceColor().IsDark())
                )
               )
            {
                aColor = Color( COL_WHITE );
            }
            ImplDrawDPILineRect( pDev, rRect, &aColor, bRound );
        }
        else
        {
            sal_uInt16 nFrameStyle = nStyle & FRAME_DRAW_STYLE;
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
                        if( bFlatBorders ) // no 3d effect
                            pDev->ImplDraw2ColorFrame( rRect,
                                                    rStyleSettings.GetShadowColor(),
                                                    rStyleSettings.GetShadowColor() );
                        else
                            pDev->ImplDraw2ColorFrame( rRect,
                                                    rStyleSettings.GetShadowColor(),
                                                    rStyleSettings.GetLightColor() );
                    }
                    else
                    {
                        if( bMenuStyle )
                            pDev->ImplDraw2ColorFrame( rRect,
                                                   rStyleSettings.GetMenuBorderColor(),
                                                   rStyleSettings.GetDarkShadowColor() );
                        else
                            pDev->ImplDraw2ColorFrame( rRect,
                                                   bFlatBorders ? // no 3d effect
                                                   rStyleSettings.GetDarkShadowColor() :
                                                   rStyleSettings.GetLightBorderColor(),
                                                   rStyleSettings.GetDarkShadowColor() );

                    }

                    rRect.Left()++;
                    rRect.Top()++;
                    rRect.Right()--;
                    rRect.Bottom()--;

                    sal_Bool bDrawn = sal_True;
                    if ( nFrameStyle == FRAME_DRAW_DOUBLEIN )
                    {
                        if( bFlatBorders ) // no 3d effect
                            pDev->ImplDraw2ColorFrame( rRect,
                                                    rStyleSettings.GetFaceColor(),
                                                    rStyleSettings.GetFaceColor() );
                        else
                            pDev->ImplDraw2ColorFrame( rRect,
                                                    rStyleSettings.GetDarkShadowColor(),
                                                    rStyleSettings.GetLightBorderColor() );
                    }
                    else
                    {
                        // flat menues have no shadow border
                        if( !bMenuStyle || !rStyleSettings.GetUseFlatMenues() )
                            pDev->ImplDraw2ColorFrame( rRect,
                                                    rStyleSettings.GetLightColor(),
                                                    rStyleSettings.GetShadowColor() );
                        else
                            bDrawn = sal_False;
                    }
                    if( bDrawn )
                    {
                        rRect.Left()++;
                        rRect.Top()++;
                        rRect.Right()--;
                        rRect.Bottom()--;
                    }
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

Rectangle DecorationView::DrawFrame( const Rectangle& rRect, sal_uInt16 nStyle )
{
    Rectangle   aRect = rRect;
    sal_Bool        bOldMap = mpOutDev->IsMapModeEnabled();
    if ( bOldMap )
    {
        aRect = mpOutDev->LogicToPixel( aRect );
        mpOutDev->EnableMapMode( sal_False );
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
                            const StyleSettings& rStyleSettings, sal_uInt16 nStyle )
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
            if ( (( (nStyle & BUTTON_DRAW_NOBOTTOMSHADOWBORDER) | BUTTON_DRAW_FLAT) == (BUTTON_DRAW_NOBOTTOMSHADOWBORDER | BUTTON_DRAW_FLAT)) &&
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

Rectangle DecorationView::DrawButton( const Rectangle& rRect, sal_uInt16 nStyle )
{
    Rectangle   aRect = rRect;
    sal_Bool        bOldMap = mpOutDev->IsMapModeEnabled();
    if ( bOldMap )
    {
        aRect = mpOutDev->LogicToPixel( aRect );
        mpOutDev->EnableMapMode( sal_False );
    }

    if ( !rRect.IsEmpty() )
    {
        const StyleSettings& rStyleSettings = mpOutDev->GetSettings().GetStyleSettings();

        if ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO )
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

// -----------------------------------------------------------------------

void DecorationView::DrawSeparator( const Point& rStart, const Point& rStop, bool bVertical )
{
    Point aStart( rStart ), aStop( rStop );
    const StyleSettings& rStyleSettings = mpOutDev->GetSettings().GetStyleSettings();

    mpOutDev->Push( PUSH_LINECOLOR );
    if ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO )
        mpOutDev->SetLineColor( Color( COL_BLACK ) );
    else
        mpOutDev->SetLineColor( rStyleSettings.GetShadowColor() );

    mpOutDev->DrawLine( aStart, aStop );
    if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
    {
        mpOutDev->SetLineColor( rStyleSettings.GetLightColor() );
        if( bVertical )
        {
            aStart.X()++;
            aStop.X()++;
        }
        else
        {
            aStart.Y()++;
            aStop.Y()++;
        }
        mpOutDev->DrawLine( aStart, aStop );
    }
    mpOutDev->Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
