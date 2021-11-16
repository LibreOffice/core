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

#include <headless/CairoCommon.hxx>
#include <dlfcn.h>
#include <vcl/BitmapTools.hxx>
#include <basegfx/utils/canvastools.hxx>

void dl_cairo_surface_set_device_scale(cairo_surface_t* surface, double x_scale, double y_scale)
{
#ifdef ANDROID
    cairo_surface_set_device_scale(surface, x_scale, y_scale);
#else
    static auto func = reinterpret_cast<void (*)(cairo_surface_t*, double, double)>(
        dlsym(nullptr, "cairo_surface_set_device_scale"));
    if (func)
        func(surface, x_scale, y_scale);
#endif
}

void dl_cairo_surface_get_device_scale(cairo_surface_t* surface, double* x_scale, double* y_scale)
{
#ifdef ANDROID
    cairo_surface_get_device_scale(surface, x_scale, y_scale);
#else
    static auto func = reinterpret_cast<void (*)(cairo_surface_t*, double*, double*)>(
        dlsym(nullptr, "cairo_surface_get_device_scale"));
    if (func)
        func(surface, x_scale, y_scale);
    else
    {
        if (x_scale)
            *x_scale = 1.0;
        if (y_scale)
            *y_scale = 1.0;
    }
#endif
}

cairo_user_data_key_t* CairoCommon::getDamageKey()
{
    static cairo_user_data_key_t aDamageKey;
    return &aDamageKey;
}

cairo_t* CairoCommon::getCairoContext(bool bXorModeAllowed, bool bAntiAlias) const
{
    cairo_t* cr;
    if (m_ePaintMode == PaintMode::Xor && bXorModeAllowed)
        cr = createTmpCompatibleCairoContext();
    else
        cr = cairo_create(m_pSurface);
    cairo_set_line_width(cr, 1);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_antialias(cr, bAntiAlias ? CAIRO_ANTIALIAS_DEFAULT : CAIRO_ANTIALIAS_NONE);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    // ensure no linear transformation and no PathInfo in local cairo_path_t
    cairo_identity_matrix(cr);
    cairo_new_path(cr);

    return cr;
}

void CairoCommon::releaseCairoContext(cairo_t* cr, bool bXorModeAllowed,
                                      const basegfx::B2DRange& rExtents) const
{
    const bool bXoring = (m_ePaintMode == PaintMode::Xor && bXorModeAllowed);

    if (rExtents.isEmpty())
    {
        //nothing changed, return early
        if (bXoring)
        {
            cairo_surface_t* surface = cairo_get_target(cr);
            cairo_surface_destroy(surface);
        }
        cairo_destroy(cr);
        return;
    }

    basegfx::B2IRange aIntExtents(basegfx::unotools::b2ISurroundingRangeFromB2DRange(rExtents));
    sal_Int32 nExtentsLeft(aIntExtents.getMinX()), nExtentsTop(aIntExtents.getMinY());
    sal_Int32 nExtentsRight(aIntExtents.getMaxX()), nExtentsBottom(aIntExtents.getMaxY());
    sal_Int32 nWidth = m_aFrameSize.getX();
    sal_Int32 nHeight = m_aFrameSize.getY();
    nExtentsLeft = std::max<sal_Int32>(nExtentsLeft, 0);
    nExtentsTop = std::max<sal_Int32>(nExtentsTop, 0);
    nExtentsRight = std::min<sal_Int32>(nExtentsRight, nWidth);
    nExtentsBottom = std::min<sal_Int32>(nExtentsBottom, nHeight);

    cairo_surface_t* surface = cairo_get_target(cr);
    cairo_surface_flush(surface);

    //For the most part we avoid the use of XOR these days, but there
    //are some edge cases where legacy stuff still supports it, so
    //emulate it (slowly) here.
    if (bXoring)
    {
        cairo_surface_t* target_surface = m_pSurface;
        if (cairo_surface_get_type(target_surface) != CAIRO_SURFACE_TYPE_IMAGE)
        {
            //in the unlikely case we can't use m_pSurface directly, copy contents
            //to another temp image surface
            cairo_t* copycr = createTmpCompatibleCairoContext();
            cairo_rectangle(copycr, nExtentsLeft, nExtentsTop, nExtentsRight - nExtentsLeft,
                            nExtentsBottom - nExtentsTop);
            cairo_set_source_surface(copycr, m_pSurface, 0, 0);
            cairo_paint(copycr);
            target_surface = cairo_get_target(copycr);
            cairo_destroy(copycr);
        }

        cairo_surface_flush(target_surface);
        unsigned char* target_surface_data = cairo_image_surface_get_data(target_surface);
        unsigned char* xor_surface_data = cairo_image_surface_get_data(surface);

        cairo_format_t nFormat = cairo_image_surface_get_format(target_surface);
        assert(nFormat == CAIRO_FORMAT_ARGB32
               && "need to implement CAIRO_FORMAT_A1 after all here");
        sal_Int32 nStride = cairo_format_stride_for_width(nFormat, nWidth * m_fScale);
        sal_Int32 nUnscaledExtentsLeft = nExtentsLeft * m_fScale;
        sal_Int32 nUnscaledExtentsRight = nExtentsRight * m_fScale;
        sal_Int32 nUnscaledExtentsTop = nExtentsTop * m_fScale;
        sal_Int32 nUnscaledExtentsBottom = nExtentsBottom * m_fScale;

        // Handle headless size forced to (1,1) by SvpSalFrame::GetSurfaceFrameSize().
        int target_surface_width = cairo_image_surface_get_width(target_surface);
        if (nUnscaledExtentsLeft > target_surface_width)
            nUnscaledExtentsLeft = target_surface_width;
        if (nUnscaledExtentsRight > target_surface_width)
            nUnscaledExtentsRight = target_surface_width;
        int target_surface_height = cairo_image_surface_get_height(target_surface);
        if (nUnscaledExtentsTop > target_surface_height)
            nUnscaledExtentsTop = target_surface_height;
        if (nUnscaledExtentsBottom > target_surface_height)
            nUnscaledExtentsBottom = target_surface_height;

        vcl::bitmap::lookup_table const& unpremultiply_table
            = vcl::bitmap::get_unpremultiply_table();
        vcl::bitmap::lookup_table const& premultiply_table = vcl::bitmap::get_premultiply_table();
        for (sal_Int32 y = nUnscaledExtentsTop; y < nUnscaledExtentsBottom; ++y)
        {
            unsigned char* true_row = target_surface_data + (nStride * y);
            unsigned char* xor_row = xor_surface_data + (nStride * y);
            unsigned char* true_data = true_row + (nUnscaledExtentsLeft * 4);
            unsigned char* xor_data = xor_row + (nUnscaledExtentsLeft * 4);
            for (sal_Int32 x = nUnscaledExtentsLeft; x < nUnscaledExtentsRight; ++x)
            {
                sal_uInt8 a = true_data[SVP_CAIRO_ALPHA];
                sal_uInt8 xor_a = xor_data[SVP_CAIRO_ALPHA];
                sal_uInt8 b = unpremultiply_table[a][true_data[SVP_CAIRO_BLUE]]
                              ^ unpremultiply_table[xor_a][xor_data[SVP_CAIRO_BLUE]];
                sal_uInt8 g = unpremultiply_table[a][true_data[SVP_CAIRO_GREEN]]
                              ^ unpremultiply_table[xor_a][xor_data[SVP_CAIRO_GREEN]];
                sal_uInt8 r = unpremultiply_table[a][true_data[SVP_CAIRO_RED]]
                              ^ unpremultiply_table[xor_a][xor_data[SVP_CAIRO_RED]];
                true_data[SVP_CAIRO_BLUE] = premultiply_table[a][b];
                true_data[SVP_CAIRO_GREEN] = premultiply_table[a][g];
                true_data[SVP_CAIRO_RED] = premultiply_table[a][r];
                true_data += 4;
                xor_data += 4;
            }
        }
        cairo_surface_mark_dirty(target_surface);

        if (target_surface != m_pSurface)
        {
            cairo_t* copycr = cairo_create(m_pSurface);
            //unlikely case we couldn't use m_pSurface directly, copy contents
            //back from image surface
            cairo_rectangle(copycr, nExtentsLeft, nExtentsTop, nExtentsRight - nExtentsLeft,
                            nExtentsBottom - nExtentsTop);
            cairo_set_source_surface(copycr, target_surface, 0, 0);
            cairo_paint(copycr);
            cairo_destroy(copycr);
            cairo_surface_destroy(target_surface);
        }

        cairo_surface_destroy(surface);
    }

    cairo_destroy(cr); // unref

    DamageHandler* pDamage
        = static_cast<DamageHandler*>(cairo_surface_get_user_data(m_pSurface, getDamageKey()));

    if (pDamage)
    {
        pDamage->damaged(pDamage->handle, nExtentsLeft, nExtentsTop, nExtentsRight - nExtentsLeft,
                         nExtentsBottom - nExtentsTop);
    }
}

cairo_t* CairoCommon::createTmpCompatibleCairoContext() const
{
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
    cairo_surface_t* target = cairo_surface_create_similar_image(
        m_pSurface,
#else
    cairo_surface_t* target = cairo_image_surface_create(
#endif
        CAIRO_FORMAT_ARGB32, m_aFrameSize.getX() * m_fScale, m_aFrameSize.getY() * m_fScale);

    dl_cairo_surface_set_device_scale(target, m_fScale, m_fScale);

    return cairo_create(target);
}

void CairoCommon::applyColor(cairo_t* cr, Color aColor, double fTransparency)
{
    if (cairo_surface_get_content(m_pSurface) == CAIRO_CONTENT_COLOR_ALPHA)
    {
        cairo_set_source_rgba(cr, aColor.GetRed() / 255.0, aColor.GetGreen() / 255.0,
                              aColor.GetBlue() / 255.0, 1.0 - fTransparency);
    }
    else
    {
        double fSet = aColor == COL_BLACK ? 1.0 : 0.0;
        cairo_set_source_rgba(cr, 1, 1, 1, fSet);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    }
}

void CairoCommon::clipRegion(cairo_t* cr, const vcl::Region& rClipRegion)
{
    RectangleVector aRectangles;
    if (!rClipRegion.IsEmpty())
    {
        rClipRegion.GetRegionRectangles(aRectangles);
    }
    if (!aRectangles.empty())
    {
        for (auto const& rectangle : aRectangles)
        {
            cairo_rectangle(cr, rectangle.Left(), rectangle.Top(), rectangle.GetWidth(),
                            rectangle.GetHeight());
        }
        cairo_clip(cr);
    }
}

void CairoCommon::clipRegion(cairo_t* cr) { CairoCommon::clipRegion(cr, m_aClipRegion); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
