/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef RENDER_HELPER
#define RENDER_HELPER
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "glm/gtx/transform.hpp"
#include <vector>
namespace oglcanvas
{
    class RenderHelper
    {
    public:

        void renderVertexConstColor(const std::vector<glm::vec2>& rVertices, const glm::vec4& vColor, GLenum mode) const;
        void renderVertexUVTex(const std::vector<glm::vec2>& rVertices, const std::vector<glm::vec2>& rUVcoords,
                               const glm::vec4& vColor, GLenum mode) const;
        void renderVertexTex(const std::vector<glm::vec2>& rVertices, GLfloat, GLfloat,
                             const glm::vec4& vColor, GLenum mode) const;
        void renderTextureTransform(const std::vector<glm::vec2>& rVertices, GLfloat fWidth,
                                    GLfloat fHeight, const glm::vec4& color, GLenum mode, const glm::mat4& transform) const;

        RenderHelper();

        void SetVP(const float width, const float height);
        void SetModelAndMVP(const glm::mat4& mat);
        void dispose();
        void  InitOpenGL();


    private:
        RenderHelper&  operator = (const RenderHelper& other);
        RenderHelper(const RenderHelper& other);
        void setupColorMVP(const unsigned int nProgramID, const glm::vec4& color) const;


        GLuint                                            m_vertexBuffer;
        GLuint                                            m_uvBuffer;

        GLuint                                            m_simpleProgID;
        GLuint                                            m_simpleTexUnf;

        GLuint                                            m_manTexUnf;
        GLuint                                            m_manCordUnf;

        GLuint                                            m_texManProgID;
        GLuint                                            m_texProgID;

        GLuint                                            m_texTransProgID;
        GLuint                                            m_transCordUnf;
        GLuint                                            m_transTexUnf;
        GLuint                                            m_transTexTransform;

        glm::mat4                                         m_VP;
        // Model matrix
        glm::mat4                                         m_Model;
        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4                                         m_MVP;

    };
}

#endif /* RENDER_HELPER */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
