/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <opengl/LineRenderUtils.hxx>
#include <opengl/VertexUtils.hxx>

namespace vcl
{

LineBuilder::LineBuilder(std::vector<Vertex>& rVertices, std::vector<GLuint>& rIndices,
                Color nColor, GLfloat fTransparency,
                GLfloat fLineWidth, bool bUseAA)
    : mrVertices(rVertices)
    , mrIndices(rIndices)
    , mR(nColor.GetRed())
    , mG(nColor.GetGreen())
    , mB(nColor.GetBlue())
    , mA((1.0f - fTransparency) * 255.0f)
    , mfLineWidth(fLineWidth)
    , mfLineWidthAndAA(bUseAA ? fLineWidth : -fLineWidth)
    , mnInitialIndexSize(rIndices.size())
    , mbIncomplete(false)
{
}

void LineBuilder::appendLineSegment(const glm::vec2& rPoint1, const glm::vec2& rNormal1, GLfloat aExtrusion1,
                                    const glm::vec2& rPoint2, const glm::vec2& rNormal2, GLfloat aExtrusion2)
{
    GLuint zero = mrVertices.size();

    mrVertices.insert(mrVertices.end(), {
        {rPoint1, glm::vec4{mR, mG, mB, mA}, glm::vec4{-rNormal1.x, -rNormal1.y, -aExtrusion1, mfLineWidthAndAA}},
        {rPoint1, glm::vec4{mR, mG, mB, mA}, glm::vec4{ rNormal1.x,  rNormal1.y,  aExtrusion1, mfLineWidthAndAA}},
        {rPoint2, glm::vec4{mR, mG, mB, mA}, glm::vec4{-rNormal2.x, -rNormal2.y, -aExtrusion2, mfLineWidthAndAA}},
        {rPoint2, glm::vec4{mR, mG, mB, mA}, glm::vec4{ rNormal2.x,  rNormal2.y,  aExtrusion2, mfLineWidthAndAA}},
    });

    mrIndices.insert(mrIndices.end(), {
        zero + 0, zero + 1, zero + 2,
        zero + 2, zero + 1, zero + 3
    });

}

void LineBuilder::appendLine(const glm::vec2& rPoint1, const glm::vec2& rPoint2)
{
    glm::vec2 aLineVector = vcl::vertex::normalize(rPoint2 - rPoint1);
    glm::vec2 aNormal = vcl::vertex::perpendicular(aLineVector);

    appendLineSegment(rPoint1, aNormal, 1.0f,
                      rPoint2, aNormal, 1.0f);
}

void LineBuilder::appendAndConnectLinePoint(const glm::vec2& rPoint, const glm::vec2& aNormal, GLfloat aExtrusion)
{
    GLuint zero = mrVertices.size();

    mrVertices.insert(mrVertices.end(), {
        {rPoint, glm::vec4{mR, mG, mB, mA}, glm::vec4{-aNormal.x, -aNormal.y, -aExtrusion, mfLineWidthAndAA}},
        {rPoint, glm::vec4{mR, mG, mB, mA}, glm::vec4{ aNormal.x,  aNormal.y,  aExtrusion, mfLineWidthAndAA}},
    });

    if (mnInitialIndexSize == mrIndices.size())
    {
        mrIndices.insert(mrIndices.end(), {
            zero + 0, zero + 1
        });
        mbIncomplete = true;
    }
    else
    {
        if (mbIncomplete)
        {
            mrIndices.insert(mrIndices.end(), {
                                    zero + 0,
                zero + 0, zero - 1, zero + 1
            });
            mbIncomplete = false;
        }
        else
        {
            mrIndices.insert(mrIndices.end(), {
                zero - 2, zero - 1, zero + 0,
                zero + 0, zero - 1, zero + 1
            });
        }
    }
}

void LineBuilder::appendMiterJoint(glm::vec2 const& point, const glm::vec2& prevLineVector,
                                   glm::vec2 const& nextLineVector)
{
    // With miter join we calculate the extrusion vector by adding normals of
    // previous and next line segment. The vector shows the way but we also
    // need the length (otherwise the line will be deformed). Length factor is
    // calculated as dot product of extrusion vector and one of the normals.
    // The value we get is the inverse length (used in the shader):
    // length = line_width / dot(extrusionVector, normal)

    glm::vec2 normal(-prevLineVector.y, prevLineVector.x);

    glm::vec2 tangent = vcl::vertex::normalize(nextLineVector + prevLineVector);
    glm::vec2 extrusionVector(-tangent.y, tangent.x);
    GLfloat length = glm::dot(extrusionVector, normal);

    appendAndConnectLinePoint(point, extrusionVector, length);
}

void LineBuilder::appendBevelJoint(glm::vec2 const& point, const glm::vec2& prevLineVector,
                                   const glm::vec2& nextLineVector)
{
    // For bevel join we just add 2 additional vertices and use previous
    // line segment normal and next line segment normal as extrusion vector.
    // All the magic is done by the fact that we draw triangle strips, so we
    // cover the joins correctly.

    glm::vec2 prevNormal(-prevLineVector.y, prevLineVector.x);
    glm::vec2 nextNormal(-nextLineVector.y, nextLineVector.x);

    appendAndConnectLinePoint(point, prevNormal, 1.0f);
    appendAndConnectLinePoint(point, nextNormal, 1.0f);
}

void LineBuilder::appendRoundJoint(glm::vec2 const& point, const glm::vec2& prevLineVector,
                                   const glm::vec2& nextLineVector)
{
    // For round join we do a similar thing as in bevel, we add more intermediate
    // vertices and add normals to get extrusion vectors in the between the
    // both normals.

    // 3 additional extrusion vectors + normals are enough to make most
    // line joins look round. Ideally the number of vectors could be
    // calculated.

    glm::vec2 prevNormal(-prevLineVector.y, prevLineVector.x);
    glm::vec2 nextNormal(-nextLineVector.y, nextLineVector.x);

    glm::vec2 middle = vcl::vertex::normalize(prevNormal + nextNormal);
    glm::vec2 middleLeft  = vcl::vertex::normalize(prevNormal + middle);
    glm::vec2 middleRight = vcl::vertex::normalize(middle + nextNormal);

    appendAndConnectLinePoint(point, prevNormal, 1.0f);
    appendAndConnectLinePoint(point, middleLeft, 1.0f);
    appendAndConnectLinePoint(point, middle, 1.0f);
    appendAndConnectLinePoint(point, middleRight, 1.0f);
    appendAndConnectLinePoint(point, nextNormal, 1.0f);
}

void LineBuilder::appendRoundLineCapVertices(const glm::vec2& rPoint1, const glm::vec2& rPoint2)
{
    constexpr int nRoundCapIteration = 12;

    glm::vec2 lineVector = vcl::vertex::normalize(rPoint2 - rPoint1);
    glm::vec2 normal(-lineVector.y, lineVector.x);
    glm::vec2 previousRoundNormal = normal;

    for (int nFactor = 1; nFactor <= nRoundCapIteration; nFactor++)
    {
        float angle = float(nFactor) * (M_PI / float(nRoundCapIteration));
        glm::vec2 roundNormal(normal.x * glm::cos(angle) - normal.y * glm::sin(angle),
                              normal.x * glm::sin(angle) + normal.y * glm::cos(angle));

        appendLineSegment(rPoint1, previousRoundNormal, 1.0f,
                          rPoint1, roundNormal, 1.0f);
        previousRoundNormal = roundNormal;
    }
}

void LineBuilder::appendSquareLineCapVertices(const glm::vec2& rPoint1, const glm::vec2& rPoint2)
{
    glm::vec2 lineVector = vcl::vertex::normalize(rPoint2 - rPoint1);
    glm::vec2 normal(-lineVector.y, lineVector.x);

    glm::vec2 extrudedPoint = rPoint1 + -lineVector * (mfLineWidth / 2.0f);

    appendLineSegment(extrudedPoint, normal, 1.0f,
                      rPoint1,       normal, 1.0f);
}

} // end vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
