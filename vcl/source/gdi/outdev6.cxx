/*************************************************************************
 *
 *  $RCSfile: outdev6.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
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

#define _SV_WALL_CXX

#include <math.h>
#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#else // REMOTE_APPSERVER
#include <tools/stream.hxx>
#endif // REMOTE_APPSERVER
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <graph.hxx>
#endif
#ifdef REMOTE_APPSERVER
#include <rmoutdev.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

// ========================================================================

DBG_NAMEEX( OutputDevice );

// ------------------------------------------------------------------------

void OutputDevice::DrawGrid( const Rectangle& rRect, const Size& rDist, ULONG nFlags )
{
    DBG_TRACE( "OutputDevice::DrawGrid()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Rectangle aDstRect( PixelToLogic( Point() ), GetOutputSize() );
    aDstRect.Intersection( rRect );

    if( aDstRect.IsEmpty() )
        return;

#ifndef REMOTE_APPSERVER
    if( !mpGraphics && !ImplGetGraphics() )
        return;

    if( mbInitClipRegion )
        ImplInitClipRegion();

    if( mbOutputClipped )
        return;
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();

    if( !pGraphics )
        return;
#endif

    const long  nDistX = Max( rDist.Width(), 1L );
    const long  nDistY = Max( rDist.Height(), 1L );
    long        nX = ( rRect.Left() >= aDstRect.Left() ) ? rRect.Left() : ( rRect.Left() + ( ( aDstRect.Left() - rRect.Left() ) / nDistX ) * nDistX );
    long        nY = ( rRect.Top() >= aDstRect.Top() ) ? rRect.Top() : ( rRect.Top() + ( ( aDstRect.Top() - rRect.Top() ) / nDistY ) * nDistY );
    const long  nRight = aDstRect.Right();
    const long  nBottom = aDstRect.Bottom();
    const long  nStartX = ImplLogicXToDevicePixel( nX );
    const long  nEndX = ImplLogicXToDevicePixel( nRight );
    const long  nStartY = ImplLogicYToDevicePixel( nY );
    const long  nEndY = ImplLogicYToDevicePixel( nBottom );
    long        nHorzCount = 0L;
    long        nVertCount = 0L;

    ::com::sun::star::uno::Sequence< sal_Int32 > aVertBuf;
    ::com::sun::star::uno::Sequence< sal_Int32 > aHorzBuf;

    if( ( nFlags & GRID_DOTS ) || ( nFlags & GRID_HORZLINES ) )
    {
        aVertBuf.realloc( aDstRect.GetHeight() / nDistY + 2L );
        aVertBuf[ nVertCount++ ] = nStartY;
        while( ( nY += nDistY ) <= nBottom )
            aVertBuf[ nVertCount++ ] = ImplLogicYToDevicePixel( nY );
    }

    if( ( nFlags & GRID_DOTS ) || ( nFlags & GRID_VERTLINES ) )
    {
        aHorzBuf.realloc( aDstRect.GetWidth() / nDistX + 2L );
        aHorzBuf[ nHorzCount++ ] = nStartX;
        while( ( nX += nDistX ) <= nRight )
            aHorzBuf[ nHorzCount++ ] = ImplLogicXToDevicePixel( nX );
    }

    if( mbInitLineColor )
        ImplInitLineColor();

    if( mbInitFillColor )
        ImplInitFillColor();

#ifndef REMOTE_APPSERVER
    const BOOL bOldMap = mbMap;
    mbMap = FALSE;

    if( nFlags & GRID_DOTS )
    {
        for( long i = 0L; i < nVertCount; i++ )
            for( long j = 0L, nY = aVertBuf[ i ]; j < nHorzCount; j++ )
                mpGraphics->DrawPixel( aHorzBuf[ j ], nY );
    }
    else
    {
        if( nFlags & GRID_HORZLINES )
        {
            for( long i = 0L; i < nVertCount; i++ )
            {
                nY = aVertBuf[ i ];
                mpGraphics->DrawLine( nStartX, nY, nEndX, nY );
            }
        }

        if( nFlags & GRID_VERTLINES )
        {
            for( long i = 0L; i < nHorzCount; i++ )
            {
                nX = aHorzBuf[ i ];
                mpGraphics->DrawLine( nX, nStartY, nX, nEndY );
            }
        }
    }

    mbMap = bOldMap;
#else // REMOTE_APPSERVER
    aHorzBuf.realloc( nHorzCount );
    aVertBuf.realloc( nVertCount );
    pGraphics->DrawGrid( nStartX, nEndX, aHorzBuf, nStartY, nEndY, aVertBuf, nFlags );
#endif // REMOTE_APPSERVER
}

// ------------------------------------------------------------------------

void OutputDevice::DrawTransparent( const PolyPolygon& rPolyPoly,
                                    USHORT nTransparencePercent )
{
    DBG_TRACE( "OutputDevice::DrawTransparent()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if( !mbFillColor || ( 0 == nTransparencePercent ) )
        DrawPolyPolygon( rPolyPoly );
    else if( 100 == nTransparencePercent )
    {
        Push( PUSH_FILLCOLOR );
        SetFillColor();
        DrawPolyPolygon( rPolyPoly );
        Pop();
    }
    else
    {
        if( mpMetaFile )
            mpMetaFile->AddAction( new MetaTransparentAction( rPolyPoly, nTransparencePercent ) );

        if( !IsDeviceOutputNecessary() || ( !mbLineColor && !mbFillColor ) )
            return;

        GDIMetaFile* pOldMetaFile = mpMetaFile;
        mpMetaFile = NULL;

        if( OUTDEV_PRINTER == meOutDevType )
        {
            Rectangle       aPolyRect( LogicToPixel( rPolyPoly ).GetBoundRect() );
            const Size      aDPISize( LogicToPixel( Size( 1, 1 ), MAP_INCH ) );
            const long      nBaseExtent = Max( FRound( aDPISize.Width() / 300. ), 1L );
            long            nMove;
            const USHORT    nTrans = ( nTransparencePercent < 13 ) ? 0 :
                                     ( nTransparencePercent < 38 ) ? 25 :
                                     ( nTransparencePercent < 63 ) ? 50 :
                                     ( nTransparencePercent < 88 ) ? 75 : 100;

            switch( nTrans )
            {
                case( 25 ): nMove = nBaseExtent * 3; break;
                case( 50 ): nMove = nBaseExtent * 4; break;
                case( 75 ): nMove = nBaseExtent * 6; break;
            }

            Push( PUSH_CLIPREGION | PUSH_LINECOLOR );
            IntersectClipRegion( rPolyPoly );
            SetLineColor( GetFillColor() );

            Rectangle aRect( aPolyRect.TopLeft(), Size( aPolyRect.GetWidth(), nBaseExtent ) );

            const BOOL bOldMap = mbMap;
            mbMap = FALSE;

            while( aRect.Top() <= aPolyRect.Bottom() )
            {
                DrawRect( aRect );
                aRect.Move( 0, nMove );
            }

            aRect = Rectangle( aPolyRect.TopLeft(), Size( nBaseExtent, aPolyRect.GetHeight() ) );
            while( aRect.Left() <= aPolyRect.Right() )
            {
                DrawRect( aRect );
                aRect.Move( nMove, 0 );
            }

            mbMap = bOldMap;
            Pop();
        }
        else
        {
#ifndef REMOTE_APPSERVER
            PolyPolygon     aPolyPoly( LogicToPixel( rPolyPoly ) );
            Rectangle       aPolyRect( aPolyPoly.GetBoundRect() );
            Point           aPoint;
            Rectangle       aDstRect( aPoint, GetOutputSizePixel() );

            aDstRect.Intersection( aPolyRect );

            if( OUTDEV_WINDOW == meOutDevType )
            {
                const Region aPaintRgn( ( (Window*) this )->GetPaintRegion() );

                if( !aPaintRgn.IsNull() )
                    aDstRect.Intersection( LogicToPixel( aPaintRgn ).GetBoundRect() );
            }

            if( !aDstRect.IsEmpty() )
            {
                VirtualDevice   aVDev( *this, 1 );
                const Size      aDstSz( aDstRect.GetSize() );
                const BYTE      cTrans = (BYTE) MinMax( FRound( nTransparencePercent * 2.55 ), 0, 255 );

                if( aDstRect.Left() || aDstRect.Top() )
                    aPolyPoly.Move( -aDstRect.Left(), -aDstRect.Top() );

                if( aVDev.SetOutputSizePixel( aDstSz ) )
                {
                    const BOOL bOldMap = mbMap;

                    mbMap = FALSE;

                    aVDev.SetLineColor( COL_BLACK );
                    aVDev.SetFillColor( COL_BLACK );
                    aVDev.DrawPolyPolygon( aPolyPoly );

                    Bitmap              aPaint( GetBitmap( aDstRect.TopLeft(), aDstSz ) );
                    Bitmap              aPolyMask( aVDev.GetBitmap( Point(), aDstSz ) );
                    BitmapWriteAccess*  pW = aPaint.AcquireWriteAccess();
                    BitmapReadAccess*   pR = aPolyMask.AcquireReadAccess();

                    if( pW && pR )
                    {
                        BitmapColor         aPixCol;
                        const BitmapColor   aFillCol( GetFillColor() );
                        const BitmapColor   aWhite( pR->GetBestMatchingColor( Color( COL_WHITE ) ) );
                        const BitmapColor   aBlack( pR->GetBestMatchingColor( Color( COL_BLACK ) ) );
                        const long          nWidth = pW->Width(), nHeight = pW->Height();
                        const long          nR = aFillCol.GetRed(), nG = aFillCol.GetGreen(), nB = aFillCol.GetBlue();
                        long                nX, nY;

                        if( aPaint.GetBitCount() <= 8 )
                        {
                            const BitmapPalette&    rPal = pW->GetPalette();
                            const USHORT            nCount = rPal.GetEntryCount();
                            BitmapColor*            pMap = (BitmapColor*) new BYTE[ nCount * sizeof( BitmapColor ) ];

                            for( USHORT i = 0; i < nCount; i++ )
                            {
                                BitmapColor aCol( rPal[ i ] );
                                pMap[ i ] = BitmapColor( (BYTE) rPal.GetBestIndex( aCol.Merge( aFillCol, cTrans ) ) );
                            }

                            if( pR->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL &&
                                pW->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL )
                            {
                                const BYTE cBlack = aBlack.GetIndex();

                                for( nY = 0; nY < nHeight; nY++ )
                                {
                                    Scanline    pWScan = pW->GetScanline( nY );
                                    Scanline    pRScan = pR->GetScanline( nY );
                                    BYTE        cBit = 128;

                                    for( nX = 0; nX < nWidth; nX++, cBit >>= 1, pWScan++ )
                                    {
                                        if( !cBit )
                                            cBit = 128, pRScan++;

                                        if( ( *pRScan & cBit ) == cBlack )
                                            *pWScan = (BYTE) pMap[ *pWScan ].GetIndex();
                                    }
                                }
                            }
                            else
                            {
                                for( nY = 0; nY < nHeight; nY++ )
                                    for( nX = 0; nX < nWidth; nX++ )
                                        if( pR->GetPixel( nY, nX ) == aBlack )
                                            pW->SetPixel( nY, nX, pMap[ pW->GetPixel( nY, nX ).GetIndex() ] );
                            }

                            delete[] (BYTE*) pMap;
                        }
                        else
                        {
                            if( pR->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL &&
                                pW->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_BGR )
                            {
                                const BYTE cBlack = aBlack.GetIndex();

                                for( nY = 0; nY < nHeight; nY++ )
                                {
                                    Scanline    pWScan = pW->GetScanline( nY );
                                    Scanline    pRScan = pR->GetScanline( nY );
                                    BYTE        cBit = 128;

                                    for( nX = 0; nX < nWidth; nX++, cBit >>= 1, pWScan += 3 )
                                    {
                                        if( !cBit )
                                            cBit = 128, pRScan++;

                                        if( ( *pRScan & cBit ) == cBlack )
                                        {
                                            pWScan[ 0 ] = COLOR_CHANNEL_MERGE( pWScan[ 0 ], nB, cTrans );
                                            pWScan[ 1 ] = COLOR_CHANNEL_MERGE( pWScan[ 1 ], nG, cTrans );
                                            pWScan[ 2 ] = COLOR_CHANNEL_MERGE( pWScan[ 2 ], nR, cTrans );
                                        }
                                    }
                                }
                            }
                            else
                            {
                                for( nY = 0; nY < nHeight; nY++ )
                                {
                                    for( nX = 0; nX < nWidth; nX++ )
                                    {
                                        if( pR->GetPixel( nY, nX ) == aBlack )
                                        {
                                            aPixCol = pW->GetColor( nY, nX );
                                            pW->SetPixel( nY, nX, aPixCol.Merge( aFillCol, cTrans ) );
                                        }
                                    }
                                }
                            }
                        }
                    }

                    aPolyMask.ReleaseAccess( pR );
                    aPaint.ReleaseAccess( pW );

                    DrawBitmap( aDstRect.TopLeft(), aPaint );

                    mbMap = bOldMap;

                    if( mbLineColor )
                    {
                        Push( PUSH_FILLCOLOR );
                        SetFillColor();
                        DrawPolyPolygon( rPolyPoly );
                        Pop();
                    }
                }
                else
                    DrawPolyPolygon( rPolyPoly );
            }
#else // REMOTE_APPSERVER
            ImplServerGraphics* pGraphics = ImplGetServerGraphics();
            if ( pGraphics )
            {
                if ( mbInitLineColor )
                    ImplInitLineColor();
                if ( mbInitFillColor )
                    ImplInitFillColor();
                pGraphics->DrawTransparent( ImplLogicToDevicePixel( rPolyPoly ), nTransparencePercent );
            }
#endif // REMOTE_APPSERVER
        }

        mpMetaFile = pOldMetaFile;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::DrawTransparent( const GDIMetaFile& rMtf, const Point& rPos,
                                    const Size& rSize, const Gradient& rTransparenceGradient )
{
    DBG_TRACE( "OutputDevice::DrawTransparent()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    const Color aBlack( COL_BLACK );

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaFloatTransparentAction( rMtf, rPos, rSize, rTransparenceGradient ) );

    if( rTransparenceGradient.GetStartColor() == aBlack && rTransparenceGradient.GetEndColor() == aBlack )
    {
        ( (GDIMetaFile&) rMtf ).WindStart();
        ( (GDIMetaFile&) rMtf ).Play( this, rPos, rSize );
        ( (GDIMetaFile&) rMtf ).WindStart();
    }
    else
    {
        GDIMetaFile*    pOldMetaFile = mpMetaFile;
        Rectangle       aOutRect( LogicToPixel( rPos ), LogicToPixel( rSize ) );
        Point           aPoint;
        Rectangle       aDstRect( aPoint, GetOutputSizePixel() );

        mpMetaFile = NULL;
        aDstRect.Intersection( aOutRect );

        if( OUTDEV_WINDOW == meOutDevType )
        {
            const Region aPaintRgn( ( (Window*) this )->GetPaintRegion() );

            if( !aPaintRgn.IsNull() )
                aDstRect.Intersection( LogicToPixel( aPaintRgn.GetBoundRect() ) );
        }

        if( !aDstRect.IsEmpty() )
        {
            VirtualDevice* pVDev = new VirtualDevice;

            pVDev->mnDPIX = mnDPIX;
            pVDev->mnDPIY = mnDPIY;

            if( pVDev->SetOutputSizePixel( aDstRect.GetSize() ) )
            {
                Bitmap      aPaint, aMask;
                AlphaMask   aAlpha;
                MapMode     aMap( GetMapMode() );
                Point       aOutPos( PixelToLogic( aDstRect.TopLeft() ) );
                const BOOL  bOldMap = mbMap;

                aMap.SetOrigin( Point( -aOutPos.X(), -aOutPos.Y() ) );
                pVDev->SetMapMode( aMap );

                // create paint bitmap
                ( (GDIMetaFile&) rMtf ).WindStart();
                ( (GDIMetaFile&) rMtf ).Play( pVDev, rPos, rSize );
                ( (GDIMetaFile&) rMtf ).WindStart();
                pVDev->EnableMapMode( FALSE );
                aPaint = pVDev->GetBitmap( Point(), pVDev->GetOutputSizePixel() );
                pVDev->EnableMapMode( TRUE );

                // create mask bitmap
                pVDev->SetLineColor( COL_BLACK );
                pVDev->SetFillColor( COL_BLACK );
                pVDev->DrawRect( Rectangle( pVDev->PixelToLogic( Point() ), pVDev->GetOutputSize() ) );
                pVDev->SetDrawMode( DRAWMODE_WHITELINE | DRAWMODE_WHITEFILL | DRAWMODE_WHITETEXT |
                                    DRAWMODE_WHITEBITMAP | DRAWMODE_WHITEGRADIENT );
                ( (GDIMetaFile&) rMtf ).WindStart();
                ( (GDIMetaFile&) rMtf ).Play( pVDev, rPos, rSize );
                ( (GDIMetaFile&) rMtf ).WindStart();
                pVDev->EnableMapMode( FALSE );
                aMask = pVDev->GetBitmap( Point(), pVDev->GetOutputSizePixel() );
                pVDev->EnableMapMode( TRUE );

                // create alpha mask from gradient
                pVDev->SetDrawMode( DRAWMODE_GRAYGRADIENT );
                pVDev->DrawGradient( Rectangle( rPos, rSize ), rTransparenceGradient );
                pVDev->SetDrawMode( DRAWMODE_DEFAULT );
                pVDev->EnableMapMode( FALSE );
                pVDev->DrawMask( Point(), pVDev->GetOutputSizePixel(), aMask, Color( COL_WHITE ) );

#ifndef REMOTE_APPSERVER
                aAlpha = pVDev->GetBitmap( Point(), pVDev->GetOutputSizePixel() );
#else
                aAlpha.ImplSetBitmap( pVDev->GetBitmap( Point(), pVDev->GetOutputSizePixel() ) );
#endif

                delete pVDev;

                mbMap = FALSE;
                DrawBitmapEx( aDstRect.TopLeft(), BitmapEx( aPaint, aAlpha ) );
                mbMap = bOldMap;
            }
            else
                delete pVDev;
        }

        mpMetaFile = pOldMetaFile;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawColorWallpaper( long nX, long nY,
                                           long nWidth, long nHeight,
                                           const Wallpaper& rWallpaper )
{
#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    // Wallpaper ohne Umrandung zeichnen
    Color aOldLineColor = GetLineColor();
    Color aOldFillColor = GetFillColor();
    SetLineColor();
    SetFillColor( rWallpaper.GetColor() );
    if ( mbInitLineColor )
        ImplInitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();
    mpGraphics->DrawRect( nX+mnOutOffX, nY+mnOutOffY, nWidth, nHeight );
    SetLineColor( aOldLineColor );
    SetFillColor( aOldFillColor );
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        Color aOldLineColor = GetLineColor();
        Color aOldFillColor = GetFillColor();
        SetLineColor();
        SetFillColor( rWallpaper.GetColor() );
        if ( mbInitLineColor )
            ImplInitLineColor();
        if ( mbInitFillColor )
            ImplInitFillColor();
        pGraphics->DrawRect( Rectangle( Point( nX+mnOutOffX, nY+mnOutOffY ), Size( nWidth, nHeight ) ) );
        SetLineColor( aOldLineColor );
        SetFillColor( aOldFillColor );
    }
#endif
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawBitmapWallpaper( long nX, long nY,
                                            long nWidth, long nHeight,
                                            const Wallpaper& rWallpaper )
{
    BitmapEx                aBmpEx;
    const BitmapEx*         pCached = rWallpaper.ImplGetImpWallpaper()->ImplGetCachedBitmap();
    Point                   aPos;
    Size                    aSize;
    GDIMetaFile*            pOldMetaFile = mpMetaFile;
    const WallpaperStyle    eStyle = rWallpaper.GetStyle();
    const BOOL              bOldMap = mbMap;
    BOOL                    bDrawn = FALSE;
    BOOL                    bDrawGradientBackground = FALSE;
    BOOL                    bDrawColorBackground = FALSE;

    if( pCached )
        aBmpEx = *pCached;
    else
        aBmpEx = rWallpaper.GetBitmap();

    const long nBmpWidth = aBmpEx.GetSizePixel().Width();
    const long nBmpHeight = aBmpEx.GetSizePixel().Height();
    const BOOL bTransparent = aBmpEx.IsTransparent();

    // draw background
    if( bTransparent )
    {
        if( rWallpaper.IsGradient() )
            bDrawGradientBackground = TRUE;
        else
        {
            if( !pCached && !rWallpaper.GetColor().GetTransparency() )
            {
                VirtualDevice aVDev( *this );
                aVDev.SetBackground( rWallpaper.GetColor() );
                aVDev.SetOutputSizePixel( Size( nBmpWidth, nBmpHeight ) );
                aVDev.DrawBitmapEx( Point(), aBmpEx );
                aBmpEx = aVDev.GetBitmap( Point(), aVDev.GetOutputSizePixel() );
            }

            bDrawColorBackground = TRUE;
        }
    }
    else if( eStyle != WALLPAPER_TILE && eStyle != WALLPAPER_SCALE )
    {
        if( rWallpaper.IsGradient() )
            bDrawGradientBackground = TRUE;
        else
            bDrawColorBackground = TRUE;
    }

    // background of bitmap?
    if( bDrawGradientBackground )
        ImplDrawGradientWallpaper( nX, nY, nWidth, nHeight, rWallpaper );
    else if( bDrawColorBackground && bTransparent )
    {
        ImplDrawColorWallpaper( nX, nY, nWidth, nHeight, rWallpaper );
        bDrawColorBackground = FALSE;
    }

    // calc pos and size
    if( rWallpaper.IsRect() )
    {
        const Rectangle aBound( LogicToPixel( rWallpaper.GetRect() ) );
        aPos = aBound.TopLeft();
        aSize = aBound.GetSize();
    }
    else
    {
        aPos = Point( nX, nY );
        aSize = Size( nWidth, nHeight );
    }

    mpMetaFile = NULL;
    mbMap = FALSE;
    Push( PUSH_CLIPREGION );
    IntersectClipRegion( Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ) );

    switch( eStyle )
    {
        case( WALLPAPER_SCALE ):
        {
            if( !pCached || ( pCached->GetSizePixel() != aSize ) )
            {
                if( pCached )
                    rWallpaper.ImplGetImpWallpaper()->ImplReleaseCachedBitmap();

                aBmpEx = rWallpaper.GetBitmap();
                aBmpEx.Scale( aSize );
                aBmpEx = BitmapEx( aBmpEx.GetBitmap().CreateDisplayBitmap( this ), aBmpEx.GetMask() );
            }
        }
        break;

        case( WALLPAPER_TOPLEFT ):
        break;

        case( WALLPAPER_TOP ):
            aPos.X() += ( aSize.Width() - nBmpWidth ) >> 1;
        break;

        case( WALLPAPER_TOPRIGHT ):
            aPos.X() += ( aSize.Width() - nBmpWidth );
        break;

        case( WALLPAPER_LEFT ):
            aPos.Y() += ( aSize.Height() - nBmpHeight ) >> 1;
        break;

        case( WALLPAPER_CENTER ):
        {
            aPos.X() += ( aSize.Width() - nBmpWidth ) >> 1;
            aPos.Y() += ( aSize.Height() - nBmpHeight ) >> 1;
        }
        break;

        case( WALLPAPER_RIGHT ):
        {
            aPos.X() += ( aSize.Width() - nBmpWidth );
            aPos.Y() += ( aSize.Height() - nBmpHeight ) >> 1;
        }
        break;

        case( WALLPAPER_BOTTOMLEFT ):
            aPos.Y() += ( aSize.Height() - nBmpHeight );
        break;

        case( WALLPAPER_BOTTOM ):
        {
            aPos.X() += ( aSize.Width() - nBmpWidth ) >> 1;
            aPos.Y() += ( aSize.Height() - nBmpHeight );
        }
        break;

        case( WALLPAPER_BOTTOMRIGHT ):
        {
            aPos.X() += ( aSize.Width() - nBmpWidth );
            aPos.Y() += ( aSize.Height() - nBmpHeight );
        }
        break;

        default:
        {
            const long  nRight = nX + nWidth - 1L;
            const long  nBottom = nY + nHeight - 1L;
            long        nFirstX;
            long        nFirstY;

            if( eStyle == WALLPAPER_TILE )
            {
                nFirstX = aPos.X();
                nFirstY = aPos.Y();
            }
            else
            {
                nFirstX = aPos.X() + ( ( aSize.Width() - nBmpWidth ) >> 1 );
                nFirstY = aPos.Y() + ( ( aSize.Height() - nBmpHeight ) >> 1 );
            }

            const long  nOffX = ( nFirstX - nX ) % nBmpWidth;
            const long  nOffY = ( nFirstY - nY ) % nBmpHeight;
            long        nStartX = nX + nOffX;
            long        nStartY = nY + nOffY;

            if( nOffX > 0L )
                nStartX -= nBmpWidth;

            if( nOffY > 0L )
                nStartY -= nBmpHeight;

            for( long nBmpY = nStartY; nBmpY <= nBottom; nBmpY += nBmpHeight )
                for( long nBmpX = nStartX; nBmpX <= nRight; nBmpX += nBmpWidth )
                    DrawBitmapEx( Point( nBmpX, nBmpY ), aBmpEx );

            bDrawn = TRUE;
        }
        break;
    }

    if( !bDrawn )
    {
        // optimized for non-transparent bitmaps
        if( bDrawColorBackground )
        {
            const Size      aBmpSize( aBmpEx.GetSizePixel() );
            const Point     aTmpPoint;
            const Rectangle aOutRect( aTmpPoint, GetOutputSizePixel() );
            const Rectangle aColRect( Point( nX, nY ), Size( nWidth, nHeight ) );
            Rectangle       aWorkRect;

            aWorkRect = Rectangle( 0, 0, aOutRect.Right(), aPos.Y() - 1L );
            aWorkRect.Justify();
            aWorkRect.Intersection( aColRect );
            if( !aWorkRect.IsEmpty() )
            {
                ImplDrawColorWallpaper( aWorkRect.Left(), aWorkRect.Top(),
                                        aWorkRect.GetWidth(), aWorkRect.GetHeight(),
                                        rWallpaper );
            }

            aWorkRect = Rectangle( 0, aPos.Y(), aPos.X() - 1L, aPos.Y() + aBmpSize.Height() - 1L );
            aWorkRect.Justify();
            aWorkRect.Intersection( aColRect );
            if( !aWorkRect.IsEmpty() )
            {
                ImplDrawColorWallpaper( aWorkRect.Left(), aWorkRect.Top(),
                                        aWorkRect.GetWidth(), aWorkRect.GetHeight(),
                                        rWallpaper );
            }

            aWorkRect = Rectangle( aPos.X() + aBmpSize.Width(), aPos.Y(), aOutRect.Right(), aPos.Y() + aBmpSize.Height() - 1L );
            aWorkRect.Justify();
            aWorkRect.Intersection( aColRect );
            if( !aWorkRect.IsEmpty() )
            {
                ImplDrawColorWallpaper( aWorkRect.Left(), aWorkRect.Top(),
                                        aWorkRect.GetWidth(), aWorkRect.GetHeight(),
                                        rWallpaper );
            }

            aWorkRect = Rectangle( 0, aPos.Y() + aBmpSize.Height(), aOutRect.Right(), aOutRect.Bottom() );
            aWorkRect.Justify();
            aWorkRect.Intersection( aColRect );
            if( !aWorkRect.IsEmpty() )
            {
                ImplDrawColorWallpaper( aWorkRect.Left(), aWorkRect.Top(),
                                        aWorkRect.GetWidth(), aWorkRect.GetHeight(),
                                        rWallpaper );
            }
        }

        DrawBitmapEx( aPos, aBmpEx );
    }

    rWallpaper.ImplGetImpWallpaper()->ImplSetCachedBitmap( aBmpEx );

    Pop();
    mbMap = bOldMap;
    mpMetaFile = pOldMetaFile;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawGradientWallpaper( long nX, long nY,
                                              long nWidth, long nHeight,
                                              const Wallpaper& rWallpaper )
{
    Rectangle       aBound;
    GDIMetaFile*    pOldMetaFile = mpMetaFile;
    const BOOL      bOldMap = mbMap;

/*
    if ( rWallpaper.IsRect() )
        aBound = LogicToPixel( rWallpaper.GetRect() );
    else
*/
        aBound = Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );

    mpMetaFile = NULL;
    mbMap = FALSE;
    Push( PUSH_CLIPREGION );
    IntersectClipRegion( Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ) );

    DrawGradient( aBound, rWallpaper.GetGradient() );

    Pop();
    mbMap = bOldMap;
    mpMetaFile = pOldMetaFile;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDrawWallpaper( long nX, long nY,
                                      long nWidth, long nHeight,
                                      const Wallpaper& rWallpaper )
{
    if( rWallpaper.IsBitmap() )
        ImplDrawBitmapWallpaper( nX, nY, nWidth, nHeight, rWallpaper );
    else if( rWallpaper.IsGradient() )
        ImplDrawGradientWallpaper( nX, nY, nWidth, nHeight, rWallpaper );
    else
        ImplDrawColorWallpaper(  nX, nY, nWidth, nHeight, rWallpaper );
}

// -----------------------------------------------------------------------

void OutputDevice::DrawWallpaper( const Rectangle& rRect,
                                  const Wallpaper& rWallpaper )
{
    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaWallpaperAction( rRect, rWallpaper ) );

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( rWallpaper.GetStyle() != WALLPAPER_NULL )
    {
        Rectangle aRect = LogicToPixel( rRect );
        aRect.Justify();

        if ( !aRect.IsEmpty() )
        {
            ImplDrawWallpaper( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(),
                               rWallpaper );
        }
    }
}

// -----------------------------------------------------------------------

void OutputDevice::Erase()
{
    if ( !IsDeviceOutputNecessary() )
        return;

    if ( mbBackground )
    {
        RasterOp eRasterOp = GetRasterOp();
        if ( eRasterOp != ROP_OVERPAINT )
            SetRasterOp( ROP_OVERPAINT );
        ImplDrawWallpaper( 0, 0, mnOutWidth, mnOutHeight, maBackground );
        if ( eRasterOp != ROP_OVERPAINT )
            SetRasterOp( eRasterOp );
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDraw2ColorFrame( const Rectangle& rRect,
                                        const Color& rLeftTopColor,
                                        const Color& rRightBottomColor )
{
#ifndef REMOTE_APPSERVER
    SetFillColor( rLeftTopColor );
    DrawRect( Rectangle( rRect.TopLeft(), Point( rRect.Left(), rRect.Bottom()-1 ) ) );
    DrawRect( Rectangle( rRect.TopLeft(), Point( rRect.Right()-1, rRect.Top() ) ) );
    SetFillColor( rRightBottomColor );
    DrawRect( Rectangle( rRect.BottomLeft(), rRect.BottomRight() ) );
    DrawRect( Rectangle( rRect.TopRight(), rRect.BottomRight() ) );
#else
    if ( mpMetaFile )
    {
        BOOL bOutputEnabled = IsOutputEnabled();
        EnableOutput( FALSE );
        SetFillColor( rLeftTopColor );
        DrawRect( Rectangle( rRect.TopLeft(), Point( rRect.Left(), rRect.Bottom()-1 ) ) );
        DrawRect( Rectangle( rRect.TopLeft(), Point( rRect.Right()-1, rRect.Top() ) ) );
        SetFillColor( rRightBottomColor );
        DrawRect( Rectangle( rRect.BottomLeft(), rRect.BottomRight() ) );
        DrawRect( Rectangle( rRect.TopRight(), rRect.BottomRight() ) );
        EnableOutput( bOutputEnabled );
    }

    if ( IsDeviceOutputNecessary() && !rRect.IsEmpty() )
    {
        ImplServerGraphics* pGraphics = ImplGetServerGraphics();
        if ( pGraphics )
        {
            if ( mbInitLineColor )
                ImplInitLineColor();
            Rectangle aRect( ImplLogicToDevicePixel( rRect ) );
            pGraphics->Draw2ColorFrame( aRect, rLeftTopColor, rRightBottomColor );
        }
    }
    SetFillColor( rRightBottomColor );
#endif
}

// -----------------------------------------------------------------------

void OutputDevice::DrawEPS( const Point& rPoint, const Size& rSize,
                            const GfxLink& rGfxLink, GDIMetaFile* pSubst )
{
    if ( mpMetaFile )
    {
        GDIMetaFile aSubst;

        if( pSubst )
            aSubst = *pSubst;

        mpMetaFile->AddAction( new MetaEPSAction( rPoint, rSize, rGfxLink, aSubst ) );
    }

    if ( !IsDeviceOutputNecessary() )
        return;

    Rectangle   aRect( ImplLogicToDevicePixel( Rectangle( rPoint, rSize ) ) );
    BOOL        bDrawn = FALSE;

    if( !aRect.IsEmpty() )
    {
        aRect.Justify();

        if( GetOutDevType() == OUTDEV_PRINTER )
        {
#ifndef REMOTE_APPSERVER
            if( !mpGraphics && !ImplGetGraphics() )
                return;

            if( mbInitClipRegion )
                ImplInitClipRegion();

            if( !mbOutputClipped )
            {
                bDrawn = mpGraphics->DrawEPS( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(),
                                              (BYTE*) rGfxLink.GetData(), rGfxLink.GetDataSize() );
            }
#else
            DBG_ERROR( "No direct EPS-support for remote appserver!" );
#endif
        }

        if( !bDrawn && pSubst )
        {
            GDIMetaFile* pOldMetaFile = mpMetaFile;

            mpMetaFile = NULL;
            Graphic( *pSubst ).Draw( this, rPoint, rSize );
            mpMetaFile = pOldMetaFile;
        }
    }
}
