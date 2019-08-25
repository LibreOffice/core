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
#include <basegfx/polygon/b2dlinegeometry.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/drawables/B2DPolyLineDrawable.hxx>
#include <vcl/drawables/B2DPolyPolyLineDrawable.hxx>
#include <vcl/drawables/PolyHairlineDrawable.hxx>

#include <salgdi.hxx>

#include <cassert>

void OutputDevice::DrawPolyLine(const tools::Polygon& rPoly, const LineInfo& rLineInfo)
{
    assert(!is_double_buffered_window());

    if (mpMetaFile)
        mpMetaFile->AddAction(new MetaPolyLineAction(rPoly, rLineInfo));

    sal_uInt16 nPoints = rPoly.GetSize();

    if (!IsDeviceOutputNecessary() || !mbLineColor || (nPoints < 2) || ImplIsRecordLayout())
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

    if (rLineInfo.IsDefault())
    {
        LineInfo aHairlineInfo;
        aHairlineInfo.SetLineJoin(basegfx::B2DLineJoin::NONE);
        aHairlineInfo.SetLineCap(css::drawing::LineCap_BUTT);

        // use b2dpolygon drawing if possible
        if (Draw(vcl::PolyHairlineDrawable(basegfx::B2DHomMatrix(), rPoly.getB2DPolygon(),
                                           aHairlineInfo)))
        {
            return;
        }

        const basegfx::B2DPolygon aB2DPolyLine(rPoly.getB2DPolygon());
        const basegfx::B2DHomMatrix aTransform(ImplGetDeviceTransformation());
        const basegfx::B2DVector aB2DLineWidth(1.0, 1.0);
        const bool bPixelSnapHairline(mnAntialiasing & AntialiasingFlags::PixelSnapHairline);

        if (mpGraphics->DrawPolyLine(
                aTransform, aB2DPolyLine, 0.0, aB2DLineWidth, basegfx::B2DLineJoin::NONE,
                css::drawing::LineCap_BUTT,
                basegfx::deg2rad(15.0) /*default fMiterMinimumAngle, not used*/, bPixelSnapHairline,
                this))
        {
            return;
        }

        tools::Polygon aPoly = ImplLogicToDevicePixel(rPoly);
        SalPoint* pPtAry = reinterpret_cast<SalPoint*>(aPoly.GetPointAry());

        // #100127# Forward beziers to sal, if any
        if (aPoly.HasFlags())
        {
            const PolyFlags* pFlgAry = aPoly.GetConstFlagAry();
            if (!mpGraphics->DrawPolyLineBezier(nPoints, pPtAry, pFlgAry, this))
            {
                aPoly = tools::Polygon::SubdivideBezier(aPoly);
                pPtAry = reinterpret_cast<SalPoint*>(aPoly.GetPointAry());
                mpGraphics->DrawPolyLine(aPoly.GetSize(), pPtAry, this);
            }
        }
        else
        {
            mpGraphics->DrawPolyLine(nPoints, pPtAry, this);
        }
    }
    else
    {
        tools::Polygon aPoly = ImplLogicToDevicePixel(rPoly);

        // #i101491#
        // Try direct Fallback to B2D-Version of DrawPolyLine
        if ((mnAntialiasing & AntialiasingFlags::EnableB2dDraw)
            && LineStyle::Solid == rLineInfo.GetStyle())
        {
            DrawPolyLine(
                rPoly.getB2DPolygon(), static_cast<double>(rLineInfo.GetWidth()),
                rLineInfo.GetLineJoin(), rLineInfo.GetLineCap(),
                basegfx::deg2rad(
                    15.0), /* default fMiterMinimumAngle, value not available in LineInfo */
                false);
            return;
        }

        const LineInfo aInfo(ImplLogicToDevicePixel(rLineInfo));
        const bool bDashUsed(LineStyle::Dash == aInfo.GetStyle());
        const bool bLineWidthUsed(aInfo.GetWidth() > 1);

        if (bDashUsed || bLineWidthUsed)
        {
            Draw(vcl::B2DPolyPolyLineDrawable(basegfx::B2DPolyPolygon(aPoly.getB2DPolygon()), aInfo));
        }
        else
        {
            // #100127# the subdivision HAS to be done here since only a pointer
            // to an array of points is given to the DrawPolyLine method, there is
            // NO way to find out there that it's a curve.
            if (aPoly.HasFlags())
            {
                aPoly = tools::Polygon::SubdivideBezier(aPoly);
                nPoints = aPoly.GetSize();
            }

            mpGraphics->DrawPolyLine(nPoints, reinterpret_cast<SalPoint*>(aPoly.GetPointAry()),
                                     this);
        }
    }

    if (mpAlphaVDev)
        mpAlphaVDev->DrawPolyLine(rPoly, rLineInfo);
}

void OutputDevice::DrawPolyLine(const basegfx::B2DPolygon& rB2DPolygon, double fLineWidth,
                                basegfx::B2DLineJoin eLineJoin, css::drawing::LineCap eLineCap,
                                double fMiterMinimumAngle, bool bUseScaffolding)
{
    LineInfo aLineInfo;
    aLineInfo.SetWidth(fLineWidth);
    aLineInfo.SetLineJoin(eLineJoin);
    aLineInfo.SetLineCap(eLineCap);

    Draw(vcl::B2DPolyLineDrawable(rB2DPolygon, aLineInfo, fMiterMinimumAngle, bUseScaffolding));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
