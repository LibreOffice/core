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

#include <ImplOutDevData.hxx>
#include <maptools.hxx>
#include <svdata.hxx>
#include <window.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <tools/UnitConversion.hxx>

// #i75163#
void OutputDevice::ImplInvalidateViewTransform()
{
    if (!mpOutDevData)
        return;

    if (mpOutDevData->mpViewTransform)
    {
        delete mpOutDevData->mpViewTransform;
        mpOutDevData->mpViewTransform = nullptr;
    }

    if (mpOutDevData->mpInverseViewTransform)
    {
        delete mpOutDevData->mpInverseViewTransform;
        mpOutDevData->mpInverseViewTransform = nullptr;
    }
}

void OutputDevice::EnableMapMode(bool bEnable)
{
    maGeometry.EnableMapMode(bEnable);

    if (mpAlphaVDev)
        mpAlphaVDev->EnableMapMode(bEnable);
}

void OutputDevice::SetMapMode()
{
    if (mpMetaFile)
        mpMetaFile->AddAction(new MetaMapModeAction(MapMode()));

    if (IsMapModeEnabled() || !maMapMode.IsDefault())
    {
        EnableMapMode(false);
        maMapMode = MapMode();

        // create new objects (clip region are not re-scaled)
        mbNewFont = true;
        mbInitFont = true;
        ImplInitMapModeObjects();

        // #106426# Adapt logical offset when changing mapmode
        maGeometry.SetXOffsetFromOriginInLogicalUnits(
            maGeometry.GetXOffsetFromOriginInPixels()); // no mapping -> equal offsets
        maGeometry.SetYOffsetFromOriginInLogicalUnits(maGeometry.GetYOffsetFromOriginInPixels());

        // #i75163#
        ImplInvalidateViewTransform();
    }

    if (mpAlphaVDev)
        mpAlphaVDev->SetMapMode();
}

void OutputDevice::SetMapMode(const MapMode& rNewMapMode)
{
    bool bRelMap = (rNewMapMode.GetMapUnit() == MapUnit::MapRelative);

    if (mpMetaFile)
    {
        mpMetaFile->AddAction(new MetaMapModeAction(rNewMapMode));
    }

    // do nothing if MapMode was not changed
    if (maMapMode == rNewMapMode)
        return;

    if (mpAlphaVDev)
        mpAlphaVDev->SetMapMode(rNewMapMode);

    // if default MapMode calculate nothing
    bool bOldMap = IsMapModeEnabled();
    EnableMapMode(!rNewMapMode.IsDefault());
    if (IsMapModeEnabled())
    {
        // if only the origin is converted, do not scale new
        if ((rNewMapMode.GetMapUnit() == maMapMode.GetMapUnit())
            && (rNewMapMode.GetScaleX() == maMapMode.GetScaleX())
            && (rNewMapMode.GetScaleY() == maMapMode.GetScaleY())
            && (bOldMap == IsMapModeEnabled()))
        {
            // set offset
            Point aOrigin = rNewMapMode.GetOrigin();
            maGeometry.SetMappingXOffset(aOrigin.X());
            maGeometry.SetMappingYOffset(aOrigin.Y());
            maMapMode = rNewMapMode;

            // #i75163#
            ImplInvalidateViewTransform();

            return;
        }
        if (!bOldMap && bRelMap)
        {
            maGeometry.SetMapScalingXNumerator(1);
            maGeometry.SetMapScalingYNumerator(1);
            maGeometry.SetMapScalingXDenominator(GetDPIX());
            maGeometry.SetMapScalingYDenominator(GetDPIY());
            maGeometry.SetMappingXOffset(0);
            maGeometry.SetMappingYOffset(0);
        }

        // calculate new MapMode-resolution
        maGeometry.GetMapMetrics().CalculateMappingResolution(rNewMapMode, GetDPIX(), GetDPIY());
    }

    // set new MapMode
    if (bRelMap)
    {
        Point aOrigin(maGeometry.GetMappingOffset().X(), maGeometry.GetMappingOffset().Y());
        // aScale? = maMapMode.GetScale?() * rNewMapMode.GetScale?()
        Fraction aScaleX = MakeFraction(
            maMapMode.GetScaleX().GetNumerator(), rNewMapMode.GetScaleX().GetNumerator(),
            maMapMode.GetScaleX().GetDenominator(), rNewMapMode.GetScaleX().GetDenominator());
        Fraction aScaleY = MakeFraction(
            maMapMode.GetScaleY().GetNumerator(), rNewMapMode.GetScaleY().GetNumerator(),
            maMapMode.GetScaleY().GetDenominator(), rNewMapMode.GetScaleY().GetDenominator());
        maMapMode.SetOrigin(aOrigin);
        maMapMode.SetScaleX(aScaleX);
        maMapMode.SetScaleY(aScaleY);
    }
    else
        maMapMode = rNewMapMode;

    // create new objects (clip region are not re-scaled)
    mbNewFont = true;
    mbInitFont = true;
    ImplInitMapModeObjects();

    // #106426# Adapt logical offset when changing mapmode
    maGeometry.SetXOffsetFromOriginInLogicalUnits(Geometry::PixelToLogic(
        maGeometry.GetXOffsetFromOriginInPixels(), GetDPIX(), maGeometry.GetMapScalingXNumerator(),
        maGeometry.GetMapScalingXDenominator()));
    maGeometry.SetXOffsetFromOriginInLogicalUnits(Geometry::PixelToLogic(
        maGeometry.GetYOffsetFromOriginInPixels(), GetDPIY(), maGeometry.GetMapScalingYNumerator(),
        maGeometry.GetMapScalingYDenominator()));

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

void OutputDevice::SetRelativeMapMode(const MapMode& rNewMapMode)
{
    // do nothing if MapMode did not change
    if (maMapMode == rNewMapMode)
        return;

    MapUnit eOld = maMapMode.GetMapUnit();
    MapUnit eNew = rNewMapMode.GetMapUnit();

    // a?F = rNewMapMode.GetScale?() / maMapMode.GetScale?()
    Fraction aXF = MakeFraction(
        rNewMapMode.GetScaleX().GetNumerator(), maMapMode.GetScaleX().GetDenominator(),
        rNewMapMode.GetScaleX().GetDenominator(), maMapMode.GetScaleX().GetNumerator());
    Fraction aYF = MakeFraction(
        rNewMapMode.GetScaleY().GetNumerator(), maMapMode.GetScaleY().GetDenominator(),
        rNewMapMode.GetScaleY().GetDenominator(), maMapMode.GetScaleY().GetNumerator());

    Point aPt(LogicToLogic(Point(), nullptr, &rNewMapMode));
    if (eNew != eOld)
    {
        if (eOld > MapUnit::MapPixel)
        {
            SAL_WARN("vcl.gdi", "Not implemented MapUnit");
        }
        else if (eNew > MapUnit::MapPixel)
        {
            SAL_WARN("vcl.gdi", "Not implemented MapUnit");
        }
        else
        {
            const auto eFrom = MapToO3tlLength(eOld, o3tl::Length::in);
            const auto eTo = MapToO3tlLength(eNew, o3tl::Length::in);
            const auto & [ mul, div ] = o3tl::getConversionMulDiv(eFrom, eTo);
            Fraction aF(div, mul);

            // a?F =  a?F * aF
            aXF = MakeFraction(aXF.GetNumerator(), aF.GetNumerator(), aXF.GetDenominator(),
                               aF.GetDenominator());
            aYF = MakeFraction(aYF.GetNumerator(), aF.GetNumerator(), aYF.GetDenominator(),
                               aF.GetDenominator());
            if (eOld == MapUnit::MapPixel)
            {
                aXF *= Fraction(GetDPIX(), 1);
                aYF *= Fraction(GetDPIY(), 1);
            }
            else if (eNew == MapUnit::MapPixel)
            {
                aXF *= Fraction(1, GetDPIX());
                aYF *= Fraction(1, GetDPIY());
            }
        }
    }

    MapMode aNewMapMode(MapUnit::MapRelative, Point(-aPt.X(), -aPt.Y()), aXF, aYF);
    SetMapMode(aNewMapMode);

    if (eNew != eOld)
        maMapMode = rNewMapMode;

    // #106426# Adapt logical offset when changing MapMode
    maGeometry.SetXOffsetFromOriginInLogicalUnits(Geometry::PixelToLogic(
        maGeometry.GetXOffsetFromOriginInPixels(), GetDPIX(), maGeometry.GetMapScalingXNumerator(),
        maGeometry.GetMapScalingXDenominator()));
    maGeometry.SetYOffsetFromOriginInLogicalUnits(Geometry::PixelToLogic(
        maGeometry.GetYOffsetFromOriginInPixels(), GetDPIY(), maGeometry.GetMapScalingYNumerator(),
        maGeometry.GetMapScalingYDenominator()));

    if (mpAlphaVDev)
        mpAlphaVDev->SetRelativeMapMode(rNewMapMode);
}

// #i75163#
basegfx::B2DHomMatrix OutputDevice::GetViewTransformation() const
{
    if (IsMapModeEnabled() && mpOutDevData)
    {
        if (!mpOutDevData->mpViewTransform)
        {
            mpOutDevData->mpViewTransform = new basegfx::B2DHomMatrix;

            const double fScaleFactorX(
                static_cast<double>(GetDPIX())
                * static_cast<double>(maGeometry.GetMapScalingXNumerator())
                / static_cast<double>(maGeometry.GetMapScalingXDenominator()));
            const double fScaleFactorY(
                static_cast<double>(GetDPIY())
                * static_cast<double>(maGeometry.GetMapScalingYNumerator())
                / static_cast<double>(maGeometry.GetMapScalingYDenominator()));
            const double fZeroPointX(
                (static_cast<double>(maGeometry.GetMappingOffset().X()) * fScaleFactorX)
                + static_cast<double>(maGeometry.GetXOffsetFromOriginInPixels()));
            const double fZeroPointY(
                (static_cast<double>(maGeometry.GetMappingOffset().Y()) * fScaleFactorY)
                + static_cast<double>(maGeometry.GetYOffsetFromOriginInPixels()));

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
    if (IsMapModeEnabled() && mpOutDevData)
    {
        if (!mpOutDevData->mpInverseViewTransform)
        {
            GetViewTransformation();
            mpOutDevData->mpInverseViewTransform
                = new basegfx::B2DHomMatrix(*mpOutDevData->mpViewTransform);
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
basegfx::B2DHomMatrix OutputDevice::GetViewTransformation(const MapMode& rMapMode) const
{
    // #i82615#
    MappingMetrics aMapRes;
    aMapRes.CalculateMappingResolution(rMapMode, GetDPIX(), GetDPIY());

    basegfx::B2DHomMatrix aTransform;

    const double fScaleFactorX(static_cast<double>(GetDPIX())
                               * static_cast<double>(aMapRes.mnMapScalingXNumerator)
                               / static_cast<double>(aMapRes.mnMapScalingXDenominator));
    const double fScaleFactorY(static_cast<double>(GetDPIY())
                               * static_cast<double>(aMapRes.mnMapScalingYNumerator)
                               / static_cast<double>(aMapRes.mnMapScalingYDenominator));
    const double fZeroPointX((static_cast<double>(aMapRes.mnMappingXOffset) * fScaleFactorX)
                             + static_cast<double>(maGeometry.GetXOffsetFromOriginInPixels()));
    const double fZeroPointY((static_cast<double>(aMapRes.mnMappingYOffset) * fScaleFactorY)
                             + static_cast<double>(maGeometry.GetYOffsetFromOriginInPixels()));

    aTransform.set(0, 0, fScaleFactorX);
    aTransform.set(1, 1, fScaleFactorY);
    aTransform.set(0, 2, fZeroPointX);
    aTransform.set(1, 2, fZeroPointY);

    return aTransform;
}

// #i75163#
basegfx::B2DHomMatrix OutputDevice::GetInverseViewTransformation(const MapMode& rMapMode) const
{
    basegfx::B2DHomMatrix aMatrix(GetViewTransformation(rMapMode));
    aMatrix.invert();
    return aMatrix;
}

basegfx::B2DHomMatrix OutputDevice::ImplGetDeviceTransformation() const
{
    basegfx::B2DHomMatrix aTransformation = GetViewTransformation();
    // TODO: is it worth to cache the transformed result?
    if (maGeometry.GetXFrameOffset() || maGeometry.GetYFrameOffset())
        aTransformation.translate(maGeometry.GetXFrameOffset(), maGeometry.GetYFrameOffset());
    return aTransformation;
}

Point OutputDevice::LogicToPixel(const Point& rLogicPt) const
{
    if (!IsMapModeEnabled())
        return rLogicPt;

    return Point(Geometry::LogicToPixel(rLogicPt.X() + maGeometry.GetMappingOffset().X(), GetDPIX(),
                                        maGeometry.GetMapScalingXNumerator(),
                                        maGeometry.GetMapScalingXDenominator())
                     + maGeometry.GetXOffsetFromOriginInPixels(),
                 Geometry::LogicToPixel(rLogicPt.Y() + maGeometry.GetMappingOffset().Y(), GetDPIY(),
                                        maGeometry.GetMapScalingYNumerator(),
                                        maGeometry.GetMapScalingYDenominator())
                     + maGeometry.GetYOffsetFromOriginInPixels());
}

Size OutputDevice::LogicToPixel(const Size& rLogicSize) const
{
    if (!IsMapModeEnabled())
        return rLogicSize;

    return Size(
        Geometry::LogicToPixel(rLogicSize.Width(), GetDPIX(), maGeometry.GetMapScalingXNumerator(),
                               maGeometry.GetMapScalingXDenominator()),
        Geometry::LogicToPixel(rLogicSize.Height(), GetDPIY(), maGeometry.GetMapScalingYNumerator(),
                               maGeometry.GetMapScalingYDenominator()));
}

tools::Rectangle OutputDevice::LogicToPixel(const tools::Rectangle& rLogicRect) const
{
    // tdf#141761 see comments above, IsEmpty() removed
    if (!IsMapModeEnabled())
        return rLogicRect;

    tools::Rectangle aRetval(
        Geometry::LogicToPixel(rLogicRect.Left() + maGeometry.GetMappingOffset().X(), GetDPIX(),
                               maGeometry.GetMapScalingXNumerator(),
                               maGeometry.GetMapScalingXDenominator())
            + maGeometry.GetXOffsetFromOriginInPixels(),
        Geometry::LogicToPixel(rLogicRect.Top() + maGeometry.GetMappingOffset().Y(), GetDPIY(),
                               maGeometry.GetMapScalingYNumerator(),
                               maGeometry.GetMapScalingYDenominator())
            + maGeometry.GetYOffsetFromOriginInPixels(),
        rLogicRect.IsWidthEmpty()
            ? 0
            : Geometry::LogicToPixel(rLogicRect.Right() + maGeometry.GetMappingOffset().X(),
                                     GetDPIX(), maGeometry.GetMapScalingXNumerator(),
                                     maGeometry.GetMapScalingXDenominator())
                  + maGeometry.GetXOffsetFromOriginInPixels(),
        rLogicRect.IsHeightEmpty()
            ? 0
            : Geometry::LogicToPixel(rLogicRect.Bottom() + maGeometry.GetMappingOffset().Y(),
                                     GetDPIY(), maGeometry.GetMapScalingYNumerator(),
                                     maGeometry.GetMapScalingYDenominator())
                  + maGeometry.GetYOffsetFromOriginInPixels());

    if (rLogicRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if (rLogicRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Polygon OutputDevice::LogicToPixel(const tools::Polygon& rLogicPoly) const
{
    if (!IsMapModeEnabled())
        return rLogicPoly;

    sal_uInt16 i;
    sal_uInt16 nPoints = rLogicPoly.GetSize();
    tools::Polygon aPoly(rLogicPoly);

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    for (i = 0; i < nPoints; i++)
    {
        const Point* pPt = &(pPointAry[i]);
        Point aPt;
        aPt.setX(Geometry::LogicToPixel(pPt->X() + maGeometry.GetMappingOffset().X(), GetDPIX(),
                                        maGeometry.GetMapScalingXNumerator(),
                                        maGeometry.GetMapScalingXDenominator())
                 + maGeometry.GetXOffsetFromOriginInPixels());
        aPt.setY(Geometry::LogicToPixel(pPt->Y() + maGeometry.GetMappingOffset().Y(), GetDPIY(),
                                        maGeometry.GetMapScalingYNumerator(),
                                        maGeometry.GetMapScalingYDenominator())
                 + maGeometry.GetYOffsetFromOriginInPixels());
        aPoly[i] = aPt;
    }

    return aPoly;
}

tools::PolyPolygon OutputDevice::LogicToPixel(const tools::PolyPolygon& rLogicPolyPoly) const
{
    if (!IsMapModeEnabled())
        return rLogicPolyPoly;

    tools::PolyPolygon aPolyPoly(rLogicPolyPoly);
    sal_uInt16 nPoly = aPolyPoly.Count();
    for (sal_uInt16 i = 0; i < nPoly; i++)
    {
        tools::Polygon& rPoly = aPolyPoly[i];
        rPoly = LogicToPixel(rPoly);
    }
    return aPolyPoly;
}

basegfx::B2DPolyPolygon
OutputDevice::LogicToPixel(const basegfx::B2DPolyPolygon& rLogicPolyPoly) const
{
    basegfx::B2DPolyPolygon aTransformedPoly = rLogicPolyPoly;
    const basegfx::B2DHomMatrix& rTransformationMatrix = GetViewTransformation();
    aTransformedPoly.transform(rTransformationMatrix);
    return aTransformedPoly;
}

vcl::Region OutputDevice::LogicToPixel(const vcl::Region& rLogicRegion) const
{
    if (IsMapModeEnabled() || rLogicRegion.IsNull() || rLogicRegion.IsEmpty())
        return rLogicRegion;

    vcl::Region aRegion;

    if (rLogicRegion.getB2DPolyPolygon())
    {
        aRegion = vcl::Region(LogicToPixel(*rLogicRegion.getB2DPolyPolygon()));
    }
    else if (rLogicRegion.getPolyPolygon())
    {
        aRegion = vcl::Region(LogicToPixel(*rLogicRegion.getPolyPolygon()));
    }
    else if (rLogicRegion.getRegionBand())
    {
        RectangleVector aRectangles;
        rLogicRegion.GetRegionRectangles(aRectangles);
        const RectangleVector& rRectangles(aRectangles); // needed to make the '!=' work

        // make reverse run to fill new region bottom-up, this will speed it up due to the used data structuring
        for (RectangleVector::const_reverse_iterator aRectIter(rRectangles.rbegin());
             aRectIter != rRectangles.rend(); ++aRectIter)
        {
            aRegion.Union(LogicToPixel(*aRectIter));
        }
    }

    return aRegion;
}

Point OutputDevice::LogicToPixel(const Point& rLogicPt, const MapMode& rMapMode) const
{
    if (rMapMode.IsDefault())
        return rLogicPt;

    // convert MapMode resolution and convert
    MappingMetrics aMapMetrics;
    aMapMetrics.CalculateMappingResolution(rMapMode, GetDPIX(), GetDPIY());

    return Point(Geometry::LogicToPixel(rLogicPt.X() + aMapMetrics.mnMappingXOffset, GetDPIX(),
                                        aMapMetrics.mnMapScalingXNumerator,
                                        aMapMetrics.mnMapScalingXDenominator)
                     + maGeometry.GetXOffsetFromOriginInPixels(),
                 Geometry::LogicToPixel(rLogicPt.Y() + aMapMetrics.mnMappingYOffset, GetDPIY(),
                                        aMapMetrics.mnMapScalingYNumerator,
                                        aMapMetrics.mnMapScalingYDenominator)
                     + maGeometry.GetYOffsetFromOriginInPixels());
}

Size OutputDevice::LogicToPixel(const Size& rLogicSize, const MapMode& rMapMode) const
{
    if (rMapMode.IsDefault())
        return rLogicSize;

    // convert MapMode resolution and convert
    MappingMetrics aMapMetrics;
    aMapMetrics.CalculateMappingResolution(rMapMode, GetDPIX(), GetDPIY());

    return Size(
        Geometry::LogicToPixel(rLogicSize.Width(), GetDPIX(), aMapMetrics.mnMapScalingXNumerator,
                               aMapMetrics.mnMapScalingXDenominator),
        Geometry::LogicToPixel(rLogicSize.Height(), GetDPIY(), aMapMetrics.mnMapScalingYNumerator,
                               aMapMetrics.mnMapScalingYDenominator));
}

tools::Rectangle OutputDevice::LogicToPixel(const tools::Rectangle& rLogicRect,
                                            const MapMode& rMapMode) const
{
    // tdf#141761 see comments above, IsEmpty() removed
    if (rMapMode.IsDefault())
        return rLogicRect;

    // convert MapMode resolution and convert
    MappingMetrics aMapMetrics;
    aMapMetrics.CalculateMappingResolution(rMapMode, GetDPIX(), GetDPIY());

    tools::Rectangle aRetval(
        Geometry::LogicToPixel(rLogicRect.Left() + aMapMetrics.mnMappingXOffset, GetDPIX(),
                               aMapMetrics.mnMapScalingXNumerator,
                               aMapMetrics.mnMapScalingXDenominator)
            + maGeometry.GetXOffsetFromOriginInPixels(),
        Geometry::LogicToPixel(rLogicRect.Top() + aMapMetrics.mnMappingYOffset, GetDPIY(),
                               aMapMetrics.mnMapScalingYNumerator,
                               aMapMetrics.mnMapScalingYDenominator)
            + maGeometry.GetYOffsetFromOriginInPixels(),
        rLogicRect.IsWidthEmpty()
            ? 0
            : Geometry::LogicToPixel(rLogicRect.Right() + aMapMetrics.mnMappingXOffset, GetDPIX(),
                                     aMapMetrics.mnMapScalingXNumerator,
                                     aMapMetrics.mnMapScalingXDenominator)
                  + maGeometry.GetXOffsetFromOriginInPixels(),
        rLogicRect.IsHeightEmpty()
            ? 0
            : Geometry::LogicToPixel(rLogicRect.Bottom() + aMapMetrics.mnMappingYOffset, GetDPIY(),
                                     aMapMetrics.mnMapScalingYNumerator,
                                     aMapMetrics.mnMapScalingYDenominator)
                  + maGeometry.GetYOffsetFromOriginInPixels());

    if (rLogicRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if (rLogicRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Polygon OutputDevice::LogicToPixel(const tools::Polygon& rLogicPoly,
                                          const MapMode& rMapMode) const
{
    if (rMapMode.IsDefault())
        return rLogicPoly;

    // convert MapMode resolution and convert
    MappingMetrics aMapMetrics;
    aMapMetrics.CalculateMappingResolution(rMapMode, GetDPIX(), GetDPIY());

    sal_uInt16 i;
    sal_uInt16 nPoints = rLogicPoly.GetSize();
    tools::Polygon aPoly(rLogicPoly);

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    for (i = 0; i < nPoints; i++)
    {
        const Point* pPt = &(pPointAry[i]);
        Point aPt;
        aPt.setX(Geometry::LogicToPixel(pPt->X() + aMapMetrics.mnMappingXOffset, GetDPIX(),
                                        aMapMetrics.mnMapScalingXNumerator,
                                        aMapMetrics.mnMapScalingXDenominator)
                 + maGeometry.GetXOffsetFromOriginInPixels());
        aPt.setY(Geometry::LogicToPixel(pPt->Y() + aMapMetrics.mnMappingYOffset, GetDPIY(),
                                        aMapMetrics.mnMapScalingYNumerator,
                                        aMapMetrics.mnMapScalingYDenominator)
                 + maGeometry.GetYOffsetFromOriginInPixels());
        aPoly[i] = aPt;
    }

    return aPoly;
}

basegfx::B2DPolyPolygon OutputDevice::LogicToPixel(const basegfx::B2DPolyPolygon& rLogicPolyPoly,
                                                   const MapMode& rMapMode) const
{
    basegfx::B2DPolyPolygon aTransformedPoly = rLogicPolyPoly;
    const basegfx::B2DHomMatrix& rTransformationMatrix = GetViewTransformation(rMapMode);
    aTransformedPoly.transform(rTransformationMatrix);
    return aTransformedPoly;
}

Point OutputDevice::PixelToLogic(const Point& rDevicePt) const
{
    if (!IsMapModeEnabled())
        return rDevicePt;

    return Point(
        Geometry::PixelToLogic(rDevicePt.X(), GetDPIX(), maGeometry.GetMapScalingXNumerator(),
                               maGeometry.GetMapScalingXDenominator())
            - maGeometry.GetMappingOffset().X() - maGeometry.GetXOffsetFromOriginInLogicalUnits(),
        Geometry::PixelToLogic(rDevicePt.Y(), GetDPIY(), maGeometry.GetMapScalingYNumerator(),
                               maGeometry.GetMapScalingYDenominator())
            - maGeometry.GetMappingOffset().Y() - maGeometry.GetYOffsetFromOriginInLogicalUnits());
}

Size OutputDevice::PixelToLogic(const Size& rDeviceSize) const
{
    if (!IsMapModeEnabled())
        return rDeviceSize;

    return Size(Geometry::PixelToLogic(rDeviceSize.Width(), GetDPIX(),
                                       maGeometry.GetMapScalingXNumerator(),
                                       maGeometry.GetMapScalingXDenominator()),
                Geometry::PixelToLogic(rDeviceSize.Height(), GetDPIY(),
                                       maGeometry.GetMapScalingYNumerator(),
                                       maGeometry.GetMapScalingYDenominator()));
}

tools::Rectangle OutputDevice::PixelToLogic(const tools::Rectangle& rDeviceRect) const
{
    // tdf#141761 see comments above, IsEmpty() removed
    if (!IsMapModeEnabled())
        return rDeviceRect;

    tools::Rectangle aRetval(
        Geometry::PixelToLogic(rDeviceRect.Left(), GetDPIX(), maGeometry.GetMapScalingXNumerator(),
                               maGeometry.GetMapScalingXDenominator())
            - maGeometry.GetMappingOffset().X() - maGeometry.GetXOffsetFromOriginInLogicalUnits(),
        Geometry::PixelToLogic(rDeviceRect.Top(), GetDPIY(), maGeometry.GetMapScalingYNumerator(),
                               maGeometry.GetMapScalingYDenominator())
            - maGeometry.GetMappingOffset().Y() - maGeometry.GetYOffsetFromOriginInLogicalUnits(),
        rDeviceRect.IsWidthEmpty() ? 0
                                   : Geometry::PixelToLogic(rDeviceRect.Right(), GetDPIX(),
                                                            maGeometry.GetMapScalingXNumerator(),
                                                            maGeometry.GetMapScalingXDenominator())
                                         - maGeometry.GetMappingOffset().X()
                                         - maGeometry.GetXOffsetFromOriginInLogicalUnits(),
        rDeviceRect.IsHeightEmpty() ? 0
                                    : Geometry::PixelToLogic(rDeviceRect.Bottom(), GetDPIY(),
                                                             maGeometry.GetMapScalingYNumerator(),
                                                             maGeometry.GetMapScalingYDenominator())
                                          - maGeometry.GetMappingOffset().Y()
                                          - maGeometry.GetYOffsetFromOriginInLogicalUnits());

    if (rDeviceRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if (rDeviceRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Polygon OutputDevice::PixelToLogic(const tools::Polygon& rDevicePoly) const
{
    if (!IsMapModeEnabled())
        return rDevicePoly;

    sal_uInt16 i;
    sal_uInt16 nPoints = rDevicePoly.GetSize();
    tools::Polygon aPoly(rDevicePoly);

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    for (i = 0; i < nPoints; i++)
    {
        const Point* pPt = &(pPointAry[i]);
        Point aPt;
        aPt.setX(Geometry::PixelToLogic(pPt->X(), GetDPIX(), maGeometry.GetMapScalingXNumerator(),
                                        maGeometry.GetMapScalingXDenominator())
                 - maGeometry.GetMappingOffset().X()
                 - maGeometry.GetXOffsetFromOriginInLogicalUnits());
        aPt.setY(Geometry::PixelToLogic(pPt->Y(), GetDPIY(), maGeometry.GetMapScalingYNumerator(),
                                        maGeometry.GetMapScalingYDenominator())
                 - maGeometry.GetMappingOffset().Y()
                 - maGeometry.GetYOffsetFromOriginInLogicalUnits());
        aPoly[i] = aPt;
    }

    return aPoly;
}

tools::PolyPolygon OutputDevice::PixelToLogic(const tools::PolyPolygon& rDevicePolyPoly) const
{
    if (!IsMapModeEnabled())
        return rDevicePolyPoly;

    tools::PolyPolygon aPolyPoly(rDevicePolyPoly);
    sal_uInt16 nPoly = aPolyPoly.Count();
    for (sal_uInt16 i = 0; i < nPoly; i++)
    {
        tools::Polygon& rPoly = aPolyPoly[i];
        rPoly = PixelToLogic(rPoly);
    }
    return aPolyPoly;
}

basegfx::B2DPolyPolygon
OutputDevice::PixelToLogic(const basegfx::B2DPolyPolygon& rPixelPolyPoly) const
{
    basegfx::B2DPolyPolygon aTransformedPoly = rPixelPolyPoly;
    const basegfx::B2DHomMatrix& rTransformationMatrix = GetInverseViewTransformation();
    aTransformedPoly.transform(rTransformationMatrix);
    return aTransformedPoly;
}

vcl::Region OutputDevice::PixelToLogic(const vcl::Region& rDeviceRegion) const
{
    if (!IsMapModeEnabled() || rDeviceRegion.IsNull() || rDeviceRegion.IsEmpty())
    {
        return rDeviceRegion;
    }

    vcl::Region aRegion;

    if (rDeviceRegion.getB2DPolyPolygon())
    {
        aRegion = vcl::Region(PixelToLogic(*rDeviceRegion.getB2DPolyPolygon()));
    }
    else if (rDeviceRegion.getPolyPolygon())
    {
        aRegion = vcl::Region(PixelToLogic(*rDeviceRegion.getPolyPolygon()));
    }
    else if (rDeviceRegion.getRegionBand())
    {
        RectangleVector aRectangles;
        rDeviceRegion.GetRegionRectangles(aRectangles);
        const RectangleVector& rRectangles(aRectangles); // needed to make the '!=' work

        // make reverse run to fill new region bottom-up, this will speed it up due to the used data structuring
        for (RectangleVector::const_reverse_iterator aRectIter(rRectangles.rbegin());
             aRectIter != rRectangles.rend(); ++aRectIter)
        {
            aRegion.Union(PixelToLogic(*aRectIter));
        }
    }

    return aRegion;
}

Point OutputDevice::PixelToLogic(const Point& rDevicePt, const MapMode& rMapMode) const
{
    // calculate nothing if default-MapMode
    if (rMapMode.IsDefault())
        return rDevicePt;

    // calculate MapMode-resolution and convert
    MappingMetrics aMapMetrics;
    aMapMetrics.CalculateMappingResolution(rMapMode, GetDPIX(), GetDPIY());

    return Point(
        Geometry::PixelToLogic(rDevicePt.X(), GetDPIX(), aMapMetrics.mnMapScalingXNumerator,
                               aMapMetrics.mnMapScalingXDenominator)
            - aMapMetrics.mnMappingXOffset - maGeometry.GetXOffsetFromOriginInLogicalUnits(),
        Geometry::PixelToLogic(rDevicePt.Y(), GetDPIY(), aMapMetrics.mnMapScalingYNumerator,
                               aMapMetrics.mnMapScalingYDenominator)
            - aMapMetrics.mnMappingYOffset - maGeometry.GetYOffsetFromOriginInLogicalUnits());
}

Size OutputDevice::PixelToLogic(const Size& rDeviceSize, const MapMode& rMapMode) const
{
    // calculate nothing if default-MapMode
    if (rMapMode.IsDefault())
        return rDeviceSize;

    // calculate MapMode-resolution and convert
    MappingMetrics aMapMetrics;
    aMapMetrics.CalculateMappingResolution(rMapMode, GetDPIX(), GetDPIY());

    return Size(
        Geometry::PixelToLogic(rDeviceSize.Width(), GetDPIX(), aMapMetrics.mnMapScalingXNumerator,
                               aMapMetrics.mnMapScalingXDenominator),
        Geometry::PixelToLogic(rDeviceSize.Height(), GetDPIY(), aMapMetrics.mnMapScalingYNumerator,
                               aMapMetrics.mnMapScalingYDenominator));
}

tools::Rectangle OutputDevice::PixelToLogic(const tools::Rectangle& rDeviceRect,
                                            const MapMode& rMapMode) const
{
    // calculate nothing if default-MapMode
    // tdf#141761 see comments above, IsEmpty() removed
    if (rMapMode.IsDefault())
        return rDeviceRect;

    // calculate MapMode-resolution and convert
    MappingMetrics aMapMetrics;
    aMapMetrics.CalculateMappingResolution(rMapMode, GetDPIX(), GetDPIY());

    tools::Rectangle aRetval(
        Geometry::PixelToLogic(rDeviceRect.Left(), GetDPIX(), aMapMetrics.mnMapScalingXNumerator,
                               aMapMetrics.mnMapScalingXDenominator)
            - aMapMetrics.mnMappingXOffset - maGeometry.GetXOffsetFromOriginInLogicalUnits(),
        Geometry::PixelToLogic(rDeviceRect.Top(), GetDPIY(), aMapMetrics.mnMapScalingYNumerator,
                               aMapMetrics.mnMapScalingYDenominator)
            - aMapMetrics.mnMappingYOffset - maGeometry.GetYOffsetFromOriginInLogicalUnits(),
        rDeviceRect.IsWidthEmpty()
            ? 0
            : Geometry::PixelToLogic(rDeviceRect.Right(), GetDPIX(),
                                     aMapMetrics.mnMapScalingXNumerator,
                                     aMapMetrics.mnMapScalingXDenominator)
                  - aMapMetrics.mnMappingXOffset - maGeometry.GetXOffsetFromOriginInLogicalUnits(),
        rDeviceRect.IsHeightEmpty()
            ? 0
            : Geometry::PixelToLogic(rDeviceRect.Bottom(), GetDPIY(),
                                     aMapMetrics.mnMapScalingYNumerator,
                                     aMapMetrics.mnMapScalingYDenominator)
                  - aMapMetrics.mnMappingYOffset - maGeometry.GetYOffsetFromOriginInLogicalUnits());

    if (rDeviceRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if (rDeviceRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Polygon OutputDevice::PixelToLogic(const tools::Polygon& rDevicePoly,
                                          const MapMode& rMapMode) const
{
    // calculate nothing if default-MapMode
    if (rMapMode.IsDefault())
        return rDevicePoly;

    // calculate MapMode-resolution and convert
    MappingMetrics aMapMetrics;
    aMapMetrics.CalculateMappingResolution(rMapMode, GetDPIX(), GetDPIY());

    sal_uInt16 i;
    sal_uInt16 nPoints = rDevicePoly.GetSize();
    tools::Polygon aPoly(rDevicePoly);

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    for (i = 0; i < nPoints; i++)
    {
        const Point* pPt = &(pPointAry[i]);
        Point aPt;
        aPt.setX(Geometry::PixelToLogic(pPt->X(), GetDPIX(), aMapMetrics.mnMapScalingXNumerator,
                                        aMapMetrics.mnMapScalingXDenominator)
                 - aMapMetrics.mnMappingXOffset - maGeometry.GetXOffsetFromOriginInLogicalUnits());
        aPt.setY(Geometry::PixelToLogic(pPt->Y(), GetDPIY(), aMapMetrics.mnMapScalingYNumerator,
                                        aMapMetrics.mnMapScalingYDenominator)
                 - aMapMetrics.mnMappingYOffset - maGeometry.GetYOffsetFromOriginInLogicalUnits());
        aPoly[i] = aPt;
    }

    return aPoly;
}

basegfx::B2DPolygon OutputDevice::PixelToLogic(const basegfx::B2DPolygon& rPixelPoly,
                                               const MapMode& rMapMode) const
{
    basegfx::B2DPolygon aTransformedPoly = rPixelPoly;
    const basegfx::B2DHomMatrix& rTransformationMatrix = GetInverseViewTransformation(rMapMode);
    aTransformedPoly.transform(rTransformationMatrix);
    return aTransformedPoly;
}

basegfx::B2DPolyPolygon OutputDevice::PixelToLogic(const basegfx::B2DPolyPolygon& rPixelPolyPoly,
                                                   const MapMode& rMapMode) const
{
    basegfx::B2DPolyPolygon aTransformedPoly = rPixelPolyPoly;
    const basegfx::B2DHomMatrix& rTransformationMatrix = GetInverseViewTransformation(rMapMode);
    aTransformedPoly.transform(rTransformationMatrix);
    return aTransformedPoly;
}

static void verifyUnitSourceDest(MapUnit eUnitSource, MapUnit eUnitDest)
{
    DBG_ASSERT(eUnitSource != MapUnit::MapSysFont && eUnitSource != MapUnit::MapAppFont
                   && eUnitSource != MapUnit::MapRelative,
               "Source MapUnit is not permitted");
    DBG_ASSERT(eUnitDest != MapUnit::MapSysFont && eUnitDest != MapUnit::MapAppFont
                   && eUnitDest != MapUnit::MapRelative,
               "Destination MapUnit is not permitted");
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
}

// return (n1 * n2 * n3) / (n4 * n5)
static tools::Long fn5(const tools::Long n1, const tools::Long n2, const tools::Long n3,
                       const tools::Long n4, const tools::Long n5)
{
    if (n1 == 0 || n2 == 0 || n3 == 0 || n4 == 0 || n5 == 0)
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
            if (a7.IsNeg())
                a7 -= a9;
            else
                a7 += a9;

            a7 /= a8;
        } // of if
        else
        {
            tools::Long n8 = n4 * n5;

            if (a7.IsNeg())
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
                if (a7.IsNeg())
                    a7 -= a9;
                else
                    a7 += a9;

                a7 /= a8;
            } // of if
            else
            {
                tools::Long n8 = n4 * n5;

                if (a7.IsNeg())
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
                if (a7.IsNeg())
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

                if (n7 < 0)
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

static tools::Long fn3(const tools::Long n1, const o3tl::Length eFrom, const o3tl::Length eTo)
{
    if (n1 == 0 || eFrom == o3tl::Length::invalid || eTo == o3tl::Length::invalid)
        return 0;
    bool bOverflow;
    const auto nResult = o3tl::convert(n1, eFrom, eTo, bOverflow);
    if (bOverflow)
    {
        const auto & [ n2, n3 ] = o3tl::getConversionMulDiv(eFrom, eTo);
        BigInt a4 = n1;
        a4 *= n2;

        if (a4.IsNeg())
            a4 -= n3 / 2;
        else
            a4 += n3 / 2;

        a4 /= n3;
        return static_cast<tools::Long>(a4);
    } // of if
    else
        return nResult;
}

Point OutputDevice::LogicToLogic(const Point& rPtSource, const MapMode* pMapModeSource,
                                 const MapMode* pMapModeDest) const
{
    if (!pMapModeSource)
        pMapModeSource = &maMapMode;

    if (!pMapModeDest)
        pMapModeDest = &maMapMode;

    if (*pMapModeSource == *pMapModeDest)
        return rPtSource;

    MappingMetrics aMapMetricsSource;
    MappingMetrics aMapMetricsDest;

    if (!IsMapModeEnabled() || pMapModeSource != &maMapMode)
    {
        if (pMapModeSource->GetMapUnit() == MapUnit::MapRelative)
            aMapMetricsSource = maGeometry.GetMapMetrics();

        aMapMetricsSource.CalculateMappingResolution(*pMapModeSource, GetDPIX(), GetDPIY());
    }
    else
    {
        aMapMetricsSource = maGeometry.GetMapMetrics();
    }

    if (!IsMapModeEnabled() || pMapModeDest != &maMapMode)
    {
        if (pMapModeDest->GetMapUnit() == MapUnit::MapRelative)
            aMapMetricsSource = maGeometry.GetMapMetrics();

        aMapMetricsDest.CalculateMappingResolution(*pMapModeDest, GetDPIX(), GetDPIY());
    }
    else
    {
        aMapMetricsDest = maGeometry.GetMapMetrics();
    }

    return Point(
        fn5(rPtSource.X() + aMapMetricsSource.mnMappingXOffset,
            aMapMetricsSource.mnMapScalingXNumerator, aMapMetricsDest.mnMapScalingXDenominator,
            aMapMetricsSource.mnMapScalingXDenominator, aMapMetricsDest.mnMapScalingXNumerator)
            - aMapMetricsDest.mnMappingXOffset,
        fn5(rPtSource.Y() + aMapMetricsSource.mnMappingYOffset,
            aMapMetricsSource.mnMapScalingYNumerator, aMapMetricsDest.mnMapScalingYDenominator,
            aMapMetricsSource.mnMapScalingYDenominator, aMapMetricsDest.mnMapScalingYNumerator)
            - aMapMetricsDest.mnMappingYOffset);
}

Size OutputDevice::LogicToLogic(const Size& rSzSource, const MapMode* pMapModeSource,
                                const MapMode* pMapModeDest) const
{
    if (!pMapModeSource)
        pMapModeSource = &maMapMode;

    if (!pMapModeDest)
        pMapModeDest = &maMapMode;

    if (*pMapModeSource == *pMapModeDest)
        return rSzSource;

    MappingMetrics aMapMetricsSource;
    MappingMetrics aMapMetricsDest;

    if (!IsMapModeEnabled() || pMapModeSource != &maMapMode)
    {
        if (pMapModeSource->GetMapUnit() == MapUnit::MapRelative)
            aMapMetricsSource = maGeometry.GetMapMetrics();

        aMapMetricsSource.CalculateMappingResolution(*pMapModeSource, GetDPIX(), GetDPIY());
    }
    else
    {
        aMapMetricsSource = maGeometry.GetMapMetrics();
    }

    if (!IsMapModeEnabled() || pMapModeDest != &maMapMode)
    {
        if (pMapModeDest->GetMapUnit() == MapUnit::MapRelative)
            aMapMetricsSource = maGeometry.GetMapMetrics();

        aMapMetricsDest.CalculateMappingResolution(*pMapModeDest, GetDPIX(), GetDPIY());
    }
    else
    {
        aMapMetricsDest = maGeometry.GetMapMetrics();
    }

    return Size(
        fn5(rSzSource.Width(), aMapMetricsSource.mnMapScalingXNumerator,
            aMapMetricsDest.mnMapScalingXDenominator, aMapMetricsSource.mnMapScalingXDenominator,
            aMapMetricsDest.mnMapScalingXNumerator),
        fn5(rSzSource.Height(), aMapMetricsSource.mnMapScalingYNumerator,
            aMapMetricsDest.mnMapScalingYDenominator, aMapMetricsSource.mnMapScalingYDenominator,
            aMapMetricsDest.mnMapScalingYNumerator));
}

tools::Rectangle OutputDevice::LogicToLogic(const tools::Rectangle& rRectSource,
                                            const MapMode* pMapModeSource,
                                            const MapMode* pMapModeDest) const
{
    if (!pMapModeSource)
        pMapModeSource = &maMapMode;

    if (!pMapModeDest)
        pMapModeDest = &maMapMode;

    if (*pMapModeSource == *pMapModeDest)
        return rRectSource;

    MappingMetrics aMapMetricsSource;
    MappingMetrics aMapMetricsDest;

    if (!IsMapModeEnabled() || pMapModeSource != &maMapMode)
    {
        if (pMapModeSource->GetMapUnit() == MapUnit::MapRelative)
            aMapMetricsSource = maGeometry.GetMapMetrics();

        aMapMetricsSource.CalculateMappingResolution(*pMapModeSource, GetDPIX(), GetDPIY());
    }
    else
    {
        aMapMetricsSource = maGeometry.GetMapMetrics();
    }

    if (!IsMapModeEnabled() || pMapModeDest != &maMapMode)
    {
        if (pMapModeDest->GetMapUnit() == MapUnit::MapRelative)
            aMapMetricsSource = maGeometry.GetMapMetrics();

        aMapMetricsDest.CalculateMappingResolution(*pMapModeDest, GetDPIX(), GetDPIY());
    }
    else
    {
        aMapMetricsDest = maGeometry.GetMapMetrics();
    }

    return tools::Rectangle(
        fn5(rRectSource.Left() + aMapMetricsSource.mnMappingXOffset,
            aMapMetricsSource.mnMapScalingXNumerator, aMapMetricsDest.mnMapScalingXDenominator,
            aMapMetricsSource.mnMapScalingXDenominator, aMapMetricsDest.mnMapScalingXNumerator)
            - aMapMetricsDest.mnMappingXOffset,
        fn5(rRectSource.Top() + aMapMetricsSource.mnMappingYOffset,
            aMapMetricsSource.mnMapScalingYNumerator, aMapMetricsDest.mnMapScalingYDenominator,
            aMapMetricsSource.mnMapScalingYDenominator, aMapMetricsDest.mnMapScalingYNumerator)
            - aMapMetricsDest.mnMappingYOffset,
        fn5(rRectSource.Right() + aMapMetricsSource.mnMappingXOffset,
            aMapMetricsSource.mnMapScalingXNumerator, aMapMetricsDest.mnMapScalingXDenominator,
            aMapMetricsSource.mnMapScalingXDenominator, aMapMetricsDest.mnMapScalingXNumerator)
            - aMapMetricsDest.mnMappingXOffset,
        fn5(rRectSource.Bottom() + aMapMetricsSource.mnMappingYOffset,
            aMapMetricsSource.mnMapScalingYNumerator, aMapMetricsDest.mnMapScalingYDenominator,
            aMapMetricsSource.mnMapScalingYDenominator, aMapMetricsDest.mnMapScalingYNumerator)
            - aMapMetricsDest.mnMappingYOffset);
}

Point OutputDevice::LogicToLogic(const Point& rPtSource, const MapMode& rMapModeSource,
                                 const MapMode& rMapModeDest)
{
    if (rMapModeSource == rMapModeDest)
        return rPtSource;

    MapUnit eUnitSource = rMapModeSource.GetMapUnit();
    MapUnit eUnitDest = rMapModeDest.GetMapUnit();
    verifyUnitSourceDest(eUnitSource, eUnitDest);

    if (rMapModeSource.IsSimple() && rMapModeDest.IsSimple())
    {
        const auto & [ eFrom, eTo ] = getCorrectedUnit(eUnitSource, eUnitDest);
        return Point(fn3(rPtSource.X(), eFrom, eTo), fn3(rPtSource.Y(), eFrom, eTo));
    }
    else
    {
        MappingMetrics aMapMetricsSource;
        MappingMetrics aMapMetricsDest;

        aMapMetricsSource.CalculateMappingResolution(rMapModeSource, 72, 72);
        aMapMetricsDest.CalculateMappingResolution(rMapModeDest, 72, 72);

        return Point(
            fn5(rPtSource.X() + aMapMetricsSource.mnMappingXOffset,
                aMapMetricsSource.mnMapScalingXNumerator, aMapMetricsDest.mnMapScalingXDenominator,
                aMapMetricsSource.mnMapScalingXDenominator, aMapMetricsDest.mnMapScalingXNumerator)
                - aMapMetricsDest.mnMappingXOffset,
            fn5(rPtSource.Y() + aMapMetricsSource.mnMappingYOffset,
                aMapMetricsSource.mnMapScalingYNumerator, aMapMetricsDest.mnMapScalingYDenominator,
                aMapMetricsSource.mnMapScalingYDenominator, aMapMetricsDest.mnMapScalingYNumerator)
                - aMapMetricsDest.mnMappingYOffset);
    }
}

Size OutputDevice::LogicToLogic(const Size& rSzSource, const MapMode& rMapModeSource,
                                const MapMode& rMapModeDest)
{
    if (rMapModeSource == rMapModeDest)
        return rSzSource;

    MapUnit eUnitSource = rMapModeSource.GetMapUnit();
    MapUnit eUnitDest = rMapModeDest.GetMapUnit();
    verifyUnitSourceDest(eUnitSource, eUnitDest);

    if (rMapModeSource.IsSimple() && rMapModeDest.IsSimple())
    {
        const auto & [ eFrom, eTo ] = getCorrectedUnit(eUnitSource, eUnitDest);
        return Size(fn3(rSzSource.Width(), eFrom, eTo), fn3(rSzSource.Height(), eFrom, eTo));
    }
    else
    {
        MappingMetrics aMapMetricsSource;
        MappingMetrics aMapMetricsDest;

        aMapMetricsSource.CalculateMappingResolution(rMapModeSource, 72, 72);
        aMapMetricsDest.CalculateMappingResolution(rMapModeDest, 72, 72);

        return Size(fn5(rSzSource.Width(), aMapMetricsSource.mnMapScalingXNumerator,
                        aMapMetricsDest.mnMapScalingXDenominator,
                        aMapMetricsSource.mnMapScalingXDenominator,
                        aMapMetricsDest.mnMapScalingXNumerator),
                    fn5(rSzSource.Height(), aMapMetricsSource.mnMapScalingYNumerator,
                        aMapMetricsDest.mnMapScalingYDenominator,
                        aMapMetricsSource.mnMapScalingYDenominator,
                        aMapMetricsDest.mnMapScalingYNumerator));
    }
}

basegfx::B2DPolygon OutputDevice::LogicToLogic(const basegfx::B2DPolygon& rPolySource,
                                               const MapMode& rMapModeSource,
                                               const MapMode& rMapModeDest)
{
    if (rMapModeSource == rMapModeDest)
    {
        return rPolySource;
    }

    const basegfx::B2DHomMatrix aTransform(LogicToLogic(rMapModeSource, rMapModeDest));
    basegfx::B2DPolygon aPoly(rPolySource);

    aPoly.transform(aTransform);
    return aPoly;
}

basegfx::B2DHomMatrix OutputDevice::LogicToLogic(const MapMode& rMapModeSource,
                                                 const MapMode& rMapModeDest)
{
    basegfx::B2DHomMatrix aTransform;

    if (rMapModeSource == rMapModeDest)
    {
        return aTransform;
    }

    MapUnit eUnitSource = rMapModeSource.GetMapUnit();
    MapUnit eUnitDest = rMapModeDest.GetMapUnit();
    verifyUnitSourceDest(eUnitSource, eUnitDest);

    if (rMapModeSource.IsSimple() && rMapModeDest.IsSimple())
    {
        const auto & [ eFrom, eTo ] = getCorrectedUnit(eUnitSource, eUnitDest);
        const double fScaleFactor(eFrom == o3tl::Length::invalid || eTo == o3tl::Length::invalid
                                      ? std::numeric_limits<double>::quiet_NaN()
                                      : o3tl::convert(1.0, eFrom, eTo));
        aTransform.set(0, 0, fScaleFactor);
        aTransform.set(1, 1, fScaleFactor);
    }
    else
    {
        MappingMetrics aMapMetricsSource;
        MappingMetrics aMapMetricsDest;

        aMapMetricsSource.CalculateMappingResolution(rMapModeSource, 72, 72);
        aMapMetricsDest.CalculateMappingResolution(rMapModeDest, 72, 72);

        const double fScaleFactorX((double(aMapMetricsSource.mnMapScalingXNumerator)
                                    * double(aMapMetricsDest.mnMapScalingXDenominator))
                                   / (double(aMapMetricsSource.mnMapScalingXDenominator)
                                      * double(aMapMetricsDest.mnMapScalingXNumerator)));
        const double fScaleFactorY((double(aMapMetricsSource.mnMapScalingYNumerator)
                                    * double(aMapMetricsDest.mnMapScalingYDenominator))
                                   / (double(aMapMetricsSource.mnMapScalingYDenominator)
                                      * double(aMapMetricsDest.mnMapScalingYNumerator)));
        const double fZeroPointX(double(aMapMetricsSource.mnMappingXOffset) * fScaleFactorX
                                 - double(aMapMetricsDest.mnMappingXOffset));
        const double fZeroPointY(double(aMapMetricsSource.mnMappingYOffset) * fScaleFactorY
                                 - double(aMapMetricsDest.mnMappingYOffset));

        aTransform.set(0, 0, fScaleFactorX);
        aTransform.set(1, 1, fScaleFactorY);
        aTransform.set(0, 2, fZeroPointX);
        aTransform.set(1, 2, fZeroPointY);
    }

    return aTransform;
}

tools::Rectangle OutputDevice::LogicToLogic(const tools::Rectangle& rRectSource,
                                            const MapMode& rMapModeSource,
                                            const MapMode& rMapModeDest)
{
    if (rMapModeSource == rMapModeDest)
        return rRectSource;

    MapUnit eUnitSource = rMapModeSource.GetMapUnit();
    MapUnit eUnitDest = rMapModeDest.GetMapUnit();
    verifyUnitSourceDest(eUnitSource, eUnitDest);

    tools::Rectangle aRetval;

    if (rMapModeSource.IsSimple() && rMapModeDest.IsSimple())
    {
        const auto & [ eFrom, eTo ] = getCorrectedUnit(eUnitSource, eUnitDest);

        auto left = fn3(rRectSource.Left(), eFrom, eTo);
        auto top = fn3(rRectSource.Top(), eFrom, eTo);

        // tdf#141761 see comments above, IsEmpty() removed
        auto right = rRectSource.IsWidthEmpty() ? 0 : fn3(rRectSource.Right(), eFrom, eTo);
        auto bottom = rRectSource.IsHeightEmpty() ? 0 : fn3(rRectSource.Bottom(), eFrom, eTo);

        aRetval = tools::Rectangle(left, top, right, bottom);
    }
    else
    {
        MappingMetrics aMapMetricsSource;
        MappingMetrics aMapMetricsDest;

        aMapMetricsSource.CalculateMappingResolution(rMapModeSource, 72, 72);
        aMapMetricsDest.CalculateMappingResolution(rMapModeDest, 72, 72);

        auto left = fn5(rRectSource.Left() + aMapMetricsSource.mnMappingXOffset,
                        aMapMetricsSource.mnMapScalingXNumerator,
                        aMapMetricsDest.mnMapScalingXDenominator,
                        aMapMetricsSource.mnMapScalingXDenominator,
                        aMapMetricsDest.mnMapScalingXNumerator)
                    - aMapMetricsDest.mnMappingXOffset;
        auto top = fn5(rRectSource.Top() + aMapMetricsSource.mnMappingYOffset,
                       aMapMetricsSource.mnMapScalingYNumerator,
                       aMapMetricsDest.mnMapScalingYDenominator,
                       aMapMetricsSource.mnMapScalingYDenominator,
                       aMapMetricsDest.mnMapScalingYNumerator)
                   - aMapMetricsDest.mnMappingYOffset;

        // tdf#141761 see comments above, IsEmpty() removed
        auto right = rRectSource.IsWidthEmpty()
                         ? 0
                         : fn5(rRectSource.Right() + aMapMetricsSource.mnMappingXOffset,
                               aMapMetricsSource.mnMapScalingXNumerator,
                               aMapMetricsDest.mnMapScalingXDenominator,
                               aMapMetricsSource.mnMapScalingXDenominator,
                               aMapMetricsDest.mnMapScalingXNumerator)
                               - aMapMetricsDest.mnMappingXOffset;
        auto bottom = rRectSource.IsHeightEmpty()
                          ? 0
                          : fn5(rRectSource.Bottom() + aMapMetricsSource.mnMappingYOffset,
                                aMapMetricsSource.mnMapScalingYNumerator,
                                aMapMetricsDest.mnMapScalingYDenominator,
                                aMapMetricsSource.mnMapScalingYDenominator,
                                aMapMetricsDest.mnMapScalingYNumerator)
                                - aMapMetricsDest.mnMappingYOffset;

        aRetval = tools::Rectangle(left, top, right, bottom);
    }

    if (rRectSource.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if (rRectSource.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Long OutputDevice::LogicToLogic(tools::Long nLongSource, MapUnit eUnitSource,
                                       MapUnit eUnitDest)
{
    if (eUnitSource == eUnitDest)
        return nLongSource;

    verifyUnitSourceDest(eUnitSource, eUnitDest);
    const auto & [ eFrom, eTo ] = getCorrectedUnit(eUnitSource, eUnitDest);
    return fn3(nLongSource, eFrom, eTo);
}

void OutputDevice::SetPixelOffset(const Size& rOffset)
{
    maGeometry.SetXOffsetFromOriginInPixels(rOffset.Width());
    maGeometry.SetYOffsetFromOriginInPixels(rOffset.Height());

    maGeometry.SetXOffsetFromOriginInLogicalUnits(Geometry::PixelToLogic(
        maGeometry.GetXOffsetFromOriginInPixels(), GetDPIX(), maGeometry.GetMapScalingXNumerator(),
        maGeometry.GetMapScalingXDenominator()));
    maGeometry.SetYOffsetFromOriginInLogicalUnits(Geometry::PixelToLogic(
        maGeometry.GetYOffsetFromOriginInPixels(), GetDPIY(), maGeometry.GetMapScalingYNumerator(),
        maGeometry.GetMapScalingYDenominator()));

    if (mpAlphaVDev)
        mpAlphaVDev->SetPixelOffset(rOffset);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
