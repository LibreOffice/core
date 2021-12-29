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

#include <headless/SvpGraphicsBackend.hxx>

#include <sal/log.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <vcl/BitmapTools.hxx>

SvpGraphicsBackend::SvpGraphicsBackend(CairoCommon& rCairoCommon)
    : m_rCairoCommon(rCairoCommon)
{
}

void SvpGraphicsBackend::Init() {}

void SvpGraphicsBackend::freeResources() {}

bool SvpGraphicsBackend::setClipRegion(const vcl::Region& /*i_rClip*/) { return false; }

void SvpGraphicsBackend::ResetClipRegion() {}

sal_uInt16 SvpGraphicsBackend::GetBitCount() const
{
    if (cairo_surface_get_content(m_rCairoCommon.m_pSurface) != CAIRO_CONTENT_COLOR_ALPHA)
        return 1;
    return 32;
}

tools::Long SvpGraphicsBackend::GetGraphicsWidth() const
{
    return m_rCairoCommon.m_pSurface ? m_rCairoCommon.m_aFrameSize.getX() : 0;
}

void SvpGraphicsBackend::SetLineColor() {}

void SvpGraphicsBackend::SetLineColor(Color /*nColor*/) {}

void SvpGraphicsBackend::SetFillColor() {}

void SvpGraphicsBackend::SetFillColor(Color /*nColor*/) {}

void SvpGraphicsBackend::SetXORMode(bool /*bSet*/, bool /*bInvertOnly*/) {}

void SvpGraphicsBackend::SetROPLineColor(SalROPColor /*nROPColor*/) {}

void SvpGraphicsBackend::SetROPFillColor(SalROPColor /*nROPColor*/) {}

void SvpGraphicsBackend::drawPixel(tools::Long /*nX*/, tools::Long /*nY*/) {}

void SvpGraphicsBackend::drawPixel(tools::Long /*nX*/, tools::Long /*nY*/, Color /*aColor*/) {}

void SvpGraphicsBackend::drawLine(tools::Long /*nX1*/, tools::Long /*nY1*/, tools::Long /*nX2*/,
                                  tools::Long /*nY2*/)
{
}

void SvpGraphicsBackend::drawRect(tools::Long /*nX*/, tools::Long /*nY*/, tools::Long /*nWidth*/,
                                  tools::Long /*nHeight*/)
{
}

void SvpGraphicsBackend::drawPolyLine(sal_uInt32 /*nPoints*/, const Point* /*pPtAry*/) {}

void SvpGraphicsBackend::drawPolygon(sal_uInt32 /*nPoints*/, const Point* /*pPtAry*/) {}

void SvpGraphicsBackend::drawPolyPolygon(sal_uInt32 /*nPoly*/, const sal_uInt32* /*pPointCounts*/,
                                         const Point** /*pPtAry*/)
{
}

bool SvpGraphicsBackend::drawPolyPolygon(const basegfx::B2DHomMatrix& /*rObjectToDevice*/,
                                         const basegfx::B2DPolyPolygon& /*rPolyPolygon*/,
                                         double /*fTransparency*/)
{
    return false;
}

bool SvpGraphicsBackend::drawPolyLine(const basegfx::B2DHomMatrix& /*rObjectToDevice*/,
                                      const basegfx::B2DPolygon& /*rPolyLine*/,
                                      double /*fTransparency*/, double /*fLineWidth*/,
                                      const std::vector<double>* /*pStroke*/,
                                      basegfx::B2DLineJoin /*eLineJoin*/,
                                      css::drawing::LineCap /*eLineCap*/,
                                      double /*fMiterMinimumAngle*/, bool /*bPixelSnapHairline*/)
{
    return false;
}

bool SvpGraphicsBackend::drawPolyLineBezier(sal_uInt32, const Point*, const PolyFlags*)
{
    SAL_INFO("vcl.gdi", "unsupported SvpSalGraphics::drawPolyLineBezier case");
    return false;
}

bool SvpGraphicsBackend::drawPolygonBezier(sal_uInt32, const Point*, const PolyFlags*)
{
    SAL_INFO("vcl.gdi", "unsupported SvpSalGraphics::drawPolygonBezier case");
    return false;
}

bool SvpGraphicsBackend::drawPolyPolygonBezier(sal_uInt32, const sal_uInt32*, const Point* const*,
                                               const PolyFlags* const*)
{
    SAL_INFO("vcl.gdi", "unsupported SvpSalGraphics::drawPolyPolygonBezier case");
    return false;
}

void SvpGraphicsBackend::copyArea(tools::Long /*nDestX*/, tools::Long /*nDestY*/,
                                  tools::Long /*nSrcX*/, tools::Long /*nSrcY*/,
                                  tools::Long /*nSrcWidth*/, tools::Long /*nSrcHeight*/,
                                  bool /*bWindowInvalidate*/)
{
}

void SvpGraphicsBackend::copyBits(const SalTwoRect& /*rTR*/, SalGraphics* /*pSrcGraphics*/) {}

void SvpGraphicsBackend::drawBitmap(const SalTwoRect& /*rPosAry*/, const SalBitmap& /*rSalBitmap*/)
{
}

void SvpGraphicsBackend::drawBitmap(const SalTwoRect& /*rPosAry*/, const SalBitmap& /*rSalBitmap*/,
                                    const SalBitmap& /*rMaskBitmap*/)
{
}

void SvpGraphicsBackend::drawMask(const SalTwoRect& /*rPosAry*/, const SalBitmap& /*rSalBitmap*/,
                                  Color /*nMaskColor*/)
{
}

std::shared_ptr<SalBitmap> SvpGraphicsBackend::getBitmap(tools::Long /*nX*/, tools::Long /*nY*/,
                                                         tools::Long /*nWidth*/,
                                                         tools::Long /*nHeight*/)
{
    return std::shared_ptr<SalBitmap>();
}

Color SvpGraphicsBackend::getPixel(tools::Long /*nX*/, tools::Long /*nY*/) { return Color(); }

void SvpGraphicsBackend::invert(tools::Long /*nX*/, tools::Long /*nY*/, tools::Long /*nWidth*/,
                                tools::Long /*nHeight*/, SalInvert /*nFlags*/)
{
}

void SvpGraphicsBackend::invert(sal_uInt32 /*nPoints*/, const Point* /*pPtAry*/,
                                SalInvert /*nFlags*/)
{
}

bool SvpGraphicsBackend::drawEPS(tools::Long /*nX*/, tools::Long /*nY*/, tools::Long /*nWidth*/,
                                 tools::Long /*nHeight*/, void* /*pPtr*/, sal_uInt32 /*nSize*/)
{
    return false;
}

bool SvpGraphicsBackend::blendBitmap(const SalTwoRect& /*rPosAry*/, const SalBitmap& /*rBitmap*/)
{
    return false;
}

bool SvpGraphicsBackend::blendAlphaBitmap(const SalTwoRect& /*rPosAry*/,
                                          const SalBitmap& /*rSrcBitmap*/,
                                          const SalBitmap& /*rMaskBitmap*/,
                                          const SalBitmap& /*rAlphaBitmap*/)
{
    return false;
}

bool SvpGraphicsBackend::drawAlphaBitmap(const SalTwoRect&, const SalBitmap& /*rSourceBitmap*/,
                                         const SalBitmap& /*rAlphaBitmap*/)
{
    return false;
}

bool SvpGraphicsBackend::drawTransformedBitmap(const basegfx::B2DPoint& /*rNull*/,
                                               const basegfx::B2DPoint& /*rX*/,
                                               const basegfx::B2DPoint& /*rY*/,
                                               const SalBitmap& /*rSourceBitmap*/,
                                               const SalBitmap* /*pAlphaBitmap*/, double /*fAlpha*/)
{
    return false;
}

bool SvpGraphicsBackend::hasFastDrawTransformedBitmap() const { return false; }

bool SvpGraphicsBackend::drawAlphaRect(tools::Long /*nX*/, tools::Long /*nY*/,
                                       tools::Long /*nWidth*/, tools::Long /*nHeight*/,
                                       sal_uInt8 /*nTransparency*/)
{
    return false;
}

bool SvpGraphicsBackend::drawGradient(const tools::PolyPolygon& /*rPolyPolygon*/,
                                      const Gradient& /*rGradient*/)
{
    return false;
}

bool SvpGraphicsBackend::implDrawGradient(basegfx::B2DPolyPolygon const& /*rPolyPolygon*/,
                                          SalGradient const& /*rGradient*/)
{
    return false;
}

bool SvpGraphicsBackend::supportsOperation(OutDevSupportType eType) const
{
    switch (eType)
    {
        case OutDevSupportType::TransparentRect:
        case OutDevSupportType::B2DDraw:
            return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
