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
    if (!texture->bitmap.tryAllocN32Pixels(nWidth, nHeight))
        return false;
    // TODO prune cache
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
