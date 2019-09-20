/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <skia/x11/gdiimpl.hxx>

X11SkiaSalGraphicsImpl::X11SkiaSalGraphicsImpl(X11SalGraphics& rParent)
    : SkiaSalGraphicsImpl(rParent, rParent.GetGeometryProvider())
    , mrX11Parent(rParent)
{
}

X11SkiaSalGraphicsImpl::~X11SkiaSalGraphicsImpl() {}

void X11SkiaSalGraphicsImpl::Init()
{
    // The m_pFrame and m_pVDev pointers are updated late in X11
    setProvider(mrX11Parent.GetGeometryProvider());
    SkiaSalGraphicsImpl::Init();
}

void X11SkiaSalGraphicsImpl::copyBits(const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics)
{
    (void)rPosAry;
    (void)pSrcGraphics;
}

void X11SkiaSalGraphicsImpl::FillPixmapFromScreen(X11Pixmap* pPixmap, int nX, int nY)
{
    (void)pPixmap;
    (void)nX;
    (void)nY;
}

bool X11SkiaSalGraphicsImpl::RenderPixmapToScreen(X11Pixmap* pPixmap, X11Pixmap* pMask, int nX,
                                                  int nY)
{
    (void)pPixmap;
    (void)pMask;
    (void)nX;
    (void)nY;
    return false;
}

bool X11SkiaSalGraphicsImpl::RenderAndCacheNativeControl(X11Pixmap* pPixmap, X11Pixmap* pMask,
                                                         int nX, int nY,
                                                         ControlCacheKey& aControlCacheKey)
{
    (void)pPixmap;
    (void)pMask;
    (void)nX;
    (void)nY;
    (void)aControlCacheKey;
    return false;
}

bool X11SkiaSalGraphicsImpl::TryRenderCachedNativeControl(ControlCacheKey& rControlCacheKey, int nX,
                                                          int nY)
{
    (void)rControlCacheKey;
    (void)nX;
    (void)nY;
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
