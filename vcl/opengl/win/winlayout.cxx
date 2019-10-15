/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <opengl/win/winlayout.hxx>

#include <opengl/win/gdiimpl.hxx>

bool OpenGLGlobalWinGlyphCache::AllocateTexture(WinGlyphDrawElement& rElement, int nWidth,
                                                int nHeight)
{
    assert(rElement.maTexture.get() == nullptr);
    OpenGLCompatibleDC::Texture* texture = new OpenGLCompatibleDC::Texture;
    rElement.maTexture.reset(texture);
    texture->texture = maPackedTextureAtlas.Reserve(nWidth, nHeight);
    if (!texture->texture)
        return false;
    std::vector<GLuint> aTextureIDs = maPackedTextureAtlas.ReduceTextureNumber(8);
    if (!aTextureIDs.empty())
    {
        for (auto& pWinGlyphCache : maWinGlyphCaches)
        {
            assert(dynamic_cast<OpenGLWinGlyphCache*>(pWinGlyphCache));
            static_cast<OpenGLWinGlyphCache*>(pWinGlyphCache)->RemoveTextures(aTextureIDs);
        }
    }
    return true;
}

void OpenGLWinGlyphCache::RemoveTextures(std::vector<GLuint>& rTextureIDs)
{
    auto it = maWinTextureCache.begin();

    while (it != maWinTextureCache.end())
    {
        assert(dynamic_cast<OpenGLCompatibleDC::Texture*>(it->second.maTexture.get()));
        GLuint nTextureID
            = static_cast<OpenGLCompatibleDC::Texture*>(it->second.maTexture.get())->texture.Id();

        if (std::find(rTextureIDs.begin(), rTextureIDs.end(), nTextureID) != rTextureIDs.end())
        {
            it = maWinTextureCache.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
