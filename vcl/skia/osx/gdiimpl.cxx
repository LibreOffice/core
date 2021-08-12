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

#include <skia/osx/gdiimpl.hxx>

#include <skia/utils.hxx>
#include <skia/zone.hxx>

#include <skia/osx/rastercontext.hxx>

using namespace SkiaHelper;

AquaSkiaSalGraphicsImpl::AquaSkiaSalGraphicsImpl(AquaSalGraphics& rParent,
                                                 AquaSharedAttributes& rShared)
    : SkiaSalGraphicsImpl(rParent, rShared.mpFrame)
    , AquaGraphicsBackendBase(rShared)
{
}

AquaSkiaSalGraphicsImpl::~AquaSkiaSalGraphicsImpl() { DeInit(); }

void AquaSkiaSalGraphicsImpl::Init()
{
    // The m_pFrame and m_pVDev pointers are updated late in X11
    //    setProvider(mX11Parent.GetGeometryProvider());
    //    SkiaSalGraphicsImpl::Init();
}

void AquaSkiaSalGraphicsImpl::DeInit()
{
    SkiaZone zone;
    SkiaSalGraphicsImpl::DeInit();
    mWindowContext.reset();
}

void AquaSkiaSalGraphicsImpl::freeResources() {}

void AquaSkiaSalGraphicsImpl::createWindowContext(bool forceRaster)
{
    SkiaZone zone;
    sk_app::DisplayParams displayParams;
    displayParams.fColorType = kN32_SkColorType;
    RenderMethod renderMethod = forceRaster ? RenderRaster : renderMethodToUse();
    switch (renderMethod)
    {
        case RenderRaster:
            displayParams.fColorType = kBGRA_8888_SkColorType; // TODO
            mWindowContext.reset(
                new AquaSkiaWindowContextRaster(GetWidth(), GetHeight(), displayParams));
            break;
        case RenderVulkan:
            abort();
            break;
    }
}

//void AquaSkiaSalGraphicsImpl::Flush() { performFlush(); }

void AquaSkiaSalGraphicsImpl::performFlush()
{
    SkiaZone zone;
    flushDrawing();
    if (mWindowContext)
    {
        if (mDirtyRect.intersect(SkIRect::MakeWH(GetWidth(), GetHeight())))
            mWindowContext->swapBuffers(&mDirtyRect); // TODO
        mDirtyRect.setEmpty();
    }
}

std::unique_ptr<sk_app::WindowContext> createVulkanWindowContext(bool /*temporary*/)
{
    return nullptr;
}

void AquaSkiaSalGraphicsImpl::prepareSkia() { SkiaHelper::prepareSkia(createVulkanWindowContext); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
