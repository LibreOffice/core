/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_OPENGL_ACCUMULATEDTEXTURES_H
#define INCLUDED_VCL_INC_OPENGL_ACCUMULATEDTEXTURES_H

#include <vcl/opengl/OpenGLHelper.hxx>

#include <o3tl/make_unique.hxx>
#include "opengl/texture.hxx"
#include <memory>

struct TextureDrawParameters
{
    std::vector<GLfloat> maVertices;
    std::vector<GLfloat> maTextureCoords;
    GLint getNumberOfVertices()
    {
        return maVertices.size() / 2;
    }
};

struct AccumulatedTexturesEntry
{
    OpenGLTexture maTexture;
    std::unordered_map<SalColor, TextureDrawParameters> maColorTextureDrawParametersMap;

    AccumulatedTexturesEntry(const OpenGLTexture& rTexture)
        : maTexture(rTexture)
    {}

    void insert(const OpenGLTexture& rTexture, const SalColor& aColor, const SalTwoRect& r2Rect)
    {
        TextureDrawParameters& aDrawParameters = maColorTextureDrawParametersMap[aColor];
        rTexture.FillCoords<GL_TRIANGLES>(aDrawParameters.maTextureCoords, r2Rect, false);

        GLfloat nX1 = r2Rect.mnDestX;
        GLfloat nY1 = r2Rect.mnDestY;
        GLfloat nX2 = r2Rect.mnDestX + r2Rect.mnDestWidth;
        GLfloat nY2 = r2Rect.mnDestY + r2Rect.mnDestHeight;

        auto& rVertices = aDrawParameters.maVertices;
        rVertices.push_back(nX1);
        rVertices.push_back(nY1);

        rVertices.push_back(nX2);
        rVertices.push_back(nY1);

        rVertices.push_back(nX1);
        rVertices.push_back(nY2);

        rVertices.push_back(nX1);
        rVertices.push_back(nY2);

        rVertices.push_back(nX2);
        rVertices.push_back(nY1);

        rVertices.push_back(nX2);
        rVertices.push_back(nY2);
    }
};

class AccumulatedTextures
{
private:
    typedef std::unordered_map<GLuint, std::unique_ptr<AccumulatedTexturesEntry>> AccumulatedTexturesMap;

    AccumulatedTexturesMap maEntries;

public:
    AccumulatedTextures()
    {}

    bool empty()
    {
        return maEntries.empty();
    }

    void clear()
    {
        maEntries.clear();
    }

    bool insert(OpenGLTexture& rTexture, const SalColor& aColor, const SalTwoRect& r2Rect)
    {
        if (!rTexture)
            return false;

        GLuint nTextureId = rTexture.Id();

        if (maEntries.find(nTextureId) == maEntries.end())
        {
            OpenGLTexture aWholeTexture(rTexture.GetWholeTexture());
            maEntries[nTextureId] = o3tl::make_unique<AccumulatedTexturesEntry>(aWholeTexture);
        }

        std::unique_ptr<AccumulatedTexturesEntry>& rEntry = maEntries[nTextureId];
        rEntry->insert(rTexture, aColor, r2Rect);

        return true;
    }

    AccumulatedTexturesMap& getAccumulatedTexturesMap()
    {
        return maEntries;
    }
};

#endif // INCLUDED_VCL_INC_OPENGL_TEXTURE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
