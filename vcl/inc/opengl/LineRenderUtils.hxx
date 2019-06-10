/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_OPENGL_LINERENDERUTILS_H
#define INCLUDED_VCL_INC_OPENGL_LINERENDERUTILS_H

#include <opengl/RenderList.hxx>

namespace vcl
{
class LineBuilder
{
private:
    std::vector<Vertex>& mrVertices;
    std::vector<GLuint>& mrIndices;
    GLubyte mR, mG, mB, mA;
    GLfloat const mfLineWidth;
    GLfloat const mfLineWidthAndAA;
    size_t const mnInitialIndexSize;
    bool mbIncomplete;

public:
    LineBuilder(std::vector<Vertex>& rVertices, std::vector<GLuint>& rIndices,
                Color nColor, GLfloat fTransparency,
                GLfloat fLineWidth, bool bUseAA);

    void appendLineSegment(const glm::vec2& rPoint1, const glm::vec2& rNormal1, GLfloat aExtrusion1,
                           const glm::vec2& rPoint2, const glm::vec2& rNormal2, GLfloat aExtrusion2);

    void appendLine(const glm::vec2& rPoint1, const glm::vec2& rPoint2);

    void appendAndConnectLinePoint(const glm::vec2& rPoint, const glm::vec2& aNormal, GLfloat aExtrusion);

    void appendMiterJoint(glm::vec2 const& point, const glm::vec2& prevLineVector,
                          glm::vec2 const& nextLineVector);
    void appendBevelJoint(glm::vec2 const& point, const glm::vec2& prevLineVector,
                          const glm::vec2& nextLineVector);
    void appendRoundJoint(glm::vec2 const& point, const glm::vec2& prevLineVector,
                          const glm::vec2& nextLineVector);
    void appendRoundLineCapVertices(const glm::vec2& rPoint1, const glm::vec2& rPoint2);
    void appendSquareLineCapVertices(const glm::vec2& rPoint1, const glm::vec2& rPoint2);
};

} // end vcl

#endif // INCLUDED_VCL_INC_OPENGL_LINERENDERUTILS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
