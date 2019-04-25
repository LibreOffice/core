/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <unx/pixmap.hxx>
#include <ControlCacheKey.hxx>

class ControlCacheKey;

class X11GraphicsImpl
{
public:
    virtual ~X11GraphicsImpl() {};

    virtual void FillPixmapFromScreen( X11Pixmap* pPixmap, int nX, int nY ) = 0;
    virtual bool RenderPixmapToScreen( X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY ) = 0;

    virtual bool TryRenderCachedNativeControl(ControlCacheKey& rControlCacheKey, int nX, int nY) = 0;
    virtual bool RenderAndCacheNativeControl(X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY,
                                             ControlCacheKey& aControlCacheKey) = 0;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
