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

X11SkiaSalGraphicsImpl::X11SkiaSalGraphicsImpl(X11SalGraphics& rParent)
    : SkiaSalGraphicsImpl(rParent, rParent.GetGeometryProvider())
    , mParent(rParent)
    , mCopyGc(None)
{
}

X11SkiaSalGraphicsImpl::~X11SkiaSalGraphicsImpl() {}

void X11SkiaSalGraphicsImpl::Init()
{
    // The m_pFrame and m_pVDev pointers are updated late in X11
    setProvider(mParent.GetGeometryProvider());
    SkiaSalGraphicsImpl::Init();
}

GC X11SkiaSalGraphicsImpl::getGC()
{
    if (mCopyGc == None)
    {
        XGCValues values;
        values.graphics_exposures = False;
        values.subwindow_mode = ClipByChildren;
        mCopyGc = XCreateGC(mParent.GetXDisplay(), mParent.GetDrawable(),
                            GCGraphicsExposures | GCSubwindowMode, &values);
    }
    return mCopyGc;
}

void X11SkiaSalGraphicsImpl::freeResources()
{
    if (mCopyGc != None)
    {
        XFreeGC(mParent.GetXDisplay(), mCopyGc);
        mCopyGc = None;
    }
}

void X11SkiaSalGraphicsImpl::performFlush()
{
    Display* dpy = mParent.GetXDisplay();
    Drawable drawable = mParent.GetDrawable();
    GC gc = getGC();
    SkPixmap pm;
    if (!mSurface->peekPixels(&pm))
        abort();
    int bitsPerPixel = pm.info().bytesPerPixel() * 8;
    XImage image;
    memset(&image, 0, sizeof(image));
    image.width = pm.width();
    image.height = pm.height();
    image.format = ZPixmap;
    image.data = (char*)pm.addr();
    image.byte_order = LSBFirst;
    image.bitmap_unit = bitsPerPixel;
    image.bitmap_bit_order = LSBFirst;
    image.bitmap_pad = bitsPerPixel;
    image.depth = 24;
    image.bytes_per_line = pm.rowBytes() - pm.width() * pm.info().bytesPerPixel();
    image.bits_per_pixel = bitsPerPixel;
    if (!XInitImage(&image))
        abort();
    // TODO XPutImage() is somewhat inefficient, XShmPutImage() should be preferred.
    XPutImage(dpy, drawable, gc, &image, 0, 0, 0, 0, pm.width(), pm.height());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
