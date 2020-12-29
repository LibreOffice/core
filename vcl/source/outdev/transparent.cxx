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

#include <cassert>

#include <sal/types.h>
#include <tools/helpers.hxx>
#include <rtl/math.hxx>

#include <memory>

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>

#include <salgdi.hxx>
#include <bitmap/BitmapWriteAccess.hxx>

namespace
{
    /**
     * Perform a safe approximation of a polygon from double-precision
     * coordinates to integer coordinates, to ensure that it has at least 2
     * pixels in both X and Y directions.
     */
    tools::Polygon toPolygon( const basegfx::B2DPolygon& rPoly )
    {
        basegfx::B2DRange aRange = rPoly.getB2DRange();
        double fW = aRange.getWidth(), fH = aRange.getHeight();
        if (0.0 < fW && 0.0 < fH && (fW <= 1.0 || fH <= 1.0))
        {
            // This polygon not empty but is too small to display.  Approximate it
            // with a rectangle large enough to be displayed.
            double nX = aRange.getMinX(), nY = aRange.getMinY();
            double nW = std::max<double>(1.0, rtl::math::round(fW));
            double nH = std::max<double>(1.0, rtl::math::round(fH));

            tools::Polygon aTarget;
            aTarget.Insert(0, Point(nX, nY));
            aTarget.Insert(1, Point(nX+nW, nY));
            aTarget.Insert(2, Point(nX+nW, nY+nH));
            aTarget.Insert(3, Point(nX, nY+nH));
            aTarget.Insert(4, Point(nX, nY));
            return aTarget;
        }
        return tools::Polygon(rPoly);
    }

    tools::PolyPolygon toPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPoly )
    {
        tools::PolyPolygon aTarget;
        for (auto const& rB2DPolygon : rPolyPoly)
            aTarget.Insert(toPolygon(rB2DPolygon));

        return aTarget;
    }
}

Color OutputDevice::ImplDrawModeToColor( const Color& rColor ) const
{
    Color aColor( rColor );
    DrawModeFlags nDrawMode = GetDrawMode();

    if( nDrawMode & ( DrawModeFlags::BlackLine | DrawModeFlags::WhiteLine |
                      DrawModeFlags::GrayLine |
                      DrawModeFlags::SettingsLine ) )
    {
        if( !aColor.IsTransparent() )
        {
            if( nDrawMode & DrawModeFlags::BlackLine )
            {
                aColor = COL_BLACK;
            }
            else if( nDrawMode & DrawModeFlags::WhiteLine )
            {
                aColor = COL_WHITE;
            }
            else if( nDrawMode & DrawModeFlags::GrayLine )
            {
                const sal_uInt8 cLum = aColor.GetLuminance();
                aColor = Color( cLum, cLum, cLum );
            }
            else if( nDrawMode & DrawModeFlags::SettingsLine )
            {
                aColor = GetSettings().GetStyleSettings().GetFontColor();
            }
        }
    }
    return aColor;
}

void OutputDevice::ImplPrintTransparent( const Bitmap& rBmp, const Bitmap& rMask,
                                         const Point& rDestPt, const Size& rDestSize,
                                         const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
    Point       aDestPt( LogicToPixel( rDestPt ) );
    Size        aDestSz( LogicToPixel( rDestSize ) );
    tools::Rectangle   aSrcRect( rSrcPtPixel, rSrcSizePixel );

    aSrcRect.Justify();

    if( rBmp.IsEmpty() || !aSrcRect.GetWidth() || !aSrcRect.GetHeight() || !aDestSz.Width() || !aDestSz.Height() )
        return;

    Bitmap  aPaint( rBmp ), aMask( rMask );
    BmpMirrorFlags nMirrFlags = BmpMirrorFlags::NONE;

    if( aMask.GetBitCount() > 1 )
        aMask.Convert( BmpConversion::N1BitThreshold );

    // mirrored horizontally
    if( aDestSz.Width() < 0 )
    {
        aDestSz.setWidth( -aDestSz.Width() );
        aDestPt.AdjustX( -( aDestSz.Width() - 1 ) );
        nMirrFlags |= BmpMirrorFlags::Horizontal;
    }

    // mirrored vertically
    if( aDestSz.Height() < 0 )
    {
        aDestSz.setHeight( -aDestSz.Height() );
        aDestPt.AdjustY( -( aDestSz.Height() - 1 ) );
        nMirrFlags |= BmpMirrorFlags::Vertical;
    }

    // source cropped?
    if( aSrcRect != tools::Rectangle( Point(), aPaint.GetSizePixel() ) )
    {
        aPaint.Crop( aSrcRect );
        aMask.Crop( aSrcRect );
    }

    // destination mirrored
    if( nMirrFlags != BmpMirrorFlags::NONE )
    {
        aPaint.Mirror( nMirrFlags );
        aMask.Mirror( nMirrFlags );
    }

    // we always want to have a mask
    if( aMask.IsEmpty() )
    {
        aMask = Bitmap( aSrcRect.GetSize(), 1 );
        aMask.Erase( COL_BLACK );
    }

    // do painting
    const tools::Long nSrcWidth = aSrcRect.GetWidth(), nSrcHeight = aSrcRect.GetHeight();
    tools::Long nX, nY; // , nWorkX, nWorkY, nWorkWidth, nWorkHeight;
    std::unique_ptr<tools::Long[]> pMapX(new tools::Long[ nSrcWidth + 1 ]);
    std::unique_ptr<tools::Long[]> pMapY(new tools::Long[ nSrcHeight + 1 ]);
    const bool bOldMap = mbMap;

    mbMap = false;

    // create forward mapping tables
    for( nX = 0; nX <= nSrcWidth; nX++ )
        pMapX[ nX ] = aDestPt.X() + FRound( static_cast<double>(aDestSz.Width()) * nX / nSrcWidth );

    for( nY = 0; nY <= nSrcHeight; nY++ )
        pMapY[ nY ] = aDestPt.Y() + FRound( static_cast<double>(aDestSz.Height()) * nY / nSrcHeight );

    // walk through all rectangles of mask
    const vcl::Region aWorkRgn(aMask.CreateRegion(COL_BLACK, tools::Rectangle(Point(), aMask.GetSizePixel())));
    RectangleVector aRectangles;
    aWorkRgn.GetRegionRectangles(aRectangles);

    for (auto const& rectangle : aRectangles)
    {
        const Point aMapPt(pMapX[rectangle.Left()], pMapY[rectangle.Top()]);
        const Size aMapSz( pMapX[rectangle.Right() + 1] - aMapPt.X(),      // pMapX[L + W] -> L + ((R - L) + 1) -> R + 1
                           pMapY[rectangle.Bottom() + 1] - aMapPt.Y());    // same for Y
        Bitmap aBandBmp(aPaint);

        aBandBmp.Crop(rectangle);
        DrawBitmap(aMapPt, aMapSz, Point(), aBandBmp.GetSizePixel(), aBandBmp);
    }

    mbMap = bOldMap;

}

// Caution: This method is nearly the same as
// void OutputDevice::DrawPolyPolygon( const basegfx::B2DPolyPolygon& rB2DPolyPoly )
// so when changes are made here do not forget to make changes there, too

void OutputDevice::DrawTransparent(
    const basegfx::B2DHomMatrix& rObjectTransform,
    const basegfx::B2DPolyPolygon& rB2DPolyPoly,
    double fTransparency)
{
    assert(!is_double_buffered_window());

    // AW: Do NOT paint empty PolyPolygons
    if(!rB2DPolyPoly.count())
        return;

    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return;

    if( mbInitClipRegion )
        InitClipRegion();

    if( mbOutputClipped )
        return;

    if( mbInitLineColor )
        InitLineColor();

    if( mbInitFillColor )
        InitFillColor();

    if(mpGraphics->supportsOperation(OutDevSupportType::B2DDraw) &&
       (RasterOp::OverPaint == GetRasterOp()) )
    {
        // b2dpolygon support not implemented yet on non-UNX platforms
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rB2DPolyPoly);

        // ensure it is closed
        if(!aB2DPolyPolygon.isClosed())
        {
            // maybe assert, prevents buffering due to making a copy
            aB2DPolyPolygon.setClosed( true );
        }

        // create ObjectToDevice transformation
        const basegfx::B2DHomMatrix aFullTransform(ImplGetDeviceTransformation() * rObjectTransform);
        // TODO: this must not drop transparency for mpAlphaVDev case, but instead use premultiplied
        // alpha... but that requires using premultiplied alpha also for already drawn data
        const double fAdjustedTransparency = mpAlphaVDev ? 0 : fTransparency;
        bool bDrawnOk(true);

        if( IsFillColor() )
        {
            bDrawnOk = mpGraphics->DrawPolyPolygon(
                aFullTransform,
                aB2DPolyPolygon,
                fAdjustedTransparency,
                *this);
        }

        if( bDrawnOk && IsLineColor() )
        {
            const bool bPixelSnapHairline(mnAntialiasing & AntialiasingFlags::PixelSnapHairline);

            for(auto const& rPolygon : aB2DPolyPolygon)
            {
                mpGraphics->DrawPolyLine(
                    aFullTransform,
                    rPolygon,
                    fAdjustedTransparency,
                    0.0, // tdf#124848 hairline
                    nullptr, // MM01
                    basegfx::B2DLineJoin::NONE,
                    css::drawing::LineCap_BUTT,
                    basegfx::deg2rad(15.0), // not used with B2DLineJoin::NONE, but the correct default
                    bPixelSnapHairline,
                    *this );
            }
        }

        if( bDrawnOk )
        {
            if( mpMetaFile )
            {
                // tdf#119843 need transformed Polygon here
                basegfx::B2DPolyPolygon aB2DPolyPoly(rB2DPolyPoly);
                aB2DPolyPoly.transform(rObjectTransform);
                mpMetaFile->AddAction(
                    new MetaTransparentAction(
                        tools::PolyPolygon(aB2DPolyPoly),
                        static_cast< sal_uInt16 >(fTransparency * 100.0)));
            }

            if (mpAlphaVDev)
                mpAlphaVDev->DrawTransparent(rObjectTransform, rB2DPolyPoly, fTransparency);

            return;
        }
    }

    // fallback to old polygon drawing if needed
    // tdf#119843 need transformed Polygon here
    basegfx::B2DPolyPolygon aB2DPolyPoly(rB2DPolyPoly);
    aB2DPolyPoly.transform(rObjectTransform);
    DrawTransparent(
        toPolyPolygon(aB2DPolyPoly),
        static_cast<sal_uInt16>(fTransparency * 100.0));
}

void OutputDevice::DrawInvisiblePolygon( const tools::PolyPolygon& rPolyPoly )
{
    assert(!is_double_buffered_window());

    // short circuit if the polygon border is invisible too
    if( !mbLineColor )
        return;

    // we assume that the border is NOT to be drawn transparently???
    Push( PushFlags::FILLCOLOR );
    SetFillColor();
    DrawPolyPolygon( rPolyPoly );
    Pop();
}

bool OutputDevice::DrawTransparentNatively ( const tools::PolyPolygon& rPolyPoly,
                                             sal_uInt16 nTransparencePercent )
{
    assert(!is_double_buffered_window());

    bool bDrawn = false;

    if (mpGraphics->supportsOperation(OutDevSupportType::B2DDraw)
#if defined UNX && ! defined MACOSX && ! defined IOS
        && GetBitCount() > 8
#endif
#ifdef _WIN32
        // workaround bad dithering on remote displaying when using GDI+ with toolbar button highlighting
        && !rPolyPoly.IsRect()
#endif
        )
    {
        // prepare the graphics device
        if( mbInitClipRegion )
            InitClipRegion();

        if( mbOutputClipped )
            return false;

        if( mbInitLineColor )
            InitLineColor();

        if( mbInitFillColor )
            InitFillColor();

        // get the polygon in device coordinates
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rPolyPoly.getB2DPolyPolygon());
        const basegfx::B2DHomMatrix aTransform(ImplGetDeviceTransformation());

        const double fTransparency = 0.01 * nTransparencePercent;
        if( mbFillColor )
        {
            // #i121591#
            // CAUTION: Only non printing (pixel-renderer) VCL commands from OutputDevices
            // should be used when printing. Normally this is avoided by the printer being
            // non-AAed and thus e.g. on WIN GdiPlus calls are not used. It may be necessary
            // to figure out a way of moving this code to its own function that is
            // overridden by the Print class, which will mean we deliberately override the
            // functionality and we use the fallback some lines below (which is not very good,
            // though. For now, WinSalGraphics::drawPolyPolygon will detect printer usage and
            // correct the wrong mapping (see there for details)
            bDrawn = mpGraphics->DrawPolyPolygon(
                aTransform,
                aB2DPolyPolygon,
                fTransparency,
                *this);
        }

        if( mbLineColor )
        {
            // disable the fill color for now
            mpGraphics->SetFillColor();

            // draw the border line
            const bool bPixelSnapHairline(mnAntialiasing & AntialiasingFlags::PixelSnapHairline);

            for(auto const& rPolygon : aB2DPolyPolygon)
            {
                mpGraphics->DrawPolyLine(
                    aTransform,
                    rPolygon,
                    fTransparency,
                    0.0, // tdf#124848 hairline
                    nullptr, // MM01
                    basegfx::B2DLineJoin::NONE,
                    css::drawing::LineCap_BUTT,
                    basegfx::deg2rad(15.0), // not used with B2DLineJoin::NONE, but the correct default
                    bPixelSnapHairline,
                    *this);
            }

            bDrawn = true;

            // prepare to restore the fill color
            mbInitFillColor = mbFillColor;
        }
    }

    return bDrawn;
}

void OutputDevice::EmulateDrawTransparent ( const tools::PolyPolygon& rPolyPoly,
                                            sal_uInt16 nTransparencePercent )
{
    // #110958# Disable alpha VDev, we perform the necessary
    VirtualDevice* pOldAlphaVDev = mpAlphaVDev;

    // operation explicitly further below.
    if( mpAlphaVDev )
        mpAlphaVDev = nullptr;

    GDIMetaFile* pOldMetaFile = mpMetaFile;
    mpMetaFile = nullptr;

    tools::PolyPolygon aPolyPoly( LogicToPixel( rPolyPoly ) );
    tools::Rectangle aPolyRect( aPolyPoly.GetBoundRect() );
    tools::Rectangle aDstRect( Point(), GetOutputSizePixel() );

    aDstRect.Intersection( aPolyRect );

    ClipToPaintRegion( aDstRect );

    if( !aDstRect.IsEmpty() )
    {
        bool bDrawn = false;

        // #i66849# Added fast path for exactly rectangular
        // polygons
        // #i83087# Naturally, system alpha blending cannot
        // work with separate alpha VDev
        if( !mpAlphaVDev && aPolyPoly.IsRect() )
        {
            // setup Graphics only here (other cases delegate
            // to basic OutDev methods)
            if ( mbInitClipRegion )
                InitClipRegion();

            if ( mbInitLineColor )
                InitLineColor();

            if ( mbInitFillColor )
                InitFillColor();

            tools::Rectangle aLogicPolyRect( rPolyPoly.GetBoundRect() );
            tools::Rectangle aPixelRect( ImplLogicToDevicePixel( aLogicPolyRect ) );

            if( !mbOutputClipped )
            {
                bDrawn = mpGraphics->DrawAlphaRect( aPixelRect.Left(), aPixelRect.Top(),
                    // #i98405# use methods with small g, else one pixel too much will be painted.
                    // This is because the source is a polygon which when painted would not paint
                    // the rightmost and lowest pixel line(s), so use one pixel less for the
                    // rectangle, too.
                                                    aPixelRect.getWidth(), aPixelRect.getHeight(),
                                                    sal::static_int_cast<sal_uInt8>(nTransparencePercent),
                                                    *this );
            }
            else
            {
                bDrawn = true;
            }
        }

        if( !bDrawn )
        {
            ScopedVclPtrInstance< VirtualDevice > aVDev(*this, DeviceFormat::BITMASK);
            const Size aDstSz( aDstRect.GetSize() );
            const sal_uInt8 cTrans = static_cast<sal_uInt8>(MinMax( FRound( nTransparencePercent * 2.55 ), 0, 255 ));

            if( aDstRect.Left() || aDstRect.Top() )
                aPolyPoly.Move( -aDstRect.Left(), -aDstRect.Top() );

            if( aVDev->SetOutputSizePixel( aDstSz ) )
            {
                const bool bOldMap = mbMap;

                EnableMapMode( false );

                aVDev->SetLineColor( COL_BLACK );
                aVDev->SetFillColor( COL_BLACK );
                aVDev->DrawPolyPolygon( aPolyPoly );

                Bitmap aPaint( GetBitmap( aDstRect.TopLeft(), aDstSz ) );
                Bitmap aPolyMask( aVDev->GetBitmap( Point(), aDstSz ) );

                // #107766# check for non-empty bitmaps before accessing them
                if( !!aPaint && !!aPolyMask )
                {
                    BitmapScopedWriteAccess pW(aPaint);
                    Bitmap::ScopedReadAccess pR(aPolyMask);

                    if( pW && pR )
                    {
                        BitmapColor aPixCol;
                        const BitmapColor aFillCol( GetFillColor() );
                        const BitmapColor aBlack( pR->GetBestMatchingColor( COL_BLACK ) );
                        const tools::Long nWidth = pW->Width();
                        const tools::Long nHeight = pW->Height();
                        const tools::Long nR = aFillCol.GetRed();
                        const tools::Long nG = aFillCol.GetGreen();
                        const tools::Long nB = aFillCol.GetBlue();
                        tools::Long nX, nY;

                        if( aPaint.GetBitCount() <= 8 )
                        {
                            const BitmapPalette& rPal = pW->GetPalette();
                            const sal_uInt16 nCount = rPal.GetEntryCount();
                            BitmapColor* pMap = reinterpret_cast<BitmapColor*>(new sal_uInt8[ nCount * sizeof( BitmapColor ) ]);

                            for( sal_uInt16 i = 0; i < nCount; i++ )
                            {
                                BitmapColor aCol( rPal[ i ] );
                                aCol.Merge( aFillCol, cTrans );
                                pMap[ i ] = BitmapColor( static_cast<sal_uInt8>(rPal.GetBestIndex( aCol )) );
                            }

                            if( pR->GetScanlineFormat() == ScanlineFormat::N1BitMsbPal &&
                                pW->GetScanlineFormat() == ScanlineFormat::N8BitPal )
                            {
                                const sal_uInt8 cBlack = aBlack.GetIndex();

                                for( nY = 0; nY < nHeight; nY++ )
                                {
                                    Scanline pWScan = pW->GetScanline( nY );
                                    Scanline pRScan = pR->GetScanline( nY );
                                    sal_uInt8 cBit = 128;

                                    for( nX = 0; nX < nWidth; nX++, cBit >>= 1, pWScan++ )
                                    {
                                        if( !cBit )
                                        {
                                            cBit = 128;
                                            pRScan += 1;
                                        }
                                        if( ( *pRScan & cBit ) == cBlack )
                                        {
                                            *pWScan = pMap[ *pWScan ].GetIndex();
                                        }
                                    }
                                }
                            }
                            else
                            {
                                for( nY = 0; nY < nHeight; nY++ )
                                {
                                    Scanline pScanline = pW->GetScanline(nY);
                                    Scanline pScanlineRead = pR->GetScanline(nY);
                                    for( nX = 0; nX < nWidth; nX++ )
                                    {
                                        if( pR->GetPixelFromData( pScanlineRead, nX ) == aBlack )
                                        {
                                            pW->SetPixelOnData( pScanline, nX, pMap[ pW->GetIndexFromData( pScanline, nX ) ] );
                                        }
                                    }
                                }
                            }
                            delete[] reinterpret_cast<sal_uInt8*>(pMap);
                        }
                        else
                        {
                            if( pR->GetScanlineFormat() == ScanlineFormat::N1BitMsbPal &&
                                pW->GetScanlineFormat() == ScanlineFormat::N24BitTcBgr )
                            {
                                const sal_uInt8 cBlack = aBlack.GetIndex();

                                for( nY = 0; nY < nHeight; nY++ )
                                {
                                    Scanline pWScan = pW->GetScanline( nY );
                                    Scanline pRScan = pR->GetScanline( nY );
                                    sal_uInt8 cBit = 128;

                                    for( nX = 0; nX < nWidth; nX++, cBit >>= 1, pWScan += 3 )
                                    {
                                        if( !cBit )
                                        {
                                            cBit = 128;
                                            pRScan += 1;
                                        }
                                        if( ( *pRScan & cBit ) == cBlack )
                                        {
                                            pWScan[ 0 ] = color::ColorChannelMerge( pWScan[ 0 ], nB, cTrans );
                                            pWScan[ 1 ] = color::ColorChannelMerge( pWScan[ 1 ], nG, cTrans );
                                            pWScan[ 2 ] = color::ColorChannelMerge( pWScan[ 2 ], nR, cTrans );
                                        }
                                    }
                                }
                            }
                            else
                            {
                                for( nY = 0; nY < nHeight; nY++ )
                                {
                                    Scanline pScanline = pW->GetScanline(nY);
                                    Scanline pScanlineRead = pR->GetScanline(nY);
                                    for( nX = 0; nX < nWidth; nX++ )
                                    {
                                        if( pR->GetPixelFromData( pScanlineRead, nX ) == aBlack )
                                        {
                                            aPixCol = pW->GetColor( nY, nX );
                                            aPixCol.Merge(aFillCol, cTrans);
                                            pW->SetPixelOnData(pScanline, nX, aPixCol);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    pR.reset();
                    pW.reset();

                    DrawBitmap( aDstRect.TopLeft(), aPaint );

                    EnableMapMode( bOldMap );

                    if( mbLineColor )
                    {
                        Push( PushFlags::FILLCOLOR );
                        SetFillColor();
                        DrawPolyPolygon( rPolyPoly );
                        Pop();
                    }
                }
            }
            else
            {
                DrawPolyPolygon( rPolyPoly );
            }
        }
    }

    mpMetaFile = pOldMetaFile;

    // #110958# Restore disabled alpha VDev
    mpAlphaVDev = pOldAlphaVDev;
}

void OutputDevice::DrawTransparent( const tools::PolyPolygon& rPolyPoly,
                                    sal_uInt16 nTransparencePercent )
{
    assert(!is_double_buffered_window());

    // short circuit for drawing an opaque polygon
    if( (nTransparencePercent < 1) || (mnDrawMode & DrawModeFlags::NoTransparency) )
    {
        DrawPolyPolygon( rPolyPoly );
        return;
    }

    // short circuit for drawing an invisible polygon
    if( !mbFillColor || (nTransparencePercent >= 100) )
    {
        DrawInvisiblePolygon( rPolyPoly );
        return; // tdf#84294: do not record it in metafile
    }

    // handle metafile recording
    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaTransparentAction( rPolyPoly, nTransparencePercent ) );

    bool bDrawn = !IsDeviceOutputNecessary() || ImplIsRecordLayout();
    if( bDrawn )
        return;

    // get the device graphics as drawing target
    if( !mpGraphics && !AcquireGraphics() )
        return;

    // try hard to draw it directly, because the emulation layers are slower
    bDrawn = DrawTransparentNatively( rPolyPoly, nTransparencePercent );

    if (!bDrawn)
        EmulateDrawTransparent( rPolyPoly, nTransparencePercent );

    // #110958# Apply alpha value also to VDev alpha channel
    if( mpAlphaVDev )
    {
        const Color aFillCol( mpAlphaVDev->GetFillColor() );
        mpAlphaVDev->SetFillColor( Color(sal::static_int_cast<sal_uInt8>(255*nTransparencePercent/100),
                                         sal::static_int_cast<sal_uInt8>(255*nTransparencePercent/100),
                                         sal::static_int_cast<sal_uInt8>(255*nTransparencePercent/100)) );

        mpAlphaVDev->DrawTransparent( rPolyPoly, nTransparencePercent );

        mpAlphaVDev->SetFillColor( aFillCol );
    }
}

void OutputDevice::DrawTransparent( const GDIMetaFile& rMtf, const Point& rPos,
                                    const Size& rSize, const Gradient& rTransparenceGradient )
{
    assert(!is_double_buffered_window());

    const Color aBlack( COL_BLACK );

    if( mpMetaFile )
    {
         // missing here is to map the data using the DeviceTransformation
        mpMetaFile->AddAction( new MetaFloatTransparentAction( rMtf, rPos, rSize, rTransparenceGradient ) );
    }

    if ( !IsDeviceOutputNecessary() )
        return;

    if( ( rTransparenceGradient.GetStartColor() == aBlack && rTransparenceGradient.GetEndColor() == aBlack ) ||
        ( mnDrawMode & DrawModeFlags::NoTransparency ) )
    {
        const_cast<GDIMetaFile&>(rMtf).WindStart();
        const_cast<GDIMetaFile&>(rMtf).Play( this, rPos, rSize );
        const_cast<GDIMetaFile&>(rMtf).WindStart();
    }
    else
    {
        GDIMetaFile* pOldMetaFile = mpMetaFile;
        tools::Rectangle aOutRect( LogicToPixel( rPos ), LogicToPixel( rSize ) );
        Point aPoint;
        tools::Rectangle aDstRect( aPoint, GetOutputSizePixel() );

        mpMetaFile = nullptr;
        aDstRect.Intersection( aOutRect );

        ClipToPaintRegion( aDstRect );

        if( !aDstRect.IsEmpty() )
        {
            // Create transparent buffer
            ScopedVclPtrInstance<VirtualDevice> xVDev(DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);

            xVDev->mnDPIX = mnDPIX;
            xVDev->mnDPIY = mnDPIY;

            if( xVDev->SetOutputSizePixel( aDstRect.GetSize() ) )
            {
                if(GetAntialiasing() != AntialiasingFlags::NONE)
                {
                    // #i102109#
                    // For MetaFile replay (see task) it may now be necessary to take
                    // into account that the content is AntiAlialiased and needs to be masked
                    // like that. Instead of masking, i will use a copy-modify-paste cycle
                    // here (as i already use in the VclPrimiziveRenderer with success)
                    xVDev->SetAntialiasing(GetAntialiasing());

                    // create MapMode for buffer (offset needed) and set
                    MapMode aMap(GetMapMode());
                    const Point aOutPos(PixelToLogic(aDstRect.TopLeft()));
                    aMap.SetOrigin(Point(-aOutPos.X(), -aOutPos.Y()));
                    xVDev->SetMapMode(aMap);

                    // copy MapMode state and disable for target
                    const bool bOrigMapModeEnabled(IsMapModeEnabled());
                    EnableMapMode(false);

                    // copy MapMode state and disable for buffer
                    const bool bBufferMapModeEnabled(xVDev->IsMapModeEnabled());
                    xVDev->EnableMapMode(false);

                    // copy content from original to buffer
                    xVDev->DrawOutDev( aPoint, xVDev->GetOutputSizePixel(), // dest
                                       aDstRect.TopLeft(), xVDev->GetOutputSizePixel(), // source
                                       *this);

                    // draw MetaFile to buffer
                    xVDev->EnableMapMode(bBufferMapModeEnabled);
                    const_cast<GDIMetaFile&>(rMtf).WindStart();
                    const_cast<GDIMetaFile&>(rMtf).Play(xVDev.get(), rPos, rSize);
                    const_cast<GDIMetaFile&>(rMtf).WindStart();

                    // get content bitmap from buffer
                    xVDev->EnableMapMode(false);

                    const Bitmap aPaint(xVDev->GetBitmap(aPoint, xVDev->GetOutputSizePixel()));

                    // create alpha mask from gradient and get as Bitmap
                    xVDev->EnableMapMode(bBufferMapModeEnabled);
                    xVDev->SetDrawMode(DrawModeFlags::GrayGradient);
                    xVDev->DrawGradient(tools::Rectangle(rPos, rSize), rTransparenceGradient);
                    xVDev->SetDrawMode(DrawModeFlags::Default);
                    xVDev->EnableMapMode(false);

                    const AlphaMask aAlpha(xVDev->GetBitmap(aPoint, xVDev->GetOutputSizePixel()));

                    xVDev.disposeAndClear();

                    // draw masked content to target and restore MapMode
                    DrawBitmapEx(aDstRect.TopLeft(), BitmapEx(aPaint, aAlpha));
                    EnableMapMode(bOrigMapModeEnabled);
                }
                else
                {
                    MapMode aMap( GetMapMode() );
                    Point aOutPos( PixelToLogic( aDstRect.TopLeft() ) );
                    const bool bOldMap = mbMap;

                    aMap.SetOrigin( Point( -aOutPos.X(), -aOutPos.Y() ) );
                    xVDev->SetMapMode( aMap );
                    const bool bVDevOldMap = xVDev->IsMapModeEnabled();

                    // create paint bitmap
                    const_cast<GDIMetaFile&>(rMtf).WindStart();
                    const_cast<GDIMetaFile&>(rMtf).Play( xVDev.get(), rPos, rSize );
                    const_cast<GDIMetaFile&>(rMtf).WindStart();
                    xVDev->EnableMapMode( false );
                    BitmapEx aPaint = xVDev->GetBitmapEx(Point(), xVDev->GetOutputSizePixel());
                    xVDev->EnableMapMode( bVDevOldMap ); // #i35331#: MUST NOT use EnableMapMode( sal_True ) here!

                    // create alpha mask from gradient
                    xVDev->SetDrawMode( DrawModeFlags::GrayGradient );
                    xVDev->DrawGradient( tools::Rectangle( rPos, rSize ), rTransparenceGradient );
                    xVDev->SetDrawMode( DrawModeFlags::Default );
                    xVDev->EnableMapMode( false );

                    AlphaMask aAlpha(xVDev->GetBitmap(Point(), xVDev->GetOutputSizePixel()));
                    aAlpha.BlendWith(aPaint.GetAlpha());

                    xVDev.disposeAndClear();

                    EnableMapMode( false );
                    DrawBitmapEx(aDstRect.TopLeft(), BitmapEx(aPaint.GetBitmap(), aAlpha));
                    EnableMapMode( bOldMap );
                }
            }
        }

        mpMetaFile = pOldMetaFile;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
