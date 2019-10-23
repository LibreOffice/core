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

bool SkiaGlobalWinGlyphCache::AllocateTexture(WinGlyphDrawElement& rElement, int nWidth,
                                              int nHeight)
{
    assert(rElement.maTexture.get() == nullptr);
    SkiaCompatibleDC::Texture* texture = new SkiaCompatibleDC::Texture;
    rElement.maTexture.reset(texture);
    // TODO use something GPU-backed?
    // TODO is it possible to have an atlas?
    if (!texture->bitmap.tryAllocN32Pixels(nWidth, nHeight))
        return false;
    mLRUOrder.push_back(texture->bitmap.getPixels());
    return true;
}

void SkiaGlobalWinGlyphCache::Prune()
{
    const int MAXSIZE = 64; // TODO
    if (mLRUOrder.size() > MAXSIZE)
    {
        size_t toRemove = mLRUOrder.size() - MAXSIZE;
        std::vector<void*> pixelsToRemove(mLRUOrder.begin(), mLRUOrder.begin() + toRemove);
        mLRUOrder.erase(mLRUOrder.begin(), mLRUOrder.begin() + toRemove);
        for (auto& pWinGlyphCache : maWinGlyphCaches)
            static_cast<SkiaWinGlyphCache*>(pWinGlyphCache)->RemoveTextures(pixelsToRemove);
    }
}

void SkiaGlobalWinGlyphCache::NotifyElementUsed(WinGlyphDrawElement& rElement)
{
    SkiaCompatibleDC::Texture* texture
        = static_cast<SkiaCompatibleDC::Texture*>(rElement.maTexture.get());
    // make the most recently used
    auto it = find(mLRUOrder.begin(), mLRUOrder.end(), texture->bitmap.getPixels());
    if (it != mLRUOrder.end())
        mLRUOrder.erase(it);
    mLRUOrder.push_back(texture->bitmap.getPixels());
}

void SkiaWinGlyphCache::RemoveTextures(const std::vector<void*>& pixelsToRemove)
{
    auto it = maWinTextureCache.begin();
    while (it != maWinTextureCache.end())
    {
        assert(dynamic_cast<SkiaCompatibleDC::Texture*>(it->second.maTexture.get()));
        void* pixels = static_cast<SkiaCompatibleDC::Texture*>(it->second.maTexture.get())
                           ->bitmap.getPixels();
        if (std::find(pixelsToRemove.begin(), pixelsToRemove.end(), pixels) != pixelsToRemove.end())
            it = maWinTextureCache.erase(it);
        else
            ++it;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
