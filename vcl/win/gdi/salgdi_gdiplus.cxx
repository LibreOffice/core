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

#include <string.h>
#include <svsys.h>
#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salgdi.h>
#include <win/salbmp.h>

#include "gdiimpl.hxx"

bool WinSalGraphics::drawPolyPolygon(
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolyPolygon& rPolyPolygon,
    double fTransparency)
{
    return mpImpl->drawPolyPolygon(
        rObjectToDevice,
        rPolyPolygon,
        fTransparency);
}

bool WinSalGraphics::drawPolyLine(
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolygon& rPolygon,
    double fTransparency,
    double fLineWidth,
    const std::vector< double >* pStroke, // MM01
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap,
    double fMiterMinimumAngle,
    bool bPixelSnapHairline)
{
    return mpImpl->drawPolyLine(
        rObjectToDevice,
        rPolygon,
        fTransparency,
        fLineWidth,
        pStroke, // MM01
        eLineJoin,
        eLineCap,
        fMiterMinimumAngle,
        bPixelSnapHairline);
}

bool WinSalGraphics::blendBitmap(
    const SalTwoRect& rTR,
    const SalBitmap& rBmp)
{
    return mpImpl->blendBitmap(rTR, rBmp);
}

bool WinSalGraphics::blendAlphaBitmap(
    const SalTwoRect& rTR,
    const SalBitmap& rSrcBmp,
    const SalBitmap& rMaskBmp,
    const SalBitmap& rAlphaBmp)
{
    return mpImpl->blendAlphaBitmap(rTR, rSrcBmp, rMaskBmp, rAlphaBmp);
}

bool WinSalGraphics::drawAlphaBitmap(
    const SalTwoRect& rTR,
    const SalBitmap& rSrcBitmap,
    const SalBitmap& rAlphaBmp)
{
    return mpImpl->drawAlphaBitmap(rTR, rSrcBitmap, rAlphaBmp);
}

bool WinSalGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY,
    const SalBitmap& rSourceBitmap,
    const SalBitmap* pAlphaBitmap,
    double fAlpha)
{
    return mpImpl->drawTransformedBitmap(rNull, rX, rY,
            rSourceBitmap, pAlphaBitmap, fAlpha);
}

bool WinSalGraphics::hasFastDrawTransformedBitmap() const
{
    return mpImpl->hasFastDrawTransformedBitmap();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
