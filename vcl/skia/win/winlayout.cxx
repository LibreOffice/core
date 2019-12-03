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

bool SkiaGlobalWinGlyphCache::AllocateTexture(WinGlyphDrawElement& rElement, CompatibleDC* dc)
{
    assert(rElement.maTexture.get() == nullptr);
    assert(dynamic_cast<SkiaCompatibleDC*>(dc));
    SkiaCompatibleDC* sdc = static_cast<SkiaCompatibleDC*>(dc);
    SkiaCompatibleDC::Texture* texture = new SkiaCompatibleDC::Texture;
    rElement.maTexture.reset(texture);
    // TODO is it possible to have an atlas?
    texture->image = sdc->getAsImage();
    mLRUOrder.push_back(texture->image->uniqueID());
    return true;
}

void SkiaGlobalWinGlyphCache::Prune()
{
    const int MAXSIZE = 64; // TODO
    if (mLRUOrder.size() > MAXSIZE)
    {
        size_t toRemove = mLRUOrder.size() - MAXSIZE;
        std::vector<uint32_t> idsToRemove(mLRUOrder.begin(), mLRUOrder.begin() + toRemove);
        mLRUOrder.erase(mLRUOrder.begin(), mLRUOrder.begin() + toRemove);
        for (auto& pWinGlyphCache : maWinGlyphCaches)
            static_cast<SkiaWinGlyphCache*>(pWinGlyphCache)->RemoveTextures(idsToRemove);
    }
}

void SkiaGlobalWinGlyphCache::NotifyElementUsed(WinGlyphDrawElement& rElement)
{
    SkiaCompatibleDC::Texture* texture
        = static_cast<SkiaCompatibleDC::Texture*>(rElement.maTexture.get());
    // make the most recently used
    auto it = find(mLRUOrder.begin(), mLRUOrder.end(), texture->image->uniqueID());
    if (it != mLRUOrder.end())
        mLRUOrder.erase(it);
    mLRUOrder.push_back(texture->image->uniqueID());
}

void SkiaWinGlyphCache::RemoveTextures(const std::vector<uint32_t>& idsToRemove)
{
    auto it = maWinTextureCache.begin();
    while (it != maWinTextureCache.end())
    {
        assert(dynamic_cast<SkiaCompatibleDC::Texture*>(it->second.maTexture.get()));
        uint32_t id = static_cast<SkiaCompatibleDC::Texture*>(it->second.maTexture.get())
                          ->image->uniqueID();
        if (std::find(idsToRemove.begin(), idsToRemove.end(), id) != idsToRemove.end())
            it = maWinTextureCache.erase(it);
        else
            ++it;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
