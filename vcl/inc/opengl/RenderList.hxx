/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_OPENGL_RENDERLIST_H
#define INCLUDED_VCL_INC_OPENGL_RENDERLIST_H

#include <unordered_map>

#include <glm/glm.hpp>

#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/salgtype.hxx>
#include <basegfx/range/b2drange.hxx>

#include "opengl/texture.hxx"

struct RenderParameters
{
    std::vector<GLfloat>   maVertices;
    std::vector<GLfloat>   maExtrusionVectors;
    std::vector<glm::vec4> maColors;
};

struct RenderTextureParameters
{
    std::vector<GLfloat>   maVertices;
    std::vector<glm::vec4> maColors;
    std::vector<GLfloat>   maTextureCoords;
    OpenGLTexture          maTexture;
};

struct RenderEntry
{
    RenderParameters maTriangleParameters;
    RenderParameters maLineParameters;
    RenderParameters maLineAAParameters;

    std::unordered_map<GLuint, RenderTextureParameters> maTextureParametersMap;

    bool hasTriangles()
    {
        return !maTriangleParameters.maVertices.empty();
    }

    bool hasLines()
    {
        return !maLineParameters.maVertices.empty();
    }

    bool hasLinesAA()
    {
        return !maLineAAParameters.maVertices.empty();
    }

    bool hasTextures()
    {
        return !maTextureParametersMap.empty();
    }
};

class RenderList
{
private:
    basegfx::B2DRange maOverlapTrackingRectangle;
    std::vector<RenderEntry> maRenderEntries;

    void checkOverlapping(const basegfx::B2DRange& rDrawRectangle)
    {
        if (maRenderEntries.empty() || maOverlapTrackingRectangle.overlaps(rDrawRectangle))
        {
            maRenderEntries.resize(maRenderEntries.size() + 1);
            maOverlapTrackingRectangle = rDrawRectangle;
        }
        else
        {
            maOverlapTrackingRectangle.expand(rDrawRectangle);
        }
    }

public:

    RenderList() = default;

    bool empty()
    {
        return maRenderEntries.empty();
    }

    void clear()
    {
        maRenderEntries.clear();
        maOverlapTrackingRectangle.reset();
    }

    std::vector<RenderEntry>& getEntries()
    {
        return maRenderEntries;
    }

    bool addDrawTextureWithMaskColor(OpenGLTexture& rTexture, const SalColor& rColor, const SalTwoRect& r2Rect);

    void addDrawPixel(long nX, long nY, const SalColor& rColor);

    void addDrawRectangle(long nX, long nY, long nWidth, long nHeight, double fTransparency,
                          const SalColor& rLineColor, const SalColor& rFillColor);

    void addDrawLine(long nX1, long nY1, long nX2, long nY2, const SalColor& rLineColor, bool bUseAA);

    void addDrawPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency,
                            const SalColor& rLineColor, const SalColor& rFillColor, bool bUseAA);
};

#endif // INCLUDED_VCL_INC_OPENGL_RENDERLIST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
