/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Some of this code is based on Skia source code, covered by the following
 * license notice (see readlicense_oo for the full license):
 *
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include <skia/osx/rastercontext.hxx>

#include <SkSurface.h>

AquaSkiaWindowContextRaster::AquaSkiaWindowContextRaster(int w, int h,
                                                         const sk_app::DisplayParams& params)
    : WindowContext(params)
{
    fWidth = w;
    fHeight = h;
    resize(w, h);
}

void AquaSkiaWindowContextRaster::resize(int w, int h)
{
    fWidth = w;
    fHeight = h;
    createSurface();
}

void AquaSkiaWindowContextRaster::setDisplayParams(const sk_app::DisplayParams& params)
{
    fDisplayParams = params;
}

void AquaSkiaWindowContextRaster::createSurface()
{
    SkImageInfo info = SkImageInfo::Make(fWidth, fHeight, fDisplayParams.fColorType,
                                         kPremul_SkAlphaType, fDisplayParams.fColorSpace);
    mSurface = SkSurface::MakeRaster(info, &fDisplayParams.fSurfaceProps);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
