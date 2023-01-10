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
#include <headless/BitmapHelper.hxx>

SvpGraphicsBackend::SvpGraphicsBackend(CairoCommon& rCairoCommon)
    : m_rCairoCommon(rCairoCommon)
{
}

void SvpGraphicsBackend::Init() {}

void SvpGraphicsBackend::freeResources() {}

void SvpGraphicsBackend::setClipRegion(const vcl::Region& i_rClip)
{
    m_rCairoCommon.m_aClipRegion = i_rClip;
}

void SvpGraphicsBackend::ResetClipRegion() { m_rCairoCommon.m_aClipRegion.SetNull(); }

sal_uInt16 SvpGraphicsBackend::GetBitCount() const { return m_rCairoCommon.GetBitCount(); }

tools::Long SvpGraphicsBackend::GetGraphicsWidth() const
{
    return m_rCairoCommon.m_pSurface ? m_rCairoCommon.m_aFrameSize.getX() : 0;
}

void SvpGraphicsBackend::SetLineColor() { m_rCairoCommon.m_oLineColor = std::nullopt; }

void SvpGraphicsBackend::SetLineColor(Color nColor) { m_rCairoCommon.m_oLineColor = nColor; }

void SvpGraphicsBackend::SetFillColor() { m_rCairoCommon.m_oFillColor = std::nullopt; }

void SvpGraphicsBackend::SetFillColor(Color nColor) { m_rCairoCommon.m_oFillColor = nColor; }

void SvpGraphicsBackend::SetXORMode(bool bSet, bool bInvertOnly)
{
    m_rCairoCommon.SetXORMode(bSet, bInvertOnly);
}

void SvpGraphicsBackend::SetROPLineColor(SalROPColor nROPColor)
{
    m_rCairoCommon.SetROPLineColor(nROPColor);
}

void SvpGraphicsBackend::SetROPFillColor(SalROPColor nROPColor)
{
    m_rCairoCommon.SetROPFillColor(nROPColor);
}

void SvpGraphicsBackend::drawPixel(tools::Long nX, tools::Long nY)
{
    m_rCairoCommon.drawPixel(m_rCairoCommon.m_oLineColor, nX, nY, getAntiAlias());
}

void SvpGraphicsBackend::drawPixel(tools::Long nX, tools::Long nY, Color aColor)
{
    m_rCairoCommon.drawPixel(aColor, nX, nY, getAntiAlias());
}

void SvpGraphicsBackend::drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2,
                                  tools::Long nY2)
{
    m_rCairoCommon.drawLine(nX1, nY1, nX2, nY2, getAntiAlias());
}

void SvpGraphicsBackend::drawRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                  tools::Long nHeight)
{
    m_rCairoCommon.drawRect(nX, nY, nWidth, nHeight, getAntiAlias());
}

void SvpGraphicsBackend::drawPolyLine(sal_uInt32 nPoints, const Point* pPtAry)
{
    m_rCairoCommon.drawPolyLine(nPoints, pPtAry, getAntiAlias());
}

void SvpGraphicsBackend::drawPolygon(sal_uInt32 nPoints, const Point* pPtAry)
{
    m_rCairoCommon.drawPolygon(nPoints, pPtAry, getAntiAlias());
}

void SvpGraphicsBackend::drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPointCounts,
                                         const Point** pPtAry)
{
    m_rCairoCommon.drawPolyPolygon(nPoly, pPointCounts, pPtAry, getAntiAlias());
}

bool SvpGraphicsBackend::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                         const basegfx::B2DPolyPolygon& rPolyPolygon,
                                         double fTransparency)
{
    return m_rCairoCommon.drawPolyPolygon(rObjectToDevice, rPolyPolygon, fTransparency,
                                          getAntiAlias());
}

bool SvpGraphicsBackend::drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                                      const basegfx::B2DPolygon& rPolyLine, double fTransparency,
                                      double fLineWidth, const std::vector<double>* pStroke,
                                      basegfx::B2DLineJoin eLineJoin,
                                      css::drawing::LineCap eLineCap, double fMiterMinimumAngle,
                                      bool bPixelSnapHairline)
{
    return m_rCairoCommon.drawPolyLine(rObjectToDevice, rPolyLine, fTransparency, fLineWidth,
                                       pStroke, eLineJoin, eLineCap, fMiterMinimumAngle,
                                       bPixelSnapHairline, getAntiAlias());
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

void SvpGraphicsBackend::copyArea(tools::Long nDestX, tools::Long nDestY, tools::Long nSrcX,
                                  tools::Long nSrcY, tools::Long nSrcWidth, tools::Long nSrcHeight,
                                  bool /*bWindowInvalidate*/)
{
    SalTwoRect aTR(nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDestX, nDestY, nSrcWidth, nSrcHeight);

    cairo_surface_t* source = m_rCairoCommon.m_pSurface;
    m_rCairoCommon.copyBitsCairo(aTR, source, getAntiAlias());
}

void SvpGraphicsBackend::copyBits(const SalTwoRect& rTR, SalGraphics* pSrcGraphics)
{
    cairo_surface_t* source = nullptr;

    if (pSrcGraphics)
    {
        SvpGraphicsBackend* pSrc = static_cast<SvpGraphicsBackend*>(pSrcGraphics->GetImpl());
        source = pSrc->m_rCairoCommon.m_pSurface;
    }
    else
    {
        source = m_rCairoCommon.m_pSurface;
    }

    m_rCairoCommon.copyBitsCairo(rTR, source, getAntiAlias());
}

void SvpGraphicsBackend::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap)
{
    m_rCairoCommon.drawBitmap(rPosAry, rSalBitmap, getAntiAlias());
}

void SvpGraphicsBackend::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                    const SalBitmap& rTransparentBitmap)
{
    drawAlphaBitmap(rPosAry, rSalBitmap, rTransparentBitmap);
}

void SvpGraphicsBackend::drawMask(const SalTwoRect& rTR, const SalBitmap& rSalBitmap,
                                  Color nMaskColor)
{
    m_rCairoCommon.drawMask(rTR, rSalBitmap, nMaskColor, getAntiAlias());
}

std::shared_ptr<SalBitmap> SvpGraphicsBackend::getBitmap(tools::Long nX, tools::Long nY,
                                                         tools::Long nWidth, tools::Long nHeight)
{
    return m_rCairoCommon.getBitmap(nX, nY, nWidth, nHeight);
}

void SvpGraphicsBackend::drawBitmapBuffer(const SalTwoRect& rTR, const BitmapBuffer* pBuffer,
                                          cairo_operator_t eOp)
{
    cairo_surface_t* source = CairoCommon::createCairoSurface(pBuffer);
    m_rCairoCommon.copyWithOperator(rTR, source, eOp, getAntiAlias());
    cairo_surface_destroy(source);
}

Color SvpGraphicsBackend::getPixel(tools::Long nX, tools::Long nY)
{
    return CairoCommon::getPixel(m_rCairoCommon.m_pSurface, nX, nY);
}

void SvpGraphicsBackend::invert(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                tools::Long nHeight, SalInvert nFlags)
{
    m_rCairoCommon.invert(nX, nY, nWidth, nHeight, nFlags, getAntiAlias());
}

void SvpGraphicsBackend::invert(sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags)
{
    m_rCairoCommon.invert(nPoints, pPtAry, nFlags, getAntiAlias());
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

bool SvpGraphicsBackend::drawAlphaBitmap(const SalTwoRect& rTR, const SalBitmap& rSourceBitmap,
                                         const SalBitmap& rAlphaBitmap)
{
    return m_rCairoCommon.drawAlphaBitmap(rTR, rSourceBitmap, rAlphaBitmap, getAntiAlias());
}

bool SvpGraphicsBackend::drawTransformedBitmap(const basegfx::B2DPoint& rNull,
                                               const basegfx::B2DPoint& rX,
                                               const basegfx::B2DPoint& rY,
                                               const SalBitmap& rSourceBitmap,
                                               const SalBitmap* pAlphaBitmap, double fAlpha)
{
    if (pAlphaBitmap && pAlphaBitmap->GetBitCount() != 8 && pAlphaBitmap->GetBitCount() != 1)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawTransformedBitmap alpha depth case: "
                                << pAlphaBitmap->GetBitCount());
        return false;
    }

    if (fAlpha != 1.0)
        return false;

    // MM02 try to access buffered BitmapHelper
    std::shared_ptr<BitmapHelper> aSurface;
    tryToUseSourceBuffer(rSourceBitmap, aSurface);
    const tools::Long nDestWidth(basegfx::fround(basegfx::B2DVector(rX - rNull).getLength()));
    const tools::Long nDestHeight(basegfx::fround(basegfx::B2DVector(rY - rNull).getLength()));
    cairo_surface_t* source(aSurface->getSurface(nDestWidth, nDestHeight));

    if (!source)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawTransformedBitmap case");
        return false;
    }

    // MM02 try to access buffered MaskHelper
    std::shared_ptr<MaskHelper> aMask;
    if (nullptr != pAlphaBitmap)
    {
        tryToUseMaskBuffer(*pAlphaBitmap, aMask);
    }

    // access cairo_surface_t from MaskHelper
    cairo_surface_t* mask(nullptr);
    if (aMask)
    {
        mask = aMask->getSurface(nDestWidth, nDestHeight);
    }

    if (nullptr != pAlphaBitmap && nullptr == mask)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawTransformedBitmap case");
        return false;
    }

    const Size aSize = rSourceBitmap.GetSize();
    cairo_t* cr = m_rCairoCommon.getCairoContext(false, getAntiAlias());
    m_rCairoCommon.clipRegion(cr);

    // setup the image transformation
    // using the rNull,rX,rY points as destinations for the (0,0),(0,Width),(Height,0) source points
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;
    cairo_matrix_t matrix;
    cairo_matrix_init(&matrix, aXRel.getX() / aSize.Width(), aXRel.getY() / aSize.Width(),
                      aYRel.getX() / aSize.Height(), aYRel.getY() / aSize.Height(), rNull.getX(),
                      rNull.getY());

    cairo_transform(cr, &matrix);

    cairo_rectangle(cr, 0, 0, aSize.Width(), aSize.Height());
    basegfx::B2DRange extents = getClippedFillDamage(cr);
    cairo_clip(cr);

    cairo_set_source_surface(cr, source, 0, 0);
    if (mask)
        cairo_mask_surface(cr, mask, 0, 0);
    else
        cairo_paint(cr);

    m_rCairoCommon.releaseCairoContext(cr, false, extents);

    return true;
}

bool SvpGraphicsBackend::hasFastDrawTransformedBitmap() const
{
    return CairoCommon::hasFastDrawTransformedBitmap();
}

bool SvpGraphicsBackend::drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                       tools::Long nHeight, sal_uInt8 nTransparency)
{
    return m_rCairoCommon.drawAlphaRect(nX, nY, nWidth, nHeight, nTransparency, getAntiAlias());
}

bool SvpGraphicsBackend::drawGradient(const tools::PolyPolygon& rPolyPolygon,
                                      const Gradient& rGradient)
{
    return m_rCairoCommon.drawGradient(rPolyPolygon, rGradient, getAntiAlias());
}

bool SvpGraphicsBackend::implDrawGradient(basegfx::B2DPolyPolygon const& rPolyPolygon,
                                          SalGradient const& rGradient)
{
    return m_rCairoCommon.implDrawGradient(rPolyPolygon, rGradient, getAntiAlias());
}

bool SvpGraphicsBackend::supportsOperation(OutDevSupportType eType) const
{
    return CairoCommon::supportsOperation(eType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
