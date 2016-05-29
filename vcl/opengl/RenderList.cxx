/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "opengl/RenderList.hxx"
#include "opengl/VertexUtils.hxx"

namespace
{

inline void lclAddLineSegmentVertices(RenderParameters& rRenderParameter, GLfloat fX1, GLfloat fY1, GLfloat fX2, GLfloat fY2,
                                   const SalColor& rColor, double fTransparency)
{
    glm::vec2 aPoint1(fX1, fY1);
    glm::vec2 aPoint2(fX2, fY2);

    glm::vec2 aLineVector = vcl::vertex::normalize(aPoint2 - aPoint1);
    glm::vec2 aNormal = glm::vec2(-aLineVector.y, aLineVector.x);

    vcl::vertex::addLinePointFirst(rRenderParameter.maVertices, rRenderParameter.maExtrusionVectors,
                                   aPoint1, aNormal, 1.0f);
    vcl::vertex::addLinePointNext (rRenderParameter.maVertices, rRenderParameter.maExtrusionVectors,
                                   aPoint1, aNormal, 1.0f,
                                   aPoint2, aNormal, 1.0f);

    vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, rColor, fTransparency);
}

} // end anonymous namespace

void RenderList::addDrawPixel(long nX, long nY, const SalColor& rColor)
{
    if (rColor == SALCOLOR_NONE)
        return;

    checkOverlapping(basegfx::B2DRange(nX, nY, nX, nY));

    RenderParameters& rRenderParameter = maRenderEntries.back().maTriangleParameters;
    vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, nX - 0.5f, nY - 0.5f, nX + 0.5f, nY + 0.5f);
    vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, rColor, 0.0f);
    vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
}

void RenderList::addDrawRectangle(long nX, long nY, long nWidth, long nHeight, const SalColor& rLineColor, const SalColor& rFillColor)
{
    if (rLineColor == SALCOLOR_NONE && rFillColor == SALCOLOR_NONE)
        return;

    GLfloat fX1(nX);
    GLfloat fY1(nY);
    GLfloat fX2(nX + nWidth  - 1);
    GLfloat fY2(nY + nHeight - 1);

    checkOverlapping(basegfx::B2DRange(fX1, fY1, fX2, fY2));

    RenderParameters& rRenderParameter = maRenderEntries.back().maTriangleParameters;

    // Draw rectangle stroke with line color
    if (rLineColor != SALCOLOR_NONE)
    {
        vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 - 0.5f, fY1 - 0.5f, fX1 + 0.5f, fY2 + 0.5f);
        vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY1 + 0.5f);
        vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX2 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY2 + 0.5f);
        vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 - 0.5f, fY2 - 0.5f, fX2 + 0.5f, fY2 + 0.5f);

        vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, rLineColor, 0.0f);
        vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, rLineColor, 0.0f);
        vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, rLineColor, 0.0f);
        vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, rLineColor, 0.0f);

        vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
        vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
        vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
        vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
    }

    if (rFillColor != SALCOLOR_NONE)
    {
        if (rLineColor == SALCOLOR_NONE)
        {
            // Draw rectangle stroke with fill color
            vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 - 0.5f, fY1 - 0.5f, fX1 + 0.5f, fY2 + 0.5f);
            vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY1 + 0.5f);
            vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX2 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY2 + 0.5f);
            vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 - 0.5f, fY2 - 0.5f, fX2 + 0.5f, fY2 + 0.5f);

            vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, rFillColor, 0.0f);
            vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, rFillColor, 0.0f);
            vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, rFillColor, 0.0f);
            vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, rFillColor, 0.0f);

            vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
            vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
            vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
            vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
        }
        // Draw rectangle fill with fill color
        vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 + 0.5f, fY1 + 0.5f, fX2 - 0.5f, fY2 - 0.5f);
        vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, rFillColor, 0.0f);
        vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
    }
}

void RenderList::addDrawLine(long nX1, long nY1, long nX2, long nY2, const SalColor& rLineColor, bool bUseAA)
{
    if (rLineColor == SALCOLOR_NONE)
        return;

    checkOverlapping(basegfx::B2DRange(nX1, nY1, nX2, nY2));

    RenderParameters& rRenderParameter = bUseAA ? maRenderEntries.back().maLineAAParameters :
                                                  maRenderEntries.back().maLineParameters;
    lclAddLineSegmentVertices(rRenderParameter, nX1, nY1, nX2, nY2, rLineColor, 0.0f);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
