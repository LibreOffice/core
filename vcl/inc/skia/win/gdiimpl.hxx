/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_SKIA_WIN_GDIIMPL_HXX
#define INCLUDED_VCL_INC_SKIA_WIN_GDIIMPL_HXX

#include <memory>
#include <vcl/dllapi.h>

#include <skia/gdiimpl.hxx>
#include <win/salgdi.h>

class ControlCacheKey;
namespace sk_app
{
class WindowContext;
}

class WinSkiaSalGraphicsImpl : public SkiaSalGraphicsImpl
{
private:
    WinSalGraphics& mWinParent;

public:
    WinSkiaSalGraphicsImpl(WinSalGraphics& rGraphics, SalGeometryProvider* mpProvider);

    virtual void Init() override;
    virtual void DeInit() override;
    virtual void copyBits(const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics) override;

    virtual void freeResources() override;

    bool TryRenderCachedNativeControl(ControlCacheKey const& rControlCacheKey, int nX, int nY);

    bool RenderAndCacheNativeControl(OpenGLCompatibleDC& rWhite, OpenGLCompatibleDC& rBlack, int nX,
                                     int nY, ControlCacheKey& aControlCacheKey);

protected:
    virtual void createSurface() override;
    virtual void performFlush() override;

private:
    std::unique_ptr<sk_app::WindowContext> mWindowContext;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
