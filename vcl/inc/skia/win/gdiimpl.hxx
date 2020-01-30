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
#include <skia/packedsurfaceatlas.hxx>
#include <win/salgdi.h>
#include <win/wingdiimpl.hxx>
#include <o3tl/lru_map.hxx>
#include <ControlCacheKey.hxx>
#include <svdata.hxx>

class ControlCacheKey;

class SkiaCompatibleDC : public CompatibleDC
{
public:
    SkiaCompatibleDC(SalGraphics& rGraphics, int x, int y, int width, int height);

    virtual std::unique_ptr<Texture> getAsMaskTexture() const override;

    virtual bool wantsTextColorWhite() const override { return true; }

    sk_sp<SkImage> getAsImage() const;
    sk_sp<SkImage> getAsMaskImage() const;
    sk_sp<SkImage> getAsImageDiff(const SkiaCompatibleDC& white) const;

    struct Texture;
    struct PackedTexture;
};

struct SkiaCompatibleDC::Texture : public CompatibleDC::Texture
{
    sk_sp<SkImage> image;
    virtual bool isValid() const { return image.get(); }
    virtual int GetWidth() const { return image->width(); }
    virtual int GetHeight() const { return image->height(); }
};

struct SkiaCompatibleDC::PackedTexture : public CompatibleDC::Texture
{
    SkiaPackedSurface packedSurface;
    virtual bool isValid() const { return packedSurface.mSurface.get(); }
    virtual int GetWidth() const { return packedSurface.mRect.GetWidth(); }
    virtual int GetHeight() const { return packedSurface.mRect.GetHeight(); }
};

class WinSkiaSalGraphicsImpl : public SkiaSalGraphicsImpl, public WinSalGraphicsImplBase
{
private:
    WinSalGraphics& mWinParent;

public:
    WinSkiaSalGraphicsImpl(WinSalGraphics& rGraphics, SalGeometryProvider* mpProvider);

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
    virtual void createWindowContext() override;
    virtual void performFlush() override;
};

typedef std::pair<ControlCacheKey, sk_sp<SkImage>> SkiaControlCachePair;
typedef o3tl::lru_map<ControlCacheKey, sk_sp<SkImage>, ControlCacheHashFunction>
    SkiaControlCacheType;

class SkiaControlsCache
{
    SkiaControlCacheType cache;

    SkiaControlsCache();

public:
    static SkiaControlCacheType& get();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
