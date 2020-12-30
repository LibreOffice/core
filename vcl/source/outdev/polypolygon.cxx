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
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <tools/poly.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <salgdi.hxx>
#include <polypolygon.hxx>

#include <memory>
#include <cassert>

void OutputDevice::DrawPolyPolygon(const tools::PolyPolygon& rPolyPoly)
{
    assert(!is_double_buffered_window());

    if (mpMetaFile)
        mpMetaFile->AddAction(new MetaPolyPolygonAction(rPolyPoly));

    sal_uInt16 nPoly = rPolyPoly.Count();

    if (!IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || !nPoly
        || ImplIsRecordLayout())
        return;

    // we need a graphics
    if (!mpGraphics && !AcquireGraphics())
        return;

    if (mbInitClipRegion)
        InitClipRegion();

    if (mbOutputClipped)
        return;

    if (mbInitLineColor)
        InitLineColor();

    if (mbInitFillColor)
        InitFillColor();

    // use b2dpolygon drawing if possible
    if (mpGraphics->supportsOperation(OutDevSupportType::B2DDraw)
        && RasterOp::OverPaint == GetRasterOp() && (IsLineColor() || IsFillColor()))
    {
        const basegfx::B2DHomMatrix aTransform(ImplGetDeviceTransformation());
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rPolyPoly.getB2DPolyPolygon());

        // ensure closed - may be asserted, will prevent buffering
        if (!aB2DPolyPolygon.isClosed())
            aB2DPolyPolygon.setClosed(true);

        if (IsFillColor())
            mpGraphics->DrawPolyPolygon(aTransform, aB2DPolyPolygon, 0.0, *this);

        if (IsLineColor())
        {
            const bool bPixelSnapHairline(mnAntialiasing & AntialiasingFlags::PixelSnapHairline);

            for (auto const& rPolygon : aB2DPolyPolygon)
            {
                mpGraphics->DrawPolyLine(
                    aTransform, rPolygon, 0.0,
                    0.0, // tdf#124848 hairline
                    nullptr, // MM01
                    basegfx::B2DLineJoin::NONE, css::drawing::LineCap_BUTT,
                    basegfx::deg2rad(
                        15.0), // not used with B2DLineJoin::NONE, but the correct default
                    bPixelSnapHairline, *this);
            }
        }

        if (mpAlphaVDev)
            mpAlphaVDev->DrawPolyPolygon(rPolyPoly);

        return;
    }

    if (nPoly == 1)
    {
        // #100127# Map to DrawPolygon
        const tools::Polygon& aPoly = rPolyPoly.GetObject(0);
        if (aPoly.GetSize() >= 2)
        {
            GDIMetaFile* pOldMF = mpMetaFile;
            mpMetaFile = nullptr;

            DrawPolygon(aPoly);

            mpMetaFile = pOldMF;
        }
    }
    else
    {
        // #100127# moved real tools::PolyPolygon draw to separate method,
        // have to call recursively, avoiding duplicate
        // ImplLogicToDevicePixel calls
        mpGraphics->DrawPolyPolygon(nPoly, ImplLogicToDevicePixel(rPolyPoly), *this);
    }

    if (mpAlphaVDev)
        mpAlphaVDev->DrawPolyPolygon(rPolyPoly);
}

// Caution: This method is nearly the same as
// OutputDevice::DrawTransparent( const basegfx::B2DPolyPolygon& rB2DPolyPoly, double fTransparency),
// so when changes are made here do not forget to make changes there, too

void OutputDevice::DrawPolyPolygon(const basegfx::B2DPolyPolygon& rB2DPolyPoly)
{
    assert(!is_double_buffered_window());

    if (mpMetaFile)
        mpMetaFile->AddAction(new MetaPolyPolygonAction(tools::PolyPolygon(rB2DPolyPoly)));

    // call helper
    ImplDrawPolyPolygonWithB2DPolyPolygon(rB2DPolyPoly);
}

void OutputDevice::ImplDrawPolyPolygonWithB2DPolyPolygon(
    const basegfx::B2DPolyPolygon& rB2DPolyPoly)
{
    // Do not paint empty PolyPolygons
    if (!rB2DPolyPoly.count() || !IsDeviceOutputNecessary())
        return;

    // we need a graphics
    if (!mpGraphics && !AcquireGraphics())
        return;

    if (mbInitClipRegion)
        InitClipRegion();

    if (mbOutputClipped)
        return;

    if (mbInitLineColor)
        InitLineColor();

    if (mbInitFillColor)
        InitFillColor();

    bool bSuccess(false);

    if (mpGraphics->supportsOperation(OutDevSupportType::B2DDraw)
        && RasterOp::OverPaint == GetRasterOp() && (IsLineColor() || IsFillColor()))
    {
        const basegfx::B2DHomMatrix aTransform(ImplGetDeviceTransformation());
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rB2DPolyPoly);

        // ensure closed - maybe assert, hinders buffering
        if (!aB2DPolyPolygon.isClosed())
            aB2DPolyPolygon.setClosed(true);

        if (IsFillColor())
            mpGraphics->DrawPolyPolygon(aTransform, aB2DPolyPolygon, 0.0, *this);

        if (bSuccess && IsLineColor())
        {
            const bool bPixelSnapHairline(mnAntialiasing & AntialiasingFlags::PixelSnapHairline);

            for (auto const& rPolygon : aB2DPolyPolygon)
            {
                mpGraphics->DrawPolyLine(
                    aTransform, rPolygon, 0.0,
                    0.0, // tdf#124848 hairline
                    nullptr, // MM01
                    basegfx::B2DLineJoin::NONE, css::drawing::LineCap_BUTT,
                    basegfx::deg2rad(
                        15.0), // not used with B2DLineJoin::NONE, but the correct default
                    bPixelSnapHairline, *this);
            }
        }
    }

    if (mpAlphaVDev)
        mpAlphaVDev->ImplDrawPolyPolygonWithB2DPolyPolygon(rB2DPolyPoly);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
