/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_OPENGL_VERTEXUTILS_H
#define INCLUDED_VCL_INC_OPENGL_VERTEXUTILS_H

#include <basegfx/numeric/ftools.hxx>
#include <epoxy/gl.h>
#include <glm/gtx/norm.hpp>
#include <tools/color.hxx>
#include <vector>

namespace vcl
{
namespace vertex
{

template<GLenum TYPE>
inline void addRectangle(std::vector<GLfloat>& rVertices, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);

template<>
inline void addRectangle<GL_TRIANGLES>(std::vector<GLfloat>& rVertices, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    rVertices.insert(rVertices.end(), {
        x1, y1, x2, y1, x1, y2,
        x1, y2, x2, y1, x2, y2
    });
}

template<>
inline void addRectangle<GL_TRIANGLE_FAN>(std::vector<GLfloat>& rVertices, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    rVertices.insert(rVertices.end(), {
        x1, y2, x1, y1,
        x2, y1, x2, y2
    });
}

inline void createColor(Color nColor, GLfloat fTransparency, GLubyte& nR, GLubyte& nG, GLubyte& nB, GLubyte& nA)
{
    nR = nColor.GetRed();
    nG = nColor.GetGreen();
    nB = nColor.GetBlue();
    nA = (1.0f - fTransparency) * 255.0f;
}

template<GLenum TYPE>
inline void addQuadColors(std::vector<GLubyte>& rColors, Color nColor, GLfloat fTransparency);

template<>
inline void addQuadColors<GL_TRIANGLES>(std::vector<GLubyte>& rColors, Color nColor, GLfloat fTransparency)
{
    GLubyte nR, nG, nB, nA;
    createColor(nColor, fTransparency, nR, nG, nB, nA);

    rColors.insert(rColors.end(), {
        nR, nG, nB, nA,
        nR, nG, nB, nA,
        nR, nG, nB, nA,
        nR, nG, nB, nA,
        nR, nG, nB, nA,
        nR, nG, nB, nA,
    });
}

inline void addLineSegmentVertices(std::vector<GLfloat>& rVertices, std::vector<GLfloat>& rExtrusionVectors,
                                   glm::vec2 prevPoint, glm::vec2 prevExtrusionVector, GLfloat prevLength,
                                   glm::vec2 currPoint, glm::vec2 currExtrusionVector, GLfloat currLength)
{
    rVertices.insert(rVertices.end(), {
        prevPoint.x, prevPoint.y,
        prevPoint.x, prevPoint.y,
        currPoint.x, currPoint.y,
        currPoint.x, currPoint.y,
        prevPoint.x, prevPoint.y,
        currPoint.x, currPoint.y,
    });

    rExtrusionVectors.insert(rExtrusionVectors.end(), {
        -prevExtrusionVector.x, -prevExtrusionVector.y, -prevLength,
         prevExtrusionVector.x,  prevExtrusionVector.y,  prevLength,
        -currExtrusionVector.x, -currExtrusionVector.y, -currLength,
        -currExtrusionVector.x, -currExtrusionVector.y, -currLength,
         prevExtrusionVector.x,  prevExtrusionVector.y,  prevLength,
         currExtrusionVector.x,  currExtrusionVector.y,  currLength,
    });
}

inline glm::vec2 normalize(const glm::vec2& vector)
{
    if (glm::length(vector) > 0.0)
        return glm::normalize(vector);
    return vector;
}

inline glm::vec2 perpendicular(const glm::vec2& vector)
{
    return glm::vec2(-vector.y, vector.x);
}

inline float lineVectorAngle(const glm::vec2& previous, const glm::vec2& next)
{
    float angle = std::atan2(previous.x * next.y - previous.y * next.x,
                             previous.x * next.x + previous.y * next.y);

    return F_PI - std::fabs(angle);
}

}} // end vcl::vertex

#endif // INCLUDED_VCL_INC_OPENGL_VERTEXUTILS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
