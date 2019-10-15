/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_WIN_WINGDIIMPL_HXX
#define INCLUDED_VCL_INC_WIN_WINGDIIMPL_HXX

#include <win/salgdi.h>
#include <ControlCacheKey.hxx>

class ControlCacheKey;

// Base class for some functionality that OpenGL/Skia/GDI backends must each implement.
class WinSalGraphicsImplBase
{
public:
    virtual ~WinSalGraphicsImplBase(){};

    virtual bool TryRenderCachedNativeControl(ControlCacheKey const& rControlCacheKey, int nX,
                                              int nY)
        = 0;

    virtual bool RenderAndCacheNativeControl(CompatibleDC& rWhite, CompatibleDC& rBlack, int nX,
                                             int nY, ControlCacheKey& aControlCacheKey)
        = 0;

    // If true is returned, the following functions are used for text rendering.
    virtual bool UseTextDraw() const { return false; }
    virtual void PreDrawText() {}
    virtual void PostDrawText() {}
    virtual void DrawMask(CompatibleDC::Texture* /*rTexture*/, Color /*nMaskColor*/,
                          const SalTwoRect& /*rPosAry*/){};
    virtual void DeferredTextDraw(const CompatibleDC::Texture* /*pTexture*/, Color /*nMaskColor*/,
                                  const SalTwoRect& /*rPosAry*/){};
};

#endif // INCLUDED_VCL_INC_WIN_WINGDIIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
