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

bool OpenGLGlobalWinGlyphCache::AllocateTexture(WinGlyphDrawElement& rElement, CompatibleDC* dc)
{
    assert(!rElement.maTexture);
    assert(dynamic_cast<OpenGLCompatibleDC*>(dc));
    OpenGLCompatibleDC* odc = static_cast<OpenGLCompatibleDC*>(dc);
    OpenGLCompatibleDC::Texture* texture = new OpenGLCompatibleDC::Texture;
    rElement.maTexture.reset(texture);
    texture->texture = maPackedTextureAtlas.Reserve(dc->getBitmapWidth(), dc->getBitmapHeight());
    if (!texture->texture)
        return false;
    if (!odc->copyToTexture(*rElement.maTexture))
        return false;
    return true;
}

void OpenGLGlobalWinGlyphCache::Prune()
{
    std::vector<GLuint> aTextureIDs = maPackedTextureAtlas.ReduceTextureNumber(8);
    if (!aTextureIDs.empty())
    {
        for (auto& pWinGlyphCache : maWinGlyphCaches)
            static_cast<OpenGLWinGlyphCache*>(pWinGlyphCache)->RemoveTextures(aTextureIDs);
    }
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
