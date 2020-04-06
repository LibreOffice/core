/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <skia/win/winlayout.hxx>

#include <skia/win/gdiimpl.hxx>

#include <SkCanvas.h>
#include <SkPaint.h>

bool SkiaGlobalWinGlyphCache::AllocateTexture(WinGlyphDrawElement& rElement, CompatibleDC* dc)
{
    assert(rElement.maTexture.get() == nullptr);
    assert(dynamic_cast<SkiaCompatibleDC*>(dc));
    SkiaCompatibleDC* sdc = static_cast<SkiaCompatibleDC*>(dc);
    SkiaCompatibleDC::PackedTexture* texture = new SkiaCompatibleDC::PackedTexture;
    rElement.maTexture.reset(texture);
    texture->packedSurface
        = mPackedSurfaceAtlas.Reserve(sdc->getBitmapWidth(), sdc->getBitmapHeight());
    if (!texture->packedSurface.mSurface)
        return false;
    // Draw the dc's content to the reserved place in the atlas.
    SkCanvas* canvas = texture->packedSurface.mSurface->getCanvas();
    const tools::Rectangle& rect = texture->packedSurface.mRect;
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // copy as is
    canvas->drawImageRect(
        sdc->getAsImage(),
        SkRect::MakeXYWH(rect.getX(), rect.getY(), rect.GetWidth(), rect.GetHeight()), &paint);
    return true;
}

void SkiaGlobalWinGlyphCache::Prune()
{
    std::vector<sk_sp<SkSurface>> aSurfaces = mPackedSurfaceAtlas.ReduceSurfaceNumber(8);
    if (!aSurfaces.empty())
    {
        for (auto& pWinGlyphCache : maWinGlyphCaches)
            static_cast<SkiaWinGlyphCache*>(pWinGlyphCache)->RemoveSurfaces(aSurfaces);
    }
}

void SkiaWinGlyphCache::RemoveSurfaces(const std::vector<sk_sp<SkSurface>>& surfaces)
{
    auto it = maWinTextureCache.begin();
    while (it != maWinTextureCache.end())
    {
        assert(dynamic_cast<SkiaCompatibleDC::PackedTexture*>(it->second.maTexture.get()));
        sk_sp<SkSurface> surface
            = static_cast<SkiaCompatibleDC::PackedTexture*>(it->second.maTexture.get())
                  ->packedSurface.mSurface;
        if (std::find(surfaces.begin(), surfaces.end(), surface) != surfaces.end())
            it = maWinTextureCache.erase(it);
        else
            ++it;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
