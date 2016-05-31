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
    basegfx::B2DRange maOverlapTrackingRectangle;

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
    std::vector<RenderEntry> maRenderEntries;
    std::vector<basegfx::B2DRange> maRectangles;

    bool doesOverlap(const basegfx::B2DRange& rDrawRectangle)
    {
        if (!maRenderEntries.back().maOverlapTrackingRectangle.overlaps(rDrawRectangle))
            return false;

        for (const basegfx::B2DRange& rRectangle : maRectangles)
        {
            if (rRectangle.overlaps(rDrawRectangle))
                return true;
        }
        return false;
    }

    void checkOverlapping(const basegfx::B2DRange& rDrawRectangle)
    {
        if (maRenderEntries.empty() || doesOverlap(rDrawRectangle))
        {
            maRenderEntries.resize(maRenderEntries.size() + 1);
            maRenderEntries.back().maOverlapTrackingRectangle = rDrawRectangle;

            maRectangles.clear();
            maRectangles.reserve(30);
            maRectangles.push_back(rDrawRectangle);
        }
        else
        {
            maRenderEntries.back().maOverlapTrackingRectangle.expand(rDrawRectangle);

            if (maRectangles.size() < 30)
            {
                maRectangles.push_back(rDrawRectangle);
            }
            else
            {
                basegfx::B2DRange aTempRectangle(maRectangles[0]);
                aTempRectangle.expand(rDrawRectangle);
                double minArea = aTempRectangle.getWidth() * aTempRectangle.getHeight();
                size_t index = 0;

                double area;
                for (size_t i = 1; i < maRectangles.size(); ++i)
                {
                    aTempRectangle = basegfx::B2DRange(maRectangles[i]);
                    aTempRectangle.expand(rDrawRectangle);
                    area = aTempRectangle.getWidth() * aTempRectangle.getHeight();
                    if (area < minArea)
                        index = i;
                }
                maRectangles[index].expand(rDrawRectangle);
            }
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
