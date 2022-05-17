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

bool SvpGraphicsBackend::setClipRegion(const vcl::Region& i_rClip)
{
    m_rCairoCommon.m_aClipRegion = i_rClip;
    return true;
}

void SvpGraphicsBackend::ResetClipRegion() { m_rCairoCommon.m_aClipRegion.SetNull(); }

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

void SvpGraphicsBackend::SetLineColor() { m_rCairoCommon.m_aLineColor = SALCOLOR_NONE; }

void SvpGraphicsBackend::SetLineColor(Color nColor) { m_rCairoCommon.m_aLineColor = nColor; }

void SvpGraphicsBackend::SetFillColor() { m_rCairoCommon.m_aFillColor = SALCOLOR_NONE; }

void SvpGraphicsBackend::SetFillColor(Color nColor) { m_rCairoCommon.m_aFillColor = nColor; }

void SvpGraphicsBackend::SetXORMode(bool bSet, bool /*bInvertOnly*/)
{
    m_rCairoCommon.m_ePaintMode = bSet ? PaintMode::Xor : PaintMode::Over;
}

void SvpGraphicsBackend::SetROPLineColor(SalROPColor nROPColor)
{
    switch (nROPColor)
    {
        case SalROPColor::N0:
            m_rCairoCommon.m_aLineColor = Color(0, 0, 0);
            break;
        case SalROPColor::N1:
            m_rCairoCommon.m_aLineColor = Color(0xff, 0xff, 0xff);
            break;
        case SalROPColor::Invert:
            m_rCairoCommon.m_aLineColor = Color(0xff, 0xff, 0xff);
            break;
    }
}

void SvpGraphicsBackend::SetROPFillColor(SalROPColor nROPColor)
{
    switch (nROPColor)
    {
        case SalROPColor::N0:
            m_rCairoCommon.m_aFillColor = Color(0, 0, 0);
            break;
        case SalROPColor::N1:
            m_rCairoCommon.m_aFillColor = Color(0xff, 0xff, 0xff);
            break;
        case SalROPColor::Invert:
            m_rCairoCommon.m_aFillColor = Color(0xff, 0xff, 0xff);
            break;
    }
}

void SvpGraphicsBackend::drawPixel(tools::Long nX, tools::Long nY)
{
    if (m_rCairoCommon.m_aLineColor != SALCOLOR_NONE)
    {
        drawPixel(nX, nY, m_rCairoCommon.m_aLineColor);
    }
}

void SvpGraphicsBackend::drawPixel(tools::Long nX, tools::Long nY, Color aColor)
{
    cairo_t* cr = m_rCairoCommon.getCairoContext(true, getAntiAlias());
    m_rCairoCommon.clipRegion(cr);

    cairo_rectangle(cr, nX, nY, 1, 1);
    m_rCairoCommon.applyColor(cr, aColor, 0.0);
    cairo_fill(cr);

    basegfx::B2DRange extents = getClippedFillDamage(cr);
    m_rCairoCommon.releaseCairoContext(cr, true, extents);
}

void SvpGraphicsBackend::drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2,
                                  tools::Long nY2)
{
    basegfx::B2DPolygon aPoly;

    // PixelOffset used: To not mix with possible PixelSnap, cannot do
    // directly on coordinates as tried before - despite being already 'snapped'
    // due to being integer. If it would be directly added here, it would be
    // 'snapped' again when !getAntiAlias(), losing the (0.5, 0.5) offset
    aPoly.append(basegfx::B2DPoint(nX1, nY1));
    aPoly.append(basegfx::B2DPoint(nX2, nY2));

    cairo_t* cr = m_rCairoCommon.getCairoContext(false, getAntiAlias());
    m_rCairoCommon.clipRegion(cr);

    // PixelOffset used: Set PixelOffset as linear transformation
    cairo_matrix_t aMatrix;
    cairo_matrix_init_translate(&aMatrix, 0.5, 0.5);
    cairo_set_matrix(cr, &aMatrix);

    AddPolygonToPath(cr, aPoly, basegfx::B2DHomMatrix(), !getAntiAlias(), false);

    m_rCairoCommon.applyColor(cr, m_rCairoCommon.m_aLineColor);

    basegfx::B2DRange extents = getClippedStrokeDamage(cr);
    extents.transform(basegfx::utils::createTranslateB2DHomMatrix(0.5, 0.5));

    cairo_stroke(cr);

    m_rCairoCommon.releaseCairoContext(cr, false, extents);
}

void SvpGraphicsBackend::drawRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                  tools::Long nHeight)
{
    implDrawRect(nX, nY, nWidth, nHeight);
}

void SvpGraphicsBackend::implDrawRect(double nX, double nY, double nWidth, double nHeight)
{
    // because of the -1 hack we have to do fill and draw separately
    Color aOrigFillColor = m_rCairoCommon.m_aFillColor;
    Color aOrigLineColor = m_rCairoCommon.m_aLineColor;
    m_rCairoCommon.m_aFillColor = SALCOLOR_NONE;
    m_rCairoCommon.m_aLineColor = SALCOLOR_NONE;

    if (aOrigFillColor != SALCOLOR_NONE)
    {
        basegfx::B2DPolygon aRect = basegfx::utils::createPolygonFromRect(
            basegfx::B2DRectangle(nX, nY, nX + nWidth, nY + nHeight));
        m_rCairoCommon.m_aFillColor = aOrigFillColor;

        drawPolyPolygon(basegfx::B2DHomMatrix(), basegfx::B2DPolyPolygon(aRect), 0.0);

        m_rCairoCommon.m_aFillColor = SALCOLOR_NONE;
    }

    if (aOrigLineColor != SALCOLOR_NONE)
    {
        // need same -1 hack as X11SalGraphicsImpl::drawRect
        basegfx::B2DPolygon aRect = basegfx::utils::createPolygonFromRect(
            basegfx::B2DRectangle(nX, nY, nX + nWidth - 1, nY + nHeight - 1));
        m_rCairoCommon.m_aLineColor = aOrigLineColor;

        drawPolyPolygon(basegfx::B2DHomMatrix(), basegfx::B2DPolyPolygon(aRect), 0.0);

        m_rCairoCommon.m_aLineColor = SALCOLOR_NONE;
    }

    m_rCairoCommon.m_aFillColor = aOrigFillColor;
    m_rCairoCommon.m_aLineColor = aOrigLineColor;
}

void SvpGraphicsBackend::drawPolyLine(sal_uInt32 nPoints, const Point* pPtAry)
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(pPtAry->getX(), pPtAry->getY()), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPoly.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].getX(), pPtAry[i].getY()));
    aPoly.setClosed(false);

    drawPolyLine(basegfx::B2DHomMatrix(), aPoly, 0.0, 1.0,
                 nullptr, // MM01
                 basegfx::B2DLineJoin::Miter, css::drawing::LineCap_BUTT,
                 basegfx::deg2rad(15.0) /*default*/, false);
}

void SvpGraphicsBackend::drawPolygon(sal_uInt32 nPoints, const Point* pPtAry)
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(pPtAry->getX(), pPtAry->getY()), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPoly.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].getX(), pPtAry[i].getY()));

    drawPolyPolygon(basegfx::B2DHomMatrix(), basegfx::B2DPolyPolygon(aPoly), 0.0);
}

void SvpGraphicsBackend::drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPointCounts,
                                         const Point** pPtAry)
{
    basegfx::B2DPolyPolygon aPolyPoly;
    for (sal_uInt32 nPolygon = 0; nPolygon < nPoly; ++nPolygon)
    {
        sal_uInt32 nPoints = pPointCounts[nPolygon];
        if (nPoints)
        {
            const Point* pPoints = pPtAry[nPolygon];
            basegfx::B2DPolygon aPoly;
            aPoly.append(basegfx::B2DPoint(pPoints->getX(), pPoints->getY()), nPoints);
            for (sal_uInt32 i = 1; i < nPoints; ++i)
                aPoly.setB2DPoint(i, basegfx::B2DPoint(pPoints[i].getX(), pPoints[i].getY()));

            aPolyPoly.append(aPoly);
        }
    }

    drawPolyPolygon(basegfx::B2DHomMatrix(), aPolyPoly, 0.0);
}

bool SvpGraphicsBackend::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                         const basegfx::B2DPolyPolygon& rPolyPolygon,
                                         double fTransparency)
{
    const bool bHasFill(m_rCairoCommon.m_aFillColor != SALCOLOR_NONE);
    const bool bHasLine(m_rCairoCommon.m_aLineColor != SALCOLOR_NONE);

    if (0 == rPolyPolygon.count() || !(bHasFill || bHasLine) || fTransparency < 0.0
        || fTransparency >= 1.0)
    {
        return true;
    }

    // don't bother trying to draw stuff which is effectively invisible
    basegfx::B2DRange aPolygonRange = rPolyPolygon.getB2DRange();
    aPolygonRange.transform(rObjectToDevice);
    if (aPolygonRange.getWidth() < 0.1 || aPolygonRange.getHeight() < 0.1)
        return true;

    cairo_t* cr = m_rCairoCommon.getCairoContext(true, getAntiAlias());
    m_rCairoCommon.clipRegion(cr);

    // Set full (Object-to-Device) transformation - if used
    if (!rObjectToDevice.isIdentity())
    {
        cairo_matrix_t aMatrix;

        cairo_matrix_init(&aMatrix, rObjectToDevice.get(0, 0), rObjectToDevice.get(1, 0),
                          rObjectToDevice.get(0, 1), rObjectToDevice.get(1, 1),
                          rObjectToDevice.get(0, 2), rObjectToDevice.get(1, 2));
        cairo_set_matrix(cr, &aMatrix);
    }

    // To make releaseCairoContext work, use empty extents
    basegfx::B2DRange extents;

    if (bHasFill)
    {
        add_polygon_path(cr, rPolyPolygon, rObjectToDevice, !getAntiAlias());

        m_rCairoCommon.applyColor(cr, m_rCairoCommon.m_aFillColor, fTransparency);
        // Get FillDamage (will be extended for LineDamage below)
        extents = getClippedFillDamage(cr);

        cairo_fill(cr);
    }

    if (bHasLine)
    {
        // PixelOffset used: Set PixelOffset as linear transformation
        cairo_matrix_t aMatrix;
        cairo_matrix_init_translate(&aMatrix, 0.5, 0.5);
        cairo_set_matrix(cr, &aMatrix);

        add_polygon_path(cr, rPolyPolygon, rObjectToDevice, !getAntiAlias());

        m_rCairoCommon.applyColor(cr, m_rCairoCommon.m_aLineColor, fTransparency);

        // expand with possible StrokeDamage
        basegfx::B2DRange stroke_extents = getClippedStrokeDamage(cr);
        stroke_extents.transform(basegfx::utils::createTranslateB2DHomMatrix(0.5, 0.5));
        extents.expand(stroke_extents);

        cairo_stroke(cr);
    }

    // if transformation has been applied, transform also extents (ranges)
    // of damage so they can be correctly redrawn
    extents.transform(rObjectToDevice);
    m_rCairoCommon.releaseCairoContext(cr, true, extents);

    return true;
}

bool SvpGraphicsBackend::drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                                      const basegfx::B2DPolygon& rPolyLine, double fTransparency,
                                      double fLineWidth, const std::vector<double>* pStroke,
                                      basegfx::B2DLineJoin eLineJoin,
                                      css::drawing::LineCap eLineCap, double fMiterMinimumAngle,
                                      bool bPixelSnapHairline)
{
    // short circuit if there is nothing to do
    if (0 == rPolyLine.count() || fTransparency < 0.0 || fTransparency >= 1.0)
    {
        return true;
    }

    // Wrap call to static version of ::drawPolyLine by
    // preparing/getting some local data and parameters
    // due to usage in vcl/unx/generic/gdi/salgdi.cxx.
    // This is mainly about extended handling of extents
    // and the way destruction of CairoContext is handled
    // due to current XOR stuff
    cairo_t* cr = m_rCairoCommon.getCairoContext(false, getAntiAlias());
    basegfx::B2DRange aExtents;
    m_rCairoCommon.clipRegion(cr);

    bool bRetval(CairoCommon::drawPolyLine(cr, &aExtents, m_rCairoCommon.m_aLineColor,
                                           getAntiAlias(), rObjectToDevice, rPolyLine,
                                           fTransparency, fLineWidth, pStroke, eLineJoin, eLineCap,
                                           fMiterMinimumAngle, bPixelSnapHairline));

    m_rCairoCommon.releaseCairoContext(cr, false, aExtents);

    return bRetval;
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
    // MM02 try to access buffered BitmapHelper
    std::shared_ptr<BitmapHelper> aSurface;
    tryToUseSourceBuffer(rSalBitmap, aSurface);
    cairo_surface_t* source = aSurface->getSurface(rPosAry.mnDestWidth, rPosAry.mnDestHeight);

    if (!source)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap case");
        return;
    }

#if 0 // LO code is not yet bitmap32-ready.
    // if m_bSupportsBitmap32 becomes true for Svp revisit this
    m_rCairoCommon.copyWithOperator(rPosAry, source, CAIRO_OPERATOR_OVER, getAntiAlias());
#else
    m_rCairoCommon.copyWithOperator(rPosAry, source, CAIRO_OPERATOR_SOURCE, getAntiAlias());
#endif
}

void SvpGraphicsBackend::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                                    const SalBitmap& rTransparentBitmap)
{
    drawAlphaBitmap(rPosAry, rSalBitmap, rTransparentBitmap);
}

void SvpGraphicsBackend::drawMask(const SalTwoRect& rTR, const SalBitmap& rSalBitmap,
                                  Color nMaskColor)
{
    /** creates an image from the given rectangle, replacing all black pixels
     *  with nMaskColor and make all other full transparent */
    // MM02 here decided *against* using buffered BitmapHelper
    // because the data gets somehow 'unmuliplied'. This may also be
    // done just once, but I am not sure if this is safe to do.
    // So for now dispense re-using data here.
    BitmapHelper aSurface(rSalBitmap, true); // The mask is argb32
    if (!aSurface.getSurface())
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawMask case");
        return;
    }
    sal_Int32 nStride;
    unsigned char* mask_data = aSurface.getBits(nStride);
#if !ENABLE_WASM_STRIP_PREMULTIPLY
    vcl::bitmap::lookup_table const& unpremultiply_table = vcl::bitmap::get_unpremultiply_table();
#endif
    for (tools::Long y = rTR.mnSrcY; y < rTR.mnSrcY + rTR.mnSrcHeight; ++y)
    {
        unsigned char* row = mask_data + (nStride * y);
        unsigned char* data = row + (rTR.mnSrcX * 4);
        for (tools::Long x = rTR.mnSrcX; x < rTR.mnSrcX + rTR.mnSrcWidth; ++x)
        {
            sal_uInt8 a = data[SVP_CAIRO_ALPHA];
#if ENABLE_WASM_STRIP_PREMULTIPLY
            sal_uInt8 b = vcl::bitmap::unpremultiply(a, data[SVP_CAIRO_BLUE]);
            sal_uInt8 g = vcl::bitmap::unpremultiply(a, data[SVP_CAIRO_GREEN]);
            sal_uInt8 r = vcl::bitmap::unpremultiply(a, data[SVP_CAIRO_RED]);
#else
            sal_uInt8 b = unpremultiply_table[a][data[SVP_CAIRO_BLUE]];
            sal_uInt8 g = unpremultiply_table[a][data[SVP_CAIRO_GREEN]];
            sal_uInt8 r = unpremultiply_table[a][data[SVP_CAIRO_RED]];
#endif
            if (r == 0 && g == 0 && b == 0)
            {
                data[0] = nMaskColor.GetBlue();
                data[1] = nMaskColor.GetGreen();
                data[2] = nMaskColor.GetRed();
                data[3] = 0xff;
            }
            else
            {
                data[0] = 0;
                data[1] = 0;
                data[2] = 0;
                data[3] = 0;
            }
            data += 4;
        }
    }
    aSurface.mark_dirty();

    cairo_t* cr = m_rCairoCommon.getCairoContext(false, getAntiAlias());
    m_rCairoCommon.clipRegion(cr);

    cairo_rectangle(cr, rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);

    basegfx::B2DRange extents = getClippedFillDamage(cr);

    cairo_clip(cr);

    cairo_translate(cr, rTR.mnDestX, rTR.mnDestY);
    double fXScale = static_cast<double>(rTR.mnDestWidth) / rTR.mnSrcWidth;
    double fYScale = static_cast<double>(rTR.mnDestHeight) / rTR.mnSrcHeight;
    cairo_scale(cr, fXScale, fYScale);
    cairo_set_source_surface(cr, aSurface.getSurface(), -rTR.mnSrcX, -rTR.mnSrcY);

    //tdf#133716 borders of upscaled images should not be blurred
    cairo_pattern_t* sourcepattern = cairo_get_source(cr);
    cairo_pattern_set_extend(sourcepattern, CAIRO_EXTEND_PAD);

    cairo_paint(cr);

    m_rCairoCommon.releaseCairoContext(cr, false, extents);
}

std::shared_ptr<SalBitmap> SvpGraphicsBackend::getBitmap(tools::Long nX, tools::Long nY,
                                                         tools::Long nWidth, tools::Long nHeight)
{
    std::shared_ptr<SvpSalBitmap> pBitmap = std::make_shared<SvpSalBitmap>();
    BitmapPalette aPal;
    vcl::PixelFormat ePixelFormat = vcl::PixelFormat::INVALID;
    if (GetBitCount() == 1)
    {
        ePixelFormat = vcl::PixelFormat::N1_BPP;
        aPal.SetEntryCount(2);
        aPal[0] = COL_BLACK;
        aPal[1] = COL_WHITE;
    }
    else
    {
        ePixelFormat = vcl::PixelFormat::N32_BPP;
    }

    if (!pBitmap->Create(Size(nWidth, nHeight), ePixelFormat, aPal))
    {
        SAL_WARN("vcl.gdi", "SvpSalGraphics::getBitmap, cannot create bitmap");
        return nullptr;
    }

    cairo_surface_t* target = CairoCommon::createCairoSurface(pBitmap->GetBuffer());
    if (!target)
    {
        SAL_WARN("vcl.gdi", "SvpSalGraphics::getBitmap, cannot create cairo surface");
        return nullptr;
    }
    cairo_t* cr = cairo_create(target);

    SalTwoRect aTR(nX, nY, nWidth, nHeight, 0, 0, nWidth, nHeight);
    CairoCommon::renderSource(cr, aTR, m_rCairoCommon.m_pSurface);

    cairo_destroy(cr);
    cairo_surface_destroy(target);

    Toggle1BitTransparency(*pBitmap->GetBuffer());

    return pBitmap;
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
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
    cairo_surface_t* target
        = cairo_surface_create_similar_image(m_rCairoCommon.m_pSurface, CAIRO_FORMAT_ARGB32, 1, 1);
#else
    cairo_surface_t* target = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
#endif

    cairo_t* cr = cairo_create(target);

    cairo_rectangle(cr, 0, 0, 1, 1);
    cairo_set_source_surface(cr, m_rCairoCommon.m_pSurface, -nX, -nY);
    cairo_paint(cr);
    cairo_destroy(cr);

    cairo_surface_flush(target);
#if !ENABLE_WASM_STRIP_PREMULTIPLY
    vcl::bitmap::lookup_table const& unpremultiply_table = vcl::bitmap::get_unpremultiply_table();
#endif
    unsigned char* data = cairo_image_surface_get_data(target);
    sal_uInt8 a = data[SVP_CAIRO_ALPHA];
#if ENABLE_WASM_STRIP_PREMULTIPLY
    sal_uInt8 b = vcl::bitmap::unpremultiply(a, data[SVP_CAIRO_BLUE]);
    sal_uInt8 g = vcl::bitmap::unpremultiply(a, data[SVP_CAIRO_GREEN]);
    sal_uInt8 r = vcl::bitmap::unpremultiply(a, data[SVP_CAIRO_RED]);
#else
    sal_uInt8 b = unpremultiply_table[a][data[SVP_CAIRO_BLUE]];
    sal_uInt8 g = unpremultiply_table[a][data[SVP_CAIRO_GREEN]];
    sal_uInt8 r = unpremultiply_table[a][data[SVP_CAIRO_RED]];
#endif
    Color aColor(ColorAlpha, a, r, g, b);
    cairo_surface_destroy(target);

    return aColor;
}

void SvpGraphicsBackend::invert(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                tools::Long nHeight, SalInvert nFlags)
{
    basegfx::B2DPolygon aRect = basegfx::utils::createPolygonFromRect(
        basegfx::B2DRectangle(nX, nY, nX + nWidth, nY + nHeight));

    m_rCairoCommon.invert(aRect, nFlags, getAntiAlias());
}

void SvpGraphicsBackend::invert(sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags)
{
    basegfx::B2DPolygon aPoly;
    aPoly.append(basegfx::B2DPoint(pPtAry->getX(), pPtAry->getY()), nPoints);
    for (sal_uInt32 i = 1; i < nPoints; ++i)
        aPoly.setB2DPoint(i, basegfx::B2DPoint(pPtAry[i].getX(), pPtAry[i].getY()));
    aPoly.setClosed(true);

    m_rCairoCommon.invert(aPoly, nFlags, getAntiAlias());
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
    if (rAlphaBitmap.GetBitCount() != 8 && rAlphaBitmap.GetBitCount() != 1)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap alpha depth case: "
                                << rAlphaBitmap.GetBitCount());
        return false;
    }

    // MM02 try to access buffered BitmapHelper
    std::shared_ptr<BitmapHelper> aSurface;
    tryToUseSourceBuffer(rSourceBitmap, aSurface);
    cairo_surface_t* source = aSurface->getSurface(rTR.mnDestWidth, rTR.mnDestHeight);

    if (!source)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap case");
        return false;
    }

    // MM02 try to access buffered MaskHelper
    std::shared_ptr<MaskHelper> aMask;
    tryToUseMaskBuffer(rAlphaBitmap, aMask);
    cairo_surface_t* mask = aMask->getSurface(rTR.mnDestWidth, rTR.mnDestHeight);

    if (!mask)
    {
        SAL_WARN("vcl.gdi", "unsupported SvpSalGraphics::drawAlphaBitmap case");
        return false;
    }

    cairo_t* cr = m_rCairoCommon.getCairoContext(false, getAntiAlias());
    m_rCairoCommon.clipRegion(cr);

    cairo_rectangle(cr, rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight);

    basegfx::B2DRange extents = getClippedFillDamage(cr);

    cairo_clip(cr);

    cairo_pattern_t* maskpattern = cairo_pattern_create_for_surface(mask);
    cairo_translate(cr, rTR.mnDestX, rTR.mnDestY);
    double fXScale = static_cast<double>(rTR.mnDestWidth) / rTR.mnSrcWidth;
    double fYScale = static_cast<double>(rTR.mnDestHeight) / rTR.mnSrcHeight;
    cairo_scale(cr, fXScale, fYScale);
    cairo_set_source_surface(cr, source, -rTR.mnSrcX, -rTR.mnSrcY);

    cairo_pattern_t* sourcepattern = cairo_get_source(cr);

    //tdf#133716 borders of upscaled images should not be blurred
    //tdf#114117 when stretching a single or multi pixel width/height source to fit an area
    //the image will be extended into that size.
    cairo_pattern_set_extend(sourcepattern, CAIRO_EXTEND_PAD);
    cairo_pattern_set_extend(maskpattern, CAIRO_EXTEND_PAD);

    //this block is just "cairo_mask_surface", but we have to make it explicit
    //because of the cairo_pattern_set_filter etc we may want applied
    cairo_matrix_t matrix;
    cairo_matrix_init_translate(&matrix, rTR.mnSrcX, rTR.mnSrcY);
    cairo_pattern_set_matrix(maskpattern, &matrix);
    cairo_mask(cr, maskpattern);

    cairo_pattern_destroy(maskpattern);

    m_rCairoCommon.releaseCairoContext(cr, false, extents);

    return true;
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

bool SvpGraphicsBackend::hasFastDrawTransformedBitmap() const { return false; }

bool SvpGraphicsBackend::drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                       tools::Long nHeight, sal_uInt8 nTransparency)
{
    const bool bHasFill(m_rCairoCommon.m_aFillColor != SALCOLOR_NONE);
    const bool bHasLine(m_rCairoCommon.m_aLineColor != SALCOLOR_NONE);

    if (!(bHasFill || bHasLine))
    {
        return true;
    }

    cairo_t* cr = m_rCairoCommon.getCairoContext(false, getAntiAlias());
    m_rCairoCommon.clipRegion(cr);

    const double fTransparency = nTransparency * (1.0 / 100);

    // To make releaseCairoContext work, use empty extents
    basegfx::B2DRange extents;

    if (bHasFill)
    {
        cairo_rectangle(cr, nX, nY, nWidth, nHeight);

        m_rCairoCommon.applyColor(cr, m_rCairoCommon.m_aFillColor, fTransparency);

        // set FillDamage
        extents = getClippedFillDamage(cr);

        cairo_fill(cr);
    }

    if (bHasLine)
    {
        // PixelOffset used: Set PixelOffset as linear transformation
        // Note: Was missing here - probably not by purpose (?)
        cairo_matrix_t aMatrix;
        cairo_matrix_init_translate(&aMatrix, 0.5, 0.5);
        cairo_set_matrix(cr, &aMatrix);

        cairo_rectangle(cr, nX, nY, nWidth, nHeight);

        m_rCairoCommon.applyColor(cr, m_rCairoCommon.m_aLineColor, fTransparency);

        // expand with possible StrokeDamage
        basegfx::B2DRange stroke_extents = getClippedStrokeDamage(cr);
        stroke_extents.transform(basegfx::utils::createTranslateB2DHomMatrix(0.5, 0.5));
        extents.expand(stroke_extents);

        cairo_stroke(cr);
    }

    m_rCairoCommon.releaseCairoContext(cr, false, extents);

    return true;
}

bool SvpGraphicsBackend::drawGradient(const tools::PolyPolygon& rPolyPolygon,
                                      const Gradient& rGradient)
{
    if (rGradient.GetStyle() != GradientStyle::Linear
        && rGradient.GetStyle() != GradientStyle::Radial)
        return false; // unsupported
    if (rGradient.GetSteps() != 0)
        return false; // We can't tell cairo how many colors to use in the gradient.

    cairo_t* cr = m_rCairoCommon.getCairoContext(true, getAntiAlias());
    m_rCairoCommon.clipRegion(cr);

    tools::Rectangle aInputRect(rPolyPolygon.GetBoundRect());
    if (rPolyPolygon.IsRect())
    {
        // Rect->Polygon conversion loses the right and bottom edge, fix that.
        aInputRect.AdjustRight(1);
        aInputRect.AdjustBottom(1);
        basegfx::B2DHomMatrix rObjectToDevice;
        AddPolygonToPath(cr, tools::Polygon(aInputRect).getB2DPolygon(), rObjectToDevice,
                         !getAntiAlias(), false);
    }
    else
    {
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rPolyPolygon.getB2DPolyPolygon());
        for (auto const& rPolygon : std::as_const(aB2DPolyPolygon))
        {
            basegfx::B2DHomMatrix rObjectToDevice;
            AddPolygonToPath(cr, rPolygon, rObjectToDevice, !getAntiAlias(), false);
        }
    }

    Gradient aGradient(rGradient);

    tools::Rectangle aBoundRect;
    Point aCenter;

    aGradient.SetAngle(aGradient.GetAngle() + 2700_deg10);
    aGradient.GetBoundRect(aInputRect, aBoundRect, aCenter);
    Color aStartColor = aGradient.GetStartColor();
    Color aEndColor = aGradient.GetEndColor();

    cairo_pattern_t* pattern;
    if (rGradient.GetStyle() == GradientStyle::Linear)
    {
        tools::Polygon aPoly(aBoundRect);
        aPoly.Rotate(aCenter, aGradient.GetAngle() % 3600_deg10);
        pattern
            = cairo_pattern_create_linear(aPoly[0].X(), aPoly[0].Y(), aPoly[1].X(), aPoly[1].Y());
    }
    else
    {
        double radius = std::max(aBoundRect.GetWidth() / 2.0, aBoundRect.GetHeight() / 2.0);
        // Move the center a bit to the top-left (the default VCL algorithm is a bit off-center that way,
        // cairo is the opposite way).
        pattern = cairo_pattern_create_radial(aCenter.X() - 0.5, aCenter.Y() - 0.5, 0,
                                              aCenter.X() - 0.5, aCenter.Y() - 0.5, radius);
        std::swap(aStartColor, aEndColor);
    }

    cairo_pattern_add_color_stop_rgba(
        pattern, aGradient.GetBorder() / 100.0,
        aStartColor.GetRed() * aGradient.GetStartIntensity() / 25500.0,
        aStartColor.GetGreen() * aGradient.GetStartIntensity() / 25500.0,
        aStartColor.GetBlue() * aGradient.GetStartIntensity() / 25500.0, 1.0);

    cairo_pattern_add_color_stop_rgba(
        pattern, 1.0, aEndColor.GetRed() * aGradient.GetEndIntensity() / 25500.0,
        aEndColor.GetGreen() * aGradient.GetEndIntensity() / 25500.0,
        aEndColor.GetBlue() * aGradient.GetEndIntensity() / 25500.0, 1.0);

    cairo_set_source(cr, pattern);
    cairo_pattern_destroy(pattern);

    basegfx::B2DRange extents = getClippedFillDamage(cr);
    cairo_fill_preserve(cr);

    m_rCairoCommon.releaseCairoContext(cr, true, extents);

    return true;
}

bool SvpGraphicsBackend::implDrawGradient(basegfx::B2DPolyPolygon const& rPolyPolygon,
                                          SalGradient const& rGradient)
{
    cairo_t* cr = m_rCairoCommon.getCairoContext(true, getAntiAlias());
    m_rCairoCommon.clipRegion(cr);

    basegfx::B2DHomMatrix rObjectToDevice;

    for (auto const& rPolygon : rPolyPolygon)
        AddPolygonToPath(cr, rPolygon, rObjectToDevice, !getAntiAlias(), false);

    cairo_pattern_t* pattern
        = cairo_pattern_create_linear(rGradient.maPoint1.getX(), rGradient.maPoint1.getY(),
                                      rGradient.maPoint2.getX(), rGradient.maPoint2.getY());

    for (SalGradientStop const& rStop : rGradient.maStops)
    {
        double r = rStop.maColor.GetRed() / 255.0;
        double g = rStop.maColor.GetGreen() / 255.0;
        double b = rStop.maColor.GetBlue() / 255.0;
        double a = rStop.maColor.GetAlpha() / 255.0;
        double offset = rStop.mfOffset;

        cairo_pattern_add_color_stop_rgba(pattern, offset, r, g, b, a);
    }
    cairo_set_source(cr, pattern);
    cairo_pattern_destroy(pattern);

    basegfx::B2DRange extents = getClippedFillDamage(cr);
    cairo_fill_preserve(cr);

    m_rCairoCommon.releaseCairoContext(cr, true, extents);

    return true;
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
