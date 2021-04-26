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

#include <tools/bigint.hxx>
#include <tools/debug.hxx>
#include <vcl/cursor.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wrkwin.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <svdata.hxx>
#include <window.h>
#include <outdev.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <o3tl/enumarray.hxx>

// we don't actually handle units beyond, hence the zeros in the arrays
const MapUnit s_MaxValidUnit = MapUnit::MapPixel;
const o3tl::enumarray<MapUnit,tools::Long> aImplNumeratorAry =
     {    1,   1,   5,  50,    1,   1,  1, 1,  1,    1, 1, 0, 0, 0 };
const o3tl::enumarray<MapUnit,tools::Long> aImplDenominatorAry =
     { 2540, 254, 127, 127, 1000, 100, 10, 1, 72, 1440, 1, 0, 0, 0 };

/*
Reduces accuracy until it is a fraction (should become
ctor fraction once); we could also do this with BigInts
*/

static Fraction ImplMakeFraction( tools::Long nN1, tools::Long nN2, tools::Long nD1, tools::Long nD2 )
{
    if( nD1 == 0 || nD2 == 0 ) //under these bad circumstances the following while loop will be endless
    {
        SAL_WARN("vcl.gdi", "Invalid parameter for ImplMakeFraction");
        return Fraction( 1, 1 );
    }

    tools::Long i = 1;

    if ( nN1 < 0 ) { i = -i; nN1 = -nN1; }
    if ( nN2 < 0 ) { i = -i; nN2 = -nN2; }
    if ( nD1 < 0 ) { i = -i; nD1 = -nD1; }
    if ( nD2 < 0 ) { i = -i; nD2 = -nD2; }
    // all positive; i sign

    Fraction aF = Fraction( i*nN1, nD1 ) * Fraction( nN2, nD2 );

    while ( !aF.IsValid() ) {
        if ( nN1 > nN2 )
            nN1 = (nN1 + 1) / 2;
        else
            nN2 = (nN2 + 1) / 2;
        if ( nD1 > nD2 )
            nD1 = (nD1 + 1) / 2;
        else
            nD2 = (nD2 + 1) / 2;

        aF = Fraction( i*nN1, nD1 ) * Fraction( nN2, nD2 );
    }

    aF.ReduceInaccurate(32);
    return aF;
}

static void ImplCalcMapResolution( const MapMode& rMapMode,
                                   tools::Long nDPIX, tools::Long nDPIY, ImplMapRes& rMapRes )
{
    switch ( rMapMode.GetMapUnit() )
    {
        case MapUnit::MapRelative:
            break;
        case MapUnit::Map100thMM:
            rMapRes.mnMapScNumX   = 1;
            rMapRes.mnMapScDenomX = 2540;
            rMapRes.mnMapScNumY   = 1;
            rMapRes.mnMapScDenomY = 2540;
            break;
        case MapUnit::Map10thMM:
            rMapRes.mnMapScNumX   = 1;
            rMapRes.mnMapScDenomX = 254;
            rMapRes.mnMapScNumY   = 1;
            rMapRes.mnMapScDenomY = 254;
            break;
        case MapUnit::MapMM:
            rMapRes.mnMapScNumX   = 5;      // 10
            rMapRes.mnMapScDenomX = 127;    // 254
            rMapRes.mnMapScNumY   = 5;      // 10
            rMapRes.mnMapScDenomY = 127;    // 254
            break;
        case MapUnit::MapCM:
            rMapRes.mnMapScNumX   = 50;     // 100
            rMapRes.mnMapScDenomX = 127;    // 254
            rMapRes.mnMapScNumY   = 50;     // 100
            rMapRes.mnMapScDenomY = 127;    // 254
            break;
        case MapUnit::Map1000thInch:
            rMapRes.mnMapScNumX   = 1;
            rMapRes.mnMapScDenomX = 1000;
            rMapRes.mnMapScNumY   = 1;
            rMapRes.mnMapScDenomY = 1000;
            break;
        case MapUnit::Map100thInch:
            rMapRes.mnMapScNumX   = 1;
            rMapRes.mnMapScDenomX = 100;
            rMapRes.mnMapScNumY   = 1;
            rMapRes.mnMapScDenomY = 100;
            break;
        case MapUnit::Map10thInch:
            rMapRes.mnMapScNumX   = 1;
            rMapRes.mnMapScDenomX = 10;
            rMapRes.mnMapScNumY   = 1;
            rMapRes.mnMapScDenomY = 10;
            break;
        case MapUnit::MapInch:
            rMapRes.mnMapScNumX   = 1;
            rMapRes.mnMapScDenomX = 1;
            rMapRes.mnMapScNumY   = 1;
            rMapRes.mnMapScDenomY = 1;
            break;
        case MapUnit::MapPoint:
            rMapRes.mnMapScNumX   = 1;
            rMapRes.mnMapScDenomX = 72;
            rMapRes.mnMapScNumY   = 1;
            rMapRes.mnMapScDenomY = 72;
            break;
        case MapUnit::MapTwip:
            rMapRes.mnMapScNumX   = 1;
            rMapRes.mnMapScDenomX = 1440;
            rMapRes.mnMapScNumY   = 1;
            rMapRes.mnMapScDenomY = 1440;
            break;
        case MapUnit::MapPixel:
            rMapRes.mnMapScNumX   = 1;
            rMapRes.mnMapScDenomX = nDPIX;
            rMapRes.mnMapScNumY   = 1;
            rMapRes.mnMapScDenomY = nDPIY;
            break;
        case MapUnit::MapSysFont:
        case MapUnit::MapAppFont:
            {
            ImplSVData* pSVData = ImplGetSVData();
            if ( !pSVData->maGDIData.mnAppFontX )
            {
                if (pSVData->maFrameData.mpFirstFrame)
                    vcl::Window::ImplInitAppFontData(pSVData->maFrameData.mpFirstFrame);
                else
                {
                    ScopedVclPtrInstance<WorkWindow> pWin( nullptr, 0 );
                    vcl::Window::ImplInitAppFontData( pWin );
                }
            }
            rMapRes.mnMapScNumX   = pSVData->maGDIData.mnAppFontX;
            rMapRes.mnMapScDenomX = nDPIX * 40;
            rMapRes.mnMapScNumY   = pSVData->maGDIData.mnAppFontY;
            rMapRes.mnMapScDenomY = nDPIY * 80;
            }
            break;
        default:
            OSL_FAIL( "unhandled MapUnit" );
            break;
    }

    const Fraction& aScaleX = rMapMode.GetScaleX();
    const Fraction& aScaleY = rMapMode.GetScaleY();

    // set offset according to MapMode
    Point aOrigin = rMapMode.GetOrigin();
    if ( rMapMode.GetMapUnit() != MapUnit::MapRelative )
    {
        rMapRes.mnMapOfsX = aOrigin.X();
        rMapRes.mnMapOfsY = aOrigin.Y();
    }
    else
    {
        auto nXNumerator = aScaleX.GetNumerator();
        auto nYNumerator = aScaleY.GetNumerator();
        assert(nXNumerator != 0 && nYNumerator != 0);

        BigInt aX( rMapRes.mnMapOfsX );
        aX *= BigInt( aScaleX.GetDenominator() );
        if ( rMapRes.mnMapOfsX >= 0 )
        {
            if (nXNumerator >= 0)
                aX += BigInt(nXNumerator / 2);
            else
                aX -= BigInt((nXNumerator + 1) / 2);
        }
        else
        {
            if (nXNumerator >= 0 )
                aX -= BigInt((nXNumerator - 1) / 2);
            else
                aX += BigInt(nXNumerator / 2);
        }
        aX /= BigInt(nXNumerator);
        rMapRes.mnMapOfsX = static_cast<tools::Long>(aX) + aOrigin.X();
        BigInt aY( rMapRes.mnMapOfsY );
        aY *= BigInt( aScaleY.GetDenominator() );
        if( rMapRes.mnMapOfsY >= 0 )
        {
            if (nYNumerator >= 0)
                aY += BigInt(nYNumerator / 2);
            else
                aY -= BigInt((nYNumerator + 1) / 2);
        }
        else
        {
            if (nYNumerator >= 0)
                aY -= BigInt((nYNumerator - 1) / 2);
            else
                aY += BigInt(nYNumerator / 2);
        }
        aY /= BigInt(nYNumerator);
        rMapRes.mnMapOfsY = static_cast<tools::Long>(aY) + aOrigin.Y();
    }

    // calculate scaling factor according to MapMode
    // aTemp? = rMapRes.mnMapSc? * aScale?
    Fraction aTempX = ImplMakeFraction( rMapRes.mnMapScNumX,
                                        aScaleX.GetNumerator(),
                                        rMapRes.mnMapScDenomX,
                                        aScaleX.GetDenominator() );
    Fraction aTempY = ImplMakeFraction( rMapRes.mnMapScNumY,
                                        aScaleY.GetNumerator(),
                                        rMapRes.mnMapScDenomY,
                                        aScaleY.GetDenominator() );
    rMapRes.mnMapScNumX   = aTempX.GetNumerator();
    rMapRes.mnMapScDenomX = aTempX.GetDenominator();
    rMapRes.mnMapScNumY   = aTempY.GetNumerator();
    rMapRes.mnMapScDenomY = aTempY.GetDenominator();
}

// #i75163#
void OutputDevice::ImplInvalidateViewTransform()
{
    if(!mpOutDevData)
        return;

    if(mpOutDevData->mpViewTransform)
    {
        delete mpOutDevData->mpViewTransform;
        mpOutDevData->mpViewTransform = nullptr;
    }

    if(mpOutDevData->mpInverseViewTransform)
    {
        delete mpOutDevData->mpInverseViewTransform;
        mpOutDevData->mpInverseViewTransform = nullptr;
    }
}

static tools::Long ImplLogicToPixel(tools::Long n, tools::Long nDPI, tools::Long nMapNum,
                                    tools::Long nMapDenom)
{
    assert(nDPI > 0);
    assert(nMapDenom != 0);
    if constexpr (sizeof(tools::Long) >= 8)
    {
        assert(nMapNum >= 0);
        //detect overflows
        assert(nMapNum == 0
               || std::abs(n) < std::numeric_limits<tools::Long>::max() / nMapNum / nDPI);
    }
    sal_Int64 n64 = n;
    n64 *= nMapNum;
    n64 *= nDPI;
    if (nMapDenom == 1)
        n = static_cast<tools::Long>(n64);
    else
    {
        n64 = 2 * n64 / nMapDenom;
        if (n64 < 0)
            --n64;
        else
            ++n64;
        n = static_cast<tools::Long>(n64 / 2);
    }
    return n;
}

static tools::Long ImplPixelToLogic(tools::Long n, tools::Long nDPI, tools::Long nMapNum,
                                    tools::Long nMapDenom)
{
    assert(nDPI > 0);
    if (nMapNum == 0)
        return 0;
    sal_Int64 nDenom = nDPI;
    nDenom *= nMapNum;

    sal_Int64 n64 = n;
    n64 *= nMapDenom;
    if (nDenom == 1)
        n = static_cast<tools::Long>(n64);
    else
    {
        n64 = 2 * n64 / nDenom;
        if (n64 < 0)
            --n64;
        else
            ++n64;
        n = static_cast<tools::Long>(n64 / 2);
    }
    return n;
}

tools::Long OutputDevice::ImplLogicXToDevicePixel( tools::Long nX ) const
{
    if ( !mbMap )
        return nX+mnOutOffX;

    return ImplLogicToPixel( nX + maMapRes.mnMapOfsX, mnDPIX,
                             maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX )+mnOutOffX+mnOutOffOrigX;
}

tools::Long OutputDevice::ImplLogicYToDevicePixel( tools::Long nY ) const
{
    if ( !mbMap )
        return nY+mnOutOffY;

    return ImplLogicToPixel( nY + maMapRes.mnMapOfsY, mnDPIY,
                             maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY )+mnOutOffY+mnOutOffOrigY;
}

tools::Long OutputDevice::ImplLogicWidthToDevicePixel( tools::Long nWidth ) const
{
    if ( !mbMap )
        return nWidth;

    return ImplLogicToPixel(nWidth, mnDPIX, maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX);
}

tools::Long OutputDevice::ImplLogicHeightToDevicePixel( tools::Long nHeight ) const
{
    if ( !mbMap )
        return nHeight;

    return ImplLogicToPixel(nHeight, mnDPIY, maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY);
}

float OutputDevice::ImplFloatLogicHeightToDevicePixel( float fLogicHeight) const
{
    if( !mbMap)
        return fLogicHeight;
    float fPixelHeight = (fLogicHeight * mnDPIY * maMapRes.mnMapScNumY) / maMapRes.mnMapScDenomY;
    return fPixelHeight;
}

tools::Long OutputDevice::ImplDevicePixelToLogicWidth( tools::Long nWidth ) const
{
    if ( !mbMap )
        return nWidth;

    return ImplPixelToLogic(nWidth, mnDPIX, maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX);
}

tools::Long OutputDevice::ImplDevicePixelToLogicHeight( tools::Long nHeight ) const
{
    if ( !mbMap )
        return nHeight;

    return ImplPixelToLogic(nHeight, mnDPIY, maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY);
}

Point OutputDevice::ImplLogicToDevicePixel( const Point& rLogicPt ) const
{
    if ( !mbMap )
        return Point( rLogicPt.X()+mnOutOffX, rLogicPt.Y()+mnOutOffY );

    return Point( ImplLogicToPixel( rLogicPt.X() + maMapRes.mnMapOfsX, mnDPIX,
                                    maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX )+mnOutOffX+mnOutOffOrigX,
                  ImplLogicToPixel( rLogicPt.Y() + maMapRes.mnMapOfsY, mnDPIY,
                                    maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY )+mnOutOffY+mnOutOffOrigY );
}

Size OutputDevice::ImplLogicToDevicePixel( const Size& rLogicSize ) const
{
    if ( !mbMap )
        return rLogicSize;

    return Size( ImplLogicToPixel( rLogicSize.Width(), mnDPIX,
                                   maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX ),
                 ImplLogicToPixel( rLogicSize.Height(), mnDPIY,
                                   maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY ) );
}

tools::Rectangle OutputDevice::ImplLogicToDevicePixel( const tools::Rectangle& rLogicRect ) const
{
    // tdf#141761 IsEmpty() removed
    // Even if rLogicRect.IsEmpty(), transform of the Position contained
    // in the Rectangle is necessary. Due to Rectangle::Right() returning
    // Left() when IsEmpty(), the code *could* stay unchanged (same for Bottom),
    // but:
    // The Rectangle constructor used with the four tools::Long values does not
    // check for IsEmpty(), so to keep that state correct there are two possibilities:
    // (1) Add a test to the Rectangle constructor in question
    // (2) Do it handish here
    // I have tried (1) first, but test Test::test_rectangle() claims that for
    //  tools::Rectangle aRect(1, 1, 1, 1);
    //    tools::Long(1) == aRect.GetWidth()
    //    tools::Long(0) == aRect.getWidth()
    // (remember: this means Left == Right == 1 -> GetWidth => 1, getWidth == 0)
    // so indeed tthe 1's have to go uncommened/unchecked into the data body
    // of rectangle. Switching to (2) *is* needed, doing so
    tools::Rectangle aRetval;

    if ( !mbMap )
    {
        aRetval = tools::Rectangle(
            rLogicRect.Left()+mnOutOffX,
            rLogicRect.Top()+mnOutOffY,
            rLogicRect.IsWidthEmpty() ? 0 : rLogicRect.Right()+mnOutOffX,
            rLogicRect.IsHeightEmpty() ? 0 : rLogicRect.Bottom()+mnOutOffY );
    }
    else
    {
        aRetval = tools::Rectangle(
            ImplLogicToPixel( rLogicRect.Left()+maMapRes.mnMapOfsX, mnDPIX, maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX )+mnOutOffX+mnOutOffOrigX,
            ImplLogicToPixel( rLogicRect.Top()+maMapRes.mnMapOfsY, mnDPIY, maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY )+mnOutOffY+mnOutOffOrigY,
            rLogicRect.IsWidthEmpty() ? 0 : ImplLogicToPixel( rLogicRect.Right()+maMapRes.mnMapOfsX, mnDPIX, maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX )+mnOutOffX+mnOutOffOrigX,
            rLogicRect.IsHeightEmpty() ? 0 : ImplLogicToPixel( rLogicRect.Bottom()+maMapRes.mnMapOfsY, mnDPIY, maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY )+mnOutOffY+mnOutOffOrigY );
    }

    if(rLogicRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if(rLogicRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Polygon OutputDevice::ImplLogicToDevicePixel( const tools::Polygon& rLogicPoly ) const
{
    if ( !mbMap && !mnOutOffX && !mnOutOffY )
        return rLogicPoly;

    sal_uInt16  i;
    sal_uInt16  nPoints = rLogicPoly.GetSize();
    tools::Polygon aPoly( rLogicPoly );

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    if ( mbMap )
    {
        for ( i = 0; i < nPoints; i++ )
        {
            const Point& rPt = pPointAry[i];
            Point aPt(ImplLogicToPixel( rPt.X()+maMapRes.mnMapOfsX, mnDPIX,
                                        maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX )+mnOutOffX+mnOutOffOrigX,
                      ImplLogicToPixel( rPt.Y()+maMapRes.mnMapOfsY, mnDPIY,
                                        maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY )+mnOutOffY+mnOutOffOrigY);
            aPoly[i] = aPt;
        }
    }
    else
    {
        for ( i = 0; i < nPoints; i++ )
        {
            Point aPt = pPointAry[i];
            aPt.AdjustX(mnOutOffX );
            aPt.AdjustY(mnOutOffY );
            aPoly[i] = aPt;
        }
    }

    return aPoly;
}

tools::PolyPolygon OutputDevice::ImplLogicToDevicePixel( const tools::PolyPolygon& rLogicPolyPoly ) const
{
    if ( !mbMap && !mnOutOffX && !mnOutOffY )
        return rLogicPolyPoly;

    tools::PolyPolygon aPolyPoly( rLogicPolyPoly );
    sal_uInt16      nPoly = aPolyPoly.Count();
    for( sal_uInt16 i = 0; i < nPoly; i++ )
    {
        tools::Polygon& rPoly = aPolyPoly[i];
        rPoly = ImplLogicToDevicePixel( rPoly );
    }
    return aPolyPoly;
}

LineInfo OutputDevice::ImplLogicToDevicePixel( const LineInfo& rLineInfo ) const
{
    LineInfo aInfo( rLineInfo );

    if( aInfo.GetStyle() == LineStyle::Dash )
    {
        if( aInfo.GetDotCount() && aInfo.GetDotLen() )
            aInfo.SetDotLen( std::max( ImplLogicWidthToDevicePixel( aInfo.GetDotLen() ), tools::Long(1) ) );
        else
            aInfo.SetDotCount( 0 );

        if( aInfo.GetDashCount() && aInfo.GetDashLen() )
            aInfo.SetDashLen( std::max( ImplLogicWidthToDevicePixel( aInfo.GetDashLen() ), tools::Long(1) ) );
        else
            aInfo.SetDashCount( 0 );

        aInfo.SetDistance( ImplLogicWidthToDevicePixel( aInfo.GetDistance() ) );

        if( ( !aInfo.GetDashCount() && !aInfo.GetDotCount() ) || !aInfo.GetDistance() )
            aInfo.SetStyle( LineStyle::Solid );
    }

    aInfo.SetWidth( ImplLogicWidthToDevicePixel( aInfo.GetWidth() ) );

    return aInfo;
}

tools::Rectangle OutputDevice::ImplDevicePixelToLogic( const tools::Rectangle& rPixelRect ) const
{
    // tdf#141761 see comments above, IsEmpty() removed
    tools::Rectangle aRetval;

    if ( !mbMap )
    {
        aRetval = tools::Rectangle(
            rPixelRect.Left()-mnOutOffX,
            rPixelRect.Top()-mnOutOffY,
            rPixelRect.IsWidthEmpty() ? 0 : rPixelRect.Right()-mnOutOffX,
            rPixelRect.IsHeightEmpty() ? 0 : rPixelRect.Bottom()-mnOutOffY );
    }
    else
    {
        aRetval = tools::Rectangle(
            ImplPixelToLogic( rPixelRect.Left()-mnOutOffX-mnOutOffOrigX, mnDPIX, maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX )-maMapRes.mnMapOfsX,
            ImplPixelToLogic( rPixelRect.Top()-mnOutOffY-mnOutOffOrigY, mnDPIY, maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY )-maMapRes.mnMapOfsY,
            rPixelRect.IsWidthEmpty() ? 0 : ImplPixelToLogic( rPixelRect.Right()-mnOutOffX-mnOutOffOrigX, mnDPIX, maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX )-maMapRes.mnMapOfsX,
            rPixelRect.IsHeightEmpty() ? 0 : ImplPixelToLogic( rPixelRect.Bottom()-mnOutOffY-mnOutOffOrigY, mnDPIY, maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY )-maMapRes.mnMapOfsY );
    }

    if(rPixelRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if(rPixelRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

vcl::Region OutputDevice::ImplPixelToDevicePixel( const vcl::Region& rRegion ) const
{
    if ( !mnOutOffX && !mnOutOffY )
        return rRegion;

    vcl::Region aRegion( rRegion );
    aRegion.Move( mnOutOffX+mnOutOffOrigX, mnOutOffY+mnOutOffOrigY );
    return aRegion;
}

void OutputDevice::EnableMapMode( bool bEnable )
{
    mbMap = bEnable;

    if( mpAlphaVDev )
        mpAlphaVDev->EnableMapMode( bEnable );
}

void OutputDevice::SetMapMode()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaMapModeAction( MapMode() ) );

    if ( mbMap || !maMapMode.IsDefault() )
    {
        mbMap       = false;
        maMapMode   = MapMode();

        // create new objects (clip region are not re-scaled)
        mbNewFont   = true;
        mbInitFont  = true;
        ImplInitMapModeObjects();

        // #106426# Adapt logical offset when changing mapmode
        mnOutOffLogicX = mnOutOffOrigX; // no mapping -> equal offsets
        mnOutOffLogicY = mnOutOffOrigY;

        // #i75163#
        ImplInvalidateViewTransform();
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetMapMode();
}

void OutputDevice::SetMapMode( const MapMode& rNewMapMode )
{

    bool bRelMap = (rNewMapMode.GetMapUnit() == MapUnit::MapRelative);

    if ( mpMetaFile )
    {
        mpMetaFile->AddAction( new MetaMapModeAction( rNewMapMode ) );
    }

    // do nothing if MapMode was not changed
    if ( maMapMode == rNewMapMode )
        return;

    if( mpAlphaVDev )
        mpAlphaVDev->SetMapMode( rNewMapMode );

     // if default MapMode calculate nothing
    bool bOldMap = mbMap;
    mbMap = !rNewMapMode.IsDefault();
    if ( mbMap )
    {
        // if only the origin is converted, do not scale new
        if ( (rNewMapMode.GetMapUnit() == maMapMode.GetMapUnit()) &&
             (rNewMapMode.GetScaleX()  == maMapMode.GetScaleX())  &&
             (rNewMapMode.GetScaleY()  == maMapMode.GetScaleY())  &&
             (bOldMap                  == mbMap) )
        {
            // set offset
            Point aOrigin = rNewMapMode.GetOrigin();
            maMapRes.mnMapOfsX = aOrigin.X();
            maMapRes.mnMapOfsY = aOrigin.Y();
            maMapMode = rNewMapMode;

            // #i75163#
            ImplInvalidateViewTransform();

            return;
        }
        if ( !bOldMap && bRelMap )
        {
            maMapRes.mnMapScNumX    = 1;
            maMapRes.mnMapScNumY    = 1;
            maMapRes.mnMapScDenomX  = mnDPIX;
            maMapRes.mnMapScDenomY  = mnDPIY;
            maMapRes.mnMapOfsX      = 0;
            maMapRes.mnMapOfsY      = 0;
        }

        // calculate new MapMode-resolution
        ImplCalcMapResolution(rNewMapMode, mnDPIX, mnDPIY, maMapRes);
    }

    // set new MapMode
    if ( bRelMap )
    {
        Point aOrigin( maMapRes.mnMapOfsX, maMapRes.mnMapOfsY );
        // aScale? = maMapMode.GetScale?() * rNewMapMode.GetScale?()
        Fraction aScaleX = ImplMakeFraction( maMapMode.GetScaleX().GetNumerator(),
                                             rNewMapMode.GetScaleX().GetNumerator(),
                                             maMapMode.GetScaleX().GetDenominator(),
                                             rNewMapMode.GetScaleX().GetDenominator() );
        Fraction aScaleY = ImplMakeFraction( maMapMode.GetScaleY().GetNumerator(),
                                             rNewMapMode.GetScaleY().GetNumerator(),
                                             maMapMode.GetScaleY().GetDenominator(),
                                             rNewMapMode.GetScaleY().GetDenominator() );
        maMapMode.SetOrigin( aOrigin );
        maMapMode.SetScaleX( aScaleX );
        maMapMode.SetScaleY( aScaleY );
    }
    else
        maMapMode = rNewMapMode;

    // create new objects (clip region are not re-scaled)
    mbNewFont   = true;
    mbInitFont  = true;
    ImplInitMapModeObjects();

    // #106426# Adapt logical offset when changing mapmode
    mnOutOffLogicX = ImplPixelToLogic( mnOutOffOrigX, mnDPIX,
                                       maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX );
    mnOutOffLogicY = ImplPixelToLogic( mnOutOffOrigY, mnDPIY,
                                       maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY );

    // #i75163#
    ImplInvalidateViewTransform();
}

void OutputDevice::SetMetafileMapMode(const MapMode& rNewMapMode, bool bIsRecord)
{
    if (bIsRecord)
        SetRelativeMapMode(rNewMapMode);
    else
        SetMapMode(rNewMapMode);
}

void OutputDevice::ImplInitMapModeObjects() {}

void OutputDevice::SetRelativeMapMode( const MapMode& rNewMapMode )
{
    // do nothing if MapMode did not change
    if ( maMapMode == rNewMapMode )
        return;

    MapUnit eOld = maMapMode.GetMapUnit();
    MapUnit eNew = rNewMapMode.GetMapUnit();

    // a?F = rNewMapMode.GetScale?() / maMapMode.GetScale?()
    Fraction aXF = ImplMakeFraction( rNewMapMode.GetScaleX().GetNumerator(),
                                     maMapMode.GetScaleX().GetDenominator(),
                                     rNewMapMode.GetScaleX().GetDenominator(),
                                     maMapMode.GetScaleX().GetNumerator() );
    Fraction aYF = ImplMakeFraction( rNewMapMode.GetScaleY().GetNumerator(),
                                     maMapMode.GetScaleY().GetDenominator(),
                                     rNewMapMode.GetScaleY().GetDenominator(),
                                     maMapMode.GetScaleY().GetNumerator() );

    Point aPt( LogicToLogic( Point(), nullptr, &rNewMapMode ) );
    if ( eNew != eOld )
    {
        if ( eOld > MapUnit::MapPixel )
        {
            SAL_WARN( "vcl.gdi", "Not implemented MapUnit" );
        }
        else if ( eNew > MapUnit::MapPixel )
        {
            SAL_WARN( "vcl.gdi", "Not implemented MapUnit" );
        }
        else
        {
            Fraction aF( aImplNumeratorAry[eNew] * aImplDenominatorAry[eOld],
                         aImplNumeratorAry[eOld] * aImplDenominatorAry[eNew] );

            // a?F =  a?F * aF
            aXF = ImplMakeFraction( aXF.GetNumerator(),   aF.GetNumerator(),
                                    aXF.GetDenominator(), aF.GetDenominator() );
            aYF = ImplMakeFraction( aYF.GetNumerator(),   aF.GetNumerator(),
                                    aYF.GetDenominator(), aF.GetDenominator() );
            if ( eOld == MapUnit::MapPixel )
            {
                aXF *= Fraction( mnDPIX, 1 );
                aYF *= Fraction( mnDPIY, 1 );
            }
            else if ( eNew == MapUnit::MapPixel )
            {
                aXF *= Fraction( 1, mnDPIX );
                aYF *= Fraction( 1, mnDPIY );
            }
        }
    }

    MapMode aNewMapMode( MapUnit::MapRelative, Point( -aPt.X(), -aPt.Y() ), aXF, aYF );
    SetMapMode( aNewMapMode );

    if ( eNew != eOld )
        maMapMode = rNewMapMode;

    // #106426# Adapt logical offset when changing MapMode
    mnOutOffLogicX = ImplPixelToLogic( mnOutOffOrigX, mnDPIX,
                                       maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX );
    mnOutOffLogicY = ImplPixelToLogic( mnOutOffOrigY, mnDPIY,
                                       maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY );

    if( mpAlphaVDev )
        mpAlphaVDev->SetRelativeMapMode( rNewMapMode );
}

// #i75163#
basegfx::B2DHomMatrix OutputDevice::GetViewTransformation() const
{
    if(mbMap && mpOutDevData)
    {
        if(!mpOutDevData->mpViewTransform)
        {
            mpOutDevData->mpViewTransform = new basegfx::B2DHomMatrix;

            const double fScaleFactorX(static_cast<double>(mnDPIX) * static_cast<double>(maMapRes.mnMapScNumX) / static_cast<double>(maMapRes.mnMapScDenomX));
            const double fScaleFactorY(static_cast<double>(mnDPIY) * static_cast<double>(maMapRes.mnMapScNumY) / static_cast<double>(maMapRes.mnMapScDenomY));
            const double fZeroPointX((static_cast<double>(maMapRes.mnMapOfsX) * fScaleFactorX) + static_cast<double>(mnOutOffOrigX));
            const double fZeroPointY((static_cast<double>(maMapRes.mnMapOfsY) * fScaleFactorY) + static_cast<double>(mnOutOffOrigY));

            mpOutDevData->mpViewTransform->set(0, 0, fScaleFactorX);
            mpOutDevData->mpViewTransform->set(1, 1, fScaleFactorY);
            mpOutDevData->mpViewTransform->set(0, 2, fZeroPointX);
            mpOutDevData->mpViewTransform->set(1, 2, fZeroPointY);
        }

        return *mpOutDevData->mpViewTransform;
    }
    else
    {
        return basegfx::B2DHomMatrix();
    }
}

// #i75163#
basegfx::B2DHomMatrix OutputDevice::GetInverseViewTransformation() const
{
    if(mbMap && mpOutDevData)
    {
        if(!mpOutDevData->mpInverseViewTransform)
        {
            GetViewTransformation();
            mpOutDevData->mpInverseViewTransform = new basegfx::B2DHomMatrix(*mpOutDevData->mpViewTransform);
            mpOutDevData->mpInverseViewTransform->invert();
        }

        return *mpOutDevData->mpInverseViewTransform;
    }
    else
    {
        return basegfx::B2DHomMatrix();
    }
}

// #i75163#
basegfx::B2DHomMatrix OutputDevice::GetViewTransformation( const MapMode& rMapMode ) const
{
    // #i82615#
    ImplMapRes          aMapRes;
    ImplCalcMapResolution(rMapMode, mnDPIX, mnDPIY, aMapRes);

    basegfx::B2DHomMatrix aTransform;

    const double fScaleFactorX(static_cast<double>(mnDPIX) * static_cast<double>(aMapRes.mnMapScNumX) / static_cast<double>(aMapRes.mnMapScDenomX));
    const double fScaleFactorY(static_cast<double>(mnDPIY) * static_cast<double>(aMapRes.mnMapScNumY) / static_cast<double>(aMapRes.mnMapScDenomY));
    const double fZeroPointX((static_cast<double>(aMapRes.mnMapOfsX) * fScaleFactorX) + static_cast<double>(mnOutOffOrigX));
    const double fZeroPointY((static_cast<double>(aMapRes.mnMapOfsY) * fScaleFactorY) + static_cast<double>(mnOutOffOrigY));

    aTransform.set(0, 0, fScaleFactorX);
    aTransform.set(1, 1, fScaleFactorY);
    aTransform.set(0, 2, fZeroPointX);
    aTransform.set(1, 2, fZeroPointY);

    return aTransform;
}

// #i75163#
basegfx::B2DHomMatrix OutputDevice::GetInverseViewTransformation( const MapMode& rMapMode ) const
{
    basegfx::B2DHomMatrix aMatrix( GetViewTransformation( rMapMode ) );
    aMatrix.invert();
    return aMatrix;
}

basegfx::B2DHomMatrix OutputDevice::ImplGetDeviceTransformation() const
{
    basegfx::B2DHomMatrix aTransformation = GetViewTransformation();
    // TODO: is it worth to cache the transformed result?
    if( mnOutOffX || mnOutOffY )
        aTransformation.translate( mnOutOffX, mnOutOffY );
    return aTransformation;
}

Point OutputDevice::LogicToPixel( const Point& rLogicPt ) const
{

    if ( !mbMap )
        return rLogicPt;

    return Point( ImplLogicToPixel( rLogicPt.X() + maMapRes.mnMapOfsX, mnDPIX,
                                    maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX )+mnOutOffOrigX,
                  ImplLogicToPixel( rLogicPt.Y() + maMapRes.mnMapOfsY, mnDPIY,
                                    maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY )+mnOutOffOrigY );
}

Size OutputDevice::LogicToPixel( const Size& rLogicSize ) const
{

    if ( !mbMap )
        return rLogicSize;

    return Size( ImplLogicToPixel( rLogicSize.Width(), mnDPIX,
                                   maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX ),
                 ImplLogicToPixel( rLogicSize.Height(), mnDPIY,
                                   maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY ) );
}

tools::Rectangle OutputDevice::LogicToPixel( const tools::Rectangle& rLogicRect ) const
{
    // tdf#141761 see comments above, IsEmpty() removed
    if ( !mbMap )
        return rLogicRect;

    tools::Rectangle aRetval(
        ImplLogicToPixel( rLogicRect.Left() + maMapRes.mnMapOfsX, mnDPIX, maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX )+mnOutOffOrigX,
        ImplLogicToPixel( rLogicRect.Top() + maMapRes.mnMapOfsY, mnDPIY, maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY )+mnOutOffOrigY,
        rLogicRect.IsWidthEmpty() ? 0 : ImplLogicToPixel( rLogicRect.Right() + maMapRes.mnMapOfsX, mnDPIX, maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX )+mnOutOffOrigX,
        rLogicRect.IsHeightEmpty() ? 0 : ImplLogicToPixel( rLogicRect.Bottom() + maMapRes.mnMapOfsY, mnDPIY, maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY )+mnOutOffOrigY );

    if(rLogicRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if(rLogicRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Polygon OutputDevice::LogicToPixel( const tools::Polygon& rLogicPoly ) const
{

    if ( !mbMap )
        return rLogicPoly;

    sal_uInt16  i;
    sal_uInt16  nPoints = rLogicPoly.GetSize();
    tools::Polygon aPoly( rLogicPoly );

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    for ( i = 0; i < nPoints; i++ )
    {
        const Point* pPt = &(pPointAry[i]);
        Point aPt;
        aPt.setX( ImplLogicToPixel( pPt->X() + maMapRes.mnMapOfsX, mnDPIX,
                                    maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX )+mnOutOffOrigX );
        aPt.setY( ImplLogicToPixel( pPt->Y() + maMapRes.mnMapOfsY, mnDPIY,
                                    maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY )+mnOutOffOrigY );
        aPoly[i] = aPt;
    }

    return aPoly;
}

tools::PolyPolygon OutputDevice::LogicToPixel( const tools::PolyPolygon& rLogicPolyPoly ) const
{

    if ( !mbMap )
        return rLogicPolyPoly;

    tools::PolyPolygon aPolyPoly( rLogicPolyPoly );
    sal_uInt16      nPoly = aPolyPoly.Count();
    for( sal_uInt16 i = 0; i < nPoly; i++ )
    {
        tools::Polygon& rPoly = aPolyPoly[i];
        rPoly = LogicToPixel( rPoly );
    }
    return aPolyPoly;
}

basegfx::B2DPolyPolygon OutputDevice::LogicToPixel( const basegfx::B2DPolyPolygon& rLogicPolyPoly ) const
{
    basegfx::B2DPolyPolygon aTransformedPoly = rLogicPolyPoly;
    const basegfx::B2DHomMatrix& rTransformationMatrix = GetViewTransformation();
    aTransformedPoly.transform( rTransformationMatrix );
    return aTransformedPoly;
}

vcl::Region OutputDevice::LogicToPixel( const vcl::Region& rLogicRegion ) const
{

    if(!mbMap || rLogicRegion.IsNull() || rLogicRegion.IsEmpty())
    {
        return rLogicRegion;
    }

    vcl::Region aRegion;

    if(rLogicRegion.getB2DPolyPolygon())
    {
        aRegion = vcl::Region(LogicToPixel(*rLogicRegion.getB2DPolyPolygon()));
    }
    else if(rLogicRegion.getPolyPolygon())
    {
        aRegion = vcl::Region(LogicToPixel(*rLogicRegion.getPolyPolygon()));
    }
    else if(rLogicRegion.getRegionBand())
    {
        RectangleVector aRectangles;
        rLogicRegion.GetRegionRectangles(aRectangles);
        const RectangleVector& rRectangles(aRectangles); // needed to make the '!=' work

        // make reverse run to fill new region bottom-up, this will speed it up due to the used data structuring
        for(RectangleVector::const_reverse_iterator aRectIter(rRectangles.rbegin()); aRectIter != rRectangles.rend(); ++aRectIter)
        {
            aRegion.Union(LogicToPixel(*aRectIter));
        }
    }

    return aRegion;
}

Point OutputDevice::LogicToPixel( const Point& rLogicPt,
                                  const MapMode& rMapMode ) const
{

    if ( rMapMode.IsDefault() )
        return rLogicPt;

    // convert MapMode resolution and convert
    ImplMapRes          aMapRes;
    ImplCalcMapResolution(rMapMode, mnDPIX, mnDPIY, aMapRes);

    return Point( ImplLogicToPixel( rLogicPt.X() + aMapRes.mnMapOfsX, mnDPIX,
                                    aMapRes.mnMapScNumX, aMapRes.mnMapScDenomX )+mnOutOffOrigX,
                  ImplLogicToPixel( rLogicPt.Y() + aMapRes.mnMapOfsY, mnDPIY,
                                    aMapRes.mnMapScNumY, aMapRes.mnMapScDenomY )+mnOutOffOrigY );
}

Size OutputDevice::LogicToPixel( const Size& rLogicSize,
                                 const MapMode& rMapMode ) const
{

    if ( rMapMode.IsDefault() )
        return rLogicSize;

    // convert MapMode resolution and convert
    ImplMapRes          aMapRes;
    ImplCalcMapResolution(rMapMode, mnDPIX, mnDPIY, aMapRes);

    return Size( ImplLogicToPixel( rLogicSize.Width(), mnDPIX,
                                   aMapRes.mnMapScNumX, aMapRes.mnMapScDenomX ),
                 ImplLogicToPixel( rLogicSize.Height(), mnDPIY,
                                   aMapRes.mnMapScNumY, aMapRes.mnMapScDenomY ) );
}

tools::Rectangle OutputDevice::LogicToPixel( const tools::Rectangle& rLogicRect,
                                      const MapMode& rMapMode ) const
{
    // tdf#141761 see comments above, IsEmpty() removed
    if ( rMapMode.IsDefault() )
        return rLogicRect;

    // convert MapMode resolution and convert
    ImplMapRes          aMapRes;
    ImplCalcMapResolution(rMapMode, mnDPIX, mnDPIY, aMapRes);

    tools::Rectangle aRetval(
        ImplLogicToPixel( rLogicRect.Left() + aMapRes.mnMapOfsX, mnDPIX, aMapRes.mnMapScNumX, aMapRes.mnMapScDenomX )+mnOutOffOrigX,
        ImplLogicToPixel( rLogicRect.Top() + aMapRes.mnMapOfsY, mnDPIY, aMapRes.mnMapScNumY, aMapRes.mnMapScDenomY )+mnOutOffOrigY,
        rLogicRect.IsWidthEmpty() ? 0 : ImplLogicToPixel( rLogicRect.Right() + aMapRes.mnMapOfsX, mnDPIX,   aMapRes.mnMapScNumX, aMapRes.mnMapScDenomX )+mnOutOffOrigX,
        rLogicRect.IsHeightEmpty() ? 0 : ImplLogicToPixel( rLogicRect.Bottom() + aMapRes.mnMapOfsY, mnDPIY, aMapRes.mnMapScNumY, aMapRes.mnMapScDenomY )+mnOutOffOrigY );

    if(rLogicRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if(rLogicRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Polygon OutputDevice::LogicToPixel( const tools::Polygon& rLogicPoly,
                                           const MapMode& rMapMode ) const
{

    if ( rMapMode.IsDefault() )
        return rLogicPoly;

    // convert MapMode resolution and convert
    ImplMapRes          aMapRes;
    ImplCalcMapResolution(rMapMode, mnDPIX, mnDPIY, aMapRes);

    sal_uInt16  i;
    sal_uInt16  nPoints = rLogicPoly.GetSize();
    tools::Polygon aPoly( rLogicPoly );

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    for ( i = 0; i < nPoints; i++ )
    {
        const Point* pPt = &(pPointAry[i]);
        Point aPt;
        aPt.setX( ImplLogicToPixel( pPt->X() + aMapRes.mnMapOfsX, mnDPIX,
                                    aMapRes.mnMapScNumX, aMapRes.mnMapScDenomX )+mnOutOffOrigX );
        aPt.setY( ImplLogicToPixel( pPt->Y() + aMapRes.mnMapOfsY, mnDPIY,
                                    aMapRes.mnMapScNumY, aMapRes.mnMapScDenomY )+mnOutOffOrigY );
        aPoly[i] = aPt;
    }

    return aPoly;
}

basegfx::B2DPolyPolygon OutputDevice::LogicToPixel( const basegfx::B2DPolyPolygon& rLogicPolyPoly,
                                                    const MapMode& rMapMode ) const
{
    basegfx::B2DPolyPolygon aTransformedPoly = rLogicPolyPoly;
    const basegfx::B2DHomMatrix& rTransformationMatrix = GetViewTransformation( rMapMode );
    aTransformedPoly.transform( rTransformationMatrix );
    return aTransformedPoly;
}

Point OutputDevice::PixelToLogic( const Point& rDevicePt ) const
{

    if ( !mbMap )
        return rDevicePt;

    return Point( ImplPixelToLogic( rDevicePt.X(), mnDPIX,
                                    maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX ) - maMapRes.mnMapOfsX - mnOutOffLogicX,
                  ImplPixelToLogic( rDevicePt.Y(), mnDPIY,
                                    maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY ) - maMapRes.mnMapOfsY - mnOutOffLogicY );
}

Size OutputDevice::PixelToLogic( const Size& rDeviceSize ) const
{

    if ( !mbMap )
        return rDeviceSize;

    return Size( ImplPixelToLogic( rDeviceSize.Width(), mnDPIX,
                                   maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX ),
                 ImplPixelToLogic( rDeviceSize.Height(), mnDPIY,
                                   maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY ) );
}

tools::Rectangle OutputDevice::PixelToLogic( const tools::Rectangle& rDeviceRect ) const
{
    // tdf#141761 see comments above, IsEmpty() removed
    if ( !mbMap )
        return rDeviceRect;

    tools::Rectangle aRetval(
        ImplPixelToLogic( rDeviceRect.Left(), mnDPIX, maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX ) - maMapRes.mnMapOfsX - mnOutOffLogicX,
        ImplPixelToLogic( rDeviceRect.Top(), mnDPIY, maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY ) - maMapRes.mnMapOfsY - mnOutOffLogicY,
        rDeviceRect.IsWidthEmpty() ? 0 : ImplPixelToLogic( rDeviceRect.Right(), mnDPIX, maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX ) - maMapRes.mnMapOfsX - mnOutOffLogicX,
        rDeviceRect.IsHeightEmpty() ? 0 : ImplPixelToLogic( rDeviceRect.Bottom(), mnDPIY, maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY ) - maMapRes.mnMapOfsY - mnOutOffLogicY );

    if(rDeviceRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if(rDeviceRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Polygon OutputDevice::PixelToLogic( const tools::Polygon& rDevicePoly ) const
{

    if ( !mbMap )
        return rDevicePoly;

    sal_uInt16  i;
    sal_uInt16  nPoints = rDevicePoly.GetSize();
    tools::Polygon aPoly( rDevicePoly );

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    for ( i = 0; i < nPoints; i++ )
    {
        const Point* pPt = &(pPointAry[i]);
        Point aPt;
        aPt.setX( ImplPixelToLogic( pPt->X(), mnDPIX,
                                    maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX ) - maMapRes.mnMapOfsX - mnOutOffLogicX );
        aPt.setY( ImplPixelToLogic( pPt->Y(), mnDPIY,
                                    maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY ) - maMapRes.mnMapOfsY - mnOutOffLogicY );
        aPoly[i] = aPt;
    }

    return aPoly;
}

tools::PolyPolygon OutputDevice::PixelToLogic( const tools::PolyPolygon& rDevicePolyPoly ) const
{

    if ( !mbMap )
        return rDevicePolyPoly;

    tools::PolyPolygon aPolyPoly( rDevicePolyPoly );
    sal_uInt16      nPoly = aPolyPoly.Count();
    for( sal_uInt16 i = 0; i < nPoly; i++ )
    {
        tools::Polygon& rPoly = aPolyPoly[i];
        rPoly = PixelToLogic( rPoly );
    }
    return aPolyPoly;
}

basegfx::B2DPolyPolygon OutputDevice::PixelToLogic( const basegfx::B2DPolyPolygon& rPixelPolyPoly ) const
{
    basegfx::B2DPolyPolygon aTransformedPoly = rPixelPolyPoly;
    const basegfx::B2DHomMatrix& rTransformationMatrix = GetInverseViewTransformation();
    aTransformedPoly.transform( rTransformationMatrix );
    return aTransformedPoly;
}

vcl::Region OutputDevice::PixelToLogic( const vcl::Region& rDeviceRegion ) const
{

    if(!mbMap || rDeviceRegion.IsNull() || rDeviceRegion.IsEmpty())
    {
        return rDeviceRegion;
    }

    vcl::Region aRegion;

    if(rDeviceRegion.getB2DPolyPolygon())
    {
        aRegion = vcl::Region(PixelToLogic(*rDeviceRegion.getB2DPolyPolygon()));
    }
    else if(rDeviceRegion.getPolyPolygon())
    {
        aRegion = vcl::Region(PixelToLogic(*rDeviceRegion.getPolyPolygon()));
    }
    else if(rDeviceRegion.getRegionBand())
    {
        RectangleVector aRectangles;
        rDeviceRegion.GetRegionRectangles(aRectangles);
        const RectangleVector& rRectangles(aRectangles); // needed to make the '!=' work

        // make reverse run to fill new region bottom-up, this will speed it up due to the used data structuring
        for(RectangleVector::const_reverse_iterator aRectIter(rRectangles.rbegin()); aRectIter != rRectangles.rend(); ++aRectIter)
        {
            aRegion.Union(PixelToLogic(*aRectIter));
        }
    }

    return aRegion;
}

Point OutputDevice::PixelToLogic( const Point& rDevicePt,
                                  const MapMode& rMapMode ) const
{

    // calculate nothing if default-MapMode
    if ( rMapMode.IsDefault() )
        return rDevicePt;

    // calculate MapMode-resolution and convert
    ImplMapRes          aMapRes;
    ImplCalcMapResolution(rMapMode, mnDPIX, mnDPIY, aMapRes);

    return Point( ImplPixelToLogic( rDevicePt.X(), mnDPIX,
                                    aMapRes.mnMapScNumX, aMapRes.mnMapScDenomX ) - aMapRes.mnMapOfsX - mnOutOffLogicX,
                  ImplPixelToLogic( rDevicePt.Y(), mnDPIY,
                                    aMapRes.mnMapScNumY, aMapRes.mnMapScDenomY ) - aMapRes.mnMapOfsY - mnOutOffLogicY );
}

Size OutputDevice::PixelToLogic( const Size& rDeviceSize,
                                 const MapMode& rMapMode ) const
{

    // calculate nothing if default-MapMode
    if ( rMapMode.IsDefault() )
        return rDeviceSize;

    // calculate MapMode-resolution and convert
    ImplMapRes          aMapRes;
    ImplCalcMapResolution(rMapMode, mnDPIX, mnDPIY, aMapRes);

    return Size( ImplPixelToLogic( rDeviceSize.Width(), mnDPIX,
                                   aMapRes.mnMapScNumX, aMapRes.mnMapScDenomX ),
                 ImplPixelToLogic( rDeviceSize.Height(), mnDPIY,
                                   aMapRes.mnMapScNumY, aMapRes.mnMapScDenomY ) );
}

tools::Rectangle OutputDevice::PixelToLogic( const tools::Rectangle& rDeviceRect,
                                      const MapMode& rMapMode ) const
{
    // calculate nothing if default-MapMode
    // tdf#141761 see comments above, IsEmpty() removed
    if ( rMapMode.IsDefault() )
        return rDeviceRect;

    // calculate MapMode-resolution and convert
    ImplMapRes          aMapRes;
    ImplCalcMapResolution(rMapMode, mnDPIX, mnDPIY, aMapRes);

    tools::Rectangle aRetval(
        ImplPixelToLogic( rDeviceRect.Left(), mnDPIX, aMapRes.mnMapScNumX, aMapRes.mnMapScDenomX ) - aMapRes.mnMapOfsX - mnOutOffLogicX,
        ImplPixelToLogic( rDeviceRect.Top(), mnDPIY, aMapRes.mnMapScNumY, aMapRes.mnMapScDenomY ) - aMapRes.mnMapOfsY - mnOutOffLogicY,
        rDeviceRect.IsWidthEmpty() ? 0 : ImplPixelToLogic( rDeviceRect.Right(), mnDPIX, aMapRes.mnMapScNumX, aMapRes.mnMapScDenomX ) - aMapRes.mnMapOfsX - mnOutOffLogicX,
        rDeviceRect.IsHeightEmpty() ? 0 : ImplPixelToLogic( rDeviceRect.Bottom(), mnDPIY, aMapRes.mnMapScNumY, aMapRes.mnMapScDenomY ) - aMapRes.mnMapOfsY - mnOutOffLogicY );

    if(rDeviceRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if(rDeviceRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Polygon OutputDevice::PixelToLogic( const tools::Polygon& rDevicePoly,
                                           const MapMode& rMapMode ) const
{

    // calculate nothing if default-MapMode
    if ( rMapMode.IsDefault() )
        return rDevicePoly;

    // calculate MapMode-resolution and convert
    ImplMapRes          aMapRes;
    ImplCalcMapResolution(rMapMode, mnDPIX, mnDPIY, aMapRes);

    sal_uInt16  i;
    sal_uInt16  nPoints = rDevicePoly.GetSize();
    tools::Polygon aPoly( rDevicePoly );

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    for ( i = 0; i < nPoints; i++ )
    {
        const Point* pPt = &(pPointAry[i]);
        Point aPt;
        aPt.setX( ImplPixelToLogic( pPt->X(), mnDPIX,
                                    aMapRes.mnMapScNumX, aMapRes.mnMapScDenomX ) - aMapRes.mnMapOfsX - mnOutOffLogicX );
        aPt.setY( ImplPixelToLogic( pPt->Y(), mnDPIY,
                                    aMapRes.mnMapScNumY, aMapRes.mnMapScDenomY ) - aMapRes.mnMapOfsY - mnOutOffLogicY );
        aPoly[i] = aPt;
    }

    return aPoly;
}

basegfx::B2DPolygon OutputDevice::PixelToLogic( const basegfx::B2DPolygon& rPixelPoly,
                                                const MapMode& rMapMode ) const
{
    basegfx::B2DPolygon aTransformedPoly = rPixelPoly;
    const basegfx::B2DHomMatrix& rTransformationMatrix = GetInverseViewTransformation( rMapMode );
    aTransformedPoly.transform( rTransformationMatrix );
    return aTransformedPoly;
}

basegfx::B2DPolyPolygon OutputDevice::PixelToLogic( const basegfx::B2DPolyPolygon& rPixelPolyPoly,
                                                    const MapMode& rMapMode ) const
{
    basegfx::B2DPolyPolygon aTransformedPoly = rPixelPolyPoly;
    const basegfx::B2DHomMatrix& rTransformationMatrix = GetInverseViewTransformation( rMapMode );
    aTransformedPoly.transform( rTransformationMatrix );
    return aTransformedPoly;
}

#define ENTER1( rSource, pMapModeSource, pMapModeDest )                 \
    if ( !pMapModeSource )                                              \
        pMapModeSource = &maMapMode;                                    \
    if ( !pMapModeDest )                                                \
        pMapModeDest = &maMapMode;                                      \
    if ( *pMapModeSource == *pMapModeDest )                             \
        return rSource;                                                 \
                                                                        \
    ImplMapRes aMapResSource;                                           \
    aMapResSource.mnMapOfsX          = 0;                               \
    aMapResSource.mnMapOfsY          = 0;                               \
    aMapResSource.mnMapScNumX        = 1;                               \
    aMapResSource.mnMapScNumY        = 1;                               \
    aMapResSource.mnMapScDenomX      = 1;                               \
    aMapResSource.mnMapScDenomY      = 1;                               \
    ImplMapRes aMapResDest(aMapResSource);                              \
                                                                        \
    if ( !mbMap || pMapModeSource != &maMapMode )                       \
    {                                                                   \
        if ( pMapModeSource->GetMapUnit() == MapUnit::MapRelative )             \
            aMapResSource = maMapRes;                                   \
        ImplCalcMapResolution( *pMapModeSource,                         \
                               mnDPIX, mnDPIY, aMapResSource );         \
    }                                                                   \
    else                                                                \
        aMapResSource = maMapRes;                                       \
    if ( !mbMap || pMapModeDest != &maMapMode )                         \
    {                                                                   \
        if ( pMapModeDest->GetMapUnit() == MapUnit::MapRelative )               \
            aMapResDest = maMapRes;                                     \
        ImplCalcMapResolution( *pMapModeDest,                           \
                               mnDPIX, mnDPIY, aMapResDest );           \
    }                                                                   \
    else                                                                \
        aMapResDest = maMapRes

static void verifyUnitSourceDest( MapUnit eUnitSource, MapUnit eUnitDest )
{
    DBG_ASSERT( eUnitSource != MapUnit::MapSysFont
                && eUnitSource != MapUnit::MapAppFont
                && eUnitSource != MapUnit::MapRelative,
                "Source MapUnit is not permitted" );
    DBG_ASSERT( eUnitDest != MapUnit::MapSysFont
                && eUnitDest != MapUnit::MapAppFont
                && eUnitDest != MapUnit::MapRelative,
                "Destination MapUnit is not permitted" );
}

#define ENTER3( eUnitSource, eUnitDest )                                \
    tools::Long nNumerator      = 1;       \
    tools::Long nDenominator    = 1;       \
    SAL_WARN_IF( eUnitSource > s_MaxValidUnit, "vcl.gdi", "Invalid source map unit");    \
    SAL_WARN_IF( eUnitDest > s_MaxValidUnit, "vcl.gdi", "Invalid destination map unit"); \
    if( (eUnitSource <= s_MaxValidUnit) && (eUnitDest <= s_MaxValidUnit) )  \
    {   \
        nNumerator   = aImplNumeratorAry[eUnitSource] *             \
                           aImplDenominatorAry[eUnitDest];              \
        nDenominator     = aImplNumeratorAry[eUnitDest] *               \
                           aImplDenominatorAry[eUnitSource];            \
    } \
    if ( eUnitSource == MapUnit::MapPixel )                                     \
        nDenominator *= 72;                                             \
    else if( eUnitDest == MapUnit::MapPixel )                                   \
        nNumerator *= 72

#define ENTER4( rMapModeSource, rMapModeDest )                          \
    ImplMapRes aMapResSource;                                           \
    aMapResSource.mnMapOfsX          = 0;                               \
    aMapResSource.mnMapOfsY          = 0;                               \
    aMapResSource.mnMapScNumX        = 1;                               \
    aMapResSource.mnMapScNumY        = 1;                               \
    aMapResSource.mnMapScDenomX      = 1;                               \
    aMapResSource.mnMapScDenomY      = 1;                               \
    ImplMapRes aMapResDest(aMapResSource);                              \
                                                                        \
    ImplCalcMapResolution( rMapModeSource, 72, 72, aMapResSource );     \
    ImplCalcMapResolution( rMapModeDest, 72, 72, aMapResDest )

// return (n1 * n2 * n3) / (n4 * n5)
static tools::Long fn5( const tools::Long n1,
                 const tools::Long n2,
                 const tools::Long n3,
                 const tools::Long n4,
                 const tools::Long n5 )
{
    if ( n1 == 0 || n2 == 0 || n3 == 0 || n4 == 0 || n5 == 0 )
        return 0;
    if (std::numeric_limits<tools::Long>::max() / std::abs(n2) < std::abs(n3))
    {
        // a6 is skipped
        BigInt a7 = n2;
        a7 *= n3;
        a7 *= n1;

        if (std::numeric_limits<tools::Long>::max() / std::abs(n4) < std::abs(n5))
        {
            BigInt a8 = n4;
            a8 *= n5;

            BigInt a9 = a8;
            a9 /= 2;
            if ( a7.IsNeg() )
                a7 -= a9;
            else
                a7 += a9;

            a7 /= a8;
        } // of if
        else
        {
            tools::Long n8 = n4 * n5;

            if ( a7.IsNeg() )
                a7 -= n8 / 2;
            else
                a7 += n8 / 2;

            a7 /= n8;
        } // of else
        return static_cast<tools::Long>(a7);
    } // of if
    else
    {
        tools::Long n6 = n2 * n3;

        if (std::numeric_limits<tools::Long>::max() / std::abs(n1) < std::abs(n6))
        {
            BigInt a7 = n1;
            a7 *= n6;

            if (std::numeric_limits<tools::Long>::max() / std::abs(n4) < std::abs(n5))
            {
                BigInt a8 = n4;
                a8 *= n5;

                BigInt a9 = a8;
                a9 /= 2;
                if ( a7.IsNeg() )
                    a7 -= a9;
                else
                    a7 += a9;

                a7 /= a8;
            } // of if
            else
            {
                tools::Long n8 = n4 * n5;

                if ( a7.IsNeg() )
                    a7 -= n8 / 2;
                else
                    a7 += n8 / 2;

                a7 /= n8;
            } // of else
            return static_cast<tools::Long>(a7);
        } // of if
        else
        {
            tools::Long n7 = n1 * n6;

            if (std::numeric_limits<tools::Long>::max() / std::abs(n4) < std::abs(n5))
            {
                BigInt a7 = n7;
                BigInt a8 = n4;
                a8 *= n5;

                BigInt a9 = a8;
                a9 /= 2;
                if ( a7.IsNeg() )
                    a7 -= a9;
                else
                    a7 += a9;

                a7 /= a8;
                return static_cast<tools::Long>(a7);
            } // of if
            else
            {
                const tools::Long n8 = n4 * n5;
                const tools::Long n8_2 = n8 / 2;

                if( n7 < 0 )
                {
                    if ((n7 - std::numeric_limits<tools::Long>::min()) >= n8_2)
                        n7 -= n8_2;
                }
                else if ((std::numeric_limits<tools::Long>::max() - n7) >= n8_2)
                    n7 += n8_2;

                return n7 / n8;
            } // of else
        } // of else
    } // of else
}

// return (n1 * n2) / n3
static tools::Long fn3( const tools::Long n1, const tools::Long n2, const tools::Long n3 )
{
    if ( n1 == 0 || n2 == 0 || n3 == 0 )
        return 0;
    if (std::numeric_limits<tools::Long>::max() / std::abs(n1) < std::abs(n2))
    {
        BigInt a4 = n1;
        a4 *= n2;

        if ( a4.IsNeg() )
            a4 -= n3 / 2;
        else
            a4 += n3 / 2;

        a4 /= n3;
        return static_cast<tools::Long>(a4);
    } // of if
    else
    {
        tools::Long        n4 = n1 * n2;
        const tools::Long  n3_2 = n3 / 2;

        if( n4 < 0 )
        {
            if ((n4 - std::numeric_limits<tools::Long>::min()) >= n3_2)
                n4 -= n3_2;
        }
        else if ((std::numeric_limits<tools::Long>::max() - n4) >= n3_2)
            n4 += n3_2;

        return n4 / n3;
    } // of else
}

Point OutputDevice::LogicToLogic( const Point& rPtSource,
                                  const MapMode* pMapModeSource,
                                  const MapMode* pMapModeDest ) const
{
    ENTER1( rPtSource, pMapModeSource, pMapModeDest );

    return Point( fn5( rPtSource.X() + aMapResSource.mnMapOfsX,
                       aMapResSource.mnMapScNumX, aMapResDest.mnMapScDenomX,
                       aMapResSource.mnMapScDenomX, aMapResDest.mnMapScNumX ) -
                  aMapResDest.mnMapOfsX,
                  fn5( rPtSource.Y() + aMapResSource.mnMapOfsY,
                       aMapResSource.mnMapScNumY, aMapResDest.mnMapScDenomY,
                       aMapResSource.mnMapScDenomY, aMapResDest.mnMapScNumY ) -
                  aMapResDest.mnMapOfsY );
}

Size OutputDevice::LogicToLogic( const Size& rSzSource,
                                 const MapMode* pMapModeSource,
                                 const MapMode* pMapModeDest ) const
{
    ENTER1( rSzSource, pMapModeSource, pMapModeDest );

    return Size( fn5( rSzSource.Width(),
                      aMapResSource.mnMapScNumX, aMapResDest.mnMapScDenomX,
                      aMapResSource.mnMapScDenomX, aMapResDest.mnMapScNumX ),
                 fn5( rSzSource.Height(),
                      aMapResSource.mnMapScNumY, aMapResDest.mnMapScDenomY,
                      aMapResSource.mnMapScDenomY, aMapResDest.mnMapScNumY ) );
}

tools::Rectangle OutputDevice::LogicToLogic( const tools::Rectangle& rRectSource,
                                      const MapMode* pMapModeSource,
                                      const MapMode* pMapModeDest ) const
{
    ENTER1( rRectSource, pMapModeSource, pMapModeDest );

    return tools::Rectangle( fn5( rRectSource.Left() + aMapResSource.mnMapOfsX,
                           aMapResSource.mnMapScNumX, aMapResDest.mnMapScDenomX,
                           aMapResSource.mnMapScDenomX, aMapResDest.mnMapScNumX ) -
                      aMapResDest.mnMapOfsX,
                      fn5( rRectSource.Top() + aMapResSource.mnMapOfsY,
                           aMapResSource.mnMapScNumY, aMapResDest.mnMapScDenomY,
                           aMapResSource.mnMapScDenomY, aMapResDest.mnMapScNumY ) -
                      aMapResDest.mnMapOfsY,
                      fn5( rRectSource.Right() + aMapResSource.mnMapOfsX,
                           aMapResSource.mnMapScNumX, aMapResDest.mnMapScDenomX,
                           aMapResSource.mnMapScDenomX, aMapResDest.mnMapScNumX ) -
                      aMapResDest.mnMapOfsX,
                      fn5( rRectSource.Bottom() + aMapResSource.mnMapOfsY,
                           aMapResSource.mnMapScNumY, aMapResDest.mnMapScDenomY,
                           aMapResSource.mnMapScDenomY, aMapResDest.mnMapScNumY ) -
                      aMapResDest.mnMapOfsY );
}

Point OutputDevice::LogicToLogic( const Point& rPtSource,
                                  const MapMode& rMapModeSource,
                                  const MapMode& rMapModeDest )
{
    if ( rMapModeSource == rMapModeDest )
        return rPtSource;

    MapUnit eUnitSource = rMapModeSource.GetMapUnit();
    MapUnit eUnitDest   = rMapModeDest.GetMapUnit();
    verifyUnitSourceDest( eUnitSource, eUnitDest );

    if (rMapModeSource.IsSimple() && rMapModeDest.IsSimple())
    {
        ENTER3( eUnitSource, eUnitDest );

        return Point( fn3( rPtSource.X(), nNumerator, nDenominator ),
                      fn3( rPtSource.Y(), nNumerator, nDenominator ) );
    }
    else
    {
        ENTER4( rMapModeSource, rMapModeDest );

        return Point( fn5( rPtSource.X() + aMapResSource.mnMapOfsX,
                           aMapResSource.mnMapScNumX, aMapResDest.mnMapScDenomX,
                           aMapResSource.mnMapScDenomX, aMapResDest.mnMapScNumX ) -
                      aMapResDest.mnMapOfsX,
                      fn5( rPtSource.Y() + aMapResSource.mnMapOfsY,
                           aMapResSource.mnMapScNumY, aMapResDest.mnMapScDenomY,
                           aMapResSource.mnMapScDenomY, aMapResDest.mnMapScNumY ) -
                      aMapResDest.mnMapOfsY );
    }
}

Size OutputDevice::LogicToLogic( const Size& rSzSource,
                                 const MapMode& rMapModeSource,
                                 const MapMode& rMapModeDest )
{
    if ( rMapModeSource == rMapModeDest )
        return rSzSource;

    MapUnit eUnitSource = rMapModeSource.GetMapUnit();
    MapUnit eUnitDest   = rMapModeDest.GetMapUnit();
    verifyUnitSourceDest( eUnitSource, eUnitDest );

    if (rMapModeSource.IsSimple() && rMapModeDest.IsSimple())
    {
        ENTER3( eUnitSource, eUnitDest );

        return Size( fn3( rSzSource.Width(),  nNumerator, nDenominator ),
                     fn3( rSzSource.Height(), nNumerator, nDenominator ) );
    }
    else
    {
        ENTER4( rMapModeSource, rMapModeDest );

        return Size( fn5( rSzSource.Width(),
                          aMapResSource.mnMapScNumX, aMapResDest.mnMapScDenomX,
                          aMapResSource.mnMapScDenomX, aMapResDest.mnMapScNumX ),
                     fn5( rSzSource.Height(),
                          aMapResSource.mnMapScNumY, aMapResDest.mnMapScDenomY,
                          aMapResSource.mnMapScDenomY, aMapResDest.mnMapScNumY ) );
    }
}

basegfx::B2DPolygon OutputDevice::LogicToLogic( const basegfx::B2DPolygon& rPolySource,
                                                const MapMode& rMapModeSource,
                                                const MapMode& rMapModeDest )
{
    if(rMapModeSource == rMapModeDest)
    {
        return rPolySource;
    }

    const basegfx::B2DHomMatrix aTransform(LogicToLogic(rMapModeSource, rMapModeDest));
    basegfx::B2DPolygon aPoly(rPolySource);

    aPoly.transform(aTransform);
    return aPoly;
}

basegfx::B2DHomMatrix OutputDevice::LogicToLogic(const MapMode& rMapModeSource, const MapMode& rMapModeDest)
{
    basegfx::B2DHomMatrix aTransform;

    if(rMapModeSource == rMapModeDest)
    {
        return aTransform;
    }

    MapUnit eUnitSource = rMapModeSource.GetMapUnit();
    MapUnit eUnitDest   = rMapModeDest.GetMapUnit();
    verifyUnitSourceDest(eUnitSource, eUnitDest);

    if (rMapModeSource.IsSimple() && rMapModeDest.IsSimple())
    {
        ENTER3(eUnitSource, eUnitDest);

        const double fScaleFactor(static_cast<double>(nNumerator) / static_cast<double>(nDenominator));
        aTransform.set(0, 0, fScaleFactor);
        aTransform.set(1, 1, fScaleFactor);
    }
    else
    {
        ENTER4(rMapModeSource, rMapModeDest);

        const double fScaleFactorX((double(aMapResSource.mnMapScNumX) * double(aMapResDest.mnMapScDenomX)) / (double(aMapResSource.mnMapScDenomX) * double(aMapResDest.mnMapScNumX)));
        const double fScaleFactorY((double(aMapResSource.mnMapScNumY) * double(aMapResDest.mnMapScDenomY)) / (double(aMapResSource.mnMapScDenomY) * double(aMapResDest.mnMapScNumY)));
        const double fZeroPointX(double(aMapResSource.mnMapOfsX) * fScaleFactorX - double(aMapResDest.mnMapOfsX));
        const double fZeroPointY(double(aMapResSource.mnMapOfsY) * fScaleFactorY - double(aMapResDest.mnMapOfsY));

        aTransform.set(0, 0, fScaleFactorX);
        aTransform.set(1, 1, fScaleFactorY);
        aTransform.set(0, 2, fZeroPointX);
        aTransform.set(1, 2, fZeroPointY);
    }

    return aTransform;
}

tools::Rectangle OutputDevice::LogicToLogic( const tools::Rectangle& rRectSource,
                                      const MapMode& rMapModeSource,
                                      const MapMode& rMapModeDest )
{
    if ( rMapModeSource == rMapModeDest )
        return rRectSource;

    MapUnit eUnitSource = rMapModeSource.GetMapUnit();
    MapUnit eUnitDest   = rMapModeDest.GetMapUnit();
    verifyUnitSourceDest( eUnitSource, eUnitDest );

    tools::Rectangle aRetval;

    if (rMapModeSource.IsSimple() && rMapModeDest.IsSimple())
    {
        ENTER3( eUnitSource, eUnitDest );

        auto left = fn3(rRectSource.Left(), nNumerator, nDenominator);
        auto top = fn3(rRectSource.Top(), nNumerator, nDenominator);

        // tdf#141761 see comments above, IsEmpty() removed
        auto right = rRectSource.IsWidthEmpty() ? 0 : fn3(rRectSource.Right(), nNumerator, nDenominator);
        auto bottom = rRectSource.IsHeightEmpty() ? 0 : fn3(rRectSource.Bottom(), nNumerator, nDenominator);

        aRetval = tools::Rectangle(left, top, right, bottom);
    }
    else
    {
        ENTER4( rMapModeSource, rMapModeDest );

        auto left = fn5( rRectSource.Left() + aMapResSource.mnMapOfsX,
                               aMapResSource.mnMapScNumX, aMapResDest.mnMapScDenomX,
                               aMapResSource.mnMapScDenomX, aMapResDest.mnMapScNumX ) -
                          aMapResDest.mnMapOfsX;
        auto top = fn5( rRectSource.Top() + aMapResSource.mnMapOfsY,
                               aMapResSource.mnMapScNumY, aMapResDest.mnMapScDenomY,
                               aMapResSource.mnMapScDenomY, aMapResDest.mnMapScNumY ) -
                          aMapResDest.mnMapOfsY;

        // tdf#141761 see comments above, IsEmpty() removed
        auto right = rRectSource.IsWidthEmpty() ? 0 : fn5( rRectSource.Right() + aMapResSource.mnMapOfsX,
                               aMapResSource.mnMapScNumX, aMapResDest.mnMapScDenomX,
                               aMapResSource.mnMapScDenomX, aMapResDest.mnMapScNumX ) -
                          aMapResDest.mnMapOfsX;
        auto bottom = rRectSource.IsHeightEmpty() ? 0 : fn5( rRectSource.Bottom() + aMapResSource.mnMapOfsY,
                               aMapResSource.mnMapScNumY, aMapResDest.mnMapScDenomY,
                               aMapResSource.mnMapScDenomY, aMapResDest.mnMapScNumY ) -
                          aMapResDest.mnMapOfsY;

        aRetval = tools::Rectangle(left, top, right, bottom);
    }

    if(rRectSource.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if(rRectSource.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Long OutputDevice::LogicToLogic( tools::Long nLongSource,
                                 MapUnit eUnitSource, MapUnit eUnitDest )
{
    if ( eUnitSource == eUnitDest )
        return nLongSource;

    verifyUnitSourceDest( eUnitSource, eUnitDest );
    ENTER3( eUnitSource, eUnitDest );

    return fn3( nLongSource, nNumerator, nDenominator );
}

void OutputDevice::SetPixelOffset( const Size& rOffset )
{
    mnOutOffOrigX  = rOffset.Width();
    mnOutOffOrigY  = rOffset.Height();

    mnOutOffLogicX = ImplPixelToLogic( mnOutOffOrigX, mnDPIX,
                                       maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX );
    mnOutOffLogicY = ImplPixelToLogic( mnOutOffOrigY, mnDPIY,
                                       maMapRes.mnMapScNumY, maMapRes.mnMapScDenomY );

    if( mpAlphaVDev )
        mpAlphaVDev->SetPixelOffset( rOffset );
}


DeviceCoordinate OutputDevice::LogicWidthToDeviceCoordinate( tools::Long nWidth ) const
{
    if ( !mbMap )
        return static_cast<DeviceCoordinate>(nWidth);

#if VCL_FLOAT_DEVICE_PIXEL
    return (double)nWidth * maMapRes.mfScaleX * mnDPIX;
#else

    return ImplLogicToPixel(nWidth, mnDPIX, maMapRes.mnMapScNumX, maMapRes.mnMapScDenomX);
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
