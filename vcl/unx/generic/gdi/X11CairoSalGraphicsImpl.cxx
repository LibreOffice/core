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

#include "X11CairoSalGraphicsImpl.hxx"

#if ENABLE_CAIRO_CANVAS

#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>

X11CairoSalGraphicsImpl::X11CairoSalGraphicsImpl(X11SalGraphics& rParent, X11Common& rX11Common)
    : X11SalGraphicsImpl(rParent)
    , mrX11Common(rX11Common)
    , mnPenColor(SALCOLOR_NONE)
    , mnFillColor(SALCOLOR_NONE)
    , meFillRule(CAIRO_FILL_RULE_EVEN_ODD)
{
}

bool X11CairoSalGraphicsImpl::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                              const basegfx::B2DPolyPolygon& rPolyPolygon,
                                              double fTransparency)
{
    if (fTransparency >= 1.0)
    {
        return true;
    }

    if (rPolyPolygon.count() == 0)
    {
        return true;
    }

    // Fallback: Transform to DeviceCoordinates
    basegfx::B2DPolyPolygon aPolyPolygon(rPolyPolygon);
    aPolyPolygon.transform(rObjectToDevice);

    if (SALCOLOR_NONE == mnFillColor && SALCOLOR_NONE == mnPenColor)
    {
        return true;
    }

    // enable by setting to something
    static const char* pUseCairoForPolygons(getenv("SAL_ENABLE_USE_CAIRO_FOR_POLYGONS"));

    if (nullptr != pUseCairoForPolygons && mrX11Common.SupportsCairo())
    {
        // snap to raster if requested
        const bool bSnapPoints(!getAntiAlias());

        if (bSnapPoints)
        {
            aPolyPolygon = basegfx::utils::snapPointsOfHorizontalOrVerticalEdges(aPolyPolygon);
        }

        cairo_t* cr = mrX11Common.getCairoContext();
        clipRegion(cr);

        for (auto const& rPolygon : std::as_const(aPolyPolygon))
        {
            const sal_uInt32 nPointCount(rPolygon.count());

            if (nPointCount)
            {
                const sal_uInt32 nEdgeCount(rPolygon.isClosed() ? nPointCount : nPointCount - 1);

                if (nEdgeCount)
                {
                    basegfx::B2DCubicBezier aEdge;

                    for (sal_uInt32 b = 0; b < nEdgeCount; ++b)
                    {
                        rPolygon.getBezierSegment(b, aEdge);

                        if (!b)
                        {
                            const basegfx::B2DPoint aStart(aEdge.getStartPoint());
                            cairo_move_to(cr, aStart.getX(), aStart.getY());
                        }

                        const basegfx::B2DPoint aEnd(aEdge.getEndPoint());

                        if (aEdge.isBezier())
                        {
                            const basegfx::B2DPoint aCP1(aEdge.getControlPointA());
                            const basegfx::B2DPoint aCP2(aEdge.getControlPointB());
                            cairo_curve_to(cr, aCP1.getX(), aCP1.getY(), aCP2.getX(), aCP2.getY(),
                                           aEnd.getX(), aEnd.getY());
                        }
                        else
                        {
                            cairo_line_to(cr, aEnd.getX(), aEnd.getY());
                        }
                    }

                    cairo_close_path(cr);
                }
            }
        }

        if (SALCOLOR_NONE != mnFillColor)
        {
            cairo_set_source_rgba(cr, mnFillColor.GetRed() / 255.0, mnFillColor.GetGreen() / 255.0,
                                  mnFillColor.GetBlue() / 255.0, 1.0 - fTransparency);
            cairo_set_fill_rule(cr, meFillRule);
            cairo_fill_preserve(cr);
        }

        if (SALCOLOR_NONE != mnPenColor)
        {
            cairo_set_source_rgba(cr, mnPenColor.GetRed() / 255.0, mnPenColor.GetGreen() / 255.0,
                                  mnPenColor.GetBlue() / 255.0, 1.0 - fTransparency);
            cairo_stroke_preserve(cr);
        }

        X11Common::releaseCairoContext(cr);
        return true;
    }

    return X11SalGraphicsImpl::drawPolyPolygon(rObjectToDevice, rPolyPolygon, fTransparency);
}

bool X11CairoSalGraphicsImpl::drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                                           const basegfx::B2DPolygon& rPolygon,
                                           double fTransparency, double fLineWidth,
                                           const std::vector<double>* pStroke,
                                           basegfx::B2DLineJoin eLineJoin,
                                           css::drawing::LineCap eLineCap,
                                           double fMiterMinimumAngle, bool bPixelSnapHairline)
{
    if (0 == rPolygon.count())
    {
        return true;
    }

    if (fTransparency >= 1.0)
    {
        return true;
    }

    // disable by setting to something
    static const char* pUseCairoForFatLines(getenv("SAL_DISABLE_USE_CAIRO_FOR_FATLINES"));

    if (nullptr == pUseCairoForFatLines && mrX11Common.SupportsCairo())
    {
        cairo_t* cr = mrX11Common.getCairoContext();
        clipRegion(cr);

        // Use the now available static drawPolyLine from the Cairo-Headless-Fallback
        // that will take care of all needed stuff
        const bool bRetval(CairoCommon::drawPolyLine(
            cr, nullptr, mnPenColor, getAntiAlias(), rObjectToDevice, rPolygon, fTransparency,
            fLineWidth, pStroke, eLineJoin, eLineCap, fMiterMinimumAngle, bPixelSnapHairline));

        X11Common::releaseCairoContext(cr);

        if (bRetval)
        {
            return true;
        }
    }

    return X11SalGraphicsImpl::drawPolyLine(rObjectToDevice, rPolygon, fTransparency, fLineWidth,
                                            pStroke, eLineJoin, eLineCap, fMiterMinimumAngle,
                                            bPixelSnapHairline);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
