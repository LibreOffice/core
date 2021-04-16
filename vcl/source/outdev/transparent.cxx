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

#include <sal/types.h>
#include <osl/diagnose.h>
#include <rtl/math.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <tools/helpers.hxx>
#include <officecfg/Office/Common.hxx>

#include <vcl/BitmapTools.hxx>
#include <vcl/metaact.hxx>
#include <vcl/print.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <pdf/pdfwriter_impl.hxx>
#include <salgdi.hxx>

#include <list>
#include <memory>

#define MAX_TILE_WIDTH  1024
#define MAX_TILE_HEIGHT 1024

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
    assert(mpGraphics);

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

            for(auto const& rPolygon : std::as_const(aB2DPolyPolygon))
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

            for(auto const& rPolygon : std::as_const(aB2DPolyPolygon))
            {
                bDrawn = mpGraphics->DrawPolyLine(
                    aTransform,
                    rPolygon,
                    fTransparency,
                    0.0, // tdf#124848 hairline
                    nullptr, // MM01
                    basegfx::B2DLineJoin::NONE,
                    css::drawing::LineCap_BUTT,
                    basegfx::deg2rad(15.0), // not used with B2DLineJoin::NONE, but the correct default
                    bPixelSnapHairline,
                    *this );
            }

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
            ScopedVclPtrInstance< VirtualDevice > aVDev(*this);
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
                if( !aPaint.IsEmpty() && !aPolyMask.IsEmpty() )
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

                        if (vcl::isPalettePixelFormat(aPaint.getPixelFormat()))
                        {
                            const BitmapPalette& rPal = pW->GetPalette();
                            const sal_uInt16 nCount = rPal.GetEntryCount();
                            std::unique_ptr<sal_uInt8[]> xMap(new sal_uInt8[ nCount * sizeof( BitmapColor )]);
                            BitmapColor* pMap = reinterpret_cast<BitmapColor*>(xMap.get());

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
                        Push( vcl::PushFlags::FILLCOLOR );
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
    if( (!mbFillColor && !mbLineColor) || (nTransparencePercent >= 100) )
        return; // tdf#84294: do not record it in metafile

    // handle metafile recording
    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaTransparentAction( rPolyPoly, nTransparencePercent ) );

    bool bDrawn = !IsDeviceOutputNecessary() || ImplIsRecordLayout();
    if( bDrawn )
        return;

    // get the device graphics as drawing target
    if( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    // try hard to draw it directly, because the emulation layers are slower
    bDrawn = DrawTransparentNatively( rPolyPoly, nTransparencePercent );

    if (!bDrawn)
        EmulateDrawTransparent( rPolyPoly, nTransparencePercent );

    // #110958# Apply alpha value also to VDev alpha channel
    if( mpAlphaVDev )
    {
        const Color aFillCol( mpAlphaVDev->GetFillColor() );
        sal_uInt8 nAlpha = 255 - sal::static_int_cast<sal_uInt8>(255*nTransparencePercent/100);
        mpAlphaVDev->SetFillColor( Color(nAlpha, nAlpha, nAlpha) );

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
        const_cast<GDIMetaFile&>(rMtf).Play(*this, rPos, rSize);
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
                    const_cast<GDIMetaFile&>(rMtf).Play(*xVDev, rPos, rSize);
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
                    const_cast<GDIMetaFile&>(rMtf).Play(*xVDev, rPos, rSize);
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

typedef ::std::pair< MetaAction*, int > Component; // MetaAction plus index in metafile

namespace {

// List of (intersecting) actions, plus overall bounds
struct ConnectedComponents
{
    ConnectedComponents() :
        aComponentList(),
        aBounds(),
        aBgColor(COL_WHITE),
        bIsSpecial(false),
        bIsFullyTransparent(false)
    {}

    ::std::list< Component > aComponentList;
    tools::Rectangle       aBounds;
    Color           aBgColor;
    bool            bIsSpecial;
    bool            bIsFullyTransparent;
};

}

namespace {

/** Determines whether the action can handle transparency correctly
  (i.e. when painted on white background, does the action still look
  correct)?
 */
bool DoesActionHandleTransparency( const MetaAction& rAct )
{
    // MetaActionType::FLOATTRANSPARENT can contain a whole metafile,
    // which is to be rendered with the given transparent gradient. We
    // currently cannot emulate transparent painting on a white
    // background reliably.

    // the remainder can handle printing itself correctly on a uniform
    // white background.
    switch( rAct.GetType() )
    {
        case MetaActionType::Transparent:
        case MetaActionType::BMPEX:
        case MetaActionType::BMPEXSCALE:
        case MetaActionType::BMPEXSCALEPART:
            return true;

        default:
            return false;
    }
}

bool doesRectCoverWithUniformColor(
        tools::Rectangle const & rPrevRect,
        tools::Rectangle const & rCurrRect,
        OutputDevice const & rMapModeVDev)
{
    // shape needs to fully cover previous content, and have uniform
    // color
    return (rMapModeVDev.LogicToPixel(rCurrRect).Contains(rPrevRect) &&
        rMapModeVDev.IsFillColor());
}

/** Check whether rCurrRect rectangle fully covers io_rPrevRect - if
    yes, return true and update o_rBgColor
 */
bool checkRect( tools::Rectangle&       io_rPrevRect,
                       Color&           o_rBgColor,
                       const tools::Rectangle& rCurrRect,
                       OutputDevice const &    rMapModeVDev )
{
    bool bRet = doesRectCoverWithUniformColor(io_rPrevRect, rCurrRect, rMapModeVDev);

    if( bRet )
    {
        io_rPrevRect = rCurrRect;
        o_rBgColor = rMapModeVDev.GetFillColor();
    }

    return bRet;
}

/** #107169# Convert BitmapEx to Bitmap with appropriately blended
    color. Convert MetaTransparentAction to plain polygon,
    appropriately colored

    @param o_rMtf
    Add converted actions to this metafile
*/
void ImplConvertTransparentAction( GDIMetaFile&        o_rMtf,
                                   const MetaAction&   rAct,
                                   const OutputDevice& rStateOutDev,
                                   Color               aBgColor )
{
    if (rAct.GetType() == MetaActionType::Transparent)
    {
        const MetaTransparentAction* pTransAct = static_cast<const MetaTransparentAction*>(&rAct);
        sal_uInt16 nTransparency( pTransAct->GetTransparence() );

        // #i10613# Respect transparency for draw color
        if (nTransparency)
        {
            o_rMtf.AddAction(new MetaPushAction(vcl::PushFlags::LINECOLOR|vcl::PushFlags::FILLCOLOR));

            // assume white background for alpha blending
            Color aLineColor(rStateOutDev.GetLineColor());
            aLineColor.SetRed(static_cast<sal_uInt8>((255*nTransparency + (100 - nTransparency) * aLineColor.GetRed()) / 100));
            aLineColor.SetGreen(static_cast<sal_uInt8>((255*nTransparency + (100 - nTransparency) * aLineColor.GetGreen()) / 100));
            aLineColor.SetBlue(static_cast<sal_uInt8>((255*nTransparency + (100 - nTransparency) * aLineColor.GetBlue()) / 100));
            o_rMtf.AddAction(new MetaLineColorAction(aLineColor, true));

            Color aFillColor(rStateOutDev.GetFillColor());
            aFillColor.SetRed(static_cast<sal_uInt8>((255*nTransparency + (100 - nTransparency)*aFillColor.GetRed()) / 100));
            aFillColor.SetGreen(static_cast<sal_uInt8>((255*nTransparency + (100 - nTransparency)*aFillColor.GetGreen()) / 100));
            aFillColor.SetBlue(static_cast<sal_uInt8>((255*nTransparency + (100 - nTransparency)*aFillColor.GetBlue()) / 100));
            o_rMtf.AddAction(new MetaFillColorAction(aFillColor, true));
        }

        o_rMtf.AddAction(new MetaPolyPolygonAction(pTransAct->GetPolyPolygon()));

        if(nTransparency)
            o_rMtf.AddAction(new MetaPopAction());
    }
    else
    {
        BitmapEx aBmpEx;

        switch (rAct.GetType())
        {
            case MetaActionType::BMPEX:
                aBmpEx = static_cast<const MetaBmpExAction&>(rAct).GetBitmapEx();
                break;

            case MetaActionType::BMPEXSCALE:
                aBmpEx = static_cast<const MetaBmpExScaleAction&>(rAct).GetBitmapEx();
                break;

            case MetaActionType::BMPEXSCALEPART:
                aBmpEx = static_cast<const MetaBmpExScaleAction&>(rAct).GetBitmapEx();
                break;

            case MetaActionType::Transparent:

            default:
                OSL_FAIL("Printer::GetPreparedMetafile impossible state reached");
                break;
        }

        Bitmap aBmp(aBmpEx.GetBitmap());
        if (aBmpEx.IsAlpha())
        {
            // blend with alpha channel
            aBmp.Convert(BmpConversion::N24Bit);
            aBmp.Blend(aBmpEx.GetAlpha(), aBgColor);
        }

        // add corresponding action
        switch (rAct.GetType())
        {
            case MetaActionType::BMPEX:
                o_rMtf.AddAction(new MetaBmpAction(
                                       static_cast<const MetaBmpExAction&>(rAct).GetPoint(),
                                       aBmp));
                break;
            case MetaActionType::BMPEXSCALE:
                o_rMtf.AddAction(new MetaBmpScaleAction(
                                       static_cast<const MetaBmpExScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaBmpExScaleAction&>(rAct).GetSize(),
                                       aBmp));
                break;
            case MetaActionType::BMPEXSCALEPART:
                o_rMtf.AddAction(new MetaBmpScalePartAction(
                                       static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestSize(),
                                       static_cast<const MetaBmpExScalePartAction&>(rAct).GetSrcPoint(),
                                       static_cast<const MetaBmpExScalePartAction&>(rAct).GetSrcSize(),
                                       aBmp));
                break;
            default:
                OSL_FAIL("Unexpected case");
                break;
        }
    }
}

// #i10613# Extracted from ImplCheckRect::ImplCreate
// Returns true, if given action creates visible (i.e. non-transparent) output
bool ImplIsNotTransparent( const MetaAction& rAct, const OutputDevice& rOut )
{
    const bool  bLineTransparency( !rOut.IsLineColor() || rOut.GetLineColor().IsFullyTransparent() );
    const bool  bFillTransparency( !rOut.IsFillColor() || rOut.GetFillColor().IsFullyTransparent() );
    bool        bRet( false );

    switch( rAct.GetType() )
    {
        case MetaActionType::POINT:
            if( !bLineTransparency )
                bRet = true;
            break;

        case MetaActionType::LINE:
            if( !bLineTransparency )
                bRet = true;
            break;

        case MetaActionType::RECT:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::ROUNDRECT:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::ELLIPSE:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::ARC:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::PIE:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::CHORD:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::POLYLINE:
            if( !bLineTransparency )
                bRet = true;
            break;

        case MetaActionType::POLYGON:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::POLYPOLYGON:
            if( !bLineTransparency || !bFillTransparency )
                bRet = true;
            break;

        case MetaActionType::TEXT:
        {
            const MetaTextAction& rTextAct = static_cast<const MetaTextAction&>(rAct);
            const OUString aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );
            if (!aString.isEmpty())
                bRet = true;
        }
        break;

        case MetaActionType::TEXTARRAY:
        {
            const MetaTextArrayAction& rTextAct = static_cast<const MetaTextArrayAction&>(rAct);
            const OUString aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );
            if (!aString.isEmpty())
                bRet = true;
        }
        break;

        case MetaActionType::PIXEL:
        case MetaActionType::BMP:
        case MetaActionType::BMPSCALE:
        case MetaActionType::BMPSCALEPART:
        case MetaActionType::BMPEX:
        case MetaActionType::BMPEXSCALE:
        case MetaActionType::BMPEXSCALEPART:
        case MetaActionType::MASK:
        case MetaActionType::MASKSCALE:
        case MetaActionType::MASKSCALEPART:
        case MetaActionType::GRADIENT:
        case MetaActionType::GRADIENTEX:
        case MetaActionType::HATCH:
        case MetaActionType::WALLPAPER:
        case MetaActionType::Transparent:
        case MetaActionType::FLOATTRANSPARENT:
        case MetaActionType::EPS:
        case MetaActionType::TEXTRECT:
        case MetaActionType::STRETCHTEXT:
        case MetaActionType::TEXTLINE:
            // all other actions: generate non-transparent output
            bRet = true;
            break;

        default:
            break;
    }

    return bRet;
}

// #i10613# Extracted from ImplCheckRect::ImplCreate
tools::Rectangle ImplCalcActionBounds( const MetaAction& rAct, const OutputDevice& rOut )
{
    tools::Rectangle aActionBounds;

    switch( rAct.GetType() )
    {
        case MetaActionType::PIXEL:
            aActionBounds = tools::Rectangle( static_cast<const MetaPixelAction&>(rAct).GetPoint(), Size( 1, 1 ) );
            break;

        case MetaActionType::POINT:
            aActionBounds = tools::Rectangle( static_cast<const MetaPointAction&>(rAct).GetPoint(), Size( 1, 1 ) );
            break;

        case MetaActionType::LINE:
        {
            const MetaLineAction& rMetaLineAction = static_cast<const MetaLineAction&>(rAct);
            aActionBounds = tools::Rectangle( rMetaLineAction.GetStartPoint(),  rMetaLineAction.GetEndPoint() );
            aActionBounds.Justify();
            const tools::Long nLineWidth(rMetaLineAction.GetLineInfo().GetWidth());
            if(nLineWidth)
            {
                const tools::Long nHalfLineWidth((nLineWidth + 1) / 2);
                aActionBounds.AdjustLeft( -nHalfLineWidth );
                aActionBounds.AdjustTop( -nHalfLineWidth );
                aActionBounds.AdjustRight(nHalfLineWidth );
                aActionBounds.AdjustBottom(nHalfLineWidth );
            }
            break;
        }

        case MetaActionType::RECT:
            aActionBounds = static_cast<const MetaRectAction&>(rAct).GetRect();
            break;

        case MetaActionType::ROUNDRECT:
            aActionBounds = tools::Polygon( static_cast<const MetaRoundRectAction&>(rAct).GetRect(),
                                            static_cast<const MetaRoundRectAction&>(rAct).GetHorzRound(),
                                            static_cast<const MetaRoundRectAction&>(rAct).GetVertRound() ).GetBoundRect();
            break;

        case MetaActionType::ELLIPSE:
        {
            const tools::Rectangle& rRect = static_cast<const MetaEllipseAction&>(rAct).GetRect();
            aActionBounds = tools::Polygon( rRect.Center(),
                                            rRect.GetWidth() >> 1,
                                            rRect.GetHeight() >> 1 ).GetBoundRect();
            break;
        }

        case MetaActionType::ARC:
            aActionBounds = tools::Polygon( static_cast<const MetaArcAction&>(rAct).GetRect(),
                                            static_cast<const MetaArcAction&>(rAct).GetStartPoint(),
                                            static_cast<const MetaArcAction&>(rAct).GetEndPoint(), PolyStyle::Arc ).GetBoundRect();
            break;

        case MetaActionType::PIE:
            aActionBounds = tools::Polygon( static_cast<const MetaPieAction&>(rAct).GetRect(),
                                            static_cast<const MetaPieAction&>(rAct).GetStartPoint(),
                                            static_cast<const MetaPieAction&>(rAct).GetEndPoint(), PolyStyle::Pie ).GetBoundRect();
            break;

        case MetaActionType::CHORD:
            aActionBounds = tools::Polygon( static_cast<const MetaChordAction&>(rAct).GetRect(),
                                            static_cast<const MetaChordAction&>(rAct).GetStartPoint(),
                                            static_cast<const MetaChordAction&>(rAct).GetEndPoint(), PolyStyle::Chord ).GetBoundRect();
            break;

        case MetaActionType::POLYLINE:
        {
            const MetaPolyLineAction& rMetaPolyLineAction = static_cast<const MetaPolyLineAction&>(rAct);
            aActionBounds = rMetaPolyLineAction.GetPolygon().GetBoundRect();
            const tools::Long nLineWidth(rMetaPolyLineAction.GetLineInfo().GetWidth());
            if(nLineWidth)
            {
                const tools::Long nHalfLineWidth((nLineWidth + 1) / 2);
                aActionBounds.AdjustLeft( -nHalfLineWidth );
                aActionBounds.AdjustTop( -nHalfLineWidth );
                aActionBounds.AdjustRight(nHalfLineWidth );
                aActionBounds.AdjustBottom(nHalfLineWidth );
            }
            break;
        }

        case MetaActionType::POLYGON:
            aActionBounds = static_cast<const MetaPolygonAction&>(rAct).GetPolygon().GetBoundRect();
            break;

        case MetaActionType::POLYPOLYGON:
            aActionBounds = static_cast<const MetaPolyPolygonAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case MetaActionType::BMP:
            aActionBounds = tools::Rectangle( static_cast<const MetaBmpAction&>(rAct).GetPoint(),
                                       rOut.PixelToLogic( static_cast<const MetaBmpAction&>(rAct).GetBitmap().GetSizePixel() ) );
            break;

        case MetaActionType::BMPSCALE:
            aActionBounds = tools::Rectangle( static_cast<const MetaBmpScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaBmpScaleAction&>(rAct).GetSize() );
            break;

        case MetaActionType::BMPSCALEPART:
            aActionBounds = tools::Rectangle( static_cast<const MetaBmpScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaBmpScalePartAction&>(rAct).GetDestSize() );
            break;

        case MetaActionType::BMPEX:
            aActionBounds = tools::Rectangle( static_cast<const MetaBmpExAction&>(rAct).GetPoint(),
                                       rOut.PixelToLogic( static_cast<const MetaBmpExAction&>(rAct).GetBitmapEx().GetSizePixel() ) );
            break;

        case MetaActionType::BMPEXSCALE:
            aActionBounds = tools::Rectangle( static_cast<const MetaBmpExScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaBmpExScaleAction&>(rAct).GetSize() );
            break;

        case MetaActionType::BMPEXSCALEPART:
            aActionBounds = tools::Rectangle( static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestSize() );
            break;

        case MetaActionType::MASK:
            aActionBounds = tools::Rectangle( static_cast<const MetaMaskAction&>(rAct).GetPoint(),
                                       rOut.PixelToLogic( static_cast<const MetaMaskAction&>(rAct).GetBitmap().GetSizePixel() ) );
            break;

        case MetaActionType::MASKSCALE:
            aActionBounds = tools::Rectangle( static_cast<const MetaMaskScaleAction&>(rAct).GetPoint(),
                                       static_cast<const MetaMaskScaleAction&>(rAct).GetSize() );
            break;

        case MetaActionType::MASKSCALEPART:
            aActionBounds = tools::Rectangle( static_cast<const MetaMaskScalePartAction&>(rAct).GetDestPoint(),
                                       static_cast<const MetaMaskScalePartAction&>(rAct).GetDestSize() );
            break;

        case MetaActionType::GRADIENT:
            aActionBounds = static_cast<const MetaGradientAction&>(rAct).GetRect();
            break;

        case MetaActionType::GRADIENTEX:
            aActionBounds = static_cast<const MetaGradientExAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case MetaActionType::HATCH:
            aActionBounds = static_cast<const MetaHatchAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case MetaActionType::WALLPAPER:
            aActionBounds = static_cast<const MetaWallpaperAction&>(rAct).GetRect();
            break;

        case MetaActionType::Transparent:
            aActionBounds = static_cast<const MetaTransparentAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case MetaActionType::FLOATTRANSPARENT:
            aActionBounds = tools::Rectangle( static_cast<const MetaFloatTransparentAction&>(rAct).GetPoint(),
                                       static_cast<const MetaFloatTransparentAction&>(rAct).GetSize() );
            break;

        case MetaActionType::EPS:
            aActionBounds = tools::Rectangle( static_cast<const MetaEPSAction&>(rAct).GetPoint(),
                                       static_cast<const MetaEPSAction&>(rAct).GetSize() );
            break;

        case MetaActionType::TEXT:
        {
            const MetaTextAction& rTextAct = static_cast<const MetaTextAction&>(rAct);
            const OUString aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );

            if (!aString.isEmpty())
            {
                const Point aPtLog( rTextAct.GetPoint() );

                // #105987# Use API method instead of Impl* methods
                // #107490# Set base parameter equal to index parameter
                rOut.GetTextBoundRect( aActionBounds, rTextAct.GetText(), rTextAct.GetIndex(),
                                       rTextAct.GetIndex(), rTextAct.GetLen() );
                aActionBounds.Move( aPtLog.X(), aPtLog.Y() );
            }
        }
        break;

        case MetaActionType::TEXTARRAY:
        {
            const MetaTextArrayAction&  rTextAct = static_cast<const MetaTextArrayAction&>(rAct);
            const OUString              aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );

            if( !aString.isEmpty() )
            {
                // #105987# ImplLayout takes everything in logical coordinates
                std::unique_ptr<SalLayout> pSalLayout = rOut.ImplLayout( rTextAct.GetText(), rTextAct.GetIndex(),
                                                         rTextAct.GetLen(), rTextAct.GetPoint(),
                                                         0, rTextAct.GetDXArray());
                if( pSalLayout )
                {
                    tools::Rectangle aBoundRect( rOut.ImplGetTextBoundRect( *pSalLayout ) );
                    aActionBounds = rOut.PixelToLogic( aBoundRect );
                }
            }
        }
        break;

        case MetaActionType::TEXTRECT:
            aActionBounds = static_cast<const MetaTextRectAction&>(rAct).GetRect();
            break;

        case MetaActionType::STRETCHTEXT:
        {
            const MetaStretchTextAction& rTextAct = static_cast<const MetaStretchTextAction&>(rAct);
            const OUString               aString( rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()) );

            // #i16195# Literate copy from TextArray action, the
            // semantics for the ImplLayout call are copied from the
            // OutDev::DrawStretchText() code. Unfortunately, also in
            // this case, public outdev methods such as GetTextWidth()
            // don't provide enough info.
            if( !aString.isEmpty() )
            {
                // #105987# ImplLayout takes everything in logical coordinates
                std::unique_ptr<SalLayout> pSalLayout = rOut.ImplLayout( rTextAct.GetText(), rTextAct.GetIndex(),
                                                         rTextAct.GetLen(), rTextAct.GetPoint(),
                                                         rTextAct.GetWidth() );
                if( pSalLayout )
                {
                    tools::Rectangle aBoundRect( rOut.ImplGetTextBoundRect( *pSalLayout ) );
                    aActionBounds = rOut.PixelToLogic( aBoundRect );
                }
            }
        }
        break;

        case MetaActionType::TEXTLINE:
            OSL_FAIL("MetaActionType::TEXTLINE not supported");
        break;

        default:
            break;
    }

    if( !aActionBounds.IsEmpty() )
    {
        // fdo#40421 limit current action's output to clipped area
        if( rOut.IsClipRegion() )
            return rOut.LogicToPixel(
                rOut.GetClipRegion().GetBoundRect().Intersection( aActionBounds ) );
        else
            return rOut.LogicToPixel( aActionBounds );
    }
    else
        return tools::Rectangle();
}

} // end anon namespace

// TODO: this massive function operates on metafiles, so eventually it should probably
// be shifted to the GDIMetaFile class
bool OutputDevice::RemoveTransparenciesFromMetaFile( const GDIMetaFile& rInMtf, GDIMetaFile& rOutMtf,
                                                     tools::Long nMaxBmpDPIX, tools::Long nMaxBmpDPIY,
                                                     bool bReduceTransparency, bool bTransparencyAutoMode,
                                                     bool bDownsampleBitmaps,
                                                     const Color& rBackground
                                                     )
{
    MetaAction*             pCurrAct;
    bool                    bTransparent( false );

    rOutMtf.Clear();

    if(!bReduceTransparency || bTransparencyAutoMode)
        bTransparent = rInMtf.HasTransparentActions();

    // #i10613# Determine set of connected components containing transparent objects. These are
    // then processed as bitmaps, the original actions are removed from the metafile.
    if( !bTransparent )
    {
        // nothing transparent -> just copy
        rOutMtf = rInMtf;
    }
    else
    {
        // #i10613#
        // This works as follows: we want a number of distinct sets of
        // connected components, where each set contains metafile
        // actions that are intersecting (note: there are possibly
        // more actions contained as are directly intersecting,
        // because we can only produce rectangular bitmaps later
        // on. Thus, each set of connected components is the smallest
        // enclosing, axis-aligned rectangle that completely bounds a
        // number of intersecting metafile actions, plus any action
        // that would otherwise be cut in two). Therefore, we
        // iteratively add metafile actions from the original metafile
        // to this connected components list (aCCList), by checking
        // each element's bounding box against intersection with the
        // metaaction at hand.
        // All those intersecting elements are removed from aCCList
        // and collected in a temporary list (aCCMergeList). After all
        // elements have been checked, the aCCMergeList elements are
        // merged with the metaaction at hand into one resulting
        // connected component, with one big bounding box, and
        // inserted into aCCList again.
        // The time complexity of this algorithm is O(n^3), where n is
        // the number of metafile actions, and it finds all distinct
        // regions of rectangle-bounded connected components. This
        // algorithm was designed by AF.

        //  STAGE 1: Detect background

        // Receives uniform background content, and is _not_ merged
        // nor checked for intersection against other aCCList elements
        ConnectedComponents aBackgroundComponent;

        // Read the configuration value of minimal object area where transparency will be removed
        double fReduceTransparencyMinArea = officecfg::Office::Common::VCL::ReduceTransparencyMinArea::get() / 100.0;
        SAL_WARN_IF(fReduceTransparencyMinArea > 1.0, "vcl",
            "Value of ReduceTransparencyMinArea config option is too high");
        SAL_WARN_IF(fReduceTransparencyMinArea < 0.0, "vcl",
            "Value of ReduceTransparencyMinArea config option is too low");
        fReduceTransparencyMinArea = std::clamp(fReduceTransparencyMinArea, 0.0, 1.0);

        // create an OutputDevice to record mapmode changes and the like
        ScopedVclPtrInstance< VirtualDevice > aMapModeVDev;
        aMapModeVDev->mnDPIX = mnDPIX;
        aMapModeVDev->mnDPIY = mnDPIY;
        aMapModeVDev->EnableOutput(false);

        // weed out page-filling background objects (if they are
        // uniformly coloured). Keeping them outside the other
        // connected components often prevents whole-page bitmap
        // generation.
        bool bStillBackground=true; // true until first non-bg action
        int nActionNum = 0, nLastBgAction = -1;
        pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction();
        if( rBackground != COL_TRANSPARENT )
        {
            aBackgroundComponent.aBgColor = rBackground;
            aBackgroundComponent.aBounds = GetBackgroundComponentBounds();
        }
        while( pCurrAct && bStillBackground )
        {
            switch( pCurrAct->GetType() )
            {
                case MetaActionType::RECT:
                {
                    if( !checkRect(
                            aBackgroundComponent.aBounds,
                            aBackgroundComponent.aBgColor,
                            static_cast<const MetaRectAction*>(pCurrAct)->GetRect(),
                            *aMapModeVDev) )
                        bStillBackground=false; // incomplete occlusion of background
                    else
                        nLastBgAction=nActionNum; // this _is_ background
                    break;
                }
                case MetaActionType::POLYGON:
                {
                    const tools::Polygon aPoly(
                        static_cast<const MetaPolygonAction*>(pCurrAct)->GetPolygon());
                    if( !basegfx::utils::isRectangle(
                            aPoly.getB2DPolygon()) ||
                        !checkRect(
                            aBackgroundComponent.aBounds,
                            aBackgroundComponent.aBgColor,
                            aPoly.GetBoundRect(),
                            *aMapModeVDev) )
                        bStillBackground=false; // incomplete occlusion of background
                    else
                        nLastBgAction=nActionNum; // this _is_ background
                    break;
                }
                case MetaActionType::POLYPOLYGON:
                {
                    const tools::PolyPolygon aPoly(
                        static_cast<const MetaPolyPolygonAction*>(pCurrAct)->GetPolyPolygon());
                    if( aPoly.Count() != 1 ||
                        !basegfx::utils::isRectangle(
                            aPoly[0].getB2DPolygon()) ||
                        !checkRect(
                            aBackgroundComponent.aBounds,
                            aBackgroundComponent.aBgColor,
                            aPoly.GetBoundRect(),
                            *aMapModeVDev) )
                        bStillBackground=false; // incomplete occlusion of background
                    else
                        nLastBgAction=nActionNum; // this _is_ background
                    break;
                }
                case MetaActionType::WALLPAPER:
                {
                    if( !checkRect(
                            aBackgroundComponent.aBounds,
                            aBackgroundComponent.aBgColor,
                            static_cast<const MetaWallpaperAction*>(pCurrAct)->GetRect(),
                            *aMapModeVDev) )
                        bStillBackground=false; // incomplete occlusion of background
                    else
                        nLastBgAction=nActionNum; // this _is_ background
                    break;
                }
                default:
                {
                    if( ImplIsNotTransparent( *pCurrAct,
                                              *aMapModeVDev ) )
                        bStillBackground=false; // non-transparent action, possibly
                                                // not uniform
                    else
                        // extend current bounds (next uniform action
                        // needs to fully cover this area)
                        aBackgroundComponent.aBounds.Union(
                            ImplCalcActionBounds(*pCurrAct, *aMapModeVDev) );
                    break;
                }
            }

            // execute action to get correct MapModes etc.
            pCurrAct->Execute( aMapModeVDev.get() );

            pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction();
            ++nActionNum;
        }

        if (nLastBgAction != -1)
        {
            size_t nActionSize = rInMtf.GetActionSize();
            // tdf#134736 move nLastBgAction to also include any trailing pops
            for (size_t nPostLastBgAction = nLastBgAction + 1; nPostLastBgAction < nActionSize; ++nPostLastBgAction)
            {
                if (rInMtf.GetAction(nPostLastBgAction)->GetType() != MetaActionType::POP)
                    break;
                nLastBgAction = nPostLastBgAction;
            }
        }

        aMapModeVDev->ClearStack(); // clean up aMapModeVDev

        // fast-forward until one after the last background action
        // (need to reconstruct map mode vdev state)
        nActionNum=0;
        pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction();
        while( pCurrAct && nActionNum<=nLastBgAction )
        {
            // up to and including last ink-generating background
            // action go to background component
            aBackgroundComponent.aComponentList.emplace_back(
                    pCurrAct, nActionNum );

            // execute action to get correct MapModes etc.
            pCurrAct->Execute( aMapModeVDev.get() );
            pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction();
            ++nActionNum;
        }

        //  STAGE 2: Generate connected components list

        ::std::vector<ConnectedComponents> aCCList; // contains distinct sets of connected components as elements.

        // iterate over all actions (start where background action
        // search left off)
        for( ;
             pCurrAct;
             pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction(), ++nActionNum )
        {
            // execute action to get correct MapModes etc.
            pCurrAct->Execute( aMapModeVDev.get() );

            // cache bounds of current action
            const tools::Rectangle aBBCurrAct( ImplCalcActionBounds(*pCurrAct, *aMapModeVDev) );

            // accumulate collected bounds here, initialize with current action
            tools::Rectangle aTotalBounds( aBBCurrAct ); // thus, aTotalComponents.aBounds is empty
                                                         // for non-output-generating actions
            bool bTreatSpecial( false );
            ConnectedComponents aTotalComponents;

            //  STAGE 2.1: Search for intersecting cc entries

            // if aBBCurrAct is empty, it will intersect with no
            // aCCList member. Thus, we can save the check.
            // Furthermore, this ensures that non-output-generating
            // actions get their own aCCList entry, which is necessary
            // when copying them to the output metafile (see stage 4
            // below).

            // #107169# Wholly transparent objects need
            // not be considered for connected components,
            // too. Just put each of them into a separate
            // component.
            aTotalComponents.bIsFullyTransparent = !ImplIsNotTransparent(*pCurrAct, *aMapModeVDev);

            if( !aBBCurrAct.IsEmpty() &&
                !aTotalComponents.bIsFullyTransparent )
            {
                if( !aBackgroundComponent.aComponentList.empty() &&
                    !aBackgroundComponent.aBounds.Contains(aTotalBounds) )
                {
                    // it seems the background is not large enough. to
                    // be on the safe side, combine with this component.
                    aTotalBounds.Union( aBackgroundComponent.aBounds );

                    // extract all aCurr actions to aTotalComponents
                    aTotalComponents.aComponentList.splice( aTotalComponents.aComponentList.end(),
                                                            aBackgroundComponent.aComponentList );

                    if( aBackgroundComponent.bIsSpecial )
                        bTreatSpecial = true;
                }

                bool                                    bSomeComponentsChanged;

                // now, this is unfortunate: since changing anyone of
                // the aCCList elements (e.g. by merging or addition
                // of an action) might generate new intersection with
                // other aCCList elements, have to repeat the whole
                // element scanning, until nothing changes anymore.
                // Thus, this loop here makes us O(n^3) in the worst
                // case.
                do
                {
                    // only loop here if 'intersects' branch below was hit
                    bSomeComponentsChanged = false;

                    // iterate over all current members of aCCList
                    for( auto aCurrCC=aCCList.begin(); aCurrCC != aCCList.end(); )
                    {
                        // first check if current element's bounds are
                        // empty. This ensures that empty actions are not
                        // merged into one component, as a matter of fact,
                        // they have no position.

                        // #107169# Wholly transparent objects need
                        // not be considered for connected components,
                        // too. Just put each of them into a separate
                        // component.
                        if( !aCurrCC->aBounds.IsEmpty() &&
                            !aCurrCC->bIsFullyTransparent &&
                            aCurrCC->aBounds.Overlaps( aTotalBounds ) )
                        {
                            // union the intersecting aCCList element into aTotalComponents

                            // calc union bounding box
                            aTotalBounds.Union( aCurrCC->aBounds );

                            // extract all aCurr actions to aTotalComponents
                            aTotalComponents.aComponentList.splice( aTotalComponents.aComponentList.end(),
                                                                    aCurrCC->aComponentList );

                            if( aCurrCC->bIsSpecial )
                                bTreatSpecial = true;

                            // remove and delete aCurrCC element from list (we've now merged its content)
                            aCurrCC = aCCList.erase( aCurrCC );

                            // at least one component changed, need to rescan everything
                            bSomeComponentsChanged = true;
                        }
                        else
                        {
                            ++aCurrCC;
                        }
                    }
                }
                while( bSomeComponentsChanged );
            }

            //  STAGE 2.2: Determine special state for cc element

            // now test whether the whole connected component must be
            // treated specially (i.e. rendered as a bitmap): if the
            // added action is the very first action, or all actions
            // before it are completely transparent, the connected
            // component need not be treated specially, not even if
            // the added action contains transparency. This is because
            // painting of transparent objects on _white background_
            // works without alpha compositing (you just calculate the
            // color). Note that for the test "all objects before me
            // are transparent" no sorting is necessary, since the
            // added metaaction pCurrAct is always in the order the
            // metafile is painted. Generally, the order of the
            // metaactions in the ConnectedComponents are not
            // guaranteed to be the same as in the metafile.
            if( bTreatSpecial )
            {
                // prev component(s) special -> this one, too
                aTotalComponents.bIsSpecial = true;
            }
            else if(!pCurrAct->IsTransparent())
            {
                // added action and none of prev components special ->
                // this one normal, too
                aTotalComponents.bIsSpecial = false;
            }
            else
            {
                // added action is special and none of prev components
                // special -> do the detailed tests

                // can the action handle transparency correctly
                // (i.e. when painted on white background, does the
                // action still look correct)?
                if( !DoesActionHandleTransparency( *pCurrAct ) )
                {
                    // no, action cannot handle its transparency on
                    // a printer device, render to bitmap
                    aTotalComponents.bIsSpecial = true;
                }
                else
                {
                    // yes, action can handle its transparency, so
                    // check whether we're on white background
                    if( aTotalComponents.aComponentList.empty() )
                    {
                        // nothing between pCurrAct and page
                        // background -> don't be special
                        aTotalComponents.bIsSpecial = false;
                    }
                    else
                    {
                        // #107169# Fixes above now ensure that _no_
                        // object in the list is fully transparent. Thus,
                        // if the component list is not empty above, we
                        // must assume that we have to treat this
                        // component special.

                        // there are non-transparent objects between
                        // pCurrAct and the empty sheet of paper -> be
                        // special, then
                        aTotalComponents.bIsSpecial = true;
                    }
                }
            }

            //  STAGE 2.3: Add newly generated CC list element

            // set new bounds and add action to list
            aTotalComponents.aBounds = aTotalBounds;
            aTotalComponents.aComponentList.emplace_back(
                    pCurrAct, nActionNum );

            // add aTotalComponents as a new entry to aCCList
            aCCList.push_back( aTotalComponents );

            SAL_WARN_IF( aTotalComponents.aComponentList.empty(), "vcl",
                        "Printer::GetPreparedMetaFile empty component" );
            SAL_WARN_IF( aTotalComponents.aBounds.IsEmpty() && (aTotalComponents.aComponentList.size() != 1), "vcl",
                        "Printer::GetPreparedMetaFile non-output generating actions must be solitary");
            SAL_WARN_IF( aTotalComponents.bIsFullyTransparent && (aTotalComponents.aComponentList.size() != 1), "vcl",
                        "Printer::GetPreparedMetaFile fully transparent actions must be solitary");
        }

        // well now, we've got the list of disjunct connected
        // components. Now we've got to create a map, which contains
        // the corresponding aCCList element for every
        // metaaction. Later on, we always process the complete
        // metafile for each bitmap to be generated, but switch on
        // output only for actions contained in the then current
        // aCCList element. This ensures correct mapmode and attribute
        // settings for all cases.

        // maps mtf actions to CC list entries
        ::std::vector< const ConnectedComponents* > aCCList_MemberMap( rInMtf.GetActionSize() );

        // iterate over all aCCList members and their contained metaactions
        for (auto const& currentItem : aCCList)
        {
            for (auto const& currentAction : currentItem.aComponentList)
            {
                // set pointer to aCCList element for corresponding index
                aCCList_MemberMap[ currentAction.second ] = &currentItem;
            }
        }

        //  STAGE 3.1: Output background mtf actions (if there are any)

        for (auto & component : aBackgroundComponent.aComponentList)
        {
            // simply add this action (above, we inserted the actions
            // starting at index 0 up to and including nLastBgAction)
            rOutMtf.AddAction( component.first );
        }

        //  STAGE 3.2: Generate banded bitmaps for special regions

        Point aPageOffset;
        Size aTmpSize( GetOutputSizePixel() );
        if( meOutDevType == OUTDEV_PDF )
        {
            auto pPdfWriter = static_cast<vcl::PDFWriterImpl*>(this);
            aTmpSize = LogicToPixel(pPdfWriter->getCurPageSize(), MapMode(MapUnit::MapPoint));

            // also add error code to PDFWriter
            pPdfWriter->insertError(vcl::PDFWriter::Warning_Transparency_Converted);
        }
        else if( meOutDevType == OUTDEV_PRINTER )
        {
            Printer* pThis = dynamic_cast<Printer*>(this);
            assert(pThis);
            aPageOffset = pThis->GetPageOffsetPixel();
            aPageOffset = Point( 0, 0 ) - aPageOffset;
            aTmpSize  = pThis->GetPaperSizePixel();
        }
        const tools::Rectangle aOutputRect( aPageOffset, aTmpSize );
        bool bTiling = dynamic_cast<Printer*>(this) != nullptr;

        // iterate over all aCCList members and generate bitmaps for the special ones
        for (auto & currentItem : aCCList)
        {
            if( currentItem.bIsSpecial )
            {
                tools::Rectangle aBoundRect( currentItem.aBounds );
                aBoundRect.Intersection( aOutputRect );

                const double fBmpArea( static_cast<double>(aBoundRect.GetWidth()) * aBoundRect.GetHeight() );
                const double fOutArea( static_cast<double>(aOutputRect.GetWidth()) * aOutputRect.GetHeight() );

                // check if output doesn't exceed given size
                if( bReduceTransparency && bTransparencyAutoMode && ( fBmpArea > ( fReduceTransparencyMinArea * fOutArea ) ) )
                {
                    // output normally. Therefore, we simply clear the
                    // special attribute, as everything non-special is
                    // copied to rOutMtf further below.
                    currentItem.bIsSpecial = false;
                }
                else
                {
                    // create new bitmap action first
                    if( aBoundRect.GetWidth() && aBoundRect.GetHeight() )
                    {
                        Point           aDstPtPix( aBoundRect.TopLeft() );
                        Size            aDstSzPix;

                        ScopedVclPtrInstance<VirtualDevice> aMapVDev;   // here, we record only mapmode information
                        aMapVDev->EnableOutput(false);

                        ScopedVclPtrInstance<VirtualDevice> aPaintVDev; // into this one, we render.
                        aPaintVDev->SetBackground( aBackgroundComponent.aBgColor );

                        rOutMtf.AddAction( new MetaPushAction( vcl::PushFlags::MAPMODE ) );
                        rOutMtf.AddAction( new MetaMapModeAction() );

                        aPaintVDev->SetDrawMode( GetDrawMode() );

                        while( aDstPtPix.Y() <= aBoundRect.Bottom() )
                        {
                            aDstPtPix.setX( aBoundRect.Left() );
                            aDstSzPix = bTiling ? Size( MAX_TILE_WIDTH, MAX_TILE_HEIGHT ) : aBoundRect.GetSize();

                            if( ( aDstPtPix.Y() + aDstSzPix.Height() - 1 ) > aBoundRect.Bottom() )
                                aDstSzPix.setHeight( aBoundRect.Bottom() - aDstPtPix.Y() + 1 );

                            while( aDstPtPix.X() <= aBoundRect.Right() )
                            {
                                if( ( aDstPtPix.X() + aDstSzPix.Width() - 1 ) > aBoundRect.Right() )
                                    aDstSzPix.setWidth( aBoundRect.Right() - aDstPtPix.X() + 1 );

                                if( !tools::Rectangle( aDstPtPix, aDstSzPix ).Intersection( aBoundRect ).IsEmpty() &&
                                    aPaintVDev->SetOutputSizePixel( aDstSzPix ) )
                                {
                                    aPaintVDev->Push();
                                    aMapVDev->Push();

                                    aMapVDev->mnDPIX = aPaintVDev->mnDPIX = mnDPIX;
                                    aMapVDev->mnDPIY = aPaintVDev->mnDPIY = mnDPIY;

                                    aPaintVDev->EnableOutput(false);

                                    // iterate over all actions
                                    for( pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction(), nActionNum=0;
                                         pCurrAct;
                                         pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction(), ++nActionNum )
                                    {
                                        // enable output only for
                                        // actions that are members of
                                        // the current aCCList element
                                        // (currentItem)
                                        if( aCCList_MemberMap[nActionNum] == &currentItem )
                                            aPaintVDev->EnableOutput();

                                        // but process every action
                                        const MetaActionType nType( pCurrAct->GetType() );

                                        if( MetaActionType::MAPMODE == nType )
                                        {
                                            pCurrAct->Execute( aMapVDev.get() );

                                            MapMode     aMtfMap( aMapVDev->GetMapMode() );
                                            const Point aNewOrg( aMapVDev->PixelToLogic( aDstPtPix ) );

                                            aMtfMap.SetOrigin( Point( -aNewOrg.X(), -aNewOrg.Y() ) );
                                            aPaintVDev->SetMapMode( aMtfMap );
                                        }
                                        else if( ( MetaActionType::PUSH == nType ) || MetaActionType::POP == nType )
                                        {
                                            pCurrAct->Execute( aMapVDev.get() );
                                            pCurrAct->Execute( aPaintVDev.get() );
                                        }
                                        else if( MetaActionType::GRADIENT == nType )
                                        {
                                            MetaGradientAction* pGradientAction = static_cast<MetaGradientAction*>(pCurrAct);
                                            Printer* pPrinter = dynamic_cast< Printer* >(this);
                                            if( pPrinter )
                                                pPrinter->DrawGradientEx( aPaintVDev.get(), pGradientAction->GetRect(), pGradientAction->GetGradient() );
                                            else
                                                DrawGradient( pGradientAction->GetRect(), pGradientAction->GetGradient() );
                                        }
                                        else
                                        {
                                            pCurrAct->Execute( aPaintVDev.get() );
                                        }

                                        Application::Reschedule( true );
                                    }

                                    const bool bOldMap = mbMap;
                                    mbMap = aPaintVDev->mbMap = false;

                                    Bitmap aBandBmp( aPaintVDev->GetBitmap( Point(), aDstSzPix ) );

                                    // scale down bitmap, if requested
                                    if( bDownsampleBitmaps )
                                        aBandBmp = vcl::bitmap::GetDownsampledBitmap(PixelToLogic(LogicToPixel(aDstSzPix), MapMode(MapUnit::MapTwip)),
                                                                         Point(), aBandBmp.GetSizePixel(),
                                                                         aBandBmp, nMaxBmpDPIX, nMaxBmpDPIY);

                                    rOutMtf.AddAction( new MetaCommentAction( "PRNSPOOL_TRANSPARENTBITMAP_BEGIN" ) );
                                    rOutMtf.AddAction( new MetaBmpScaleAction( aDstPtPix, aDstSzPix, aBandBmp ) );
                                    rOutMtf.AddAction( new MetaCommentAction( "PRNSPOOL_TRANSPARENTBITMAP_END" ) );

                                    aPaintVDev->mbMap = true;
                                    mbMap = bOldMap;
                                    aMapVDev->Pop();
                                    aPaintVDev->Pop();
                                }

                                // overlapping bands to avoid missing lines (e.g. PostScript)
                                aDstPtPix.AdjustX(aDstSzPix.Width() );
                            }

                            // overlapping bands to avoid missing lines (e.g. PostScript)
                            aDstPtPix.AdjustY(aDstSzPix.Height() );
                        }

                        rOutMtf.AddAction( new MetaPopAction() );
                    }
                }
            }
        }

        aMapModeVDev->ClearStack(); // clean up aMapModeVDev

        //  STAGE 4: Copy actions to output metafile

        // iterate over all actions and duplicate the ones not in a
        // special aCCList member into rOutMtf
        for( pCurrAct=const_cast<GDIMetaFile&>(rInMtf).FirstAction(), nActionNum=0;
             pCurrAct;
             pCurrAct=const_cast<GDIMetaFile&>(rInMtf).NextAction(), ++nActionNum )
        {
            const ConnectedComponents* pCurrAssociatedComponent = aCCList_MemberMap[nActionNum];

            // NOTE: This relies on the fact that map-mode or draw
            // mode changing actions are solitary aCCList elements and
            // have empty bounding boxes, see comment on stage 2.1
            // above
            if( pCurrAssociatedComponent &&
                (pCurrAssociatedComponent->aBounds.IsEmpty() ||
                 !pCurrAssociatedComponent->bIsSpecial) )
            {
                // #107169# Treat transparent bitmaps special, if they
                // are the first (or sole) action in their bounds
                // list. Note that we previously ensured that no
                // fully-transparent objects are before us here.
                if( DoesActionHandleTransparency( *pCurrAct ) &&
                    pCurrAssociatedComponent->aComponentList.begin()->first == pCurrAct )
                {
                    // convert actions, where masked-out parts are of
                    // given background color
                    ImplConvertTransparentAction(rOutMtf,
                                                 *pCurrAct,
                                                 *aMapModeVDev,
                                                 aBackgroundComponent.aBgColor);
                }
                else
                {
                    // simply add this action
                    rOutMtf.AddAction( pCurrAct );
                }

                pCurrAct->Execute(aMapModeVDev.get());
            }
        }

        rOutMtf.SetPrefMapMode( rInMtf.GetPrefMapMode() );
        rOutMtf.SetPrefSize( rInMtf.GetPrefSize() );

#if OSL_DEBUG_LEVEL > 1
        // iterate over all aCCList members and generate rectangles for the bounding boxes
        rOutMtf.AddAction( new MetaFillColorAction( COL_WHITE, false ) );
        for(auto const& aCurr:aCCList)
        {
            if( aCurr.bIsSpecial )
                rOutMtf.AddAction( new MetaLineColorAction( COL_RED, true) );
            else
                rOutMtf.AddAction( new MetaLineColorAction( COL_BLUE, true) );

            rOutMtf.AddAction( new MetaRectAction( aMapModeVDev->PixelToLogic( aCurr.aBounds ) ) );
        }
#endif
    }
    return bTransparent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
