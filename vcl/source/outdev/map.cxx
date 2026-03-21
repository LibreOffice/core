/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>

#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/bigint.hxx>
#include <tools/debug.hxx>

#include <vcl/cursor.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wrkwin.hxx>

#include <ImplOutDevData.hxx>
#include <svdata.hxx>
#include <window.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <tools/UnitConversion.hxx>

static auto setMapRes(ImplMapRes& rMapRes, const o3tl::Length eUnit)
{
    const auto [nNum, nDen] = o3tl::getConversionMulDiv(eUnit, o3tl::Length::in);
    rMapRes.mfMapScX = rMapRes.mfMapScY = double(nNum) / nDen;
};

static void ImplCalcMapResolution( const MapMode& rMapMode,
                                   tools::Long nDPIX, tools::Long nDPIY, ImplMapRes& rMapRes )
{
    switch ( rMapMode.GetMapUnit() )
    {
        case MapUnit::MapRelative:
            break;
        case MapUnit::Map100thMM:
            setMapRes(rMapRes, o3tl::Length::mm100);
            break;
        case MapUnit::Map10thMM:
            setMapRes(rMapRes, o3tl::Length::mm10);
            break;
        case MapUnit::MapMM:
            setMapRes(rMapRes, o3tl::Length::mm);
            break;
        case MapUnit::MapCM:
            setMapRes(rMapRes, o3tl::Length::cm);
            break;
        case MapUnit::Map1000thInch:
            setMapRes(rMapRes, o3tl::Length::in1000);
            break;
        case MapUnit::Map100thInch:
            setMapRes(rMapRes, o3tl::Length::in100);
            break;
        case MapUnit::Map10thInch:
            setMapRes(rMapRes, o3tl::Length::in10);
            break;
        case MapUnit::MapInch:
            setMapRes(rMapRes, o3tl::Length::in);
            break;
        case MapUnit::MapPoint:
            setMapRes(rMapRes, o3tl::Length::pt);
            break;
        case MapUnit::MapTwip:
            setMapRes(rMapRes, o3tl::Length::twip);
            break;
        case MapUnit::MapPixel:
            rMapRes.mfMapScX   = 1.0 / nDPIX;
            rMapRes.mfMapScY   = 1.0 / nDPIY;
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
            rMapRes.mfMapScX   = double(pSVData->maGDIData.mnAppFontX) / (nDPIX * 40);
            rMapRes.mfMapScY   = double(pSVData->maGDIData.mnAppFontY) / (nDPIY * 80);
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
        auto funcCalcOffset = [](const Fraction& rScale, tools::Long& rnMapOffset, tools::Long nOrigin)
        {
            auto nNumerator = rScale.GetNumerator();
            assert(nNumerator != 0);

            BigInt aX( rnMapOffset );
            aX *= BigInt( rScale.GetDenominator() );
            if ( rnMapOffset >= 0 )
            {
                if (nNumerator >= 0)
                    aX += BigInt(nNumerator / 2);
                else
                    aX -= BigInt((nNumerator + 1) / 2);
            }
            else
            {
                if (nNumerator >= 0 )
                    aX -= BigInt((nNumerator - 1) / 2);
                else
                    aX += BigInt(nNumerator / 2);
            }
            aX /= BigInt(nNumerator);
            rnMapOffset = static_cast<tools::Long>(aX) + nOrigin;
        };

        funcCalcOffset(aScaleX, rMapRes.mnMapOfsX, aOrigin.X());
        funcCalcOffset(aScaleY, rMapRes.mnMapOfsY, aOrigin.Y());
    }

    // calculate scaling factor according to MapMode
    // aTemp? = rMapRes.mnMapSc? * aScale?
    rMapRes.mfMapScX = double(aScaleX) * rMapRes.mfMapScX;
    rMapRes.mfMapScY = double(aScaleY) * rMapRes.mfMapScY;
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

static tools::Long ImplLogicToPixel(tools::Long n, tools::Long nDPI, double fMap)
{
    assert(nDPI > 0);
    assert(fMap >= 0);
    double nRes = n * fMap * nDPI;
    //detect overflows
    assert(std::abs(nRes) < static_cast<double>(std::numeric_limits<tools::Long>::max()));
    return std::llround(nRes);
}

static double ImplLogicToSubPixel(tools::Long n, tools::Long nDPI, double fMap)
{
    assert(nDPI > 0);
    assert(fMap != 0);
    return n * fMap * nDPI;
}

static tools::Long ImplSubPixelToLogic(double n, tools::Long nDPI, double fMap)
{
    assert(nDPI > 0);
    assert(fMap != 0);

    return std::llround(n / fMap / nDPI);
}

static tools::Long ImplPixelToLogic(tools::Long n, tools::Long nDPI, double fMap)
{
    assert(nDPI > 0);
    if (fMap == 0)
        return 0;

    return std::llround(n / fMap / nDPI);
}

static double ImplPixelToLogicDouble(double n, tools::Long nDPI, double fMap)
{
    assert(nDPI > 0);
    if (fMap == 0)
        return 0;
    return n / fMap / nDPI;
}

tools::Long OutputDevice::ImplLogicXToDevicePixel( tools::Long nX ) const
{
    if ( !mbMap )
        return nX+mnOutOffX;

    return ImplLogicToPixel( nX + maMapRes.mnMapOfsX, mnDPIX,
                             maMapRes.mfMapScX )+mnOutOffX+mnOutOffOrigX;
}

tools::Long OutputDevice::ImplLogicYToDevicePixel( tools::Long nY ) const
{
    if ( !mbMap )
        return nY+mnOutOffY;

    return ImplLogicToPixel( nY + maMapRes.mnMapOfsY, mnDPIY,
                             maMapRes.mfMapScY )+mnOutOffY+mnOutOffOrigY;
}

tools::Long OutputDevice::ImplLogicWidthToDevicePixel( tools::Long nWidth ) const
{
    if ( !mbMap )
        return nWidth;

    return ImplLogicToPixel(nWidth, mnDPIX, maMapRes.mfMapScX);
}

tools::Long OutputDevice::ImplLogicHeightToDevicePixel( tools::Long nHeight ) const
{
    if ( !mbMap )
        return nHeight;

    return ImplLogicToPixel(nHeight, mnDPIY, maMapRes.mfMapScY);
}

tools::Long OutputDevice::ImplDevicePixelToLogicWidth( tools::Long nWidth ) const
{
    if ( !mbMap )
        return nWidth;

    return ImplPixelToLogic(nWidth, mnDPIX, maMapRes.mfMapScX);
}

SAL_DLLPRIVATE double OutputDevice::ImplDevicePixelToLogicWidthDouble(double nWidth) const
{
    if (!mbMap)
        return nWidth;

    return ImplPixelToLogicDouble(nWidth, mnDPIX, maMapRes.mfMapScX);
}

tools::Long OutputDevice::ImplDevicePixelToLogicHeight( tools::Long nHeight ) const
{
    if ( !mbMap )
        return nHeight;

    return ImplPixelToLogic(nHeight, mnDPIY, maMapRes.mfMapScY);
}

double OutputDevice::ImplDevicePixelToLogicHeightDouble(double nHeight) const
{
    if (!mbMap)
        return nHeight;

    return ImplPixelToLogicDouble(nHeight, mnDPIY, maMapRes.mfMapScY);
}

Point OutputDevice::ImplLogicToDevicePixel( const Point& rLogicPt ) const
{
    if ( !mbMap )
        return Point( rLogicPt.X()+mnOutOffX, rLogicPt.Y()+mnOutOffY );

    return Point( ImplLogicToPixel( rLogicPt.X() + maMapRes.mnMapOfsX, mnDPIX,
                                    maMapRes.mfMapScX )+mnOutOffX+mnOutOffOrigX,
                  ImplLogicToPixel( rLogicPt.Y() + maMapRes.mnMapOfsY, mnDPIY,
                                    maMapRes.mfMapScY )+mnOutOffY+mnOutOffOrigY );
}

Size OutputDevice::ImplLogicToDevicePixel( const Size& rLogicSize ) const
{
    if ( !mbMap )
        return rLogicSize;

    return Size( ImplLogicToPixel( rLogicSize.Width(), mnDPIX,
                                   maMapRes.mfMapScX ),
                 ImplLogicToPixel( rLogicSize.Height(), mnDPIY,
                                   maMapRes.mfMapScY ) );
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
    // (2) Do it by hand here
    // I have tried (1) first, but test Test::test_rectangle() claims that for
    //  tools::Rectangle aRect(1, 1, 1, 1);
    //    tools::Long(1) == aRect.GetWidth()
    //    tools::Long(0) == aRect.getWidth()
    // (remember: this means Left == Right == 1 -> GetWidth => 1, getWidth == 0)
    // so indeed the 1's have to go uncommented/unchecked into the data body
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
            ImplLogicToPixel( rLogicRect.Left()+maMapRes.mnMapOfsX, mnDPIX, maMapRes.mfMapScX )+mnOutOffX+mnOutOffOrigX,
            ImplLogicToPixel( rLogicRect.Top()+maMapRes.mnMapOfsY, mnDPIY, maMapRes.mfMapScY )+mnOutOffY+mnOutOffOrigY,
            rLogicRect.IsWidthEmpty() ? 0 : ImplLogicToPixel( rLogicRect.Right()+maMapRes.mnMapOfsX, mnDPIX, maMapRes.mfMapScX )+mnOutOffX+mnOutOffOrigX,
            rLogicRect.IsHeightEmpty() ? 0 : ImplLogicToPixel( rLogicRect.Bottom()+maMapRes.mnMapOfsY, mnDPIY, maMapRes.mfMapScY )+mnOutOffY+mnOutOffOrigY );
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
                                        maMapRes.mfMapScX )+mnOutOffX+mnOutOffOrigX,
                      ImplLogicToPixel( rPt.Y()+maMapRes.mnMapOfsY, mnDPIY,
                                        maMapRes.mfMapScY )+mnOutOffY+mnOutOffOrigY);
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

basegfx::B2DPolygon OutputDevice::ImplLogicToDevicePixel(const basegfx::B2DPolygon& rLogicPoly) const
{
    if (!mbMap && !mnOutOffX && !mnOutOffY)
        return rLogicPoly;

    sal_uInt32 nPoints = rLogicPoly.count();
    basegfx::B2DPolygon aPoly(rLogicPoly);

    basegfx::B2DPoint aC1;
    basegfx::B2DPoint aC2;

    if (mbMap)
    {
        for (sal_uInt32 i = 0; i < nPoints; ++i)
        {
            const basegfx::B2DPoint& rPt = aPoly.getB2DPoint(i);
            basegfx::B2DPoint aPt(ImplLogicToPixel( rPt.getX()+maMapRes.mnMapOfsX, mnDPIX,
                                        maMapRes.mfMapScX )+mnOutOffX+mnOutOffOrigX,
                                  ImplLogicToPixel( rPt.getY()+maMapRes.mnMapOfsY, mnDPIY,
                                        maMapRes.mfMapScY )+mnOutOffY+mnOutOffOrigY);

            const bool bC1 = aPoly.isPrevControlPointUsed(i);
            if (bC1)
            {
                const basegfx::B2DPoint aB2DC1(aPoly.getPrevControlPoint(i));

                aC1 = basegfx::B2DPoint(ImplLogicToPixel( aB2DC1.getX()+maMapRes.mnMapOfsX, mnDPIX,
                                            maMapRes.mfMapScX )+mnOutOffX+mnOutOffOrigX,
                                        ImplLogicToPixel( aB2DC1.getY()+maMapRes.mnMapOfsY, mnDPIY,
                                            maMapRes.mfMapScY )+mnOutOffY+mnOutOffOrigY);
            }

            const bool bC2 = aPoly.isNextControlPointUsed(i);
            if (bC2)
            {
                const basegfx::B2DPoint aB2DC2(aPoly.getNextControlPoint(i));

                aC2 = basegfx::B2DPoint(ImplLogicToPixel( aB2DC2.getX()+maMapRes.mnMapOfsX, mnDPIX,
                                            maMapRes.mfMapScX )+mnOutOffX+mnOutOffOrigX,
                                        ImplLogicToPixel( aB2DC2.getY()+maMapRes.mnMapOfsY, mnDPIY,
                                            maMapRes.mfMapScY )+mnOutOffY+mnOutOffOrigY);
            }

            aPoly.setB2DPoint(i, aPt);

            if (bC1)
                aPoly.setPrevControlPoint(i, aC1);

            if (bC2)
                aPoly.setNextControlPoint(i, aC2);
        }
    }
    else
    {
        for (sal_uInt32 i = 0; i < nPoints; ++i)
        {
            const basegfx::B2DPoint& rPt = aPoly.getB2DPoint(i);
            basegfx::B2DPoint aPt(rPt.getX() + mnOutOffX, rPt.getY() + mnOutOffY);

            const bool bC1 = aPoly.isPrevControlPointUsed(i);
            if (bC1)
            {
                const basegfx::B2DPoint aB2DC1(aPoly.getPrevControlPoint(i));

                aC1 = basegfx::B2DPoint(aB2DC1.getX() + mnOutOffX, aB2DC1.getY() + mnOutOffY);
            }

            const bool bC2 = aPoly.isNextControlPointUsed(i);
            if (bC2)
            {
                const basegfx::B2DPoint aB2DC2(aPoly.getNextControlPoint(i));

                aC2 = basegfx::B2DPoint(aB2DC2.getX() + mnOutOffX, aB2DC2.getY() + mnOutOffY);
            }

            aPoly.setB2DPoint(i, aPt);

            if (bC1)
                aPoly.setPrevControlPoint(i, aC1);

            if (bC2)
                aPoly.setNextControlPoint(i, aC2);
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
            ImplPixelToLogic( rPixelRect.Left()-mnOutOffX-mnOutOffOrigX, mnDPIX, maMapRes.mfMapScX )-maMapRes.mnMapOfsX,
            ImplPixelToLogic( rPixelRect.Top()-mnOutOffY-mnOutOffOrigY, mnDPIY, maMapRes.mfMapScY )-maMapRes.mnMapOfsY,
            rPixelRect.IsWidthEmpty() ? 0 : ImplPixelToLogic( rPixelRect.Right()-mnOutOffX-mnOutOffOrigX, mnDPIX, maMapRes.mfMapScX )-maMapRes.mnMapOfsX,
            rPixelRect.IsHeightEmpty() ? 0 : ImplPixelToLogic( rPixelRect.Bottom()-mnOutOffY-mnOutOffOrigY, mnDPIY, maMapRes.mfMapScY )-maMapRes.mnMapOfsY );
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
            maMapRes.mfMapScX    = 1.0 / mnDPIX;
            maMapRes.mfMapScY    = 1.0 / mnDPIY;
            maMapRes.mnMapOfsX      = 0;
            maMapRes.mnMapOfsY      = 0;
        }

        // calculate new MapMode-resolution
        ImplCalcMapResolution(rNewMapMode, mnDPIX, mnDPIY, maMapRes);
    }

    // set new MapMode
    if (bRelMap)
    {
        maMapMode.SetScaleX(Fraction::MakeFraction(
            maMapMode.GetScaleX().GetNumerator(), rNewMapMode.GetScaleX().GetNumerator(),
            maMapMode.GetScaleX().GetDenominator(), rNewMapMode.GetScaleX().GetDenominator()));

        maMapMode.SetScaleY(Fraction::MakeFraction(
            maMapMode.GetScaleY().GetNumerator(), rNewMapMode.GetScaleY().GetNumerator(),
            maMapMode.GetScaleY().GetDenominator(), rNewMapMode.GetScaleY().GetDenominator()));

        maMapMode.SetOrigin(Point(maMapRes.mnMapOfsX, maMapRes.mnMapOfsY));
    }
    else
    {
        maMapMode = rNewMapMode;
    }

    // create new objects (clip region are not re-scaled)
    mbNewFont   = true;
    mbInitFont  = true;
    ImplInitMapModeObjects();

    // #106426# Adapt logical offset when changing mapmode
    mnOutOffLogicX = ImplPixelToLogic( mnOutOffOrigX, mnDPIX,
                                       maMapRes.mfMapScX );
    mnOutOffLogicY = ImplPixelToLogic( mnOutOffOrigY, mnDPIY,
                                       maMapRes.mfMapScY );

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
    Fraction aXF = Fraction::MakeFraction( rNewMapMode.GetScaleX().GetNumerator(),
                                     maMapMode.GetScaleX().GetDenominator(),
                                     rNewMapMode.GetScaleX().GetDenominator(),
                                     maMapMode.GetScaleX().GetNumerator() );
    Fraction aYF = Fraction::MakeFraction( rNewMapMode.GetScaleY().GetNumerator(),
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
            const auto eFrom = MapToO3tlLength(eOld, o3tl::Length::in);
            const auto eTo = MapToO3tlLength(eNew, o3tl::Length::in);
            const auto [mul, div] = o3tl::getConversionMulDiv(eFrom, eTo);
            Fraction aF(div, mul);

            // a?F =  a?F * aF
            aXF = Fraction::MakeFraction( aXF.GetNumerator(),   aF.GetNumerator(),
                                    aXF.GetDenominator(), aF.GetDenominator() );
            aYF = Fraction::MakeFraction( aYF.GetNumerator(),   aF.GetNumerator(),
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
                                       maMapRes.mfMapScX );
    mnOutOffLogicY = ImplPixelToLogic( mnOutOffOrigY, mnDPIY,
                                       maMapRes.mfMapScY );
}

// #i75163#
basegfx::B2DHomMatrix OutputDevice::GetViewTransformation() const
{
    if(mbMap && mpOutDevData)
    {
        if(!mpOutDevData->mpViewTransform)
        {
            mpOutDevData->mpViewTransform = new basegfx::B2DHomMatrix;

            const double fScaleFactorX(static_cast<double>(mnDPIX) * maMapRes.mfMapScX);
            const double fScaleFactorY(static_cast<double>(mnDPIY) * maMapRes.mfMapScY);
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

    const double fScaleFactorX(static_cast<double>(mnDPIX) * aMapRes.mfMapScX);
    const double fScaleFactorY(static_cast<double>(mnDPIY) * aMapRes.mfMapScY);
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
                                    maMapRes.mfMapScX )+mnOutOffOrigX,
                  ImplLogicToPixel( rLogicPt.Y() + maMapRes.mnMapOfsY, mnDPIY,
                                    maMapRes.mfMapScY )+mnOutOffOrigY );
}

Size OutputDevice::LogicToPixel( const Size& rLogicSize ) const
{

    if ( !mbMap )
        return rLogicSize;

    return Size( ImplLogicToPixel( rLogicSize.Width(), mnDPIX,
                                   maMapRes.mfMapScX ),
                 ImplLogicToPixel( rLogicSize.Height(), mnDPIY,
                                   maMapRes.mfMapScY ) );
}

tools::Rectangle OutputDevice::LogicToPixel( const tools::Rectangle& rLogicRect ) const
{
    // tdf#141761 see comments above, IsEmpty() removed
    if ( !mbMap )
        return rLogicRect;

    tools::Rectangle aRetval(
        ImplLogicToPixel( rLogicRect.Left() + maMapRes.mnMapOfsX, mnDPIX, maMapRes.mfMapScX )+mnOutOffOrigX,
        ImplLogicToPixel( rLogicRect.Top() + maMapRes.mnMapOfsY, mnDPIY, maMapRes.mfMapScY )+mnOutOffOrigY,
        rLogicRect.IsWidthEmpty() ? 0 : ImplLogicToPixel( rLogicRect.Right() + maMapRes.mnMapOfsX, mnDPIX, maMapRes.mfMapScX )+mnOutOffOrigX,
        rLogicRect.IsHeightEmpty() ? 0 : ImplLogicToPixel( rLogicRect.Bottom() + maMapRes.mnMapOfsY, mnDPIY, maMapRes.mfMapScY )+mnOutOffOrigY );

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
                                    maMapRes.mfMapScX )+mnOutOffOrigX );
        aPt.setY( ImplLogicToPixel( pPt->Y() + maMapRes.mnMapOfsY, mnDPIY,
                                    maMapRes.mfMapScY )+mnOutOffOrigY );
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
    const basegfx::B2DHomMatrix aTransformationMatrix = GetViewTransformation();
    aTransformedPoly.transform( aTransformationMatrix );
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
                                    aMapRes.mfMapScX )+mnOutOffOrigX,
                  ImplLogicToPixel( rLogicPt.Y() + aMapRes.mnMapOfsY, mnDPIY,
                                    aMapRes.mfMapScY )+mnOutOffOrigY );
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
                                   aMapRes.mfMapScX ),
                 ImplLogicToPixel( rLogicSize.Height(), mnDPIY,
                                   aMapRes.mfMapScY ) );
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
        ImplLogicToPixel( rLogicRect.Left() + aMapRes.mnMapOfsX, mnDPIX, aMapRes.mfMapScX )+mnOutOffOrigX,
        ImplLogicToPixel( rLogicRect.Top() + aMapRes.mnMapOfsY, mnDPIY, aMapRes.mfMapScY )+mnOutOffOrigY,
        rLogicRect.IsWidthEmpty() ? 0 : ImplLogicToPixel( rLogicRect.Right() + aMapRes.mnMapOfsX, mnDPIX,   aMapRes.mfMapScX )+mnOutOffOrigX,
        rLogicRect.IsHeightEmpty() ? 0 : ImplLogicToPixel( rLogicRect.Bottom() + aMapRes.mnMapOfsY, mnDPIY, aMapRes.mfMapScY )+mnOutOffOrigY );

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
                                    aMapRes.mfMapScX )+mnOutOffOrigX );
        aPt.setY( ImplLogicToPixel( pPt->Y() + aMapRes.mnMapOfsY, mnDPIY,
                                    aMapRes.mfMapScY )+mnOutOffOrigY );
        aPoly[i] = aPt;
    }

    return aPoly;
}

basegfx::B2DPolyPolygon OutputDevice::LogicToPixel( const basegfx::B2DPolyPolygon& rLogicPolyPoly,
                                                    const MapMode& rMapMode ) const
{
    basegfx::B2DPolyPolygon aTransformedPoly = rLogicPolyPoly;
    const basegfx::B2DHomMatrix aTransformationMatrix = GetViewTransformation( rMapMode );
    aTransformedPoly.transform( aTransformationMatrix );
    return aTransformedPoly;
}

Point OutputDevice::PixelToLogic( const Point& rDevicePt ) const
{

    if ( !mbMap )
        return rDevicePt;

    return Point( ImplPixelToLogic( rDevicePt.X(), mnDPIX,
                                    maMapRes.mfMapScX ) - maMapRes.mnMapOfsX - mnOutOffLogicX,
                  ImplPixelToLogic( rDevicePt.Y(), mnDPIY,
                                    maMapRes.mfMapScY ) - maMapRes.mnMapOfsY - mnOutOffLogicY );
}

Point OutputDevice::SubPixelToLogic(const basegfx::B2DPoint& rDevicePt) const
{
    if (!mbMap)
    {
        assert(floor(rDevicePt.getX() == rDevicePt.getX()) && floor(rDevicePt.getY() == rDevicePt.getY()));
        return Point(rDevicePt.getX(), rDevicePt.getY());
    }

    return Point(ImplSubPixelToLogic(rDevicePt.getX(), mnDPIX,
                                     maMapRes.mfMapScX) - maMapRes.mnMapOfsX - mnOutOffLogicX,
                 ImplSubPixelToLogic(rDevicePt.getY(), mnDPIY,
                                     maMapRes.mfMapScY) - maMapRes.mnMapOfsY - mnOutOffLogicY);
}

Size OutputDevice::PixelToLogic( const Size& rDeviceSize ) const
{

    if ( !mbMap )
        return rDeviceSize;

    return Size( ImplPixelToLogic( rDeviceSize.Width(), mnDPIX,
                                   maMapRes.mfMapScX ),
                 ImplPixelToLogic( rDeviceSize.Height(), mnDPIY,
                                   maMapRes.mfMapScY ) );
}

tools::Rectangle OutputDevice::PixelToLogic( const tools::Rectangle& rDeviceRect ) const
{
    // tdf#141761 see comments above, IsEmpty() removed
    if ( !mbMap )
        return rDeviceRect;

    tools::Rectangle aRetval(
        ImplPixelToLogic( rDeviceRect.Left(), mnDPIX, maMapRes.mfMapScX ) - maMapRes.mnMapOfsX - mnOutOffLogicX,
        ImplPixelToLogic( rDeviceRect.Top(), mnDPIY, maMapRes.mfMapScY ) - maMapRes.mnMapOfsY - mnOutOffLogicY,
        rDeviceRect.IsWidthEmpty() ? 0 : ImplPixelToLogic( rDeviceRect.Right(), mnDPIX, maMapRes.mfMapScX ) - maMapRes.mnMapOfsX - mnOutOffLogicX,
        rDeviceRect.IsHeightEmpty() ? 0 : ImplPixelToLogic( rDeviceRect.Bottom(), mnDPIY, maMapRes.mfMapScY ) - maMapRes.mnMapOfsY - mnOutOffLogicY );

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
                                    maMapRes.mfMapScX ) - maMapRes.mnMapOfsX - mnOutOffLogicX );
        aPt.setY( ImplPixelToLogic( pPt->Y(), mnDPIY,
                                    maMapRes.mfMapScY ) - maMapRes.mnMapOfsY - mnOutOffLogicY );
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
    const basegfx::B2DHomMatrix aTransformationMatrix = GetInverseViewTransformation();
    aTransformedPoly.transform( aTransformationMatrix );
    return aTransformedPoly;
}

basegfx::B2DRectangle OutputDevice::PixelToLogic(const basegfx::B2DRectangle& rDeviceRect) const
{
    basegfx::B2DRectangle aTransformedRect = rDeviceRect;
    const basegfx::B2DHomMatrix aTransformationMatrix = GetInverseViewTransformation();
    aTransformedRect.transform(aTransformationMatrix);
    return aTransformedRect;
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
                                    aMapRes.mfMapScX ) - aMapRes.mnMapOfsX - mnOutOffLogicX,
                  ImplPixelToLogic( rDevicePt.Y(), mnDPIY,
                                    aMapRes.mfMapScY ) - aMapRes.mnMapOfsY - mnOutOffLogicY );
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
                                   aMapRes.mfMapScX ),
                 ImplPixelToLogic( rDeviceSize.Height(), mnDPIY,
                                   aMapRes.mfMapScY ) );
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
        ImplPixelToLogic( rDeviceRect.Left(), mnDPIX, aMapRes.mfMapScX ) - aMapRes.mnMapOfsX - mnOutOffLogicX,
        ImplPixelToLogic( rDeviceRect.Top(), mnDPIY, aMapRes.mfMapScY ) - aMapRes.mnMapOfsY - mnOutOffLogicY,
        rDeviceRect.IsWidthEmpty() ? 0 : ImplPixelToLogic( rDeviceRect.Right(), mnDPIX, aMapRes.mfMapScX ) - aMapRes.mnMapOfsX - mnOutOffLogicX,
        rDeviceRect.IsHeightEmpty() ? 0 : ImplPixelToLogic( rDeviceRect.Bottom(), mnDPIY, aMapRes.mfMapScY ) - aMapRes.mnMapOfsY - mnOutOffLogicY );

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
                                    aMapRes.mfMapScX ) - aMapRes.mnMapOfsX - mnOutOffLogicX );
        aPt.setY( ImplPixelToLogic( pPt->Y(), mnDPIY,
                                    aMapRes.mfMapScY ) - aMapRes.mnMapOfsY - mnOutOffLogicY );
        aPoly[i] = aPt;
    }

    return aPoly;
}

basegfx::B2DPolygon OutputDevice::PixelToLogic( const basegfx::B2DPolygon& rPixelPoly,
                                                const MapMode& rMapMode ) const
{
    basegfx::B2DPolygon aTransformedPoly = rPixelPoly;
    const basegfx::B2DHomMatrix aTransformationMatrix = GetInverseViewTransformation( rMapMode );
    aTransformedPoly.transform( aTransformationMatrix );
    return aTransformedPoly;
}

basegfx::B2DPolyPolygon OutputDevice::PixelToLogic( const basegfx::B2DPolyPolygon& rPixelPolyPoly,
                                                    const MapMode& rMapMode ) const
{
    basegfx::B2DPolyPolygon aTransformedPoly = rPixelPolyPoly;
    const basegfx::B2DHomMatrix aTransformationMatrix = GetInverseViewTransformation( rMapMode );
    aTransformedPoly.transform( aTransformationMatrix );
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
    ImplMapRes aMapResDest;                                             \
                                                                        \
    if ( !mbMap || pMapModeSource != &maMapMode )                       \
    {                                                                   \
        if ( pMapModeSource->GetMapUnit() == MapUnit::MapRelative )     \
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

namespace
{
auto getCorrectedUnit(MapUnit eMapSrc, MapUnit eMapDst)
{
    o3tl::Length eSrc = o3tl::Length::invalid;
    o3tl::Length eDst = o3tl::Length::invalid;
    if (eMapSrc > MapUnit::MapPixel)
        SAL_WARN("vcl.gdi", "Invalid source map unit");
    else if (eMapDst > MapUnit::MapPixel)
        SAL_WARN("vcl.gdi", "Invalid destination map unit");
    else if (eMapSrc != eMapDst)
    {
        // Here 72 PPI is assumed for MapPixel
        eSrc = MapToO3tlLength(eMapSrc, o3tl::Length::pt);
        eDst = MapToO3tlLength(eMapDst, o3tl::Length::pt);
    }
    return std::make_pair(eSrc, eDst);
}

std::pair<ImplMapRes, ImplMapRes> ENTER4(const MapMode& rMMSource, const MapMode& rMMDest)
{
    std::pair<ImplMapRes, ImplMapRes> result;
    ImplCalcMapResolution(rMMSource, 72, 72, result.first);
    ImplCalcMapResolution(rMMDest, 72, 72, result.second);
    return result;
}
}

static tools::Long fn5( const tools::Long n1,
                 const double n2,
                 const double n3 )
{
    if (n3 == 0.0)
        return 0;
    return std::llround(n1 * n2 / n3);
}

static tools::Long fn3(const tools::Long n1, const o3tl::Length eFrom, const o3tl::Length eTo)
{
    if (n1 == 0 || eFrom == o3tl::Length::invalid || eTo == o3tl::Length::invalid)
        return 0;
    bool bOverflow;
    const auto nResult = o3tl::convert(n1, eFrom, eTo, bOverflow);
    if (bOverflow)
    {
        const auto [n2, n3] = o3tl::getConversionMulDiv(eFrom, eTo);
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
        return nResult;
}

Point OutputDevice::LogicToLogic( const Point& rPtSource,
                                  const MapMode* pMapModeSource,
                                  const MapMode* pMapModeDest ) const
{
    ENTER1( rPtSource, pMapModeSource, pMapModeDest );

    return Point( fn5( rPtSource.X() + aMapResSource.mnMapOfsX,
                       aMapResSource.mfMapScX, aMapResDest.mfMapScX ) -
                  aMapResDest.mnMapOfsX,
                  fn5( rPtSource.Y() + aMapResSource.mnMapOfsY,
                       aMapResSource.mfMapScY, aMapResDest.mfMapScY ) -
                  aMapResDest.mnMapOfsY );
}

Size OutputDevice::LogicToLogic( const Size& rSzSource,
                                 const MapMode* pMapModeSource,
                                 const MapMode* pMapModeDest ) const
{
    ENTER1( rSzSource, pMapModeSource, pMapModeDest );

    return Size( fn5( rSzSource.Width(),
                      aMapResSource.mfMapScX, aMapResDest.mfMapScX ),
                 fn5( rSzSource.Height(),
                      aMapResSource.mfMapScY, aMapResDest.mfMapScY ) );
}

tools::Rectangle OutputDevice::LogicToLogic( const tools::Rectangle& rRectSource,
                                      const MapMode* pMapModeSource,
                                      const MapMode* pMapModeDest ) const
{
    ENTER1( rRectSource, pMapModeSource, pMapModeDest );

    return tools::Rectangle( fn5( rRectSource.Left() + aMapResSource.mnMapOfsX,
                           aMapResSource.mfMapScX, aMapResDest.mfMapScX ) -
                      aMapResDest.mnMapOfsX,
                      fn5( rRectSource.Top() + aMapResSource.mnMapOfsY,
                           aMapResSource.mfMapScY, aMapResDest.mfMapScY ) -
                      aMapResDest.mnMapOfsY,
                      fn5( rRectSource.Right() + aMapResSource.mnMapOfsX,
                           aMapResSource.mfMapScX, aMapResDest.mfMapScX ) -
                      aMapResDest.mnMapOfsX,
                      fn5( rRectSource.Bottom() + aMapResSource.mnMapOfsY,
                           aMapResSource.mfMapScY, aMapResDest.mfMapScY ) -
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
        const auto [eFrom, eTo] = getCorrectedUnit(eUnitSource, eUnitDest);
        return Point(fn3(rPtSource.X(), eFrom, eTo), fn3(rPtSource.Y(), eFrom, eTo));
    }
    else
    {
        const auto [aMapResSource, aMapResDest] = ENTER4( rMapModeSource, rMapModeDest );

        return Point( fn5( rPtSource.X() + aMapResSource.mnMapOfsX,
                           aMapResSource.mfMapScX, aMapResDest.mfMapScX ) -
                      aMapResDest.mnMapOfsX,
                      fn5( rPtSource.Y() + aMapResSource.mnMapOfsY,
                           aMapResSource.mfMapScY, aMapResDest.mfMapScY ) -
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
        const auto [eFrom, eTo] = getCorrectedUnit(eUnitSource, eUnitDest);
        return Size(fn3(rSzSource.Width(), eFrom, eTo), fn3(rSzSource.Height(), eFrom, eTo));
    }
    else
    {
        const auto [aMapResSource, aMapResDest] = ENTER4( rMapModeSource, rMapModeDest );

        return Size( fn5( rSzSource.Width(),
                          aMapResSource.mfMapScX, aMapResDest.mfMapScX ),
                     fn5( rSzSource.Height(),
                          aMapResSource.mfMapScY, aMapResDest.mfMapScY ) );
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
        const auto [eFrom, eTo] = getCorrectedUnit(eUnitSource, eUnitDest);
        const double fScaleFactor(eFrom == o3tl::Length::invalid || eTo == o3tl::Length::invalid
                                      ? std::numeric_limits<double>::quiet_NaN()
                                      : o3tl::convert(1.0, eFrom, eTo));
        aTransform.set(0, 0, fScaleFactor);
        aTransform.set(1, 1, fScaleFactor);
    }
    else
    {
        const auto [aMapResSource, aMapResDest] = ENTER4(rMapModeSource, rMapModeDest);

        const double fScaleFactorX(aMapResSource.mfMapScX / aMapResDest.mfMapScX);
        const double fScaleFactorY(aMapResSource.mfMapScY / aMapResDest.mfMapScY);
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
        const auto [eFrom, eTo] = getCorrectedUnit(eUnitSource, eUnitDest);

        auto left = fn3(rRectSource.Left(), eFrom, eTo);
        auto top = fn3(rRectSource.Top(), eFrom, eTo);

        // tdf#141761 see comments above, IsEmpty() removed
        auto right = rRectSource.IsWidthEmpty() ? 0 : fn3(rRectSource.Right(), eFrom, eTo);
        auto bottom = rRectSource.IsHeightEmpty() ? 0 : fn3(rRectSource.Bottom(), eFrom, eTo);

        aRetval = tools::Rectangle(left, top, right, bottom);
    }
    else
    {
        const auto [aMapResSource, aMapResDest] = ENTER4( rMapModeSource, rMapModeDest );

        auto left = fn5( rRectSource.Left() + aMapResSource.mnMapOfsX,
                               aMapResSource.mfMapScX, aMapResDest.mfMapScX ) -
                          aMapResDest.mnMapOfsX;
        auto top = fn5( rRectSource.Top() + aMapResSource.mnMapOfsY,
                               aMapResSource.mfMapScY, aMapResDest.mfMapScY ) -
                          aMapResDest.mnMapOfsY;

        // tdf#141761 see comments above, IsEmpty() removed
        auto right = rRectSource.IsWidthEmpty() ? 0 : fn5( rRectSource.Right() + aMapResSource.mnMapOfsX,
                               aMapResSource.mfMapScX, aMapResDest.mfMapScX ) -
                          aMapResDest.mnMapOfsX;
        auto bottom = rRectSource.IsHeightEmpty() ? 0 : fn5( rRectSource.Bottom() + aMapResSource.mnMapOfsY,
                               aMapResSource.mfMapScY, aMapResDest.mfMapScY ) -
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
    const auto [eFrom, eTo] = getCorrectedUnit(eUnitSource, eUnitDest);
    return fn3(nLongSource, eFrom, eTo);
}

void OutputDevice::SetPixelOffset( const Size& rOffset )
{
    mnOutOffOrigX  = rOffset.Width();
    mnOutOffOrigY  = rOffset.Height();

    mnOutOffLogicX = ImplPixelToLogic( mnOutOffOrigX, mnDPIX,
                                       maMapRes.mfMapScX );
    mnOutOffLogicY = ImplPixelToLogic( mnOutOffOrigY, mnDPIY,
                                       maMapRes.mfMapScY );
}


double OutputDevice::ImplLogicWidthToDeviceSubPixel(tools::Long nWidth) const
{
    if (!mbMap)
        return nWidth;

    return ImplLogicToSubPixel(nWidth, mnDPIX,
                               maMapRes.mfMapScX);
}

double OutputDevice::ImplLogicHeightToDeviceSubPixel(tools::Long nHeight) const
{
    if (!mbMap)
        return nHeight;

    return ImplLogicToSubPixel(nHeight, mnDPIY,
                               maMapRes.mfMapScY);
}

basegfx::B2DPoint OutputDevice::ImplLogicToDeviceSubPixel(const Point& rPoint) const
{
    if (!mbMap)
        return basegfx::B2DPoint(rPoint.X() + mnOutOffX, rPoint.Y() + mnOutOffY);

    return basegfx::B2DPoint(ImplLogicToSubPixel(rPoint.X() + maMapRes.mnMapOfsX, mnDPIX,
                                           maMapRes.mfMapScX)
                                           + mnOutOffX + mnOutOffOrigX,
                       ImplLogicToSubPixel(rPoint.Y() + maMapRes.mnMapOfsY, mnDPIY,
                                           maMapRes.mfMapScY)
                                           + mnOutOffY + mnOutOffOrigY);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
