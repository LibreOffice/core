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

#include "polypolygon.hxx"

#include <memory>
#include <cassert>

void OutputDevice::DrawPolygon(const basegfx::B2DPolygon& rB2DPolygon)
{
    assert(!is_double_buffered_window());

    // AW: Do NOT paint empty polygons
    if (rB2DPolygon.count())
    {
        basegfx::B2DPolyPolygon aPP(rB2DPolygon);
        DrawPolyPolygon(aPP);
    }
}

void OutputDevice::DrawPolygon(const tools::Polygon& rPoly)
{
    assert(!is_double_buffered_window());

    if (mpMetaFile)
        mpMetaFile->AddAction(new MetaPolygonAction(rPoly));

    sal_uInt16 nPoints = rPoly.GetSize();

    if (!IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || (nPoints < 2)
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
        basegfx::B2DPolygon aB2DPolygon(rPoly.getB2DPolygon());
        bool bSuccess(true);

        // ensure closed - maybe assert, hinders buffering
        if (!aB2DPolygon.isClosed())
        {
            aB2DPolygon.setClosed(true);
        }

        if (IsFillColor())
        {
            bSuccess = mpGraphics->DrawPolyPolygon(aTransform, basegfx::B2DPolyPolygon(aB2DPolygon),
                                                   0.0, *this);
        }

        if (bSuccess && IsLineColor())
        {
            const bool bPixelSnapHairline(mnAntialiasing & AntialiasingFlags::PixelSnapHairline);

            mpGraphics->DrawPolyLine(
                aTransform, aB2DPolygon, 0.0,
                0.0, // tdf#124848 hairline
                nullptr, // MM01
                basegfx::B2DLineJoin::NONE, css::drawing::LineCap_BUTT,
                basegfx::deg2rad(15.0), // not used with B2DLineJoin::NONE, but the correct default
                bPixelSnapHairline, *this);
        }

        if (bSuccess)
        {
            if (mpAlphaVDev)
                mpAlphaVDev->DrawPolygon(rPoly);
            return;
        }
    }

    tools::Polygon aPoly = ImplLogicToDevicePixel(rPoly);
    const Point* pPtAry = aPoly.GetConstPointAry();

    // #100127# Forward beziers to sal, if any
    if (aPoly.HasFlags())
    {
        const PolyFlags* pFlgAry = aPoly.GetConstFlagAry();
        if (!mpGraphics->DrawPolygonBezier(nPoints, pPtAry, pFlgAry, *this))
        {
            aPoly = tools::Polygon::SubdivideBezier(aPoly);
            pPtAry = aPoly.GetConstPointAry();
            mpGraphics->DrawPolygon(aPoly.GetSize(), pPtAry, *this);
        }
    }
    else
    {
        mpGraphics->DrawPolygon(nPoints, pPtAry, *this);
    }
    if (mpAlphaVDev)
        mpAlphaVDev->DrawPolygon(rPoly);
}

void OutputDevice::ImplDrawPolygon(const tools::Polygon& rPoly,
                                   const tools::PolyPolygon* pClipPolyPoly)
{
    if (pClipPolyPoly)
    {
        ImplDrawPolyPolygon(rPoly, pClipPolyPoly);
    }
    else
    {
        sal_uInt16 nPoints = rPoly.GetSize();

        if (nPoints < 2)
            return;

        const Point* pPtAry = rPoly.GetConstPointAry();
        mpGraphics->DrawPolygon(nPoints, pPtAry, *this);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
