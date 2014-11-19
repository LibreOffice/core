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
#include <glm/gtc/type_ptr.hpp>
namespace oglcanvas
{
    class RenderHelper
    {
    public:

        void renderVertexConstColor(GLfloat vertices[], glm::vec4 color, GLenum mode) const;
        void renderVertexUVTex(GLfloat vertices[], GLfloat uvCoordinates[], glm::vec4 color, GLenum mode) const;
        void renderVertexTex(GLfloat vertices[], GLfloat, GLfloat, glm::vec4 color, GLenum mode) const;

        RenderHelper();

        void SetVP(int width, int height);
        void SetModelAndMVP(glm::mat4 mat);
        void dispose();
        void  InitOpenGL();

    private:
        RenderHelper&  operator = (const RenderHelper& other);
        RenderHelper(const RenderHelper& other);

        GLuint                                            m_vertexBuffer;
        GLuint                                            m_uvBuffer;


        GLuint                                            m_simpleProgID;
        GLuint                                            m_simpleUvAttrb;
        GLuint                                            m_simplePosAttrb;
        GLuint                                            m_simpleTexUnf;
        GLuint                                            m_simpleColorUnf;
        GLuint                                            m_simpleMVPUnf;

        GLuint                                            m_manTexUnf;
        GLuint                                            m_manPosAttrb;
        GLuint                                            m_manCordUnf;
        GLuint                                            m_manColorUnf;
        GLuint                                            m_manMVPUnf;

        GLuint                                            m_texPosAttrb;
        GLuint                                            m_texColorUnf;
        GLuint                                            m_texManProgID;
        GLuint                                            m_texProgID;
        GLuint                                            m_texMVPUnf;
        //dimension
        int                                               m_iWidth;
        int                                               m_iHeight;

        // Projection matrix : default 45 degree Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4                                         m_Projection;
        // Camera matrix
        glm::mat4                                         m_View;
        // Model matrix : an identity matrix (model will be at the origin
        glm::mat4                                         m_Model;
        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4                                         m_MVP;

    };
}

#endif /* RENDER_HELPER */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
