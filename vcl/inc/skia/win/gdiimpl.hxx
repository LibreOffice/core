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
#include <win/wingdiimpl.hxx>
#include <o3tl/lru_map.hxx>
#include <ControlCacheKey.hxx>
#include <svdata.hxx>

class ControlCacheKey;
namespace sk_app
{
class WindowContext;
}

class SkiaCompatibleDC : public CompatibleDC
{
public:
    SkiaCompatibleDC(SalGraphics& rGraphics, int x, int y, int width, int height);

    virtual std::unique_ptr<Texture> getAsMaskTexture() override;

    virtual bool copyToTexture(Texture& aTexture) override;

    virtual bool wantsTextColorWhite() const override { return true; }

    SkBitmap getAsBitmap();
    SkBitmap getAsMaskBitmap();

    struct Texture;
};

struct SkiaCompatibleDC::Texture : public CompatibleDC::Texture
{
    SkBitmap bitmap; // TODO SkBitmap, SkSurface, SkImage?
    virtual bool isValid() const { return !bitmap.drawsNothing(); }
    virtual int GetWidth() const { return bitmap.width(); }
    virtual int GetHeight() const { return bitmap.height(); }
};

class WinSkiaSalGraphicsImpl : public SkiaSalGraphicsImpl, public WinSalGraphicsImplBase
{
private:
    WinSalGraphics& mWinParent;

public:
    WinSkiaSalGraphicsImpl(WinSalGraphics& rGraphics, SalGeometryProvider* mpProvider);

    virtual void Init() override;
    virtual void DeInit() override;

    virtual void freeResources() override;

    virtual bool UseRenderNativeControl() const override { return true; }
    virtual bool TryRenderCachedNativeControl(ControlCacheKey const& rControlCacheKey, int nX,
                                              int nY) override;
    virtual bool RenderAndCacheNativeControl(CompatibleDC& rWhite, CompatibleDC& rBlack, int nX,
                                             int nY, ControlCacheKey& aControlCacheKey) override;

    virtual bool UseTextDraw() const override { return true; }
    virtual void PreDrawText() override;
    virtual void PostDrawText() override;
    virtual void DrawTextMask(CompatibleDC::Texture* rTexture, Color nMaskColor,
                              const SalTwoRect& rPosAry) override;
    virtual void DeferredTextDraw(const CompatibleDC::Texture* pTexture, Color nMaskColor,
                                  const SalTwoRect& rPosAry) override;

protected:
    virtual void createSurface() override;
    virtual void performFlush() override;

private:
    std::unique_ptr<sk_app::WindowContext> mWindowContext;
};

typedef std::pair<ControlCacheKey, SkBitmap> SkiaControlCachePair;
typedef o3tl::lru_map<ControlCacheKey, SkBitmap, ControlCacheHashFunction> SkiaControlCacheType;

class SkiaControlsCache
{
    SkiaControlCacheType cache;

    SkiaControlsCache();

public:
    static SkiaControlCacheType& get();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
