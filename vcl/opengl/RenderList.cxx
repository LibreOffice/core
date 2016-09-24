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
#include "opengl/LineRenderUtils.hxx"

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dtrapezoid.hxx>

namespace
{

/** Append vertices for the polyline
 *
 * OpenGL polyline drawing algorithm inspired by:
 * - http://mattdesl.svbtle.com/drawing-lines-is-hard
 * - https://www.mapbox.com/blog/drawing-antialiased-lines/
 * - https://cesiumjs.org/2013/04/22/Robust-Polyline-Rendering-with-WebGL/
 * - http://artgrammer.blogspot.si/2011/05/drawing-nearly-perfect-2d-line-segments.html
 * - http://artgrammer.blogspot.si/2011/07/drawing-polylines-by-tessellation.html
 *
 */
void appendPolyLine(vcl::LineBuilder& rBuilder, const basegfx::B2DPolygon& rPolygon,
                    basegfx::B2DLineJoin eLineJoin, css::drawing::LineCap eLineCap,
                    double fMiterMinimumAngle)
{
    sal_uInt32 nPoints = rPolygon.count();
    bool bClosed = rPolygon.isClosed();

    if (nPoints == 2 || eLineJoin == basegfx::B2DLineJoin::NONE)
    {
        // If line joint is NONE or a simple line with 2 points, draw the polyline
        // each line segment separatly.

        for (sal_uInt32 i = 0; i < (bClosed ? nPoints : nPoints - 1); ++i)
        {
            sal_uInt32 index1 = (i + 0) % nPoints; // loop indices - important when polyline is closed
            sal_uInt32 index2 = (i + 1) % nPoints;

            glm::vec2 aPoint1(rPolygon.getB2DPoint(index1).getX(), rPolygon.getB2DPoint(index1).getY());
            glm::vec2 aPoint2(rPolygon.getB2DPoint(index2).getX(), rPolygon.getB2DPoint(index2).getY());

            rBuilder.appendLine(aPoint1, aPoint2);
        }
    }
    else if (nPoints > 2)
    {
        int i = 0;
        int lastPoint = int(nPoints);

        glm::vec2 p0(rPolygon.getB2DPoint(nPoints - 1).getX(), rPolygon.getB2DPoint(nPoints - 1).getY());
        glm::vec2 p1(rPolygon.getB2DPoint(0).getX(), rPolygon.getB2DPoint(0).getY());
        glm::vec2 p2(rPolygon.getB2DPoint(1).getX(), rPolygon.getB2DPoint(1).getY());

        glm::vec2 nextLineVector;
        glm::vec2 previousLineVector;
        glm::vec2 normal; // perpendicular to the line vector

        nextLineVector = vcl::vertex::normalize(p2 - p1);

        if (!bClosed)
        {
            normal = glm::vec2(-nextLineVector.y, nextLineVector.x); // make perpendicular
            rBuilder.appendAndConnectLinePoint(p1, normal, 1.0f);

            i++; // first point done already
            lastPoint--; // last point will be calculated separatly from the loop

            p0 = p1;
            previousLineVector = nextLineVector;
        }
        else
        {
            lastPoint++; // we need to connect last point to first point so one more line segment to calculate
            previousLineVector = vcl::vertex::normalize(p1 - p0);
        }

        for (; i < lastPoint; ++i)
        {
            int index1 = (i + 0) % nPoints; // loop indices - important when polyline is closed
            int index2 = (i + 1) % nPoints;

            p1 = glm::vec2(rPolygon.getB2DPoint(index1).getX(), rPolygon.getB2DPoint(index1).getY());
            p2 = glm::vec2(rPolygon.getB2DPoint(index2).getX(), rPolygon.getB2DPoint(index2).getY());

            if (p1 == p2) // skip equal points, normals could div-by-0
                continue;

            nextLineVector = vcl::vertex::normalize(p2 - p1);

            if (eLineJoin == basegfx::B2DLineJoin::Miter)
            {
                if (vcl::vertex::lineVectorAngle(previousLineVector, nextLineVector) < fMiterMinimumAngle)
                    rBuilder.appendBevelJoint(p1, previousLineVector, nextLineVector);
                else
                    rBuilder.appendMiterJoint(p1, previousLineVector, nextLineVector);
            }
            else if (eLineJoin == basegfx::B2DLineJoin::Bevel)
            {
                rBuilder.appendBevelJoint(p1, previousLineVector, nextLineVector);
            }
            else if (eLineJoin == basegfx::B2DLineJoin::Round)
            {
                rBuilder.appendRoundJoint(p1, previousLineVector, nextLineVector);
            }
            p0 = p1;
            previousLineVector = nextLineVector;
        }

        if (!bClosed)
        {
            // Create vertices for the last point. There is no line join so just
            // use the last line segment normal as the extrusion vector.
            p1 = glm::vec2(rPolygon.getB2DPoint(nPoints - 1).getX(), rPolygon.getB2DPoint(nPoints - 1).getY());
            normal = glm::vec2(-previousLineVector.y, previousLineVector.x);
            rBuilder.appendAndConnectLinePoint(p1, normal, 1.0f);
        }
    }

    if (!bClosed && nPoints >= 2 && (eLineCap == css::drawing::LineCap_ROUND || eLineCap == css::drawing::LineCap_SQUARE))
    {
        glm::vec2 aBeginCapPoint1(rPolygon.getB2DPoint(0).getX(), rPolygon.getB2DPoint(0).getY());
        glm::vec2 aBeginCapPoint2(rPolygon.getB2DPoint(1).getX(), rPolygon.getB2DPoint(1).getY());

        glm::vec2 aEndCapPoint1(rPolygon.getB2DPoint(nPoints - 1).getX(), rPolygon.getB2DPoint(nPoints - 1).getY());
        glm::vec2 aEndCapPoint2(rPolygon.getB2DPoint(nPoints - 2).getX(), rPolygon.getB2DPoint(nPoints - 2).getY());

        if (eLineCap == css::drawing::LineCap_ROUND)
        {
            rBuilder.appendRoundLineCapVertices(aBeginCapPoint1, aBeginCapPoint2);
            rBuilder.appendRoundLineCapVertices(aEndCapPoint1, aEndCapPoint2);
        }
        else if (eLineCap == css::drawing::LineCap_SQUARE)
        {
            rBuilder.appendSquareLineCapVertices(aBeginCapPoint1, aBeginCapPoint2);
            rBuilder.appendSquareLineCapVertices(aEndCapPoint1, aEndCapPoint2);
        }
    }
}

inline void appendTrapezoid(std::vector<Vertex>& rVertices, std::vector<GLuint>& rIndices,
                       GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2,
                       GLfloat x3, GLfloat y3, GLfloat x4, GLfloat y4,
                       SalColor nColor, GLfloat fTransparency)
{
    GLubyte nR, nG, nB, nA;
    vcl::vertex::createColor(nColor, fTransparency, nR, nG, nB, nA);

    GLuint zero = rVertices.size();

    rVertices.insert(rVertices.end(), {
        Vertex{glm::vec2{x1, y1}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
        Vertex{glm::vec2{x2, y2}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
        Vertex{glm::vec2{x3, y3}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
        Vertex{glm::vec2{x4, y4}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
    });

    rIndices.insert(rIndices.end(), {
        zero + 0, zero + 1, zero + 2,
        zero + 2, zero + 1, zero + 3
    });
}

void appendRectangle(std::vector<Vertex>& rVertices, std::vector<GLuint>& rIndices,
                     GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2,
                     SalColor nColor, GLfloat fTransparency)
{
    GLubyte nR, nG, nB, nA;
    vcl::vertex::createColor(nColor, fTransparency, nR, nG, nB, nA);

    GLuint zero = rVertices.size();

    rVertices.insert(rVertices.end(), {
        Vertex{glm::vec2{x1, y1}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
        Vertex{glm::vec2{x2, y1}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
        Vertex{glm::vec2{x1, y2}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
        Vertex{glm::vec2{x2, y2}, glm::vec4{nR, nG, nB, nA}, glm::vec4{0.0f, 0.0f, 0.0f, 0.0f}},
    });

    rIndices.insert(rIndices.end(), {
        zero + 0, zero + 1, zero + 2,
        zero + 2, zero + 1, zero + 3
    });
}

} // end anonymous namespace

void RenderList::addDrawPixel(long nX, long nY, SalColor nColor)
{
    if (nColor == SALCOLOR_NONE)
        return;

    checkOverlapping(basegfx::B2DRange(nX, nY, nX, nY));

    RenderParameters& rRenderParameter = maRenderEntries.back().maTriangleParameters;
    appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                    nX - 0.5f, nY - 0.5f, nX + 0.5f, nY + 0.5f, nColor, 0.0f);
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
        appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                    fX1 - 0.5f, fY1 - 0.5f, fX1 + 0.5f, fY2 + 0.5f, nLineColor, fTransparency);
        appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                    fX1 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY1 + 0.5f, nLineColor, fTransparency);
        appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                    fX2 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY2 + 0.5f, nLineColor, fTransparency);
        appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                    fX1 - 0.5f, fY2 - 0.5f, fX2 + 0.5f, fY2 + 0.5f, nLineColor, fTransparency);
    }

    if (nFillColor != SALCOLOR_NONE)
    {
        if (nLineColor == SALCOLOR_NONE)
        {
            appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                        fX1 - 0.5f, fY1 - 0.5f, fX1 + 0.5f, fY2 + 0.5f, nFillColor, fTransparency);
            appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                        fX1 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY1 + 0.5f, nFillColor, fTransparency);
            appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                        fX2 - 0.5f, fY1 - 0.5f, fX2 + 0.5f, fY2 + 0.5f, nFillColor, fTransparency);
            appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                        fX1 - 0.5f, fY2 - 0.5f, fX2 + 0.5f, fY2 + 0.5f, nFillColor, fTransparency);
        }
        // Draw rectangle fill with fill color
        appendRectangle(rRenderParameter.maVertices, rRenderParameter.maIndices,
                        fX1 + 0.5f, fY1 + 0.5f, fX2 - 0.5f, fY2 - 0.5f, nFillColor, fTransparency);
    }
}

void RenderList::addDrawLine(long nX1, long nY1, long nX2, long nY2, SalColor nLineColor, bool bUseAA)
{
    if (nLineColor == SALCOLOR_NONE)
        return;

    checkOverlapping(basegfx::B2DRange(nX1, nY1, nX2, nY2));

    RenderParameters& rRenderParameter = maRenderEntries.back().maLineParameters;

    glm::vec2 aPoint1(nX1, nY1);
    glm::vec2 aPoint2(nX2, nY2);

    vcl::LineBuilder aBuilder(rRenderParameter.maVertices, rRenderParameter.maIndices, nLineColor, 0.0f, 1.0f, bUseAA);
    aBuilder.appendLine(aPoint1, aPoint2);
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

    RenderParameters& rLineRenderParameter = maRenderEntries.back().maLineParameters;
    RenderParameters& rTriangleRenderParameter = maRenderEntries.back().maTriangleParameters;

    if (nFillColor != SALCOLOR_NONE)
    {
        basegfx::B2DTrapezoidVector aTrapezoidVector;
        basegfx::tools::trapezoidSubdivide(aTrapezoidVector, rPolyPolygon);

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

                appendTrapezoid(rTriangleRenderParameter.maVertices, rTriangleRenderParameter.maIndices,
                                topX1,    topY, topX2,    topY,
                                bottomX1, bottomY, bottomX2, bottomY,
                                nFillColor, fTransparency);
            }
        }
    }

    if (nLineColor != SALCOLOR_NONE || bUseAA)
    {
        SalColor nColor = (nLineColor == SALCOLOR_NONE) ? nFillColor : nLineColor;

        vcl::LineBuilder aBuilder(rLineRenderParameter.maVertices, rLineRenderParameter.maIndices,
                                  nColor, fTransparency, 1.0f, bUseAA);

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

                aBuilder.appendLine(glm::vec2(x1, y1), glm::vec2(x2, y2));
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

void RenderList::addDrawPolyLine(const basegfx::B2DPolygon& rPolygon, double fTransparency,
                                 const basegfx::B2DVector& rLineWidth, basegfx::B2DLineJoin eLineJoin,
                                 css::drawing::LineCap eLineCap, double fMiterMinimumAngle,
                                 SalColor nLineColor, bool bUseAA)
{
    if (rPolygon.count() <= 1)
        return;
    if (nLineColor == SALCOLOR_NONE)
        return;
    if (fTransparency == 1.0)
        return;

    const bool bIsHairline = (rLineWidth.getX() == rLineWidth.getY()) && (rLineWidth.getX() <= 1.2);
    const float fLineWidth = bIsHairline ? 1.0f : rLineWidth.getX();

    basegfx::B2DPolygon aPolygon(rPolygon);
    if (rPolygon.areControlPointsUsed())
        aPolygon = rPolygon.getDefaultAdaptiveSubdivision();

    checkOverlapping(aPolygon.getB2DRange());

    RenderParameters& rParameter = maRenderEntries.back().maLineParameters;

    vcl::LineBuilder aBuilder(rParameter.maVertices, rParameter.maIndices,
                              nLineColor, fTransparency, fLineWidth, bUseAA);

    appendPolyLine(aBuilder, aPolygon, eLineJoin, eLineCap, fMiterMinimumAngle);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
