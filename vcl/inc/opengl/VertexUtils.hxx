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

#include <glm/gtx/norm.hpp>

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

inline glm::vec4 createGLColor(const SalColor& rColor, GLfloat rTransparency)
{
    return glm::vec4(SALCOLOR_RED(rColor)   / 255.0f,
                     SALCOLOR_GREEN(rColor) / 255.0f,
                     SALCOLOR_BLUE(rColor)  / 255.0f,
                     1.0f - rTransparency);
}

template<GLenum TYPE>
inline void addQuadColors(std::vector<glm::vec4>& rColors, const SalColor& rColor, GLfloat rTransparency);

template<>
inline void addQuadColors<GL_TRIANGLES>(std::vector<glm::vec4>& rColors, const SalColor& rColor, GLfloat rTransparency)
{
    glm::vec4 color = createGLColor(rColor, rTransparency);

    rColors.insert(rColors.end(), {
        color, color, color,
        color, color, color
    });
}

template<GLenum TYPE>
inline void addQuadEmptyExtrusionVectors(std::vector<GLfloat>& rExtrusions);

template<>
inline void addQuadEmptyExtrusionVectors<GL_TRIANGLES>(std::vector<GLfloat>& rExtrusions)
{
    rExtrusions.insert(rExtrusions.end(), {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
    });
}

inline void addLineVertex(std::vector<GLfloat>& rVertices, std::vector<GLfloat>& rExtrusionVectors, glm::vec2 point, glm::vec2 extrusionVector, float length)
{
    rVertices.push_back(point.x);
    rVertices.push_back(point.y);

    rExtrusionVectors.push_back(extrusionVector.x);
    rExtrusionVectors.push_back(extrusionVector.y);
    rExtrusionVectors.push_back(length);
}

inline void addLineVertexPair(std::vector<GLfloat>& rVertices, std::vector<GLfloat>& rExtrusionVectors, const glm::vec2& point, const glm::vec2& extrusionVector, float length)
{
    addLineVertex(rVertices, rExtrusionVectors, point, -extrusionVector, -length);
    addLineVertex(rVertices, rExtrusionVectors, point,  extrusionVector,  length);
}

inline void addLinePointFirst(std::vector<GLfloat>& rVertices, std::vector<GLfloat>& rExtrusionVectors,
                               glm::vec2 point, glm::vec2 extrusionVector, float length)
{
    addLineVertex(rVertices, rExtrusionVectors, point, -extrusionVector, -length);
    addLineVertex(rVertices, rExtrusionVectors, point,  extrusionVector,  length);
}

inline void addLinePointNext(std::vector<GLfloat>& rVertices, std::vector<GLfloat>& rExtrusionVectors,
                             glm::vec2 prevPoint, glm::vec2 prevExtrusionVector, float prevLength,
                             glm::vec2 currPoint, glm::vec2 currExtrusionVector, float currLength)
{
    addLineVertex(rVertices, rExtrusionVectors, currPoint, -currExtrusionVector, -currLength);
    addLineVertex(rVertices, rExtrusionVectors, currPoint, -currExtrusionVector, -currLength);
    addLineVertex(rVertices, rExtrusionVectors, prevPoint,  prevExtrusionVector,  prevLength);
    addLineVertex(rVertices, rExtrusionVectors, currPoint,  currExtrusionVector,  currLength);
}

inline glm::vec2 normalize(const glm::vec2& vector)
{
    if (glm::length(vector) > 0.0)
        return glm::normalize(vector);
    return vector;
}

}} // end vcl::vertex

#endif // INCLUDED_VCL_INC_OPENGL_VERTEXUTILS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
