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

#include <skia/x11/gdiimpl.hxx>

// TODO
#define SK_ASSUME_GL 1
#define SK_ASSUME_GL_ES 0

#include <tools/sk_app/unix/WindowContextFactory_unix.h>
#include <tools/sk_app/WindowContext.h>

X11SkiaSalGraphicsImpl::X11SkiaSalGraphicsImpl(X11SalGraphics& rParent)
    : SkiaSalGraphicsImpl(rParent, rParent.GetGeometryProvider())
    , mParent(rParent)
{
}

X11SkiaSalGraphicsImpl::~X11SkiaSalGraphicsImpl() {}

void X11SkiaSalGraphicsImpl::Init()
{
    // The m_pFrame and m_pVDev pointers are updated late in X11
    setProvider(mParent.GetGeometryProvider());
    SkiaSalGraphicsImpl::Init();
}

void X11SkiaSalGraphicsImpl::createSurface()
{
    if (isOffscreen())
        return SkiaSalGraphicsImpl::createSurface();
    sk_app::DisplayParams displayParams;
    // TODO The Skia Xlib code actually requires the non-native color type to work properly.
    // Use a macro to hide an unreachable code warning.
#define GET_FORMAT                                                                                 \
    kN32_SkColorType == kBGRA_8888_SkColorType ? kRGBA_8888_SkColorType : kBGRA_8888_SkColorType
    displayParams.fColorType = GET_FORMAT;
#undef GET_FORMAT
    sk_app::window_context_factory::XlibWindowInfo winInfo;
    winInfo.fDisplay = mParent.GetXDisplay();
    winInfo.fWindow = mParent.GetDrawable();
    assert(winInfo.fDisplay && winInfo.fWindow != None);
    winInfo.fFBConfig = nullptr; // not used
    winInfo.fVisualInfo = const_cast<SalVisual*>(&mParent.GetVisual());
    winInfo.fWidth = GetWidth();
    winInfo.fHeight = GetHeight();
    mWindowContext.reset(sk_app::window_context_factory::NewRasterForXlib(winInfo, displayParams));
    assert(SkToBool(mWindowContext)); // TODO
    mSurface = mWindowContext->getBackbufferSurface();
}

void X11SkiaSalGraphicsImpl::DeInit()
{
    mWindowContext.reset();
    SkiaSalGraphicsImpl::DeInit();
}

void X11SkiaSalGraphicsImpl::freeResources() {}

void X11SkiaSalGraphicsImpl::performFlush()
{
    // TODO XPutImage() is somewhat inefficient, XShmPutImage() should be preferred.
    mWindowContext->swapBuffers();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
