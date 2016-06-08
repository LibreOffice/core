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

void RenderList::addDrawPixel(long nX, long nY, SalColor nColor)
{
    if (nColor == SALCOLOR_NONE)
        return;

    checkOverlapping(basegfx::B2DRange(nX, nY, nX, nY));

    RenderParameters& rRenderParameter = maRenderEntries.back().maTriangleParameters;
    vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, nX - 0.5f, nY - 0.5f, nX + 0.5f, nY + 0.5f);
    vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, nColor, 0.0f);
    vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
}

void RenderList::addDrawRectangle(long nX, long nY, long nWidth, long nHeight, double fTransparency,
                                  SalColor nLineColor, SalColor nFillColor)
{
    if (nLineColor == SALCOLOR_NONE && nFillColor == SALCOLOR_NONE)
        return;
    if (fTransparency == 1.0f)
        return;

    GLfloat fX1(nX);
    GLfloat fY1(nY);
    GLfloat fX2(nX + nWidth  - 1);
    GLfloat fY2(nY + nHeight - 1);

    checkOverlapping(basegfx::B2DRange(fX1, fY1, fX2, fY2));

    RenderParameters& rRenderParameter = maRenderEntries.back().maTriangleParameters;

    // Draw rectangle stroke with line color
    if (nLineColor != SALCOLOR_NONE)
    {
        vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 - 0.5f, fY1 - 0.5f, fX1 + 0.5f, fY2 + 0.5f);
        vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY1 + 0.5f);
        vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX2 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY2 + 0.5f);
        vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 - 0.5f, fY2 - 0.5f, fX2 + 0.5f, fY2 + 0.5f);

        vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, nLineColor, fTransparency);
        vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, nLineColor, fTransparency);
        vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, nLineColor, fTransparency);
        vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, nLineColor, fTransparency);

        vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
        vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
        vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
        vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
    }

    if (nFillColor != SALCOLOR_NONE)
    {
        if (nLineColor == SALCOLOR_NONE)
        {
            // Draw rectangle stroke with fill color
            vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 - 0.5f, fY1 - 0.5f, fX1 + 0.5f, fY2 + 0.5f);
            vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY1 + 0.5f);
            vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX2 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY2 + 0.5f);
            vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 - 0.5f, fY2 - 0.5f, fX2 + 0.5f, fY2 + 0.5f);

            vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, nFillColor, fTransparency);
            vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, nFillColor, fTransparency);
            vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, nFillColor, fTransparency);
            vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, nFillColor, fTransparency);

            vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
            vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
            vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
            vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
        }
        // Draw rectangle fill with fill color
        vcl::vertex::addRectangle<GL_TRIANGLES>(rRenderParameter.maVertices, fX1 + 0.5f, fY1 + 0.5f, fX2 - 0.5f, fY2 - 0.5f);
        vcl::vertex::addQuadColors<GL_TRIANGLES>(rRenderParameter.maColors, nFillColor, fTransparency);
        vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rRenderParameter.maExtrusionVectors);
    }
}

void RenderList::addDrawLine(long nX1, long nY1, long nX2, long nY2, SalColor nLineColor, bool bUseAA)
{
    if (nLineColor == SALCOLOR_NONE)
        return;

    checkOverlapping(basegfx::B2DRange(nX1, nY1, nX2, nY2));

    RenderParameters& rRenderParameter = bUseAA ? maRenderEntries.back().maLineAAParameters :
                                                  maRenderEntries.back().maLineParameters;
    lclAddLineSegmentVertices(rRenderParameter, nX1, nY1, nX2, nY2, nLineColor, 0.0f);
}

void RenderList::addDrawPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency,
                        SalColor nLineColor, SalColor nFillColor, bool bUseAA)
{
    if (rPolyPolygon.count() <= 0)
        return;
    if (nLineColor == SALCOLOR_NONE && nFillColor == SALCOLOR_NONE)
        return;
    if (fTransparency == 1.0)
        return;

    checkOverlapping(rPolyPolygon.getB2DRange());

    RenderParameters& rLineParameter = maRenderEntries.back().maLineParameters;
    RenderParameters& rLineAAParameter = maRenderEntries.back().maLineAAParameters;

    if (nFillColor != SALCOLOR_NONE)
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
                vcl::vertex::addQuadColors<GL_TRIANGLES>(rTriangleParameter.maColors, nFillColor, fTransparency);
                vcl::vertex::addQuadEmptyExtrusionVectors<GL_TRIANGLES>(rTriangleParameter.maExtrusionVectors);

                if (bUseAA)
                {
                    lclAddLineSegmentVertices(rLineAAParameter, topX1, topY, topX2, topY,
                                              nFillColor, fTransparency);
                    lclAddLineSegmentVertices(rLineAAParameter, topX2, topY, bottomX2, bottomY,
                                              nFillColor, fTransparency);
                    lclAddLineSegmentVertices(rLineAAParameter, bottomX2, bottomY, bottomX1, bottomY,
                                              nFillColor, fTransparency);
                    lclAddLineSegmentVertices(rLineAAParameter, bottomX1, bottomY, topX1, topY,
                                              nFillColor, fTransparency);
                }
            }
        }
    }

    if (nLineColor != SALCOLOR_NONE && nLineColor != nFillColor)
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

                lclAddLineSegmentVertices(rParameter, x1, y1, x2, y2, nLineColor, fTransparency);
            }
        }
    }
}

bool RenderList::addDrawTextureWithMaskColor(OpenGLTexture& rTexture, SalColor nColor, const SalTwoRect& r2Rect)
{
    if (!rTexture)
        return false;

    GLfloat fX1 = r2Rect.mnDestX;
    GLfloat fY1 = r2Rect.mnDestY;
    GLfloat fX2 = fX1 + r2Rect.mnDestWidth;
    GLfloat fY2 = fY1 + r2Rect.mnDestHeight;

    checkOverlapping(basegfx::B2DRange(fX1, fY1, fX2, fY2));

    GLuint nTextureId = rTexture.Id();

    RenderTextureParameters& rTextureParameter = maRenderEntries.back().maTextureParametersMap[nTextureId];
    rTextureParameter.maTexture = rTexture;

    rTexture.FillCoords<GL_TRIANGLES>(rTextureParameter.maTextureCoords, r2Rect, false);

    vcl::vertex::addRectangle<GL_TRIANGLES>(rTextureParameter.maVertices, fX1, fY1, fX2, fY2);
    vcl::vertex::addQuadColors<GL_TRIANGLES>(rTextureParameter.maColors, nColor, 0.0f);

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
