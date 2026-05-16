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

#include <sal/config.h>

#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/bigint.hxx>
#include <tools/debug.hxx>
#include <tools/mapunit.hxx>

#include <vcl/cursor.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wrkwin.hxx>

#include <CoordinateMapper.hxx>
#include <ImplOutDevData.hxx>
#include <svdata.hxx>
#include <window.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <tools/UnitConversion.hxx>

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

static tools::Long lcl_logicToPixel(tools::Long n, tools::Long nDPI, double fMap)
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

static tools::Long lcl_pixelToLogic(tools::Long n, tools::Long nDPI, double fMap)
{
    assert(nDPI > 0);
    if (fMap == 0)
        return 0;

    return std::llround(n / fMap / nDPI);
}

static double lcl_pixelToLogicDouble(double n, tools::Long nDPI, double fMap)
{
    assert(nDPI > 0);
    if (fMap == 0)
        return 0;
    return n / fMap / nDPI;
}

tools::Long OutputDevice::ImplLogicXToDevicePixel( tools::Long nX ) const
{
    if ( !mpMapper->IsMapModeEnabled() )
        return nX+GetOutOffXPixel();

    return lcl_logicToPixel( nX + maMapRes.mnMapOfsX, GetDPIX(),
                             maMapRes.mfMapScX )+GetOutOffXPixel()+mnOutOffOrigX;
}

tools::Long OutputDevice::ImplLogicYToDevicePixel( tools::Long nY ) const
{
    if ( !mpMapper->IsMapModeEnabled() )
        return nY+GetOutOffYPixel();

    return lcl_logicToPixel( nY + maMapRes.mnMapOfsY, GetDPIY(),
                             maMapRes.mfMapScY )+GetOutOffYPixel()+mnOutOffOrigY;
}

tools::Long OutputDevice::LogicWidthToDevicePixel(tools::Long nWidth) const
{
    if ( !mpMapper->IsMapModeEnabled() )
        return nWidth;

    return lcl_logicToPixel(nWidth, GetDPIX(), maMapRes.mfMapScX);
}

tools::Long OutputDevice::LogicHeightToDevicePixel( tools::Long nHeight ) const
{
    if ( !mpMapper->IsMapModeEnabled() )
        return nHeight;

    return lcl_logicToPixel(nHeight, GetDPIY(), maMapRes.mfMapScY);
}

tools::Long OutputDevice::DevicePixelToLogicWidth( tools::Long nWidth ) const
{
    if ( !mpMapper->IsMapModeEnabled() )
        return nWidth;

    return lcl_pixelToLogic(nWidth, GetDPIX(), maMapRes.mfMapScX);
}

SAL_DLLPRIVATE double OutputDevice::ImplDevicePixelToLogicWidthDouble(double nWidth) const
{
    if (!mpMapper->IsMapModeEnabled())
        return nWidth;

    return lcl_pixelToLogicDouble(nWidth, GetDPIX(), maMapRes.mfMapScX);
}

tools::Long OutputDevice::DevicePixelToLogicHeight( tools::Long nHeight ) const
{
    if ( !mpMapper->IsMapModeEnabled() )
        return nHeight;

    return lcl_pixelToLogic(nHeight, GetDPIY(), maMapRes.mfMapScY);
}

double OutputDevice::ImplDevicePixelToLogicHeightDouble(double nHeight) const
{
    if (!mpMapper->IsMapModeEnabled())
        return nHeight;

    return lcl_pixelToLogicDouble(nHeight, GetDPIY(), maMapRes.mfMapScY);
}

Point OutputDevice::LogicToDevicePixel(const Point& rLogicPt) const
{
    if ( !mpMapper->IsMapModeEnabled() )
        return Point( rLogicPt.X()+GetOutOffXPixel(), rLogicPt.Y()+GetOutOffYPixel() );

    return Point( lcl_logicToPixel( rLogicPt.X() + maMapRes.mnMapOfsX, GetDPIX(),
                                    maMapRes.mfMapScX )+GetOutOffXPixel()+mnOutOffOrigX,
                  lcl_logicToPixel( rLogicPt.Y() + maMapRes.mnMapOfsY, GetDPIY(),
                                    maMapRes.mfMapScY )+GetOutOffYPixel()+mnOutOffOrigY );
}

Size OutputDevice::ImplLogicToDevicePixel( const Size& rLogicSize ) const
{
    if ( !mpMapper->IsMapModeEnabled() )
        return rLogicSize;

    return Size( lcl_logicToPixel( rLogicSize.Width(), GetDPIX(),
                                   maMapRes.mfMapScX ),
                 lcl_logicToPixel( rLogicSize.Height(), GetDPIY(),
                                   maMapRes.mfMapScY ) );
}

tools::Rectangle OutputDevice::LogicToDevicePixel(const tools::Rectangle& rLogicRect) const
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

    if ( !mpMapper->IsMapModeEnabled() )
    {
        aRetval = tools::Rectangle(
            rLogicRect.Left()+GetOutOffXPixel(),
            rLogicRect.Top()+GetOutOffYPixel(),
            rLogicRect.IsWidthEmpty() ? 0 : rLogicRect.Right()+GetOutOffXPixel(),
            rLogicRect.IsHeightEmpty() ? 0 : rLogicRect.Bottom()+GetOutOffYPixel() );
    }
    else
    {
        aRetval = tools::Rectangle(
            lcl_logicToPixel( rLogicRect.Left()+maMapRes.mnMapOfsX, GetDPIX(), maMapRes.mfMapScX )+GetOutOffXPixel()+mnOutOffOrigX,
            lcl_logicToPixel( rLogicRect.Top()+maMapRes.mnMapOfsY, GetDPIY(), maMapRes.mfMapScY )+GetOutOffYPixel()+mnOutOffOrigY,
            rLogicRect.IsWidthEmpty() ? 0 : lcl_logicToPixel( rLogicRect.Right()+maMapRes.mnMapOfsX, GetDPIX(), maMapRes.mfMapScX )+GetOutOffXPixel()+mnOutOffOrigX,
            rLogicRect.IsHeightEmpty() ? 0 : lcl_logicToPixel( rLogicRect.Bottom()+maMapRes.mnMapOfsY, GetDPIY(), maMapRes.mfMapScY )+GetOutOffYPixel()+mnOutOffOrigY );
    }

    if(rLogicRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if(rLogicRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Polygon OutputDevice::ImplLogicToDevicePixel( const tools::Polygon& rLogicPoly ) const
{
    if ( !mpMapper->IsMapModeEnabled() && !GetOutOffXPixel() && !GetOutOffYPixel() )
        return rLogicPoly;

    const sal_uInt16 nPoints = rLogicPoly.GetSize();
    tools::Polygon aPoly( rLogicPoly );

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    if ( mpMapper->IsMapModeEnabled() )
    {
        for (sal_uInt16 i = 0; i < nPoints; i++)
        {
            const Point& rPt = pPointAry[i];
            Point aPt(lcl_logicToPixel( rPt.X()+maMapRes.mnMapOfsX, GetDPIX(),
                                        maMapRes.mfMapScX )+GetOutOffXPixel()+mnOutOffOrigX,
                      lcl_logicToPixel( rPt.Y()+maMapRes.mnMapOfsY, GetDPIY(),
                                        maMapRes.mfMapScY )+GetOutOffYPixel()+mnOutOffOrigY);
            aPoly[i] = aPt;
        }
    }
    else
    {
        for (sal_uInt16 i = 0; i < nPoints; i++)
        {
            Point aPt = pPointAry[i];
            aPt.AdjustX(GetOutOffXPixel() );
            aPt.AdjustY(GetOutOffYPixel() );
            aPoly[i] = aPt;
        }
    }

    return aPoly;
}

basegfx::B2DPolygon OutputDevice::ImplLogicToDevicePixel(const basegfx::B2DPolygon& rLogicPoly) const
{
    if (!mpMapper->IsMapModeEnabled() && !GetOutOffXPixel() && !GetOutOffYPixel())
        return rLogicPoly;

    const sal_uInt32 nPoints = rLogicPoly.count();
    basegfx::B2DPolygon aPoly(rLogicPoly);

    basegfx::B2DPoint aC1;
    basegfx::B2DPoint aC2;

    if (mpMapper->IsMapModeEnabled())
    {
        for (sal_uInt32 i = 0; i < nPoints; ++i)
        {
            const basegfx::B2DPoint& rPt = aPoly.getB2DPoint(i);
            basegfx::B2DPoint aPt(lcl_logicToPixel( rPt.getX()+maMapRes.mnMapOfsX, GetDPIX(),
                                        maMapRes.mfMapScX )+GetOutOffXPixel()+mnOutOffOrigX,
                                  lcl_logicToPixel( rPt.getY()+maMapRes.mnMapOfsY, GetDPIY(),
                                        maMapRes.mfMapScY )+GetOutOffYPixel()+mnOutOffOrigY);

            const bool bC1 = aPoly.isPrevControlPointUsed(i);
            if (bC1)
            {
                const basegfx::B2DPoint aB2DC1(aPoly.getPrevControlPoint(i));

                aC1 = basegfx::B2DPoint(lcl_logicToPixel( aB2DC1.getX()+maMapRes.mnMapOfsX, GetDPIX(),
                                            maMapRes.mfMapScX )+GetOutOffXPixel()+mnOutOffOrigX,
                                        lcl_logicToPixel( aB2DC1.getY()+maMapRes.mnMapOfsY, GetDPIY(),
                                            maMapRes.mfMapScY )+GetOutOffYPixel()+mnOutOffOrigY);
            }

            const bool bC2 = aPoly.isNextControlPointUsed(i);
            if (bC2)
            {
                const basegfx::B2DPoint aB2DC2(aPoly.getNextControlPoint(i));

                aC2 = basegfx::B2DPoint(lcl_logicToPixel( aB2DC2.getX()+maMapRes.mnMapOfsX, GetDPIX(),
                                            maMapRes.mfMapScX )+GetOutOffXPixel()+mnOutOffOrigX,
                                        lcl_logicToPixel( aB2DC2.getY()+maMapRes.mnMapOfsY, GetDPIY(),
                                            maMapRes.mfMapScY )+GetOutOffYPixel()+mnOutOffOrigY);
            }

            aPoly.setB2DPoint(i, aPt);

            if (bC1)
                aPoly.setPrevControlPoint(i, aC1);

            if (bC2)
                aPoly.setNextControlPoint(i, aC2);
        }

        return aPoly;
    }

    for (sal_uInt32 i = 0; i < nPoints; ++i)
    {
        const basegfx::B2DPoint& rPt = aPoly.getB2DPoint(i);
        basegfx::B2DPoint aPt(rPt.getX() + GetOutOffXPixel(), rPt.getY() + GetOutOffYPixel());

        const bool bC1 = aPoly.isPrevControlPointUsed(i);
        if (bC1)
        {
            const basegfx::B2DPoint aB2DC1(aPoly.getPrevControlPoint(i));

            aC1 = basegfx::B2DPoint(aB2DC1.getX() + GetOutOffXPixel(), aB2DC1.getY() + GetOutOffYPixel());
        }

        const bool bC2 = aPoly.isNextControlPointUsed(i);
        if (bC2)
        {
            const basegfx::B2DPoint aB2DC2(aPoly.getNextControlPoint(i));

            aC2 = basegfx::B2DPoint(aB2DC2.getX() + GetOutOffXPixel(), aB2DC2.getY() + GetOutOffYPixel());
        }

        aPoly.setB2DPoint(i, aPt);

        if (bC1)
            aPoly.setPrevControlPoint(i, aC1);

        if (bC2)
            aPoly.setNextControlPoint(i, aC2);
    }

    return aPoly;
}

tools::PolyPolygon OutputDevice::ImplLogicToDevicePixel( const tools::PolyPolygon& rLogicPolyPoly ) const
{
    if ( !mpMapper->IsMapModeEnabled() && !GetOutOffXPixel() && !GetOutOffYPixel() )
        return rLogicPolyPoly;

    tools::PolyPolygon aPolyPoly( rLogicPolyPoly );
    const sal_uInt16 nPoly = aPolyPoly.Count();

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
            aInfo.SetDotLen(std::max(LogicWidthToDevicePixel(aInfo.GetDotLen()), tools::Long(1)));
        else
            aInfo.SetDotCount( 0 );

        if( aInfo.GetDashCount() && aInfo.GetDashLen() )
            aInfo.SetDashLen(std::max(LogicWidthToDevicePixel(aInfo.GetDashLen()), tools::Long(1)));
        else
            aInfo.SetDashCount( 0 );

        aInfo.SetDistance(LogicWidthToDevicePixel(aInfo.GetDistance()));

        if( ( !aInfo.GetDashCount() && !aInfo.GetDotCount() ) || !aInfo.GetDistance() )
            aInfo.SetStyle( LineStyle::Solid );
    }

    aInfo.SetWidth(LogicWidthToDevicePixel(aInfo.GetWidth()));

    return aInfo;
}

tools::Rectangle OutputDevice::ImplDevicePixelToLogic( const tools::Rectangle& rPixelRect ) const
{
    // tdf#141761 see comments above, IsEmpty() removed
    tools::Rectangle aRetval;

    if ( !mpMapper->IsMapModeEnabled() )
    {
        aRetval = tools::Rectangle(
            rPixelRect.Left()-GetOutOffXPixel(),
            rPixelRect.Top()-GetOutOffYPixel(),
            rPixelRect.IsWidthEmpty() ? 0 : rPixelRect.Right()-GetOutOffXPixel(),
            rPixelRect.IsHeightEmpty() ? 0 : rPixelRect.Bottom()-GetOutOffYPixel() );
    }
    else
    {
        aRetval = tools::Rectangle(
            lcl_pixelToLogic( rPixelRect.Left()-GetOutOffXPixel()-mnOutOffOrigX, GetDPIX(), maMapRes.mfMapScX )-maMapRes.mnMapOfsX,
            lcl_pixelToLogic( rPixelRect.Top()-GetOutOffYPixel()-mnOutOffOrigY, GetDPIY(), maMapRes.mfMapScY )-maMapRes.mnMapOfsY,
            rPixelRect.IsWidthEmpty() ? 0 : lcl_pixelToLogic( rPixelRect.Right()-GetOutOffXPixel()-mnOutOffOrigX, GetDPIX(), maMapRes.mfMapScX )-maMapRes.mnMapOfsX,
            rPixelRect.IsHeightEmpty() ? 0 : lcl_pixelToLogic( rPixelRect.Bottom()-GetOutOffYPixel()-mnOutOffOrigY, GetDPIY(), maMapRes.mfMapScY )-maMapRes.mnMapOfsY );
    }

    if(rPixelRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if(rPixelRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

vcl::Region OutputDevice::ImplPixelToDevicePixel( const vcl::Region& rRegion ) const
{
    if ( !GetOutOffXPixel() && !GetOutOffYPixel() )
        return rRegion;

    vcl::Region aRegion( rRegion );
    aRegion.Move( GetOutOffXPixel()+mnOutOffOrigX, GetOutOffYPixel()+mnOutOffOrigY );
    return aRegion;
}

tools::Long OutputDevice::GetOutputWidthPixel() const { return mpMapper->GetOutputWidthPixel(); }

tools::Long OutputDevice::GetOutputHeightPixel() const { return mpMapper->GetOutputHeightPixel(); }

void OutputDevice::SetOutputWidthPixel(tools::Long nWidth) { mpMapper->SetOutputWidthPixel(nWidth); }

void OutputDevice::SetOutputHeightPixel(tools::Long nHeight) { mpMapper->SetOutputHeightPixel(nHeight); }

Size OutputDevice::GetOutputSizePixel() const { return Size(GetOutputWidthPixel(), GetOutputHeightPixel()); }

tools::Long OutputDevice::GetOutOffXPixel() const { return mpMapper->GetOutOffXPixel(); }

tools::Long OutputDevice::GetOutOffYPixel() const { return mpMapper->GetOutOffYPixel(); }

void OutputDevice::SetOutOffXPixel(tools::Long nOutOffX) { return mpMapper->SetOutOffXPixel(nOutOffX); }

void OutputDevice::SetOutOffYPixel(tools::Long nOutOffY) { return mpMapper->SetOutOffYPixel(nOutOffY); }

Point OutputDevice::GetOutputOffPixel() const { return mpMapper->GetOutputOffPixel(); }

bool OutputDevice::IsMapModeEnabled() const { return mpMapper->IsMapModeEnabled(); }

void OutputDevice::EnableMapMode(bool bEnabled) { mpMapper->EnableMapMode(bEnabled); }

const MapMode& OutputDevice::GetMapMode() const { return mpMapper->GetMapMode(); }

void OutputDevice::SetMapMode()
{
    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaMapModeAction( MapMode() ) );

    if (!mpMapper->IsMapModeEnabled() && mpMapper->IsDefaultMapMode())
        return;

    mpMapper->EnableMapMode(false);
    mpMapper->ResetMapMode();

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

void OutputDevice::SetMapMode( const MapMode& rNewMapMode )
{

    bool bRelMap = (rNewMapMode.GetMapUnit() == MapUnit::MapRelative);

    if ( mpMetaFile )
    {
        mpMetaFile->AddAction( new MetaMapModeAction( rNewMapMode ) );
    }

    // do nothing if MapMode was not changed
    if (mpMapper->GetMapMode() == rNewMapMode)
        return;

     // if default MapMode calculate nothing
    bool bOldMap = mpMapper->IsMapModeEnabled();
    mpMapper->EnableMapMode(!rNewMapMode.IsDefault());
    if ( mpMapper->IsMapModeEnabled() )
    {
        // if only the origin is converted, do not scale new
        if ( (rNewMapMode.GetMapUnit() == mpMapper->GetMapUnit()) &&
             (rNewMapMode.GetScaleX()  == mpMapper->GetScaleX())  &&
             (rNewMapMode.GetScaleY()  == mpMapper->GetScaleY())  &&
             (bOldMap                  == mpMapper->IsMapModeEnabled()) )
        {
            // set offset
            Point aOrigin = rNewMapMode.GetOrigin();
            maMapRes.mnMapOfsX = aOrigin.X();
            maMapRes.mnMapOfsY = aOrigin.Y();
            mpMapper->ResetMapMode(rNewMapMode);

            // #i75163#
            ImplInvalidateViewTransform();

            return;
        }
        if ( !bOldMap && bRelMap )
        {
            maMapRes.mfMapScX    = 1.0 / GetDPIX();
            maMapRes.mfMapScY    = 1.0 / GetDPIY();
            maMapRes.mnMapOfsX      = 0;
            maMapRes.mnMapOfsY      = 0;
        }

        // calculate new MapMode-resolution
        maMapRes.CalcMapResolution(rNewMapMode, GetDPIX(), GetDPIY());
    }

    // set new MapMode
    if (bRelMap)
    {
        mpMapper->SetScaleX(mpMapper->GetScaleX() * rNewMapMode.GetScaleX());
        mpMapper->SetScaleY(mpMapper->GetScaleY() * rNewMapMode.GetScaleY());
        mpMapper->SetOrigin(Point(maMapRes.mnMapOfsX, maMapRes.mnMapOfsY));
    }
    else
    {
        mpMapper->ResetMapMode(rNewMapMode);
    }

    // create new objects (clip region are not re-scaled)
    mbNewFont   = true;
    mbInitFont  = true;
    ImplInitMapModeObjects();

    // #106426# Adapt logical offset when changing mapmode
    mnOutOffLogicX = lcl_pixelToLogic( mnOutOffOrigX, GetDPIX(),
                                       maMapRes.mfMapScX );
    mnOutOffLogicY = lcl_pixelToLogic( mnOutOffOrigY, GetDPIY(),
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
    if (mpMapper->GetMapMode() == rNewMapMode)
        return;

    MapUnit eOld = mpMapper->GetMapUnit();
    MapUnit eNew = rNewMapMode.GetMapUnit();

    double fXF = rNewMapMode.GetScaleX() / mpMapper->GetScaleX();
    double fYF = rNewMapMode.GetScaleY() / mpMapper->GetScaleY();

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
            double aF = double(div) / mul;

            // a?F =  a?F * aF
            fXF = fXF * aF;
            fYF = fYF * aF;
            if ( eOld == MapUnit::MapPixel )
            {
                fXF *= GetDPIX();
                fYF *= GetDPIY();
            }
            else if ( eNew == MapUnit::MapPixel )
            {
                fXF /= GetDPIX();
                fYF /= GetDPIY();
            }
        }
    }

    MapMode aNewMapMode( MapUnit::MapRelative, Point( -aPt.X(), -aPt.Y() ), fXF, fYF );
    SetMapMode( aNewMapMode );

    if ( eNew != eOld )
        mpMapper->ResetMapMode(rNewMapMode);

    // #106426# Adapt logical offset when changing MapMode
    mnOutOffLogicX = lcl_pixelToLogic( mnOutOffOrigX, GetDPIX(),
                                       maMapRes.mfMapScX );
    mnOutOffLogicY = lcl_pixelToLogic( mnOutOffOrigY, GetDPIY(),
                                       maMapRes.mfMapScY );
}

// #i75163#
basegfx::B2DHomMatrix OutputDevice::GetViewTransformation() const
{
    if (!mpMapper->IsMapModeEnabled() || !mpOutDevData)
        return basegfx::B2DHomMatrix();

    if(!mpOutDevData->mpViewTransform)
    {
        mpOutDevData->mpViewTransform = new basegfx::B2DHomMatrix;

        const double fScaleFactorX(static_cast<double>(GetDPIX()) * maMapRes.mfMapScX);
        const double fScaleFactorY(static_cast<double>(GetDPIY()) * maMapRes.mfMapScY);
        const double fZeroPointX((static_cast<double>(maMapRes.mnMapOfsX) * fScaleFactorX) + static_cast<double>(mnOutOffOrigX));
        const double fZeroPointY((static_cast<double>(maMapRes.mnMapOfsY) * fScaleFactorY) + static_cast<double>(mnOutOffOrigY));

        mpOutDevData->mpViewTransform->set(0, 0, fScaleFactorX);
        mpOutDevData->mpViewTransform->set(1, 1, fScaleFactorY);
        mpOutDevData->mpViewTransform->set(0, 2, fZeroPointX);
        mpOutDevData->mpViewTransform->set(1, 2, fZeroPointY);
    }

    return *mpOutDevData->mpViewTransform;
}

// #i75163#
basegfx::B2DHomMatrix OutputDevice::GetInverseViewTransformation() const
{
    if (!mpMapper->IsMapModeEnabled() || !mpOutDevData)
        return basegfx::B2DHomMatrix();

    if(!mpOutDevData->mpInverseViewTransform)
    {
        GetViewTransformation();
        mpOutDevData->mpInverseViewTransform = new basegfx::B2DHomMatrix(*mpOutDevData->mpViewTransform);
        mpOutDevData->mpInverseViewTransform->invert();
    }

    return *mpOutDevData->mpInverseViewTransform;
}

// #i75163#
basegfx::B2DHomMatrix OutputDevice::GetViewTransformation( const MapMode& rMapMode ) const
{
    // #i82615#
    ImplMapRes          aMapRes;
    aMapRes.CalcMapResolution(rMapMode, GetDPIX(), GetDPIY());

    basegfx::B2DHomMatrix aTransform;

    const double fScaleFactorX(static_cast<double>(GetDPIX()) * aMapRes.mfMapScX);
    const double fScaleFactorY(static_cast<double>(GetDPIY()) * aMapRes.mfMapScY);
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
    if( GetOutOffXPixel() || GetOutOffYPixel() )
        aTransformation.translate( GetOutOffXPixel(), GetOutOffYPixel() );
    return aTransformation;
}

Point OutputDevice::LogicToPixel( const Point& rLogicPt ) const
{

    if ( !mpMapper->IsMapModeEnabled() )
        return rLogicPt;

    return Point( lcl_logicToPixel( rLogicPt.X() + maMapRes.mnMapOfsX, GetDPIX(),
                                    maMapRes.mfMapScX )+mnOutOffOrigX,
                  lcl_logicToPixel( rLogicPt.Y() + maMapRes.mnMapOfsY, GetDPIY(),
                                    maMapRes.mfMapScY )+mnOutOffOrigY );
}

Size OutputDevice::LogicToPixel( const Size& rLogicSize ) const
{

    if ( !mpMapper->IsMapModeEnabled() )
        return rLogicSize;

    return Size( lcl_logicToPixel( rLogicSize.Width(), GetDPIX(),
                                   maMapRes.mfMapScX ),
                 lcl_logicToPixel( rLogicSize.Height(), GetDPIY(),
                                   maMapRes.mfMapScY ) );
}

tools::Rectangle OutputDevice::LogicToPixel( const tools::Rectangle& rLogicRect ) const
{
    // tdf#141761 see comments above, IsEmpty() removed
    if ( !mpMapper->IsMapModeEnabled() )
        return rLogicRect;

    tools::Rectangle aRetval(
        lcl_logicToPixel( rLogicRect.Left() + maMapRes.mnMapOfsX, GetDPIX(), maMapRes.mfMapScX )+mnOutOffOrigX,
        lcl_logicToPixel( rLogicRect.Top() + maMapRes.mnMapOfsY, GetDPIY(), maMapRes.mfMapScY )+mnOutOffOrigY,
        rLogicRect.IsWidthEmpty() ? 0 : lcl_logicToPixel( rLogicRect.Right() + maMapRes.mnMapOfsX, GetDPIX(), maMapRes.mfMapScX )+mnOutOffOrigX,
        rLogicRect.IsHeightEmpty() ? 0 : lcl_logicToPixel( rLogicRect.Bottom() + maMapRes.mnMapOfsY, GetDPIY(), maMapRes.mfMapScY )+mnOutOffOrigY );

    if(rLogicRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if(rLogicRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Polygon OutputDevice::LogicToPixel( const tools::Polygon& rLogicPoly ) const
{

    if ( !mpMapper->IsMapModeEnabled() )
        return rLogicPoly;

    const sal_uInt16 nPoints = rLogicPoly.GetSize();
    tools::Polygon aPoly( rLogicPoly );

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    for (sal_uInt16 i = 0; i < nPoints; i++)
    {
        const Point* pPt = &(pPointAry[i]);
        Point aPt;
        aPt.setX( lcl_logicToPixel( pPt->X() + maMapRes.mnMapOfsX, GetDPIX(),
                                    maMapRes.mfMapScX )+mnOutOffOrigX );
        aPt.setY( lcl_logicToPixel( pPt->Y() + maMapRes.mnMapOfsY, GetDPIY(),
                                    maMapRes.mfMapScY )+mnOutOffOrigY );
        aPoly[i] = aPt;
    }

    return aPoly;
}

tools::PolyPolygon OutputDevice::LogicToPixel( const tools::PolyPolygon& rLogicPolyPoly ) const
{

    if ( !mpMapper->IsMapModeEnabled() )
        return rLogicPolyPoly;

    tools::PolyPolygon aPolyPoly( rLogicPolyPoly );
    const sal_uInt16 nPoly = aPolyPoly.Count();

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

    if(!mpMapper->IsMapModeEnabled() || rLogicRegion.IsNull() || rLogicRegion.IsEmpty())
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
    aMapRes.CalcMapResolution(rMapMode, GetDPIX(), GetDPIY());

    return Point( lcl_logicToPixel( rLogicPt.X() + aMapRes.mnMapOfsX, GetDPIX(),
                                    aMapRes.mfMapScX )+mnOutOffOrigX,
                  lcl_logicToPixel( rLogicPt.Y() + aMapRes.mnMapOfsY, GetDPIY(),
                                    aMapRes.mfMapScY )+mnOutOffOrigY );
}

Size OutputDevice::LogicToPixel( const Size& rLogicSize,
                                 const MapMode& rMapMode ) const
{

    if ( rMapMode.IsDefault() )
        return rLogicSize;

    // convert MapMode resolution and convert
    ImplMapRes          aMapRes;
    aMapRes.CalcMapResolution(rMapMode, GetDPIX(), GetDPIY());

    return Size( lcl_logicToPixel( rLogicSize.Width(), GetDPIX(),
                                   aMapRes.mfMapScX ),
                 lcl_logicToPixel( rLogicSize.Height(), GetDPIY(),
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
    aMapRes.CalcMapResolution(rMapMode, GetDPIX(), GetDPIY());

    tools::Rectangle aRetval(
        lcl_logicToPixel( rLogicRect.Left() + aMapRes.mnMapOfsX, GetDPIX(), aMapRes.mfMapScX )+mnOutOffOrigX,
        lcl_logicToPixel( rLogicRect.Top() + aMapRes.mnMapOfsY, GetDPIY(), aMapRes.mfMapScY )+mnOutOffOrigY,
        rLogicRect.IsWidthEmpty() ? 0 : lcl_logicToPixel( rLogicRect.Right() + aMapRes.mnMapOfsX, GetDPIX(),   aMapRes.mfMapScX )+mnOutOffOrigX,
        rLogicRect.IsHeightEmpty() ? 0 : lcl_logicToPixel( rLogicRect.Bottom() + aMapRes.mnMapOfsY, GetDPIY(), aMapRes.mfMapScY )+mnOutOffOrigY );

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
    aMapRes.CalcMapResolution(rMapMode, GetDPIX(), GetDPIY());

    const sal_uInt16 nPoints = rLogicPoly.GetSize();
    tools::Polygon aPoly( rLogicPoly );

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    for (sal_uInt16 i = 0; i < nPoints; i++)
    {
        const Point* pPt = &(pPointAry[i]);
        Point aPt;
        aPt.setX( lcl_logicToPixel( pPt->X() + aMapRes.mnMapOfsX, GetDPIX(),
                                    aMapRes.mfMapScX )+mnOutOffOrigX );
        aPt.setY( lcl_logicToPixel( pPt->Y() + aMapRes.mnMapOfsY, GetDPIY(),
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

    if ( !mpMapper->IsMapModeEnabled() )
        return rDevicePt;

    return Point( lcl_pixelToLogic( rDevicePt.X(), GetDPIX(),
                                    maMapRes.mfMapScX ) - maMapRes.mnMapOfsX - mnOutOffLogicX,
                  lcl_pixelToLogic( rDevicePt.Y(), GetDPIY(),
                                    maMapRes.mfMapScY ) - maMapRes.mnMapOfsY - mnOutOffLogicY );
}

Point OutputDevice::SubPixelToLogic(const basegfx::B2DPoint& rDevicePt) const
{
    if (!mpMapper->IsMapModeEnabled())
    {
        assert(floor(rDevicePt.getX() == rDevicePt.getX()) && floor(rDevicePt.getY() == rDevicePt.getY()));
        return Point(rDevicePt.getX(), rDevicePt.getY());
    }

    return Point(ImplSubPixelToLogic(rDevicePt.getX(), GetDPIX(),
                                     maMapRes.mfMapScX) - maMapRes.mnMapOfsX - mnOutOffLogicX,
                 ImplSubPixelToLogic(rDevicePt.getY(), GetDPIY(),
                                     maMapRes.mfMapScY) - maMapRes.mnMapOfsY - mnOutOffLogicY);
}

Size OutputDevice::PixelToLogic( const Size& rDeviceSize ) const
{

    if ( !mpMapper->IsMapModeEnabled() )
        return rDeviceSize;

    return Size( lcl_pixelToLogic( rDeviceSize.Width(), GetDPIX(),
                                   maMapRes.mfMapScX ),
                 lcl_pixelToLogic( rDeviceSize.Height(), GetDPIY(),
                                   maMapRes.mfMapScY ) );
}

tools::Rectangle OutputDevice::PixelToLogic( const tools::Rectangle& rDeviceRect ) const
{
    // tdf#141761 see comments above, IsEmpty() removed
    if ( !mpMapper->IsMapModeEnabled() )
        return rDeviceRect;

    tools::Rectangle aRetval(
        lcl_pixelToLogic( rDeviceRect.Left(), GetDPIX(), maMapRes.mfMapScX ) - maMapRes.mnMapOfsX - mnOutOffLogicX,
        lcl_pixelToLogic( rDeviceRect.Top(), GetDPIY(), maMapRes.mfMapScY ) - maMapRes.mnMapOfsY - mnOutOffLogicY,
        rDeviceRect.IsWidthEmpty() ? 0 : lcl_pixelToLogic( rDeviceRect.Right(), GetDPIX(), maMapRes.mfMapScX ) - maMapRes.mnMapOfsX - mnOutOffLogicX,
        rDeviceRect.IsHeightEmpty() ? 0 : lcl_pixelToLogic( rDeviceRect.Bottom(), GetDPIY(), maMapRes.mfMapScY ) - maMapRes.mnMapOfsY - mnOutOffLogicY );

    if(rDeviceRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if(rDeviceRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Polygon OutputDevice::PixelToLogic( const tools::Polygon& rDevicePoly ) const
{

    if ( !mpMapper->IsMapModeEnabled() )
        return rDevicePoly;

    const sal_uInt16 nPoints = rDevicePoly.GetSize();
    tools::Polygon aPoly( rDevicePoly );

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    for (sal_uInt16 i = 0; i < nPoints; i++)
    {
        const Point* pPt = &(pPointAry[i]);
        Point aPt;
        aPt.setX( lcl_pixelToLogic( pPt->X(), GetDPIX(),
                                    maMapRes.mfMapScX ) - maMapRes.mnMapOfsX - mnOutOffLogicX );
        aPt.setY( lcl_pixelToLogic( pPt->Y(), GetDPIY(),
                                    maMapRes.mfMapScY ) - maMapRes.mnMapOfsY - mnOutOffLogicY );
        aPoly[i] = aPt;
    }

    return aPoly;
}

tools::PolyPolygon OutputDevice::PixelToLogic( const tools::PolyPolygon& rDevicePolyPoly ) const
{

    if ( !mpMapper->IsMapModeEnabled() )
        return rDevicePolyPoly;

    tools::PolyPolygon aPolyPoly( rDevicePolyPoly );
    const sal_uInt16 nPoly = aPolyPoly.Count();

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

    if(!mpMapper->IsMapModeEnabled() || rDeviceRegion.IsNull() || rDeviceRegion.IsEmpty())
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
    aMapRes.CalcMapResolution(rMapMode, GetDPIX(), GetDPIY());

    return Point( lcl_pixelToLogic( rDevicePt.X(), GetDPIX(),
                                    aMapRes.mfMapScX ) - aMapRes.mnMapOfsX - mnOutOffLogicX,
                  lcl_pixelToLogic( rDevicePt.Y(), GetDPIY(),
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
    aMapRes.CalcMapResolution(rMapMode, GetDPIX(), GetDPIY());

    return Size( lcl_pixelToLogic( rDeviceSize.Width(), GetDPIX(),
                                   aMapRes.mfMapScX ),
                 lcl_pixelToLogic( rDeviceSize.Height(), GetDPIY(),
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
    aMapRes.CalcMapResolution(rMapMode, GetDPIX(), GetDPIY());

    tools::Rectangle aRetval(
        lcl_pixelToLogic( rDeviceRect.Left(), GetDPIX(), aMapRes.mfMapScX ) - aMapRes.mnMapOfsX - mnOutOffLogicX,
        lcl_pixelToLogic( rDeviceRect.Top(), GetDPIY(), aMapRes.mfMapScY ) - aMapRes.mnMapOfsY - mnOutOffLogicY,
        rDeviceRect.IsWidthEmpty() ? 0 : lcl_pixelToLogic( rDeviceRect.Right(), GetDPIX(), aMapRes.mfMapScX ) - aMapRes.mnMapOfsX - mnOutOffLogicX,
        rDeviceRect.IsHeightEmpty() ? 0 : lcl_pixelToLogic( rDeviceRect.Bottom(), GetDPIY(), aMapRes.mfMapScY ) - aMapRes.mnMapOfsY - mnOutOffLogicY );

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
    aMapRes.CalcMapResolution(rMapMode, GetDPIX(), GetDPIY());

    const sal_uInt16 nPoints = rDevicePoly.GetSize();
    tools::Polygon aPoly( rDevicePoly );

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    for (sal_uInt16 i = 0; i < nPoints; i++)
    {
        const Point* pPt = &(pPointAry[i]);
        Point aPt;
        aPt.setX( lcl_pixelToLogic( pPt->X(), GetDPIX(),
                                    aMapRes.mfMapScX ) - aMapRes.mnMapOfsX - mnOutOffLogicX );
        aPt.setY( lcl_pixelToLogic( pPt->Y(), GetDPIY(),
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

static ImplMapRes lcl_resolveMapRes(const MapMode* pMode, const MapMode& rDefaultMapMode, const ImplMapRes& rDefaultMapRes,
                                    bool bMap, tools::Long nDPIX, tools::Long nDPIY)
{
    const MapMode* pEffectiveMode = pMode ? pMode : &rDefaultMapMode;

    if (bMap && pEffectiveMode == &rDefaultMapMode)
        return rDefaultMapRes;

    ImplMapRes aRes;

    if (pEffectiveMode->GetMapUnit() == MapUnit::MapRelative)
        aRes = rDefaultMapRes; // Pre-fill, do NOT return early

    aRes.CalcMapResolution(*pEffectiveMode, nDPIX, nDPIY);

    return aRes;
}

static void lcl_verifyUnitSourceDest( MapUnit eUnitSource, MapUnit eUnitDest )
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

static auto lcl_getCorrectedUnit(MapUnit eMapSrc, MapUnit eMapDst)
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

static std::pair<ImplMapRes, ImplMapRes> lcl_calcConversionMapRes(const MapMode& rMMSource, const MapMode& rMMDest)
{
    std::pair<ImplMapRes, ImplMapRes> result;
    result.first.CalcMapResolution(rMMSource, 72, 72);
    result.second.CalcMapResolution(rMMDest, 72, 72);
    return result;
}

static tools::Long lcl_scaleLogicValue( const tools::Long n1,
                 const double n2,
                 const double n3 )
{
    if (n3 == 0.0)
        return 0;
    return std::llround(n1 * n2 / n3);
}

static tools::Long lcl_convertLogicValue(const tools::Long n1, const o3tl::Length eFrom, const o3tl::Length eTo)
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
    }

    if (!bOverflow)
        return nResult;

    const auto [n2, n3] = o3tl::getConversionMulDiv(eFrom, eTo);
    BigInt a4 = n1;
    a4 *= n2;

    if ( a4.IsNeg() )
        a4 -= n3 / 2;
    else
        a4 += n3 / 2;

    a4 /= n3;

    return static_cast<tools::Long>(a4);
}

Point OutputDevice::LogicToLogic( const Point& rPtSource,
                                  const MapMode* pMapModeSource,
                                  const MapMode* pMapModeDest ) const
{
    const MapMode* pSrc = pMapModeSource ? pMapModeSource : &mpMapper->GetMapMode();
    const MapMode* pDst = pMapModeDest ? pMapModeDest : &mpMapper->GetMapMode();

    if (*pSrc == *pDst)
        return rPtSource;

    ImplMapRes aMapResSource = lcl_resolveMapRes(pMapModeSource, mpMapper->GetMapMode(), maMapRes, mpMapper->IsMapModeEnabled(), GetDPIX(), GetDPIY());
    ImplMapRes aMapResDest   = lcl_resolveMapRes(pMapModeDest, mpMapper->GetMapMode(), maMapRes, mpMapper->IsMapModeEnabled(), GetDPIX(), GetDPIY());

    return Point( lcl_scaleLogicValue( rPtSource.X() + aMapResSource.mnMapOfsX,
                       aMapResSource.mfMapScX, aMapResDest.mfMapScX ) -
                  aMapResDest.mnMapOfsX,
                  lcl_scaleLogicValue( rPtSource.Y() + aMapResSource.mnMapOfsY,
                       aMapResSource.mfMapScY, aMapResDest.mfMapScY ) -
                  aMapResDest.mnMapOfsY );
}

Size OutputDevice::LogicToLogic( const Size& rSzSource,
                                 const MapMode* pMapModeSource,
                                 const MapMode* pMapModeDest ) const
{
    const MapMode* pSrc = pMapModeSource ? pMapModeSource : &mpMapper->GetMapMode();
    const MapMode* pDst = pMapModeDest ? pMapModeDest : &mpMapper->GetMapMode();

    if (*pSrc == *pDst)
        return rSzSource;

    ImplMapRes aMapResSource = lcl_resolveMapRes(pMapModeSource, mpMapper->GetMapMode(), maMapRes, mpMapper->IsMapModeEnabled(), GetDPIX(), GetDPIY());
    ImplMapRes aMapResDest   = lcl_resolveMapRes(pMapModeDest, mpMapper->GetMapMode(), maMapRes, mpMapper->IsMapModeEnabled(), GetDPIX(), GetDPIY());

    return Size( lcl_scaleLogicValue( rSzSource.Width(),
                      aMapResSource.mfMapScX, aMapResDest.mfMapScX ),
                 lcl_scaleLogicValue( rSzSource.Height(),
                      aMapResSource.mfMapScY, aMapResDest.mfMapScY ) );
}

tools::Rectangle OutputDevice::LogicToLogic( const tools::Rectangle& rRectSource,
                                      const MapMode* pMapModeSource,
                                      const MapMode* pMapModeDest ) const
{
    const MapMode* pSrc = pMapModeSource ? pMapModeSource : &mpMapper->GetMapMode();
    const MapMode* pDst = pMapModeDest ? pMapModeDest : &mpMapper->GetMapMode();

    if (*pSrc == *pDst)
        return rRectSource;

    ImplMapRes aMapResSource = lcl_resolveMapRes(pMapModeSource, mpMapper->GetMapMode(), maMapRes, mpMapper->IsMapModeEnabled(), GetDPIX(), GetDPIY());
    ImplMapRes aMapResDest   = lcl_resolveMapRes(pMapModeDest, mpMapper->GetMapMode(), maMapRes, mpMapper->IsMapModeEnabled(), GetDPIX(), GetDPIY());

    return tools::Rectangle( lcl_scaleLogicValue( rRectSource.Left() + aMapResSource.mnMapOfsX,
                           aMapResSource.mfMapScX, aMapResDest.mfMapScX ) -
                      aMapResDest.mnMapOfsX,
                      lcl_scaleLogicValue( rRectSource.Top() + aMapResSource.mnMapOfsY,
                           aMapResSource.mfMapScY, aMapResDest.mfMapScY ) -
                      aMapResDest.mnMapOfsY,
                      lcl_scaleLogicValue( rRectSource.Right() + aMapResSource.mnMapOfsX,
                           aMapResSource.mfMapScX, aMapResDest.mfMapScX ) -
                      aMapResDest.mnMapOfsX,
                      lcl_scaleLogicValue( rRectSource.Bottom() + aMapResSource.mnMapOfsY,
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
    lcl_verifyUnitSourceDest( eUnitSource, eUnitDest );

    if (rMapModeSource.IsSimple() && rMapModeDest.IsSimple())
    {
        const auto [eFrom, eTo] = lcl_getCorrectedUnit(eUnitSource, eUnitDest);
        return Point(lcl_convertLogicValue(rPtSource.X(), eFrom, eTo), lcl_convertLogicValue(rPtSource.Y(), eFrom, eTo));
    }

    const auto [aMapResSource, aMapResDest] = lcl_calcConversionMapRes( rMapModeSource, rMapModeDest );

    return Point( lcl_scaleLogicValue( rPtSource.X() + aMapResSource.mnMapOfsX,
                       aMapResSource.mfMapScX, aMapResDest.mfMapScX ) -
                  aMapResDest.mnMapOfsX,
                  lcl_scaleLogicValue( rPtSource.Y() + aMapResSource.mnMapOfsY,
                       aMapResSource.mfMapScY, aMapResDest.mfMapScY ) -
                  aMapResDest.mnMapOfsY );
}

Size OutputDevice::LogicToLogic( const Size& rSzSource,
                                 const MapMode& rMapModeSource,
                                 const MapMode& rMapModeDest )
{
    if ( rMapModeSource == rMapModeDest )
        return rSzSource;

    MapUnit eUnitSource = rMapModeSource.GetMapUnit();
    MapUnit eUnitDest   = rMapModeDest.GetMapUnit();
    lcl_verifyUnitSourceDest( eUnitSource, eUnitDest );

    if (rMapModeSource.IsSimple() && rMapModeDest.IsSimple())
    {
        const auto [eFrom, eTo] = lcl_getCorrectedUnit(eUnitSource, eUnitDest);
        return Size(lcl_convertLogicValue(rSzSource.Width(), eFrom, eTo), lcl_convertLogicValue(rSzSource.Height(), eFrom, eTo));
    }

    const auto [aMapResSource, aMapResDest] = lcl_calcConversionMapRes( rMapModeSource, rMapModeDest );

    return Size( lcl_scaleLogicValue( rSzSource.Width(),
                      aMapResSource.mfMapScX, aMapResDest.mfMapScX ),
                 lcl_scaleLogicValue( rSzSource.Height(),
                      aMapResSource.mfMapScY, aMapResDest.mfMapScY ) );
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
    lcl_verifyUnitSourceDest(eUnitSource, eUnitDest);

    if (rMapModeSource.IsSimple() && rMapModeDest.IsSimple())
    {
        const auto [eFrom, eTo] = lcl_getCorrectedUnit(eUnitSource, eUnitDest);
        const double fScaleFactor(eFrom == o3tl::Length::invalid || eTo == o3tl::Length::invalid
                                      ? std::numeric_limits<double>::quiet_NaN()
                                      : o3tl::convert(1.0, eFrom, eTo));
        aTransform.set(0, 0, fScaleFactor);
        aTransform.set(1, 1, fScaleFactor);

        return aTransform;
    }

    const auto [aMapResSource, aMapResDest] = lcl_calcConversionMapRes(rMapModeSource, rMapModeDest);

    const double fScaleFactorX(aMapResSource.mfMapScX / aMapResDest.mfMapScX);
    const double fScaleFactorY(aMapResSource.mfMapScY / aMapResDest.mfMapScY);
    const double fZeroPointX(double(aMapResSource.mnMapOfsX) * fScaleFactorX - double(aMapResDest.mnMapOfsX));
    const double fZeroPointY(double(aMapResSource.mnMapOfsY) * fScaleFactorY - double(aMapResDest.mnMapOfsY));

    aTransform.set(0, 0, fScaleFactorX);
    aTransform.set(1, 1, fScaleFactorY);
    aTransform.set(0, 2, fZeroPointX);
    aTransform.set(1, 2, fZeroPointY);

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
    lcl_verifyUnitSourceDest( eUnitSource, eUnitDest );

    tools::Rectangle aRetval;

    if (rMapModeSource.IsSimple() && rMapModeDest.IsSimple())
    {
        const auto [eFrom, eTo] = lcl_getCorrectedUnit(eUnitSource, eUnitDest);

        auto left = lcl_convertLogicValue(rRectSource.Left(), eFrom, eTo);
        auto top = lcl_convertLogicValue(rRectSource.Top(), eFrom, eTo);

        // tdf#141761 see comments above, IsEmpty() removed
        auto right = rRectSource.IsWidthEmpty() ? 0 : lcl_convertLogicValue(rRectSource.Right(), eFrom, eTo);
        auto bottom = rRectSource.IsHeightEmpty() ? 0 : lcl_convertLogicValue(rRectSource.Bottom(), eFrom, eTo);

        aRetval = tools::Rectangle(left, top, right, bottom);
    }
    else
    {
        const auto [aMapResSource, aMapResDest] = lcl_calcConversionMapRes( rMapModeSource, rMapModeDest );

        auto left = lcl_scaleLogicValue( rRectSource.Left() + aMapResSource.mnMapOfsX,
                               aMapResSource.mfMapScX, aMapResDest.mfMapScX ) -
                          aMapResDest.mnMapOfsX;
        auto top = lcl_scaleLogicValue( rRectSource.Top() + aMapResSource.mnMapOfsY,
                               aMapResSource.mfMapScY, aMapResDest.mfMapScY ) -
                          aMapResDest.mnMapOfsY;

        // tdf#141761 see comments above, IsEmpty() removed
        auto right = rRectSource.IsWidthEmpty() ? 0 : lcl_scaleLogicValue( rRectSource.Right() + aMapResSource.mnMapOfsX,
                               aMapResSource.mfMapScX, aMapResDest.mfMapScX ) -
                          aMapResDest.mnMapOfsX;
        auto bottom = rRectSource.IsHeightEmpty() ? 0 : lcl_scaleLogicValue( rRectSource.Bottom() + aMapResSource.mnMapOfsY,
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

    lcl_verifyUnitSourceDest( eUnitSource, eUnitDest );
    const auto [eFrom, eTo] = lcl_getCorrectedUnit(eUnitSource, eUnitDest);
    return lcl_convertLogicValue(nLongSource, eFrom, eTo);
}

void OutputDevice::SetPixelOffset( const Size& rOffset )
{
    mnOutOffOrigX  = rOffset.Width();
    mnOutOffOrigY  = rOffset.Height();

    mnOutOffLogicX = lcl_pixelToLogic( mnOutOffOrigX, GetDPIX(),
                                       maMapRes.mfMapScX );
    mnOutOffLogicY = lcl_pixelToLogic( mnOutOffOrigY, GetDPIY(),
                                       maMapRes.mfMapScY );
}


double OutputDevice::LogicWidthToDeviceSubPixel(tools::Long nWidth) const
{
    if (!mpMapper->IsMapModeEnabled())
        return nWidth;

    return ImplLogicToSubPixel(nWidth, GetDPIX(),
                               maMapRes.mfMapScX);
}

double OutputDevice::LogicHeightToDeviceSubPixel(tools::Long nHeight) const
{
    if (!mpMapper->IsMapModeEnabled())
        return nHeight;

    return ImplLogicToSubPixel(nHeight, GetDPIY(),
                               maMapRes.mfMapScY);
}

basegfx::B2DPoint OutputDevice::LogicToDeviceSubPixel(const Point& rPoint) const
{
    if (!mpMapper->IsMapModeEnabled())
        return basegfx::B2DPoint(rPoint.X() + GetOutOffXPixel(), rPoint.Y() + GetOutOffYPixel());

    return basegfx::B2DPoint(ImplLogicToSubPixel(rPoint.X() + maMapRes.mnMapOfsX, GetDPIX(),
                                           maMapRes.mfMapScX)
                                           + GetOutOffXPixel() + mnOutOffOrigX,
                       ImplLogicToSubPixel(rPoint.Y() + maMapRes.mnMapOfsY, GetDPIY(),
                                           maMapRes.mfMapScY)
                                           + GetOutOffYPixel() + mnOutOffOrigY);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
