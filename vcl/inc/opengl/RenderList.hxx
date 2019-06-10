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

#include <vcl/salgtype.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <opengl/texture.hxx>

#include <com/sun/star/drawing/LineCap.hpp>

struct Vertex
{
    glm::vec2 position;
    glm::vec4 color;
    glm::vec4 lineData;
};

static_assert(sizeof(Vertex) == (2*4 + 4*4 + 4*4), "Vertex struct has wrong size/alignment");


struct RenderParameters
{
    std::vector<Vertex> maVertices;
    std::vector<GLuint> maIndices;
};

struct RenderTextureParameters
{
    std::vector<GLfloat>   maVertices;
    std::vector<GLfloat>   maTextureCoords;
    std::vector<GLubyte>   maColors;
    OpenGLTexture          maTexture;
};

struct RenderEntry
{
    basegfx::B2DRange maOverlapTrackingRectangle;

    RenderParameters maTriangleParameters;
    RenderParameters maLineParameters;

    std::unordered_map<GLuint, RenderTextureParameters> maTextureParametersMap;

    bool hasTriangles()
    {
        return !maTriangleParameters.maVertices.empty();
    }

    bool hasLines()
    {
        return !maLineParameters.maVertices.empty();
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
            maRenderEntries.emplace_back();
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
                    aTempRectangle = maRectangles[i];
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

    void addDrawTextureWithMaskColor(OpenGLTexture const & rTexture, Color nColor, const SalTwoRect& r2Rect);

    void addDrawPixel(long nX, long nY, Color nColor);

    void addDrawRectangle(long nX, long nY, long nWidth, long nHeight, double fTransparency,
                          Color nLineColor, Color nFillColor);

    void addDrawLine(long nX1, long nY1, long nX2, long nY2, Color nLineColor, bool bUseAA);

    void addDrawPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency,
                            Color nLineColor, Color nFillColor, bool bUseAA);

    void addDrawPolyLine(const basegfx::B2DPolygon& rPolygon, double fTransparency,
                         const basegfx::B2DVector& rLineWidth, basegfx::B2DLineJoin eLineJoin,
                         css::drawing::LineCap eLineCap, double fMiterMinimumAngle,
                         Color nLineColor, bool bUseAA);
};

#endif // INCLUDED_VCL_INC_OPENGL_RENDERLIST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
