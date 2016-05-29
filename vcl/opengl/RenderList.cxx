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

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dtrapezoid.hxx>

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

void RenderList::addDrawPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency,
                        const SalColor& rLineColor, const SalColor& rFillColor, bool bUseAA)
{
    if (rPolyPolygon.count() <= 0)
        return;
    if (rLineColor == SALCOLOR_NONE && rFillColor == SALCOLOR_NONE)
        return;
    if (fTransparency == 1.0)
        return;

    checkOverlapping(rPolyPolygon.getB2DRange());

    RenderParameters& rLineParameter = maRenderEntries.back().maLineParameters;
    RenderParameters& rLineAAParameter = maRenderEntries.back().maLineAAParameters;

    if (rFillColor != SALCOLOR_NONE)
    {
        RenderParameters& rTriangleParameter = maRenderEntries.back().maTriangleParameters;

        const basegfx::B2DPolyPolygon& aSimplePolyPolygon = ::basegfx::tools::solveCrossovers(rPolyPolygon);
        basegfx::B2DTrapezoidVector aTrapezoidVector;
        basegfx::tools::trapezoidSubdivide(aTrapezoidVector, aSimplePolyPolygon);

        if (!aTrapezoidVector.empty())
        {
            for (basegfx::B2DTrapezoid & rTrapezoid : aTrapezoidVector)
            {
                GLfloat topX1 = rTrapezoid.getTopXLeft();
                GLfloat topX2 = rTrapezoid.getTopXRight();
                GLfloat topY  = rTrapezoid.getTopY();

                GLfloat bottomX1 = rTrapezoid.getBottomXLeft();
                GLfloat bottomX2 = rTrapezoid.getBottomXRight();
                GLfloat bottomY  = rTrapezoid.getBottomY();

                vcl::vertex::addTrapezoid<GL_TRIANGLES>(rTriangleParameter.maVertices,
                                                         topX1,    topY,
                                                         topX2,    topY,
                                                         bottomX1, bottomY,
                                                         bottomX2, bottomY);
                vcl::vertex::addQuadColors<GL_TRIANGLES>(rTriangleParameter.maColors, rFillColor, fTransparency);
                vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rTriangleParameter.maExtrusionVectors);

                if (bUseAA)
                {
                    lclAddLineSegmentVertices(rLineAAParameter, topX1, topY, topX2, topY,
                                              rFillColor, fTransparency);
                    lclAddLineSegmentVertices(rLineAAParameter, topX2, topY, bottomX2, bottomY,
                                              rFillColor, fTransparency);
                    lclAddLineSegmentVertices(rLineAAParameter, bottomX2, bottomY, bottomX1, bottomY,
                                              rFillColor, fTransparency);
                    lclAddLineSegmentVertices(rLineAAParameter, bottomX1, bottomY, topX1, topY,
                                              rFillColor, fTransparency);
                }
            }
        }
    }

    if (rLineColor != SALCOLOR_NONE && rLineColor != rFillColor)
    {
        RenderParameters& rParameter = bUseAA ? rLineAAParameter : rLineParameter;

        for (const basegfx::B2DPolygon& rPolygon : rPolyPolygon)
        {
            basegfx::B2DPolygon aPolygon(rPolygon);
            if (rPolygon.areControlPointsUsed())
                aPolygon = rPolygon.getDefaultAdaptiveSubdivision();

            sal_uInt32 nPoints = aPolygon.count();

            GLfloat x1, y1, x2, y2;
            sal_uInt32 index1, index2;

            for (sal_uInt32 i = 0; i <= nPoints; ++i)
            {
                index1 = (i)     % nPoints;
                index2 = (i + 1) % nPoints;

                x1 = aPolygon.getB2DPoint(index1).getX();
                y1 = aPolygon.getB2DPoint(index1).getY();
                x2 = aPolygon.getB2DPoint(index2).getX();
                y2 = aPolygon.getB2DPoint(index2).getY();

                lclAddLineSegmentVertices(rParameter, x1, y1, x2, y2, rLineColor, fTransparency);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
